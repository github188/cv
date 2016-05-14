/*
 * ң�������ն�ͨ�ŷ������˹�������ඨ���ļ�
 * V1.3.1.0 2012-05-02  1������CNetManager��ĵ�����������Ӧ���
 *                      2�����Ӷ�IPv6���ӵ�֧��
 * V1.3.0.0 2012-02-28  ʹ��NetManagerģ������������ʹ�ñ�ģ�������������������רע��ҵ���߼�
 * V1.2.0.0 2011-12-09  ʹ��CommonThread��SimpleMemPoolģ������Դ�ĸ�������������Ϣ����Ĵ������ By Lzy
 * V1.1.0.0 2011-11-24  �����ڲ�ʵ�� By Lzy
 * V0.0.0.0 2011-02-23  �����ļ� By Lzy
 */

#include "NetServ.h"
#include <iostream>
#include "PMessageProcess.h"
#include "OMessageProcess.h"

using namespace thread_operation;
using namespace simple_mem_pool;

CNetServ *CNetServ::m_pNetServInstance = NULL;

/*
 * ��˽�С����캯�����������Ÿ�ң�����ļ����˿ڼ����ӷ�ʽ
 * @param in unsigned int uCommandPort,һ����Ϣ�����˿�
 * @param in EConnectTypeEnum eCommandConnectType,һ����Ϣ����֧�ֵ����ӷ�ʽ
 * @param in unsigned int uNootifyPort,������Ϣ���Ķ˿ڣ����ڼ���ң�������ı�����Ϣ�����ն˷��͵�״̬��Ϣ������ң����������ĳЩ������Ϣ�������󣬶˿ںŲ�����ListenerPort��ͬ
 * @param in unsigned int uNotifyListLen,ͨ���ݴ������󳤶ȣ���Ч��Χ[10-5000]��Խ���Զ�������
 */
