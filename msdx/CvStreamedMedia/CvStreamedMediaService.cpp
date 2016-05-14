#include "CvStreamedMediaService.h"

#include <Windows.h>
#include <stdio.h>
#include <process.h>
#include <Psapi.h>

#include <QtCore/QCoreApplication>

#include <log/Log.h>
#include <msdx/ExportUtil.h>
#include <dbus/channel/type/streamedMedia/service/CvStreamedMediaAdaptor.h>
#include <dbus/channelDispatcher/client/ChannelDispatcherIf.h>
#include <dbus/channelDispatcher/common/ChannelDispatcherServiceCommon.h>
#include <msdx/config.h>
#include <util/report/RTCPReportWriter.h>
#include "NConfig.h"
#include "QosReportSender.h"



CvStreamedMedia* CvStreamedMedia::_instance = NULL;
QString CvStreamedMedia::_mediaID;
QString CvStreamedMedia::_userID;

#define VIDEOCAPTRUEDEVICE "Logitech HD Webcam C270" //"USB2.0 UVC 1.3M WebCam"

//#define VIDEOCAPTRUEDEVICE "USB2.0 PC Camera" //"USB2.0 UVC 1.3M WebCam"
#define AUDIO_INPUT_DEVICE "����ӳ����Input"
#define AUDIO_OUTPUT_DEVICE "����ӳ����Out"


///���ڲ���rtcp����Ļص������Ƿ�����
void testRtcpReportCallback( void* pObject , const RtpStatItemInfo& info )
{
	RtpStatItem item;
	RtpStatItemInfo::RtpStatItemInfo2RtpStatItem( info , item );
	printRtpStat( NULL , &item );
}

//
// functions
//

QDataStream& operator>>( QDataStream& in, StreamMediaStatus &status )
{
    in >> status.video_send_dest >> status.audio_send_dest >>
        status.enable_video >> status.enable_audio >> status.window_handle;
    return in;
}

QDataStream& operator<<( QDataStream& out, StreamMediaStatus& status )
{
    out << status.video_send_dest << status.audio_send_dest 
        << status.enable_video << status.enable_audio << status.window_handle;
    return out;
}

//
// CvStreamedMedia
//

CvStreamedMedia::CvStreamedMedia()
{

}

/**
 * @brief ��ʼ��
 * @param mediaID ���ڱ�ʶÿһ·ý��������ݣ��������û�ID
 */
bool CvStreamedMedia::Init( const QString mediaID )
{
	msdx_initial();
	new CvStreamedMediaAdaptor(this);
	QDBusConnection connection = QDBusConnection::sessionBus();
	QString serviceName = QString(STREAMED_MEDIA_SERVICE_NAME) + QString(".") + mediaID;
	QString objectPath = QString(STREAMED_MEDIA_SERVICE_OBJECT_PATH) + QString("/") + mediaID;

	bool ret = connection.registerService(serviceName);
	if (!ret)
	{
		LOG_ERROR("registerService failed. name=%s", serviceName.toLocal8Bit().data());
		return false;
	}
	ret = connection.registerObject(objectPath ,this);
	if (!ret)
	{
		LOG_ERROR("registerObject failed. name=%s", objectPath.toLocal8Bit().data());
		return false;
	}

	this->_mediaDirection = MediaDirection_Unknow;
	this->_graphID = -1;
	this->_instance = this;
	this->_mediaID = mediaID;
	this->_userID = "";
	this->_videoWidth = 0;
	this->_videoHeight = 0;
    this->_file_play_controller = nullptr;

    ResetBackupStatus();
    stream_media_status_.valid = true;
	backup_manager_ = BackupManager::Instance();

	if( mediaID=="test" )
	{
		//���Է���
		char remoterAddr[50];
		strcpy_s( remoterAddr, "127.0.0.1");
		SendGraphInfo info;
		info.initial( 0,VIDEOCAPTRUEDEVICE , AUDIO_INPUT_DEVICE , AUDIO_OUTPUT_DEVICE , MSDX_CONF_AUDIO_INPUT_NOAEC, 
            remoterAddr,8500,"SPEEX" ,
			16000, 16 , 1, false, remoterAddr , 8600,30,false, 0 , 640 ,480, "" , "" );
		QByteArray bytesArray;
		QDataStream out(&bytesArray , QIODevice::WriteOnly );
		out.setVersion( QDataStream::Qt_4_4 );
		out << info;

		this->SetMediaInfo( Action_InitialSendGraph , bytesArray );
	}
		
	// ChannelDispatcher
	_channelDispatcher = new ChannelDispatcherIf( CHANNEL_DISPATCHER_SERVICE_NAME,
		CHANNEL_DISPATCHER_OBJECT_PATH, QDBusConnection::sessionBus() );
	//ע��:֮����ֱ�ӵ���ChannelDispatcher�ķ��������Ƿ���RunningStateChanged�ź�,����Ϊ
	//Ŀǰ����ChannelDispatcherΪDBus����(��dbus\services�������ļ�),��ChannelDispatcher
	//������,��������ź�,ChannelDispatcher���ᱻ����,�������䷽��DBus���Զ���������
	//Liaokz, 2013-11����˵��
	SetCurrentState(StreamedMedia_NoInitial);

	LOG_INFO("StreamedMedia[%s] initialized.", mediaID.toLocal8Bit().data());

  connect(&video_window_check_timer_, &QTimer::timeout,
    this, &CvStreamedMedia::HandleVideoWindowCheckTimerTimeoutSlot);
  video_window_handle_ = 0;

	return true;
}

CvStreamedMedia::~CvStreamedMedia()
{
	msdx_uninitial();
}

/**
 * @brief �˳�����
 */
void CvStreamedMedia::Destroy()
{
	LOG_DEBUG("Destroy media %s" , _mediaID.toLocal8Bit().data() );
	int result = 0;

	if (-1 != _graphID)
	{
        ResetBackupStatus();
        BackupStatus();

		if( _mediaDirection==MediaDirection_In )
		{
      video_window_check_timer_.stop();
			result = msdx_destroy_recv_graph(_graphID);
		}
        else if( _mediaDirection==MediaDirection_out )
		{
			result = msdx_destroy_send_graph();
		}
        else if( _mediaDirection==MediaDirection_PlayOut)
        {
          if (_file_play_controller)
          {
            _file_play_controller->Stop();
            delete _file_play_controller;
            _file_play_controller = nullptr;
          }
        }

		if( result!=0 )
		{
			LOG_ERROR("����ý����ʱ��������");
		}else
		{
			LOG_DEBUG("�ɹ�����ý����");
			emit RunningStateChanged(_mediaID, StreamedMedia_Destroyed);
		}
	}
	
	QCoreApplication::instance()->quit();	
}

