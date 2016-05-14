#include "record_status_widget.h"

#include "controller_manager_interface.h"
#include "tx_controller_interface.h"

#include "profile/RunningProfile.h"
#include "terminal_description.h"

#include <QMessageBox>
#include <QScrollBar>


#define STATUS_TEXT_ONLINE "����"
#define STATUS_TEXT_WAIT_FOR_REPLY "������"
#define STATUS_TEXT_WAIT_FOR_DATA "������"
#define STATUS_TEXT_RECORDING "¼����"
#define STATUS_TEXT_COMPLETE "���"
#define STATUS_TEXT_NO_DATA "������"
#define STATUS_TEXT_NO_INPUT "���ź�"


// �����ͷ�������
enum TerminalTableHeaderIndex
{
    HEADER_NAME,
    HEADER_STATUS,
    HEADER_REQUESTER,
    HEADER_DURATION,
    HEADER_SIZE,
    HEADER_IP,
    HEADER_VIDEO_PORT,
    HEADER_VIDEO_BAND,
    HEADER_VIDEO_LOST,
    HEADER_AUDIO_PORT,
    HEADER_AUDIO_BAND,
    HEADER_AUDIO_LOST,
    HEADER_FILE,
    HEADER_COUNT //���������Զ������ͷ����
};

/**
 * @brief ��URI��VirtualURIת��Ϊ�ײ�ý����ʹ�õ�MediaID
 */
QString UriToUserID(const QString &vuri)
{
	return TerminalHelper::GetUsername(vuri);
}

/**
 * @brief ������ת����QTableWidgetItem *
 */
 QTableWidgetItem *ConvertToQtTableItem(const QString str,
   Qt::Alignment alignment = Qt::AlignCenter) {
     QTableWidgetItem *tempItem = new QTableWidgetItem(str);
     tempItem->setTextAlignment(alignment);
     return tempItem;
}

template<typename T>
QTableWidgetItem *ConvertToQtTableItem(const QString fmt,
  Qt::Alignment alignment, T var) {
    QString strItem = QString(fmt).arg(var);
    QTableWidgetItem *tempItem = new QTableWidgetItem(strItem);
    tempItem->setTextAlignment(alignment);
    return tempItem;
}

template<typename T1, typename T2>
QTableWidgetItem *ConvertToQtTableItem(const QString fmt,
  Qt::Alignment alignment, T1 var1, T2 var2) {
    QString strItem = QString(fmt).arg(var1).arg(var2);
    QTableWidgetItem *tempItem = new QTableWidgetItem(strItem);
    tempItem->setTextAlignment(alignment);
    return tempItem;
}


