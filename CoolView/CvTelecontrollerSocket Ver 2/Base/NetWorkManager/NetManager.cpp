/*
 * ��������࣬��CommonSocketģ������϶�����������з�װ ��ʵ���ļ� By Lzy
 * 2012-05-09   V1.0.1.1    �����������йܵ������ʹ��ͬ�����ݽ��ղ���
 * 2012-05-02   V1.0.1.0    1������IOCP�����¶Ͽ�������δ�����������
 *                          2����������ʶ����룬��ֹ���ѻ��յ������õ����Ӳ���
 *                          3�����¿������̳߳�ͻ����
 * 2012-02-27   V1.0.0.0    ������ɣ�δ����linux��epoll����
 * 2012-01-27   V0.0.0.0    �����ļ�
 */

#include "NetManager.h"

using namespace simple_mem_pool;
using namespace thread_operation;

CNetManager *CNetManager::m_pNetManager = NULL;

/*
 * ������������̬����ȡ��ʵ���ĺ����������δ����ʵ�����Զ�����
 * @return CNetManager *,������ʵ��
 */
CNetManager *CNetManager::GetInstance()
{
    if (CNetManager::m_pNetManager == NULL)
        CNetManager::m_pNetManager = new CNetManager;
    return CNetManager::m_pNetManager;
}

/*
 * ������������̬���ͷ���ʵ��
 * @return void
 */
void CNetManager::ReleaseInstance()
{
    if (CNetManager::m_pNetManager != NULL)
    {
        delete CNetManager::m_pNetManager;
        CNetManager::m_pNetManager = NULL;
    }
}

/*
 * ��˽�С�����̬/�ص�������MemPool�ͷ�SServNode�ṹʱ���õĺ���
 * @param in void *pObject,������ͷŶ����ָ��
 * @param in void *pCommonSocket,CCommonSocket��ʵ��ָ��
 * @return void
 */
void CNetManager::_ForMemPoolReleaseServNode(void *pObject,void *pCommonSocket)
{
    SServNode *pServNode = (SServNode *)pObject;
    pServNode->m_bExit = true;
    ((CCommonSocket *)pCommonSocket)->CloseSocket(pServNode->m_oSocket);
    while (pServNode->m_nVisitingCount > 0)
        Sleep_Thread(5);
    ReleaseMemPool(&pServNode->m_oClientPool,pCommonSocket);
    ReleaseCS(pServNode->m_pClientPoolLock);
    ReleaseCS(pServNode->m_pClientLinkLock);
    ++pServNode->m_oIDO.m_chUID;
}

/*
 * ��˽�С�����̬/�ص�������MemPool�ͷ�SClientNode�ṹʱ���õĺ���
 * @param in void *pObject,������ͷŶ����ָ��
 * @param in void *pCommonSocket,CCommonSocket��ʵ��ָ��
 * @return void
 */
void CNetManager::_ForMemPoolReleaseClientNode(void *pObject,void *pCommonSocket)
{
    SClientNode *pClientNode = (SClientNode *)pObject;
    pClientNode->m_bExit = true;
    ((CCommonSocket *)pCommonSocket)->CloseSocket(pClientNode->m_oSocket);
    while (1)
    {
        while (pClientNode->m_nVisitingCount > 0)
            Sleep_Thread(5);
        EnterCS(pClientNode->m_pVisitingCountLock);
        if (pClientNode->m_nVisitingCount > 0)
            ExitCS(pClientNode->m_pVisitingCountLock);
        else
            break;
    }
    if (pClientNode->m_pfDisconnect != NULL && CNetManager::GetInstance()->m_bObjectRelease == false)
        pClientNode->m_pfDisconnect(pClientNode->m_oIDO,pClientNode->m_oServIDO,pClientNode->m_eProtocol,pClientNode->m_bErrorClose,pClientNode->m_pUser);
    ReleaseCS(pClientNode->m_pVisitingCountLock);
    ++pClientNode->m_oIDO.m_chUID;
}

/*
 * ��˽�С�����̬/�̺߳��������������������е��̺߳���
 * @param in void *pObject,�̹߳����SServNode�ṹָ��
 * @return void *,����ֵ��ΪNULL
 */
void *CNetManager::_ForServNodeThread(void *pObject)
{
    SServNodeThreadParam *pServNodeThreadParam = (SServNodeThreadParam *)pObject;
    SServNode *pServNode = pServNodeThreadParam->m_pServNode;
    CNetManager *pNetManager = pServNodeThreadParam->m_pNetManager;
    ++pServNode->m_nVisitingCount;
    pServNodeThreadParam->m_bHasRead = true;
    CCommonSocket *pCommonSocket = CCommonSocket::GetInstance();
    while (pServNode->m_bExit == false)
    {
        void *pNewSocket = pCommonSocket->AcceptSocket(pServNode->m_oSocket);
        if (pNewSocket != NULL)
        {
            SNodeID oIDO;
            //1������Դ
            if (TryEnterCS(pServNode->m_pClientPoolLock,pServNode->m_bExit) == false)//>���ڴ���
                break;
            //2��������Դ
            SClientNode *pNewClientNode = (SClientNode *)AllocMemory(pServNode->m_oClientPool,NULL,&(oIDO.m_nPID));
            if (pNewClientNode == NULL)
            {
                ExitCS(pServNode->m_pClientPoolLock);
                break;
            }
            //3�����������Ϣ
            pNewClientNode->m_oSocket = pNewSocket;
            pNewClientNode->m_eManagerType = NET_NONE;
            pNewClientNode->m_nVisitingCount = 0;
            pNewClientNode->m_bExit = false;
            pNewClientNode->m_eProtocol = TCP_Protocol;
            pNewClientNode->m_oServIDO = pServNode->m_oIDO;
            pNewClientNode->m_oIDO.m_nPID = oIDO.m_nPID;
            ++pNewClientNode->m_oIDO.m_chUID;
            oIDO.m_chUID = pNewClientNode->m_oIDO.m_chUID;
            pNewClientNode->m_bErrorClose = false;
            pNewClientNode->m_pVisitingCountLock = CreateCS();
            pNewClientNode->m_pfDisconnect = NULL;
            ExitCS(pServNode->m_pClientPoolLock);//<���ڴ���
            //>>��������
            if (TryEnterCS(pServNode->m_pClientLinkLock,pNetManager->m_bObjectRelease) == false)
                break;
            pNewClientNode->m_pNext = (SClientNode *)pServNode->m_pHeadClientNode;
            if (pServNode->m_pHeadClientNode != NULL)
                ((SClientNode *)pServNode->m_pHeadClientNode)->m_pPre = pNewClientNode;
            pNewClientNode->m_pPre = NULL;
            pServNode->m_pHeadClientNode = pNewClientNode;
            pNewClientNode->m_bAddedLink = true;
            //<<�������
            ExitCS(pServNode->m_pClientLinkLock);
            //4�����ûص�
            if (pServNode->m_pfAccept != NULL)
            {
                if (pServNode->m_pfAccept(oIDO,pServNode->m_oIDO,pServNode->m_pUser) == false)
                    pNetManager->_CloseNodes(pServNode,pNewClientNode,false,true);
            }
        }
    }
    pNetManager->_CloseNodes(pServNode,NULL,true,true);
    --pServNode->m_nVisitingCount;
    return NULL;
}

/*
 * ��˽�С�����̬/�̺߳�����ʹ�ö����̹߳��������ӵ��йܷ�ʽʱ���̺߳���
 * @param in void *pObject,Ҫ�����SClientNode�ڵ�ָ��
 * @return void *,�㷵��NULL
 */
