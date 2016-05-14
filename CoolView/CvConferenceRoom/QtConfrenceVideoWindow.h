#pragma once

#include "ui_ConfrenceVideoWindow.h"
#include "qtmeetingplacewidget.h"

#include <QString>
#include <QTimer>

//zhenHua.sun 2010-09-01
#include <util/report/RtpStat.h>
#include <util/report/RecordStat.h>
#include <dbus/conferenceRoom/common/ConferenceRoomServiceCommon.h>


class QtMeetingPlaceWidget;
class QLabel;
class RecordCtrlPanel;
class RecordAdvancedPanel;

class QtConfrenceVideoWindow:public QWidget
{
	friend class QtMeetingPlaceWidget;
	friend class VideoFrame;
Q_OBJECT

public:
	//QtConfrenceVideoWindow(QtMeetingPlaceWidget * parent);
	QtConfrenceVideoWindow();
	~QtConfrenceVideoWindow(void);

	QFrame *getVideoFrame();

  QString getUserId(){return _userId;}
  void setUserId(const QString &user);

	bool getUseStatus(){return isUsed;}
	void setUseStatus(bool usedStatus){isUsed = usedStatus;}

	void updateRtcpEventHandler( const RtpStatItem& item );
    void updateRecordEventHandler(const RecStatItem& item);

	/**
	 * @brief ����ý���������״̬����ͼ����ʾ
	 * @param type ý�����ͣ�audio��video
	 * @param state ý��״̬
	 * @param isAudioSend ���Ϊtrue����Ҫ����˷�ͼ����в���������Ӧ�ö�����ͼ����в���
	 */
	void setMediaState( const QString type , const UiMediaState state,bool isAudioSend);
	void setRecordState(const UiMediaState state);

	bool isAudioEnable();
	bool isVideoEnable();

	/**
	 * @brief �ͷŴ�����Դ
	 */
	void release();

	
public Q_SLOTS:
	///@brief ����Ƶ��������ʾrtcp��Ϣ
	void showRtcpMessage();

	///@brief ������Ƶ�����ϵ�rtcp��Ϣ
	void hideRtcpMessage( );

	QString getDisplayName() const { return _displayName; }
  void setDisplayName( const QString displayName );

  //Record control
  void startRecord();
  void stopRecord();
  void ShowRecordAdvancedPanel(bool);

protected:
  virtual void resizeEvent ( QResizeEvent * event );
  virtual void moveEvent(QMoveEvent * event);
  virtual void hideEvent(QHideEvent * event);
  virtual void showEvent(QShowEvent * event);

private:
  void createLabels();
  void updateTextLabel(const QString &str);
  void updateRecordTextLabel(const QString &str);
  void resizeTextLabel();
  void adjustLabelStyle();

private:
	Ui::ConfrenceVideoWindow _ui;
	QString _userId;
	QString _displayName;		//��ý�崰������ʾ������
  QString _rtcpMsg;
  QString _recMsg;
	bool isUsed;
	bool _detached;
	bool _fullScreen;
  bool _showRtcp;
  bool _isLabelTransparent;

	//rtcp��ʾ��
	QFrame* _stopAudioFrame;
	//QFrame* _stopVideoFrame;
	QFrame* _stopRecvAudioFrame;
	QFrame* _mediaStateFrame;

  QLabel* _textLabel; // ������ʾ͸����Ļ���ı��� - by Liaokz
  QLabel* _videoStatuslabel;

  //¼�ƿ���
  RecordCtrlPanel * _recordCtrlPanel;
  bool _isRecordCtrlPanelVisible;
  RecordAdvancedPanel *_recordAdvancedPanel;
};
