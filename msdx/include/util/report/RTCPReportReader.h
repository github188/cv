/**
 * @brief ���ù����ڴ�ķ�ʽ��ȡRTCP����
 */

#ifndef RTCP_REPORT_READER_H
#define RTCP_REPORT_READER_H
#include <string>
using namespace std;

class SingalMemoryMap;
typedef void*		HANDLE;

#include <util/report/RtpStat.h>

#include "ReporterCommon.h"

class RTCPReportReader
{
public:

	RTCPReportReader( const RTCPReportType type );
	~RTCPReportReader();

	/**
	 * @brief ��ʼ����������
	 */
	void setReader(const string& mediaID);

	/**
	 * @brief ��ȡrtcp����
	 */
	void readRtcpReport( RtpStatItem& item  );

private:
	SingalMemoryMap*	_memoryMap;

	///�����߳̾��
	HANDLE				_hThread;

	void*				_pHandlerObject;

	RTCPReportType		_reportType;
};


#endif