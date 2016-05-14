#include "display_controller.h"

#include <cassert>

#include <QApplication>
#include <QDesktopWidget>

#include <dbus/conferenceRoom/client/ConferenceRoomIf.h>
#include <dbus/conferenceRoom/common/ConferenceRoomServiceCommon.h>

#include <dbus/telecontroller/client/TelecontrollerIf.h>
#include <dbus/telecontroller/common/TelecontrollerServiceCommon.h>

#include <dbus/channel/type/streamedMedia/common/StreamedMediaServiceCommon.h>

#include "util/ConferenceManager.h"
#include "util/MediaManager.h"
#include "config/RunningConfig.h"
#include "util/ini/CVIniConfig.h"

#include "conference_controller_interface.h"
#include "util/desktop/screen_helper.h"

namespace {

ConfRoomDisplayModel ConvertToDisplayModel(
    IDisplayController::ScreenLayout layout) {
  switch (layout) {
  case IDisplayController::kLayoutOf1x1:
    return CF_DM_1X1; break;
  case IDisplayController::kLayoutOf2x2:
    return CF_DM_2X2; break;
  case IDisplayController::kLayoutOf3x3:
    return CF_DM_3X3; break;
  case IDisplayController::kLayoutOf1plus5:
    return CF_DM_L3X3; break;
  case IDisplayController::kLayoutOfAuto:
    return CF_DM_AUTO; break;
  case IDisplayController::kLayoutOfDocument:
    return CF_DM_DOC; break;
  case IDisplayController::kLayoutOfTop1x2:
    return CF_DM_TOP_1_2; break;
  case IDisplayController::kLayoutOf4x4:
    return CF_DM_4X4; break;
  case IDisplayController::kLayoutOfAirplay:
    return CF_DM_AIRPLAY; break;
  default:
    return CF_DM_AUTO;
    break;
  }
}

} // namespace

DisplayController::DisplayController(QObject *parent, 
                                     MediaManager *media_manager) 
    : IDisplayController(parent), 
      media_manager_(media_manager) {
  conference_controller_ = nullptr;
  has_initialized_ = false;
  screen_count_ = QApplication::desktop()->screenCount();
  is_model_hd_ = CVIniConfig::getInstance().IsModelHD();
  for (int i = 0; i < screen_count_; ++i) {
    screen_layout_.push_back(kLayoutOfUnknown);
  }
  check_receive_video_ack_timer_ = nullptr;
}

DisplayController::~DisplayController() {
}

void DisplayController::Initialize(
    IConferenceController *conference_controller) {
  assert(!has_initialized_);
  assert(conference_controller);

  conference_controller_ = conference_controller;
  connect(
    conference_controller, 
    &IConferenceController::NotifyTerminalUpdateSignal,
    this,
    &DisplayController::HandleTerminalUpdateNotifySlot);

  connect(
    conference_controller,
    SIGNAL(NotifyConferenceStateChangedSignal(
      const QString&, IConferenceController::ConferenceState)),
    this,
    SLOT(HandleConferenceStateChangedNotifySlot(
      const QString&, IConferenceController::ConferenceState)));

  connect(
    conference_controller,
    &IConferenceController::NotifyScreenReceiveStartedSignal,
    this, 
    &DisplayController::HandleScreenReceiveStartedNotifySlot);

  connect(
    conference_controller,
    &IConferenceController::NotifyScreenReceiveStoppedSignal,
    this, 
    &DisplayController::HandleScreenReceiveStoppedNotifySlot);

  connect(
    conference_controller,
    &IConferenceController::RequestStreamMediaAutoReceiveSignal,
    this,
    &DisplayController::HandleRequestStreamMediaAutoReceiveSlot);

  connect(
    conference_controller,
    &IConferenceController::NotifyStreamMediaReceiveReplySignal,
    this,
    &DisplayController::HandleStreamMediaReceiveReplyNotifySlot);

  //  [12/10/2015 slt begin]
  connect(
      conference_controller,
      &IConferenceController::NotifyEraseAudioDisplayDictSignal,
      this,
      &DisplayController::HandleClearAudioDisplayDictNotifySlot);

  telecontroller_proxy_ = new TelecontrollerIf(
    _GetTelecontrollerServiceName("sock"),
    _GetTelecontrollerObjectPath("sock"),
    QDBusConnection::sessionBus());

  check_receive_video_ack_timer_ = new QTimer(this);
  connect(
    check_receive_video_ack_timer_, 
    &QTimer::timeout,
    this, 
    &DisplayController::HandleCheckReceiveVideoAckSlot);
  check_receive_video_ack_timer_->start(3000);

  has_initialized_ = true;
}

