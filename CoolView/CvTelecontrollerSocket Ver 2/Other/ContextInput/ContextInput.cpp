/*
 * ��ָ�������ݴ��͸��������뷨���  �ඨ���ļ�
 * 2012-05-02   V1.0.1.0    ����CNetManager��ĵ�����������Ӧ���
 * 2012-02-28   V1.0.0.0    ������� By LZY
 * 2012-01-27   V0.0.0.0    �����ļ� By Lzy
 */

#include "ContextInput.h"

CContextInput *CContextInput::m_pInstanceObject = NULL;

/*
 * ��˽�С��๹�캯�������ģ���ʼ��
 * @
 */
CContextInput::CContextInput(ENUM_IPVersion eIPVer,unsigned int uPort)
{
    this->m_oTCPListenerID = CNetManager::GetInstance()->CreateTCPServ(NULL,eIPVer,uPort,NULL);
    if (this->m_oTCPListenerID.m_nPID < 0)
        printf("[E]CContextInput:Create TCP Serv Failed\n");
}

/*
 * ��˽�С��������������ͷ���Դ
 */
CContextInput::~CContextInput()
{
    if (this->m_oTCPListenerID.m_nPID >= 0)
    {
        SNodeID oClientID;
        oClientID.m_nPID = CLIENT_NONE;
        CNetManager::GetInstance()->CloseNet(oClientID,this->m_oTCPListenerID,true);
    }
}

/*
 * ������������Ҫ��������ݸ����뷨
 * @param in unsigned short usCodePageNum,������ݵı���ҳ���
 * @param in const char *pszContext,�������
 * @param int nContextLen,������ݵĳ���
 * @return void
 */
void CContextInput::SendContext(unsigned short usCodePageNum,const char *pszContext,int nContextLen)
{
    char *pSendBuf = new char[nContextLen + 3];
    if (pSendBuf != NULL)
    {
        SNodeID oClientID;
        oClientID.m_nPID = CLIENT_ALL;
        *((unsigned short *)pSendBuf) = usCodePageNum;
        memcpy(pSendBuf + 2,pszContext,nContextLen);
        CNetManager::GetInstance()->SendNet(oClientID,this->m_oTCPListenerID,pSendBuf,nContextLen + 2);
        delete pSendBuf;
    }
}
