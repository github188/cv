/* tx_soapCvTxProxy.h
   Generated by gSOAP 2.8.21 from cvtx.h

Copyright(C) 2000-2014, Robert van Engelen, Genivia Inc. All Rights Reserved.
The generated code is released under one of the following licenses:
GPL or Genivia's license for commercial use.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
*/

#ifndef tx_soapCvTxProxy_H
#define tx_soapCvTxProxy_H
#include "tx_soapH.h"

namespace tx_soap {

class SOAP_CMAC CvTxProxy : public soap
{ public:
	/// Endpoint URL of service 'CvTxProxy' (change as needed)
	const char *soap_endpoint;
	/// Variables globally declared in cvtx.h (non-static)
	/// Constructor
	CvTxProxy();
	/// Construct from another engine state
	CvTxProxy(const struct soap&);
	/// Constructor with endpoint URL
	CvTxProxy(const char *url);
	/// Constructor with engine input+output mode control
	CvTxProxy(soap_mode iomode);
	/// Constructor with URL and input+output mode control
	CvTxProxy(const char *url, soap_mode iomode);
	/// Constructor with engine input and output mode control
	CvTxProxy(soap_mode imode, soap_mode omode);
	/// Destructor frees deserialized data
	virtual	~CvTxProxy();
	/// Initializer used by constructors
	virtual	void CvTxProxy_init(soap_mode imode, soap_mode omode);
	/// Delete all deserialized data (with soap_destroy and soap_end)
	virtual	void destroy();
	/// Delete all deserialized data and reset to default
	virtual	void reset();
	/// Disables and removes SOAP Header from message
	virtual	void soap_noheader();
	/// Get SOAP Header structure (NULL when absent)
	virtual	const SOAP_ENV__Header *soap_header();
	/// Get SOAP Fault structure (NULL when absent)
	virtual	const SOAP_ENV__Fault *soap_fault();
	/// Get SOAP Fault string (NULL when absent)
	virtual	const char *soap_fault_string();
	/// Get SOAP Fault detail as string (NULL when absent)
	virtual	const char *soap_fault_detail();
	/// Close connection (normally automatic, except for send_X ops)
	virtual	int soap_close_socket();
	/// Force close connection (can kill a thread blocked on IO)
	virtual	int soap_force_close_socket();
	/// Print fault
	virtual	void soap_print_fault(FILE*);
#ifndef WITH_LEAN
	/// Print fault to stream
#ifndef WITH_COMPAT
	virtual	void soap_stream_fault(std::ostream&);
#endif

	/// Put fault into buffer
	virtual	char *soap_sprint_fault(char *buf, size_t len);
#endif

	/// Web service operation 'QueryDBConference' (returns error code or SOAP_OK)
	virtual	int QueryDBConference(int offset, int count, ns__QueryDBConferenceResult *result) { return this->QueryDBConference(NULL, NULL, offset, count, result); }
	virtual	int QueryDBConference(const char *endpoint, const char *soap_action, int offset, int count, ns__QueryDBConferenceResult *result);

	/// Web service operation 'QueryDBTerminal' (returns error code or SOAP_OK)
	virtual	int QueryDBTerminal(int cid, ns__QueryDBTerminalResult *result) { return this->QueryDBTerminal(NULL, NULL, cid, result); }
	virtual	int QueryDBTerminal(const char *endpoint, const char *soap_action, int cid, ns__QueryDBTerminalResult *result);

	/// Web service operation 'QueryDBEpisodeDates' (returns error code or SOAP_OK)
	virtual	int QueryDBEpisodeDates(int cid, std::string uri, ns__QueryDBEpisodeDatesResult *result) { return this->QueryDBEpisodeDates(NULL, NULL, cid, uri, result); }
	virtual	int QueryDBEpisodeDates(const char *endpoint, const char *soap_action, int cid, std::string uri, ns__QueryDBEpisodeDatesResult *result);

	/// Web service operation 'QueryDBEpisode' (returns error code or SOAP_OK)
	virtual	int QueryDBEpisode(int cid, std::string uri, int offset, int count, ns__QueryDBEpisodeResult *result) { return this->QueryDBEpisode(NULL, NULL, cid, uri, offset, count, result); }
	virtual	int QueryDBEpisode(const char *endpoint, const char *soap_action, int cid, std::string uri, int offset, int count, ns__QueryDBEpisodeResult *result);

	/// Web service operation 'QueryDBEpisodeByTime' (returns error code or SOAP_OK)
	virtual	int QueryDBEpisodeByTime(int cid, std::string uri, LONG64 start, LONG64 end, ns__QueryDBEpisodeResult *result) { return this->QueryDBEpisodeByTime(NULL, NULL, cid, uri, start, end, result); }
	virtual	int QueryDBEpisodeByTime(const char *endpoint, const char *soap_action, int cid, std::string uri, LONG64 start, LONG64 end, ns__QueryDBEpisodeResult *result);

	/// Web service operation 'QueryDBView' (returns error code or SOAP_OK)
	virtual	int QueryDBView(LONG64 eid, ns__QueryDBViewResult *result) { return this->QueryDBView(NULL, NULL, eid, result); }
	virtual	int QueryDBView(const char *endpoint, const char *soap_action, LONG64 eid, ns__QueryDBViewResult *result);

	/// Web service operation 'StartRecord' (returns error code or SOAP_OK)
	virtual	int StartRecord(ns__StartRecordParam param, int *result) { return this->StartRecord(NULL, NULL, param, result); }
	virtual	int StartRecord(const char *endpoint, const char *soap_action, ns__StartRecordParam param, int *result);

	/// Web service operation 'StopRecord' (returns error code or SOAP_OK)
	virtual	int StopRecord(ns__StopRecordParam param, int *result) { return this->StopRecord(NULL, NULL, param, result); }
	virtual	int StopRecord(const char *endpoint, const char *soap_action, ns__StopRecordParam param, int *result);

	/// Web service operation 'GetRecordStatus' (returns error code or SOAP_OK)
	virtual	int GetRecordStatus(std::string focus, std::string uri, ns__GetRecordStatusResult *result) { return this->GetRecordStatus(NULL, NULL, focus, uri, result); }
	virtual	int GetRecordStatus(const char *endpoint, const char *soap_action, std::string focus, std::string uri, ns__GetRecordStatusResult *result);
};

} // namespace tx_soap

#endif