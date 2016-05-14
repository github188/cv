/*
 * ֧�ֶϵ��������ֶδ�����ļ��ϴ������صķ�������ģ�飬��֧�ֶ�4G�����ļ��Ĳ��������ͬʱ�����ļ���Ϊ5000��
 * 2012-05-17   V1.0.0.0    ����
 * 2012-05-09   V0.0.0.0    �����ļ�
 */

#ifdef LINUX
#define _FILE_OFFSET_BITS 64
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "FileUD.h"
#include "SimpleMemPool.h"
#include "json.h"
using namespace simple_mem_pool;
#ifdef WIN32
#define FILE_STAT _stati64
#define STAT_STRUCT struct _stat64
#define FILE_SEEK _fseeki64_nolock
#define FILE_WRITE _fwrite_nolock
#else
#define FILE_STAT stat
#define STAT_STRUCT struct stat
#define FILE_SEEK fseek
#define FILE_WRITE fwrite
#endif

#ifdef WIN32
#pragma warning ( disable : 4996 )
#endif

//------���������------
//  ------RegUpdateTrans��RegDownloadTrans�����Ĵ�����------
#define CFUD_UPDATE_SET_PATH_NULL -1 //�ϴ�λ�ò���ΪNULL
#define CFUD_UPDATE_FILE_SIZE_ERROR -2 //�ϴ��ļ���Сֵ�쳣
#define CFUD_FILE_PATH_IS_DIR -3 //������·������Ŀ¼·��
#define CFUD_FILE_INFO_PATH_IS_DIR -4 //������ָ��������Ϣ�ļ���·������Ŀ¼·��
#define CFUD_FILE_PATH_SAME -5 //ָ���Ĵ����ļ���������Ϣ�ļ�Ϊ��ͬ�ļ����������ָ��������ͬ���ļ�
#define CFUD_UPDATE_FILE_EXIST -6 //ָ�����ϴ�λ�ô���ͬ���ļ�����δָ��������
#define CFUD_FILE_OPERATION_ERROR -7 //��Ҫ�����ļ��Ĳ���ʧ��
#define CFUD_INFO_FILE_PATH_NULL -8 //������Ϣ��¼�ļ���·������ΪNULL
#define CFUD_ERROR_UNKNOW -9 //δ֪����
#define CFUD_OBJECT_RELEASE -10 //��ʵ��������������ǰִ�еĲ�������ֹ
#define CFUD_INFO_FILE_OPERATION_ERROR -11 //���ϴ��ļ���������Ϣ��¼�ļ�����ʧ��
#define CFUD_FILE_UPDATING -12 //ָ��Ҫ�ϴ����ļ����ڱ��ϴ���
#define CFUD_TRANS_TASK_FULL -13 //��ǰ���ڴ�����ļ����ﵽ����ʵ�������ɵ����ֵ����ֵ��ʵ������ʱ��nMaxTransNum������ָ����
#define CFUD_UPDATE_BLOCK_COUNT_ERROR -14 //�ϴ��ļ��ķֿ���Ŀ�쳣
#define CFUD_ALLOC_MEMORY_FAILED -15 //�����ڴ�ʧ��
#define CFUD_CREATE_THREAD_FAILED -16 //�����߳�ʧ��
#define CFUD_DOWNLOAD_FILE_NOT_EXIST -17 //Ҫ���ص��ļ�������

//  ------�������ӷ��ظ��ͻ��˵Ĵ���Ȩ������������ResultCode�ֶΡ���------
#define CFUD_TRANS_PASS 0 //û�з�������ͬʱ��ʾ����Ȩ������ͨ��
#define CFUD_TRANS_JSON_ERROR 1 //���͵������޷���json��ʽ����
#define CFUD_TRANS_NO_TRANS_CODE 2 //ȱ�ٴ���Ȩ����
#define CFUD_TRANS_CODE_UNAVAILABLE 3 //����Ȩ�޺���Ч
#define CFUD_TRANS_OBJECT_RELEASE 4 //ģ�鼴������
#define CFUD_TRANS_CONNECTION_FULL 5 //���ļ��Ĳ�������������

//------�������ӷ��ص�JSON���ݡ�Command���ֶ��е�ֵ������Ķ���ֵ�����������޸�------
#define JSON_COMMAND_ID_RESULT 0 //���ص���ϢΪ�Դ���Ȩ��ֵ�Ĵ�����
#define JSON_COMMAND_SET_TRANS_RANGE 1 //���ص���ϢΪ��֪�ͻ������Ӵ���ķ�Χֵ
#define JSON_COMMAND_WAIT_TRANS_RANGE 2 //���ص���ϢΪҪ��ͻ��������ṩ���ط�Χֵ

CFileUpDownServ *CFileUpDownServ::m_pCFUDInstance = NULL;

/*
 * ����̬����������������ʵ���������ʵ���Ѵ�����ֱ�ӷ���
 * @addition info,nMaxTransNum���з�Χ���Ƶģ�������Ч��ΧΪ[1-5000]�������ⷶΧ�ڵİ����ֵ����
 * @param in int nListenPort,��������Ķ˿ں�
 * @param in short snMaxTransNum,֧�ֲ���������ļ��������ϴ��������ļ������㣩
 * @param in unsigned int uWaitOverTime,�ļ�������Ϣע���ȴ��ͻ��˷��������ӵ����ȴ�ʱ�䣨ʱ�䵥λΪһ��ɨ���̵߳�ɨ��ѭ����������ֵ�������0�������Զ�������
 * @return CFileUpDownServ *,���ش����õ���ʵ��������NULL��ʾ����ʧ��
 */
CFileUpDownServ *CFileUpDownServ::CreateInstance(int nListenPort,short snMaxTransNum,unsigned int uWaitOverTime)
{
    if (CFileUpDownServ::m_pCFUDInstance == NULL)
        CFileUpDownServ::m_pCFUDInstance = new CFileUpDownServ(nListenPort,snMaxTransNum,uWaitOverTime);
    return CFileUpDownServ::m_pCFUDInstance;
}

/*
 * ����̬���������������������������ͷŵ�������
 * @return void
 */
void CFileUpDownServ::ReleaseInstance()
{
    delete CFileUpDownServ::m_pCFUDInstance;
}

/*
 * ��˽�С��๹�캯��
 * @addition info,nMaxTransNum���з�Χ���Ƶģ�������Ч��ΧΪ[1-5000]�������ⷶΧ�ڵİ����ֵ����
 * @param in int nListenPort,��������Ķ˿ں�
 * @param in short snMaxTransNum,֧�ֲ���������ļ��������ϴ��������ļ������㣩
 * @param in unsigned int uWaitOverTime,�ļ�������Ϣע���ȴ��ͻ��˷��������ӵ����ȴ�ʱ�䣨ʱ�䵥λΪһ��ɨ���̵߳�ɨ��ѭ����������ֵ�������0�������Զ�������
 */
