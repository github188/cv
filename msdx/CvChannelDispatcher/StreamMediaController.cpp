#include "StreamMediaController.h"

#include <psapi.h>
#include <time.h>

#include "util\ProcessManager.h"
#include "log/Log.h"
#include "dbus/channel/type/streamedMedia/common/StreamedMediaServiceCommon.h"

StreamMediaController::StreamMediaController(const QString &mediaID)
    : BaseMediaController(mediaID, STREAMED_MEDIA_SERVICE_NAME)
{

}

StreamMediaController::~StreamMediaController()
{

}

void StreamMediaController::Initialize()
{
    //����StreamMedia�ͻ��˽ӿڲ����ӵ�DBus����.
    //���۶�Ӧý���������Ƿ���ڽӿڷ������ɵ���;��ͨ��isValid�ж϶�Ӧý���������Ƿ����
    QString serviceName = getStreamedMediaServiceName(media_id_);
    QString objectPath = getStreamedMediaObjectPath(media_id_);
    CvStreamedMediaIf * ptr = new CvStreamedMediaIf(serviceName, 
        objectPath, 
        QDBusConnection::sessionBus(),
        this); //���ø�����������ͬ�߳�������
    media_if_.reset(ptr);

    //����ý��������״̬������ź����,���Ƽ��˷�ʽ!
    //����:����ֱ�ӵ���ChannelDispatcher��ChannelStateChanged���������Ƿ��͸��ź�,����Ϊ
    //Ŀǰ����ChannelDispatcherΪDBus����(��dbus\services�������ļ�),��ChannelDispatcher
    //������,��������ź�,ChannelDispatcher���ᱻ����,�������䷽��DBus���Զ���������
    //Liaokz, 2013-11
    /*connect(ptr, &CvStreamedMediaIf::RunningStateChanged,
            this, &StreamMediaController::HandleStateChanged);*/

    //�����ø����ʼ������,��ȷ���´�����children���ڶ����߳���ִ��
    BaseMediaController::Initialize();
}

int StreamMediaController::SpecCreate( const QByteArray &param )
{
    QString procName = QString(STREAMED_MEDIA_PROC_NAME) + QString(".exe");
    QString procArgs = media_id_;
    proc_handle_ = ProcessManager::startProcess(procName.toLocal8Bit().data(), 
        procArgs.toLocal8Bit().data(), debug_mode_);
    if (INVALID_HANDLE_VALUE == proc_handle_)
    {
        return -1;
    }

    init_memory_usage_ = 0;
    return 0;
}

int StreamMediaController::SpecModify( const int type, const QByteArray &param )
{
    if (!isProxyValid())
    {
        return -1;
    }
    media_if_->SetMediaInfo(type, param);
    return 0;
}

int StreamMediaController::SpecRelease()
{
    if (!isProxyValid())
    {
        return -1;
    }
    media_if_->Destroy();
    return 0;
}

int StreamMediaController::SpecStateChanged( const int state )
{
    if (!isProxyValid())
    {
        return -1;
    }

    StreamedMediaState currentState = static_cast<StreamedMediaState>(state);

    switch (currentState)
    {
    case StreamedMedia_NoInitial:
        if(isSendMediaID(media_id_)) // send
        {
            LOG_PRINTF("Action_InitialSendGraph %s", media_id_.toLocal8Bit().data());
            media_if_->SetMediaInfo(Action_InitialSendGraph , start_param_);
        }
        else if(isSmallVideoMediaID(media_id_)) // small
        {
            LOG_PRINTF("Action_InitialSmallVideoGraph %s", media_id_.toLocal8Bit().data());
            media_if_->SetMediaInfo(Action_InitialSmallVideoGraph, start_param_);
        }
        else if (isFilePlayMediaID(media_id_)) // file play
        {
          LOG_PRINTF("Action_InitialFilePlayGraph %s", media_id_.toLocal8Bit().data());
          media_if_->SetMediaInfo(Action_InitialFilePlayGraph, start_param_);
        }
        else // recv
        {
            LOG_PRINTF("Action_InitialRecvGraph %s", media_id_.toLocal8Bit().data());
            media_if_->SetMediaInfo(Action_InitialRecvGraph , start_param_);
        }
        SetState(Running);
        init_time_ = time(nullptr);
        break;
    case StreamedMedia_Running:
        break;
    case StreamedMedia_Pause:
        break;
    case StreamedMedia_Stop:
        break;
    case StreamedMedia_Destroyed:
        break;
    default:
        break;
    }

    return 0;
}

bool StreamMediaController::isProxyValid()
{
    //QTֻ�᷵�ش���ʱ��״̬����ÿ�ζ�Ҫ�½�һ���ټ���׼ȷ
    QString serviceName = getStreamedMediaServiceName(media_id_);
    QString objectPath = getStreamedMediaObjectPath(media_id_);
    CvStreamedMediaIf * ptr = new CvStreamedMediaIf(serviceName, 
        objectPath, 
        QDBusConnection::sessionBus(),
        this);
    return ptr->isValid();
}

QString StreamMediaController::GetType()
{
    return STREAMED_MEDIA_SERVICE_NAME;
}

int StreamMediaController::SpecRunningCheck()
{
    if (Running != state_) { //ȷ����running״̬��
        return 0;
    }
    if (time(nullptr) - init_time_ < 60) { //����һ���Ӻ��ڴ�ռ����ֵ��׼ȷ
        return 0;
    }

    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(proc_handle_, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        if (0 == init_memory_usage_) {
            init_memory_usage_ = pmc.PrivateUsage;
            LOG_INFO("Process %s initial memory usage: %d MB", 
                media_id_.toLocal8Bit().data(), init_memory_usage_/(1<<20));
        } 
        else if (pmc.PrivateUsage > init_memory_usage_ && //ע����ֵ��unsigned long��ֱ�Ӽ����ܻ��ɺܴ�����Ӷ���ɱ
                 pmc.PrivateUsage - init_memory_usage_ > kMemoryIncreaseLimit) {
            //���ڴ���������һ����ʱ����Ϊ�����ڴ�й¶��ɱ�����̣�
            //������BaseStreamControllerʵ�֣�״̬�ָ���CvStreamMedia�����Լ����
            ::TerminateProcess(proc_handle_, -1);
            LOG_WARN("Process %s memory usage: %d MB, init: %d MB, kill it", 
                media_id_.toLocal8Bit().data(), pmc.PrivateUsage/(1<<20), init_memory_usage_/(1<<20));
        }
    }
    return 0;
}
