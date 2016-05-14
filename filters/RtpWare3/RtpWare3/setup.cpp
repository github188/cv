
#pragma warning(disable:4995)

#include <streams.h>
#include <initguid.h>

#include "guids.h"
#include "rtp_sender_filter.h"
#include "rtp_receiver_filter.h"
#include "Log/Log.h"
#include "ortp_logger.h"

#include "ortp/ortp.h"


// Setup data

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
	&MEDIATYPE_NULL,            // Major type
	&MEDIASUBTYPE_NULL          // Minor type
};

const AMOVIESETUP_PIN sudRendererPins =
{
	L"Input",                   // Pin string name. Obsolete, not used.
	FALSE,                      // Is it rendered
	FALSE,                      // Is it an output
	FALSE,                      // Allowed none
	FALSE,                      // Likewise many
	&CLSID_NULL,                // Connects to filter. Obsolete.
	L"Output",                  // Connects to pin. Obsolete.
	1,                          // Number of types
	&sudPinTypes                // Pin information
};

const AMOVIESETUP_FILTER sudRenderer =
{
	&CLSID_SCUTRtpSender,  // Filter CLSID
	g_wszSCUTRtpSenderFilter,	// String name
	MERIT_DO_NOT_USE,				// Filter merit
	1,								// Number pins
	&sudRendererPins				// Pin details
};

const AMOVIESETUP_PIN sudSourcePin = 
{
	L"Output",      // Obsolete, not used.
	FALSE,          // Is this pin rendered?
	TRUE,           // Is it an output pin?
	FALSE,          // Can the filter create zero instances?
	FALSE,          // Does the filter create multiple instances?
	&CLSID_NULL,    // Obsolete.
	NULL,           // Obsolete.
	1,              // Number of media types.
	&sudPinTypes    // Pointer to media types.
};

const AMOVIESETUP_FILTER sudPushSource =
{
	&CLSID_SCUTRtpReceiver,	// Filter CLSID
	g_wszSCUTRtpReceiverFilter,	// String name
	MERIT_DO_NOT_USE,				// Filter merit
	1,								// Number pins
	&sudSourcePin					// Pin details
};


//
//  Object creation stuff
//
CFactoryTemplate g_Templates[]= {
	{ 
		g_wszSCUTRtpSenderFilter,
		&CLSID_SCUTRtpSender, 
		CRtpSenderFilter::CreateInstance, 
		NULL, 
		&sudRenderer 
	},
	{ 
		g_wszSCUTRtpReceiverFilter,		  // Name
		&CLSID_SCUTRtpReceiver,        // CLSID
		CRtpReceiverFilter::CreateInstance, // Method to create an instance of MyComponent
		NULL,								  // Initialization function
		&sudPushSource						  // Set-up information (for filters)
	}
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

//
// DllRegisterSever
//
// Handle the registration of this filter
//
STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2( TRUE );

} // DllRegisterServer


//
// DllUnregisterServer
//
STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2( FALSE );

} // DllUnregisterServer


//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
					  DWORD  dwReason, 
					  LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}


//OrtpInitializeHelper������ʼ�����ͷ�ortp��
//ortp��ȫ��ֻ�ܳ�ʼ��һ�Σ�����DLLж��ʱӦ�����١�
//��������ƵҪ��������filter���䴴�������ٵ��Ⱥ�˳��Ҳ�ǲ����ģ�
//���Բ������κ�һ��filter�Ĺ��캯�����������������������
//���Ǵ���һ�����ڶ��ϵ�ȫ�ֱ��������ڳ����˳�ж��DLLʱ����������
//������������������ortp����ȷ����ȫ��

class GlobalInitializeHelper
{
public:
  GlobalInitializeHelper()
  {
    //logger
    SetLogComponent("rtp");

    //ortp
    ortp_init();
    ortp_set_log_level_mask(/*ORTP_WARNING|*/ORTP_ERROR|ORTP_FATAL);
    ortp_set_log_handler(OrtpLogHandler);
    //ortp_scheduler_init(); //û�ã����ǲ��õ�������������ˣ��ǵ�Ҫ����ortp_exit()����Ȼ�����˳�ʱ������
    LOG_PRINTF("ortp init");
  }
  ~GlobalInitializeHelper()
  {
    ortp_exit();
    LOG_PRINTF("ortp exit");
  }
} global_init_helper;

