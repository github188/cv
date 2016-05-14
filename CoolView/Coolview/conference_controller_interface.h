#ifndef _CONFERENCE_CONTROLLER_INTERFACE_H_
#define _CONFERENCE_CONTROLLER_INTERFACE_H_

#include <QObject>

class QRect;

#include "terminal_description.h"
#include "conference_description.h"
#include "terminal_manager.h"

#include "util/report/RtpStat.h"
#include "msg_receiver_interface.h"

enum MediaStreamType;
enum MediaControlType;

class IConferenceController : public QObject {
  Q_OBJECT
 public:
  enum ConferenceState {
    kIsJoiningConference = 0,
    kIsInConference,
    kIsLeavingConference,
    kIsNotInConference,
  };

  enum JoinConferenceError {
    kOk = 0,
    kUnknownError,
    kErrorOfIsAlreadyInOneConference,
    kErrorOfIsJoiningOneConference,
    kErrorOfConferenceNotExist,
    kErrorOfJoinRejected,
    kErrorOfIsLeavingConference,
    kErrorOfPasswordNeeded,
  };

 public:
  IConferenceController(QObject *parent) : QObject(parent) {}
  virtual ~IConferenceController() {}

  static void RegisterMetaType() {
    qRegisterMetaType<IConferenceController::ConferenceState>(
      "IConferenceController::ConferenceState");
    qRegisterMetaType<IConferenceController::JoinConferenceError>(
      "IConferenceController::JoinConferenceError");
    qRegisterMetaType<RtpStatItem>("RtpStatItem");
  }

  // Ϊ���̰߳�ȫ��һ��public��Ա������ֻ�ܹ�controller����
  virtual bool IsInConference() const = 0;
  virtual bool IsMulticastConference() const = 0;
  virtual ConferenceState GetCurrentState() const = 0;
  virtual ConferencePointer GetCurrentConference() const = 0;
  virtual TerminalManager &GetTerminalManager() /*const*/ = 0;

 Q_SIGNALS:
  void NotifyConferenceStateChangedSignal(
    const QString &conference_uri, 
    IConferenceController::ConferenceState state);

  void NotifyJoinConferenceErrorSignal(
    const QString &conference_uri, 
    IConferenceController::JoinConferenceError error);

  void NotifyConferenceListReceivedSignal(
    const ConstConferenceList &conference_list);

  void NotifyTerminalReceivedSignal(
    ConstTerminalPointer terminal,
    const QString &conference_uri);

  void NotifyTerminalRemovedSignal(
    const QString &terminal_uri,
    const QString &conference_uri);

  void NotifyTerminalUpdateSignal(
    ConstTerminalPointer terminal,
    const QString &conference_uri);
  
  void NotifyTerminalReloginSignal(
    ConstTerminalPointer terminal,
    const QString &conference_uri);

  void NotifyTerminalCountChangedSignal(
    int count, 
    const QString &conference_uri);

  // ֪ͨ����remote_vuri������Ƶ
  void NotifyStreamMediaReceiveStartedSignal(const QString &remote_vuri);

  // ֪ͨ����remote_vuri�Ļظ����
  void NotifyStreamMediaReceiveReplySignal(const QString &remote_vuri, bool permission);
  
  // ֹ֪ͨͣ����remote_vuri������Ƶ
  void NotifyStreamMediaReceiveStoppedSignal(const QString &remote_vuri);

  //���DisplayController�б������Ƶ�б�   //  [12/10/2015 slt begin]
  void NotifyEraseAudioDisplayDictSignal();

  // ֪ͨ����remote_uri�Ĺ�����Ļ
  void NotifyScreenReceiveStartedSignal(
    const QString &remote_uri,
    int screen_index);

  // ֹ֪ͨͣ����remote_uri�Ĺ�����Ļ
  void NotifyScreenReceiveStoppedSignal(
    const QString &remote_uri, 
    int screen_index);
    
  // ֪ͨ����terminal����
  void NotifyTerminalSpeakSinal(const QString &remote_uri, bool allow);

  void NotifyRTPStatReceivedSignal(const RtpStatItem &rtp_state);
  
  void NotifyTerminalHandUpSignal(bool handup);



  // �����Զ�����remote_vuri������Ƶ
  void RequestStreamMediaAutoReceiveSignal(const QString &remote_vuri);



 public Q_SLOTS:
  virtual void HandleJoinConferenceRequestSlot(const QString &uri) = 0;
  virtual void HandleLeaveConferenceRequestSlot() = 0;
  virtual void HandleConferenceListRequestSlot() = 0;
  virtual void HandleTerminalListRequestSlot() = 0;
  virtual void HandleHandUpRequestSlot(bool hand_up) = 0;

  // ����QoS��Ϣ��QoS������
  virtual void HandleReceiveUDPQoSReportSlot(
    const RtpStatItem& rtp_stat) = 0;

  // ��ָ���û�����ý����
  // seat: ��Ƶ��������λ�ã�1��ʾ��һ�����ڣ��Դ�����
  virtual void HandleAddRecvStreamMediaRequestSlot(
    const QString &remote_vuri,
    int screen_index, 
    int seat,
    bool use_small_video,
    bool enable_audio) = 0;

  // ��ָ���û�����ý�����󣬴���ȷ�ϳ�ʱ
  virtual void HandleStartMediaReplyTimeoutSlot(
    const QString &remote_vuri) = 0;

  // ��֪Զ���û�ֹͣ������Ƶ��������
  virtual void HandleRemoveRecvStreamMediaRequestSlot(
    const QString &remote_vuri) = 0;

  // �����Ƿ�����Զ���ն˷���
  virtual void HanldePermitTerminalSpeakRequestSlot(
    const QString &uri, 
    bool allow) = 0;

  // �ı�������ʾλ��
  virtual void HandleChangeVideoSeatRequestSlot( 
    const QString &remote_vuri,
    int screen_index,
    int seat) = 0;

  // �������ط��ͽ���
  virtual void HandleCreateSendStreamMediaRequestSlot() = 0;

  // ��֪���вλ��ն˵�ý�巢��״̬�������ж�ý�������п���
  // type: ý�����ͣ�ֻ֧��kAudioControlType��kVideoControlType
  // enable: �������ý��������Ϊtrue������Ϊfalse
  // virtual_index: ָ��Ϊ��һ·��Ƶ������ƵΪ����·
  virtual void HandleControlSendMediaRequestSlot(
    MediaControlType type,
    bool enable,
    int virtual_index = 0) = 0;

  // �������͹�����Ļ��
  virtual void HandleCreateSendScreenMediaRequestSlot(const QRect &wnd) = 0;

  // �������չ�����Ļ��
  virtual void HandleCreateRecvScreenMediaRequestSlot(
    const QString &remote_uri, 
    int screen_index) = 0;

  // ��֪Զ���û�ֹͣ������Ļ��
  virtual void HandleRemoveRecvScreenMediaRequestSlot(
    const QString &remote_uri,
    int screen_index) = 0;

  // ���Ʊ�����Ļ������/ֹͣ
  // ͬʱ��֪�����ն˴���/���ٵ�ǰ�ն˵���Ļ�����ս���
  virtual void HandleScreenShareControlRequestSlot(bool enable) = 0;

  // �ָ�ý�����
  virtual void HandleRecoveryMediaProcessRequestSlot(
    MediaStreamType type, 
    bool is_send, 
    const QString &remove_user_id) = 0;

  virtual void HandleAllMediaProcessExitSlot() = 0;
};

#endif // _CONFERENCE_CONTROLLER_INTERFACE_H_