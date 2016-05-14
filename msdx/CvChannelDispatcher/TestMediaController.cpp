#include "TestMediaController.h"
#include "util\ProcessManager.h"
#include "log/Log.h"
#include "dbus/channel/type/testMedia/common/TestMediaServiceCommon.h"
#include "dbus/channelDispatcher/common/ChannelDispatcherServiceCommon.h"

TestMediaController::TestMediaController(const QString &mediaID)
    : BaseMediaController(mediaID, TEST_MEDIA_SERVICE_NAME)
{

}

TestMediaController::~TestMediaController()
{

}

void TestMediaController::Initialize()
{
    //����StreamMedia�ͻ��˽ӿڲ����ӵ�DBus����.
    //���۶�Ӧý���������Ƿ���ڽӿڷ������ɵ���;��ͨ��isValid�ж϶�Ӧý���������Ƿ����
    QString serviceName = getTestMediaServiceName(media_id_);
    QString objectPath = getTestMediaObjectPath(media_id_);
    CvTestMediaIf * ptr = new CvTestMediaIf(serviceName, 
        objectPath, 
        QDBusConnection::sessionBus(),
        this); //���ø�����������ͬ�߳�������
    media_if_.reset(ptr);

    //����ý��������״̬������ź����.�ѷ���,���ɼ�StreamMediaController
    /*connect(ptr, &CvTestMediaIf::RunningStateChanged,
        this, &TestMediaController::HandleStateChanged);*/

    //�����ø����ʼ������,��ȷ���´�����children���ڶ����߳���ִ��
    BaseMediaController::Initialize();
}

int TestMediaController::SpecCreate( const QByteArray &param )
{
    QString procName = QString(TEST_MEDIA_PROC_NAME) + QString(".exe");
    QString procArgs = media_id_;
    proc_handle_ = ProcessManager::startProcess(procName.toLocal8Bit().data(), 
        procArgs.toLocal8Bit().data(), debug_mode_);
    if (INVALID_HANDLE_VALUE == proc_handle_)
    {
        return -1;
    }

    return 0;
}

int TestMediaController::SpecModify( const int type, const QByteArray &param )
{
    if (!isProxyValid())
    {
        return -1;
    }
    media_if_->SetMediaInfo(type, param);
    return 0;
}

int TestMediaController::SpecRelease()
{
    if (!isProxyValid())
    {
        return -1;
    }
    media_if_->Destroy();
    return 0;
}

int TestMediaController::SpecStateChanged( const int state )
{
    if (!isProxyValid())
    {
        return -1;
    }

    TestMediaState currentState = static_cast<TestMediaState>(state);

    switch (currentState)
    {
    case TestMedia_NoInitial:
        if(media_id_ == FILE_TEST_MEDIA_ID)
        {
            media_if_->SetMediaInfo(TestMediaAction_InitialFileTest, start_param_);
            SetState(Running);
        }
        else if (media_id_ == DEVICE_TEST_MEDIA_ID)
        {
            media_if_->SetMediaInfo(TestMediaAction_InitialDeviceTest, start_param_);
            SetState(Running);
        }
        else if (media_id_ == VIDEO_PREVIEW_MEDIA_ID)
        {
            media_if_->SetMediaInfo(TestMediaAction_InitialPreviewTest, start_param_);
            SetState(Running);
        }
        else
        {
            LOG_ERROR("Unknown test media type for %s.", media_id_.toLocal8Bit().data());
            return -2;
        }
        break;
    case TestMedia_Running:
        break;
    case TestMedia_Pause:
        break;
    case TestMedia_Stop:
        break;
    case TestMedia_Destroyed:
        break;
    default:
        break;
    }
    return 0;
}

bool TestMediaController::isProxyValid()
{
    if (NULL == media_if_ || !media_if_->isValid())
    {
        return false;
    }
    return true;
}

QString TestMediaController::GetType()
{
    return TEST_MEDIA_SERVICE_NAME;
}
