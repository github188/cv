#pragma once
#include "ui_screenShare.h"
#include <QtGui/QtGui>
#include <dbus/conferenceRoom/common/ConferenceRoomServiceCommon.h>
class QtConferenceRoom;
class CvStreamedMediaIf;
class ChannelDispatcherIf;
class QtScreenWidget;
class QtScreenShare:public QWidget
{
	Q_OBJECT
public:
	static enum PPTControlType
	{
		///δ֪
		PPTControl_Unknow,

		///��һ��PPT
		PPTControl_Next,

		///��һ��PPT
		PPTControl_Pre,

		///�ر�PPT
		PPTControl_Close,
	};

    enum SharedMode{
      kVideoAudioMode,
      kDocumentMode,
      kAirPlayMode,
    };

	QtScreenShare( QtConferenceRoom* room );
	~QtScreenShare(void);
	void stopScreenGraph();
	void cvHostUnShareScreenEventHandler();
	void openFile(QString filename , int screenIndex);

	QtConferenceRoom* ConfRoom() const { return _confRoom; }
	void ConfRoom(QtConferenceRoom* val) { _confRoom = val; }

	/**
	 * @brief ������Ļ���ڵľ��ֵ
	 * @return ������ɹ��򷵻�0�����򷵻ؾ��ֵ
	 */
	int getScreenWindow( );

	QString UserID() const { return _userID; }
	void UserID(QString val) { _userID = val; }

	void setMediaState( const UiMediaState state );

	bool getScreenShareState() {return isScreenShare ;}

	/**
	 * @brief ����PPT�Ĳ���
	 * @param type �������ͣ�����PPTControlTypeö������
	 */
	void controlPPT( const PPTControlType type );
	
	void killPPT();

	/**
	 * @brief ���ƹ�����Ļ�Ľ���İ�ť�������ǰ�ն�Ϊ��������ѡ���Ļ������ť��ʾ����������������
	 * @param control ���Ϊtrue������ʾ��ť���������ذ�ť
	 */
	void ControlScreenShare( const bool control );

    void SetSharedMode(SharedMode mode);
    SharedMode GetSharedMode() const { return _sharedMode; }

Q_SIGNALS:
	void cvHostUnShareScreenSinal();
public Q_SLOTS:
	void openFile();
	void closeFile();
	/**
	 * @brief ������Ļ��������Ļ��ͼ
	 * @params screenIndex ��ͼ���ڵ���Ļ����
	 * @params left ��ͼ��߽���screenIndex��Ļ�µ���߽�
	 * @params top ��ͼ��߽���screenIndex��Ļ�µ��ϱ߽�
	 * @params right ��ͼ��߽���screenIndex��Ļ�µ��ұ߽�
	 * @params bottom ��ͼ��߽���screenIndex��Ļ�µ��±߽�
	 */
	void sendSpecificScreen( const int left=0,const int top=0,const int right=0, const int bottom=0 );

	void recvScreen( const bool enable);
	void cvHostUnShareScreenSlot();

	/** 
	 * @brief �����Ļ���������״̬
	 */
	void release( );

	/**
	 * @brief ��ppt�ȸ����ļ��Ľ����˳���ʱ�򴥷���SLOT
	 */
	void pptProcessFinishSlot(int,QProcess::ExitStatus );

protected:
	virtual bool eventFilter(QObject*, QEvent*);
	virtual void resizeEvent ( QResizeEvent * event );
private:
    void changeToDocumentMode();
    void changeToAirPlayMode();
    void changeToVideoAudioMode();
    void notifyAllScreenSharedState(bool isShared);
    bool activateAirServer();

private:
	Ui::ScreenShare *_ui;
	QtScreenWidget *widget;
	bool firstOpenFile;
	bool isSendScreen;
	bool isRecvScreen;
	bool isScreenShare;

	QtConferenceRoom*	_confRoom;

	CvStreamedMediaIf*		_streamProxy;
	ChannelDispatcherIf*	_channelProxy;

	QString			_userID;

	///���ڴ�PPT�Ľ���ָ��
	QProcess*	_pptProcess;
	///PPT�Ĵ���ָ��
	HWND		_pptWnd;
	///�õ�Ƭָ��
	HWND		_slideWnd;


    // AirServer�Ĵ��ھ��
    HWND _airServerWnd;

    SharedMode _sharedMode;
};
