#include "aac_frame_splitter.h"
#include "frame_units.h"
#include "ortp_unit.h"
#include "core_defs.h"
#include "log/log.h"
#include <ortp/str_utils.h>

AACFrameSplitter::AACFrameSplitter(const CMediaType &type, ILogger *logger)
  : AudioFrameSplitter(type, logger)
{
}

AACFrameSplitter::~AACFrameSplitter()
{
}

IFrameUnitsPtr AACFrameSplitter::Split( 
  const unsigned char *buf, 
  unsigned long size, 
  long long timestamp )
{
  const unsigned long aac_pt_size = size + 4; //4�ֽ��Ƿ�װAAC��Ƶ�Ĺ̶�ͷ��
  //��Ƶ֡һ���С�����ָ��������ˣ���¼���棬���������Է���
  if (aac_pt_size > max_rtp_payload_size) {
    LOG_WARN("aac frame size %d larger than mtu size limit %d", 
      aac_pt_size, max_rtp_payload_size);
  }

  mblk_t *m = allocb(aac_pt_size, 0);
  unsigned char *payload = m->b_wptr;
  //AU-headers-length, �����2�ֽ�AUͷ�ı��س���
  payload[0] = 0x00;  
  payload[1] = 0x10;
  //AU-header
  payload[2] = (unsigned char)((size & 0x1fe0) >> 5);  
  payload[3] = (unsigned char)((size & 0x1f) << 3); 
  //data
  memcpy(payload+4, buf, size);
  m->b_wptr += aac_pt_size;

  ISendUnitPtr send_unit(new OrtpUnit(m));
  IFrameUnitsPtr frame_units(new FrameUnits);
  frame_units->PushUnit(send_unit);
  freemsg(m);

  frame_units->SetTimestamp((unsigned long)(timestamp * ratio_));
  //LOG_PRINTF("%I64d %u", timestamp, frame_units->GetTimestamp()); //Debug
  return frame_units;
}
