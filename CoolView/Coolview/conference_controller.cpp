#include "conference_controller.h"

#include <ctime>

#include <winsock2.h>

#include <QtGlobal>
#include <QtDBus/QDBusConnection>
#include <QMessageBox>

#include <dbus/sipwrapper/common/SipWrapperCommonService.h>
#include <dbus/channel/type/streamedMedia/common/StreamedMediaServiceCommon.h>

#include <dbus/telecontroller/client/TelecontrollerIf.h>
#include <dbus/telecontroller/common/TelecontrollerServiceCommon.h>

#include "CoolviewCommon.h"

#include "profile/RunningProfile.h"
#include "config/RunningConfig.h"
#include "util/ini/CVIniConfig.h"

#include "util/ini/TxConfig.h"
#include "util/report/RecordStat.h"
#include "msdx/config.h"

#include "util/MediaManager.h"

#include "util/STcpSender.h"
#include "util/NetworkUtil.h"

#include "sip_msg_parser.h"
#include "sip_msg_builder.h"
#include "msg_manager_interface.h"
#include "controller_manager_interface.h"
#include "tx_controller_interface.h"
#include "tx_client_controller_interface.h"

// TODO: remove
#include "util/report/RtpStat.h"
#include "http_msg_manager.h"

#define DEFAULT_IP_ADDRESS  "0.0.0.0"        //Ĭ�ϵ�һ����ַ��������ʾ���������IP��ַ������graph�ã�ʵ�ʲ���������

#define MEDIA_CONTROL_ENABLE    //��������˸ú꣬��ý�����Ŀ�����Ҫ�ͻ�����Ʒ���������Э��

#define PPT_TERMINAL_URI	"test_t1@sip.ccnl.scut.edu.cn"
#define MULTICAST_URI       "multicast@sip.ccnl.scut.edu.cn"
#define MULTICAST_SMALL_URI "multicasts@sip.ccnl.scut.edu.cn"

ConferenceController::ConferenceController(QObject *parent, 
                                           RunningProfile *profile,
                                           RunningConfig *config, 
                                           MediaManager *media_manager,
                                           IControllerManager *controller_manager)
    : IConferenceController(parent),
      profile_(profile),
      running_config_(config),
      media_manager_(media_manager),
      controller_manager_(controller_manager) {

  has_initialized_ = false;
  state_ = kIsNotInConference;
  is_waitting_permission_ = false;
  total_terminal_count_ = -1;
  check_terminal_list_timer_ = nullptr;
  send_heart_beat_timer_ = nullptr;
  server_heart_beat_timer_ = nullptr;
  need_to_query_extra_info_ = true;
  need_to_set_local_uri_ = true;

  join_first_conference_ = true;

  check_terminal_list_timer_ = nullptr;
}

ConferenceController::~ConferenceController() {
}

void ConferenceController::Initialize(
    IMsgManager *msg_manager, 
    HttpMsgManager *http_msg_manager,
    ISessionController *session_controller,
    IFloorController *floor_controller) {
  assert(profile_ != nullptr);
  assert(running_config_ != nullptr);
  assert(media_manager_ != nullptr);
  assert(controller_manager_ != nullptr);
  assert(has_initialized_ == false);

  send_heart_beat_timer_ = new QTimer(this);
  connect(send_heart_beat_timer_, SIGNAL(timeout()),
    this, SLOT(SendOnlineHeartBeatSlot()));

  server_heart_beat_timer_ = new QTimer(this);
  connect(server_heart_beat_timer_, SIGNAL(timeout()),
    this, SLOT(ServerHeartBeatTimeoutSlot()));

  check_terminal_list_timer_ = new QTimer(this);
  connect(check_terminal_list_timer_, SIGNAL(timeout()),
    this, SLOT(CheckTerminalListSlot()));

  connect(
    session_controller, &ISessionController::NotifySessionStateChangedSignal,
    this, &ConferenceController::HandleSessionStateChangedNotifySlot);

  state_ = kIsNotInConference;
  is_waitting_permission_ = false;
  StopAllTimer();
  ClearConferenceInfo();

  ConnectWithMsgManager(msg_manager);

  connect(
    this, &ConferenceController::SendTerminalListRequestByCIDSignal,
    http_msg_manager, &HttpMsgManager::RequestTerminalListSlot);
  connect(
    http_msg_manager, &HttpMsgManager::NotifyTerminalListReceivedSignal,
    this, &ConferenceController::HandleTerminalListReceivedNotifySlot);

  connect(
    this, &ConferenceController::CreateFloorControlClientSignal,
    floor_controller, &IFloorController::HandleCreateClientRequestSlot);
  connect(
    this, &ConferenceController::DestoryFloorControlClientSignal,
    floor_controller, &IFloorController::HandleDestroyClientRequestSlot);
  connect(
    this, &ConferenceController::ApplyFloorRequestSignal,
    floor_controller, &IFloorController::HandleApplyFloorRequstSlot);
  connect(
    this, &ConferenceController::AcceptFloorRequestSignal,
    floor_controller, &IFloorController::HandleAcceptFloorRequestSlot);
  connect(
    this, &ConferenceController::ReleaseFloorRequestSignal,
    floor_controller, &IFloorController::HandleReleaseFloorRequestSlot);
  connect(
    floor_controller, &IFloorController::NotifyFloorRequestStatusSignal,
    this, &ConferenceController::HandleFloorRequestStatusNotifySlot);

  has_initialized_ = true;
}

void ConferenceController::ConnectWithMsgManager( IMsgManager *msg_manager ) {
  assert(msg_manager);
  ConnectWithMsgReceiver(msg_manager->GetMsgReceiver());
  ConnectWithMsgSender(msg_manager->GetMsgSender());
}

void ConferenceController::ConnectWithMsgReceiver( IMsgReceiver *receiver ) {
  assert(receiver);
  connect(
    receiver, &IMsgReceiver::NotifyRecvConferenceListSignal,
    this, &ConferenceController::HandleConferenceListReceivedNotifySlot);
  
  connect(
    receiver, &IMsgReceiver::NotifyRecvQoSServerInfoSignal,
    this, &ConferenceController::HandleQoSServerInfoNotifySlot);
  connect(
    receiver, &IMsgReceiver::NotifyRecvQoSLoginPermissionSignal,
    this, &ConferenceController::HandleQoSLoginPermissionNotifySlot);
  connect(
    receiver, &IMsgReceiver::NotifyRecvQoSLoginRejectionSignal,
    this, &ConferenceController::HandleQoSLoginRejectionNotifySlot);

  connect(
    receiver, SIGNAL(NotifyRecvTerminalListSignal(const TerminalList&)),
    this, SLOT(HandleTerminalListReceivedNotifySlot(const TerminalList&)));
  connect(
    receiver, SIGNAL(NotifyRecvTerminalLoginSignal(const TerminalList&)),
    this, SLOT(HandleTerminalLoginNotifySlot(const TerminalList&)));
  connect(
    receiver, SIGNAL(NotifyRecvTerminalLogoutSignal(const QString&)),
    this, SLOT(HandleTerminalLogoutNotifySlot(const QString&)));
  connect(
    receiver, SIGNAL(NotifyRecvTerminalHandUpSignal(const QString&)),
    this, SLOT(HandleTerminalHandUpNotifySlot(const QString&)));
  connect(
    receiver, SIGNAL(NotifyRecvTerminalHandDownSignal(const QString&)),
    this, SLOT(HandleTerminalHandDownNotifySlot(const QString&)));
  connect(
    receiver, SIGNAL(NotifyRecvPermitSpeakSignal(const QString&)),
    this, SLOT(HandleTerminalPermitSpeakNotifySlot(const QString&)));
  connect(
    receiver, SIGNAL(NotifyRecvForbidSpeakSignal(const QString&)),
    this, SLOT(HandleTerminalForbidSpeakNotifySlot(const QString&)));
  connect(
    receiver, SIGNAL(NotifyRecvSpeakerTerminalSignal(const QString&)),
    this, SLOT(HandleSpeakerTerminalNotifySlot(const QString&)));
  connect(
    receiver, SIGNAL(NotifyRecvChairmanUpdateSignal(const QString&)),
    this, SLOT(HandleChairmanUpdateNotifySlot(const QString&)));
  connect(
    receiver, SIGNAL(NotifyRecvChairmanSignal(const QString&)),
    this, SLOT(HandleChairmanTerminalNotifySlot(const QString&)));
  connect(
    receiver, SIGNAL(NotifyRecvSharedScreenTerminalSignal(const QString&)),
    this, SLOT(HandleSharedScreenTerminalNotifySlot(const QString&)));
  connect(
    receiver, SIGNAL(NotifyRecvSharedScreenControlSignal(const QString&, bool)),
    this, SLOT(HandleSharedScreenControlNotifySlot(const QString&, bool)));
  connect(
    receiver, &IMsgReceiver::NotifyRecvStartMediaRelaySignal,
    this, &ConferenceController::HandleStartMediaRelayNotifySlot);
  connect(
    receiver, &IMsgReceiver::NotifyRecvStartMediaReplySignal,
    this, &ConferenceController::HandleStartMediaReplyNotifySlot);
  connect(
    receiver, SIGNAL(NotifyRecvStopMediaRelaySignal(const MediaRelayMsg&)),
    this, SLOT(HandleStopMediaRelayNotifySlot(const MediaRelayMsg&)));
  connect(
    receiver, SIGNAL(NotifyRecvStopMediaReplySignal(const MediaReplyMsg&)),
    this, SLOT(HandleStopMediaReplyNotifySlot(const MediaReplyMsg&)));
  connect(
    receiver, &IMsgReceiver::NotifyRecvOnlineMessageSignal,
    this, &ConferenceController::HandleOnlineMessageNotifySlot);
  connect(
    receiver, SIGNAL(NotifyRecvRemoteMediaStatusSignal(const MediaStatusMsg&)),
    this, SLOT(HandleRemoteMediaStatusNotifySlot(const MediaStatusMsg&)));
  connect(
    receiver, SIGNAL(NotifyRecvMediaControlInfoSignal(const MediaControlMsg&)),
    this, SLOT(HandleMediaControlNotifySlot(const MediaControlMsg&)));
  connect(
    receiver, SIGNAL(NotifyRecvFloorControlInfoSignal(const FloorControlInfoPointer&)),
    this, SLOT(HandleFloorControlInfoNotifySlot(const FloorControlInfoPointer&)));
}