void *CNetManager::_ForClientNodeThread(void *pObject)
{
    SClientNodeThreadParam *pClientNodeThreadParam = (SClientNodeThreadParam *)pObject;
    SServNode *pServNode = pClientNodeThreadParam->m_pServNode;
    SClientNode *pClientNode = pClientNodeThreadParam->m_pClientNode;
    CNetManager *pNetManager = pClientNodeThreadParam->m_pNetManager;
    pClientNodeThreadParam->m_bHasRead = true;
    if (TryEnterCS(pClientNode->m_pVisitingCountLock,pClientNode->m_bExit) == false)
        return NULL;
    ++pClientNode->m_nVisitingCount;
    ExitCS(pClientNode->m_pVisitingCountLock);
    char bBuf[NET_MANAGER_RECEIVE_BUFFER_SIZE];//�������ݻ�����
    int nReceiveLen = 0;
    while (pClientNode->m_bExit == false)
    {
        if (pClientNode->m_eProtocol == TCP_Protocol)
            nReceiveLen = pNetManager->m_pCommonSocket->ReceiveSocket(pClientNode->m_oSocket,bBuf,NET_MANAGER_RECEIVE_BUFFER_SIZE);
        else if (pClientNode->m_eProtocol == UDP_Protocol)
            nReceiveLen = pNetManager->m_pCommonSocket->ReceiveSocket(pClientNode->m_oSocket,bBuf,NET_MANAGER_RECEIVE_BUFFER_SIZE,(sockaddr *)(&pClientNode->m_uAddr),&pClientNode->m_nAddrLen);
        else
            nReceiveLen = SOCKET_ERROR;
        if (nReceiveLen <= 0)
        {
            if (pClientNode->m_bIsUDPServ == false)//���ڷ�UDP SERV���ͣ�������˳��߳�
            {
#ifdef _DEBUG
                printf ("[E]Thread Managed Client Receive Error Code=%d\n",WSAGetLastError());
#endif
                break;
            }
        }
        else if (pClientNode->m_pfRecv != NULL)
            pClientNode->m_pfRecv(pClientNode->m_oIDO,pClientNode->m_oServIDO,pClientNode->m_eProtocol,bBuf,nReceiveLen,pClientNode->m_pUser);
    }
    //�������ӽڵ�
    if (nReceiveLen == SOCKET_ERROR)
        pClientNode->m_bErrorClose = true;
    pNetManager->_CloseNodes(pServNode,pClientNode,true,true);
    EnterCS(pClientNode->m_pVisitingCountLock);
    --pClientNode->m_nVisitingCount;
    ExitCS(pClientNode->m_pVisitingCountLock);
    return NULL;
}

/*
 * ��˽�С�����̬/�̺߳��������IOCP�߳�ʹ��״��������IOCP�̵߳��̺߳���
 * @param in void *pObject,CNetManagerʵ������ָ��
 * @return void *,����ֵ��ΪNULL
 */
void *CNetManager::_ForIOCPThreadManageThread(void *pObject)
{
    CNetManager *pNetManager = (CNetManager *)pObject;
    while (pNetManager->m_bObjectRelease == false)
    {
        if (pNetManager->m_nCurCreatedIOCPThreadCount == pNetManager->m_nCurRunningIOCPThreadCount)
        {
            Sleep_Thread(100);
            if (pNetManager->m_nCurCreatedIOCPThreadCount == pNetManager->m_nCurRunningIOCPThreadCount)
            {
                if (pNetManager->m_nCurCreatedIOCPThreadCount + 1 <= MAX_IOCP_THREAD_COUNT || MAX_IOCP_THREAD_COUNT <= 0)
                {
                    if (TryEnterCS(pNetManager->m_pCurIOCPThreadCountVarLock,pNetManager->m_bObjectRelease) == true)
                    {
                        if (CreateThreadManager(_ForIOCPPoolThread,true,pNetManager) != NULL)//���������߳�
                            ++pNetManager->m_nCurCreatedIOCPThreadCount;
                        ExitCS(pNetManager->m_pCurIOCPThreadCountVarLock);
                    }
                }
            }
        }
        Sleep_Thread(1000);
    }
    return NULL;
}

/*
 * ��˽�С�����̬/�̺߳�����IOCP�̳߳�ʹ�õ��̺߳���
 * @param in void *pObject,CNetManagerʵ������ָ��
 * @return void *,����ֵ��ΪNULL
 */
void *CNetManager::_ForIOCPPoolThread(void *pObject)
{
    CNetManager *pNetManager = (CNetManager *)pObject;
    DWORD dwUnUsedVar1,dwUnUsedVar2;
    OVERLAPPED *pReceiveOverLapped;//��Ž��յ���OVERLAPPED�ṹ
    char bBuf[NET_MANAGER_RECEIVE_BUFFER_SIZE];//���ݽ��ջ�����
    int nReceiveLen;//���յ������ݵĳ���
#ifdef _DEBUG
    printf("[I]IOCP Thread Created,Total Count = %d\n",pNetManager->m_nCurCreatedIOCPThreadCount);
#endif
    while (1)
    {
        if (GetQueuedCompletionStatus(pNetManager->m_hIOCP_Handle,&dwUnUsedVar1,&dwUnUsedVar2,&pReceiveOverLapped,INFINITE) == FALSE)
            continue;
        if (pReceiveOverLapped == NULL)
            break;
        SClientNode *pClientNode = (SClientNode *)pReceiveOverLapped;
        //�����߳���+1
        if (TryEnterCS(pNetManager->m_pCurIOCPThreadCountVarLock,pNetManager->m_bObjectRelease) == false)
            break;
        ++pNetManager->m_nCurRunningIOCPThreadCount;
        ExitCS(pNetManager->m_pCurIOCPThreadCountVarLock);
        //��������
        if (TryEnterCS(pClientNode->m_pVisitingCountLock,pClientNode->m_bExit) == true)
        {
            ++pClientNode->m_nVisitingCount;
            ExitCS(pClientNode->m_pVisitingCountLock);
            if (pClientNode->m_eProtocol == TCP_Protocol)
                nReceiveLen = pNetManager->m_pCommonSocket->ReceiveSocket(pClientNode->m_oSocket,bBuf,NET_MANAGER_RECEIVE_BUFFER_SIZE);
            else
                nReceiveLen = SOCKET_ERROR;//����UDP���ӽ�ֱ�ӱ��ܾ�
            //���պ�Ĵ���
            if (nReceiveLen > 0)
            {
                if (pClientNode->m_pfRecv != NULL)
                    pClientNode->m_pfRecv(pClientNode->m_oIDO,pClientNode->m_oServIDO,pClientNode->m_eProtocol,bBuf,nReceiveLen,pClientNode->m_pUser);
                //���·����첽����
                if (WSARecv(pNetManager->m_pCommonSocket->GetSocket(pClientNode->m_oSocket),&(pNetManager->m_oWSARecvZeroBuf),1,&(pNetManager->m_dwWSARecvReceiveLen),&(pNetManager->m_dwWSARecvFlag),&(pClientNode->m_OverLapped),NULL) == SOCKET_ERROR)//�����첽����
                {
                    if (WSAGetLastError() != WSA_IO_PENDING)
                        nReceiveLen = SOCKET_ERROR;
                }
            }
            //������
            if (nReceiveLen <= 0 && pClientNode->m_bAddedLink == true)
            {
                if (nReceiveLen == SOCKET_ERROR)
                {
                    pClientNode->m_bErrorClose = true;
#ifdef _DEBUG
            printf ("[E]IOCP Managed Client Receive Error Code=%d\n",WSAGetLastError());
#endif
                }
                if (pClientNode->m_oServIDO.m_nPID >= 0)
                {
                    SServNode *pServNode = (SServNode *)FindMemory(pNetManager->m_oServPool,pClientNode->m_oServIDO.m_nPID);
                    if (pServNode != NULL)
                        pNetManager->_CloseNodes(pServNode,pClientNode,true,true);
                }
                else
                    pNetManager->_CloseNodes(NULL,pClientNode,true,true);
            }
            EnterCS(pClientNode->m_pVisitingCountLock);
            --pClientNode->m_nVisitingCount;
            ExitCS(pClientNode->m_pVisitingCountLock);
        }
        //�����߳���-1
        EnterCS(pNetManager->m_pCurIOCPThreadCountVarLock);
        --pNetManager->m_nCurRunningIOCPThreadCount;
        ExitCS(pNetManager->m_pCurIOCPThreadCountVarLock);    
    }
    EnterCS(pNetManager->m_pCurIOCPThreadCountVarLock);
    --pNetManager->m_nCurCreatedIOCPThreadCount;
    ExitCS(pNetManager->m_pCurIOCPThreadCountVarLock);
#ifdef _DEBUG
    printf("[I]IOCP Thread Exit,Total Left = %d\n",pNetManager->m_nCurCreatedIOCPThreadCount);
#endif
    return NULL;
}

