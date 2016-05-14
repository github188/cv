#include <QtWidgets/QtWidgets>

#include "MediaWindowManager.h"

#include "VideoFrame.h"
#include "QtConfrenceVideoWindow.h"
#include "QtMeetingPlaceWidget.h"
#include "qtconferenceroom.h"
#include "QtScreenShare.h"
#include <dbus/conferenceRoom/common/ConferenceRoomServiceCommon.h>
#include <dbus/channel/type/streamedMedia/common/StreamedMediaServiceCommon.h>

#include <dbus/channel/type/screenMedia/common/ScreenMediaServiceCommon.h>

#include <dbus/channelDispatcher/common/ChannelDispatcherServiceCommon.h>
#include <dbus/channelDispatcher/client/ChannelDispatcherIf.h>

#include <dbus/telecontroller/client/TelecontrollerIf.h>
#include <dbus/telecontroller/common/TelecontrollerServiceCommon.h>

#include <dbus/kinect/client/KinectIf.h>
#include <dbus/kinect/common/KinectServiceCommon.h>

//core proxy
#include <dbus/core/common/CvCoreServiceCommon.h>
#include <dbus/core/client/CvCoreIf.h>

#include "kinectinfowidget.h"
#include "uicommon.h"
#include <log/Log.h>
#include "util/ini/CvIniConfig.h"
#include "util/ini/TxConfig.h"
#include "util/report/RecordStat.h"

MediaWindowManager::MediaWindowManager()
  : _updateScreenTimer(this)
{
	//������ʾ������Ŀ����������UI
    bool is_model_hd = 
      CVIniConfig::getInstance().IsModelHD();

	const int screenNum = (is_model_hd ? QApplication::desktop()->numScreens() : 
      CTxConfig::getInstance().GetRecUiScreenCount());
	for( int screenIndex=0 ; screenIndex<screenNum ; screenIndex++ )
	{
		QtConferenceRoom* conferenceRoom = new QtConferenceRoom(screenIndex);
		
		_confRoomVector.push_back( conferenceRoom );
        conferenceRoom->setVisible(is_model_hd);
    }

	//connect( QApplication::desktop() , SIGNAL(screenCountChanged(int)) , 
	//	this , SLOT(ScreenCountChanged(int)) );
	
	_isShowingRtcp = false;

	_teleControlProxy = new TelecontrollerIf( _GetTelecontrollerServiceName("sock"),
		_GetTelecontrollerObjectPath("sock") , QDBusConnection::sessionBus() );
	_channelDispatcherProxy = new ChannelDispatcherIf( CHANNEL_DISPATCHER_SERVICE_NAME,
		CHANNEL_DISPATCHER_OBJECT_PATH , QDBusConnection::sessionBus() );

	_screenShareWidget = NULL;

	_screenPort = 0;
	_screenIP = "";

	_isSendingScreen = false;

	//kinect��ز���...zhenHua.sun 2011-08-18
	_kinectProxy = new KinectIf( KINECT_SERVICE_NAME,
		KINECT_OBJECT_PATH , QDBusConnection::sessionBus() );
	connect( _kinectProxy , SIGNAL(GestureOccur(const QString, int)), 
		this, SLOT(KinectGestureSlot(const QString,int)) );
	connect( _kinectProxy , SIGNAL(KinectMessage(int, const QString )),
		this, SLOT(KinectMessageSlot(int, const QString)) );

	_kinectInfoWidget = new KinectInfoWidget();
	_kinectInfoWidget->hide();

#ifdef NDEBUG
    connect(&_updateScreenTimer, &QTimer::timeout,
      this, &MediaWindowManager::updateScreenSlot);
    _updateScreenTimer.start(2000);
#endif

  //CoreProxy
  _coreProxy = new CvCoreIf(CVCORE_SERVICE_NAME, CVCORE_SERVICE_OBJECT_PATH, 
    QDBusConnection::sessionBus());

	//����kinect��λ���ڵ�һ����Ļ������
	QRect screenRect = QApplication::desktop()->availableGeometry();
	int xpos = ( screenRect.width()-_kinectInfoWidget->width() )/2;
	int ypos = ( screenRect.height()-_kinectInfoWidget->height() )/2;
	_kinectInfoWidget->setGeometry( xpos,ypos,_kinectInfoWidget->width(),_kinectInfoWidget->height());
}

MediaWindowManager::~MediaWindowManager()
{
	SAFE_DELETE(_kinectInfoWidget );
}

/** 
 * @brief ����ָ����������ý�崰��
 * @return ����ɹ��򷵻�ý�崰�ڣ����򷵻�NULL
 */