void DisplayController::HandleReceiveVideoRequestSlot(const QString &vuri,
                                                      int screen_index, 
                                                      int seat_index, 
                                                      bool use_small_video) {
  if (seat_index < 1 || screen_index < 0 || screen_index >= screen_count_) {
    return;
  }

  // ��coolview HD�ˣ�ֻ������ʾ����λ����ʾ��Ƶ��
  if (seat_index > GetVideoSeatCount(screen_index)) {
    return;
  }

  // �����ص�vuriת��Ϊlocal perview media id
  QString id = vuri;
  if (!isLocalPreviewMediaID(vuri) && IsLocalTerminalURI(vuri)) {
    id = getLocalPreviewMediaID(TerminalHelper::GetVirtualIndex(vuri));
  }

  // �رն�Ӧλ�õ��������ս���
  auto other_display = FindVideoSeat(screen_index, seat_index);
  if (other_display && other_display->terminal_vuri != id) {
    HandleStopVideoRequestSlot(other_display->terminal_vuri);
  }

  auto display = FindVideoSeat(id);
  if (!display) { // δ��ʾ
    // �����µĽ��ս���
    AddVideoSeat(id, screen_index, seat_index, use_small_video);

  } else { // ����ʾ��
    // ����ͬһ����Ļ�ڻ��С����ͬ
    if (display->screen_index != screen_index || 
        display->use_small_video != use_small_video) {
      // �رվɵĽ��ս���
      HandleStopVideoRequestSlot(id);
      
      // �����µĽ��ս���
      AddVideoSeat(id, screen_index, seat_index, use_small_video);

    } else {  // ��ͬһ����Ļ���Ҵ�С��һ��
      // ���λ�ò�ͬ�������λ��
      if (display->seat_index != seat_index) {
        // �ı�λ��
        conference_controller_->HandleChangeVideoSeatRequestSlot(
          id, screen_index, seat_index);

        // TODO: �ȴ��ײ��֪ͨ���ֽ׶ΰ�����һ�ɳɹ�
        // ֪ͨ���棬����ɵ�λ��
        emit NotifyStopVideoSignal(id, screen_index, display->seat_index);
        
        display->seat_index = seat_index;

        // ֪ͨ���棬��ʾ�µ�λ��
        emit NotifyReceiveVideoSignal(
          id, screen_index, seat_index, use_small_video);
      }
    }
  }
}

void DisplayController::AddVideoSeat(const QString &vuri,
                                     int screen_index, 
                                     int seat_index, 
                                     bool use_small_video) {
  // ��¼�µ���λ��Ϣ
  VideoSeatInfo new_video_info;
  new_video_info.terminal_vuri = vuri;
  new_video_info.screen_index = screen_index;
  new_video_info.seat_index = seat_index;
  new_video_info.use_small_video = use_small_video;
  new_video_info.is_ack = false; // ack״̬Ϊδ�յ�reply
  new_video_info.ack_time_counter.start(); // ������ÿ24h���㣬�˴���Ӱ��
  video_display_dict_.insert(vuri, new_video_info);

  bool enable_audio = !is_model_hd_;
  if (is_model_hd_ && !isLocalPreviewMediaID(vuri)) {
    AudioSeatInfo new_audio_info;
    new_audio_info.terminal_vuri = vuri;

    auto &seat_list = audio_display_dict_[TerminalHelper::GetTerminalURI(vuri)];
    auto it = find_if(seat_list.begin(), seat_list.end(), 
      [](const AudioSeatInfo &info) {
        return info.is_audio_enabled;});
    enable_audio = new_audio_info.is_audio_enabled = (it == seat_list.end());
    new_audio_info.is_audio_received = true;

    seat_list.push_back(new_audio_info);
  }

  // �����µĽ��ս��� - ����ڼ�¼λ����Ϣ��ȷ���յ�Replyʱ��ȷ���ack״̬
  conference_controller_->HandleAddRecvStreamMediaRequestSlot(
    vuri, screen_index, seat_index, use_small_video, enable_audio);

  // TODO: �ȴ��ײ��֪ͨ���ֽ׶ΰ�����һ�ɳɹ�
  emit NotifyReceiveVideoSignal(
    vuri, screen_index, seat_index, use_small_video);
}

