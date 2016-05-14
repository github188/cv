#ifndef TX_CONTROLLER_H
#define TX_CONTROLLER_H

#include <QTimer>
#include <QHash>
#include <QList>

#include "tx_controller_interface.h"

#include "session_controller_interface.h"
#include "conference_controller_interface.h"

class MediaManager;
class IConferenceController;
class IDisplayController;
class IMsgManager;
class RunningConfig;
class RunningProfile;
class UDPReportSender;
class TxMonitorIf;

class StartRecordParam;
class StopRecordParam;
class RecordTerminalStatus;
struct RecordControlMsg;

class TXController : public ITXController {
  Q_OBJECT

 public:
  TXController(QObject *parent = nullptr,
               RunningProfile *profile = nullptr,
               RunningConfig *config = nullptr);
  ~TXController();

  void Initialize(
    ISessionController *session_controller,
    IMsgManager *msg_manager,
    IConferenceController *conference_controller,
    IDisplayController *display_controller,
    MediaManager *media_manager);

  QString GenerateRecordFilePath(TerminalPointer terminal) override;
  int GetRecordInitialJobId(const QString &vuri) override;
  bool HasRecording() override;
  
 public Q_SLOTS: 
  //
  // ����UDP����
  //
  // ����¼��״̬����
  void HandleReceiveUDPRecReportSlot(
    const RecStatItem &rec_stat) override; // connect in CoreService2
  // ����QoS����
  void HandleReceiveUDPQoSReportSlot(
    const RtpStatItem &rtp_stat) override; // connect in CoreService2

  //
  // ����ConferenceController�ź�
  //
  // �ն�״̬������Զ�����¼��
  void HandleTerminalStatusChangedSlot(
    ConstTerminalPointer terminal,
    const QString &conference_uri) override;
  // �ն��ص�½
  void HandleTerminalReloginSlot(
    ConstTerminalPointer terminal,
    const QString &conference_uri) override;
  // ��Ƶ�ֶ�
  void HandleSegmentMediaRequestSlot(const QString &uri_or_vuri) override;
  // ��¼״̬���ʱ������Ӧ
  void HandleSessionStateChangedNotifySlot(
    ISessionController::SessionState state) override;
  // ����״̬���
  void HandleConferenceStateChangedSlot(
    const QString &conference_uri, 
    IConferenceController::ConferenceState state) override;

  //
  // ͨ��SIPԶ�̿���¼���ѷ�����
  //
  void HandleRecvRecordControlSlot(const RecordControlMsg &info) override;

  //
  // ����¼���DBus�ӿ�
  //
  int ControlRecord(int op, const QByteArray &params);
  QByteArray GetRecordStatus(const QString &focus, const QString &uri);

protected slots:
  // ��ý����״̬���������Ӧ
  void HandleStreamMediaReceiveStartedSlot(const QString &remote_vuri);
  void HandleStreamMediaReceiveStoppedSlot(const QString &remote_vuri);
  // �ӳ������Զ�¼��
  void HandleAutoRecordTimerTimeoutSlot();

 private:

  struct ViewRecordNode {
    QString vuri;
    RecStatItem record_status; //��������״̬
    bool start;
    bool complete;
  };
  typedef std::shared_ptr<ViewRecordNode> ViewRecordPointer;
  typedef QList<ViewRecordPointer> ViewRecordList;

  struct EpisodeUserInfo {
    QString title;
    QString participants;
    QString keywords;
    QString description;
  };
  struct EpisodeSysInfo {
    int job_id; //���ں͵ײ㱨���¼����Ϣ���
    long long start_time; //�ϸ������ĸ߾���ʱ���������Ϊ���ݿ��е�id
    bool segmented; //���ڷ�ֹ�ظ��ֶ�
    ViewRecordList views; //�ּ��µ��ӽ��б�
  };
  typedef std::shared_ptr<EpisodeSysInfo> EpisodeSysInfoPointer;
  typedef QList<EpisodeSysInfoPointer> EpisodeList;

