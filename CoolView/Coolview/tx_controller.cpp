#include "tx_controller.h"

#include <ctime>

#include "util/MediaManager.h"
#include "util/ini/CVIniConfig.h"
#include "util/ini/TxConfig.h"
#include "config/RunningConfig.h"
#include "profile/RunningProfile.h"
#include "udp_report_sender.h"
#include "CoolviewCommon.h"
#include "tx_util.h"

#include "dbus/txController/service/TxControllerAdaptor.h"
#include "dbus/txController/common/TxControllerServiceCommon.h"
#include "dbus/channel/type/streamedMedia/common/StreamedMediaServiceCommon.h"
#include "dbus/conferenceRoom/common/ConferenceRoomServiceCommon.h"
#include "dbus/txMonitor/common/TxMonitorServiceCommon.h"
#include "dbus/txMonitor/client/TxMonitorIf.h"

#include "display_controller_interface.h"
#include "msg_manager_interface.h"

using namespace tx_util;

TXController::TXController(QObject *parent, 
                           RunningProfile *profile,
                           RunningConfig *config)
    : ITXController(parent), 
      profile_(profile), 
      running_config_(config) {
  new TxControllerAdaptor(this);

  conference_controller_ = nullptr;
  media_manager_ = nullptr;
  is_tx_terminal_ = CVIniConfig::getInstance().IsModelTX();
  recording_path_ = TXGetRecordRootDir();
  auto_record_delay_ = CTxConfig::getInstance().GetRecAutoStartDelay();
}

TXController::~TXController() {
}

void TXController::Initialize(
    ISessionController *session_controller,
    IMsgManager *msg_manager,
    IConferenceController *conference_controller,
    IDisplayController *display_controller,
    MediaManager *media_manager) 
{
  if (!is_tx_terminal_) {
    //��TX�򲻴����κ���Ϣ
    return;
  }

  // ע��DBus
  QDBusConnection connection = QDBusConnection::sessionBus();
  bool ret = connection.registerService(TXCONTROLLER_SERVICE_NAME);
  ret = connection.registerObject(TXCONTROLLER_SERVICE_OBJECT_PATH ,this);

  assert(conference_controller);
  conference_controller_ = conference_controller;
  terminal_manager_ = &(conference_controller->GetTerminalManager());

  // ��������controllers���ź�
  // �����ն�״̬�������Щ��QueuedConnectionȷ�������źŵķ���������ɺ��ٵ�������Ĳ�
  connect(conference_controller_, &IConferenceController::NotifyTerminalReceivedSignal,
    this, &TXController::HandleTerminalStatusChangedSlot, Qt::QueuedConnection); //�״��յ��ն��б�������ź�
  connect(conference_controller_, &IConferenceController::NotifyTerminalUpdateSignal,
    this, &TXController::HandleTerminalStatusChangedSlot, Qt::QueuedConnection); //�ն�״̬����������ص�½����������ź�
  connect(conference_controller_, &IConferenceController::NotifyTerminalReloginSignal,
    this, &TXController::HandleTerminalReloginSlot); //�˴���Ҫʹ��QueuedConnection��ȷ���ڻָ�ý��������ǰ���ò�

  connect(conference_controller_, &IConferenceController::NotifyStreamMediaReceiveStartedSignal,
    this, &TXController::HandleStreamMediaReceiveStartedSlot);
  connect(conference_controller_, &IConferenceController::NotifyStreamMediaReceiveStoppedSignal,
    this, &TXController::HandleStreamMediaReceiveStoppedSlot);
  connect(conference_controller_, &IConferenceController::NotifyConferenceStateChangedSignal,
    this, &ITXController::HandleConferenceStateChangedSlot);
  connect(this, &TXController::NotifyAllRecordJobFinishSignal,
    conference_controller_, &IConferenceController::HandleAllMediaProcessExitSlot);
  
  assert(session_controller);
  connect(
    session_controller, 
    &ISessionController::NotifySessionStateChangedSignal,
    this,
    &TXController::HandleSessionStateChangedNotifySlot);

  assert(media_manager);
  media_manager_ = media_manager;

  assert(display_controller);
  connect(this, &TXController::StartReceiveVideoRequestSignal,
    display_controller, &IDisplayController::HandleRequestStreamMediaAutoReceiveSlot);
  connect(this, &TXController::StopReceiveVideoRequestSignal,
    display_controller, &IDisplayController::HandleStopVideoRequestSlot);

  IMsgReceiver *receiver = msg_manager->GetMsgReceiver();
  assert(receiver);
  connect(
    receiver, &IMsgReceiver::NotifyRecvRecordControlSignal,
    this, &TXController::HandleRecvRecordControlSlot);

  IMsgSender *sender = msg_manager->GetMsgSender();
  assert(sender);
  connect(
    this, &TXController::SendRecordControlRequestAckSignal,
    sender, &IMsgSender::SendRecordControlAckSlot);

  // timers
  auto_record_timer_ = new QTimer;
  connect(auto_record_timer_, SIGNAL(timeout()),
    this, SLOT(HandleAutoRecordTimerTimeoutSlot()));

  // tx monitor client
  tx_monitor_.reset(new TxMonitorIf(TX_MONITOR_SERVICE_NAME, 
    TX_MONITOR_SERVICE_OBJECT_PATH, QDBusConnection::sessionBus(), this));
  assert(tx_monitor_);

  // initialize variables
  max_file_size_ = CTxConfig::getInstance().GetRecMaxFileSize();
  if (0 > max_file_size_) max_file_size_ = 0;

  max_file_duration_ = CTxConfig::getInstance().GetRecMaxFileDuration() * 1000; // sec to ms
  if (0 > max_file_duration_) max_file_duration_ = 0;
}

