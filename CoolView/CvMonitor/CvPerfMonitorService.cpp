//#include "stdafx.h"
#include <QtCore/QtCore>

#include <dbus/performance/monitor/service/CvPerfMonitorAdaptor.h>
#include <dbus/performance/monitor/common/CvPerfMonitorServiceCommon.h>
#include <dbus/core/client/CvCoreIf.h>
#include <dbus/core/common/CvCoreServiceCommon.h>

//channel dispatcher
#include <dbus/channelDispatcher/client/ChannelDispatcherIf.h>
#include <dbus/channelDispatcher/common/ChannelDispatcherServiceCommon.h>
#include <dbus/channel/type/testMedia/common/TestMediaServiceCommon.h>


#include <dbus/performance/process/client/CvPerfProcessIf.h>
#include <dbus/performance/process/common/PerformanceProcessCommon.h>

#include "CvPerfMonitorService.h"
#include "MonitorThread.h"
#include "MediaMonitorThread.h"




#include <log/Log.h>
CvPerfMonitorService::CvPerfMonitorService()
{
	new CvPerfMonitorAdaptor( this );
	QDBusConnection connec = QDBusConnection::sessionBus();
	bool res = connec.registerService( PERFORMANCE_MONITOR_SERVICE_NAME );
	res = connec.registerObject( PERFORMANCE_MONITOR_OBJECT_PATH , this);
	
	_monitorCofInfoTimer = new QTimer(this);
	
	_coreProxy = new CvCoreIf( CVCORE_SERVICE_NAME , CVCORE_SERVICE_OBJECT_PATH , QDBusConnection::sessionBus() );
	
	connect(_monitorCofInfoTimer,SIGNAL(timeout()),this,SLOT(UpgradeMonitorCofInfoSlot()));

	_monitorCofInfoTimer->start(10000);  //ÿʮ����м�����ݸ���
	int i = 1;
	
	_mediaMonitorThread = new MediaMonitorThread();
	//_mediaMonitorThread->start(QThread::HighPriority );

    _findAirServer = true;
    _monitorAirServerTimer = new QTimer(this);
    connect(_monitorAirServerTimer, SIGNAL(timeout()), this, SLOT(UpgradeMonitorAirServerSlot()));
    _monitorAirServerTimer->start(5000);
}

CvPerfMonitorService::~CvPerfMonitorService()
{

}


void CvPerfMonitorService::HelloInfo( int processID, const QByteArray &processImage )
{
	for( MonitoredProcessMap::iterator it = _processMap.begin() ; it!=_processMap.end() ; it++ )
	{
		MonitoredProcess* process = it.value();
		if( process && (process->_processID == processID) )
		{
			process->_processImage = processImage;
			for( int i=0 ; i<process->_triggerVector.size(); i++ )
			{
				//����Ƿ���TTL���������еĻ�
				unsigned short monitorType = ( process->_triggerVector.at(i)._monitorValue & 0xffff0000 ) >> 16;
				if( monitorType==Trigger::TIME_TO_LIVE )
				{
					//��ȡTTL��ֵ���涨TTL�����ֵΪ5
					unsigned short ttl = (process->_triggerVector.at(i)._monitorValue & 0x0000ffff) ;
					if( ttl<5 )
					{
						//TTL��1
						process->_triggerVector[i]._monitorValue ++;
					}

				}
			}
			return;
		}
	}
}

