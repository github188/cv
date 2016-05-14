/*********************************************************************
 �ع�Graphʱ��Ϊ�˽��Ķ�������msdxģ���ڲ���û�иĶ�����API��
 ��Ϊ��ʷԭ������ܶ�API���ѷ���������ʱ�޷�ʹ���ˡ����ΪDeprecated
 ��API���Ѿ�ȷ������ʹ�õġ������ΪNot implementated��API����ζ�����
 �ڻ������壬�������ܻ��������á�
                                          ���� Liaokz�� 2014-11
**********************************************************************/

#include "stdafx.h"
#include "assert.h"

#include <string> //for wstring

#include "Exportutil.h"
#include "Util.h"
#include "log\Log.h"
#include "msdx\config.h"
#include "IGraphController.h"


//������ExportUtil_Common.cpp��
extern std::wstring Crossbar[20];
extern int NumberOfCrossbar;
extern HRESULT ListAudioCaptureDevice(std::wstring *pWStringArray, int &NumOfDevices);
extern std::wstring AudioDeviceList[20];
extern int NumberOfAudioDevice;
extern HRESULT ListVideoCrossbar(std::wstring * pWStringArray, int & NumOfDevices);
extern int GetAudioSourceDisplayNameFromFriendlyName(std::string friendly_name, std::string &display_name);

//������Ƶ�ĺ�����Ȼ�Ƿ��ͺͽ��չ��õģ�Ҫ��ExportUtil_Common.cpp������send_graph_param�����Բ�����static
msdx::SendGraphParam send_graph_param;
static msdx::ISendGraphController *send_graph_controller;

//��һ������������ά�������ֲ��������Ա���ɳ�ʼ�����˳��Զ�����
class SendGraphControllerHelper {
public:
    SendGraphControllerHelper()
    {
        SetDefaultParam();
        send_graph_controller = nullptr;
    }

    ~SendGraphControllerHelper()
    {
        if (send_graph_controller)
        {
            delete send_graph_controller;
            send_graph_controller = nullptr;
        }
    }

    void SetDefaultParam()
    {
        send_graph_param.video.crossbar_physical_type = msdx::PhysConn_Auto;
        send_graph_param.video.codec = msdx::DX_VIDEO_CODEC_H264;
        send_graph_param.video.frame_width = 1280;
        send_graph_param.video.frame_height = 720;
        send_graph_param.video.fps = 30;
        send_graph_param.video.enable_hwcodec = true;
        send_graph_param.video.preview.window_handle = 0;
        send_graph_param.video.preview.position_left = 0;
        send_graph_param.video.preview.position_top = 0;
        send_graph_param.video.preview.width = 0;
        send_graph_param.video.preview.height = 0;

        send_graph_param.audio.codec = msdx::DX_AUDIO_CODEC_SPEEX;
        send_graph_param.audio.sample_rate = 16000;
        send_graph_param.audio.channels = 1;
        send_graph_param.audio.bitcount = 16;
        send_graph_param.audio.latency = 40;
        send_graph_param.audio.enable_aec = true;
    }
};

static SendGraphControllerHelper send_graph_helper;


/****** Implemetation of API ******/

void MSDXAPI msdx_set_default_config()
{
    if (send_graph_controller != nullptr)
    {
        LOG_WARN("Init send graph param after graph controller created.");
    }

    send_graph_helper.SetDefaultParam();
}

int MSDXAPI msdx_initial()
{
    //��ʼ��COM
    HRESULT hr;
    hr = CoInitialize(NULL);

    msdx_set_default_config();
    if (FAILED(hr))
    {
        LOG_ERROR("assertion failed.");
        return -1;
    }

    setlocale(LC_CTYPE, ""); //���ڿ��ַ�����wprintf

    return 0;
}


void MSDXAPI msdx_uninitial()
{
    //����send graph
    msdx_destroy_send_graph();
    //����recv graph
    msdx_destroy_recv_graphs();
    //����small graph
    msdx_destroy_smallvideo_graphs();

    //����COM
    CoUninitialize();
}