RecordStatusWidget::RecordStatusWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	/*connect(ui.startRecButton, &QPushButton::clicked,
        this, &RecordStatusWidget::startRecSlot);
	connect(ui.stopRecButton, &QPushButton::clicked,
        this, &RecordStatusWidget::stopRecSlot);
	connect(ui.cutFileButton, &QPushButton::clicked,
        this, &RecordStatusWidget::cutFileSlot);*/

  QVector<QString> table_header;
	table_header.resize(HEADER_COUNT);
	table_header[HEADER_NAME]    = QString::fromLocal8Bit("����ʶ");
  table_header[HEADER_STATUS]	 = QString::fromLocal8Bit("״̬");
  table_header[HEADER_REQUESTER] = QString::fromLocal8Bit("������");
  table_header[HEADER_DURATION] = QString::fromLocal8Bit("ʱ��");
  table_header[HEADER_SIZE]	 = QString::fromLocal8Bit("�ļ���С");
	table_header[HEADER_IP]		 = QString::fromLocal8Bit("���ͷ�IP");
	table_header[HEADER_VIDEO_PORT] = QString::fromLocal8Bit("��Ƶ�˿�");
	table_header[HEADER_VIDEO_BAND] = QString::fromLocal8Bit("��Ƶ����");
	table_header[HEADER_VIDEO_LOST] = QString::fromLocal8Bit("������");
	table_header[HEADER_AUDIO_PORT] = QString::fromLocal8Bit("��Ƶ�˿�");
	table_header[HEADER_AUDIO_BAND] = QString::fromLocal8Bit("��Ƶ����");
	table_header[HEADER_AUDIO_LOST] = QString::fromLocal8Bit("������");
	table_header[HEADER_FILE]    = QString::fromLocal8Bit("�ļ�״̬");

	QStringList tableHeader;
	for (int i = 0; i < table_header.size(); ++i)
	{
		tableHeader << table_header[i];
	}

	ui.TerminalTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.TerminalTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.TerminalTable->setColumnCount(HEADER_COUNT);
	ui.TerminalTable->setHorizontalHeaderLabels(tableHeader);

	//ui.TerminalTable->resizeColumnsToContents();
	//ui.TerminalTable->resizeRowsToContents();
	ui.TerminalTable->setColumnWidth(HEADER_NAME, 150);
  ui.TerminalTable->setColumnWidth(HEADER_STATUS, 50);
  ui.TerminalTable->setColumnWidth(HEADER_REQUESTER, 80);
  ui.TerminalTable->setColumnWidth(HEADER_DURATION, 60);
  ui.TerminalTable->setColumnWidth(HEADER_SIZE, 60);
	ui.TerminalTable->setColumnWidth(HEADER_IP, 100);
	ui.TerminalTable->setColumnWidth(HEADER_VIDEO_PORT, 100);
	ui.TerminalTable->setColumnWidth(HEADER_VIDEO_BAND, 80);
	ui.TerminalTable->setColumnWidth(HEADER_VIDEO_LOST, 60);
	ui.TerminalTable->setColumnWidth(HEADER_AUDIO_PORT, 100);
	ui.TerminalTable->setColumnWidth(HEADER_AUDIO_BAND, 80);
	ui.TerminalTable->setColumnWidth(HEADER_AUDIO_LOST, 60);
	ui.TerminalTable->setColumnWidth(HEADER_FILE, 500);

  //ui.verticalLayout->removeWidget(ui.statBrowser);
  ui.statBrowser->hide();
}

RecordStatusWidget::~RecordStatusWidget()
{

}

void RecordStatusWidget::Initialize( IControllerManager *controller_manager )
{
    assert(controller_manager);
    controller_manager_ = controller_manager;

    IConferenceController *conference_controller =
        controller_manager->GetConferenceController();
    assert(conference_controller);

    // ���Ӵ������״̬������źźͲ�
    connect(
        conference_controller, 
        &IConferenceController::NotifyConferenceStateChangedSignal,
        this,
        &RecordStatusWidget::HandleConferenceStateChangedNotifySlot);

    // ���Ӵ�����յ����ն��б��źźͲ�
    connect(
        conference_controller,
        &IConferenceController::NotifyTerminalReceivedSignal,
        this, 
        &RecordStatusWidget::HandleTerminalReceivedNotifySlot);

    // ���Ӵ�����յ��ն���Ϣ���µ��źźͲ�
    connect(
        conference_controller,
        &IConferenceController::NotifyTerminalUpdateSignal,
        this,
        &RecordStatusWidget::HandleTerminalUpdateNotifySlot);

    connect(
        conference_controller,
        &IConferenceController::NotifyStreamMediaReceiveStartedSignal,
        this,
        &RecordStatusWidget::HandleNotifyStreamMediaReceiveStartedSlot);

    connect(
        conference_controller,
        &IConferenceController::NotifyStreamMediaReceiveStoppedSignal,
        this,
        &RecordStatusWidget::HandleNotifyStreamMediaReceiveStoppedSlot);

    connect(
        conference_controller,
        &IConferenceController::NotifyStreamMediaReceiveReplySignal,
        this,
        &RecordStatusWidget::HandleNotifyStreamMediaReceiveReplySlot);

    // ���Ӵ���RTP״̬������źźͲ�
    connect(
        conference_controller,
        &IConferenceController::NotifyRTPStatReceivedSignal,
        this,
        &RecordStatusWidget::HandleNotifyRTPStatReceivedSlot);

    ITXController *tx_controller = controller_manager->GetTXController();
    assert(tx_controller);
    // ���Ӵ���¼��״̬������źźͲ�
    connect(
        tx_controller,
        &ITXController::NotifyRecStatReceivedSignal,
        this,
        &RecordStatusWidget::HandleNotifyRecStatReceivedSlot);

    connect(
      tx_controller,
      &ITXController::NotifyRecordRequesterSignal,
      this,
      &RecordStatusWidget::HandleNotifyRecordRequesterSlot);
}

