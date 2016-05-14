#ifndef CONFERENCE_CONTROLLER_H
#define CONFERENCE_CONTROLLER_H

#include <QMutex>
#include <QHash>
#include <QTimer>
#include <QRect>
#include <QLinkedList>

#include "conference_controller_interface.h"
#include "stream_receiver_info_manager.h"
#include "terminal_manager.h"

#include "session_controller_interface.h"
#include "floor_controller_interface.h"

#include "util/udp/UdpSender.h"

class RunningProfile;
class RunningConfig;

class MediaManager;
class IMsgManager;
class IMsgReceiver;
class IMsgSender;

class IControllerManager;

struct ConferenceJoinParam;
struct MediaRelayParam;
struct MediaReplyParam;
struct MediaControlParam;
struct ScreenControlParam;

class SDPInfo;
struct LoginPermissionInfo;
struct LoginRejectionInfo;
struct QoSServerInfo;
struct MediaControlMsg;
struct MediaStatusMsg;
struct MediaRelayMsg;
struct MediaReplyMsg;

enum MediaControlType;
enum MediaStreamType;

class TxMinitorIf;
class CvPerfMonitorIf;

class HttpMsgManager;

class ConferenceController : public IConferenceController {
  Q_OBJECT
 public:
  enum MyEnum {
    kSendHearBeatInfoInterval = 60000,
    kServerHeartBeatTimeout = 180000,
    kTestTimeout = 30000,
    kCheckTerminalListInterval = 20000,
  };

 public:
  ConferenceController(QObject *parent,
                       RunningProfile *profile,
                       RunningConfig *config,
                       MediaManager *media_manager,
                       IControllerManager *controller_manager);

  ~ConferenceController();

  void Initialize(
    IMsgManager *msg_manager,
    HttpMsgManager *http_msg_manager,
    ISessionController *session_controller,
    IFloorController *floor_controller);

  bool IsInConference() const;
  bool IsMulticastConference() const;

  ConferenceState GetCurrentState() const override { return state_; }
  ConferencePointer GetCurrentConference() const override { return current_conference_; };
  TerminalManager &GetTerminalManager() override { return terminals_; };

 public Q_SLOTS:
  void HandleConferenceListRequestSlot() override;
  void HandleJoinConferenceRequestSlot(const QString &uri) override;
  void HandleLeaveConferenceRequestSlot() override;
  void HandleTerminalListRequestSlot() override;
  void HandleHandUpRequestSlot( bool handup ) override;

  // ����QoS��Ϣ��QoS������
  void HandleReceiveUDPQoSReportSlot(
    const RtpStatItem& rtp_stat) override;

  // ��ָ���û�����ý����
  // seat:��Ƶ��������λ�ã�1��ʾ��һ�����ڣ��Դ�����
  void HandleAddRecvStreamMediaRequestSlot(
    const QString &remote_vuri,
    int screen_index, 
    int seat,
    bool use_small_video,
    bool enable_audio) override;

  // ��ָ���û�����ý����������ȷ�ϳ�ʱ 
  void HandleStartMediaReplyTimeoutSlot(
    const QString &remote_vuri) override;

  // ��֪Զ���û�ֹͣ������Ƶ��������
  void HandleRemoveRecvStreamMediaRequestSlot(
    const QString &remote_vuri) override;

  // �ı�������ʾλ��
  // �ɵײ��߼��ж�
  void HandleChangeVideoSeatRequestSlot( 
    const QString &remote_vuri,
    int screen_index,
    int seat) override;

  // �����Ƿ�����Զ���ն˷���
  void HanldePermitTerminalSpeakRequestSlot(
    const QString &uri, 
    bool allow) override;

  // �������ط��ͽ���
  void HandleCreateSendStreamMediaRequestSlot() override;

  // ��֪���вλ��ն˵�ý�巢��״̬�������ж�ý�������п���
  // type: ý�����ͣ�ֻ֧��kAudioControlType��kVideoControlType
  // enable: �������ý��������Ϊtrue������Ϊfalse
  // virtual_index: ָ��Ϊ��һ·��Ƶ������ƵΪ����·
  void HandleControlSendMediaRequestSlot(
    MediaControlType type, 
    bool enable,
    int virtual_index = 0) override;

  // �������͹�����Ļ��
  void HandleCreateSendScreenMediaRequestSlot(const QRect &wnd) override;

