/************************************************************************/
/* ��ʵ�ֿ��Կ���������ʵ��һ��״̬�������Ǵ�����������������Ư���ˣ� */
/* ֱ��switch���ˡ�                                                     */
/************************************************************************/

#include <assert.h>
#include <windows.h>

#include <QCoreApplication>
#include <QFile>
#include <QDataStream>

#include "BaseMediaController.h"
#include "util/ProcessManager.h"
#include "util/Backupmanager/BackupManager.h"
#include "log/Log.h"


#define PROC_TERMINATE_TIMEOUT 5 //���̽�����ʱ���,(s)
#define PROC_CHECK_INTERVAL 500 //���̼����,(ms)


//Impl of BaseMediaController methods

BaseMediaController::BaseMediaController(const QString &mediaID, const QString &type)
    : QObject(NULL)
    , media_id_(mediaID)
    , media_type_(type)
    , state_(Stopped)
    , pid_(0)
    , debug_mode_(false)
    , checker_timer_(this) //���ø�������ʹ������ͬ�߳���ִ��
    , proc_handle_(INVALID_HANDLE_VALUE)
    , recent_resume_times_(0)
{
#ifdef _DEBUG
    base_class_initialized = false;
#endif
}

BaseMediaController::~BaseMediaController()
{
    //thread_.quit();
    //thread_.wait();
}

void BaseMediaController::Initialize()
{
    LOG_PRINTF("Create controller for %s.", media_id_.toLocal8Bit().data());
#ifdef _DEBUG
    base_class_initialized = true;
#endif
    debug_mode_ = false;
    checker_timer_.setInterval(PROC_CHECK_INTERVAL);

    //connect signals
    connect(this, &BaseMediaController::CreateSignal,
        this, &BaseMediaController::HandleCreate);
    connect(this, &BaseMediaController::ModifySignal,
        this, &BaseMediaController::HandleModify);
    connect(this, &BaseMediaController::ReleaseSignal,
        this, &BaseMediaController::HandleRelease);
    connect(this, &BaseMediaController::StateChangedSignal,
        this, &BaseMediaController::HandleStateChanged);
    connect(this, &BaseMediaController::DestroySignal,
        this, &BaseMediaController::HandleDestroy);
    connect(&checker_timer_, &QTimer::timeout, 
        this, &BaseMediaController::HandleCheckerTimeout);

    //try to restore data if backup exist
    RestoreData();

    //set my slots to execute in a separate thread
    //moveToThread(&thread_);
    //thread_.start();
}

void BaseMediaController::SetDebugMode(bool debug)
{
    debug_mode_ = debug;
}

void BaseMediaController::Destroy()
{
    emit DestroySignal();
}

void BaseMediaController::HandleCreate( const QByteArray &param )
{
    assert(base_class_initialized);

    start_param_ = param;

    switch (state_)
    {
    case BaseMediaController::Stopped:
        SetState(StartPending);
        //���Դ�������
        if (DoCreate())
        {
            SetState(Started);
        } 
        else
        {
            SetState(Stopped);
        }
        break;

    case BaseMediaController::StartPending:
    case BaseMediaController::Started:
    case BaseMediaController::StopPending:
        //��Щ״̬�²�������δ����,ֻ���²�������
        LOG_PRINTF("Create process %s scheduled, update parameters.",
            media_id_.toLocal8Bit().data());
        SetState(StartPending);
        break;

    case BaseMediaController::Running:
        LOG_WARN("Create process %s while running, release the previous one.",
            media_id_.toLocal8Bit().data());
        DoRelease();
        SetState(StartPending);
        break;
    default:
        break;
    }
}

void BaseMediaController::HandleModify( const int type, const QByteArray &param )
{
    assert(base_class_initialized);
    
    switch (state_)
    {
    case BaseMediaController::Stopped:
    case BaseMediaController::StartPending:
    case BaseMediaController::Started:
    case BaseMediaController::StopPending:
        LOG_WARN("Try to modify %s before it is running.", media_id_.toLocal8Bit().data());
        break;
    case BaseMediaController::Running:
        LOG_PRINTF("Modify process %s", media_id_.toLocal8Bit().data());
        SpecModify(type, param);
        break;
    default:
        break;
    }
}

