#include "FilePlayCtrlFilter.h"

#include <InitGuid.h>
#include <process.h>

#include "log/Log.h"
#include "guid.h"
#include "Util.h"
#include "VideoTypeHandler.h"
#include "AudioTypeHandler.h"


CUnknown * WINAPI FilePlayCtrlFilter::CreateInstance( LPUNKNOWN punk, HRESULT *phr )
{
	ASSERT(phr);

	FilePlayCtrlFilter *pNewObject = new FilePlayCtrlFilter(NAME("File Play Ctrl"), punk, phr);
	if (pNewObject == NULL) {
		if (phr)
			*phr = E_OUTOFMEMORY;
	}

	return pNewObject;
}



FilePlayCtrlFilter::FilePlayCtrlFilter( TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr )
	: CBaseFilter(tszName, punk, &filter_lock_, CLSID_SCUTFilePlayCtrl)
{
    GenerateRandomSyncId(sync_id_, sizeof(sync_id_));
    ResetTimebase();
}

FilePlayCtrlFilter::~FilePlayCtrlFilter()
{
	// Delete the pins

	for (UINT i = 0; i < GetInputPinCount(); ++i)
	{
		if (input_pin_list_[i])
		{
			delete input_pin_list_[i];
			input_pin_list_[i] = NULL;
		}
	}

	for (unsigned int i = 0; i < output_pin_table_.size(); ++i)
	{
		delete output_pin_table_[i];
		output_pin_table_[i] = NULL;
	}
    output_pin_table_.clear();

}

FilePlayInPin * FilePlayCtrlFilter::CreateInputPin(LPCWSTR wszPinName, int nIndex)
{
	HRESULT hr = NOERROR;
	FilePlayInPin * pPin = new FilePlayInPin(NAME("pin in"),
		this,					// Owner filter
		&hr,					// Result code
		wszPinName,				// Pin name
		nIndex);	// Pin index

	if (FAILED(hr) || pPin == NULL) 
	{
		if (pPin) delete pPin;
		return NULL;
	}

	return pPin;

}

FilePlayOutPin * FilePlayCtrlFilter::CreateOutputPin( LPCWSTR wszPinName )
{
	HRESULT hr = NOERROR;
	FilePlayOutPin *pPin = new FilePlayOutPin(NAME("pin out"),
		this,            // Owner filter
		&hr,             // Result code
		wszPinName);   // Pin name

	if (FAILED(hr) || pPin == NULL) 
	{
		if (pPin) delete pPin;
		return NULL;
	}

	return pPin;

}

int FilePlayCtrlFilter::GetPinCount()
{
    if (GetInputPinCount() == 0)
    {
        FilePlayInPin * pPin = CreateNextInputPin();
        // Can't fail
        ASSERT(pPin != NULL);
    }
    return GetInputPinCount() + output_pin_table_.size();
}

CBasePin *FilePlayCtrlFilter::GetPin(int n)
{
    if (n < 0) return NULL;
    if (n < (int)GetInputPinCount()) {
        return input_pin_list_[n];
    }
    else if (n < GetPinCount()) {
        return output_pin_table_[n-(int)GetInputPinCount()];
    }
    return NULL;
}

HRESULT FilePlayCtrlFilter::Receive( const INT nInputPinIndex, IMediaSample *pSample )
{
    FilePlayOutPin *outpin = output_pin_table_[nInputPinIndex];
    if (outpin) {
        return outpin->PutSample(pSample);
    }
	return NOERROR;
}

STDMETHODIMP FilePlayCtrlFilter::Stop()
{
    CAutoLock lock(&filter_lock_);

    //run��ʱ����run�Լ���run base��stopʱ������
    HRESULT hr = CBaseFilter::Stop();
    if (FAILED(hr)) {
        LOG_ERROR("stop CBaseFilter failed");
        return hr;
    }

    for (unsigned int i = 0; i < output_pin_table_.size(); ++i)
    {
        if (output_pin_table_[i]) {
            output_pin_table_[i]->StopDeliverThread();
        }
    }

    ResetTimebase();
    return NOERROR;
}

STDMETHODIMP FilePlayCtrlFilter::Pause()
{
    CAutoLock lock(&filter_lock_);

	return CBaseFilter::Pause();
}

