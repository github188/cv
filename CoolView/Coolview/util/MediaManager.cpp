#include "MediaManager.h"

#include <dbus/conferenceRoom/client/ConferenceRoomIf.h>
#include <dbus/conferenceRoom/common/ConferenceRoomServiceCommon.h>

#include <dbus/channel/type/streamedMedia/client/CvStreamedMediaIf.h>
#include <dbus/channel/type/streamedMedia/common/StreamedMediaServiceCommon.h>

#include <dbus/channelDispatcher/client/ChannelDispatcherIf.h>
#include <dbus/channelDispatcher/common/ChannelDispatcherServiceCommon.h>

#include <dbus/channel/type/screenMedia/common/ScreenMediaServiceCommon.h>
#include <dbus/channel/type/screenMedia/client/CvScreenMediaIf.h>

#include <QApplication>
#include <QDesktopWidget>

#include "../config/RunningConfig.h"
#include "config/config_helper.h"
#include "../profile/RunningProfile.h"

#include "ConferenceManager.h"
#include <util/ProcessManager.h>

#include <log/Log.h>

#include "conference_controller_interface.h"


// ��Ҫ��recordstatuswidget.h���MAX_STR_PER_TERMINAL
//const int TERMINAL_STR_NO = 4;
//
//const int port_increment = 2000;



// �ú�����uri��virtualURIת��Ϊ@ǰ����user_id
QString uriToUserId(const QString &uriStr)
{
	QString user_id = uriStr;
	if (user_id.indexOf("@") != -1)
	{
		user_id = user_id.left(user_id.indexOf("@"));
	}
	return user_id;
}

MediaManager::MediaManager()
{

	_confRoomProxy = new ConferenceRoomIf( CONF_ROOM_SERVICE_NAME , CONF_ROOM_SERVICE_OBJECT_PATH , QDBusConnection::sessionBus());
	_channelDispatcherProxy = new ChannelDispatcherIf( CHANNEL_DISPATCHER_SERVICE_NAME , CHANNEL_DISPATCHER_OBJECT_PATH ,
		QDBusConnection::sessionBus() );

    connect(_channelDispatcherProxy, &ChannelDispatcherIf::NofityChannelStateChanged,
        this, &MediaManager::HandleNofityChannelStateChanged);

	int screenCount = QApplication::desktop()->screenCount();
	_localPreviewScreen = 0;
	//_localPreviewSeet = 6;
	//������Ե�λ����Ϣ
	//this->AddRecvMemberPosition( RunningProfile::getInstance()->user_uri().c_str() , _localPreviewScreen, _localPreviewSeet );
	
	_isSendingMedia = false;
	_remoteScreenMember = "";
	_videoDscp = 0;
	_audioDscp = 0;
	_conferenceManager = NULL;

	_rtcpReportTimer = new QTimer(this);
	connect(_rtcpReportTimer , SIGNAL(timeout() ) , this , SLOT(rtcpReportTimerTimeout() ) );

	_lastFileCutTime = QDateTime::currentDateTime();
  _isModelTx = CVIniConfig::getInstance().IsModelTX();

    conference_controller_ = nullptr;
}


MediaManager::~MediaManager()
{
	disconnect(_rtcpReportTimer , SIGNAL(timeout() ) , this , SLOT(rtcpReportTimerTimeout() ) );
	delete _rtcpReportTimer;
	_rtcpReportTimer = NULL;
}


void MediaManager::AddRecvMedia( RecvGraphInfo& info )
{
	QByteArray bytes;
	QDataStream out( &bytes , QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );

	int screenIndex;
	int seet;
	bool smallVideo;

	//����id
	info.user_id = uriToUserId(info.user_id);
	out << info;

	GetRecvMemberPositionFromName( info.user_id, &screenIndex , &seet, &smallVideo);

	// Tx��ConfRoom��������ң����ͬ����������ý�������ô��ƹ�ConfRoomֱ�ӿ��Ƶز�
	if (_isModelTx || screenIndex == -1 || seet == -1)
	{
		_channelDispatcherProxy->CreateChannel( userID2MediaID(info.user_id), QString(STREAMED_MEDIA_SERVICE_NAME) , bytes);
	}
  if (screenIndex > -1 && seet > -1)
	{
		_confRoomProxy->AddMediaWindow( false , bytes , screenIndex , seet );
	}
}

void MediaManager::RemoveRecvMedia( const QString& memberUri )
{
	QString user_id = uriToUserId(memberUri);
	int screenIndex;
	int seet;
	bool smallVideo;
	GetRecvMemberPositionFromName( user_id, &screenIndex , &seet, &smallVideo);

	if (CVIniConfig::getInstance().IsModelTX() || screenIndex == -1 || seet == -1)
	{
		_channelDispatcherProxy->ReleaseChannel(userID2MediaID(user_id), QString(STREAMED_MEDIA_SERVICE_NAME));
	}
	if (screenIndex > -1 && seet > -1)
	{
		_confRoomProxy->CloseMediaWindow( user_id );
	}
}


