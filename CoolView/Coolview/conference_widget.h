#ifndef CONFERENCE_WIDGET_H
#define CONFERENCE_WIDGET_H

#include <QWidget>
#include "ui_conference_list_widget.h"

#include "conference_controller_interface.h"
#include "session_controller_interface.h"

class IControllerManager;

class ConferenceWidget : public QWidget {
  Q_OBJECT
public:
  ConferenceWidget(QWidget *parent = 0);
  ~ConferenceWidget();

  void Initialize(IControllerManager *controller_manager);

Q_SIGNALS:
  // ��conference controller���͵��ź�
  void JoinConferenceSignal(const QString &conference_uri);
  void LeaveConferenceSignal();
  void RequestConferenceListSignal();

  // ���������ڵı���
  void SetWindowTitleSignal(const QString &title);

public Q_SLOTS:
  void HandleSessionStateChangedNotifySlot(
    ISessionController::SessionState state);

  void HandleConferenceListReceivedNotifySlot(
    const ConstConferenceList &conferences);

  void HandleConferenceStateChangedNotifySlot(
    const QString &conference_uri,
    IConferenceController::ConferenceState state);

  void HandleJoinConferenceErrorNotifySlot(
    const QString &conference_uri, 
    IConferenceController::JoinConferenceError error);

private Q_SLOTS:
  void TryToJoinConferenceSlot();
  void ShowConferenceTipSlot(int index);
  void ShowHelpWidgetSlot();
  void UpdateConferenceListSlot();

private:
  void HandleJoinConferenceNotify(const QString &conference_uri);
  void HandleLeaveConferenceNotify();

  void SetupGui();
  void SetConferenceTip(ConstConferencePointer conference);

  // ������ת��Ϊxx��xxʱxx��xx���ʽ���ַ���
  static QString ConvertDurationMinToQString(int mins);

private:
  Ui::ConferenceListWidget ui_;

  // �洢�����б�
  // key: uri, value: conference
  ConstConferenceDictionary conference_dict_;
  
  // ��¼��ǰ���ڽ��еĻ���
  QString current_conference_uri_;
  // ��¼�뿪���������Ҫ����Ļ���
  QString attempt_conference_uri_;

  QWidget *help_widget_;
  
  bool has_initialized_;
};

#endif // CONFERENCE_WIDGET_H