void CvStreamedMedia::Stop()
{
	LOG_DEBUG( "Start stopping media graph" );
	LOG_DEBUG("Stop Media graph" );

	int result = -1;
	if( _currentState!=StreamedMedia_Stop )
	{
		if( _mediaDirection==MediaDirection_In)
		{
			result = msdx_stop_recv_graph(_graphID);
		}else if( _mediaDirection==MediaDirection_out)
		{
			result = msdx_stop_send_graph();
		} else if (_mediaDirection == MediaDirection_PlayOut && _file_play_controller)
    {
      _file_play_controller->Stop();
		}
	}
	LOG_DEBUG("Stop action done");
	if( result==0 )
	{
		SetCurrentState(StreamedMedia_Stop);
	}

	LOG_DEBUG( "Stop graph successfully");
}

void CvStreamedMedia::Pause()
{
// 	int result = -1;
// 	if( _currentState==StreamedMedia_Running && _graphID>=0 )
// 	{
// 		//ֻ��Graph�����е�ʱ����ܹ���ͣ
// 		if( _mediaDirection == MediaDirection_In )
// 		{
// 			result = msdx_pause_recv_graph(_graphID);
// 		}else if( _mediaDirection==MediaDirection_out )
// 		{
// 			result = msdx_pause_send_graph();
// 		}
// 	}
// 	if( result==0 )
// 	{
// 		SetCurrentState(StreamedMedia_Pause);
// 	}
}

void CvStreamedMedia::Run()
{
	LOG_DEBUG( "Run Graph Command");
	int result = -1;
	if( _currentState!=StreamedMedia_Running  )
	{
		//���Graph��û���У�������Graph
		if( _mediaDirection == MediaDirection_In )
		{
			LOG_DEBUG("Run Recv Graph");
			//RecvGraph
			if( _currentState==StreamedMedia_Pause)
				//���Graph������ͣ״̬����ô�ظ�����
				result = msdx_resume_recv_graph(_graphID );
			else
				//������ֱ������
				result = msdx_run_recv_graph(_graphID);
		}else if( _mediaDirection==MediaDirection_out )
		{
			LOG_DEBUG("Run Send Graph");
			//SendGraph
			if( _currentState==StreamedMedia_Pause)
				result = msdx_resume_send_graph(); //����֧�֣�����ʲôҲ����
			else
				result = msdx_run_send_graph();
		} else if (_mediaDirection == MediaDirection_PlayOut && _file_play_controller)
    {
      _file_play_controller->Run();
		}
	}
	if( result==0 )
	{
		SetCurrentState(StreamedMedia_Running);
	}
}

