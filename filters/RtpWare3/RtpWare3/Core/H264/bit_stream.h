#pragma once
#pragma warning(disable: 4290)

class BitStream
{
public:
    BitStream(const char *d, unsigned int len);
    ~BitStream(void);

    enum SetPosMode {kAbsolutePos, kRelativePos};

    //bitλ�ò���
    unsigned int SetPos(unsigned int pos, SetPosMode mode = kAbsolutePos);
    unsigned int GetPos() const { return pos_; }
    bool Eos() const { return pos_ >= length_; }
    void Reset();

    //���ݶ�ȡ����
    char NextByte() const;
    char ReadByte();
    unsigned int NextBytesToUInt32(unsigned int n) const throw(unsigned int);
    unsigned int ReadBytesToUInt32(unsigned int n) throw(unsigned int);
    bool ReadNextBytes(void *buf, unsigned int n);

    bool NextBit() const;
    bool ReadBit();
    /*unsigned int NextBitsToUInt32(unsigned int n) const throw(unsigned int);
    unsigned int ReadBitsToUInt32(unsigned int n) throw(unsigned int);*/

    unsigned int ReadGolombCode() throw(unsigned int);

    //�ض�λ�õ�ָ���ȡ
    const char * MoveToNextNaluStart(); //�ƶ���ǰλ�õ���һ��NALU��ʼ�����Ҳ����򷵻�EndPtr
    const char * MoveToNaluEnd(); //�ƶ���ǰλ�õ�NALU��β�ĺ�һ���ַ�
    const char * BeginPtr() const; //��������������ʼ�ֽ�ָ��
    const char * EndPtr() const; //���������������һ���ֽڵ���һ���ֽ�ָ��
    const char * CurPtr() const; //�����������ĵ�ǰ�ֽ�ָ�룬��bitλ�����ֽ�

private:
    inline unsigned int NextBytePos() const;
    BitStream &operator=(const BitStream &) {};

private:
    const char *data_;
    const unsigned int length_; //in bits count
    unsigned int pos_; //��һ��Ҫ����bitλ��
};

