/*
 * ֧�ֶϵ��������ֶδ�����ļ��ϴ������صķ�������ģ�飬��֧�ֶ�4G�����ļ��Ĳ��������ͬʱ�����ļ���Ϊ5000��
 * 2012-05-17   V1.0.0.0    ����
 * 2012-05-09   V0.0.0.0    �����ļ�
 */

#include <stdio.h>
#include <string>
#include <map>
using namespace std;

#include "CommonSocket.h"
#include "CommonThread.h"
using namespace thread_operation;

#define CFUDS_CHECK_GAP_TIME 500 //���´�����Ϣ�ڵ�ȴ�������ʱ���̵߳ĸ��¼��������Ϊ��λ����ʵ�ʸ��¼������Ҫ�ڴ�ֵ�����ϼ����������ʱ�������һЩϵͳ����ʱ��
#define CFUDS_ENABLED_IPV4 //���������ʶ�����IPv4���Ӵ�������
#define CFUDS_ENABLED_IPV6 //���������ʶ�����IPv6���Ӵ�������

#define CFUDS_READ_WRITE_BUF 3072 //����ÿ�����Ӷ���д�ļ���Ĵ�С���ֽ�Ϊ��λ������Ϊint�������ֵ�����޲�Ӧ��С��CFUDS_RECV_BUFֵ�� �ϴ�״̬�����յ����������ﵽ��ֵ�������쳣�Ͽ�ʱ�ͻ�����ݻ�д���ļ��� ����״̬��ÿ�δ��ļ���ȡ��ֵ��С��������
#define CFUDS_RECV_BUF 128 //����ÿ�����ӽ����������ݵĻ�������С����ֵ�ĳ�������������ָ���������ݵĴ��䣬Ĭ��ֵ128���������޸�

class CFileUpDownServ //����ң�����ϴ��ļ����ն˵���
{
public:
    static inline CFileUpDownServ *GetInstance(){return CFileUpDownServ::m_pCFUDInstance;};//��ȡ��������ʹ��ǰ��������CreateInstance��������
    static void ReleaseInstance();//�ͷŵ�������
    static CFileUpDownServ *CreateInstance(int nListenPort,short snMaxTransNum,unsigned int uWaitOverTime);//������ʵ���������ʵ���Ѵ�����ֱ�ӷ���
    int RegUpdateTrans(const char *pszFilePath,const char *pszInfoFilePath,long long llFileSize,unsigned int uMaxConnectionCount,unsigned short snBlockCount,bool bIsOverWrite,long long &rllFinished);//�����ϴ���Ϣ���������ϴ�Ȩ���룬��Ȩ�����ڴ���ǰ��Ҫ��֤
    int RegDownloadTrans(const char *pszFilePath,unsigned int uMaxConnectionCount,long long &rllFileSize);//����������Ϣ������������Ȩ���룬��Ȩ�����ڴ���ǰ��Ҫ��֤

private:
    //------�ṹ��------
    struct SUpdatePartNode //�ϴ��ֿ��������Ϣ
    {
        long long m_llStart;//�����������������
        long long m_llEnd;//�յ���������������
        void *m_pSocket;//������˿��ϴ����ݵ�����
        long long m_llFinished;//�˿��Ѵ����������
        SUpdatePartNode *m_pNext;//��һ�ڵ��ַ
    };

