/*
 * ��������࣬��CommonSocketģ������϶�����������з�װ ͷ�ļ� By Lzy
 * 2012-05-02   V1.0.1.0    1������IOCP�����¶Ͽ�������δ�����������
 *                          2����������ʶ����룬��ֹ���ѻ��յ������õ����Ӳ���
 *                          3�����¿������̳߳�ͻ����
 * 2012-02-27   V1.0.0.0    ������ɣ�δ����linux��epoll����
 * 2012-01-27   V0.0.0.0    �����ļ�
 */

#ifndef LZY_NET_MANAGER_H
#define LZY_NET_MANAGER_H

#include "CommonSocket.h"
#include "CommonThread.h"

//===================ģ�����������=================
#define NET_MANAGER_RECEIVE_BUFFER_SIZE 512 //�����й����ӵ�����µ����������ݻ������Ĵ�С
#define NET_RECYCLE_ARRAY_SIZE 256 //��������/�ͻ��˽ڵ�����ѭ�����еĴ�С
#ifdef WIN32
#define MAX_IOCP_THREAD_COUNT 0 //����IOCP�̳߳�������߳�����������ֵС�ڵ���0ʱ����ʾ������
#endif

#define SERV_NONE   -1 //ָδ���ض��������˹���
#define SERV_ALL    -2 //��ָ����������ID����ʾ��������Ϊȫ�������
#define CLIENT_NONE -1 //ָδ���ض��ͻ��˹���
#define CLIENT_ALL  -2 //��ָ���ͻ���ID����ʾ��������Ϊ���������ID�����µ����пͻ���

//===================������Ŷ���===================
#define OP_MANAGE   0 //�йܲ���
#define OP_SEND     1 //ͬ��������Ϣ����
#define OP_RECEIVE  2 //ͬ��������Ϣ����
#define OP_CLOSE    3 //�ر����Ӳ���

//===================����ֵ�õĽṹ�嶨��============
struct SNodeID //���ӽڵ㣨SServNode��SClientNode�ڵ㣩����Чʶ����Ϣ����ֹ�ռ临�ú�ԭ���Ľڵ���������
{
    int m_nPID;//�������ڴ�������ڵ��ID��
    unsigned char m_chUID;//����Ƿ񱻸��ö����ID�ţ��ú�ÿ�η����+1

    bool operator == (const SNodeID &oCompare)
    {
        if (oCompare.m_nPID == this->m_nPID && oCompare.m_chUID == this->m_chUID)
            return true;
        else
            return false;
    }
};

//===================�ص���������===================
/*
 * �����������accept��������ʱ�Ļص���������
 * @param in SNodeID oClientIDO,�����ӵ����ӱ�ʶ����
 * @param in SNodeID oServIDO,�յ���������ӵķ������˱�ʶ����
 * @param in void *pUser,�û�����
 * @return bool,����true��ʾ������������ӣ�����false��ʾ�����գ���ѡ�񲻽���ʱ����������ӻᱻ�ر�
 */
typedef bool (*Accept_FuncP)(SNodeID oClientIDO,SNodeID oServIDO,void *pUser);

/*
 * ���������յ�����Ϣʱ�Ļص���������
 * @addition info,�˻ص�����ֻ���ڸ����Ӵ����й�״̬�²��ܱ�����
 * @param in SNodeID oClientIDO,������Ϣ�����ӵ����ӱ�ʶ����
 * @param in SNodeID oServIDO,������Ϣ�����������ķ������˱�ʶ����
 * @param in ENUM_Protocol eProtocol,ָ���������ͣ�TCP_Protocol,UDP_Protocol��
 * @param in const char *pData,�յ�������
 * @param in int nDataLen,�յ������ݵĳ���
 * @param in void *pUser,�û�����
 * @return void
 */
typedef void (*Recv_FuncP)(SNodeID oClientIDO,SNodeID oServIDO,ENUM_Protocol eProtocol,const char *pData,int nDataLen,void *pUser);

/*
 * �������ӶϿ�ʱ�Ļص���������
 * @param in SNodeID oClientIDO,�Ͽ������ӵ����ӱ�ʶ����
 * @param in SNodeID oServIDO,�Ͽ������������ķ������˱�ʶ����
 * @param in ENUM_Protocol eProtocol,ָ���������ͣ�TCP_Protocol,UDP_Protocol��
 * @param in bool bError,ָʾ��ǰ�ĶϿ����쳣�Ͽ�(true)���������Ͽ�(false)
 * @param in void *pUser,�û�����
 * @return void
 */
