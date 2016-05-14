#include "record_db_controller.h"

#include "dbus/txMonitor/common/TxMonitorServiceCommon.h"
#include "record_sqlite_db.h"


RecordDBController::RecordDBController()
{
  logger_.reset(CreateLogger());
  logger_->SetModuleName("rec_db");

  db_.reset(new RecordSqliteDB);
}

RecordDBController::~RecordDBController()
{
}

void RecordDBController::HandleRecordComplete( 
  const RecordCompleteParam &param)
{
  LOG_PRINTF_EX(logger_, "insert record for [%s][%d]", 
    qPrintable(param.terminal.uri), param.view.camera_index);

  //�������Ƿ����
  ConferencePointer conference = FindConference(param.conference.cid);
  if (!conference) {
    conference = CreateConference(param);
    if (!conference) {
      return;
    }
  }

  //�ټ���ն��Ƿ����
  TerminalPointer terminal = FindTerminal(conference, param.terminal.uri);
  if (!terminal) {
    terminal = CreateTerminal(param);
    if (!terminal) {
      return;
    }
  }

  //Ȼ����ֶ��Ƿ����
  EpisodePointer episode = FindEpisode(terminal, param.episode.id);
  if (!episode) {
    episode = CreateEpisode(terminal, param);
    if (!episode) {
      return;
    }
  }

  //��������Ƶ��¼
  if (CreateView(episode, param) != 0) {
    //�������
  }
}

RecordDBController::ConferencePointer RecordDBController::FindConference(
  int cid)
{
  if (db_cache_.contains(cid)) {
    //��������
    return db_cache_[cid];
  }

  //���������ݿ����
  IRecordDB::ConferenceEntityPointer conference_entity = 
    db_->SelectConference(cid);
  if (!conference_entity) {
    return nullptr;
  }

  //�ҵ�����뻺�沢����
  ConferencePointer conference = CacheConference(conference_entity);

  return conference;
}

RecordDBController::ConferencePointer RecordDBController::CreateConference( 
  const RecordCompleteParam &param)
{
  //����ظ���
  ConferencePointer conference = FindConference(param.conference.cid);
  if (conference) {
    LOG_ERROR_EX(logger_, "insert conference exist, cid=%d", 
      param.conference.cid);
    return conference;
  }

  //����ʵ��
  IRecordDB::ConferenceEntityPointer conference_entity(
    new IRecordDB::ConferenceEntity);
  conference_entity->cid = param.conference.cid;
  conference_entity->title = param.conference.title;
  conference_entity->start_time = param.conference.start_time;
  conference_entity->description = param.conference.description;

  IRecordDB::TerminalEntityPointer terminal_entity(
    new IRecordDB::TerminalEntity);
  terminal_entity->uri = param.terminal.uri;
  terminal_entity->name = param.terminal.name;

  conference_entity->terminal_list.insert(
    terminal_entity->uri, terminal_entity);

  //�������ݿ�
  if (db_->InsertConference(conference_entity) != 0) {
    LOG_ERROR_EX(logger_, "insert conference entity failed, cid=%d", 
      conference_entity->cid);
    return nullptr;
  }

  //д����
  conference = CacheConference(conference_entity);
  return conference;
}

RecordDBController::ConferencePointer RecordDBController::CacheConference( 
  const IRecordDB::ConferenceEntityPointer conference_entity)
{
  if (!conference_entity) return nullptr;

  ConferencePointer conference(new ConferenceNode);
  conference->cid = conference_entity->cid;
  for (auto &terminal_entity : conference_entity->terminal_list) {
    TerminalPointer terminal(new TerminalNode);
    terminal->uri = terminal_entity->uri;
    conference->terminals.insert(terminal->uri, terminal);
  }

  db_cache_.insert(conference->cid, conference); //���ԭ�����ڣ��Ḳ�Ǿɵ�ֵ
  return conference;
}

RecordDBController::TerminalPointer RecordDBController::FindTerminal( 
  ConferencePointer conference, const QString &uri )
{
  if (conference->terminals.contains(uri)) {
    //��������
    return conference->terminals[uri];
  }

  //terminal��Conference�е��ֶΣ�����û�о���û���ˣ�
  //���ǻ���û���£���ֻ������д���ݿ⣬�ʻ��������ݿ��Ǳ���һ�µ�
  return nullptr;
}

RecordDBController::TerminalPointer RecordDBController::CreateTerminal( 
  const RecordCompleteParam &param )
{
  //��ȡ����ʵ��
  IRecordDB::ConferenceEntityPointer conference_entity = 
    db_->SelectConference(param.conference.cid);
  if (!conference_entity) {
    LOG_ERROR_EX(logger_, "insert terminal failed, conference not exist");
    return nullptr;
  }

  //�����ն�ʵ��
  IRecordDB::TerminalEntityPointer terminal_entity(new IRecordDB::TerminalEntity);
  terminal_entity->uri = param.terminal.uri;
  terminal_entity->name = param.terminal.name;

  conference_entity->terminal_list.insert(terminal_entity->uri, terminal_entity);

  //���»���ʵ��
  if (db_->UpdateConference(conference_entity) != 0) {
    LOG_ERROR_EX(logger_, "update conference failed");
    return nullptr;
  }

  //���»���
  //ע�ⲻ��ֱ�ӽ�����ʵ��ת�ɻ��渲��ԭ���ģ�ֻ�ܽ��ն˲����ֵ䣬��Ȼ���ƻ����еĻ���״̬��
  ConferencePointer conference = FindConference(conference_entity->cid);
  TerminalPointer terminal = CacheTerminal(conference, terminal_entity);

  //����
  return terminal;
}

