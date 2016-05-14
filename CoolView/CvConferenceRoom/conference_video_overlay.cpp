#include "conference_video_overlay.h"

#include <QGraphicsDropShadowEffect>
#include <QResizeEvent>

// ע�⣺
// ����ΪQWidgetָ�������󣬷���������������ָô����Ҵ��ڲ��ḡ���²��ͼ�񴰿���
// ����ָ��Qt::Window�������޷���DirectShow EVR��ʾ����Ƶͼ����ʵ��͸��
ConferenceVideoOverlay::ConferenceVideoOverlay(QWidget *parent)
	: QWidget(parent)
	, pending_icon_frame_(nullptr)
{
	ui.setupUi(this);

	// ��������
	setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
	setAutoFillBackground(false);
	setAttribute(Qt::WA_TranslucentBackground);

	// �ı���ͼ��
	SetLabelTransparentEffect(ui.qosLabel);
	SetLabelTransparentEffect(ui.userNameLabel);

	ui.stopSendAudioFrame->setStyleSheet("QFrame{border-image:"
		"url(:/CvConferenceRoom/Resources/MicrophoneRed.png);"
		"border: 0px solid darkGray;}");

	ui.stopRecvAudioFrame->setStyleSheet("QFrame{border-image:"
		"url(:/CvConferenceRoom/Resources/stopaudio.png);"
		"border: 0px solid darkGray;}");

	pending_icon_frame_ = new QFrame(this);
	pending_icon_frame_->setStyleSheet(
		"QFrame{ border-image:url(:/CvConferenceRoom/Resources/mediaWaiting.png);"
		"border: 2px solid darkGray; border-radius: 5px; background: transparent;}");

	AdjustStyle();
	Reset();
}

ConferenceVideoOverlay::~ConferenceVideoOverlay()
{
	if (pending_icon_frame_) {
		pending_icon_frame_->deleteLater();
		pending_icon_frame_ = nullptr;
	}
}

void ConferenceVideoOverlay::SetLabelTransparentEffect( QLabel * label )
{
	if (!label) {
		return;
	}
	QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
	effect->setBlurRadius(0);
	effect->setColor(QColor("#0"));
	effect->setOffset(1,1);
	label->setGraphicsEffect(effect);
	label->setAttribute(Qt::WA_TranslucentBackground);
}

void ConferenceVideoOverlay::AdjustStyle()
{
	const QRect rect = geometry();
	const float unit = rect.width() > rect.height() ? 
		rect.height() / 400.0 : rect.width() / 400.0; // �����յĻ������ȣ�
	  //����ʱ���ڸ�400���صĴ����н��еģ��ʳ���400��Ϊ�˷���

	int font_size_in_px = 0;

	font_size_in_px = (int)20 * unit;
	if (font_size_in_px < 16) font_size_in_px = 16;
	ui.qosLabel->setStyleSheet(QString(
		"color: white; font: bold %1px ΢���ź�;")
		.arg(font_size_in_px));

	font_size_in_px = (int)26 * unit;
	if (font_size_in_px < 20) font_size_in_px = 20;
	ui.userNameLabel->setStyleSheet(QString(
		"color: white; font: bold %1px; "
		"qproperty-alignment: AlignCenter;")
		.arg(font_size_in_px));

	font_size_in_px = (int)48 * unit;
	if (font_size_in_px < 36) font_size_in_px = 36;
	ui.videoStatusLabel->setStyleSheet(QString(
		"color: white; font: bold %1px; qproperty-alignment: AlignCenter;"
		"border: %2px solid darkGrey; border-radius: %3px; "
		"background-color: rgba(230, 230, 230, 25%);")
		.arg(font_size_in_px)
		.arg(2 * unit)
		.arg(10 * unit));
}

void ConferenceVideoOverlay::resizeEvent( QResizeEvent * event )
{
	const int width = event->size().width();
	const int height = event->size().height();

	{
		const int frame_width = height / 3;
		int left = (width - frame_width) / 2;
		int top = (height-frame_width) / 2;
		pending_icon_frame_->setGeometry( left , top , frame_width , frame_width );
	}

	//�����ı���С
	AdjustStyle();
}

void ConferenceVideoOverlay::Reset()
{
	SetUserName("");
	SetStopSendAudioIconVisible(false);
	SetStopRecvAudioIconVisible(false);
	SetStopVideoStatusVisible(false);
	SetPendingIconVisible(true);
}

void ConferenceVideoOverlay::SetQosInfo( const QString &str )
{
	ui.qosLabel->setText(str);
}

void ConferenceVideoOverlay::SetQosInfoVisible( bool visible /*= true*/ )
{
	ui.qosLabel->setVisible(visible);
}
void ConferenceVideoOverlay::SetUserName( const QString &str )
{
	ui.userNameLabel->setText(str);
}

void ConferenceVideoOverlay::SetUserNameVisible( bool visible /*= true*/ )
{
	ui.userNameLabel->setVisible(visible);
}

void ConferenceVideoOverlay::SetStopSendAudioIconVisible( 
	bool visible /*= true*/ )
{
	ui.stopSendAudioFrame->setVisible(visible);
}

bool ConferenceVideoOverlay::IsStopSendAudioIconVisible()
{
	return !ui.stopSendAudioFrame->isHidden();
}

void ConferenceVideoOverlay::SetStopRecvAudioIconVisible( 
	bool visible /*= true*/ )
{
	ui.stopRecvAudioFrame->setVisible(visible);
}

bool ConferenceVideoOverlay::IsStopRecvAudioIconVisible()
{
	return !ui.stopRecvAudioFrame->isHidden();
}

void ConferenceVideoOverlay::SetStopVideoStatusVisible( 
	bool visible /*= true*/ )
{
	ui.videoStatusLabel->setVisible(visible);
}

bool ConferenceVideoOverlay::IsStopVideoStatusVisible()
{
	return !ui.videoStatusLabel->isHidden();
}

void ConferenceVideoOverlay::SetPendingIconVisible( bool visible /*= true*/ )
{
	pending_icon_frame_->setVisible(visible);
}

bool ConferenceVideoOverlay::IsPendingIconVisible()
{
	return !pending_icon_frame_->isHidden();
}