    struct STransNode //�������ѭ���б��нڵ�ṹ
    {
        unsigned int m_uTimeLeft;//ʣ��ĵȴ�����ʱ�䣨ֻ�����ú�����ɨ���̻߳�ı����ֵ��ɨ���߳�ÿ��ɨ��ʱ�������m_uCurConnectionCount=0������ֵ��1����0ʱ�������ڵ���Ч��
        short m_snIdentify;//��ʶֵ����ڵ�������ͬ���һ��int32���ͣ���Ϊ��������ʶ���
        bool m_bIsUp;//��ǿͻ����Ƿ��ϴ���������������ز�������Ϊfalse
        unsigned int m_uMaxConnectionCount;//������ļ��������������
        unsigned int m_uCurConnectionCount;//Ϊ������ļ���ǰ������������
        CRITICAL_SECTION *m_pCurConnectionLock;//�޸�m_uCurConnectionCountֵ�������߳�д��m_pNewSocketIdentifyʱ�Ļ�����
        FILE *m_pTransFile;//�����ļ��Ĳ������
        CRITICAL_SECTION *m_pReadWriteFileLock;//����д�ļ�ʱ����
        void *m_pNewSocketIdentify;//�½��������ӵ�socket��ʶ���ϴ�������ڵ�����߳��ڴ�����Ѵ��ֶι�NULL�������µ����ӿ��ٴ�д�� ��������������߳����а����Ӽ��뵽�����������У��Ա�����ʱ�ͷ�
        int m_nNextIndex;//����ڵ㴦����Ч״̬��m_uTimeLeft > 0�������ʾ��һ��ʹ���нڵ������ֵ�������ʾ��һ�����нڵ������
        //------���ϴ�ר�á�-------
        long long m_llBlockSize;//�ϴ��ļ��ķֿ��С���ֽ�Ϊ��λ
        long long m_llFinishedBlockData;//��ɴ���ķֿ��е��������������ڴ����еķֿ鲻�������ڣ�
        SUpdatePartNode *m_pUpdatePartList;//ʣ�ഫ�������б�ֻ�нڵ�����̻߳����������
        string m_szInfoFilePath;//������Ϣ�ļ���·��
    };

    //------����------
    static CFileUpDownServ *m_pCFUDInstance;//�൥������
    unsigned int m_uWaitOverTime;//�׸����ӷ������ȴ�ʱ��
    unsigned int m_uRandSeed;//�������
    void *m_pUpdatePartListMemPool;//����SUpdatePartNode�ṹ���ڴ��
    CRITICAL_SECTION *m_pUpdatePartListMemPoolLock;//ʹ��m_pUpdatePartListMemPool�ڴ��ʱ�Ļ�����
    STransNode *m_pTransNodeArrayList;//������Ϣ��������
    short m_snTransNodeArrayListCap;//������Ϣ���������
    int m_nNextUsingIndex;//��һ��ʹ���д�����Ϣ�ڵ�����
    int m_nNextFreeIndex;//��һ���ɷ��䴫����Ϣ�ڵ�����
    CRITICAL_SECTION *m_pAdjustNodeLock;//����������Ϣ��������m_nNextUsingIndex��m_nNextFreeIndexֵʱ����
    
    void *m_pConnectionMemPool;//���������ӱ�ʶ���ڴ�أ��Ա�����ʱ�ܿ������ӹرղ���
    CRITICAL_SECTION *m_pConnectionMemPoolLock;//ʹ��m_pConnectionMemPool�ڴ��ʱ�Ļ�����

    bool m_bIsRelease;//�����ͷű��
#ifdef CFUDS_ENABLED_IPV4
    void *m_pTCPServV4;//�����������ӵ�IPv4��������
#endif
#ifdef CFUDS_ENABLED_IPV6
    void *m_pTCPServV6;//�����������ӵ�IPv6��������
#endif

    //------����------
    CFileUpDownServ(int nListenPort,short snMaxTransNum,unsigned int uWaitOverTime);
    ~CFileUpDownServ();
    CFileUpDownServ(const CFileUpDownServ&){};
    static void *_RefreshWaitTimeoutThread(void *pNull);//�������д���ڵ�ȴ���ʱֵ���̺߳���
    static void *_SocketManageThread(void *pSocket);//��������������������ӵ��̺߳���
    static void *_UpdateNodeManageThread(void *pIndex);//�������ϴ��ڵ���̺߳���
    static void *_DownloadNodeManageThread(void *pIndex);//���������ؽڵ���̺߳���
    static void *_TransThread(void *pSocket);//���������ӵ��߳�
    static void _ForReleaseDownloadConnection(void *pObject,void *pCommonSocket);//�ڴ���ͷ���������ʱ���õĺ���
};
