// DeviceManager.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include <stdio.h>
#include <algorithm>

#include <DeviceManager/DeviceManager.h>
#include "DirectXUtils.h"

using std::vector;
using std::string;
using std::wstring;

//vista���ϰ汾֧��
#include <mmdeviceapi.h>
#include <propsys.h>
#include <Endpointvolume.h>
#include <Audioclient.h>
#include <Functiondiscoverykeys_devpkey.h>

//�����豸
#include <IPTypes.h>
#include <IPHlpApi.h>
#include <WinSock2.h>

typedef BOOL (_stdcall *DHCPNOTIFYPROC)(LPWSTR , LPWSTR, BOOL , DWORD,DWORD,DWORD,int );

BOOL NotifyIPChanged( LPCTSTR lpszAdaptorName , int nIndex , LPCTSTR pIPAddress , LPCTSTR pNetMask, bool enableDHCP )
{
	BOOL bResult = FALSE;
	HINSTANCE hDhcpDll;
	DHCPNOTIFYPROC	pDhcpNotifyProc;

	WCHAR wcAdapterName[256];
	MultiByteToWideChar( CP_ACP , 0 , lpszAdaptorName , -1 , wcAdapterName,256 );

	if( (hDhcpDll=LoadLibrary("dhcpcsvc"))==NULL )
		return FALSE;

	if( (pDhcpNotifyProc=(DHCPNOTIFYPROC)GetProcAddress(hDhcpDll,"DhcpNotifyConfigChange"))!=NULL )
	{
		if( enableDHCP )
		{
			if( (pDhcpNotifyProc)(NULL , wcAdapterName , TRUE , nIndex , inet_addr(pIPAddress),inet_addr(pNetMask),1)==ERROR_SUCCESS )
				bResult = TRUE;
		}else
		{
			if( (pDhcpNotifyProc)(NULL , wcAdapterName , TRUE , nIndex , inet_addr(pIPAddress),inet_addr(pNetMask),2)==ERROR_SUCCESS )
				bResult = TRUE;
		}
	}
	FreeLibrary( hDhcpDll );
	return bResult;

}

DeviceManager::DeviceManager() {
  CoInitialize(NULL);
}

DeviceManager::~DeviceManager() {
  CoUninitialize();
}


void DeviceManager::GetVideoCaptureDevices( vector<string>* deviceList )
{
	if( deviceList==NULL )
		return;

	deviceList->clear();

	vector<wstring> Devices;
	int deviceNum =0;

	HRESULT result = ListCaptureDevice(CLSID_VideoInputDeviceCategory, Devices ,deviceNum );
	if( SUCCEEDED(result) )
	{
		wstring s1(L"PCIe SD");
		wstring s2(L"Timeleak sd");
		for( int i=0 ; i<deviceNum ; i+=2 )
		{
			wstring str1head = Devices[i].substr(0,7);
			wstring str2head = Devices[i].substr(0,11);
			if ( str1head == s1 || str2head == s2);
			else
				deviceList->push_back( wstr2str(Devices[i]) );
		}
	}else
	{
		printf("��ȡ����ͷ�豸��������");
	}

}

// ���ش˺������Զ���Ƶ�ɼ��������
void DeviceManager::GetVideoCaptureDevices(VCapDeviceList* deviceList) {
  deviceList->clear();

  vector<DeviceInfo> devices;
  int counter = 0;

  HRESULT hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, devices);
  for (int i = 0; i < devices.size(); ++i) {
	VCapDevice captureDevice;
	if (devices[i].friendly_name.substr(0,7) != L"PCIe SD" ||
        devices[i].friendly_name.substr(0,11) != L"Timeleak sd") {
	  captureDevice.friendlyName = wstr2str(devices[i].friendly_name);
	  captureDevice.devicePath = wstr2str(devices[i].device_path);

	  captureDevice.displayName = wstr2str(devices[i].display_name);
	  captureDevice.deviceNo = counter;
	  deviceList->push_back(captureDevice);
	  ++counter;
	}

  }
  //VCapDevice d;
  //string strName = "Kinect Camera V2test";
  //string strPath = "@device:sw:{860BB310-5D01-11D0-BD3B-00A0C911CE86}\\{E48ECF1A-A5E7-4EB0-8BF7-E15185D66FA4}";
  //d.friendlyName = strName;
  //d.devicePath = strPath;
  //d.deviceNo = counter;
  //deviceList->push_back(d);
  //++counter;
}

void DeviceManager::GetAudioCaptureDevices( vector<string>* deviceList )
{
	if( deviceList==NULL )
		return;

	deviceList->clear();

	vector<wstring> Devices;
	int deviceNum =0;

	HRESULT result = ListCaptureDevice(CLSID_AudioInputDeviceCategory, Devices ,deviceNum );
	if( SUCCEEDED(result) )
	{
		for( int i=0 ; i<deviceNum ; i+=2 )
		{
			deviceList->push_back( wstr2str(Devices[i]) );
		}
	}else
	{
		printf("��ȡ����ͷ�豸��������");
	}
}

void DeviceManager::GetAudioOutputDevices( vector<string>* deviceList )
{
	if( deviceList==NULL )
		return;

	deviceList->clear();

	vector<wstring> Devices;
	int deviceNum =0;

	HRESULT result = ListCaptureDevice(CLSID_AudioRendererCategory, Devices ,deviceNum );
	if( SUCCEEDED(result) )
	{
		for( int i=0 ; i<deviceNum ; i+=2 )
		{
            deviceList->push_back( wstr2str(Devices[i]) );
		}
	}else
	{
		printf("��ȡ����ͷ�豸��������");
	}
}

void DeviceManager::GetCrossbarDeviceList( const string captureDevice, vector<string> *deviceList )
{
	if( deviceList==NULL )
		return;

	deviceList->clear();

	int numOfDevice = 0;
	vector<wstring> deviceCaptureList;
	HRESULT result = ListCaptureDevice( AM_KSCATEGORY_CROSSBAR , deviceCaptureList , numOfDevice );
	if( FAILED( result ) )
		return ;

    //����friendlyName��displayName���ײ�ҪdisplayName
	for( int deviceCount=0 ; deviceCount<numOfDevice ; ++deviceCount )
	{
		string deviceName = wstr2str( deviceCaptureList[deviceCount] );
		deviceList->push_back( deviceName );
	}
}

void DeviceManager::GetCrossbarInputType( const std::string &crossbarDisplayName , vector<string> *typeList )
{
	//ת��Ϊ���ַ���
	std::wstring w_crossbarName = str2wstr( crossbarDisplayName );
	int numOfDevice = 0;
	vector<wstring> deviceCaptureList;
	HRESULT result = ListCaptureDevice( AM_KSCATEGORY_CROSSBAR , deviceCaptureList , numOfDevice );
	if( FAILED( result ) )
		return ;

	/*for( int deviceCount=0 ; deviceCount<numOfDevice ; deviceCount+=2 )
	{
		if( wcscmp( w_crossbarName.c_str() , deviceCaptureList[deviceCount].c_str() )==0 )
		{*/
			IMoniker *pMoniker=NULL;
			IBaseFilter *m_pCrossbarFilter=0;
			IBindCtx *pBindCtx;
			HRESULT hr=CreateBindCtx(0,&pBindCtx);
			ULONG chEaten=0;
			//hr= MkParseDisplayName(pBindCtx,deviceCaptureList[deviceCount+1].c_str(),&chEaten,&pMoniker);
			hr= MkParseDisplayName(pBindCtx,w_crossbarName.c_str(),&chEaten,&pMoniker);
			pBindCtx->Release();
			if(FAILED(hr))
			{
				return ;
			}
			pMoniker->AddRef();
			hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pCrossbarFilter);
			if (m_pCrossbarFilter == NULL) 
			{
				pMoniker->Release();
				return ;
			}

			//��ȡcrossbar filter���������
			IAMCrossbar *m_pCrossBar;
			hr=m_pCrossbarFilter->QueryInterface(IID_IAMCrossbar,(void   **)&m_pCrossBar);
			if( FAILED(hr) )
			{
				pMoniker->Release();
				m_pCrossbarFilter->Release();
				return;
			}

			long outCount = -1; //�����ż���
			long inCount = -1; //������ż���
			m_pCrossBar->get_PinCounts(&outCount,&inCount);

			for(int i=0;i<inCount;i++)
			{
				long index = 0;
				//pinType�����������pin����������,��Svideo��
				long pinType;		

				m_pCrossBar->get_CrossbarPinInfo(TRUE,i,&index,&pinType );
				switch( pinType )
				{
				case PhysConn_Video_Composite:
					typeList->push_back("Composite");
					break;
				case PhysConn_Video_SVideo:
					typeList->push_back("SVideo");
					break;
				case PhysConn_Video_Tuner:
					typeList->push_back("Tuner");
					break;
				}
			}

			//�ͷ���Դ
			pMoniker->Release();
			m_pCrossbarFilter->Release();
			m_pCrossBar->Release();
			/*break;
		}
	}*/
}

