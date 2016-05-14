#ifndef MSG_RECEIVER_INTERFACE_H
#define MSG_RECEIVER_INTERFACE_H

#include <QObject>
#include <QHash>

#include "conference_description.h"
#include "terminal_description.h"
#include "qos_description.h"
#include "media_description.h"
#include "config_description.h"
#include "floor_control_info_description.h"

struct MediaRelayMsg {
  QString remote_uri;
  QString remote_ip;
  QString local_vuri;
  MediaStreamType type;
};

struct MediaReplyMsg {
  QString remote_uri;
  QString remote_vuri;
  MediaStreamType type;
  bool permission;
};

struct MediaStatusMsg {
  QString remote_uri;
  MediaControlType type;
  bool send;
};

struct MediaControlMsg {
  QString terminal_uri; // Զ���ն˻򱾵��ն�
  MediaControlType type;
  bool send;
};

struct RecordControlMsg {
  QString requester_uri;
  QString requester_ip;
  QString tx_uri;
  QString target_vuri;
  int transaction_id;
  int type;
  int port;
};

struct RecordControlAckMsg {
  QString requester_uri;
  QString tx_uri;
  QString target_vuri;
  int transaction_id;
  int error_code;
};

class IMsgReceiver : public QObject {
  Q_OBJECT
public:
  IMsgReceiver(QObject *parent) : QObject(parent) {

  }
  virtual ~IMsgReceiver() {};

  static void RegisterMetaType() {
    qRegisterMetaType<LoginPermissionInfo>("LoginPermissionInfo");
    qRegisterMetaType<LoginRejectionInfo>("LoginRejectionInfo");
    qRegisterMetaType<QoSServerInfo>("QoSServerInfo");

    qRegisterMetaType<MediaRelayMsg>("MediaRelayMsg");
    qRegisterMetaType<MediaReplyMsg>("MediaReplyMsg");
    qRegisterMetaType<MediaStatusMsg>("MediaStatusMsg");
    qRegisterMetaType<MediaControlMsg>("MediaControlMsg");

    qRegisterMetaType<RecordControlMsg>("RecordControlMsg");
    qRegisterMetaType<RecordControlAckMsg>("RecordControlAckMsg");

    MediaHelper::RegisterMetaType();
    TerminalHelper::RegisterMetaType();
    ConferenceHelper::RegisterMetaType();
    ConfigRegister::RegisterMetaType();
    FloorControlInfoRegister::RegisterMetaType();
  }

Q_SIGNALS:
  // ���յ������½Sip��������״̬��Ӧ
  // state��ֵ�ο�enum SipAccountState
  void NotifySipAccountStateChangedSignal(const QString &name, int state);

  // ���յ������ն��Ƿ�����ʵԴ��ַ��֤
  void NotifyRecvSAVAResultSignal(bool valid);

  // ���յ��ն�������Ϣ
  void NotifyRecvTerminalConfigSignal(const ConfigDict &config);

  // ���յ�������Ϣ
  void NotifyRecvConferenceListSignal(const ConferenceList &conferences);
  void NotifyRecvFloorControlInfoSignal(const FloorControlInfoPointer &floor_control_info);
  void NotifyRecvTerminalListSignal(const TerminalList &terminals);
  void NotifyRecvTerminalLoginSignal(const TerminalList &terminals);
  void NotifyRecvTerminalLogoutSignal(const QString &uri);
  void NotifyRecvTerminalHandUpSignal(const QString &uri);
  void NotifyRecvTerminalHandDownSignal(const QString &uri);

  // ���յ�Զ���ն��򱾵��ն��������䷢��ý����
  void NotifyRecvStartMediaRelaySignal(const MediaRelayMsg &info);

  // ���յ�Զ���ն˶Ա����ն�����������ý�����Ļظ�
  void NotifyRecvStartMediaReplySignal(const MediaReplyMsg &info);

  // ���յ�Զ���ն��򱾵��ն�����ֹͣ���䷢��ý����
  void NotifyRecvStopMediaRelaySignal(const MediaRelayMsg &info);

  // ���յ�Զ���ն˶Ա����ն���������ֹͣ����ý�����Ļظ�
  void NotifyRecvStopMediaReplySignal(const MediaReplyMsg &info);

  // ���յ�Զ���ն˵�ý��������״̬
  void NotifyRecvRemoteMediaStatusSignal(const MediaStatusMsg &info);

  // ���յ�ý����������Ϣ
  void NotifyRecvMediaControlInfoSignal(const MediaControlMsg &info);

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
  void NotifyRecvSharedScreenControlSignal(const QString &ip, bool enable);

  // �յ�sip��������������Ϣ
  // typeΪtest��heartbeat
  void NotifyRecvOnlineMessageSignal(const QString &type, const QString &from);

  void NotifyTextMessageSignal(const QString &text, const QString &sender);

  // �յ�¼�ƿ��Ƶ�����
  void NotifyRecvRecordControlSignal(const RecordControlMsg &msg, const QString &from);
  // �յ�¼�ƿ��ƵĻظ�
  void NotifyRecvRecordControlAckSignal(const RecordControlAckMsg &msg, const QString &from);
};

#endif // MSG_RECEIVER_INTERFACE_H
