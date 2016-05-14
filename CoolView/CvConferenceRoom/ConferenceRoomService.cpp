#include "ConferenceRoomService.h"
#include <dbus/conferenceRoom/common/ConferenceRoomServiceCommon.h>
#include <dbus/conferenceRoom/service/ConferenceRoomAdaptor.h>
//msdx
#include <dbus/msdx/common/MsdxServiceCommon.h>


//channel dispatcher
#include <dbus/channelDispatcher/client/ChannelDispatcherIf.h>
#include <dbus/channelDispatcher/common/ChannelDispatcherServiceCommon.h>

//streamedMedia tunnel
#include <dbus/channel/type/streamedMedia/client/CvStreamedMediaIf.h>
#include <dbus/channel/type/streamedMedia/common/StreamedMediaServiceCommon.h>

#include <dbus/telecontroller/common/TelecontrollerServiceCommon.h>
#include <dbus/telecontroller/service/TelecontrollerAdaptor.h>

#include <dbus/kinect/common/KinectServiceCommon.h>

#include "QtConfrenceVideoWindow.h"
#include "VideoFrame.h"
#include <Windows.h>

#include "QoSReportReceiverThread.h"
#include <util/ini/CVIniConfig.h>

#include <QMessageBox>

ConferenceRoomService::ConferenceRoomService()
{
	//����Conference Room����
	new ConferenceRoomAdaptor(this);

	//Ϊ����Ӧ����Ļ���նˣ�ע��ʱ��Ҫ��������
	QDBusConnection connection = QDBusConnection::sessionBus();
	QString serviceName = QString(CONF_ROOM_SERVICE_NAME) ;
	QString objectPath = QString(CONF_ROOM_SERVICE_OBJECT_PATH);
	bool ret = connection.registerService(serviceName);
	//if( !ret)
	//	CV_LOG_ERROR( "ע��CONF_ROOM_SERVICEʧ��");

	ret = connection.registerObject( objectPath, this);

	//����Telecontroller����
	new TelecontrollerAdaptor(this);
	ret = connection.registerService( _GetTelecontrollerServiceName("confRoom") );
	ret = connection.registerObject( _GetTelecontrollerObjectPath("confRoom"),this );

	///��Ҫʹ�õ�tunnelDispatcher����
	_channelDispatcherProxy = new ChannelDispatcherIf( CHANNEL_DISPATCHER_SERVICE_NAME , CHANNEL_DISPATCHER_OBJECT_PATH,
		QDBusConnection::sessionBus());

  _mediaWindowManager = MediaWindowManager::getInstance();

	//����������34543�˿�
  _qosRportThread = new QThread();
  _qosRportThread->start();

	_qosRportReceiver = new QoSReportReceiverThread();
  _qosRportReceiver->Initialize("127.0.0.1", 
    CVIniConfig::getInstance().getConfRoomQosServerUdpPort());
  _qosRportReceiver->moveToThread(_qosRportThread); // �ڵ����߳���ִ��
  connect(
    _qosRportReceiver, 
    &QoSReportReceiverThread::SendUDPQoSReportSignal,
    _mediaWindowManager, 
    &MediaWindowManager::UpdateRtcpMessage);

   connect(
    _qosRportReceiver, 
    &QoSReportReceiverThread::SendUDPRecReportSignal,
    _mediaWindowManager, 
    &MediaWindowManager::UpdateRecordMessage);

    _qosRportReceiver->Start();
	// For Test
	/*VideoFrame * frame = MediaWindowManager::getInstance()->AddMediaWindow("Test", "Test", 0, 2, 320, 240);
	MediaWindowManager::getInstance()->ShowVideoFrame( frame );*/
}

ConferenceRoomService::~ConferenceRoomService()
{
	if( _qosRportReceiver )
	{
		_qosRportReceiver->Stop();
	}
}