void DeviceManager::GetVideoDeviceMediaType( const std::string &deviceName , vector<string>* typeList )
{
	char media_type_info[56];
	HRESULT hr;
	IAMStreamConfig* pStreamConfig = NULL;
	IBaseFilter*  pCaptureDevice = NULL;
	CComBSTR bstrName(deviceName.c_str());
	hr = FindMyCaptureDevice(&pCaptureDevice, bstrName);
	if ((hr != S_OK) || !pCaptureDevice) {
		return ;
	}

	//initialize IAMStreamConfig
	pStreamConfig = GetIAMStreamConfig(pCaptureDevice);
	if (!pStreamConfig) {
		return ;
	}

	int iCount, iSize;
	hr = pStreamConfig->GetNumberOfCapabilities(&iCount, &iSize);

	VIDEO_STREAM_CONFIG_CAPS scc;
	if (sizeof(scc) != iSize) {
		printf("wrong config structure");
		pStreamConfig->Release();
		return ;
	}
	//vector<string> resolutionList;

	for (int i = 0; i < iCount; i++) {
		AM_MEDIA_TYPE * pmt = NULL;
		hr = pStreamConfig->GetStreamCaps(i, &pmt, reinterpret_cast<BYTE *>(&scc));
		if (hr == S_OK) {

			VIDEOINFOHEADER * pvi = (VIDEOINFOHEADER *) pmt->pbFormat;

			if( pmt->subtype == MEDIASUBTYPE_YUY2 
				|| pmt->subtype==MEDIASUBTYPE_RGB24 
				)
			{
				if( pvi->bmiHeader.biWidth<300 || pvi->bmiHeader.biHeight<200 )
					continue;

				int fps = 10000000 / pvi->AvgTimePerFrame;
				if( fps<10 )		//����15֡�Ĳ�Ҫ
					continue;

				sprintf_s( media_type_info , "%dX%dX%d", pvi->bmiHeader.biWidth , pvi->bmiHeader.biHeight, fps );
				std::string mediaTypeInfo = media_type_info;

				//�ж��Ƿ��Ѿ����ڣ�����Ѿ����������
				vector<string>::iterator it;
				for( it = typeList->begin() ; it!=typeList->end() ; it++ )
				{
					if( *it == mediaTypeInfo )
					{
						break;
					}
				}
				if( it==typeList->end() )
				{	
					//resolutionList.push_back(mediaTypeInfo);
					//��ÿ�ֱַ��ʣ�ȡöȡ�õ��ĵ�һ��֡��
					//sprintf_s(media_type_info, "X%d", pvi->AvgTimePerFrame ? 10000000 / pvi->AvgTimePerFrame : 0);
					//mediaTypeInfo += media_type_info;
					typeList->push_back(mediaTypeInfo);
				}
			}
		}
	}

	pCaptureDevice->Release();
	pStreamConfig->Release();

}

/**
 * @brief GetAudioOutputVolumn���ڻ�ȡ�豸�������
 * @remark ��ָ���豸���ܵ��»�ȡ������Ϊ0
 */
int DeviceManager::GetAudioOutputVolumn( const std::string &deviceName )
{
    //����DirectSound�豸����ȡ���Ӧ�����豸����
    std::string deviceFriendlyName = deviceName;
    if (deviceName.find("DirectSound: ") == 0) 
    {
        //�豸����"DirectSound: "��ʼ
        deviceFriendlyName = deviceName.substr(strlen("DirectSound: "));
    }

	OSVERSIONINFO osVer;
	osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx( &osVer );
	if( osVer.dwMajorVersion==6 )
	{
		//vista��win7����ȡ��ͬ�ķ�ʽ��ȡ����
		IMMDeviceEnumerator* pEnumerator;
		IMMDeviceCollection* pCollection = NULL;
		IMMDevice *pDevice = NULL;
		IPropertyStore *pProperties=NULL;  
		IAudioEndpointVolume *pVolumeAPI=NULL;  
		UINT deviceCount = 0;

		float fVolume = -1;

		CoInitializeEx( NULL , COINIT_MULTITHREADED );

		HRESULT hr=CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),(void**)&pEnumerator);
		if (hr != S_OK)  
		{  
			printf("CoCreateInstance Failed!\n");  
			return 0;  
		}  

		//hr = pEnumerator->EnumAudioEndpoints(eCapture /*eRender*/, DEVICE_STATE_ACTIVE, &pCollection);
		hr = pEnumerator->EnumAudioEndpoints( eRender , DEVICE_STATE_ACTIVE , &pCollection );
		if (hr != S_OK)  
		{  
			printf("EnumAudioEndpoints Failed!\n");  
			goto releasepEnumerator;  
		}  

		hr = pCollection->GetCount(&deviceCount);  
		if (hr != S_OK)  
		{  
			printf("GetCount Failed!\n");  
			goto releasepCollection;  
		}  

		for (UINT dev=0;dev<deviceCount;dev++)  
		{  
			pDevice = NULL;  
			hr = pCollection->Item(dev,&pDevice);  
			if (hr == S_OK)  
			{  

				hr = pDevice->OpenPropertyStore(STGM_READ,&pProperties);  
				if (hr == S_OK)  
				{  
					PROPVARIANT varName;  
					PropVariantInit(&varName);  
					hr = pProperties->GetValue(PKEY_Device_FriendlyName, &varName); 
					if( SUCCEEDED(hr) )
					{
            if (varName.vt != VT_LPWSTR || 
                (wstr2str(varName.pwszVal).find(deviceFriendlyName) != 0)) //�����ֵ���ܲ���������Ҫǰ����ƥ��
            {
              continue;
            }
						hr=pDevice->Activate(__uuidof(IAudioEndpointVolume),CLSCTX_ALL,NULL,(void **)(&pVolumeAPI));  
						if (hr==S_OK)  
						{  
							hr = pVolumeAPI->GetMasterVolumeLevelScalar( &fVolume );
              if (S_OK == hr) break;
						} 
					}

					SAFE_RELEASE(pProperties);  
				}  

				SAFE_RELEASE(pDevice);  
			}  
		}  

releasepCollection:  

		SAFE_RELEASE(pCollection);  

releasepEnumerator:  

		SAFE_RELEASE(pEnumerator);  

		return  fVolume*100;
	}else
	{
		MMRESULT          rc;     // ��ý�庯�����ؽ������
		HMIXER            hMixer; // ������豸���
		MIXERLINE         mxl;    // ��Ƶ��·��׼״̬��Ϣ�ṹ��
		MIXERLINECONTROLS mxlc;   // ��Ƶ��·������������Ϣ�ṹ��
		MIXERCONTROL      mxc;    // ������Ƶ��·��������Ϣ�ṹ��

		// �򿪻�����豸
		rc = mixerOpen(&hMixer, // ���ص��豸���
			0,       // ���������豸IDΪ��
			0,       // ��ʹ�ûص�����
			0,       // �ص���������
			0);      // MIXER_OBJECTF_MIXER���ֵ����ʾ��ѡ��Ч�豸ID

		// �򿪻�����豸�޴�Ļ�����
		if (MMSYSERR_NOERROR == rc)
		{
			// MIXERLINE �ṹ���������
			ZeroMemory(&mxl, sizeof(MIXERLINE));

			mxl.cbStruct = sizeof(MIXERLINE); // ΢���ô˰취�жϰ汾
			// ָ����Ҫ��ȡ��ͨ������������Ƶ�����MIXERLINE_COMPONENTTYPE_DST_SPEAKERS
			mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

			// ȡ�û�����豸��ָ����·��Ϣ
			rc = mixerGetLineInfo((HMIXEROBJ)hMixer,
				&mxl,
				// ȡ��MIXERLINE::dwComponentTypeָ�����͵ĵ�һ����Ƶ��·��Ϣ
				MIXER_GETLINEINFOF_COMPONENTTYPE);

			// ȡ�û�����豸��ָ����·��Ϣ�ɹ��Ļ�����
			if (MMSYSERR_NOERROR == rc)
			{
				// MIXERCONTROL �ṹ���������
				ZeroMemory(&mxc, sizeof(MIXERCONTROL));

				mxc.cbStruct = sizeof(mxc); // ΢���ô˰취�жϰ汾

				// MIXERLINECONTROLS �ṹ���������
				ZeroMemory(&mxlc, sizeof(MIXERLINECONTROLS));

				mxlc.cbStruct = sizeof(mxlc); // ΢���ô˰취�жϰ汾

				mxlc.dwLineID = mxl.dwLineID; // ����ȡ�õ�������Ƶ�����·��ʶ
				// ��������Ϊ��������
				mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
				mxlc.cControls = 1;          // ʹ�� MIXERCONTROL �ṹ���������
				mxlc.pamxctrl = &mxc;        // MIXERCONTROL �ṹ�����ָ��
				mxlc.cbmxctrl = sizeof(mxc); // MIXERCONTROL �ṹ������ֽڴ�С

				// ȡ�ÿ�������Ϣ
				rc = mixerGetLineControls((HMIXEROBJ)hMixer,
					&mxlc,
					MIXER_GETLINECONTROLSF_ONEBYTYPE);

				// ȡ�ÿ�������Ϣ�ɹ��Ļ�����
				if (MMSYSERR_NOERROR == rc)
				{
					// ��ȡ�������е�ֵ��������Χ��mxc.Bounds.lMinimum��mxc.Bounds.lMaximum.
					MIXERCONTROLDETAILS        mxcd;      // ��������״̬��Ϣ
					MIXERCONTROLDETAILS_SIGNED volStruct; // �����ṹ�����(��һ����Ա��)

					// MIXERCONTROLDETAILS �ṹ���������
					ZeroMemory(&mxcd, sizeof(mxcd));

					mxcd.cbStruct = sizeof(mxcd);       // ΢���ô˰취�жϰ汾
					mxcd.dwControlID = mxc.dwControlID; // ����ȡ�õĿ�������ʶ
					mxcd.paDetails = &volStruct;        // �����ṹ�����ָ��
					mxcd.cbDetails = sizeof(volStruct); // �����ṹ������ֽڴ�С
					mxcd.cChannels = 1;                 // ȡ�û�����ȫ��ͨ��

					//  ��ȡ�������е�ֵ��������Χ��mxc.Bounds.lMinimum��mxc.Bounds.lMaximum. 

					// �������ֵ
					rc = mixerGetControlDetails((HMIXEROBJ)hMixer,
						&mxcd,
						MIXER_GETCONTROLDETAILSF_VALUE);

					if (MMSYSERR_NOERROR == rc)
					{
						long step = ( mxc.Bounds.lMaximum - mxc.Bounds.lMinimum ) /100;

						int index = ( volStruct.lValue - mxc.Bounds.lMinimum) /step ;
						printf("������%X", volStruct.lValue);
						return index;
					}
				}
			}
		}
	}

	return -1;
}