void ConferenceController::ConnectWithMsgSender( IMsgSender *sender ) {
  assert(sender);
  connect(
    this, SIGNAL(SendConferenceListRequestMsgSignal()),
    sender, SLOT(RequestConferenceListSlot()));
  connect(
    this, &ConferenceController::SendJoinConferenceRequestMsgSignal,
    sender, &IMsgSender::SendConferenceJoinInfoSlot);
  connect(this, SIGNAL(SendServerOnlineTestSignal()),
          sender, SLOT(SendOnlineTestSlot()));
  connect(
    this, SIGNAL(SendHeartBeatToKeepInConferenceMsgSignal(const QString&)),
    sender, SLOT(SendOnlineHeartBeatToKeepInConferenceSlot(const QString&)));
  connect(
    this, SIGNAL(SendLeaveConferenceRequestMsgSignal(const QString&)),
    sender, SLOT(SendConferenceLeaveInfoSlot(const QString&)));
  connect(
    this, SIGNAL(SendTerminalListRequestMsgSignal(const QString&)),
    sender, SLOT(RequestTerminalListSlot(const QString&)));
  connect(
    this, SIGNAL(SendMainSpeakerInfoRequestSignal(const QString&)),
    sender, SLOT(QueryMainSpeakerSlot(const QString&)));
  connect(
    this, SIGNAL(SendChairmanInfoRequestSignal(const QString&)),
    sender, SLOT(QueryChairmanSlot(const QString&)));
  connect(
    this, SIGNAL(SendSharedScreenInfoRequestSignal(const QString&)),
    sender, SLOT(QueryShareScreenSlot(const QString&)));
  connect(
    this, SIGNAL(SendHandUpMsgSignal(const QString&, const QString&, bool)),
    sender, SLOT(SendHandUpMsgSlot(const QString&, const QString&, bool)));
  connect(
    this, SIGNAL(SendAllowSpeakMsgSignal(const QString&, const QString&, bool)),
    sender, SLOT(SendAllowSpeakMsgSlot(const QString&, const QString&, bool)));
  connect(
    this, SIGNAL(SendRequestStartMediaMsgSignal(const MediaRelayParam&)),
    sender, SLOT(RequestStartMediaSlot(const MediaRelayParam&)));
  connect(
    this, SIGNAL(SendReplyStartMediaMsgSignal(const MediaReplyParam&)),
    sender, SLOT(ReplyStartMediaRequestSlot(const MediaReplyParam&)));
  connect(
    this, SIGNAL(SendRequestStopMediaMsgSignal(const MediaRelayParam&)),
    sender, SLOT(RequestStopMediaSlot(const MediaRelayParam&)));
  connect(
    this, SIGNAL(SendReplyStopMediaMsgSignal(const MediaReplyParam&)),
    sender, SLOT(ReplyStopMediaRequestSlot(const MediaReplyParam&)));
  connect(
    this, SIGNAL(SendScreenControlMsgSignal(const ScreenControlParam&)),
    sender, SLOT(SendScreenShareControlSlot(const ScreenControlParam&)));
  connect(
    this, SIGNAL(SendMediaControlMsgSignal(const MediaControlParam&)),
    sender, SLOT(SendMediaControlInfoSlot(const MediaControlParam&)));
  connect(
    this, SIGNAL(SendFloorControlInfoRequest(const QString&)),
    sender, SLOT(RequestFloorControlInfoSlot(const QString&)));
}

void ConferenceController::HandleConferenceListReceivedNotifySlot(
    const ConferenceList &conferences) {
  ConstConferenceList const_conferences;
  const_conferences.reserve(conferences.size());
  for (auto conference : conferences) {
    conferences_.insert(conference->uri, conference);
    const_conferences.push_back(conference);
  }
  emit NotifyConferenceListReceivedSignal(const_conferences);
  if (!conferences.empty() && join_first_conference_) {
    join_first_conference_ = false;
    HandleJoinConferenceRequestSlot(conferences[0]->uri);
  }
}

void ConferenceController::HandleTerminalHandUpNotifySlot(
    const QString &terminal_uri) {
  if (!IsInConference()) {
    return;
  }
  auto terminal = terminals_.FindTerminal(terminal_uri);
  if (terminal && !terminal->is_handup) {
    terminal->is_handup = true;
    NotifyTerminalUpdateSignal(terminal, current_conference_->uri);
  }
}

void ConferenceController::HandleTerminalHandDownNotifySlot( 
    const QString &terminal_uri) {
  if (!IsInConference()) {
    return;
  }
  auto terminal = terminals_.FindTerminal(terminal_uri);
  if (terminal && terminal->is_handup) {
    terminal->is_handup = false;
    NotifyTerminalUpdateSignal(terminal, current_conference_->uri);
  }
}

// note:
// ��report����ģʽ��disscus����ģʽ��
// ��������ζ���ܹ�������Ƶ�ͷ��͹�����Ļ
void ConferenceController::HandleTerminalPermitSpeakNotifySlot( 
    const QString &terminal_uri ) {
  if (!IsInConference() || current_conference_->use_FCS) {
    return;
  }
  // ��speaker�ն�û�иı䣬�򷵻�
  if (speaker_uri_ == terminal_uri) {
    return;
  }
  
  if (!speaker_uri_.isEmpty()) {
    ForbidTerminalSpeak(speaker_uri_);
  }

  auto terminal = terminals_.FindTerminal(terminal_uri);
  if (terminal) {
    terminal->is_speaking = true;
    terminal->is_main_speaker = true;
    speaker_uri_ = terminal_uri;
    emit NotifyTerminalUpdateSignal(terminal, current_conference_->uri);

    if (IsLocalTerminal(terminal_uri)) {
      // ��ָ�������ն�Ϊ������
      media_manager_->ControlAudioSend(true);
      media_manager_->ControlScreenShare(true);
    } else {
      // ��ָ������Զ���ն�Ϊ������
      media_manager_->ChangeMediaState(
        terminal_uri, UiStateTypeAudio, UiMediaState_Run, true);
    }
  }
}

void ConferenceController::HandleTerminalForbidSpeakNotifySlot( 
    const QString &terminal_uri ) {
  ForbidTerminalSpeak(terminal_uri);
}

void ConferenceController::ForbidTerminalSpeak( 
    const QString &terminal_uri) {
  if (!IsInConference() || current_conference_->use_FCS) {
    return;
  }
  if (terminal_uri != speaker_uri_) {
    return;
  }

  auto terminal = terminals_.FindTerminal(terminal_uri);
  if (terminal) {
    terminal->is_main_speaker = false;
    terminal->is_speaking = false;
    emit NotifyTerminalUpdateSignal(terminal, current_conference_->uri);

    if (IsLocalTerminal(terminal_uri)) {
      media_manager_->ControlScreenShare(false);
    }
    if (current_conference_->control_mode == "report") {
      terminal->is_speaking = false;
      if (IsLocalTerminal(terminal_uri)) {
        media_manager_->ControlAudioSend(false);
      } else {
        media_manager_->ChangeMediaState(
          terminal_uri, UiStateTypeAudio, UiMediaState_Stop, true);
      }
    }
  }
  speaker_uri_.clear();
}

bool ConferenceController::IsLocalTerminal( const QString &uri ) const {
  QString local_uri = QString::fromStdString(profile_->user_uri());
  return local_uri == uri;
}

void ConferenceController::HandleFloorRequestStatusNotifySlot(
    uint16_t user_id, bfcpc::RequestStatus status) {
  if (IsInConference()) {
    assert(current_conference_->use_FCS);
    auto terminal = terminals_.FindTerminalById(user_id);
    if (terminal) {
      if (IsLocalTerminal(terminal->uri)) {
        if (status == bfcpc::RequestStatus::kPending ||
            status == bfcpc::RequestStatus::kAccepted ||
            status == bfcpc::RequestStatus::kGranted) {
          terminal->is_handup = true;
          emit NotifyTerminalHandUpSignal(true);
        } else {
          terminal->is_handup = false;
          emit NotifyTerminalHandUpSignal(false);
        }

      } else {
        if (status == bfcpc::RequestStatus::kPending ||
            status == bfcpc::RequestStatus::kAccepted) {
            terminal->is_handup = true;
        } else {
          terminal->is_handup = false;
        }
      }
      
      terminal->is_speaking = status == bfcpc::RequestStatus::kGranted;
      if (IsLocalTerminal(terminal->uri)) {
        if (terminal->is_speaking) {
          QMessageBox::information(nullptr, 
            QString::fromLocal8Bit("֪ͨ"), 
            QString::fromLocal8Bit("���Ѿ����Ȩ��"),
            QMessageBox::Ok);
        }
        media_manager_->ControlAudioSend(terminal->is_speaking);
      } else {
        media_manager_->ChangeMediaState(
          terminal->uri, 
          UiStateTypeAudio, 
          terminal->is_speaking ? UiMediaState_Run : UiMediaState_Stop,
          true);
      }
      emit NotifyTerminalUpdateSignal(terminal, current_conference_->uri);
    }
  }
}


void ConferenceController::HandleSpeakerTerminalNotifySlot( 
    const QString &terminal_uri ) {
  // TODO: any difference from permit speak?
  if (IsInConference())
    speaker_uri_ = terminal_uri;
}

void ConferenceController::HandleChairmanTerminalNotifySlot( 
    const QString &terminal_uri ) {
  if (IsInConference())
    chairman_uri_ = terminal_uri;
}

void ConferenceController::HandleChairmanUpdateNotifySlot(
    const QString &terminal_uri ) {
  if (!IsInConference()) {
    return;
  }

  if (chairman_uri_ == terminal_uri) {
    return;
  }

  auto old_chairman = terminals_.FindTerminal(chairman_uri_);
  if (old_chairman) {
    old_chairman->is_chairman_terminal = false;
    if (current_conference_->control_mode == "report") {
      // TODO: why not stop audio stream?
      old_chairman->is_speaking = false;
      emit NotifyTerminalUpdateSignal(old_chairman, current_conference_->uri);
    }
  }

  auto new_chairman = terminals_.FindTerminal(terminal_uri);
  if (!new_chairman) {
    chairman_uri_.clear();
  } else {
    new_chairman->is_chairman_terminal = true;
    new_chairman->is_speaking = true;
    chairman_uri_ = terminal_uri;
    emit NotifyTerminalUpdateSignal(new_chairman, current_conference_->uri);
  }
}

void ConferenceController::HandleSharedScreenControlNotifySlot( 
    const QString &terminal_uri, bool enable ) {
  if (!IsInConference()) {
    return;
  }

  if (enable) {
    StartRecvSharedScreen(terminal_uri);
  } else {
    media_manager_->RemoveRecvScreenMedia();
  }
}

void ConferenceController::HandleSharedScreenTerminalNotifySlot(
    const QString &terminal_uri ) {
  screen_shared_uri_ = terminal_uri;
  if (!terminal_uri.isEmpty() && !IsLocalTerminal(terminal_uri)) {
    StartRecvSharedScreen(terminal_uri);
  }
}

void ConferenceController::StartRecvSharedScreen( 
    const QString &remote_uri) {
  // ���ڷ���PPT���ն��޷�������Ļ��
  if (!IsInConference()) {
    return;
  }
  auto terminal = terminals_.FindTerminal(remote_uri);
  if (terminal) {
    // ������Ļ������TCP����˲�֧���鲥
    QString local_ip_addr = QString::fromStdString(profile_->ip_address());
    // �ȴ������ս��̣���Ҫ����Ļ���ͷ����ӵ���ǰ�Ľ��ս���
    media_manager_->CreateMainSpeakerRecvScreenMedia(
      remote_uri, current_conference_->ppt_port, local_ip_addr, 0);

    ::Sleep(2000);

    // ��������鲥���飬��ô��Ҫ����Sip��ϢҪ��Է��û�����ý����
    MediaRelayParam param;
    param.conference_uri = current_conference_->uri;
    param.remote_uri = terminal->uri;
    param.remote_vuri = terminal->virtual_uri;
    param.local_ip_addr = local_ip_addr;
    param.type = kScreenType;
    emit SendRequestStartMediaMsgSignal(param);
  }
}

