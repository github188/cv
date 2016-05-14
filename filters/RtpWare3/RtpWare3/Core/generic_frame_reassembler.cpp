#include "generic_frame_reassembler.h"
#include "media_frame.h"
#include "Log/Log.h"

//
// RtpPacketBuffer
//

RtpPacketBuffer::RtpPacketBuffer(ILogger *logger)
  : CCircularBuffer()
  , end_pos_(0)
  , logger_(logger)
{
}

RtpPacketBuffer::~RtpPacketBuffer()
{
  Destroy();
}

int RtpPacketBuffer::Put( IRtpPacket* packet )
{
  unsigned short seq = packet->GetSeqNumber();

  //����λ��
  unsigned short pos = 0; //��ʼΪ��һ��λ�ã����ͱ���Ϊunsigned short����seqһ�£�ԭ�����
  if ((*this)[0]) {
    unsigned short first_seq = (*this)[0]->GetSeqNumber();
    pos = seq - first_seq;
    //ע�⿩��rtp���кŻ�ѭ�������65535����������ȴû�п�������ѭ���������
    //д�����򣬿���unsigned short��0-65535�����ɶ��
  }

  //���λ����Ч��
  if (pos >= BufSize()) {
    LOG_ERROR_EX(logger_,"rtp buf overflow(packet arrived too late?), seq=%u, pos=%u", 
      seq, pos);
    return -1;
  }

  //��������
  if ((*this)[pos]) {
    //�������ݣ��ѵ����к���ȣ����ǲ��÷����ģ�̫�����ˣ����µ��滻
    LOG_WARN_EX(logger_, "rtp seq repetition, seq=%u", seq);
  }
  (*this)[pos] = packet;
  if (pos >= end_pos_) end_pos_ = pos + 1;
  
  return pos;
}

void RtpPacketBuffer::GetFirstValidFrameInterval( int &begin, int &end )
{
  begin = end = -1;
  bool lost = false;
  unsigned int cur_begin = 0;
  for (unsigned int i = 0; i < end_pos_; ++i) {
    if ((*this)[i] == nullptr) {
      lost = true; //��Ƕ���
    }
    else if ((*this)[i]->GetMarkbit()) {
      if (!lost) {
        //û�ж�����֡����
        begin = cur_begin;
        end = i;
        return;
      } else {
        //֡������������¿�ʼ���Ϊ��һ֡��ʼ����������
        cur_begin = i + 1;
        lost = false; //���ö������
      }
    }
  }
}

void RtpPacketBuffer::Free( unsigned int n )
{
  if (n > end_pos_) return;
  end_pos_ -= n; //�ȼ�����ѭ������ѭ��������n���
  while (n-- > 0) {
    delete (*this)[0];
    Pop();
  }
}

void RtpPacketBuffer::Clear()
{
  for (unsigned int i = 0; i < end_pos_; ++i) {
    delete (*this)[i];
  }
  end_pos_ = 0;
  CCircularBuffer::Clear();
}

bool RtpPacketBuffer::Full()
{
  if (BufSize() == end_pos_) {
    return true;
  }
  return false;
}


//
// GenericFrameReassembler
//

//��ʼ��������д�С����λ����
//1080p30fps��Ƶ������10Mbps���㣬ÿ��1.25MB��ÿ��MTU���1500B
//��1000����Լ������1s����Ƶ�����ݡ�ʵ�ʲ���ÿ��MTU����1500B��
//��1000��Ҳ�㹻�ˣ��ܻ���3֡�ͺã�����̫��֡�ᵼ���ӳ١�������
//����£�һֻ֡Ҫ����������ȡ���ˣ�ֻ�в������ĲŻᱻ����
const unsigned short GenericFrameReassembler::rtp_buffer_size_ = 1000;

//MTU���1500B��200�����㹻1080p��һ֡�ˣ�Ҳֻռ��150k�ڴ�
//����Ժ����4k��Ƶ����Ҫ������������������С
const unsigned long GenericFrameReassembler::reassemble_buffer_size_ = 1500 * 200;


GenericFrameReassembler::GenericFrameReassembler(ILogger *logger)
  : logger_(logger)
  , rtp_packet_buffer_(logger)
  , ssrc_(0)
  , packet_dropped_(true)
  , first_mark_bit_received_(false)
{
  rtp_packet_buffer_.Allocate(rtp_buffer_size_);
  reassemble_buffer_ = new unsigned char[reassemble_buffer_size_];
}


GenericFrameReassembler::~GenericFrameReassembler(void)
{
  if (reassemble_buffer_) {
    delete reassemble_buffer_;
    reassemble_buffer_ = nullptr;
  }
}