void MediaManager::SegmentMedia( const QString& memberUri, int job_id )
{
  QString userId = uriToUserId(memberUri);
	if (isSendMediaID(userId))
	{
		QDateTime curTime = QDateTime::currentDateTime();
		if (_lastFileCutTime.secsTo(curTime) <= 10) // ���ηָ�С��10�룬�����
		{
			return;
		}
		_lastFileCutTime = curTime;

		RunningConfig* config = RunningConfig::Instance();
		int numberOfStreams = config->VideoCaptureDeviceCount();

		for (int i = 0; i < numberOfStreams; i++)
		{
			QString mediaID = getSendMediaID(i);

			QByteArray recvByteArray;
			QDataStream out( &recvByteArray , QIODevice::WriteOnly );
			out << QString("");
			_channelDispatcherProxy->ModifyChannel(mediaID
				, QString(STREAMED_MEDIA_SERVICE_NAME), Action_CutRecordingVideo, recvByteArray);
		}
	}
	else
	{
		CutRecordInfo info;
    info.job_id = job_id;

		QByteArray recvByteArray;
		QDataStream out( &recvByteArray , QIODevice::WriteOnly );
		out << info;
		_channelDispatcherProxy->ModifyChannel(userID2MediaID(userId)
			, QString(STREAMED_MEDIA_SERVICE_NAME), Action_CutRecordingVideo, recvByteArray);
		//}
	}
}

/** 
 * @brief ������������Ƶ��
 * @param ip ý������Ŀ���ַ
 * @param audioPort ���ػ���ý��������ƵĿ��˿ڣ�ӦΪ0
 * @param videoPort ���ػ���ý��������ƵĿ��˿�
 * @param smallVideo �Ƿ�ʹ��С��
 * @param screenPort ��Ļ�˿ڣ�����
 * @param audioEnable �Ƿ�����Ƶ
 */
void MediaManager::CreateSendMedia( int virtualIndex, const QString& ip , const int&audioPort , const int&videoPort , const bool smallVideo, const int&screenPort,const bool audioEnable)
{
	if( true/*false == _isSendingMedia*/ ) // TODO:Ӧ���������ͷ������
	{
		RunningConfig* config = RunningConfig::Instance();
		const int numberOfStreams = config->VideoCaptureDeviceCount();
		QString mediaID = getSendMediaID(virtualIndex);

		/*for (int i = 0; i < numberOfStreams; i++)
		{*/
		VCapSetting captureDevice = config->VideoCaptureDevice(virtualIndex);
        
    int aec_type = 0;
    ConfigHelper::GetAudioInputTypeByUIOption(config->AudioInputType(), aec_type);
    if (aec_type < 0) aec_type = 0;

		SendGraphInfo sendGraph;

		//�����������ͷ��������Ƶ��·
		if (virtualIndex == 0)
		{
			//int screenIndex;
			//int seet;
			//GetRecvMemberPositionFromName( LOCAL_PLAY_USERID , &screenIndex , &seet ); // �Ѳ��� - Liaokz, 2013-10

			//SendGraphInfo sendGraph;
			sendGraph.initial( screenPort, qPrintable(captureDevice.devicePath), qPrintable(config->AudioCaptureDevice()) ,
				qPrintable(config->AudioOutputDevice()) , aec_type,
                qPrintable(ip), audioPort/*9000 + 10 * i*/ , qPrintable(config->AudioCodec()) , 
				config->AudioSampleRate(), config->AudioBitsPerSample(), config->AudioChannel(), audioEnable,
				qPrintable(ip), videoPort/*9100 + 10 * i*/, captureDevice.fps, smallVideo, 0 , captureDevice.width , captureDevice.height, 
				qPrintable(captureDevice.crossBarDisplayName), qPrintable(captureDevice.crossBarType));
			sendGraph.dscp_info.video_dscp = _videoDscp;
			sendGraph.dscp_info.audio_dscp =_audioDscp;
			sendGraph.preview_wnd = NULL;

			/*QByteArray output_array;
			QDataStream out( &output_array , QIODevice::WriteOnly );
			out.setVersion( QDataStream::Qt_4_4 );
			out << sendGraph;*/

			//_confRoomProxy->AddMediaWindow( true , output_array , screenIndex ,seet ); // �Ѳ��� - Liaokz, 2013-10

			//mediaID = QString(SEND_STREAMED_MEDIA_ID);

			_rtcpReportTimer->start(5000);
		}
		//������ͷ��ֻ����������Ƶ������·��û����Ƶ��
		else
		{
			//SendGraphInfo sendGraph;
			sendGraph.initial( screenPort, qPrintable(captureDevice.devicePath), NULL/*qPrintable(config->AudioCaptureDevice())*/ ,
				NULL/*qPrintable(config->AudioOutputDevice())*/ , aec_type, qPrintable(ip), audioPort/*9000 + 10 * i*/ , NULL/*"SPEEX"*/ , 16000,16,1,false/*audioEnable*/,
				qPrintable(ip), videoPort/*9100 + 10 * i*/, captureDevice.fps, smallVideo, 0 , captureDevice.width , captureDevice.height, qPrintable(captureDevice.crossBarDisplayName),
				qPrintable(captureDevice.crossBarType));
			sendGraph.dscp_info.video_dscp = _videoDscp;
			sendGraph.dscp_info.audio_dscp =_audioDscp;
			sendGraph.preview_wnd = NULL;

			//mediaID = QString(SEND_STREAMED_MEDIA_ID) + QString::number(captureDevice.deviceNo);
		}

		QByteArray output_array;
		QDataStream out( &output_array , QIODevice::WriteOnly );
		out.setVersion( QDataStream::Qt_4_4 );
		out << sendGraph;

		_channelDispatcherProxy->CreateChannel(mediaID, STREAMED_MEDIA_SERVICE_NAME, output_array);
		if (smallVideo)
		{
			QString smallMediaID = getSmallVideoMediaID(virtualIndex);
			_channelDispatcherProxy->CreateChannel(smallMediaID, STREAMED_MEDIA_SERVICE_NAME, output_array);
		}

		//����mediaID��ӳ���ϵ
		QString serviceName = QString( STREAMED_MEDIA_SERVICE_NAME ) + QString(".") + mediaID;
		QString objectPath = QString( STREAMED_MEDIA_SERVICE_OBJECT_PATH ) + QString("/") + mediaID;
		CvStreamedMediaIf* streamedMediaIf = new CvStreamedMediaIf( serviceName, objectPath,QDBusConnection::sessionBus() );
		_sendMediaProxyMap.insert(StreamedMediaProxyMap::value_type( mediaID , streamedMediaIf ) );
		//}

		_isSendingMedia = true; // TODO��Ӧ���������ͷʱ������

	}
	else
	{
		CV_LOG_ERROR("����ý�����Ѿ������ɹ�");
	}
}