void CvPerfMonitorService::RegisterProcess( const QByteArray &processInfo )
{
	QDataStream in( processInfo);
	in.setVersion( QDataStream::Qt_4_4);
	MonitoredProcess* pProcess = new MonitoredProcess();
	in >> *pProcess;

	MonitoredProcessMap::iterator it = _processMap.find( pProcess->_processName );
	if( it==_processMap.end() )
	{
		sprintf_s( __global_msg , sizeof(__global_msg) , "RegisterProcess(%s:%x)" , qPrintable(pProcess->_processName) ,pProcess->_processID );
		MONITOR_LOG_DEBUG( __global_msg );

		//���������м����߳�
		pProcess->_monitorThread = new MonitorThread( pProcess );
		pProcess->_monitorThread->start(QThread::HighPriority );
		pProcess->SetMonitorMark( true);
		_processMap.insert( pProcess->_processName , pProcess );
	}else
	{
		MonitoredProcess* existProcess = it.value();
		//��������Դ�Ѿ�����
		if( existProcess->GetMonitorMark() )
		{
			//����ڼ�ع����б���ؽ���������ע���ˣ���ô���Լ���Է����̸ñ�������Ӧ�ûָ���
			CvPerfProcessIf processProxy( __GetCvPerfProcessServiceName(existProcess->_processName),
				__GetCvPerfProcessObjectPath(existProcess->_processName) , QDBusConnection::sessionBus());
			processProxy.Recover( existProcess->_processImage );
			QDateTime dateTime = QDateTime::currentDateTime();
			sprintf_s( __global_msg , sizeof(__global_msg) , "Recover Process[%s] : %s" , qPrintable(existProcess->_processName),qPrintable(dateTime.toString("MM-dd hh:mm:ss")) );
			MONITOR_LOG_DEBUG(__global_msg );
		}else
		{
			//ֹͣ�����
			sprintf_s( __global_msg , sizeof(__global_msg) ,"Restart process[%s] monitor." , qPrintable(existProcess->_processName) );
			MONITOR_LOG_DEBUG( __global_msg );
		}

		//Ȼ��������ڵļ�ؽ������ݣ����������
		*existProcess = *pProcess;		
		if( existProcess->_monitorThread==NULL )
			existProcess->_monitorThread = new MonitorThread( pProcess );

		//��Ҫ�����߳���ֹ������ֹ�߳���ԭ����״̬�ﻹû���˳�
		((MonitorThread*)existProcess->_monitorThread)->EndMark(false);
		existProcess->_monitorThread->start( QThread::HighestPriority );
		existProcess->SetMonitorMark( true );

		//�ͷ���ʱ��Դ
		delete pProcess;
		pProcess = NULL;
	}
}

void CvPerfMonitorService::UnregisterProcess( int processID )
{
	for( MonitoredProcessMap::iterator it = _processMap.begin() ; it!=_processMap.end() ; it++ )
	{
		MonitoredProcess* process = it.value();
		if( process && (process->_processID == processID) )
		{
			sprintf_s( __global_msg , sizeof(__global_msg ) ,"Unregister Process(%s:%x)" , qPrintable(it.value()->_processName) , it.value()->_processID);
			MONITOR_LOG_DEBUG( __global_msg );
			MonitoredProcess* pProcess = it.value();
			if( pProcess )
			{
				//ֹͣ����߳�
				((MonitorThread*)pProcess->_monitorThread)->EndMark(true);

				//��־��ؽ���Ϊ�Ǽ��
				pProcess->SetMonitorMark( false );
			}
			return;
		}
	}
	sprintf_s( __global_msg , sizeof(__global_msg ) ,"Can't Find Process(%x)" , processID);
	MONITOR_LOG_DEBUG( __global_msg );

}

void CvPerfMonitorService::UpgradeMonitorCofInfoSlot()
{
 
  double inBandWidth , outBandWidth;
 
  _monitorComfigInfo.cpuUsage=_perforUtil.getTotalCPUUsage();
  _monitorComfigInfo.totalMemory=_perforUtil.getToalMemoryUsage()/1024;
  _monitorComfigInfo.availableMemory = _perforUtil.getAvailableMemoryUsage()/1024;

  _perforUtil.getNetworkFlow(outBandWidth,inBandWidth);
  //���ڶ�ʱ����10��һ�Σ������Ҫ����ʮ�� �Լ�����KB��λ
  _monitorComfigInfo.netWorkInBandWidth = inBandWidth/10;
  _monitorComfigInfo.netWorkOutBandWidth = outBandWidth/10;

  QByteArray output_bytes;
  QDataStream out( &output_bytes, QIODevice::WriteOnly );
  out.setVersion(QDataStream::Qt_4_4 );
  out<< _monitorComfigInfo;
  _coreProxy->TeleCommand(CoreAction_RecvMonitorCofinfo,output_bytes);
}