  struct TerminalRecordNode {
    QString uri;
    QString name; //�����ն�����Ϊʲô��ô����ViewRecordComplete����
    QString requester_uri; //����¼�Ƶ���
    int next_job_id;
    //QString source_ip; //���ͷ���ַ
    //int report_port; //���ͷ�����¼�Ʊ���Ķ˿�
    //std::shared_ptr<UDPReportSender> reporter; //¼�񱨸淢����
    EpisodeUserInfo episode_user_info; //�û������¼����Ϣ
    EpisodeList episodes;
  };
  typedef std::shared_ptr<TerminalRecordNode> TerminalRecordPointer;
  typedef QHash<QString, TerminalRecordPointer> TerminalRecordDict; //key:uri

  //�ж�¼����Ƶ�Ƿ�Ӧ�÷ֶ�
  bool ShouldNewEpisodeStart(ViewRecordPointer view);
  //����Զ������¼������
  int RemoteStartRecord(const StartRecordParam &info); 
  //����Զ��ֹͣ¼������
  int RemoteStopRecord(const StopRecordParam &info); 
  //����¼��
  TerminalRecordPointer StartRecord(
    const StartRecordParam &param, 
    bool manual);
  //�ӽǵ�һ��¼�����
  void ViewRecordComplete(const QString &vuri, const RecStatItem &rec_stat);
  //¼���ļ���Ч
  void ViewRecordAbandon(const QString &vuri, const RecStatItem &rec_stat);
  //¼�������ֹͣ
  void RecordProcessExit(const QString &vuri);

  //��dict�в����µ�node��������;���Ѵ��ڣ�ʧ���򷵻�nullptr
  TerminalRecordPointer AddTerminalNode(const QString uri_or_vuri);
  //��TerminalRecordNode�в����µ�episode��ʧ���򷵻�nullptr
  EpisodeSysInfoPointer AddEpisodeNode(TerminalRecordPointer terminal);
  //��episode�в����µ�ViewRecordNode�����Ѵ����򷵻����еģ�ʧ���򷵻�nullptr
  ViewRecordPointer AddViewNode(EpisodeSysInfoPointer episode, const QString &vuri);
  //�ҵ�������Episode��ָ�룬û���򷵻�nullptr
  EpisodeSysInfoPointer FindEpisodeNode(const QString &uri_or_vuri, int job_id);
  //�ҵ�������ViewRecordNode��ָ�룬û���򷵻�nullptr
  ViewRecordPointer FindViewNode(const QString &vuri, int job_id);
  //ɾ��node
  void RemoveFinishedEpisodeNode(const QString &uri);

 private:

  RunningProfile *profile_;
  RunningConfig *running_config_;
  IConferenceController *conference_controller_;
  MediaManager *media_manager_;
  TerminalManager *terminal_manager_; //ָ��conference_controller_�еĶ���

  std::shared_ptr<TxMonitorIf> tx_monitor_;

  QString recording_path_; //¼���ļ��洢��Ŀ¼
  bool is_tx_terminal_; //���ն��Ƿ�Ϊtx
  __int64 max_file_size_;
  __int64 max_file_duration_;

  //���е�ǰ����ָ������ã���ֹ�˳������ֹͣ¼��ʱ�ò���������Ϣ
  //������ÿ�ν������ʱ���£��˳�����ʱ���ܸ�ֵnullptr��
  ConferencePointer current_conference_;

  //����һ���ն��б�TerminalManager���ܲ�ѯ�����նˣ�
  typedef QVector<QString> TerminalUriList;
  TerminalUriList terminal_uri_list_;
  
  TerminalRecordDict terminal_record_dict_;
  QHash<QString, QString> username_to_vuri_dict_;

  struct AutoStartPendingInfo {
    QString uri;
    time_t online_time;
  };
  QList<AutoStartPendingInfo> auto_record_pending_list_;
  QTimer *auto_record_timer_;
  int auto_record_delay_; //s

  static const int kAutoRecordCheckTimeOut = 1000; //ms
};

#endif // TX_CONTROLLER_H