/**
 * @brief ��ӷ���Ŀ��
 * @param ip ý������Ŀ���ַ
 * @param audioPort ý��������ƵĿ��˿�
 * @param videoPort ý��������ƵĿ��˿�
 * @param smallVideo �Ƿ����С��
 * @param screenPort ��Ļ�˿ڣ�����
 * @param audioEnable �Ƿ�����Ƶ
 */
void MediaManager::AddSendMedia( int virtualIndex, const QString& ip , const int&audioPort , const int&videoPort ,const bool smallVideo,const int&screenPort, const bool audioEnable)
{
  if( !_isSendingMedia ) {
    CV_LOG_ERROR("����ý������δ�������޷�����µķ���Ŀ��");
  }

  //�ȴ�����ý�������̵����������ڷ���ý�������������������ģ���˿��Բ�������Ĺ�����ʽ
  /*while(!ProcessManager::isRunning(TEXT("CvStreamedMedia.exe") ) )
  {
	  Sleep(2000);
  }*/
    
  // TODO: ��ʱ����
  // ����ֻ��virtualIndexΪ0��streamMedia������Ƶ, �����Ҫ�ֿ���������Ƶ
  // ������Ƶ���͵���virtualIndexΪ0����Ƶ��
  // ����Ƶ���͵���virtualIndex����Ƶ���ִ�С����

  // ����virtualIndex��Ӧ��ý����������������Ƶ��
  NetInfo netInfo;
  netInfo.ip_addr = ip;
  netInfo.audio_port = 0; // ��ָ����Ƶ
  netInfo.video_port = videoPort;
  netInfo.enable_small_video = smallVideo;
    
  QByteArray output_array;
  QDataStream out( &output_array , QIODevice::WriteOnly );
  out.setVersion( QDataStream::Qt_4_4 );
  out << netInfo;

  if (smallVideo) {
    // ����virtualIndex��Ӧ��С��ý����������������Ƶ��
    CvStreamedMediaIf streamedMeidaProxy( getStreamedMediaServiceName(getSmallVideoMediaID(virtualIndex)) ,
      getStreamedMediaObjectPath( getSmallVideoMediaID(virtualIndex) ),QDBusConnection::sessionBus() );
    streamedMeidaProxy.SetMediaInfo( Action_AddSendDst , output_array );
  } else {
    // ����virtualIndex��Ӧ�Ĵ���ý����������������Ƶ��
    CvStreamedMediaIf * pSendMediaProxy = NULL;
    auto it = _sendMediaProxyMap.find(getSendMediaID(virtualIndex));
    if (it != _sendMediaProxyMap.end()) {
      pSendMediaProxy = (*it).second;
      pSendMediaProxy->SetMediaInfo( Action_AddSendDst , output_array );
    } else {
      // TODO: δ�����и�ý����
      // д��־
      return;
    }
  }

  if (audioEnable) {
    // ����virtualIndexΪ0��ý����������������Ƶ��
    CvStreamedMediaIf *stream_media_with_audio = NULL;
    auto it = _sendMediaProxyMap.find(getSendMediaID(0));
    if (it != _sendMediaProxyMap.end()) {
      stream_media_with_audio = (*it).second;

      NetInfo info;
      info.ip_addr = ip;
      info.audio_port = audioPort;
      info.video_port = 0; // ��ָ����Ƶ��
      info.enable_small_video = false;

      QByteArray output_array;
      QDataStream out( &output_array , QIODevice::WriteOnly );
      out.setVersion( QDataStream::Qt_4_4 );
      out << info;

      stream_media_with_audio->SetMediaInfo(Action_AddSendDst, output_array);
    }
  }
}


