#ifndef VIDEO_FILE_PLAY_CTRL_H
#define VIDEO_FILE_PLAY_CTRL_H

// ȡ�����ں�����ʱ�ľ���
#pragma warning(disable:4995)

#include <streams.h>
#include <atlbase.h>

#include <queue>
#include <vector>
#include <map>

#include "ITypeHandler.h"
#include "SyncManager.h"
#include "FilePlayCtrlInterface.h"


/************************************************************************/
/* Override input pin class                                             */
/************************************************************************/

class FilePlayCtrlFilter;
class FilePlayOutPin;

class FilePlayInPin : public CBaseInputPin
{
	friend class FilePlayCtrlFilter;
    friend class FilePlayOutPin;

public:

	FilePlayInPin(
		TCHAR *pObjectName,
		FilePlayCtrlFilter *pTransformFilter,
		HRESULT * phr,
		LPCWSTR pName,
		INT nIndex);

	virtual STDMETHODIMP Receive(IMediaSample * pSample);
    virtual STDMETHODIMP Disconnect();
    virtual HRESULT CompleteConnect(IPin *pReceivePin);
    virtual HRESULT CheckMediaType( const CMediaType * );
    virtual STDMETHODIMP BeginFlush(void);
    virtual STDMETHODIMP EndFlush(void);
    virtual STDMETHODIMP EndOfStream(void);

    //our method
    CMediaType &CurrentMediaType() { return m_mt; }
    ITypeHandler *GetTypeHandler() { return type_handler_; };
    FilePlayOutPin *CorrespondOutPin();

private:
    int pin_index_;
    ITypeHandler *type_handler_;
    bool in_flush_;
    bool end_of_stream_;

};


/************************************************************************/
/* Override output pin class                                            */
/************************************************************************/

class FilePlayOutPin : public CBaseOutputPin
{
	friend class FilePlayCtrlFilter;

public:

	FilePlayOutPin(
		TCHAR *pObjectName,
		FilePlayCtrlFilter *pTransformFilter,
		HRESULT * phr,
		LPCWSTR pPinName);

    //pin�������
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
    //STDMETHODIMP FilePlayOutPin::EnumMediaTypes(IEnumMediaTypes **ppEnum);
    HRESULT CheckMediaType(const CMediaType* pmtOut);
    HRESULT SetMediaType(const CMediaType* pmtOut);
    HRESULT DecideBufferSize(
        IMemAllocator * pAllocator,
        ALLOCATOR_PROPERTIES* pProp);

    //IQualityControl
    STDMETHODIMP Notify(IBaseFilter *pSender, Quality q);

    //Our method
    CMediaType &CurrentMediaType() { return m_mt; }
    FilePlayInPin *GetCorrespondInPin();
    HRESULT StartDeliverThread();
    HRESULT StopDeliverThread();
    HRESULT PutSample(IMediaSample *sample);
    HRESULT FlushSample();

protected:
    void SampleDeliverProc();
    static unsigned int __stdcall ThreadDispatcher(void *obj);

protected:

    CCritSec sample_queue_lock_;
    typedef std::queue<IMediaSample*> SampleQueue;
    SampleQueue sample_queue_;
    REFERENCE_TIME next_deliver_sample_orig_ts_;

    HANDLE deliver_thread_;
    volatile bool stop_deliver_;

    bool sample_discontinuety_;
    REFERENCE_TIME last_deliver_ts_; //������͵�����������ʱ���
};


/************************************************************************/
/* Override filter class and interface                                  */
/************************************************************************/

class FilePlayCtrlFilter : public CBaseFilter, public ISCUTFilePlayCtrl
{

    friend class FilePlayInPin;
    friend class FilePlayOutPin;

public:

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    //
	// Implementation of IUnknown Interface
    //
	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

    //
	// IBaseFilter Interface
    //
    int GetPinCount();
    CBasePin *GetPin(int n);

	// ���ظú��������������ӳɹ���ʱ���ȡý������
	HRESULT CompleteConnect( PIN_DIRECTION direction , IPin *pReceivePin, const INT nInputPinIndex = 0 );

	// ���ظú�����������ͬ��
	HRESULT Receive(const INT nInputPinIndex, IMediaSample *pSample);

	// ����Receive����Ҫ���صĺ���������������ʱȷ��������������������
	// ����Ҫ����BeginFlush��EndFlush����ΪĿǰ��֧��Seek
	virtual HRESULT EndOfStream(const INT nInputPinIndex);

	// ��������ֹͣʱ��ջ������
	STDMETHODIMP Stop();
	STDMETHODIMP Pause();
	STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP GetState(DWORD dw, FILTER_STATE *pState);

    //
    // ISCUTFilePlayCtrl Interface
    //
    HRESULT SetSyncId(const char *id);
    HRESULT SetInitialTsOffset(REFERENCE_TIME offset);
    HRESULT Operate(unsigned long op_id, PlayOpCode op, PlayOpParam *param);
    HRESULT GetMediaFormat(MediaFormat &fmt);
    HRESULT GetProgress(ProgressInfo &info);

    //
	// CMultiInputTransformFilter
    //
	virtual FilePlayInPin * CreateInputPin(LPCWSTR wszPinName, int nIndex);
	virtual FilePlayOutPin * CreateOutputPin(LPCWSTR wszPinName);
 
    //
    // my members
    //
    void FixSampleTimestamp(REFERENCE_TIME &tStart, REFERENCE_TIME &tEnd);
    bool ShouldSampleDeliver(REFERENCE_TIME tStart);
    inline void ResetTimebase();
    inline void UpdateTimebase();

private:

	// Constructor
	FilePlayCtrlFilter(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);
	~FilePlayCtrlFilter();

    unsigned int GetInputPinCount() { return input_pin_list_.size(); }
    FilePlayInPin *CreateNextInputPin();
    UINT GetFreeInputPinCount();

    HRESULT PausePlay();
    HRESULT ResumePlay();
    HRESULT Seek(LONGLONG t);

    inline REFERENCE_TIME GetStreamTime();

private:
    static const unsigned long MAX_INPUT_PIN_COUNT = 16;
    static const unsigned long TIMELINE_START_DELAY = 20000000; //2s
    static const unsigned long PLAYBACK_DELAY = 5000000; //500ms

    CCritSec filter_lock_;

    char sync_id_[32];
    SyncManager sync_manager_;
    SyncManager::SharedData sync_private_;
    REFERENCE_TIME timestamp_offset_;
    long next_version_;
    long long seek_time_; //���ڼ���ʱ���У׼ֵ
    long long pause_time_; //���ڿ�����ͣ���Լ�����ָ����ź��ʱ���У׼ֵ
    long long last_deliver_orig_ts_; //������͵�������ԭʼʱ��������ڻ�ȡ���Ž���
    bool wait_deliver_;
    bool in_seeking_;

    typedef std::vector<FilePlayInPin *> InputPinList;
    InputPinList input_pin_list_;

    typedef std::map<int, FilePlayOutPin*> OutputPinTable;
    OutputPinTable output_pin_table_;

};

#endif