void ConferenceRoomService::AddMediaWindow( bool send, const QByteArray &input_garray, int screenIndex, int seet )
{
    screenIndex = ScreenHelper::Instance()->PosIndexToScreenIndex(screenIndex);
	VideoFrame* videoFrame = NULL;
	QString mediaID;
	QByteArray output_garray;
	QDataStream inDataStream( input_garray );
	inDataStream.setVersion( QDataStream::Qt_4_4 );

	if( send )
	{
		QMessageBox::warning(NULL, "Warning", QString::fromLocal8Bit("������յ����ط�������ʶ"));
		return;
		//�����Ż��Դ���
		/*SendGraphInfo info;
		inDataStream>> info;*/

		//MediaWindowManager::getInstance()->SetScreenPort( info.net_info.screen_port );
		//MediaWindowManager::getInstance()->SetScreenIP( info.net_info.ip_addr );

		//videoFrame = MediaWindowManager::getInstance()->FindMediaWindow(LOCAL_PLAY_USERID);
		//if(videoFrame )
		//{
		//	//�ھ�λ������ʾ��Ƶ
		//	MediaWindowManager::getInstance()->ShowVideoFrame( videoFrame);
		//	//����λ������ʾ����Ƶ����
		//	//MediaWindowManager::getInstance()->ChangeWindowPostion( LOCAL_PLAY_USERID , screenIndex , seet );
		//	return;
		//}

		//videoFrame = MediaWindowManager::getInstance()->AddMediaWindow( LOCAL_PLAY_USERID , LOCAL_PLAY_USERID,screenIndex , seet, info.video_info.width, info.video_info.height );
		//if( videoFrame ==NULL )
		//	return;

		//MediaWindowManager::getInstance()->ShowVideoFrame( videoFrame );
		//info.preview_wnd = (int)videoFrame->LinkWin->getVideoFrame()->winId();

		/*QDataStream outDataStream( &output_garray , QIODevice::WriteOnly  );
		outDataStream.setVersion( QDataStream::Qt_4_4 );
		outDataStream << info;

		mediaID = SEND_STREAMED_MEDIA_ID;*/
		//��smallvideograph����...û�취����ʱ��ôŪ
		/*QString serviceName = QString( STREAMED_MEDIA_SERVICE_NAME ) + QString(".") + userID2MediaID("localpreview");
		QString objectPath = QString( STREAMED_MEDIA_SERVICE_OBJECT_PATH ) + QString("/") + userID2MediaID("localpreview");
		CvStreamedMediaIf* streamedMediaIf = new CvStreamedMediaIf( serviceName, objectPath,QDBusConnection::sessionBus() );
		_mediaProxyMap.insert(StreamedMediaProxyMap::value_type( userID2MediaID("localpreview") , streamedMediaIf ) );*/

	}else
	{
		RecvGraphInfo info;
		inDataStream >> info;

		videoFrame = MediaWindowManager::getInstance()->AddMediaWindow( 
      info.user_id , info.user_name, screenIndex , seet, 
      info.video_info.width, info.video_info.height );
		if( videoFrame==NULL )
			return;

		MediaWindowManager::getInstance()->ShowVideoFrame( videoFrame ); //ȷ���ɼ�
		info.recv_wnd = (int)videoFrame->LinkWin->getVideoFrame()->winId();
    qDebug() << "Show LinkWin:" << videoFrame->LinkWin->getVideoFrame()->winId();

		QDataStream outDataStream( &output_garray , QIODevice::WriteOnly  );
		outDataStream.setVersion( QDataStream::Qt_4_4 );
		outDataStream << info;

		//�������mediaID����ý��������
		if (info.net_info.video_port != 0 || info.net_info.audio_port != 0)
		{
			mediaID = userID2MediaID( info.user_id);	
		}
	}

	if (CVIniConfig::getInstance().IsModelHD() && 
		!mediaID.isEmpty())
	{
		//Ҫ��channelDispatcher������Ӧ��ý�������̣�����Ѿ���������ʼ������
		_channelDispatcherProxy->CreateChannel( mediaID , STREAMED_MEDIA_SERVICE_NAME , 
			output_garray );
		StreamedMediaProxyMap::iterator it = _mediaProxyMap.find( mediaID );
		if( it!=_mediaProxyMap.end() )
		{
			delete it->second;
		}
		//����mediaID��ӳ���ϵ
		QString serviceName = QString( STREAMED_MEDIA_SERVICE_NAME ) + QString(".") + mediaID;
		QString objectPath = QString( STREAMED_MEDIA_SERVICE_OBJECT_PATH ) + QString("/") + mediaID;
		CvStreamedMediaIf* streamedMediaIf = new CvStreamedMediaIf( serviceName, objectPath,QDBusConnection::sessionBus() );
		//connect(streamedMediaIf , SIGNAL(QosNofity(const QByteArray& , int )) , this ,SLOT(QoSNotifySlot(const QByteArray& , int)) );

		_mediaProxyMap[mediaID] = streamedMediaIf;
	}

	//ͨ��ң����
	if( videoFrame && videoFrame->LinkWin )
		MediaWindowManager::getInstance()->MemberLocationAddNotify( videoFrame->LinkWin->getUserId() , videoFrame->_screenIndex , videoFrame->_seet , 
			videoFrame->LinkWin->isAudioEnable() , videoFrame->LinkWin->isVideoEnable() );
}