void BaseMediaController::HandleRelease()
{
    assert(base_class_initialized);

    switch (state_)
    {
    case BaseMediaController::StopPending:
    case BaseMediaController::Stopped:
        //�Ѿ�����ָֹͣ���ֹͣ,��ִ��
        break;
    case BaseMediaController::StartPending:
        //����������ʱ��Ҫ��ֹͣ,���л�Ϊ�ȴ�ֹͣ��ȡ���������������
        LOG_PRINTF("Release process %s in start pending, cancel scheduled start.", media_id_.toLocal8Bit().data());
        SetState(StopPending);
        break;
    case BaseMediaController::Started:
    case BaseMediaController::Running:
        //������������֮
        SetState(StopPending);
        DoRelease();
        break;
    default:
        break;
    }
}

void BaseMediaController::HandleStateChanged( int state )
{
    assert(base_class_initialized);
    
    switch (state_)
    {
    case BaseMediaController::Stopped:
        //δ��������,����״̬���֪ͨ
        LOG_WARN("Receive state change notify of %s, but it is not running.", 
            media_id_.toLocal8Bit().data());
        break;
    case BaseMediaController::StopPending:
    case BaseMediaController::StartPending:
    case BaseMediaController::Started:
    case BaseMediaController::Running:
        LOG_PRINTF("Process %s state change to %d.", 
            media_id_.toLocal8Bit().data(), state);
        SpecStateChanged(state);
        break;
    default:
        break;
    }
}

void BaseMediaController::HandleCheckerTimeout()
{
    if (INVALID_HANDLE_VALUE == proc_handle_)
    {
        //�������,ֻ���ǽ���δ�������Ѿ�ֹͣ
        LOG_ERROR("Check %s error. Invalid handle.", media_id_.toLocal8Bit().data());
        SetState(Stopped);
        checker_timer_.stop();
        return;
    }
    if (!ProcessManager::isRunning(proc_handle_))
    {
        //��ֹͣ
        OnExited();
    } 
    else if ((StopPending == state_ || StartPending == state_) && 
        PROC_TERMINATE_TIMEOUT <= stop_request_time_.secsTo(QTime::currentTime()))
    {
        //ֹͣ�����ѳ�ʱ��Debugʱ��ǿ�ƹرգ����ڵ��ԣ�
#ifndef _DEBUG
        LOG_WARN("Process %s(PID=%d) exit timeout. Try to terminate.", media_id_.toLocal8Bit().data(), pid_);
        ::TerminateProcess(proc_handle_, -1);
#endif // !_DEBUG
        stop_request_time_ = QTime::currentTime(); //���ó�ʱ���ʱ��
    }
    else if (Running == state_) {
        SpecRunningCheck();
    }
}

void BaseMediaController::OnExited()
{
    checker_timer_.stop();
    if (INVALID_HANDLE_VALUE != proc_handle_)
    {
        DWORD exitCode = 0;
        GetExitCodeProcess(proc_handle_, &exitCode);
        LOG_PRINTF("Process %s(PID=%d) exit. Return code: %d", 
            media_id_.toLocal8Bit().data(), pid_, exitCode);
        CloseHandle(proc_handle_);
        proc_handle_ = INVALID_HANDLE_VALUE;
    }
    else
    {
        LOG_ERROR("Process %s(PID=%d) exit, handle invalid.", 
            media_id_.toLocal8Bit().data(), pid_);
    }
    pid_ = 0;

    switch (state_)
    {
    case BaseMediaController::Stopped:
    case BaseMediaController::StopPending:
        SetState(Stopped);
        break;

    case BaseMediaController::Started:
    case BaseMediaController::Running:
        LOG_ERROR("Process %s exit unexpectedly. Try to resume.", 
            media_id_.toLocal8Bit().data());
        //Do the same work as in StartPending state

    case BaseMediaController::StartPending:
        if (DoCreate())
        {
            SetState(Started);
        } 
        else
        {
            SetState(Stopped);
        }

        break;
    default:
        break;
    }
}

void BaseMediaController::HandleDestroy()
{
    HandleRelease();
    //����controller�����ڲ��߳�������,��ɾ��ǰҪ�����ƶ������߳�
    //���򽫲�������Ϣѭ������controller��deleteLater����
    //moveToThread(QCoreApplication::instance()->thread());
    deleteLater();
}