void RecordStatusWidget::Reset()
{
    while (ui.TerminalTable->rowCount() > 0)
    {
        ui.TerminalTable->removeRow(0);
    }
    ui.statBrowser->clear();

    mediaID_name_map_.clear();
    name_uri_map.clear();
}

void RecordStatusWidget::ResetRow(int row)
{
  for (int j = 1; j < HEADER_COUNT; ++j)
  {
    if (HEADER_STATUS == j)
    {
      ui.TerminalTable->setItem(row, j, new QTableWidgetItem(QString::fromLocal8Bit(STATUS_TEXT_COMPLETE)));
      continue;
    }
    ui.TerminalTable->setItem(row, j, new QTableWidgetItem(""));
  }
}

void RecordStatusWidget::HandleConferenceStateChangedNotifySlot( 
    const QString &conference_uri, 
    IConferenceController::ConferenceState state )
{
    // �ڽ��������˳�����ʱ����Ҫ����״̬��
    if (state == IConferenceController::kIsJoiningConference ||
        state == IConferenceController::kIsNotInConference) {
            Reset();
    }
}

void RecordStatusWidget::HandleTerminalReceivedNotifySlot( 
    ConstTerminalPointer terminal, const QString &conference_uri)
{
  if (nullptr == terminal ||
      terminal->uri == RunningProfile::getInstance()->user_uri().c_str() ||
      TerminalHelper::IsModelTX(terminal->model)) {
    return;
  }

	ui.TerminalTable->resizeRowsToContents();

	QString virtualUri = terminal->virtual_uri;
	int row = ui.TerminalTable->rowCount();

	// if already have
	if (name_uri_map.keys(virtualUri).size() > 0)
	{
		HandleTerminalUpdateNotifySlot(terminal, conference_uri);
		return;
	}

	QString name = terminal->name;

	ui.TerminalTable->insertRow(row);

	QString mediaID = UriToUserID(virtualUri);
	QString strItem = name;
	mediaID_name_map_[mediaID] = strItem;
	name_uri_map[strItem] = virtualUri;

	// name
	QTableWidgetItem *nameItem = ConvertToQtTableItem("%1", 
        Qt::AlignLeft | Qt::AlignVCenter, strItem);
	ui.TerminalTable->setItem(row, HEADER_NAME, nameItem);

	// status
	strItem = (terminal->is_available ? QString::fromLocal8Bit(STATUS_TEXT_ONLINE) : "");
	UpdateStatusColumn(row, strItem);

  // sort
  ui.TerminalTable->sortByColumn(HEADER_NAME, Qt::AscendingOrder);
}

void RecordStatusWidget::HandleTerminalUpdateNotifySlot(
    ConstTerminalPointer terminal, const QString &conference_uri)
{
	QString virtualUri = terminal->virtual_uri;
	QString mediaID = UriToUserID(virtualUri);

	int i = GetRow(mediaID);
	if (i < 0)
	{
    // �����ڸ��ն�
		return;
	}

	QString strItem = (terminal->is_available ? QString::fromLocal8Bit(STATUS_TEXT_ONLINE) : "");
	UpdateStatusColumn(i, strItem);
}

