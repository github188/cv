/************************************************************************/
/* ��ÿ��filter�Ĺ����װ���࣬���㸴��                                 */
/* ע�⣺���ʱû�п����ظ���ӻ�����filter���йط����ظ����ý�������Ϊ */
/* ����Ԥ��.                                                            */
/************************************************************************/

#ifndef FILTER_CONTAINER_H
#define FILTER_CONTAINER_H

#include <atlbase.h>
#include <streams.h>
#include <string>
#include <list>
#include <map>

#include "DxDefinitions.h"
#include "util/report/RtpStat.h"

namespace msdx
{


//�ɼ�������
class CrossbarContainer
{
public:
    CrossbarContainer();
    ~CrossbarContainer();

    //���ｫgraph builder��COMָ��ת���˾���ָ�룬�д�����COMָ�룬��ô����û�����
    //��ΪCOMָ�벻��STL����ָ�룬COMָ��������ü����Ƕ��ڲ�����ָ��ִ��p->AddRef()
    //ʵ�����ü���������pָ��Ķ����ڡ�COMָ���ά������������
    dxStatus AddFilter(IGraphBuilder *owner_graph, std::string display_name);
    dxStatus AddFilterAuto(ICaptureGraphBuilder2 *capture, IBaseFilter *source_filter);
    IBaseFilter *GetFilter() { return video_crossbar_filter_; }
    dxStatus SetPhysicalType(long type);
    dxStatus ConnectToVideoSource(IBaseFilter *filter);

private:
    CComPtr<IGraphBuilder> graph_builder_;
    CComPtr<IBaseFilter> video_crossbar_filter_;
    long physical_type_;
};

//��Ƶ��������
class VideoSourceContainer
{
public:
    VideoSourceContainer();
    ~VideoSourceContainer();

    dxStatus AddToGraph(IGraphBuilder *owner_graph, std::string device_path);
	dxStatus AddToGraph(IGraphBuilder *owner_graph, std::string device_path,std::string displayName);
    IBaseFilter *GetFilter() { return video_source_filter_; }
    IPin *GetOutputPin() { return video_pin_out_; }
    const CMediaType &GetType() { return candidate_type_; }
    //������Ƶ��ʽ��֡�ʵ�ǰ��δʹ��
    dxStatus SetFormat(unsigned int width, unsigned int height, unsigned int fps);
	dxStatus SetFormatKinect( unsigned int width, unsigned int height, unsigned int fps );//add by hjf 2015.11.19

private:
    void SeekRealFormat(IPin * pPin);

private:
    CComPtr<IGraphBuilder> graph_builder_;
    CComPtr<IBaseFilter> video_source_filter_;
    CComPtr<IPin> video_pin_out_;
    CMediaType candidate_type_;
};

//��Ƶ����������
class VideoEncoderContainer
{
public:
    virtual ~VideoEncoderContainer() {};

    //��ӱ����������ӵ�upstream_pin�󣬿�����ӱ�Ҫ�ĸ�ʽת��filter
    virtual dxStatus AddToGraph(IGraphBuilder *owner_graph, IPin *upstream_pin, CMediaType mt) = 0;
    virtual IBaseFilter *GetFilter() = 0;

    struct EncoderParam
    {
        int width;
        int height;
        int target_kbps;
    };
    virtual dxStatus Config(EncoderParam &param) { return DX_WARN_NOT_IMPL; };
    virtual dxStatus PauseEncoding(bool pause) = 0;
};

class IntelH264EncoderContainer : public VideoEncoderContainer
{
public:
    IntelH264EncoderContainer();
    ~IntelH264EncoderContainer();

    virtual dxStatus AddToGraph(IGraphBuilder *owner_graph, IPin *upstream_pin, CMediaType mt);