/*
 * ��˽�С�����̬/�̺߳���������Ҫ�رջ���յĽڵ���̺߳���
 * @param in void *pObject,CNetManager����ָ��
 * @return void *,�㷵��NULL
 */
void *CNetManager::_ForNodeManage(void *pObject)
{
    CNetManager *pNetManager = (CNetManager *)pObject;
    int nNextReadIndex;
    while (pNetManager->m_bObjectRelease == false)
    {
        while (pNetManager->m_nWriteIndex != pNetManager->m_nReadIndex && pNetManager->m_bObjectRelease == false)
        {
            nNextReadIndex = pNetManager->m_nReadIndex;
            if (pNetManager->m_aRecycle[nNextReadIndex].m_pClientNode != NULL) //���յ����ͻ��˽ڵ�
            {
                if (pNetManager->m_aRecycle[nNextReadIndex].m_pServNode != NULL)
                {
                    if (TryEnterCS(pNetManager->m_aRecycle[nNextReadIndex].m_pServNode->m_pClientPoolLock,pNetManager->m_bObjectRelease) == false)
                        return NULL;
                    RecycleMemory(pNetManager->m_aRecycle[nNextReadIndex].m_pServNode->m_oClientPool,pNetManager->m_aRecycle[nNextReadIndex].m_pClientNode,pNetManager->m_pCommonSocket);
                    ExitCS(pNetManager->m_aRecycle[nNextReadIndex].m_pServNode->m_pClientPoolLock);
                }
                else
                {
                    if (TryEnterCS(pNetManager->m_pClientPoolLock,pNetManager->m_bObjectRelease) == false)
                        return NULL;
                    RecycleMemory(pNetManager->m_oClientPool,pNetManager->m_aRecycle[nNextReadIndex].m_pClientNode,pNetManager->m_pCommonSocket);
                    ExitCS(pNetManager->m_pClientPoolLock);
                }
            }
            else //���ն���ڵ�
            {
                if (pNetManager->m_aRecycle[nNextReadIndex].m_pServNode == NULL || (pNetManager->m_aRecycle[nNextReadIndex].m_pServNode != NULL && pNetManager->m_aRecycle[nNextReadIndex].m_bRecycleServNode == false))
                {
                    if (TryEnterCS(pNetManager->m_pClientPoolLock,pNetManager->m_bObjectRelease) == false)
                        return NULL;
                    RecycleMemory(pNetManager->m_oClientPool,NULL,pNetManager->m_pCommonSocket);
                    pNetManager->m_aRecycle[nNextReadIndex].m_pServNode->m_pHeadClientNode = NULL;
                    ExitCS(pNetManager->m_pClientPoolLock);
                }
                else
                {
                    if (TryEnterCS(pNetManager->m_pServPoolLock,pNetManager->m_bObjectRelease) == false)
                        return NULL;
                    RecycleMemory(pNetManager->m_oServPool,pNetManager->m_aRecycle[nNextReadIndex].m_pServNode,pNetManager->m_pCommonSocket);
                    ExitCS(pNetManager->m_pServPoolLock);
                }
            }
            ++nNextReadIndex;
            if (nNextReadIndex >= NET_RECYCLE_ARRAY_SIZE)
                nNextReadIndex = 0;
            pNetManager->m_nReadIndex = nNextReadIndex;
        }
        Sleep_Thread(100);
    }
    return NULL;
}

/*
 * ��˽�С����캯������ʼ����Ա����
 */
CNetManager::CNetManager()
{
    this->m_pCommonSocket = CCommonSocket::GetInstance();
    this->m_oServPool = CreateMemPool(sizeof(SServNode),NULL,_ForMemPoolReleaseServNode);
    this->m_pServPoolLock = CreateCS();
    this->m_pServLinkLock = CreateCS();
    this->m_pHeadServNode = NULL;
    this->m_oClientPool = CreateMemPool(sizeof(SClientNode),NULL,_ForMemPoolReleaseClientNode);
    this->m_pClientPoolLock = CreateCS();
    this->m_pClientLinkLock = CreateCS();
    this->m_pHeadClientNode = NULL;
    this->m_nReadIndex = 0;
    this->m_nWriteIndex = 0;
    this->m_pWriteLock = CreateCS();
    this->m_bObjectRelease = false;
    if (CreateThreadManager(_ForNodeManage,true,this) == NULL)
    {
        printf("[E]CNetManager Create Thread Failed\n");
        this->m_bObjectRelease = true;
    }
#ifdef WIN32
    this->m_hIOCP_Handle = NULL;
#endif
}

/*
 * ��˽�С������������ͷų�Ա����
 */
CNetManager::~CNetManager()
{
    EnterCS(this->m_pWriteLock);
    this->m_bObjectRelease = true;
    EnterCS(this->m_pServLinkLock);
    EnterCS(this->m_pServPoolLock);
    ReleaseMemPool(&this->m_oServPool,this->m_pCommonSocket);
    ReleaseCS(this->m_pServPoolLock);
    ReleaseCS(this->m_pServLinkLock);
    EnterCS(this->m_pClientLinkLock);
    EnterCS(this->m_pClientPoolLock);
    ReleaseMemPool(&this->m_oClientPool,this->m_pCommonSocket);
    ReleaseCS(this->m_pClientPoolLock);
    ReleaseCS(this->m_pClientLinkLock);
#ifdef WIN32
    //IOCP����
    if (this->m_hIOCP_Handle != NULL)
    {
        while (this->m_nCurCreatedIOCPThreadCount != 0)
        {
            PostQueuedCompletionStatus(this->m_hIOCP_Handle,0,NULL,NULL);
            Sleep_Thread(50);
        }
        ReleaseCS(this->m_pCurIOCPThreadCountVarLock);
        CloseHandle(this->m_hIOCP_Handle);
    }
#endif
    ReleaseCS(this->m_pWriteLock);
    CCommonSocket::ReleaseInstance();
}

/*
 * ������������TCP�ķ�������
 * @info �����������н��յ�����Ĭ��ֻ����������ͷŹ������û�ڱ�������������Ч��pfAccept����������Щ�����ӣ���ͬ�����н��յ����ӵ��շ����޷�����
 * @param in const char *pszIP,�������˼���IP��ַ��������ΪNULL����ʱ������IP�汾ʹ��Ĭ�ϵ�IP��ַ
 * @param in ENUM_IPVersion eIPVer,IP�汾��
 * @param in unsigned int nPort,�����Ķ˿�
 * @param in Accept_FuncP pfAccept = NULL,����������������ʱ��֪ͨ�ص��������������������ص���������ֻ�лص���������trueʱ����������Żᱻ����
 * @param in void *pUser = NULL,�û��������ݣ������ݽ�����֪ͨ�ص����������û���øûص���������˲�����������Ч
 * @return SNodeID,���صĽṹ��m_nPID�Ǹ�����ʾ�������˵ļ����ţ�-1��ʾ����
 */