void ConferenceController::HandleStartMediaRelayNotifySlot(
    const MediaRelayMsg &info) {
  if (!IsInConference()) {
    return;
  }

  QString local_virtual_uri = info.local_vuri;
  // ���ݷǶ�·֧�ֵľɰ�
  if (local_virtual_uri.isEmpty()) {
    local_virtual_uri = ConstructDefaultLocalVirtualURI();
  }
  // TODO: �޸�permissionΪ��������
  bool permission = false;
  if (info.type == kScreenType) {
    permission = true;
  }
  
  if (info.type == kStreamType || 
      info.type == kSmallStreamType || 
      info.type == kUnknowType) {
    auto terminal = terminals_.FindTerminal(local_virtual_uri);
    if (terminal) {
      QString device_path = 
        running_config_->VideoCaptureDevice(terminal->virtual_index).devicePath;
      if (!device_path.isEmpty()) {
        permission = true;
      }
      if (info.type == kSmallStreamType && 
          !running_config_->isEnableSmallVideo()) {
        // ���ն˲�֧��С��
        permission = false;
      }
    }
  }

  // ��Զ���ն˻ظ����ն˵����
  if (!info.remote_uri.isEmpty()) {
    MediaReplyParam param;
    param.conference_uri = current_conference_->uri;
    param.remote_uri = info.remote_uri;
    param.local_vuri = local_virtual_uri;
    param.type = info.type;
    param.permission = permission;
    emit SendReplyStartMediaMsgSignal(param);
  }

  // �����������ý������Զ���ն�
  if (permission) {
    StartSendStreamMediaToTerminal(info);
  }
}

QString ConferenceController::ConstructDefaultLocalVirtualURI() const {
  QString local_uri = QString::fromStdString(profile_->user_uri());
  return TerminalHelper::ConstructDefaultVirtualURI(local_uri);
}

void ConferenceController::StartSendStreamMediaToTerminal(
    const MediaRelayMsg &info) {
  if (info.type == kScreenType) {
    media_manager_->AddSendScreenMediaDst(
      current_conference_->ppt_port, info.remote_ip);
    // TODO: д��־

  } else if (info.type == kUnknowType || 
      info.type == kSmallStreamType ||
      info.type == kStreamType) {
    // �ж��Ƿ��鲥���飬��������Ŀ��˿��ǶԷ��ģ��鲥����Ŀ��˿����Լ���
    if (IsMulticastConference()) {
      return;
    }
    QString local_virtual_uri = info.local_vuri;
    if (local_virtual_uri.isEmpty()) {
      local_virtual_uri = ConstructDefaultLocalVirtualURI();
    }
    auto terminal = terminals_.FindTerminal(local_virtual_uri);
    if (terminal) {
      // ��������ʹ�öԷ�IP��ַ�����͵��Լ��Ķ˿�
      int audio_port = 
        running_config_->AudioCodec().isEmpty() ? 0 : terminal->audio_port;

      bool enable_small_stream = info.type == kSmallStreamType;
      int video_port = 0;
      VCapSetting setting = 
        running_config_->VideoCaptureDevice(terminal->virtual_index);
      if (!setting.videoCodec.isEmpty()) {
        video_port = enable_small_stream ? 
          terminal->small_video_port : terminal->video_port;
      }
      StreamReceiverInfo receiver_info;
      receiver_info.local_virtual_index = terminal->virtual_index;
      receiver_info.recv_uri = info.remote_uri;
      receiver_info.recv_ip = info.remote_ip;
      receiver_info.video_port = video_port;
      receiver_info.audio_port = audio_port;
      receiver_info.is_small_video = enable_small_stream;
      AddRemoteVideoAndAudioStreamReceiver(receiver_info);
    }
  }
}

void ConferenceController::HandleStartMediaReplyNotifySlot(
    const MediaReplyMsg &info) {
  if (!IsInConference()) {
    return;
  }

  if (info.type != kScreenType) {
    // ֪ͨ����ý����״̬�ı�
    emit NotifyStreamMediaReceiveReplySignal(info.remote_vuri, info.permission);
  }

  if (!info.permission) {
    // �ı����Զ���ն˵�����Ƶ״̬
    // TODO: ����permission�����������Ϊ����״̬
    media_manager_->ChangeMediaState(info.remote_vuri, UiStateTypeAny, UiMediaState_Ready, true); // ���صȴ�ͼ�꣬����������Ƶ
    media_manager_->ChangeMediaState(info.remote_vuri, UiStateTypeVideo, UiMediaState_Stop, true);
    //media_manager_->ChangeMediaState(info.remote_vuri, UiStateTypeAudio, UiMediaState_Stop, true);
      // ��Ƶ״̬��HandleAddRecvStreamMediaRequestSlot�д���������ʱ�����ն���Ϣ����
    return;
  }

  // ����ý����״̬
  if (info.type == kScreenType) {
    media_manager_->ChangeMediaState(info.remote_vuri, UiStateTypeScreen, UiMediaState_Ready, true);
  } else {
    media_manager_->ChangeMediaState(info.remote_vuri, UiStateTypeAny, UiMediaState_Ready, true); // ���صȴ�ͼ�꣬����������Ƶ
    media_manager_->ChangeMediaState(info.remote_vuri, UiStateTypeVideo, UiMediaState_Run, true);
    //media_manager_->ChangeMediaState(info.remote_vuri, UiStateTypeAudio, UiMediaState_Run, true);
  }
}

void ConferenceController::HandleStopMediaRelayNotifySlot(
    const MediaRelayMsg &info) {
  if (!IsInConference()) {
    return;
  }

  bool permission = true;
  QString local_virtual_uri = info.local_vuri;
  if (local_virtual_uri.isEmpty()) {
    local_virtual_uri = ConstructDefaultLocalVirtualURI();
  }
  if (!info.remote_uri.isEmpty()) {
    MediaReplyParam param;
    param.conference_uri = current_conference_->uri;
    param.remote_uri = info.remote_uri;
    param.local_vuri = local_virtual_uri;
    param.type = info.type;
    param.permission = permission;
    emit SendReplyStopMediaMsgSignal(param);
  }

  if (permission) {
    StopSendStreamMediaToTerminal(info);
  }
}

void ConferenceController::StopSendStreamMediaToTerminal(
    const MediaRelayMsg &info ) {
  QString local_virtual_uri = info.local_vuri;
  if (local_virtual_uri.isEmpty()) {
    local_virtual_uri = ConstructDefaultLocalVirtualURI();
  }
  if (info.type == kScreenType) {
    // ������Ļ������TCP����˲�֧���鲥
    media_manager_->RemoveSendScreenMediaDst(current_conference_->ppt_port, info.remote_ip);
    // TODO: д��־

  } else {
    // �鲥���鲻ֹͣ����
    if (IsMulticastConference()) {
      return;
    }

    auto terminal = terminals_.FindTerminal(local_virtual_uri);
    if (!terminal) {
      return;
    }
    StreamReceiverInfo *receiver = receivers_.FindRemoteReceiverByIP(
      info.remote_ip, terminal->virtual_index);
    if (receiver) {
      if (receiver->is_small_video != (info.type == kSmallStreamType)) {
        // TODO: д��־��ý����Ϣ��ƥ��
      }
      RemoveRemoteVideoAndAudioStreamReceiver(*receiver);
    }
  }
}

void ConferenceController::RemoveRemoteVideoAndAudioStreamReceiver(
    const StreamReceiverInfo &info,
    bool search_by_recv_ip /*= true*/) {
  // TODO: д��־
  media_manager_->RemoveSendMedia(
    info.local_virtual_index,
    info.recv_ip, 
    info.audio_port, 
    info.video_port,
    info.is_small_video);

  if (search_by_recv_ip) {
    receivers_.RemoveReceiverByIP(
      info.recv_ip, 
      info.local_virtual_index);
  } else {
    receivers_.RemoveReceiverByURI(
      info.recv_uri, 
      info.local_virtual_index);
  }

  // TODO: ֪ͨ���棬����ֹͣ����ý�������ź�
}

void ConferenceController::HandleStopMediaReplyNotifySlot(
    const MediaReplyMsg &info) {
  if (!IsInConference()) {
    return;
  }
  // TODO: д��־
}

void ConferenceController::SendOnlineHeartBeatSlot() {
  if (IsInConference()) {
    // TODO: ����ʱ�����ȥ���Ա�����������֮����ӳ�
    emit SendHeartBeatToKeepInConferenceMsgSignal(
      current_conference_->uri);
  }
}

void ConferenceController::SendServerOnlineTestSlot() {
  emit SendServerOnlineTestSignal();
}

void ConferenceController::HandleOnlineMessageNotifySlot(
    const QString &type, const QString &from) {
  qDebug() << "Received" << type << " message from " << from;
  // �յ�����global service�������ظ�
  if (IsInConference()) {
    // ������������������
    // TODO:
  }
  // �յ����Ե�����
  // TODO:
}

void ConferenceController::ServerHeartBeatTimeoutSlot() {
  // TODO: ������շ�����������Ϣ��ʱ
}

void ConferenceController::HandleConferenceListRequestSlot() {
  emit SendConferenceListRequestMsgSignal();
}

void ConferenceController::HandleJoinConferenceRequestSlot( const QString &uri ) {
  switch (state_) {
    case IConferenceController::kIsInConference:
      emit NotifyJoinConferenceErrorSignal(uri, kErrorOfIsAlreadyInOneConference);
      break;
    case IConferenceController::kIsLeavingConference:
      emit NotifyJoinConferenceErrorSignal(uri, kErrorOfIsLeavingConference);
      break;
    case IConferenceController::kIsJoiningConference:
      if (!is_waitting_permission_) {
        // �Ѿ������QoS����������Ӧ,���ڵ�½
        emit NotifyJoinConferenceErrorSignal(
          uri, kErrorOfIsJoiningOneConference);
        break;
      } else {
        // ȡ���ȴ�QoS����������Ӧ
        // ����ԭ����׼��ȴ��������������
        is_waitting_permission_ = false;
      }
      // fall through
    case IConferenceController::kIsNotInConference:
      // �������һ������
      ApplyForJoiningConference(uri); break;
    default:
      assert(false); break;
  }
}

void ConferenceController::ApplyForJoiningConference( const QString &uri ) {
  // ��ʾ�����Ҵ���
  startConferenceRoom();

  ConferencePointer conference = FindConferenceByURI(uri);
  if (!conference) {
    emit NotifyJoinConferenceErrorSignal(uri, kErrorOfConferenceNotExist);
    SetState(uri, kIsNotInConference);
    return;
  }

  if (conference->join_mode == "password") {
    emit NotifyJoinConferenceErrorSignal(uri, kErrorOfPasswordNeeded);
    SetState(uri, kIsNotInConference);
    return;
  }

  focus_uri_ = uri;
  SetState(uri, kIsJoiningConference);
  is_waitting_permission_ = true;

  // ���������������Ļ�����Ϣ
  ConferenceJoinParam param;
  ConstructJoinConferenceParam(conference, param);

  SDPInfo info;
  ConstructValidSDPInfo(info);
  emit SendJoinConferenceRequestMsgSignal(param, info);
}