STDMETHODIMP FilePlayCtrlFilter::Run(REFERENCE_TIME tStart)
{
    CAutoLock lock(&filter_lock_);
    HRESULT hr = CBaseFilter::Run(tStart);
    if (FAILED(hr)) return hr;

    wait_deliver_ = true;
    in_seeking_ = false;
    next_version_ = 0;
    pause_time_ = -1;
    //��Ҫ����seek_time_�����ֵ��SetInitialTsOffset�������ã�
    LOG_INFO("initial_seek_time=%I64d", seek_time_);

    LOG_INFO("sync_id=%s", sync_id_);
    sync_manager_.Init(sync_id_);
    sync_private_.version = -1; //�趨Ϊδ�״�ͬ��
    sync_private_.start_time = GetDShowTimeFromUnixTime(nullptr) + TIMELINE_START_DELAY; //����ʱ�����runʱ����
    sync_private_.base_stream_time = PLAYBACK_DELAY; //ʱ���Ὺʼ�����ӳ�һ�㲥�ţ�ȷ��ͬ��
    UpdateTimebase();

    for (unsigned int i = 0; i < output_pin_table_.size(); ++i)
    {
        if (output_pin_table_[i] && output_pin_table_[i]->IsConnected()) {
            output_pin_table_[i]->StartDeliverThread();
        }
    }

	return S_OK;
}

HRESULT FilePlayCtrlFilter::GetState(DWORD dw, FILTER_STATE *pState)
{
    //��Ϊʹ�������̵߳���Sample���ͣ���Ҫ��ͣʱ����VFW_S_CANT_CUE����ֹ��������
    //�ο�CBaseFilter.GetState method
    //http://msdn.microsoft.com/en-us/library/windows/desktop/dd373798(v=vs.85).aspx
    CheckPointer(pState, E_POINTER);
    *pState = m_State;
    if (m_State == State_Paused)
        return VFW_S_CANT_CUE;
    else
        return S_OK;
}

HRESULT FilePlayCtrlFilter::EndOfStream( const INT nInputPinIndex )
{
    FilePlayOutPin *outpin = output_pin_table_[nInputPinIndex];
    if (outpin) {
        outpin->DeliverEndOfStream();
    }
	return NOERROR;
}

HRESULT FilePlayCtrlFilter::CompleteConnect( PIN_DIRECTION direction , IPin *pReceivePin, INT nInputPinIndex )
{
    if (PINDIR_INPUT == direction) {
        //�Ƿ�����һ������pin��
        if (GetFreeInputPinCount() == 0 && GetInputPinCount() < MAX_INPUT_PIN_COUNT)
        {
            CreateNextInputPin();
        }
        //���û�ж�Ӧ�����pin������һ��
        if (output_pin_table_.find(nInputPinIndex) == output_pin_table_.end() ||
            output_pin_table_[nInputPinIndex] == nullptr) {
                FilePlayOutPin *pin = CreateOutputPin(L"pin out");
                output_pin_table_[nInputPinIndex] = pin;
                IncrementPinVersion();
        } else {
            output_pin_table_[nInputPinIndex]->Disconnect();
        }
    }
    
	return NOERROR;
}


STDMETHODIMP FilePlayCtrlFilter::NonDelegatingQueryInterface( REFIID riid, void **ppv )
{
	  CheckPointer( ppv , E_POINTER );

    if(riid == IID_IFilePlayCtrl)
    {
        return GetInterface((ISCUTFilePlayCtrl*)this, ppv);
    }
	  return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
}

FilePlayInPin * FilePlayCtrlFilter::CreateNextInputPin()
{
    CAutoLock lock(&filter_lock_);

    WCHAR wszPinName[20];
    swprintf_s(wszPinName, L"XForm In %d", GetInputPinCount() + 1);

    FilePlayInPin * pPin = CreateInputPin(wszPinName, GetInputPinCount());

    if (pPin == NULL) 
    {
        return NULL;
    }

    //pPin->AddRef();
    input_pin_list_.push_back(pPin);
    IncrementPinVersion();

    return pPin;
}

UINT FilePlayCtrlFilter::GetFreeInputPinCount()
{
    UINT n = 0;
    for (UINT i = 0; i < GetInputPinCount(); ++i)
    {
        if (!input_pin_list_[i]->IsConnected())
        {
            ++n;
        }
    }
    return n;
}

