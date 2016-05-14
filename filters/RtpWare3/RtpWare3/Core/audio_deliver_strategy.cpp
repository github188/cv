#include "audio_deliver_strategy.h"

#include "media_sync.h"
#include "../Util/util.h"
#include "Log/Log.h"

AudioDeliverStrategy::AudioDeliverStrategy(
  IBaseFilter *filter, CMediaType &type, ILogger *logger)
  : filter_(filter)
  , logger_(logger)
  , sample_rate_(0)
  , sample_count_(0)
  , base_rtp_timestamp_(0)
  , base_wallclock_time_(0)
  , recv_start_wallclock_time_(0)
  , last_sample_timestamp_(0)
  , got_time_base_from_rtcp_(false)
  , time_base_reset_(false)
  , sr_count_(0)
  , last_sr_ts_(0)
{
  if (type.IsValid() && type.formattype == FORMAT_WaveFormatEx) {
    WAVEFORMATEX *wfx = reinterpret_cast<WAVEFORMATEX *>(type.pbFormat);
    sample_rate_ = wfx->nSamplesPerSec;
    if (8000 > sample_rate_) {
      LOG_ERROR_EX(logger_,"init audio deliver strategy failed: invalid audio sample rate: %.0f",
        sample_rate_);
    }
  } else {
    LOG_ERROR_EX(logger_,"init audio deliver strategy failed: invalid audio format");
  }
  LOG_PRINTF_EX(logger_, "audio_sample_rate=%.0f", sample_rate_);
}


AudioDeliverStrategy::~AudioDeliverStrategy(void)
{
}

bool AudioDeliverStrategy::ShouldFrameDiscard( IFrame &frame )
{
#ifdef USE_SR_NTP
  if (!got_time_base_from_rtcp_) {
    return true;
  }
#endif
  //��Ƶ���ÿ��Ƕ�֡����
  return false;
}

