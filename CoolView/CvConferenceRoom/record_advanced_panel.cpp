#include "record_advanced_panel.h"

#include <windows.h>
#include <shellapi.h>

#include "util/ProcessManager.h"
#include "log/Log.h"

RecordAdvancedPanel::RecordAdvancedPanel(QWidget *parent)
    : QFrame(parent)
{
  ui.setupUi(this);
  connect(ui.resetButton, &QPushButton::clicked,
    this, &RecordAdvancedPanel::ResetInfo);
  ResetInfo();

  //QT�ٷ��ĵ�˵������WA_InputMethodEnabled���������QApplication.setAutoSipEnabled(true);
  //�Ϳ����ڵ�������ʱ���ô����豸��Ļ���̡����ˣ�ò�����ֻ���ƶ��ն���Ч�����϶�˵��Windows��Ч��
  //ui.titleLineEdit->setAttribute(Qt::WA_InputMethodEnabled, true);

  //���ڵĽ�������ǣ�������QT�豸��Touch�¼���Ȼ��ͨ���¼�����������Touch�¼���Ȼ���ֶ�������Ļ���̳���
  ui.titleLineEdit->setAttribute(Qt::WA_AcceptTouchEvents);
  ui.titleLineEdit->installEventFilter(this);
  ui.keywordsLineEdit->setAttribute(Qt::WA_AcceptTouchEvents);
  ui.keywordsLineEdit->installEventFilter(this);
  ui.participantsLineEdit->setAttribute(Qt::WA_AcceptTouchEvents);
  ui.participantsLineEdit->installEventFilter(this);
  ui.descTextEdit->setAttribute(Qt::WA_AcceptTouchEvents);
  ui.descTextEdit->installEventFilter(this);
}

RecordAdvancedPanel::~RecordAdvancedPanel()
{

}

void RecordAdvancedPanel::ResetInfo()
{
  ui.titleLineEdit->setText("");
  ui.keywordsLineEdit->setText("");
  ui.participantsLineEdit->setText("");
  ui.descTextEdit->setPlainText("");
}

void RecordAdvancedPanel::GetRecordInfo( StartRecordParam &param )
{
  param.title = ui.titleLineEdit->text();
  param.keywords = ui.keywordsLineEdit->text();
  param.participants = ui.participantsLineEdit->text();
  param.description = ui.descTextEdit->toPlainText();
}

bool RecordAdvancedPanel::eventFilter(QObject *watched, QEvent *event)
{
  /*if (QEvent::MouseMove != event->type()) {
  _snprintf(__global_msg , sizeof(__global_msg) , "Event %d", event->type());
  CONFROOM_LOG_INFO(__global_msg);
  }*/
  switch(event->type())
  {
  case QEvent::TouchBegin:
  case QEvent::TouchUpdate:
  case QEvent::TouchEnd:
  case QEvent::RequestSoftwareInputPanel:
    //Ƕ��ʽϵͳ��QProcessû�á���
    /*QProcess *process = new QProcess(this);
    process->start("osk", QStringList());*/

    //����Ѿ������ˣ���Ҫ����������Ȼ���̳���һֱ�����㣬û������
    if (!ProcessManager::isRunning(TEXT("osk.exe"))) {
      ShellExecute(NULL , TEXT("open"), TEXT("osk.exe"), NULL, NULL,  SW_SHOW);
    }
    //CONFROOM_LOG_INFO("osk started");
    break;
  };
  return QObject::eventFilter(watched, event);
}
