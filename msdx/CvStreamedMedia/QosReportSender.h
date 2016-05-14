//////////////////////////////////////////////////////////////////////////
///@brief	����Qos���淢������
///@date	2011/3/15
//////////////////////////////////////////////////////////////////////////
#ifndef QOSREPORTSENDER_H
#define QOSREPORTSENDER_H

#include <string>
#include "util/report/RtpStat.h"
#include "util/report/RecordStat.h"
#include "util/report/FilePlayStat.h"

class UdpSender;

class QosReportSender{
	
public:
	QosReportSender( const char* remoteIP, const int port );
	~QosReportSender();
	void HandleRtpStat(const RtpStatItem& rtpstat);

	/**
	 * @brief ��rtcp�Ķ����������͸�������������
	 */
	void sendRtpStat( const RtpStatItem &rtpstat );
	/**
	 * @brief ��¼��״̬�Ķ����������͸�������������
	 */
  void sendRecStat( const RecStatItem &stat );

  void sendFilePlayStat( const FilePlayStatItem &stat );

protected:
    void init( const char* remoteIP , const int port );
	void convertRtpStatReport(const RtpStatItem& rtpstat, char *msg, int &msgsize); 
    void sendStatPackage(const struct UDPReportHeader &, const char * data);

private:
	int _currentConferenceId;
	std::string _qosServerIP;
	int _qosServerPort;
	UdpSender *_reportSender;
};


#endif