HRESULT FilePlayCtrlFilter::GetMediaFormat(MediaFormat &fmt)
{
    memset(&fmt, 0, sizeof(MediaFormat));
    fmt.video_codec = ISCUTFilePlayCtrl::VIDEO_NONE;
    fmt.audio_codec = ISCUTFilePlayCtrl::AUDIO_NONE;

    for (auto input_pin : input_pin_list_)
    {
        if (!input_pin->IsConnected()) continue;
        if (input_pin->m_mt.majortype == MEDIATYPE_Video)
        {
            //Ŀǰ��֧��H264
            if (input_pin->m_mt.subtype != MEDIASUBTYPE_H264) return E_FAIL;
            fmt.video_codec = ISCUTFilePlayCtrl::H264;

            if (!VideoTypeHandler::GetVideoResolution(input_pin->m_mt,
              fmt.video_width, fmt.video_height, fmt.video_fps)) 
              return E_FAIL;
        }
        else if (input_pin->m_mt.majortype == MEDIATYPE_Audio) {
            //Ŀǰ��֧��WAVE���͵���Ƶ����
            if (input_pin->m_mt.formattype != FORMAT_WaveFormatEx) return E_FAIL;
            fmt.audio_codec = ISCUTFilePlayCtrl::AAC_LC;

            WAVEFORMATEX * pFmt = (WAVEFORMATEX *)input_pin->m_mt.pbFormat;
            fmt.audio_sample_rate = pFmt->nSamplesPerSec;
            fmt.audio_channel = pFmt->nChannels;
            fmt.audio_bit_count = pFmt->wBitsPerSample;
        }
    }
    return S_OK;
}

HRESULT FilePlayCtrlFilter::GetProgress(ProgressInfo &info)
{
    if (m_State != State_Running) info.status = ISCUTFilePlayCtrl::ST_STOP;
    else if (pause_time_ != -1) info.status = ISCUTFilePlayCtrl::ST_PAUSE;
    else if (wait_deliver_) info.status = ISCUTFilePlayCtrl::ST_WAIT;
    else {
        bool all_track_eos = true;
        for (UINT i = 0; i < GetInputPinCount(); ++i) {
            if (!input_pin_list_[i]->IsConnected()) continue;
            all_track_eos = input_pin_list_[i]->end_of_stream_ && all_track_eos;
        }
        if (all_track_eos) info.status = ISCUTFilePlayCtrl::ST_EOS;
        else info.status = ISCUTFilePlayCtrl::ST_PLAYING;
    }

    //ֻҪ���κ�һ�������ӵ�����pin��ȡ��Ϣ����
    for (UINT i = 0; i < GetInputPinCount(); ++i) {
        if (!input_pin_list_[i]->IsConnected()) continue;
        CComPtr<IPin> upstream_pin = input_pin_list_[i]->GetConnected();
        CComQIPtr<IMediaSeeking> media_seek = upstream_pin;
        if (media_seek) {
            media_seek->GetDuration(&info.duration);
            //long long stop;
            //media_seek->GetPositions(&pos, &stop); //�����õ�����һ��Ҫ���͵�������ʱ�������׼ȷŶ
            //seek������ʱ����������seek_time��0��ʼ������Ҫ����seek_time���ܻ�ԭʵ�ʵĲ��Ž���
            info.position = last_deliver_orig_ts_;
            info.position += (seek_time_ < 0 ? 0 : seek_time_); //seek_time_<0ֻ�Ǵ����ӳٲ���Ŷ
            if (info.position > info.duration) info.position = info.duration;
            break;
        } else {
            LOG_ERROR("GetProgress failed: Media Seeking not support");
        }
    }
    return S_OK;
}

HRESULT FilePlayCtrlFilter::SetSyncId( const char *id )
{
    if (!id) return E_INVALIDARG;
    strcpy_s(sync_id_, sizeof(sync_id_), id);
    return S_OK;
}

HRESULT FilePlayCtrlFilter::SetInitialTsOffset( REFERENCE_TIME offset )
{
    if (offset < 0) return E_INVALIDARG;
    seek_time_ = -offset;
    return S_OK;
}