int DeviceManager::SetAudioOutputVolumn( int volumnIndex )
{
	OSVERSIONINFO osVer;
	osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx( &osVer );
	if( osVer.dwMajorVersion==6 )
	{
		//vista��win7����ȡ��ͬ�ķ�ʽ��ȡ����
		IMMDeviceEnumerator* pEnumerator;
		IMMDeviceCollection* pCollection = NULL;
		IMMDevice *pDevice = NULL;
		IPropertyStore *pProperties=NULL;  
		IAudioEndpointVolume *pVolumeAPI=NULL;  
		UINT deviceCount = 0;

		float fVolume = -1;

		CoInitializeEx( NULL , COINIT_MULTITHREADED );

		HRESULT hr=CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),(void**)&pEnumerator);
		if (hr != S_OK)  
		{  
			printf("CoCreateInstance Failed!\n");  
			return 0;  
		}  

		//hr = pEnumerator->EnumAudioEndpoints(eCapture /*eRender*/, DEVICE_STATE_ACTIVE, &pCollection);
		hr = pEnumerator->EnumAudioEndpoints( eRender , DEVICE_STATE_ACTIVE , &pCollection );
		if (hr != S_OK)  
		{  
			printf("EnumAudioEndpoints Failed!\n");  
			goto releasepEnumerator;  
		}  

		hr = pCollection->GetCount(&deviceCount);  
		if (hr != S_OK)  
		{  
			printf("GetCount Failed!\n");  
			goto releasepCollection;  
		}  

		for (UINT dev=0;dev<deviceCount;dev++)  
		{  
			pDevice = NULL;  
			hr = pCollection->Item(dev,&pDevice);  
			if (hr == S_OK)  
			{  

				hr = pDevice->OpenPropertyStore(STGM_READ,&pProperties);  
				if (hr == S_OK)  
				{  
					PROPVARIANT varName;  
					PropVariantInit(&varName);  
					hr = pProperties->GetValue(PKEY_Device_FriendlyName, &varName); 
					if( SUCCEEDED(hr) )
					{
						hr=pDevice->Activate(__uuidof(IAudioEndpointVolume),CLSCTX_ALL,NULL,(void **)(&pVolumeAPI));  
						if (hr==S_OK)  
						{  
							float volumnLevel = (float)volumnIndex /100;
							hr = pVolumeAPI->SetMasterVolumeLevelScalar( volumnLevel, NULL );
						} 
					}
					SAFE_RELEASE(pProperties);  

				}  

				SAFE_RELEASE(pDevice);  
			}  
		}  

releasepCollection:  

		SAFE_RELEASE(pCollection);  

releasepEnumerator:  

		SAFE_RELEASE(pEnumerator);  

		return  fVolume*100;
	}else
	{
		MMRESULT          rc;     // ��ý�庯�����ؽ������
		HMIXER            hMixer; // ������豸���
		MIXERLINE         mxl;    // ��Ƶ��·��׼״̬��Ϣ�ṹ��
		MIXERLINECONTROLS mxlc;   // ��Ƶ��·������������Ϣ�ṹ��
		MIXERCONTROL      mxc;    // ������Ƶ��·��������Ϣ�ṹ��

		// �򿪻�����豸
		rc = mixerOpen(&hMixer, // ���ص��豸���
			0,       // ���������豸IDΪ��
			0,       // ��ʹ�ûص�����
			0,       // �ص���������
			0);      // MIXER_OBJECTF_MIXER���ֵ����ʾ��ѡ��Ч�豸ID

		// �򿪻�����豸�޴�Ļ�����
		if (MMSYSERR_NOERROR == rc)
		{
			// MIXERLINE �ṹ���������
			ZeroMemory(&mxl, sizeof(MIXERLINE));

			mxl.cbStruct = sizeof(MIXERLINE); // ΢���ô˰취�жϰ汾
			// ָ����Ҫ��ȡ��ͨ������������Ƶ�����MIXERLINE_COMPONENTTYPE_DST_SPEAKERS
			mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

			// ȡ�û�����豸��ָ����·��Ϣ
			rc = mixerGetLineInfo((HMIXEROBJ)hMixer,
				&mxl,
				// ȡ��MIXERLINE::dwComponentTypeָ�����͵ĵ�һ����Ƶ��·��Ϣ
				MIXER_GETLINEINFOF_COMPONENTTYPE);

			// ȡ�û�����豸��ָ����·��Ϣ�ɹ��Ļ�����
			if (MMSYSERR_NOERROR == rc)
			{
				// MIXERCONTROL �ṹ���������
				ZeroMemory(&mxc, sizeof(MIXERCONTROL));

				mxc.cbStruct = sizeof(mxc); // ΢���ô˰취�жϰ汾

				// MIXERLINECONTROLS �ṹ���������
				ZeroMemory(&mxlc, sizeof(MIXERLINECONTROLS));

				mxlc.cbStruct = sizeof(mxlc); // ΢���ô˰취�жϰ汾

				mxlc.dwLineID = mxl.dwLineID; // ����ȡ�õ�������Ƶ�����·��ʶ
				// ��������Ϊ��������
				mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
				mxlc.cControls = 1;          // ʹ�� MIXERCONTROL �ṹ���������
				mxlc.pamxctrl = &mxc;        // MIXERCONTROL �ṹ�����ָ��
				mxlc.cbmxctrl = sizeof(mxc); // MIXERCONTROL �ṹ������ֽڴ�С

				// ȡ�ÿ�������Ϣ
				rc = mixerGetLineControls((HMIXEROBJ)hMixer,
					&mxlc,
					MIXER_GETLINECONTROLSF_ONEBYTYPE);

				// ȡ�ÿ�������Ϣ�ɹ��Ļ�����
				if (MMSYSERR_NOERROR == rc)
				{
					// ��ȡ�������е�ֵ��������Χ��mxc.Bounds.lMinimum��mxc.Bounds.lMaximum.
					MIXERCONTROLDETAILS        mxcd;      // ��������״̬��Ϣ
					MIXERCONTROLDETAILS_SIGNED volStruct; // �����ṹ�����(��һ����Ա��)

					// MIXERCONTROLDETAILS �ṹ���������
					ZeroMemory(&mxcd, sizeof(mxcd));

					mxcd.cbStruct = sizeof(mxcd);       // ΢���ô˰취�жϰ汾
					mxcd.dwControlID = mxc.dwControlID; // ����ȡ�õĿ�������ʶ
					mxcd.paDetails = &volStruct;        // �����ṹ�����ָ��
					mxcd.cbDetails = sizeof(volStruct); // �����ṹ������ֽڴ�С
					mxcd.cChannels = 1;                 // ȡ�û�����ȫ��ͨ��

					//  ��ȡ�������е�ֵ��������Χ��mxc.Bounds.lMinimum��mxc.Bounds.lMaximum. 

					// �������ֵ
					rc = mixerGetControlDetails((HMIXEROBJ)hMixer,
						&mxcd,
						MIXER_GETCONTROLDETAILSF_VALUE);

					if (MMSYSERR_NOERROR == rc)
					{
						long step = ( mxc.Bounds.lMaximum - mxc.Bounds.lMinimum ) /100;
						volStruct.lValue = volumnIndex*step;	 // ��Ҫ���õ�ֵ  
						rc = mixerSetControlDetails((HMIXEROBJ)hMixer,  
							&mxcd,  
							MIXER_SETCONTROLDETAILSF_VALUE);  

						if (MMSYSERR_NOERROR == rc)  
						{  
							return 1;
						}  
					}
				}
			}
		}
	}
	
	return -1;
}