void CvStreamedMedia::SetMediaInfo( int action_id, const QByteArray &input_garray )
{
	QDataStream out( input_garray );
	out.setVersion(QDataStream::Qt_4_4 );
	switch( action_id )
	{
	case Action_InitialSendGraph:
		{
			LOG_DEBUG("Initial Send Graph");
			if( _currentState!=StreamedMedia_NoInitial )
			{
				LOG_DEBUG("Send Graph�Ѿ���ʼ��������ִ�г�ʼ��������");
				return;
			}

			SendGraphInfo info;
			out >> info;
			_userID = _mediaID;

            int result = 0;
            _mediaDirection = MediaDirection_out;
			result = CreateSendGraph( info );
			if( result>=0)
			{
				SetCurrentState(StreamedMedia_Running);

				//�������ȼ�Ϊʵʱ
				::SetPriorityClass( ::GetCurrentProcess() , HIGH_PRIORITY_CLASS);
			}

			return;
		}
		break;
	case Action_InitialRecvGraph:
		{
			LOG_DEBUG("Initial Recv Graph");
			if (_mediaID.indexOf(SEND_STREAMED_MEDIA_ID) != -1)
			{
				LOG_DEBUG("Cannot create recv graph in send process!");
				return;
			}

			//������ԭ����ý����
            if( _graphID>0  ) {
                msdx_destroy_recv_graph(_graphID);
                SetCurrentState(StreamedMedia_Destroyed);
            }

			RecvGraphInfo info;
			out >> info;
            _userID = info.user_id;
            _mediaDirection = MediaDirection_In;
			int result = CreateRecvGraph( info );
			if( result >=0 )
			{
				SetCurrentState(StreamedMedia_Running);

				//�������ȼ�Ϊ��
				::SetPriorityClass( ::GetCurrentProcess() , ABOVE_NORMAL_PRIORITY_CLASS );
			}
			return;
		}
		break;
	case Action_InitialSmallVideoGraph:
		{
			LOG_DEBUG("Initial Small Video Graph");
            if( _graphID>0  ) {
                msdx_destroy_recv_graph(_graphID);
                SetCurrentState(StreamedMedia_Destroyed);
            }

			SendGraphInfo info;
			out >> info;

			RecvGraphInfo recvInfo;
			recvInfo.initial( "OUTPUT" , "INPUT" , "0.0.0.0" , 0, "" , 0, 0, 0, 
				"127.0.0.1" , info.net_info.video_port, info.preview_wnd , info.video_info.width ,info.video_info.height , info.video_info.fps, 
				qPrintable(_mediaID) , qPrintable(_mediaID), true );
			_userID = recvInfo.user_id;
			_mediaDirection = MediaDirection_out;
			int result = CreateSmallVideoGraph( recvInfo );
			if( result >=0 )
			{
				SetCurrentState(StreamedMedia_Running);

				//�������ȼ�Ϊ��
				::SetPriorityClass( ::GetCurrentProcess() , ABOVE_NORMAL_PRIORITY_CLASS );
			}
			return;
		}
		break;
  case Action_InitialFilePlayGraph:
    {
      LOG_DEBUG("Initial File Play Graph");
      FilePlayInfo info;
      out >> info;
      _userID = info.vuser_id;
      _mediaDirection = MediaDirection_PlayOut;
      int result = CreateFilePlayGraph( info );
      if( result >=0 )
      {
        SetCurrentState(StreamedMedia_Running);

        //�������ȼ�Ϊ��
        ::SetPriorityClass( ::GetCurrentProcess() , ABOVE_NORMAL_PRIORITY_CLASS );
      }
      return;
    }
    break;
	case Action_SetVideoWindow:
		{
			LOG_DEBUG("Set Video Window");
			WndInfo info;
			out >> info;
			ResetVideoWindow( info.video_hwnd );
			return;
		}
		break;
	case Action_AddSendDst:
		{
			LOG_DEBUG("Add Send Dst");
			NetInfo info;
			out >> info;
			if(_mediaDirection == MediaDirection_out)
			{
                AddSendDest(info);
			}
			return;
		}
		break;
	case Action_RemoveSendDst:
        {
            LOG_DEBUG("Remove Send Dst");
			NetInfo info;
			out >> info;
			if (_mediaDirection == MediaDirection_out) 
            {
                RemoveSendDest(info);
			}
			return;
		}
		break;
	case Action_SetQoSInfo:
		{
            //deprecated
            /*LOG_DEBUG("Set DSCP");
            QosDscpInfo info;
            out >> info;
            if( _mediaDirection==MediaDirection_out )
            {
            msdx_send_set_video_dscp( info.video_dscp );
            msdx_send_set_audio_dscp( info.audio_dscp );
            }*/
			return;
		}
		break;
	case Action_ControlVideoSend:
		{
			bool	sendVideo;
			out >> sendVideo;
			LOG_DEBUG("Control VideoSend:%d" , sendVideo );
			EnableVideo(sendVideo);
			return;
		}
		break;
	case Action_ControlAudioSend:
		{
			bool	sendAudio;
			out >> sendAudio;
			LOG_DEBUG("Control AudioSend:%d" , sendAudio );
			EnableAudio(sendAudio);
			return;
		}
		break;
	case Action_EnableAutoResync:
		{
            //deprecated
            /*bool	enable;
            out >> enable;
            LOG_DEBUG("Enable Auto Resync:%d" , enable );
            if( _mediaDirection==MediaDirection_In )
            {
            msdx_recv_all_enable_auto_resync(enable);
            }*/
			return;
		}
		break;
	case Action_ControlAudioRecv:
		{
			bool enable;
			out >> enable;
			LOG_DEBUG("Control Audio Recv:%d" , enable );
			EnableAudio(enable);
			return;
		}
		break;
	case Action_ControlVideoRecv:
		{
			bool enable;
			out >> enable;
			LOG_DEBUG("Control Audio Recv:%d" , enable );
			EnableVideo(enable);
			return;
		}
		break;
	case Action_AdaptiveControlCodeRate:
		{
            //deprecated
			/*if( _mediaDirection==MediaDirection_out )
			{		
				bool enable;
				out >> enable;
				msdx_send_enable_adaptive_coderate(enable);
			}*/
			return;
		}
	case Action_CutRecordingVideo:
		{
			CutRecordInfo cut;
			out >> cut;
			LOG_DEBUG("Cut recording: jobid=%d" , cut.job_id);

			// �����Ƿ��ͷ�������Ҫ����շ��ָ�
			if (_mediaID.indexOf(SEND_STREAMED_MEDIA_ID) == 0)
			{
				msdx_send_cut_file();
			}
			// �����ǽ��շ������յ��ָ�ָ��ʱ�ָ�
			else
			{
				if (cut.file_name.length() > 0) {
					msdx_record_cut_file(_graphID, cut.job_id, qPrintable(cut.file_name));
				} else {
					msdx_record_cut_file(_graphID, cut.job_id, NULL);
				} 
				}
			return;
		}
  case Action_ShowConsoleWindow:
        {
            bool is_show = false;
            out >> is_show;
            LOG_DEBUG("Show console window: %s" , is_show ? "true" : "false" );
            HWND console_window = GetConsoleWindow();
            ShowWindow(console_window, is_show ? SW_SHOW : SW_HIDE);
            return;
        }
  case Action_PlayControl:
      {
          FilePlayCtrlInfo info;
          out >> info;
          ControlPlay(info);
          return;
      }
	default:
		LOG_PRINTF("Invalid SetMediaInfo state code %d\n", action_id);
	}
}

