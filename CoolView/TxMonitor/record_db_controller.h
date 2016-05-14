#ifndef RECORD_DB_CONTROLLER_H
#define RECORD_DB_CONTROLLER_H

#include <QtCore>
#include <memory>

#include "record_db_interface.h"
#include "Log.h"

class RecordCompleteParam;

class RecordDBController
{
public:
  RecordDBController();
  ~RecordDBController();

  void HandleRecordComplete(const RecordCompleteParam &param);

protected:
  //���ڻ������������
  struct EpisodeNode {
    long long id;
  };
  typedef std::shared_ptr<EpisodeNode> EpisodePointer;
  typedef QVector<EpisodePointer> EpisodeList;

  struct TerminalNode {
    QString uri;
    EpisodeList episodes;
  };
  typedef std::shared_ptr<TerminalNode> TerminalPointer;
  typedef QHash<QString, TerminalPointer> TerminalDict;

  struct ConferenceNode {
    int cid;
    TerminalDict terminals;
  };
  typedef std::shared_ptr<ConferenceNode> ConferencePointer;
  typedef QHash<int, ConferencePointer> ConferenceDict;

  //�������ݿ���ط���
  ConferencePointer CreateConference(const RecordCompleteParam &param);
  ConferencePointer FindConference(int cid);
  ConferencePointer CacheConference(const IRecordDB::ConferenceEntityPointer);

  TerminalPointer CreateTerminal(const RecordCompleteParam &param);
  TerminalPointer FindTerminal(ConferencePointer conference, const QString &uri);
  TerminalPointer CacheTerminal(ConferencePointer conference, 
    const IRecordDB::TerminalEntityPointer);

  EpisodePointer CreateEpisode(TerminalPointer terminal, const RecordCompleteParam &param);
  EpisodePointer FindEpisode(TerminalPointer terminal, long long id);
  EpisodePointer CacheEpisode(TerminalPointer terminal, const IRecordDB::EpisodeEntityPointer);

  int CreateView(EpisodePointer episode, const RecordCompleteParam &param);

private:
  //ʹ��cache�������ݿ�Ĳ�ѯ��cache�е�����Ӧ�����ܵ���
  ConferenceDict db_cache_;
  std::shared_ptr<IRecordDB> db_;

  std::shared_ptr<ILogger> logger_;
};

#endif