CNetServ::CNetServ(unsigned int uCommandPort,EConnectTypeEnum eCommandConnectType,unsigned int uNotifyPort,unsigned int uNotifyListLen)
{
    int nWarnCount = 0;
    //1���ڲ���������ֵ
    printf("[I]Initializing(1/6):Base Param Setting\n");
    this->m_oNotifyListenerV4ID.m_nPID = -1;
    this->m_oNotifyListenerV6ID.m_nPID = -1;
    this->m_bThreadEnd = false;
    this->m_bNotifySendThreadIsRunning = false;
    //2��������Ϣ���Ͷ��г�ʼ��
    printf("[I]Initializing(2/6):Notify List Initializing\n");
    this->m_nNotifyListLen = uNotifyListLen > 5000 ? 5000 : (uNotifyListLen < 10 ? 10 : uNotifyListLen);
    this->m_pNotifyList = (SNotifyNode*)malloc(sizeof(SNotifyNode)*this->m_nNotifyListLen);
    if (this->m_pNotifyList == NULL)
    {
        printf("[E]Notify List Initialize Failed\n");
        return;
    }
    this->m_nNotifyListReadIndex = 0;
    this->m_nNotifyListWriteIndex = 0;
    this->m_pNotifyListLock = CreateCS();//��ʼ��ͨ����Ϣ������ز����ٽ���
    //3��������������ʼ��
    printf("[I]Initializing(3/6):Network Initialing\n");
    this->m_pNetManagerInstance = CNetManager::GetInstance();
    if (this->m_pNetManagerInstance == NULL)
    {
        printf("[E]Network Initialize Failed\n");
        return;
    }
    //4������ң����������Ϣ�����TCP����
    printf("[I]Initializing(4/6):Creating Command TCP Listener If Necessary\n");
    if (eCommandConnectType == CNetServ::Type_Tcp || eCommandConnectType == CNetServ::Type_Both)
    {
        this->m_oCommandListenerV4TCPID = this->m_pNetManagerInstance->CreateTCPServ(NULL,Version4,uCommandPort,CNetServ::_CommandListenerTCPAcceptFunc,this->m_pNetManagerInstance);
        if (this->m_oCommandListenerV4TCPID.m_nPID < 0)
        {
            printf("[W]Create Command IPv4 TCP Listener Failed\n");
            ++nWarnCount;
        }
        this->m_oCommandListenerV6TCPID = this->m_pNetManagerInstance->CreateTCPServ(NULL,Version6,uCommandPort,CNetServ::_CommandListenerTCPAcceptFunc,this->m_pNetManagerInstance);
        if (this->m_oCommandListenerV6TCPID.m_nPID < 0)
        {
            printf("[W]Create Command IPv6 TCP Listener Failed\n");
            ++nWarnCount;
        }
        if (nWarnCount == 2)
        {
            printf("[E]All Command TCP Listener Had Failed To Create\n");
            return;
        }
    }
    //5������ң����������Ϣ�����UDP����
    printf("[I]Initializing(5/6):Creating Command UDP Listener If Necessary\n");
    if (eCommandConnectType == CNetServ::Type_Udp || eCommandConnectType == CNetServ::Type_Both)
    {
        this->m_oCommandListenerV4UDPID = this->m_pNetManagerInstance->CreateUDPServ(NULL,Version4,uCommandPort,NET_THREAD,_CommandClientRecvFunc,NULL,this->m_pNetManagerInstance);
        if (this->m_oCommandListenerV4UDPID.m_nPID < 0)
        {
            printf("[W]Create Command IPv4 UDP Listener Failed\n");
            nWarnCount+=4;
        }
        this->m_oCommandListenerV6UDPID = this->m_pNetManagerInstance->CreateUDPServ(NULL,Version6,uCommandPort,NET_THREAD,_CommandClientRecvFunc,NULL,this->m_pNetManagerInstance);
        if (this->m_oCommandListenerV6UDPID.m_nPID < 0)
        {
            printf("[W]Create Command IPv6 UDP Listener Failed\n");
            nWarnCount+=4;
        }
        if (nWarnCount >= 8)
        {
            printf("[E]All Command UDP Listener Had Failed To Create\n");
            return;
        }
    }
    //6������ң����������Ϣ��ͨ����Ϣ�������TCP����
    printf("[I]Initializing(6/6):Creating Notify TCP Listener\n");
    this->m_oNotifyListenerV4ID = this->m_pNetManagerInstance->CreateTCPServ(NULL,Version4,uNotifyPort,_NotifyListenerTCPAcceptFunc,this->m_pNetManagerInstance);
    if (this->m_oNotifyListenerV4ID.m_nPID < 0)
    {
        printf("[W]Create Notify IPv4 TCP Listener Failed\n");
        nWarnCount+=16;
    }
    this->m_oNotifyListenerV6ID = this->m_pNetManagerInstance->CreateTCPServ(NULL,Version6,uNotifyPort,_NotifyListenerTCPAcceptFunc,this->m_pNetManagerInstance);
    if (this->m_oNotifyListenerV6ID.m_nPID < 0)
    {
        printf("[W]Create Notify IPv6 TCP Listener Failed\n");
        nWarnCount+=16;
    }
    if (nWarnCount >= 32)
    {
        printf("[E]All Notify TCP Listener Had Failed To Create\n");
        return;
    }
    CreateThreadManager(_NotifySendTCPThread,true,this);//����������Ϣ��ͨ����Ϣ�������߳�
    //8��������ʼ��
    if (nWarnCount == 0)
        printf("[I]All Initialize Success\n");
    else
        printf("[I]Initialize Success With Some Warning\n");
}

/*
 * ��˽�С���������
 */
CNetServ::~CNetServ(void)
{
    //1��֪ͨ�����߳̽���
    printf("[I]Destructing(1/3):Waiting Notify Send Thread Close\n");
    this->m_bThreadEnd = true;
    while (this->m_bNotifySendThreadIsRunning == true)
        Sleep(5);
    //2���ͷ�������Դ
    printf("[I]Destructing(2/3):Releasing Network\n");
    CNetManager::ReleaseInstance();
    //3���ͷ�ͨ����Ϣ�������
    printf("[I]Destructing(3/3):Releasing Notify List\n");
    if (this->m_pNotifyList != NULL)
    {
        EnterCS(this->m_pNotifyListLock);
        free(this->m_pNotifyList);
        ReleaseCS(this->m_pNotifyListLock);
    }
    printf("[I]All Destruct Success\n");
}

/*
 * ��˽�С�����̬�����ص�������TCP����ͨ�ż������µ����ӷ���ʱ�Ļص�����
 * @param in SNodeID oClientID,�����ӵ�����������
 * @param in SNodeID oServID,�յ���������ӵķ�������������
 * @param in void *pNetManager,CNetManager��ʵ��ָ��
 * @return bool,���������ӵ��������󷵻�true�����򷵻�false
 */
bool CNetServ::_CommandListenerTCPAcceptFunc(SNodeID oClientID,SNodeID oServID,void *pNetManager)
{
    if (((CNetManager *)pNetManager)->ManageClient(oClientID,oServID,NET_SYSTEM_RELATIVE,_CommandClientRecvFunc,NULL,pNetManager) != 0)
        return true;
    else
        return false;
}

