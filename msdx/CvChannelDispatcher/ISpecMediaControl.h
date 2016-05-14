#ifndef ISPECMEDIACONTROL_H
#define ISPECMEDIACONTROL_H

//ʵ���ض�ý������صĲ���(Specific Media Control)
//��IMediaControl��������,�ⲿ��������䷽�����һЩͨ�ö���(�����)��,
//���ݲ�ͬý�������͵��ñ��ӿ���Ӧ������ɾ��嶯��.
class ISpecMediaControl
{
public:
    virtual ~ISpecMediaControl() {}

    virtual int SpecCreate(const QByteArray &param) = 0;
    virtual int SpecModify(const int type, const QByteArray &param) = 0;
    virtual int SpecRelease() = 0;
    virtual int SpecStateChanged(const int state) = 0;
    virtual int SpecRunningCheck() = 0;

    virtual bool isProxyValid() = 0;
};

#endif