HRESULT FilePlayCtrlFilter::Operate(
    unsigned long op_id, PlayOpCode op, PlayOpParam *param)
{
    //����runningʱ��֧����Щ����
    if (m_State != State_Running) {
        LOG_ERROR("Operate while not running");
        return E_FAIL;
    }

    if (op_id < 1) {
        LOG_ERROR("op_id cannot less than 1");
        return E_INVALIDARG;
    }
    (long)op_id > next_version_ ? next_version_ = (long)op_id : ++next_version_;
    HRESULT hr = E_INVALIDARG;

    //Ҳû����������ֱ��switch��
    switch (op)
    {
    case ISCUTFilePlayCtrl::kOpPause:
        hr = PausePlay();
        break;
    case ISCUTFilePlayCtrl::kOpResume:
        hr = ResumePlay();
        break;
    case ISCUTFilePlayCtrl::kOpSeek:
        if (param) hr = Seek(param->seek.t);
        break;
    default:
        LOG_ERROR("Unknown operation, op_id=%d, op=%d", op_id, op);
        break;
    }
    return hr;
}

HRESULT FilePlayCtrlFilter::PausePlay()
{
    CAutoLock lock(&filter_lock_);
    pause_time_ = GetStreamTime();
    LOG_PRINTF("pause at %I64d", pause_time_);
    return S_OK;
}

HRESULT FilePlayCtrlFilter::ResumePlay()
{
    CAutoLock lock(&filter_lock_);
    long long pause_dur = GetStreamTime() - pause_time_;
    sync_private_.base_stream_time += pause_dur + PLAYBACK_DELAY;
    UpdateTimebase();
    pause_time_ = -1;
    LOG_PRINTF("Resume after stream time %I64d", pause_dur);
    return S_OK;
}

HRESULT FilePlayCtrlFilter::Seek( LONGLONG t )
{
    CAutoLock lock(&filter_lock_);
    //��¼seek��ʱ�䣬�Ա�UpdateTimebase��������ʱ���ƫ����
    //Ϊʲô����������ֱ�Ӽ����й�ʱ��ı�����ԭ��μ�FilePlayInPin::EndFlush()
    seek_time_ = t;

    //��ͣ���ţ���Seek��ɺ��ٻָ����ţ���ȷ������ʱ�����ȷ
    in_seeking_ = true;

    //ֻҪ���κ�һ�������ӵ�����pin����λ���󷢸����μ���
    //media_seek->SetPositions�л������������pin��FilePlayInPin::BeginFlush()���������OutPin�л����Sample
    for (UINT i = 0; i < GetInputPinCount(); ++i) {
        if (!input_pin_list_[i]->IsConnected()) continue;
        CComPtr<IPin> upstream_pin = input_pin_list_[i]->GetConnected();
        CComQIPtr<IMediaSeeking> media_seek = upstream_pin;
        if (media_seek) {
            LONGLONG current_time = (t < 0 ? 0 : t);
            LONGLONG duration = 0;
            media_seek->GetDuration(&duration);
            media_seek->SetPositions(&current_time, AM_SEEKING_AbsolutePositioning, 
                NULL, AM_SEEKING_NoPositioning);
            LOG_PRINTF("Seek to stream time %I64d(dur: %I64d)", current_time, duration);
            break;
        } else {
            LOG_ERROR("Media Seeking not support");
        }
    }

    //���������йر���
    sync_private_.base_stream_time = GetStreamTime() + PLAYBACK_DELAY;
    UpdateTimebase();
    wait_deliver_ = true;
    last_deliver_orig_ts_ = 0;
    if (pause_time_ != -1) {
      //����ͣʱseek����Ҫ������ͣʱ�䣬��Ȼ�ָ����ź�������
      pause_time_ = GetStreamTime();
    }

    //��յ�ǰ�����sample
    for (auto &output : output_pin_table_) {
        output.second->FlushSample();
    }

    //�ָ�����
    in_seeking_ = false;

    return S_OK;
}

void FilePlayCtrlFilter::FixSampleTimestamp( REFERENCE_TIME &tStart, REFERENCE_TIME &tEnd )
{
    REFERENCE_TIME tDur = tEnd - tStart;
    tStart = tStart + timestamp_offset_ + sync_private_.base_stream_time;
    tEnd = tStart + tDur;
}

bool FilePlayCtrlFilter::ShouldSampleDeliver( REFERENCE_TIME tStart )
{
    if (tStart > GetStreamTime()) {
        //��������ʱ�������ʱ�䣬˵��ʱ��δ��
        return false;
    }
    return true;
}