int CvStreamedMedia::CreateRecvGraph( const RecvGraphInfo& info )
{
	LOG_INFO("CreateRecvGraph->AUDIO_INPUT_DEVICE:%s, AUDIO_OUTPUT_DEVICE:%s \n"
		"\tIP:%s,VIDEO_PORT:%d , AUDIO_PORT:%d \n"
		"\tVIDEO_CODEC:%s , AUDIO_CODEC:%s,\n"
		"\tVIDEO_WIDTH:%d ,  VIDEO_HEIGHT:%d, VIDEO_FPS:%d \n"
		"\tAUDIO_CHANNELS:%d , AUDIO_SAMPLE_RATE:%d, AUDIO_SAMPLE_BTIS:%d, AUDIO_ENABLE:%d\n"
		"\tRecord:%s, JobId:%d \n" ,  
		qPrintable(info.audio_input_device), qPrintable(info.audio_output_device ),
		qPrintable(info.net_info.ip_addr) , info.net_info.video_port , info.net_info.audio_port ,
		qPrintable(info.video_info.codec_name) , qPrintable( info.audio_info.codec_name ) , 
		info.video_info.width,   info.video_info.height, info.video_info.fps,
		info.audio_info.channels , info.audio_info.sample_rate, info.audio_info.bits_per_sample, info.audio_info.enable,
        qPrintable(info.record_info.file_name), info.record_info.job_id);

	// Create Graph

	int xpos = 0, ypos = 0;
	float width = 0, height = 0;
	int result;
	int already_sending = 0;
	RECT WindowRect;

	//������Ƶ�����豸 zhenHua.sun 20110307
	msdx_config_set_audio_device( qPrintable(info.audio_input_device) , qPrintable(info.audio_output_device) );

	//��������graph
	_graphID = msdx_create_recv_graph(qPrintable(info.user_id));
	if(_graphID < 0)
	{
		LOG_ERROR("��������Graphʱ��������");
		goto ErrorOut;
	}
	
	//Tx -- By Liaokz
	if (info.record_info.file_name.length() > 0)
	{
		msdx_set_record_file_name(_graphID, qPrintable(info.record_info.file_name));
    //¼����ҵid
    msdx_set_record_jobid(_graphID, info.record_info.job_id);
	}

	if(info.net_info.audio_port == 0)
	{
		//��֧����Ƶ��ֱ��ȥ��Ƶ
		LOG_WARN("��������Ƶ��Ϣ");
		goto video;
	}

	char codecName[256] = {0};
	strcpy_s( codecName , qPrintable(info.audio_info.codec_name ) ); 
	result = msdx_set_recv_audio_codec(_graphID,
		codecName,
		info.audio_info.sample_rate,
		info.audio_info.channels,
		info.audio_info.bits_per_sample);
	if(result != 0)
	{
		LOG_ERROR("������Ƶ������ʱ��������graphID:%d , codec:%s , sample_rate:%d , channel:%d,bits:%d" , _graphID , qPrintable(info.audio_info.codec_name) ,
			info.audio_info.sample_rate , info.audio_info.channels , info.audio_info.bits_per_sample );
		goto ErrorOut;
	}

video:

	if(info.net_info.video_port == 0)
	{
		LOG_WARN("��������Ƶ��Ϣ");
		goto NoRecvVideo;
	}

	_videoWidth = info.video_info.width;
	_videoHeight = info.video_info.height;
	memset( codecName , 0 , sizeof(codecName) );
	strcpy_s(codecName , qPrintable(info.video_info.codec_name) );
	result = msdx_set_recv_video_codec(_graphID,
		codecName,
		info.video_info.width,
		info.video_info.height,
		info.video_info.fps,
		info.video_info.bit_count);
	if(result != 0)
	{
		LOG_ERROR("������Ƶ������ʱ��������");
		goto ErrorOut;
	}

NoRecvVideo:
	result = msdx_set_recv_address(_graphID,
		info.net_info.ip_addr.toStdString().c_str(),
		info.net_info.audio_port,
		info.net_info.ip_addr.toStdString().c_str(),
		info.net_info.video_port);
	if(result != 0)
	{
		LOG_ERROR("������Ƶ���յ�ַʱ��������");
		goto ErrorOut;
	}

	result = msdx_connect_recv_graph(_graphID);
	if(result != 0)
	{
		LOG_ERROR("����Filterʱ��������");
		goto ErrorOut;
    }

	//����RTPͳ�ƻص�����
	msdx_recv_set_video_rtpstatcallback(_graphID, NULL, deliverRtpStatInfo);
	msdx_recv_set_audio_rtpstatcallback(_graphID, NULL, deliverRtpStatInfo);
	//msdx_recv_enable_auto_resync(_graphID, info.autoResync); //��ʹ��ʱ�ӣ�������Ҫ�˹��ܿ���ͬ�� --Liaokz,2014-3

	//Tx -- By Liaokz
	msdx_set_record_callback(_graphID, this, RecordCallback);

	//������ʾ�Ĵ��ھ��
	if(info.recv_wnd && info.video_info.width>0 & info.video_info.height>0)
	{
		ResetVideoWindow(info.recv_wnd);
	}

	result = msdx_run_recv_graph(_graphID);
	if(result != 0)
	{
		LOG_ERROR("����RecvGraph��ʱ�����쳣");
		goto ErrorOut;
	}

  //�Ƿ�����Ƶ
  EnableAudio(info.audio_info.enable);

  video_window_check_timer_.start(1000);

	LOG_DEBUG( "RecvGraph �����ɹ�");
	return _graphID;

ErrorOut:

	msdx_destroy_recv_graph(_graphID);

	return -1;
}