int DeviceManager::GetAudioInputVolumn( const std::string &deviceName )
{
	OSVERSIONINFO osVer;
	osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx( &osVer );
	if( osVer.dwMajorVersion==6 )
	{
		//vista��win7����ȡ��ͬ�ķ�ʽ��ȡ����
		IMMDeviceEnumerator* pEnumerator;
		IMMDeviceCollection* pCollection = NULL;
		IMMDevice *pDevice = NULL;
		IPropertyStore *pProperties=NULL;  
		IAudioEndpointVolume *pVolumeAPI=NULL;  
		UINT deviceCount = 0;

		float fVolume = -1;

		CoInitializeEx( NULL , COINIT_MULTITHREADED );

		HRESULT hr=CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),(void**)&pEnumerator);
		if (hr != S_OK)  
		{  
			printf("CoCreateInstance Failed!\n");  
			return 0;  
		}  

		hr = pEnumerator->EnumAudioEndpoints( eCapture , DEVICE_STATE_ACTIVE , &pCollection );
		if (hr != S_OK)  
		{  
			printf("EnumAudioEndpoints Failed!\n");  
			goto releasepEnumerator;  
		}  

		hr = pCollection->GetCount(&deviceCount);  
		if (hr != S_OK)  
		{  
			printf("GetCount Failed!\n");  
			goto releasepCollection;  
		}  

		for (UINT dev=0;dev<deviceCount;dev++)  
		{  
			pDevice = NULL;  
			hr = pCollection->Item(dev,&pDevice);  
			if (hr == S_OK)  
			{  

				hr = pDevice->OpenPropertyStore(STGM_READ,&pProperties);  
				if (hr == S_OK)  
				{  
					PROPVARIANT varName;  
					PropVariantInit(&varName);  
					hr = pProperties->GetValue(PKEY_Device_FriendlyName, &varName); 
					if( SUCCEEDED(hr) )
					{
						hr=pDevice->Activate(__uuidof(IAudioEndpointVolume),CLSCTX_ALL,NULL,(void **)(&pVolumeAPI));  
						if (hr==S_OK)  
						{  
							hr = pVolumeAPI->GetMasterVolumeLevelScalar( &fVolume );
						} 
					}

					SAFE_RELEASE(pProperties);  

				}  

				SAFE_RELEASE(pDevice);  
			}  
		}  

releasepCollection:  

		SAFE_RELEASE(pCollection);  

releasepEnumerator:  

		SAFE_RELEASE(pEnumerator);  

		return  fVolume*100;
	}else
	{
		MMRESULT          rc;     // ��ý�庯�����ؽ������
		HMIXER            hMixer; // ������豸���
		MIXERLINE         mxl;    // ��Ƶ��·��׼״̬��Ϣ�ṹ��
		MIXERLINECONTROLS mxlc;   // ��Ƶ��·������������Ϣ�ṹ��
		MIXERCONTROL      mxc;    // ������Ƶ��·��������Ϣ�ṹ��

		//ö�������������������ö�ٺ���Ҫ���ܶ�������mixOpenʱֻ�����˵�һ���豸����
		//mixerOpen(&hMixer, 0 , 0 ,0 )��������������豸�µ���mixerGetLineInfo�������ˣ�
		//���ܻ��Ǹ����������йء�
		for( int deviceID =0 ; true ; deviceID++ )
		{
			// �򿪻�����豸
			rc = mixerOpen(&hMixer, // ���ص��豸���
				deviceID,       // ���������豸IDΪ��
				0,       // ��ʹ�ûص�����
				0,       // �ص���������
				MIXER_OBJECTF_MIXER);      // MIXER_OBJECTF_MIXER���ֵ����ʾ��ѡ��Ч�豸ID
			if( MMSYSERR_NOERROR!=rc )
			{
				//����ѭ��
				break;
			}
			// �򿪻�����豸�޴�Ļ�����

			// MIXERLINE �ṹ���������
			ZeroMemory(&mxl, sizeof(MIXERLINE));

			mxl.cbStruct = sizeof(MIXERLINE); // ΢���ô˰취�жϰ汾
			// ָ����Ҫ��ȡ��ͨ������������Ƶ�����MIXERLINE_COMPONENTTYPE_DST_SPEAKERS
			mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;

			// ȡ�û�����豸��ָ����·��Ϣ
			rc = mixerGetLineInfo((HMIXEROBJ)hMixer,
				&mxl,
				// ȡ��MIXERLINE::dwComponentTypeָ�����͵ĵ�һ����Ƶ��·��Ϣ
				MIXER_OBJECTF_HMIXER|MIXER_GETLINEINFOF_COMPONENTTYPE);
			if( rc!=MMSYSERR_NOERROR )
			{
				//�޷���ȡ����������·��������������
				continue;
			}

			// ȡ�û�����豸��ָ����·��Ϣ�ɹ��Ļ�����

			// ������������ 
			DWORD   dwConnections   =   mxl.cConnections; 
			// ׼����ȡ��˷��豸��ID 
			DWORD   dwLineID = -1; 
			for( DWORD i=0 ; i<dwConnections; i++ )
			{ 
				// ö��ÿһ���豸����Source��ID���ڵ�ǰ�ĵ������� 
				mxl.dwSource = i;
				// ����SourceID������ӵ���Ϣ 
				rc = mixerGetLineInfo(   (HMIXEROBJ)hMixer,   &mxl, 
					MIXER_OBJECTF_HMIXER   |   MIXER_GETLINEINFOF_SOURCE   ); 
				// �жϺ���ִ�д��� 
				if( MMSYSERR_NOERROR==rc )
				{ 
					// �����ǰ�豸��������˷磬������ѭ���� 
					if( mxl.dwComponentType==MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE )
					{ 
						dwLineID = mxl.dwLineID; 
						break; 
					}
				}
			}
			// ���û���ҵ�������ʧ�ܡ� 
			if( dwLineID==-1 )
			{ 
				//������������
				continue;
			}

			// MIXERCONTROL �ṹ���������
			ZeroMemory(&mxc, sizeof(MIXERCONTROL));

			mxc.cbStruct = sizeof(mxc); // ΢���ô˰취�жϰ汾

			// MIXERLINECONTROLS �ṹ���������
			ZeroMemory(&mxlc, sizeof(MIXERLINECONTROLS));

			mxlc.cbStruct = sizeof(mxlc); // ΢���ô˰취�жϰ汾

			mxlc.dwLineID = dwLineID; // ����ȡ�õ�������Ƶ������·��ʶ
			// ��������Ϊ��������
			mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
			mxlc.cControls = 1;          // ʹ�� MIXERCONTROL �ṹ���������
			mxlc.pamxctrl = &mxc;        // MIXERCONTROL �ṹ�����ָ��
			mxlc.cbmxctrl = sizeof(mxc); // MIXERCONTROL �ṹ������ֽڴ�С

			// ȡ�ÿ�������Ϣ
			rc = mixerGetLineControls((HMIXEROBJ)hMixer,
				&mxlc,
				MIXER_GETLINECONTROLSF_ONEBYTYPE);

			// ȡ�ÿ�������Ϣ�ɹ��Ļ�����
			if (MMSYSERR_NOERROR == rc)
			{
				// ��ȡ�������е�ֵ��������Χ��mxc.Bounds.lMinimum��mxc.Bounds.lMaximum.
				MIXERCONTROLDETAILS        mxcd;      // ��������״̬��Ϣ
				MIXERCONTROLDETAILS_SIGNED volStruct; // �����ṹ�����(��һ����Ա��)

				// MIXERCONTROLDETAILS �ṹ���������
				ZeroMemory(&mxcd, sizeof(mxcd));

				mxcd.cbStruct = sizeof(mxcd);       // ΢���ô˰취�жϰ汾
				mxcd.dwControlID = mxc.dwControlID; // ����ȡ�õĿ�������ʶ
				mxcd.paDetails = &volStruct;        // �����ṹ�����ָ��
				mxcd.cbDetails = sizeof(volStruct); // �����ṹ������ֽڴ�С
				mxcd.cChannels = 1;                 // ȡ�û�����ȫ��ͨ��

				//  ��ȡ�������е�ֵ��������Χ��mxc.Bounds.lMinimum��mxc.Bounds.lMaximum. 

				// �������ֵ
				rc = mixerGetControlDetails((HMIXEROBJ)hMixer,
					&mxcd,
					MIXER_GETCONTROLDETAILSF_VALUE);

				if (MMSYSERR_NOERROR == rc)
				{
					long step = ( mxc.Bounds.lMaximum - mxc.Bounds.lMinimum ) /100;

					int index = ( volStruct.lValue - mxc.Bounds.lMinimum) /step ;
					mixerClose( hMixer );
					return index;
				}
			}
		}

		if( hMixer )
			mixerClose( hMixer );
	}

	return -1;
}

int DeviceManager::SetAudioInputVolumn( const std::string &deviceName , int volumnIndex )
{
	OSVERSIONINFO osVer;
	osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx( &osVer );
	if( osVer.dwMajorVersion==6 )
	{
		//vista��win7����ȡ��ͬ�ķ�ʽ��ȡ����
		IMMDeviceEnumerator* pEnumerator;
		IMMDeviceCollection* pCollection = NULL;
		IMMDevice *pDevice = NULL;
		IPropertyStore *pProperties=NULL;  
		IAudioEndpointVolume *pVolumeAPI=NULL;  
		UINT deviceCount = 0;

		float fVolume = -1;

		CoInitializeEx( NULL , COINIT_MULTITHREADED );

		HRESULT hr=CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),(void**)&pEnumerator);
		if (hr != S_OK)  
		{  
			printf("CoCreateInstance Failed!\n");  
			return 0;  
		}  

		hr = pEnumerator->EnumAudioEndpoints(eCapture /*eRender*/, DEVICE_STATE_ACTIVE, &pCollection);
		//hr = pEnumerator->EnumAudioEndpoints( eRender , DEVICE_STATE_ACTIVE , &pCollection );
		if (hr != S_OK)  
		{  
			printf("EnumAudioEndpoints Failed!\n");  
			goto releasepEnumerator;  
		}  

		hr = pCollection->GetCount(&deviceCount);  
		if (hr != S_OK)  
		{  
			printf("GetCount Failed!\n");  
			goto releasepCollection;  
		}  

		for (UINT dev=0;dev<deviceCount;dev++)  
		{  
			pDevice = NULL;  
			hr = pCollection->Item(dev,&pDevice);  
			if (hr == S_OK)  
			{  

				hr = pDevice->OpenPropertyStore(STGM_READ,&pProperties);  
				if (hr == S_OK)  
				{  
					PROPVARIANT varName;  
					PropVariantInit(&varName);  
					hr = pProperties->GetValue(PKEY_Device_FriendlyName, &varName); 
					if( SUCCEEDED(hr) )
					{
						hr=pDevice->Activate(__uuidof(IAudioEndpointVolume),CLSCTX_ALL,NULL,(void **)(&pVolumeAPI));  
						if (hr==S_OK)  
						{  
							float volumnLevel = (float)volumnIndex/100;
							hr = pVolumeAPI->SetMasterVolumeLevelScalar( volumnLevel , NULL );
							UINT iChannelSum=0;   
							hr = pVolumeAPI->GetChannelCount(&iChannelSum);   

							if (SUCCEEDED(hr) &&(iChannelSum>=2))   
							{   
								for (int i=0;i<iChannelSum;i++)   
								{   
									pVolumeAPI->SetChannelVolumeLevelScalar(i,volumnLevel,NULL);   
								}   
							}   
						} 
					}

					SAFE_RELEASE(pProperties);  

				}  

				SAFE_RELEASE(pDevice);  
			}  
		}  

