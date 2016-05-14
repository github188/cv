#ifndef BASEMEDIACONTROLLER_H
#define BASEMEDIACONTROLLER_H

#include <windows.h>

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QThread>

#include "IMediaControl.h"
#include "ISpecMediaControl.h"

class BaseMediaController 
    : public QObject
    , public IMediaControl
    , public ISpecMediaControl
{
    Q_OBJECT

protected: // TYPES
    enum MediaProcState
    {
        Stopped = 0, //��ֹͣ���δ����
        StartPending,//�ȴ�����,��Ϊ֮ǰ�Ľ�����δ����
        Started,     //�Ѿ�����,�ȴ���ʼ��
        Running,     //�ѳ�ʼ��,����������
        StopPending  //�ѷ���ָֹͣ��,�ȴ�ֹͣ
    };

public: // METHODS
    BaseMediaController(const QString &mediaID, const QString &type);
    virtual ~BaseMediaController();

    virtual void SetDebugMode(bool debug);

    //��ʼ��.ע��Ӧ�����������е��û���Ĵ˷��������Ĭ�ϵĳ�ʼ��.
    virtual void Initialize();
    //ɾ������ʱ���ô˷���,����ֱ��delete�����QObject::deleteLater
    virtual void Destroy();

    //IMPL IMediaControl
    virtual void Create(const QByteArray &param) override;
    virtual void Modify(const int type, const QByteArray &param) override;
    virtual void Release() override;
    virtual void StateChanged(const int state) override;
    virtual HANDLE GetHandle() override;
    virtual QString GetMediaID() override;

Q_SIGNALS:
    void CreateSignal(const QByteArray &param);
    void ModifySignal(const int type, const QByteArray &param);
    void ReleaseSignal();
    void StateChangedSignal(const int state);
    void DestroySignal();

protected:
    void OnExited();
    void SetState(MediaProcState state);
    void BackupData();
    void RestoreData();
    inline bool DoCreate();
    inline bool DoRelease();

protected slots:
    //����ý����������ģ�巽��,����ɼ����Լ�״̬ת�Ƶ�Ĭ�϶���
    void HandleCreate(const QByteArray &param);
    void HandleModify(const int type, const QByteArray &param);
    void HandleRelease();
    void HandleDestroy();
    void HandleStateChanged(const int state);
    void HandleCheckerTimeout();

protected: // PROPERTIES

    const QString media_id_; //ý��������ID
    const QString media_type_; //����
    HANDLE proc_handle_; //���̾��,û��ʹ��QProcess,��Ϊ���ָ̻����޷��ָ�QProcess��Ϣ

    //���ָ̻�ʱ��Ҫ�ָ������ݶ���
    MediaProcState state_; //ý��������״̬,ȷ��ֻͨ��SetState�����޸�
    QByteArray start_param_; //��������
    unsigned long pid_; //����PID
    //End

    bool debug_mode_; //�Ƿ����ģʽ,�����ж��Ƿ���ʾ����̨


private: // PRIVATE PROPERTY

#ifdef _DEBUG
    bool base_class_initialized; //���ڼ���������Ƿ�����˻����Init����
#endif

    //QThread thread_;
    QTimer  checker_timer_; //���ڼ�����״̬

    QTime stop_request_time_; //�����ֹͣ��ʱ��,����ֹͣ��ʱ���
    QTime last_start_time_; //���һ���������̵�ʱ��,����һ������һ���ֹ�����Ƴ��Իָ�
    int recent_resume_times_; //��������������̵Ĵ���
};

#endif // BASEMEDIACONTROLLER_H