typedef void (*Disconnect_FuncP)(SNodeID oClientIDO,SNodeID oServIDO,ENUM_Protocol eProtocol,bool bError,void *pUser);
//==================================================

enum EManageType //ָ���й�����
{
    NET_NONE,//δ�Ϲ����ⲿ���й�����ֵ�������ڲ���
    NET_THREAD,//�����̹߳���
    NET_WIN32_IOCP,//win��ר�ã�IOCP��ʽ����
    NET_LINUX_EPOLL,//linux��ר�ã�ePoll��ʽ����
    NET_SYSTEM_RELATIVE,//��Ӧϵͳ��ʶ��win��tcp�Ŀͻ���ʹ��IOCP,udp�ķ�������ʹ��thread��������udp��������ʹ��IOCP�ǳ����ڴ����Ŀ��ǣ�,linux��ʹ��ePool
};

class CNetManager
{
public:
    static CNetManager *GetInstance();//��ȡ��ʵ���������ʵ�����������Զ�����
    static void ReleaseInstance();//�ͷ���ʵ��
    SNodeID CreateTCPServ(const char *pszIP,ENUM_IPVersion eIPVer,unsigned int nPort,Accept_FuncP pfAccept = NULL,void *pUser = NULL);//����TCP��������
    SNodeID CreateUDPServ(const char *pszIP,ENUM_IPVersion eIPVer,unsigned int nPort,EManageType eManageType,Recv_FuncP pfRecv = NULL,Disconnect_FuncP pfDisconnect = NULL,void *pUser = NULL);//����UDP��������
    SNodeID CreateClient(const char *pszIP,ENUM_IPVersion eIPVer,unsigned int nPort,ENUM_Protocol eProtocol,EManageType eManageType,Recv_FuncP pfRecv = NULL,Disconnect_FuncP pfDisconnect = NULL,void *pUser = NULL);//�����ͻ���
    int ManageClient(SNodeID oClientIDO,SNodeID oServIDO,EManageType eManageType,Recv_FuncP pfRecv = NULL,Disconnect_FuncP pfDisconnect = NULL,void *pUser = NULL);//�й�ָ��������/�����飨Ҳ���ô˺�������ָ������/������Ļص��������û����ݣ�
    int SendNet(SNodeID oClientIDO,SNodeID oServIDO,const char *pData,int nDataLen);//��ָ�������ӷ�����Ϣ
    int ReceiveNet(SNodeID oClientIDO,SNodeID oServIDO,char *pDataBuf,int nBufLen);//��ָ�����ӽ�����Ϣ�����������������Ϊ�йܣ��˺�����ִ��ʧ��
    void CloseNet(SNodeID oClientIDO,SNodeID oServIDO,bool bActiveDisconnectCallBack);//�Ͽ�ָ��������/������
private:
    //------�ṹ��------
    struct SServNode //�����������˵Ľڵ�
    {
        void *m_oSocket;//CommonSocket����Ķ���
        Accept_FuncP m_pfAccept;//accept�ص�����
        void *m_oClientPool;//��������accept�������ӵ��ڴ�ع������
        void *m_pHeadClientNode;//���¹��������accept�������γɵ�����ı�ͷ
        CRITICAL_SECTION *m_pVisitingCountLock;//�޸�m_nVisitingCount�����Ļ�����
        CRITICAL_SECTION *m_pClientPoolLock;//���ʱ��ṹ���е�m_oClientPool�Ļ�����
        CRITICAL_SECTION *m_pClientLinkLock;//������Ŀͻ�������ķ��ʡ��޸Ļ�����
        bool m_bExit;//֪ͨ����˽ṹ���߳��˳��ı��
        bool m_bAddedLink;//ָʾ�ڵ��Ƿ��Ѽ��뵽������
        SNodeID m_oIDO;//��ǰ�ڵ��ID���ݽṹ
        void *m_pUser;//�û����������
        int m_nVisitingCount;//ָ����ǰ�������˽ڵ���Ϣ���ڱ����ʵļ�������Ҫ�رմ˷������˽ڵ�ʱ��Ҫ�ȴ���ֵΪ0ʱ���ܲ�������Ϊ��ֵ�ĸı�ֻ�ڵ��߳��н��У���������������
        SServNode *m_pNext;//�������е���һ�ڵ�
    };