void DisplayController::HandleStopVideoRequestSlot( const QString &vuri ) {
  // �����ص�vuriת��Ϊlocal perview media id
  QString id = vuri;
  if (!isLocalPreviewMediaID(vuri) && IsLocalTerminalURI(vuri)) {
    id = getLocalPreviewMediaID(TerminalHelper::GetVirtualIndex(vuri));
  }

  auto remove_seat = FindVideoSeat(id);
  if (remove_seat) {
    conference_controller_->HandleRemoveRecvStreamMediaRequestSlot(id);

    // ������ն�coolview hd������Ҫ��֤��һ·��Ƶ�ڲ���
    bool enable_audio = false;
    if (!isLocalPreviewMediaID(id)) {
      auto &seat_list = audio_display_dict_[TerminalHelper::GetTerminalURI(id)];
      auto it = find_if(seat_list.begin(), seat_list.end(), 
        [&remove_seat](const AudioSeatInfo &info) {
          return remove_seat->terminal_vuri == info.terminal_vuri;});

      if (it != seat_list.end()) {
        seat_list.erase(it);
        // ����Ƴ���λ�����ڲ�����Ƶ������Ҫѡ����һ�����õ�λ������������
        RearrangeAudioRender(id);
      }
    }

    // TODO: �ȴ��ײ��֪ͨ���ֽ׶ΰ�����һ�ɳɹ�
    emit NotifyStopVideoSignal(
      id, remove_seat->screen_index, remove_seat->seat_index);

    video_display_dict_.remove(id); 
  }
}

DisplayController::VideoSeatInfo* 
DisplayController::FindVideoSeat( const QString &vuri ) {
  auto it = video_display_dict_.find(vuri);
  if (it != video_display_dict_.end()) {
    return &(*it);
  }
  return nullptr;
}

DisplayController::VideoSeatInfo* 
DisplayController::FindVideoSeat( int screen_index, int seat_index ) {
  for (auto it = video_display_dict_.begin();
       it != video_display_dict_.end();
       ++it) {
    auto &display = it.value();
    if (it->screen_index == screen_index && it->seat_index == seat_index) {
      return &display;
    }
  }
  return nullptr;
}

void DisplayController::HandleReceiveAudioRequestSlot(
    const QString &uri, bool enable ) {
  media_manager_->ControlAudioRecv(uri, enable);
  emit NotifyReceiveAudioSignal(uri, enable);
}

void DisplayController::HandleReceiveDocumentRequestSlot(
    const QString &uri, int screen_index) {
  if (isLocalPreviewMediaID(uri)) {
    return;
  }
  if (is_model_hd_ && conference_controller_->IsInConference()) {
    conference_controller_->HandleCreateRecvScreenMediaRequestSlot(
      uri, screen_index);
  }
}

void DisplayController::HandleScreenReceiveStartedNotifySlot( 
    const QString &remote_uri, int screen_index) {
  if (screen_index < 0 || screen_index >= screen_count_) {
    return;
  }

  if (conference_controller_->IsInConference()) {
    auto shared_screen = FindSharedScreenSeat(remote_uri);
    if (shared_screen) {
      if (shared_screen->screen_index == screen_index) {
        return;
      } else {
        // TODO: ȷ��
        // ���ڽ��չ�����Ļʱ���ܹ��Զ�ֹͣ�ɵ�
        // �������ֱ�Ӹ�����Ļ������Ȼ��ʼ�µĽ��ս���
        emit NotifyStopDocumentSignal(remote_uri, shared_screen->screen_index);
        shared_screen->screen_index = screen_index;
      }
    } else {
      SharedScreenSeatInfo info;
      info.terminal_uri = remote_uri;
      info.screen_index = screen_index;
      shared_screen_list_.push_back(info);
    }
    emit NotifyReceiveDocumentSignal(remote_uri, screen_index);
  }
}