REFERENCE_TIME FilePlayCtrlFilter::GetStreamTime()
{
    return GetDShowTimeFromUnixTime(nullptr) - sync_private_.start_time;
}

void FilePlayCtrlFilter::ResetTimebase()
{
    timestamp_offset_ = 0;
    seek_time_ = 0;
    pause_time_ = -1;
    last_deliver_orig_ts_ = 0;
}

void FilePlayCtrlFilter::UpdateTimebase()
{
    sync_manager_.Lock();
    if (next_version_ == sync_manager_.data_->version) {
        //˵��������������������ǰ���ύ�ˣ��򸲸��Լ�������
        memcpy_s(&sync_private_, sizeof(SyncManager::SharedData), 
            sync_manager_.data_, sizeof(SyncManager::SharedData));
        LOG_DEBUG("update sync data");
    } else {
        sync_private_.version = next_version_;
        sync_private_.modifier_id = GetCurrentProcessId();

        if (next_version_ > sync_manager_.data_->version) {
            //˵�����ǵ����ݱȹ��������£������ύ����
            memcpy_s(sync_manager_.data_, sizeof(SyncManager::SharedData), 
                &sync_private_, sizeof(SyncManager::SharedData));
            LOG_DEBUG("commit sync data");
        } else {
            //next_version_ < sync_manager_.data_->version
            //˵��������������̫�£����������ǿ��Խ��ܵģ����������Լ�������
            //��������ڲ���Ƶ��ʱ���̵��Ȳ���ʱ����������ĳ���̽������������������������յ���һ��������
            LOG_DEBUG("sync data too new, ignore it");
        }
    }
    sync_manager_.Unlock();

    //seek_time_ < 0��ʾ������Ҫ�ӳ�seek_time_ʱ������
    timestamp_offset_ = (seek_time_ >= 0 ? 0 : -seek_time_);

    LOG_PRINTF("ver=%d, base_stream_time_=%I64d, timestamp_offset_=%I64d", 
        sync_private_.version, sync_private_.base_stream_time, timestamp_offset_);
}


/************************************************************************/
/* Method for input pin class                                           */
/************************************************************************/

FilePlayInPin::FilePlayInPin( TCHAR *pObjectName, FilePlayCtrlFilter *pFilter, HRESULT * phr, LPCWSTR pName, INT nIndex )
: CBaseInputPin(pObjectName, pFilter, &pFilter->filter_lock_, phr, pName)
, pin_index_(nIndex)
, type_handler_(nullptr)
, in_flush_(false)
, end_of_stream_(false)
{

}

STDMETHODIMP FilePlayInPin::Receive( IMediaSample * pSample )
{
	HRESULT hr;
	ASSERT(pSample);

    if (in_flush_) return S_OK; //flush״̬�²���������

    FilePlayCtrlFilter *filter = (FilePlayCtrlFilter*)m_pFilter;
    if (end_of_stream_) end_of_stream_ = false;

	// check all is well with the base class
	//hr = CBaseInputPin::Receive(pSample);
	//if (S_OK == hr) {
	hr = filter->Receive(pin_index_, pSample);
	//}

	return hr;

}

HRESULT FilePlayInPin::CompleteConnect( IPin *pReceivePin )
{
    HRESULT hr = NOERROR;
    FilePlayCtrlFilter *filter = (FilePlayCtrlFilter*)m_pFilter;

    hr = CBaseInputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr)) {
        return hr;
    }

    hr = filter->CompleteConnect(PINDIR_INPUT, pReceivePin, pin_index_);
    if (FAILED(hr)) {
        return hr;
    }

    return NOERROR;
}

STDMETHODIMP FilePlayInPin::Disconnect()
{
	HRESULT hr = CBasePin::Disconnect();
	if (FAILED(hr))
	{
		return hr;
	}

    FilePlayOutPin *outpin = CorrespondOutPin();
    if (outpin) {
        outpin->Disconnect();
    }

    delete type_handler_;
    type_handler_ = nullptr;

	return S_OK;
}

HRESULT FilePlayInPin::CheckMediaType( const CMediaType * mtIn)
{
    //Ŀǰֻ��������Ƶ
    if (mtIn->majortype == MEDIATYPE_Video) {
        if (!type_handler_ || !type_handler_->IsVideo()) {
            delete type_handler_;
            type_handler_ = new VideoTypeHandler;
        }
    }
    else if (mtIn->majortype == MEDIATYPE_Audio) {
        if (!type_handler_ || !type_handler_->IsAudio()) {
            delete type_handler_;
            type_handler_ = new AudioTypeHandler;
        }
    }

    if (type_handler_) return type_handler_->CheckMediaType(*mtIn);
    return E_FAIL;
}