//crossbar�ǲɼ����������Ѿ����������ˣ�ֻ��VGA�ɼ���Ҫ
void MSDXAPI msdx_config_set_video_device(
    const char *device, /*����ͷ���豸��ʾ�����������豸·��*/
    const char *crossbar, /*crossbar�豸����*/
    const char *cb_input) /*crossbar��������*/
{
    if (device) send_graph_param.video.device = device;

    //����crossbar�豸��
    if (crossbar) 
    {
        send_graph_param.video.crossbar = crossbar;

        //�����Ǿɵķ�ʽ����ǰ�ϲ㴫�����friendlyName��Ҫת��displayName...
        /*int i = 0;
        ListVideoCrossbar(Crossbar, NumberOfCrossbar);
        for (; i < NumberOfCrossbar; i+=2) 
        {
            if (Crossbar[i] == Util::str2wstr(crossbar)) //ż��������friendlyName�������ŵ������Ƕ�ӦdisplayName
            {
                send_graph_param.video.crossbar = Util::wstr2str(Crossbar[i+1]);
                break;
            }
        }
        if (i == NumberOfCrossbar) {
            LOG_ERROR("Ignore invalid crossbar friendlyName: %s", crossbar);
            return;
        }*/
    }

    using namespace msdx;

    //���crossbar�ǿ�,������pin�����ʹ���
    if(cb_input && !send_graph_param.video.crossbar.empty())
    {
        //reference: Working with Crossbars
        //http://msdn.microsoft.com/en-us/library/windows/desktop/dd390991(v=vs.85).aspx
        if(strcmp(cb_input, "Tuner") == 0) 
            send_graph_param.video.crossbar_physical_type = PhysConn_Video_Tuner;
        else if(strcmp(cb_input, "Composite") == 0)
            send_graph_param.video.crossbar_physical_type = PhysConn_Video_Composite;
        else if(strcmp(cb_input, "SVideo") == 0)
            send_graph_param.video.crossbar_physical_type = PhysConn_Video_SVideo;
        else 
            send_graph_param.video.crossbar_physical_type = PhysConn_Auto; //����������������д������ֵ���Զ���ģ�����ϵͳ�����Ŷ
    }
    else
    {
        send_graph_param.video.crossbar_physical_type = PhysConn_Auto;
    }
}

void MSDXAPI msdx_config_set_audio_format(int SampleRate, int Channels, int BitsPerSample)
{
    send_graph_param.audio.sample_rate = SampleRate;
    send_graph_param.audio.channels = Channels;
    send_graph_param.audio.bitcount = BitsPerSample;
}

/* 
��ȡ����Graph��״̬
@return
    *0:initial
    *1:running
    *2:stop
    *3:pause
**/
int MSDXAPI msdx_send_get_graph_state()
{
    if (send_graph_controller == nullptr) return -1;
    return static_cast<int>(send_graph_controller->GetStatus());
}

/*@ return Graph ID:-1 indicate there is no send graph*/
int MSDXAPI msdx_get_send_graph()
{
    if(send_graph_controller == nullptr) return -1;
    return 0;
}

//����¼��������ʵ�ʴ���graph
int MSDXAPI msdx_create_send_graph( bool enable_small_video, bool disable_hwcodec)
{
    if(send_graph_controller != nullptr) return -1;

    send_graph_param.video.enable_hwcodec = !disable_hwcodec;
    return 0;
}

int MSDXAPI msdx_pause_send_graph()
{
    //TODO: Not implemented
    return 0;
}

int MSDXAPI msdx_resume_send_graph()
{
    //TODO: Not implemented
    return 0;
}

int MSDXAPI msdx_run_send_graph()
{
    if(send_graph_controller == nullptr) return -1;
    return static_cast<int>(send_graph_controller->Run());
}

int MSDXAPI msdx_stop_send_graph()
{
    if(send_graph_controller == nullptr) return -1;
    return static_cast<int>(send_graph_controller->Stop());
}

int MSDXAPI msdx_destroy_send_graph()
{
    if(send_graph_controller) 
    {
        send_graph_controller->Stop();
        delete send_graph_controller;
        send_graph_controller = nullptr;
    }
    return 0;
}