    virtual IBaseFilter *GetFilter() { return video_encoder_filter_; }
    dxStatus Config(EncoderParam &param);
    dxStatus PauseEncoding(bool pause);

protected:
    CComPtr<IGraphBuilder> graph_builder_;
	CComPtr<IBaseFilter> video_encoder_filter_;
	CComPtr<IBaseFilter> video_converter_filter_;
};

class FfdshowH264EncoderContainer:public IntelH264EncoderContainer//Ϊ��Ӧkinect����дһ�����࣬������һ��graph --hjf 2015.12.14
{
public:
	~FfdshowH264EncoderContainer();
	virtual dxStatus AddToGraph(IGraphBuilder *owner_graph, IPin *upstream_pin, CMediaType mt);
private:
	//ffdshow
	
	CComPtr<IBaseFilter> video_ffdshow_encoder_filter_;
	CComPtr<IBaseFilter> video_ffdshow_decoder_filter_;
};

class LeadH264EncoderContainer:public VideoEncoderContainer//Ϊ��Ӧkinect����дһ�����࣬������һ��graph --hjf 2015.12.14
{
public:
	~LeadH264EncoderContainer();
	virtual dxStatus AddToGraph(IGraphBuilder *owner_graph, IPin *upstream_pin, CMediaType mt);
    virtual IBaseFilter *GetFilter() { return video_encoder_filter_; }
private:
	dxStatus PauseEncoding(bool pause){ return DX_WARN_NOT_IMPL;};
	CComPtr<IGraphBuilder> graph_builder_;
	CComPtr<IBaseFilter> video_encoder_filter_;
	CComPtr<IBaseFilter> video_CSconverter_filter_;
};



class ScutH264EncoderContainer : public VideoEncoderContainer
{
public:
    ~ScutH264EncoderContainer();

    virtual dxStatus AddToGraph(IGraphBuilder *owner_graph, IPin *upstream_pin, CMediaType mt);
    virtual IBaseFilter *GetFilter() { return video_encoder_filter_; }
    dxStatus Config(EncoderParam &param);
    dxStatus PauseEncoding(bool pause);

private:
    CComPtr<IGraphBuilder> graph_builder_;
    CComPtr<IBaseFilter> video_encoder_filter_;
};




//��Ƶ������
class VideoDecoderContainer
{
public:
    virtual ~VideoDecoderContainer() {};
    virtual dxStatus AddFilter(IGraphBuilder *owner_graph, IPin *upstream_pin) = 0;
    virtual IBaseFilter *GetFilter() = 0;
};

class IntelH264DecoderContainer : public VideoDecoderContainer
{
public:
    ~IntelH264DecoderContainer();
    virtual dxStatus AddFilter(IGraphBuilder *owner_graph, IPin *upstream_pin);
    virtual IBaseFilter *GetFilter() { return video_decoder_filter_; }

private:
    CComPtr<IGraphBuilder> graph_builder_;
    CComPtr<IBaseFilter> video_decoder_filter_;
};

class ScutH264DecoderContainer : public VideoDecoderContainer
{
public:
    ~ScutH264DecoderContainer();
    virtual dxStatus AddFilter(IGraphBuilder *owner_graph, IPin *upstream_pin);
    virtual IBaseFilter *GetFilter() { return video_decoder_filter_; }

private:
    CComPtr<IGraphBuilder> graph_builder_;
    CComPtr<IBaseFilter> video_decoder_filter_;
};


//rtp����

typedef struct
{
    void *obj;
    RtpStatCallback func;
} RtpCallback;

class RTPSenderContainer
{
public:
    virtual ~RTPSenderContainer() {}

    enum RtpType
    {
        RTP_TYPE_NONE,
        RTP_TYPE_VIDEO,
        RTP_TYPE_AUDIO,
    };
    virtual dxStatus AddToGraph(IGraphBuilder *owner_graph, RtpType type, IPin *upstream_pin) = 0;
    //��ӷ��Ͷ˿ڣ��ص�������ѡ
    virtual dxStatus AddDest(NetAddr, RtpCallback *) = 0;
    virtual dxStatus RemoveDest(NetAddr) = 0;
    virtual dxStatus Clear() = 0;
    virtual dxStatus PauseOverallSending(bool stop) = 0;
    virtual unsigned long GetCount() = 0;
};

class RTPReceiverContainer
{
public:
    virtual ~RTPReceiverContainer() {}

