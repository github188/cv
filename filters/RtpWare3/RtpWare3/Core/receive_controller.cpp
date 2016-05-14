#include "receive_controller.h"

#include <process.h>
#include <time.h>

#include "RtpStat.h"
#include "Log/Log.h"


ReceiveController::ReceiveController(ILogger *logger)
  : receive_thread_(INVALID_HANDLE_VALUE)
  , deliver_thread_(INVALID_HANDLE_VALUE)
  , stop_thread_exec_(true)
  , receiver_(nullptr)
  , reassembler_(nullptr)
  , deliverer_(nullptr)
  , logger_(logger)
{
  rtp_callback_.obj = nullptr;
  rtp_callback_.func = nullptr;
  memset(&rtp_stat_item_, 0, sizeof(rtp_stat_item_));
}


ReceiveController::~ReceiveController(void)
{
  Stop();
}

int ReceiveController::Initialize(IRtpReceiver *receiver, 
                                  IFrameReassembler *reassembler, 
                                  IFrameDeliverer *deliverer)
{
  if (receiver_ || reassembler_ || deliverer_) {
    LOG_ERROR_EX(logger_, "receive controller has already been initialized");
    return -1;
  }

  if (nullptr == receiver) {
    LOG_ERROR_EX(logger_, "init receive controller with null receiver");
  } else {
    receiver_ = receiver;
  }

  if (nullptr == reassembler) {
    LOG_ERROR_EX(logger_, "init receive controller with null reassembler");
  } else {
    reassembler_ = reassembler;
  }

  if (nullptr == deliverer) {
    LOG_ERROR_EX(logger_, "init receive controller with null deliverer");
  } else {
    deliverer_ = deliverer;
  }
  return 0;
}

int ReceiveController::Start()
{
  // reset stop flag
  last_recv_packet_time_ = 0;
  last_callback_time_ = 0;
  stop_thread_exec_ = false;
  ThreadParam param;
  param.thread_started = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (!param.thread_started) {
    LOG_ERROR_EX(logger_, "start controller failed: create sync event failed");
    return -1;
  }

  // check
  if (!receiver_ || !reassembler_ || !deliverer_) {
    LOG_ERROR_EX(logger_, "start controller failed: Initialize() may not be called");
    return -1;
  }

  // receive thread
  param.obj = this;
  param.proc = &ReceiveController::ReceiveProc;
  receive_thread_ = (HANDLE)_beginthreadex(nullptr, 0, ExecProc, static_cast<void*>(&param), 0, nullptr);
  WaitForSingleObject(param.thread_started, INFINITE); // wait util thread started to ensure param available

  // deliver thread
  param.proc = &ReceiveController::DeliverProc;
  deliver_thread_ = (HANDLE)_beginthreadex(nullptr, 0, ExecProc, static_cast<void*>(&param), 0, nullptr);
  WaitForSingleObject(param.thread_started, INFINITE);

  CloseHandle(param.thread_started);
  return 0;
}

int ReceiveController::Stop()
{
  stop_thread_exec_ = true;
  DWORD wait_result = 0;

  if (receive_thread_ != INVALID_HANDLE_VALUE) {
    wait_result = WaitForSingleObject(receive_thread_, 500);
    if (WAIT_OBJECT_0 != wait_result) {
      LOG_ERROR_EX(logger_, "stop receive thread failed");
      TerminateThread(receive_thread_, -2);
    }
    receive_thread_ = INVALID_HANDLE_VALUE;
  }

  if (deliver_thread_ != INVALID_HANDLE_VALUE) {
    wait_result = WaitForSingleObject(deliver_thread_, 500);
    if (WAIT_OBJECT_0 != wait_result) {
      LOG_ERROR_EX(logger_, "stop deliver thread failed");
      TerminateThread(deliver_thread_, -2);
    }
    deliver_thread_ = INVALID_HANDLE_VALUE;
  }

  return 0;
}

