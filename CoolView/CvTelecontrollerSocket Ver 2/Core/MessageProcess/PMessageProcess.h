/*
 * ң��������������Ϣ�����������ļ�
 * V1.0.0.0 2011-12-15    �ض���Ϣ�������������������� By Lzy
 * V0.0.0.0 2011-11-23    By Lzy
 */

#ifndef P_MESSAGE_PROCESS_H
#define P_MESSAGE_PROCESS_H

#include "json.h"
#include <map>
#include <string>

using namespace std;

class CNetServ;

class CPMessageProcess
{
public:
    CPMessageProcess(CNetServ *pNetServ,int nSockResIndex);
    void MainProcess(const char *pData,int nLen);//ң����������Ϣ������

private:
    //------���Ͷ���------
    typedef bool (CPMessageProcess::*FDealProcess)(Json::Value &rJObject);//����Ϣʵ�ֺ�����ԭ�Ͷ��壬����ֵΪtrueʱ������_SurProcess�Żᱻִ��
    //------����------
    CNetServ *m_pNetServ;//CNetServ��ʵ��ָ�룬�������������ʱ��Ҫʹ��
    int m_nSockResIndex;//��ǰ��Ϣ���󷽵�������Ϣ�������������������ʱ��Ҫ�õ�
    static map<string,FDealProcess> ms_oFuncP;//��Ϣ����ָ��
    string m_sKey;//�����������������ļ�
    //------����------
    
    /*
     * ��˽�С���������������Ϣ��ֵ
     * @param const char *pszMessageKey,��Ϊ��Ϣ�����ַ���
     * @return void
     */
    void _SetMessageKey(const char *pszMessageKey)
    {
        if (pszMessageKey != NULL)
            this->m_sKey = pszMessageKey;
        else
            this->m_sKey.clear();
    }

    CPMessageProcess(const CPMessageProcess&){};//�ո��ƹ��캯������ֹ�ิ��
    void operator=(const CPMessageProcess&){};//�ո���/��ֵ��������ֹ�ิ��
    bool _PreProcess(Json::Value &rJObject);//��ϢԤ������
    void _SurProcess(Json::Value &rJObject);//��Ϣ��������
    inline int _SendAsNotify(const char *pData,int nLen,int nTarget);//��ָ��Ŀ�귢��ͨ����Ϣ
    inline int _SendAsCommandBack(const char *pData,int nLen);//��ǰָ��������ӷ��ͷ�����Ϣ

    ////////////////////�����Զ��屾����ר�õĹ��ܺ���////////////////////////////
    
    /*
     * ��˽�С�����������Json����ָ������ת�������ַ��أ�ת���Ķ���ֻ����int���ͻ��ַ�������
     * @param Json::Value &rJObject,JSon��������
     * @param const char *pszName,������
     * @return int,����ת�����ֵ
     */
    int JToInt(Json::Value &rJObject,const char *pszName)
    {
        if (rJObject[pszName].isInt())
            return rJObject[pszName].asInt();
        else
            return atoi(rJObject[pszName].asCString());
    }

    ////////////////�ض���Ϣ�����������涨��////////////////////////
    bool QueryOnline(Json::Value &rJObject);
    bool RecvMedia(Json::Value &rJObject);
    bool StopMedia(Json::Value &rJObject);
    bool SetVolumnIndex(Json::Value &rJObject);
    bool ChangeMediaLoc(Json::Value &rJObject);
    bool ChangeLayout(Json::Value &rJObject);
    bool ShowRTCP(Json::Value &rJObject);
    bool Shutdown(Json::Value &rJObject);
    bool Restart(Json::Value &rJObject);
    bool StopMyAudio(Json::Value &rJObject);
    bool StopMyVideo(Json::Value &rJObject);
    bool MouseMove(Json::Value &rJObject);
    bool ReceiveAudio(Json::Value &rJObject);
    bool EnterMeeting(Json::Value &rJObject);
    bool QuitMeeting(Json::Value &rJObject);
    bool ControllerQuit(Json::Value &rJObject);
    bool OpenScreenShare(Json::Value &rJObject);
    bool CloseScreenShare(Json::Value &rJObject);
	bool PPTControlCommand(Json::Value &rJObject);
    bool ApplyAccess(Json::Value &rJObject);
    bool ApplyResult(Json::Value &rJObject);
    bool SetNotifyIndex(Json::Value &rJObject);
    bool KeyInput(Json::Value &rJObject);
    bool ContextInput(Json::Value &rJObject);
    bool UpdateFile(Json::Value &rJObject);
	bool GetTerminalConfig(Json::Value &rJObject);
	bool SetPanelVisible(Json::Value &rJObject);
	bool SegmentVideo(Json::Value &rJObject);
};

#endif