void MediaManager::RemoveSendMedia( int virtualIndex, const QString&ip , const int&audioPort , const int&videoPort,  const bool isSmallVideo )
{
  // TODO: ��ʱ����
  // ����ֻ��virtualIndexΪ0��streamMedia������Ƶ, �����Ҫ�ֿ���������Ƶ
  // ������Ƶ���͵���virtualIndexΪ0����Ƶ��
  // ����Ƶ���͵���virtualIndex����Ƶ���ִ�С����

  NetInfo info;
  info.ip_addr = ip;
  info.audio_port = 0; // ��ָ����Ƶ��
  info.video_port = videoPort;
  info.enable_small_video = isSmallVideo;

  QByteArray output_bytes;
  QDataStream out(&output_bytes , QIODevice::WriteOnly );
  out.setVersion( QDataStream::Qt_4_4 );
  out << info;

  if (isSmallVideo) {
    //  ����virtualIndex��Ӧ��С��ý������ֹͣ��������Ƶ��ָ����Ŀ�ꡣ
    CvStreamedMediaIf streamedMeidaProxy( getStreamedMediaServiceName(getSmallVideoMediaID(virtualIndex)) ,
		getStreamedMediaObjectPath( getSmallVideoMediaID(virtualIndex) ),QDBusConnection::sessionBus() );
	streamedMeidaProxy.SetMediaInfo( Action_RemoveSendDst , output_bytes );
  } else {
    //  ����virtualIndex��Ӧ�Ĵ���ý������ֹͣ��������Ƶ��ָ����Ŀ�ꡣ
    CvStreamedMediaIf * pSendMediaProxy = NULL;
    auto it = _sendMediaProxyMap.find(getSendMediaID(virtualIndex));
    if (it != _sendMediaProxyMap.end()) {
      pSendMediaProxy = (*it).second;
      pSendMediaProxy->SetMediaInfo(Action_RemoveSendDst, output_bytes);
    } else {
      // δ�����и�ý����
      // TODO��д��־
      return;
    }
  }

  // ����virtualIndexΪ0��ý��������ֹͣ��������Ƶ����ָ����Ŀ�ꡣ
  CvStreamedMediaIf *stream_media_with_audio = NULL;
  auto it = _sendMediaProxyMap.find(getSendMediaID(0));
  if (it != _sendMediaProxyMap.end()) {
    stream_media_with_audio = (*it).second;

    NetInfo info;
    info.ip_addr = ip;
    info.audio_port = audioPort;
    info.video_port = 0; // ��ָ����Ƶ��
    info.enable_small_video = false;

    QByteArray output_array;
    QDataStream out( &output_array , QIODevice::WriteOnly );
    out.setVersion( QDataStream::Qt_4_4 );
    out << info;

    stream_media_with_audio->SetMediaInfo(Action_RemoveSendDst, output_array);
  }
}