SNodeID CNetManager::CreateTCPServ(const char *pszIP,ENUM_IPVersion eIPVer,unsigned int nPort,Accept_FuncP pfAccept,void *pUser)
{
    SNodeID oIDO;//��Ŵ����ķ������˵ı�ʶ����
    oIDO.m_nPID = -1;
    //>�ڴ������
    if (TryEnterCS(this->m_pServPoolLock,this->m_bObjectRelease) == false)
        return oIDO;
    SServNode *pNewNode = (SServNode *)AllocMemory(this->m_oServPool,NULL,&(oIDO.m_nPID));
    if (pNewNode == NULL)
    {
        ExitCS(this->m_pServPoolLock);//<��֧һ�ڴ�ؽ���
        oIDO.m_nPID = -1;
        return oIDO;
    }
    pNewNode->m_pVisitingCountLock = CreateCS();
    pNewNode->m_pClientPoolLock = CreateCS();
    pNewNode->m_pClientLinkLock = CreateCS();
    pNewNode->m_oClientPool = NULL;
    pNewNode->m_nVisitingCount = 0;
    pNewNode->m_oSocket = this->m_pCommonSocket->CreateSocket(eIPVer,TCP_Protocol);
    this->m_pCommonSocket->BindSocket(pNewNode->m_oSocket,pszIP,nPort);
    if (this->m_pCommonSocket->ListenSocket(pNewNode->m_oSocket) == SOCKET_ERROR)
        goto tag_OnTCPServErrRelease;
    pNewNode->m_oClientPool = CreateMemPool(sizeof(SClientNode),NULL,_ForMemPoolReleaseClientNode);
    pNewNode->m_pfAccept = pfAccept;
    pNewNode->m_pUser = pUser;
    pNewNode->m_bExit = false;
    pNewNode->m_bAddedLink = false;
    pNewNode->m_oIDO.m_nPID = oIDO.m_nPID;
    ++pNewNode->m_oIDO.m_chUID;
    oIDO.m_chUID = pNewNode->m_oIDO.m_chUID;
    pNewNode->m_pHeadClientNode = NULL;
    ExitCS(this->m_pServPoolLock);//<��֧���ڴ�ؽ���
    //>>��������
    if (TryEnterCS(this->m_pServLinkLock,this->m_bObjectRelease) == false)
        goto tag_OnTCPServErrReturn;
    pNewNode->m_pNext = this->m_pHeadServNode;
    this->m_pHeadServNode = pNewNode;
    pNewNode->m_bAddedLink = true;
    //<<�������
    ExitCS(this->m_pServLinkLock);
    SServNodeThreadParam sServNodeThreadParam;
    sServNodeThreadParam.m_pServNode = pNewNode;
    sServNodeThreadParam.m_pNetManager = this;
    sServNodeThreadParam.m_bHasRead = false;
    if (CreateThreadManager(_ForServNodeThread,true,&sServNodeThreadParam) != NULL) //���������̣߳�����ʧ��ʱ�����������
    {
        while (sServNodeThreadParam.m_bHasRead == false)
            Sleep_Thread(5);
        return oIDO;
    }
    else
    {
        this->_CloseNodes(pNewNode,NULL,false,true);
        goto tag_OnTCPServErrReturn;
    }
tag_OnTCPServErrRelease:
    if (TryEnterCS(this->m_pServPoolLock,this->m_bObjectRelease) == true)//>>>�ڴ������
    {
        RecycleMemory(this->m_oServPool,pNewNode,this->m_pCommonSocket);
        ExitCS(this->m_pServPoolLock);//<<<�ڴ�ؽ���
    }
tag_OnTCPServErrReturn:
    oIDO.m_nPID = -1;
    return oIDO;
}

/*
 * �������������ͻ���
 * @info ����UDP�ͻ��˽��޷������йܲ��������UDP�ͻ����µ�eManageType�����ᱻ����
 * @param in const char *pszIP,�Է���IP��ַ
 * @param in ENUM_IPVersion eIPVer,IP�汾��
 * @param in unsigned int nPort,�Է��Ķ˿ں�
 * @param in ENUM_Protocol eProtocol,���ӷ�ʽ��ָʹ�õ���TCP����UDP��
 * @param in EManageType eManageType,����ʹ�õ��йܷ�ʽ��������ΪNET_NONE����Ϊ�û����й����û���ʹ��ManageClient��������Щ���й��������תΪ�йܹ���
 * @param in Recv_FuncP pfRecv = NULL,���յ���Ϣʱ�Ļص����������������û���йܣ���˻ص��������ᱻ�õ�
 * @param in Disconnect_FuncP pfDisconnect = NULL,���ӶϿ�ʱ�Ļص�����
 * @param in void *pUser = NULL,�û��Զ������ݣ������ݻ��ڽ��ջص�������Ͽ��ص�����������ʱ��Ϊ��������֮һ���͸��ص�����
 * @return SNodeID,���صĽṹ��m_nPID�Ǹ�����ʾ�������˵ļ����ţ�-1��ʾ����
 */
SNodeID CNetManager::CreateClient(const char *pszIP,ENUM_IPVersion eIPVer,unsigned int nPort,ENUM_Protocol eProtocol,EManageType eManageType,Recv_FuncP pfRecv,Disconnect_FuncP pfDisconnect,void *pUser)
{
    return this->_CreateClient(pszIP,eIPVer,nPort,eProtocol,false,eManageType,pfRecv,pfDisconnect,pUser);
}

/*
 * ������������UDP��������
 * @param in const char *pszIP,�󶨵�IP��ַ������ΪNULL��ʹ�ñ��ص�ַ
 * @param in ENUM_IPVersion eIPVer,IP�汾��
 * @param in unsigned int nPort,�󶨵�IP�˿�
 * @param in EManageType eManageType,�й�����
 * @param in Recv_FuncP pfRecv = NULL,���յ���Ϣʱ�Ļص�����
 * @param in Disconnect_FuncP pfDisconnect = NULL,UDP�������ر�ʱ�Ļص�����
 * @param in void *pUser = NULL,�û��Զ���������ڵ��ûص�����ʱ��Ѵ˲�������ȥ
 * @return SNodeID,���صĽṹ��m_nPID�Ǹ�����ʾ�������˵ļ����ţ�-1��ʾ����
 */
SNodeID CNetManager::CreateUDPServ(const char *pszIP,ENUM_IPVersion eIPVer,unsigned int nPort,EManageType eManageType,Recv_FuncP pfRecv,Disconnect_FuncP pfDisconnect,void *pUser)
{
    return this->_CreateClient(pszIP,eIPVer,nPort,UDP_Protocol,true,eManageType,pfRecv,pfDisconnect,pUser);
}

/*
 * ��˽�С�����TCP�ͻ���,UDP�ͻ��˻��������
 * @param in const char *pszIP,�Է���IP��ַ
 * @param in ENUM_IPVersion eIPVer,IP�汾��
 * @param in unsigned int nPort,�Է��Ķ˿ں�
 * @param in ENUM_Protocol eProtocol,���ӷ�ʽ��ָʹ�õ���TCP����UDP��
 * @param in bool bIsUDPServ,ֻ��eProtocol��UDP_Protocolʱ�˲�������Ч��ָ���Ƿ񴴽�UDP�ķ�������
 * @param in EManageType eManageType,����ʹ�õ��йܷ�ʽ��������ΪNET_NONE����Ϊ�û����й����û���ʹ��ManageClient��������Щ���й��������תΪ�йܹ���
 * @param in Recv_FuncP pfRecv,���յ���Ϣʱ�Ļص����������������û���йܣ���˻ص��������ᱻ�õ�
 * @param in Disconnect_FuncP pfDisconnect,���ӶϿ�ʱ�Ļص�����
 * @param in void *pUser,�û��Զ������ݣ������ݻ��ڽ��ջص�������Ͽ��ص�����������ʱ��Ϊ��������֮һ���͸��ص�����
 * @return SNodeID,���صĽṹ��m_nPID�Ǹ�����ʾ�������˵ļ����ţ�-1��ʾ����
 */ 
