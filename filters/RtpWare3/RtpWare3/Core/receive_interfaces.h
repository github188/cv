/************************************************************************/
/* �˴��������������ģ��Ľӿ�                                         */
/************************************************************************/

#ifndef RECEIVE_INTERFACES_H
#define RECEIVE_INTERFACES_H

#include "core_defs.h"
#include "RtpStat.h"
#include <string>

#define USE_SR_NTP

class IRtpPacket
{
public:
  virtual ~IRtpPacket() {}

  virtual bool GetMarkbit() = 0;
  virtual unsigned int GetSSRC() = 0;
  virtual unsigned int GetTimestamp() = 0;
  virtual unsigned short GetSeqNumber() = 0;
  virtual unsigned short GetPayloadType() = 0;
  virtual unsigned short GetPlayloadSize() = 0;
  virtual unsigned char *GetPayload() = 0;
};


class IFrame
{
public:
  virtual ~IFrame() {}

  virtual bool IsSyncPoint() = 0; //�Ƿ�ͬ����(�ؼ�֡)
  virtual void SetSyncPoint(bool) = 0;
  virtual bool IsDiscontinuity() = 0;
  virtual void SetDiscontinuity(bool) = 0;
  virtual unsigned long GetTimestamp() = 0;
  virtual void SetTimestamp(unsigned long) = 0;
  virtual unsigned long GetDataSize() = 0;
  virtual unsigned char *GetData() = 0;
  virtual void SetData(unsigned char *, unsigned long) = 0;
};


//
//rtp��������Ŀǰֻ��ortpʵ��
//
class IRtpReceiver
{
public:
  virtual ~IRtpReceiver() {}

  virtual int InitSession(std::string ip, unsigned short port, RtpPayloadType payload_type) = 0;
  virtual int DestroySession() = 0;
  virtual void ResetSession() = 0;

  virtual int Receive(IRtpPacket *&packet) = 0;
  virtual int GetStatistics(RtpStatItem &) = 0;
  virtual void GetNTPReferenceTimestamp(unsigned long long &ntp_ts, unsigned long &ts) = 0;

};


//
//��֡�������ݲ�ͬý�����ͣ�������Ҫ��ͬ����֡����
//
class IFrameReassembler
{
public:
  virtual ~IFrameReassembler() {}

  //���һ��rtp�������У�����ֵ>0Ӧ����PopFrontFrame����Ƿ�������֡��<0��ʾ����
  virtual int PushRtpPacket(IRtpPacket *packet) = 0;
  //�����ǰ��֡������������֡ʱ����nullptr
  virtual IFrame *PopFrontFrame() = 0;

public:
  static const int PUSH_ERR = -1;
  static const int PUSH_CHK_FRAME = 1;
  static const int PUSH_SSRC_CHANGE = 2;
  static const int PUSH_PACKET_DROP = 4;
};


//
//���Ͳ������������IFrameDeliverer
//
struct IMediaSample;

class IDeliverStrategy
{
public:
  virtual ~IDeliverStrategy() {}

  virtual bool ShouldFrameDiscard(IFrame &frame) = 0;
  virtual int ConvertFrameToSample(IFrame &frame, IMediaSample &) = 0;
  virtual bool ShouldSampleDeliver(IMediaSample &) = 0;
  virtual void SetDiscontinuity() = 0;
  //����RTPʱ�����NTPʱ���Ӧ��ϵ
  virtual void SetNTPReferenceTimestamp(unsigned long long ntp_ts, unsigned long ts) = 0;
  virtual void ResetNTPReferenceTimestamp() = 0;
  //������ʱ����ʵʱ�̶�Ӧ�ľ���ʱ��
  virtual void SetStreamTimeBase(long long t) = 0;
};


//
//Sample��������һ����filterʵ�ָýӿڣ���Ϊ�漰DirectShow��������
//
class IFrameDeliverer
{
public:
  virtual ~IFrameDeliverer() {}

  //��Frame�������Ͷ���
  virtual int AddFrame(IFrame *frame) = 0;
  //��������Sample
  virtual int DeliverFrame() = 0;
  //��ȡ�ڲ�IDeliverStrategy
  virtual IDeliverStrategy *GetStrategy() = 0;
  //֪ͨ���Ͷ˸ı���
  virtual void SSRCChanged() = 0;
};


#endif