CFileUpDownServ::CFileUpDownServ(int nListenPort,short snMaxTransNum,unsigned int uWaitOverTime)
{
    this->m_bIsRelease = false;
    this->m_pUpdatePartListMemPoolLock = NULL;
    this->m_pUpdatePartListMemPool = NULL;
    this->m_pTransNodeArrayList = NULL;
    this->m_pConnectionMemPool = NULL;
    this->m_pConnectionMemPoolLock = NULL;
    this->m_pAdjustNodeLock = CreateCS();
    this->m_pUpdatePartListMemPoolLock = CreateCS();
    this->m_pConnectionMemPoolLock = CreateCS();
    if (this->m_pAdjustNodeLock == NULL || 
        this->m_pUpdatePartListMemPoolLock == NULL || 
        this->m_pConnectionMemPoolLock == NULL)
    {
        printf("[E]CFileUpDownServ:Create CREATICAL_SECTION Failed\n");
        goto tag_ConstructError;
    }
    if (uWaitOverTime < 0)
        this->m_uWaitOverTime = 1;
    else
        this->m_uWaitOverTime = uWaitOverTime;
    this->m_pUpdatePartListMemPool = CreateMemPool(sizeof(SUpdatePartNode));
    this->m_pConnectionMemPool = CreateMemPool(sizeof(void *),NULL,_ForReleaseDownloadConnection);
    if (this->m_pUpdatePartListMemPool == NULL || this->m_pConnectionMemPool == NULL)
    {
        printf("[E]CFileUpDownServ:Create MemPool Failed\n");
        goto tag_ConstructError;
    }
    //1������������Ϣ����
    if (snMaxTransNum < 1)
        snMaxTransNum = 1;
    if (snMaxTransNum > 5000)
        snMaxTransNum = 5000;
    this->m_pTransNodeArrayList = NULL;
    this->m_pTransNodeArrayList = (STransNode *)malloc(sizeof(STransNode) * snMaxTransNum);
    if (this->m_pTransNodeArrayList == NULL)
    {
        printf("[E]CFileUpDownServ:Create Array Failed\n");
        goto tag_ConstructError;
    }
    this->m_snTransNodeArrayListCap = snMaxTransNum;
    this->m_nNextUsingIndex = -1;
    this->m_nNextFreeIndex = 0;
    for (short snLoopVar = 0;snLoopVar < snMaxTransNum;++snLoopVar)
    {
        STransNode *pTmpViewNode = &(this->m_pTransNodeArrayList[snLoopVar]);
        pTmpViewNode->m_pCurConnectionLock = CreateCS();//ʡ�Դ�����
        pTmpViewNode->m_pReadWriteFileLock = CreateCS();//ʡ�Դ�����
        new (&pTmpViewNode->m_szInfoFilePath) string;//placement new
        pTmpViewNode->m_pTransFile = NULL;
        pTmpViewNode->m_pUpdatePartList = NULL;
        pTmpViewNode->m_uTimeLeft = 0;
        pTmpViewNode->m_uCurConnectionCount = 0;
        pTmpViewNode->m_nNextIndex = snLoopVar + 1;
    }
    this->m_pTransNodeArrayList[snMaxTransNum - 1].m_nNextIndex = -1;
    CFileUpDownServ::m_pCFUDInstance = this;//Ϊ�������������̲߳�������������������
    //2��������ʱ����߳�
    if (CreateThreadManager(_RefreshWaitTimeoutThread,true) == NULL)
        printf("[W]CFileUpDownServ:Can Not Create Thread For Refreshing Wait Time Out\n");
    //3����������������˼���
    CCommonSocket *pCommonSocket = CCommonSocket::GetInstance();
    if (pCommonSocket == NULL)
    {
        printf("[E]CFileUpDownServ:Network Initialize Failed\n");
        goto tag_ConstructError;
    }
#ifdef CFUDS_ENABLED_IPV4
    this->m_pTCPServV4 = pCommonSocket->CreateSocket(Version4,TCP_Protocol);
    if (this->m_pTCPServV4 != NULL)
    {
        if (pCommonSocket->BindSocket(this->m_pTCPServV4,NULL,nListenPort) == SOCKET_ERROR)
            printf("[W]CFileUpDownServ:Bind IPv4 Socket Failed\n");
        if (CreateThreadManager(_SocketManageThread,true,this->m_pTCPServV4) == NULL)
            printf("[W]CFileUpDownServ:Create IPv4 Socket Manage Thread Failed\n");
    }
    else
        printf("[W]CFileUpDownServ:IPv4 Socket Initialize Failed\n");
#endif
#ifdef CFUDS_ENABLED_IPV6
    this->m_pTCPServV6 = pCommonSocket->CreateSocket(Version6,TCP_Protocol);
    if (this->m_pTCPServV6 != NULL)
    {
        if (pCommonSocket->BindSocket(this->m_pTCPServV6,NULL,nListenPort) == SOCKET_ERROR)
            printf("[W]CFileUpDownServ:Bind IPv6 Socket Failed\n");
        if (CreateThreadManager(_SocketManageThread,true,this->m_pTCPServV6) == NULL)
            printf("[W]CFileUpDownServ:Create IPv6 Socket Manage Thread Failed\n");
    }
    else
        printf("[W]CFileUpDownServ:IPv6 Socket Initialize Failed\n");
#endif
    return;
    //E��������
tag_ConstructError:
    this->m_bIsRelease = true;
    if (this->m_pAdjustNodeLock != NULL)
        ReleaseCS(this->m_pAdjustNodeLock);
    if (this->m_pUpdatePartListMemPoolLock != NULL)
        ReleaseCS(this->m_pUpdatePartListMemPoolLock);
    if (this->m_pUpdatePartListMemPool != NULL)
        ReleaseMemPool(&this->m_pUpdatePartListMemPool);
    if (this->m_pConnectionMemPool != NULL)
        ReleaseMemPool(&this->m_pConnectionMemPool,NULL);
    if (this->m_pConnectionMemPoolLock != NULL)
        ReleaseCS(this->m_pConnectionMemPoolLock);
    if (this->m_pTransNodeArrayList != NULL)
    {
        for (short snLoopVar = 0;snLoopVar < snMaxTransNum;++snLoopVar)
        {
            STransNode *pTmpViewNode = &(this->m_pTransNodeArrayList[snLoopVar]);
            ReleaseCS(pTmpViewNode->m_pCurConnectionLock);
            ReleaseCS(pTmpViewNode->m_pReadWriteFileLock);
            pTmpViewNode->m_szInfoFilePath.~string();//placement new ��Ӧ����������
        }
    }
}

/*
 * ��˽�С�����������
 */
CFileUpDownServ::~CFileUpDownServ()
{
    if (this->m_bIsRelease == true)
        return;
    this->m_bIsRelease = true;
    //1���رշ������˼���
    CCommonSocket *pCommonSocket = CCommonSocket::GetInstance();
    if (pCommonSocket != NULL)
    {
#ifdef CFUDS_ENABLED_IPV4
        if (this->m_pTCPServV4 != NULL)
            pCommonSocket->CloseSocket(this->m_pTCPServV4);
#endif
#ifdef CFUDS_ENABLED_IPV4
        if (this->m_pTCPServV6 != NULL)
            pCommonSocket->CloseSocket(this->m_pTCPServV6);
#endif
        ;//�������գ���ֹ����������䶼�����ڵ����
    }
    //2���ͷŴ�����Ϣ���鼰�ڴ����Դ
    ReleaseMemPool(&this->m_pConnectionMemPool,pCommonSocket);//Ҫ���ͷ�����ڴ����Ϊ�˹ر�������������
    if (this->m_pTransNodeArrayList != NULL)
    {
        while (1)
        {
            while (this->m_nNextUsingIndex != -1)
                Sleep_Thread(5);
            EnterCS(this->m_pAdjustNodeLock);
            if (this->m_nNextUsingIndex != -1)
                ExitCS(this->m_pAdjustNodeLock);
            else
                break;
        }
        for (short snLoopVar = 0;snLoopVar < this->m_snTransNodeArrayListCap;++snLoopVar)
        {
            this->m_pTransNodeArrayList[snLoopVar].m_szInfoFilePath.~string();//placement new ��Ӧ����������
            ReleaseCS(this->m_pTransNodeArrayList[snLoopVar].m_pCurConnectionLock);
        }
        free(this->m_pTransNodeArrayList);
    }
    ReleaseMemPool(&this->m_pUpdatePartListMemPool);
    //3���ͷ��ٽ���
    ReleaseCS(this->m_pAdjustNodeLock);
    ReleaseCS(this->m_pUpdatePartListMemPoolLock);
    ReleaseCS(this->m_pConnectionMemPoolLock);
}

/*
 * ����̬����˽�С����ص��������ڴ���ͷ���������ʱ���õĺ���
 * @param in void *pObject,Ҫ�ͷŵ����ӵı�ʶ
 * @param in void *pCommonSocket,CCommonSocket���ʵ��ָ��
 * @return void
 */
void CFileUpDownServ::_ForReleaseDownloadConnection(void *pObject,void *pCommonSocket)
{
    if (pCommonSocket != NULL && (*(void **)(pObject)) != NULL)
    {
        ((CCommonSocket *)pCommonSocket)->CloseSocket(*(void **)(pObject));
        *(void **)(pObject) = NULL;
    }
}

/*
 * �������������ϴ���Ϣ���������ϴ�Ȩ���룬��Ȩ�����ڴ���ǰ��Ҫ��֤
 * @addition info,1������bIsOverWrite�����Ķ��ⲹ�䣺���ָ�����ļ����ڣ�������δ�ϴ���ɵ��ļ����ļ���С��Ҫ�ϴ����ļ�����ƥ�䣩��������������������Ǹ���
 *                2���ϴ��ֿ���snBlockCountֻ�Ƕ��ļ��ֿ��һ������ֵ��ģ��ʵ�ʲ���ʱ���ܻ���������
 * @param in const char *pszFilePath,�ϴ��ļ��Ĵ��·��
 * @param in const char *pszInfoFilePath,��¼�ϴ��ļ�������Ϣ���ļ��Ĵ��·����ÿ���ϴ��ļ�������Ψһ��Ӧһ��������¼�ļ�
 * @param in long long llFileSize,�ϴ��ļ��Ĵ�С
 * @param in unsigned int uMaxConnectionCount,�ϴ����ļ�ʱ����ͬʱ���������ϴ����������������Ϊ0�����ͬ�ڷ����������
 * @param in unsigned short snBlockCount,�ϴ��ļ��ķֿ�����������ļ�������״̬�������ó��ν���ʱ�����ã��˲�����Ч
 * @param in bool bIsOverWrite,������ļ��Ѿ����ڣ��Ƿ񸲸Ǹ��ļ�
 * @param out long long &rllFinished,��������ɴ�������������ֽ�Ϊ��λ��������ļ���������״̬�������������Ϊ0
 * @return int,�������ֵ>=0�����ʾ���óɹ�������ֵΪ�ϴ�Ȩ���룬�������ֵС��0���ʾ���ص��Ǵ����룬��ϸ�����붨���ڱ��ļ�ǰ����˵��
 */