void ConferenceController::ConstructJoinConferenceParam(
    ConstConferencePointer conference,
    ConferenceJoinParam &param) const {
  param.cid = conference->cid;
  param.focus_uri = conference->uri;
  param.uri = QString::fromStdString(profile_->user_uri());
  param.ip_addr = QString::fromStdString(profile_->ip_address());
  char gateway_str[60];
  memset(gateway_str, 0, 60);
  GetGatewayByHostIP(param.ip_addr.toLocal8Bit().constData(), gateway_str);
  param.gateway = QString::fromLocal8Bit(gateway_str);
  // TODO: min_rate������Ҫ����
  param.min_rate = 50; 
  param.max_rate = CalculateRequireBandwith();
  param.virtual_terminal_count = running_config_->VideoCaptureDeviceCount();
}

int ConferenceController::CalculateRequireBandwith() const {
  // ���ݵ�ǰ��Ƶ���ü�������Ҫ�Ĵ���
  // ���ڲο���������ɢֵ�����ȡ�ֱ��ʺ�֡�ʲ�С�ڵ�ǰ�ֱ��ʺ�֡�ʵ���С����
  static const int resolution_table[] = {
    1279 * 719, 1280 * 720, 1280 * 720, 1280 * 720, 1280 * 720, 1920 * 1080
  };
  static const int fps_table[] = {
    50, 10, 15, 30, 50, 25
  };
  static const int bandwidth_table[] = {
    1024, 1.5 * 1024, 2.5 * 1024, 4 * 1024, 6 * 1024
  };

  // TODO: ��ʱʹ����Щ����
  // �������ļ��ж�ȡ
  int resolution = 1920 * 1080;
  int fps = 30;

  int table_size = sizeof(bandwidth_table) / sizeof(bandwidth_table[0]);
  int required_bandwidth = 0;
  for(int i = 0; i < table_size; ++i) {
    if(resolution_table[i] >= resolution && fps_table[i] >= fps) {
      required_bandwidth = bandwidth_table[i];
      break;
    };
  }
  if(required_bandwidth == 0)
    required_bandwidth = bandwidth_table[table_size - 1];
  return required_bandwidth;
}

void ConferenceController::ConstructValidSDPInfo( SDPInfo &info ) const {
  // TODO:�Ѳ����ʺ϶�·�ɼ�����ʱ��ȡһ·��Ч����
  info._videoHeight = 0;
  info._videoWidth = 0;
  info._videoFps = 0;
  for (int i = 0; i < running_config_->VideoCaptureDeviceCount(); ++i) {
    VCapSetting setting = running_config_->VideoCaptureDevice(i);
    if (setting.devicePath == "") {
      continue;
    }
    info._videoCodec = setting.videoCodec;
    info._videoHeight = setting.height;
    info._videoWidth = setting.width;
    info._videoFps = setting.fps;
    break;
  }
  info._audioCodec = running_config_->AudioCodec();
  // ȷ����SipWrapper��wifo\phapi\mxcodec.h�����MIMEһ��
  if (info._audioCodec == MSDX_CONF_AUDIO_CODEC_AAC) {
    info._audioCodec = "AAC_LC";
  } else if(info._audioCodec == MSDX_CONF_AUDIO_CODEC_SPEEX) {
    info._audioCodec = "SPEEX";
  } // �����������Ĭ��ֵ
  info._audioRate = running_config_->AudioSampleRate();
  info._audioBits = running_config_->AudioBitsPerSample();
  info._audioChannel = running_config_->AudioChannel();
}

void ConferenceController::HandleQoSLoginPermissionNotifySlot(
    const LoginPermissionInfo &info) {
  // TODO: ���õ�½�����Ƿ���focus_uri_��ͬ
  if (state_ == kIsJoiningConference && is_waitting_permission_) {
    is_waitting_permission_ = false;
    
    int video_dscp = info.dscp;
    int audio_dscp = info.dscp;
#ifdef _DEBUG
    // ǿ��ʹ��ini�����е�DSCPֵ���÷�ʽ�����ڲ��ԣ���������ʱӦʹ�÷����������
    CVIniConfig &config = CVIniConfig::getInstance();
    if (config.isUseCustomedDscp()) {
      video_dscp = config.getVideoDscp();
      audio_dscp = config.getAudioDscp();
    }
#endif
    media_manager_->SetDSCP(video_dscp, audio_dscp);
    
    // QoS��֤ͨ�����������
    JoinConference(focus_uri_);

    // ֪ͨң����������ϯ
    SendChairmanChangedNotify(true);

    // ������������ն��б�
    HandleTerminalListRequestSlot();
  }
}

void ConferenceController::JoinConference( const QString &focus_uri ) {
  if (state_ != kIsJoiningConference) {
    return;
  }

  ConferencePointer conference = FindConferenceByURI(focus_uri);
  if (!conference) {
    emit NotifyJoinConferenceErrorSignal(focus_uri, kErrorOfConferenceNotExist);
    SetState(focus_uri, kIsNotInConference);
  } else {
    StopAllTimer();
    // �����¼�ľ�conference��Ϣ
    ClearConferenceInfo();

    // ָ���»���
    current_conference_ = conference;

    // ��¼��ǰ������Ϣ������QoS����
    CVIniConfig &config = CVIniConfig::getInstance();
    config.setCurrentConferenceUri(conference->uri.toLocal8Bit().data());
    config.setCurrentConferenceCid(conference->cid.toInt());

    // ��discussģʽ�£���ʾ������Ļ��ť
    media_manager_->ControlScreenShare(conference->control_mode == "discuss");

    SetState(focus_uri, kIsInConference);

    send_heart_beat_timer_->start(kSendHearBeatInfoInterval);
    server_heart_beat_timer_->start(kServerHeartBeatTimeout);
  }
}

ConferencePointer 
ConferenceController::FindConferenceByURI(const QString &uri ) {
  ConferenceDictionary::iterator it = conferences_.find(uri);
  if (it != conferences_.end()) {
    return it.value();
  }
  return nullptr;
}

void ConferenceController::StopAllTimer() {
  send_heart_beat_timer_->stop();
  server_heart_beat_timer_->stop();
  check_terminal_list_timer_->stop();
}

void ConferenceController::ClearConferenceInfo() {
  current_conference_ = nullptr;
  chairman_uri_.clear();
  speaker_uri_.clear();
  screen_shared_uri_.clear();
  total_terminal_count_ = -1;
  terminals_.Clear();
  need_to_query_extra_info_ = true;
  need_to_set_local_uri_ = true;
  fcs_info_ = nullptr;
}

void ConferenceController::SetState(const QString &conference_uri,
                                     ConferenceState state ) {
  if (state_ != state) {
    state_ = state;
    emit NotifyConferenceStateChangedSignal(conference_uri, state);
    // ���뿪����ʱ���ͻ����б�����
    if (state == kIsNotInConference) {
      emit SendConferenceListRequestMsgSignal();
    }
  }
}

void ConferenceController::HandleQoSLoginRejectionNotifySlot( 
    const LoginRejectionInfo &info) {
  // ���õ�½�����Ƿ��ǶԱ����ն�
  if (QString::fromStdString(profile_->user_uri()) != info.user_uri) {
    return;
  }
  // TODO: ���õ�½�����Ƿ���focus_uri��ͬ
  if (state_ == kIsJoiningConference && is_waitting_permission_) {
    is_waitting_permission_ = false;
    HandleConferenceJoinRejection(focus_uri_, info.description);
  }
}

void ConferenceController::HandleConferenceJoinRejection(
    const QString &uri, 
    const QString &reason) {
  if (focus_uri_ != uri || state_ != kIsJoiningConference) {
    return;
  }
  SetState(uri, kIsNotInConference);
  emit NotifyJoinConferenceErrorSignal(uri, kErrorOfJoinRejected);
}

void ConferenceController::HandleLeaveConferenceRequestSlot() {
  assert(has_initialized_);
  if (!IsInConference()) {
    return;
  }
  // ֹͣ���ͺͽ��չ�����Ļ
  media_manager_->RemoveRecvScreenMedia();
  media_manager_->RemoveScreenShareSend();

  // ֹͣ���ս���
  media_manager_->ExitConference();

  // ���˱��ػ��Խ��̣��ر������ķ��ͽ���
  ResetSendStreamMedia();

  SendChairmanChangedNotify(false);

  StopAllTimer();

  emit NotifyEraseAudioDisplayDictSignal();
  // �����˳��������Ϣ�����������
  emit SendLeaveConferenceRequestMsgSignal(current_conference_->uri);
 
  // �رշ���Ȩ�ͻ���
  emit DestoryFloorControlClientSignal();

  if (CVIniConfig::getInstance().IsModelTX() && 
      controller_manager_->GetTXController()->HasRecording()) {
    //�ȴ�¼���������TxControllerͨ���źŲ۵���ConferenceController::HandleAllMediaProcessExitSlot
    SetState(current_conference_->uri, kIsLeavingConference);
  } else {
    SetState(current_conference_->uri, kIsNotInConference);
    ClearConferenceInfo();
  }
  conferences_.clear();

  CVIniConfig::getInstance().setCurrentConferenceUri("");
  CVIniConfig::getInstance().setCurrentConferenceCid(-1);
}

void ConferenceController::ResetSendStreamMedia() {
  // ɾ�����˱��ػ�������ķ��Ͷ�
  StreamReceiverInfoManager::ReceiverList receivers;
  receivers.swap(receivers_.ToList());
  for (auto &receiver : receivers) {
    if (isLocalPreviewMediaID(receiver.recv_uri) || 
        isLocalRecordMediaID(receiver.recv_uri)){
      continue;
    }
    RemoveRemoteVideoAndAudioStreamReceiver(receiver, false);
  }
}

void ConferenceController::CheckTerminalListSlot() {
  if (HasReceivedAllTerminals()) {
    check_terminal_list_timer_->stop();
  } else{
    HandleTerminalListRequestSlot();
  }
}

void ConferenceController::HandleTerminalListRequestSlot() {
  assert(has_initialized_);
  if (running_config_->isEnableHttp()) {
    emit SendTerminalListRequestByCIDSignal(current_conference_->cid);
  } else {
    emit SendTerminalListRequestMsgSignal(current_conference_->uri);
    if (!check_terminal_list_timer_->isActive()) {
      check_terminal_list_timer_->start(kCheckTerminalListInterval);
    }
  }
}

void ConferenceController::HandleTerminalListReceivedNotifySlot(
    const TerminalList &terminals) {
  if (!IsInConference() || terminals.isEmpty()) {
    return;
  }

  // �ٶ������µõ����ն��б���total_terminal_num��һ��
  // ���ֻ��Ҫ����һ���ն˵�total terminal num�Ƿ���ԭ���Ĳ�ͬ
  HandleTotalTerminalCountChanged(terminals[0]->total_terminal_num);

  for (auto &terminal : terminals) {
    // �ȸ���uri
    if (need_to_set_local_uri_) {
      QString username = TerminalHelper::GetUsername(terminal->uri);
      if (username == QString::fromStdString(profile_->username())) {
        profile_->set_user_uri(terminal->uri.toStdString());
        need_to_set_local_uri_ = false;
      }
    }

    // ��ʱ�÷���Ȩ����ʱ����Ҫ�޸��ն˵ķ���״̬
    if (current_conference_->use_FCS) {
      terminal->is_handup = false;
      terminal->is_speaking = false;
      terminal->is_main_speaker = false;
    }
    // ���������ն�
    TerminalManager::ActionType action = 
      terminals_.AddTerminal(terminal, current_conference_->use_FCS);
    SendTeriminalNotifySignal(action, terminal);

    // ���Լ��������ն���ȫ��,��¼��ǰ�ն���Ϣ,����ʼ�����ͽ���
    if (IsLocalTerminal(terminal->uri) &&
        action == TerminalManager::kInsertAction) {
      int current_local_terminals_count = 
        terminals_.GetVirtualTerminalCount(terminal->uri);
      if (current_local_terminals_count == terminal->virtual_count) {
        InitSendStreamMedia();
      }
    }
  }
 
  // ����ն��б��Ƿ�����
  if (!HasReceivedAllTerminals()) {
    // ���ն�δ����ʱ�������ն��б�
    if (!check_terminal_list_timer_->isActive()) {
      check_terminal_list_timer_->start(kCheckTerminalListInterval);
    }
  } else {
    // ���ն�����ʱ��ֹͣ�����ն��б�
    check_terminal_list_timer_->stop();

    // ���ն�����ʱ��������Ȩ������Ϣ
    if (current_conference_->use_FCS) {
      emit SendFloorControlInfoRequest(current_conference_->cid);
    } else {
      if (need_to_query_extra_info_) {
        need_to_query_extra_info_ = false;
        // ���������ն��б�󣬲�ѯ�����ˡ���ϯ�͹�����Ļ�ն�
        emit SendMainSpeakerInfoRequestSignal(current_conference_->uri);
        emit SendChairmanInfoRequestSignal(current_conference_->uri);
        emit SendSharedScreenInfoRequestSignal(current_conference_->uri);
      }
    }
  }
}

