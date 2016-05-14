
#include <streams.h>
#include <dsound.h>
#include "IAECProp.h"
#include <string>

//�㹻16k��Ƶ��1s��16k * 16bits / (8bits/B) = 32kB
#define MAX_RAWBUFFER_SIZE 32000

struct RawBufferStruct
{
	char RawBuffer[MAX_RAWBUFFER_SIZE];//��ȡ�������õ�������
	int HeadPos;// ���ݴ洢��ͷ����
	int EndPos;//���ݴ洢��β����  
	int nRawBfferSize;//�������õ����ݴ����ݴ�С

	RawBufferStruct()
	{
		HeadPos=0;
		EndPos=0;
		nRawBfferSize=0;
		memset(&RawBuffer, 0, MAX_RAWBUFFER_SIZE);
	}
};

class CStaticMediaBuffer : public CBaseMediaBuffer {
public:
	/*STDMETHODIMP_(ULONG) AddRef() {return 2;}
	STDMETHODIMP_(ULONG) Release() {return 1;}*/
    void Init(BYTE *pData, ULONG ulSize, ULONG ulData) {
        m_pData = pData;
        m_ulSize = ulSize;
        m_ulData = ulData;
    }
};

class CAudioAECFilterPin;


class CAudioAECFilter : public CSource
{
public:
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

private:
    CAudioAECFilter(LPUNKNOWN lpunk, HRESULT *phr);
	~CAudioAECFilter();

}; 


class CAudioAECFilterPin : public CSourceStream,IAMStreamConfig,IAMBufferNegotiation,IAECProperty
{

public:

    CAudioAECFilterPin(HRESULT *phr, CAudioAECFilter *pParent, LPCWSTR pPinName);
    ~CAudioAECFilterPin();

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
    DECLARE_IUNKNOWN;

    HRESULT FillBuffer(IMediaSample *pms);


    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);

    HRESULT SetMediaType(const CMediaType *pMediaType);

    HRESULT CheckMediaType(const CMediaType *pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType *pmt);

    HRESULT OnThreadCreate(void);
	HRESULT OnThreadDestroy(void);

    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

	//IAMBufferNegotiation
	STDMETHODIMP SuggestAllocatorProperties(const ALLOCATOR_PROPERTIES *pprop);
    STDMETHODIMP GetAllocatorProperties(__out  ALLOCATOR_PROPERTIES *pprop);

	//IAMStreamConfig
	STDMETHODIMP SetFormat( AM_MEDIA_TYPE *pmt);   
	STDMETHODIMP GetFormat(__out  AM_MEDIA_TYPE **ppmt);
	STDMETHODIMP GetNumberOfCapabilities(__out  int *piCount,__out  int *piSize);
	STDMETHODIMP GetStreamCaps(int iIndex, __out  AM_MEDIA_TYPE **ppmt,__out  BYTE *pSCC);

	//IAECProperty
	STDMETHODIMP put_CapDevFriendName(wchar_t* capDevFriendName,int capNameSize,wchar_t* rendDevFriendName,int rendNameSize);
	STDMETHODIMP put_PreviewHandle(HWND hPreviewHandle);

private:

	HRESULT CreateBasicBuffer(LPDIRECTSOUND8 lpDirectSound, LPDIRECTSOUNDBUFFER8* ppDsb8);

private:

	//dsh
	std::wstring m_szCaptureDeviceName;//��׽�豸����,����Friendly Name
	std::wstring m_szRenderDeviceName;//��׽�豸����
	int m_mode;//ģʽѡ���Ƿ������
	int m_feat;//�Ƿ��޸�����������Ϊonʱ�����漸�������òŻ���Ч
	int m_ns;//�Ƿ�ȥ��
	int m_agc;//�Ƿ�����
	int m_cntrclip;//�Ƿ�����в�

	IMediaObject* pDMO;//��������DMO
	IPropertyStore* pPS;//��������DMO��������

	RawBufferStruct  m_RawBuffer;

	

	int m_iIntervalTime;//ȡһ֡������ʱ�䣬��DecideBufferSizeҲ�й�ϵ

	/*FILE * pfMicOutPCM;  // dump output signal using PCM format
	FILE * pfMicSize;

	FILE *pfDeliverPCM;*/
	


    CCritSec m_cSharedState;            // Lock on m_rtSampleTime and m_Ball
    CRefTime m_rtSampleTime;            // The time stamp for each sample

	bool bSetFormat;
	AM_MEDIA_TYPE *m_mediaType;	//use for setformt and getformat

	LPDIRECTSOUND8  m_PlayDev;
	LPDIRECTSOUNDBUFFER8 m_PlayBuffer;
	GUID m_capDevID;
	GUID m_rendDevID;
	HWND m_previewHwnd;

	//FillBuffer
	CStaticMediaBuffer * m_pOutputBuffer;
    DMO_OUTPUT_DATA_BUFFER * m_pOutputBufferStruct;
	BYTE *m_pbOutputBuffer;

	//void recordMessage(char* message);
	//FILE *errorlog;
};
    