void MediaManager::ChangeMemberSeat( const QString&memberURI , const int newScreenIndex , const int newSeet )
{
	QString user_id = uriToUserId(memberURI);
	QByteArray output_bytes;
	QDataStream out( &output_bytes , QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << newScreenIndex << newSeet;
	_confRoomProxy->ModifyMediaWindow( user_id , MWA_ChangeWindowPosition, output_bytes );

	//if( oldScreenIndex!=newScreenIndex )
	//{
	//	//����û�����ʾ��Ļ�����˸ı䣬��ôӦ�ùر�ԭ�еĴ���
	//	RemoveRecvMedia( userID );
	//}

	////Ȼ������µ�ý�崰��
	//AddRecvMemberPosition( userID , newScreenIndex , newSeet );
	//RecvGraphInfo info;
	//info.initial( "" , 0 , "" , 0 , 0 , 0 , "", 0 , 0 , 0 ,0 ,qPrintable(userID) );
	//AddRecvMedia( info);
}

void MediaManager::ExitConference()
{
	for( RecvMemberVector::iterator it = _recvMemberVector.begin() ; it!=_recvMemberVector.end() ;it++ )
	{
		RecvMemberPosition * pos = (*it);
		if (pos == NULL || isLocalPreviewMediaID(pos->_memberURI) ||
        isLocalRecordMediaID(pos->_memberURI))
		{
			continue;
		}
		RemoveRecvMedia(pos->_memberURI);
	}

  if (_isModelTx) {
    _channelDispatcherProxy->Destroy();
  }
	_confRoomProxy->ExitConference();
	//_isSendingMedia = false;
	
	_rtcpReportTimer->stop();
	//����û���λ����Ϣ
	clearRecvMemberPosition();
}


void MediaManager::SetDSCP( const int vdieoDscp , const int audioDscp )
{
	_videoDscp = vdieoDscp;
	_audioDscp = audioDscp;

	//��������ִ��ֻ��ý�������ͽ���CvStreamedMedia_send�Ѿ�����ʱ����Ч
	QosDscpInfo info;
	info.video_dscp = vdieoDscp;
    info.audio_dscp = audioDscp;
	QByteArray byteArray;
	QDataStream out( &byteArray , QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << info;
	//��ȡ����ý����̵ķ���ӿڣ�Ŀǰqosֻ��Է��ͽ���
	CvStreamedMediaIf streamedMediaIf( getStreamedMediaServiceName(QString(SEND_STREAMED_MEDIA_ID) ) ,
		getStreamedMediaObjectPath(QString(SEND_STREAMED_MEDIA_ID)) , QDBusConnection::sessionBus() );
	streamedMediaIf.SetMediaInfo( Action_SetQoSInfo , byteArray );
}



void MediaManager::showRtcpMsg( bool visiable , int screenIndex/*=-1 */ )
{
	_confRoomProxy->ShowRtcpMsg( screenIndex , visiable );
}

void MediaManager::ChangeLayout( const ConfRoomDisplayModel displayModel , int screenIndex/*=-1 */ )
{
    static int try_times = 15; //���ڵ�һ�γ��Եȴ�
    while (try_times-- > 0) {
        //֮ǰ������_confRoomProxy->isValid()�ж�����������Ч���ο��źŲ�ԭ��
        if (ProcessManager::isRunning(TEXT("CvConferenceRoom.exe")) && 
            ProcessManager::isRunning(TEXT("CvTelecontrollerSocket.exe"))) {
            try_times = 0;
            Sleep(200); //ȷ����ע��
            break;
        }
        Sleep(200);
    }
    _confRoomProxy->ChangeLayout( screenIndex , displayModel );
}

void MediaManager::AddRecvMemberPosition( const QString& memberURI , const int screenIndex , const int seet, const bool smallVideo )
{
	RecvMemberVector::iterator it = _recvMemberVector.begin();
	for(  ; it!=_recvMemberVector.end() ; it++ )
	{
		if( (*it)->_memberURI == memberURI )
		{
			//���µ�ǰ��Ƶ����λ��
			(*it)->_screenIndex = screenIndex;
			(*it)->_seat = seet;
			(*it)->_smallVideo = smallVideo;
			break;
		}
	}
	if( it==_recvMemberVector.end() )
	{
		//��û�н��ո��û�������ӽ�ȥ
		RecvMemberPosition* recvMember = new RecvMemberPosition();
		recvMember->_memberURI = memberURI;
		recvMember->_screenIndex = screenIndex;
		recvMember->_seat = seet;
		recvMember->_smallVideo = smallVideo;


		string mediaID;
		if( isLocalPreviewMediaID(memberURI) )
		{
			int index = getVirtualIndexFromLocalPreviewMediaID(memberURI);
			mediaID = getSendMediaID(index).toStdString();
		}else
		{
			QString user_id = uriToUserId(memberURI);
			mediaID = userID2MediaID(user_id).toStdString();
		}
		recvMember->_rtcpVideoReportReader->setReader( mediaID );
		recvMember->_rtcpAudioReportReader->setReader( mediaID );
		_recvMemberVector.push_back( recvMember );
	}
}

void MediaManager::RemoveRecvMemberPosition( const QString& memberURI )
{
	RecvMemberVector::iterator it = _recvMemberVector.begin();
	for(  ; it!=_recvMemberVector.end() ; it++ )
	{
		if( (*it)->_memberURI == memberURI )
		{
			RecvMemberPosition * recvMember = *it;
			if (recvMember)
			{
				delete recvMember;
			}
			_recvMemberVector.erase( it );
			break;
		}
	}
}

int MediaManager::GetRecvMemberPositionFromURI( const QString&memberURI , int* screenIndex , int* seet, bool *smallVideo )
{
	RecvMemberVector::iterator it = _recvMemberVector.begin();
	for(  ; it!=_recvMemberVector.end() ; it++ )
	{
		if( (*it)->_memberURI == memberURI )
		{
			if (screenIndex) *screenIndex = (*it)->_screenIndex;
			if (seet) *seet = (*it)->_seat;
			if (smallVideo) *smallVideo = (*it)->_smallVideo;
			return 1;
		}
	}
	if( it==_recvMemberVector.end() )
	{
		if (screenIndex) *screenIndex=-1;
		if (seet) *seet = -1;
		if (smallVideo) *smallVideo = false;
	}
	return -1;
}


int MediaManager::GetRecvMemberPositionFromName( const QString &userId , int* screenIndex , int* seet, bool *smallVideo )
{
	RecvMemberVector::iterator it = _recvMemberVector.begin();

	for(  ; it!=_recvMemberVector.end() ; it++ )
	{
		if ((*it) == NULL)
		{
			continue;
		}
		QString user_id_to_check = uriToUserId((*it)->_memberURI);

		if( user_id_to_check == userId )
		{
			if (screenIndex) *screenIndex = (*it)->_screenIndex;
			if (seet) *seet = (*it)->_seat;
			if (smallVideo) *smallVideo = (*it)->_smallVideo;
			return 1;
		}
	}
	if( it==_recvMemberVector.end() )
	{
		if (screenIndex) *screenIndex=-1;
		if (seet) *seet = -1;
		if (smallVideo) *smallVideo = false;
	}
	return -1;
}

void MediaManager::ControlVideoSend(int virtualIndex, bool videoSend )
{
	QByteArray byteArray;
	QDataStream out( &byteArray , QIODevice::WriteOnly );
	out.setVersion(QDataStream::Qt_4_4);
	out << videoSend;

	//��ȡ����ý����̵ķ���ӿڣ�Ŀǰqosֻ��Է��ͽ���
    
	CvStreamedMediaIf streamedMediaIf( getStreamedMediaServiceName(getSendMediaID(virtualIndex)),
		getStreamedMediaObjectPath(QString(SEND_STREAMED_MEDIA_ID)) , QDBusConnection::sessionBus() );
	streamedMediaIf.SetMediaInfo( Action_ControlVideoSend , byteArray );

	QString localID = getLocalPreviewMediaID(virtualIndex);
	if( videoSend )
		ChangeMediaState( localID , UiStateTypeVideo , UiMediaState_Run, true);
	else
		ChangeMediaState( localID , UiStateTypeVideo , UiMediaState_Stop,true);
}

void MediaManager::ControlVideoSend( bool videoSend ) {
  QByteArray byteArray;
  QDataStream out( &byteArray , QIODevice::WriteOnly );
  out.setVersion(QDataStream::Qt_4_4);
  out << videoSend;

  //��ȡ����ý����̵ķ���ӿڣ�Ŀǰqosֻ��Է��ͽ���
  CvStreamedMediaIf streamedMediaIf( getStreamedMediaServiceName(QString(SEND_STREAMED_MEDIA_ID) ) ,
    getStreamedMediaObjectPath(QString(SEND_STREAMED_MEDIA_ID)) , QDBusConnection::sessionBus() );
  streamedMediaIf.SetMediaInfo( Action_ControlVideoSend , byteArray );

  for (int i = 0; i < RunningConfig::Instance()->VideoCaptureDeviceCount(); ++i) {
    QString localID = getLocalPreviewMediaID(i);
    if( videoSend )
      ChangeMediaState( localID , UiStateTypeVideo, UiMediaState_Run, true);
    else
       ChangeMediaState( localID , UiStateTypeVideo, UiMediaState_Stop,true);
  }
}

void MediaManager::ControlAudioSend( bool audioSend )
{
	QByteArray byteArray;
	QDataStream out( &byteArray , QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << audioSend;

	//��ȡ����ý����̵ķ���ӿڣ�Ŀǰqosֻ��Է��ͽ���
	CvStreamedMediaIf streamedMediaIf( getStreamedMediaServiceName(QString(SEND_STREAMED_MEDIA_ID) ) ,
		getStreamedMediaObjectPath(QString(SEND_STREAMED_MEDIA_ID)) , QDBusConnection::sessionBus() );
	streamedMediaIf.SetMediaInfo( Action_ControlAudioSend , byteArray );

	for (int i = 0; i < RunningConfig::Instance()->VideoCaptureDeviceCount(); ++i)
	{
		QString localID = getLocalPreviewMediaID(i);
		if( audioSend )
			ChangeMediaState( localID , UiStateTypeAudio, UiMediaState_Run, true);
		else
			ChangeMediaState( localID , UiStateTypeAudio, UiMediaState_Stop,true);
	}
}

//ע��:�˷�������TX���������˳�ʱ���ý����.�����ͨ�ն˵���,�����·��ͽ��̱���ֹ!
void MediaManager::ClearStreamedMedia()
{
	_remoteScreenMember = "";
	//TODO
	while (!_recvMemberVector.empty())
	{
		RecvMemberVector::iterator it = _recvMemberVector.begin();
		RecvMemberPosition * pos = *it;
		if (NULL == pos)
		{
			_recvMemberVector.erase(it);
			continue;
		}
		RemoveRecvMedia(pos->_memberURI);
		RemoveRecvMemberPosition(pos->_memberURI);
	}
	_channelDispatcherProxy->Destroy();
	_sendMediaProxyMap.clear();
	_isSendingMedia = false;
	//ExitConference();
}

void MediaManager::ChangeMediaState( const QString&memberURI , const QString&mediaType , const UiMediaState state ,bool isAudioSend)
{
	QString user_id = uriToUserId(memberURI);

	QByteArray byteArray;
	QDataStream out( &byteArray , QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out<<mediaType<<state<<isAudioSend;

	_confRoomProxy->ModifyMediaWindow( user_id ,MWA_ChangeMediaState, byteArray);
}


void MediaManager::CreateSendScreenMedia( const int port ,const QString&dst, const QRect wnd )
{
	ScreenMediaInfo info;
	info._port = port;
	info._ipAddress = dst;
	info._screenWnd = wnd;

	QByteArray outputBytes;
	QDataStream out(&outputBytes,QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << info;
	_channelDispatcherProxy->CreateChannel( SEND_SCREEN_MEDIA_ID, SCREEN_MEDIA_SERVICE_NAME,
		outputBytes );

}

void MediaManager::AddSendScreenMediaDst( const int port , const QString&dst )
{
	QByteArray outputBytes;
	QDataStream out(&outputBytes,QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << dst << port;
	CvScreenMediaIf proxy( getScreenMediaServiceName(SEND_SCREEN_MEDIA_ID),
		getScreenMediaObjectPath(SEND_SCREEN_MEDIA_ID) , QDBusConnection::sessionBus() );
	proxy.SetMediaInfo( ScreenMediaAction_AddSendDst , outputBytes );
}

void MediaManager::RemoveSendScreenMediaDst( const int port , const QString&dst )
{
	QByteArray outputBytes;
	QDataStream out(&outputBytes,QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << dst << port;
	CvScreenMediaIf proxy( getScreenMediaServiceName(SEND_SCREEN_MEDIA_ID),
		getScreenMediaObjectPath(SEND_SCREEN_MEDIA_ID) , QDBusConnection::sessionBus() );
	proxy.SetMediaInfo( ScreenMediaAction_RemoveSendDst , outputBytes );
}

void MediaManager::RemoveRecvScreenMedia( const QString&remoteURI )
{
	QByteArray outputBytes;
	QDataStream out(&outputBytes,QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	_confRoomProxy->ModifyMediaWindow( remoteURI ,MWA_RemoveScreenRecv , outputBytes );
}

void MediaManager::CreateRecvScreenMedia( const QString&memberURI , const int port ,const QString&src, const int wnd, const int screenIndex )
{

	QByteArray outputBytes;
	QDataStream out(&outputBytes,QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << src << port << screenIndex;

	_confRoomProxy->ModifyMediaWindow( memberURI , MWA_AddScreenRecv, outputBytes );
}

void MediaManager::EnableRecvAutoResync( bool enable )
{
	QByteArray byteArray;
	QDataStream out( &byteArray , QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << enable;
	_channelDispatcherProxy->ModifyChannel("", STREAMED_MEDIA_SERVICE_NAME, MMT_EnableAutoResync, byteArray);
}

void MediaManager::ControlVideoRecv( const QString memberURI, const bool videoRecv )
{
	QString memberName = memberURI;
	int index= memberName.indexOf("@");
	if(  index!=-1 )
		memberName = memberName.left(index);

	QByteArray byteArray;
	QDataStream out( &byteArray , QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << videoRecv;

	QString mediaID = userID2MediaID( memberName );
	CvStreamedMediaIf streamProxy( getStreamedMediaServiceName(mediaID ), 
		getStreamedMediaObjectPath(mediaID), QDBusConnection::sessionBus() );
	streamProxy.SetMediaInfo( Action_ControlVideoRecv , byteArray );

	if( videoRecv )
		ChangeMediaState( memberURI , UiStateTypeVideo, UiMediaState_Run,false);
	else
		ChangeMediaState( memberURI , UiStateTypeVideo, UiMediaState_Stop,false);
}

void MediaManager::ControlAudioRecv( const QString memberURI, const bool audioRecv )
{
	QString memberName = memberURI;
	int index= memberName.indexOf("@");
	if(  index!=-1 )
		memberName = memberName.left(index);

	QByteArray byteArray;
	QDataStream out( &byteArray , QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << audioRecv;

	QString mediaID = userID2MediaID( memberName );
	CvStreamedMediaIf streamProxy( getStreamedMediaServiceName(mediaID ), 
		getStreamedMediaObjectPath(mediaID), QDBusConnection::sessionBus() );
	streamProxy.SetMediaInfo( Action_ControlAudioRecv , byteArray );

	if( audioRecv )
		ChangeMediaState( memberURI , UiStateTypeAudio , UiMediaState_Run,false);
	else
		ChangeMediaState( memberURI , UiStateTypeAudio , UiMediaState_Stop,false);

}

void MediaManager::EnableAdaptiveCodeRate( bool enable )
{
    QByteArray byteArray;
    QDataStream out( &byteArray , QIODevice::WriteOnly );
    out.setVersion( QDataStream::Qt_4_4 );
    out << enable;
    CvStreamedMediaIf streamedMediaIf( getStreamedMediaServiceName(QString(SEND_STREAMED_MEDIA_ID) ) ,
        getStreamedMediaObjectPath(QString(SEND_STREAMED_MEDIA_ID)) , QDBusConnection::sessionBus() );
    streamedMediaIf.SetMediaInfo( Action_AdaptiveControlCodeRate , byteArray );
}

void MediaManager::clearRecvMemberPosition()
{
	RecvMemberVector localPreviewPos;

	for( RecvMemberVector::iterator it = _recvMemberVector.begin() ; it!=_recvMemberVector.end() ;it++ )
	{
		RecvMemberPosition * recvMember = *it;
		if (isLocalPreviewMediaID(recvMember->_memberURI))
		{
			localPreviewPos.push_back(recvMember);
			continue;
		}
		if( recvMember!=NULL )
		{
			delete recvMember;
			*it = NULL;
		}
	}

	_recvMemberVector.clear();

	//���¼��뱾�ػ��Ե�λ����Ϣ
	_recvMemberVector = localPreviewPos;
}

void MediaManager::rtcpReportTimerTimeout()
{
	if( _conferenceManager!=NULL )
	{
		
		RtpStatItem item;
		for( RecvMemberVector::iterator it = _recvMemberVector.begin(); it!=_recvMemberVector.end() ; it++ )
		{
			///�����Լ��ı���������⣬��������
			if( (*it)->_memberURI != RunningProfile::getInstance()->user_uri().c_str() )
			{
				(*it)->_rtcpAudioReportReader->readRtcpReport(item );
				_conferenceManager->sendRTPStatToQoSServer(item );
				(*it)->_rtcpVideoReportReader->readRtcpReport( item );
				_conferenceManager->sendRTPStatToQoSServer(item );
			}

		}
	}

    if (conference_controller_) {
      RtpStatItem item;
      for( RecvMemberVector::iterator it = _recvMemberVector.begin(); it!=_recvMemberVector.end() ; it++ )
      {
        ///�����Լ��ı���������⣬��������
        if( (*it)->_memberURI != RunningProfile::getInstance()->user_uri().c_str() )
        {
          (*it)->_rtcpAudioReportReader->readRtcpReport(item );
          conference_controller_->HandleReceiveUDPQoSReportSlot(item );
          (*it)->_rtcpVideoReportReader->readRtcpReport( item );
          conference_controller_->HandleReceiveUDPQoSReportSlot(item );
        }

      }

    }

}

/**
 * @brief ���ƹ�����Ļ�Ľ���İ�ť�������ǰ�ն�Ϊ��������ѡ���Ļ������ť��ʾ����������������
 * @param control ���Ϊtrue������ʾ��ť���������ذ�ť
 */
void MediaManager::ControlScreenShare( const bool control )
{
	QByteArray byteArray;
	QDataStream out( &byteArray , QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << control;
	_confRoomProxy->ModifyMediaWindow( "", MWA_ControlScreenShare, byteArray );
}


/**
 * @brief ����ָ���������������˵Ľ�����Ļ��
 * @param port ��Ļ��ռ�õĶ˿�
 * @dst Ŀ���ַ����������鲥������Ϊ"0.0.0.0"
 * @wnd �������Ļ����
 */
void MediaManager::CreateMainSpeakerRecvScreenMedia( const QString&memberURI , const int port ,const QString&src, const int wnd)
{
	QByteArray outputBytes;
	QDataStream out(&outputBytes,QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << src << port;

	_confRoomProxy->ModifyMediaWindow( memberURI , MWA_AddMainSpeakerScreenRecv, outputBytes );

}


void MediaManager:: RemoveScreenShareSend()
{
	QByteArray outputBytes;
	QDataStream out(&outputBytes,QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );

	_confRoomProxy->ModifyMediaWindow( "" , MWA_RemoveSendShareScreen, outputBytes );
}

void MediaManager:: PPTControlCommand(int type)
{
	QByteArray outputBytes;
	QDataStream out(&outputBytes,QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out<<type;

	_confRoomProxy->ModifyMediaWindow( "" , MWA_PPTControlCommand, outputBytes );

}

void MediaManager::RecoveryRecvMedia( RecvGraphInfo& info )
{

	QByteArray bytes;
	QDataStream out( &bytes , QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << info;
	int screenIndex;
	int seet;
	bool smallVideo;
	GetRecvMemberPositionFromName( info.user_id, &screenIndex , &seet, &smallVideo);
	_confRoomProxy->RecoveryRecvMediaProcess( bytes , screenIndex , seet );

}

void MediaManager::HandleNofityChannelStateChanged( const QString &channel_id, 
                                                   const QString &user_id, 
                                                   const QString &channel_type, 
                                                   int channel_state )
{
    if (channel_type == STREAMED_MEDIA_SERVICE_NAME) {
        if (isSendMediaID(channel_id)) {
            emit NotifySendMediaStateChanged(user_id, channel_state);
        } 
        else {
            emit NotifyRecvMediaStateChanged(user_id, channel_state);
        }
    }
}