SNodeID CNetManager::_CreateClient(const char *pszIP,ENUM_IPVersion eIPVer,unsigned int nPort,ENUM_Protocol eProtocol,bool bIsUDPServ,EManageType eManageType,Recv_FuncP pfRecv,Disconnect_FuncP pfDisconnect,void *pUser)
{
    SNodeID oIDO;//��Ŵ����Ŀͻ��˵ı�ʶ����
    oIDO.m_nPID = -1;
    //>�ڴ������
    if (TryEnterCS(this->m_pClientPoolLock,this->m_bObjectRelease) == false)
        return oIDO;
    SClientNode *pNewNode = (SClientNode *)AllocMemory(this->m_oClientPool,NULL,&(oIDO.m_nPID));
    if (pNewNode == NULL)
    {
        ExitCS(this->m_pClientPoolLock);//<��֧һ�ڴ�ؽ���
        return oIDO;
    }
    pNewNode->m_oSocket = this->m_pCommonSocket->CreateSocket(eIPVer,eProtocol);
    pNewNode->m_pVisitingCountLock = CreateCS();
    pNewNode->m_eManagerType = NET_NONE;
    pNewNode->m_bExit = false;
    pNewNode->m_bAddedLink = false;
    pNewNode->m_nVisitingCount = 0;
    pNewNode->m_oServIDO.m_nPID = SERV_NONE;
    pNewNode->m_oIDO.m_nPID = oIDO.m_nPID;
    ++pNewNode->m_oIDO.m_chUID;
    oIDO.m_chUID = pNewNode->m_oIDO.m_chUID;
    pNewNode->m_bErrorClose = false;
    pNewNode->m_eProtocol = eProtocol;
    ExitCS(this->m_pClientPoolLock);//<��֧���ڴ�ؽ���
    if (eProtocol == TCP_Protocol)
    {
        if (this->m_pCommonSocket->ConnectSocket(pNewNode->m_oSocket,pszIP,nPort) == SOCKET_ERROR)
            goto tag_OnClientErrRelease;
    }
    else if (eProtocol == UDP_Protocol)
    {
        pNewNode->m_bIsUDPServ = bIsUDPServ;
        if (bIsUDPServ == false)
        {
            pNewNode->m_nAddrLen = this->m_pCommonSocket->GetSockAddr(eIPVer,pszIP,nPort,(sockaddr *)(&pNewNode->m_uAddr));
            if (pNewNode->m_nAddrLen == 0)
                goto tag_OnClientErrRelease;
            if (this->m_pCommonSocket->SendSocket(pNewNode->m_oSocket,NULL,0,(sockaddr *)(&pNewNode->m_uAddr),pNewNode->m_nAddrLen) == SOCKET_ERROR)
                goto tag_OnClientErrRelease;
        }
        else
        {
            pNewNode->m_nAddrLen = sizeof(pNewNode->m_uAddr);
            if (this->m_pCommonSocket->BindSocket(pNewNode->m_oSocket,pszIP,nPort) == SOCKET_ERROR)
                goto tag_OnClientErrRelease;
        }
    }
    //>>��������
    if (TryEnterCS(this->m_pClientLinkLock,this->m_bObjectRelease) == false)
        goto tag_OnClientErrReturn;
    pNewNode->m_pNext = this->m_pHeadClientNode;
    if (this->m_pHeadClientNode != NULL)
        this->m_pHeadClientNode->m_pPre = pNewNode;
    pNewNode->m_pPre = NULL;
    this->m_pHeadClientNode = pNewNode;
    pNewNode->m_bAddedLink = true;
    //<<�������
    ExitCS(this->m_pClientLinkLock);
    if (this->_ManageClient(NULL,pNewNode,eManageType,pfRecv,pfDisconnect,pUser) == 1)//д���й���Ϣ���йܴ���ʧ��ʱ�����������
        return oIDO;
    else
    {
        this->_CloseNodes(NULL,pNewNode,false,true);
        goto tag_OnClientErrReturn;
    }
tag_OnClientErrRelease:
    //>>>���ڴ����
    if (TryEnterCS(this->m_pClientPoolLock,this->m_bObjectRelease) == true)
    {
        RecycleMemory(this->m_oClientPool,pNewNode,this->m_pCommonSocket);
        ExitCS(this->m_pClientPoolLock);//<<<����ڴ����
    }
tag_OnClientErrReturn:
    oIDO.m_nPID = -1;
    return oIDO;
}

/*
 * ��˽�С����ض���Χ�����ӽ���ָ������
 * @param in SNodeID oClientIDO,���ӱ�ʶ���ݣ������m_nPID��ʹ��CLIENT_ALL��ʾ������nServID�µ��������ӻ�CLIENT_NONE��ʾ���κ������޹�
 * @param in SNodeID oServIDO,���������ķ������˵ı�ʶ���ݣ������ͨ��CreateClient���������ӣ��������m_nPID�ֶ�����SERV_NONE������ʹ��SERV_ALL��ʾ������Ч�ķ�������
 * @param in int nOperationNum,������ţ���ͷ�ļ������ԡ�OP_����ͷ��Ԥ����ֵ����
 * @param in void *pParam,������������ͬ��������
 * @return int,����ֵ��Ӧ������ͬ����ͬ��������漰���Զ�����ӵĲ���������ֵֻ��ʾ���һ�������ķ���ֵ
 */
