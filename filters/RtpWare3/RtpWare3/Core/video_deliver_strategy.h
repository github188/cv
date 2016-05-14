#ifndef VIDEO_DELIVER_STRATEGY_H
#define VIDEO_DELIVER_STRATEGY_H

#include "receive_interfaces.h"
#include "Log/Log.h"
#include "streams.h"

class VideoDeliverStrategy :
  public IDeliverStrategy
{
public:
  VideoDeliverStrategy(IBaseFilter *filter, CMediaType &type, ILogger *logger);
  ~VideoDeliverStrategy(void);

  virtual bool ShouldFrameDiscard( IFrame &frame );
  virtual int ConvertFrameToSample( IFrame &frame, IMediaSample &sample );
  virtual bool ShouldSampleDeliver( IMediaSample &sample );
  virtual void SetDiscontinuity() { discontinuity_ = true; }
  virtual void SetNTPReferenceTimestamp(unsigned long long ntp_ts, unsigned long ts);
  virtual void ResetNTPReferenceTimestamp();
  virtual void SetStreamTimeBase(long long t);

protected:
  static const unsigned int sample_rate_ = 90000; //����rtpЭ����Ƶ�̶�Ϊ90000
  IBaseFilter *filter_;
  ILogger *logger_;

  //ʵʱ״̬����
  unsigned long long sample_count_;
  bool discontinuity_;

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


#endif