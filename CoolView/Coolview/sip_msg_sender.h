#ifndef SIP_MSG_SENDER_H
#define SIP_MSG_SENDER_H

#include "msg_sender_interface.h"

class CvSipWrapperIf;

class SipMsgSender : public IMsgSender {
  Q_OBJECT

public:
  SipMsgSender(QObject *parent);
  virtual ~SipMsgSender();

  void Initialize();

public Q_SLOTS:
  // �ն���sip��������½
  void SendTerminalLoginInfoSlot(const TerminalLoginParam &param) override;

  // �ն�������������ն���ʵԴ��ַ��֤��Ϣ
  void SendTerminalInfoForSAVASlot(const TerminalSAVAParam &param) override;

  // ���ڵ�½����Sip�������ı���
  void SendRegisterHeartBeatToSipServerSlot() override;

  // ���ڵ�½�������������ı���
  void SendOnlineHeartBeatToKeepLoginSlot() override;

  // ���ڵ�½�������������������߲�����Ϣ
  void SendOnlineTestSlot() override;

  // �ն���sip�������ǳ�
  void SendTerminalLogoutInfoSlot() override;

  // ��������������Ϣ
  void SendGetTerminalConfigSlot() override;

  // �����ն�������Ϣ
  void SendReportTerminalConfigSlot(const ConfigDict &param) override;

  // ��������б�
  void RequestConferenceListSlot() override;

  // ������Ȩ������Ϣ
  void RequestFloorControlInfoSlot(const QString &cid) override;

  // ����������
  void SendConferenceJoinInfoSlot(
    const ConferenceJoinParam &param,
    const SDPInfo &sdp_info) override;

  // ���ֱ����ն��ڻ���������
  void SendOnlineHeartBeatToKeepInConferenceSlot(
    const QString &conference_uri) override;

  // �����뿪������Ϣ
  void SendConferenceLeaveInfoSlot(
    const QString &conference_uri) override;

  // ��ȡ�ն��б�
  void RequestTerminalListSlot(const QString &conference_uri) override;

  // ��ѯ��ǰ��������
  void QueryMainSpeakerSlot(const QString &conference_uri) override;
  // ��ѯ��ϯ�ն�
  void QueryChairmanSlot(const QString &conference_uri) override;
  // ��ѯ������Ļ�ն�
  void QueryShareScreenSlot(const QString &conference_uri) override;

  // ���;�����Ϣ
  void SendHandUpMsgSlot(
    const QString &conference_uri,
    const QString &local_uri,
    bool handup) override;

  // ����ѡ����������Ϣ
  void SendAllowSpeakMsgSlot(
    const QString &conference_uri,
    const QString &terminal_uri,
    bool allow) override;

  // ����Զ���ն��򱾵��ն˷���ý����
  void RequestStartMediaSlot(const MediaRelayParam &param) override;

  // �ظ�Զ���ն��򱾵��ն�������ý����������
  void ReplyStartMediaRequestSlot(const MediaReplyParam &param) override;

  // ����Զ���ն��򱾵��ն�ֹͣ����ý����
  void RequestStopMediaSlot(const MediaRelayParam &param) override;

  // �ظ�Զ���ն��򱾵��ն���������ֹͣ����ý����������
  void ReplyStopMediaRequestSlot(const MediaReplyParam &param) override;

  // ��֪���в����նˣ������ն˹�����Ļ�ķ���״̬
  // enableΪtrue����Ϊ����״̬
  // enableΪfalse����Ϊֹͣ״̬
  void SendScreenShareControlSlot(const ScreenControlParam &param) override;

  // ��֪���вλ��նˣ������ն�ý�巢��״̬
  // enableΪtrue����Ϊ����״̬
  // enableΪfalse����Ϊֹͣ״̬
  void SendMediaControlInfoSlot(const MediaControlParam &param) override;

  void SendRecordControlSlot( const RecordControlParam &param ) override;

  void SendRecordControlAckSlot( const RecordControlAckParam &param ) override;

private:
  // StartMedia��StopMedia����Ϣ�У�С��ʹ���˲�ͬ������
  // ����start�������ö�Ӧ������
  QString ConvertToQString(MediaStreamType type, bool start) const;
  QString ConvertToQString(MediaControlType type) const;

  // �ж�ip��ַ�ǲ���ipv6��ַ
  bool IsIpv6Address(const QString &ip) const;

private:
  CvSipWrapperIf *sip_wrapper_proxy_; 
  
  QString username_;
  QString regist_content_;
  QString ip_addr_;
  QString global_service_;
};

#endif // SIP_MSG_SENDER_H