int CvStreamedMedia::CreateSendGraph( const SendGraphInfo& info )
{
	LOG_INFO("CreateSendGraph->IP:%s , VIDEO_PORT:%d , AUDIO_PORT:%d \n"
		"\tVIDEO_CODEC:%s , AUDIO_CODEC:%s \n"
		"\tVIDEO_DEVICE:%s \n"
		"\tVIDEO_WIDTH:%d ,  VIDEO_HEIGHT:%d, VIDEO_FPS:%d, SMALL_VIDEO:%d \n"
		"\tCROSSBAR:%s , CROSSBAR_INPUT_TYPE:%s,\n"
        "\tAUDIO_INPUT_DEVICE:%s , AUDIO_OUTPUT_DEVICE:%s, AUDIO_INPUT_TYPE:%d\n"
		"\tAUDIO_CHANNELS:%d , AUDIO_SAMPLE_RATE:%d, AUDIO_SAMPLE_BTIS:%d, AUDIO_ENABLE:%d\n",  
		qPrintable(info.net_info.ip_addr) , info.net_info.video_port , info.net_info.audio_port ,
		qPrintable(info.video_info.codec_name) , qPrintable( info.audio_info.codec_name ) , qPrintable(info.device.video_capture),
		info.video_info.width,   info.video_info.height, info.video_info.fps, info.net_info.enable_small_video,
		qPrintable(info.device.video_crossbar) , qPrintable( info.device.video_crossbar_type) ,
		qPrintable(info.device.audio_input_device), qPrintable( info.device.audio_output_device), info.device.audio_input_type,
		info.audio_info.channels , info.audio_info.sample_rate, info.audio_info.bits_per_sample,(int)info.audio_info.enable);
    
	int result = -1;

	int already_sending = 0;
	//�Ѿ�����send graph
	if(msdx_get_send_graph( ) == -1)
	{
		msdx_config_set_video_device(
			qPrintable(info.device.video_capture), 
			qPrintable(info.device.video_crossbar),
			qPrintable(info.device.video_crossbar_type));
		//MessageBox(NULL,"setVideoDevice","a",MB_OK);

		msdx_config_set_audio_device(
			qPrintable(info.device.audio_input_device),
			qPrintable(info.device.audio_output_device));
		//MessageBox(NULL,"setAudioDevice","a",MB_OK);
		msdx_create_send_graph(info.net_info.enable_small_video);
		//MessageBox(NULL,"createSendGraph","a",MB_OK);
	}else
	{
		LOG_DEBUG( "Send Graph Already Exits.");
		already_sending = 1;
	}

	// �޼�C270����ͷ��˷���Ҫ�����ø�ʽ��������AEC���޷���ȷ��ʼ�����ᷢ��֨֨��
	if (info.device.audio_input_device.indexOf("Webcam") != -1)
	{
		LOG_INFO("\nWarm-up microphone device %s", qPrintable(info.device.audio_input_device));
		result = msdx_create_audio_test_graph(MSDX_LOCAL_ATYPE_DEVICE, qPrintable(info.device.audio_input_device), NULL);
		if (0 == result)
		{
			msdx_destroy_audio_test_graph();
			LOG_INFO("Warm-up successful.");
		}
		else
		{
			LOG_INFO("Warm-up failed.");
		}
	}

	if(!already_sending)
	{
		if(info.device.video_capture.length() == 0)
		{
			//��֧����Ƶ��ֱ������
			LOG_INFO("No Video device.");
			goto NoCaptureVideo;
		}

		//��Ƶ������,������������Ƶ������
		_videoWidth = info.video_info.width;
		_videoHeight = info.video_info.height;
		result = msdx_set_send_video_vodec(qPrintable(info.video_info.codec_name),
			info.video_info.width,
			info.video_info.height,
            info.video_info.fps);
		//MessageBox(NULL,"setVideoCodec","a",MB_OK);
		if(result != 0)
		{
			LOG_ERROR("������Ƶ���������ʱ�������󣡣�%s, %d, %d��" , 
				qPrintable(info.video_info.codec_name) , info.video_info.width , info.video_info.height);
			goto ErrorOut;
		}
	}

NoCaptureVideo:

	if(!already_sending)
	{
		//��Ƶ������
		result = msdx_set_send_audio_codec(qPrintable(info.audio_info.codec_name),
			info.audio_info.sample_rate,
			info.audio_info.channels,
			info.audio_info.bits_per_sample,
            info.device.audio_input_type != MSDX_CONF_AUDIO_INPUT_NOAEC); //TODO:Ŀǰֻ��NoAEC��WinAEC
		//MessageBox(NULL,"setAudioCodec","a",MB_OK);
		if(result != 0)
		{
			LOG_ERROR("������Ƶ��������ʱ��������");
			goto ErrorOut;
		}

	}

	if(!already_sending)
	{
		result = msdx_connect_send_graph();
		//MessageBox(NULL,"connect_send_graph","a",MB_OK);
		if(result != 0)
		{
			LOG_ERROR("����Filter��ʱ��������");
			goto ErrorOut;
		}
    }

    //����RTPͳ�ƻص�����
    if( !already_sending )
    {
        msdx_send_set_video_rtpstatcallback(NULL, deliverRtpStatInfo);
        msdx_send_set_audio_rtpstatcallback(NULL, deliverRtpStatInfo);
        msdx_send_set_video_dscp(info.dscp_info.video_dscp);
        msdx_send_set_audio_dscp(info.dscp_info.audio_dscp);
    }

    //���Բ����ϴα�����״̬���ݣ�����ҵ���ָ�״̬
    //ע�⣬�ڴ˲���֮ǰ����ִ����ӷ��Ͷ˿ڡ���������Ƶ������״̬�Ȼ�ı䱸�����ݵĲ���
    //Ҫȷ����Щ������ı䱸��״̬���ɲ�����Щ����������BackupStatus()
    if (RestoreStatus() != 0) {
        //û��״̬���ݣ������һЩ��ʼĬ�ϵķ��Ͷ˿ڣ�����Ĭ��״̬

        int index = getVirtualIndexFromSendMediaID(_mediaID);

        //���͸�С�� - ����Ƶ
        if (info.net_info.enable_small_video && info.video_info.codec_name != NULL)
        {
            NetInfo addr;
            addr.ip_addr = "127.0.0.1";
            addr.video_port = 8600 + 10 * index;
            addr.audio_port = 0;
            addr.enable_small_video = false;
            addr.screen_port = 0;

            if(AddSendDest(addr) != EVENT_R_JOB_DONE)
            {
                LOG_ERROR("���С�����͵�ַ��ʱ��������");
                goto ErrorOut;
            }
        }

        //�˴��Զ�����Ƶ���͸����ػ���
        //�ж϶˿������С����Χ����С�����͸����ػ��ԣ������ɴ�������
        if (info.net_info.video_port != 0 &&
            info.net_info.video_port % LOCALPRE_VIDEO_PORT_BASE < 100)
        {
            NetInfo addr;
            addr.ip_addr = "127.0.0.1";
            addr.video_port = info.net_info.video_port;
            addr.audio_port = 0;
            addr.screen_port = 0;
            addr.enable_small_video = false;
            if(AddSendDest(addr) != EVENT_R_JOB_DONE)
            {
                LOG_ERROR("��ӱ��ػ��Է��͵�ַ��ʱ��������");
                goto ErrorOut;
            }
        }

        //�����������Ƶ����ôͨ���ӿ��޸�һ����Ƶ�������Ĺ���״̬
        if( !info.audio_info.enable )
        {
            EnableAudio( false );
        }
    }

	if(msdx_send_get_graph_state() != 1)
		result = msdx_run_send_graph();
	if(result != 0)
	{
		LOG_ERROR("���з���Graph��ʱ��������");
		goto ErrorOut;
	}

	LOG_DEBUG("SendGraph�����ɹ�");
	return EVENT_R_JOB_DONE;

ErrorOut:

	msdx_destroy_send_graph();
	return EVENT_R_JOB_FAIL;
}

QByteArray CvStreamedMedia::GetMediaInfo( int info_type )
{
	//û��ʵ��
	QByteArray byteArray;
	return byteArray;
}

void CvStreamedMedia::ResetVideoWindow( const int wnd )
{
	if( _videoWidth <= 0 || _videoHeight <= 0 )
	{
		LOG_DEBUG( "��Ƶ��ʽ���ڴ��󣺿�Ϊ(%d)����Ϊ(%d)" , _videoWidth , _videoHeight );
		return ;
	}

  video_window_handle_ = wnd;

	RECT WindowRect;
	GetWindowRect( (HWND)wnd, &WindowRect);
	LOG_DEBUG( "Window Rect of 0x%08x is:%d,%d,%d,%d" , wnd, WindowRect.left , WindowRect.top , WindowRect.right ,WindowRect.bottom );
	long width = WindowRect.right - WindowRect.left;
	long height = WindowRect.bottom - WindowRect.top;
	float xpos =0 , ypos=0;
	if(width != 0 && height != 0)
	{
		float ratio = (float)_videoWidth/ _videoHeight;
		if(((float)width / height) >= ratio )
		{
			xpos = (width - height * ratio) / 2;
			width = height * ratio;
		}
		else
		{
			ypos = (height - width * 1 / ratio) / 2;
			height = width * 1 / ratio;
		}
		//LOG_DEBUG("resized video postion: %f,%f,%d,%d" , xpos , ypos , width , height );
		if( _mediaDirection==MediaDirection_In )
		{
			msdx_set_recv_display_window( _graphID , (HWND)wnd ,  xpos,  ypos,
				width, height , 0 );

            //����״̬
            stream_media_status_.video_width = _videoWidth;
            stream_media_status_.video_height = _videoHeight;
            stream_media_status_.window_handle = wnd;
            BackupStatus();
		}
        //deprecated
        /*else if( _mediaDirection==MediaDirection_out )
		{
			msdx_set_preview_display_window((HWND)wnd ,  xpos,  ypos,
				width, height , 0 );
		}*/
	}
}

