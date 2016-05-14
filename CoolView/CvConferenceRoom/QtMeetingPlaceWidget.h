#pragma once

#include "ui_MeetingPlaceWidget.h"
#include <QtGui/QtGui>
#include <vector>
#include <map>
#include <util/report/RtpStat.h>
using namespace std;


#include <dbus/conferenceRoom/common/ConferenceRoomServiceCommon.h>
#include "qtconferenceroom.h"

class VideoFrame;
class QtMeetingPlaceWidget:public QWidget
{
	friend class ConferenceRoomService;
	friend class MediaWindowManager;

	Q_OBJECT
public:
	QtMeetingPlaceWidget( QtConferenceRoom* confRoom);
	~QtMeetingPlaceWidget(void);

public slots:
	///��ʾý�崰��
	void showVideoWindow( VideoFrame* frame );

	///����ý�崰��
	void hideVideoWindow( VideoFrame* frame );

	



//
//	QtConfrenceVideoWindow *GetUseableWin();
//	void closeWindow(const std::string &userId);
//	void closeAllWindow();
//	//����Ƿ��Ѿ������˴��û�������Ƶ
//	bool checkUserExist(std::string &userId);
//	QGridLayout * getWidgetLayout();
//
//	/**
//	 * @brief �Ƿ���ʾRTCP��Ϣ
//	 */
//	bool isShowingRtcpMsg(){ return showRtcpMessageEnbaled; }
//
//	/**
//	 * @brief ������Ƶ����Ļ��Դ���
//	 * @return ���ط���Ļ��Դ��ھ��
//	 */
//	HWND setPreviewWnd();
//
//	/**
//	 * @brief ��ȡ���Դ���
//	 */
//	HWND getPreviewWnd();
//
//	/**
//	 * @brief ��ȡuserID����Ƶ�����е�λ��
//	 * @userID	�û���
//	 * @return �������򷵻�-1
//	 */
//	int	findUserSeet( QString usreID );
//
//	/**
//	 * @brief ����λ�ϼ���ָ���û�
//	 * @param seet ��λ���
//	 * @param userID �û���
//	 * @return ���ʧ���򷵻�NULL
//	 */
//	QtConfrenceVideoWindow* assignUserSeet( const int& seet , const QString userID );
//
//
//	void teleChangeLayoutNotify();//Add By LZY 2010-10-09 ����֪ͨң������Ƶ���ֱ��
//
//	ConferenceRoomService*	getConferenceRoomService( ) { return _qtConferenceRoom->getConferenceRoomService(); }
//
//Q_SIGNALS:
//	void closeVideoWinSinal(QtConfrenceVideoWindow * videoWin);
//
//	void windowResizeSignal( QString userID , int wnd );
//
//	/**
//	 * @brief ��ʾrtcpInfoWidget���´���ͳ����Ϣ
//	 */
//	void updateTranStateSignal( const TranStatInfo& tranStateInfo );
//
//private Q_SLOTS:
//	void closeVideoWinSlot(QtConfrenceVideoWindow * videoWin);
//
//	void setSizeSlot(QtConfrenceVideoWindow *videoWin,int width,int height);
//	void mouseMoveSlot(QtConfrenceVideoWindow *videoWin,const QPoint &point);
//	void unReceiveMedia(QString userId);//Remove Param Const By LZY 2010-09-28
//
	void sizeAll();
	void size3And3();
	void size2And2();
	void size1And1();
	void size1And1Real();
	void sizeL3And3();
	void sizeL4And4();
	void sizeL5And5();
	void sizeAUTO();
    void size4And4();

	void sizeL1_20();
    void sizeTop1And2();
//
//	void addVideoWindow();
//
//	/**
//	 * @brief ����RTCP��Ϣ
//	 */
//	void updateRtcpEventHandler( const RtpStatItemInfo &rtpstatInfo );
//
//
//	/**
//	 * @brief ���´���ͳ����Ϣ
//	 */
//	void updateTranStateEventHandler( const TranStatInfo &tranStatInfo );
//
//
//	/**
//	 * @brief RTCP��Ϣ��ʾ����
//	 * @param visiable true����ʾ��������ʾ
//	 */
//	void showRtcpMessage( bool visiable );
//
	/**
	 * @brief ������ʾģʽ
	 * @param displayModel ��ʾģʽ
	 */
	void updateDisplay( ConfRoomDisplayModel displayModel );
//
//	void teleSetLayoutTypeCommand(QString LayoutType); //Modify By LZY 2010-09-27 ����SetLayoutType����Slot
//
//	///@brief ��ĳ���û�����Ƶ����ȫ��
//	void FullScreenShow( QString userID );
//
//
//	///���ڵ���
//	void windowResizeSlot( QString userID , int wnd );

private:

	Ui::MeetingPlaceWidget *_ui;

	//�ܹ���Ĵ����б�
	std::vector<VideoFrame*>	memberFrames;

	QWidget *widget;
	QGridLayout *mainLayout;

	///@brief �Ƿ�����Ƶ������ʾrtcp��Ϣ
	bool showRtcpMessageEnbaled;

	///��¼��ǰ��ʾģʽ
	ConfRoomDisplayModel _displayModel;

	///�����
	QtConferenceRoom*		_qtConferenceRoom;


};

