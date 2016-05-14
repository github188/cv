#ifndef MEDIA_MANAGER_H
#define MEDIA_MANAGER_H

#include <dbus/msdx/common/MsdxServiceCommon.h>
#include <dbus/conferenceRoom/common/ConferenceRoomServiceCommon.h>

//rtcp report reader
#include <util/report/RTCPReportReader.h>

class ConferenceRoomIf;
class ChannelDispatcherIf;
class ScreenMediaInfo;
class ConferenceManager;
class CvStreamedMediaIf; 

class IConferenceController;

#include <map>
#include <vector>
using namespace std;

typedef std::map<QString , CvStreamedMediaIf*>   StreamedMediaProxyMap;
#define PORTINCREMENT 200;

class RecvMemberPosition
{
public:
	RecvMemberPosition()
	{
		_memberURI = "";
		_screenIndex = -1;
		_seat = -1;
		_smallVideo = false;
		_rtcpVideoReportReader = new RTCPReportReader(RTCPReportType::ReportType_video);
		_rtcpAudioReportReader = new RTCPReportReader(RTCPReportType::ReportType_audio);
	}

	~RecvMemberPosition()
	{
		_memberURI = "";
		_screenIndex = -1;
		_seat = -1;
		delete _rtcpVideoReportReader;
		_rtcpVideoReportReader = NULL;
		delete _rtcpAudioReportReader;
		_rtcpAudioReportReader = NULL;
	}

	QString				_memberURI;
	int					_screenIndex;
	int					_seat;
	bool                _smallVideo;
	RTCPReportReader*	_rtcpVideoReportReader;
	RTCPReportReader*	_rtcpAudioReportReader;
};

typedef vector<RecvMemberPosition*>		RecvMemberVector;

class MediaManager: public QObject
{
	Q_OBJECT
public:
	static MediaManager* getInstance()
	{
		static MediaManager manager;
		return &manager;
	}

	void setConferenceManager( ConferenceManager* confManager )
	{
		this->_conferenceManager = confManager;
	}

    void setConferenceController(IConferenceController *controller) {
      conference_controller_ = controller;
    }

	//��ӽ����û���λ����Ϣ
	void AddRecvMemberPosition( const QString& memberURI , const int screenIndex , const int seet, const bool smallVideo );
	//ɾ�������û���Ϣ��λ����Ϣ
	void RemoveRecvMemberPosition( const QString& memberURI );
	//�����û�URI��ȡλ����Ϣ
	int GetRecvMemberPositionFromURI( const QString&memberURI , int *screenIndex , int *seet, bool *smallVideo );

	/**
	 * @param memberName URI��@��ߵ��ַ����ҽ�#�滻Ϊ_
	 */
	int GetRecvMemberPositionFromName( const QString &memberName , int *screenIndex , int *seet, bool *smallVideo );
	/**
	 * @brief ������е��û�λ����Ϣ
	 */
	void clearRecvMemberPosition( );

	//��������ý����
	void AddRecvMedia( RecvGraphInfo& info );

	//�Ƴ�����ý����
	void RemoveRecvMedia( const QString& memberURI );

	//�����û�����ʾλ��
	void ChangeMemberSeat( const QString&memberURI , const int newScreenIndex , const int newSeet );

	/** 
	 * @brief ������������Ƶ��
	 * @param ip ý������Ŀ���ַ
	 * @param audioPort ý��������ƵĿ��˿�
	 * @param videoPort ý��������ƵĿ��˿�
	 * @param smallVideo �Ƿ�ʹ��С��
	 * @param screenPort ��Ļ�˿ڣ�����
	 * @param audioEnable �Ƿ�����Ƶ
	 * @param virtualNo  �����ն˱��
	 */
	void CreateSendMedia( int virtualIndex, const QString& ip , const int&audioPort , const int&videoPort , const bool smallVideo, const int&screenPort,const bool audioEnable=true);


	//Tx�ֶο���
	void SegmentMedia( const QString& userId, int job_id );

	/**
	 * @brief ��ӷ���Ŀ��
	 * @param ip ý������Ŀ���ַ
	 * @param audioPort ý��������ƵĿ��˿�
	 * @param videoPort ý��������ƵĿ��˿�
	 * @param smallVideo �Ƿ����С��
	 * @param screenPort ��Ļ�˿ڣ�����
	 * @param audioEnable �Ƿ�����Ƶ
	 */
	void AddSendMedia( int virtualIndex, const QString& ip , const int&audioPort , const int&videoPort , const bool smallVideo, const int&screenPort,const bool audioEnable=true );

	//�Ƴ�����ý����
	void RemoveSendMedia( int virtualIndex, const QString&ip , const int&audioPort , const int&videoPort, const bool isSmallVideo = false );

	//����dscp
	void SetDSCP(const int vdieoDscp , const int audioDscp );

	//��ʾRTCP
	void showRtcpMsg( bool visiable , int screenIndex=-1 );

	//�˳����飬ֹͣ����ý����
	void ExitConference( );

	//��������ý����
	void ClearStreamedMedia();

	//�ı���沼��
	void ChangeLayout(  const ConfRoomDisplayModel displayModel , int screenIndex=-1 );

    //������Ƶ���ķ���
    void ControlVideoSend( bool videoSend );

	//������Ƶ���ķ���
	void ControlVideoSend(int virtualIndex, bool videoSend );

	//������Ƶ���ķ���
	void ControlAudioSend( bool audioSend );

	//������Ƶ���Ľ���
	void ControlVideoRecv( const QString memberURI, const bool videoRecv );