/*
 * ��˽�С�����IDת��������ת�����ֵ��ֱ���������紫��
 * @addition info,ת������int���ͳ��Ȳ�����32λ������裬�ҵ�nClientPID��ֵ����16λʱ������ת��ʧ��
 * @param in SNodeID &rClientID,���ӵı�ʶ
 * @param in unsigned char nExternAdd,Ҫ���ӵ�ת������ϵ�ֵ
 * @param out int &rResult,���ת����Ľ��ֵ
 * @return bool,ת���ɹ�����true�����򷵻�false
 */
bool CNetServ::_IDParse(SNodeID &rClientID,unsigned char nExternAdd,int &rResult)
{
    if (rClientID.m_nPID > 65535)
        return false;
    rResult = rClientID.m_nPID;
    rResult += ((int)(rClientID.m_chUID)) << 16;
    rResult += ((int)(nExternAdd)) << 24;
    return true;
}

/*
 * ��˽�С�ID��ת������
 * @param in int nUnParseID,Ҫ��ת����ֵ
 * @param out SNodeID &rClientID,�����ת��������ӱ�ʶ��Ϣ
 * @param out unsigned char &rExternAdd,�����ת����ĸ��ӽ��ֵ
 * @return void
 */
void CNetServ::_IDUnParse(int nUnParseID,SNodeID &rClientID,unsigned char &rExternAdd)
{
    rClientID.m_nPID = nUnParseID & 65535;
    rClientID.m_chUID = (nUnParseID >> 16) & 255;
    rExternAdd = (nUnParseID >> 24) & 255;
}

/*
 * ��˽�С�����̬�����ص�����������ͨ���н��յ���Ϣʱ�Ļص�����
 * @param in SNodeID oClientID,������Ϣ�����ӵ�����������
 * @param in SNodeID oServID,������Ϣ�����������ķ�������������
 * @param in ENUM_Protocol eProtocol,ָ���������ͣ�TCP_Protocol,UDP_Protocol��
 * @param in const char *pData,�յ�������
 * @param in int nDataLen,�յ������ݵĳ��ȣ���ֵ>0
 * @param in void *pNetManager,CNetManager��ʵ��ָ��
 * @return void
 */
void CNetServ::_CommandClientRecvFunc(SNodeID oClientID,SNodeID oServID,ENUM_Protocol eProtocol,const char *pData,int nDataLen,void *pNetManager)
{
    int nIdentify;//������ӱ�ʶ������Ҫ������ӷ��ͷ�������ʱ���õ�
    bool bParseResult = false;//��¼ת��ID�Ľ��
    CNetServ *pNetServ = CNetServ::GetInstance();
    if (eProtocol == TCP_Protocol)
    {
        printf("[I]New Command TCP Connection Create,ID = %d\n",oClientID.m_nPID);
        if (oServID == pNetServ->m_oCommandListenerV4TCPID)
            bParseResult = pNetServ->_IDParse(oClientID,1,nIdentify);//����ֵ���壺Bxxxx xxx1 = TCP,Bxxxx xx0x = IPv4
        else if (oServID == pNetServ->m_oCommandListenerV6TCPID)
            bParseResult = pNetServ->_IDParse(oClientID,3,nIdentify);//����ֵ���壺Bxxxx xxx1 = TCP,Bxxxx xx1x = IPv6
    }
    else
    {
        printf("[I]New Command UDP Connection Create,ID = %d\n",oClientID.m_nPID);
        if (oClientID == pNetServ->m_oCommandListenerV4UDPID)
            bParseResult = pNetServ->_IDParse(oClientID,0,nIdentify);//����ֵ���壺Bxxxx xxx0 = UDP,Bxxxx xx0x = IPv4
        else if (oClientID == pNetServ->m_oCommandListenerV6UDPID)
            bParseResult = pNetServ->_IDParse(oClientID,2,nIdentify);//����ֵ���壺Bxxxx xxx0 = UDP,Bxxxx xx1x = IPv6
    }
    if (bParseResult == false)
    {
        printf("[E]ID Parse Error,Command TCP Connection ID = %d\n",oClientID.m_nPID);
        return;
    }
#ifdef _DEBUG
    ((char *)pData)[nDataLen] = '\0';
#endif
    CPMessageProcess oMessageProcess(CNetServ::m_pNetServInstance,nIdentify);
    oMessageProcess.MainProcess(pData,nDataLen);//��������������Ϣ
    if (eProtocol == TCP_Protocol)
        ((CNetManager *)pNetManager)->CloseNet(oClientID,oServID,false);//�ر�����
}