RecordDBController::TerminalPointer RecordDBController::CacheTerminal( 
  ConferencePointer conference, 
  const IRecordDB::TerminalEntityPointer terminal_entity)
{
  if (!conference) {
    return nullptr;
  }

  TerminalPointer terminal(new TerminalNode);
  terminal->uri = terminal_entity->uri;

  conference->terminals.insert(terminal->uri, terminal); //���ԭ�����ڣ��Ḳ�Ǿɵ�ֵ

  return terminal;
}

RecordDBController::EpisodePointer RecordDBController::FindEpisode( 
  TerminalPointer terminal, 
  long long id )
{
  if (!terminal) {
    return nullptr;
  }

  EpisodeList::iterator episode_it = std::find_if(
    terminal->episodes.begin(),
    terminal->episodes.end(), 
    [&id](const EpisodePointer &e){
      return e->id == id;
    });
  if (episode_it != terminal->episodes.end()) {
    //��������
    return (*episode_it);
  }

  //���������ݿ����
  IRecordDB::EpisodeEntityPointer episode_entity = 
    db_->SelectEpisode(id);
  if (!episode_entity) {
    return nullptr;
  }

  //�ҵ�����뻺�沢����
  EpisodePointer episode = CacheEpisode(terminal, episode_entity);

  return episode;
}

RecordDBController::EpisodePointer RecordDBController::CreateEpisode( 
  TerminalPointer terminal, 
  const RecordCompleteParam &param )
{
  //����ظ���
  EpisodePointer episode = FindEpisode(terminal, param.episode.id);
  if (episode) {
    LOG_ERROR_EX(logger_, "insert episode exist, id=%d", 
      param.episode.id);
    return episode;
  }

  //����ʵ��
  IRecordDB::EpisodeEntityPointer episode_entity(
    new IRecordDB::EpisodeEntity);
  episode_entity->id = param.episode.id;
  episode_entity->cid = param.conference.cid;
  episode_entity->terminal_uri = param.terminal.uri;
  episode_entity->requester = param.episode.requester;
  episode_entity->title = param.episode.title;
  episode_entity->start_time = param.view.start_time/1000; //��view��ʱ��Ϊ׼��ע��view��ʱ�侫����ms
  episode_entity->duration = 0; //����viewʱ�ټ���
  episode_entity->participants = param.episode.participants;
  episode_entity->keywords = param.episode.keywords;
  episode_entity->description = param.episode.description;

  //�������ݿ�
  if (db_->InsertEpisode(episode_entity) != 0) {
    LOG_ERROR_EX(logger_, "insert episode entity failed, id=%d", 
      episode_entity->id);
    return nullptr;
  }

  //д����
  episode = CacheEpisode(terminal, episode_entity);
  return episode;
}

RecordDBController::EpisodePointer RecordDBController::CacheEpisode( 
  TerminalPointer terminal, 
  const IRecordDB::EpisodeEntityPointer episode_entity)
{
  if (!episode_entity) return nullptr;

  EpisodeList::iterator episode_it = std::find_if(
    terminal->episodes.begin(),
    terminal->episodes.end(), 
    [&episode_entity](const EpisodePointer &e){
      return e->id == episode_entity->id;
  });
  if (episode_it != terminal->episodes.end() ||
      terminal->episodes.size() > 3) { 
    //��������ظ�Ҫ�󻺴�ĳ����¼�������֮ǰ�����м�¼�����²���
    //���߻��泬��3����Ҳ���������(3���Ǹ��ݵײ�ּ�ʱ�����Թ��Ƶģ�������С�����ֵ)
    terminal->episodes.clear();
  }

  EpisodePointer episode(new EpisodeNode);
  episode->id = episode_entity->id;

  terminal->episodes.push_back(episode);
  return episode;
}

int RecordDBController::CreateView( 
  EpisodePointer episode, 
  const RecordCompleteParam &param )
{
  //view����֤�ظ��ԣ�Ĭ��ÿ�������Ķ����¼�¼

  //����ʵ��
  IRecordDB::ViewEntityPointer view_entity(new IRecordDB::ViewEntity);
  view_entity->vid = 0; // ����ʱ���Ը�ֵ��
  view_entity->episode_id = episode->id;
  view_entity->camera_index = param.view.camera_index;
  view_entity->start_time = param.view.start_time;
  view_entity->duration = param.view.duration;
  view_entity->media_format = param.view.media_format;
  view_entity->file = param.view.file;

  //�������ݿ�
  if (db_->InsertView(view_entity) != 0) {
    LOG_ERROR_EX(logger_, "insert view entity failed, episode_id=%d, file=%s", 
      episode->id, qPrintable(view_entity->file));
    return -1;
  }

  //���¼���episode��duration
  IRecordDB::EpisodeEntityPointer episode_entity = 
    db_->SelectEpisode(episode->id);
  if (episode_entity) {
    bool need_update = false;
    if (episode_entity->start_time > view_entity->start_time/1000) {
      //�ø����view��ʼʱ�����ԭʱ�䣬ע��view��ʱ�侫����ms��episode��s
      episode_entity->start_time = view_entity->start_time/1000;
      need_update = true;
    }
    //ע��view��ʱ�侫����ms��episode��s
    long long dur = 
      (view_entity->start_time + view_entity->duration)/1000 - episode_entity->start_time;
    if (episode_entity->duration < dur) {
      episode_entity->duration = dur;
      need_update = true;
    }
    if (need_update) db_->UpdateEpisode(episode_entity);
  }

  return 0;
}