void CvPerfMonitorService::CreateTestMedia(const QString &media_id, const QByteArray &input_garray)
{

	ChannelDispatcherIf  channelDispatcherProxy (
		CHANNEL_DISPATCHER_SERVICE_NAME, CHANNEL_DISPATCHER_OBJECT_PATH , QDBusConnection::sessionBus());

    if(media_id == VIDEO_PREVIEW_MEDIA_ID)
	{
		QDataStream in( input_garray );
		in.setVersion( QDataStream::Qt_4_4 );
		VideoPreviewInfo videoInfo;
		in >> videoInfo;

		QByteArray recvByteArray;
		QDataStream out( &recvByteArray , QIODevice::WriteOnly );
		out << videoInfo;

		channelDispatcherProxy.CreateChannel(VIDEO_PREVIEW_MEDIA_ID, TEST_MEDIA_SERVICE_NAME, recvByteArray);
	}
	else
	{
		QDataStream in( input_garray );
		in.setVersion( QDataStream::Qt_4_4 );
		TestMediaInfo info;
		in >> info;

		QByteArray recvByteArray;
		QDataStream out( &recvByteArray , QIODevice::WriteOnly );
		out << info;

		if(media_id == FILE_TEST_MEDIA_ID)
		{

			channelDispatcherProxy.CreateChannel(FILE_TEST_MEDIA_ID, TEST_MEDIA_SERVICE_NAME, recvByteArray);
		}
		else
			if(media_id == DEVICE_TEST_MEDIA_ID)
			{
				channelDispatcherProxy.CreateChannel(DEVICE_TEST_MEDIA_ID, TEST_MEDIA_SERVICE_NAME, recvByteArray);
			}
	}
}

void CvPerfMonitorService::ReleaseTestMedia(const QString &media_id)
{
	ChannelDispatcherIf  channelDispatcherProxy (
		CHANNEL_DISPATCHER_SERVICE_NAME, CHANNEL_DISPATCHER_OBJECT_PATH , QDBusConnection::sessionBus());

	if(media_id == FILE_TEST_MEDIA_ID)
	{
		channelDispatcherProxy.ReleaseChannel(FILE_TEST_MEDIA_ID, TEST_MEDIA_SERVICE_NAME);
	}
	else
	if(media_id == DEVICE_TEST_MEDIA_ID)
	{
		channelDispatcherProxy.ReleaseChannel(DEVICE_TEST_MEDIA_ID, TEST_MEDIA_SERVICE_NAME);
	}
	else
	if(media_id == VIDEO_PREVIEW_MEDIA_ID)
	{
		channelDispatcherProxy.ReleaseChannel(VIDEO_PREVIEW_MEDIA_ID, TEST_MEDIA_SERVICE_NAME);
	}
}

void CvPerfMonitorService::RegisterMediaProcess(const QByteArray &processInfo)
{
	QDataStream in( processInfo);
	in.setVersion( QDataStream::Qt_4_4);
	MediaMonitoredProcess pProcess;
	in >> pProcess;

	//_mediaMonitorThread->addMediaMonitoredProcess(pProcess);
}

void CvPerfMonitorService::UnregisterMediaProcess(int processID)
{
	//_mediaMonitorThread->removeMediaMonitoredProcess(processID);
}

void CvPerfMonitorService::UpgradeMonitorAirServerSlot()
{
    HWND airServerWnd = FindWindow(TEXT("AirServer::MainFrame"), NULL);
    if (_findAirServer && NULL == airServerWnd) {
      _findAirServer = false;
      sprintf_s( __global_msg , sizeof(__global_msg ) ,"Cannot Find AirServer");
      MONITOR_LOG_INFO( __global_msg );

      if (!RecoverAirServer()) {
        sprintf_s( __global_msg , sizeof(__global_msg ) ,"Cannot Start AirServer");
        MONITOR_LOG_ERROR( __global_msg );
      }

    } else if (!_findAirServer && airServerWnd) {
      _findAirServer = true;
      sprintf_s( __global_msg , sizeof(__global_msg ) ,"Find AirServer");
      MONITOR_LOG_INFO( __global_msg );
    }
}

bool CvPerfMonitorService::RecoverAirServer()
{
  QString dir = "C:\\Program Files\\App Dynamic\\AirServer\\";
  if (!QFile::exists(dir)) {
    dir = "C:\\Program Files (x86)\\App Dynamic\\AirServer\\";
    if (!QFile::exists(dir)) {
      return false;
    }
  }
  QString cmd = dir + "AirServer.exe";
  char cmdLine[128] = {0};
  strcpy_s(cmdLine, qPrintable(cmd));

  STARTUPINFOA si;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESHOWWINDOW;
  QString appDir = QCoreApplication::applicationDirPath();
  appDir.replace("/" , "\\" );
  QString debugPath = appDir + "\\debug";
  si.wShowWindow = QFile::exists(debugPath) ? SW_SHOW : SW_HIDE;

  PROCESS_INFORMATION pi;
  DWORD flag = HIGH_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
  return CreateProcessA(NULL, cmdLine, NULL, NULL, NULL, flag, NULL, qPrintable(dir), &si, &pi);
}