VideoFrame* MediaWindowManager::AddMediaWindow( const QString& userID ,const QString& displayName, const int screenIndex, const int seet ,const int videoWidth , const int videoHeight)
{
	if( screenIndex > (_confRoomVector.size()-1) )
	{
		//�����Ļ������ֵ���������Ļ����ֵscreen num -1 ����ôֱ�ӷ���null
		return NULL;
	}
	VideoFrame *vFrame = NULL;
	//�����ж��Ƿ����ͬ�����ֵ�videoFrame���еĻ�ֱ��������
	vFrame = FindMediaWindow( userID );
	if( vFrame )
	{
		if( vFrame->isUsed )
		{
			//��������Ѿ����ڲ���������ʾ����ôֻ��Ҫ��������Ϊ��λ��
			this->ChangeWindowPostion( userID , screenIndex , seet );
			return NULL;
		}else
		{
			vFrame->isUsed = true;
			vFrame->_screenIndex = screenIndex;
			vFrame->_seet = seet;
			vFrame->LinkWin->setUserId( userID );
			vFrame->LinkWin->setDisplayName( displayName );
			vFrame->_videoWidth = videoWidth;
			vFrame->_videoHeight = videoHeight;
			vFrame->LinkWin->show();
		}
	}else
	{
		//���û�п��õĴ��ڣ����½�һ��
		//VideoFrame *vFrame = NULL;
		if( screenIndex>0 )
			vFrame = new VideoFrame( _confRoomVector.at(screenIndex)->getMeetingPlaceWidget());
		else
			vFrame = new VideoFrame( _confRoomVector.at(0)->getMeetingPlaceWidget() );

		QtConfrenceVideoWindow *child = new QtConfrenceVideoWindow();
		vFrame->layout.addWidget(child);
		vFrame->LinkWin = child;//Add By LZY 2010-10-09 ��������Ĵ���ָ��
		vFrame->LinkWin->setUserId( userID );
		vFrame->LinkWin->setDisplayName( displayName );
		vFrame->isUsed = true;
		vFrame->_screenIndex = screenIndex;
		vFrame->_seet = seet;
		vFrame->_videoWidth = videoWidth;
		vFrame->_videoHeight = videoHeight;
		//video_frame_map[child] = vFrame;
		_memberFrames.push_back(vFrame);
		child->show();
	}

	if( vFrame && vFrame->LinkWin  )
	{
		//�����¼���Ĵ���Ҫ�ж��Ƿ���ʾrtcp��Ϣ
		if( _isShowingRtcp /*&& userID!=LOCAL_PLAY_USERID*/ )
			vFrame->LinkWin->showRtcpMessage();
		else
			vFrame->LinkWin->hideRtcpMessage();

    //�´�����ʾ�ȴ����ݱ�ʶ
    ChangeMediaState(vFrame, "", UiMediaState_Initializing, false);
	}

	return vFrame;
}

void MediaWindowManager::CloseMediaWindow( const QString& userID )
{
	VideoFrame* frame = FindMediaWindow( userID );

	if( frame )
	{
		if( frame->LinkWin )
		{
			this->MemberLocationRemoveNotify( userID ,  frame->_screenIndex , frame->_seet );
		}

		//���ڱ��ػ��Դ��ڣ����ڵ���Դ���ܱ��ͷ�
		/*if( userID==LOCAL_PLAY_USERID )
			return;*/

		frame->Release();
	}
}


void MediaWindowManager::ClearMediaWindow()
{
	//_memberFramesLock.lock();

	MediaWindowVector localPreviewWindows;
	//ע�ⲻҪ������ػ��ԵĴ���
	for( int i=0  ; i<_memberFrames.size() ; i++ )
	{
		VideoFrame* frame = _memberFrames.at(i);
		if (frame == NULL)
		{
			continue;
		}

		if (isLocalPreviewMediaID(frame->LinkWin->getUserId()))
		{
			localPreviewWindows.push_back(frame);
			continue;
		}
		//��meetingplace����������Ƶ���ڵĹ���
		HideVideoFrame( frame );

		frame->isUsed = false;
		if( frame->LinkWin )
		{
			frame->LinkWin->close();
			delete frame->LinkWin;
			frame->LinkWin = NULL;
		}
		delete frame;
		frame = NULL;
	}
	_memberFrames.clear();
	_memberFrames = localPreviewWindows;
	//_memberFramesLock.unlock();

	_screenPort = 0;
	_screenIP = "";

	for(int i=0 ; i<_confRoomVector.size();i++ )
	{
		if( _confRoomVector.at(i)&& _confRoomVector.at(i)->getScreenShareWidget() )
		{
			//�˳������ͬʱ��֮ǰ���˵�ppt�ص�  zhongBao.liang
			_confRoomVector.at(i)->getScreenShareWidget()->killPPT();
			_confRoomVector.at(i)->getScreenShareWidget()->release();
		}	
	}

}