void ConferenceController::HandleTotalTerminalCountChanged(
    int new_total_terminal_count) {
  if (new_total_terminal_count != total_terminal_count_) {
    emit NotifyTerminalCountChangedSignal(
      new_total_terminal_count, current_conference_->uri);
    total_terminal_count_ = new_total_terminal_count;
  }
}

void ConferenceController::InitSendStreamMedia() {
  if (!CVIniConfig::getInstance().IsModelHD()) {
    // ������������նˣ�����ʼ��
    return;
  }
  QString local_uri = 
    QString::fromStdString(RunningProfile::getInstance()->user_uri());
  const TerminalList &my_virtual_terminals = terminals_[local_uri];
  if (my_virtual_terminals.empty()) {
    return;
  }

  media_manager_->ControlAudioSend(my_virtual_terminals[0]->is_speaking);

  bool enable_small_video = running_config_->isEnableSmallVideo();

  for (auto terminal : my_virtual_terminals) {
    // ����Ƿ�����Ƶ�ɼ��豸
    VCapSetting vsetting = running_config_->VideoCaptureDevice(
      terminal->virtual_index);
    if (vsetting.devicePath.isEmpty()) {
      continue;
    }

    // ����ý�������鲥��ַ
    if(IsMulticastConference()) {
      StreamReceiverInfo info;
      info.local_virtual_index = terminal->virtual_index;

      // ���ʹ���
      info.recv_uri = MULTICAST_URI;
      info.recv_ip = terminal->multicast_address;
      info.video_port = terminal->video_port;
      info.audio_port = 
        running_config_->AudioCodec() == "" ? 0 : terminal->audio_port;
      info.is_small_video = false;
      AddRemoteVideoAndAudioStreamReceiver(info);

      // ����С��
      if (enable_small_video) {
        info.recv_uri = MULTICAST_SMALL_URI;
        info.video_port = terminal->small_video_port;
        info.audio_port = 0;
        info.is_small_video = true;
        AddRemoteVideoAndAudioStreamReceiver(info);
      }
    }
  }
}

void ConferenceController::AddRemoteVideoAndAudioStreamReceiver(
    const StreamReceiverInfo &info) {
  // ��¼���շ���Ϣ������ɾ��
  receivers_.AddRemoteReceiver(info);

  // ����ý����
  media_manager_->AddSendMedia(
    info.local_virtual_index,
    info.recv_ip, 
    info.audio_port, 
    info.video_port, 
    info.is_small_video, 
    info.screen_port,
    info.audio_port != 0);

  // д��־
}

void ConferenceController::AddRecvLocalMedia(const QString &local_media_id, 
                                             int screen_index, 
                                             int seat, 
                                             bool small_video /*= false*/) {
  QString base_media_id;

  StreamReceiverInfo receiver_info;
  receiver_info.recv_ip = "127.0.0.1";
  receiver_info.is_small_video = 
    small_video && running_config_->isEnableSmallVideo();

  int video_width = 0;
  int video_height = 0;
  int video_fps = 0;
  
  // ���������ն˺�������Ϣ
  if (isLocalPreviewMediaID(local_media_id)) {
    receiver_info.local_virtual_index = 
      getVirtualIndexFromLocalPreviewMediaID(local_media_id);
    if (receiver_info.is_small_video) {
      receiver_info.video_port = 
        LOCALPRE_SMALL_PORT_BASE + 10 * receiver_info.local_virtual_index;
    } else {
      receiver_info.video_port = 
        LOCALPRE_VIDEO_PORT_BASE + 10 * receiver_info.local_virtual_index;
    }
    // ���ػ��Բ���Ҫ��Ƶ
    base_media_id = LOCAL_PREVIEW_MEDIA_ID;

  } else if (isLocalRecordMediaID(local_media_id)) {
    receiver_info.local_virtual_index = 
      getVirtualIndexFromLocalRecordMediaID(local_media_id);
    receiver_info.is_small_video = false;
    receiver_info.video_port = 
      LOCALREC_VIDEO_PORT_BASE + 10 * receiver_info.local_virtual_index;
    receiver_info.audio_port = 
      LOCALREC_AUDIO_PORT_BASE + 10 * receiver_info.local_virtual_index;
    base_media_id = LOCAL_RECORD_MEDIA_ID;
  } else {
    return;
  }

  receiver_info.recv_uri = base_media_id;

  // ���ݸ�·��Ƶ������Ҫ�޸���Ϣ
  VCapSetting setting = 
    running_config_->VideoCaptureDevice(receiver_info.local_virtual_index);
  if (setting.devicePath.isEmpty()) {
    receiver_info.video_port = 0;
    receiver_info.audio_port = 0;
  }
  video_width = small_video ? MSDX_CONF_SMALL_VIDEO_WIDTH : setting.width;
  video_height = small_video ? MSDX_CONF_SMALL_VIDEO_HEIGHT : setting.height;
  video_fps = setting.fps;

  // Step2: ��ӷ��Ͷ�
  AddRemoteVideoAndAudioStreamReceiver(receiver_info);

  // Step3: ֪ͨý�����������������ս���
  media_manager_->AddRecvMemberPosition(
    local_media_id, 
    screen_index, 
    seat, 
    receiver_info.is_small_video);

  // TODO: Ӳ�����±��ػ��Բ�������Ƶ��·ʱ�˳��������������ʱ�������һ��û�����ݵĶ˿�
  if (isLocalPreviewMediaID(local_media_id) && receiver_info.audio_port == 0) {
    receiver_info.audio_port = 8500 + 10 * receiver_info.local_virtual_index;
  }

  RecvGraphInfo recv_graph_info;
  recv_graph_info.initial(
    qPrintable(running_config_->AudioCaptureDevice()),
    qPrintable(running_config_->AudioOutputDevice()),
    qPrintable(receiver_info.recv_ip),
    receiver_info.audio_port,
    qPrintable(running_config_->AudioCodec()), 
    running_config_->AudioSampleRate(),
    running_config_->AudioBitsPerSample(),
    running_config_->AudioChannel(),
    qPrintable(receiver_info.recv_ip),
    receiver_info.video_port,
    NULL,
    video_width,
    video_height,
    video_fps, 
    qPrintable(local_media_id),
    "", // �˴������ڲ�ת���õ���UTF8�����Ա��ڷ��������ݣ����ط�UTF8���ں��浥����ֵ
    false);

  recv_graph_info.user_name = QString::fromLocal8Bit("������Ƶ(%1)")
      .arg(receiver_info.local_virtual_index + 1);
  recv_graph_info.audio_info.enable = false;

  if (isLocalRecordMediaID(local_media_id)) {
    //// �����������¼�ƣ�������ڻ�����
    /*QString local_uri = QString::fromStdString(profile_->user_uri());
    auto &local_virtual_terminals = terminals_[local_uri];
    for (auto &terminal : local_virtual_terminals) {
      if (terminal->virtual_index == receiver_info.local_virtual_index) {*/
        QString filename = controller_manager_->GetTXClientController()->GenerateLocalRecordFilePath(
          receiver_info.local_virtual_index);
        recv_graph_info.record_info.file_name = filename;
        /*break;
      }
    }*/
    if (recv_graph_info.record_info.file_name.isEmpty()) {
      return;
    }
    // ¼����Ƶ�迪����Ƶ
    recv_graph_info.audio_info.enable = true;
    // TODO:��TxController�ó�ʼjob_id
    recv_graph_info.record_info.job_id = 0;
  }
  media_manager_->AddRecvMedia(recv_graph_info);

  // Step4:
  // ����һ��αACKȷ���ź�
  emit NotifyStreamMediaReceiveReplySignal(local_media_id, true);

  media_manager_->ChangeMediaState(local_media_id, UiStateTypeAny, UiMediaState_Ready, true); // ���صȴ�ͼ��
  // Video status
  if (setting.devicePath.isEmpty()) {
    media_manager_->ChangeMediaState(local_media_id, UiStateTypeVideo, UiMediaState_Stop, true);
  } else {
    media_manager_->ChangeMediaState(local_media_id, UiStateTypeVideo, UiMediaState_Run, true);
  }
  // Audio Status
  if (running_config_->AudioCaptureDevice().isEmpty()) {
    media_manager_->ChangeMediaState(local_media_id, UiStateTypeAudio, UiMediaState_Stop, true);
  } else {
    media_manager_->ChangeMediaState(local_media_id, UiStateTypeAudio, UiMediaState_Run, true);
  }
}

bool ConferenceController::HasReceivedAllTerminals() const {
  // ���uri����
  if (total_terminal_count_ != -1 && 
      terminals_.GetTerminalCount() < total_terminal_count_) {
    return false;
  }
  return terminals_.IsAllVirtualTerminalReceived();
}

void ConferenceController::HandleTerminalLoginNotifySlot( 
    const TerminalList &terminals) {
  for (auto terminal : terminals) {
    // ����Ƿ��ص�¼
    bool is_relogin = false;
    if (terminal->virtual_index == 0) {
      auto recorded_terminal = terminals_.FindTerminal(terminal->virtual_uri);
      if (recorded_terminal && 
          recorded_terminal->virtual_index == 0 &&
          recorded_terminal->is_available) {
        // ���ն��ص�¼����ʾ�ն˷������ǳ������ٵ�½
        // ��ʱ��Ҫ���ԭ����ý����
        StopStreamBetweenLocalAndRemoteTerminal(terminal->uri);
        is_relogin = true;
      }

      // ���ն�Ϊ��ϯ�նˣ�����Ҫ�ı��ն���Ϣ
      if (terminal->uri == chairman_uri_) {
        terminal->is_chairman_terminal = true;
        terminal->is_speaking = true;
      }
    }
    
    TerminalManager::ActionType action = terminals_.AddTerminal(
      terminal, current_conference_->use_FCS);
    if (is_relogin) {
      action = TerminalManager::kUpdateAction;
      emit NotifyTerminalReloginSignal(terminal, current_conference_->uri);
    }
    SendTeriminalNotifySignal(action, terminal);
  }
}