int CFileUpDownServ::RegUpdateTrans(const char *pszFilePath,const char *pszInfoFilePath,long long llFileSize,unsigned int uMaxConnectionCount,unsigned short snBlockCount,bool bIsOverWrite,long long &rllFinished)
{
    //1��������
    if (pszFilePath == NULL)
        return CFUD_UPDATE_SET_PATH_NULL;
    if (pszInfoFilePath == NULL)
        return CFUD_INFO_FILE_PATH_NULL;
    if (llFileSize < 0)
        return CFUD_UPDATE_FILE_SIZE_ERROR;
    if (snBlockCount < 1)
        return CFUD_UPDATE_BLOCK_COUNT_ERROR;
    size_t nPathMaxLen = strlen(pszInfoFilePath);
    if (nPathMaxLen < strlen(pszFilePath))
        nPathMaxLen = strlen(pszFilePath);
    if (strncmp(pszFilePath,pszInfoFilePath,nPathMaxLen) == 0)
        return CFUD_FILE_PATH_SAME;
    //2�������ڵ������̲�����Ƿ��п�����Ϣ�洢�ڵ�ɷ��䣬���������߳�
    int nReturnValue = 0;
    if (TryEnterCS(this->m_pAdjustNodeLock,this->m_bIsRelease) == false)
        return CFUD_OBJECT_RELEASE;
    void *pNodeManageThread = CreateThreadManager(_UpdateNodeManageThread,true,(void *)this->m_nNextFreeIndex,true);
    if (pNodeManageThread == NULL)
    {
        nReturnValue = CFUD_CREATE_THREAD_FAILED;
        goto tag_Update_Set_Error;
    }
    if (this->m_nNextFreeIndex == -1)
    {
        nReturnValue = CFUD_TRANS_TASK_FULL;
        goto tag_Update_Set_Error;
    }
    //3������ļ��Ƿ����
    STAT_STRUCT sFileInfo;//����ļ���Ϣ
    bool bCanContinueTrans = false;//��¼�ļ��Ƿ��������
    if (FILE_STAT(pszFilePath,&sFileInfo) == 0)//���Ҫ�ϴ����ļ��Ƿ����
    {
        if ((sFileInfo.st_mode & S_IFDIR) == S_IFDIR)//��������·���Ƿ����Ŀ¼·��
        {
            nReturnValue = CFUD_FILE_PATH_IS_DIR;
            goto tag_Update_Set_Error;
        }
        if (sFileInfo.st_size == llFileSize)//���Ҫ�ϴ��ļ���С��ָ���ļ���С�Ƿ���ͬ
        {
            if (FILE_STAT(pszInfoFilePath,&sFileInfo) == 0)//����ϴ��ļ���Ӧ��������Ϣ��¼�ļ��Ƿ����
            {
                if ((sFileInfo.st_mode & S_IFDIR) == S_IFDIR)//��������������Ϣ�ļ�·���Ƿ����Ŀ¼·��
                {
                    nReturnValue = CFUD_FILE_INFO_PATH_IS_DIR;
                    goto tag_Update_Set_Error;
                }
                bCanContinueTrans = true;
            }
        }
        if (bCanContinueTrans == false && bIsOverWrite == false)
        {
            nReturnValue = CFUD_UPDATE_FILE_EXIST;
            goto tag_Update_Set_Error;
        }
    }
    //4����ʼ���ֿ��С��Ϣ
    long long llBlockSize = llFileSize / snBlockCount;
    if (llFileSize < snBlockCount)
        ++llBlockSize;
    //5����ȡ������Ϣ�ڵ㲢��ʼ��������Ϣ��������������ļ�����������֣�
    STransNode *pTargetNode = &(this->m_pTransNodeArrayList[this->m_nNextFreeIndex]);
    this->m_uRandSeed += (unsigned int)time(NULL);
    srand(this->m_uRandSeed);
    pTargetNode->m_bIsUp = true;
    pTargetNode->m_llBlockSize = llBlockSize;
    pTargetNode->m_llFinishedBlockData = 0;
    pTargetNode->m_snIdentify = rand();
    if (pTargetNode->m_snIdentify < 0)
        pTargetNode->m_snIdentify *= -1;//�˾���Ϊ�˷�ֹ���صĴ���Ȩ������32λint�ϳ��ָ�������ģ��Ҫ��int����Ϊ32λ���ȣ�
    pTargetNode->m_szInfoFilePath = pszInfoFilePath;
    //pTargetNode->m_uCurConnectionCount = 0; //��ȻΪ0������˵�������˴���
    //pTargetNode->m_pUpdatePartList = NULL; //��ȻΪNULL������˵�������˴���
    pTargetNode->m_uMaxConnectionCount = uMaxConnectionCount;
    pTargetNode->m_uTimeLeft = this->m_uWaitOverTime;
    pTargetNode->m_pNewSocketIdentify = NULL;
    //6����������ļ����ϴ���ǣ����ļ��Ƿ������ϴ��У�[��]��״̬������������ļ��ϴ���ǣ��������ϴ���Ϣ
    FILE *pTmpF = fopen(pszInfoFilePath,"r+b");
    bool bIsReading = true;
    if (pTmpF != NULL)//����ļ����ڣ��������ֽ���Ϣ
    {
        if (fread(&bIsReading,sizeof(bool),1,pTmpF) == 1)
        {
            if (bIsReading == false)
            {
                bIsReading = true;
                if (FILE_SEEK(pTmpF,0,SEEK_SET) != 0)
                    nReturnValue = CFUD_INFO_FILE_OPERATION_ERROR;;
                if (nReturnValue < 0 || FILE_WRITE(&bIsReading,sizeof(bool),1,pTmpF) != 1)//д���ϴ��б��
                    nReturnValue = CFUD_INFO_FILE_OPERATION_ERROR;
                if (nReturnValue < 0 || fread(&(pTargetNode->m_llBlockSize),sizeof(long long),1,pTmpF) != 1)//��ȡ��С�ֿ��С
                    nReturnValue = CFUD_INFO_FILE_OPERATION_ERROR;
                if (nReturnValue < 0 || fread(&(pTargetNode->m_llFinishedBlockData),sizeof(long long),1,pTmpF) != 1)//��ȡ����ɴ���ķֿ����������
                    nReturnValue = CFUD_INFO_FILE_OPERATION_ERROR;
                if (nReturnValue < 0 || fread(&snBlockCount,sizeof(snBlockCount),1,pTmpF) != 1)//��ȡ�ѻ��ֵ�������
                    nReturnValue = CFUD_INFO_FILE_OPERATION_ERROR;
                SUpdatePartNode *pPreNode = NULL;
                if (TryEnterCS(this->m_pUpdatePartListMemPoolLock,this->m_bIsRelease) == false)
                {
                    nReturnValue = CFUD_OBJECT_RELEASE;
                    fclose(pTmpF);
                    goto tag_Update_Set_Error;
                }
                while (snBlockCount > 0 && nReturnValue >= 0)
                {
                    SUpdatePartNode *pNewNode = (SUpdatePartNode *)AllocMemory(this->m_pUpdatePartListMemPool);
                    if (pNewNode != NULL)
                    {
                        if (fread(pNewNode,sizeof(SUpdatePartNode),1,pTmpF) != 1)
                            nReturnValue = CFUD_INFO_FILE_OPERATION_ERROR;
                        pNewNode->m_pSocket = NULL;
                        pNewNode->m_pNext = NULL;
                        if (pTargetNode->m_pUpdatePartList == NULL)
                            pTargetNode->m_pUpdatePartList = pNewNode;
                        if (pPreNode != NULL)
                            pPreNode->m_pNext = pNewNode;
                        pPreNode = pNewNode;
                    }
                    else
                        nReturnValue = CFUD_ALLOC_MEMORY_FAILED;
                    --snBlockCount;
                }
                if (nReturnValue < 0) //���ִ���ʱ�ͷ�����
                {
                    pPreNode = pTargetNode->m_pUpdatePartList;
                    while (pPreNode != NULL)
                    {
                        pTargetNode->m_pUpdatePartList = pPreNode->m_pNext;
                        RecycleMemory(this->m_pUpdatePartListMemPool,pPreNode);
                        pPreNode = pTargetNode->m_pUpdatePartList;
                    }
                }
                ExitCS(this->m_pUpdatePartListMemPoolLock);
            }
            else
                nReturnValue = CFUD_FILE_UPDATING;
        }
        else
            bCanContinueTrans = false;
        fclose(pTmpF);
        if (nReturnValue < 0)
            goto tag_Update_Set_Error;
    }
    //7����ʼ��Ҫ�ϴ����ļ���������Ϣ�ļ�
    if (bCanContinueTrans == false)
    {
        //7.1������Ҫ�ϴ����ļ�������Ӳ�̿ռ�
        pTmpF = fopen(pszFilePath,"wb");
        if (pTmpF == NULL)
        {
            nReturnValue = CFUD_FILE_OPERATION_ERROR;
            goto tag_Update_Set_Error;
        }
        if (FILE_SEEK(pTmpF,llFileSize - 1,SEEK_SET) != 0)
            nReturnValue = CFUD_FILE_OPERATION_ERROR;
        char bFillByte = 0;//���ֵ
        if (nReturnValue < 0 || FILE_WRITE(&bFillByte,sizeof(char),1,pTmpF) != 1)
            nReturnValue = CFUD_FILE_OPERATION_ERROR;
        fclose(pTmpF);
        if (nReturnValue < 0)
            goto tag_Update_Set_Error;
        //7.2�������ϴ��ļ���Ӧ��������Ϣ�ļ�
        pTmpF = fopen(pszInfoFilePath,"wb");
        if (pTmpF == NULL)
        {
            nReturnValue = CFUD_INFO_FILE_OPERATION_ERROR;
            goto tag_Update_Set_Error;
        }
        bIsReading = true;
        if (FILE_WRITE(&bIsReading,sizeof(bool),1,pTmpF) != 1)//д���ϴ��б��
            nReturnValue = CFUD_INFO_FILE_OPERATION_ERROR;
        if (nReturnValue < 0 || FILE_WRITE(&llBlockSize,sizeof(llBlockSize),1,pTmpF) != 1)//д����С�ֿ��С
            nReturnValue = CFUD_INFO_FILE_OPERATION_ERROR;
        if (nReturnValue < 0 || FILE_WRITE(&(pTargetNode->m_llFinishedBlockData),sizeof(long long),1,pTmpF) != 1)//д������ɴ����ĺϼ�����
            nReturnValue = CFUD_INFO_FILE_OPERATION_ERROR;
        snBlockCount = 1;
        if (nReturnValue < 0 || FILE_WRITE(&snBlockCount,sizeof(snBlockCount),1,pTmpF) != 1)//д�뻮����������
            nReturnValue = CFUD_INFO_FILE_OPERATION_ERROR;
        if (nReturnValue >= 0)//д��������Ϣ
        {
            if (TryEnterCS(this->m_pUpdatePartListMemPoolLock,this->m_bIsRelease) == false)
            {
                fclose(pTmpF);
                nReturnValue = CFUD_OBJECT_RELEASE;
                goto tag_Update_Set_Error;
            }
            pTargetNode->m_pUpdatePartList = (SUpdatePartNode *)AllocMemory(this->m_pUpdatePartListMemPool);
            if (pTargetNode->m_pUpdatePartList != NULL)
            {
                pTargetNode->m_pUpdatePartList->m_llStart = 1;
                pTargetNode->m_pUpdatePartList->m_llEnd = llFileSize;
                pTargetNode->m_pUpdatePartList->m_llFinished = 0;
                pTargetNode->m_pUpdatePartList->m_pSocket = NULL;
                pTargetNode->m_pUpdatePartList->m_pNext = NULL;
                if (FILE_WRITE(pTargetNode->m_pUpdatePartList,sizeof(SUpdatePartNode),1,pTmpF) != 1)
                {
                    RecycleMemory(this->m_pUpdatePartListMemPool,pTargetNode->m_pUpdatePartList);
                    pTargetNode->m_pUpdatePartList = NULL;
                    nReturnValue = CFUD_INFO_FILE_OPERATION_ERROR;
                }
            }
            else
                nReturnValue = CFUD_ALLOC_MEMORY_FAILED;
            ExitCS(this->m_pUpdatePartListMemPoolLock);
        }
        fclose(pTmpF);
        if (nReturnValue < 0)
            goto tag_Update_Set_Error;
    }
    //8����䴫���ļ����
    pTmpF = fopen(pszFilePath,"r+b");
    if (pTmpF == NULL)
    {
        nReturnValue = CFUD_FILE_OPERATION_ERROR;
        goto tag_Update_Set_Error;
    }
    else
        pTargetNode->m_pTransFile = pTmpF;
    //9����������ɴ����������
    rllFinished = pTargetNode->m_llFinishedBlockData;
    SUpdatePartNode *pViewUpdatePartNode = pTargetNode->m_pUpdatePartList;
    while (pViewUpdatePartNode != NULL)
    {
        rllFinished += pViewUpdatePartNode->m_llFinished;
        pViewUpdatePartNode = pViewUpdatePartNode->m_pNext;
    }
    //10�����㴫��Ȩ����͵������ýڵ�
    nReturnValue = this->m_nNextFreeIndex;//���㴫��Ȩ����
    this->m_nNextFreeIndex = pTargetNode->m_nNextIndex;
    pTargetNode->m_nNextIndex = this->m_nNextUsingIndex;
    this->m_nNextUsingIndex = nReturnValue;
    nReturnValue += (int)(pTargetNode->m_snIdentify) << 16;//�����ʶ��
    Start_Thread(pNodeManageThread);
    ExitCS(this->m_pAdjustNodeLock);
    return nReturnValue;
tag_Update_Set_Error:
    //Error���������ٽ����ͷ�
    if (pNodeManageThread != NULL)
        ReleaseThreadManager(pNodeManageThread);
    pTmpF = fopen(pszInfoFilePath,"r+b");
    if (pTmpF != NULL)
    {
        bIsReading = false;
        FILE_WRITE(&bIsReading,sizeof(bool),1,pTmpF);
        fclose(pTmpF);
    }
    else
        printf("[W]CFileUpDownServ:\"%s\" May Be Destroyed\n",pszInfoFilePath);
    ExitCS(this->m_pAdjustNodeLock);
    if (nReturnValue >= 0)
        nReturnValue = CFUD_ERROR_UNKNOW;
    return nReturnValue;
}