void CvStreamedMedia::EnableVideo( bool b )
{
    LOG_INFO("enable video: %d", b);
    if( _mediaDirection==MediaDirection_out )
    {
        int result = msdx_control_video_encoder( b );
    }
    else if( _mediaDirection==MediaDirection_In )
    {
        msdx_control_video_recv( _graphID, b );
    }
    //����״̬
    stream_media_status_.enable_video = b;
    BackupStatus();
}

void CvStreamedMedia::EnableAudio( bool b )
{
    LOG_INFO("enable audio: %d", b);
    if( _mediaDirection==MediaDirection_out )
    {
        int result = msdx_control_audio_encoder( b );
    }
    else if( _mediaDirection==MediaDirection_In )
    {
        msdx_control_audio_recv( _graphID, b );
    }
    //����״̬
    stream_media_status_.enable_audio = b;
    BackupStatus();
}

int CvStreamedMedia::AddSendDest( const NetInfo &info )
{
	LOG_DEBUG("AddSendGraph->IP:%s , VIDEO_PORT:%d , AUDIO_PORT:%d",
		qPrintable(info.ip_addr) , info.video_port , info.audio_port);

    if (info.video_port == 0 && info.audio_port == 0) {
        return EVENT_R_JOB_DONE;
    }

	int result = -1;

    if (!info.enable_small_video)
    {
        result = msdx_add_send_address(info.ip_addr.toStdString().c_str(),
            info.audio_port,
            info.ip_addr.toStdString().c_str(),
            info.video_port, info.enable_small_video);
    }
    else
    {
        result = msdx_add_smallvideo_address(info.ip_addr.toStdString().c_str(), 
            info.audio_port, 
            info.ip_addr.toStdString().c_str(), 
            info.video_port,
            info.enable_small_video );
    }
	
	if(result != 0)
	{
		LOG_ERROR("��ӷ��͵�ַ��ʱ��������");
		goto ErrorOut;
	}

	LOG_DEBUG("��ӷ���Ŀ��ɹ�");

    //����״̬
    {
        StreamMediaStatus::SendDest dest = qMakePair(info.ip_addr, info.video_port);
        if (!stream_media_status_.video_send_dest.contains(dest)) {
            stream_media_status_.video_send_dest.push_back(dest);
        }
        dest = qMakePair(info.ip_addr, info.audio_port);
        if (!stream_media_status_.audio_send_dest.contains(dest)) {
            stream_media_status_.audio_send_dest.push_back(dest);
        }
        BackupStatus();
    }

	return EVENT_R_JOB_DONE;

ErrorOut:

	return EVENT_R_JOB_FAIL;
}

int CvStreamedMedia::RemoveSendDest(const NetInfo &info)
{
    LOG_DEBUG("Remove Send Dst:IP(%s), VideoPort(%d), AudioPort(%d)", 
        qPrintable(info.ip_addr), info.video_port, info.audio_port );
    if (info.video_port == 0 && info.audio_port == 0) {
        return EVENT_R_JOB_DONE;
    }

    int result = -1;
    if (info.enable_small_video)
    {
        if (msdx_get_smallvideo_graph() < 0)
        {
            LOG_ERROR("������SmallVideo����Graph��");
        }
        result  = msdx_delete_smallvideo_address(info.ip_addr.toStdString().c_str(),
            info.audio_port,
            info.ip_addr.toStdString().c_str(),
            info.video_port);
    }
    else
    {
        if(msdx_get_send_graph() < 0)
        {
            LOG_ERROR("�����ڷ���Graph��");
        }
        result  = msdx_delete_send_address(info.ip_addr.toStdString().c_str(),
            info.audio_port,
            info.ip_addr.toStdString().c_str(),
            info.video_port);
    }

    //����״̬
    {
        StreamMediaStatus::SendDest dest = qMakePair(info.ip_addr, info.video_port);
        int index = stream_media_status_.video_send_dest.indexOf(dest);
        if (index >= 0) {
            stream_media_status_.video_send_dest.remove(index);
        }
        dest = qMakePair(info.ip_addr, info.audio_port);
        index = stream_media_status_.audio_send_dest.indexOf(dest);
        if (index >= 0) {
            stream_media_status_.audio_send_dest.remove(index);
        }
        BackupStatus();
    }

    return EVENT_R_JOB_DONE;
}