QString TXController::GenerateRecordFilePath(TerminalPointer terminal) {
  FileNameParam param;
  param.camid = terminal->virtual_index + 1; // index��Ҫ��1��ʼ������Ҫ��1

  ConferencePointer current_conference = conference_controller_->GetCurrentConference();
  assert(current_conference);
  int pos = current_conference->start_time.indexOf(' '); //����ʱ��֮���и��ո�ֻȡ����
  if (pos != -1) {
    param.conf_date = current_conference->start_time.mid(0, pos);
  }
  param.conf = current_conference->title;

  param.terminal = terminal->name;
  pos = param.terminal.lastIndexOf('(');
  if (-1 < pos) {
    param.terminal.truncate(pos);
  }

  if (terminal_record_dict_.contains(terminal->uri) && terminal_record_dict_[terminal->uri]) {
    TerminalRecordPointer terminal_record = terminal_record_dict_[terminal->uri];
    param.title = terminal_record->episode_user_info.title;
    param.participant = terminal_record->episode_user_info.participants;
    param.keywords = terminal_record->episode_user_info.keywords;
    param.description = terminal_record->episode_user_info.description;
  } else {
    param.title = param.terminal;
  }

  QString file = QString::fromLocal8Bit(
    CTxConfig::getInstance().GetRecFileNameFormat().c_str());
  if (file.isEmpty()) {
    // ·����{¼�ƻ���λ��}/{��������}_{������}/{¼������}/{�ն���}/{�ļ���}
    // �ļ�����{����}-{����ʱ��}-{ý�������}-{��Ƶ��ʽ}.mp4
    QString file = QString("%1\\%2_%3\\%4\\%5\\%6_%7_%8_%9.mp4")
      .arg(recording_path_)
      .arg(param.conf_date).arg(param.conf).arg(TX_FNFMT_DATE).arg(param.terminal)
      .arg(param.title).arg(TX_FNFMT_DATETIME).arg(param.camid).arg(TX_FNFMT_VINFO);
  } else {
    QString f = GetRecordFileNamePath(file, param);
    if (f.size() > 1 && f[0] == '\\') {
      file = recording_path_ + f;
    } else {
      file = recording_path_ + "\\" + f;
    }
  }

  return file;
}

int TXController::GetRecordInitialJobId(const QString &vuri) {
  const QString uri = TerminalHelper::GetTerminalURI(vuri);
  if (terminal_record_dict_.contains(uri)) {
    //���ڼ�¼˵��¼����Զ�̻��Զ����������������ն˵����صǺ����½���
    //Զ�̺��Զ�����ʱ�����ȼӼ�¼�ŵ���һ���ģ����Կ϶���һ��episode��¼��
    //�ֶ�����ʱ���ն��׸�vuri�������Ҳ�����¼����ִ��if����߼��������vuriҲһ�����ҵ���¼��
    //�������½��գ���ԭ������episode��¼����episode��job_id����0��ʼ����HandleTerminalReloginSlot
    TerminalRecordPointer terminal_record_node = terminal_record_dict_[uri];
    if (terminal_record_node->episodes.empty()) {
      //�߼��Ƿ���Ҫ��¼��־
      return 0;
    }
    int job_id = terminal_record_node->episodes.last()->job_id;
    ViewRecordPointer view = FindViewNode(vuri, job_id);
    if (!view || view->start) {
      qWarning() << "Start record [" << vuri << "] with old job_id=" << job_id;
    }
    return job_id;
  }

  //��û�м�¼����Ӧ�����Զ�������Զ�̿�����������ķ�ʽ����ֱ�Ӳ���TX�ֶ�������
  //��ʱӦ�ô�����¼�ڵ�
  StartRecordParam param;
  param.target_uri = uri;
  param.requester_uri = QString::fromLocal8Bit("�ֶ�");
  TerminalRecordPointer terminal_record_node = StartRecord(param, true);
  if (!terminal_record_node || terminal_record_node->episodes.empty()) {
    //��������쳣�����Ҫ��¼��־��
    return 0;
  }
  return terminal_record_node->episodes.last()->job_id;
}