    struct SClientNode //�������ӵĽڵ�
    {
#ifdef WIN32
        OVERLAPPED m_OverLapped;//�ص��ṹ�������Ϊӭ��WSARecv��Ҫ��û������;��ʹ��IOCP�й�ʱ�õ���
#endif
        void *m_oSocket;//CommonSocket����Ķ���
        union {sockaddr_in6 u_oAddr6Client;sockaddr_in u_oAddr4Client;}m_uAddr;//UDPר�ã����淢�ͷ���Ϣ/���ý��շ���Ϣ
        socklen_t m_nAddrLen;//UDPר�ã�ָ��m_uAddr��ʵ��ʹ�ô�С
        bool m_bIsUDPServ;//UDPר�ã�ָ����ǰ�ڵ��Ƿ���ΪUDP��������ʹ��
        ENUM_Protocol m_eProtocol;//��������
        Recv_FuncP m_pfRecv;//���յ���Ϣʱ�Ļص�����
        Disconnect_FuncP m_pfDisconnect;//�Ͽ�ʱ�Ļص�����
        void *m_pUser;//�û����������
        EManageType m_eManagerType;//ָ�������ӵ��й�״̬
        SNodeID m_oServIDO;//�����ķ�������ID�ṹ��û����������˹����������m_nPID��ΪSERV_NONE
        SNodeID m_oIDO;//��ǰ�ڵ��ID���ݽṹ
        bool m_bErrorClose;//ָ�������Ƿ��������رգ��쳣�رգ�����ʼֵΪfalse
        int m_nVisitingCount;//ָ����ǰ�ͻ��˽ڵ���Ϣ���ڱ����ʵļ�������Ҫ�رմ˿ͻ��˽ڵ�ʱ��Ҫ�ȴ���ֵΪ0ʱ���ܲ���
        CRITICAL_SECTION *m_pVisitingCountLock;//�޸�m_nVisitingCount����ֵʱ��Ҫ����
        bool m_bExit;//�˽ṹ�㼴�������յı��
        bool m_bAddedLink;//ָʾ�ڵ��Ƿ��Ѽ��뵽������
        SClientNode *m_pPre;//˫�����е���һ�ڵ�
        SClientNode *m_pNext;//˫�����е���һ�ڵ�
    };

    struct SManageParam //����йܲ���(OP_MANAGE))�����Ľṹ��
    {
        EManageType m_eManageType;//�й�����
        Recv_FuncP m_pfRecv;//���յ���Ϣʱ�Ļص�����
        Disconnect_FuncP m_pfDisconnect;//�Ͽ�����ʱ�Ļص����������й�״̬�£����ָ���˻ص�����������CloseNet������ָ���������ص����Ͽ����ӵ�����⣬��������µĶϿ����ᴥ���ص�������
        void *m_pUser;//�û��Զ���Ĵ����ص�����������ָ��
    };

    struct SSendReceiveParam //��ŷ���/������Ϣ����(OP_SEND)�����Ľṹ��
    {
        union{const char *u_pData;char *u_pReceiveBuf;}m_pUnion;//Ҫ���͵��������ڵĻ�����/���յ������ݴ�ŵĻ�����
        int m_nLen;//Ҫ�������ݵ��ֽ���/�������ݻ������Ĵ�С���ֽ�����
    };

    struct SClientNodeThreadParam //��Ŵ��͸�_ForClientNodeThread���������Ľṹ��
    {
        SServNode *m_pServNode;//��pClientNode��ָ���ӹ�����SServNode�ڵ�ָ�룬��������Ӳ������ɷ����������������ģ�����������SERV_NONE
        SClientNode *m_pClientNode;//��������ӽڵ�ָ��
        CNetManager *m_pNetManager;//��ǰ��CNetManager����
        bool m_bHasRead;//�ṹ���ڵ������Ƿ��ѱ���ȡ�ı�ǣ���ֹ�ṹ������ʧЧ
    };

    struct SServNodeThreadParam //��Ŵ��͸�_ForServNodeThread���������Ľṹ��
    {
        SServNode *m_pServNode;//����ķ������ڵ�ָ��
        CNetManager *m_pNetManager;//��ǰ��CNetManager����
        bool m_bHasRead;//�ṹ���ڵ������Ƿ��ѱ���ȡ�ı�ǣ���ֹ�ṹ������ʧЧ
    };

    struct SLoopNodes //���մ���ѭ������ı�ڵ�ṹ
    {
        SServNode *m_pServNode;//��������ָ��
        SClientNode *m_pClientNode;//�ͻ���ָ��
        bool m_bRecycleServNode;//�����������Ƿ������ˣ�����ֶξ����Ƿ���շ������ڵ�
    };

