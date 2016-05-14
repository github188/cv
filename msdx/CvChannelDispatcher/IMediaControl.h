#ifndef IMEDIACONTROL_H
#define IMEDIACONTROL_H

#include <windows.h>

#include <QSharedPointer>

//ʵ�ֽӿ���ʵ�ֿ���ý�������̵Ļ�������
class IMediaControl
{
public:
    virtual ~IMediaControl() {}

    virtual void SetDebugMode(bool debug) = 0;
    
    //ý�������̻�������
    virtual void Create(const QByteArray &param) = 0;
    virtual void Modify(const int type, const QByteArray &param) = 0;
    virtual void Release() = 0;
    virtual void StateChanged(const int state) = 0;

    virtual HANDLE GetHandle() = 0;
    virtual QString GetMediaID() = 0;
    virtual QString GetType() = 0;
};

typedef QSharedPointer<IMediaControl> IMediaControlPtr;

#endif