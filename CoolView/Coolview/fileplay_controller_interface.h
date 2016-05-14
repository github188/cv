#ifndef FILEPLAY_INTERFACE_H
#define FILEPLAY_INTERFACE_H

#include <QObject>

#include "conference_controller_interface.h"
#include "terminal_description.h"
#include "util/report/FilePlayStat.h"

class IFilePlayController : public QObject {
  Q_OBJECT

 public:
  IFilePlayController(QObject *parent) : QObject(parent) {}
  virtual ~IFilePlayController() {}

  static void RegisterMetaType() {
    qRegisterMetaType<FilePlayStatItem>("FilePlayStatItem");
  }

 Q_SIGNALS:
  //֪ͨ���Ž���,��λ��
  void NotifyPlayProgressSignal(unsigned long pos, unsigned long duration);

  //֪ͨ�ļ������쳣
  void NotifyFileErrorSignal(const QString &msg);

  //��������ն��б��źţ�������Ӳ����б��Ӧ�������ն�
  void NotifyVirtualRecvTerminalListSignal(const TerminalList&);

 public Q_SLOTS:
  //���һ�������б�һ���ļ���Ӧ����һ���ն�Ŷ
  virtual void HandleAddPlayListSlot(const QStringList&) = 0;

  //������в����б�
  virtual void HandleClearPlayListsSlot() = 0;

  //���ſ���
  virtual void HandlePlaySlot() = 0;
  virtual void HandlePauseSlot() = 0;
  virtual void HandleStopSlot() = 0;
  virtual void HandleSeekSlot(unsigned long sec) = 0;

  //�������״̬���
  virtual void HandleConferenceStateChangedSlot(
    const QString &conference_uri, 
    IConferenceController::ConferenceState state) = 0;

  //������״̬UDP����
  virtual void HandleReceiveUDPFilePlayReportSlot(const FilePlayStatItem &) = 0;

};

#endif // end FILEPLAY_INTERFACE_H