    //------����------
    //  ��ʵ��ָ��
    static CNetManager *m_pNetManager;//���൥ʵ����ʵ��ʵ��ָ��
    CCommonSocket *m_pCommonSocket;//CCommonSocket��ʵ��ָ��
    //  �����������
    void *m_oServPool;//������з������˵���Ϣ���ڴ�ع������
    CRITICAL_SECTION *m_pServPoolLock;//���ʱ����Ա�е�m_oServPool�Ļ�����
    CRITICAL_SECTION *m_pServLinkLock;//���ʡ��޸ķ�����������Ļ�����
    SServNode *m_pHeadServNode;//����������Ϣ������׽ڵ�
    //  Client�����
    void *m_oClientPool;//���������CreateClient�������������ӵ���Ϣ���ڴ�ع������
    CRITICAL_SECTION *m_pClientPoolLock;//���ʱ����Ա�е�m_oClientPool�Ļ�����
    CRITICAL_SECTION *m_pClientLinkLock;//���ʡ��޸��޷������󶨵Ŀͻ�������Ļ�����
    SClientNode *m_pHeadClientNode;//Client����Ϣ������׽ڵ�
    //  �ڵ㴦��ѭ���������
    CRITICAL_SECTION *m_pWriteLock;//m_nWriteIndex���ʡ��޸Ļ�����
    int m_nWriteIndex;//ѭ������дָ��
    int m_nReadIndex;//ѭ�����ж�ָ��
    SLoopNodes m_aRecycle[NET_RECYCLE_ARRAY_SIZE];//ѭ������
    //  ����
    bool m_bObjectRelease;//ָʾ�Ƿ�Ҫ�ͷŶ���
#ifdef WIN32
    //  IOCP�������
    HANDLE m_hIOCP_Handle;
    unsigned int m_nCurCreatedIOCPThreadCount;//��ǰ�Ѵ�����IOCP�����õ��߳���
    unsigned int m_nCurRunningIOCPThreadCount;//��ǰ���н��մ����е��߳���Ŀ���޸Ĵ�ֵ��Ҫ�ӻ�����
    CRITICAL_SECTION *m_pCurIOCPThreadCountVarLock;//�޸�CurRunningIOCPThreadCount/m_nCurCreatedIOCPThreadCountֵʱ�Ļ�����
    //  ӭ��WSARecv������Ҫ�����õı���
	DWORD m_dwWSARecvReceiveLen;//�����WSARecv������lpNumberOfBytesRecvd����
	DWORD m_dwWSARecvFlag;//�����WSARecv������lpFlags����
	WSABUF m_oWSARecvZeroBuf;//�����WSARecv������lpBuffers����������Ϊ�㳤�ȵĻ�����
#endif
    //------����------
    CNetManager();
    ~CNetManager();
    CNetManager(const CNetManager &){};
    static void _ForMemPoolReleaseServNode(void *pObject,void *pCommonSocket);//MemPool�ͷ�SServNode�ṹʱ���õĺ���
    static void _ForMemPoolReleaseClientNode(void *pObject,void *pCommonSocket);//MemPool�ͷ�SClientNode�ṹʱ���õĺ���
    static void *_ForServNodeThread(void *pObject);//ÿ���������˵����ܵ��߳�
    static void *_ForClientNodeThread(void *pObject);//ʹ�ö����̹߳��������ӵ��йܷ�ʽʱ���̺߳���
    static void *_ForNodeManage(void *pObject);//����Ҫ�رջ���յĽڵ���̺߳���
#ifdef WIN32
    static void *_ForIOCPThreadManageThread(void *pObject);//���IOCP�߳�ʹ��״��������IOCP�̵߳��̺߳���
    static void *_ForIOCPPoolThread(void *pObject);//IOCP�̳߳�ʹ�õ��̺߳���
#endif
    int _RangeOperation(SNodeID oClientIDO,SNodeID oServIDO,int nOperationNum,void *pParam);//���ض���Χ�����ӽ���ָ������
    int _ManageClient(SServNode *pServNode,SClientNode *pClientNode,EManageType eManageType,Recv_FuncP pfRecv,Disconnect_FuncP pfDisconnect,void *pUser);//ManageClient���ڲ��棬�й�ָ�������ӣ�����������飬������Ҳ�����ⲿ����ʱ����
    SNodeID _CreateClient(const char *pszIP,ENUM_IPVersion eIPVer,unsigned int nPort,ENUM_Protocol eProtocol,bool bIsUDPServ,EManageType eManageType,Recv_FuncP pfRecv,Disconnect_FuncP pfDisconnect,void *pUser);//����TCP�ͻ���,UDP�ͻ��˻��������
    void _CloseNodes(SServNode *pServNode,SClientNode *pClientNode,bool bActiveCallBack,bool bRecycleServNode);//�رշ�����/�ͻ��˽ڵ㣬�ѽڵ������մ�����У����Ӻ�������Ͼ����Ƿ񴥷��رջص�
};

#endif