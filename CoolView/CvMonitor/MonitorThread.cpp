#include "MonitorThread.h"
#include "PerformanceUtil.h"
#include "MonitorActionExecutor.h"
#include <dbus/performance/monitor/common/CvPerfMonitorServiceCommon.h>
#include <log/Log.h>
#include <util/processManager.h>

MonitorThread::MonitorThread( MonitoredProcess* monitoredProcess )
{
	this->_monitoredProcess = monitoredProcess;
	this->_endMark = false;
}

void MonitorThread::run()
{
	if( _monitoredProcess 
		&& _monitoredProcess->_monitorInterval>0 
		&& _monitoredProcess->_triggerVector.size()>0 
		)
	{
		while(!_endMark )
		{
			//ÿ��һ��ʱ�����ص���ֵ

			int triggerNum = _monitoredProcess->_triggerVector.size();
			for( int i=0 ; i< triggerNum; i++ )
			{
				const Trigger& trigger = _monitoredProcess->_triggerVector.at(i);
				if( trigger._monitorValue!=Trigger::NO_TRIGGER )
				{
					unsigned short monitorType = (trigger._monitorValue & 0xffff0000) >> 16;
					unsigned short threshHold = trigger._monitorValue & 0x0000ffff;
					switch( monitorType )
					{
					case Trigger::PROCESS_ALIVE:
						{
							//�жϽ����Ƿ�����ϵͳ�����б���
							QString processName = _monitoredProcess->_processName;
							if( !processName.endsWith(".exe") )
								processName+= ".exe";
							if( !ProcessManager::isRunning(processName.toStdWString().c_str()) )
							{
								//�����Ѿ��˳��������ý���
								MonitorActionExecutor::ExecuteMonitorAction( _monitoredProcess , trigger._action );
							}
						}break;
					case Trigger::CPU_USAGE:
						{
							//����CPUռ����
							int cpuUsage = _perforUtil.getProcCpuUsage( 
								_monitoredProcess->_processName.toStdWString().c_str() );
							//qDebug("Process[%s] CPU Usage is %d , threshold is %d" ,
							//	qPrintable(_monitoredProcess->_processName), cpuUsage , threshHold );
							if( cpuUsage> threshHold )
							{
								//ִ���ض�����
								sprintf_s( __global_msg , sizeof(__global_msg) , "Process[%s] CPU Usage[%d] Unnormal" ,
									qPrintable(_monitoredProcess->_processName), cpuUsage );
								MONITOR_LOG_DEBUG( __global_msg );

								int action = MonitorActionExecutor::ExecuteMonitorAction( _monitoredProcess , trigger._action );
								_endMark = true;
							}
						};break;
					case Trigger::MEM_USAGE:
						{
							//�����ڴ�ռ�ô�С
							int memUsage = _perforUtil.getProcMemoryUsage(
								_monitoredProcess->_processName.toStdWString().c_str() );
							//qDebug("Process[%s] MEM Usage is %dKB, threshold is %dKB" ,
							//	qPrintable(_monitoredProcess->_processName) , memUsage , threshHold*1024 );
							if( memUsage>(threshHold*1024)  )	//ע��threshhold�ĵ�λΪMB
							{
								sprintf_s( __global_msg , sizeof(__global_msg) , "Process[%s] Memory Usage[%dKB] Unnormal" ,
									qPrintable(_monitoredProcess->_processName),memUsage );
								MONITOR_LOG_ERROR( __global_msg );

								int actionResult = MonitorActionExecutor::ExecuteMonitorAction( _monitoredProcess , trigger._action );
								_endMark = true;
							}
						};break;
					case Trigger::TIME_TO_LIVE:
						{
							if( threshHold==0 )
							{
								//ִ���ض�����
								sprintf_s( __global_msg , sizeof(__global_msg) , "Process[%s] Seems Dead" ,
									qPrintable(_monitoredProcess->_processName));
								MONITOR_LOG_ERROR(__global_msg);

								int actionResult = MonitorActionExecutor::ExecuteMonitorAction( _monitoredProcess , trigger._action );

								//ִ���궯��֮���˳���ؽ���
								_endMark = true;
							}else
							{
								//qDebug("Process[%s] TTL is %d , will sub 1"  , qPrintable(_monitoredProcess->_processName) , threshHold );
								_monitoredProcess->_triggerVector[i]._monitorValue--;	//time to live ��1
							}
						};break;
					}
				}
			}

			//����߳�˯��һ��ʱ��
			msleep( _monitoredProcess->_monitorInterval );
		}
		_endMark = true;
	}
}