releasepCollection:  

		SAFE_RELEASE(pCollection);  

releasepEnumerator:  

		SAFE_RELEASE(pEnumerator);  

		return  volumnIndex;
	}else
	{
		MMRESULT          rc;     // ��ý�庯�����ؽ������
		HMIXER            hMixer; // ������豸���
		MIXERLINE         mxl;    // ��Ƶ��·��׼״̬��Ϣ�ṹ��
		MIXERLINECONTROLS mxlc;   // ��Ƶ��·������������Ϣ�ṹ��
		MIXERCONTROL      mxc;    // ������Ƶ��·��������Ϣ�ṹ��

		//ö�������������������ö�ٺ���Ҫ���ܶ�������mixOpenʱֻ�����˵�һ���豸����
		//mixerOpen(&hMixer, 0 , 0 ,0 )��������������豸�µ���mixerGetLineInfo�������ˣ�
		//���ܻ��Ǹ����������йء�
		for( int deviceID =0 ; true ; deviceID++ )
		{
			// �򿪻�����豸
			rc = mixerOpen(&hMixer, // ���ص��豸���
				deviceID,       // ���������豸IDΪ��
				0,       // ��ʹ�ûص�����
				0,       // �ص���������
				MIXER_OBJECTF_MIXER);      // MIXER_OBJECTF_MIXER���ֵ����ʾ��ѡ��Ч�豸ID
			if( MMSYSERR_NOERROR!=rc )
			{
				//����ѭ��
				break;
			}
			// �򿪻�����豸�޴�Ļ�����

			// MIXERLINE �ṹ���������
			ZeroMemory(&mxl, sizeof(MIXERLINE));

			mxl.cbStruct = sizeof(MIXERLINE); // ΢���ô˰취�жϰ汾
			// ָ����Ҫ��ȡ��ͨ������������Ƶ�����MIXERLINE_COMPONENTTYPE_DST_SPEAKERS
			mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;

			// ȡ�û�����豸��ָ����·��Ϣ
			rc = mixerGetLineInfo((HMIXEROBJ)hMixer,
				&mxl,
				// ȡ��MIXERLINE::dwComponentTypeָ�����͵ĵ�һ����Ƶ��·��Ϣ
				MIXER_OBJECTF_HMIXER|MIXER_GETLINEINFOF_COMPONENTTYPE);
			if( rc!=MMSYSERR_NOERROR )
			{
				//�޷���ȡ����������·��������������
				continue;
			}

			// ȡ�û�����豸��ָ����·��Ϣ�ɹ��Ļ�����

			// ������������ 
			DWORD   dwConnections   =   mxl.cConnections; 
			// ׼����ȡ��˷��豸��ID 
			DWORD   dwLineID = -1; 
			for( DWORD i=0 ; i<dwConnections; i++ )
			{ 
				// ö��ÿһ���豸����Source��ID���ڵ�ǰ�ĵ������� 
				mxl.dwSource = i;
				// ����SourceID������ӵ���Ϣ 
				rc = mixerGetLineInfo(   (HMIXEROBJ)hMixer,   &mxl, 
					MIXER_OBJECTF_HMIXER   |   MIXER_GETLINEINFOF_SOURCE   ); 
				// �жϺ���ִ�д��� 
				if( MMSYSERR_NOERROR==rc )
				{ 
					// �����ǰ�豸��������˷磬������ѭ���� 
					if( mxl.dwComponentType==MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE )
					{ 
						dwLineID = mxl.dwLineID; 
						break; 
					}
				}
			}
			// ���û���ҵ�������ʧ�ܡ� 
			if( dwLineID==-1 )
			{ 
				//������������
				continue;
			}

			// MIXERCONTROL �ṹ���������
			ZeroMemory(&mxc, sizeof(MIXERCONTROL));

			mxc.cbStruct = sizeof(mxc); // ΢���ô˰취�жϰ汾

			// MIXERLINECONTROLS �ṹ���������
			ZeroMemory(&mxlc, sizeof(MIXERLINECONTROLS));

			mxlc.cbStruct = sizeof(mxlc); // ΢���ô˰취�жϰ汾

			mxlc.dwLineID = dwLineID; // ����ȡ�õ�������Ƶ������·��ʶ
			// ��������Ϊ��������
			mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
			mxlc.cControls = 1;          // ʹ�� MIXERCONTROL �ṹ���������
			mxlc.pamxctrl = &mxc;        // MIXERCONTROL �ṹ�����ָ��
			mxlc.cbmxctrl = sizeof(mxc); // MIXERCONTROL �ṹ������ֽڴ�С

			// ȡ�ÿ�������Ϣ
			rc = mixerGetLineControls((HMIXEROBJ)hMixer,
				&mxlc,
				MIXER_GETLINECONTROLSF_ONEBYTYPE);

			// ȡ�ÿ�������Ϣ�ɹ��Ļ�����
			if (MMSYSERR_NOERROR == rc)
			{
				// ��ȡ�������е�ֵ��������Χ��mxc.Bounds.lMinimum��mxc.Bounds.lMaximum.
				MIXERCONTROLDETAILS        mxcd;      // ��������״̬��Ϣ
				MIXERCONTROLDETAILS_SIGNED volStruct; // �����ṹ�����(��һ����Ա��)

				// MIXERCONTROLDETAILS �ṹ���������
				ZeroMemory(&mxcd, sizeof(mxcd));

				mxcd.cbStruct = sizeof(mxcd);       // ΢���ô˰취�жϰ汾
				mxcd.dwControlID = mxc.dwControlID; // ����ȡ�õĿ�������ʶ
				mxcd.paDetails = &volStruct;        // �����ṹ�����ָ��
				mxcd.cbDetails = sizeof(volStruct); // �����ṹ������ֽڴ�С
				mxcd.cChannels = 1;                 // ȡ�û�����ȫ��ͨ��

				//  ��ȡ�������е�ֵ��������Χ��mxc.Bounds.lMinimum��mxc.Bounds.lMaximum. 

				// �������ֵ
				rc = mixerGetControlDetails((HMIXEROBJ)hMixer,
					&mxcd,
					MIXER_GETCONTROLDETAILSF_VALUE);

				if (MMSYSERR_NOERROR == rc)
				{
					long step = ( mxc.Bounds.lMaximum - mxc.Bounds.lMinimum ) /100;
					volStruct.lValue = volumnIndex*step;	 // ��Ҫ���õ�ֵ  
					rc = mixerSetControlDetails((HMIXEROBJ)hMixer,  
						&mxcd,  
						MIXER_SETCONTROLDETAILSF_VALUE);  

					if (MMSYSERR_NOERROR == rc)  
					{  
						mixerClose( hMixer );
						return 1;
					}  
				}
			}
		}

		if( hMixer )
			mixerClose( hMixer );
	}

	return -1;
}

void DeviceManager::GetNetworkAdaptorList( vector<string> *adaptorList )
{
	adaptorList->clear();

	ULONG ulAdapterInfoSize = sizeof(IP_ADAPTER_INFO);
	IP_ADAPTER_INFO *pAdapterInfo = (IP_ADAPTER_INFO*)new char[ulAdapterInfoSize];
	IP_ADAPTER_INFO *pAdapterInfoBkp = pAdapterInfo;
	if( GetAdaptersInfo(pAdapterInfo, &ulAdapterInfoSize) == ERROR_BUFFER_OVERFLOW ) // ������������
	{
		delete pAdapterInfo;
		pAdapterInfo = (IP_ADAPTER_INFO*)new char[ulAdapterInfoSize];
		pAdapterInfoBkp = pAdapterInfo;
	}
	if( GetAdaptersInfo(pAdapterInfo, &ulAdapterInfoSize) == ERROR_SUCCESS )
	{
		do{ // ��������������
			if(pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET)	// �ж��Ƿ�Ϊ��̫���ӿ�
			{
				adaptorList->push_back( pAdapterInfo->Description);
				// pAdapterInfo->Description ������������
				// pAdapterInfo->AdapterName ������������
			}
			pAdapterInfo = pAdapterInfo->Next;
		}while(pAdapterInfo);
	}
	if( pAdapterInfoBkp )
		delete pAdapterInfoBkp;
}