	//������Ƶ���Ľ���
	void ControlAudioRecv( const QString memberURI, const bool audioRecv );

	

	/**
	 * @brief ����Զ���ն˵�SIP��Ϣ�ı��Ӧ����ý����������Ƶ״̬��
	 * @param memberURI Զ���ն˵�SipURI
	 * @param mediaType	ý������
	 * @param state ý��״̬��trueΪ������falseΪ��ֹ
	 * @param isAudioSend ��Ƶ״̬��trueΪ���ط�����Ƶ��falseΪ�����ն���Ƶ
	 */
	void ChangeMediaState( const QString&memberURI , const QString&mediaType , const UiMediaState state ,bool isAudioSend);

	/**
	 * @brief ���ƹ�����Ļ�Ľ���İ�ť�������ǰ�ն�Ϊ��������ѡ���Ļ������ť��ʾ����������������
	 * @param control ���Ϊtrue������ʾ��ť���������ذ�ť
	 */
	void ControlScreenShare( const bool control );

	/**
	 * @brief ����ָ����������������Ļ��
	 * @param port ��Ļ��ռ�õĶ˿�
	 * @dst Ŀ���ַ����������鲥������Ϊ"0.0.0.0"
	 * @wnd �������Ļ����
	 */
	void CreateSendScreenMedia( const int port ,const QString&dst, const QRect wnd );

	/**
	 * @brief ����ָ����������������Ļ��
	 * @param port ��Ļ��ռ�õĶ˿�
	 * @dst Ŀ���ַ����������鲥������Ϊ"0.0.0.0"
	 * @wnd �������Ļ����
	 */
	void CreateRecvScreenMedia( const QString&memberURI , const int port ,const QString&src, const int wnd, const int screenIndex );

	/**
	 * @brief �����Ļ���ķ��͵�ַ
	 * @param port ��Ļռ�õĶ˿�
	 * @param dst Ŀ���ַ,
	 */
	void AddSendScreenMediaDst( const int port , const QString&dst );

	/**
	 * @brief ɾ����Ļ���ķ��͵�ַ
	 * @param port ��Ļռ�õĶ˿�
	 * @param dst Ŀ���ַ,
	 */
	void RemoveSendScreenMediaDst( const int port , const QString&dst );

	/**
	 * @brief �رս�����Ļ��
	 * @param info ��Ļ����Ϣ
	 */
	void RemoveRecvScreenMedia( const QString&remoteURI="");


	/**
	 * @brief �Ƿ���������Ƶ�����Զ�ͬ��ƫ�����, �����ý������н����������á�
	 *			  ʵ��ԭ���Ǽ����ջ������������Զ���������ջ��壬������������Ƶ����ͬ��
	 * @param enable true=���� false=����
	 */
	void EnableRecvAutoResync(bool enable);

    //�Ƿ���������Ӧ���ʿ��� -- ���Ͷ�
    // �÷�����ʵ��ֻ��ý�������ͽ���CvStreamedMedia_send�Ѿ������˲���Ч
    void EnableAdaptiveCodeRate(bool enable);


	//��ȡ���ػ��Ե���Ļ
	int GetLocalPreviewScreen() const { return _localPreviewScreen; }

	/**
	 * @brief ����ָ���������������˵Ľ�����Ļ��
	 * @param port ��Ļ��ռ�õĶ˿�
	 * @dst Ŀ���ַ����������鲥������Ϊ"0.0.0.0"
	 * @wnd �������Ļ����
	 */
	void CreateMainSpeakerRecvScreenMedia( const QString&memberURI , const int port ,const QString&src, const int wnd);

	 /**
	 * @brief �����Ƴ���Ļ�����ͽ���
	 */
	void RemoveScreenShareSend();

	 /**
	 * @brief ִ��PPTҳ����Ĳ���
	 * @param  type ҳ��������ͣ�����ǰ��ҳ�������ҳ
	 */
	void PPTControlCommand(int type);

	//�ָ�����ý����
	void RecoveryRecvMedia( RecvGraphInfo& info );

Q_SIGNALS:
    void NotifySendMediaStateChanged(const QString &media_id, int type);
    void NotifyRecvMediaStateChanged(const QString &user_id, int type);

private slots:
	void rtcpReportTimerTimeout();
    void HandleNofityChannelStateChanged(const QString &channel_id, 
        const QString &user_id, const QString &channel_type, int channel_state);

private:
	MediaManager();
	~MediaManager();
	//�����·����streammedia�Ĵ������
	StreamedMediaProxyMap _sendMediaProxyMap;

	ConferenceRoomIf*		_confRoomProxy;
	ChannelDispatcherIf*	_channelDispatcherProxy;

	RecvMemberVector	_recvMemberVector;

	//�Ƿ��ڷ�������Ƶ
	bool			_isSendingMedia;

	//��¼��ǰ���յ���Ļ����Զ���û�URI
	QString			_remoteScreenMember;

	//��¼DSCPֵ
	int _videoDscp;   //��Ƶ��dscp
	int _audioDscp;  //��Ƶ��dscp


	///��ȡRTCP����ļ�ʱ��
	QTimer*			_rtcpReportTimer;

	///����������
	ConferenceManager*	_conferenceManager;

    // ������ƶ���
    IConferenceController *conference_controller_;

	//���ػ��Դ������ڵ�λ��
	int				_localPreviewScreen;
	//int				_localPreviewSeet;

	// Tx_temp - ��ֹ��Ƶ���ָ�
	QDateTime       _lastFileCutTime;
  bool _isModelTx;
};
#endif