DisplayController::SharedScreenSeatInfo* 
DisplayController::FindSharedScreenSeat( const QString &uri ) {
  for (auto &info : shared_screen_list_) {
    if (info.terminal_uri == uri) {
      return &info;
    }
  }
  return nullptr;
}

void DisplayController::HandleStopDocumentRequestSlot(const QString &uri) { 
  if (is_model_hd_ && conference_controller_->IsInConference()) {
    if (uri.isEmpty()) {
      // ֹͣ�������й�����Ļ
      while (!shared_screen_list_.isEmpty()) {
        conference_controller_->HandleRemoveRecvScreenMediaRequestSlot(
          shared_screen_list_[0].terminal_uri,
          shared_screen_list_[0].screen_index);
      }
    } else {
      // ֹͣ����ָ��uri�Ĺ�����Ļ
      auto shared_screen = FindSharedScreenSeat(uri);
      if (shared_screen) {
        conference_controller_->HandleRemoveRecvScreenMediaRequestSlot(
          uri, shared_screen->screen_index);
      }
    }
  }
}

void DisplayController::HandleScreenReceiveStoppedNotifySlot(
    const QString &remote_uri, int screen_index) {
  if (screen_index < 0 || screen_index >= screen_count_) {
    return;
  }

  if (conference_controller_->IsInConference()) {
    RemoveSharedScreenSeat(remote_uri);
    emit NotifyReceiveDocumentSignal(remote_uri, screen_index);
  }
}

void DisplayController::HandleOpenDocumentRequestSlot(
    const QString &filename, int screen_index) {
  if (is_model_hd_ && conference_controller_->IsInConference()) {
    ConferenceRoomIf* confRoomProxy = new ConferenceRoomIf( 
      CONF_ROOM_SERVICE_NAME,
      CONF_ROOM_SERVICE_OBJECT_PATH,
      QDBusConnection::sessionBus());
    confRoomProxy->OpenScreenShareFile(filename, screen_index);
  }
}

void DisplayController::HandleTerminalUpdateNotifySlot(
    ConstTerminalPointer terminal, const QString &conference_uri) {
  if (conference_controller_->IsInConference()) {
    bool enable_audio = true;
    auto &seat_list = audio_display_dict_[terminal->uri];
    auto it = find_if(seat_list.begin(), seat_list.end(), 
      [&terminal](const AudioSeatInfo &info) {
        return info.terminal_vuri == terminal->virtual_uri;});
    if (it != seat_list.end()) {
      enable_audio = (*it).is_audio_enabled;
    }

    if (terminal->is_available == false) { // �ն�������
      // �����¼����Ƶλ��
      auto video_seat = FindVideoSeat(terminal->virtual_uri);
      if (video_seat) {
        emit NotifyStopVideoSignal(
          video_seat->terminal_vuri, 
          video_seat->screen_index, 
          video_seat->seat_index);
      }
      video_display_dict_.remove(terminal->virtual_uri);
      if (it != seat_list.end()) {
        seat_list.erase(it);
      }
        
      // �����¼�Ĺ�����Ļλ��
      auto shared_sceen_seat = FindSharedScreenSeat(terminal->virtual_uri);
      if (shared_sceen_seat) {
        emit NotifyStopDocumentSignal(
          shared_sceen_seat->terminal_uri, shared_sceen_seat->screen_index);
      }
      RemoveSharedScreenSeat(terminal->uri);

    } else { // �ն˵�½��
      auto video_seat = FindVideoSeat(terminal->virtual_uri);
      if (video_seat) {
        // �ٴν�������Ƶ

        conference_controller_->HandleAddRecvStreamMediaRequestSlot(
          video_seat->terminal_vuri,
          video_seat->screen_index, 
          video_seat->seat_index, 
          video_seat->use_small_video,
          enable_audio); 
      }

      int virtual_index = 
        TerminalHelper::GetVirtualIndex(terminal->virtual_uri);
      if (virtual_index == 0) {
        QString remote_uri = 
          TerminalHelper::GetTerminalURI(terminal->virtual_uri);
        auto shared_screen = FindSharedScreenSeat(remote_uri);
        if (shared_screen) {
          // �ٴν��չ�����Ļ
          conference_controller_->HandleCreateRecvScreenMediaRequestSlot(
            shared_screen->terminal_uri, shared_screen->screen_index);
        }
      }
    }
  }
}

