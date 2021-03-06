/* tx_soapCvTxProxy.cpp
   Generated by gSOAP 2.8.21 from cvtx.h

Copyright(C) 2000-2014, Robert van Engelen, Genivia Inc. All Rights Reserved.
The generated code is released under one of the following licenses:
GPL or Genivia's license for commercial use.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
*/

#include "tx_soapCvTxProxy.h"

namespace tx_soap {

CvTxProxy::CvTxProxy()
{	CvTxProxy_init(SOAP_IO_DEFAULT, SOAP_IO_DEFAULT);
}

CvTxProxy::CvTxProxy(const struct soap &_soap) : soap(_soap)
{ }

CvTxProxy::CvTxProxy(const char *url)
{	CvTxProxy_init(SOAP_IO_DEFAULT, SOAP_IO_DEFAULT);
	soap_endpoint = url;
}

CvTxProxy::CvTxProxy(soap_mode iomode)
{	CvTxProxy_init(iomode, iomode);
}

CvTxProxy::CvTxProxy(const char *url, soap_mode iomode)
{	CvTxProxy_init(iomode, iomode);
	soap_endpoint = url;
}

CvTxProxy::CvTxProxy(soap_mode imode, soap_mode omode)
{	CvTxProxy_init(imode, omode);
}

CvTxProxy::~CvTxProxy()
{ }

void CvTxProxy::CvTxProxy_init(soap_mode imode, soap_mode omode)
{	soap_imode(this, imode);
	soap_omode(this, omode);
	soap_endpoint = NULL;
	static const struct Namespace namespaces[] =
{
	{"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
	{"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
	{"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
	{"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
	{"ns", "urn:CvTx", NULL, NULL},
	{NULL, NULL, NULL, NULL}
};
	soap_set_namespaces(this, namespaces);
}

void CvTxProxy::destroy()
{	soap_destroy(this);
	soap_end(this);
}

void CvTxProxy::reset()
{	destroy();
	soap_done(this);
	soap_initialize(this);
	CvTxProxy_init(SOAP_IO_DEFAULT, SOAP_IO_DEFAULT);
}

void CvTxProxy::soap_noheader()
{	this->header = NULL;
}

const SOAP_ENV__Header *CvTxProxy::soap_header()
{	return this->header;
}

const SOAP_ENV__Fault *CvTxProxy::soap_fault()
{	return this->fault;
}

const char *CvTxProxy::soap_fault_string()
{	return *soap_faultstring(this);
}

const char *CvTxProxy::soap_fault_detail()
{	return *soap_faultdetail(this);
}

int CvTxProxy::soap_close_socket()
{	return soap_closesock(this);
}

int CvTxProxy::soap_force_close_socket()
{	return soap_force_closesock(this);
}

void CvTxProxy::soap_print_fault(FILE *fd)
{	::soap_print_fault(this, fd);
}

#ifndef WITH_LEAN
#ifndef WITH_COMPAT
void CvTxProxy::soap_stream_fault(std::ostream& os)
{	::soap_stream_fault(this, os);
}
#endif

char *CvTxProxy::soap_sprint_fault(char *buf, size_t len)
{	return ::soap_sprint_fault(this, buf, len);
}
#endif

int CvTxProxy::QueryDBConference(const char *endpoint, const char *soap_action, int offset, int count, ns__QueryDBConferenceResult *result)
{	struct soap *soap = this;
	struct ns__QueryDBConference soap_tmp_ns__QueryDBConference;
	if (endpoint)
		soap_endpoint = endpoint;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://127.0.0.1:8091/CvTx";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__QueryDBConference.offset = offset;
	soap_tmp_ns__QueryDBConference.count = count;
	soap_serializeheader(soap);
	soap_serialize_ns__QueryDBConference(soap, &soap_tmp_ns__QueryDBConference);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__QueryDBConference(soap, &soap_tmp_ns__QueryDBConference, "ns:QueryDBConference", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__QueryDBConference(soap, &soap_tmp_ns__QueryDBConference, "ns:QueryDBConference", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	result->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	result->soap_get(soap, "", NULL);
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

int CvTxProxy::QueryDBTerminal(const char *endpoint, const char *soap_action, int cid, ns__QueryDBTerminalResult *result)
{	struct soap *soap = this;
	struct ns__QueryDBTerminal soap_tmp_ns__QueryDBTerminal;
	if (endpoint)
		soap_endpoint = endpoint;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://127.0.0.1:8091/CvTx";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__QueryDBTerminal.cid = cid;
	soap_serializeheader(soap);
	soap_serialize_ns__QueryDBTerminal(soap, &soap_tmp_ns__QueryDBTerminal);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__QueryDBTerminal(soap, &soap_tmp_ns__QueryDBTerminal, "ns:QueryDBTerminal", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__QueryDBTerminal(soap, &soap_tmp_ns__QueryDBTerminal, "ns:QueryDBTerminal", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	result->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	result->soap_get(soap, "", NULL);
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

int CvTxProxy::QueryDBEpisodeDates(const char *endpoint, const char *soap_action, int cid, std::string uri, ns__QueryDBEpisodeDatesResult *result)
{	struct soap *soap = this;
	struct ns__QueryDBEpisodeDates soap_tmp_ns__QueryDBEpisodeDates;
	if (endpoint)
		soap_endpoint = endpoint;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://127.0.0.1:8091/CvTx";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__QueryDBEpisodeDates.cid = cid;
	soap_tmp_ns__QueryDBEpisodeDates.uri = uri;
	soap_serializeheader(soap);
	soap_serialize_ns__QueryDBEpisodeDates(soap, &soap_tmp_ns__QueryDBEpisodeDates);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__QueryDBEpisodeDates(soap, &soap_tmp_ns__QueryDBEpisodeDates, "ns:QueryDBEpisodeDates", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__QueryDBEpisodeDates(soap, &soap_tmp_ns__QueryDBEpisodeDates, "ns:QueryDBEpisodeDates", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	result->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	result->soap_get(soap, "", NULL);
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

int CvTxProxy::QueryDBEpisode(const char *endpoint, const char *soap_action, int cid, std::string uri, int offset, int count, ns__QueryDBEpisodeResult *result)
{	struct soap *soap = this;
	struct ns__QueryDBEpisode soap_tmp_ns__QueryDBEpisode;
	if (endpoint)
		soap_endpoint = endpoint;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://127.0.0.1:8091/CvTx";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__QueryDBEpisode.cid = cid;
	soap_tmp_ns__QueryDBEpisode.uri = uri;
	soap_tmp_ns__QueryDBEpisode.offset = offset;
	soap_tmp_ns__QueryDBEpisode.count = count;
	soap_serializeheader(soap);
	soap_serialize_ns__QueryDBEpisode(soap, &soap_tmp_ns__QueryDBEpisode);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__QueryDBEpisode(soap, &soap_tmp_ns__QueryDBEpisode, "ns:QueryDBEpisode", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__QueryDBEpisode(soap, &soap_tmp_ns__QueryDBEpisode, "ns:QueryDBEpisode", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	result->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	result->soap_get(soap, "", NULL);
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

int CvTxProxy::QueryDBEpisodeByTime(const char *endpoint, const char *soap_action, int cid, std::string uri, LONG64 start, LONG64 end, ns__QueryDBEpisodeResult *result)
{	struct soap *soap = this;
	struct ns__QueryDBEpisodeByTime soap_tmp_ns__QueryDBEpisodeByTime;
	if (endpoint)
		soap_endpoint = endpoint;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://127.0.0.1:8091/CvTx";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__QueryDBEpisodeByTime.cid = cid;
	soap_tmp_ns__QueryDBEpisodeByTime.uri = uri;
	soap_tmp_ns__QueryDBEpisodeByTime.start = start;
	soap_tmp_ns__QueryDBEpisodeByTime.end = end;
	soap_serializeheader(soap);
	soap_serialize_ns__QueryDBEpisodeByTime(soap, &soap_tmp_ns__QueryDBEpisodeByTime);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__QueryDBEpisodeByTime(soap, &soap_tmp_ns__QueryDBEpisodeByTime, "ns:QueryDBEpisodeByTime", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__QueryDBEpisodeByTime(soap, &soap_tmp_ns__QueryDBEpisodeByTime, "ns:QueryDBEpisodeByTime", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	result->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	result->soap_get(soap, "", NULL);
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

int CvTxProxy::QueryDBView(const char *endpoint, const char *soap_action, LONG64 eid, ns__QueryDBViewResult *result)
{	struct soap *soap = this;
	struct ns__QueryDBView soap_tmp_ns__QueryDBView;
	if (endpoint)
		soap_endpoint = endpoint;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://127.0.0.1:8091/CvTx";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__QueryDBView.eid = eid;
	soap_serializeheader(soap);
	soap_serialize_ns__QueryDBView(soap, &soap_tmp_ns__QueryDBView);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__QueryDBView(soap, &soap_tmp_ns__QueryDBView, "ns:QueryDBView", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__QueryDBView(soap, &soap_tmp_ns__QueryDBView, "ns:QueryDBView", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	result->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	result->soap_get(soap, "", NULL);
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

int CvTxProxy::StartRecord(const char *endpoint, const char *soap_action, ns__StartRecordParam param, int *result)
{	struct soap *soap = this;
	struct ns__StartRecord soap_tmp_ns__StartRecord;
	struct ns__StartRecordResponse *soap_tmp_ns__StartRecordResponse;
	if (endpoint)
		soap_endpoint = endpoint;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://127.0.0.1:8091/CvTx";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__StartRecord.param = param;
	soap_serializeheader(soap);
	soap_serialize_ns__StartRecord(soap, &soap_tmp_ns__StartRecord);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__StartRecord(soap, &soap_tmp_ns__StartRecord, "ns:StartRecord", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__StartRecord(soap, &soap_tmp_ns__StartRecord, "ns:StartRecord", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	soap_default_int(soap, result);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns__StartRecordResponse = soap_get_ns__StartRecordResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns__StartRecordResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (result && soap_tmp_ns__StartRecordResponse->result)
		*result = *soap_tmp_ns__StartRecordResponse->result;
	return soap_closesock(soap);
}

int CvTxProxy::StopRecord(const char *endpoint, const char *soap_action, ns__StopRecordParam param, int *result)
{	struct soap *soap = this;
	struct ns__StopRecord soap_tmp_ns__StopRecord;
	struct ns__StopRecordResponse *soap_tmp_ns__StopRecordResponse;
	if (endpoint)
		soap_endpoint = endpoint;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://127.0.0.1:8091/CvTx";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__StopRecord.param = param;
	soap_serializeheader(soap);
	soap_serialize_ns__StopRecord(soap, &soap_tmp_ns__StopRecord);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__StopRecord(soap, &soap_tmp_ns__StopRecord, "ns:StopRecord", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__StopRecord(soap, &soap_tmp_ns__StopRecord, "ns:StopRecord", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	soap_default_int(soap, result);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns__StopRecordResponse = soap_get_ns__StopRecordResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns__StopRecordResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (result && soap_tmp_ns__StopRecordResponse->result)
		*result = *soap_tmp_ns__StopRecordResponse->result;
	return soap_closesock(soap);
}

int CvTxProxy::GetRecordStatus(const char *endpoint, const char *soap_action, std::string focus, std::string uri, ns__GetRecordStatusResult *result)
{	struct soap *soap = this;
	struct ns__GetRecordStatus soap_tmp_ns__GetRecordStatus;
	if (endpoint)
		soap_endpoint = endpoint;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://127.0.0.1:8091/CvTx";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__GetRecordStatus.focus = focus;
	soap_tmp_ns__GetRecordStatus.uri = uri;
	soap_serializeheader(soap);
	soap_serialize_ns__GetRecordStatus(soap, &soap_tmp_ns__GetRecordStatus);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__GetRecordStatus(soap, &soap_tmp_ns__GetRecordStatus, "ns:GetRecordStatus", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__GetRecordStatus(soap, &soap_tmp_ns__GetRecordStatus, "ns:GetRecordStatus", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	result->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	result->soap_get(soap, "", NULL);
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

} // namespace tx_soap

/* End of client proxy code */
