////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   RtpwareProxy.h
/// @brief ���嶯̬����rtpware dllʱ��ȡ�䵼�������ķ���
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef  RW_RtpwareProxy_H
#define  RW_RtpwareProxy_H

#include "RtpStat.h"
#include <windows.h>

//��־��¼��
#ifdef _DEBUG
#include  <stdio.h>
#define  rwp_log	 printf
#else
#define  rwp_log(...)
#endif // _DEBUG


#define RTPWARE_DLL_FILE_NAME "rtpware.dll"
#define RTPWARE_DLL_RELATIVE_FILE_NAME ".\\filter\\rtpware.dll"     //rtpware dll�����·����һ��λ��filterĿ¼��

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief rtpware dll���������Ĵ���ӿڣ����ڵ�ʵ����
////////////////////////////////////////////////////////////////////////////////////////////////////
class RtpwareProxy
{
public:
    static RtpwareProxy& getInstance();

    /*
        * @brief  ����RtpWare���д���Ự����ͳ����Ϣ���ѽ�����浽��������С�
        *             ע��ú�����dll����������msdx��ͨ����̬����dll��ʽ���øú���������ǩ��������������ǩ����������ı䣬�����Ըı����֪ͨmsdx�޸ġ�
        * @param [in,out] transtat   ����һ��TranStat�ṹָ�������ͳ�ƽ��ֵ�����浽�ñ�����
        * @return ����0��ʾ��ȡ�ɹ���-1���ʾ��ȡʧ��.
        */
    int computeGlobalTranStat(TranStat * transtat);


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief	 ���÷��Ͳ��Ա���ķ�������ַ�Ͷ˿ڣ�UDP��ʽ��
    /// @param	serverIP	The server ip.
    /// @param	port		The port.
    /// @return	����0�����óɹ������ظ������ʾ��������ʧ�ܻ���ִ��ʧ��
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    int setSimluteReportServer(const char * serverIP, const int port);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief	 ����һ���µķ�����
    /// @param	sessionId	�ỰID
    /// @param	remoteIP	���ն�(Ŀ��)����IP������Ϊ��
    /// @param	remotePort	 Ŀ��˿�
    /// @param	sendRate		�������ʣ���λkbps
    /// @param	dscp		     dscpֵ��С��0-63
    /// @param	duration	��������ʱ�䣬��λ��second
    /// @return �����ɹ��򷵻�0�����ظ������ʾ��������ʧ�ܻ���ִ��ʧ��
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    int startNewSimluteSender(int sessionId, const  char* remoteIP, int remotePort, int sendRate, int dscp, int duration);


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief	 ����һ���µĽ�����
    /// @param	sessionId	�ỰID
    /// @param	sourceIP	������Դ������IP�����鲥��ַ
    /// @param	localPort	���ض˿�
    /// @param	duration	 ��������ʱ�䣬��λ��second
    /// @return �����ɹ��򷵻�0�����ظ������ʾ��������ʧ�ܻ���ִ��ʧ��
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    int startNewSimluteReceiver(int sessionId, const char* sourceIP, int localPort, int duration);

private:
    RtpwareProxy();
    ~RtpwareProxy();

    //����rtpware dll
    void loadRtpwareDLL();

    //ж��rtpware dll������������
    void unloadRtpwareDLL();

    //������������ֻ���岻ʵ�֣���ֹ�����࿽�����ž� CClass object = CClass::getInstance()�ĳ���
    RtpwareProxy(const RtpwareProxy&);
    RtpwareProxy& operator=(const RtpwareProxy&);

private:

    /************************************************************************/
    /* ����һЩrtpware dll����������ָ������                                           */
    /************************************************************************/
    typedef int (* Func_computeGlobalTranStat)(TranStat* transtat);

    typedef int  (* Func_setSimluteReportServer)(const char* serverIP, const int port);

    typedef int  (* Func_startNewSimluteSender)(int sessionId, const  char* remoteIP, int remotePort, int sendRate, int dscp, int duration);

    typedef int  (* Func_startNewSimluteReceiver)(int sessionId, const char* sourceIP, int localPort, int duration);

private:
    HINSTANCE _hRtpwareDLL; //���
    Func_computeGlobalTranStat _computeGlobalTranStat;
    Func_setSimluteReportServer _setSimluteReportServer;
    Func_startNewSimluteSender _startNewSimluteSender;
    Func_startNewSimluteReceiver _startNewSimluteReceiver;
};