//by dsh �ģ����Ӳ����ʵȲ�����10-07-30
int MSDXAPI msdx_set_send_audio_codec(
    const char *codec_MIME, 
    const int sample_rate, 
    const int channels, 
    const int bits_per_sample, 
    const bool aec_enable)
{
    //Codec
    if (strcmp(codec_MIME, MSDX_CONF_AUDIO_CODEC_SPEEX) == 0)
        send_graph_param.audio.codec = msdx::DX_AUDIO_CODEC_SPEEX;
    else if (strcmp(codec_MIME, MSDX_CONF_AUDIO_CODEC_AAC) == 0)
        send_graph_param.audio.codec = msdx::DX_AUDIO_CODEC_AAC;
    else
        send_graph_param.audio.codec = msdx::DX_AUDIO_CODEC_UNKNOWN;

    //Other
    send_graph_param.audio.sample_rate = sample_rate;
    send_graph_param.audio.channels = channels;
    send_graph_param.audio.bitcount = bits_per_sample;
    send_graph_param.audio.enable_aec = aec_enable;

    return 0;
}

int MSDXAPI msdx_set_send_video_vodec(const char * codecMIME, const int width, const int height, const int fps)
{
    send_graph_param.video.codec = msdx::DX_VIDEO_CODEC_H264; //Ŀǰ��֧��H264
    send_graph_param.video.frame_width = width;
    send_graph_param.video.frame_height = height;
    send_graph_param.video.fps = fps <= 5 ? 5 : fps; //ԭ���ϲ�����С��5֡�ĸ�ʽ

	return 0;
}

int MSDXAPI msdx_set_send_screen_codec(char * codecMIME)
{
    //Deprecated
    return 0;
}

int MSDXAPI msdx_set_screen_filter_config(int left, int up, int right, int down)
{
    //Deprecated
    return 0;
}

int MSDXAPI msdx_add_send_address(const char * audio_ip, const int audio_port, 
                                  const char * video_ip, const int video_port,
                                  const bool small_video) //С��������Ч
{
    if(send_graph_controller == nullptr) return -1;

    int ret = 0;
    msdx::NetAddr addr;

    if (audio_port != 0) {
        addr.ip = audio_ip;
        addr.port = audio_port;
        ret = static_cast<int>(send_graph_controller->AddAudioDestination(addr));
        if (ret < 0) return ret;
    }

    if (video_port != 0) {
        addr.ip = video_ip;
        addr.port = video_port;
        ret = static_cast<int>(send_graph_controller->AddVideoDestination(addr));
    }
    return ret;
}


int MSDXAPI msdx_add_send_address2(const char * audioIP, const int audioport, const char * videoIP,
                                   const int videoport, const char *ScreenIP, const int ScreenPort)
{
    //Deprecated
    return 0;
}

int MSDXAPI msdx_delete_send_address(const char * audio_ip, const int audio_port, 
                                     const char * video_ip, const int video_port)
{
    if(send_graph_controller == nullptr) return -1;

    int ret = 0;
    msdx::NetAddr addr;

    addr.ip = audio_ip;
    addr.port = audio_port;
    ret = static_cast<int>(send_graph_controller->DeleteAudioDestination(addr));
    //if (ret < 0) return ret; //��������ɾ����Ƶ

    addr.ip = video_ip;
    addr.port = video_port;
    ret = static_cast<int>(send_graph_controller->DeleteVideoDestination(addr));
    return ret;
}

int MSDXAPI msdx_delete_send_address2(const char * audioIP, const int audioport)
{
    //Deprecated
    return 0;
}

int MSDXAPI msdx_delete_screen_send_address( const char *ScreenIP, const int ScreenPort)
{
    //Deprecated
    return 0;
}

//���ڽ�����ʱ�ã�CoolView�б��ػ�����һ�����նˣ����������Ԥ����
//����Ҳû�п��Ƕ�̬������ڵ������recv���п��Ƕ�̬������ڣ��ο�recv��ʵ��
int MSDXAPI msdx_set_send_preview_window(HWND WndHandle, long left, long top, long width, long height, int fullScreen)
{
    send_graph_param.video.preview.window_handle = (unsigned long)WndHandle;
    send_graph_param.video.preview.position_left = left;
    send_graph_param.video.preview.position_top = top;
    send_graph_param.video.preview.width = width;
    send_graph_param.video.preview.height = height;
    return 0;
}