void RecordStatusWidget::HandleNotifyRTPStatReceivedSlot( const RtpStatItem &rtpstat )
{
	QString mediaID = rtpstat.member_id;
	//mediaID = mediaID.replace("Recv_", "");
	int i = GetRow(mediaID);
	if (-1 == i)
	{
		return;
	}

	QTableWidgetItem * ipItem = ConvertToQtTableItem("%1", Qt::AlignCenter, rtpstat.rem_addr);
    QTableWidgetItem * bandwidthItem = ConvertToQtTableItem("%1kb/s", 
        Qt::AlignRight | Qt::AlignVCenter, rtpstat.recv_bandwidth);
    QTableWidgetItem * portItem = ConvertToQtTableItem("%1->%2", 
        Qt::AlignCenter, rtpstat.rem_port, rtpstat.local_port);
    QTableWidgetItem * lostItem = ConvertToQtTableItem("%1%", 
        Qt::AlignRight | Qt::AlignVCenter, rtpstat.lost);
	ui.TerminalTable->setItem(i, HEADER_IP, ipItem);

	if (rtpstat.media_type == eMT_Video)
	{
		ui.TerminalTable->setItem(i, HEADER_VIDEO_BAND, bandwidthItem);
		ui.TerminalTable->setItem(i, HEADER_VIDEO_PORT, portItem);
		ui.TerminalTable->setItem(i, HEADER_VIDEO_LOST, lostItem);
	}

	if (rtpstat.media_type == eMT_Audio)
	{
		ui.TerminalTable->setItem(i, HEADER_AUDIO_BAND, bandwidthItem);
		ui.TerminalTable->setItem(i, HEADER_AUDIO_PORT, portItem);
		ui.TerminalTable->setItem(i, HEADER_AUDIO_LOST, lostItem);
	}

}

void RecordStatusWidget::HandleNotifyRecStatReceivedSlot( const RecStatItem& recstat )
{
	QDateTime time;
	time.setTime_t(recstat.time);
	QString strDt = time.toString("hh:mm:ss");

	QString user_name = recstat.user_name;
	//strMediaId = strMediaId.replace("Recv_", "");
	int row = GetRow(user_name);
	if (-1 == row)
	{
		return;
	}

	QString msg;
	QString status;
	QString file;
	switch (recstat.statType)
	{
	case REC_STAT_MUX_RUN:
		msg = QString::fromLocal8Bit("%1δ�����ļ�").arg(user_name);
		status = QString::fromLocal8Bit("�ȴ���Ƶ�ؼ�֡");
		break;
  case REC_STAT_MUX_STOP:
    ResetRow(row);
    break;
	case REC_STAT_FILE_CREATE:
		msg = QString::fromLocal8Bit("���� %1").arg(QString::fromLocal8Bit(recstat.rec.file));
		status = QString::fromLocal8Bit("����");
		file = QString::fromLocal8Bit(recstat.rec.file);
		break;
	case REC_STAT_FILE_START:
		msg = QString::fromLocal8Bit("д�� %1").arg(QString::fromLocal8Bit(recstat.rec.file));
		UpdateStatusColumn(row, QString::fromLocal8Bit(STATUS_TEXT_RECORDING));
		status = QString::fromLocal8Bit("д��");
		file = QString::fromLocal8Bit(recstat.rec.file);
		break;
	case REC_STAT_FILE_CLOSE:
		msg = QString::fromLocal8Bit("�ر� %1").arg(QString::fromLocal8Bit(recstat.rec.file));
		break;
	case REC_STAT_FILE_ABANDON:
		msg = QString::fromLocal8Bit("ɾ�� %1").arg(QString::fromLocal8Bit(recstat.rec.file));
		//updateColumnStatus(row, QString::fromLocal8Bit(STATUS_TEXT_NO_DATA));
		status = QString::fromLocal8Bit("������,ɾ��");
		file = QString::fromLocal8Bit(recstat.rec.file);
		break;
	case REC_STAT_CUT_REQUEST:
		msg = QString::fromLocal8Bit("д�����ļ�%1���ر�%2").
            arg(QString::fromLocal8Bit(recstat.cut.next)).
            arg(QString::fromLocal8Bit(recstat.cut.cur));
		break;
    case REC_STAT_FILE_WRITING:
        {
            unsigned long secs = (unsigned long)recstat.rec.duration / 1000;
            unsigned long hour = secs / 3600;
            unsigned long min = secs / 60 % 60;
            unsigned long sec = secs % 60;
            QString durStr = QString("%1:%2:%3").arg(hour)
                .arg(min, 2, 10, QChar('0'))
                .arg(sec, 2, 10, QChar('0'));
            QTableWidgetItem * durationItem = ConvertToQtTableItem(durStr, Qt::AlignCenter);
            ui.TerminalTable->setItem(row, HEADER_DURATION, durationItem);

            QString sizeStr = QString("%1MB").arg(recstat.rec.size/1000000.0, 0, 'f', 1);
            QTableWidgetItem * sizeItem = ConvertToQtTableItem(sizeStr, 
                Qt::AlignRight | Qt::AlignVCenter);
            ui.TerminalTable->setItem(row, HEADER_SIZE, sizeItem);
        }
        break;
	default:
		msg = QString::fromLocal8Bit("%1δ֪״̬:%2").arg(user_name).arg(recstat.statType);
	}

	msg = strDt + " " + msg;
	ui.statBrowser->append(msg);

	if (!status.isEmpty())
	{
		const int indexDash = file.lastIndexOf("\\");
		if (indexDash != -1)
		{
			file = file.mid(indexDash + 1, file.lastIndexOf("."));
		}
		UpdateFileColumn(row, strDt + " " + status + " " + file);
	}

	/*QScrollBar *sb = ui.statBrowser->verticalScrollBar();
	sb->setValue(sb->maximum());*/
}

