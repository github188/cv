#ifndef CORE_DEFS_H
#define CORE_DEFS_H


enum RtpPayloadType
{
  kPayloadUnknown = 0,
  kPayloadSpeex = 50,
  kPayloadH264 = 96,
  kPayloadAAC = 97,
};

//���rtp���س��ȣ�1500��ȥIPv6��40B����udp��8B����rtp��12B���õ�
const unsigned long max_rtp_payload_size = 1440;

#endif