void ConferenceRoomService::ChangeLayout( int screenIndex, int displayModel )
{
    screenIndex = ScreenHelper::Instance()->PosIndexToScreenIndex(screenIndex);
	MediaWindowManager::getInstance()->ChangeLayout( screenIndex ,displayModel );
}

void ConferenceRoomService::CloseMediaWindow( const QString &user_id )
{
	VideoFrame* frame = MediaWindowManager::getInstance()->FindMediaWindow( user_id );
	if( frame )
	{
		if( CVIniConfig::getInstance().IsModelHD() /*&&
			user_id!=LOCAL_PLAY_USERID*/ )
		{
			//�ͷ���Ӧ��channel
			_channelDispatcherProxy->ReleaseChannel( userID2MediaID(user_id), STREAMED_MEDIA_SERVICE_NAME );
    }
    MediaWindowManager::getInstance()->HideVideoFrame( frame );
		MediaWindowManager::getInstance()->CloseMediaWindow( user_id );
	}
}

void ConferenceRoomService::CloseWindow()
{

}

void ConferenceRoomService::ExitConference()
{
	if( _mediaProxyMap.size()>0 )
	{
		//�ͷ����е�ý����channel
		//2013-10-30 Liaokz���䣺���������Ϊ�˳����鲻�������ػ��ԣ��ʲ�������ChannelDispatcher��
		//�Ǳ��ػ��Խ��ս��̵Ľ�����������CoolView�н���
		/*if( NConfig::getInstance().getModelCategory() == CVCFG_VALUE_MODEL_CATEGORY_HD)
		{
			_channelDispatcherProxy->Destroy();
		}*/	

		StreamedMediaProxyMap::iterator it = _mediaProxyMap.begin();
		for( ; it!=_mediaProxyMap.end(); )
		{
			if (isLocalPreviewMediaID(it->first))
			{
				++it;
				continue;
			}
			CvStreamedMediaIf* streamedMediaIf = it->second;
			//streamedMediaIf->Stop();
			delete streamedMediaIf;
			streamedMediaIf = NULL;
			StreamedMediaProxyMap::iterator itOld = it++;
			_mediaProxyMap.erase(itOld); //Map��erase��Ӱ�����������
		}
		//_mediaProxyMap.clear();
	}
	
	
	MediaWindowManager::getInstance()->ClearMediaWindow();

	//ChangeLayout( 0 , CF_DM_L1_5 );
	//ChangeLayout( 1 , CF_DM_L1_5 );
}

/**
 * @brief ��ý�崰�ڵ����Խ��и���
 * @param user_id �û�����ͨ����sip�ʺŵ��û����ƣ�������"@realm"�ֶ�
 * @param actionIndex �������ͣ��鿴MediaWindowActionö������
 * @param arguments ��������Ҫ�Ĳ���
 */
