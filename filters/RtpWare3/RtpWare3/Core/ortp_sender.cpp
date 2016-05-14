#include "ortp_sender.h"
#include "log/log.h"

#include <process.h>

ORtpSender::ORtpSender(void)
{
  logger_ = CreateLogger(); //ʹ��ÿ��sender�Լ���logger���Ա������ĸ�����Ŀ�����

  rtp_session_ = nullptr;
  send_thread_ = INVALID_HANDLE_VALUE;
  stop_thread_exec_ = true;
}


ORtpSender::~ORtpSender(void)
{
  DestroySession();
}

int ORtpSender::InitSession(std::string ip, unsigned short port, RtpPayloadType payload_type)
{
  DestroySession();
  ip_ = ip;
  port_ = port;

  //init rtp session
  //��Ҫ������DllMain�е���ortp_init();�Գ�ʼ��ortp�⣡

  rtp_session_ = rtp_session_new(RTP_SESSION_SENDONLY);  

  rtp_session_set_scheduling_mode(rtp_session_, 0);  
  //rtp_session_set_blocking_mode(rtp_session_, 0); //��rtp_session_set_scheduling_mode����ʱ��Ч
  /** use_connectΪtrueʱ�������һ�����յ���RTP/RTCP������socket���ӣ��ų�������Դ��RTP����
   ** ����Ϊfalse�����������з������ض˿ڵ����ݰ�	*/
  rtp_session_set_connected_mode(rtp_session_, false);
  /** symmetric_rtpΪtrueʱ�����ѽ��յ��ĵ�һ��RTP/RTCP���ķ��͵�ַ�����������а�����Դ��ַ����ֵ��������Ϊtrue*/
  rtp_session_set_symmetric_rtp(rtp_session_, true); 
  rtp_session_set_profile(rtp_session_, &av_profile);
  rtp_session_set_rtp_socket_send_buffer_size(rtp_session_, 1024 * 64); //Socket�Ļ���������С��0��ʾ��ϵͳĬ��ֵ�������UDP���ã�
  rtp_session_enable_rtcp(rtp_session_, true);
  rtp_session_enable_adaptive_jitter_compensation(rtp_session_, false); //������������Ӧ����������ᵼ����Ƶ�ܿ�
  rtp_session_enable_jitter_buffer(rtp_session_, false); //��ʹ�ö�������
  rtp_session_set_send_payload_type(rtp_session_, payload_type); // 96 is for H.264
  rtp_session_set_remote_addr(rtp_session_, ip.c_str(), port); 

  //֧���鲥�ػ�
  rtp_session_set_multicast_loopback(rtp_session_, 1);

  {
    //ʹ�ô˰汾��oRTP����ֱ������socket���ԣ�������������°��oRTP���У�������취
    //ʹ������ģʽ��������һ����ʱ������������socket��ʱ����дʱ�����ֶ����ԣ�Ҳ��ֹfilter����
    unsigned long nonblock = 0;
    int timeout = 20; //���20ms��һ��ȴ�3ms socket�Ϳ�д�ˣ�20msΪ50֡ʱ�����ͼ���������ٴ��ˣ���Ȼ����������
    ioctlsocket(rtp_session_->rtp.socket, FIONBIO, &nonblock);
    setsockopt(rtp_session_->rtp.socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
    nonblock = 1;
    ioctlsocket(rtp_session_->rtcp.socket, FIONBIO, &nonblock); //rtcp��Ҫ�������ղ������Թ�
  }

  rtp_session_reset(rtp_session_); //����rtp sessionͳ��

  /*m_SSRC  = getenv("SSRC");  
  if (m_SSRC != NULL)   
  {  
    rtp_session_set_ssrc(rtp_session_mgr.rtp_session, atoi(m_SSRC));  
  } */  

  //start thread
  stop_thread_exec_ = false;
  send_thread_ = (HANDLE)_beginthreadex(nullptr, 0, SendProc, static_cast<void*>(this), 0, nullptr);

  //����thisָ������logger��ʶ�������Ϳ����������ĸ�sender�����־��
  char logger_name[32] = {0};
  sprintf_s(logger_name, sizeof(logger_name), "rtp%d", (int)this);
  logger_->SetModuleName(logger_name);
  LOG_INFO_EX(logger_, "sender thread start, ready send payload %d to [%s]:%d",
    payload_type, ip.c_str(), port); //��¼һ���̶߳�Ӧ�ķ���Ŀ����Ϣ

  return 0;
}

int ORtpSender::DestroySession()
{
  //stop thread
  stop_thread_exec_ = true;
  DWORD wait_result = 0;
  if (send_thread_ != INVALID_HANDLE_VALUE) {
    wait_result = WaitForSingleObject(send_thread_, INFINITE);
    if (WAIT_OBJECT_0 != wait_result) {
      LOG_ERROR_EX(logger_, "stop sender thread failed");
    } else {
      LOG_INFO_EX(logger_, "sender thread stopped");
    }
    send_thread_ = INVALID_HANDLE_VALUE;
  }

  //destroy session
  if (rtp_session_) {
    rtp_session_destroy(rtp_session_);
    rtp_session_ = nullptr;
  }

  //clear queued data
  while (!frame_units_queue_.empty()) {
    frame_units_queue_.pop();
  }
  return 0;
}

int ORtpSender::Send(IFrameUnitsPtr p)
{
  //��ע��ԭ���ǲ��ö��̷߳��͵ģ���������û�б�Ҫ������Ч�ʺܸߣ�һ���߳��㹻�ˣ�
  //����Ҳ���ü����ˣ������õ��˶��У���ʵҲû������ ����Liaokz��2015.4
  CAutoLock lock(&queue_lock_);
  if (frame_units_queue_.size() > 3) {
    //��һ���Ͷ˿������������ˣ���Ҫ�ڻ������
    LOG_WARN_EX(logger_, "too many packet queued, ignore new packet");
    return 0;
  }

  //ÿ��Sender����ʱ��ı�p�ڲ������ݿ���У���Ҫ����һ��
  //ע�⣺���Ʋ�����������p�ڲ������ݿ������ָ����У����������ݿ鱾��Ŷ
  IFrameUnitsPtr new_p(p->Clone());
  frame_units_queue_.push(new_p);

  return 0;
}

unsigned int __stdcall ORtpSender::SendProc( void *param )
{
  ORtpSender *obj = static_cast<ORtpSender*>(param);
  while (!obj->stop_thread_exec_) {
    obj->DoSend();
  }
  return 0;
}

void ORtpSender::DoSend()
{
  IFrameUnitsPtr frame_units;
  {
    //���������ڳ�����
    CAutoLock lock(&queue_lock_);
    if (!frame_units_queue_.empty()) {
      frame_units = frame_units_queue_.front();
      frame_units_queue_.pop();
    }
  }
  if (!frame_units) {
    Sleep(5); //û��������ȴ�һ��ʱ��
    return;
  }

  unsigned long ts = frame_units->GetTimestamp();
  while (!frame_units->Empty()) {
    ISendUnitPtr unit = frame_units->PopUnit();
    if (!unit) continue;
    if (rtp_session_) {
      //create packet
      mblk_t* m = rtp_session_create_packet(rtp_session_, 
        RTP_FIXED_HEADER_SIZE, 
        (uint8_t *)unit->GetData(), 
        unit->GetSize());
      //set markbit
      if (frame_units->Empty()) rtp_set_markbit(m, 1);
      else rtp_set_markbit(m, 0);
      //send
      //�ڷ�����ģʽ�£����ܷ���-1��rtp.send_errno==WSAEWOULDBLOCK����ʾ��Դ��ʱ�����ã���˳��Է������ɴ�
      //�����ڳ�ʼ��ʱ���������������������ӳ٣����Բ����ԣ�ʧ�ܾ����ʧ����
      int packet_size = msgdsize(m);
      //��Դ����Կ���rtp_session_sendm_with_ts����ʧ����񶼻��ͷ�m�����Բ��õ��� freemsg(m);
      int sent_size = rtp_session_sendm_with_ts(rtp_session_, m, ts);
      if (sent_size < packet_size) {
        LOG_WARN_EX(logger_, "send data error, data_size=%u, sent_size=%d, socket_err=%d", 
          packet_size, sent_size, rtp_session_->rtp.send_errno);
      }
    }
  }
}