STDMETHODIMP FilePlayInPin::BeginFlush( void )
{
    in_flush_ = true;
    FilePlayOutPin *outpin = CorrespondOutPin();
    if (outpin) {
        outpin->FlushSample();
    }
    return S_OK;
}

STDMETHODIMP FilePlayInPin::EndFlush( void )
{
    in_flush_ = false;
    return S_OK;
}

FilePlayOutPin * FilePlayInPin::CorrespondOutPin()
{
    FilePlayCtrlFilter *filter = (FilePlayCtrlFilter*)m_pFilter;
    FilePlayOutPin *outpin = filter->output_pin_table_[pin_index_];
    return outpin;
}

STDMETHODIMP FilePlayInPin::EndOfStream( void )
{
    LOG_INFO("input[%d] EOS", pin_index_);
    end_of_stream_ = true;
    return S_OK;
}


/************************************************************************/
/* Method for output pin class                                          */
/************************************************************************/

FilePlayOutPin::FilePlayOutPin( TCHAR *pObjectName, FilePlayCtrlFilter *pFilter, HRESULT * phr, LPCWSTR pPinName )
	: CBaseOutputPin(pObjectName, pFilter, &pFilter->filter_lock_, phr, pPinName)
    , deliver_thread_(NULL)
    , stop_deliver_(true)
{

}

FilePlayInPin * FilePlayOutPin::GetCorrespondInPin()
{
    FilePlayCtrlFilter *filter = (FilePlayCtrlFilter*)m_pFilter;
    for (unsigned int i = 0; i < (filter)->GetInputPinCount(); ++i) {
        if (filter->output_pin_table_.find(i) != filter->output_pin_table_.end() &&
            filter->output_pin_table_[i] == this) {
                return dynamic_cast<FilePlayInPin *>(filter->input_pin_list_[i]);
        }
    }
    return nullptr;
}

HRESULT FilePlayOutPin::GetMediaType(int iPosition, CMediaType *pmt)
{
    CheckPointer(pmt,E_POINTER);
    CAutoLock cAutoLock(m_pLock);

    if(iPosition < 0)
        return E_INVALIDARG;

    // Have we run off the end of types?
    if(iPosition > 0)
        return VFW_S_NO_MORE_ITEMS;

    //pin�м̳е�m_mt��ԱΪ���Ӻ�ĸ�ʽ��������֧�ֵĸ�ʽ������Ӧ����֧�ֵĸ�ʽ
    //�����ҵ���Ӧ����pin���ӵ����pin��֧������
    FilePlayInPin *input_pin = GetCorrespondInPin();
    if (input_pin == NULL || input_pin->GetConnected() == NULL) return VFW_E_NOT_CONNECTED;

    *pmt = input_pin->CurrentMediaType();

    return NOERROR;
}

/*STDMETHODIMP FilePlayOutPin::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
    CAutoLock lock_it(m_pLock);
    ASSERT(ppEnum);

    //�����ҵ���Ӧ����pin���ӵ����pin��֧������
    FilePlayInPin *input_pin = GetCorrespondInPin();
    if (input_pin == NULL || input_pin->GetConnected() == NULL) return VFW_E_NOT_CONNECTED;

    // We will simply return the enumerator of our input pin's peer
    return input_pin->GetConnected()->EnumMediaTypes(ppEnum);

} // EnumMediaTypes
*/

HRESULT FilePlayOutPin::CheckMediaType( const CMediaType* pmtOut )
{
    CheckPointer(pmtOut,E_POINTER);
    CAutoLock cAutoLock(m_pLock);

    FilePlayInPin *input_pin = GetCorrespondInPin();
    if (input_pin == NULL || input_pin->GetConnected() == NULL) return VFW_E_NOT_CONNECTED;

    if (*pmtOut != input_pin->CurrentMediaType()) {
        return E_FAIL;
    }

    return NOERROR;
}

