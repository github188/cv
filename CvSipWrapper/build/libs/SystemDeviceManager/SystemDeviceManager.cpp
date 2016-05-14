// AudioDevice.cpp : ���� DLL Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "SystemDeviceManager.h"
#include <string>
#include <tchar.h>
#include <dshow.h>
#include <atlbase.h>
#include <qedit.h>
#include <comutil.h> //for _bstr_t

using namespace std;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

//���ߺ���----------------------------------------------------zhenHua.sun 2010-07-19
std::wstring str2wstr(const std::string& str)
{
	std::string curLocale = setlocale(LC_ALL, NULL); 
	setlocale(LC_ALL, "chs"); 

	wchar_t *buff = (wchar_t*) malloc(sizeof(wchar_t) * (str.size() + 1));
	mbstowcs(buff, str.c_str(), str.size() + 1);
	std::wstring wstr(buff);
	free(buff);

	setlocale(LC_ALL, curLocale.c_str());
	return wstr;
}

std::string wstr2str( const std::wstring& wstr )
{
	std::string curLocale = setlocale(LC_ALL, NULL); 
	setlocale(LC_ALL, "chs"); 

	char *buff = (char*) malloc( sizeof(char)*(wstr.size()+1) );
	wcstombs( buff , wstr.c_str() , wstr.size()+1 );
	std::string str( buff );
	free( buff );

	setlocale(LC_ALL, curLocale.c_str());
	return str;
}


/**
 * @brief ö��ָ��ClassID�µ��豸�Ѻ����ƺ��豸����
 * @param clsidDeviceClass Ҫö�ٵ��豸ClassID
 * @param pWStringArray �洢�豸���Ƶ�����
 * @NumOfDevices �������������ÿ���豸�������������ƣ���˴�С���豸��������2
 */
HRESULT  ListCaptureDevice(const CLSID & clsidDeviceClass,std::wstring * pWStringArray,int & NumOfDevices)
{
	//�豸ö����Interface
	ICreateDevEnum *pDevEnum = NULL; 
	
	//����ö��Interface
	IEnumMoniker *pEnum = NULL;

	// Create the System Device Enumerator.
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, 
		reinterpret_cast<void**>(&pDevEnum)); //�����豸ö��COM����
	if( FAILED( hr ) )
	{
		return E_FAIL;
	}
	
	// Create an enumerator for the video capture category.
	hr = pDevEnum->CreateClassEnumerator(
			clsidDeviceClass,	//CLSID_VideoInputDeviceCategory or CLSID_AudioInputDeviceCategory
			&pEnum, 0); //������Ƶ�ɼ��豸ö��COM����
	if( pEnum==NULL )
	{
		pDevEnum->Release();
		return E_FAIL;
	}
	
	NumOfDevices=0;
	IMoniker *pMoniker = NULL;
	while (pEnum->Next(1, &pMoniker, NULL) == S_OK) //����ö�٣�ֱ��Ϊ��
	{
		IPropertyBag *pPropBag;
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
			(void**)(&pPropBag));
		if (FAILED(hr))
		{
			pMoniker->Release();
			continue; // Skip this one, maybe the next one will work.
		} 
		// Find the description or friendly name.
		VARIANT varName;
		VariantInit(&varName);
		
		hr = pPropBag->Read(L"Description", &varName, 0);
		if (FAILED(hr))
		{
			hr = pPropBag->Read(L"FriendlyName", &varName, 0); //�豸�Ѻ�����
		}
		if (SUCCEEDED(hr))
		{
			// Add it to the application's list box.
		//	char displayName[1024];
		//	WideCharToMultiByte(CP_ACP,0,varName.bstrVal,-1,displayName,1024,"",NULL);
			
			pWStringArray[NumOfDevices++]=varName.bstrVal;//varName.bstrVal;
			VariantClear(&varName); 

			WCHAR * wszDiaplayName=NULL;
			pMoniker->GetDisplayName(NULL,NULL,&wszDiaplayName);
			pWStringArray[NumOfDevices++]=wszDiaplayName;
			CoTaskMemFree(wszDiaplayName);
		//	SysFreeString(varName.bstrVal); 
		}

		pPropBag->Release();
		pMoniker->Release();
	}

	pEnum->Release();
	pDevEnum->Release();
	return S_OK;
}

void SystemDeviceManager::getAudioInputDeviceList( vector<string>* deviceList )
{
	int numOfDevice = 0;
	std::wstring deviceCaptureList[20];
	HRESULT result = ListCaptureDevice( CLSID_AudioInputDeviceCategory, deviceCaptureList , numOfDevice );
	if( FAILED( result ) )
		return;

	for( int deviceCount=0 ; deviceCount<numOfDevice ; deviceCount+=2 )
	{
		string friendlyName = wstr2str( deviceCaptureList[deviceCount] );
		deviceList->push_back( friendlyName );//friendly name
		//deviceCaptureList[deviceCount+1] is device name
	}
}

void SystemDeviceManager::getAudioOuputDeviceList( vector<string>* deviceList )
{
	int numOfDevice = 0;
	std::wstring deviceCaptureList[20];
	HRESULT result = ListCaptureDevice( CLSID_AudioRendererCategory, deviceCaptureList , numOfDevice );
	if( FAILED( result ) )
		return ;

	for( int deviceCount=0 ; deviceCount<numOfDevice ; deviceCount+=2 )
	{
		string friendlyName = wstr2str( deviceCaptureList[deviceCount] );
		deviceList->push_back( friendlyName );//friendly name
		//deviceCaptureList[deviceCount+1] is device name
	}
}