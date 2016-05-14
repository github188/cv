#ifndef MSG_SENDER_INTERFACE_H
#define MSG_SENDER_INTERFACE_H

#include <QObject>

#include <dbus/sipwrapper/common/SipWrapperCommonService.h>
#include "media_description.h"
#include "config_description.h"

struct TerminalLoginParam {
  QString display_name; // �ն���ʾ��
  QString username; // �ն��û���
  QString identity; // �ն˱�ʶ
  QString password; // ����
  QString realm;    // �������������
  QString proxy_server;    // �����������ַ
  QString register_server; // ע���������ַ
  QString ip_address; // �ն˵�ip��ַ
  QString mac;        // �ն˵�mac��ַ
  QString global_service; // ȫ�ֻ��������
};

struct TerminalSAVAParam {
  QString ipv4_addr; // �ն˵�ipv4��ַ
  QString ipv6_addr; // �ն˵�ipv6��ַ
  QString terminal_version; // �ն˵İ汾��
  int screen_count; // �ն˵���ʾ����Ŀ
};

struct ConferenceJoinParam {
  QString cid; // ����id
  QString focus_uri; // �����uri
  QString uri;  // �ն˵�uri
  int min_rate; // ��ʹ���ֵ
  int max_rate; // ������ֵ
  QString ip_addr;   // �ն˵�ip��ַ
  QString gateway; // �ն˵�����
  int virtual_terminal_count; // �ն˵������ն���
};

struct MediaRelayParam {
  QString conference_uri;
  QString remote_uri;
  QString remote_vuri;
  QString local_ip_addr;
  MediaStreamType type;
};

struct MediaReplyParam {
  QString conference_uri;
  QString remote_uri;
  QString local_vuri;
  MediaStreamType type;
  bool permission;
};

struct ScreenControlParam {
  QString conference_uri;
  QString terminal_uri;
  QString ip_addr;
  QString ppt_port;
  bool enable;
};

struct MediaControlParam {
  QString conference_uri;
  QString local_uri; // �����ն�uri
  MediaControlType type;
  bool enable;
};

struct RecordControlParam {
  QString conference_uri;
  QString requester_uri;
  QString requester_ip;
  QString tx_uri;
  QString target_vuri;
  int transaction_id;
  int type;
  int port;
};

struct RecordControlAckParam {
  QString conference_uri;
  QString requester_uri;
  QString tx_uri;
  QString target_vuri;
  int transaction_id;
  int error_code;
};

class IMsgSender : public QObject {
  Q_OBJECT
public:
  IMsgSender(QObject *parent) : QObject(parent) {}
  virtual ~IMsgSender() {}

  static void RegisterMetaType() {
    qRegisterMetaType<TerminalLoginParam>("TerminalLoginParam");
    qRegisterMetaType<TerminalSAVAParam>("TerminalSAVAParam");
    qRegisterMetaType<ConferenceJoinParam>("ConferenceJoinParam");
    qRegisterMetaType<MediaRelayParam>("MediaRelayParam");
    qRegisterMetaType<MediaReplyParam>("MediaReplyParam");
    qRegisterMetaType<ScreenControlParam>("ScreenControlParam");
    qRegisterMetaType<MediaControlParam>("MediaControlParam");
    qRegisterMetaType<SDPInfo>("SDPInfo");
    qRegisterMetaType<RecordControlParam>("RecordControlParam");
    qRegisterMetaType<RecordControlAckParam>("RecordControlParamAck");
  }

public Q_SLOTS:
  // �ն���sip��������½
  virtual void SendTerminalLoginInfoSlot(
    const TerminalLoginParam &info) = 0;

  // �ն�������������ն���ʵԴ��ַ��֤��Ϣ
  virtual void SendTerminalInfoForSAVASlot(
    const TerminalSAVAParam &info) = 0;

  // ���ڵ�½����Sip�������ı���
  virtual void SendRegisterHeartBeatToSipServerSlot() = 0;

  // ���ڵ�½�������������ı���
  virtual void SendOnlineHeartBeatToKeepLoginSlot() = 0;

  // ���ڵ�½�������������������߲�����Ϣ
  virtual void SendOnlineTestSlot() = 0;

  // �ն���sip�������ǳ�
  virtual void SendTerminalLogoutInfoSlot() = 0;

  // ��������������Ϣ
  virtual void SendGetTerminalConfigSlot() = 0;

  // �����ն�������Ϣ
  virtual void SendReportTerminalConfigSlot(const ConfigDict &param) = 0;

  // ��������б�
  virtual void RequestConferenceListSlot() = 0;

  // ������Ȩ������Ϣ
  virtual void RequestFloorControlInfoSlot(const QString &cid) = 0;

  // ����������
  virtual void SendConferenceJoinInfoSlot(
    const ConferenceJoinParam &conference_info,
    const SDPInfo &sdp_info) = 0;

  // ���ֱ����ն��ڻ���������
  // TODO: ���ʱ���
  virtual void SendOnlineHeartBeatToKeepInConferenceSlot(
    const QString &conference_uri) = 0;

  // �����뿪������Ϣ
  virtual void SendConferenceLeaveInfoSlot(
    const QString &conference_uri) = 0;

  // ��ȡ�ն��б�
  virtual void RequestTerminalListSlot(const QString &conference_uri) = 0;

  // ��ѯ��ǰ��������
  virtual void QueryMainSpeakerSlot(const QString &conference_uri) = 0;
  // ��ѯ��ϯ�ն�
  virtual void QueryChairmanSlot(const QString &conference_uri) = 0;
  // ��ѯ������Ļ�ն�
  virtual void QueryShareScreenSlot(const QString &conference_uri) = 0;

  // ���;�����Ϣ
  virtual void SendHandUpMsgSlot(
    const QString &conference_uri,
    const QString &local_uri,
    bool handup) = 0;

  // ����ѡ����������Ϣ
  virtual void SendAllowSpeakMsgSlot(
    const QString &conference_uri,
    const QString &terminal_uri, // Ϊ�����ն�uri��Զ���ն�uri
    bool allow) = 0;

  // ����Զ���ն��򱾵��ն˷���ý����
  virtual void RequestStartMediaSlot(const MediaRelayParam &param) = 0;

  // �ظ�Զ���ն��򱾵��ն�������ý����������
  virtual void ReplyStartMediaRequestSlot(const MediaReplyParam &param) = 0;

  // ����Զ���ն��򱾵��ն�ֹͣ����ý����
  virtual void RequestStopMediaSlot(const MediaRelayParam &param) = 0;

  // �ظ�Զ���ն��򱾵��ն���������ֹͣ����ý����������
  virtual void ReplyStopMediaRequestSlot(const MediaReplyParam &param) = 0;

  // ��֪���вλ��նˣ������ն˹�����Ļ�ķ���״̬
  // enableΪtrue����Ϊ����״̬
  // enableΪfalse����Ϊֹͣ״̬
  virtual void SendScreenShareControlSlot(const ScreenControlParam &param) = 0;

  // ��֪���вλ��նˣ������ն�ý�巢��״̬
  // enableΪtrue����Ϊ����״̬
  // enableΪfalse����Ϊֹͣ״̬
  virtual void SendMediaControlInfoSlot(const MediaControlParam &param) = 0;

  // ֪ͨTX��Ŀ��vuri��ʼ/ֹͣ¼��
  virtual void SendRecordControlSlot(const RecordControlParam &param) = 0;

  // TX�������ߵĻظ�
  virtual void SendRecordControlAckSlot(const RecordControlAckParam &param) = 0;
};


#endif // MSG_SENDER_INTERFACE_H
