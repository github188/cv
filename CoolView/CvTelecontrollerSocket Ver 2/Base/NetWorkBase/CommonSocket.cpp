/*
 * ��������ඨ���ļ� By Lzy
 * @info ���ಿ�ֺ������̰߳�ȫ���ⲻ����ָͬһʱ��ͬ�����������ÿ��ܳ����⣬Ҳ������Ѻ����в�ͬ�����ĵ��ã�Ҫ����̰߳�ȫ�ĺ�������Ҫ����MUL_THREAD����ⲿ���б�֤
 * 2012-02-27   V1.0.2.5    ����GetSocket��GetSockAddr������������
 * 2012-02-26   V1.0.2.4    1������UDP�õ�SendSocket��ReceiveSocket�������������
 *                          2��ConnectSocket�������Ӷ�addrinfo�ṹ���ͷ�
 * 2012-02-03   V1.0.2.3    ����ConnectSocket��BindSocket����������IP����Ϊconst����
 * 2011-12-14   V1.0.2.2    ����BindSocket������û���ͷ�addrinfo�ṹ�������ڴ�й©������
 * 2011-12-05   V1.0.2.1    ʹ��CommonThreadģ������ģ��Ķ��߳�֧��
 * 2011-11-25   V1.0.2.0    1��ֱ��ʹ��SimpleMemPoolģ�����ģ���������Ϣ�洢
 *                          2����ԭ��cpp��ȫ�ֵ�NumCStr�����ĳ����Ա����_NumCStr
 *                          3��SockNode�ṹ���϶�IP�汾�ļ�¼��ͬʱɾȥ���ӺͰ�����������Ҫ����IP�汾�Ĳ���
 *                          4�����Ʋ���ע��
 * 2011-11-23   V1.0.1.0    �޸�ԭ��������Ĵ洢��ʽΪ����飬�Լ����ڴ���Ƭ�����̷��䣬�ͷŵ�ʱ��
 * 2011-11-22   V1.0.0.0    By Lzy
 * 2011-04-01   V0.0.0.0    �����ļ� By Lzy
 */

#include "CommonSocket.h"

using namespace simple_mem_pool;

CCommonSocket *CCommonSocket::SocketClassP = NULL;

/*
 * ��˽�С�����̬�����̳߳�ר�á�Ϊ�̳߳س�ʼ��������
 * @param void *pObject,��Ч�Ķ���ָ��
 * @param void *pNULL,������Ϊһ����ָ�룬����Ϻ���������裬�����õ�
 * @return void
 */
void CCommonSocket::ForMemPoolInitialSock(void *pObject,void *pNULL)
{
    SocketNode *pNode = (SocketNode *)pObject;
    pNode->SetUsing(false);
}

/*
 * ��˽�С�����̬�����̳߳�ר�á�Ϊ�̳߳��ͷŶ�����
 * @param void *pObject,��Ч�Ķ���ָ��
 * @param void *pNULL,������Ϊһ����ָ�룬����Ϻ���������裬�����õ�
 * @return void
 */
void CCommonSocket::ForMemPoolReleaseSock(void *pObject,void *pNULL)
{
    SocketNode *pNode = (SocketNode *)pObject;
    if (pNode->GetUsing() != false)
    {
        CCommonSocket::_CloseSock(pNode->Socket);
        pNode->SetUsing(false);
    }
}

/*
 * ��˽�С����캯�������Win����ϵͳ�ж���Ĵ�������
 */
CCommonSocket::CCommonSocket()
{
    this->InitSuccess = true;
#ifdef MUL_THREAD
    this->DestructObject = false;
    this->OpLock = CreateCS();
    if (this->OpLock == NULL)
    {
        this->InitSuccess = false;
        return;
    }
#endif
    this->MemPoolNode = CreateMemPool(sizeof(SocketNode),CCommonSocket::ForMemPoolInitialSock,CCommonSocket::ForMemPoolReleaseSock);
    if (this->MemPoolNode == NULL)
    {
        this->InitSuccess = false;
        return;
    }
#ifdef WIN32
    WSADATA wsaData;
    int rc = WSAStartup(MAKEWORD(2,2),&wsaData);
    if (rc!=0)
        this->InitSuccess = false;
#endif
}

/*
 * ��˽�С��������������Win����ϵͳ�ж���Ĵ��������趨����MUL_THREAD������̰߳�ȫ��
 */
CCommonSocket::~CCommonSocket()
{
#ifdef MUL_THREAD
    this->DestructObject = true;
    EnterCS(this->OpLock);
#endif
    ReleaseMemPool(&this->MemPoolNode);
#ifdef MUL_THREAD
    ReleaseCS(this->OpLock);
#endif
#ifdef WIN32
    if (this->InitSuccess)
        WSACleanup();
#endif
}