int GenericFrameReassembler::PushRtpPacket( IRtpPacket *packet )
{
  if (!packet) return PUSH_ERR;

  int result = 0;

  //δ�յ���һ��markbitʱ����ȷ����һ֡�Ƿ��������������а�
  if (!first_mark_bit_received_) {
    if (packet->GetMarkbit())  first_mark_bit_received_ = true;
    delete packet;
    return PUSH_PACKET_DROP;
  }

  //check ssrc
  if (0 == ssrc_) ssrc_ = packet->GetSSRC();
  else if (packet->GetSSRC() != ssrc_) {
      //ssrc�ı䣬˵�����Ͷ˿������������������йر���
    LOG_WARN_EX(logger_,"rtp ssrc differs, expected=%u, received=%u", ssrc_, packet->GetSSRC());
    rtp_packet_buffer_.Clear();
    ssrc_ = packet->GetSSRC();
    packet_dropped_ = true;
    first_mark_bit_received_ = true; //�����Ҫ���ڷ�ֹ��һ֡����������ssrc�ı�ʱһ���Ƿ��Ͷ����¿�ʼ���ͣ�֡�������ģ�������Ϊfalse
    result |= PUSH_SSRC_CHANGE;
  }

  //��黺�����Ƿ�����
  if (rtp_packet_buffer_.Full()) {
    unsigned int n = (rtp_packet_buffer_.BufSize() > 10 ? rtp_packet_buffer_.BufSize() / 10 : 1);
    LOG_WARN_EX(logger_, "rtp buffer is full, free %u packets", n);
    rtp_packet_buffer_.Free(n); //�ͷ�10%λ��
    packet_dropped_ = true; //��¼����
    result |= PUSH_PACKET_DROP;
  }

  //put packet to buffer
  int pos = rtp_packet_buffer_.Put(packet);
  if (pos < 0) {
    //����������������쳣�����û����
    rtp_packet_buffer_.Clear();
    packet_dropped_ = true; //��¼����
    first_mark_bit_received_ = false; //���¿�ʼ����֡
    LOG_WARN_EX(logger_, "reset rtp packet buffer, maybe a lot of packets lost");
    return PUSH_ERR;
  }

  if (packet->GetMarkbit() || pos != rtp_packet_buffer_.GetEndPos() - 1) {
    //���յ�����֡������ǵİ������յ�һ���������İ�ʱ��Ӧ���֡������
    result |= PUSH_CHK_FRAME;
  }
  return result;
}

IFrame * GenericFrameReassembler::PopFrontFrame()
{
  int begin_pos, end_pos;
  rtp_packet_buffer_.GetFirstValidFrameInterval(begin_pos, end_pos);
  if (begin_pos == -1) return nullptr;

  const unsigned long buf_size_need = (end_pos-begin_pos+1) * 1500;
  if (buf_size_need > reassemble_buffer_size_) {
    LOG_ERROR_EX(logger_,"reassemble buf insufficient, need=%uB, actual=%uB", 
      buf_size_need, reassemble_buffer_size_);
    return nullptr;
  }

  //��֡���������ԣ�ֱ��ƴ��
  unsigned long data_size = ReassembleFrameData(begin_pos, end_pos);

  //��¼��1������һ֡��ÿ��������markbit������һ����
  IRtpPacket *first_pkt = rtp_packet_buffer_[begin_pos];
  IFrame *frame = nullptr;
  if (first_pkt) {
    frame = new MediaFrame;
    if (frame) {
      frame->SetData(reassemble_buffer_, data_size);
      frame->SetTimestamp(first_pkt->GetTimestamp());
      frame->SetSyncPoint(CheckSyncPoint());

      //begin_pos��Ϊ0ʱ����ζ��ǰ���в�����֡
      //����Ĳ����ǣ���������֡��������ǰ�治������֡
      if (begin_pos == 0 && !packet_dropped_) frame->SetDiscontinuity(false);
      else {
        frame->SetDiscontinuity(true);
        LOG_WARN_EX(logger_, "some packets lost, ignore %d packets", begin_pos);
      }

      //���ö������
      packet_dropped_ = false;
    } else {
      LOG_ERROR_EX(logger_,"alloc media frame failed, maybe memory leak?");
    }
  } else {
    LOG_ERROR_EX(logger_,"cannot retrieve timestamp, null rtp packet");
  }

  //�ͷ�rtp��
  rtp_packet_buffer_.Free(end_pos + 1); //end_pos�Ǳ������Ͻ磬�ǵ�+1

  return frame;
}

unsigned long GenericFrameReassembler::ReassembleFrameData( 
  unsigned short begin_pos, unsigned short end_pos )
{
  unsigned char *buf_tail = reassemble_buffer_;
  reassembled_data_size_ = 0;

  for (unsigned short i = begin_pos; i <= end_pos; ++i) {
    IRtpPacket *pkt = rtp_packet_buffer_[i];
    if (pkt) {
      memcpy_s(buf_tail, reassemble_buffer_size_ - reassembled_data_size_,
        pkt->GetPayload(), pkt->GetPlayloadSize());
      buf_tail += pkt->GetPlayloadSize();
      reassembled_data_size_ += pkt->GetPlayloadSize();
    } else {
      //���������ﲻ����null�����������Ҫ��¼�쳣
      LOG_WARN_EX(logger_, "null pkt found when reassembling frame");
    }
  }
  return reassembled_data_size_;
}

bool GenericFrameReassembler::CheckSyncPoint()
{
  //Ĭ��һ�ɷ���true
  return true;
}
