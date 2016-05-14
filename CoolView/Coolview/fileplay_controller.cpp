#include "fileplay_controller.h"

#include "util\MediaManager.h"
#include "msg_manager_interface.h"
#include "vconf_sip_msg_receiver.h"
#include "conference_controller_interface.h"
#include "util/ini/CVIniConfig.h"
#include "util/ini/TxConfig.h"
#include "dbus/channelDispatcher/client/ChannelDispatcherIf.h"
#include "dbus/channelDispatcher/common/ChannelDispatcherServiceCommon.h"
#include "dbus/channel/type/streamedMedia/common/StreamedMediaServiceCommon.h"

#include <QApplication>
#include <QFileDialog>
#include <time.h>

const unsigned short base_port = 20000;
const QString sync_id_prefix = "vod_sync_";
const QString fileplay_channel_type = QString(STREAMED_MEDIA_SERVICE_NAME);
const QString media_info_file_ext = ".minf";

//
// Util functions
//
void GenerateRandomSyncId( char *buf, unsigned long len )
{
    const char ch_set[] = "0123456789abcdef";
    const unsigned long ch_set_len = strlen(ch_set);
    const unsigned long rand_len = len;
    srand((unsigned int)time(NULL));
    for (unsigned long i = 0; i < rand_len; ++i) {
        buf[i] = ch_set[rand() % ch_set_len];
    }
    buf[len] = '\0'; // ensure space for tail '\0'
}

//
//members
//
FilePlayController::FilePlayController(QObject *parent /*= nullptr*/)
  : IFilePlayController(parent)
{
  conference_controller_ = nullptr;
  msg_receiver_ = nullptr;
  media_manager_ = nullptr;
  channel_dispatcher_ = nullptr;
  op_id_ = 1;
  total_duration_ = 0;
  current_play_position_ = 0;
  last_seek_op_time_ = 0;
}

FilePlayController::~FilePlayController()
{

}

void FilePlayController::Initialize( 
  IConferenceController *conference_controller,
  MediaManager *media_manager, 
  IMsgManager *msg_manager)
{
  assert(conference_controller != nullptr);
  conference_controller_ = conference_controller;
  connect(conference_controller_, &IConferenceController::NotifyConferenceStateChangedSignal,
    this, &FilePlayController::HandleConferenceStateChangedSlot, Qt::QueuedConnection);

  assert(msg_manager != nullptr);
  IMsgReceiver *receiver = msg_manager->GetMsgReceiver();
  msg_receiver_ = dynamic_cast<VConfSipMsgReceiver*>(receiver);
  if (msg_receiver_) {
    connect(this, &FilePlayController::NotifyVirtualRecvTerminalListSignal,
      msg_receiver_, &VConfSipMsgReceiver::HandleVRecvTerminalListSlot, Qt::QueuedConnection);
  }

  assert(media_manager != nullptr);
  media_manager_ = media_manager;

  channel_dispatcher_ = new ChannelDispatcherIf(
    CHANNEL_DISPATCHER_SERVICE_NAME, 
    CHANNEL_DISPATCHER_OBJECT_PATH,
    QDBusConnection::sessionBus());
}

