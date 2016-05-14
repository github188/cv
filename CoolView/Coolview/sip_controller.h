#ifndef SIP_CONTROLLER_H
#define SIP_CONTROLLER_H

#include <functional>

#include <QObject>
#include <QString>
#include <QHash>

#include <dbus/sipwrapper/common/SipWrapperCommonService.h>

class CvSipWrapperIf;

#include "media_description.h"
#include "sip_msg_parser.h"

class SipController : public QObject {
  Q_OBJECT
public:
  struct TerminalLoginInfo {
    QString display_name; // �ն���ʾ��
    QString username; // �ն��û���
    QString identity; // �ն˱�ʶ
    QString password; // ����
    QString realm;    // �������������
    QString proxy_server;    // �����������ַ
    QString register_server; // ע���������ַ
    QString ip_address; // �ն˵�ip��ַ
    QString mac;        // �ն˵�mac��ַ
  };

  struct TerminalSAVAInfo {
    QString ipv4_addr; // �ն˵�ipv4��ַ
    QString ipv6_addr; // �ն˵�ipv6��ַ
    QString terminal_version; // �ն˵İ汾��
    int screen_count; // �ն˵���ʾ����Ŀ
  };

  struct ConferenceJoinInfo {
    QString cid; // ����id
    QString focus_uri; // �����uri
    QString uri;  // �ն˵�uri
    int min_rate; // ��ʹ���ֵ
    int max_rate; // ������ֵ
    QString ip_addr;   // �ն˵�ip��ַ
    QString gateway; // �ն˵�����
    int virtual_terminal_count; // �ն˵������ն���
  };

public:
  SipController(QObject *parent);
  ~SipController();

  void Initialize();

signals:
  // ���յ������½Sip��������״̬��Ӧ
  // state��ֵ�ο�enum SipAccountState
  void NotifySipAccountStateChangedSignal(const QString &name, int state);

  // ���յ������ն��Ƿ�����ʵԴ��ַ��֤
  void NotifyRecvSAVAResultSignal(bool valid);

  // ���յ�������Ϣ
  void NotifyRecvConferenceListSignal(const ConferenceList &conferences);
  void NotifyRecvTerminalListSignal(const TerminalList &terminals);
  void NotifyRecvTerminalLoginSignal(const TerminalList &terminals);
  void NotifyRecvTerminalLogoutSignal(const QString &uri);
  void NotifyRecvTerminalHandUpSignal(const QString &uri);
  void NotifyRecvTerminalHandDownSignal(const QString &uri);

  // ���յ�Զ���ն��򱾵��ն��������䷢��ý����
  void NotifyRecvStartMediaRelaySignal(
    const QString &remote_uri,
    const QString &remote_ip,
    const QString &local_vuri,
    MediaStreamType type);

  // ���յ�Զ���ն˶Ա����ն�����������ý�����Ļظ�
  void NotifyRecvMediaReplySignal(
    const QString &remote_vuri,
    MediaStreamType type,
    bool permission);

  // ���յ�Զ���ն��򱾵��ն�����ֹͣ���䷢��ý����
  void NotifyRecvStopMediaRelaySignal(
    const QString &remote_uri,
    const QString &remote_ip,
    const QString &local_vuri,
    MediaStreamType type);
  
  // ���յ�Զ���ն˶Ա����ն���������ֹͣ����ý�����Ļظ�
  void NotifyRecvStopMediaReplySignal(
    const QString &remote_vuri,
    MediaStreamType type,
    bool permission);

  // ���յ�Զ���ն˵�ý��������״̬
  void NotifyRecvSendMediaControlSignal(
    const QString &remote_uri,
    MediaControlType type,
    bool send);

  // ���յ�ý����������Ϣ
  void NotifyRecvMediaControlInfoSignal(
    const QString &terminal_uri, // Զ���ն˻򱾵��ն�
    MediaControlType type,
    bool send);

  // �յ�QoS�����½����Ϣ
  void NotifyRecvQoSLoginPermissionSignal(const LoginPermissionInfo &info);
  // �յ�QoS�ܾ���½����Ϣ
  void NotifyRecvQoSLoginRejectionSignal(const LoginRejectionInfo &info);
  // �յ�QoS��������Ϣ
  void NotifyRecvQoSServerInfoSignal(const QoSServerInfo &info);

