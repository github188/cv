/*
 * ��������������ļ� By Lzy
 * @info ���ಿ�ֺ������̰߳�ȫ���ⲻ����ָͬһʱ��ͬ�����������ÿ��ܳ����⣬Ҳ������Ѻ����в�ͬ�����ĵ��ã�Ҫ����̰߳�ȫ�ĺ�������Ҫ����MUL_THREAD����ⲿ���б�֤
 * 2012-02-27   V1.0.2.5    ����GetSocket��GetSockAddr������������
 * 2012-02-26   V1.0.2.4    ����UDP�õ�SendSocket��ReceiveSocket�������������
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

#ifndef COMMONSOCKET_H
#define COMMONSOCKET_H

//#define MUL_THREAD //���������������ʹCommonSocket�����̰߳�ȫ��

#include <string>
#include <string.h>
#include "SimpleMemPool.h"

#ifdef MUL_THREAD
#include "CommonThread.h"
using namespace thread_operation;
#endif
using namespace std;

#ifndef NULL
#define NULL 0
#endif

#ifdef LINUX
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
typedef int SOCKET;
#else
#ifdef WIN32
#include <Winsock2.h>
#include <ws2tcpip.h>
typedef int ssize_t;
typedef int socklen_t;
#endif
#endif

enum ENUM_IPVersion//IP�汾ö������
{
    Version4,
    Version6
};

enum ENUM_Protocol//Э������ö��
{
    TCP_Protocol,
    UDP_Protocol
};

class CCommonSocket
{
public:
    static CCommonSocket *GetInstance();//�����µ�ʵ���������ʵ���Ѿ����ڣ���ֱ�ӷ��ظ�ʵ��
    static void ReleaseInstance();//�ͷŴ�����ʵ������
    void *CreateSocket(ENUM_IPVersion IP_Version,ENUM_Protocol Protocol);//�����µ�Socket
    int BindSocket(void *SocketIdentify,const char *IP,int Port);//ΪSocket��IP��ַ�Ͷ˿ں�
    int ListenSocket(void *SocketIdentify);//��socket�ļ���
    void *AcceptSocket(void *SocketIdentify);//ת��socket
    int ConnectSocket(void *SocketIdentify,const char *IP,int Port);//������������
    int CloseSocket(void *SocketIdentify);//�ر�ָ����SOCKET
    ssize_t SendSocket(void *SocketIdentify,const char *Data,int Len);//���ѽ��������ӵ�Socket��������
    ssize_t SendSocket(void *SocketIdentify, const char *Data, int Len,sockaddr *To,socklen_t ToLen);//��ָ�����շ���������
    ssize_t ReceiveSocket(void *SocketIdentify,char *RecvData,int MaxLen);//���ѽ��������ӵ�Socket����������
    ssize_t ReceiveSocket(void *SocketIdentify, char *RecvData, int MaxLen,sockaddr *From,socklen_t *FormLen);//�������ݲ���÷��ͷ���Ϣ
    socklen_t GetSockAddr(ENUM_IPVersion IP_Version,const char *IP,int Port,sockaddr *TargetSockAddr);//���sockaddr����
    SOCKET GetSocket(void *SocketIdentify);//�ӹ���ṹ�л�ȡSOCKET����
private:
    struct SocketNode//���Socket�ṹ�Ľڵ�
    {
        SOCKET Socket;
        ENUM_IPVersion IP_Version;
        void SetUsing(bool State) {this->Using = State;}
        bool GetUsing() {return this->Using;}
    private:
        bool Using;
    };
    //------------------
    CCommonSocket();
    ~CCommonSocket();
    CCommonSocket(const CCommonSocket&){};
    static void ForMemPoolInitialSock(void *pObject,void *pNULL);//Ϊ�̳߳س�ʼ��������
    static void ForMemPoolReleaseSock(void *pObject,void *pNULL);//Ϊ�̳߳��ͷŶ�����
    inline SocketNode *_GetSocketNode();//��ȡ����SocketNode�ڵ�
#ifdef WIN32
    inline void _NumCStr(int Num,char *Buf);//10������ֵת���ַ����ĺ��������ڶ˿ں�ת������Win32�汾��
#else
    void _NumCStr(int Num,char *Buf);//10������ֵת���ַ����ĺ��������ڶ˿ں�ת����������ϵͳ�汾��
#endif
    
    /*
     * ��˽�С�����̬�������������ر������ú���
     * @param SOCKET Sock,Ҫ�رյ�SOCKET��ʶ
     * @return int,�������
     */
    static int _CloseSock(SOCKET Sock)
    {
#ifdef LINUX
        return shutdown(Node->Socket,SHUT_RDWR);
#endif
#ifdef WIN32
        return closesocket(Sock);
#endif
    }
    //------------------
    static CCommonSocket *SocketClassP;//��Ŵ�����ʵ��
    bool InitSuccess;//ʵ����ʼ�����
    void *MemPoolNode;//�ڴ�ع���ָ��
#ifdef MUL_THREAD
    CRITICAL_SECTION *OpLock;//���߳��µĲ�����
    bool DestructObject;//������������֪ͨ����ִ���еĺ����˳��ı���
#endif
};

#endif