void FilePlayController::HandleAddPlayListSlot(const QStringList &playlist)
{
  // generate sync_id
  char sync_id_suffix[6];
  GenerateRandomSyncId(sync_id_suffix, sizeof(sync_id_suffix)-1);
  sync_id_ = sync_id_prefix + sync_id_suffix;

  // reset variables
  op_id_ = 1;
  earliest_start_time_ = -1;
  total_duration_ = 0;
  current_play_position_ = 0;
  last_seek_op_time_ = 0;

  //������ͬһ���ն���Ƶ��Ϊһ���ն��µ������ն�Ŀǰ���ز��ŵĽ���취�ǣ�
  //(1)Լ���ļ������»���_�ָͬ����;(2)��һ����Ϊ�ն���;(3)virtual_indexΪ_1_����ʽ;
  //(4)Լ��ʱ���Ϊts��ʼ�Ĳ���

  //��ȷ���м��������նˣ�ÿ���ն��м�·��Ƶ��ÿ�ն���಻����4·
  QHash<QString, QMap<int, QString>> terminal_to_file_table;
  for (unsigned int i = 0; i < playlist.size(); ++i)
  {
    //�����ļ���
    QString file_path = playlist[i];
    file_path.replace("/", "\\");
    //��ȡ�ļ���
    int name_begin_pos = file_path.lastIndexOf("\\") + 1;
    int name_end_pos = file_path.lastIndexOf(".");
    { 
      //�ļ����к����»��߷ָ�֣�����ȡ�õ�һ������Ϊ�����ն�����
      int d = file_path.indexOf("_", name_begin_pos);
      if (d != -1 && d < name_end_pos) name_end_pos = d;
    }
    QString name = file_path.mid(name_begin_pos, name_end_pos - name_begin_pos);
    //��ȡvirtual_index���������1-4����Ϊ0��������ظ����򱣴�ʱ������
    int virtual_index = 0;
    if (file_path.indexOf("_1_", name_end_pos) != -1) virtual_index = 0;
    else if (file_path.indexOf("_2_", name_end_pos) != -1) virtual_index = 1;
    else if (file_path.indexOf("_3_", name_end_pos) != -1) virtual_index = 2;
    else if (file_path.indexOf("_4_", name_end_pos) != -1) virtual_index = 3;
    //������Ϣ
    auto &index_to_file_map = terminal_to_file_table[name];
    if (!index_to_file_map.contains(virtual_index)) {
      index_to_file_map[virtual_index] = file_path;
    }
  }

  int port_count = 0;
  int terminal_count = 0;
  //�ȴ������ļ������ҳ��ļ�����Ŀ�ʼʱ��
  for (auto m = terminal_to_file_table.begin(); 
       m != terminal_to_file_table.end(); ++m)
  {
    for (auto f = m.value().begin(); f != m.value().end(); ++f) 
    {
      VirtualSenderInfo sender_info;
      sender_info.uri = QString("vod") + QString::number(terminal_count) + "@local";
      sender_info.virtual_count = m->count();
      sender_info.virtual_index = f.key(); //��ǰ������ÿ���ն�ֻ��һ������ͷģ��
      sender_info.name = QString("%1(%2)").arg(m.key()).arg(sender_info.virtual_index+1);
      sender_info.media_id = getFilePlayMediaID(sender_info.virtual_index, 
        TerminalHelper::GetUsername(sender_info.uri));

      sender_info.file = *f;
      sender_info.video_port = base_port + port_count * 4; // ÿ��RTP��RTCPռ��2���˿ڣ�����Ƶռ��4���˿�
      sender_info.audio_port = sender_info.video_port + 2;
      sender_info.start_time = GetFileStartTime(sender_info.file);
      sender_info.duration = 0;
      virtual_sender_dict_[sender_info.media_id] = sender_info;

      if (earliest_start_time_ == -1 || earliest_start_time_ > sender_info.start_time) {
        earliest_start_time_ = sender_info.start_time;
      }
      port_count += 4;
    }
    ++terminal_count;
  }

  //Ȼ�����ÿ����Ƶ�Ĳ����ӳ٣�����ý�������̣�
  //�ȴ��䱨��ý���ʽ���ٴ��������նˣ�������ʱ������MediaFormatInfoReceived
  for (auto &sender_info : virtual_sender_dict_)
  {
    sender_info.start_delay = sender_info.start_time - earliest_start_time_;

    FilePlayInfo info;
    info.vuser_id = sender_info.media_id;
    info.vuser_name = sender_info.name;
    info.sync_id = sync_id_;
    info.initial_delay = sender_info.start_delay;
    info.file_list.push_back(sender_info.file); //��Vector��Ϊ�˷����Ժ����������ŵ��ļ��б�
    info.net_info.ip_addr = "127.0.0.1";
    info.net_info.video_port = sender_info.video_port;
    info.net_info.audio_port = sender_info.audio_port;

    QByteArray bytes;
    QDataStream out( &bytes , QIODevice::WriteOnly );
    out.setVersion( QDataStream::Qt_4_4 );
    out << info;
    channel_dispatcher_->CreateChannel(sender_info.media_id, fileplay_channel_type, bytes);
  }
}

void FilePlayController::HandleClearPlayListsSlot()
{
  for (auto sender_info : virtual_sender_dict_) {
    channel_dispatcher_->ReleaseChannel(sender_info.media_id, fileplay_channel_type);
  }
  virtual_sender_dict_.clear();
}

void FilePlayController::HandlePlaySlot()
{
  FilePlayCtrlInfo info;
  info.ctrl_type = FilePlayCtrlInfo::kCtrlResume;
  info.op_id = op_id_;

  QByteArray bytes;
  QDataStream out( &bytes , QIODevice::WriteOnly );
  out.setVersion( QDataStream::Qt_4_4 );
  out << info;

  for (auto sender_info : virtual_sender_dict_) {
    channel_dispatcher_->ModifyChannel(sender_info.media_id, fileplay_channel_type,
      Action_PlayControl, bytes);
  }
  ++op_id_;
}