void RecordStatusWidget::HandleNotifyStreamMediaReceiveStartedSlot( 
    const QString &vuri )
{
	QString mediaID = UriToUserID(vuri);
	int i = GetRow(mediaID);
	if (i < 0)
	{
		return;
	}

  QString strItem = QString::fromLocal8Bit(STATUS_TEXT_WAIT_FOR_REPLY);
  UpdateStatusColumn(i, strItem);
}

void RecordStatusWidget::HandleNotifyStreamMediaReceiveStoppedSlot( 
    const QString &vuri )
{
	
}

void RecordStatusWidget::HandleNotifyStreamMediaReceiveReplySlot(
  const QString &vuri, bool permission)
{
  QString mediaID = UriToUserID(vuri);
  int i = GetRow(mediaID);
  if (i < 0)
  {
    return;
  }

  QString status = ui.TerminalTable->item(i, HEADER_STATUS)->text();
  if (status == QString::fromLocal8Bit(STATUS_TEXT_RECORDING))
  {
    return;
  }

  QString strItem;
  if (permission) {
    strItem = QString::fromLocal8Bit(STATUS_TEXT_WAIT_FOR_DATA);
  } else {
    strItem = QString::fromLocal8Bit(STATUS_TEXT_NO_INPUT);
  }
  UpdateStatusColumn(i, strItem);
}

int RecordStatusWidget::GetRow( QString mediaIDOrName )
{
    for (int i=0; i < ui.TerminalTable->rowCount(); ++i)
    {
        QString tbName = ui.TerminalTable->item(i, HEADER_NAME)->text();
        if (tbName == mediaID_name_map_[mediaIDOrName] || tbName == mediaIDOrName)
        {
            return i;
        }
    }
    return -1;
}

void RecordStatusWidget::UpdateFileColumn( const int row, const QString text )
{
	QString strItem = text;
	QTableWidgetItem * tempItem = new QTableWidgetItem(strItem);
	tempItem->setTextAlignment(Qt::AlignLeft);
	ui.TerminalTable->setItem(row, HEADER_FILE, tempItem);
}

void RecordStatusWidget::UpdateStatusColumn( const int row, const QString text )
{
	QString strItem = text;
	QTableWidgetItem * tempItem = new QTableWidgetItem(strItem);
	tempItem->setTextAlignment(Qt::AlignCenter);
	ui.TerminalTable->setItem(row, HEADER_STATUS, tempItem);
}

void RecordStatusWidget::HandleNotifyRecordRequesterSlot( 
  const QString &vuri, const QString &requester_uri )
{
  QString user_id = UriToUserID(vuri);
  QString requester_id = UriToUserID(requester_uri);
  int row = GetRow(user_id);
  if (-1 == row)
  {
    return;
  }

  QTableWidgetItem * tempItem = new QTableWidgetItem(requester_id);
  tempItem->setTextAlignment(Qt::AlignCenter);
  ui.TerminalTable->setItem(row, HEADER_REQUESTER, tempItem);
}


