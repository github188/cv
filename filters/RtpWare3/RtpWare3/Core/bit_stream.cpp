#include "bit_stream.h"

#include <assert.h>
#include <memory.h>


BitStream::BitStream(const char *d, unsigned int len)
  : data_(d)
  , length_(len)
  , pos_(0)
{
}


BitStream::~BitStream(void)
{
}

unsigned int BitStream::SetPos( unsigned int new_pos, SetPosMode mode/*= kAbsolutePos*/ )
{
  if (kRelativePos == mode) new_pos = pos_ + new_pos;
  return (this->pos_ = new_pos); //�ⲿ������Ӧ���Eos����
}

void BitStream::Reset()
{
  pos_ = 0;
}

//����ǰpos�����ֽڶ���ģ���pos���뵽��һ���ֽ�
unsigned int BitStream::NextBytePos() const
{
  return ((pos_ - 1) & (~7)) + 8;
}

char BitStream::NextByte() const
{
  unsigned int new_pos = NextBytePos();
  if (new_pos >= length_)
  {
    return 0;
  }
  return data_[new_pos>>3];
}

char BitStream::ReadByte()
{
  pos_ = NextBytePos();
  if (pos_ >= length_)
  {
    return 0;
  }
  char c = data_[pos_>>3];
  pos_+=8;
  return c; //���ز�ָ����һ��λ��
}

unsigned int BitStream::NextBytesToUInt32( unsigned int n ) const
  throw(unsigned int)
{
  if (n < 1 || n > 4) {
    throw n;
  }

  unsigned int byte_pos = NextBytePos() >> 3;
  if (byte_pos + n > length_ >> 3) {
    n = (length_ - pos_) >> 3;
  }

  union 
  {
    char a[4];
    unsigned int i;
  } u;

  u.i = 0;
  const char *d = data_ + byte_pos;
#ifdef ENDIAN_LITTLE
  for (unsigned int i = 0; i < n; ++i) {
    u.a[i] = d[n-i-1];
  }
#else
  for (unsigned int i = 0; i < n; ++i) {
    u.a[i] = d[i];
  }
#endif

  return u.i;
}

unsigned int BitStream::ReadBytesToUInt32( unsigned int n ) 
  throw(unsigned int)
{
  unsigned int i = NextBytesToUInt32(n);
  pos_ = NextBytePos() + (n << 3);
  return i;
}

bool BitStream::ReadNextBytes( void *buf, unsigned int n )
{
  unsigned int new_pos = NextBytePos();
  if (nullptr == buf || new_pos + (n << 3) > length_) {
    return false;
  }
  memcpy(buf, data_ + (new_pos>>3), n);
  pos_ = new_pos + (n << 3);
  return true;
}

bool BitStream::NextBit() const
{
  if (pos_ >= length_) return false;
  unsigned int off = pos_ % 8; //�ֽ���ƫ��
  char c = data_[pos_>>3];
  return (c & (1 << (7-off))) > 0;
}

bool BitStream::ReadBit()
{
  bool b = NextBit();
  ++pos_;
  return b;
}

unsigned int BitStream::ReadGolombCode()
{
  unsigned int g = 1;
  unsigned int prefix_zero = 0;

  while (!Eos() && !ReadBit()) ++prefix_zero;
  if (0 == prefix_zero) return 0; //��ǰ׺��ʾ0
  while (prefix_zero-- > 0) {
    if (Eos()) throw 1; //bad coding
    g <<= 1;
    if (ReadBit()) g |= 1;
  }
  return --g;
}

//ʹ�ô˷������NALU�Ŀ�ʼ�룬�����Ч��
const char * BitStream::MoveToNextNaluStart()
{
  unsigned int byte_pos = NextBytePos() >> 3;
  unsigned int byte_len = length_ >> 3;
  while (byte_pos < byte_len - 3) { //������ֽڳ���0x000001���������
    if (data_[byte_pos] == 0x00 && 
      data_[byte_pos+1] == 0x00 && 
      data_[byte_pos+2] == 0x01) {
      pos_ = (byte_pos+3) << 3;
      return BeginPtr() + byte_pos;
    }
    ++byte_pos;
  }
  pos_ = length_; //set end
  return EndPtr();
}

const char * BitStream::MoveToNaluEnd()
{
  unsigned int byte_pos = NextBytePos() >> 3;
  unsigned int byte_len = length_ >> 3;
  //����0x000000��0x000001,�μ�ISO14496-10 Annex B.2
  while (byte_pos < byte_len - 3) {
    if (data_[byte_pos] == 0x00 && 
      data_[byte_pos+1] == 0x00 && 
      (data_[byte_pos+2] == 0x01 || data_[byte_pos+2] == 0x00)) {
        pos_ = (byte_pos) << 3;
        return BeginPtr() + byte_pos;
    }
    ++byte_pos;
  }
  pos_ = length_; //set end
  return EndPtr();
}

const char * BitStream::BeginPtr() const {
  return data_;
}

const char * BitStream::EndPtr() const {
  return data_ + (length_ >> 3);
}

const char * BitStream::CurPtr() const {
  if (pos_ >= length_) return EndPtr();
  return data_ + (pos_ >> 3);
}