/*
 * ������������̬������ʵ���������ʵ��δ�����򴴽�һ������ʵ������ֻ�ᴴ��һ��
 * @return CCommonSocket*,�����´�����ʵ�����󣬸�ʵ������ֻ����ReleaseInstance�ͷ�
 */
CCommonSocket *CCommonSocket::GetInstance()
{
    if (CCommonSocket::SocketClassP == NULL)
        CCommonSocket::SocketClassP = new CCommonSocket();
    return CCommonSocket::SocketClassP;
}

/*
 * ������������̬���ͷŴ�����ʵ������
 * @return void
 */
void CCommonSocket::ReleaseInstance()
{
    if (CCommonSocket::SocketClassP != NULL)
    {
        delete CCommonSocket::SocketClassP;
        CCommonSocket::SocketClassP = NULL;
    }
}

/*
 * �������������µ�Socket���趨����MUL_THREAD������̰߳�ȫ��
 * @param ENUM_IPVersion IP_Version,ʹ�õ�IP�汾
 * @param ENUM_Protocol Protocol,ʹ�õ�Э��
 * @return void*,���ظ�Socket��ʶ���ַ
 */
void *CCommonSocket::CreateSocket(ENUM_IPVersion IP_Version,ENUM_Protocol Protocol)
{
    if (this->InitSuccess == false)
        return NULL;
    int Domain,Type;
    if (IP_Version == Version4)
        Domain = AF_INET;
    else
        Domain = AF_INET6;
    if (Protocol == TCP_Protocol)
        Type = SOCK_STREAM;
    else
        Type = SOCK_DGRAM;
    SOCKET NewSocket = socket(Domain,Type,0);
    if (NewSocket != INVALID_SOCKET)
    {
        SocketNode *NewNode = this->_GetSocketNode();
        if (NewNode == NULL)
        {
            this->_CloseSock(NewSocket);
            return NULL;
        }
        NewNode->IP_Version = IP_Version;
        NewNode->Socket = NewSocket;
        NewNode->SetUsing(true);
        return NewNode;
    }
    else
        return NULL;
}

/*
 * ��������ΪSocket��IP��ַ�Ͷ˿ں�
 * @param void *SocketIdentify,Socket��ʶ����ת��ΪSocketNode�ṹָ��ȥ��������������ָ���Ƿ��ж�Ӧ�Ľڵ�
 * @param const char *IP,Ҫ������IP������ΪNULL��ʹ��Ĭ��
 * @param int Port,Ҫ�����Ķ˿�
 * @return int,�緢�����󽫷���SOCKET_ERROR
 */
int CCommonSocket::BindSocket(void *SocketIdentify,const char *IP,int Port)
{
    if (SocketIdentify == NULL)
        return SOCKET_ERROR;
    SocketNode *Node = (SocketNode*)SocketIdentify;
    addrinfo Hints,*addr;
    memset(&Hints,0,sizeof(Hints));
    if (Node->IP_Version != Version6)
        Hints.ai_family = AF_INET;
    else
        Hints.ai_family = AF_INET6;
    Hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
    char PortStr[10];
    this->_NumCStr(Port,&(PortStr[0]));
    if (getaddrinfo(IP,PortStr,&Hints,&addr) != 0)
        return SOCKET_ERROR;
    int nResult = bind(Node->Socket,addr->ai_addr,addr->ai_addrlen);
    freeaddrinfo(addr);
    return nResult;
}

/*
 * ����������socket�ļ���
 * @param void *SocketIdentify,Socket��ʶ����ת��ΪSocketNode�ṹָ��ȥ��������������ָ���Ƿ��ж�Ӧ�Ľڵ�
 * @return int,�緢�����󽫷���SOCKET_ERROR
 */
int CCommonSocket::ListenSocket(void *SocketIdentify)
{
    if (SocketIdentify == NULL)
        return SOCKET_ERROR;
    SocketNode *Node = (SocketNode*)SocketIdentify;
    return listen(Node->Socket,SOMAXCONN);
}

/*
 * ��������ת��socket���趨����MUL_THREAD������̰߳�ȫ��
 * @param void *SocketIdentify,����Socket��ʶ����ת��ΪSocketNode�ṹָ��ȥ��������������ָ���Ƿ��ж�Ӧ�Ľڵ�
 * @return int,�����µ�socket��Ϣ��ʶ���緢�����󽫷���NULL
 */
void * CCommonSocket::AcceptSocket(void *SocketIdentify)
{
    if (SocketIdentify == NULL)
        return NULL;
    SocketNode *Node = (SocketNode*)SocketIdentify;
    SOCKET NewSocket = accept(Node->Socket,NULL,NULL);
    if (NewSocket != INVALID_SOCKET)
    {
        SocketNode *NewNode = this->_GetSocketNode();
        if (NewNode == NULL)
        {
            this->_CloseSock(NewSocket);
            return NULL;
        }
        NewNode->IP_Version = Node->IP_Version;
        NewNode->Socket = NewSocket;
        NewNode->SetUsing(true);
        return NewNode;
    }
    else
        return NULL;
}