void ConferenceController::SendTeriminalNotifySignal(
    TerminalManager::ActionType action, TerminalPointer terminal) {
  switch (action) {
    case TerminalManager::kInsertAction:
      emit NotifyTerminalReceivedSignal(terminal, current_conference_->uri);
      break;
    case TerminalManager::kUpdateAction:
      emit NotifyTerminalUpdateSignal(terminal, current_conference_->uri);
      break;
  }
}

void ConferenceController::HandleTerminalLogoutNotifySlot(
    const QString &terminal_uri) {
  if (!IsInConference()) {
    return;
  }

  auto main_terminal = terminals_.FindTerminal(terminal_uri);
  if (main_terminal && main_terminal->is_available) {
    if (!current_conference_->use_FCS) {
      // �����ն�����ϯ����Ҫ�޸���Ӧ����Ϣ
      if (main_terminal->uri == chairman_uri_) {
        main_terminal->is_chairman_terminal = false;
        if (current_conference_->control_mode == "report") {
          main_terminal->is_speaking = false;
        }
        chairman_uri_.clear();
      }
    }

    // ֹͣ�������ն˵�ý��������
    StopStreamBetweenLocalAndRemoteTerminal(terminal_uri);

    auto &virtual_terminals = terminals_[terminal_uri];
    for (auto terminal : virtual_terminals) {
      // �������������ն�״̬
      terminal->is_available = false;
      emit NotifyTerminalUpdateSignal(terminal, current_conference_->uri);
    }
  }
}

void ConferenceController::StopSendStreamMediaToTerminal(
    const QString &recver_uri) {
  // ���������£���������˳�����Ƿ��з���Է���ý����������ֹͣ����
  if (!IsMulticastConference()) {
    const int virtual_count = running_config_->VideoCaptureDeviceCount();
    
    // ���ÿһ·������
    QString recv_ip;
    for (int i = 0; i < virtual_count; ++i) {
      StreamReceiverInfo *receiver_info = 
        receivers_.FindRemoteReceiverByURI(recver_uri, i);
      if (receiver_info) {
        if (!receiver_info->recv_ip.isEmpty()) {
          recv_ip = receiver_info->recv_ip;
        }
        RemoveRemoteVideoAndAudioStreamReceiver(*receiver_info);
      }
    }
    // ֹͣ���͹�����Ļ��
    media_manager_->RemoveSendScreenMediaDst(
      current_conference_->ppt_port, recv_ip);
  }
}

void ConferenceController::StopRecvStreamMediaFromTerminal(
    const QString &sender_uri) {
  // �رս��յ���Ļ��
  media_manager_->RemoveRecvScreenMedia(sender_uri);
  // TODO: ֪ͨ���棬����ֹͣ���չ�����Ļ�����ź�
  // emit NotifyStopRecvScreenStream(sender_uri);

  // �رս��յ�����Ƶ��
  if (terminals_.ContainTerminal(sender_uri)) {
    auto &virtual_terminals = terminals_[sender_uri];
    for (auto terminal : virtual_terminals) {
      media_manager_->RemoveRecvMedia(terminal->virtual_uri);
      // TODO: ֪ͨ���棬����ֹͣ����ý�������ź�
      // emit NotifyStopRecvMediaStream(terminal->virtual_uri);
    }
  }
}

void ConferenceController::HandleHandUpRequestSlot( bool hand_up ) {
  if (IsInConference()) {
    QString local_terminal_uri = 
      QString::fromStdString(profile_->user_uri());
    if (current_conference_->use_FCS) {
      auto terminal = terminals_.FindTerminal(local_terminal_uri);
      if (hand_up) {
        emit ApplyFloorRequestSignal();
      } else {
        emit ReleaseFloorRequestSignal(terminal->id);
      }
    } else {
      emit SendHandUpMsgSignal(
        current_conference_->uri, 
        local_terminal_uri,
        hand_up);
    }
    emit NotifyTerminalHandUpSignal(hand_up);
  }
}

bool ConferenceController::IsMulticastConference() const {
  assert(current_conference_);
  QString ip_addr = current_conference_->multicast_address;
  return (!ip_addr.isEmpty() && ip_addr != "0.0.0.0");
}

void ConferenceController::SendChairmanChangedNotify( bool enable ) {
  TelecontrollerIf teleControlProxy(
    _GetTelecontrollerServiceName("sock"),
    _GetTelecontrollerObjectPath("sock"),
    QDBusConnection::sessionBus());

  QString uri = enable ? current_conference_->chairman : "";
  QByteArray output_array;
  QDataStream out(&output_array , QIODevice::WriteOnly );
  out.setVersion(QDataStream::Qt_4_4);
  out << TELE_PostMeetingInfo << uri;
  teleControlProxy.TeleInfo(TELE_PostMeetingInfo, -1, output_array);
}

void ConferenceController::HandleRemoteMediaStatusNotifySlot(
    const MediaStatusMsg &info) {
  if (!info.remote_uri.isEmpty() && info.type != kUnknowControlType) {
    QString type;
    switch (info.type) {
      case kAudioControlType:
        type = UiStateTypeAudio; break;
      case kVideoControlType:
        type = UiStateTypeVideo; break;
      case kScreenControlType: 
        type = UiStateTypeScreen; break;
      default:
        assert(false); break;
    }

    media_manager_->ChangeMediaState(
      info.remote_uri,
      type,
      info.send ? UiMediaState_Run : UiMediaState_Stop,
      true);
  }
}

void ConferenceController::HandleMediaControlNotifySlot(
    const MediaControlMsg &info) {
  if (!IsInConference()) {
    return;
  }
  if (info.type == kAudioControlType) {
    auto terminal = terminals_.FindTerminal(info.terminal_uri);
    if (terminal && !terminal->is_speaking) {
      terminal->is_speaking = info.send;
      emit NotifyTerminalUpdateSignal(terminal, current_conference_->uri);
    }
    if (IsLocalTerminal(info.terminal_uri)) {
      media_manager_->ControlAudioSend(info.send);
    } else {
      media_manager_->ChangeMediaState(
        info.terminal_uri, UiStateTypeAudio, UiMediaState_Stop, true);
    }
  }
}

void ConferenceController::HandleQoSServerInfoNotifySlot( 
  const QoSServerInfo &info) {
    // ����qos server sip uri��
    if (!info.sip_uri.isEmpty()) {
      CVIniConfig &ini_config = CVIniConfig::getInstance();
      ini_config.setQosServerHostIP(info.ip.toLocal8Bit().data());
      ini_config.setQosServerSipUri(info.sip_uri.toLocal8Bit().data());
      ini_config.setQosServerTcpPort(info.tcp_port);
      ini_config.setQosServerUdpPort(info.operation_udp_port);
      ini_config.saveConfig();

      running_config_->setQosServerURI(info.sip_uri);
    }

    // TODO: remove to somewhere else
    // TODO: ����һ��NetworkMeasurerSender�࣬���Է��������Ϣ
    // ����һ��xml��Ϣ������������̣�֪ͨ���޸�QoS��������Ϣ
    SipMsgBuilder msg_builder("qos", "QoSServerInfo");
    msg_builder.AddArgument("ip", info.ip);
    msg_builder.AddArgument("simulatetestTCPPort", info.tcp_port);
    msg_builder.AddArgument("simulatetestUDPPort", info.simulate_test_udp_port);
    QString content = msg_builder.GetMsg();
    SendMessageToNetworkMeasurer(content);
}

int ConferenceController::SendMessageToNetworkMeasurer(
    const QString &msg) const {
  // TODO: remove to somewhere else
  if (msg.isEmpty()) {
    return -1;
  }
  STcpSender tcp_sender;
  // TODO: ��ȡ�����ļ�,��ȡ��ַ�Ͷ˿�
  bool connected = tcp_sender.connect("127.0.0.1", 5150);
  if (!connected) {
    return -1;
  } else {
    QByteArray byte_array = msg.toLocal8Bit();
    int size = byte_array.size();
    int len_in_big_endian = htonl(size);
    int data_len = size + sizeof(len_in_big_endian);
    unsigned char *data = new unsigned char[data_len];

    // ��������Ϣ��ŵ�data��
    memcpy(data, &len_in_big_endian, sizeof(len_in_big_endian));
    // ����Ϣ�ŵ�data��
    memcpy(data, byte_array.data(), size);
    int ret = tcp_sender.send(data, data_len);
    delete data;
    tcp_sender.close();
    return ret;
  }
}

void ConferenceController::HandleReceiveUDPQoSReportSlot(
    const RtpStatItem& rtp_stat) {
  if (!IsInConference()) {
    return;
  }
  if (rtp_stat.local_addr_len == 0 || rtp_stat.rem_addr_len == 0) {
    return;
  }
  QString reporter_uri = QString::fromStdString(profile_->user_uri());
  QString reporter_ip = QString::fromStdString(profile_->ip_address());
  QString sender_uri;
  QString sender_ip;
  int sender_port = 0;
  QString receiver_uri;
  QString receiver_ip;
  int receiver_port = 0;
  int bandwidth = 0;
  QString endpoint_type;
  if (rtp_stat.rtp_endpoint_type == eET_Sender) { // �����߱���
    endpoint_type = "sender";
    sender_uri = reporter_uri;
    sender_ip = reporter_ip;
    sender_port = rtp_stat.local_port;
    receiver_ip = rtp_stat.rem_addr;
    receiver_port = rtp_stat.rem_port;
    
    const int video_capture_count = running_config_->VideoCaptureDeviceCount();
    for (int i = 0; i < video_capture_count; ++i) {
      auto receiver = receivers_.FindRemoteReceiverByIP(rtp_stat.rem_addr, i);
      if (receiver) {
        receiver_uri = receiver->recv_uri;
        break;
      }
    }
    bandwidth = rtp_stat.send_bandwidth;

  } else { // �����߱���
    endpoint_type = "receiver";
    sender_ip = rtp_stat.rem_addr;
    sender_port = rtp_stat.rem_port;
    // ���ݶ˿��ҵ����Ͷ˵�uri
    TerminalPointer terminal;
    if (rtp_stat.media_type == eMT_Audio) {
      terminal = terminals_.FindTerminalByAudioPort(rtp_stat.local_port);
    } else if (rtp_stat.media_type == eMT_Video) {
      terminal = terminals_.FindTerminalByVideoPort(rtp_stat.local_port);
    }
    if (terminal) {
      sender_uri = terminal->uri;
    }
    receiver_uri = reporter_uri;
    receiver_ip = reporter_ip;
    receiver_port = rtp_stat.local_port;
    bandwidth = rtp_stat.recv_bandwidth;
  }
  QString flow_class;
  switch (rtp_stat.media_type) {
    case eMT_Audio:
      flow_class = "audio"; break;
    case eMT_Video:
      flow_class = "video"; break;
    default:
      flow_class = "app"; break;
  }
  SipMsgBuilder msg_builder("qos", "clientQosReport");
  msg_builder.AddArgument("cid", current_conference_->cid);
  msg_builder.AddArgument("messageSenderIP", sender_ip);
  msg_builder.AddArgument("messageSenderUri", sender_uri);
  msg_builder.AddArgument("seq", rtp_stat.seqnumber);
  msg_builder.AddArgument("endpointType", endpoint_type);
  msg_builder.AddArgument("senderIP", sender_ip);
  msg_builder.AddArgument("senderUri", sender_uri);
  msg_builder.AddArgument("senderPort", sender_port);
  msg_builder.AddArgument("receiverIP", receiver_ip);
  msg_builder.AddArgument("receiverUri", receiver_uri);
  msg_builder.AddArgument("receiverPort", receiver_port);
  msg_builder.AddArgument("flowClass", flow_class);
  msg_builder.AddArgument("bandwidth", bandwidth);
  msg_builder.AddArgument("lossRate", rtp_stat.lost);
  msg_builder.AddArgument("delay", rtp_stat.delay);
  msg_builder.AddArgument("jitter", rtp_stat.jitter);
  msg_builder.AddArgument("interval", rtp_stat.interval);
  msg_builder.AddArgument("timestamp", rtp_stat.timestamp);
  QString content = msg_builder.GetMsg();

  SendUDPMessageToQoSServer(content);
  emit NotifyRTPStatReceivedSignal(rtp_stat);
}