  // �������չ�����Ļ��
  void HandleCreateRecvScreenMediaRequestSlot(
    const QString &remote_uri, 
    int screen_index) override;

  // ��֪Զ���û�ֹͣ������Ļ��
  void HandleRemoveRecvScreenMediaRequestSlot(
    const QString &remote_uri,
    int screen_index) override;

  // ���Ʊ�����Ļ������/ֹͣ
  // ͬʱ��֪�����ն˴���/���ٵ�ǰ�ն˵���Ļ�����ս���
  void HandleScreenShareControlRequestSlot(bool enable) override;

  // �ָ�ý�����
  void HandleRecoveryMediaProcessRequestSlot(
    MediaStreamType type, 
    bool is_send, 
    const QString &remove_user_id) override;

  void HandleAllMediaProcessExitSlot() override;

Q_SIGNALS:
  void SendConferenceListRequestMsgSignal();
  void SendJoinConferenceRequestMsgSignal(
    const ConferenceJoinParam &conference_param,
    const SDPInfo &sdp_info);
  void SendServerOnlineTestSignal();
  void SendHeartBeatToKeepInConferenceMsgSignal(const QString &conference_uri);
  void SendLeaveConferenceRequestMsgSignal(const QString &conference_uri);
  void SendTerminalListRequestMsgSignal(const QString &conference_uri);
  void SendMainSpeakerInfoRequestSignal(const QString &conference_uri);
  void SendChairmanInfoRequestSignal(const QString &conference_uri);
  void SendSharedScreenInfoRequestSignal(const QString &conference_uri);
  void SendHandUpMsgSignal(
    const QString &conference_uri,
    const QString &local_uri,
    bool handup);
  void SendAllowSpeakMsgSignal(
    const QString &conference_uri,
    const QString &terminal_uri,
    bool allow);
  void SendRequestStartMediaMsgSignal(const MediaRelayParam &param);
  void SendReplyStartMediaMsgSignal(const MediaReplyParam &param);
  void SendRequestStopMediaMsgSignal(const MediaRelayParam &param);
  void SendReplyStopMediaMsgSignal(const MediaReplyParam &param);
  void SendScreenControlMsgSignal(const ScreenControlParam &param);
  void SendMediaControlMsgSignal(const MediaControlParam &param);

  void SendTerminalListRequestByCIDSignal(const QString &cid);
  void SendFloorControlInfoRequest(const QString &cid);

  // ����Ȩ����
  void CreateFloorControlClientSignal(
    uint16_t userid, const FloorControlInfoPointer &info);
  void ApplyFloorRequestSignal();
  void AcceptFloorRequestSignal(uint16_t user_id);
  void ReleaseFloorRequestSignal(uint16_t user_id);
  void DestoryFloorControlClientSignal();

 private Q_SLOTS:
  void HandleSessionStateChangedNotifySlot(
    ISessionController::SessionState state);

  void HandleOnlineMessageNotifySlot(const QString &type, const QString &from);
  void HandleQoSLoginPermissionNotifySlot(const LoginPermissionInfo &info);
  void HandleQoSLoginRejectionNotifySlot(const LoginRejectionInfo &info);
  void HandleQoSServerInfoNotifySlot(const QoSServerInfo &info);
  void HandleConferenceListReceivedNotifySlot(const ConferenceList &conferences);
  void HandleTerminalListReceivedNotifySlot(const TerminalList &terminals);
  void HandleTerminalLoginNotifySlot(const TerminalList &terminals);
  void HandleTerminalLogoutNotifySlot(const QString &terminal_uri);
  void HandleTerminalHandUpNotifySlot(const QString &terminal_uri);
  void HandleTerminalHandDownNotifySlot(const QString &terminal_uri);
  void HandleTerminalPermitSpeakNotifySlot(const QString &terminal_uri);
  void HandleTerminalForbidSpeakNotifySlot(const QString &terminal_uri);
  void HandleSpeakerTerminalNotifySlot(const QString &terminal_uri);
  void HandleChairmanTerminalNotifySlot(const QString &terminal_uri);
  void HandleChairmanUpdateNotifySlot(const QString &terminal_uri);
  void HandleSharedScreenControlNotifySlot(const QString &sender_ip, bool enable);
  void HandleSharedScreenTerminalNotifySlot(const QString &terminal_uri);
  void HandleStartMediaRelayNotifySlot(const MediaRelayMsg &info);
  void HandleStartMediaReplyNotifySlot(const MediaReplyMsg &info);
  void HandleStopMediaRelayNotifySlot(const MediaRelayMsg &info);
  void HandleStopMediaReplyNotifySlot(const MediaReplyMsg &info);
  void HandleRemoteMediaStatusNotifySlot(const MediaStatusMsg &info);
  void HandleMediaControlNotifySlot(const MediaControlMsg &info);
  void HandleFloorControlInfoNotifySlot(const FloorControlInfoPointer &info);