void FilePlayController::HandlePauseSlot()
{
  FilePlayCtrlInfo info;
  info.ctrl_type = FilePlayCtrlInfo::kCtrlPause;
  info.op_id = op_id_;

  QByteArray bytes;
  QDataStream out( &bytes , QIODevice::WriteOnly );
  out.setVersion( QDataStream::Qt_4_4 );
  out << info;

  for (auto sender_info : virtual_sender_dict_) {
    channel_dispatcher_->ModifyChannel(sender_info.media_id, fileplay_channel_type,
      Action_PlayControl, bytes);
  }
  ++op_id_;
}

void FilePlayController::HandleStopSlot()
{
  
}

void FilePlayController::HandleSeekSlot(unsigned long sec)
{
  FilePlayCtrlInfo info;
  info.ctrl_type = FilePlayCtrlInfo::kCtrlSeek;
  info.op_id = op_id_;

  for (auto sender_info : virtual_sender_dict_) {
    info.seek_time = sec * 1000LL; // sec to ms
    //������ö���Ƶ��ʱ�䡣����seek����ʱ���10s������Ƶ�����ӳ�3s����Ӧseek������Ƶ��7s
    //�����Ǹ�����ͬ���������seek����ʱ���2s����ö���Ƶseek��-1s����ʾӦ�ӳ�1s����
    info.seek_time -= sender_info.start_delay;

    QByteArray bytes;
    QDataStream out( &bytes , QIODevice::WriteOnly );
    out.setVersion( QDataStream::Qt_4_4 );
    out << info;
    channel_dispatcher_->ModifyChannel(sender_info.media_id, fileplay_channel_type,
      Action_PlayControl, bytes);
  }
  ++op_id_;

  last_seek_op_time_ = QDateTime::currentMSecsSinceEpoch();
  current_play_position_ = 0; //���õȴ��ײ���ȱ������
}

void FilePlayController::HandleConferenceStateChangedSlot( 
  const QString &conference_uri, 
  IConferenceController::ConferenceState state)
{
  if (state == IConferenceController::kIsInConference) {
    if (CVIniConfig::getInstance().isVirtualConferenceEnable() &&
      ConferenceHelper::IsVodVirtualConference(conference_uri)) {
        //���ص㲥���飬�����ļ�ѡ�񴰿�
        QString app_dir = QApplication::applicationDirPath();
        QStringList files = QFileDialog::getOpenFileNames(nullptr, 
          QString::fromLocal8Bit("ѡ����Ƶ�ļ�"), 
          app_dir + "\\" + CTxConfig::getInstance().GetDownloadPath(),
          "MPEG-4 (*.mp4)", 0, 
          QFileDialog::HideNameFilterDetails);
        HandleAddPlayListSlot(files);
    }
  }
  else if (state == IConferenceController::kIsNotInConference) {
    //�˳�����ֹͣ���ز�����
    HandleClearPlayListsSlot();
  }
}

void FilePlayController::HandleReceiveUDPFilePlayReportSlot(
  const FilePlayStatItem &item)
{
  if (virtual_sender_dict_.find(item.id) == virtual_sender_dict_.end()) {
    //TODO�����ŵ��ļ������ڣ���¼��־
    return;
  }
  switch (item.type)
  {
  case FILEPLAY_STAT_MEDIAINFO:
    MediaFormatInfoReceived(item);
    break;
  case FILEPLAY_STAT_PROGRESS:
    ProgressInfoReceived(item);
    break;
  default:
    break;
  }
}

void FilePlayController::MediaFormatInfoReceived(const FilePlayStatItem &item)
{
  QString media_id = item.id;
  VirtualSenderInfo &sender_info = virtual_sender_dict_[media_id];

  //����ý���ʽ
  sender_info.duration = item.format.file_duration;
  sender_info.video_codec = item.format.video_codec;
  sender_info.video_width = item.format.video_width;
  sender_info.video_height = item.format.video_height;
  sender_info.video_fps = item.format.video_fps;
  sender_info.audio_codec = item.format.audio_codec;
  sender_info.audio_sample_rate = item.format.audio_sample_rate;
  sender_info.audio_channel = item.format.audio_channel;
  sender_info.audio_bit = item.format.audio_bit_count;
  CreateVirtualTerminal(sender_info);

  long long my_duration = sender_info.start_delay + sender_info.duration;
  if (total_duration_ < my_duration) {
    total_duration_ = my_duration;
    emit NotifyPlayProgressSignal(0, (unsigned long)(total_duration_/1000));
  }
}