/*
�����������ƣ���rtpware dll�ĵ�����������һ��
*/
static  char* g_computeGlobalTranStat_name = "computeGlobalTranStat";
static  char* g_setSimluteReportServer_name = "setSimluteReportServer";
static  char* g_startNewSimluteSender_name = "startNewSimluteSender";
static  char* g_startNewSimluteReceiver_name = "startNewSimluteReceiver";

inline RtpwareProxy& RtpwareProxy::getInstance()
{
    static RtpwareProxy rwdllproxy;
    return rwdllproxy;
}


inline RtpwareProxy::RtpwareProxy()
{
    _hRtpwareDLL = NULL;
    _computeGlobalTranStat = NULL;
    _setSimluteReportServer = NULL;
    _startNewSimluteSender = NULL;
    _startNewSimluteReceiver = NULL;

    //����rtpware dll
    loadRtpwareDLL();
}

inline RtpwareProxy::~RtpwareProxy()
{
    unloadRtpwareDLL();
}

inline void RtpwareProxy::loadRtpwareDLL()
{
    HINSTANCE hRtpwareDll = LoadLibraryA("rtpware.dll");            //�ӵ�ǰĿ¼�²��Ҽ���
    if(hRtpwareDll == NULL)
    {
        hRtpwareDll = LoadLibraryA(RTPWARE_DLL_RELATIVE_FILE_NAME); //�������·���µ�
        if(hRtpwareDll == NULL)
        {
            rwp_log("Could not load rtpware.dll, error code: 0x%x \n", GetLastError());
            return;
        }
    }
    //��ȡ������ַ
    _computeGlobalTranStat = (Func_computeGlobalTranStat)GetProcAddress(hRtpwareDll, g_computeGlobalTranStat_name);
    _setSimluteReportServer = (Func_setSimluteReportServer)GetProcAddress(hRtpwareDll, g_setSimluteReportServer_name);
    _startNewSimluteSender = (Func_startNewSimluteSender)GetProcAddress(hRtpwareDll, g_startNewSimluteSender_name);
    _startNewSimluteReceiver = (Func_startNewSimluteReceiver)GetProcAddress(hRtpwareDll, g_startNewSimluteReceiver_name);

    if(!_computeGlobalTranStat && !_setSimluteReportServer && !_startNewSimluteSender && !_startNewSimluteReceiver)
    {
        rwp_log("faild to load rtpware func: %s\n", g_computeGlobalTranStat_name);
        unloadRtpwareDLL();         //������к������Ҳ�����ַ����ж��dll
    }
    else
        rwp_log("sucess to load rtpware dll and export func.\n");
}

inline void RtpwareProxy::unloadRtpwareDLL()
{
    if(_hRtpwareDLL)
    {
        if(FreeLibrary(_hRtpwareDLL) == 0)
            rwp_log("Could not unload rtpware dll.\n");
    }
    _computeGlobalTranStat = NULL;
    _setSimluteReportServer = NULL;
    _startNewSimluteSender = NULL;
    _startNewSimluteReceiver = NULL;
}


inline int RtpwareProxy::computeGlobalTranStat( TranStat* transtat )
{
    if (_computeGlobalTranStat)
        return _computeGlobalTranStat(transtat);
    else	return -1;
}

inline  int RtpwareProxy::setSimluteReportServer( const char* serverIP, const int port )
{
    if (_setSimluteReportServer)
        return _setSimluteReportServer(serverIP, port);
    else
        return -1;
}

inline int RtpwareProxy::startNewSimluteSender( int sessionId, const char* remoteIP, int remotePort, int sendRate, int dscp, int duration )
{
    if (_startNewSimluteSender)
        return _startNewSimluteSender(sessionId, remoteIP, remotePort,  sendRate,  dscp,  duration );
    else
        return -1;
}

inline  int RtpwareProxy::startNewSimluteReceiver( int sessionId, const char* sourceIP, int localPort, int duration )
{
    if (_startNewSimluteReceiver)
        return _startNewSimluteReceiver(sessionId, sourceIP, localPort, duration );
    else
        return -1;
}

#endif  RW_RtpwareProxy_H