    //rtp_id��ѡ��ϵͳ��rtp���ն˿����ж�������ָ��һ����������
    virtual dxStatus AddFilter(IGraphBuilder *owner_graph, 
        CMediaType media_type, const char *rtp_id) = 0;
    //���ý���Դ���ص���ѡ
    virtual dxStatus SetSource(NetAddr addr) = 0;
    virtual dxStatus SetCallback(RtpCallback *callback) = 0;
    virtual dxStatus StopReceiving(bool stop) = 0;
    virtual IBaseFilter *GetFilter() = 0;
    virtual IPin *GetOutputPin() = 0;
    virtual const CMediaType &GetType() = 0;
};

//��һ��RTP
class RTPSender1Container : public RTPSenderContainer
{
public:
    RTPSender1Container();
    ~RTPSender1Container();

    dxStatus AddToGraph(IGraphBuilder *owner_graph, RtpType type, IPin *upstream_pin);
    dxStatus AddDest(NetAddr, RtpCallback *);
    dxStatus RemoveDest(NetAddr);
    dxStatus Clear();
    dxStatus PauseOverallSending(bool stop);
    unsigned long GetCount();

private:
    std::string GetKey(const NetAddr &addr);
    IBaseFilter *GetIdleFilter();
    inline char *RtpTypeName();
    void PrintPorts(); // for debug

private:
    typedef std::map<std::string, IBaseFilter*> WorkingFilterMap;
    typedef std::list<IBaseFilter*> IdleFilterList;

    WorkingFilterMap working_filters_;
    IdleFilterList idle_filters_;
    RtpType rtp_type_;
    CComPtr<IGraphBuilder> graph_builder_;
    CComPtr<IBaseFilter> multi_smart_tee_filter_;
};

class RTPReceiver1Container : public RTPReceiverContainer
{
public:
    ~RTPReceiver1Container();

    virtual dxStatus AddFilter(IGraphBuilder *owner_graph, 
        CMediaType media_type, const char *rtp_id);
    virtual dxStatus SetSource(NetAddr addr);
    virtual dxStatus SetCallback(RtpCallback *callback);
    virtual dxStatus StopReceiving(bool stop);
    virtual IBaseFilter *GetFilter() { return rtp_receiver_filter_; }
    virtual IPin *GetOutputPin();
    virtual const CMediaType &GetType() { return media_type_; }

private:
    CComPtr<IGraphBuilder> graph_builder_;
    CComPtr<IBaseFilter> rtp_receiver_filter_;
    CMediaType media_type_;
};


//�ڶ���RTP
class RTPSender2Container : public RTPSenderContainer
{
public:
    ~RTPSender2Container();

    dxStatus AddToGraph(IGraphBuilder *owner_graph, RtpType type, IPin *upstream_pin);
    dxStatus AddDest(NetAddr, RtpCallback *);
    dxStatus RemoveDest(NetAddr);
    dxStatus Clear();
    dxStatus PauseOverallSending(bool stop);
    unsigned long GetCount();

private:
    inline char *RtpTypeName();
    
private:
    CComPtr<IGraphBuilder> graph_builder_;
    CComPtr<IBaseFilter> rtp_sender_filter_;
    RtpType rtp_type_;
};

class RTPReceiver2Container : public RTPReceiverContainer
{
public:
    ~RTPReceiver2Container();

    virtual dxStatus AddFilter(IGraphBuilder *owner_graph, 
        CMediaType media_type, const char *rtp_id);
    virtual dxStatus SetSource(NetAddr addr);
    virtual dxStatus SetCallback(RtpCallback *callback);
    virtual dxStatus StopReceiving(bool stop);
    virtual IBaseFilter *GetFilter() { return rtp_receiver_filter_; }
    virtual IPin *GetOutputPin();
    virtual const CMediaType &GetType() { return media_type_; }

private:
    inline char *RtpTypeName();

private:
    CComPtr<IGraphBuilder> graph_builder_;
    CComPtr<IBaseFilter> rtp_receiver_filter_;
    CMediaType media_type_;
};


//��Ƶ��������
class AudioSourceContainer
{
public:
    virtual ~AudioSourceContainer() {};

