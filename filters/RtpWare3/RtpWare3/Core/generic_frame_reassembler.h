/************************************************************************/
/* ͨ�õ�rtp��֡��                                                      */
/* ���ص��ǣ�1������rtp����                                             */
/*           2���յ�markΪ1�İ�ʱ���֡�Ƿ�������                       */
/*           3��ÿ�ε���PopFrontFrameʱ������ǰ�������֡������֡ǰ���� */
/*              ��������֡������                                      */
/************************************************************************/

#ifndef GENERIC_FRAME_REASSEMBLER_H
#define GENERIC_FRAME_REASSEMBLER_H

#include "receive_interfaces.h"
#include "../Util/circular_buffer.h"
#include "log/Log.h"


class RtpPacketBuffer : public CCircularBuffer<IRtpPacket*>
{
public:
  RtpPacketBuffer(ILogger *logger);
  ~RtpPacketBuffer();

  //����һ������������ڻ�������Χ���򷵻ظ�ֵ�����򷵻�����λ��
  int Put(IRtpPacket* pkt);
  //�������ݵĽ���λ��
  unsigned int GetEndPos() { return end_pos_; }
  //��ȡ��һ������֡����������[begin, end]
  void GetFirstValidFrameInterval(int &begin, int &end);
  //�ͷ�ǰn��λ�õ����ݰ�
  void Free(unsigned int n);
  void Clear();
  bool Full();

private:
  unsigned int end_pos_; //��ǰ���һ��������λ�õ���һ��λ��

  ILogger *logger_;

};


class GenericFrameReassembler :
  public IFrameReassembler
{
public:
  GenericFrameReassembler(ILogger *logger);
  ~GenericFrameReassembler(void);

  virtual int PushRtpPacket( IRtpPacket *packet );
  virtual IFrame * PopFrontFrame();

protected:
  //��֡�㷨������Ϊrtp������ʼ�ͽ���λ�ã�����֡����
  virtual unsigned long ReassembleFrameData(
    unsigned short begin_pos, unsigned short end_pos);
  //�����֡�����е������Ƿ�Ϊͬ����
  virtual bool CheckSyncPoint();

protected:
  ILogger *logger_;

  static const unsigned short rtp_buffer_size_;
  RtpPacketBuffer rtp_packet_buffer_; //rtp�����򻺴�

  static const unsigned long reassemble_buffer_size_;
  unsigned char *reassemble_buffer_; //�������
  unsigned long reassembled_data_size_;

  unsigned int ssrc_;
  bool packet_dropped_;
  bool first_mark_bit_received_;

};


#endif