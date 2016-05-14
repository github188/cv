#ifndef CV_TUNNEL_DISPATCHER_SERVICE_H
#define CV_TUNNEL_DISPATCHER_SERVICE_H

#include <QtCore/QtCore>

#include "IMediaControl.h"

typedef QMap<QString, IMediaControlPtr> MediaControllerMap;


class CvChannelDispatcherService : public QObject
{
    Q_OBJECT
public:
    CvChannelDispatcherService();
    ~CvChannelDispatcherService();

    /**
     * @brief ��ʼ��.���ڹ��캯����ִ�и��ӳ�ʼ������,��Ϊ��ֹ��������ʱ���ڸ���
     * @return 0Ϊ��ʼ���ɹ�������Ϊ������ֵ
     */
    int Initial();

public Q_SLOTS: // METHODS
    void ChannelStateChanged(const QString &mediaID, const QString &user_id, const QString &mediaType, int state);
    void CreateChannel(const QString &mediaID, const QString &mediaType, const QByteArray &param);
    void Destroy();
    // ModifyChannel����˵����cpp�ļ�
    void ModifyChannel(const QString &mediaID, const QString &mediaType, const int type, const QByteArray &param);
    void ReleaseChannel(const QString &mediaID, const QString &mediaType);
Q_SIGNALS: // SIGNALS
    void NofityChannelStateChanged(const QString &channel_id, const QString &user_id, 
        const QString &channel_type, int channel_state);

private: // PRIVATE METHODS
    void RestoreControllers();
    void ModifyMultipleChannel(const QString &mediaType, const int type, const QByteArray &param);
    int  ReinterpretActionType(const QString &mediaType, const int type);
    void ShowConsoleWindow(const bool is_show);
    
private:
    MediaControllerMap media_controllers_;
    BOOL debug_mode_; // �Ƿ�������̨����
    BOOL destroy_; //�����,��Destroy���ú����ֹ�κ���������
    QReadWriteLock lock_; //�˴�����һ����д��,Ŀ���Ǳ�֤Destroy����ʱ,��Ψһ���޸ı����,�������������ɵ���.
                          //����ʱ��,�κ��������������������ƵĲ��е���.
                          //��ע:����Qt DBus Adapter���ڵ��߳���(�˴�Ϊ����Ϣѭ��)����
                          //queued signal/slot connections��ʽ���е�,�ʼ�����ʵ������.
                          //Liaokz, 2013-11
    
};
#endif