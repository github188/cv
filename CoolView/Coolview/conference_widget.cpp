#include "conference_widget.h"

#include <QMessageBox>
#include <QTextStream>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDate>

#include "controller_manager_interface.h"
#include "help-widget.h"

ConferenceWidget::ConferenceWidget(QWidget *parent)
    : QWidget(parent), help_widget_(nullptr) {
  ui_.setupUi(this);
  has_initialized_ = false;
}

ConferenceWidget::~ConferenceWidget() {
  if (help_widget_) {
    help_widget_->close();
    delete help_widget_;
    help_widget_ = nullptr;
  }
}

void ConferenceWidget::Initialize( IControllerManager *controller_manager ) {
  assert(controller_manager != nullptr);

  // ���û��ǳ�ʱ��Ҫ��������б�
  ISessionController *session_controller =
    controller_manager->GetSessionController();
  assert(session_controller != nullptr);
  connect(
    session_controller, &ISessionController::NotifySessionStateChangedSignal,
    this, &ConferenceWidget::HandleSessionStateChangedNotifySlot);

  IConferenceController *conference_controller = 
    controller_manager->GetConferenceController();
  assert(conference_controller != nullptr); 
  
  // ��Ӧ����״̬�ı���ź�
  connect(
    conference_controller,
    SIGNAL(NotifyConferenceStateChangedSignal(
      const QString&, IConferenceController::ConferenceState)),
    this,
    SLOT(HandleConferenceStateChangedNotifySlot(
      const QString&, IConferenceController::ConferenceState)));

  // ��Ӧ������������ź�
  connect(
    conference_controller,
    SIGNAL(NotifyJoinConferenceErrorSignal(
      const QString&, IConferenceController::JoinConferenceError)),
    this,
    SLOT(HandleJoinConferenceErrorNotifySlot(
      const QString&, IConferenceController::JoinConferenceError)));

  // ��Ӧ��ȡ�������б���ź�
  connect(
    conference_controller,
    &IConferenceController::NotifyConferenceListReceivedSignal,
    this,
    &ConferenceWidget::HandleConferenceListReceivedNotifySlot);

  // ���������źźͲ۵Ľ���
  connect(
    this,
    SIGNAL(JoinConferenceSignal(const QString&)),
    conference_controller,
    SLOT(HandleJoinConferenceRequestSlot(const QString&)));

  // �뿪������źźͲ۵Ľ���
  connect(
    this,
    SIGNAL(LeaveConferenceSignal()),
    conference_controller,
    SLOT(HandleLeaveConferenceRequestSlot()));

  connect(
    this, 
    SIGNAL(RequestConferenceListSignal()),
    conference_controller,
    SLOT(HandleConferenceListRequestSlot()));

  SetupGui();

  has_initialized_ = true;
}

void ConferenceWidget::SetupGui() {
  connect(ui_.listWidget, SIGNAL(currentRowChanged(int)),
          this, SLOT(ShowConferenceTipSlot(int)));

  // TODO: ����˫��������鹦�ܣ�
  connect(ui_.enterConferenceAction, SIGNAL(triggered()),
          this, SLOT(TryToJoinConferenceSlot()));

  connect(ui_.updateConferenceAction, SIGNAL(triggered()),
          this, SLOT(UpdateConferenceListSlot()));

  connect(
    ui_.actionHelp, SIGNAL(triggered()),
    this, SLOT(ShowHelpWidgetSlot()));
}

void ConferenceWidget::UpdateConferenceListSlot() {
  conference_dict_.clear();
  ui_.listWidget->clear();
  current_conference_uri_.clear();
  emit RequestConferenceListSignal();
}

void ConferenceWidget::HandleConferenceListReceivedNotifySlot(
    const ConstConferenceList &conferences) {
  for (auto conference : conferences) {
    conference_dict_.insert(conference->uri, conference);
    auto items = 
      ui_.listWidget->findItems(conference->title, Qt::MatchExactly);
    for (auto item : items) {
      if (item->toolTip() == conference->uri) {
        ui_.listWidget->takeItem(ui_.listWidget->row(item));
        delete item;
        break;
      }
    }
    QListWidgetItem *item = 
      new QListWidgetItem(conference->title, ui_.listWidget);
    item->setToolTip(conference->uri);
    int count = ui_.listWidget->count();
    if (count == 1) {
      ui_.listWidget->setCurrentRow(0);
    }
    
    // ѡ��Ϊcurrent_conference_uri_��Ӧ�Ļ�������
    if (!current_conference_uri_.isEmpty() && 
        conference->uri == current_conference_uri_) {
      ui_.listWidget->setCurrentItem(item);
    }

    if (!attempt_conference_uri_.isEmpty() && 
        conference->uri == attempt_conference_uri_) {
      emit JoinConferenceSignal(attempt_conference_uri_);
    }
  }
}

void ConferenceWidget::TryToJoinConferenceSlot() {
  assert(has_initialized_);
  const int conference_index = ui_.listWidget->currentRow();
  if (conference_index == -1) { // δѡ�л���
    QMessageBox::warning(this, tr("CoolView"),
      QString::fromLocal8Bit("��ѡ����Ҫ����Ļ��飡"));
    return;
  }

  QString new_conference_uri = 
    ui_.listWidget->item(conference_index)->toolTip();

  if (current_conference_uri_ == new_conference_uri) {
    QMessageBox::warning(this, tr("CoolView"),
      QString::fromLocal8Bit("���Ѿ������˸û��飡"));
    return;
  }

  // ���Խ������
  emit JoinConferenceSignal(new_conference_uri);
}

