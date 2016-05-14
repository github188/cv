/************************************************************************/
/* ���ļ�����graph�Ŀ�����Ϳ����๤����Ķ��⵼���ӿ�                  */
/* ʹ�ýӿڵ�Ŀ���ǣ����ؾ���������ʵ�֣��Լ��ٱ�©����Ҫ�ĳ�Ա��     */
/************************************************************************/

#ifndef I_GRAPH_CONTROLLER_H
#define I_GRAPH_CONTROLLER_H

#include "DxDefinitions.h"
#include "util\report\RtpStat.h" //����RTP�ص����涨��
#include "util\report\RecordStat.h" //����¼��ص����涨��
#include "util\report\FilePlayStat.h"
//�����⵼���ļ������������ļ����������Ҫ�õ���û�취

#include <string>
#include <vector>


#ifdef MSDX_EXPORTS
#define MSDXAPI __declspec(dllexport)
#else
#define MSDXAPI __declspec(dllimport)
#endif


namespace msdx //ע�����ж��嶼��msdx�����ռ���
{


//����Graph�������ӿ�
//��������Graph�Ĳ����ṹ
typedef struct
{
    struct Video
    {
        std::string crossbar; //�ɼ����豸DisplayName����ʵ�ɼ�������Ӧ��Ҳ�����ֶ������ˣ������Ǳ���
        long crossbar_physical_type; //�ɼ�����������
		std::string displayName;//���һ��displayName��Ϊѡ���豸��ʶ--hjf
        std::string device; //����ͷDevicePath
        dxVideoCodec codec;
        unsigned int frame_width;
        unsigned int frame_height;
        unsigned int fps;
        bool enable_hwcodec; //�Ƿ�ʹ��Ӳ�����ٱ���

        //Ԥ�����ھ����������
        VideoWindow preview;
    } video;

    struct Audio
    {
        std::string input_device;
        std::string output_device;
        dxAudioCodec codec;
        unsigned int sample_rate;
        unsigned int channels;
        unsigned int bitcount;
        unsigned int latency;
        bool enable_aec; //ʹ�û�������audio echo cancellation
    } audio;

} SendGraphParam;

class MSDXAPI ISendGraphController {
public:
    virtual ~ISendGraphController() {};
    //��ȡgraph����״̬
    virtual dxGraphStatus GetStatus() = 0;

    //��������ͣgraph
    virtual dxStatus Build(SendGraphParam &) = 0;
    virtual dxStatus Run() = 0;
    virtual dxStatus Stop() = 0;

    //��Ӻ�ɾ�����Ͷ˿�
    virtual dxStatus AddVideoDestination(const NetAddr &) = 0;
    virtual dxStatus DeleteVideoDestination(const NetAddr &) = 0;
    virtual dxStatus AddAudioDestination(const NetAddr &) = 0;
    virtual dxStatus DeleteAudioDestination(const NetAddr &) = 0;

    //����ֹͣ��������Ƶ����
    virtual dxStatus StopSendingVideo(bool stop) = 0;
    virtual dxStatus StopSendingAudio(bool stop) = 0;

    //RTPͳ�ƻص���û�зŵ�SendGraphParam�У��Ա��ڶ�̬����
    virtual dxStatus SetVideoRtpStatCallBack(void *obj, RtpStatCallback func) = 0;
    virtual dxStatus SetAudioRtpStatCallBack(void *obj, RtpStatCallback func) = 0;
};



//С������Graph�������ӿ�
typedef struct
{
    struct Video
    {
        NetAddr addr;
        dxVideoCodec codec;
        unsigned int frame_width;
        unsigned int frame_height;
        unsigned int fps;
        bool enable_hwcodec; //�Ƿ�ʹ��Ӳ�����ٱ���
    } video;

} SmallGraphParam;

class MSDXAPI ISmallGraphController {
public:
    virtual ~ISmallGraphController() {}
    //��ȡgraph����״̬
    virtual dxGraphStatus GetStatus() = 0;

    //��������ͣgraph
    virtual dxStatus Build(SmallGraphParam &) = 0;
    virtual dxStatus Run() = 0;
    virtual dxStatus Stop() = 0;

    //��Ӻ�ɾ�����Ͷ˿�
    virtual dxStatus AddVideoDestination(const NetAddr &) = 0;
    virtual dxStatus DeleteVideoDestination(const NetAddr &) = 0;