/*
 * ��������������������Ϣ���ͷ��ص�����
 * @param in const char *pData,���͵���Ϣ����
 * @param in int nLen,���͵���Ϣ�ĳ���
 * @param in int nSockResIndex,���ӱ�ʶ��
 * @return int,���ط��͵��ֽ��������ִ��󷵻�SOCKET_ERROR �� 0
 */
int CNetServ::CommandSendBack(const char *pData,int nLen,int nSockResIndex)
{
    //�ѱ�ʶ��ת��������������
    SNodeID oServID,oClientID;
    this->_IDUnParse(nSockResIndex,oClientID,oServID.m_chUID);
    if ((oServID.m_chUID & 1) == 1)//TCP���� ����ֵ���壺Bxxxx xxx1 = TCP
    {
        if ((oServID.m_chUID & 2) == 0)//IPv4 ����ֵ���壺Bxxxx xx0x = IPv4
            oServID = this->m_oCommandListenerV4TCPID;
        else//IPv6 ����ֵ���壺Bxxxx xx1x = IPv6
            oServID = this->m_oCommandListenerV6TCPID;
    }
    else//UDP���� ����ֵ���壺Bxxxx xxx0 = UDP
        oServID.m_nPID = SERV_NONE;
    //���ͷ�������
    return this->m_pNetManagerInstance->SendNet(oClientID,oServID,pData,nLen);
}

/*
 * ��˽�С�����̬�����ص�������TCPͨ��ͨ�ż������µ����ӷ���ʱ�Ļص�����
 * @param in SNodeID oClientID,�����ӵ�����������
 * @param in SNodeID oServID,�յ���������ӵķ�������������
 * @param in void *pNetManager,CNetManager��ʵ��ָ��
 * @return bool,�㷵��true
 */
bool CNetServ::_NotifyListenerTCPAcceptFunc(SNodeID oClientID,SNodeID oServID,void *pNetManager)
{
    printf("[I]New Notify TCP Connection Create,ID = %d\n",oClientID.m_nPID);
    ((CNetManager *)pNetManager)->ManageClient(oClientID,oServID,NET_WIN32_IOCP,NULL,_NotifyClientDisconnectFunc);//���й����ӣ������öϿ��ص�����
    CNetServ *pNetServ = CNetServ::GetInstance();
    int nParseID;
    unsigned char chExternAdd = 0;
    if (oServID == pNetServ->m_oNotifyListenerV6ID)
        chExternAdd = 1;
    if (pNetServ->_IDParse(oClientID,chExternAdd,nParseID) == false)
    {
        printf("[E]Notify Listener Accept:ID Parse Error,ClientID = %d\n",oClientID.m_nPID);
        return false;
    }
    Notify_Process_Space::NewNotifyConnectionCreated(nParseID);//�����ⲿȥ����ͨ�����������⴦��
    return true;
}

/*
 * ��˽�С�����̬�����ص�������TCPͨ��ͨ�����ӶϿ�ʱ�Ļص�����
 * @param in SNodeID oClientID,�Ͽ������ӵ�����������
 * @param in SNodeID oServID,�Ͽ������������ķ�������������
 * @param in ENUM_Protocol eProtocol,ָ���������ͣ�TCP_Protocol,UDP_Protocol��
 * @param in bool bError,ָʾ��ǰ�ĶϿ����쳣�Ͽ�(true)���������Ͽ�(false)
 * @param in void *,û�õ�������
 * @return void
 */
void CNetServ::_NotifyClientDisconnectFunc(SNodeID oClientID,SNodeID oServID,ENUM_Protocol eProtocol,bool bError,void *)
{
    int nParseID;
    unsigned char chExternAdd = 0;
    CNetServ *pNetServ = CNetServ::GetInstance();
    if (oServID == pNetServ->m_oNotifyListenerV6ID)
        chExternAdd = 1;
    if (pNetServ->_IDParse(oClientID,chExternAdd,nParseID) == false)
    {
        printf("[E]Notify Client Disconnect:ID Parse Error,ClientID = %d\n",oClientID.m_nPID);
        return;
    }
    Notify_Process_Space::NotifyConnectionClosed(nParseID);//�����ⲿȥ������Ч��ͨ�����������⴦��
}

/*
 * ��˽�С�����̬�����̺߳�����������ͨ����Ϣ��ң�������������Ϣ���̺߳���
 * @param in void *pParam,CNetServ��ʵ��ָ��
 * @return void *,����㷵��NULL
 */