int CvStreamedMedia::CreateSmallVideoGraph( const RecvGraphInfo& info )
{
	LOG_INFO("CreateSmallVideoGraph-> IP:%s, VIDEO_PORT:%d , VIDEO_CODEC:%s \n"
		"\tVIDEO_WIDTH:%d , VIDEO_HEIGHT:%d, VIDEO_FPS:%d \n" ,  
		qPrintable(info.net_info.ip_addr) , info.net_info.video_port , qPrintable(info.video_info.codec_name) , 
		info.video_info.width, info.video_info.height, info.video_info.fps);

	int result;

	//����smallvideograph
	int createresult = msdx_create_smallvideo_graph(qPrintable(info.user_id));
	if( createresult < 0)
	{
		LOG_ERROR("��������Graphʱ��������");
		goto ErrorOut;
	}

video:
	if(info.net_info.video_port == 0)
	{
		LOG_WARN("��������Ƶ��Ϣ");
		goto NoRecvVideo;
	}

	_videoWidth = info.video_info.width;
	_videoHeight = info.video_info.height;
    char codecName[256] = {0};
	memset( codecName , 0 , sizeof(codecName) );
	strcpy_s(codecName , qPrintable(info.video_info.codec_name) );
	result = msdx_set_preview_video_codec(
		codecName,
		info.video_info.width,
		info.video_info.height,
		info.video_info.fps,
		info.video_info.bit_count);
	if(result != 0)
	{
		LOG_ERROR("������Ƶ������ʱ��������");
		goto ErrorOut;
	}

NoRecvVideo:
	//����RTPͳ�ƻص�����
	msdx_preview_set_video_rtpstatcallback(NULL, deliverRtpStatInfo);

	const int index = getVirtualIndexFromSmallMediaID(_mediaID);

	result = msdx_set_preview_address(
		info.net_info.ip_addr.toStdString().c_str(),
		/*info.net_info.audio_port*/0,
		info.net_info.ip_addr.toStdString().c_str(),
		/*info.net_info.video_port*/8600 + 10 * index);
	if(result != 0) 
	{
		LOG_ERROR("������Ƶ���յ�ַʱ��������");
		goto ErrorOut;
	}

	result = msdx_connect_smallvideo_graph();
	if(result != 0)
	{
		LOG_ERROR("����Filterʱ��������");
		goto ErrorOut;
	}

    //���Բ����ϴα�����״̬���ݣ�����ҵ���ָ�״̬
    //ע�⣬�ڴ˲���֮ǰ����ִ����ӷ��Ͷ˿ڵȻ�ı䱸�����ݵĲ���
    //Ҫȷ����Щ������ı䱸��״̬���ɲ�����Щ����������BackupStatus()
    RestoreStatus();

    if (!stream_media_status_.valid) {
        //û���ҵ��������һЩ��ʼĬ�ϵķ��Ͷ˿ڣ�����Ĭ��״̬

        //���͸����ػ���
        if (info.net_info.video_port != 0 &&
            info.net_info.video_port % LOCALPRE_SMALL_PORT_BASE < 100)
        {
            NetInfo addr;
            addr.ip_addr = "127.0.0.1";
            addr.video_port = info.net_info.video_port;
            addr.audio_port = 0;
            addr.screen_port = 0;
            addr.enable_small_video = true;
            if(AddSendDest(addr) != EVENT_R_JOB_DONE)
            {
                LOG_ERROR("��ӱ��ػ��Է��͵�ַ��ʱ��������");
                goto ErrorOut;
            }
        }
    }

	result = msdx_run_smallvideo_graph();
	if(result != 0)
	{
		LOG_ERROR("����RecvGraph��ʱ�����쳣");
		goto ErrorOut;
	}

	LOG_DEBUG( "RecvGraph �����ɹ���");
	return 0;

ErrorOut:

	/*msdx_destroy_recv_graph(_graphID);*/

	return -1;
}

int CvStreamedMedia::CreateFilePlayGraph(const FilePlayInfo& info)
{
  LOG_INFO("CreateFilePlayGraph-> ID:%s, NAME:%s, SYNC_ID:%s\n"
    "\tIP:%d , VIDEO_PORT:%d, AUDIO_PORT:%d\n"
    "\tPLAY_LIST_SIZE:%d",  
    qPrintable(info.vuser_id), qPrintable(info.vuser_name), qPrintable(info.sync_id), 
    qPrintable(info.net_info.ip_addr), info.net_info.video_port , info.net_info.audio_port, 
    info.file_list.size());

  msdx::dxStatus status = msdx::DX_ERR_NONE;
  msdx::FilePlayGraphParam param;

  param.graph_name = qPrintable(info.vuser_id);
  param.sync_id = qPrintable(info.sync_id);
  param.initial_delay = info.initial_delay * 10000LL;
  param.video.addr.ip = qPrintable(info.net_info.ip_addr);
  param.video.addr.port = info.net_info.video_port;
  param.audio.addr.ip = qPrintable(info.net_info.ip_addr);
  param.audio.addr.port = info.net_info.audio_port;

  if (info.file_list.size() == 0)
  {
    LOG_ERROR("Empty playlist");
    return -1;
  }

  for (unsigned int i = 0; i < info.file_list.size(); ++i)
  {
    std::string std_file_path = qPrintable(info.file_list[i]);
    param.file_list.push_back(std_file_path);
    LOG_PRINTF("play idx %d: %s", i, std_file_path.c_str());
  }

  _file_play_controller = msdx::GraphControllerFactory::GetInstance().CreateFilePlayGraphController();
  if (!_file_play_controller)
  {
    LOG_ERROR("Create fileplay graph failed");
  }

  //�ص���Ҫ��buildǰ���ã���Ȼ���ܱ�������Ƶ��ʽŶ
  _file_play_controller->SetFilePlayStatCallBack(this, CvStreamedMedia::FilePlayCallback);
  status = _file_play_controller->Build(param);
  if (STATUS_ERR(status)) return -1;

  _file_play_controller->Run(); //�Ȳ�Ҫrun���ȴ��ϲ�ָ��

  return 0;
}

int CvStreamedMedia::deliverRtpStatInfo( void * pclass, const RtpStatItem * rtpstat )
{
    if(rtpstat == NULL)
        return -1;
    return sendRtpStatInfoToClient(*rtpstat);
}

void CvStreamedMedia::SetCurrentState( StreamedMediaState state )
{
    _currentState = state;
    if (_channelDispatcher) {
        _channelDispatcher->ChannelStateChanged( _mediaID, _userID,
            STREAMED_MEDIA_SERVICE_NAME, _currentState);
    }
}

void CvStreamedMedia::HandleVideoWindowCheckTimerTimeoutSlot()
{
    //������QT����������ڴ�Сʱ���ᷢ���¼�������֪ͨ�ײ㡣
    //���ǣ�ʵ����QT�����¼���ϵͳ���ڲ��������ϱ仯����͵��µײ����GetWindowRectʱ�����õ��ɵĴ��ڴ�С���Ӷ���ʾ������
    //Ŀǰ�Ľ���������ǣ���ʱ���ϼ�鴰��λ�ã������ֱ��ʱ��������ʾ��ʽ��
    static RECT rect;
    if (video_window_handle_ != 0) 
    {
        RECT rect_now;
        GetWindowRect((HWND)video_window_handle_, &rect_now);
        if (rect.top != rect_now.top || rect.left != rect_now.left ||
            rect.bottom != rect_now.bottom || rect.right != rect_now.right) 
        {
             LOG_PRINTF("Window Rect change detected: wnd:0x%08x, left:%d, top:%d, right:%d, bottom:%d", video_window_handle_, 
               rect_now.left, rect_now.top, rect_now.right, rect_now.bottom);
             ResetVideoWindow(video_window_handle_);
             rect = rect_now;
        }
    }
}