  // �������������������
  void SendOnlineHeartBeatSlot();
  // �����������������߲�������
  void SendServerOnlineTestSlot();
  // ������������������Ƿ�ʱ
  void ServerHeartBeatTimeoutSlot();
  // ����ն��б��Ƿ��룬��δ���������ն��б�����
  void CheckTerminalListSlot();

  // ������Ȩ״̬
  void HandleFloorRequestStatusNotifySlot(
    uint16_t user_id, bfcpc::RequestStatus status);
  
 private:
  void ConnectWithMsgManager(IMsgManager *msg_manager);
  void ConnectWithMsgReceiver(IMsgReceiver *receiver);
  void ConnectWithMsgSender(IMsgSender *sender);

  // ���������飬���͵�½��Ϣ��QoS������
  void ApplyForJoiningConference(const QString &uri);
  
  // ��������������Ĳ���param
  void ConstructJoinConferenceParam(
    ConstConferencePointer conference, 
    ConferenceJoinParam &param) const;
  
  // ����һ·��Ч��SDP��Ϣ
  void ConstructValidSDPInfo(SDPInfo &info) const;

  // �����������
  int CalculateRequireBandwith() const;
  
  // ִ�м������Ķ���
  void JoinConference(const QString &focus_uri);
  
  // ������QoS�ܾ���½������
  void HandleConferenceJoinRejection(
    const QString &focus_uri, 
    const QString &reason);

  ConferencePointer FindConferenceByURI(const QString &uri);

  // ֹͣ���м�ʱ��
  void StopAllTimer();

  // ��ջ��������Ϣ
  void ClearConferenceInfo();

  // ���õ�ǰ�ն�״̬���������ź�
  void SetState(const QString &conference_uri, ConferenceState state);
  
  // �����ն������仯������
  void HandleTotalTerminalCountChanged(int new_total_terminal_count);
  
  bool HasReceivedAllTerminals() const;

  // TODO: �Ƶ������ط�
  int SendMessageToNetworkMeasurer(const QString &msg) const;
  void SendUDPMessageToQoSServer(const QString &msg);

  // �����ն˳�ʼ������ý����
  void InitSendStreamMedia();

  // �˳���������÷��ͽ���
  void ResetSendStreamMedia();

  // �������ػ��Ժͱ���¼�ƽ���.
  // ע��:������������mediaID��Ϊ��ײ㽻���ı�ʶ��,����virtualURI
  void AddRecvLocalMedia(
    const QString &local_media_id, 
    int screen_index, 
    int seat, 
    bool is_small_video = false);

  // ֹͣ�������Ľ���. 
  // ע��:������������mediaID��Ϊ��ײ㽻���ı�ʶ��,����virtualURI
  void RemoveRecvLocalMedia( const QString &local_media_id);

  // ����Զ���ն˷���ý����
  void RequestStartMediaRelay(
    TerminalPointer &remote_terminal, 
    bool use_small_video);

  // ����������Ϣ����һ·����Ƶ������Ļ����Զ���ն�
  void StartSendStreamMediaToTerminal(const MediaRelayMsg &info);
  // ����������Ϣֹͣһ·����Ƶ������Ļ����Զ���ն�
  void StopSendStreamMediaToTerminal(const MediaRelayMsg &info);

  // ��ӷ���һ·����Ƶ��Զ���ն�
  void AddRemoteVideoAndAudioStreamReceiver(const StreamReceiverInfo &info);
  
  // ֹͣ����һ·����Ƶ��Զ���ն�
  void RemoveRemoteVideoAndAudioStreamReceiver(
    const StreamReceiverInfo &info,
    bool search_by_recv_ip = true);

