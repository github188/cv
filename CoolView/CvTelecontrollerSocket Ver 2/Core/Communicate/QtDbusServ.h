/*
 * QtDbusͨ�ſ����������ļ�
 * V1.0.0.0	2012-12-15	DBus����������� By Lzy
 * V0.0.0.0	2011-12-14  �����ļ� By Lzy
 */

#ifndef QT_DBUS_SERV_H
#define QT_DBUS_SERV_H

#include <QObject>
#include <dbus/telecontroller/service/TelecontrollerAdaptor.h>
#include <dbus/telecontroller/client/TelecontrollerIf.h>
#include <dbus/telecontroller/common/TelecontrollerServiceCommon.h>
#include <dbus/core/client/CvCoreIf.h>
#include <dbus/conferenceRoom/client/ConferenceRoomIf.h>
#include <dbus/telecontroller/client/TelecontrollerIf.h>
#include <dbus/core/common/CvCoreServiceCommon.h>
#include <dbus/conferenceRoom/common/ConferenceRoomServiceCommon.h>

class CQtDBusServ : public QObject
{
	Q_OBJECT
public:
	//------����------
    static CQtDBusServ *GetInstance();//��ȡCQtDBusServ�����ľ�̬����
    static void ReleaseInstance();//�ͷ�CQtDBusServ�����ľ�̬����
	//------����------
	CvCoreIf *m_pCoreProxy;//coolview�������ӿ�
	ConferenceRoomIf *m_pConfRoomProxy;//�����ҳ������ӿ�
	TelecontrollerIf *m_pConfTeleProxy;//�����ҳ����telecontrollerר�ýӿ�
    TelecontrollerIf *m_pTeleProxy;//ָ���Լ���dbus�ӿ�

private:
    //------����------
    CQtDBusServ();
    ~CQtDBusServ();
    CQtDBusServ(const CQtDBusServ&){};
    //------����------
    static CQtDBusServ *m_pQtDbusServ;//CQtDBusServ�ľ�̬����
    TelecontrollerAdaptor *m_pDbusAdaptor;

public Q_SLOTS: // METHODS

	/*
     * �����������յ��ն�Dbus��Ϣʱ��������Ӧ��
     * @param int nInfoIndex,ͨ����Ϣ��ʶ
     * @param int nNotifyId,ͨ����Ϣ���ն���ң��������ID,��ֵ��ʾͨ������ң����
     * @param const QByteArray &rInputArray ͨ����Ϣ����
     * @return void
     */
	void TeleInfo(int nInfoIndex,int nNotifyId,const QByteArray &rInputArray);
};

#endif