  // �յ�ѡ�������˵���Ϣ
  void NotifyRecvPermitSpeakSignal(const QString &uri);
  // �յ�ȡ�������˵���Ϣ
  void NotifyRecvForbidSpeakSignal(const QString &uri);
  // �յ���Ϊ�����˵��ն�
  void NotifyRecvSpeakerTerminalSignal(const QString &uri);

  // �յ�ѡ����ϯ����Ϣ
  void NotifyRecvChairmanUpdateSignal(const QString &uri);
  // �յ���Ϊ��ϯ���ն�
  void NotifyRecvChairmanSignal(const QString &uri);

  // �յ���Ϊ������Ļ���ն�
  void NotifyRecvSharedScreenTerminalSignal(const QString &uri);
  // �յ�������Ļ�ն˵ķ���״̬
  void NotifyRecvSharedScreenControlSignal(const QString &uri, bool enable);

  // �յ�sip��������������Ϣ
  // typeΪtest��heartbeat
  void NotifyRecvOnlineMessageSignal(const QString &type);
  
public slots:
  // �ն���sip��������½
  void SendTerminalLoginInfoSlot(
    const SipController::TerminalLoginInfo &info);

  // �ն�������������ն���ʵԴ��ַ��֤��Ϣ
  void SendTerminalInfoForSAVASlot(
    const SipController::TerminalSAVAInfo &info);

  // ���ڵ�½����Sip�������ı���
  void SendRegisterHeartBeatToSipServerSlot();

  // ���ڵ�½�������������ı���
  void SendOnlineHeartBeatToKeepLoginSlot();

  // ���ڵ�½�������������������߲�����Ϣ
  void SendOnlineTestSlot();

  // �ն���sip�������ǳ�
  void SendTerminalLogoutInfoSlot();

  // ��������б�
  void RequestConferenceListSlot();

  // ����������
  void SendConferenceJoinInfoSlot(
    const SipController::ConferenceJoinInfo &conference_info,
    const SDPInfo &sdp_info);

  // ���ֱ����ն��ڻ���������
  void SendOnlineHeartBeatToKeepInConferenceSlot(
    const QString &conference_uri);
  
  // �����뿪������Ϣ
  void SendConferenceLeaveInfoSlot(
    const QString &conference_uri);

  // ��ȡ�ն��б�
  void RequestTerminalListSlot(const QString &conference_uri);

  // ��ѯ��ǰ��������
  void QueryMainSpeakerSlot(const QString &conference_uri);
  // ��ѯ��ϯ�ն�
  void QueryChairmanSlot(const QString &conference_uri);
  // ��ѯ������Ļ�ն�
  void QueryShareScreenSlot(const QString &conference_uri);

  // ���;�����Ϣ
  void SendHandUpMsgSlot(
    const QString &conference_uri,
    const QString &local_uri,
    bool handup);

  // ����ѡ����������Ϣ
  void SendAllowSpeakMsgSlot(
    const QString &conference_uri,
    const QString &terminal_uri,
    bool allow);

  // ����Զ���ն��򱾵��ն˷���ý����
  void RequestStartMediaSlot(
    const QString &conference_uri,
    const QString &remote_uri,
    const QString &remote_vuri,
    const QString &local_ip_addr,
    MediaStreamType type);

  // �ظ�Զ���ն��򱾵��ն�������ý����������
  void ReplyStartMediaRequestSlot(
    const QString &conference_uri,
    const QString &remote_uri,
    const QString &local_vuri,
    MediaStreamType type,
    bool permission);

  // ����Զ���ն��򱾵��ն�ֹͣ����ý����
  void RequestStopMediaSlot(
    const QString &conference_uri,
    const QString &remote_uri,
    const QString &remote_vuri,
    const QString &local_ip_addr,
    MediaStreamType type);

  // �ظ�Զ���ն��򱾵��ն���������ֹͣ����ý����������
  void ReplyStopMediaRequestSlot(
    const QString &conference_uri,
    const QString &remote_uri,
    const QString &local_vuri,
    MediaStreamType type,
    bool permission);

