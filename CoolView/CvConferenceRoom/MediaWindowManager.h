/**
 * @brief ���ڹ�����Ƶ����
 */

#ifndef MEDIA_WINDOW_MANAGER_H
#define MEDIA_WINDOW_MANAGER_H

#include <QtCore/QtCore>
#include <vector>
#include <dbus/conferenceRoom/common/ConferenceRoomServiceCommon.h>
#include <util/desktop/screen_helper.h>

class VideoFrame;
typedef std::vector<VideoFrame*> MediaWindowVector;
typedef struct _RecStatItem RecStatItem;

class QtConfrenceVideoWindow;
class QtMeetingPlaceWidget;
class RtpStatItemInfo;
class QtConferenceRoom;
class TelecontrollerIf;
class QtScreenShare;
class ChannelDispatcherIf;
class KinectIf;
class KinectInfoWidget;
class CvCoreIf;

class MediaWindowManager : public QObject
{
	Q_OBJECT
public:
	static MediaWindowManager* getInstance()
	{
		static MediaWindowManager manager;
		return &manager;
	}


public slots:
	/** 
	 * @brief ����ָ����������ý�崰��
	 * @return ����ɹ��򷵻�ý�崰�ڣ����򷵻�NULL
	 */
	VideoFrame* AddMediaWindow( const QString& userID , const QString& displayName , const int screenIndex, const int seet
		,const int videoWidth , const int videoHeight );

	/**
	 * @brief ���ý����
	 * @params userID SIP�˺�
	 * @return NULL
	 */
	void AddScreenMediaRecv( const QString& userID , const QString&ip , const int port , const int screenIndex=-1 );

	/**
	 * @brief �����������Ļ������
	 * @params userID SIP�˺�
	 * @return NULL
	 */
	void AddMainSpeakerScreenMediaRecv( const QString& userID , const QString&ip , const int port );

	/**
	 * @brief �Ƴ�ý����
	 * @params userID SIP�˺�
	 * @return NULL
	 */
	void RemoveScreenMediaRecv( const QString& userID,const int screenIndex=-1 );


	/**
	 * @brief �ر�ý�崰��
	 */
	void CloseMediaWindow( const QString& userID );

	/**
	 * @brief �ͷ����е�ý�崰��
	 */
	void ClearMediaWindow( );

	/// ��ʾý�崰��
	void ShowVideoFrame( VideoFrame* frame );

	/// ����ý�崰��
	void HideVideoFrame( VideoFrame* frame );

	/**
	 * @brief ���Ľ���ý������״̬
	 * @param frame ý��������
	 * @param mediaType audioΪ��Ƶ��videoΪ��Ƶ
	 * @param enable ���������ΪTrue������Ϊfalse
	 */
	void ChangeMediaState( const VideoFrame* frame , const QString& mediaType , const UiMediaState mediaState,bool isAudioSend);


	/// ����ý�崰��λ��
	void ChangeWindowPostion( const QString& userID , const int newScreenIndex, const int newSeet );

	/**
	 * @brief ��ʾrtcp��Ϣ
	 */
	void ShowRtcpMessage( const int screenIndex, const bool show );

	/**
	 * @biref ����rtcp��Ϣ
	 */
	//void UpdateRtcpMessage( const RtpStatItemInfo& stateInfo );

	/**
	 * @brief ����rtcp��Ϣ
	 */
	void UpdateRtcpMessage( const RtpStatItem& stateItem);
    void UpdateRecordMessage( const RecStatItem& stateItem );

	/**
	 * @brief ���Ľ��沼��
	 */
	void ChangeLayout( int screenIndex, int displayModel );

	/**
	 * @brief ���ƹ�����Ļ�Ľ���İ�ť�������ǰ�ն�Ϊ��������ѡ���Ļ������ť��ʾ����������������
	 * @param control ���Ϊtrue������ʾ��ť���������ذ�ť
	 */
	void ControlScreenShare( const bool control );

	 /**
	 * @brief ��ָ����Ļ�´�ָ��·���µĹ�����Ļ�ļ�
	 */
	void OpenScreenShareFile(const QString &filePath, int screenIndex);

	 /**
	 * @brief �Ƴ���Ļ�����ͽ���
	 */
	void RemoveSendShareScreen();

	/**
	 * @brief ��ȡָ���û�����ý�崰�ڡ��������߳�ͬ������Ӧ���ɵ����߹���ͬ����
	 * @return ���ʧ���򷵻�NULL
	 */
	VideoFrame* FindMediaWindow( const QString& userID );

	/**
	 * @brief ��ʾ����Ŀ�����ı�ʱ��Ҫ������Ļ�����UI����Ƶ����
	 */
	void ScreenCountChanged( int newCount );