int CNetManager::_RangeOperation(SNodeID oClientIDO,SNodeID oServIDO,int nOperationNum,void *pParam)
{
    //1��������Χ����������λ����
    unsigned char bRange = 0;
    if (oClientIDO.m_nPID >= 0)
        bRange |= 3;//B 0000 0011
    else if (oClientIDO.m_nPID == CLIENT_ALL)
        bRange |= 1;//B 0000 0001
    else if (oClientIDO.m_nPID == CLIENT_NONE)
        bRange |= 2;//B 0000 0010
    if (oServIDO.m_nPID >= 0)
        bRange |= 12;//B 0000 1100
    else if (oServIDO.m_nPID == SERV_ALL)
        bRange |= 4;//B 0000 0100
    else if (oServIDO.m_nPID == SERV_NONE)
        bRange |= 8;//B 0000 1000
    //2����鷶Χ�Ƿ���Ч���������OP_RECEIVE����������ļ����
    if ((bRange & 3) == 0 || (bRange & 12) == 0 || (nOperationNum == OP_RECEIVE && bRange != 15))
        return 0;
    //3.>����������������
    SServNode *pServView = NULL;
    if ((bRange & 12) != 8)
    {
        if (TryEnterCS(this->m_pServLinkLock,this->m_bObjectRelease) == false)
            return 0;
        if (oServIDO.m_nPID >= 0) //ֻ��Ե����ڵ����
        {
            pServView = (SServNode *)FindMemory(this->m_oServPool,oServIDO.m_nPID);
            if (pServView == NULL || pServView->m_bAddedLink == false || oServIDO.m_chUID != pServView->m_oIDO.m_chUID) //���ڵ��Ƿ���ʧЧ
            {
                ExitCS(this->m_pServLinkLock);
                return 0;
            }
        }
        else
            pServView = this->m_pHeadServNode;
    }
    //4������ָ����Χ�ķ������˶���
    int nResult = 0;
    while (pServView != NULL || oServIDO.m_nPID == SERV_NONE)
    {
        SClientNode *pClientView = NULL;
        //4.1��>>A.��������Ч�������������������� >>B.����������Ч�������޹����ͻ�������
        SServNode *pNextServView;
        if (pServView != NULL)
        {
            if (TryEnterCS(pServView->m_pClientLinkLock,pServView->m_bExit) == false)
                break;
            pNextServView = pServView->m_pNext;
            if (oClientIDO.m_nPID >= 0) //ֻ��Ե����ڵ����
                pClientView = (SClientNode *)FindMemory(pServView->m_oClientPool,oClientIDO.m_nPID);
            else if (oClientIDO.m_nPID == CLIENT_ALL)
                pClientView = (SClientNode *)pServView->m_pHeadClientNode;
        }
        else
        {
            if (TryEnterCS(this->m_pClientLinkLock,this->m_bObjectRelease) == false)
                break;
            if (oClientIDO.m_nPID >= 0)
                pClientView = (SClientNode *)FindMemory(this->m_oClientPool,oClientIDO.m_nPID);
            else if (oClientIDO.m_nPID == CLIENT_ALL)
                pClientView = this->m_pHeadClientNode;
        }
        if (pClientView == NULL || pClientView->m_bAddedLink == false || (bRange == 15 && oClientIDO.m_chUID != pClientView->m_oIDO.m_chUID) ) //���ڵ��Ƿ���ʧЧ
            pClientView = NULL;
        //4.2��OP_CLOSE����ר�ã���Ҫ�رյ���һ���̳߳��ڵ���������ʱ����������ֱ���Թ�����Ŀͻ��˱���
        bool bActiveCallBack = true;
        if (oClientIDO.m_nPID < 0 && nOperationNum == OP_CLOSE)
        {
            bActiveCallBack = *((bool *)pParam);
            if (oClientIDO.m_nPID == CLIENT_ALL)
            {
                this->_CloseNodes(pServView,NULL,bActiveCallBack,false);
                pClientView = NULL;
            }
            else if (oClientIDO.m_nPID == CLIENT_NONE && pServView != NULL)
            {
                this->_CloseNodes(pServView,NULL,bActiveCallBack,true);
                goto tag_NextServ;
            }
        }
        //4.3�������ͻ���
        while (pClientView != NULL)
        {
            SClientNode *pNextClientView = pClientView->m_pNext;
            //4.3.1��ִ�в���
            nResult = 0;
            switch (nOperationNum)
            {
            case OP_MANAGE:
            {
                SManageParam *pManageParam = (SManageParam *)pParam;
                nResult = this->_ManageClient(pServView,pClientView,pManageParam->m_eManageType,pManageParam->m_pfRecv,pManageParam->m_pfDisconnect,pManageParam->m_pUser);
                break;
            }
            case OP_SEND:
            {
                SSendReceiveParam *pSendParam = (SSendReceiveParam *)pParam;
                if (pClientView->m_eProtocol == TCP_Protocol)
                    nResult = this->m_pCommonSocket->SendSocket(pClientView->m_oSocket,pSendParam->m_pUnion.u_pData,pSendParam->m_nLen);
                else if (pClientView->m_eProtocol == UDP_Protocol)
                    nResult = this->m_pCommonSocket->SendSocket(pClientView->m_oSocket,pSendParam->m_pUnion.u_pData,pSendParam->m_nLen,(sockaddr *)(&pClientView->m_uAddr),pClientView->m_nAddrLen);
                else
                    break;
                if (nResult == SOCKET_ERROR && (pClientView->m_eProtocol != UDP_Protocol || pClientView->m_bIsUDPServ == false)) //���������쳣����������Ӳ���UDP Serv������caseֱ��ת��OP_CLOSE����
                {
                    pClientView->m_bErrorClose = true;
                    goto tag_OP_CLOSE;
                }
                break;
            }
            case OP_RECEIVE:
            {
                SSendReceiveParam *pReceiveParam = (SSendReceiveParam *)pParam;
                if (pClientView->m_eProtocol == TCP_Protocol)
                    nResult = this->m_pCommonSocket->ReceiveSocket(pClientView->m_oSocket,pReceiveParam->m_pUnion.u_pReceiveBuf,pReceiveParam->m_nLen);
                else if (pClientView->m_eProtocol == UDP_Protocol)
                    nResult = this->m_pCommonSocket->ReceiveSocket(pClientView->m_oSocket,pReceiveParam->m_pUnion.u_pReceiveBuf,pReceiveParam->m_nLen,(sockaddr *)(&pClientView->m_uAddr),&pClientView->m_nAddrLen);
                else
                    break;
                if (nResult == SOCKET_ERROR && (pClientView->m_eProtocol != UDP_Protocol || pClientView->m_bIsUDPServ == false)) //���������쳣����caseֱ��ת��OP_CLOSE����ȥ�������ӣ��������Ͽ����ӵĻص�����
                {
                    pClientView->m_bErrorClose = true;
                    goto tag_OP_CLOSE;
                }
                break;
            }
            case OP_CLOSE:
tag_OP_CLOSE:
                this->_CloseNodes(pServView,pClientView,bActiveCallBack,true);
                break;
            }
            //4.3.2���ƶ�ָ��
            if (oClientIDO.m_nPID >= 0)
                break;
            else
                pClientView = pNextClientView;
        }
        //4.4��>>A.��������Ч������������������� >>B.����������Ч������޹����ͻ������� | �ƶ�ָ��
        if (pServView != NULL)
            ExitCS(pServView->m_pClientLinkLock);
        else
            ExitCS(this->m_pClientLinkLock);
tag_NextServ:
        if (oServIDO.m_nPID != SERV_ALL)
            break;
        else
            pServView = pNextServView;
    }
    //5��<�������������ڴ����������
    if ((bRange & 12) != 8)
        ExitCS(this->m_pServLinkLock);
    return nResult;
}

/*
 * ���������й�ָ��������/�����飨Ҳ�����ڶ����йܵ�����/�������������ûص��������û��Զ������ݣ�
 * @info 1��������ֻ�ܰѴ���NET_NONE״̬������תΪeManageType����ָ����������NET_NONE�й�״̬������Ƕ�һ����NET_NONE�й�״̬�����ӵ��ñ����������������eManageType���������������ӵĻص��������û��Զ���������Ϣ��
 *       2������йܵ�Ϊ�������ӣ���ֻҪ��һ�������йܳɹ������᷵�سɹ���ʵ���ж�������Ҫ����������nClientID��nServID����ֵ��ͬ�����ģ���nClientID=CLIENT_UNDEFINE,nServID=0�����ʾ����Ϊ0�ķ��������µ��������Ӿ����й�
 *       3��NET_SYSTEM_RELATIVE�йܷ�ʽ��win�µı��ַ����֣�����tcp����ʹ��NET_WIN32_IOCP��������udp��ʹ��NET_THREAD����
 *       4������δ�ɹ��������ݵ�UDP�ͻ��ˣ��йܻᵼ����������ݳ�����ر�
 *       5����Ϣ����ص�����ֻ���ڸ����Ӵ����й�״̬ʱ�Żᴥ���������ӶϿ��ص������ڸ�����δ���йܵ����������������쳣�Ͽ�����Ҳ�ᱻ����
 * @param in SNodeID oClientIDO,����ʶ��ṹ�������m_nPID��ʹ��CLIENT_ALL��ʾ������nServID�µ��������ӣ���CLIENT_NONEֵ�ڴ˺�������Ч��
 * @param in SNodeID oServIDO,���������ķ������˵�ʶ��ṹ�������ͨ��CreateClient���������ӣ���ṹ���m_nPID����SERV_NONE��Ҳ��ʹ��SERV_ALL��ʾ������Ч�ķ�������
 * @param in EManageType eManageType,�йܷ�ʽ�������Ѵ����й�״̬�����ӣ��˲����ᱻ����
 * @param in Recv_FuncP pfRecv = NULL,���յ���Ϣʱ�Ļص�����
 * @param in Disconnect_FuncP pfDisconnect = NULL,���ӶϿ�ʱ�Ļص����������й�״̬�£����ָ���˻ص�����������CloseNet������ָ���������ص����Ͽ����ӵ�����⣬��������µĶϿ����ᴥ���ص�������
 * @param in void *pUser = NULL,�û��Զ���Ĵ����ص�����������ָ��
 * @return int,�йܳɹ�����1�����򷵻�0������й����Ӳ�ֹһ������˷���ֵ����ӳ���һ���������ӵ��йܽ��
 */
int CNetManager::ManageClient(SNodeID oClientIDO,SNodeID oServIDO,EManageType eManageType,Recv_FuncP pfRecv,Disconnect_FuncP pfDisconnect,void *pUser)
{
    SManageParam sManageParam;
    sManageParam.m_eManageType = eManageType;
    sManageParam.m_pfRecv = pfRecv;
    sManageParam.m_pfDisconnect = pfDisconnect;
    sManageParam.m_pUser = pUser;
    return this->_RangeOperation(oClientIDO,oServIDO,OP_MANAGE,&sManageParam);
}