void TXController::HandleSegmentMediaRequestSlot(const QString &uri_or_vuri) {
  QString uri = TerminalHelper::GetTerminalURI(uri_or_vuri);
  if (!terminal_record_dict_.contains(uri)) return;

  TerminalRecordPointer terminal = terminal_record_dict_[uri];
  EpisodeSysInfoPointer last_episode;
  if (terminal->episodes.size() == 0) {
    //�ɵļ���û�У�̸ʲô�ּ�����
    return;
  }
  last_episode = terminal->episodes.back();

  //���ηּ�����С��20�룬�Ա�ײ�������ָ���Ȼ������ӵ�Ŷ
  long long now = GetNonrepeatableTimestamp(); // in us
  if (now - last_episode->start_time < 20 * 1000000LL) {
    return;
  }

  //�����µķּ�
  EpisodeSysInfoPointer new_episode = AddEpisodeNode(terminal);

  //����һ·Ҫ��ָ��÷��Ͷ�����¼�Ƶ�������Ƶ���ָ�Ա�ͳһ�ּ�
  if (new_episode) {
    //��¼�Ѿ��������ֶ�ָ��
    //���磬ĳ�ն���ABCD��·��Ƶ������A�������ƴ����Զ��ֶΣ����ʱ��·�Ѿ�����Ҫ�󡱷ָ�һ�Σ�
    //��Ҫ��ָ�����������´�ײ㣬�������ڽ��̼�ͨ�ŵ�ͬ�����⼰��Ƶ�����ؼ�֡�ŷָ��ԭ��
    //�ײ㲢δ���̾ͷָ��ǡ�ô�ʱBҲ�������ƣ�����ٴ����ֶΣ�����ܵ�����Щ��Ƶ����ָ�һ��
    //���³���һ������Ķ���Ƶ
    last_episode->segmented = true;

    //�����ֶ�ָ��
    for (ViewRecordPointer view : new_episode->views) {
      view->start = true;
      media_manager_->SegmentMedia(view->vuri, new_episode->job_id);
    }
  }
}

void TXController::HandleSessionStateChangedNotifySlot(
    ISessionController::SessionState state) {
  if (is_tx_terminal_) {
    if (state == ISessionController::kIsOnline) {
      startWeblibUploader();
    } else {
      stopWeblibUploader();
    }
  }
}

void TXController::HandleConferenceStateChangedSlot(
  const QString &conference_uri, 
  IConferenceController::ConferenceState state) {
  if (state == IConferenceController::kIsInConference) {
    current_conference_ = conference_controller_->GetCurrentConference();
    terminal_record_dict_.clear(); // ���ν������֮��Ӧ�ȴ�ǰһ�λ����ý������ȫ�ͷţ�����¼����ҵ���޷�������
    terminal_uri_list_.clear();
    auto_record_timer_->start(kAutoRecordCheckTimeOut);
  } else if (state == IConferenceController::kIsNotInConference) {
    auto_record_timer_->stop();
  }
}

void TXController::HandleStreamMediaReceiveStartedSlot(const QString &remote_vuri) {
  QString uri = TerminalHelper::GetTerminalURI(remote_vuri);
  if (!terminal_record_dict_.contains(uri)) {
    return;
  }
  TerminalRecordPointer terminal_record_node = terminal_record_dict_[uri];
  ViewRecordPointer view = FindViewNode(remote_vuri, 
    terminal_record_node->next_job_id - 1);
  if (view) {
    view->start = true;
  }
}

void TXController::HandleStreamMediaReceiveStoppedSlot(const QString &remote_vuri) {
  //nothing to be done
  //��Ҫ�������Ƴ�node����������¼����ɱ���Ҫ����Զ��
}

void TXController::HandleReceiveUDPQoSReportSlot(const RtpStatItem& rtp_stat) {
  //TODO:������ʱ������Ϊ��ȡ��TerminalDescription��ȱ�ٵ�ip��Ҫ����Щ�͵øķ�����
  if (strlen(rtp_stat.rem_addr) == 0) {
    //��һ�������޵�ַ
    return;
  }
  if (!username_to_vuri_dict_.contains(rtp_stat.member_id)) return;
  QString uri = username_to_vuri_dict_[rtp_stat.member_id];
  uri = TerminalHelper::GetTerminalURI(uri); // vuri to uri
  if (!terminal_record_dict_.contains(uri)) return;
  auto terminal_record_node = terminal_record_dict_[uri];
  //if (!terminal_record_node->reporter) {
  //  terminal_record_node->source_ip = rtp_stat.rem_addr;
  //  //TODO:�˿����Ҳ�ӷ�������
  //  terminal_record_node->report_port = CVIniConfig::getInstance().getConfRoomQosServerUdpPort();
  //  terminal_record_node->reporter.reset(new UDPReportSender);
  //  terminal_record_node->reporter->Connect(terminal_record_node->source_ip, terminal_record_node->report_port);
  //}
}