/*
 * ������������������Ϣ������������Ȩ���룬��Ȩ�����ڴ���ǰ��Ҫ��֤
 * @param in const char *pszFilePath,Ҫ�����ļ���λ��
 * @param in unsigned int uMaxConnectionCount,�������󲢷����������������Ϊ0�����ͬ�ڷ����������
 * @param out long long &rllFileSize,���������ļ��Ĵ�С���ֽ�Ϊ��λ
 * @return int,�������ֵ>=0�����ʾ���óɹ�������ֵΪ����Ȩ���룬�������ֵС��0���ʾ���ص��Ǵ����룬��ϸ�����붨���ڱ��ļ�ǰ����˵��
 */
int CFileUpDownServ::RegDownloadTrans(const char *pszFilePath,unsigned int uMaxConnectionCount,long long &rllFileSize)
{
    //1��������
    if (pszFilePath == NULL)
        return CFUD_UPDATE_SET_PATH_NULL;
    //2�������ڵ������̲�����Ƿ��п�����Ϣ�洢�ڵ�ɷ���
    int nReturnValue = 0;
    if (TryEnterCS(this->m_pAdjustNodeLock,this->m_bIsRelease) == false)
        return CFUD_OBJECT_RELEASE;
    if (this->m_nNextFreeIndex == -1)
    {
        nReturnValue = CFUD_TRANS_TASK_FULL;
        goto tag_Download_Set_Error;
    }
    void *pNodeManageThread = CreateThreadManager(_DownloadNodeManageThread,true,(void *)this->m_nNextFreeIndex,true);
    if (pNodeManageThread == NULL)
    {
        nReturnValue = CFUD_CREATE_THREAD_FAILED;
        goto tag_Download_Set_Error;
    }
    //3����ȡ�ļ��Ĵ�С
    STAT_STRUCT sFileInfo;//����ļ���Ϣ
    if (FILE_STAT(pszFilePath,&sFileInfo) == 0)//���Ҫ�ϴ����ļ��Ƿ����
    {
        if ((sFileInfo.st_mode & S_IFDIR) == S_IFDIR)//��������·���Ƿ����Ŀ¼·��
        {
            nReturnValue = CFUD_FILE_PATH_IS_DIR;
            goto tag_Download_Set_Error;
        }
        rllFileSize = sFileInfo.st_size;//����Ҫ�����ļ��Ĵ�С
    }
    else
    {
        nReturnValue = CFUD_DOWNLOAD_FILE_NOT_EXIST;
        goto tag_Download_Set_Error;
    }
    //4����ʼ������ڵ���Ϣ����Ҫ������ļ�
    STransNode *pTargetNode = &(this->m_pTransNodeArrayList[this->m_nNextFreeIndex]);
    this->m_uRandSeed += (unsigned int)time(NULL);
    srand(this->m_uRandSeed);
    pTargetNode->m_bIsUp = false;
    pTargetNode->m_snIdentify = rand();
    if (pTargetNode->m_snIdentify < 0)
        pTargetNode->m_snIdentify *= -1;//�˾���Ϊ�˷�ֹ���صĴ���Ȩ������32λint�ϳ��ָ�������ģ��Ҫ��int����Ϊ32λ���ȣ�
    //pTargetNode->m_uCurConnectionCount = 0; //��ȻΪ0������˵�������˴���
    pTargetNode->m_uMaxConnectionCount = uMaxConnectionCount;
    pTargetNode->m_uTimeLeft = this->m_uWaitOverTime;
    pTargetNode->m_pNewSocketIdentify = NULL;
    FILE *pTmpF = fopen(pszFilePath,"rb");
    if (pTmpF == NULL)
    {
        nReturnValue = CFUD_FILE_OPERATION_ERROR;
        goto tag_Download_Set_Error;
    }
    else
        pTargetNode->m_pTransFile = pTmpF;
    //5�����㴫��Ȩ����͵������ýڵ�
    nReturnValue = this->m_nNextFreeIndex;//���㴫��Ȩ����
    this->m_nNextFreeIndex = pTargetNode->m_nNextIndex;
    pTargetNode->m_nNextIndex = this->m_nNextUsingIndex;
    this->m_nNextUsingIndex = nReturnValue;
    nReturnValue += (int)(pTargetNode->m_snIdentify) << 16;//�����ʶ��
    Start_Thread(pNodeManageThread);
    ExitCS(this->m_pAdjustNodeLock);
    return nReturnValue;
tag_Download_Set_Error:
    //Error��������
    if (pNodeManageThread != NULL)
        ReleaseThreadManager(pNodeManageThread);
    ExitCS(this->m_pAdjustNodeLock);
    if (nReturnValue >= 0)
        nReturnValue = CFUD_ERROR_UNKNOW;
    return nReturnValue;
}