void DisplayController::RemoveSharedScreenSeat( const QString &uri ) {
  for (auto it = shared_screen_list_.begin(); 
       it != shared_screen_list_.end();
       ++it) {
    if (it->terminal_uri == uri) {
      shared_screen_list_.erase(it);
      return;
    }
  }
}

void DisplayController::HandlePPTControlSlot( int type ) {
  if (is_model_hd_ && conference_controller_->IsInConference()) {
    media_manager_->PPTControlCommand(type);
  }
}

void DisplayController::HandleShowRTCPMessageRequestSlot( bool show, int screen_index ) {
  if (screen_index < 0 || screen_index >= screen_count_) {
    return;
  }
  // note: Ŀǰrtcp����ʾ���޸��Ƕ�������Ļ��
  media_manager_->showRtcpMsg(show, screen_index);
  emit NotifyShowRTCPMessageSignal(show, screen_index);
}

void DisplayController::HandleSendLocalAudioRequestSlot( bool enable ) {
  if (is_model_hd_ && conference_controller_->IsInConference()) {
    conference_controller_->HandleControlSendMediaRequestSlot(
      kAudioControlType, enable);
    // TODO: �ȴ��ײ��֪ͨ
    // �ֽ׶ΰ�����һ�ɳɹ�
    emit NotifySendLocalAudioSignal(enable);
  }
}

void DisplayController::HandleSendLocalVideoRequestSlot(
    int virutal_index, bool enable ) {
  if (is_model_hd_ && conference_controller_->IsInConference()) {
    if (virutal_index != -1) {
      conference_controller_->HandleControlSendMediaRequestSlot(
        kVideoControlType, enable, virutal_index);
      // TODO: �ȴ��ײ��֪ͨ
      // �ֽ׶ΰ�����һ�ɳɹ�
      emit NotifySendLocalVideoSignal(virutal_index, enable);

    } else {
      const int video_count = RunningConfig::Instance()->VideoCaptureDeviceCount();
      for (int i = 0; i < video_count; ++i) {
        conference_controller_->HandleControlSendMediaRequestSlot(
          kVideoControlType, enable, i);
        // TODO: �ȴ��ײ��֪ͨ
        // �ֽ׶ΰ�����һ�ɳɹ�
        emit NotifySendLocalVideoSignal(i, enable);
      }
    }
  }
}

void DisplayController::HandleVoiceChangeRequestSlot( AudioDeviceType type, int volume ) {
  if (!is_model_hd_) return;

  RunningConfig *config = RunningConfig::Instance();
  switch (type) {
    case kMicrophone: {
      QString device_name = config->AudioCaptureDevice();
      if (!device_name.isEmpty()) {
        DeviceManager::GetInstatnce()->SetAudioInputVolumn(device_name.toStdString(), volume);
        emit NotifyVoiceChangeSignal(type, volume);
      }
      break;
    }
    case kSpeaker: {
      QString device_name = config->AudioOutputDevice();
      if (!device_name.isEmpty()) {
        DeviceManager::GetInstatnce()->SetAudioOutputVolumn(volume);
        emit NotifyVoiceChangeSignal(type, volume);
      }
      break;
    }
    default:
      assert(false);
      break;
  }
}

void DisplayController::HandleVoiceMuteRequestSlot( AudioDeviceType type, bool isMute ) {
  if (!is_model_hd_) return;

  RunningConfig *config = RunningConfig::Instance();
  switch (type) {
    case kMicrophone: {
      // TODO: to be implemented
      assert(false);
      break;
    }
    case kSpeaker: {
      QString device_name = config->AudioOutputDevice();
      if (!device_name.isEmpty()) {
        DeviceManager::GetInstatnce()->SetAudioOutputMute(isMute);
        emit NotifyMuteChangeSignal(kSpeaker, isMute);
      }
      break;
    }
    default:
      assert(false);
      break;
  }
}

