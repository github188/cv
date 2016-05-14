#ifndef CONFERENCE_VIDEO_OVERLAY_H
#define CONFERENCE_VIDEO_OVERLAY_H

#include <QWidget>
#include "ui_conference_video_overlay.h"

class ConferenceVideoOverlay : public QWidget
{
	Q_OBJECT

public:
	ConferenceVideoOverlay(QWidget *parent = 0);
	~ConferenceVideoOverlay();

	void Reset();

	void SetUserName(const QString &str);
	void SetUserNameVisible(bool visible = true);

	void SetQosInfo(const QString &str);
	void SetQosInfoVisible(bool visible = true);

	void SetStopSendAudioIconVisible(bool visible = true);
	bool IsStopSendAudioIconVisible();

	void SetStopRecvAudioIconVisible(bool visible = true);
	bool IsStopRecvAudioIconVisible();

	void SetStopVideoStatusVisible(bool visible = true);
	bool IsStopVideoStatusVisible();

	void SetPendingIconVisible(bool visible = true);
	bool IsPendingIconVisible();

protected:
	void resizeEvent(QResizeEvent * event);

private:
	void SetLabelTransparentEffect(QLabel * label);
	void AdjustStyle(); // ����С�ı�ʱ������Ԫ�ر���

private:
	Ui::ConferenceVideoOverlay ui;

	QFrame * pending_icon_frame_; // �ȴ���ʶ��Ҫ�ص�������Ԫ���Ϸ�������Ҫ��̬����
};

#endif // CONFERENCE_VIDEO_OVERLAY_H
