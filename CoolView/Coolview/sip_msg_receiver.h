#ifndef SIP_MSG_RECEIVER_H
#define SIP_MSG_RECEIVER_H

#include <functional>
#include <QHash>
#include "msg_receiver_interface.h"

class CvSipWrapperIf;
class SipMsgParser;

class SipMsgReceiver : public IMsgReceiver {
  Q_OBJECT

public:
  SipMsgReceiver(QObject *parent);
  virtual ~SipMsgReceiver();

  void Initialize();

private Q_SLOTS:
  // ������SipWrapper���յ�����Ϣ
  void HandleMessageReceivedNotifySlot(const QString &msg, const QString &from);

protected:
  MediaStreamType ConvertToMediaStreamType(const QString &type) const;
  MediaControlType ConvertToMediaControlType(const QString &type) const;

  // �ж�ip��ַ�ǲ���ipv6��ַ
  bool IsIpv6Address(const QString &ip) const;

  // ע�����е�sip��Ϣ��������
  virtual void RegistHandlers();

  // ���������ն˵�¼����֤����Ϣ
  virtual void RecvTerminalLoginNotifyHandler(const SipMsgParser &parser, const QString &from);

  // �����ն����õ���Ϣ
  void RecvTerminalConfigHandler(const SipMsgParser &parser, const QString &from);

  // ����������ص���Ϣ
  virtual void RecvConferenceListHandler(const SipMsgParser &parser, const QString &from);
  virtual void RecvTerminalListHandler(const SipMsgParser &parser, const QString &from);
  virtual void RecvTerminalLoginHandler(const SipMsgParser &parser, const QString &from);
  virtual void RecvTerminalLogoutHandler(const SipMsgParser &parser, const QString &from);
  virtual void RecvTerminalHandUpHandler(const SipMsgParser &parser, const QString &from);
  virtual void RecvTerminalHandDownHandler(const SipMsgParser &parser, const QString &from);

  // ����ý������ص���Ϣ
  virtual void RecvStartMediaReplyHandler(const SipMsgParser &parser, const QString &from);
  virtual void RecvStartMediaRelayHandler(const SipMsgParser &parser, const QString &from);
  virtual void RecvStopMediaReplyHandler(const SipMsgParser &parser, const QString &from);
  virtual void RecvStopMediaRelayHandler(const SipMsgParser &parser, const QString &from);
  virtual void RecvSendMediaControlHandler(const SipMsgParser &parser, const QString &from);
  virtual void RecvMediaControlInfoHandler(const SipMsgParser &parser, const QString &from);

  // ����QoS��ص���Ϣ
  virtual void RecvQoSMsgHandler(const SipMsgParser &parser, const QString &from);

  // ������������ص���Ϣ
  virtual void RecvSetSpeakerTerminalHandler(const SipMsgParser &parser, const QString &from);
  virtual void RecvSpeakerTerminalInfoHandler(const SipMsgParser &parser, const QString &from);
  virtual void RecvForbidSpeakerHandler(const SipMsgParser &parser, const QString &from);
  // ������ϯ�ն���ص���Ϣ
  virtual void RecvSetChairmanTerminalHandler(const SipMsgParser &parser, const QString &from);
  virtual void RecvChairmanTerminalInfoHandler(const SipMsgParser &parser, const QString &from);
  // ������Ļ��������ص���Ϣ
  virtual void RecvShareScreenTerminalInfoHandler(const SipMsgParser &parser, const QString &from);
  virtual void RecvShareScreenControlInfoHandler(const SipMsgParser &parser, const QString &from);

  // ������������������Ϣ
  virtual void RecvOnlineMessageInfoHandler(const SipMsgParser &parser, const QString &from);

  virtual void RecvTextMessageHandler(const SipMsgParser &parser, const QString &from);

  // ����¼�ƿ��Ƶ���Ϣ
  virtual void RecvRecordControlHandler(const SipMsgParser &parser, const QString &from);
  // ����¼�ƿ��ƻظ�����Ϣ
  virtual void RecvRecordControlAckHandler(const SipMsgParser &parser, const QString &from);

  // ��������Ȩ������Ϣ
  virtual void RecvFloorControlInfoHandler(const SipMsgParser &parser, const QString &from);

protected:
  typedef std::function<
    void(SipMsgReceiver&, const SipMsgParser&, const QString&)
  > Handler;

  typedef QHash<QString, Handler> HandlerDictionary;

  CvSipWrapperIf *sip_wrapper_proxy_;
  HandlerDictionary type_handler_;
};

#endif // SIP_MSG_RECEIVER_H
