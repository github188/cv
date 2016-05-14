#ifndef TX_CLIENT_CONTROLLER_H
#define TX_CLIENT_CONTROLLER_H

#include "tx_client_controller_interface.h"

#include <QTimer>
#include <QThread>

class RunningProfile;
class RunningConfig;
class IConferenceController;
class IMsgManager;
class MediaManager;
class UDPReportSender;
namespace tx_soap {
class CvTxProxy;
}

class TXClientController : public ITXClientController {
  Q_OBJECT

 public:
  TXClientController(QObject *parent = nullptr,
                     RunningProfile *profile = nullptr,
                     RunningConfig *config = nullptr);
  ~TXClientController();

  void Initialize(
    IMsgManager *msg_manager,
    IConferenceController *conference_controller,
    MediaManager *media_manager);

  QString GenerateLocalRecordFilePath(int virtual_index) override;

 public Q_SLOTS: 
  //
  // �����ն˱���¼��ķ���
  //
  // ������Ƶ�ɼ�״̬�ı�ʱ��������Ҫ�����Զ�¼��
  void HandleSendMediaStateChangedNotifySlot(const QString &media_id, int type) override;
  // ��������Ƶ¼�Ʒָ�����
  void HandleSegmentLocalRecordVideoRequestSlot() override;
  // �������״̬���
  void HandleConferenceStateChangedSlot(
    const QString &conference_uri, 
    IConferenceController::ConferenceState state) override;

  //
  // Զ��¼�������ط���
  //
  void HandleOneVideoRecordRequestSlot(const QString &user_id, int type) override; //connect in CoreService2
  void HandleWholeVideoRecordRequestSlot(const QString &user_id, int type, QByteArray param) override; //connect in CoreService2
  void HandleRecvRecordControlAckNotifySlot(const RecordControlAckMsg &info) override;
  // ��ʱ��ȡ¼��״̬
  void HandleGetTxRecordStatusTimerTimeoutSlot();

 private:
  // ��TX����¼�ƿ���ָ��
  void SendRecordControlRequest(const QString &uri, int type, QByteArray param);
  // ����TXԶ�̵��ô���
  void CreateTxSoapProxy();
  // ����ConfRoom��ĳ�ն����������ն���ͼ��¼��״̬
  void ChangeRecordState(const QString &uri, int state);
  // ��vuriת����¼�Ʊ����user_name������Ǳ��նˣ���ת���ɱ��ػ���id
  inline void GetRecUDPReportUserNameFromVirtualUri(char *buf, int len, const QString &vuri);
  inline bool IsRecordCtrlAllow();
  inline bool IsSelfRecordCtrlAllow();
  inline bool IsOnlySelfRecordCtrlAllow();

 private:
  RunningProfile *profile_;
  RunningConfig *running_config_;
  IConferenceController *conference_controller_;
  TerminalManager *terminal_manager_; //ָ��conference_controller_�еĶ���
  MediaManager *media_manager_;

  QThread *thread_; //Զ�̵���ʱΪ�˲��������̣߳�������Ҫ�ڵ����߳�������

  QString recording_path_; //¼���ļ��洢��Ŀ¼
  QTimer *send_video_auto_cut_timer_; //����¼����Զ��ֶμ�ʱ��

  tx_soap::CvTxProxy *tx_soap_proxy_;
  QTimer *get_tx_record_status_timer_; //��ʱ��ȡTX¼��״̬�ļ�ʱ��
  UDPReportSender *rec_stat_sender_to_confroom_;
};

#endif