void TXController::HandleReceiveUDPRecReportSlot( const RecStatItem& rec_stat )
{
  //step1: �����ź�֪ͨ����
  emit NotifyRecStatReceivedSignal(rec_stat);

  //step2�����ͱ����Դ��
  if (!username_to_vuri_dict_.contains(rec_stat.user_name)) {
    //�����쳣��Ӧ�ü�¼��־
    return;
  }
  QString vuri = username_to_vuri_dict_[rec_stat.user_name];
  QString uri = TerminalHelper::GetTerminalURI(vuri);
  if (!terminal_record_dict_.contains(uri)) {
    //�����쳣��Ӧ�ü�¼��־
    return;
  }

  /*TerminalRecordPointer terminal_record_node = terminal_record_dict_[uri];
  if (terminal_record_node->reporter)
    (terminal_record_node->reporter)->SendUDPRecReport(rec_stat);*/

  //step3������״̬������⹤��
  switch (rec_stat.statType) {
  case REC_STAT_FILE_WRITING:
    {
      ViewRecordPointer view_node = FindViewNode(vuri, rec_stat.rec.job_id);
      if (!view_node) return; //�����쳣�����ü�¼��־��
      //���±��� - ��һ����չ������
      view_node->record_status = rec_stat;
      if (ShouldNewEpisodeStart(view_node)) {
        //���Ҫ�Զ��ֶ�
        HandleSegmentMediaRequestSlot(uri);
      }
    }
    break;
  case REC_STAT_FILE_CLOSE:
    ViewRecordComplete(vuri, rec_stat);
    break;
  case REC_STAT_FILE_ABANDON:
    ViewRecordAbandon(vuri, rec_stat);
    break;
  case REC_STAT_MUX_STOP:
    RecordProcessExit(vuri);
    break;
  }
}

bool TXController::ShouldNewEpisodeStart(ViewRecordPointer view)
{
  //���ֶ�ָ���Ƿ��Ѿ�������������η����ֶ�ָ��
  EpisodeSysInfoPointer episode = 
    FindEpisodeNode(view->vuri, view->record_status.rec.job_id);
  if (episode->segmented)
    return false;
  //���ļ���С�ָ����ļ���С�Ƿ񳬹�����
  if (0 != max_file_size_ && 
      view->record_status.rec.size >= max_file_size_)
    return true;
  //����ļ�ʱ���Ƿ񳬹��������
  if (0 != max_file_duration_ && 
      view->record_status.rec.duration >= max_file_duration_)
    return true;
  return false;
}

void TXController::HandleTerminalStatusChangedSlot(
  ConstTerminalPointer terminal,
  const QString &conference_uri)
{
  //������ͨ�նˣ�����TX�Լ�����������
  if (TerminalHelper::IsModelTX(terminal->model) ||
      terminal->uri == profile_->user_uri().c_str()) {
    return;
  }

  QString uri = terminal->uri;

  //Step1����¼�ն�uri
  if (!terminal_uri_list_.contains(uri)) {
    terminal_uri_list_.push_back(uri);
  }
  
  //Step2�������Զ�¼��
  if (!CTxConfig::getInstance().GetRecAutoStart()) {
    //û�������Զ�¼��
    return;
  }

  //¼���Ѵ��ڣ���������ظ�����ն��б��µģ�ɶҲ����
  if (terminal_record_dict_.contains(uri)) {
    return;
  }

  //����Ҫͬʱ��������·��¼�ƣ�������ն˵������ն�δ��ȫ�����Ȳ�����
  TerminalList virtual_terminals = (*terminal_manager_)[uri];
  if (virtual_terminals.size() == 0 || 
      virtual_terminals.size() < terminal->virtual_count) {
    return;
  }

  //�Է������ߣ����Զ�����
  if (!terminal->is_available) {
    return; //����жϲ������Լ���Ϊ�˷�ֹ��������TX�ͺ����ó���ͨ�ն�ʱ�����½����Լ�����
  }

  //�ӳ������Զ�¼��
  //��������������Ϊ����̫��ʱ�����Ͷ˿���û����ȫ���յ��ն��б������ܸ����Ƿ�ý����
  AutoStartPendingInfo pending;
  pending.uri = uri;
  pending.online_time = time(nullptr);
  auto_record_pending_list_.push_back(pending);
}

void TXController::HandleTerminalReloginSlot(
  ConstTerminalPointer terminal,
  const QString &conference_uri)
{
  //������virtual indexΪ0�������նˣ������ظ�ִ��
  if (!terminal || terminal->virtual_index != 0) {
    return;
  }

  QString uri = terminal->uri;
  //֮ǰû����¼���ص�½Ҳ��¼
  if (!terminal_record_dict_.contains(uri)) return;

  TerminalRecordPointer terminal_record_node = terminal_record_dict_[uri];
  //�����µķּ����Ա�ý������������ǰ��ȡjob_id
  EpisodeSysInfoPointer new_episode = AddEpisodeNode(terminal_record_node);
}