void ConferenceWidget::HandleConferenceStateChangedNotifySlot(
    const QString &uri, IConferenceController::ConferenceState state ) {
  switch (state) {
    case IConferenceController::kIsInConference:
      HandleJoinConferenceNotify(uri);
      break;
    case IConferenceController::kIsNotInConference:
      if (current_conference_uri_ == uri) {
        HandleLeaveConferenceNotify();
      }
      break;
  }
}

void ConferenceWidget::HandleJoinConferenceNotify(
    const QString &conference_uri) {
  current_conference_uri_ = conference_uri;
  // ���ô��ڱ���
  QString title = QString::fromLocal8Bit("���ڽ��л���: %1")
    .arg(conference_dict_[conference_uri]->title);
  emit SetWindowTitleSignal(title);
  setVisible(false);
}

void ConferenceWidget::HandleLeaveConferenceNotify() {
  conference_dict_.clear();
  ui_.listWidget->clear();
  current_conference_uri_.clear();
  setVisible(true);
}

void ConferenceWidget::HandleJoinConferenceErrorNotifySlot(
    const QString &uri, IConferenceController::JoinConferenceError error ) {
  // ��ռ�¼��Ҫ���Խ���Ļ���
  attempt_conference_uri_.clear();

  const QString error_title = QString::fromLocal8Bit("����������");
  switch (error) {
    case IConferenceController::kOk:
      // empty
      break;
    case IConferenceController::kErrorOfIsAlreadyInOneConference: {
      if (current_conference_uri_ == uri) {
        return;
      }

      QString info_format = QString::fromLocal8Bit(
        "���Ѿ�������\"%1\"���飬Ҫ�뿪�û��鲢��\"%2\"������");

      QString info = info_format
        .arg(conference_dict_[current_conference_uri_]->title)
        .arg(conference_dict_[uri]->title);

      QMessageBox::StandardButton res = QMessageBox::information(
        this, tr("CoolView"), info, QMessageBox::Ok | QMessageBox::Cancel);

      if (res == QMessageBox::Ok) {
        attempt_conference_uri_ = uri;
        emit LeaveConferenceSignal();
      }
      break; 
    }

    case IConferenceController::kErrorOfConferenceNotExist:
      current_conference_uri_.clear();
      QMessageBox::warning(
        this, error_title, QString::fromLocal8Bit("�û��鲻����"));
      break;

    case IConferenceController::kErrorOfIsLeavingConference:
      QMessageBox::warning(
        this, error_title, QString::fromLocal8Bit("�����뿪����"));
      break;

    case IConferenceController::kErrorOfPasswordNeeded:
      current_conference_uri_.clear();
      QMessageBox::warning(
        this, error_title, QString::fromLocal8Bit("�û�����Ҫ����"));
      break;

    case IConferenceController::kErrorOfJoinRejected:
      current_conference_uri_.clear();
      QMessageBox::warning(
        this, error_title, QString::fromLocal8Bit("QoS�������ܾ�����ļ���"));
      break;

    case IConferenceController::kUnknownError:
      // fall through
    default:
      current_conference_uri_.clear();
      QMessageBox::warning(
        this, error_title, QString::fromLocal8Bit("δ֪����"));
      break;
  }
}

void ConferenceWidget::ShowConferenceTipSlot( int index ) {
  if (index == -1) return;
  auto item = ui_.listWidget->item(index);
  QString conference_uri = item->toolTip();
  auto it = conference_dict_.find(conference_uri);
  if (it != conference_dict_.end()) {
    SetConferenceTip(it.value());
  }
}

void ConferenceWidget::SetConferenceTip( ConstConferencePointer conference ) {
  ui_.confNameLabel->setText(conference->title);
  ui_.confDateLabel->setText(QDate::currentDate().toString());
  ui_.confDurationLabel->setText(
    ConvertDurationMinToQString(conference->duration.toInt()));
  ui_.confStartTimeLabel->setText(conference->start_time);
  ui_.confLevelLabel->setText(conference->level);
  ui_.multicastAddress->setText(conference->multicast_address);
  ui_.joinModeLabel->setText(conference->join_mode);
  ui_.controlModeLabel->setText(conference->control_mode);
  if (conference->multicast_address.isEmpty() || 
      conference->multicast_address == "0.0.0.0") {
    ui_.isMulticast->setText(QString::fromLocal8Bit("��"));
  } else {
    ui_.isMulticast->setText(QString::fromLocal8Bit("��"));
  }
}

QString ConferenceWidget::ConvertDurationMinToQString( int duration_min ) {
  int days, hours, minutes;
  days = duration_min / (60 * 24);
  duration_min -= days * 60 * 24;
  hours = duration_min / 60;
  duration_min -= hours * 60;
  minutes = duration_min;
  if (days > 0) {
    return QString::fromLocal8Bit("%1��%2ʱ%3��").arg(
      days).arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0'));
  } else {
    return QString::fromLocal8Bit("%1ʱ%2��").arg(
      hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0'));
  }
}

void ConferenceWidget::HandleSessionStateChangedNotifySlot(
    ISessionController::SessionState state) {
  if (state == ISessionController::kIsOffline) {
    conference_dict_.clear();
    ui_.listWidget->clear();
    current_conference_uri_.clear();
  }
}

void ConferenceWidget::ShowHelpWidgetSlot() {
  if (help_widget_ == nullptr) {
    help_widget_ = new HelpWidget(nullptr);
  }
  help_widget_->show();
  help_widget_->raise();
  help_widget_->setFocus();
}