/*
 * ����̬����˽�С����̺߳������������ϴ��ڵ���̺߳���
 * @param in void *pIndex,������Ĵ���ڵ������ֵ
 * @return void *,�㷵��NULL
 */
void *CFileUpDownServ::_UpdateNodeManageThread(void *pIndex)
{
    CFileUpDownServ *pInstance = CFileUpDownServ::GetInstance();
    CCommonSocket *pCommonSocket = CCommonSocket::GetInstance();//������䷵��ֵ����������²���ΪNULL
    int nIndex = (int)pIndex;
    STransNode *pManageNode = &(pInstance->m_pTransNodeArrayList[nIndex]);
    SUpdatePartNode *pListViewNext,*pListViewPre,*pListTmp;
    SUpdatePartNode *pFreeLink = NULL;//�ݴ�Ҫ�ͷŵ�����ڵ�
    //1��������
    while (pInstance->m_bIsRelease == false && pManageNode->m_uTimeLeft > 0)
    {
        //1.1����鲢����������
        if (pManageNode->m_pNewSocketIdentify != NULL)
        {
            pListViewPre = NULL;
            pListViewNext = pManageNode->m_pUpdatePartList;
            while (pListViewNext != NULL && pListViewNext->m_pSocket != NULL)
            {
                pListViewPre = pListViewNext;
                pListViewNext = pListViewNext->m_pNext;
            }
            if (pListViewNext != NULL)//����������������˵����û��δ��ʼ����ķֿ�
            {
                if ((pListViewNext->m_llEnd - pListViewNext->m_llStart + 1) / pManageNode->m_llBlockSize >= 2)
                {
                    if (TryEnterCS(pInstance->m_pUpdatePartListMemPoolLock,pInstance->m_bIsRelease) == false)
                        break;
                    pListTmp = (SUpdatePartNode *)AllocMemory(pInstance->m_pUpdatePartListMemPool);
                    ExitCS(pInstance->m_pUpdatePartListMemPoolLock);
                    if (pListTmp != NULL)
                    {
                        pListTmp->m_llStart = pListViewNext->m_llStart;
                        pListTmp->m_llEnd = pListViewNext->m_llStart + pManageNode->m_llBlockSize - 1;
                        pListTmp->m_llFinished = 0;
                        pListTmp->m_pNext = pListViewNext;
                        pListTmp->m_pSocket = NULL;
                        pListViewNext->m_llStart = pListTmp->m_llEnd + 1;
                        if (pListViewPre != NULL)
                            pListViewPre->m_pNext = pListTmp;
                        else
                            pManageNode->m_pUpdatePartList = pListTmp;
                        pListViewNext = pListTmp;
                    }
                    else
                    {
                        printf("[E]CFileUpDownServ:Alloc Memory Failed In _UpdateNodeManageThread\n");
                        pListViewNext = NULL;
                    }
                }
                if (pListViewNext != NULL)
                    pListViewNext->m_pSocket = pManageNode->m_pNewSocketIdentify;
            }
            else//û�п��зֿ������¹ر�����
                pCommonSocket->CloseSocket(pManageNode->m_pNewSocketIdentify);
            pManageNode->m_pNewSocketIdentify = NULL;
        }
        //1.2������Ƿ����Ѿ�������ɵĿ�
        pListViewPre = NULL;
        pListTmp = pManageNode->m_pUpdatePartList;
        while (pListTmp != NULL)
        {
            if (pListTmp->m_llFinished == pListTmp->m_llEnd - pListTmp->m_llStart + 1)//�Ѵ������
            {
                pManageNode->m_llFinishedBlockData += pListTmp->m_llFinished;
                pListViewNext = pListTmp->m_pNext;
                while (pListViewNext != NULL && pListViewNext->m_pSocket != NULL)//������һδ��ʼ���������
                    pListViewNext = pListViewNext->m_pNext;
                bool bIsUnUsedBlock = false;
                void *pTmpSockIdentify = pListTmp->m_pSocket;
                pListTmp->m_pSocket = NULL;
                if (pListViewNext != NULL)//����δ��ʼ���������
                {
                    if ((pListViewNext->m_llEnd - pListViewNext->m_llStart + 1) / pManageNode->m_llBlockSize >= 2)//��������Ƿ�ɷ�
                    {
                        pListTmp->m_llStart = pListViewNext->m_llStart;
                        pListTmp->m_llEnd = pListViewNext->m_llStart + pManageNode->m_llBlockSize - 1;
                        pListTmp->m_llFinished = 0;
                        pListViewNext->m_llStart = pListTmp->m_llEnd + 1;
                        pListViewNext = pListTmp;//��������ڰ����鲻�ɷֵ������ɷֵ����ͳһ
                    }
                    else
                        bIsUnUsedBlock = true;//���鲻�ɷ֣�ֱ�������������������飬������ɵ�������գ���������ݴ������У�
                    pListViewNext->m_pSocket = pTmpSockIdentify;
                }
                else
                {
                    pCommonSocket->CloseSocket(pTmpSockIdentify);//�ر�����
                    bIsUnUsedBlock = true;//�Ѿ�û��δ��ʼ��������飬��ǰ������������
                }
                if (bIsUnUsedBlock == true)
                {
                    if (pListViewPre != NULL)
                        pListViewPre->m_pNext = pListTmp->m_pNext;
                    else
                        pManageNode->m_pUpdatePartList = pListTmp->m_pNext;
                    pListTmp->m_pNext = pFreeLink;
                    pFreeLink = pListTmp;
                    break;
                }
            }
            pListViewPre = pListTmp;
            pListTmp = pListTmp->m_pNext;
        }
        //1.3���ж��Ƿ�������ϴ�
        if (pManageNode->m_pUpdatePartList == NULL)
            break;
        else
            Sleep_Thread(5);
    }
    //2����д�������ݣ�����ڵ�����Դ
    //2.1���ر����Ӳ���ֹ�����ӵļ��룬�ȴ����������˳���ر��ϴ��ļ����
    pManageNode->m_uMaxConnectionCount = 0;
    pListViewNext = pManageNode->m_pUpdatePartList;
    while (pListViewNext != NULL)
    {
        if (pListViewNext->m_pSocket != NULL)
            pCommonSocket->CloseSocket(pListViewNext->m_pSocket);//�ر�����
        pListViewNext = pListViewNext->m_pNext;
    }
    while (pManageNode->m_uCurConnectionCount != 0)
        Sleep_Thread(5);
    fclose(pManageNode->m_pTransFile);
    pManageNode->m_pTransFile = NULL;
    //2.2����д�������ݻ�ɾ��������Ϣ�ļ�
    if (pManageNode->m_pUpdatePartList != NULL)
    {
        bool bWriteError = false;
        FILE *pInfoFile = fopen(pManageNode->m_szInfoFilePath.c_str(),"r+b");
        if (pInfoFile != NULL)
        {
            if (FILE_WRITE(&bWriteError,sizeof(bool),1,pInfoFile) != 1)//��д�ϴ��������
                bWriteError = true;
            if (bWriteError == true || FILE_WRITE(&(pManageNode->m_llBlockSize),sizeof(long long),1,pInfoFile) != 1)//��д��С�ֿ�����
                bWriteError = true;
            if (bWriteError == true || FILE_WRITE(&(pManageNode->m_llFinishedBlockData),sizeof(long long),1,pInfoFile) != 1)//��д��ɴ���ķֿ��������
                bWriteError = true;
            if (bWriteError == true || FILE_SEEK(pInfoFile,sizeof(unsigned short),SEEK_CUR) != 0)//�������Էֿ�����д��
                bWriteError = true;
            unsigned short snTotalBlock = 0;
            pListViewNext = pManageNode->m_pUpdatePartList;
            while (bWriteError == false && pListViewNext != NULL)
            {
                if (bWriteError == true || FILE_WRITE(pListViewNext,sizeof(SUpdatePartNode),1,pInfoFile) != 1)//д��ֿ���Ϣ
                    bWriteError = true;
                pListViewNext = pListViewNext->m_pNext;
                ++snTotalBlock;
            }
            if (bWriteError == true || FILE_SEEK(pInfoFile,sizeof(bool) + sizeof(long long) * 2,SEEK_SET) != 0)//��λ�ֿ���д���λ��
                bWriteError = true;
            if (bWriteError == true || FILE_WRITE(&snTotalBlock,sizeof(unsigned short),1,pInfoFile) != 1)//д��ֿ���
                bWriteError = true;
            fclose(pInfoFile);
            if (bWriteError == true)
                printf("[E]CFileUpDownServ:Info File Write Error In _UpdateNodeManageThread,The File May Be Destroyed\n");
        }
        else
            printf("[E]CFileUpDownServ:Info File Can Not Open In _UpdateNodeManageThread\n");
    }
    else
    {
#ifdef WIN32
        string szCommand = "del ";
#else
        string szCommand = "rm -f ";
#endif
        szCommand += pManageNode->m_szInfoFilePath;
        system(szCommand.c_str());
    }
    //2.3������ֿ�����
    if (TryEnterCS(pInstance->m_pUpdatePartListMemPoolLock,pInstance->m_bIsRelease) == true)
    {
        while (pFreeLink != NULL)
        {
            pListTmp = pFreeLink;
            pFreeLink = pFreeLink->m_pNext;
            RecycleMemory(pInstance->m_pUpdatePartListMemPool,pListTmp);
        }
        pListViewNext = pManageNode->m_pUpdatePartList;
        while (pListViewNext != NULL)
        {
            pListTmp = pListViewNext;
            pListViewNext = pListViewNext->m_pNext;
            RecycleMemory(pInstance->m_pUpdatePartListMemPool,pListTmp);
        }
        ExitCS(pInstance->m_pUpdatePartListMemPoolLock);
    }
    pManageNode->m_pUpdatePartList = NULL;
    //3���ѽڵ�Żص����ö�����
    EnterCS(pInstance->m_pAdjustNodeLock);
    STransNode *pViewNode = &(pInstance->m_pTransNodeArrayList[pInstance->m_nNextUsingIndex]);
    if (pViewNode == pManageNode)
        pInstance->m_nNextUsingIndex = pManageNode->m_nNextIndex;
    else
    {
        while (&(pInstance->m_pTransNodeArrayList[pViewNode->m_nNextIndex]) != pManageNode)//�����ܱ�֤����䲻�������Ч����ֵ
            pViewNode = &(pInstance->m_pTransNodeArrayList[pViewNode->m_nNextIndex]);
        pViewNode->m_nNextIndex = pManageNode->m_nNextIndex;
    }
    pManageNode->m_nNextIndex = pInstance->m_nNextFreeIndex;
    pInstance->m_nNextFreeIndex = nIndex;
    ExitCS(pInstance->m_pAdjustNodeLock);
    return NULL;
}