  // ֹͣ��Զ���ն�֮���ý�������䣬��������Ƶ���͹�����Ļ��
  void StopStreamBetweenLocalAndRemoteTerminal(const QString &remote_uri);
  
  // ֹͣ����ý������Զ���նˣ���������Ƶ���͹�����Ļ��
  void StopSendStreamMediaToTerminal(const QString &recver_uri);
  
  // ֹͣ��Զ���ն˽���ý��������������Ƶ���͹�����Ļ��
  void StopRecvStreamMediaFromTerminal(const QString &sender_uri);

  // ���챾���ն˵�Ĭ�������ն�uri�����Ϊ0
  QString ConstructDefaultLocalVirtualURI() const;

  // ��uri��Ӧ���ն��Ƿ�Ϊ�����ն�
  bool IsLocalTerminal(const QString &uri) const;
  
  // ��report����ģʽ�£���ֹ������ζ�Ž�ֹ������Ƶ�͹�����Ļ
  // ��disscus����ģʽ�£���ֹ������ζ�Ž�ֹ���͹�����Ļ
  void ForbidTerminalSpeak(const QString &terminal_uri);

  // ����Զ���ն˷��͹�����Ļ�����ն�
  void StartRecvSharedScreen(const QString &remote_uri);

  // ֪ͨң����������ϯ
  void SendChairmanChangedNotify(bool enable);

  // ���ݶ��ն���Ϣִ�еĶ����������ն���Ϣ�ź�
  void SendTeriminalNotifySignal(
    TerminalManager::ActionType action,
    TerminalPointer terminal);

 private:
   RunningProfile *profile_;
   RunningConfig *running_config_;
   MediaManager *media_manager_;
   IControllerManager *controller_manager_;

   // �Ƿ��ʼ����
   bool has_initialized_;

   // ��ǰ�ն˵�ǰ״̬
   ConferenceState state_;
   // �Ƿ��ڵȴ�QoS������������
   bool is_waitting_permission_;

   // ��ǰ����URI
   // �������ڽ��룬���ڿ����С����ܾ����롢�����˳���״̬
   QString focus_uri_;

   // ָ������ʼ��Ϣ
   // ֻ�����ڿ����У���ָ�����Ч
   ConferencePointer current_conference_;
   QString speaker_uri_; // �ڻ�������п��ܻ�ı�
   QString chairman_uri_;  // �ڻ�������п��ܻ�ı�
   QString screen_shared_uri_; // �ڻ�������п��ܻ�ı�
   FloorControlInfoPointer fcs_info_; //��¼��ǰ����ķ���Ȩ������Ϣ

   // ��Ż����б�������ǳ�ʼ�Ļ�����Ϣ
   ConferenceDictionary conferences_;
   
   // ����ն��б�
   TerminalManager terminals_;
   
   // ��ǰ������ն������������������ն�
   int total_terminal_count_;

   // ���ڶ�ʱ��������������������Ϣ�ļ�ʱ��
   QTimer *send_heart_beat_timer_;
   
   // ���ڷ�����������ʱ�ļ�ʱ��
   QTimer *server_heart_beat_timer_;
   
   // ��¼���ձ����ն�ý������Զ���ն���Ϣ
   StreamReceiverInfoManager receivers_;

   // ���ڶ�ʱ����ն��б��Ƿ���ļ�ʱ��
   QTimer *check_terminal_list_timer_;

   //  QoS���淢����
   UdpSender qos_report_sender_;

   // �����»�����ն��б��Ƿ��һ����������
   bool need_to_query_extra_info_;

   // TODO: �����⣿
   // �����ն���������Ϊipʱ���ڽ���������Ҫ��������
   // ͨ����ȡ�����ն˵��������������Լ�������
   bool need_to_set_local_uri_;

   bool join_first_conference_;
};

inline
bool ConferenceController::IsInConference() const {
  return state_ == kIsInConference && current_conference_;
}

inline 
void ConferenceController::StopStreamBetweenLocalAndRemoteTerminal(
    const QString &remote_uri) {
  // ֹͣ����ý�����������ն�
  StopSendStreamMediaToTerminal(remote_uri);
  // �رս��յ�����Ƶ��
  StopRecvStreamMediaFromTerminal(remote_uri);
}

#endif // CONFERENCE_CONTROLLER_H