unsigned int __stdcall ReceiveController::ExecProc( void *p )
{
  ThreadParam *param = static_cast<ThreadParam *>(p);
  if (param) {
    ProcPtr proc = param->proc;
    ReceiveController *obj = param->obj;
    SetEvent(param->thread_started); //��¼�˲����󣬸Ĵ˱�ǣ���start()��֪�����Լ�����
    //����ͨ����Ա����ָ����ó�Ա����Ŷ���﷨��ɬ������
    if (obj) 
      return (unsigned int)(obj->*proc)();
  }
  return -1;
}

int ReceiveController::ReceiveProc()
{
  if (!receiver_) {
    LOG_ERROR_EX(logger_, "start receive proc failed: null rtp receiver");
    return -1;
  }
  if (!reassembler_) {
    LOG_ERROR_EX(logger_, "start receive proc failed: null frame reassembler");
    return -1;
  }
  LOG_INFO_EX(logger_, "receive proc start");

  while (!stop_thread_exec_) {
    IRtpPacket *rtp_packet = nullptr;
    receiver_->Receive(rtp_packet);
    if (rtp_packet) {
      //�յ�rtp��
      last_recv_packet_time_ = time(nullptr);

      int ret = reassembler_->PushRtpPacket(rtp_packet);
      if (ret & IFrameReassembler::PUSH_SSRC_CHANGE) {
        //Դ�ı䣬����deliver
        LOG_WARN_EX(logger_, "ssrc changed, reset deliver");
        deliverer_->SSRCChanged();
      }
      if (ret & IFrameReassembler::PUSH_CHK_FRAME) {
        IFrame *frame = nullptr;
        //ѭ��ȡ������������֡
        while (frame = reassembler_->PopFrontFrame()) {
          //ÿ�����µ�֡����ʱ�����Ӧ��ϵ
          unsigned long long ntp_ts = 0;
          unsigned long rtp_ts = 0;
          receiver_->GetNTPReferenceTimestamp(ntp_ts, rtp_ts);
          IDeliverStrategy *strategy = deliverer_->GetStrategy();
          if (strategy) strategy->SetNTPReferenceTimestamp(ntp_ts, rtp_ts);
          //��deliver��֤�̰߳�ȫ
          deliverer_->AddFrame(frame);
        }
      }
    } else {
      //��ҪSleep������2ms���ᶪ��������������socket�Ľ������������Ż����գ���ortp_receiverʵ��
      //Sleep(2); //û�а�����ȡ����2ms����

      if (last_recv_packet_time_ != 0 && time(nullptr) - last_recv_packet_time_ > 5) {
        LOG_WARN_EX(logger_, "waiting for packet too long, sender might restart, reset session");
        last_recv_packet_time_ = 0;
        receiver_->ResetSession();
      }
    }
  }

  LOG_INFO_EX(logger_, "receive proc exit");
  return 0;
}

int ReceiveController::DeliverProc()
{
  if (!deliverer_) {
    LOG_ERROR_EX(logger_, "start deliver proc failed: null frame deliver");
    return -1;
  }
  LOG_INFO_EX(logger_, "deliver proc start");

  while (!stop_thread_exec_) {
    deliverer_->DeliverFrame();
    ReportRtpStatistics();
    Sleep(5);
  }

  LOG_INFO_EX(logger_, "deliver proc exit");
  return 0;
}

int ReceiveController::ReportRtpStatistics()
{
  if (!receiver_) {
    //LOG_ERROR_EX(logger_, "start rtp statistics proc failed: null rtp receiver");
    return -1;
  }

  time_t now = time(nullptr);
  if (now - last_callback_time_ < 5) {
    //ÿ5�뱨��һ��
    return 1;
  }
  last_callback_time_ = now;

  receiver_->GetStatistics(rtp_stat_item_);
  if (rtp_callback_.func) {
    rtp_callback_.func(rtp_callback_.obj, &rtp_stat_item_);
  } else {
    printf_s("from %s :%u, %.1fkbps, lost:%.1f%%, delay:%ums\r", 
      rtp_stat_item_.rem_addr, rtp_stat_item_.rem_port, rtp_stat_item_.recv_bandwidth,
      rtp_stat_item_.lost, rtp_stat_item_.delay);
  }
  return 0;
}

void ReceiveController::SetRtpCallback( RtpCallbackInfo & cb )
{
  rtp_callback_.obj = cb.obj;
  rtp_callback_.func = cb.func;
}