void ConferenceController::SendUDPMessageToQoSServer(const QString &msg) {
  if (!qos_report_sender_.isConnected()) {
    CVIniConfig &ini_config = CVIniConfig::getInstance();
    if (!qos_report_sender_.connect(ini_config.getQosServerHostIP(), 
                                    ini_config.getQosServerUdpPort())) {
      return;
    }
  }
  qos_report_sender_.send(qPrintable(msg), msg.size());
}

void ConferenceController::RemoveRecvLocalMedia(const QString &local_media_id) {
  QString base_media_id;
  int virtual_index = 0;
  if (isLocalPreviewMediaID(local_media_id)) {
    virtual_index = getVirtualIndexFromLocalPreviewMediaID(local_media_id);
    base_media_id = LOCAL_PREVIEW_MEDIA_ID;

  } else if (isLocalRecordMediaID(local_media_id)) {
    virtual_index = getVirtualIndexFromLocalRecordMediaID(local_media_id);
    base_media_id = LOCAL_RECORD_MEDIA_ID;

  } else {
    return;
  }

  StreamReceiverInfo *receiver = 
    receivers_.FindRemoteReceiverByURI(base_media_id, virtual_index);
  if (receiver) {
    assert(receiver->local_virtual_index == virtual_index);
    // �Ƴ����Ͷ�
    // ��Ϊ���ڱ��ػ��ԡ�����¼�Ƶ�IP��Ϊ127.0.0.1�Ľ��ս��̣�����Ҫ��URI����
    RemoveRemoteVideoAndAudioStreamReceiver(*receiver, false);
  }

  // ֹͣ���ս���
  media_manager_->RemoveRecvMedia(local_media_id);

  // ɾ��λ����Ϣ
  media_manager_->RemoveRecvMemberPosition(local_media_id);
}

void ConferenceController::HandleAddRecvStreamMediaRequestSlot(
    const QString &remote_vuri,
    int screen_index, 
    int seat, 
    bool use_small_video, 
    bool enable_audio) {

  // ���remote�Ǳ��ػ��Ե�mediaIDʱ������ձ���Ԥ��
  if (isLocalPreviewMediaID(remote_vuri) || isLocalRecordMediaID(remote_vuri)) {
    AddRecvLocalMedia(
      remote_vuri, 
      screen_index,
      seat, 
      use_small_video);
    return;
  }

  if (!IsInConference()) {
    return;
  }

  // Step1: �ռ���Ϣ
  auto remote_terminal = terminals_.FindTerminal(remote_vuri);
  if(!remote_terminal || !remote_terminal->is_available) {
    // TODO: �����źţ���֪���ն˲�����
    return;
  }

  // ����û�Ҫ����ܵ��Ǳ��ػ��ԣ���ת���ô������ؽ��ս��̵ķ���
  if (IsLocalTerminal(remote_terminal->uri)) {
    AddRecvLocalMedia(
      getLocalPreviewMediaID(remote_terminal->virtual_index), 
      screen_index,
      seat, 
      use_small_video);
    return;
  }

  bool is_record = false; // �Ƿ�Ϊ¼�ƽ���
  if (CVIniConfig::getInstance().IsModelTX()) {
    is_record = true;
    use_small_video = false; // TX¼��ֻʹ�ô���
  } else if (!running_config_->isEnableSmallVideo()) {
    use_small_video = false;
  }

  // �����ն˵�ý����
  int video_width = 0, video_height = 0;
  int video_fps;
  QString audio_codec;
  int audio_rate = 0, audio_bits = 0, audio_channel = 0;
  TerminalHelper::GetMediaInfoFromSDP(
    remote_terminal->sdp,
    video_width,
    video_height,
    video_fps,
    audio_codec,
    audio_rate,
    audio_bits,
    audio_channel);

  // ����Ƶ��ʽ��MIMEת����CoolView�����ַ���
  if (audio_codec == "AAC_LC") {
    audio_codec = MSDX_CONF_AUDIO_CODEC_AAC;
  } else if (audio_codec == "SPEEX") {
    audio_codec = MSDX_CONF_AUDIO_CODEC_SPEEX;
  }

  int remote_audio_port = 
    audio_codec.isEmpty() ? 0 : remote_terminal->audio_port;
  int remote_video_port = 0;

  // ��ʼ��ý������Ϣ
  if(use_small_video) {
    remote_video_port = (video_height == 0 || video_width == 0) ?
      0 : remote_terminal->small_video_port;

    // ȫ��������ͷ����Ƶ��������4����460x270�����Ǳ���������֧�ָ���Ƶ��ʽ�����ͳһΪ320x180
    video_width = MSDX_CONF_SMALL_VIDEO_WIDTH;
    video_height = MSDX_CONF_SMALL_VIDEO_HEIGHT;
    video_fps /= 2;

  } else {
    remote_video_port = (video_height == 0 || video_width == 0) ? 
      0 : remote_terminal->video_port;
  }

  QString local_address = "127.0.0.1";
  bool is_multicast = false;
  // Step2: �����Ͷ˷�������
  if(IsMulticastConference()) {  // ����Զ���鲥
    local_address = current_conference_->multicast_address;
    is_multicast = true;

  } else { // ����Զ�̵���
    local_address = QString::fromStdString(profile_->ip_address());

    // ��������鲥���飬��ô��Ҫ����Sip��ϢҪ��Է��û�����ý����
    RequestStartMediaRelay(remote_terminal, use_small_video);
  }

  // Step3: ֪ͨý�����������������ս���
  // ��������ӽ��ճ�Ա��λ����Ϣ
  media_manager_->AddRecvMemberPosition(
    remote_terminal->virtual_uri, 
    screen_index, 
    seat, 
    use_small_video);

  RecvGraphInfo info;
  info.initial( 
    qPrintable(running_config_->AudioCaptureDevice()),
    qPrintable(running_config_->AudioOutputDevice()),
    local_address.toStdString().c_str(),
    remote_audio_port,
    audio_codec.toStdString().c_str(),
    audio_rate, 
    audio_bits,
    audio_channel,
    local_address.toStdString().c_str(),
    remote_video_port,
    0,
    video_width,
    video_height,
    video_fps, 
    remote_terminal->virtual_uri.toStdString().c_str(),
    remote_terminal->name.toStdString().c_str(),
    false);
  info.audio_info.enable = enable_audio;

  if (is_record) {
    ITXController *tx_controller = controller_manager_->GetTXController();
    assert(tx_controller);
    info.record_info.file_name = tx_controller->GenerateRecordFilePath(
      remote_terminal);
    info.record_info.job_id = tx_controller->GetRecordInitialJobId(remote_vuri); 
  }

  media_manager_->AddRecvMedia(info);

  // Step4: ��������
  // ��Ƶͬ��
  if(!remote_terminal->is_speaking) {
    // ���Ļ���������Ƶ�����ϵ���Ƶͼ��
    media_manager_->ChangeMediaState(
      remote_terminal->virtual_uri, UiStateTypeAudio, UiMediaState_Stop, true);
  }
  if(is_multicast) {
    // ����һ��αACKȷ���ź�
    emit NotifyStreamMediaReceiveReplySignal(
      remote_terminal->virtual_uri, true);
    media_manager_->ChangeMediaState(
      remote_terminal->virtual_uri, UiStateTypeAny, UiMediaState_Ready, true); // Ready����������Ƶ
  }

  //֪ͨ�Ѵ������ս���
  emit NotifyStreamMediaReceiveStartedSignal(remote_terminal->virtual_uri);

  // TODO: д��־
}

void ConferenceController::HandleStartMediaReplyTimeoutSlot(
    const QString &remote_vuri) {
  auto remote_terminal = terminals_.FindTerminal(remote_vuri);
  if(!remote_terminal || !remote_terminal->is_available) {
    // TODO: �����źţ���֪���ն˲�����
    return;
  }

  bool use_small_video;
  if (media_manager_->GetRecvMemberPositionFromURI(
    remote_vuri, nullptr, nullptr, &use_small_video) == -1) {
    // TODO: �����źţ���֪���ն˲�����
      return;
  }

  RequestStartMediaRelay(remote_terminal, use_small_video);
}

void ConferenceController::RequestStartMediaRelay(
    TerminalPointer &remote_terminal, bool use_small_video) {
  QString local_address = QString::fromStdString(profile_->ip_address());
  MediaRelayParam param;
  param.conference_uri = current_conference_->uri;
  param.remote_uri = remote_terminal->uri;
  param.remote_vuri = remote_terminal->virtual_uri;
  param.local_ip_addr = local_address;
  param.type = use_small_video ? 
    MediaStreamType::kSmallStreamType : MediaStreamType::kStreamType;
  emit SendRequestStartMediaMsgSignal(param);
}

void ConferenceController::HandleRemoveRecvStreamMediaRequestSlot(
    const QString &remote_vuri) {
  // ���remote_vuri�Ǳ��ػ���mediaID����ֹͣ���ձ��ػ���
  if (isLocalPreviewMediaID(remote_vuri)) {
    RemoveRecvLocalMedia(remote_vuri);
    // TODO: ����֪ͨ���棿
    return;
  }

  if (!IsInConference()) {
    return;
  }
  auto remote_terminal = terminals_.FindTerminal(remote_vuri);
  if (!remote_terminal) {
    return;
  }

  if (IsLocalTerminal(remote_terminal->uri)) {
    RemoveRecvLocalMedia(getLocalPreviewMediaID(remote_terminal->virtual_index));
    // TODO: ����֪ͨ���棿
    return;
  }

  // �ж϶Է��Ƿ�ΪС��
  int index = 0, seat = 0;
  bool is_small_video = false;
  media_manager_->GetRecvMemberPositionFromURI(
    remote_terminal->virtual_uri, &index, &seat, &is_small_video);

  if(IsMulticastConference()) {
    // �鲥��ʲôҲ����
  } else {
    // ��������鲥����ʹ�������ն˺ŷ���ֹͣ�Է�����ý����������
    MediaRelayParam param;
    param.conference_uri = current_conference_->uri;
    param.remote_uri = remote_terminal->uri;
    param.remote_vuri = remote_terminal->virtual_uri;
    param.local_ip_addr = QString::fromStdString(profile_->ip_address());
    param.type = is_small_video ? 
      MediaStreamType::kSmallStreamType : MediaStreamType::kStreamType;
    emit SendRequestStopMediaMsgSignal(param);
  }

  // ֹͣ���ս���
  media_manager_->RemoveRecvMedia(remote_terminal->virtual_uri);

  // ɾ��λ����Ϣ
  media_manager_->RemoveRecvMemberPosition(remote_terminal->virtual_uri);
  
  // ֪ͨ���������Ӧ
  emit NotifyStreamMediaReceiveStoppedSignal(remote_terminal->virtual_uri);
}

