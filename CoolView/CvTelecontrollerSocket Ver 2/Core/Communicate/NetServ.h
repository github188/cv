/*
 * ң�������ն�ͨ�ŷ������˹�������������ļ�
 * V1.3.1.0 2012-05-02  1������CNetManager��ĵ�����������Ӧ���
 *                      2�����Ӷ�IPv6���ӵ�֧��
 * V1.3.0.0 2012-02-28  ʹ��NetManagerģ������������ʹ�ñ�ģ�������������������רע��ҵ���߼�
 * V1.2.0.0 2011-12-09  ʹ��CommonThread��SimpleMemPoolģ������Դ�ĸ�������������Ϣ����Ĵ������ By Lzy
 * V1.1.0.0 2011-11-24  �����ڲ�ʵ�� By Lzy
 * V0.0.0.0 2011-02-23  �����ļ� By Lzy
 */
#ifndef NETSERV_H
#define NETSERV_H

#include "NetManager.h"

#define NOTIFY_BUFFER_SIZE 2048 //ͨ����Ϣ��������С
#define NOTIFY_ALL 0x4000000 //AddNotify������ָ��������ͨ�����ӷ�����Ϣ�ı�ʶ��

class CNetServ
{
public:
    //------ö��------
    enum EConnectTypeEnum{Type_Tcp,//ʹ��TCP���ӷ�ʽ
                         Type_Udp,//ʹ��UDP���ӷ�ʽ
                         Type_Both};//ͬʱʹ��TCP��UDP���ӷ�ʽ
    //------����------
    
    /*
     * ��������������������̬������CNetServ��ĵ�ʵ��
     * @param ������˵����CNetServ�Ĺ��캯��
     * @return CNetServ *,���ع���õĵ�ʵ��ָ��
     */
    static CNetServ *CreateInstance(unsigned int uCommandPort,EConnectTypeEnum eCommandConnectType,unsigned int uNotifyPort,unsigned int uNotifyListLen = 50)
    {
        if (CNetServ::m_pNetServInstance != NULL)
            delete CNetServ::m_pNetServInstance;
        CNetServ::m_pNetServInstance = new CNetServ(uCommandPort,eCommandConnectType,uNotifyPort,uNotifyListLen);
        return CNetServ::m_pNetServInstance;
    }

    /*
     * ��������������������̬����ȡCNetServ��ʵ��ָ�룬���ûʹ��CNetServ::CreateInstance����ʵ����ʼ���������ȡ���Ľ�����NULLֵ
     * @return CNetServ *,���ص�ʵ����ָ��ֵ�����ʵ��δ�����򷵻�NULL
     */
    static CNetServ *GetInstance()
    {
        return CNetServ::m_pNetServInstance;
    }

    /*
     * ��������������������̬���ͷ�CNetServ��ʵ��
     * @return void
     */
    static void ReleaseInstance()
    {
        if (CNetServ::m_pNetServInstance != NULL)
            delete CNetServ::m_pNetServInstance;
        CNetServ::m_pNetServInstance = NULL;
    }

    int CommandSendBack(const char *pData,int nLen,int nSockResIndex);//������������Ϣ���ͷ��ص�����
    int AddNotify(const char *pData,int nLen,int nTarget = NOTIFY_ALL);//���Ҫ���͵�ͨ����Ϣ�����Ͷ����Ӻ���У�
private:
    //------�ṹ��------
    struct SNotifyNode//ͨ���ݴ���нڵ�
    {
        char m_pBuffer[NOTIFY_BUFFER_SIZE];
        int m_nUsedSize;//Buffer����Ч���ݳ���
        int m_nTargetIndex;//ͨ����Ϣ���ص�Ŀ��ڵ���������m_pTargetAddrֱ����أ�����-1��ʾȫ�壬���Ŀ������Ӧ�����Ӳ��Ƿ���ͨ����Ϣ�����ӣ����Ŀ��Ҳֱ���Թ�
    };
    //------����-------
    static CNetServ *m_pNetServInstance;//CNetServ��ĵ�ʵ��ָ��
    bool m_bThreadEnd;//�߳̽�����־��֪ͨ�����߳̽�������*/
    //  ------�����������------
    CNetManager *m_pNetManagerInstance;//�������ģ��ʵ��ָ��
    //    ------ң��������������Ϣ�������------
    SNodeID m_oCommandListenerV4TCPID;//����ң����IPv4�µ�TCP��ʽ�����������ӵ������ṹ
    SNodeID m_oCommandListenerV6TCPID;//����ң����IPv6�µ�TCP��ʽ�����������ӵ������ṹ
    SNodeID m_oCommandListenerV4UDPID;//����ң����IPv4�µ�UDP��ʽ�����������ӵ������ṹ
    SNodeID m_oCommandListenerV6UDPID;//����ң����IPv6�µ�UDP��ʽ�����������ӵ������ṹ
    //  ------ң����ͨ����Ϣ�������------
    SNodeID m_oNotifyListenerV4ID;//����ң����������Ϣ���������IPv4���ӵ������ṹ
    SNodeID m_oNotifyListenerV6ID;//����ң����������Ϣ���������IPv6���ӵ������ṹ
    bool m_bNotifySendThreadIsRunning;//���_NotifySendTCPThread�����Ƿ���Ȼ������
    //        ------ͨ����Ϣ�������------
    SNotifyNode *m_pNotifyList;//ͨ���ݴ���У�����ѭ�����У�
    int m_nNotifyListLen;//ͨ���ݴ���пɷ��ʵĳ���
    int m_nNotifyListReadIndex;//ͨ���ݴ����[��]����
    int m_nNotifyListWriteIndex;//ͨ���ݴ����[д]����
    CRITICAL_SECTION *m_pNotifyListLock;//ͨ����Ϣ���й������黥���������׼ȷ��˵����ֻ��д������Ч������������Ҫ������
    //------����------
    CNetServ(unsigned int uCommandPort,EConnectTypeEnum eCommandConnectType,unsigned int uNotifyPort,unsigned int uNotifyListLen);
    ~CNetServ();
    CNetServ(const CNetServ&){};
    bool _IDParse(SNodeID &rClientID,unsigned char nExternAdd,int &rResult);//IDת������
    void _IDUnParse(int nUnParseID,SNodeID &rClientID,unsigned char &rExternAdd);//ID��ת������
    //    ------�ص�����------
    static bool _CommandListenerTCPAcceptFunc(SNodeID oClientID,SNodeID oServID,void *pNetManager);//TCP����ͨ�ż������µ����ӷ���ʱ�Ļص�����
    static void _CommandClientRecvFunc(SNodeID oClientID,SNodeID oServID,ENUM_Protocol eProtocol,const char *pData,int nDataLen,void *pNetManager);//����ͨ���н��յ���Ϣʱ�Ļص�����
    static bool _NotifyListenerTCPAcceptFunc(SNodeID oClientID,SNodeID oServID,void *pNetManager);//TCPͨ��ͨ�ż������µ����ӷ���ʱ�Ļص�����
    static void _NotifyClientDisconnectFunc(SNodeID oClientID,SNodeID oServID,ENUM_Protocol eProtocol,bool bError,void *);//TCPͨ��ͨ�����ӶϿ�ʱ�Ļص�����
    //    ------�̺߳���------
    static void *_NotifySendTCPThread(void *pParam);//������ͨ����Ϣ��ң�������������Ϣ���̺߳���
};
#endif