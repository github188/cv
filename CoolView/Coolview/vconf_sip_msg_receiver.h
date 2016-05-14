#ifndef VCONF_SIP_MSG_RECEIVER_H
#define VCONF_SIP_MSG_RECEIVER_H

#include "sip_msg_receiver.h"

struct MediaRelayParam;

class VConfSipMsgReceiver 
  : public SipMsgReceiver {
  Q_OBJECT

 public:
  VConfSipMsgReceiver(QObject *parent);
  ~VConfSipMsgReceiver();

  void Initialize();

 public Q_SLOTS:

  //������ղ�����ʹ��QueuedConnectionģ�����ʵ

  //�������������ղ���������ʾ�������
  void HandleVRecvConferenceListSlot();

  //�����������ʱ��ģ�����������
  void HandleVRecvQoSLoginPermissionSlot();

  //ģ����յ��ն��б�
  void HandleVRecvTerminalListSlot(const TerminalList&);

  //ģ�ⷢ�Ͷ˷���ý����ack
  void HandleVRecvStartMediaReplySlot(const MediaRelayParam &param);

 protected:
  // ��д�źŴ���handler
  void RecvConferenceListHandler(const SipMsgParser &parser, const QString &from);

 private:
  void AddVirtualConferenceToList(ConferenceList &conference);
};

#endif