void DisplayController::HandlePermitSpeakRequestSlot(
    const QString &uri, bool allow) {
  if (conference_controller_->IsInConference()) {
    conference_controller_->HanldePermitTerminalSpeakRequestSlot(uri, allow);
    // TODO: �ȴ��ײ��֪ͨ
    // �ֽ׶ΰ�����һ�ɳɹ�
    emit NotifyTerminalSpeakSignal(uri, allow);
  }
}

void DisplayController::HandleConferenceStateChangedNotifySlot(
    const QString &conference_uri, 
    IConferenceController::ConferenceState state) {
  if (state == IConferenceController::kIsInConference) {
    current_conference_uri_ = conference_uri;
  } else if (current_conference_uri_ == conference_uri && 
             state == IConferenceController::kIsNotInConference) {
    // TODO: �Ժ���ܷ����źţ������ͬ��
    // ע�⣺Ŀǰ������Ҫ���˳�����ʱ������Լ���״̬
    QList<VideoSeatInfo> local_display;
    for (auto &video_seat : video_display_dict_) {
      if (!isLocalPreviewMediaID(video_seat.terminal_vuri)) {
        emit NotifyStopVideoSignal(
          video_seat.terminal_vuri, 
          video_seat.screen_index, 
          video_seat.seat_index);
      } else {
        local_display.push_back(video_seat);
      }
    }
    video_display_dict_.clear();
    for (auto &video_seat : local_display) {
      video_display_dict_.insert(video_seat.terminal_vuri, video_seat);
    }

    for (auto &shared_screen_seat : shared_screen_list_) {
      emit NotifyStopDocumentSignal(
        shared_screen_seat.terminal_uri, 
        shared_screen_seat.screen_index);
    }
    shared_screen_list_.clear();
  }
}

void DisplayController::HandleScreenLayoutRequestSlot(int screen_index, 
                                                      ScreenLayout layout) {
  if (screen_index < 0 || screen_index >= screen_count_) {
    return;
  }
  screen_layout_[screen_index] = layout;
  media_manager_->ChangeLayout(ConvertToDisplayModel(layout), screen_index);
  emit NotifyScreenLayoutChangedSignal(screen_index, layout);
}

void DisplayController::HandlePanelVisibleStateChangedNotifySlot(
    int telecontroller_index, bool visible) {
  if( telecontroller_proxy_ ) {
    QByteArray output_array;
    QDataStream out(&output_array , QIODevice::WriteOnly );
    out << TELE_PanelState << visible; 
    telecontroller_proxy_->TeleInfo(
      TELE_PanelState, telecontroller_index, output_array);
  }
}

bool DisplayController::IsLocalTerminalURI( const QString &vuri ) const {
  QString local_uri = 
    QString::fromStdString(RunningProfile::getInstance()->user_uri());
  return TerminalHelper::GetTerminalURI(vuri) == local_uri;
}

void DisplayController::HandleReceiveAllLocalVideoNotifySlot() {
  const int videos = RunningConfig::Instance()->VideoCaptureDeviceCount();
  int i = 0;
  int screen_index = 0;

  //���Զ����յı�����Ƶ���ڴ�����/����ʾ��/�����������λ�õ���Ļgmlan20150706
  if (CVIniConfig::getInstance().isAutoSetPrimaryScreen())
  {
	  screen_index = ScreenHelper::Instance()->GetPrimaryScreen();
	  //printf("------screen_index:%d",screen_index);
	  qDebug()<<"----------screen_index:"<<screen_index;
  } 
  else
  {
	  screen_index = RunningConfig::Instance()->MainScreen();
	  //printf("------MainScreen:%d",screen_index);
	  qDebug()<<"----------MainScreen:"<<screen_index;
  }
  
  while (i < videos && screen_index < screen_count_) {
    int seat_index = FindFreeVideoSeat(screen_index);
    if (seat_index > 0) {
      HandleReceiveVideoRequestSlot(
        getLocalPreviewMediaID(i++), screen_index, seat_index, false);
    } else {
      ++screen_index;
    }
  }
}