void ConferenceRoomService::ModifyMediaWindow( const QString &user_id, int actionIdex, const QByteArray &arguments )
{
	QDataStream in( arguments);
	in.setVersion( QDataStream::Qt_4_4 );
	switch( actionIdex )
	{
	case MWA_ChangeWindowPosition:
		{
			int screenIndex;
			int seet;
			in >> screenIndex >> seet;
            screenIndex = ScreenHelper::Instance()->PosIndexToScreenIndex(screenIndex);
			VideoFrame* frame = MediaWindowManager::getInstance()->FindMediaWindow(user_id);
			if( frame )
			{
				//ֻ����ͬһ����Ļ�ڵ���Ƶ���ڲ��ܽ����϶�
				//if( user_id!=LOCAL_PLAY_USERID ||
				//	( user_id==LOCAL_PLAY_USERID && screenIndex==frame->_screenIndex)  )
				//{
				//	//����ý�崰��
				//	MediaWindowManager::getInstance()->ChangeWindowPostion( user_id , screenIndex , seet );
				//}else
				//{
				//	//���Ѿ����ص��Ĵ�����ʾ����
				//	MediaWindowManager::getInstance()->ShowVideoFrame(frame);
				//}

				//��������
				MediaWindowManager::getInstance()->ChangeWindowPostion( user_id , screenIndex , seet );
			}
			
			return;
		}
	case MWA_ChangeMediaState:
		{
			QString mediaType;
			int state;
			bool isAudioSend;
			in >> mediaType >> state>>isAudioSend;
			if( mediaType!="screen" )
			{
				VideoFrame* frame = MediaWindowManager::getInstance()->FindMediaWindow(user_id);
				if( frame )
				{
					MediaWindowManager::getInstance()->ChangeMediaState( frame , mediaType , static_cast<UiMediaState>(state),isAudioSend);
				}
			}else
			{
				MediaWindowManager::getInstance()->ChangeMediaState( NULL , "screen" , static_cast<UiMediaState>(state),isAudioSend);
			}

			return;
		}
	case MWA_AddScreenRecv:
		{
			QString ip;
			int port;
			int screenIndex;
			in >> ip >> port >> screenIndex;
            screenIndex = ScreenHelper::Instance()->PosIndexToScreenIndex(screenIndex);
			MediaWindowManager::getInstance()->AddScreenMediaRecv( user_id , ip , port , screenIndex );
			return;
		}
	case MWA_RemoveScreenRecv:
		{
			MediaWindowManager::getInstance()->RemoveScreenMediaRecv( user_id );
			return;
		}
	case MWA_ControlScreenShare:
		{
			bool control;
			in >> control;
			MediaWindowManager::getInstance()->ControlScreenShare( control );
			return;
		}
	case MWA_AddMainSpeakerScreenRecv:
		{
			QString ip;
			int port;
			in >> ip >> port;
			MediaWindowManager::getInstance()->AddMainSpeakerScreenMediaRecv( user_id , ip , port );
			return;
		}
	case  MWA_RemoveSendShareScreen:
		{
			
			MediaWindowManager::getInstance()->RemoveSendShareScreen();
			return;
		}
	case  MWA_PPTControlCommand:
		{
			int commd;
			in >> commd;
			int type=-1;
			switch(commd)
			{
				case 0 :
					{
						type=KG_RightHandLift;
						break;
					}
				case 1:
					{
						type=KG_LeftHandLift;
						break;
					}
			}
	
			MediaWindowManager::getInstance()->KinectGestureSlot("",type);
			return;
		}
	}
}

void ConferenceRoomService::ShowRtcpMsg( int screenIndex, bool show )
{
    screenIndex = ScreenHelper::Instance()->PosIndexToScreenIndex(screenIndex);
	MediaWindowManager::getInstance()->ShowRtcpMessage( screenIndex , show );
}

void ConferenceRoomService::UpdateRtcpMsg( const QByteArray &input_garray )
{
	QDataStream in( input_garray );
	in.setVersion( QDataStream::Qt_4_4 );
}