/*
 * ����̬����˽�С����̺߳������������д���ڵ�ȴ���ʱֵ���̺߳���
 * @param in void *pNull,�����õ��Ĳ���
 * @return void *,�㷵��NULL
 */
void *CFileUpDownServ::_RefreshWaitTimeoutThread(void *pNull)
{
    CFileUpDownServ *pInstance = CFileUpDownServ::GetInstance();
    while (pInstance->m_bIsRelease == false)
    {
        for (short snLoopVar = 0;snLoopVar < pInstance->m_snTransNodeArrayListCap;++snLoopVar)
        {
            if (pInstance->m_pTransNodeArrayList[snLoopVar].m_uTimeLeft > 0 && pInstance->m_pTransNodeArrayList[snLoopVar].m_uCurConnectionCount == 0)
                --pInstance->m_pTransNodeArrayList[snLoopVar].m_uTimeLeft;
            if (pInstance->m_bIsRelease == true)
                break;
        }
        Sleep_Thread(CFUDS_CHECK_GAP_TIME);
    }
    return NULL;
}

/*
 * ����̬����˽�С����̺߳�������������������������ӵ��̺߳���
 * @param in void *pSocket,�߳�������ķ������˱�ʶ
 * @return void *,�㷵��NULL
 */
void *CFileUpDownServ::_SocketManageThread(void *pSocket)
{
    CFileUpDownServ *pInstance = CFileUpDownServ::GetInstance();
    CCommonSocket *pCommonSocket = CCommonSocket::GetInstance();
    if (pCommonSocket == NULL)
        return NULL;
    while (pInstance->m_bIsRelease == false)
    {
        pCommonSocket->ListenSocket(pSocket);
        void *pNewSocket = pCommonSocket->AcceptSocket(pSocket);
        if (pNewSocket != NULL)
        {
            if (CreateThreadManager(_TransThread,true,pNewSocket) == NULL)//�������̹߳���ô�������
                pCommonSocket->CloseSocket(pNewSocket);
        }
        Sleep_Thread(5);
    }
    return NULL;
}

/*
 * ����̬����˽�С����̺߳��������������ӵ��߳�
 * @param in void *pSocket,Ҫ����Ĵ�������
 * @return void *,�㷵��NULL
 */