int AudioDeliverStrategy::ConvertFrameToSample( 
  IFrame &frame, IMediaSample &sample )
{
  //data
  BYTE *data_buf = nullptr;
  sample.GetPointer(&data_buf);
  long buf_size = sample.GetSize();

  if (!data_buf || 0 >= buf_size) {
    LOG_ERROR_EX(logger_,"invalid dshow media sample");
    return -1;
  }
  if (buf_size < (long)frame.GetDataSize()) {
    LOG_ERROR_EX(logger_,"dshow media sample insufficient, buf_size=%d, need=%u",
      buf_size, frame.GetDataSize());
    return -1;
  }
  memcpy_s(data_buf, buf_size, frame.GetData(), frame.GetDataSize());
  sample.SetActualDataLength(frame.GetDataSize());

  //timestamp
  long long timestamp = 0;
  {
    //���������򣬼��ٳ�������ʱ��
    CAutoLock lock(&timestamp_lock_);

#ifdef USE_SR_NTP

    /* ����Ƶʱ����뷢�Ͷ˵ľ���ʱ��(wallclock time)��������ԣ����Ҫ��ԭ����Ƶʱ���������ԣ�
       ʵ������Ƶͬ��������Ҫ�����Sample����ʱ�̵ķ��Ͷ�ntpʱ�䡣
       ͨ��SetNTPReferenceTimestamp���ǻ����rtpʱ���rtp_ts���Լ����Ӧ�ķ��Ͷ˾���ʱ��ntp_ts��
       ��sample��ʱ���ts��rtp_ts�Ĳ�ֵ�����sample_rate_���Ϳ������ts��rtp_ts��ntpʱ������
       ����ntp_ts���Եõ�sample�ڷ��Ͷ˲���ʱ��Ӧ��ntpʱ�̡�
     */
    //��δ��ȡʱ���׼��������֡
    if (!got_time_base_from_rtcp_) return -1;

    //step1:��rtpʱ�����ֵ���������ʱ�����rtp��׼ʱ�����ntpʱ���ֵ
    //���ÿ���ʱ���ѭ����������������⣬�޷����������պ�����ȷ���
    timestamp = (long long)((frame.GetTimestamp() - base_rtp_timestamp_) * 1e7 / sample_rate_);

    //step2:�Ӳ�ֵ������������ʱ��Ӧ��ntpʱ�䣬�Դ���Ϊʱ�������������Ƶͬ��
    timestamp += base_wallclock_time_;

    //��¼һ����־�����ڷ�������ʱ���׼������
    if (time_base_reset_) {
      LOG_INFO_EX(logger_, "rtp_ts=%u, ts=%I64dms, increament=%I64dms", 
        frame.GetTimestamp(), Ts2Ms(timestamp), Ts2Ms(timestamp-last_sample_timestamp_));
      time_base_reset_ = false;
    }

#else // USE_SR_NTP

    //��ʹ��SR�����ʱ��ο�ֵ��ֱ�ӹ�����ʱ�䣬�˷�������֤����Ƶͬ��
    //��δ����ʱ���׼��������
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

  //���´�������Ƶ��ͬ
  sample.SetDiscontinuity(TRUE);
  sample.SetSyncPoint(TRUE);

  return 0;
}

bool AudioDeliverStrategy::ShouldSampleDeliver(IMediaSample &sample)
{
  //��Ƶ���Ǿ������ͣ�������ͬ������
  REFERENCE_TIME start_time, end_time;
  sample.GetTime(&start_time, &end_time);

  //��֤����Ƶͬ����ֻ�ж�Ӧʱ�����Ƶ�Ѿ������ˣ���������Ƶ
  //��Ҫע�⣬����һ��80ms����Ƶ�����ͺ�������ʼʱ���80ms�ڵ���ƵӦ�ö��������ͣ�
  //��Ҫ����һ��ֵ
  const REFERENCE_TIME ts_offset = 750000LL;
  MediaSync::Lock();
  if (start_time + ts_offset > MediaSync::delivered_stream_ts_) {
    MediaSync::delivered_stream_ts_ = start_time + ts_offset;
    MediaSync::delivered_stream_ts_wallclock_ = GetDShowTimeFromUnixTime(nullptr);
  }
  MediaSync::Unlock();

  //LOG_PRINTF_EX(logger_, "ts=%I64dms, sync_ts=%I64dms", start_time/10000, MediaSync::delivered_stream_ts_/10000); //debug
  return true;
}

void AudioDeliverStrategy::SetNTPReferenceTimestamp( unsigned long long ntp_ts, unsigned long ts )
{
  /* ��1��������ÿ�յ�һ��RTCP��SR���棨ʦ��������3s��һ�����������Ը���ʱ����Ĳο���׼�����ǣ�
     ���������������Ͳ������ϸ�ȼ���ģ���ˣ����Ƶ�����²ο���׼������ɽ��ն˵�����ʱ��
     ����������������ΪÿСʱ����һ�Ρ�
     ��2������Ҫע��һ�����⣬���32λ��RTPʱ���ѭ����ο���׼δ���£�Ҳ�ǻ������ġ����¼����
     ��Ƶ����90000Hz��Ƶ�ʼ�ʱ��32λʱ���Ҳ����2^32/90000/3600=13.25h����Ƶ��������Ƶ����
     һ�㲻����44kHz������ÿСʱ����һ���㹻�ˡ�
     ��3�����⻹��һ�����⣬ʵ���з���ortp�����ĵ�һ��SR����ʱ������ԣ�ƫ��90�����ң�Ҫ��������
     һ���һ��SR��һ���ڷ��ͣ�ts��С��90000��RFC�涨����Ƶʱ������ȣ������ǲ����Դ���Ϊ�ж�
     ��һ��SR����ı�׼��������һ���Ͷ˲���һ��ʼ������������Ƶ���ǹ���һ��ʱ��ſ�ʼ�����һ
     ֡�أ�����������������鷳���ֶ�
   */
  long long sender_wallclock = GetDShowTimeFromNTPTime(ntp_ts);
  if (sender_wallclock <= 0) return;

  //���Ե�һ��SR
  if (ts == last_sr_ts_) return;
  else { ++sr_count_; last_sr_ts_ = ts; }
  if (1 == sr_count_) return;

#ifdef USE_SR_NTP
  if (!got_time_base_from_rtcp_ || 
      sender_wallclock - base_wallclock_time_ >= 3600*10000000LL) {
    CAutoLock lock(&timestamp_lock_);
    //����ǰ1�롣��Ϊ���Ͷ˵�RTCP�����ڷ���ĳһ֡�󷢳��������ǵ�n֡���е�ʱ����ն��յ���SR���棬
    //ȴ��û����n-1֡����ʱ��ͱ����ˡ�Ϊ�˽��������⣬���ǰѻ�׼��ǰһ��
    base_rtp_timestamp_ = ts - (unsigned long)sample_rate_;
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

void AudioDeliverStrategy::ResetNTPReferenceTimestamp()
{
  LOG_INFO_EX(logger_, "ntp reference reset");
  CAutoLock lock(&timestamp_lock_);
  got_time_base_from_rtcp_ = false;
  last_sr_ts_ = 0;
  sr_count_ = 0;
}

void AudioDeliverStrategy::SetStreamTimeBase( long long t )
{
  recv_start_wallclock_time_ = t;
  base_rtp_timestamp_ = 0;
  base_wallclock_time_ = 0;
#ifdef USE_SR_NTP
  LOG_INFO_EX(logger_, "audio waiting for SR report");
#else // USE_SR_NTP
  LOG_INFO_EX(logger_, "audio start receiver_wallclock=%I64dms", Ts2Ms(recv_start_wallclock_time_));
#endif // USE_SR_NTP
}
