#ifndef VIDEO_TYPE_HANDLER_H
#define VIDEO_TYPE_HANDLER_H

#include "itypehandler.h"


class VideoTypeHandler :
    public ITypeHandler
{
public:
    VideoTypeHandler(void);
    ~VideoTypeHandler(void);

    virtual HRESULT CheckMediaType(const CMediaType &mt);
    virtual HRESULT CalcSampleSize(const CMediaType &mt, long &size);
    virtual bool IsVideo() { return true; }
    virtual bool IsAudio() { return false; }

    //�ض������ͷ���
    static bool GetVideoResolution(const CMediaType &mt, long &width, long &height, long &fps);
};


#endif
