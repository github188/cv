#ifndef _TX_CONTROLLER_INTERFACE_H_
#define _TX_CONTROLLER_INTERFACE_H_

#include <QObject>

#include "session_controller_interface.h"
#include "conference_controller_interface.h"
#include "terminal_description.h"
#include "msg_manager_interface.h"
#include "util/report/RecordStat.h"
#include "util/report/RtpStat.h"

struct RecordControlMsg;

class ITXController : public QObject {
  Q_OBJECT

 public:
  ITXController(QObject *parent) : QObject(parent) {}
  virtual ~ITXController() {}
  
  static void RegisterMetaType() {
    qRegisterMetaType<RecStatItem>("RecStatItem");
  }

  virtual QString GenerateRecordFilePath(TerminalPointer terminal) = 0;
  virtual int GetRecordInitialJobId(const QString &vuri) = 0;
  //�Ƿ�������¼�Ƶ���ҵ
  virtual bool HasRecording() = 0;

 Q_SIGNALS:
  void StartReceiveVideoRequestSignal(const QString &vuri);
  void StopReceiveVideoRequestSignal(const QString &vuri);

  //֪ͨ��Ƶ¼�Ƶķ�������Ϣ�����һ������UI���
  void NotifyRecordRequesterSignal(const QString &vuri, 
    const QString &requester_uri);
  //֪ͨ�յ�¼��״̬����
  void NotifyRecStatReceivedSignal(const RecStatItem &rec_state);
  //Զ�̿������
  void SendRecordControlRequestAckSignal(const RecordControlAckParam &param);
  //֪ͨ����¼�ƶ��ѽ���
  void NotifyAllRecordJobFinishSignal();

 public Q_SLOTS: 
  // ����¼��״̬����
  virtual void HandleReceiveUDPRecReportSlot(
    const RecStatItem& recstat) = 0;
  // ����QoS���桪����ʱ�ģ�����ȡ�÷��Ͷ�IP
  virtual void HandleReceiveUDPQoSReportSlot(
    const RtpStatItem& rtp_stat) = 0;
  // �����ն�״̬���
  virtual void HandleTerminalStatusChangedSlot(
    ConstTerminalPointer terminal,
    const QString &conference_uri) = 0;
  // �����ն��ص�½
  virtual void HandleTerminalReloginSlot(
    ConstTerminalPointer terminal,
    const QString &conference_uri) = 0;
  // �ֶ�����
  virtual void HandleSegmentMediaRequestSlot(const QString &vuri) = 0;
  // ��½״̬���ʱ������Ӧ
  virtual void HandleSessionStateChangedNotifySlot(
    ISessionController::SessionState state) = 0;
  // ����״̬���
  virtual void HandleConferenceStateChangedSlot(
    const QString &conference_uri, 
    IConferenceController::ConferenceState state) = 0;

  // Զ��¼�������ط���
  virtual void HandleRecvRecordControlSlot(const RecordControlMsg &info) = 0;
};

#endif // _TX_CONTROLLER_INTERFACE_H_