void *CNetServ::_NotifySendTCPThread(void *pParam)
{
    CNetServ* pNetServ = (CNetServ *)(pParam);
    pNetServ->m_bNotifySendThreadIsRunning = true;
    SNodeID oClientID;
    unsigned char chExternAdd;
    while (pNetServ->m_bThreadEnd == false)
    {
        if (pNetServ->m_nNotifyListReadIndex != pNetServ->m_nNotifyListWriteIndex)//����Ƿ���ͨ��ɶ�
        {
            int nTmpRead = pNetServ->m_nNotifyListReadIndex;//�����ݴ�ͨ���ݴ���е�[��]����
            const char *pToSendBuf = &(pNetServ->m_pNotifyList[nTmpRead].m_pBuffer[0]);//ָ��Ҫ���͵Ļ�������ָ��
            int nToSendLen = pNetServ->m_pNotifyList[nTmpRead].m_nUsedSize;//���������ݴ�С
            if (nToSendLen > 0)
            {
                pNetServ->_IDUnParse(pNetServ->m_pNotifyList[nTmpRead].m_nTargetIndex,oClientID,chExternAdd);
                if ((chExternAdd & 4) == 0)
                {
                    if ((chExternAdd & 1) == 0)
                        pNetServ->m_pNetManagerInstance->SendNet(oClientID,pNetServ->m_oNotifyListenerV4ID,pToSendBuf,nToSendLen);
                    else
                        pNetServ->m_pNetManagerInstance->SendNet(oClientID,pNetServ->m_oNotifyListenerV6ID,pToSendBuf,nToSendLen);
                }
                else
                {
                    oClientID.m_nPID = CLIENT_ALL;
                    pNetServ->m_pNetManagerInstance->SendNet(oClientID,pNetServ->m_oNotifyListenerV4ID,pToSendBuf,nToSendLen);
                    pNetServ->m_pNetManagerInstance->SendNet(oClientID,pNetServ->m_oNotifyListenerV6ID,pToSendBuf,nToSendLen);
                }
            }
            if (++nTmpRead >= pNetServ->m_nNotifyListLen)
                nTmpRead = 0;
            pNetServ->m_nNotifyListReadIndex = nTmpRead;
        }
        Sleep_Thread(50);
    }
    pNetServ->m_bNotifySendThreadIsRunning = false;
    return NULL;
}

/*
 * �����������Ҫ���͵�ͨ����Ϣ�����Ͷ����Ӻ���У�
 * @param in char *pData,Ҫ��ӵ�ͨ����Ϣ
 * @param in int nLen,ͨ����Ϣ����
 * @param in int nTarget = NOTIFY_ALL,ͨ�淢��Ŀ�꣬NOTIFY_ALL��ʾ���͸�ȫ�������ߣ�����ֵ��Ϊֻ���͸��ض���һ��������
 * @return int,���ز������ 0-�������� 1�����ݴ�������� 2�������༴������������ִ��ȡ�� 3����Ҫ�������ݳ���ֵ������ ��������Ҫ���͵����ݳ�����ǰ��������С��ֵ����ֵ��
 */
int CNetServ::AddNotify(const char *pData,int nLen,int nTarget)
{
    //nLen = nLen + 1;                                //��1�ǲ���'\0'��ֵ...zhenyi.lie 20110324
    int nRst = 0;
    if (TryEnterCS(this->m_pNotifyListLock,this->m_bThreadEnd) == false)
        return 2;
    int nTmpIndex = this->m_nNotifyListWriteIndex;
    if (++nTmpIndex >= this->m_nNotifyListLen)
        nTmpIndex = 0;
    if (nTmpIndex != this->m_nNotifyListReadIndex)
    {
        if (nLen < NOTIFY_BUFFER_SIZE && nLen > 0)
        {
            this->m_pNotifyList[this->m_nNotifyListWriteIndex].m_nUsedSize = nLen;
            this->m_pNotifyList[this->m_nNotifyListWriteIndex].m_nTargetIndex = nTarget;
            memcpy(this->m_pNotifyList[this->m_nNotifyListWriteIndex].m_pBuffer,pData,nLen);
            this->m_nNotifyListWriteIndex = nTmpIndex;
#ifdef _DEBUG
            printf("[D]Debug output in AddNotify->\nWriteIndex:%d\nData:%s\n",nTmpIndex,pData);
#endif
        }
        else if (nLen > NOTIFY_BUFFER_SIZE)
        {
            printf("[W]Data length is bigger than BUFFER_SIZE[%d]\n",NOTIFY_BUFFER_SIZE);
            nRst = NOTIFY_BUFFER_SIZE - nLen;
        }
        else
            nRst = 3;
    }
    else
        nRst = -1;
    ExitCS(this->m_pNotifyListLock);
    return nRst;
}