void *CFileUpDownServ::_TransThread(void *pSocket)
{
    char bRecvBuf[CFUDS_RECV_BUF];//�������ݵĻ�������С
    CCommonSocket *pCommonSocket = CCommonSocket::GetInstance();
    if (pCommonSocket == NULL)
        return NULL;
    CFileUpDownServ *pInstance = CFileUpDownServ::GetInstance();
    Json::Reader oDataRead;
    Json::Value oJObject;//�����ʹ��json�����õĶ���
    Json::FastWriter oJWriter;
    string szReturn;//��Ż��Ϳͻ��˵�����
    STransNode *pTargetNode = NULL;//����ļ�������Ϣ�Ľڵ�
    //0��Ϊ�ܹرտͻ��˶���¼�ͻ������ӱ�ʶ
    void *ppConnection = NULL;//���ڴ�ط�������ڴ���޷���������ʱ�������з��ص����ӵı�ʶ��
    if (TryEnterCS(pInstance->m_pConnectionMemPoolLock,pInstance->m_bIsRelease) == true)
    {
        ppConnection = AllocMemory(pInstance->m_pConnectionMemPool);
        if (ppConnection == NULL)
        {
            pCommonSocket->CloseSocket(pSocket);
            return NULL;
        }
        else
            *(void **)(ppConnection) = pSocket;
        ExitCS(pInstance->m_pConnectionMemPoolLock);
    }
    else
    {
        pCommonSocket->CloseSocket(pSocket);
        return NULL;
    }
    //1���ȴ��ͻ��˷��ʹ����ʶ
    if (pCommonSocket->ReceiveSocket(pSocket,bRecvBuf,CFUDS_RECV_BUF) <= 0)
    {
        if (TryEnterCS(pInstance->m_pConnectionMemPoolLock,pInstance->m_bIsRelease) == true)
        {
            RecycleMemory(pInstance->m_pConnectionMemPool,ppConnection,pCommonSocket);
            ExitCS(pInstance->m_pConnectionMemPoolLock);
        }
        return NULL;
    }
    int nResultCode = CFUD_TRANS_PASS;
    int nIndex = -1;//����ڵ�����
    if ( ! oDataRead.parse(bRecvBuf,oJObject))
        nResultCode = CFUD_TRANS_JSON_ERROR;
    else
    {
        if (oJObject["TID"].isNull() == false) //����Ƿ���ڴ���Ȩ������ֶ�
        {
            int nTransCode = oJObject["TID"].asInt();
            nIndex = nTransCode & 0x0ffff;
            if (nIndex < pInstance->m_snTransNodeArrayListCap && nIndex >= 0)
            {
                if ((nTransCode >> 16) == pInstance->m_pTransNodeArrayList[nIndex].m_snIdentify)
                {
                    pTargetNode = &(pInstance->m_pTransNodeArrayList[nIndex]);
                    //����Ƿ��Ѵﵽ������������
                    if (TryEnterCS(pTargetNode->m_pCurConnectionLock,pInstance->m_bIsRelease) == true)
                    {
                        if (pTargetNode->m_uCurConnectionCount < pTargetNode->m_uMaxConnectionCount)
                            ++pTargetNode->m_uCurConnectionCount;
                        else
                            nResultCode = CFUD_TRANS_CONNECTION_FULL;
                        ExitCS(pTargetNode->m_pCurConnectionLock);
                    }
                    else
                        nResultCode = CFUD_TRANS_OBJECT_RELEASE;
                }
            }
            if (pTargetNode == NULL && nResultCode == CFUD_TRANS_PASS)
                nResultCode = CFUD_TRANS_CODE_UNAVAILABLE;
        }
        else
            nResultCode = CFUD_TRANS_NO_TRANS_CODE;
    }
    oJObject.clear();
    oJObject["Command"] = JSON_COMMAND_ID_RESULT;
    oJObject["ResultCode"] = nResultCode;
    szReturn = oJWriter.write(oJObject);
    pCommonSocket->SendSocket(pSocket,szReturn.c_str(),szReturn.length());//�������ﴫ���Ϊ��Ӣ�ĺ����֣���Щ�ַ������ﶼʹ���˵��ֽڴ洢�������ַ������ȵ���ʵ��Ҫ���͵��ֽ���
    if (nResultCode != CFUD_TRANS_PASS)
    {
        if (TryEnterCS(pInstance->m_pConnectionMemPoolLock,pInstance->m_bIsRelease) == true)
        {
            RecycleMemory(pInstance->m_pConnectionMemPool,ppConnection,pCommonSocket);
            ExitCS(pInstance->m_pConnectionMemPoolLock);
        }
        return NULL;
    }
    //2��ѡ��Ҫ��������䣨���ϴ����������������
    char bFileBuf[CFUDS_READ_WRITE_BUF];//д���ļ������ݵĻ�������С���������������������쳣�Ͽ�ʱ�����ݾͻ�д�뵽�ļ�
    if (pTargetNode->m_bIsUp == true) //�ͻ����ϴ��ļ�
    {
        //2.A.0��ɾ��ǰ���¼�Ŀͻ������ӱ�ʶ
        *(void **)ppConnection = NULL;
        EnterCS(pInstance->m_pConnectionMemPoolLock);
        RecycleMemory(pInstance->m_pConnectionMemPool,ppConnection,NULL);
        ExitCS(pInstance->m_pConnectionMemPoolLock);
        //2.A.1�����������ӱ�־
        EnterCS(pTargetNode->m_pCurConnectionLock);
        while (pTargetNode->m_pNewSocketIdentify != NULL)//�ȴ�����ڵ�����߳���ɲ���
            Sleep_Thread(5);
        pTargetNode->m_pNewSocketIdentify = pSocket;
        ExitCS(pTargetNode->m_pCurConnectionLock);
        int nReceiveLen = 1;//�������ݵĳ���
        while (pInstance->m_bIsRelease == false && nReceiveLen > 0)
        {
            //2.A.2���ȴ��������ϴ��������
            SUpdatePartNode *pTransRangeNode = NULL;
            bool bHasUnAllocRange = true;//��ʶ�Ƿ�����δ���������
            while (pTransRangeNode == NULL && bHasUnAllocRange == true && pInstance->m_bIsRelease == false)
            {
                pTransRangeNode = pTargetNode->m_pUpdatePartList;
                bHasUnAllocRange = false;
                while (pTransRangeNode != NULL)
                {
                    if (pTransRangeNode->m_pSocket != pSocket)
                    {
                        if (pTransRangeNode->m_pSocket == NULL)
                            bHasUnAllocRange = true;
                        pTransRangeNode = pTransRangeNode->m_pNext;
                    }
                    else
                        break;
                }
                Sleep_Thread(5);
            }
            if (pTransRangeNode == NULL && bHasUnAllocRange == false)
                break;
            //2.A.3�������ϴ�������Ϣ�������ݣ����û�п��ഫ��������䣬���ӹرյĹ����ɴ���ڵ�����߳���ɣ�
            if (pTransRangeNode != NULL && pInstance->m_bIsRelease == false && pTransRangeNode->m_llFinished < pTransRangeNode->m_llEnd - pTransRangeNode->m_llStart + 1)
            {
                //2.A.3.1��֪ͨ�ͻ���Ҫ�ϴ������䣨�����ϡ����޶�Ϊȫ������ϵ��
                oJObject.clear();
                oJObject["Command"] = JSON_COMMAND_SET_TRANS_RANGE;
                long long llRange = pTransRangeNode->m_llStart + pTransRangeNode->m_llFinished;
                oJObject["Start_1"] = (int)(llRange & 0x0ffff);//����64λ���дӵ�λ����0-15λ
                oJObject["Start_2"] = (int)((llRange >> 16) & 0x0ffff);//����64λ���дӵ�λ����16-31λ
                oJObject["Start_3"] = (int)((llRange >> 32) & 0x0ffff);//����64λ���дӵ�λ����32-47λ
                oJObject["Start_4"] = (int)((llRange >> 48) & 0x0ffff);//����64λ���дӵ�λ����48-63λ
                llRange = pTransRangeNode->m_llEnd;
                oJObject["End_1"] = (int)(llRange & 0x0ffff);//����64λ���дӵ�λ����0-15λ
                oJObject["End_2"] = (int)((llRange >> 16) & 0x0ffff);//����64λ���дӵ�λ����16-31λ
                oJObject["End_3"] = (int)((llRange >> 32) & 0x0ffff);//����64λ���дӵ�λ����32-47λ
                oJObject["End_4"] = (int)((llRange >> 48) & 0x0ffff);//����64λ���дӵ�λ����48-63λ
                szReturn = oJWriter.write(oJObject);
                pCommonSocket->SendSocket(pSocket,szReturn.c_str(),szReturn.length());//�������ﴫ���Ϊ��Ӣ�ĺ����֣���Щ�ַ������ﶼʹ���˵��ֽڴ洢�������ַ������ȵ���ʵ��Ҫ���͵��ֽ���
                //2.A.3.2���ϴ�
                int nWriteIndex = 0;//�ļ����ݻ�������дָ��
                int nRecvIndex;//�������ݻ������Ķ�ָ��
                bool bFinishOneBlock = false;//��ʶ�Ƿ������һ�����ݿ�Ĵ���
                do
                {
                    //2.A.3.2.1 ����������
                    nReceiveLen = pCommonSocket->ReceiveSocket(pSocket,bRecvBuf,CFUDS_RECV_BUF);
                    nRecvIndex = 0;
                    //2.A.3.2.2�������ݷ���д�ļ�����������д���ļ�
                    do
                    {
                        //2.A.3.2.2.1��ȷ��Ҫ����д�ļ���������������
                        if (nRecvIndex < nReceiveLen)
                        {
                            int nCopyLen = nReceiveLen - nRecvIndex;
                            if (nCopyLen > CFUDS_READ_WRITE_BUF - nWriteIndex)
                                nCopyLen = CFUDS_READ_WRITE_BUF - nWriteIndex;
                            if (nCopyLen + nWriteIndex > pTransRangeNode->m_llEnd - pTransRangeNode->m_llStart + 1 - pTransRangeNode->m_llFinished)//�յ������������ڸ��������������
                            {
                                nCopyLen = pTransRangeNode->m_llEnd - pTransRangeNode->m_llStart + 1 - pTransRangeNode->m_llFinished - nWriteIndex;
                                nReceiveLen = nCopyLen;//�������������
                            }
                            memcpy(bFileBuf + nWriteIndex,bRecvBuf + nRecvIndex,nCopyLen);
                            nRecvIndex += nCopyLen;
                            nWriteIndex += nCopyLen;
                        }
                        //2.A.3.2.2.2��������д���ļ�
                        if (nWriteIndex > 0 && (nReceiveLen <= 0 || nWriteIndex == CFUDS_READ_WRITE_BUF || nWriteIndex + pTransRangeNode->m_llFinished == pTransRangeNode->m_llEnd - pTransRangeNode->m_llStart + 1))
                        {
                            EnterCS(pTargetNode->m_pReadWriteFileLock);
                            if (FILE_SEEK(pTargetNode->m_pTransFile,pTransRangeNode->m_llStart + pTransRangeNode->m_llFinished - 1,SEEK_SET) == 0
                                 && FILE_WRITE(bFileBuf,nWriteIndex,1,pTargetNode->m_pTransFile) == 1)//�����������if��������ǰ�����жϵ��ص��д
                                fflush(pTargetNode->m_pTransFile);
                            else
                                printf("[E]CFileUpDownServ:Write File Error,Match Info File Is %s\n",pTargetNode->m_szInfoFilePath.c_str());
                            if (pTransRangeNode->m_llFinished + nWriteIndex == pTransRangeNode->m_llEnd - pTransRangeNode->m_llStart + 1)
                                bFinishOneBlock = true;
                            pTransRangeNode->m_llFinished += nWriteIndex;
                            nWriteIndex = 0;
                            ExitCS(pTargetNode->m_pReadWriteFileLock);
                        }
                    }while(nRecvIndex < nReceiveLen);
                }while(nReceiveLen > 0 && bFinishOneBlock == false && pInstance->m_bIsRelease == false);
            }
        }
    }
    else //�ͻ��������ļ�
    {
        bool bHasError = false;//������
        while (pInstance->m_bIsRelease == false && bHasError == false)
        {
            //2.B.1��Ҫ��ͻ��˷���Ҫ���ص�����ֵ
            oJObject.clear();
            oJObject["Command"] = JSON_COMMAND_WAIT_TRANS_RANGE;
            szReturn = oJWriter.write(oJObject);
            pCommonSocket->SendSocket(pSocket,szReturn.c_str(),szReturn.length());//�������ﴫ���Ϊ��Ӣ�ĺ����֣���Щ�ַ������ﶼʹ���˵��ֽڴ洢�������ַ������ȵ���ʵ��Ҫ���͵��ֽ���
            //2.B.2���ȴ��ͻ��˵�����������Ϣ
            if (pCommonSocket->ReceiveSocket(pSocket,bRecvBuf,CFUDS_RECV_BUF) <= 0 || oDataRead.parse(bRecvBuf,oJObject) == false)
                bHasError = true;
            else
            {
                //2.B.3����ϴ��䷶Χֵ
                long long llStart,llEnd;
                if (oJObject["Start_1"].isNull() == false && oJObject["Start_2"].isNull() == false &&
                    oJObject["Start_3"].isNull() == false && oJObject["Start_4"].isNull() == false)
                {
                    llStart = oJObject["Start_1"].asInt();
                    llStart += (long long)(oJObject["Start_2"].asInt()) << 16;
                    llStart += (long long)(oJObject["Start_3"].asInt()) << 32;
                    llStart += (long long)(oJObject["Start_4"].asInt()) << 48;
                }
                else
                    bHasError = true;
                if (oJObject["End_1"].isNull() == false && oJObject["End_2"].isNull() == false &&
                    oJObject["End_3"].isNull() == false && oJObject["End_4"].isNull() == false)
                {
                    llEnd = oJObject["End_1"].asInt();
                    llEnd += (long long)(oJObject["End_2"].asInt()) << 16;
                    llEnd += (long long)(oJObject["End_3"].asInt()) << 32;
                    llEnd += (long long)(oJObject["End_4"].asInt()) << 48;
                }
                else
                    bHasError = true;
                //2.B.4����λ�����Ϳͻ������������
                while (pInstance->m_bIsRelease == false && llStart <= llEnd && bHasError == false)
                {
                    int nReadIndex = 0;//�ļ����ݻ������Ķ�ָ��
                    int nReadLen = llEnd - llStart + 1;//�ݴ��ȡ�ļ����������ͷ��ͳɹ����ֽ���
                    if (nReadLen > CFUDS_READ_WRITE_BUF)
                        nReadLen = CFUDS_READ_WRITE_BUF;
                    if (bHasError == false && nReadLen > 0)
                    {
                        EnterCS(pTargetNode->m_pReadWriteFileLock);
                        if (FILE_SEEK(pTargetNode->m_pTransFile,llStart - 1,SEEK_SET) == 0 && 
                            fread(bFileBuf,nReadLen,1,pTargetNode->m_pTransFile) == 1)//�����������if��������ǰ�����жϵ��ص��д
                        {
                            ExitCS(pTargetNode->m_pReadWriteFileLock);
                            llStart += nReadLen;
                            //2.B.4.x�����Ͷ������ļ����ݣ�ֱ�����ͽ���
                            while (pInstance->m_bIsRelease == false && bHasError == false && nReadIndex < nReadLen)
                            {
                                int nSendLen = pCommonSocket->SendSocket(pSocket,bFileBuf + nReadIndex,nReadLen - nReadIndex);//ͨ��nReadLen����ʵ�ʵķ���������
                                if (nSendLen >= 0)
                                    nReadIndex += nSendLen;
                                if (nSendLen < 0)
                                    bHasError = true;
                            }
                        }
                        else
                        {
                            ExitCS(pTargetNode->m_pReadWriteFileLock);
                            bHasError = true;
                            printf("[E]CFileUpDownServ:Read File Error\n");
                        }
                    }
                }
            }
        }
        //2.B.5���ͷŽڵ�
        if (ppConnection != NULL && TryEnterCS(pInstance->m_pConnectionMemPoolLock,pInstance->m_bIsRelease) == true)
        {
            RecycleMemory(pInstance->m_pConnectionMemPool,ppConnection,pCommonSocket);
            ExitCS(pInstance->m_pConnectionMemPoolLock);
        }
    }
    //3����������1
    EnterCS(pTargetNode->m_pCurConnectionLock);
    --pTargetNode->m_uCurConnectionCount;
    ExitCS(pTargetNode->m_pCurConnectionLock);
    return NULL;
}

