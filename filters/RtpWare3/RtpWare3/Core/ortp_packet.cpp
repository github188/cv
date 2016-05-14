#include "ortp_packet.h"

#include "mediastreamer2/msqueue.h"
#include "ortp/ortp.h"

ORtpPacket::ORtpPacket(mblk_t *m)
  : m_(m)
{
}

ORtpPacket::ORtpPacket( const ORtpPacket &p )
{
  m_ = dupmsg(p.m_);
}

ORtpPacket::~ORtpPacket()
{
  if (m_) {
    freemsg(m_);
    m_ = nullptr;
  }
}

bool ORtpPacket::GetMarkbit()
{
  if (!m_) return false;
  return (1 == rtp_get_markbit(m_));
}

unsigned int ORtpPacket::GetSSRC()
{
  if (!m_) return 0;
  return rtp_get_ssrc(m_);
}

unsigned int ORtpPacket::GetTimestamp()
{
  if (!m_) return 0;
  return rtp_get_timestamp(m_);
}

unsigned short ORtpPacket::GetSeqNumber()
{
  if (!m_) return 0;
  return rtp_get_seqnumber(m_);
}

unsigned short ORtpPacket::GetPayloadType()
{
  if (!m_) return 0;
  return rtp_get_payload_type(m_);
}

unsigned short ORtpPacket::GetPlayloadSize()
{
  if (!m_) return 0;
  //���ܻ�ȡ���������ȣ�Ҫȥ��rtpͷ����
  //return msgdsize(m_);
  unsigned char *p = nullptr;
  return rtp_get_payload(m_, &p);
}

unsigned char * ORtpPacket::GetPayload()
{
  if (!m_) return nullptr;
  unsigned char *p = nullptr;
  rtp_get_payload(m_, &p);
  return p;
}

mblk_t * ORtpPacket::GetMsgb()
{
  if (!m_) return nullptr;
  //ע�⣺�ú������Ḵ��ʵ�����ݣ�ֻ������һ�����ã������´������ָ��
  return dupmsg(m_);
}

