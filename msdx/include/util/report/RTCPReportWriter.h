/**
 * @brief �������ڴ���д�뱨��
 */

#ifndef RTCP_REPORT_WRITER_H
#define RTCP_REPORT_WRITER_H
#include <Windows.h>


class SingalMemoryMap;
#include "RtpStat.h"
#include "ReporterCommon.h"
#include <string>
using namespace std;
class RTCPReportWriter
{
public:
	/**
	 * @brief writerID���õ���ý������ID
	 */
	RTCPReportWriter( const string& writerID , const RTCPReportType type);
	~RTCPReportWriter();

	void writeRTCPReport( const RtpStatItem& item );

private:
	SingalMemoryMap*	_memoryMap;

	///�����ź���
	static const int	_MAX_SEMAPHORE_COUNT = 1;

	///�������ڴ���д�뱨��Ĵ���
	UINT32				_writeCount;

	///Writer������
	RTCPReportType		_reportType;
};

#endif