void ConferenceController::HandleChangeVideoSeatRequestSlot(
    const QString &remote_vuri, int screen_index, int seat) {  
  QString vuri = remote_vuri;
  if (!isLocalPreviewMediaID(vuri)) {
    if (!IsInConference()) {
      return;
    }
    auto terminal = terminals_.FindTerminal(remote_vuri);
    if (terminal && IsLocalTerminal(terminal->uri)) {
      vuri = getLocalPreviewMediaID(terminal->virtual_index);
    }
  }

  media_manager_->ChangeMemberSeat(vuri, screen_index, seat);
}

void ConferenceController::HandleCreateSendStreamMediaRequestSlot() {
  if (!CVIniConfig::getInstance().IsModelHD()) {
    // ������������նˣ�����������ý����
    return;
  }
  bool enable_small_video = running_config_->isEnableSmallVideo();

  // ��������ͷ������������ý����
  const int stream_count = running_config_->VideoCaptureDeviceCount();
  const int receive_port = LOCALPRE_VIDEO_PORT_BASE;
  
  for (int i = 0; i < stream_count; ++i) {
    VCapSetting vcap = running_config_->VideoCaptureDevice(i);
    if (vcap.devicePath != "") {
      // ����ý��������
      // ֮����Ҫ�ڴ�����������ʱ��ָ�����ػ��Զ˿ڣ�����Ϊ�������һ������ͬ������
      // һ��ģ�����Ҫ�ڷ������ɹ����к󣬲�����ӷ��Ͷ˿ڡ�
      // �����ػ��Խ��ս����뷢�ͽ��̼���ͬʱ������
      // ���޷���֤AddRecvStreamMedia����ʱ�������Ѿ�����
      // ��˷�����Ӧ�ڴ�����ɺ�����������Ƶ�������ػ���
      // Liaokz����˵��
      media_manager_->CreateSendMedia(
        i,
        "127.0.0.1",
        0, //���ͱ��ػ�����Ƶ����������Ƶ����
        receive_port + 10 * i,
        enable_small_video,
        0);
    }
  }

  // ����¼���ӳٿ���������Ϊֻ�з��ͽ��̾���������ӷ��Ͷ˿�
  // ��TXClientController��HandleSendMediaStateChangedNotifySlot
  // Liaokz
}

void ConferenceController::HandleControlSendMediaRequestSlot(
    MediaControlType type, bool enable, int virtual_index) {
  if (!IsInConference()) {
    return;
  }

  MediaControlParam param;
  if (type == kAudioControlType) {
    media_manager_->ControlAudioSend(enable);
    param.local_uri = QString::fromStdString(profile_->user_uri());
  } else if (type == kVideoControlType) {
    media_manager_->ControlVideoSend(virtual_index, enable);
    param.local_uri = TerminalHelper::ConstructDefaultVirtualURI(
      QString::fromStdString(profile_->user_uri()),
      virtual_index);
  } else {
    return;
  }

  param.conference_uri = current_conference_->uri;
  param.enable = enable;
  param.type = type;

  emit SendMediaControlMsgSignal(param);
}

void ConferenceController::HandleCreateSendScreenMediaRequestSlot(
    const QRect &wnd) {
  if (CVIniConfig::getInstance().IsModelHD() && IsInConference()) {
    media_manager_->CreateSendScreenMedia(
      current_conference_->ppt_port,
      current_conference_->multicast_address,
      wnd);

    // TODO: д��־
  }
}

void ConferenceController::HandleCreateRecvScreenMediaRequestSlot( 
    const QString& remote_uri, int screen_index) {
  if (!IsInConference()) {
    return;
  }
  auto terminal = terminals_.FindTerminal(remote_uri);
  if (terminal) {
    if (!terminal->is_main_speaker) {
      // TODO: �����źţ���֪���ն˷�������
      return;
    }

    QString local_address;
    if (IsMulticastConference()) {
      local_address = current_conference_->multicast_address;
    } else {
      local_address = QString::fromStdString(profile_->ip_address());
      MediaRelayParam param;
      param.conference_uri = current_conference_->uri;
      param.remote_uri = terminal->uri;
      param.remote_vuri = terminal->virtual_uri;
      param.local_ip_addr = local_address;
      param.type = kScreenType;
      emit SendRequestStartMediaMsgSignal(param);
    }
    media_manager_->CreateRecvScreenMedia(
      remote_uri, 
      current_conference_->ppt_port,
      local_address,
      0,
      screen_index);

    emit NotifyScreenReceiveStartedSignal(remote_uri, screen_index);
  }
}

void ConferenceController::HandleRemoveRecvScreenMediaRequestSlot(
    const QString &remote_uri, int screen_index) {
  if (!IsInConference()) {
    return;
  }
  auto terminal = terminals_.FindTerminal(remote_uri);
  if (terminal) {
    // ������Ļ������TCP����˲�֧���鲥
    MediaRelayParam param;
    param.conference_uri = current_conference_->uri;
    param.remote_uri = terminal->uri;
    param.remote_vuri = terminal->virtual_uri;
    param.local_ip_addr = QString::fromStdString(profile_->ip_address());
    param.type = kScreenType;
    // ����ֹͣ�Է�����ý����������
    emit SendRequestStopMediaMsgSignal(param);

    // �ͷ���Ļ��
    media_manager_->RemoveRecvScreenMedia(remote_uri);

    emit NotifyScreenReceiveStoppedSignal(remote_uri, screen_index);

    // TODO: д��־
  }
}

void ConferenceController::HandleScreenShareControlRequestSlot( bool enable ) {
  if (!IsInConference()) {
    return;
  }
  QString local_uri = QString::fromStdString(profile_->user_uri());
  QString local_ip_address;
  auto local_terminal = terminals_.FindTerminal(local_uri);
  if (local_terminal) {
    // ������Ļ������TCPʵ�֣���˲���֧���鲥
    local_ip_address = QString::fromStdString(profile_->ip_address());
  }
  ScreenControlParam param;
  param.conference_uri = current_conference_->uri;
  param.terminal_uri = local_uri;
  param.ppt_port = current_conference_->ppt_port;
  param.ip_addr = local_ip_address;
  param.enable = enable;
  emit SendScreenControlMsgSignal(param);

  if (!enable) {
    media_manager_->RemoveScreenShareSend();
  }
}

void ConferenceController::HandleRecoveryMediaProcessRequestSlot(
    MediaStreamType type, bool is_send, const QString &remote_user_id) {
  if (type != kStreamType || is_send) {
    return;
  }
  
  int screen_index = -1;
  int seat_index = -1;
  bool use_small_video = false;;
  media_manager_->GetRecvMemberPositionFromName(
    remote_user_id, &screen_index, &seat_index, &use_small_video);
  if (seat_index < 0 && screen_index < 0) {
    return;
  }
  // TODO: ��ȡ�����ļ�
  QString uri = QString("%1@%2")
    .arg(remote_user_id)
    .arg(QString::fromStdString(profile_->realm()));

  auto terminal = terminals_.FindTerminal(uri);
  if (!terminal) {
    return;
  }

  int video_width = 0;
  int video_height = 0;
  int video_fps = 0;
  QString audio_codec;
  int audio_rate = 0, audio_bits = 0, audio_channel = 0;
  TerminalHelper::GetMediaInfoFromSDP(
    terminal->sdp, video_width, video_height, video_fps,
    audio_codec, audio_rate, audio_bits, audio_channel);
  int remote_audio_port = audio_codec.isEmpty() ? 0 : terminal->audio_port;
  int remote_video_port = 0;
  QString local_ip_addr;
  bool is_multicast = false;
  if (IsMulticastConference()) {
    local_ip_addr = current_conference_->multicast_address;
    is_multicast = true;
  } else {
    local_ip_addr = QString::fromStdString(profile_->ip_address());
  }
  if (use_small_video) {
    remote_video_port = (video_width == 0 || video_height == 0) ?
      0 : terminal->small_video_port;
    video_width = MSDX_CONF_SMALL_VIDEO_WIDTH;
    video_width = MSDX_CONF_SMALL_VIDEO_HEIGHT;
    video_fps /= 2;
  } else {
    remote_video_port = (video_width == 0 || video_height == 0) ? 
      0 : terminal->video_port;
  }
  RecvGraphInfo info;
  info.initial(
    running_config_->AudioCaptureDevice().toStdString().c_str(),
    running_config_->AudioOutputDevice().toStdString().c_str(),
    local_ip_addr.toStdString().c_str(),
    remote_audio_port,
    "SPEEX",
    16000, 
    16,
    1,
    local_ip_addr.toStdString().c_str(),
    remote_video_port,
    0,
    video_width,
    video_height,
    video_fps,
    remote_user_id.toStdString().c_str(),
    terminal->name.toStdString().c_str(),
    false); //RunningConfig::getInstance()->isEnableRecvAutoResync());

  media_manager_->RecoveryRecvMedia( info );
}

void ConferenceController::HanldePermitTerminalSpeakRequestSlot(
    const QString &uri, bool allow) {
  if (!IsInConference() || IsLocalTerminal(uri)) {
    return;
  }
  auto terminal = terminals_.FindTerminal(uri);
  if (terminal) {
    if (current_conference_->use_FCS) {
      if (allow) {
        emit AcceptFloorRequestSignal(terminal->id);
      } else {
        emit ReleaseFloorRequestSignal(terminal->id);
      }
    } else {
      emit SendAllowSpeakMsgSignal(
        current_conference_->uri,
        terminal->uri,
        allow);
    }
    emit NotifyTerminalSpeakSinal(uri, allow);
  }
}

void ConferenceController::HandleSessionStateChangedNotifySlot(
    ISessionController::SessionState state) {
  // ����¼�ɹ�ʱ�����ͻ����б�����
  if (state == ISessionController::kIsOnline) {
    if (state_ == kIsInConference) { 
      emit SendConferenceListRequestMsgSignal();
      state_ = kIsNotInConference;
      // ����Ѿ��ڻ������ˣ������ص�½������Ҫ���½������
      ApplyForJoiningConference(current_conference_->uri);
    } else {
      join_first_conference_ = true;
      emit SendConferenceListRequestMsgSignal();
    }
  }
}

void ConferenceController::HandleFloorControlInfoNotifySlot(
    const FloorControlInfoPointer &info) {
  if (IsInConference() && 
      current_conference_->cid == info->cid &&
      current_conference_->use_FCS) {
    fcs_info_ = info;
    auto terminal = terminals_.FindTerminal(
      QString::fromStdString(profile_->user_uri()));
    if (terminal) {
      for (auto &chair : info->chairs) {
        if (chair.user_id == terminal->id) {
          terminal->is_chairman_terminal = true;
          break;
        }
      }
      emit CreateFloorControlClientSignal(terminal->id, fcs_info_);
      emit NotifyTerminalUpdateSignal(terminal, current_conference_->uri);
    }
  }
}

void ConferenceController::HandleAllMediaProcessExitSlot()
{
  if (state_ == kIsLeavingConference) {
    SetState(current_conference_->uri, kIsNotInConference);
    ClearConferenceInfo();
  }
}