//�÷����ѷ�������������
void TXController::HandleRecvRecordControlSlot(const RecordControlMsg &info)
{
  if (!(conference_controller_->IsInConference() && 
      CVIniConfig::getInstance().IsModelTX())) {
    return;
  }

  qDebug() << "Record ctrl type:" << info.type << "vuri:" << info.target_vuri;
  switch (info.type)
  {
  case RecCtrl_Start:
    //RemoteStartRecord(info);
    break;
  case RecCtrl_Stop:
    //RemoteStopRecord(info);
    break;
  default:
    break;
  }
}

int TXController::ControlRecord(int op, const QByteArray &bytes)
{
  if (conference_controller_ && !conference_controller_->IsInConference()) {
    //���ڻ����У�������
    return TXRecord_NotInFocus;
  }

  switch (op)
  {
  case TXRecordOp_Start:
    {
      StartRecordParam param = DBusParamFromByteArray<StartRecordParam>(bytes);
      return RemoteStartRecord(param);
    }
    break;
  case TXRecordOp_Stop:
    {
      StopRecordParam param = DBusParamFromByteArray<StopRecordParam>(bytes);
      return RemoteStopRecord(param);
    }
    break;
  default:
    break;
  }
  return TXRecord_Error;
}

QByteArray TXController::GetRecordStatus(const QString &focus, const QString &uri)
{
  RecordFocusStatus focus_status;

  if (!terminal_manager_ || !current_conference_ ||
      focus != current_conference_->uri) {
    return DBusParamToByteArray(focus_status);
  }

  //ֻ��������¼�Ƶ��ն���Ϣ
  for (const QString &uri_in_focus : terminal_uri_list_) {
    if (!uri.isEmpty() && uri != uri_in_focus) continue; //���ָ����uri��ֻ�����ض�uri��¼����Ϣ

    if (terminal_record_dict_.contains(uri_in_focus)) {
      //����Ϣ����ʾ���ڱ�¼��
      TerminalRecordPointer terminal = terminal_record_dict_[uri_in_focus];
      if (terminal->episodes.empty()) continue; //��Ӧ�����쳣������Ժ���
      EpisodeSysInfoPointer episode = terminal->episodes.last();
      if (episode->views.empty()) continue; //��Ӧ�����쳣������Ժ���

      for (ViewRecordPointer view : episode->views) {
        TerminalPointer terminal_pointer = terminal_manager_->FindTerminal(view->vuri);
        RecordTerminalStatus terminal_status;
        terminal_status.vuri = view->vuri;
        terminal_status.name = (terminal_pointer ? terminal_pointer->name : 
          QString("%1(%2)").arg(terminal->name).arg(TerminalHelper::GetVirtualIndex(view->vuri)));
        terminal_status.status = TxRecordStatus_Recording;
        terminal_status.virtual_count = (terminal_pointer ? terminal_pointer->virtual_count : 0);
        terminal_status.requester_uri = terminal->requester_uri;
        if (view->record_status.statType == REC_STAT_FILE_WRITING) {
          terminal_status.duration = view->record_status.rec.duration;
          terminal_status.recorded_bytes = view->record_status.rec.size;
        }
        focus_status.terminals_status.push_back(terminal_status);
      }
    } else {
      //�ն�û�б�¼��
      TerminalPointer terminal_pointer = terminal_manager_->FindTerminal(uri_in_focus);
      if (!terminal_pointer) continue; //��Ӧ�����쳣������Ժ���
      RecordTerminalStatus terminal_status;
      terminal_status.vuri = terminal_pointer->virtual_uri;
      terminal_status.name = terminal_pointer->name;
      terminal_status.status = (terminal_pointer->is_available ? TxRecordStatus_Online : TxRecordStatus_Offline);
      focus_status.terminals_status.push_back(terminal_status);
    }
  }

  return DBusParamToByteArray(focus_status);
}

int TXController::RemoteStartRecord( const StartRecordParam &param )
{
  //׼����������
  assert(current_conference_);
  QString current_user_uri = QString::fromStdString(profile_->user_uri());
  const QString target_uri = TerminalHelper::GetTerminalURI(param.target_uri);

  //�Ƿ�ͬһ����
  if (param.focus != current_conference_->uri) {
    return TXRecord_NotInFocus;
  }

  //����ն��Ƿ���Ч
  TerminalList virtual_terminals = (*terminal_manager_)[target_uri];
  if (virtual_terminals.empty()) {
    return TXRecord_UriNotExist; // ����ʧ��
  }

  //ֻҪ���ն��κ�һ·�Ѿ���¼���ˣ��Ͳ�����������¼�ƣ���Ȼ�߼��ø��ӣ����׳����
  //TODO:TX�ֶ�¼��ʱ��������ÿ·����ϰ�����
  if (terminal_record_dict_.contains(target_uri)) {
    return TXRecord_IsRecording; // �Ѿ�������
  }

  //����¼��
  TerminalRecordPointer terminal_record_node = StartRecord(param, false);
  if (!terminal_record_node) {
    return TXRecord_Error; // ����ʧ��
  }

  return TXRecord_Success;
}