void DeviceManager::GetNetworkAdaptorInfo( const std::string& adaptorName , std::string* ip , 
										  std::string* submask , std::string* defaultGateway, std::string* dnsServer, bool* enableDHCP )
{

	ULONG ulAdapterInfoSize = sizeof(IP_ADAPTER_INFO);
	IP_ADAPTER_INFO *pAdapterInfo = (IP_ADAPTER_INFO*)new char[ulAdapterInfoSize];
	IP_ADAPTER_INFO *pAdapterInfoBkp = pAdapterInfo;
	if( GetAdaptersInfo(pAdapterInfo, &ulAdapterInfoSize) == ERROR_BUFFER_OVERFLOW ) // ������������
	{
		delete pAdapterInfo;
		pAdapterInfo = (IP_ADAPTER_INFO*)new char[ulAdapterInfoSize];
		pAdapterInfoBkp = pAdapterInfo;
	}
	if( GetAdaptersInfo(pAdapterInfo, &ulAdapterInfoSize) == ERROR_SUCCESS )
	{
		do{ // ��������������
			if(pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET)	// �ж��Ƿ�Ϊ��̫���ӿ�
			{
				if( adaptorName == pAdapterInfo->Description )
				{
					string adaptorKey = "System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\";
					adaptorKey = adaptorKey + pAdapterInfo->AdapterName;

					HKEY hKey;
					if( RegOpenKeyEx( HKEY_LOCAL_MACHINE , adaptorKey.c_str() , 0, KEY_READ , &hKey )!=ERROR_SUCCESS )
					{
						return;
					}

					DWORD dwIndex = 0;
					DWORD dwBufSize = 256;
					DWORD dwDataType=REG_SZ;
					DWORD dwDwordType = REG_DWORD;
					DWORD dwDwordValue;
					DWORD dwDwordSize = sizeof( dwDwordValue);
					char szSubKey[256];
					unsigned char szData[256] = {0};
					char keyValue[256] = {0};
					

					//if( RegQueryValueEx(hKey , "EnableDHCP",NULL, &dwDwordType, (LPBYTE)&dwDwordValue , &dwDwordSize )!=ERROR_SUCCESS )
					//{
					//	return;
					//}
					//if( dwDwordValue==1 )
					//	*enableDHCP = true;
					//else
					//	*enableDHCP = false;
					*enableDHCP = pAdapterInfo->DhcpEnabled;

					if( RegQueryValueEx( hKey , "DhcpNameServer" , NULL , &dwDataType , szData , &dwBufSize )!=ERROR_SUCCESS )
					{
						if( RegQueryValueEx( hKey , "NameServer",NULL, &dwDataType , szData, &dwBufSize)!=ERROR_SUCCESS  )
						{
							return ;
						}
					}

					
					memcpy( keyValue , szData , sizeof(szData ));
					(*dnsServer) = keyValue;

					if( RegQueryValueEx( hKey , "DhcpIPAddress" , NULL , &dwDataType , szData , &dwBufSize )!=ERROR_SUCCESS )
					{
						if( RegQueryValueEx( hKey , "IPAddress",NULL, &dwDataType , szData, &dwBufSize)!=ERROR_SUCCESS  )
						{
							return;
						}
					}
					memcpy( keyValue ,szData , sizeof(szData ) );
					(*ip) = keyValue;

					*defaultGateway = pAdapterInfo->GatewayList.IpAddress.String;

					*submask = pAdapterInfo->IpAddressList.IpMask.String;
					
					RegCloseKey( hKey );
					break;
				}
			}
			pAdapterInfo = pAdapterInfo->Next;
		}while(pAdapterInfo);
	}
	if( pAdapterInfoBkp )
		delete pAdapterInfoBkp;
}