/*
 * ��������������������
 * @param void *SocketIdentify,Socket��ʶ����ת��ΪSocketNode�ṹָ��ȥ��������������ָ���Ƿ��ж�Ӧ�Ľڵ�
 * @param const char *IP,Ҫ���ӵ�IP
 * @param int Port,Ҫ���ӵĶ˿�
 * @return int,�緢�����󽫷���SOCKET_ERROR
 */
int CCommonSocket::ConnectSocket(void *SocketIdentify,const char *IP,int Port)
{
    if (SocketIdentify == NULL)
        return SOCKET_ERROR;
    SocketNode *Node = (SocketNode*)SocketIdentify;
    addrinfo Hints,*addr;
    memset(&Hints,0,sizeof(Hints));
    if (Node->IP_Version != Version6)
        Hints.ai_family = AF_INET;
    else
        Hints.ai_family = AF_INET6;
    Hints.ai_flags = AI_NUMERICHOST;
    char PortStr[10];
    this->_NumCStr(Port,&(PortStr[0]));
    if (getaddrinfo(IP,PortStr,&Hints,&addr) != 0)
        return SOCKET_ERROR;
    int nResult = connect(Node->Socket,addr->ai_addr,addr->ai_addrlen);
    freeaddrinfo(addr);
    return nResult;
}

/*
 * ���������ر�ָ����SOCKET���趨����MUL_THREAD������̰߳�ȫ��
 * @param void *SocketIdentify,Socket��ʶ����ת��ΪSocketNode�ṹָ��ȥ����
 * @return int,�緢�����󽫷���SOCKET_ERROR
 */
int CCommonSocket::CloseSocket(void *SocketIdentify)
{
    if (this->InitSuccess == false)
        return SOCKET_ERROR;
    if (SocketIdentify == NULL)
        return SOCKET_ERROR;
    SocketNode *Node = (SocketNode*)SocketIdentify;
#ifdef MUL_THREAD
    if (TryEnterCS(this->OpLock,this->DestructObject) == false)
        return 0;
#endif
    RecycleMemory(this->MemPoolNode,Node);
#ifdef MUL_THREAD
    ExitCS(this->OpLock);
#endif
    return 0;
}

/*
 * �����������ѽ��������ӵ�Socket��������
 * @param void *SocketIdentify,Socket��ʶ����ת��ΪSocketNode�ṹָ��ȥ����
 * @param const char *Data,Ҫ�������ݵ�ָ��
 * @param int Len,�������ݵĳ���
 * @return ssize_t,�緢�����󽫷���SOCKET_ERROR�����򽫷��ط��ͳ�ȥ�����ݳ���
 */
ssize_t CCommonSocket::SendSocket(void *SocketIdentify,const char *Data,int Len)
{
    if (SocketIdentify == NULL)
        return SOCKET_ERROR;
    SocketNode *Node = (SocketNode*)SocketIdentify;
    return send(Node->Socket,Data,Len,0);
}

/*
 * �����������ѽ��������ӵ�Socket����������
 * @param void *SocketIdentify,Socket��ʶ����ת��ΪSocketNode�ṹָ��ȥ����
 * @param const char *Data,��Ž��յ������ݵ�ָ��
 * @param int Len,���ݽ������Ĵ�С
 * @return ssize_t,�緢�����󽫷���SOCKET_ERROR�����򽫷��ؽ��յ������ݳ���
 */
ssize_t CCommonSocket::ReceiveSocket(void *SocketIdentify,char *RecvData,int MaxLen)
{
    if (SocketIdentify == NULL)
        return SOCKET_ERROR;
    SocketNode *Node = (SocketNode*)SocketIdentify;
    return recv(Node->Socket,RecvData,MaxLen,0);
}

/*
 * ���������������ݲ���÷��ͷ���Ϣ
 * @param void *SocketIdentify,Socket��ʶ����ת��ΪSocketNode�ṹָ��ȥ����
 * @param const char *Data,��Ž��յ������ݵ�ָ��
 * @param int Len,���ݽ������Ĵ�С
 * @param sockaddr *From,���ڴ洢���ͷ���Ϣ��ָ��
 * @param socklen_t *FormLen,���From��ΪNULL����������ʱ����д��Form�Ľṹ�Ĵ�С
 * @return ssize_t,�緢�����󽫷���SOCKET_ERROR�����򽫷��ؽ��յ������ݳ���
 */