  // ��֪���в����նˣ������ն˹�����Ļ�ķ���״̬
  // enableΪtrue����Ϊ����״̬
  // enableΪfalse����Ϊֹͣ״̬
  void SendScreenShareControlSlot(
    const QString &conference_uri,
    const QString &ip,
    const QString &ppt_port, 
    bool enable);

  // ��֪���вλ��նˣ������ն�ý�巢��״̬
  // enableΪtrue����Ϊ����״̬
  // enableΪfalse����Ϊֹͣ״̬
  void SendMediaControlInfoSlot(
    const QString &conference_uri,
    const QString &local_uri, // �����ն�uri
    MediaControlType type,
    bool enable);

private slots:
  // ������SipWrapper���յ�����Ϣ
  void HandleMessageReceivedNotifySlot(const QString &msg, const QString &from);

private:
  // StartMedia��StopMedia����Ϣ�У�С��ʹ���˲�ͬ������
  // ����start�������ö�Ӧ������
  QString ConvertToQString(MediaStreamType type, bool start) const;
  MediaStreamType ConvertToMediaStreamType(const QString &type);

  QString ConvertToQString(MediaControlType type) const;
  MediaControlType ConvertToMediaControlType(const QString &type) const;

  // �ж�ip��ַ�ǲ���ipv6��ַ
  bool IsIpv6Address(const QString &ip) const;

  // ע�����е�sip��Ϣ��������
  void RegistHandlers();

  // ���������ն˵�¼����֤����Ϣ
  void RecvTerminalLoginNotifyHandler(const SipMsgParser &parser, const QString &from);

  // ����������ص���Ϣ
  void RecvConferenceListHandler(const SipMsgParser &parser, const QString &from);
  void RecvTerminalListHandler(const SipMsgParser &parser, const QString &from);
  void RecvTerminalLoginHandler(const SipMsgParser &parser, const QString &from);
  void RecvTerminalLogoutHandler(const SipMsgParser &parser, const QString &from);
  void RecvTerminalHandUpHandler(const SipMsgParser &parser, const QString &from);
  void RecvTerminalHandDownHandler(const SipMsgParser &parser, const QString &from);

  // ����ý������ص���Ϣ
  void RecvStartMediaReplyHandler(const SipMsgParser &parser, const QString &from);
  void RecvStartMediaRelayHandler(const SipMsgParser &parser, const QString &from);
  void RecvStopMediaReplyHandler(const SipMsgParser &parser, const QString &from);
  void RecvStopMediaRelayHandler(const SipMsgParser &parser, const QString &from);
  void RecvSendMediaControlHandler(const SipMsgParser &parser, const QString &from);
  void RecvMediaControlInfoHandler(const SipMsgParser &parser, const QString &from);

  // ����QoS��ص���Ϣ
  void RecvQoSMsgHandler(const SipMsgParser &parser, const QString &from);

  // ������������ص���Ϣ
  void RecvSetSpeakerTerminalHandler(const SipMsgParser &parser, const QString &from);
  void RecvSpeakerTerminalInfoHandler(const SipMsgParser &parser, const QString &from);
  void RecvForbidSpeakerHandler(const SipMsgParser &parser, const QString &from);
  // ������ϯ�ն���ص���Ϣ
  void RecvSetChairmanTerminalHandler(const SipMsgParser &parser, const QString &from);
  void RecvChairmanTerminalInfoHandler(const SipMsgParser &parser, const QString &from);
  // ������Ļ��������ص���Ϣ
  void RecvShareScreenTerminalInfoHandler(const SipMsgParser &parser, const QString &from);
  void RecvShareScreenControlInfoHandler(const SipMsgParser &parser, const QString &from);
  
  // ������������������Ϣ
  void RecvOnlineMessageInfoHandler(const SipMsgParser &parser, const QString &from);

private:
  typedef std::function<
    void(SipController&, const SipMsgParser&, const QString&)
  > Handler;

  typedef QHash<QString, Handler> HandlerDictionary;

  CvSipWrapperIf *sip_wrapper_proxy_;
  HandlerDictionary type_handler_;

  QString username_;
  QString regist_content_;
  QString ip_addr_;
  QString global_service_;
};

#endif // SIP_CONTROLLER_H