/*
 * ����̬����˽�С����̺߳��������������ؽڵ���̺߳���
 * @param in void *pIndex,Ҫ����Ľڵ������
 * @return void *,�㷵��NULL
 */
void *CFileUpDownServ::_DownloadNodeManageThread(void *pIndex)
{
    CFileUpDownServ *pInstance = CFileUpDownServ::GetInstance();
    int nIndex = (int)pIndex;
    STransNode *pManageNode = &(pInstance->m_pTransNodeArrayList[nIndex]);
    //�ȴ��ڵ�����ź�
    while (pInstance->m_bIsRelease == false && pManageNode->m_uTimeLeft > 0)
        Sleep_Thread(5);
    pManageNode->m_uMaxConnectionCount = 0;
    while (pManageNode->m_uCurConnectionCount != 0)
        Sleep_Thread(5);
    fclose(pManageNode->m_pTransFile);
    pManageNode->m_pTransFile = NULL;
    //���մ�����Ϣ�ڵ�
    EnterCS(pInstance->m_pAdjustNodeLock);
    STransNode *pViewNode = &(pInstance->m_pTransNodeArrayList[pInstance->m_nNextUsingIndex]);
    if (pViewNode == pManageNode)
        pInstance->m_nNextUsingIndex = pManageNode->m_nNextIndex;
    else
    {
        while (&(pInstance->m_pTransNodeArrayList[pViewNode->m_nNextIndex]) != pManageNode)//�����ܱ�֤����䲻�������Ч����ֵ
            pViewNode = &(pInstance->m_pTransNodeArrayList[pViewNode->m_nNextIndex]);
        pViewNode->m_nNextIndex = pManageNode->m_nNextIndex;
    }
    pManageNode->m_nNextIndex = pInstance->m_nNextFreeIndex;
    pInstance->m_nNextFreeIndex = nIndex;
    ExitCS(pInstance->m_pAdjustNodeLock);
    return NULL;
}