void ConferenceRoomService::resizeMediaWindow( const QString& userID , const int wnd )
{
	int xpos = 0, ypos = 0, width = 0, height = 0;
	int graphID = 0;
	int *value = NULL;
	RECT WindowRect;
	if( userID.length()<=0 )
	{
		return;
	}
	if (wnd)
	{
		QString mediaID;
		/*if( userID==LOCAL_PLAY_USERID )
		{
		    mediaID = SMALLVIDEO_STREAMED_MEDIA_ID;
		}else
		{*/
		mediaID = userID2MediaID(userID);
		//}
		StreamedMediaProxyMap::iterator it = _mediaProxyMap.find(mediaID );
		if( it!=_mediaProxyMap.end() )
		{
			HWND remoteHwnd = (HWND)wnd;
			GetWindowRect(remoteHwnd, &WindowRect);
			width = WindowRect.right - WindowRect.left;
			height = WindowRect.bottom - WindowRect.top;
			if (width == 0 || height == 0)
			{
				return;
			}
			if ( "screen_share"!= userID )
			{
				if ((width / height) >= (176 / 144))
				{
					xpos = (width - height * 176 / 144) / 2;
					width = height * 176 / 144;
				}
				else
				{
					ypos = (height - width * 144 / 176) / 2;
					height = width * 144 / 176;
				}
			}
			WndInfo info;
			info.initial( -1 , qPrintable(userID) , wnd , xpos , ypos , width , height );

			QByteArray outArray;
			QDataStream outDataStream(&outArray , QIODevice::WriteOnly);
			outDataStream.setVersion( QDataStream::Qt_4_4 );
			outDataStream << info;

			it->second->SetMediaInfo( Action_SetVideoWindow , outArray );
		}
	}
}

void ConferenceRoomService::TeleInfo( int info_index, int subscribe_id, const QByteArray &input_array )
{
	switch( info_index )
	{
	case TELE_QueryLayout:
		{
			MediaWindowManager::getInstance()->LayoutChangedNofity(subscribe_id);
			return;
		}
	case TELE_QueryMemberLocation:
		{
			MediaWindowManager::getInstance()->MemberLocationUpdateNotify(subscribe_id);
			return;
		}
	case TELE_QueryScreenShareState :
		{
			MediaWindowManager::getInstance()->ScreenShareStateUpdateNotify(subscribe_id);
			return;
		}
	}
}

void ConferenceRoomService:: OpenScreenShareFile(const QString &fileName, int screenIndex)
{
    screenIndex = ScreenHelper::Instance()->PosIndexToScreenIndex(screenIndex);
	QString appDir = QApplication::applicationDirPath();
	appDir.replace("/" , "\\" );
	QString tempPath = appDir +  "\\share\\"+fileName;
	MediaWindowManager::getInstance()->OpenScreenShareFile(tempPath,screenIndex);
	
}

void ConferenceRoomService::RecoveryRecvMediaProcess(const QByteArray &input_garray, int screenIndex, int seet)
{
    screenIndex = ScreenHelper::Instance()->PosIndexToScreenIndex(screenIndex);
	VideoFrame* videoFrame = NULL;
	QString mediaID;
	QByteArray output_garray;
	QDataStream inDataStream( input_garray );
	inDataStream.setVersion( QDataStream::Qt_4_4 );
	
	RecvGraphInfo info;
	inDataStream >> info;

	//videoFrame = MediaWindowManager::getInstance()->AddMediaWindow( info.user_id , info.user_name, screenIndex , seet, info.video_info.width, info.video_info.height );
	videoFrame = MediaWindowManager::getInstance()->FindMediaWindow(info.user_id);
	if( videoFrame==NULL )
		return;

	//MediaWindowManager::getInstance()->ShowVideoFrame( videoFrame );

	info.recv_wnd = (int)videoFrame->LinkWin->getVideoFrame()->winId();

	QDataStream outDataStream( &output_garray , QIODevice::WriteOnly  );
	outDataStream.setVersion( QDataStream::Qt_4_4 );
	outDataStream << info;

	//�������mediaID����ý��������
	mediaID = userID2MediaID( info.user_id);	

	//Ҫ��channelDispatcher����ý�������̣�����Ѿ���������ʼ������
	if (CVIniConfig::getInstance().IsModelHD())
	{
		_channelDispatcherProxy->CreateChannel( mediaID , STREAMED_MEDIA_SERVICE_NAME , 
			output_garray );
	}



}