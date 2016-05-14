// Transform.h : Transform DLL ����ͷ�ļ�
//

#pragma once
//#include "stdafx.h"
//#ifndef __AFXWIN_H__
//#error include 'stdafx.h' before including this file for PCH
//#endif

#include "resource.h"		// ������
#include <streams.h>
#include <stdio.h>
#include <time.h>


#include <windows.h>
#include <initguid.h>
#include <olectl.h>

#if (1100 > _MSC_VER)
#include <olectlid.h>
#endif


#pragma warning(disable:4238)  // nonstd extension used: class rvalue used as lvalue


// {E2C7D3A5-819A-4881-87BA-F1C93319D18A}
//DEFINE_GUID(CLSID_SCUTX264Encoder, 
//			0xe2c7d3a5, 0x819a, 0x4881, 0x87, 0xba, 0xf1, 0xc9, 0x33, 0x19, 0xd1, 0x8a);

// {828ED7A4-0769-4751-8016-D9E42A65FA7D}
DEFINE_GUID(CLSID_RGB24TOYUV2, 
0x828ed7a4, 0x769, 0x4751, 0x80, 0x16, 0xd9, 0xe4, 0x2a, 0x65, 0xfa, 0x7d);


/****************************************************************************************
���ܣ��������ӿڵ�ý������
���ߣ��¿���  2009-12-08
*****************************************************************************************
*/
//#ifndef OUR_GUID_ENTRY
//    #define OUR_GUID_ENTRY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
//    DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8);
//#endif
//HDYC{43594448-0000-0010-8000-00AA00389B71}///�¿��� add 09-12-08
//OUR_GUID_ENTRY(MEDIASUBTYPE_HDYC,
//0x43594448, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);


//����һЩ��ɫ�ռ�ת����ص�    �¿��� 2009-12-13
#define SCALEBITS 8
#define ONE_HALF  (1 << (SCALEBITS - 1))
#define FIX(x)    ((int) ((x) * (1L<<SCALEBITS) + 0.5))
typedef unsigned char uint8_t;



/****************************************************************************************/

class CTransform : public CTransformFilter
{

public:

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

	// Reveals IContrast
	//STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	DECLARE_IUNKNOWN;

	HRESULT Transform(IMediaSample *pIn, IMediaSample *pOut);
	HRESULT CheckInputType(const CMediaType *mtIn);
	HRESULT CheckTransform(const CMediaType *mtIn,const CMediaType *mtOut);
	HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
	HRESULT DecideBufferSize(IMemAllocator *pAlloc,
		ALLOCATOR_PROPERTIES *pProperties);

	BOOL DoColorChange(BYTE* pYUV, BYTE* pOUT, LONG lWidth, LONG lHeight);

    ~CTransform();
	//end

	//����receive���������ڱ���˵Ŀ���
	HRESULT Receive( IMediaSample *pSample );

		// --- ISpecifyPropertyPages ---
	//STDMETHODIMP GetPages(CAUUID *pPages);

	// Basic COM - used here to reveal our own interfaces
	//STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);


private:
	SwsContext		*pYuyToBgraCtx;

	// Constructor
	CTransform(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);	
	
	bool init;
	int  width;
	int  height;

	BYTE * pBufferDest;

	//�¿���add 09-09-03
	BYTE* YUV2_Value;

	//������������ͷ��ɫ�ʿռ����� �¿��� 2009-12-11
	enum sub_mediatype
	{
		YUY2=0,
		HDYC=1,
		RGB24=2
	};
	sub_mediatype sub_cam;


    //�������һfilter���ݵ��ڴ�ռ䣬�ڹ��캯���з��䣬�������������ͷ� --�¿��� 09-09-11 add
	BYTE * data_result;
		
	//dsh ����2010-11-22��Ϊ�˼����ӳ٣���new�Ĵ�������
	//BYTE *m_pRGBInverse;//����Ƶ�ɼ�ΪRGB24ʱ��ͼ��Ϊ�������Ҫ����ת�����ڴ洢��ת�������


}; // CContrast