    //����ֹͣ��������Ƶ����
    virtual dxStatus StopSendingVideo(bool stop) = 0;
};



//����Graph�������ӿ�
typedef struct
{
    std::string graph_name; //һ������graph�ı�ʶ�������ϲ��user_id
    bool used_as_record; //�Ƿ���¼����;

    struct Video
    {
        NetAddr addr;
        dxVideoCodec codec;
        unsigned int frame_width;
        unsigned int frame_height;
        unsigned int fps;

        //��ʾ���ھ��
        VideoWindow preview;
    } video;

    struct Audio
    {
        NetAddr addr;
        std::string output_device;
        dxAudioCodec codec;
        unsigned int sample_rate;
        unsigned int channels;
        unsigned int bitcount;
    } audio;

    struct Record
    {
        long job_id;
        std::string file_path_name;
    } record;

} RecvGraphParam;

class MSDXAPI IRecvGraphController {
public:
    virtual ~IRecvGraphController() {}
    //��ȡgraph����״̬
    virtual dxGraphStatus GetStatus() = 0;

    virtual dxStatus Build(RecvGraphParam &) = 0;
    virtual dxStatus Run() = 0;
    virtual dxStatus Stop() = 0;

    //����ֹͣ��������Ƶ����
    virtual dxStatus StopReceivingVideo(bool stop) = 0;
    virtual dxStatus StopReceivingAudio(bool stop) = 0;

    //�����Ƶ����
    virtual dxStatus ChangeVideoWindow(VideoWindow window) = 0;

    //¼�����
    virtual dxStatus StartRecordingNewFile(long job_id, std::string new_file_name) = 0;
    virtual dxStatus SetRecordStatCallBack(void *obj, RecordStatCallback func) = 0;

    //RTPͳ�ƻص���û�зŵ�SendGraphParam�У��Ա��ڶ�̬����
    virtual dxStatus SetVideoRtpStatCallBack(void *obj, RtpStatCallback func) = 0;
    virtual dxStatus SetAudioRtpStatCallBack(void *obj, RtpStatCallback func) = 0;
};



//��Ƶ�ļ�����Graph�������ӿ�
typedef struct
{
    std::string graph_name; //һ������graph�ı�ʶ
    std::string sync_id; //���ڶ�·����Ƶͬ����ͬ�����ʶ
    std::vector<std::string> file_list; //Ҫ���ŵ��ļ��б�
    long long initial_delay; //��ʼ����ʱ���ӳ٣����ڶ�·��Ƶͬ��

    struct Video
    {
        NetAddr addr;
    } video;

    struct Audio
    {
        NetAddr addr;
    } audio;

} FilePlayGraphParam;

class MSDXAPI IFilePlayGraphController {
public:
    virtual ~IFilePlayGraphController() {}
    //��ȡgraph����״̬
    virtual dxGraphStatus GetStatus() = 0;

    virtual dxStatus Build(FilePlayGraphParam &) = 0;
    virtual dxStatus Run() = 0;
    virtual dxStatus Stop() = 0;

    //���Ų�����op_idӦ��1��ʼ
    virtual dxStatus Pause(unsigned long op_id) = 0;
    virtual dxStatus Resume(unsigned long op_id) = 0;
    //������ʣ�t����Ϊ1/10000000��
    virtual dxStatus SeekTo(unsigned long op_id, long long t) = 0;

    virtual dxStatus SetFilePlayStatCallBack(void *obj, FilePlayStatCallback func) = 0;
    //ͨ���ص���������Ƶ��ʽ
    virtual void ReportMediaFormat() = 0;
};



//�豸����Graph�������ӿ�
typedef struct
{
    //video/audioֻ�ܶ�ѡһ
    struct Video
    {
        std::string crossbar; //�ɼ����豸DisplayName
        long crossbar_physical_type; //�ɼ�����������
        std::string device; //����ͷdisplay_name
        unsigned int frame_width;
        unsigned int frame_height;
        VideoWindow preview;
    } video;

    struct Audio
    {
        //��Ƶ���Խ�֧��Ĭ�ϲ����ʺ�����
        std::string input_device; //�豸display_name,����
        std::string input_file; //�ļ�����ͬʱָ���������豸ʱ��������Ч
        std::string output_device;
    } audio;
} TestGraphParam;

class MSDXAPI ITestGraphController {
public:
    virtual ~ITestGraphController() {}

    virtual dxStatus Build(TestGraphParam &) = 0;
    virtual dxStatus Run() = 0;
    virtual dxStatus Stop() = 0;
};



//���ڴ���graph����Ĺ�����
class MSDXAPI GraphControllerFactory
{
private:
    GraphControllerFactory(void);
    ~GraphControllerFactory(void);

public:
    static GraphControllerFactory &GetInstance();

    ISendGraphController *CreateSendGraphController();
    ISmallGraphController *CreateSmallGraphController();
    IRecvGraphController *CreateRecvGraphController();
    ITestGraphController *CreateTestGraphController();
    IFilePlayGraphController *CreateFilePlayGraphController();

	ISendGraphController *CreateTestSendGraphController();
};


} // end namespace msdx

#endif
