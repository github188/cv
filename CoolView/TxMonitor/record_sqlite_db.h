#ifndef RECORD_SQLITE_DB_H
#define RECORD_SQLITE_DB_H

#include "record_db_interface.h"

#include <memory>
#include <functional>

#include <QtSql\QSqlDatabase>
#include <QtSql\QtSql>

#include "Log.h"

//����QT SQLģ�ͣ�QSqlDatabase�����̰߳�ȫ�ģ�
//�������߳���ص�(thread affinity)��ֻ���ڴ����ö�����߳��е��ã�
//��ˣ�Ϊ�˼���ܼ������ݿ���������InitialDB()����ݵ�ǰ�߳�ID�����Ƿ񴴽��µ�
//QSqlDatabaseʵ�������������ȷ����������ʱ����ö���ʱ����ͬһ���߳��У�
class RecordSqliteDB : public IRecordDB
{
  Q_OBJECT

public:
  RecordSqliteDB(QObject *parent = nullptr);
  ~RecordSqliteDB();

  int InsertConference(const ConferenceEntityPointer &c) override;
  int UpdateConference(const ConferenceEntityPointer &c) override;
  ConferenceEntityPointer SelectConference(int cid) override;
  int SelectConferenceCount(int &count) override;
  int SelectConferences(int offset, int limit, 
    ConferenceEntityList &list) override;

  int SelectTerminal(int cid, TerminalEntityList &list) override;

  int InsertEpisode(const EpisodeEntityPointer &e) override;
  int UpdateEpisode(const EpisodeEntityPointer &e) override;
  EpisodeEntityPointer SelectEpisode(long long id) override;
  int SelectEpisodeCount(int cid, const QString &uri, int &count) override;
  int SelectEpisodes(int cid, const QString &uri,
    int offset, int limit, EpisodeEntityList &list) override;
  int SelectEpisodeDate(int cid, const QString &uri,
    EpisodeDateList &dates) override;
  int SelectEpisodesByInterval(int cid, const QString &uri,
    long long start, long long end, EpisodeEntityList &list) override;

  int InsertView(const ViewEntityPointer &v) override;
  ViewEntityPointer SelectViewByFilePath(const QString &path) override;
  int SelectViewsByEpisode(long long eid, ViewEntityList &list) override;
  int UpdateView(const ViewEntityPointer &v) override;
  int UpdateViewSetFlagsByPath(const QString &path, int flags) override;
  int UpdateViewResetFlagsByPath(const QString &path, int flags) override;

private:
  void InitialDB();
  void CreateConferenceTable();
  void CreateEpisodeTable();
  void CreateViewTable();

  QString TerminalDictToJson(const TerminalDict &dict);
  void TerminalContainerFromJson(const QString &json, 
    std::function<void (TerminalEntityPointer)>);

  void HandleQueryError(const QString &msg);

private:
  QSqlDatabase db_;
  QSqlQuery query_;

  std::shared_ptr<ILogger> logger_;
};

#endif