void BaseMediaController::Create( const QByteArray &param )
{
    //controller���Լ����߳�������,���������̵߳ĵ�����Ҫʹ���ź�/�۹���.
    //֮���Բ������ֿ���ȥ�鷳�ķ�ʽ,��Ϊ��ȷ�����̲߳�������,�Ը������Ӧ
    //����DBus��������������.����,һ�����߳���ĳ��controller������,������
    //controller�Ĳ���Ҳ���޷��������,ֱ����ǰ���������.
    emit CreateSignal(param);
}

void BaseMediaController::Modify( const int type, const QByteArray &param )
{
    emit ModifySignal(type, param);
}

void BaseMediaController::Release()
{
    emit ReleaseSignal();
}

void BaseMediaController::StateChanged(const int state)
{
    emit StateChangedSignal(state);
}

//�˷��������̰߳�ȫ��.�������Ƿ�ֹCvChannelDispatcherService::Destroy������
HANDLE BaseMediaController::GetHandle()
{
    return proc_handle_;
}

//�˷��������̰߳�ȫ��.
QString BaseMediaController::GetMediaID()
{
    return media_id_;
}

void BaseMediaController::SetState(MediaProcState state)
{
    BackupData();
    state_ = state;
}

void BaseMediaController::BackupData()
{
    BackupManagerPtr backupMgr = BackupManager::Instance();
    if (NULL == backupMgr)
    {
        return;
    }
    QByteArray data;
    QDataStream stream(&data, QIODevice::ReadWrite);
    stream << state_ << start_param_ << (quint32)pid_; //TODO: ע��64bit����
    backupMgr->Backup(media_id_, media_type_, 0, data); //����ʽ����
}

void BaseMediaController::RestoreData()
{
    BackupManagerPtr backupMgr = BackupManager::Instance();
    if (NULL == backupMgr)
    {
        return;
    }
    //��ԭ��������
    QByteArray data;
    if (0 != backupMgr->FetchData(media_id_, 0, data)) //����ʽ����
    {
        if (isProxyValid())
        {
            LOG_WARN("Detect %s out of control.", 
                media_id_.toLocal8Bit().data());
        }
        return;
    }

    if (data.size())
    {
        QDataStream stream(data);
        quint32 temp_pid;
        stream >> (qint32&)state_ >> start_param_ >> temp_pid; //TODO: ע��64bit����
        pid_ = temp_pid;
        proc_handle_ = (pid_ ? ProcessManager::getHandleByPID(pid_) : INVALID_HANDLE_VALUE);
        LOG_INFO("Restore %s succeed: \n\tState:%d \n\tParam:%dBytes \n\tPID  :%d", 
            media_id_.toLocal8Bit().data(),
            state_,
            start_param_.size(),
            pid_);
    }

    //restore data according to state
    switch (state_)
    {
    case BaseMediaController::Stopped:
        break;
    case BaseMediaController::StartPending:
        if (isProxyValid()) //���̴���,���Խ�����
        {
            DoRelease();
        }
        //break;
    case BaseMediaController::StopPending:
        stop_request_time_ = QTime::currentTime(); //���赱ǰʱ����ָֹͣ���ʱ��
        //break;
    case BaseMediaController::Started:
    case BaseMediaController::Running:
        checker_timer_.start();
        break;
    default:
        break;
    }
}

bool BaseMediaController::DoCreate()
{
    if (0 > SpecCreate(start_param_))
    {
        //����ʧ��
        LOG_ERROR("Create process %s failed.", media_id_.toLocal8Bit().data());
        return false;
    }
    pid_ = ProcessManager::getPIDByHandle(proc_handle_);
    last_start_time_ = QTime::currentTime();
    checker_timer_.start();
    LOG_PRINTF("Create process %s(PID=%d) succeed. Start parameter data size is %d bytes.", 
        media_id_.toLocal8Bit().data(), pid_, start_param_.size());
    return true;
}

bool BaseMediaController::DoRelease()
{
    if (0 > SpecRelease())
    {
        //ʧ��,����ǿ�ƽ���
        if (INVALID_HANDLE_VALUE != proc_handle_)
        {
            LOG_WARN("Release process %s failed. Try to terminate.", media_id_.toLocal8Bit().data());
            ::TerminateProcess(proc_handle_, -1);
        }
    }
    else
    {
        LOG_PRINTF("Release process %s scheduled.", media_id_.toLocal8Bit().data());
    }
    stop_request_time_ = QTime::currentTime(); //���ó�ʱ���ʱ��
    return true;
}