void DisplayController::HandleRequestStreamMediaAutoReceiveSlot( 
  const QString &remote_uri )
{
  if (nullptr != FindVideoSeat(remote_uri)) {
    // �Ѿ��ڽ��գ��������½���
    return;
  }
  for (int i = 0; i < screen_count_; ++i) {
    int seat_index = FindFreeVideoSeat(i);
    if (0 < seat_index) {
      AddVideoSeat(remote_uri, i, seat_index, false);
      return;
    }
  }
}

int DisplayController::FindFreeVideoSeat( int screen_index )
{
  int seat_count = GetVideoSeatCount(screen_index);

  for (int i = 1; i <= seat_count; ++i) {
    if (nullptr == FindVideoSeat(screen_index, i)) {
      return i;
    }
  }
  return -1;
}

int DisplayController::GetVideoSeatCount( int screen_index ) {
  int seat_count = 0;
  const IDisplayController::ScreenLayout layout = screen_layout_[screen_index];
  switch (layout) {
  case IDisplayController::kLayoutOf1x1:
    seat_count = 1; break;
  case IDisplayController::kLayoutOf2x2:
    seat_count = 4; break;
  case IDisplayController::kLayoutOf3x3:
    seat_count = 9; break;
  case IDisplayController::kLayoutOf1plus5:
    seat_count = 6; break;
  case IDisplayController::kLayoutOfAuto:
    seat_count = 6; break;
  case IDisplayController::kLayoutOfDocument:
    seat_count = 0; break;
  case IDisplayController::kLayoutOfTop1x2:
    seat_count = 2; break;
  case IDisplayController::kLayoutOf4x4:
    seat_count = 16; break;
  default:
    seat_count = 0; break;
  }
  return seat_count;
}

void DisplayController::HandleCheckReceiveVideoAckSlot()
{
  for (auto it = video_display_dict_.begin();
    it != video_display_dict_.end();
    ++it) {
    if (!it->is_ack && it->ack_time_counter.elapsed() > 3000) {
      // 3s��ʱ
      qDebug() << "Wait MediaReceiveReply timeout " << it->terminal_vuri;
      conference_controller_->HandleStartMediaReplyTimeoutSlot(
        it->terminal_vuri);
      it->ack_time_counter.restart(); //����
    }
  }
}

void DisplayController::HandleStreamMediaReceiveReplyNotifySlot( 
  const QString &remote_vuri, bool permission )
{
  qDebug() << "Recv MediaReceiveReply from " << remote_vuri;
  QString uri = TerminalHelper::GetTerminalURI(remote_vuri);
  // video
  auto it = video_display_dict_.find(remote_vuri);
  if (it != video_display_dict_.end()) {
    it->is_ack = true;
  }
  // audio
  auto &seat_list = audio_display_dict_[uri];
  auto temp = find_if(seat_list.begin(), seat_list.end(), 
    [&remote_vuri](const AudioSeatInfo &info) {    
      return info.terminal_vuri == remote_vuri;
  });
  if (temp != seat_list.end()) {
    temp->is_audio_received = permission;
    if (!permission) {
      // ���Ͷ˲��Դ�·������Ƶ��������Ҫ������Ƶ����
      temp->is_audio_enabled = false;
      RearrangeAudioRender(uri);
    }
  }
}

void DisplayController::RearrangeAudioRender( const QString &id )
{
  auto &seat_list = audio_display_dict_[TerminalHelper::GetTerminalURI(id)];
  //������Ƶ�Ƿ����ڲ�����
  auto temp = find_if(seat_list.begin(), seat_list.end(), 
    [](const AudioSeatInfo &info) {    
      return info.is_audio_enabled;
  });
  if (temp == seat_list.end()) {
    //��ǰû�в��Ÿ��ն���Ƶ
    temp = find_if(seat_list.begin(), seat_list.end(), 
      [](const AudioSeatInfo &info) {    
        return info.is_audio_received;
    });
    if (temp != seat_list.end()) {
      qDebug() << "Rearrange audio render by" << temp->terminal_vuri;
      (*temp).is_audio_enabled = true;
      HandleReceiveAudioRequestSlot((*temp).terminal_vuri, true);
    } else {
      qDebug() << "No audio from" << TerminalHelper::GetTerminalURI(id);
    }
  }
}

//  [12/10/2015 slt begin]
void DisplayController::HandleClearAudioDisplayDictNotifySlot()
{
    audio_display_dict_.clear();
}