int MSDXAPI msdx_connect_send_graph()
{
    //�ȼ��graph�Ƿ����
    if(send_graph_controller != nullptr) return -1;

    //����graph controller
    send_graph_controller = 
        msdx::GraphControllerFactory::GetInstance().CreateTestSendGraphController();//CreateSendGraphController();HJF 2015.12.15
    if(send_graph_controller == nullptr)
    {
        LOG_ERROR("Create graph controller failed.");
        return -1;
    }

    //��������û���������Ҫ�������豸FriendlyNameתΪDisplayName���ⶼ����ʷ������
    //Ϊ�˼��ݸ�����ʷ���⣬��Ҳ����ƴ�ĶԲ��ԣ�
    if (!send_graph_param.audio.enable_aec) 
    {
        if (GetAudioSourceDisplayNameFromFriendlyName(
            send_graph_param.audio.input_device, send_graph_param.audio.input_device) < 0)
            return -1;
    }

    return static_cast<int>(send_graph_controller->Build(send_graph_param));
}

//ֱ��������������
int MSDXAPI msdx_set_send_video_quantizer(int quantizer)
{
    //TODO: Not implemented
    return 0;
}

//���ݵ�ǰ���ʺ�Ŀ�����ʵ�������
void MSDXAPI msdx_adapt_encode_rate(float currentRate, float destRate)
{
    //TODO: Not implemented
}

int MSDXAPI msdx_set_Rtcp_Recv_Filter(char *ListenIP, int ListenPort)
{
    //TODO: Not implemented
    return 0;
}

/**
* @author zhenHua.sun
* @date 2010-08-06
* @brief �趨��Ƶ�������Ĺ���״̬
* @param work ���Ϊtrue�����������������������������
* @return 0��ʾ��������
*/
int MSDXAPI msdx_control_audio_encoder( bool work )
{
    if(send_graph_controller == nullptr) return -1;
    return static_cast<int>(send_graph_controller->StopSendingAudio(!work));
}
/**
* @author zhenHua.sun
* @date 2010-08-06
* @brief �趨��Ƶ�������Ĺ���״̬
* @param work ���Ϊtrue�����������������������������
* @return 0��ʾ��������
*/
int MSDXAPI msdx_control_video_encoder( bool work )
{
    if(send_graph_controller == nullptr) return -1;
    return static_cast<int>(send_graph_controller->StopSendingVideo(!work));
}


int MSDXAPI msdx_delete_Rtcp_Recv_address(char *ListenIP, int ListenPort)
{
    //Deprecated
    return 0;
}

void MSDXAPI msdx_send_set_video_rtpstatcallback( void *obj, void *func )
{
    assert(send_graph_controller != nullptr); //��Ҫ�ȵ���msdx_connect_send_graph()
    if(send_graph_controller == nullptr) return;
    send_graph_controller->SetVideoRtpStatCallBack(obj, 
        static_cast<RtpStatCallback>(func));
}

void MSDXAPI msdx_send_set_audio_rtpstatcallback( void *obj, void *func )
{
    assert(send_graph_controller != nullptr); //��Ҫ�ȵ���msdx_connect_send_graph()
    if(send_graph_controller == nullptr) return;
    send_graph_controller->SetAudioRtpStatCallBack(obj, 
        static_cast<RtpStatCallback>(func));
}

//by dsh 10-08-06
void MSDXAPI msdx_set_audio_link_state(bool state)
{
    //Deprecated
}


int MSDXAPI msdx_send_set_video_dscp(const int dscp )
{
	//TODO: Not implemented
    return 0;
}

int MSDXAPI msdx_send_set_audio_dscp(const int dscp )
{
	//TODO: Not implemented
    return 0;
}

/**
	*	@brief	ͳ�ƴ����������
	 * @param transtat ����һ��TranStat�ṹָ�������ͳ�ƽ��ֵ�����浽�ñ�����;	TranStat�ṹ�Ķ�����μ�RtpStat.h
	 * @see RtpStat.h
	 *	@return 0��ʾͳ�Ƴɹ���������ͳ��ʧ�ܻ����rtpware��Ӧ�ĺ���ʧ��
	 */
int  msdx_compute_global_transtat( void * transtat )
{
    //TODO: Not implemented
    return 0;
}


void MSDXAPI msdx_send_enable_adaptive_coderate( bool enable )
{
	//TODO: Not implemented
}


int MSDXAPI msdx_send_cut_file()
{
	//Deprecated
	return 0;
}

/******* Test Code *******/

int MSDXAPI testRoute()
{
    //Deprecated
    return 0;
}
