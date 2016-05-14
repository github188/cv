#include "ortp_receiver.h"

#include <assert.h>

#include "ortp/ortp.h"
#include "../util/util.h"
#include "ortp_packet.h"


ORtpReceiver::ORtpReceiver(ILogger *logger)
  : logger_(logger)
{
  memset(&rtp_stat_, 0, sizeof(rtp_stat_));
}

ORtpReceiver::~ORtpReceiver(void)
{
  DestroySession();
}

int ORtpReceiver::InitSession(std::string ip, unsigned short port, RtpPayloadType payload_type)
{
  DestroySession();

  LOG_INFO_EX(logger_, "ortp receiver init, ip=%s, port=%u, payload_type=%d", 
    ip.c_str(), port, payload_type);

  //ortp init
  //��Ҫ������DllMain�е���ortp_init();�Գ�ʼ��ortp�⣡

  rtp_session_ = rtp_session_new(RTP_SESSION_RECVONLY);  
  rtp_session_set_profile(rtp_session_, &av_profile);

  /** use_connectΪtrueʱ����ʹ��UDP��connectģʽ��ֻ���յ�һ�����ݰ�Դ�İ����ų�������Դ��RTP����
   ** ����Ϊfalse�����������з������ض˿ڵ����ݰ�	*/
  rtp_session_set_connected_mode(rtp_session_, true);
  /** symmetric_rtpΪtrueʱ�����ѽ��յ��ĵ�һ��RTP/RTCP���ķ��͵�ַ�����������а�����Դ��ַ��
   ** ��ֵ��������Ϊtrue����������������RR��*/
  rtp_session_set_symmetric_rtp(rtp_session_, true);
  rtp_session_enable_rtcp(rtp_session_, true);
  rtp_session_->rtp.max_rq_size = 1000; /* maximum number of packet allowed to be queued */

  rtp_session_set_scheduling_mode(rtp_session_, false); //��ʹ�õ�����
  rtp_session_set_blocking_mode(rtp_session_, false); //������ģʽ
  //rtp_session_set_dscp(rtp_session_, 0); //QoS����ʹ��ortpĬ��ֵ
  rtp_session_set_local_addr(rtp_session_, ip.c_str(), port); //��Ȼ�Ǳ��ص�ַ������Ҫָ��������IPv4/6���鲥
  rtp_session_set_multicast_loopback(rtp_session_, true); //Ĭ��֧���鲥�ػ�

  rtp_session_enable_adaptive_jitter_compensation(rtp_session_, false);
  rtp_session_enable_jitter_buffer(rtp_session_, false);

  rtp_session_set_payload_type(rtp_session_, payload_type);  
  /** 1M Byte��Socket��������40Mbits/s����Ƶ��Ҳ����200ms�� */
  rtp_session_set_rtp_socket_recv_buffer_size(rtp_session_, 1 << 20); 
  rtp_session_->rtp.max_rq_size = 1000; /* maximum number of packet allowed to be queued */

  {
    //ʹ�ô˰汾��oRTP����ֱ������socket���ԣ�������������°��oRTP���У�������취
    //������Ϊʦ��������Щ�����Ǻܺõģ���û���յ����ݰ�ʱ��������������ֹ����ѭ��ռ��CPU
    //ͬʱ���������20ms����������Ӱ���߳�ѭ�������ֹ��Ǻ������˳�
    unsigned long nonblock = 0;
    int timeout = 20;
    ioctlsocket(rtp_session_->rtp.socket, FIONBIO, &nonblock);
    setsockopt(rtp_session_->rtp.socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
  }

  //other members
  last_recv_ts_ = 0;
  ip_ = ip;
  port_ = port;
  payload_type_ = payload_type;
  InitRtpStat();

  return 0;
}

int ORtpReceiver::DestroySession()
{
  if (rtp_session_) {
    rtp_session_destroy(rtp_session_);
    rtp_session_ = nullptr;
  }
  memset(&rtp_stat_, 0, sizeof(rtp_stat_));

  return 0;
}

int ORtpReceiver::Receive( IRtpPacket *&packet )
{
  packet = nullptr;

  if (rtp_session_ == nullptr) {
    LOG_ERROR_EX(logger_, "try to receive rtp packet before init session");
    return -1;
  }

  mblk_t *m = rtp_session_recvm_with_ts(rtp_session_, last_recv_ts_);
  if (nullptr == m) {
    /*
      Ϊʲô��3000�أ�����RTP�淶����Ƶ��ʱ��Ƶ��Ϊ90000����30֡����Ƶÿ֡+3000��
      ��Ƶʱ��Ƶ��Ϊ������ʣ���16k����Ƶ����16000. ����rtp_session_recvm_with_ts��
      ��ֻ����ʱ�����user_tsС��rtp������ÿ�μ�3000�Ѿ��ܱ�֤���ݵĽ����ˡ���ʹ
      �ӹ�ͷҲû�£��յ����ݰ���last_recv_ts_�ᱻ��Ϊ���һ�����ݰ���ʱ���+3000
    */
    last_recv_ts_ += 3000;
    return 1;
  }
  
  packet = new ORtpPacket(m);
  last_recv_ts_ = packet->GetTimestamp() + 3000; //����ʱ���

  return 0;
}

int ORtpReceiver::GetStatistics( RtpStatItem &stat )
{
  if (rtp_session_ == nullptr) return -1;

  //��Ҫʵʱ��ȡ��session������Ϣ
  rtp_stat_.ssrc = rtp_session_->rcv.ssrc;
  rtp_stat_.payload_type = rtp_session_get_recv_payload_type(rtp_session_);
  rtp_stat_.media_type = GetRtpStatMedaiTypeFromPayload(
    (RtpPayloadType)rtp_session_get_recv_payload_type(rtp_session_));

  //ͳ����Ϣ
  const rtp_stats_t* session_stat = rtp_session_get_stats(rtp_session_);
  if(session_stat) {
    rtp_stat_.packet_sent = session_stat->packet_sent;
    rtp_stat_.sent = session_stat->sent ;
    rtp_stat_.packet_recv = session_stat->packet_recv ;
    rtp_stat_.hw_recv = session_stat->hw_recv;
    rtp_stat_.recv = session_stat->recv ;
    rtp_stat_.outoftime = session_stat->outoftime;
    rtp_stat_.cum_packet_loss = session_stat->cum_packet_loss ;
    rtp_stat_.discarded = session_stat->discarded;
  }

  //���ƴ���
  rtp_stat_.recv_bandwidth = rtp_session_compute_recv_bandwidth(rtp_session_) / 1000; //������Ϊ1000���ƣ���ͬ�ڴ洢

  RtpStream *stream = &rtp_session_->rtp;

  //��öԷ���ַ��ֻ�û�ȡһ�Σ���Ϊ������UDP connectģʽ�����ֻ�ܽ���һ�����Ͷ˵�����
  //TODO:ʦ�ֶ���ĳ��Ȳ�����Ҫ���漰CoolView��֮��ĺܶ�ģ��
  //assert(sizeof(rtp_stat_.rem_addr) >= INET6_ADDRSTRLEN);
  if (rtp_stat_.rem_addr_len == 0) {
    unsigned short port = 0;
    if (GetInfoFromSockAddrStorage(&stream->rem_addr, stream->rem_addrlen,
        rtp_stat_.rem_addr, sizeof(rtp_stat_.rem_addr), &port) == 0) {
      rtp_stat_.rem_addr_len = strlen(rtp_stat_.rem_addr);
      rtp_stat_.rem_port = port;
    }
  }

  //��ȡǰһ���յ�RR��ʱ����ó������������ӳ�(ms)
  if (rtp_stat_.recv_bandwidth > 0) {
    rtp_stat_.delay = stream->delay;
    rtp_stat_.lost = stream->lost;
    rtp_stat_.jitter = stream->jitter;
  } else {
    rtp_stat_.delay = 0;
    rtp_stat_.lost = 0;
    rtp_stat_.jitter = 0;
  }

  stat = rtp_stat_;

  return 0;
}

void ORtpReceiver::InitRtpStat()
{
  //�����������ʼ���ˣ�����Ҫ���Ի�ȡ��ȷ���޴�
  rtp_stat_.payload_type = rtp_session_get_recv_payload_type(rtp_session_);
  rtp_stat_.media_type = GetRtpStatMedaiTypeFromPayload(
    (RtpPayloadType)rtp_session_get_recv_payload_type(rtp_session_));

  rtp_stat_.rtp_endpoint_type = eET_Receiver;
  rtp_stat_.seqnumber = 0;
  rtp_stat_.interval = 5000; //TODO:���ֵò������û���ˣ���ʱд����ע�Ᵽ�ָ�RecvController�еļ��һ��
  rtp_stat_.local_port = port_;

  //��ȡ����IP�Ĺ��������鷳
  addrinfo *rtp_ip_addr = nullptr;
  if (AddrStrToAddrInfo(ip_.c_str(), &rtp_ip_addr) == 0) {
    char local_addr[INET6_ADDRSTRLEN]; //�㹻���Ļ�����

    //���ݴ���ĵ�ַ���ͻ�ȡ��Ӧ�ı�����ַ
    if (IsIPv6Addr(rtp_ip_addr)) {
      GetLocalIPv6Addr(local_addr, sizeof(local_addr));
    } else {
      GetLocalIPv4Addr(local_addr, sizeof(local_addr));
    }

    if (IsMulticastAddr(rtp_ip_addr)) {
      //���鲥��ַ�Ļ���localҪ����ʵ������ַ���������鲥��ַ
      strcpy_s(rtp_stat_.local_addr, sizeof(rtp_stat_.local_addr), local_addr);
      rtp_stat_.local_addr_len = strlen(rtp_stat_.local_addr);
      rtp_stat_.isMulticast = true;
      strcpy_s(rtp_stat_.mcast_addr, sizeof(rtp_stat_.mcast_addr), ip_.c_str());
      rtp_stat_.mcast_addr_len = strlen(rtp_stat_.mcast_addr);
    }
    else if (IsLoopbackAddr(rtp_ip_addr)) {
      //�ǻػ���ַ�Ļ���localҪ����ʵ������ַ
      strcpy_s(rtp_stat_.local_addr, sizeof(rtp_stat_.local_addr), local_addr);
      rtp_stat_.local_addr_len = strlen(rtp_stat_.local_addr);
    } else {
      strcpy_s(rtp_stat_.local_addr, sizeof(rtp_stat_.local_addr), ip_.c_str());
      rtp_stat_.local_addr_len = strlen(rtp_stat_.local_addr);
    }

    FreeAddrInfo(rtp_ip_addr);
  }
}

void ORtpReceiver::GetNTPReferenceTimestamp( unsigned long long &ntp_ts, unsigned long &ts )
{
  if (rtp_session_) {
    ntp_ts = rtp_session_->rtp.last_rcv_SR_ntp_ts;
    ts = rtp_session_->rtp.last_rcv_SR_rtp_ts;
  } else {
    ntp_ts = ts = 0;
  }
}

void ORtpReceiver::ResetSession()
{
  InitSession(ip_, port_, payload_type_);
}