/*
 * ��˽�С�ManageClient���ڲ��棬�й�ָ�������ӣ�����������飬������Ҳ�����ⲿ����ʱ����
 * @info 1��NET_SYSTEM_RELATIVE�йܷ�ʽ��win�µı��ַ����֣�����tcp����ʹ��NET_WIN32_IOCP��������udp��ʹ��NET_THREAD����
 *       2��udp��֧��NET_WIN32_IOCP����ʹ�ô��йܷ�ʽ������0����֧�ֵ�ԭ����ʹ��WSARecvFrom����ʱ�����ṩ�㹻��Ļ������Ž��յ����ݣ�����iocp��
 *          ���յ����ݺ���������234��ERROR_MORE_DATA���󡣶�tcp����Դ���0��С������������iocp����ʱ��recv�����ݡ�����udp������ϵͳ�ڲ���������
 *          �أ����þͻ�ֱ�Ӷ�ʧ�����Ҫ��udpʹ��iocp���������ÿ��udp�������˸���һ��ר�õĻ���������ɶ�����ڴ濪��������Ƿ�������������
 * @param in SServNode *pServNode,Ҫ�������ӹ���ķ������ڵ�ָ�룬û�й��������������NULL
 * @param in SClientNode *pClientNode,ָ��Ҫ���������Ӷ�Ӧ�Ľṹָ�룬����Ĭ�ϴ˲�������Ч�ģ�����NULL���
 * @param in EManageType eManageType,�йܷ�ʽ�������Ѵ����й�״̬�����ӣ��˲����ᱻ����
 * @param in Recv_FuncP pfRecv = NULL,���յ���Ϣʱ�Ļص�����
 * @param in Disconnect_FuncP pfDisconnect = NULL,���ӶϿ�ʱ�Ļص����������й�״̬�£����ָ���˻ص�����������CloseNet������ָ���������ص����Ͽ����ӵ�����⣬��������µĶϿ����ᴥ���ص�������
 * @param in void *pUser = NULL,�û��Զ���Ĵ����ص�����������ָ��
 * @return int,�йܳɹ�����1�����򷵻�0������й����Ӳ�ֹһ������˷���ֵ����ӳ���һ���������ӵ��йܽ��
 */
int CNetManager::_ManageClient(SServNode *pServNode,SClientNode *pClientNode,EManageType eManageType,Recv_FuncP pfRecv,Disconnect_FuncP pfDisconnect,void *pUser)
{
    pClientNode->m_pUser = pUser;
    pClientNode->m_pfRecv = pfRecv;
    pClientNode->m_pfDisconnect = pfDisconnect;
    if (pClientNode->m_eManagerType == NET_NONE)
    {
        if (eManageType == NET_THREAD || (eManageType == NET_SYSTEM_RELATIVE && pClientNode->m_eProtocol == UDP_Protocol))
        {
            SClientNodeThreadParam sClientNodeThreadParam;
            sClientNodeThreadParam.m_pServNode = pServNode;
            sClientNodeThreadParam.m_pClientNode = pClientNode;
            sClientNodeThreadParam.m_pNetManager = this;
            sClientNodeThreadParam.m_bHasRead = false;
            if (CreateThreadManager(_ForClientNodeThread,true,&sClientNodeThreadParam) == NULL)//���������߳�
                return 0;
            while (sClientNodeThreadParam.m_bHasRead == false)
                Sleep_Thread(5);
        }
        else
        {
#ifdef WIN32
            if (eManageType == NET_SYSTEM_RELATIVE || (eManageType == NET_WIN32_IOCP && pClientNode->m_eProtocol == TCP_Protocol)) //����TCP���ô��йܷ�ʽ
            {
                if (this->m_hIOCP_Handle == NULL) //��ʼ��IOCP
                {
                    this->m_nCurCreatedIOCPThreadCount = 0;
                    this->m_nCurRunningIOCPThreadCount = 0;
                    this->m_dwWSARecvFlag = 0;
                    this->m_oWSARecvZeroBuf.len = 0;
                    this->m_hIOCP_Handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);
                    if (this->m_hIOCP_Handle == NULL)
                        return 0;
                    this->m_pCurIOCPThreadCountVarLock = CreateCS();
                    if (CreateThreadManager(_ForIOCPThreadManageThread,true,this) == NULL)//���������߳�
                        return 0;
                }
                memset(&(pClientNode->m_OverLapped),0,sizeof(OVERLAPPED));
                if (CreateIoCompletionPort((HANDLE)this->m_pCommonSocket->GetSocket(pClientNode->m_oSocket),this->m_hIOCP_Handle,0,0) == NULL)
                    return 0;
                int nResult;
                if (pClientNode->m_eProtocol == TCP_Protocol) //�����첽����
                    nResult = WSARecv(this->m_pCommonSocket->GetSocket(pClientNode->m_oSocket),&(this->m_oWSARecvZeroBuf),1,&(this->m_dwWSARecvReceiveLen),&(this->m_dwWSARecvFlag),&(pClientNode->m_OverLapped),NULL);
                //else ��ֹ��UDPʹ��IOCP��ʽ����
                //    nResult = WSARecvFrom(this->m_pCommonSocket->GetSocket(pClientNode->m_oSocket),&(this->m_oWSARecvZeroBuf),1,&(this->m_dwWSARecvReceiveLen),&(this->m_dwWSARecvFlag),(sockaddr *)(&(pClientNode->m_uAddr)),&(pClientNode->m_nAddrLen),&(pClientNode->m_OverLapped),NULL);
                if (nResult == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != WSA_IO_PENDING)
                        return 0;
                }
            }
#else
#ifdef LINUX
            if (eManageType == NET_SYSTEM_RELATIVE || eManageType == NET_LINUX_EPOLL)
            {
                ;
            }
#else
            return 0;
#endif
#endif
        }
        pClientNode->m_eManagerType = eManageType;
    }
    return 1;
}

/*
 * ���������Ͽ�ָ��������/������
 * @info ����Ҫ�ر�ĳ���������µ��������ӣ������رշ������ˣ���nClientIDӦ������ΪCLIENT_ALL������Ҫ����������һ��ر�ʱ��nClientIDӦ����ΪCLIENT_NONE
 * @param in SNodeID oClientIDO,���ӱ�ʶ���ݣ������m_nPID�ֶο�ʹ��CLIENT_ALL��ʾ������nServID�µ��������ӻ�CLIENT_NONE��ʾ���κ������޹�
 * @param in SNodeID oServIDO,���������ķ������˵ı�ʶ���ݣ������ͨ��CreateClient���������ӣ����������m_nPID�ֶ�����SERV_NONE����������SERV_ALL��ʾ������Ч�ķ�������
 * @param in bool bActiveDisconnectCallBack,�ر�����/������ʱ�Ƿ񴥷���Ӧ�ص�����
 * @return void
 */
void CNetManager::CloseNet(SNodeID oClientIDO,SNodeID oServIDO,bool bActiveDisconnectCallBack)
{
    this->_RangeOperation(oClientIDO,oServIDO,OP_CLOSE,&bActiveDisconnectCallBack);
}

/*
 * ����������ָ�������ӷ�����Ϣ
 * @info 1���˺�������UDP��������ͬ����Ч������÷��������Ѿ��յ���ĳ����ַ��������Ϣ����˷��Ͳ�����Ŀ����Ϊ���һ���յ���Ϣ�ķ��ͷ������򽫷���ʧ��
 *       2��������ʱ����SOCKET_ERRORʱ����������Ӳ�����ΪUDP�������ˣ���ᱻ�����رղ�����
 * @param in SNodeID oClientIDO,���ӱ�ʶ���ݣ������m_nPID�ֶο�ʹ��CLIENT_ALL��ʾ������nServID�µ��������ӣ����Ը��ֶ�����CLIENT_NONEֵ�ڴ˺�������Ч��
 * @param in SNodeID oServIDO,���������ķ������˵ı�ʶ���ݣ������ͨ��CreateClient���������ӣ����������m_nPID����SERV_NONE��Ҳ��ʹ��SERV_ALL��ʾ������Ч�ķ�������
 * @param in const char *pData,Ҫ���͵��������ڵĻ�����
 * @param in nDataLen,Ҫ���͵��ֽ���
 * @return int,����ʵ�ʷ��ͳ��ȣ����ӳ��ִ��󷵻�SOCKET_ERROR���������󷵻�0,���Ҫ�������ݵ����Ӳ�ֹһ������˷���ֵ����ӳ���һ���������ӵķ��ͽ��
 */
