/*
 * QtDbusͨ�ſ����ඨ���ļ�
 * V1.0.0.0	2012-12-15	DBus����������� By Lzy
 * V0.0.0.0	2011-12-14  �����ļ� By Lzy
 */

#include "QtDbusServ.h"
#include <dbus/core/common/CvCoreServiceCommon.h>
#include <dbus/conferenceRoom/ConfRoomClient.h>
#include "NMessageProcess.h"

CQtDBusServ *CQtDBusServ::m_pQtDbusServ = NULL;

/*
 * ������������̬����ȡCQtDBusServ�����ľ�̬����
 * @return CQtDBusServ *,����CQtDBusServ�ĵ�������
 */
CQtDBusServ *CQtDBusServ::GetInstance()
{
    if (CQtDBusServ::m_pQtDbusServ == NULL)
        CQtDBusServ::m_pQtDbusServ = new CQtDBusServ();
    return CQtDBusServ::m_pQtDbusServ;
}

/*
 * ������������̬���ͷ�CQtDBusServ�����ľ�̬����
 * @return void
 */
void CQtDBusServ::ReleaseInstance()
{
    if (CQtDBusServ::m_pQtDbusServ != NULL)
    {
        delete CQtDBusServ::m_pQtDbusServ;
        CQtDBusServ::m_pQtDbusServ = NULL;
    }
}

/*
 * ��˽�С����캯������ʼ��Dbus��Ϣ�շ�����
 */
CQtDBusServ::CQtDBusServ()
{
	QDBusConnection oConnect = QDBusConnection::sessionBus();
    this->m_pCoreProxy = new CvCoreIf(CVCORE_SERVICE_NAME,CVCORE_SERVICE_OBJECT_PATH,oConnect);
    this->m_pConfRoomProxy = new ConferenceRoomIf(CONF_ROOM_SERVICE_NAME,CONF_ROOM_SERVICE_OBJECT_PATH,oConnect);
	this->m_pConfTeleProxy = new TelecontrollerIf(_GetTelecontrollerServiceName("confRoom"),_GetTelecontrollerObjectPath("confRoom"),oConnect);
	this->m_pTeleProxy = new TelecontrollerIf(_GetTelecontrollerServiceName("sock"),_GetTelecontrollerObjectPath("sock"),oConnect);
    this->m_pDbusAdaptor = new TelecontrollerAdaptor(this);
	oConnect.registerService(QString(TELECONTROLLER_SERVICE_NAME) + ".sock");
	oConnect.registerObject(QString(TELECONTROLLER_SERVICE_OBJECT_PATH) + "/sock" ,this);
}

/*
 * ��˽�С������������ͷ���Դ
 */
CQtDBusServ::~CQtDBusServ()
{
	delete this->m_pDbusAdaptor;
    delete this->m_pConfRoomProxy;
    delete this->m_pCoreProxy;
	delete this->m_pConfTeleProxy;
}

/*
 * ����������DBus�ۺ��������յ��ն�Dbus��Ϣʱ��������Ӧ��
 * @param int nInfoIndex,ͨ����Ϣ��ʶ
 * @param int nNotifyId,ͨ����Ϣ���ն���ң��������ID,��ֵ��ʾͨ������ң����
 * @param const QByteArray &rInputArray ͨ����Ϣ����
 * @return void
 */
void CQtDBusServ::TeleInfo(int nInfoIndex,int nNotifyId,const QByteArray &rInputArray)
{
	CNMessageProcess oNMessageProcess;
    oNMessageProcess.MainProcess(nInfoIndex,nNotifyId,rInputArray);
}