    //��Ȼ����Ƶ�����豸�������������ȿ���Ҫ�õ�����豸
    virtual dxStatus AddFilter(IGraphBuilder *owner_graph, 
        std::string input_device, std::string output_device) = 0;

    virtual dxStatus SetFormat(unsigned int sample_rate, unsigned int channel_count, 
        unsigned int bits_per_sample, unsigned int latency) = 0;

    virtual IBaseFilter *GetFilter() = 0;
    virtual IPin *GetOutputPin() = 0;
    virtual const CMediaType &GetType() = 0;
};

class MicrophoneSourceContainer : public AudioSourceContainer
{
public:
    MicrophoneSourceContainer();
    ~MicrophoneSourceContainer();

    //����豸��Ч
    virtual dxStatus AddFilter(IGraphBuilder *owner_graph, 
        std::string input_device, std::string output_device);

    virtual dxStatus SetFormat(unsigned int sample_rate, unsigned int channel_count, 
        unsigned int bits_per_sample, unsigned int latency);

    virtual IBaseFilter *GetFilter() { return audio_source_filter_; }
    virtual IPin *GetOutputPin() { return audio_pin_out_; }
    virtual const CMediaType &GetType() { return candidate_type_; }

protected:
    CComPtr<IGraphBuilder> graph_builder_;
    CComPtr<IBaseFilter> audio_source_filter_;
    CComPtr<IPin> audio_pin_out_;
    CMediaType candidate_type_;
};

//ע�⣺AEC�Ǽ̳���MicrophoneSourceContainer�ģ���Ϊֻ��Ҫ��дAddFilter����
class AECSourceContainer : public MicrophoneSourceContainer
{
public:
    AECSourceContainer();
    ~AECSourceContainer();

    //����豸��Ч
    virtual dxStatus AddFilter(IGraphBuilder *owner_graph, 
        std::string input_device, std::string output_device);
};


//��Ƶ����������

class AudioEncoderContainer
{
public:
    virtual ~AudioEncoderContainer() {};

    virtual dxStatus AddFilter(IGraphBuilder *owner_graph, IPin *upstream_pin, CMediaType mt) = 0;
    virtual IBaseFilter *GetFilter() = 0;

    struct EncoderParam
    {
        //��ʱ�ò������õ��Ժ���˵
    };
    virtual dxStatus Config(EncoderParam &param) { return DX_WARN_NOT_IMPL; };
};

class SpeexEncoderContainer : public AudioEncoderContainer
{
public:
    ~SpeexEncoderContainer();

    virtual dxStatus AddFilter( IGraphBuilder *owner_graph, IPin *upstream_pin, CMediaType mt );
    virtual IBaseFilter * GetFilter() { return audio_encoder_filter_; };

private:
    CComPtr<IGraphBuilder> graph_builder_;
    CComPtr<IBaseFilter> audio_encoder_filter_;
};

class AACEncoderContainer : public AudioEncoderContainer
{
public:
    ~AACEncoderContainer();

    virtual dxStatus AddFilter( IGraphBuilder *owner_graph, IPin *upstream_pin, CMediaType mt );
    virtual IBaseFilter * GetFilter() { return audio_encoder_filter_; };

private:
    CComPtr<IGraphBuilder> graph_builder_;
    CComPtr<IBaseFilter> audio_encoder_filter_;
};


//��Ƶ����������

class AudioDecoderContainer
{
public:
    virtual ~AudioDecoderContainer() {}

    virtual dxStatus AddFilter(IGraphBuilder *owner_graph, IPin *upstream_pin, CMediaType mt) = 0;
    virtual IBaseFilter *GetFilter() = 0;
};

class SpeexDecoderContainer : public AudioDecoderContainer
{
public:
    ~SpeexDecoderContainer();