int TXController::RemoteStopRecord( const StopRecordParam &param )
{
  //׼����������
  assert(current_conference_);
  QString target_uri = TerminalHelper::GetTerminalURI(param.target_uri);

  //�Ƿ�ͬһ����
  if (param.focus != current_conference_->uri) {
    return TXRecord_NotInFocus;
  }

  if (!terminal_record_dict_.contains(target_uri)) {
    //û����¼
    return TXRecord_IsNotRecording;
  }

  //�������ն���������ȷ��ÿ·��ֹͣ�ˣ�
  TerminalList terminals = (*terminal_manager_)[target_uri];
  for (auto &t : terminals) {
    //ֹͣ���պ�¼��
    emit StopReceiveVideoRequestSignal(t->virtual_uri);
    //�����������Ϣ��UI
    emit NotifyRecordRequesterSignal(t->virtual_uri, "");
  }

  //��Ҫɾ��UDP���淢�������ײ���ļ�����������ӳٵ��ɾ��reporter
  //�ᵼ�±����޷��ͳ����´η���¼��ʱ������IP�Ͷ˿ڵ�
  //record_terminal_iter->reporter = nullptr;

  return TXRecord_Success;
}

TXController::TerminalRecordPointer TXController::StartRecord(
  const StartRecordParam &param, 
  bool manual)
{
  QString uri = param.target_uri;
  TerminalList virtual_terminals = (*terminal_manager_)[uri];
  if (terminal_record_dict_.contains(uri) || 
      virtual_terminals.empty() || 
      !virtual_terminals[0]->is_available) {
    //�Ѿ���¼���У����ն˲����ڣ������ߣ�������ʧ��
    return nullptr;
  }

  //����ն˵�¼�Ƽ�¼
  TerminalRecordPointer terminal_record_node = AddTerminalNode(uri);
  if (!terminal_record_node) {
    //���ʧ��
    return nullptr;
  }

  //�ӿ��Ʋ�����ȡ��Ƶ����Ϣ
  if (param.title.isEmpty()) {
    //û��������⣬Ĭ�����ն���������
    terminal_record_node->episode_user_info.title = terminal_record_node->name;
  } else {
    terminal_record_node->episode_user_info.title = param.title;
  }
  terminal_record_node->episode_user_info.participants = param.participants;
  terminal_record_node->episode_user_info.keywords = param.keywords;
  terminal_record_node->episode_user_info.description = param.description;
  terminal_record_node->requester_uri = param.requester_uri;

  //TODO:�������������ʼ��reporter������TerminalDescription��û��ip  ||-_-
  //     ���Գ�ʼ��reporter���յ��ײ��QoS����󣬻����ip�ٴ���

  assert(terminal_record_node->episodes.size() == 1);
  for (auto &t : virtual_terminals) {
    //����user_name��vuriӳ���ϵ
    QString user_name = TerminalHelper::GetUsername(t->virtual_uri);
    username_to_vuri_dict_[user_name] = t->virtual_uri;
    //������һ����view_node
    AddViewNode(terminal_record_node->episodes.at(0), t->virtual_uri);
    if (!manual) {
      //���ֶ�����ʱ��Ҫ�������պ�¼�񣬾���ʵ�ּ����ӵĲ�
      //�ֶ�������ζ�Ž��ս����Ѿ�����������Ҫ�ٴ�������
      emit StartReceiveVideoRequestSignal(t->virtual_uri); 
    }
    //���²�������Ϣ��UI
    emit NotifyRecordRequesterSignal(t->virtual_uri, param.requester_uri);
  }

  return terminal_record_node;
}

