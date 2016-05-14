#ifndef GENERIC_DELIVER_STRATEGY_H
#define GENERIC_DELIVER_STRATEGY_H

#include "receive_interfaces.h"
#include "Log/Log.h"
#include <streams.h>

class AudioDeliverStrategy :
  public IDeliverStrategy
{
public:
  AudioDeliverStrategy(IBaseFilter *filter, CMediaType &type, ILogger *logger);
  ~AudioDeliverStrategy(void);

  virtual bool ShouldFrameDiscard( IFrame &frame );
  virtual int ConvertFrameToSample( IFrame &frame, IMediaSample &sample );
  virtual bool ShouldSampleDeliver( IMediaSample &sample );
  virtual void SetDiscontinuity() {}
  virtual void SetNTPReferenceTimestamp( unsigned long long ntp_ts, unsigned long ts );
  virtual void ResetNTPReferenceTimestamp();
  virtual void SetStreamTimeBase( long long t );

protected:
  IBaseFilter *filter_;
  double sample_rate_; //��double������㣬����ÿ��ת��
  ILogger *logger_;

  //ʵʱ״̬����
  unsigned long long sample_count_; //����96k����Ƶ���޷���intֻ����(2^32-1)/96000/86400=0.5��

  //ʱ�����׼
  unsigned long base_rtp_timestamp_; //rtpʱ�����׼�������rtp��ͷ��32λ�޷���һ�£�
    //ʱ���ѭ����Ҫ�������Զ�������Լ���ʱ���ֵ
  long long base_wallclock_time_; //base_rtp_timestamp_��Ӧ�ķ��Ͷ˾���ʱ�䣬��λ100ns
  long long recv_start_wallclock_time_; //��Ƶ����ʼʱ�̶�Ӧ�Ľ��ն˾���ʱ�䣬��filter����ʱ��
  long long last_sample_timestamp_;
  bool got_time_base_from_rtcp_; //��������ʱ���׼�ǹ���ֵ���Ǵ�SR����õ��Ŀɿ�ֵ
  bool time_base_reset_; //for logging 
  CCritSec timestamp_lock_;

  unsigned long sr_count_; //�յ���SR�����������ڽ��һ����������⣬���ü�SetNTPReferenceTimestampʵ��
  unsigned long last_sr_ts_; //���ڱ��sr�����Ƿ�����ˣ�����ͬ��
};


#endif // !GENERIC_DELIVER_STRATEGY_H