	/**
	 * @brief �������ϵ��û�֪ͨ��ң�������Ա�ͬ��
	 * @param controllerIndex��֪ͨ�¼����ظ�ң����������ֵ��-1��������ң���������յ�ͨ��
	 */
	void MemberLocationUpdateNotify( const int controllerIndex=-1);

	/**
	 * @brief ����Ļ�����״̬������Ϣ֪ͨ��ң�������Ա�ͬ��
	 * @param controllerIndex��֪ͨ�¼����ظ�ң����������ֵ��-1��������ң���������յ�ͨ��
	 */
	void ScreenShareStateUpdateNotify( const int controllerIndex=-1);

	/**
	 * @brief ��������û�����Ϣͨ��������ն�
	 * @param memberName Զ���û������ƣ�����Ǳ��ػ�������LOCAL_PLAY_USER_ID
	 * @param screen ��Ļ������
	 * @param seet ����Զ���û��Ĵ���λ�ã���1��ʼ��
	 * @param audioEnable ����ڽ�����Ƶ������true
	 * @param videoEnable ����ڽ�����Ƶ������true
	 */
	void MemberLocationAddNotify( const QString& memberName , const int screen , const int seet , const bool audioEnable , const bool videoEnable );

	/**
	 * @brief ���Ƴ��û�����Ϣͨ��������ն�
	 */
	void MemberLocationRemoveNotify( const QString& memberName, const int screen , const int seet  );

	/**
	 * @brief ���Ƴ��û�����Ϣͨ��������ն�
	 * @memberName �û��Ļ�Ա��
	 */
	void MemberLocationChangeNotify( const QString& memberName , const int newScreenIndex, const int newSeet );

	/**
	 * @brief ���沼�ָ���ʱ֪ͨң����
	 * @param controllerIndex��֪ͨ�¼����ظ�ң����������ֵ��-1��������ң���������յ�ͨ��
	 */
	void LayoutChangedNofity( const int controllerIndex=-1 );

	/**
	 * @brief ��ý���״̬������Ϣ֪ͨ��ң�������Ա�ͬ��
	 * @param controllerIndex��֪ͨ�¼����ظ�ң����������ֵ��-1��������ң���������յ�ͨ��
	 */
	void MediaStateChangedNotify( const QString&memberName , const QString&mediaType, const QString&mediaState, const int controllerIndex=-1 );

	

	/**
	 * @brief ������Ļ����Ķ˿�
	 */
	void SetScreenPort( const int screenPort ){ _screenPort = screenPort;}
	int GetScreenPort( ){ return _screenPort; }
	/**
	 * @brief ������Ļ����ĳ�ʼ����IP
	 */
	void SetScreenIP( const QString& screenIP ){ _screenIP = screenIP; }
	QString GetScreenIP() { return _screenIP; }

	void KinectGestureSlot(const QString &to, int type);

  void ControlVideoRecording(const QString userId, int type, QByteArray param);

public:	
	bool	isSendingScreen(){ return _isSendingScreen; }
	void	isSendingScreen(bool sendScreen ) { _isSendingScreen = sendScreen; }


private slots:
	void KinectMessageSlot(int type, const QString &message);
    void updateScreenSlot();

private:
	MediaWindowManager();
	~MediaWindowManager();
	
	//memberFrames���ڶ��̻߳��������ã���Ҫͬ���������õĵط�ʵ��̫�࣬Ŀǰֻ��RTCP��ط����м�����
  //���߳�֮�佻���Ѹ����źŲ����ӣ�������Ҫ��
	//QMutex _memberFramesLock;
	MediaWindowVector	_memberFrames;

	//QtMeetingPlaceWidget*	_meetingPlaceWidgets[2];

	//��Ż�����UI���б���С����ʾ������һ��
	QVector<QtConferenceRoom*>	_confRoomVector;

	QtScreenShare*				_screenShareWidget;

	///Kinect��Ϣ����
	KinectInfoWidget*			_kinectInfoWidget;

	//Telecontroller dbus proxy
	TelecontrollerIf*			_teleControlProxy;

	//channeldispatcher dbus proxy
	ChannelDispatcherIf*		_channelDispatcherProxy;

	//kinect dbus proxy
	KinectIf*				_kinectProxy;

  CvCoreIf *_coreProxy;

	//��¼��Ļ����Ķ˿ڣ��ڴ������ػ��Ե�ʱ������
	//���˳������ʱ������
	int							_screenPort;
	QString						_screenIP;

	///�Ƿ����ڷ�����Ļ����
	bool						_isSendingScreen;

  //�Ƿ�������ʾrtcp��Ϣ
  bool				_isShowingRtcp;

  QTimer _updateScreenTimer;
};

#endif