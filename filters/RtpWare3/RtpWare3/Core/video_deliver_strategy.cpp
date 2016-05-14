#include "video_deliver_strategy.h"

#include "media_sync.h"
#include "../Util/util.h"
#include "Log/Log.h"

VideoDeliverStrategy::VideoDeliverStrategy(
  IBaseFilter *filter, CMediaType &type, ILogger *logger)
  : filter_(filter)
  , logger_(logger)
  , sample_count_(0)
  , discontinuity_(true)
  , base_rtp_timestamp_(0)
  , base_wallclock_time_(0)
  , recv_start_wallclock_time_(0)
  , last_sample_timestamp_(0)
  , got_time_base_from_rtcp_(false)
  , time_base_reset_(false)
  , sr_count_(0)
  , last_sr_ts_(0)
{
}


VideoDeliverStrategy::~VideoDeliverStrategy(void)
{
}

bool VideoDeliverStrategy::ShouldFrameDiscard( IFrame &frame )
{
#ifdef USE_SR_NTP
  if (!got_time_base_from_rtcp_) {
    return true;
  }
#endif
  discontinuity_ = discontinuity_ || frame.IsDiscontinuity();
  if (discontinuity_ && !frame.IsSyncPoint()) {
    return true;
  }
  return false;
}

int VideoDeliverStrategy::ConvertFrameToSample( IFrame &frame, IMediaSample &sample )
{
  //data
  BYTE *data_buf = nullptr;
  sample.GetPointer(&data_buf);
  long buf_size = sample.GetSize();

  if (!data_buf || 0 >= buf_size) {
    LOG_ERROR_EX(logger_, "invalid dshow media sample");
    return -1;
  }
  if (buf_size < (long)frame.GetDataSize()) {
    LOG_ERROR_EX(logger_, "dshow media sample insufficient, buf_size=%d, need=%u",
      buf_size, frame.GetDataSize());
    return -1;
  }
  memcpy_s(data_buf, buf_size, frame.GetData(), frame.GetDataSize());
  sample.SetActualDataLength(frame.GetDataSize());

  //timestamp
  //���㷽ʽͬaudio
  long long timestamp = 0;
  {
    //���������򣬼��ٳ�������ʱ��
    CAutoLock lock(&timestamp_lock_);

#ifdef USE_SR_NTP

    if (!got_time_base_from_rtcp_) return -1;

    //step1:��rtpʱ�����ֵ�����������ο�ʱ����ľ���ʱ���ֵ
    //���ÿ���ʱ���ѭ����������������⣬�޷����������պ�����ȷ���
    timestamp = (long long)((frame.GetTimestamp() - base_rtp_timestamp_) * 1e7 / sample_rate_);

    //step2:�Ӳ�ֵ������������ʱ��Ӧ�ķ��Ͷ˾���ʱ�䣬�Դ���Ϊʱ�������������Ƶͬ��
    timestamp += base_wallclock_time_;

    //��¼һ����־�����ڷ�������ֵ��ʵ��ֵ�����
    if (time_base_reset_) {
      LOG_INFO_EX(logger_, "rtp_ts=%u, ts=%I64dms, increament=%I64dms", 
        frame.GetTimestamp(), Ts2Ms(timestamp), Ts2Ms(timestamp-last_sample_timestamp_));
      time_base_reset_ = false;
    }

#else // USE_SR_NTP

    if (base_rtp_timestamp_ == 0) {
      base_rtp_timestamp_ = frame.GetTimestamp();
      base_wallclock_time_ = GetDShowTimeFromUnixTime(nullptr) - recv_start_wallclock_time_;
    }

    //step1:��rtpʱ�����ֵ���������ʱ�����rtp��׼ʱ�����ntpʱ���ֵ
    timestamp = (long long)((frame.GetTimestamp() - base_rtp_timestamp_) * 1e7 / sample_rate_);

    //step2:�Ӳ�ֵ����������Ӧ����ʱ��
    timestamp += base_wallclock_time_;
    //LOG_PRINTF_EX(logger_, "ts=%I64d", Ts2Ms(timestamp)); //debug

#endif // USE_SR_NTP

    //step3:��ֹʱ����ǵ�������
    if (last_sample_timestamp_ == 0) { 
      //��ʹtimestampΪ������ǡΪ0���������������Ҳû����
      last_sample_timestamp_ = timestamp - 1;
    }
    if (last_sample_timestamp_ >= timestamp) {
      LOG_WARN_EX(logger_, "ts=%I64d less than last_ts=%I64d", timestamp, last_sample_timestamp_);
      //ʱ�����������һ֡�ģ�����Ϊ��һ֡��10ms
      timestamp = last_sample_timestamp_ + 100000;
    }

    last_sample_timestamp_ = timestamp;
  }

  REFERENCE_TIME start_time, end_time;
  start_time = timestamp;
  end_time = start_time + 1; //end_timeû�˿��ģ�΢�����������ӵ�
  sample.SetTime(&start_time, &end_time);

  //other
  {
    REFERENCE_TIME start_media_time, end_media_time;
    start_media_time = sample_count_;
    end_media_time = ++sample_count_;
    sample.SetMediaTime(&start_media_time, &end_media_time);
  }

  sample.SetDiscontinuity(discontinuity_ || frame.IsDiscontinuity());
  sample.SetSyncPoint(frame.IsSyncPoint());

  //reset discontinuity
  if (frame.IsSyncPoint()) {
    discontinuity_ = false; //��ǿ��Բ��ö���֡��
  }

  return 0;
}