// ����UpdateRtcpMessage�ж��̵߳��ã�����ͬ������Ӧ���ɵ����߹���ͬ����
VideoFrame* MediaWindowManager::FindMediaWindow( const QString& userID )
{
	VideoFrame * pFrame = NULL;

	MediaWindowVector::iterator it = _memberFrames.begin();
	for( ; it!=_memberFrames.end();it++ )
	{
		if( (*it)->LinkWin && (*it)->LinkWin->getUserId()==userID )
		{
			pFrame = (*it);
			break;
		}
	}

	return pFrame;
}

void MediaWindowManager::ShowRtcpMessage( const int screenIndex, const bool show )
{
	//_memberFramesLock.lock();
	MediaWindowVector::iterator it = _memberFrames.begin();
	for( ; it!=_memberFrames.end();it++ )
	{
		if( (*it)->isUsed /*&& ((*it)->LinkWin->getUserId()!=LOCAL_PLAY_USERID)*/ )
		{
			if( show )
				(*it)->LinkWin->showRtcpMessage();
			else
				(*it)->LinkWin->hideRtcpMessage();
		}
	}
	//_memberFramesLock.unlock();
	_isShowingRtcp = show;
}

void MediaWindowManager::ShowVideoFrame( VideoFrame* frame )
{
	if( frame->_screenIndex>_confRoomVector.size()-1 )
	{
		CV_LOG_ERROR("��Ƶ�������ڵ���Ļ�������ڣ�");
		return ;
	}
	if( frame->_screenIndex>0 )
	{
		_confRoomVector.at(frame->_screenIndex)->getMeetingPlaceWidget()->showVideoWindow( frame );
	}else
	{
		_confRoomVector.at(0)->getMeetingPlaceWidget()->showVideoWindow( frame );
	}

}

void MediaWindowManager::HideVideoFrame( VideoFrame* frame )
{
	if( frame->_screenIndex>_confRoomVector.size()-1 )
	{
		CV_LOG_ERROR("��Ƶ�������ڵ���Ļ�������ڣ�");
		return ;
	}
	if( frame->_screenIndex>0 )
	{
		_confRoomVector.at(frame->_screenIndex)->getMeetingPlaceWidget()->hideVideoWindow( frame );
	}else
	{
		_confRoomVector.at(0)->getMeetingPlaceWidget()->hideVideoWindow( frame );
	}

}


void MediaWindowManager::ChangeLayout( int screenIndex, int displayModel )
{
	if( screenIndex>_confRoomVector.size()-1 || _confRoomVector.size()==0  )
	{
		_snprintf(__global_msg , sizeof(__global_msg), "�ı䲼��ʱ������Ļ����ֵΪ%d����Ļ��ĿΪ:%d" , screenIndex, _confRoomVector.size() );
		CV_LOG_ERROR(__global_msg);
		return ;
	}

	int index = 0;
	if( screenIndex>0 )
		index = screenIndex;

	QtConferenceRoom* pConfRoom = _confRoomVector.at(index);
	if( pConfRoom )
	{
		if( displayModel==CF_DM_DOC )
		{
			//����������ĵ�����
			//��ʼ��������Ļ
			pConfRoom->showScreenShare( );
            pConfRoom->getScreenShareWidget()->SetSharedMode(QtScreenShare::kDocumentMode);

		} else if (displayModel == CF_DM_AIRPLAY) {
            pConfRoom->showScreenShare();
            pConfRoom->getScreenShareWidget()->SetSharedMode(QtScreenShare::kAirPlayMode);
        } else {
            pConfRoom->getScreenShareWidget()->SetSharedMode(QtScreenShare::kVideoAudioMode);
			pConfRoom->showMeetringPlace();
			pConfRoom->getMeetingPlaceWidget()->updateDisplay( static_cast<ConfRoomDisplayModel>(displayModel) );
		}


		this->LayoutChangedNofity();
	}

}

void MediaWindowManager::ChangeWindowPostion( const QString& userID , const int newScreenIndex, const int newSeet )
{
	VideoFrame* frame = FindMediaWindow( userID );
	if( frame )
	{
		if( /*userID!=LOCAL_PLAY_USERID &&*/ frame->_screenIndex==newScreenIndex && frame->_seet==newSeet )
		{
			//�Ǳ��ػ��Ե�����£�λ�ò����ʱ�򲻽����κβ���
			return;
		}

		if( frame->_screenIndex!=newScreenIndex )
		{
			//�ƶ�������һ����Ļ��ʱ��graph�������������������Ƶ����ص�run��״̬
			this->ChangeMediaState( frame , "video" , UiMediaState_Run ,true);
			this->ChangeMediaState( frame , "audio" , UiMediaState_Run ,true);
		}

		if( frame->_screenIndex!=newScreenIndex 
			|| frame->_seet != newSeet )
		{
			this->HideVideoFrame( frame );	
		}
		frame->_seet = newSeet;
		frame->_screenIndex = newScreenIndex;
		this->ShowVideoFrame( frame );
	}

	this->MemberLocationChangeNotify( userID , newScreenIndex , newSeet );
}

