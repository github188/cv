#include "RTCPReportReader.h"
#include <util/sharememory/SingalMemoryMap.h>
#include <log/Log.h>
#include <dbus/msdx/common/MsdxServiceCommon.h>


RTCPReportReader::RTCPReportReader( const RTCPReportType type )
{
	_memoryMap = NULL;
	_reportType = type;
}

RTCPReportReader::~RTCPReportReader()
{

	//�ͷ�ԭ�е���Դ
	if( _memoryMap!=NULL )
	{
		delete _memoryMap;
		_memoryMap = NULL;
	}

}

void RTCPReportReader::setReader( const string& mediaID  )
{
	if( _memoryMap!=NULL )
	{
		delete _memoryMap;
		_memoryMap = NULL;
	}

	string workerID = getWorkerIDFromMediaID(mediaID , _reportType );

	//�����µĹ����ڴ�
	string mapFileName = workerID +"_MapFile";
	_memoryMap = new SingalMemoryMap( mapFileName );
	if( _memoryMap==NULL )
	{
		LOG_ERROR("���������ڴ�ʱʧ��!");
	}
}

void RTCPReportReader::readRtcpReport( RtpStatItem& item )
{
	if( _memoryMap )
	{
		_memoryMap->readData( &item , sizeof(item) );
	}
}