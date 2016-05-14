/*
 * ������Ϣ����ģ�鹫�����ֵķ�װ �����ļ�
 * 2011-12-15   V1.0.0.0    By Lzy
 */

#include "CommonMessageProcess.h"

namespace Common_Message_Space
{
    /*
     * �������������µ�Ȩ����Ϣ��������ң��������ͨ��
     * @param const char *pszSipAccount,��ÿ���Ȩ�޵�Sip�ʺ�������ָ�������Ч
     * @param int nNotifyIndex,��ÿ���Ȩ�޵�ң����������ͨ������������
     * @return void
     */
    void SetNewMonitor(const char *pszSipAccount,int nNotifyIndex)
    {
        //����Ȩ����Ϣ
        CDataManager *pDataManager = CDataManager::GetInstance();
        pDataManager->SetDataString(pszSipAccount,false,"MonitorSip");
        const char *pszMonitor = pDataManager->GetDataString("MonitorSip");
        if (pszMonitor != NULL && strcmp(pszMonitor,pszSipAccount) == 0)
            pDataManager->SetDataLong(nNotifyIndex,false,"MonitorNotifyIndex");
        //����ͨ����Ϣ
        Json::Value oRoot;
        oRoot["info_type"] = Json::Value(TELE_CurrentController);
        oRoot["usersip"] = Json::Value(pszSipAccount);
        Json::FastWriter oWriter;
        string strSend = oWriter.write(oRoot);
        CNetServ::GetInstance()->AddNotify(strSend.c_str(),strSend.length(),NOTIFY_ALL);
    }
    
};