//void MediaWindowManager::UpdateRtcpMessage( const RtpStatItemInfo& stateInfo )
//{
//	_memberFramesLock.lock();
//	VideoFrame* frame = FindMediaWindow( stateInfo.user_id );
//	if( frame )
//	{
//		if( frame->LinkWin )
//		{
//			frame->LinkWin->updateRtcpEventHandler( stateInfo );
//		}
//	}
//	_memberFramesLock.unlock();
//}

void MediaWindowManager::UpdateRtcpMessage( const RtpStatItem& stateItem )
{
	//_memberFramesLock.lock();
	if( stateItem.media_type==eMT_Video )
	{
		VideoFrame* frame = FindMediaWindow( stateItem.member_id );
		if( frame )
		{
			if( frame->LinkWin )
			{
				frame->LinkWin->updateRtcpEventHandler( stateItem );
			}
		}
	}
	//_memberFramesLock.unlock();
}

void MediaWindowManager::UpdateRecordMessage( const RecStatItem& stateItem )
{
	//_memberFramesLock.lock();
    QString member_id = stateItem.user_name;
    //������¼�ƵĽ���id�滻�ɻطŽ���id���Ա�ƥ�䴰��
    member_id = member_id.replace(LOCAL_RECORD_MEDIA_ID, LOCAL_PREVIEW_MEDIA_ID);
    VideoFrame* frame = FindMediaWindow(member_id);
    if( frame )
    {
        if( frame->LinkWin )
        {
            frame->LinkWin->updateRecordEventHandler( stateItem );
        }
    }
	//_memberFramesLock.unlock();
}

void MediaWindowManager::ScreenCountChanged( int newCount )
{
	if (!CVIniConfig::getInstance().IsModelHD())
	{
		return;
	}

	if( newCount==_confRoomVector.size() || newCount==0 )
	{
		//�����Ļ��������UI����һ�£�������Ļ��������Ϊ0ʱ��
		//�������κβ���
		return;
	}
	if( newCount < _confRoomVector.size() )
	{
		//��ʾ�������ˣ�ȥ������Ľ���
		for( int screenIndex=_confRoomVector.size()-1 ; screenIndex>=newCount ; screenIndex-- )
		{
			QtConferenceRoom* pConfRoom = _confRoomVector.at(screenIndex);

			//_memberFramesLock.lock();
			int memberFrameCount = _memberFrames.size();
			//�رն�����ʾ���ϵ���Ƶ����
			MediaWindowVector::iterator it = _memberFrames.begin();
			for( ; it!=_memberFrames.end() ; )
			{
				VideoFrame*	frame = *it;
				if( frame->_screenIndex==screenIndex )
				{
					//��meetingplace����������Ƶ���ڵĹ���
					HideVideoFrame( frame );
					frame->isUsed = false;

					if( frame->LinkWin )
					{
						//�ͷ�ý����
						_channelDispatcherProxy->ReleaseChannel( userID2MediaID(frame->LinkWin->getUserId()), STREAMED_MEDIA_SERVICE_NAME );

						frame->LinkWin->close();
						delete frame->LinkWin;
						frame->LinkWin = NULL;
					}

					delete frame;
					frame = NULL;

					it = _memberFrames.erase(it);
				}else
				{

					it++;
				}
			}

			//_memberFramesLock.unlock();

			//�رջ�����UI
			delete pConfRoom;
			pConfRoom = NULL;
			_confRoomVector.pop_back();
		}
	}else
	{
		//�����ʾ�������ˣ���ô����µĽ���UI
		int addUiNum = newCount - _confRoomVector.size();
		for( int i=0 ; i<addUiNum ; i++ )
		{
			QtConferenceRoom* confRoom = new QtConferenceRoom( _confRoomVector.size() );
			confRoom->show();
			_confRoomVector.push_back(confRoom);
		}
	}
	this->LayoutChangedNofity();
}