HRESULT FilePlayOutPin::SetMediaType( const CMediaType* pmtOut )
{
    HRESULT hr = NOERROR;

    hr = CheckMediaType(pmtOut);
    if (FAILED(hr)) {
        return hr;
    }

    // Set the base class media type (should always succeed)
    hr = CBasePin::SetMediaType(pmtOut);
    if (FAILED(hr)) {
        return hr;
    }

    return NOERROR;
}

HRESULT FilePlayOutPin::DecideBufferSize( IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES* pProp )
{
    CheckPointer(pAlloc,E_POINTER);
    CheckPointer(pProp,E_POINTER);

    const int BUFFER_COUNT = 19;
    HRESULT hr = NOERROR;
    pProp->cBuffers = BUFFER_COUNT;   
    pProp->cbAlign = 1;  

    CMediaType& out_mt = CurrentMediaType();

    //�����piný�����ͼ����̶�Ҫ������pin�����ӣ�DecideBufferSize���������ǰ���ã�
    //���ܵ����������pin��Ȼ�Ѿ����Ӳ�ȷ��ý������
    FilePlayInPin *input_pin = GetCorrespondInPin();
    ASSERT(input_pin->GetTypeHandler() != nullptr);
    hr = input_pin->GetTypeHandler()->CalcSampleSize(out_mt, pProp->cbBuffer); 
    if(FAILED(hr))   
    {   
        return hr;   
    }

    ASSERT(pProp->cbBuffer);   

    //   Ask   the   allocator   to   reserve   us   some   sample   memory,   NOTE   the   function   
    //   can   succeed   (that   is   return   NOERROR)   but   still   not   have   allocated   the   
    //   memory   that   we   requested,   so   we   must   check   we   got   whatever   we   wanted   

    ALLOCATOR_PROPERTIES Actual;   
    hr = pAlloc->SetProperties(pProp, &Actual);   
    if(FAILED(hr))   
    {   
        return hr;   
    }   

    ASSERT(Actual.cBuffers == BUFFER_COUNT);   

    if(pProp->cBuffers > Actual.cBuffers ||   
        pProp->cbBuffer > Actual.cbBuffer)   
    {   
        return E_FAIL;   
    }

    return NOERROR;
}

STDMETHODIMP FilePlayOutPin::Notify( IBaseFilter *pSender, Quality q )
{
    FilePlayInPin *input_pin = GetCorrespondInPin();
    if (!input_pin) return NOERROR;

    //�ο�DShow sample�е�InfTeeʵ��
    if(input_pin->m_pQSink!=NULL)
    {
        return input_pin->m_pQSink->Notify(m_pFilter, q);
    }
    else
    {
        // No sink set, so pass it upstream
        HRESULT hr;
        IQualityControl * pIQC;

        hr = VFW_E_NOT_FOUND;
        if(input_pin->m_Connected)
        {
            input_pin->m_Connected->QueryInterface(IID_IQualityControl,(void**)&pIQC);

            if(pIQC!=NULL)
            {
                hr = pIQC->Notify(m_pFilter, q);
                pIQC->Release();
            }
        }
        return hr;
    }
    return NOERROR;
}

HRESULT FilePlayOutPin::StartDeliverThread()
{
    if (deliver_thread_) {
        //�����Ч��Ӧ���Ǵ���ͣ�лָ��ģ����ٴ����߳�
        LOG_INFO("deliver thread already started, maybe resume from pause");
        return S_OK;
    }
    stop_deliver_ = false;
    deliver_thread_ = (HANDLE)_beginthreadex(nullptr, 0, ThreadDispatcher, static_cast<void*>(this), 0, nullptr);
    if (deliver_thread_ == NULL || deliver_thread_ == INVALID_HANDLE_VALUE) {
        LOG_ERROR("start deliver thread failed");
        deliver_thread_ = NULL;
    } else {
        LOG_INFO("deliver thread 0x%08x start", deliver_thread_);
    }
    return NOERROR;
}

HRESULT FilePlayOutPin::StopDeliverThread()
{
    if (deliver_thread_ != NULL) {
        stop_deliver_ = true;
        LOG_DEBUG("Waiting for deliver thread 0x%08x to exit", deliver_thread_);
        DWORD wait_result = WaitForSingleObject(deliver_thread_, INFINITE);
        if (WAIT_OBJECT_0 != wait_result) {
            TerminateThread(deliver_thread_, -1);
            LOG_WARN("Wait timeout, terminate deliver thread");
        }
        deliver_thread_ = NULL;

        //clear sample queue
        FlushSample();
    }
    return NOERROR;
}