ssize_t CCommonSocket::ReceiveSocket(void *SocketIdentify, char *RecvData, int MaxLen,sockaddr *From,socklen_t *FromLen)
{
    if (SocketIdentify == NULL)
        return SOCKET_ERROR;
    SocketNode *Node = (SocketNode*)SocketIdentify;
    return recvfrom(Node->Socket,RecvData,MaxLen,0,From,FromLen);
}

/*
 * ����������ָ�����շ���������
 * @param void *SocketIdentify,Socket��ʶ����ת��ΪSocketNode�ṹָ��ȥ����
 * @param const char *Data,���Ҫ�������ݵ�ָ��
 * @param int Len,Ҫ�������ݵĴ�С
 * @param sockaddr *To,����ָ�����ͷ���Ϣ��ָ��
 * @param socklen_t ToLen,Toָ��ָ��Ľṹ�Ĵ�С
 * @return ssize_t,�緢�����󽫷���SOCKET_ERROR�����򽫷��ؽ��յ������ݳ���
 */
ssize_t CCommonSocket::SendSocket(void *SocketIdentify, const char *Data, int Len,sockaddr *To,socklen_t ToLen)
{
    if (SocketIdentify == NULL)
        return SOCKET_ERROR;
    SocketNode *Node = (SocketNode*)SocketIdentify;
    return sendto(Node->Socket,Data,Len,0,To,ToLen);
}

/*
 * �����������sockaddr����
 * @param ENUM_IPVersion IP_Version,����汾��
 * @param const char *IP,����IP��ַ
 * @param int Port,���Ķ˿ں�
 * @param sockaddr *TargetSockAddr,Ҫ����sockaddr����ָ��
 * @return socklen_t,�������Ķ���Ĵ�С�����ʧ�ܷ���0
 */
socklen_t CCommonSocket::GetSockAddr(ENUM_IPVersion IP_Version,const char *IP,int Port,sockaddr *TargetSockAddr)
{
    addrinfo Hints,*addr;
    if (TargetSockAddr == NULL)
        return 0;
    memset(&Hints,0,sizeof(Hints));
    if (IP_Version != Version6)
        Hints.ai_family = AF_INET;
    else
        Hints.ai_family = AF_INET6;
    Hints.ai_flags = AI_NUMERICHOST;
    char PortStr[10];
    this->_NumCStr(Port,&(PortStr[0]));
    if (getaddrinfo(IP,PortStr,&Hints,&addr) != 0)
        return 0;
    memcpy(TargetSockAddr,addr->ai_addr,addr->ai_addrlen);
    socklen_t AddrLen = addr->ai_addrlen;
    freeaddrinfo(addr);
    return AddrLen;
}

/*
 * ���������ӹ���ṹ�л�ȡSOCKET����
 * @param void *SocketIdentify,Socket��ʶ����ת��ΪSocketNode�ṹָ��ȥ����
 * @return SOCKET,���ش�SocketIdentify��ȡ�õ�SOCKET���󣬻�ȡʧ�ܷ���0
 */
SOCKET CCommonSocket::GetSocket(void *SocketIdentify)
{
    if (SocketIdentify == NULL)
        return 0;
    else
        return ((SocketNode*)SocketIdentify)->Socket;
}

/*
 * ��˽�С�����������ȡ����SocketNode�ڵ㣬�趨����MUL_THREAD������̰߳�ȫ��
 * @return SocketNode *,���ؿ��ýڵ�ָ�룬���������ֱ���׳��쳣
 */
CCommonSocket::SocketNode *CCommonSocket::_GetSocketNode()
{
#ifdef MUL_THREAD
    if (TryEnterCS(this->OpLock,this->DestructObject) == false)
        return NULL;
    SocketNode *pTmpNode = (SocketNode *)AllocMemory(this->MemPoolNode);
    ExitCS(this->OpLock);
    return pTmpNode;
#else
    return (SocketNode *)AllocMemory(this->MemPoolNode);
#endif
}

/*
 * ��˽�С�����������Win32�汾����10������ֵת���ַ����ĺ��������ڶ˿ں�ת����
 * @param int Num,Ҫת��������
 * @param char *Buf,�ַ���������
 * @return void
 */
 void CCommonSocket::_NumCStr(int Num,char *Buf)
{
#ifdef WIN32
    _itoa(Num,Buf,10);
#else
    int BufP = 0;
    int CountBit = 10;
    while (Num / CountBit > 0)
    {
        ++BufP;
        CountBit *= 10;
    }
    CountBit /= 10;
    Buf[BufP + 1] = '\0';
    int LoopVar = 0;
    while (LoopVar <= BufP)
    {
        Buf[LoopVar] = '0' + (Num / CountBit);
        Num = Num - (Num / CountBit) * CountBit;
        ++LoopVar;
        CountBit /= 10;
    }
#endif
}
