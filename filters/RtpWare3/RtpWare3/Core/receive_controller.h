#ifndef RECEIVE_CONTROLLER_H
#define RECEIVE_CONTROLLER_H


#include "windows.h"
#include "receive_interfaces.h"
#include "RtpStat.h"
#include "Log/Log.h"


struct RtpCallbackInfo
{
  void *obj;
  RtpStatCallback func;
};

class ReceiveController
{
public:
  ReceiveController(ILogger *logger);
  ~ReceiveController(void);

  int Initialize(IRtpReceiver *receiver, 
    IFrameReassembler *reassembler, 
    IFrameDeliverer *deliverer);

  void SetRtpCallback(RtpCallbackInfo &);

  int Start();
  int Stop();

private:
  //�ڲ��߳�ִ����
  static unsigned int __stdcall ExecProc(void *param);

  //�߳̾��幤��
  typedef int (ReceiveController::*ProcPtr)();
  struct ThreadParam
  {
    ReceiveController *obj;
    ProcPtr proc;
    HANDLE thread_started;
  };
  int ReceiveProc();
  int DeliverProc();
  int ReportRtpStatistics();

private:

  HANDLE receive_thread_; //�����߳�
  HANDLE deliver_thread_; //�����߳�
  bool stop_thread_exec_; //�߳�ֹͣ���

  IRtpReceiver *receiver_;
  IFrameReassembler *reassembler_;
  IFrameDeliverer *deliverer_;

  RtpCallbackInfo rtp_callback_;
  RtpStatItem rtp_stat_item_;
  time_t last_callback_time_;

  time_t last_recv_packet_time_;

  ILogger *logger_;

};


#endif