void MediaWindowManager::MemberLocationUpdateNotify( const int controllerIndex/*=-1*/ )
{
	QByteArray output_array;
	QDataStream out(&output_array , QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << TELE_MemberLocation;
	out << (int)0;							//���ڽ��ճ�Ա�����ݣ���þ���ֵ֮������д��

	//���ȼ������Ƶ����
	int recvUserNum = 0;
	//_memberFramesLock.lock();

	MediaWindowVector::iterator it = _memberFrames.begin();
	for( ; it!=_memberFrames.end() ; it++ )
	{
		VideoFrame* frame = *it;
		if( frame->isUsed&&!frame->LinkWin->isHidden() )
		{
			if( frame->LinkWin )
			{
				MemberLocation location;
				location._memberName = frame->LinkWin->getUserId();
				location._screenIndex = ScreenHelper::Instance()->ScreenIndexToPosIndex(frame->_screenIndex);
				location._seet = frame->_seet;
				if( frame->LinkWin )
				{
					if( frame->LinkWin->isAudioEnable() )
					{
						location._audioEnable = true;
					}else
					{
						location._audioEnable = false;
					}
					if( frame->LinkWin->isVideoEnable() )
					{
						location._videoEnable = true;
					}else
					{
						location._videoEnable = false;
					}

				}
				out << location;
				recvUserNum++;
			}
		}
	}

	//_memberFramesLock.unlock();

	//Ȼ������Ļ������
	for( int i=0 ; i<_confRoomVector.size();i++ )
	{
		QtConferenceRoom* room = _confRoomVector.at(i);

		MemberLocation location;
		QString name = room->getScreenShareWidget()->UserID();
		int index = name.indexOf("@");
		if( index>=0 )
			name = name.left( index );
		location._memberName = name;
		location._screenIndex = ScreenHelper::Instance()->ScreenIndexToPosIndex(i);
		location._seet = 10;			//������ң��������Ļ������õ��ǵ�10�����ڣ�����������10
		location._audioEnable = false;
		location._videoEnable = false;
		out << location;
		recvUserNum++;
		
	}

	//����дָ�룬���ý�����Ա��Ŀ
	out.device()->seek( sizeof(int) );
	out << recvUserNum;

	if( _teleControlProxy )
	{
		_teleControlProxy->TeleInfo(TELE_MemberLocation , controllerIndex,output_array );
	}

}

void MediaWindowManager::LayoutChangedNofity( const int controllerIndex/*=-1 */ )
{
	QByteArray output_array;
	QDataStream out(&output_array , QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << TELE_Layout;
	out << _confRoomVector.size();
	for( int i=0 ; i<_confRoomVector.size() ; i++ )
	{
		ScreenInfo screen;
		screen._screenIndex = ScreenHelper::Instance()->ScreenIndexToPosIndex(i);
		if( _confRoomVector.at(i)->getMeetingPlaceWidget()->isHidden() )
		{
			//��Ļ������ʾ�ĵ���
            switch (_confRoomVector.at(i)->getScreenShareWidget()->GetSharedMode()) {
              case QtScreenShare::kDocumentMode:
                screen._layout = "D";
                break;
              case QtScreenShare::kAirPlayMode:
                screen._layout = "AirPlay";
            }
		}else
		{
			switch( _confRoomVector.at(i)->getMeetingPlaceWidget()->_displayModel )
			{
			case CF_DM_1X1:
				screen._layout = "1";
				break;
			case CF_DM_2X2:
				screen._layout = "2";
				break;
			case CF_DM_3X3:
				screen._layout = "3";
				break;
			case CF_DM_L3X3:
			case CF_DM_L1_5:
				screen._layout = "L1_5";
				break;
			case CF_DM_L1_20:
				screen._layout = "L1_20";
				break;
			case CF_DM_AUTO:
				screen._layout = "AUTO";
				break;
            case CF_DM_TOP_1_2:
                screen._layout = "T1x2";
                break;
            case CF_DM_4X4:
                screen._layout = "4";
                break;
			default:
				screen._layout = "3";
			}
		}

		out << screen;
	}

	if( _teleControlProxy )
	{
		_teleControlProxy->TeleInfo( TELE_Layout , controllerIndex, output_array );
	}
}

/**
 * @brief ���Ľ���ý������״̬
 * @param frame ý��������
 * @param mediaType audioΪ��Ƶ��videoΪ��Ƶ
 * @param state ý������״̬���ο�ö����UiMediaState
 * @param isAudioSend ���ΪTrue˵����ǰ��ý����Ϊ������������Ϊfalse
 * @param enable ���������ΪTrue������Ϊfalse
 */
void MediaWindowManager::ChangeMediaState( const VideoFrame* frame , const QString& mediaType , const UiMediaState state,bool isAudioSend)
{
	if( mediaType=="screen" )
	{
		QVector<QtConferenceRoom*>::iterator it = _confRoomVector.begin();
		QtConferenceRoom* pConfRoom;
		for( ; it!=_confRoomVector.end() ; it++ )
		{
			pConfRoom = *it;
			pConfRoom->getScreenShareWidget()->setMediaState(state);
		}
  }
  else /*if( mediaType=="audio" || mediaType=="video" )*/
  {
    if( frame && frame->LinkWin )
    {
      frame->LinkWin->setMediaState( mediaType , state ,isAudioSend);

      if( state==UiMediaState_Stop )
      {
        QString mediaState = "stop";
        this->MediaStateChangedNotify( frame->LinkWin->getUserId() , mediaType , mediaState );
      }else if( state==UiMediaState_Run )
      {
        QString mediaState = "run";
        this->MediaStateChangedNotify( frame->LinkWin->getUserId() , mediaType , mediaState  );
      }

    }
  }
	
}

/**
 * @brief �����Ļ�����������ڽ���Զ���ն˵���Ļ��Ϣ�������ǰ�ն��Ѿ���������Ļ����������Ļ���ͽ��̣���ô
          ��������ͷŵ���Щ��Դ��Ҳ���޷��ڷ��͵�ͬʱ����Զ���ն˵���Ļ��
 * @param userID Զ���ն˵��ն���
 * @param ip Զ���ն˵�IP
 * @param port ��Ļ����˿�
 * @param screenIndex ��ǰ�ն���ʾԶ����Ļ����ʾ��
 */
void MediaWindowManager::AddScreenMediaRecv( const QString& userID , const QString&ip , const int port ,const int screenIndex)
{
	if( screenIndex>_confRoomVector.size()-1 || _confRoomVector.size()==0  )
	{
		_snprintf(__global_msg , sizeof(__global_msg), "�����Ļ��ʱ������Ļ����ֵΪ%d����Ļ��ĿΪ:%d" , screenIndex, _confRoomVector.size() );
		CV_LOG_ERROR(__global_msg);
		return ;
	}


	
	//�����ͷ�ԭ������Ļ������
	_channelDispatcherProxy->ReleaseChannel( RECV_SCREEN_MEDIA_ID , SCREEN_MEDIA_SERVICE_NAME);


	//��֪��Ļˢ��
	int index = 0;
	if( screenIndex>0 )
		index = screenIndex;
	QtConferenceRoom* pConfRoom = _confRoomVector.at(index);
	if( pConfRoom && pConfRoom->getScreenShareWidget() )
	{
		//�ȹر��Ѿ��򿪵��ĵ����ͷ�ý����
		pConfRoom->getScreenShareWidget()->release();

		
		pConfRoom->getScreenShareWidget()->recvScreen(true );
		pConfRoom->getScreenShareWidget()->UserID(userID );
		pConfRoom->getScreenShareWidget()->setMediaState( UiMediaState_Initializing );
		
		ScreenMediaInfo info;
		info._port = port;
		info._ipAddress = ip;

		//����µ���Ļ������
		QByteArray outputBytes;
		QDataStream out(&outputBytes, QIODevice::WriteOnly );
		out << info;
		_channelDispatcherProxy->CreateChannel( RECV_SCREEN_MEDIA_ID, SCREEN_MEDIA_SERVICE_NAME,
			outputBytes );

		QString memberName = userID;
		int index = memberName.indexOf("@");
		if( index>-1 )
		{
			memberName = memberName.left(index);
		}
		this->MemberLocationAddNotify( memberName, screenIndex , 10 , false , false );
	}
}


void MediaWindowManager::AddMainSpeakerScreenMediaRecv( const QString& userID , const QString&ip , const int port )
{

	
	//�����ͷ�ԭ������Ļ������
	_channelDispatcherProxy->ReleaseChannel( RECV_SCREEN_MEDIA_ID , SCREEN_MEDIA_SERVICE_NAME);


	for(int i=0 ; i<_confRoomVector.size();i++ )
	{
		if( _confRoomVector.at(i)&& _confRoomVector.at(i)->getScreenShareWidget() )
		{
			QtConferenceRoom* pConfRoom = _confRoomVector.at(i);
			//�ȹر��Ѿ��򿪵��ĵ����ͷ�ý����
			pConfRoom->getScreenShareWidget()->closeFile();
			pConfRoom->getScreenShareWidget()->stopScreenGraph();


			pConfRoom->getScreenShareWidget()->recvScreen(true );
			pConfRoom->getScreenShareWidget()->UserID(userID );
			pConfRoom->getScreenShareWidget()->setMediaState( UiMediaState_Initializing );
		}	
	}

	ScreenMediaInfo info;
	info._port = port;
	info._ipAddress = ip;

	//����µ���Ļ������
	QByteArray outputBytes;
	QDataStream out(&outputBytes, QIODevice::WriteOnly );
	out << info;
	_channelDispatcherProxy->CreateChannel( RECV_SCREEN_MEDIA_ID, SCREEN_MEDIA_SERVICE_NAME,
		outputBytes );
}


void MediaWindowManager::RemoveScreenMediaRecv( const QString& userID,const int screenIndex/*=-1 */ )
{
	if( screenIndex>_confRoomVector.size()-1 || _confRoomVector.size()==0  )
	{
		_snprintf(__global_msg , sizeof(__global_msg), "�����Ļ��ʱ������Ļ����ֵΪ%d����Ļ��ĿΪ:%d" , screenIndex, _confRoomVector.size() );
		CV_LOG_ERROR(__global_msg);
		return ;
	}

	if( screenIndex>=0&& screenIndex<_confRoomVector.size()  )
	{
		//ֱ�ӻ�ȡ�����ҵ�ָ�벢ֹͣ��Ļ��
		QtConferenceRoom* pConfRoom = _confRoomVector.at(screenIndex );
		if( pConfRoom && pConfRoom->getScreenShareWidget() && pConfRoom->getScreenShareWidget()->UserID()==userID )
		{
			//�ͷ���Ļ������
			_channelDispatcherProxy->ReleaseChannel( RECV_SCREEN_MEDIA_ID , SCREEN_MEDIA_SERVICE_NAME);

			//ֹͣˢ��
			pConfRoom->getScreenShareWidget()->recvScreen(false );
			pConfRoom->getScreenShareWidget()->UserID("");
			pConfRoom->getScreenShareWidget()->setMediaState(UiMediaState_Destroy);

			QString memberName = userID;
			int index = memberName.indexOf("@");
			if( index>-1 )
			{
				memberName = memberName.left(index);
			}
			this->MemberLocationRemoveNotify( memberName , screenIndex , 10 );
			return;
		}
	}else
	{
		//����ö�����й�����Ļ������userID
		int screenSize = _confRoomVector.size();
		for( int i=0 ; i<screenSize; i++ )
		{
			QtConferenceRoom* pConfRoom = _confRoomVector.at(i);
			if( pConfRoom && pConfRoom->getScreenShareWidget()&&
				(userID=="" || pConfRoom->getScreenShareWidget()->UserID()==userID) )
			{

				//�ͷ���Ļ������
				_channelDispatcherProxy->ReleaseChannel( RECV_SCREEN_MEDIA_ID , SCREEN_MEDIA_SERVICE_NAME);

				//ֹͣˢ��
				pConfRoom->getScreenShareWidget()->recvScreen(false );
				pConfRoom->getScreenShareWidget()->UserID("");
				pConfRoom->getScreenShareWidget()->setMediaState(UiMediaState_Destroy);

				QString memberName = userID;
				int index = memberName.indexOf("@");
				if( index>-1 )
				{
					memberName = memberName.left(index);
				}
				this->MemberLocationRemoveNotify( memberName , i , 10 );

			}
		}
	}
}

void MediaWindowManager::MediaStateChangedNotify( const QString&memberName , const QString&mediaType, const QString&mediaState, const int controllerIndex/*=-1 */ )
{
	QByteArray output_array;
	QDataStream out(&output_array , QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << TELE_MediaState;

	MediaStateNotify notify;
	notify._memberName = memberName;
	notify._mediaState = mediaState;
	notify._mediaType = mediaType;
	out << notify;

	if( _teleControlProxy )
	{
		_teleControlProxy->TeleInfo(TELE_MediaState , controllerIndex,output_array );
	}
}

void MediaWindowManager::MemberLocationAddNotify( const QString& memberName , const int screen , const int seet , const bool audioEnable , const bool videoEnable )
{
	QByteArray output_array;
	QDataStream out(&output_array , QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << TELE_AddMemberLocation;

	MemberLocation location;
	location._memberName = memberName;
	location._screenIndex = ScreenHelper::Instance()->ScreenIndexToPosIndex(screen);
	location._seet = seet;

	location._audioEnable = audioEnable;
	location._videoEnable = videoEnable;

	out << location;

	if( _teleControlProxy )
	{
		_teleControlProxy->TeleInfo(TELE_AddMemberLocation,-1,output_array );
	}
}

void MediaWindowManager::MemberLocationRemoveNotify( const QString& memberName , const int screen , const int seet  )
{
	QByteArray output_array;
	QDataStream out(&output_array , QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << TELE_RemoveMemeberLocation;

	out << memberName;
	out << ScreenHelper::Instance()->ScreenIndexToPosIndex(screen);
	out << seet;
	if( _teleControlProxy )
	{
		_teleControlProxy->TeleInfo(TELE_RemoveMemeberLocation,-1,output_array );
	}
}

void MediaWindowManager::MemberLocationChangeNotify( const QString& memberName , const int newScreenIndex, const int newSeet )
{
	QByteArray output_array;
	QDataStream out(&output_array , QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << TELE_ChangeMemberLocation;

	out << memberName;
	out << ScreenHelper::Instance()->ScreenIndexToPosIndex(newScreenIndex);
	out << newSeet;
	if( _teleControlProxy )
	{
		_teleControlProxy->TeleInfo(TELE_ChangeMemberLocation,-1,output_array );
	}
}




void MediaWindowManager::KinectGestureSlot( const QString &to, int type )
{
	//QMessageBox::information(0,"Kinect", QString::number(type) , QMessageBox::Ok );

	//��ָ��������ת��ΪPPT��������
	QtScreenShare::PPTControlType pptControlType = QtScreenShare::PPTControl_Unknow;
	switch( type )
	{
	case KG_LeftHandLift:
		pptControlType = QtScreenShare::PPTControl_Pre;
		break;
	case KG_RightHandLift:
		pptControlType = QtScreenShare::PPTControl_Next;
		break;
	case KG_RightHandUp:
		pptControlType = QtScreenShare::PPTControl_Close;
		break;
	}

	//ֱ�ӻ�ȡ�����ҵ�ָ�벢ֹͣ��Ļ��
	for( int i=0 ; i<_confRoomVector.size(); i++ )
	{
		QtConferenceRoom* pConfRoom = _confRoomVector.at(i);
		if( pConfRoom && pConfRoom->getScreenShareWidget())
		{
			//������Ļ��PPT�Ĳ���
			pConfRoom->getScreenShareWidget()->controlPPT( pptControlType );
		}
	}
}

void MediaWindowManager::KinectMessageSlot( int type, const QString &message )
{
	static QString lastMessage = "";
	switch( type )
	{
	case KM_Normal:
		_kinectInfoWidget->hide();
		break;
	case KM_Information:
	case KM_Warning:
	case KM_Error:
		{
			if( _kinectInfoWidget->isHidden()&&lastMessage!=message )
			{
				//ͬһ����Ϣֻ��ʾһ��
				_kinectInfoWidget->SetInfoText( message );
				_kinectInfoWidget->show();
				lastMessage = message;
			}

		}
		
		break;
	}
}

void MediaWindowManager::ControlScreenShare( const bool control )
{
	int screenSize = _confRoomVector.size();
	for( int i=0 ; i<screenSize; i++ )
	{
		QtConferenceRoom* pConfRoom = _confRoomVector.at(i);
		if( pConfRoom && pConfRoom->getScreenShareWidget() )
		{
			pConfRoom->getScreenShareWidget()->ControlScreenShare( control );
		}
	}
}


void MediaWindowManager:: OpenScreenShareFile(const QString &filePath, int screenIndex)
{
	if(screenIndex >= _confRoomVector.size() || screenIndex<0)
		return;

	QtConferenceRoom* pConfRoom = _confRoomVector.at(screenIndex);
	pConfRoom->getScreenShareWidget()->openFile(filePath,screenIndex);


}


void MediaWindowManager:: RemoveSendShareScreen()
{
	
	for(int i=0; i<_confRoomVector.size();i++)
	{
		QtConferenceRoom* pConfRoom = _confRoomVector.at(i);
		pConfRoom->getScreenShareWidget()->closeFile();
		pConfRoom->getScreenShareWidget()->stopScreenGraph();
	}

}


void MediaWindowManager:: ScreenShareStateUpdateNotify( const int controllerIndex /*=-1*/)
{
	bool shareState = false;
	
	for(int i=0; i<_confRoomVector.size();i++)
	{
		QtConferenceRoom* pConfRoom = _confRoomVector.at(i);
		if(pConfRoom->getScreenShareWidget()->getScreenShareState())
		{
			shareState = true;
			break;
		}
		
	}

	QByteArray output_array;
	QDataStream out(&output_array , QIODevice::WriteOnly );
	out<<TELE_ScreenShareState;
	out << shareState;

	if( _teleControlProxy )
	{
		_teleControlProxy->TeleInfo( TELE_ScreenShareState , controllerIndex, output_array );
	}

}

void MediaWindowManager::ControlVideoRecording( const QString userId, int type, QByteArray param )
{
  if (_coreProxy) {
    QByteArray output_array;
    QDataStream out(&output_array , QIODevice::WriteOnly );
    out << userId << type << param;

    _coreProxy->TeleCommand(CoreAction_ControlVideoRecord, output_array);
  }
}

void MediaWindowManager::updateScreenSlot()
{
  ScreenHelper *helper = ScreenHelper::Instance();
  helper->Update();
  int screen_count = helper->GetScreenCount();
  if (CVIniConfig::getInstance().isAutoSetPrimaryScreen() && 
      helper->SetPrimaryScreen(screen_count - 1)){
    for (int i = 0; i < screen_count; ++i) {
      _confRoomVector[i]->showMaximizedToScreen();
    }
  }
}