int DeviceManager::GetNetworkAdaptorState( const std::string& mac , InterfaceStatus& status)
{
	DWORD dwSize;
	DWORD dwRetVal = -1;
	MIB_IFTABLE* pIfTable = NULL;
	MIB_IFROW*	 pIfRow = NULL;
	__try{
		pIfTable = (MIB_IFTABLE *)malloc(sizeof (MIB_IFTABLE));
		dwSize = sizeof (MIB_IFTABLE);

		// Make an initial call to GetIfTable to get the
		// necessary size into dwSize
		if (GetIfTable(pIfTable, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
			free(pIfTable);
			pIfTable = (MIB_IFTABLE *)malloc(dwSize);
			if (pIfTable == NULL) {
				printf("Error allocating memory needed to call GetIfTable\n");
				__leave;
			}
		}

		if( pIfTable!=NULL )
		{
			// Make a second call to GetIfTable to get the actual
			// data we want.
			if ((dwRetVal = GetIfTable(pIfTable, &dwSize, FALSE)) == NO_ERROR) {
				for (int i = 0; i < pIfTable->dwNumEntries; i++) {
					pIfRow = (MIB_IFROW *) & pIfTable->table[i];

					char phyAddr[60] = {0};	//�������ַת�����ַ���
					if( pIfRow->dwPhysAddrLen!=6 )
						continue;

					_snprintf( phyAddr, sizeof(phyAddr) , "%02x:%02x:%02x:%02x:%02x:%02x", 
						pIfRow->bPhysAddr[0], pIfRow->bPhysAddr[1],pIfRow->bPhysAddr[2],
						pIfRow->bPhysAddr[3], pIfRow->bPhysAddr[4],pIfRow->bPhysAddr[5] );

					if( stricmp( phyAddr , mac.c_str())==0 )
					{
						MIB_IFROW ifRow;
						ifRow.dwIndex = pIfRow->dwIndex;
						if( GetIfEntry( &ifRow )==NO_ERROR )
						{	
							switch( ifRow.dwOperStatus)
							{
							case IF_OPER_STATUS_OPERATIONAL:
								status = IF_STATUS_OPERATIONAL;
								break;
							case IF_OPER_STATUS_NON_OPERATIONAL:
								status = IF_STATUS_NON_OPERATIONAL;
								break;
							case IF_OPER_STATUS_UNREACHABLE:
								status = IF_STATUS_UNREACHABLE;
								break;
							case IF_OPER_STATUS_CONNECTED:
								status = IF_STATUS_CONNECTED;
								break;
							case IF_OPER_STATUS_CONNECTING:
								status = IF_STATUS_CONNECTING;
								break;
							case IF_OPER_STATUS_DISCONNECTED:
								status = IF_STATUS_DISCONNECTED;
								break;
							};	
							dwRetVal = 0;
						};

						__leave;
					}
				}
			}
		}


	}
	__finally{
		if( pIfTable )
			free( pIfTable );

		return dwRetVal;
	}
}

void DeviceManager::SetNetworkAdaptorInfo( const std::string& adaptorName ,std::string& ip , std::string& submask , std::string& defaultGateway, std::string& dnsServer, bool enableDHCP )
{
	ULONG ulAdapterInfoSize = sizeof(IP_ADAPTER_INFO);
	IP_ADAPTER_INFO *pAdapterInfo = (IP_ADAPTER_INFO*)new char[ulAdapterInfoSize];
	IP_ADAPTER_INFO *pAdapterInfoBkp = pAdapterInfo;
	if( GetAdaptersInfo(pAdapterInfo, &ulAdapterInfoSize) == ERROR_BUFFER_OVERFLOW ) // ������������
	{
		delete pAdapterInfo;
		pAdapterInfo = (IP_ADAPTER_INFO*)new char[ulAdapterInfoSize];
		pAdapterInfoBkp = pAdapterInfo;
	}
	if( GetAdaptersInfo(pAdapterInfo, &ulAdapterInfoSize) == ERROR_SUCCESS )
	{
		do{ // ��������������
			if(pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET)	// �ж��Ƿ�Ϊ��̫���ӿ�
			{
				if( adaptorName == pAdapterInfo->Description )
				{
					string adaptorKey = "System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\";
					adaptorKey = adaptorKey + pAdapterInfo->AdapterName;

					HKEY hKey;
					if( RegOpenKeyEx( HKEY_LOCAL_MACHINE , adaptorKey.c_str() , 0, KEY_WRITE , &hKey )!=ERROR_SUCCESS )
					{
						return;
					}

					char mszIPAddress[100] = {0};
					char mszNetMask[100] = {0};
					char mszNetGate[100] = {0};
					char mszNameServer[100] = {0};
					DWORD enableDHCPWord;

					
					strncpy( mszIPAddress , ip.c_str() , 98 );
					strncpy( mszNetGate , defaultGateway.c_str(), 98 );
					strncpy( mszNetMask , submask.c_str() , 98 );
					strncpy( mszNameServer , dnsServer.c_str() ,98);
					if( enableDHCP )
						enableDHCPWord = 1;
					else
						enableDHCPWord = 0;

					int nIP , nMask , nGate , nDns;

					nIP = strlen( mszIPAddress );
					nMask = strlen( mszNetMask );
					nGate = strlen( mszNetGate );
					nDns = strlen( mszNameServer);

					*(mszIPAddress +nIP +1 ) = 0x00;	//REG_MULTI_SZ������Ҫ�ں����ټӸ�0
					nIP += 2;

					*(mszNetMask + nMask +1 ) = 0x00;	
					nMask += 2;

					*(mszNetGate+nGate+1 ) = 0x00;
					nGate += 2;

					*(mszNameServer+nDns+1) = 0x00;
					nDns += 2;


					RegSetValueEx( hKey , "IPAddress" , NULL , REG_MULTI_SZ , (unsigned char*)mszIPAddress , nIP );
					RegSetValueEx( hKey , "SubnetMask" , NULL , REG_MULTI_SZ , (unsigned char*)mszNetMask , nMask );
					RegSetValueEx( hKey , "DefaultGateway" , NULL , REG_MULTI_SZ , (unsigned char*)mszNetGate , nGate );
					RegSetValueEx( hKey , "NameServer" , NULL , REG_SZ , (unsigned char*)mszNameServer , nDns );
					RegSetValueEx( hKey , "EnableDHCP" , NULL , REG_DWORD ,(const BYTE*)(&enableDHCPWord) , sizeof(DWORD) );


					RegCloseKey( hKey );

					NotifyIPChanged(pAdapterInfo->AdapterName , 0 , ip.c_str() , submask.c_str() , enableDHCP );
					break;
				}
			}
			pAdapterInfo = pAdapterInfo->Next;
		}while(pAdapterInfo);
	}
	if( pAdapterInfoBkp )
		delete pAdapterInfoBkp;
}

void DeviceManager::GetDisplayDevices( DisplayDeviceList* pDeviceList )
{
	HDC hScrDC = NULL;
	DISPLAY_DEVICE device;
	device.cb = sizeof( DISPLAY_DEVICE );
	DWORD deviceNum = 0;

	pDeviceList->clear();
	while( EnumDisplayDevicesA( NULL ,deviceNum++, &device, NULL ) )
	{
		
		hScrDC = CreateDC( device.DeviceName , NULL , NULL , NULL );
		// ����豸��Ϣ
		DisplayDevice displayDevice;
		displayDevice._deviceName = device.DeviceName;
		char friendlyName[36] = {0};
		_snprintf( friendlyName,36 , "��ʾ��%d" , deviceNum );
		displayDevice._friendlyName = friendlyName;
		displayDevice._xResolution = GetDeviceCaps(hScrDC, HORZRES);
		displayDevice._yResolution = GetDeviceCaps(hScrDC, VERTRES);
		if( displayDevice._xResolution==0 || displayDevice._yResolution==0 )
			continue;

		//ö����ʾ�����õ����ò���
		DEVMODE deviceMode;
		ZeroMemory( &deviceMode , sizeof(DEVMODE) );
		deviceMode.dmSize = sizeof(DEVMODE);
		DWORD modeNum = 0;
		DWORD bitsPerPel = 0;

		//��ȡ��ǰ����ʾ��������Ϣ����Ҫ����ɫλ��
		if( EnumDisplaySettings(device.DeviceName, ENUM_CURRENT_SETTINGS,&deviceMode) )
		{
			bitsPerPel = deviceMode.dmBitsPerPel;
		}
		
		//��ʼö��
		char resolution[36]={0};
		while( EnumDisplaySettings( device.DeviceName , modeNum++, &deviceMode ) )
		{
			DWORD height = deviceMode.dmPelsHeight;
			DWORD width = deviceMode.dmPelsWidth;
			
			if( width>800 && height>600&& bitsPerPel==deviceMode.dmBitsPerPel )
			{
				if( height==displayDevice._yResolution && width==displayDevice._xResolution )
					_snprintf( resolution , 36, "%d x %d ����ǰ���ã�" , width , height );
				else
					_snprintf( resolution , 36, "%d x %d" , width , height );
				vector<string>::iterator it = std::find( displayDevice._resolutionList.begin(), displayDevice._resolutionList.end(),
					resolution );
				if( it==displayDevice._resolutionList.end() )
				{
					displayDevice._resolutionList.push_back(  resolution );
				}
			}
		}
		//����һ�����Ƽ�����
		int lastIndex = displayDevice._resolutionList.size()-1;
		displayDevice._resolutionList[lastIndex] = displayDevice._resolutionList[lastIndex] + "���Ƽ���";

		pDeviceList->push_back( displayDevice );

	}
	if( hScrDC )
	{
		DeleteDC(hScrDC);
	}
}

int DeviceManager::SetDisplayDevice( const DisplayDevice& newDevice, long& code )
{
	DEVMODE deviceMode;
	ZeroMemory( &deviceMode , sizeof(DEVMODE) );
	deviceMode.dmSize = sizeof(DEVMODE);
	if( EnumDisplaySettings( newDevice._deviceName.c_str() , ENUM_CURRENT_SETTINGS, &deviceMode ) )
	{
		deviceMode.dmPelsHeight = newDevice._yResolution;
		deviceMode.dmPelsWidth = newDevice._xResolution;

		code = ChangeDisplaySettings(&deviceMode,CDS_UPDATEREGISTRY);   
		if (code==DISP_CHANGE_SUCCESSFUL)   
		{    
			return 1;
		}   
		else  
		{   
			return -1;
		}  
	}

	return 0;
}

int DeviceManager::GetAudioOutputMute( const std::string &deviceName )
{
    OSVERSIONINFO osVer;
	osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx( &osVer );
	if( osVer.dwMajorVersion==6 )
	{
		//vista��win7����ȡ��ͬ�ķ�ʽ��ȡ����
		IMMDeviceEnumerator* pEnumerator;
		IMMDeviceCollection* pCollection = NULL;
		IMMDevice *pDevice = NULL;
		IPropertyStore *pProperties=NULL;  
		IAudioEndpointVolume *pVolumeAPI=NULL;  
		UINT deviceCount = 0;

		BOOL bMute = FALSE;
        BOOL bFindDevice = FALSE;

		CoInitializeEx( NULL , COINIT_MULTITHREADED );

		HRESULT hr=CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),(void**)&pEnumerator);
		if (hr != S_OK)  
		{  
			printf("CoCreateInstance Failed!\n");  
			return 0;  
		}  

		//hr = pEnumerator->EnumAudioEndpoints(eCapture /*eRender*/, DEVICE_STATE_ACTIVE, &pCollection);
		hr = pEnumerator->EnumAudioEndpoints( eRender , DEVICE_STATE_ACTIVE , &pCollection );
		if (hr != S_OK)  
		{  
			printf("EnumAudioEndpoints Failed!\n");  
			goto releasepEnumerator;  
		}  

		hr = pCollection->GetCount(&deviceCount);  
		if (hr != S_OK)  
		{  
			printf("GetCount Failed!\n");  
			goto releasepCollection;  
		}  

		for (UINT dev=0;dev<deviceCount;dev++)  
		{  
			pDevice = NULL;  
			hr = pCollection->Item(dev,&pDevice);  
			if (hr == S_OK)  
			{  

				hr = pDevice->OpenPropertyStore(STGM_READ,&pProperties);  
				if (hr == S_OK)  
				{  
					PROPVARIANT varName;  
					PropVariantInit(&varName);  
					hr = pProperties->GetValue(PKEY_Device_FriendlyName, &varName); 
					if( SUCCEEDED(hr) )
					{
                        if (varName.vt != VT_LPWSTR || 
                            (wstr2str(varName.pwszVal).find(deviceName) != 0)) //�����ֵ���ܲ���������Ҫǰ����ƥ��
                        {
                          continue;
                        }
						hr=pDevice->Activate(__uuidof(IAudioEndpointVolume),CLSCTX_ALL,NULL,(void **)(&pVolumeAPI));  
						if (hr==S_OK)  
						{  
							hr = pVolumeAPI->GetMute(&bMute);
                            bFindDevice = TRUE;
						} 
					}

					SAFE_RELEASE(pProperties);  
				}  

				SAFE_RELEASE(pDevice);  
			}
            if (bFindDevice) break;
		}  

releasepCollection:  

		SAFE_RELEASE(pCollection);  

releasepEnumerator:  

		SAFE_RELEASE(pEnumerator);  

		return  hr == S_OK ? bMute : -1;
	}else
	{
		MMRESULT          rc;     // ��ý�庯�����ؽ������
		HMIXER            hMixer; // ������豸���
		MIXERLINE         mxl;    // ��Ƶ��·��׼״̬��Ϣ�ṹ��
		MIXERLINECONTROLS mxlc;   // ��Ƶ��·������������Ϣ�ṹ��
		MIXERCONTROL      mxc;    // ������Ƶ��·��������Ϣ�ṹ��


		// �򿪻�����豸
		rc = mixerOpen(&hMixer, // ���ص��豸���
			0,       // ���������豸IDΪ��
			0,       // ��ʹ�ûص�����
			0,       // �ص���������
			0);      // MIXER_OBJECTF_MIXER���ֵ����ʾ��ѡ��Ч�豸ID

		// �򿪻�����豸�޴�Ļ�����
		if (MMSYSERR_NOERROR == rc)
		{
			// MIXERLINE �ṹ���������
			ZeroMemory(&mxl, sizeof(MIXERLINE));

			mxl.cbStruct = sizeof(MIXERLINE); // ΢���ô˰취�жϰ汾
			// ָ����Ҫ��ȡ��ͨ������������Ƶ�����MIXERLINE_COMPONENTTYPE_DST_SPEAKERS
			mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

			// ȡ�û�����豸��ָ����·��Ϣ
			rc = mixerGetLineInfo((HMIXEROBJ)hMixer,
				&mxl,
				// ȡ��MIXERLINE::dwComponentTypeָ�����͵ĵ�һ����Ƶ��·��Ϣ
				MIXER_GETLINEINFOF_COMPONENTTYPE);

			// ȡ�û�����豸��ָ����·��Ϣ�ɹ��Ļ�����
			if (MMSYSERR_NOERROR == rc)
			{
				// MIXERCONTROL �ṹ���������
				ZeroMemory(&mxc, sizeof(MIXERCONTROL));

				mxc.cbStruct = sizeof(mxc); // ΢���ô˰취�жϰ汾

				// MIXERLINECONTROLS �ṹ���������
				ZeroMemory(&mxlc, sizeof(MIXERLINECONTROLS));

				mxlc.cbStruct = sizeof(mxlc); // ΢���ô˰취�жϰ汾

				mxlc.dwLineID = mxl.dwLineID; // ����ȡ�õ�������Ƶ�����·��ʶ
				// ��������Ϊ���ƾ���
				mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
				mxlc.cControls = 1;          // ʹ�� MIXERCONTROL �ṹ���������
				mxlc.pamxctrl = &mxc;        // MIXERCONTROL �ṹ�����ָ��
				mxlc.cbmxctrl = sizeof(mxc); // MIXERCONTROL �ṹ������ֽڴ�С

				// ȡ�ÿ�������Ϣ
				rc = mixerGetLineControls((HMIXEROBJ)hMixer,
					&mxlc,
					MIXER_GETLINECONTROLSF_ONEBYTYPE);

				// ȡ�ÿ�������Ϣ�ɹ��Ļ�����
				if (MMSYSERR_NOERROR == rc)
				{
					// ��ȡ�������е�ֵ��������Χ��mxc.Bounds.lMinimum��mxc.Bounds.lMaximum.
					MIXERCONTROLDETAILS        mxcd;      // ��������״̬��Ϣ
                    MIXERCONTROLDETAILS_BOOLEAN mcdMute;

					// MIXERCONTROLDETAILS �ṹ���������
					ZeroMemory(&mxcd, sizeof(mxcd));

					mxcd.cbStruct = sizeof(mxcd);       // ΢���ô˰취�жϰ汾
					mxcd.dwControlID = mxc.dwControlID; // ����ȡ�õĿ�������ʶ
					mxcd.paDetails = &mcdMute;        // �����ṹ�����ָ��
					mxcd.cbDetails = sizeof(mcdMute); // �����ṹ������ֽڴ�С
					mxcd.cChannels = 1;                 // ȡ�û�����ȫ��ͨ��

					// ��þ���ֵ
					rc = mixerGetControlDetails((HMIXEROBJ)hMixer,
						&mxcd,
						MIXER_GETCONTROLDETAILSF_VALUE);

					if (MMSYSERR_NOERROR == rc)
					{
                        BOOL bMute = mcdMute.fValue;
                        return bMute;
					}
				}
			}
		}
	}

	return -1;
}