void FilePlayController::ProgressInfoReceived(const FilePlayStatItem &item)
{
  qint64 now = QDateTime::currentMSecsSinceEpoch();
  if (now - last_seek_op_time_ < 200) {
    //��λ�������·��󣬿����յ���λǰ�Ľ��ȱ��棬��ʱ��Ӧ�÷��������źţ�
    //��˺��Զ�λ������200ms���ڵĽ��ȱ���
    return;
  }

  QString media_id = item.id;
  VirtualSenderInfo &sender_info = virtual_sender_dict_[media_id];
  unsigned long pos = (unsigned long)(item.progress.current_pos + sender_info.start_delay);
  if (pos > current_play_position_) {
    //ֻ�н������ӲŸ��£���ý���������
    current_play_position_ = pos;
    emit NotifyPlayProgressSignal(current_play_position_/1000, total_duration_/1000);
  }
}

void FilePlayController::CreateVirtualTerminal(const VirtualSenderInfo &sender_info)
{
  //�������ⷢ�Ͷ� 
  TerminalList terminals;
  TerminalPointer terminal(new TerminalDescription);
  terminal->uri = sender_info.uri;
  terminal->virtual_count = sender_info.virtual_count;
  terminal->virtual_index = sender_info.virtual_index;
  terminal->virtual_uri = TerminalHelper::ConstructDefaultVirtualURI(
    terminal->uri, terminal->virtual_index);
  terminal->name = sender_info.name;

  terminal->video_port = sender_info.video_port;
  terminal->audio_port = sender_info.audio_port;
  //����SDP���˴�������ǷǱ�׼�ģ�ֻ����CoolView��Ҫ����С������
  //��ʽע��Ҫ��SipWrapper�й����һ�¡�SipWrapper����Ҫ���úܶ෽�����ܹ���SDP������������ʱ��������
  QString sdp_info = "v=0\n";
  if (!sender_info.audio_codec.isEmpty()) {
    terminal->has_audio = true;
    QString sdp_audio = QString("m=audio RTP/AVP 103\na=ptime:20\na=rtpmap:103 %1/%2/%3\n").
      arg(sender_info.audio_codec).arg(sender_info.audio_sample_rate).arg(sender_info.audio_channel);
    sdp_info += sdp_audio;
  } else {
    terminal->has_audio = false;
  }
  if (!sender_info.video_codec.isEmpty()) {
    terminal->has_video = true;
    QString sdp_video = QString("m=video RTP/AVP 34\na=fmtp:34 CUSTOM=%1,%2 MaxBR=1960 FPS=%3\n"
      "a=rtpmap:34 H263/90000/1\n").
      arg(sender_info.video_width).arg(sender_info.video_height).arg(sender_info.video_fps);
    sdp_info += sdp_video;
  } else {
    terminal->has_video = false;
  }
  terminal->sdp = sdp_info;

  terminal->is_available = true;
  terminal->is_chairman_terminal = false;
  terminal->is_handup = false;
  terminal->is_main_speaker = false;
  terminal->is_speaking = true;

  terminals.push_back(terminal);
  emit NotifyVirtualRecvTerminalListSignal(terminals);
}

long long FilePlayController::GetFileStartTime( const QString &file_path )
{
  long long start_time = 0;
  /*QString info_file = 
      file_path.mid(0, file_path.lastIndexOf('.')) + media_info_file_ext;
  QByteArray info_file_data = info_file.toLocal8Bit();
  const char *file_name_buf = info_file_data.data();

  FILE *f = nullptr;
  f = fopen(file_name_buf, "r");
  if (f) {
    fscanf_s(f, "%I64d", &start_time);
    fclose(f);
  }*/

  QString file_name = file_path;
  int dash_pos = file_name.lastIndexOf("\\");
  if (dash_pos >= 0) {
    file_name = file_name.mid(dash_pos + 1);
  }
  int dot_pos = file_name.lastIndexOf(".");
  if (dot_pos >= 0) {
    file_name = file_name.left(dot_pos);
  }
  QStringList parts = file_name.split("_");
  for (QString &p : parts) {
    if (p.startsWith("ts")) {
      start_time = p.mid(2).toLongLong();
    }
  }  
  return start_time;
}