    dxStatus AddFilter(IGraphBuilder *owner_graph, IPin *upstream_pin, CMediaType mt);
    IBaseFilter *GetFilter() {return audio_decoder_filter_; }

private:
    CComPtr<IGraphBuilder> graph_builder_;
    CComPtr<IBaseFilter> audio_decoder_filter_;
};

class AACDecoderContainer : public AudioDecoderContainer
{
public:
    ~AACDecoderContainer();

    dxStatus AddFilter(IGraphBuilder *owner_graph, IPin *upstream_pin, CMediaType mt);
    IBaseFilter *GetFilter() {return audio_decoder_filter_; }

private:
    CComPtr<IGraphBuilder> graph_builder_;
    CComPtr<IBaseFilter> audio_decoder_filter_;
};


//video renderer

class VideoRendererContainer
{
public:
    virtual ~VideoRendererContainer() {};

    //��ֻ�����Ⱦ������������Ԥ�����ڣ�����ܣ���ʹ��EVRʱ������graphʱ���Զ�����һ������
    virtual dxStatus AddToGraph(IGraphBuilder *owner_graph, IPin *upstream_pin, bool use_evr) = 0;
    virtual dxStatus SetPreviewWindow(const VideoWindow &window) = 0;
};

//�Զ���Ƶ��Ⱦ��ͨ��IGraphBuilder::Render��ɣ��ʺ�ֻ��һ����ʾ���ڵ�����
class AutoVideoRendererContainer : public VideoRendererContainer
{
public:
    virtual ~AutoVideoRendererContainer();

    virtual dxStatus AddToGraph(IGraphBuilder *owner_graph, IPin *upstream_pin, bool use_evr);
    virtual dxStatus SetPreviewWindow(const VideoWindow &window);

protected:
    dxStatus SetVMRWindow(unsigned long preview_window, 
        long left, long top, long width, long height);
    dxStatus SetEVRWindow(unsigned long preview_window, 
        long left, long top, long width, long height);

private:
    CComPtr<IGraphBuilder> graph_builder_;
    CComPtr<IBaseFilter> evr_renderer_filter_;
};



//audio renderer
class AudioRendererContainer
{
public:
    virtual ~AudioRendererContainer(){};

    virtual dxStatus AddFilter(IGraphBuilder *owner_graph, IPin *upstream_pin, 
        std::string display_name) = 0;
};

//������Ƶrenderer�����ɣ�auto��ʾĬ�ϵ�renderer
class AutoAudioRendererContainer : public AudioRendererContainer
{
public:
    ~AutoAudioRendererContainer();

    dxStatus AddFilter(IGraphBuilder *owner_graph, IPin *upstream_pin, 
        std::string display_name);

private:
    CComPtr<IGraphBuilder> graph_builder_;
    CComPtr<IBaseFilter> audio_renderer_filter_;
};


//infinit_pin_tee container

class InfPinTeeContainer
{
public:
    ~InfPinTeeContainer();

    //name��ѡ�����ڹ��õ�filter�����ָ��һ��name������
    dxStatus AddToGraph(IGraphBuilder *owner_graph, IPin *upstream_pin, const wchar_t *name);
    IBaseFilter * GetFilter() { return inf_pin_tee_filter_; };
    IPin * GetUnconnectedPin();

private:
    CComPtr<IGraphBuilder> graph_builder_;
    CComPtr<IBaseFilter> inf_pin_tee_filter_;
    std::wstring filter_name_;
};


//file source container

class FileSourceContainer
{
public:
    ~FileSourceContainer();

    dxStatus AddFilter(IGraphBuilder *owner_graph, std::string file);
    IBaseFilter *GetFilter() { return file_source_filter_; }
    IPin *GetOutputPin() { return pin_out_; }

private:
    CComPtr<IGraphBuilder> graph_builder_;
    CComPtr<IBaseFilter> file_source_filter_;
    CComPtr<IPin> pin_out_;
};


}

#endif // !FILTER_CONTAINER_H