int CNetManager::SendNet(SNodeID oClientIDO,SNodeID oServIDO,const char *pData,int nDataLen)
{
    SSendReceiveParam sSendParam;
    sSendParam.m_pUnion.u_pData = pData;
    sSendParam.m_nLen = nDataLen;
    return this->_RangeOperation(oClientIDO,oServIDO,OP_SEND,&sSendParam);
}

/*
 * ����������ָ�����ӽ�����Ϣ�����������������Ϊ�йܣ��˺�����ִ��ʧ��
 * @info 1���������йܵ��������ӣ��˺�����������κ����ݣ�ֻ�Ƿ���0
 *       2���������UDP�ͻ���ʹ�ô˺�������UDP�ͻ���ʹ�ô˺����ᵼ�����ֿ��ܣ���δ�ɹ�ʹ�øÿͻ��˷�������ǰ�������᷵��SOCKET_ERROR��ͬʱ���ӻᱻ�رգ�
 *                                                                            ���ɹ����͹����ݺ������ı������Ӷ˿�������ģ����Ա�����ָ���������˿ڵ�UDP���ݵ������ʹ�˺�������
 *       3��������SOCKET_ERROR����ʱ�������ô�������ӻᱻ�رղ���ϵͳ����
 *       4���˺�����֧�ֶԶ������ͬʱ����
 * @param in SNodeID oClientIDO,���ӱ�ʶ���ݣ������m_nPID�ֶβ���ʹ��CLIENT_ALL��CLIENT_NONEֵ������ָ����������
 * @param in SNodeID oServIDO,���������ķ������˵ı�ʶ���ݣ������ͨ��CreateClient���������ӣ���ṹ���m_nPIDӦ����SERV_NONE���˺�������ֶβ�֧��ʹ��SERV_ALLֵ
 * @param in char *pDataBuf,��Ž��յ������ݵĻ�����
 * @param in int nBufLen,pDataBufָ�򻺳����Ĵ�С���ɴ�ŵ�����ֽ�����
 * @return int,����ʵ�ʽ��յ����ݵĳ��ȣ����ӳ��ִ��󷵻�SOCKET_ERROR���������󷵻�0��������Ӵ����й�״̬��Ҳ����0
 */
int CNetManager::ReceiveNet(SNodeID oClientIDO,SNodeID oServIDO,char *pDataBuf,int nBufLen)
{
    SSendReceiveParam sReceiveParam;
    sReceiveParam.m_pUnion.u_pReceiveBuf = pDataBuf;
    sReceiveParam.m_nLen = nBufLen;
    return this->_RangeOperation(oClientIDO,oServIDO,OP_RECEIVE,&sReceiveParam);
}

/*
 * ��˽�С��رշ�����/�ͻ��˽ڵ㣬�ѽڵ������մ�����У����Ӻ�������Ͼ����Ƿ񴥷��رջص�
 * @addition info,1���˺�������ָ���Ľڵ��Ƿ񻹹����������������Ƿ�ִ�в���
 *                2�������Ĳ����ֳɽڵ�������жϿ��Ͱѽڵ���봦����У��ȴ����߳�ȥ����
 *                3��pServNode��pClientNode�����Ч��Ϊ��
 *                   |  NULL   |   ��NULL  |  �ر�ָ�����޹����ͻ���
 *                   |  NULL   |    NULL   |  �ر������޹����ͻ���
 *                   | ��NULL  |   ��NULL  |  �ر�ָ���Ĺ����ͻ���
 *                   | ��NULL  |    NULL   |  �ر�ָ���������������й����ͻ���
 * @param in SServNode *pServNode,�����������ķ������ڵ�ָ�룬��pClientNode��Ͼ���ִ��Ч��
 * @param in SClientNode *pClientNode,Ҫ���������ӽڵ�ָ�룬��pServNode��Ͼ���ִ��Ч��
 * @param bool bActiveCallBack,�Ƿ񴥷��رջص�����
 * @param bool bRecycleServNode,���pClientNode = NULL,pServNode��NULL����ϣ�ָ���Ƿ����Serv�ڵ�
 * @return void
 */
void CNetManager::_CloseNodes(SServNode *pServNode,SClientNode *pClientNode,bool bActiveCallBack,bool bRecycleServNode)
{
    if (pServNode != NULL)
    {
        if (TryEnterCS(pServNode->m_pClientLinkLock,pServNode->m_bExit) == false)
            return;
    }
    else
    {
        if (TryEnterCS(this->m_pClientLinkLock,this->m_bObjectRelease) == false)
            return;
    }
    if (pClientNode != NULL)
    {
        if (pClientNode->m_bAddedLink == false)
        {
            if (pServNode != NULL)
                ExitCS(pServNode->m_pClientLinkLock);
            else
                ExitCS(this->m_pClientLinkLock);;
            return;
        }
        if (pClientNode->m_pPre != NULL)
            pClientNode->m_pPre->m_pNext = pClientNode->m_pNext;
        else
        {
            if (pServNode != NULL)
                pServNode->m_pHeadClientNode = pClientNode->m_pNext;
            else
                this->m_pHeadClientNode = pClientNode->m_pNext;
        }
        if (pClientNode->m_pNext != NULL)
            pClientNode->m_pNext->m_pPre = pClientNode->m_pPre;
        pClientNode->m_bAddedLink = false;
        pClientNode->m_bExit = true;
        if (bActiveCallBack == false)
            pClientNode->m_pfDisconnect = NULL;
    }
    else
    {
        if (pServNode != NULL)
        {
            if (bRecycleServNode == true)
            {
                if (TryEnterCS(this->m_pServLinkLock,this->m_bObjectRelease) == false)
                {
                    ExitCS(pServNode->m_pClientLinkLock);
                    return;
                }
                if (pServNode->m_bAddedLink == false)
                {
                    ExitCS(pServNode->m_pClientLinkLock);
                    ExitCS(this->m_pServLinkLock);
                    return;
                }
                SServNode *pViewServ = this->m_pHeadServNode;
                if (pViewServ == pServNode)
                    this->m_pHeadServNode = pServNode->m_pNext;
                else
                {
                    while (pViewServ != NULL && pViewServ->m_pNext != pServNode)
                        pViewServ = pViewServ->m_pNext;
                    if (pViewServ != NULL)
                        pViewServ->m_pNext = pServNode->m_pNext;
                }
                pServNode->m_bAddedLink = false;
                pServNode->m_bExit = true;
                ExitCS(this->m_pServLinkLock);
            }
            pClientNode = (SClientNode *)pServNode->m_pHeadClientNode;
            pServNode->m_pHeadClientNode = NULL;
        }
        else
        {
            pClientNode = this->m_pHeadClientNode;
            this->m_pHeadClientNode = NULL;
        }
        while (pClientNode != NULL)
        {
            pClientNode->m_bAddedLink = false;
            pClientNode->m_bExit = true;
            if (bActiveCallBack == false)
                pClientNode->m_pfDisconnect = NULL;
            pClientNode = pClientNode->m_pNext;
        }
    }
    if (pServNode != NULL)
        ExitCS(pServNode->m_pClientLinkLock);
    else
        ExitCS(this->m_pClientLinkLock);
    //���봦�����
    if (TryEnterCS(this->m_pWriteLock,this->m_bObjectRelease) == true)
    {
        int nNextWriteIndex = this->m_nWriteIndex + 1;
        if (nNextWriteIndex >= NET_RECYCLE_ARRAY_SIZE)
            nNextWriteIndex = 0;
        while (nNextWriteIndex == this->m_nReadIndex) //ѭ����������
            Sleep_Thread(5);
        this->m_aRecycle[this->m_nWriteIndex].m_pServNode = pServNode;
        this->m_aRecycle[this->m_nWriteIndex].m_pClientNode = pClientNode;
        this->m_aRecycle[this->m_nWriteIndex].m_bRecycleServNode = bRecycleServNode;
        this->m_nWriteIndex = nNextWriteIndex;
        ExitCS(this->m_pWriteLock);
    }
}