void CvStreamedMedia::BackupStatus()
{
    if (backup_manager_) {
        QByteArray data;
        QDataStream stream(&data, QIODevice::ReadWrite);
        stream << stream_media_status_; //TODO: ע��64bit����
        backup_manager_->Backup(_mediaID, STREAMED_MEDIA_SERVICE_NAME, 1, data); //���浽1λ�ã�0Ϊ������������ChannelDispatcher���Ѿ�����
    }
}

int CvStreamedMedia::RestoreStatus()
{
    if (backup_manager_) {
        //��ԭ��������
        QByteArray data;
        if (0 != backup_manager_->FetchData(_mediaID, 1, data)) //����ʽ����
        {
            LOG_ERROR("no backup status to restore.");
            return -1;
        }

        if (data.size() == 0) return -1;

        QDataStream stream(data);
        StreamMediaStatus backup_status;
        stream >> backup_status;

        if (!backup_status.valid) return -1;

        //��ʼ��ԭ
        LOG_WARN("backup stream media status found, restoring...");
        if(_mediaDirection == MediaDirection_out)
        {
            //���Ͷ�
            //������״̬
            EnableVideo(backup_status.enable_video);
            EnableAudio(backup_status.enable_audio);

            //���Ͷ˿�
            bool is_small = isSmallVideoMediaID(_mediaID);
            for (auto dest : backup_status.video_send_dest) {
                NetInfo addr;
                addr.ip_addr = dest.first;
                addr.video_port = dest.second;
                addr.audio_port = 0;
                addr.screen_port = 0;
                addr.enable_small_video = is_small;

                if(AddSendDest(addr) != EVENT_R_JOB_DONE)
                {
                    LOG_ERROR("restore video send dest failed.");
                }
            }
            for (auto dest : backup_status.audio_send_dest) {
                NetInfo addr;
                addr.ip_addr = dest.first;
                addr.video_port = 0;
                addr.audio_port = dest.second;
                addr.screen_port = 0;
                addr.enable_small_video = is_small;

                if(AddSendDest(addr) != EVENT_R_JOB_DONE)
                {
                    LOG_ERROR("restore audio send dest failed.");
                }
            }
        }
        else if (_mediaDirection == MediaDirection_In)
        {
            //���ն�
            _videoWidth = backup_status.video_width;
            _videoHeight = backup_status.video_height;
            ResetVideoWindow(backup_status.window_handle);
        }
        LOG_INFO("restoring stream media status complete!");
    }
    return 0;
}

void CvStreamedMedia::ResetBackupStatus()
{
    stream_media_status_.valid = false;
    stream_media_status_.video_send_dest.clear();
    stream_media_status_.audio_send_dest.clear();
    stream_media_status_.window_handle = 0;
    stream_media_status_.enable_video = true;
    stream_media_status_.enable_audio = true;
}

int CvStreamedMedia::sendRtpStatInfoToClient( const RtpStatItem& rtpstat )
{
    RtpStatItem report = rtpstat;
    strncpy( report.member_id , qPrintable(_userID ),sizeof(report.member_id) );

    initQosReportSender();

    //���͸�CoolView���̣�Ȼ���ٴ������QoS������
    //static QosReportSender qosReportSenderToCoolview( "127.0.0.1", NConfig::getInstance().getCoolviewQosServerUdpPort() );
    qosReportSenderToCoolview->sendRtpStat( report );

    //����QoS�������ڱ����ն˻�������Ƶ���ڵ���ʾ
    //if( /*_userID!=LOCAL_PLAY_USERID &&*/ !isSendMediaID(_mediaID) )
    //{
    //ֻ��ý�������ս��̲ŻὫRTCP���淢�͸���������
    //ͨ��udp���͸�coolview�е��������̣���������Ƿ��͸�������UI
    //static QosReportSender qosReportSenderToConfRoom( "127.0.0.1", NConfig::getInstance().getConfRoomQosServerUdpPort() );	
    qosReportSenderToConfRoom->sendRtpStat( report );
    //}

    return 0;
}

QosReportSender *CvStreamedMedia::qosReportSenderToCoolview = nullptr;
QosReportSender *CvStreamedMedia::qosReportSenderToConfRoom = nullptr;

void CvStreamedMedia::RecordCallback(void * obj, RecStatItem &stat)
{
	switch(stat.statType)
	{
    case REC_STAT_FILE_WRITING:
        break;
	case REC_STAT_CUT_REQUEST:
		LOG_PRINTF("Cut file %s, create file %s", 
      stat.cut.cur, 
      stat.cut.next ? stat.cut.next : "null");
		break;
	default:
		LOG_PRINTF("Recording status report code %d", stat.statType);
	}

	QByteArray bytes = ((CvStreamedMedia *)obj)->_userID.toLocal8Bit();
	strcpy_s(stat.user_name, bytes.data());

  initQosReportSender();
  qosReportSenderToCoolview->sendRecStat( stat );
}

void CvStreamedMedia::FilePlayCallback(void * obj, FilePlayStatItem &stat)
{
	QByteArray bytes = ((CvStreamedMedia *)obj)->_userID.toLocal8Bit();
	strcpy_s(stat.id, bytes.data());

  initQosReportSender();
  qosReportSenderToCoolview->sendFilePlayStat( stat );	
}

void CvStreamedMedia::initQosReportSender()
{
  if (!qosReportSenderToCoolview)
  {
    qosReportSenderToCoolview = new QosReportSender(
      "127.0.0.1", NConfig::getInstance().getCoolviewQosServerUdpPort());
  }
  if (!qosReportSenderToConfRoom)
  {
    qosReportSenderToConfRoom = new QosReportSender(
      "127.0.0.1", NConfig::getInstance().getConfRoomQosServerUdpPort());
  }
}

void CvStreamedMedia::ControlPlay( const FilePlayCtrlInfo &info )
{
    if (!_file_play_controller) return;

    switch (info.ctrl_type)
    {
    case FilePlayCtrlInfo::kCtrlPause:
        LOG_PRINTF("Pause play");
        _file_play_controller->Pause(info.op_id);
        break;
    case FilePlayCtrlInfo::kCtrlResume:
        LOG_PRINTF("Resume play");
        _file_play_controller->Resume(info.op_id);
        break;
    case FilePlayCtrlInfo::kCtrlSeek:
        LOG_PRINTF("Seek to %I64dms", info.seek_time);
        _file_play_controller->SeekTo(info.op_id, info.seek_time * 10000LL);
        break;
    default:
        break;
    }
}