void TXController::ViewRecordComplete(
  const QString &vuri, 
  const RecStatItem &rec_stat)
{
  QString uri = TerminalHelper::GetTerminalURI(vuri);
  //�ܵ�������һ�п϶�û����
  TerminalRecordPointer terminal_record_node = terminal_record_dict_[uri];
  assert(terminal_record_node);
  EpisodeSysInfoPointer episode = FindEpisodeNode(uri, rec_stat.close.job_id);

  //��Ҫ����Ҫ��terminal_manager_��ȡ���ݣ���Ϊ�÷����������˳���������
  //���˳����顪��¼����ɡ������ô˴�������ʱterminal_manager_�Ѿ���ա�
  //����Ҫ�õ����ն���ϢҪ��terminal_record_node�渱��
  //TerminalPointer terminal = terminal_manager_->FindTerminal(vuri);

  RecordCompleteParam param;
  if (current_conference_) {
    param.conference.cid = current_conference_->cid.toInt();
    param.conference.title = current_conference_->title;
    param.conference.start_time = current_conference_->start_time;
    param.conference.description = current_conference_->description;
  }
  if (terminal_record_node && episode) {
    param.terminal.uri = terminal_record_node->uri;
    param.terminal.name = terminal_record_node->name;

    param.episode.id = episode->start_time; //��û���ظ���ʱ�����Ϊ����ʱid
    param.episode.requester = terminal_record_node->requester_uri;
    param.episode.title = terminal_record_node->episode_user_info.title;
    param.episode.participants = terminal_record_node->episode_user_info.participants;
    param.episode.keywords = terminal_record_node->episode_user_info.keywords;
    param.episode.description = terminal_record_node->episode_user_info.description;
    param.episode.start_time = episode->start_time / 1000000LL; // ΢��ת������
  }
  param.view.camera_index = TerminalHelper::GetVirtualIndex(vuri);
  param.view.start_time = rec_stat.close.start_time; // in ms
  param.view.duration = rec_stat.close.duration; // in ms
  param.view.file = QString::fromLocal8Bit(rec_stat.close.file);
  {
    // JSON
    QJsonObject video;
    video.insert("width", rec_stat.close.video_width);
    video.insert("height", rec_stat.close.video_height);
    video.insert("fps", rec_stat.close.video_fps);
    QJsonObject audio;
    audio.insert("rate", rec_stat.close.audio_sample_rate);
    audio.insert("channel", rec_stat.close.audio_channel);
    audio.insert("bit", rec_stat.close.audio_bits_count);
    QJsonObject media;
    media.insert("video", video);
    media.insert("audio", audio);
    QJsonDocument document;
    document.setObject(media);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);

    param.view.media_format = QString(byte_array);
  }

  //tell tx monitor
  if (tx_monitor_) {
    tx_monitor_->ReportRecStat(RecStat_FileClose, DBusParamToByteArray(param));
  }

  ViewRecordPointer view = FindViewNode(vuri, rec_stat.close.job_id);
  if (view) {
    view->complete = true;
  }
  //����¼�Ƽ�¼
  RemoveFinishedEpisodeNode(vuri);
}

void TXController::ViewRecordAbandon(
  const QString &vuri, 
  const RecStatItem &rec_stat)
{
  ViewRecordPointer view = FindViewNode(vuri, rec_stat.close.job_id);
  if (view) {
    view->complete = true;
    qDebug() << "Record [" << view->vuri << "] file abandon";
  }
  //����¼�Ƽ�¼
  RemoveFinishedEpisodeNode(vuri);
}

void TXController::RecordProcessExit(const QString &vuri)
{
  //¼������˳��������зּ���Ӧ��view��Ӧ�ô������ˡ�
  //������һ����û�����߼�©��û�йرյķּ����еĻ���¼��־����ɾ���ն˼�¼������Ӱ���´�¼��
  const QString uri = TerminalHelper::GetTerminalURI(vuri);
  if (!terminal_record_dict_.contains(uri)) return;

  TerminalRecordPointer terminal_record_node = terminal_record_dict_[uri];

  for (auto episode : terminal_record_node->episodes) {
    for (auto view : episode->views) {
      if (view->vuri == vuri && !view->complete) {
        qWarning() << "Record [" << vuri << "] not closed properly. Episode id=" 
          << episode->start_time;
        view->complete = true;
      }
    }
  }
  RemoveFinishedEpisodeNode(vuri);

  if (terminal_record_node->episodes.empty()) {
    terminal_record_dict_.remove(uri);
  }

  if (terminal_record_dict_.empty()) {
    emit NotifyAllRecordJobFinishSignal();
  }
}

TXController::TerminalRecordPointer TXController::AddTerminalNode( 
  const QString uri_or_vuri )
{
  QString uri = TerminalHelper::GetTerminalURI(uri_or_vuri);
  TerminalPointer terminal = terminal_manager_->FindTerminal(uri);
  if (!terminal) return nullptr;

  //�Ѵ�����nullptr
  if (terminal_record_dict_.contains(uri)) 
    return nullptr;

  TerminalRecordPointer terminal_record_node(new TerminalRecordNode);
  terminal_record_node->uri = uri;
  terminal_record_node->name = 
    terminal->name.left(terminal->name.lastIndexOf("(")); //ȥ��ĩβ�ı��
  terminal_record_node->next_job_id = 0;
  //terminal_record_node->report_port = 0;

  AddEpisodeNode(terminal_record_node); //��ӵ�һ���ּ���¼

  //��Ҫ�������view��¼��
  //��Ϊ������Զ����յ��õ��������ʱ�ն��б����û�����꣬������view_nodeҲ�Ͳ�������
  //���Ի���Ҫ��ʵ���յ�¼�Ʊ���ʱ�����view��¼����HandleReceiveUDPRecReportSlot

  terminal_record_dict_.insert(uri, terminal_record_node);
  return terminal_record_node;
}

TXController::EpisodeSysInfoPointer TXController::AddEpisodeNode(
  TerminalRecordPointer terminal)
{
  if (!terminal) {
    return nullptr;
  }
  EpisodeSysInfoPointer episode(new EpisodeSysInfo);
  episode->job_id = terminal->next_job_id++;
  episode->start_time = GetNonrepeatableTimestamp(); //΢�뼶���ظ���ʱ���
  episode->segmented = false;

  //������ǵ�һ��������֮ǰ�ļ�¼����view_node
  if (terminal->episodes.size()) {
    for (ViewRecordPointer view : terminal->episodes[0]->views) {
      AddViewNode(episode, view->vuri);
    }
  }

  terminal->episodes.push_back(episode);
  return episode;
}

