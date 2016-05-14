#include "AudioTypeHandler.h"
#include "Log/Log.h"

AudioTypeHandler::AudioTypeHandler(void)
{
}


AudioTypeHandler::~AudioTypeHandler(void)
{
}

HRESULT AudioTypeHandler::CheckMediaType( const CMediaType &mt )
{
    if (mt.majortype != MEDIATYPE_Audio) return E_FAIL;
    return S_OK;
}

HRESULT AudioTypeHandler::CalcSampleSize( const CMediaType &mt, long &size )
{
    size = 0;
    if (mt.majortype != MEDIATYPE_Audio) return E_FAIL;

    if (mt.formattype == FORMAT_WaveFormatEx)
    {
        WAVEFORMATEX * pFmt = (WAVEFORMATEX *)mt.pbFormat;
        //�����㹻1��Ĵ�С
        //size = pFmt->nAvgBytesPerSec; //���ֵ�����׶���������0
        size = pFmt->nSamplesPerSec * pFmt->nChannels * pFmt->wBitsPerSample / 8;
        LOG_INFO("Audio sample size=%dHz*%dch*%dbits=%dBytes", 
            pFmt->nSamplesPerSec, pFmt->nChannels, pFmt->wBitsPerSample, size);
    }
    else
    {
        size = 96000 * 2 * 16 / 8; //�㹻96k˫����16bit��Ƶ1��
        LOG_INFO("Using default audio sample size=%dBytes", size);
    }

    return S_OK;
}