int DeviceManager::SetAudioOutputMute( bool isMute )
{
    OSVERSIONINFO osVer;
	osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx( &osVer );
	if( osVer.dwMajorVersion==6 )
	{
		//vista��win7����ȡ��ͬ�ķ�ʽ��ȡ����
		IMMDeviceEnumerator* pEnumerator;
		IMMDeviceCollection* pCollection = NULL;
		IMMDevice *pDevice = NULL;
		IPropertyStore *pProperties=NULL;  
		IAudioEndpointVolume *pVolumeAPI=NULL;  
		UINT deviceCount = 0;

		CoInitializeEx( NULL , COINIT_MULTITHREADED );

		HRESULT hr=CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),(void**)&pEnumerator);
		if (hr != S_OK)  
		{  
			printf("CoCreateInstance Failed!\n");  
			return 0;  
		}  

		//hr = pEnumerator->EnumAudioEndpoints(eCapture /*eRender*/, DEVICE_STATE_ACTIVE, &pCollection);
		hr = pEnumerator->EnumAudioEndpoints( eRender , DEVICE_STATE_ACTIVE , &pCollection );
		if (hr != S_OK)  
		{  
			printf("EnumAudioEndpoints Failed!\n");  
			goto releasepEnumerator;  
		}  

		hr = pCollection->GetCount(&deviceCount);  
		if (hr != S_OK)  
		{  
			printf("GetCount Failed!\n");  
			goto releasepCollection;  
		}  

		for (UINT dev=0;dev<deviceCount;dev++)  
		{  
			pDevice = NULL;  
			hr = pCollection->Item(dev,&pDevice);  
			if (hr == S_OK)  
			{  

				hr = pDevice->OpenPropertyStore(STGM_READ,&pProperties);  
				if (hr == S_OK)  
				{  
					PROPVARIANT varName;  
					PropVariantInit(&varName);  
					hr = pProperties->GetValue(PKEY_Device_FriendlyName, &varName); 
					if( SUCCEEDED(hr) )
					{
						hr=pDevice->Activate(__uuidof(IAudioEndpointVolume),CLSCTX_ALL,NULL,(void **)(&pVolumeAPI));  
						if (hr==S_OK)  
						{  
							hr = pVolumeAPI->SetMute(isMute ? TRUE : FALSE, NULL);
						} 
					}
					SAFE_RELEASE(pProperties);  

				}  

				SAFE_RELEASE(pDevice);  
			}  
		}  

releasepCollection:  

		SAFE_RELEASE(pCollection);  

releasepEnumerator:  

		SAFE_RELEASE(pEnumerator);  

		return  hr == S_OK ? 0 : -1;
	}else
	{
		MMRESULT          rc;     // ��ý�庯�����ؽ������
		HMIXER            hMixer; // ������豸���
		MIXERLINE         mxl;    // ��Ƶ��·��׼״̬��Ϣ�ṹ��
		MIXERLINECONTROLS mxlc;   // ��Ƶ��·������������Ϣ�ṹ��
		MIXERCONTROL      mxc;    // ������Ƶ��·��������Ϣ�ṹ��

		// �򿪻�����豸
		rc = mixerOpen(&hMixer, // ���ص��豸���
			0,       // ���������豸IDΪ��
			0,       // ��ʹ�ûص�����
			0,       // �ص���������
			0);      // MIXER_OBJECTF_MIXER���ֵ����ʾ��ѡ��Ч�豸ID

		// �򿪻�����豸�޴�Ļ�����
		if (MMSYSERR_NOERROR == rc)
		{
			// MIXERLINE �ṹ���������
			ZeroMemory(&mxl, sizeof(MIXERLINE));

			mxl.cbStruct = sizeof(MIXERLINE); // ΢���ô˰취�жϰ汾
			// ָ����Ҫ��ȡ��ͨ������������Ƶ�����MIXERLINE_COMPONENTTYPE_DST_SPEAKERS
			mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

			// ȡ�û�����豸��ָ����·��Ϣ
			rc = mixerGetLineInfo((HMIXEROBJ)hMixer,
				&mxl,
				// ȡ��MIXERLINE::dwComponentTypeָ�����͵ĵ�һ����Ƶ��·��Ϣ
				MIXER_GETLINEINFOF_COMPONENTTYPE);

			// ȡ�û�����豸��ָ����·��Ϣ�ɹ��Ļ�����
			if (MMSYSERR_NOERROR == rc)
			{
				// MIXERCONTROL �ṹ���������
				ZeroMemory(&mxc, sizeof(MIXERCONTROL));

				mxc.cbStruct = sizeof(mxc); // ΢���ô˰취�жϰ汾

				// MIXERLINECONTROLS �ṹ���������
				ZeroMemory(&mxlc, sizeof(MIXERLINECONTROLS));

				mxlc.cbStruct = sizeof(mxlc); // ΢���ô˰취�жϰ汾

				mxlc.dwLineID = mxl.dwLineID; // ����ȡ�õ�������Ƶ�����·��ʶ
				// ��������Ϊ���ƾ���
				mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
				mxlc.cControls = 1;          // ʹ�� MIXERCONTROL �ṹ���������
				mxlc.pamxctrl = &mxc;        // MIXERCONTROL �ṹ�����ָ��
				mxlc.cbmxctrl = sizeof(mxc); // MIXERCONTROL �ṹ������ֽڴ�С

				// ȡ�ÿ�������Ϣ
				rc = mixerGetLineControls((HMIXEROBJ)hMixer,
					&mxlc,
					MIXER_GETLINECONTROLSF_ONEBYTYPE);

				// ȡ�ÿ�������Ϣ�ɹ��Ļ�����
				if (MMSYSERR_NOERROR == rc)
				{
					// ��ȡ�������е�ֵ��������Χ��mxc.Bounds.lMinimum��mxc.Bounds.lMaximum.
					MIXERCONTROLDETAILS        mxcd;      // ��������״̬��Ϣ
                    MIXERCONTROLDETAILS_BOOLEAN mcdMute;

					// MIXERCONTROLDETAILS �ṹ���������
					ZeroMemory(&mxcd, sizeof(mxcd));

					mxcd.cbStruct = sizeof(mxcd);       // ΢���ô˰취�жϰ汾
					mxcd.dwControlID = mxc.dwControlID; // ����ȡ�õĿ�������ʶ
					mxcd.paDetails = &mcdMute;        // �����ṹ�����ָ��
					mxcd.cbDetails = sizeof(mcdMute); // �����ṹ������ֽڴ�С
					mxcd.cChannels = 1;                 // ȡ�û�����ȫ��ͨ��

					// ��þ���ֵ
					rc = mixerGetControlDetails((HMIXEROBJ)hMixer,
						&mxcd,
						MIXER_GETCONTROLDETAILSF_VALUE);

					if (MMSYSERR_NOERROR == rc)
					{
						mcdMute.fValue = isMute ? TRUE : FALSE;	 // ��Ҫ���õ�ֵ  
						rc = mixerSetControlDetails((HMIXEROBJ)hMixer,  
							&mxcd,  
							MIXER_SETCONTROLDETAILSF_VALUE);  

						if (MMSYSERR_NOERROR == rc)  
						{  
							return 0;
						}  
					}
				}
			}
		}
	}
	
	return -1;
}