TXController::ViewRecordPointer TXController::AddViewNode( 
  EpisodeSysInfoPointer episode, 
  const QString &vuri )
{
  if (!TerminalHelper::IsVirtualURI(vuri)) return nullptr;

  //��Ӧ��view_node�Ѵ��ڣ���ʧ��
  for (ViewRecordPointer view : episode->views) {
    if (view->vuri == vuri) {
      return view;
    }
  }

  ViewRecordPointer view(new ViewRecordNode);
  view->vuri = vuri;
  view->start = false;
  view->complete = false;
  memset(&view->record_status, 0, sizeof(RecStatItem));
  episode->views.push_back(view);

  return view;
}

TXController::EpisodeSysInfoPointer TXController::FindEpisodeNode( 
  const QString &uri_or_vuri, int job_id)
{
  QString uri = TerminalHelper::GetTerminalURI(uri_or_vuri);
  if (!terminal_record_dict_.contains(uri)) return nullptr;

  TerminalRecordPointer terminal_record_node = terminal_record_dict_[uri];
  auto it = std::find_if(
    terminal_record_node->episodes.begin(),
    terminal_record_node->episodes.end(),
    [&job_id](const EpisodeSysInfoPointer &node) {
      return node->job_id == job_id;
    });

  if (it == terminal_record_node->episodes.end()) return nullptr;
  return (*it);
}

TXController::ViewRecordPointer TXController::FindViewNode( 
  const QString &vuri, int job_id)
{
  EpisodeSysInfoPointer episode = FindEpisodeNode(vuri, job_id);
  if (!episode) return nullptr;

  auto it = std::find_if(
    episode->views.begin(),
    episode->views.end(),
    [&vuri](const ViewRecordPointer &node) {
      return node->vuri == vuri;
  });

  if (it == episode->views.end()) return nullptr;
  return (*it);
}

void TXController::HandleAutoRecordTimerTimeoutSlot()
{
  while (!auto_record_pending_list_.empty()) {
    AutoStartPendingInfo pending = auto_record_pending_list_.front();
    if (time(nullptr) - pending.online_time < auto_record_delay_) {
      //��һ���������ĵȴ�ʱ�仹δ���������϶�Ҳû��
      return;
    }

    auto_record_pending_list_.pop_front();

    //���������Զ�¼��
    StartRecordParam param;
    param.target_uri = pending.uri;
    param.requester_uri = QString::fromLocal8Bit("�Զ�");
    TerminalRecordPointer terminal_record_node = StartRecord(param, false);
    if (!terminal_record_node) {
      //����ʧ�������������Զ�����ǰ���ֶ������ˣ������ն�������
      continue;
    }
  }
}

void TXController::RemoveFinishedEpisodeNode( const QString &uri_or_vuri )
{
  const QString uri = TerminalHelper::GetTerminalURI(uri_or_vuri);
  if (!terminal_record_dict_.contains(uri)) return;

  TerminalRecordPointer terminal_record_node = terminal_record_dict_[uri];
  int max_deleted_job_id = 0;

  for (auto epi_it = terminal_record_node->episodes.begin();
       epi_it != terminal_record_node->episodes.end();) {
    auto cur_epi_it = epi_it;
    bool should_delete = true;
    //����δ���¼�Ƶ��ӽ�
    for (auto view : (*cur_epi_it)->views) {
      if (!view->complete) {
        should_delete = false;
        break;
      }
    }
    if (should_delete) {
      //�����ӽǶ�����ˣ���÷ּ���Ϣ����ɾ����
      max_deleted_job_id = (*cur_epi_it)->job_id;
      epi_it = terminal_record_node->episodes.erase(cur_epi_it);
    } else {
      ++epi_it;
    }
  }

  //������û��֮ǰ����ĳ��δ֪ԭ��û������ɾ����episode��¼
  //�ײ�MP4Mux��֤�˹ر�˳��Ӿɵ��£�����ĳjob_id�ر��ˣ����С��job_id�϶���Ч�ˡ�
  //ע��Ŷ����Ȼǿ��ɾ�����ͷ����ڴ棬��û�д����������ݿ����
  for (auto epi_it = terminal_record_node->episodes.begin();
       epi_it != terminal_record_node->episodes.end();) {
    auto cur_epi_it = epi_it;
    if ((*cur_epi_it)->job_id < max_deleted_job_id) {
      epi_it = terminal_record_node->episodes.erase(cur_epi_it);
      //����Ժ�������־ģ�飬�Ǹ���־
    } else {
      ++epi_it;
    }
  }
}

bool TXController::HasRecording()
{
  return !terminal_record_dict_.empty();
}