bool VideoDeliverStrategy::ShouldSampleDeliver( IMediaSample &sample )
{
  bool ret = false;
  REFERENCE_TIME start_time, end_time;
  sample.GetTime(&start_time, &end_time);

  //��֤����Ƶͬ����ֻ�������϶�Ӧʱ�����Ƶ�Ѿ������ˣ���������Ƶ
  MediaSync::Lock();
  //�����������������򵥵��жϣ�������ͷ���ͣ������Ƶ����Ƶ�Ῠס
  //if (start_time <= MediaSync::delivered_stream_ts_) ret = true;
  //��ô��������ȷ��
  if (MediaSync::delivered_stream_ts_wallclock_ > 0) {
    long long now = GetDShowTimeFromUnixTime(nullptr);
    long long audio_stream_time = now - MediaSync::delivered_stream_ts_wallclock_ + 
      MediaSync::delivered_stream_ts_;
    if (start_time <= audio_stream_time) ret = true;
  } else {
    //û����Ƶ�ο�����Ƶ�����ܿ������
    ret = true;
    //LOG_DEBUG_EX(logger_, "%I64d %I64d", start_time, video_stream_time);
  }
  MediaSync::Unlock();

  //printf_s("%s", ret ? ">" : "-"); //debug
  return ret;
}

void VideoDeliverStrategy::SetNTPReferenceTimestamp( 
  unsigned long long ntp_ts, unsigned long ts )
{
  //����ͬAudioDeliverStrategy::SetNTPReferenceTimestamp
  long long sender_wallclock = GetDShowTimeFromNTPTime(ntp_ts);
  if (sender_wallclock <= 0) return;

  //����AudioDeliverStrategy�е����ɣ���Ƶ����һ������Ҫ���Ե�һ��SR��
  //IntelӲ�������ڱ����һ֡ʱ����200ms�����ӳ٣���ortp�ڷ��͵�һ֡ʱ�ᷢ��SR���棬
  //���Է��͵�һ֡��ʱ����Ϊntp_tsֵ���Ǿ���˵����һ����ƵSR�����е�ntp_tsֵ�ӳ�200ms���ң�
  //����Դ���Ϊ�ο�����������Ƶ�ӳ٣�����Ҫ���Ե�һ��SR���档
  if (ts == last_sr_ts_) return;
  else { ++sr_count_; last_sr_ts_ = ts; }
  if (1 == sr_count_) return;

#ifdef USE_SR_NTP
  if (!got_time_base_from_rtcp_ || 
      sender_wallclock - base_wallclock_time_ >= 3600*10000000LL) {
    CAutoLock lock(&timestamp_lock_);
    //����ǰ1�롣����ͬ��Ƶ
    base_rtp_timestamp_ = ts - sample_rate_;
    base_wallclock_time_ = sender_wallclock - 10000000LL;
    time_base_reset_ = true;
#else
  if (!got_time_base_from_rtcp_) {
#endif
    got_time_base_from_rtcp_ = true;
    LOG_INFO_EX(logger_, "by SR report, rtp_ts=%u correspond to sender_wallclock=%I64dms", 
      ts, Ts2Ms(sender_wallclock));
  }
}

void VideoDeliverStrategy::ResetNTPReferenceTimestamp()
{
  LOG_INFO_EX(logger_, "ntp reference reset");
  CAutoLock lock(&timestamp_lock_);
  got_time_base_from_rtcp_ = false;
  last_sr_ts_ = 0;
  sr_count_ = 0;
}

void VideoDeliverStrategy::SetStreamTimeBase( long long t )
{
  recv_start_wallclock_time_ = t;
  base_rtp_timestamp_ = 0;
  base_wallclock_time_ = 0;
#ifdef USE_SR_NTP
  LOG_INFO_EX(logger_, "video waiting for SR report");
#else // USE_SR_NTP
  LOG_INFO_EX(logger_, "video start receiver_wallclock=%I64dms", Ts2Ms(recv_start_wallclock_time_));
#endif // USE_SR_NTP
}