HRESULT FilePlayOutPin::PutSample( IMediaSample *sample )
{
    if (!IsConnected()) return S_FALSE;
    if (((FilePlayCtrlFilter*)m_pFilter)->m_State == State_Stopped) return S_FALSE;

    CAutoLock lock(&sample_queue_lock_);
    sample->AddRef(); //����Ҫ��ʱ����У��������ü���
    sample_queue_.push(sample);
    return NOERROR;
}

void FilePlayOutPin::SampleDeliverProc()
{
    FilePlayCtrlFilter *filter = (FilePlayCtrlFilter*)m_pFilter;
    sample_discontinuety_ = false;
    last_deliver_ts_ = -1;

    while (!stop_deliver_)
    {
        //ע�⣺����Ҫͬʱ����������ʱ��Ϊ��ֹ����������ĿԼ����
        //����������sample_queue_lock_��������filter�е���
        CritSecHelper sample_lock(&sample_queue_lock_);
        if (filter->pause_time_ > -1 || filter->in_seeking_ ||
            sample_queue_.empty()) {
            //��ͣ�л��޴���������ʱ
            sample_lock.Unlock();
            Sleep(5);
            continue;
        }

        CComPtr<IMediaSample> next_deliver_sample;
        next_deliver_sample.Attach(sample_queue_.front()); //����PutSample��AddRef����Ҫֱ�Ӹ�ֵ��������ظ�AddRef

        if (next_deliver_sample) {
            REFERENCE_TIME start_time, end_time;
            next_deliver_sample->GetTime(&start_time, &end_time);
            //LOG_DEBUG("%I64d -", start_time); //debug
            next_deliver_sample_orig_ts_ = start_time;

            //fix time
            filter->FixSampleTimestamp(start_time, end_time);
            //LOG_DEBUG("ts=%I64d, stream=%I64d", start_time, filter->GetStreamTime()); //debug

            //should sample discard
            // DO NOT change the order of the following statements
            if (next_deliver_sample->IsDiscontinuity() == S_OK) sample_discontinuety_ = true;
            if (next_deliver_sample->IsSyncPoint() == S_OK) sample_discontinuety_ = false;
            if (sample_discontinuety_ || next_deliver_sample_orig_ts_ < 0 || 
                next_deliver_sample->IsPreroll() == S_OK) {
                next_deliver_sample = nullptr; //deref
                sample_queue_.pop(); //�Ӷ������Ƴ�sample
                sample_discontinuety_ = true;
                //LOG_DEBUG("drop %I64d", start_time); //debug
                continue; //ע���ͷų��е���
            } // otherwise the sample_discontinuety_ must be false

            //ȷ��ʱ�����������
            if (start_time <= last_deliver_ts_) {
                next_deliver_sample = nullptr;
                sample_queue_.pop(); //�Ӷ������Ƴ�sample
                sample_discontinuety_ = true;
                LOG_WARN("ts less than last, drop %I64d", start_time);
                continue; //ע���ͷų��е���
            }

            if (filter->ShouldSampleDeliver(start_time)) {
                next_deliver_sample->SetTime(&start_time, &end_time);
                Deliver(next_deliver_sample);
                next_deliver_sample = nullptr;
                sample_queue_.pop(); //�Ӷ������Ƴ�sample
                last_deliver_ts_ = start_time;
                filter->last_deliver_orig_ts_ = next_deliver_sample_orig_ts_;
                filter->wait_deliver_ = false;
                printf("\t\t\tprogress: %I64d\r", start_time); //debug
            } else {
                next_deliver_sample.Detach(); //��Ҫ������
                sample_lock.Unlock();
                Sleep(5);
                continue;
            }
        }
    }
    LOG_INFO("deliver thread 0x%08x exit normally", deliver_thread_);
}

unsigned int __stdcall FilePlayOutPin::ThreadDispatcher( void *obj )
{
    static_cast<FilePlayOutPin*>(obj)->SampleDeliverProc();
    return 0;
}

HRESULT FilePlayOutPin::FlushSample()
{
    CAutoLock lock(&sample_queue_lock_);
    while (!sample_queue_.empty()) {
        IMediaSample *sample = sample_queue_.front();
        sample->Release();
        sample_queue_.pop();
    }
    return NOERROR;
}
