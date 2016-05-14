/*
 * ͨ����Ϣ�����������ļ�
 * V1.0.0.0	2011-12-15	�ض���Ϣ�������������������� By Lzy
 * V0.0.0.0 2011-11-23  By Lzy
 */

#ifndef N_MESSAGE_PROCESS_H
#define N_MESSAGE_PROCESS_H

#include <map>
#include <Qtcore/QtCore>

using namespace std;

//��������˳����Ʊ���
#ifdef N_MESSAGE_PROCESS_CPP
bool g_bProgramme_Exit;
#else
extern bool g_bProgramme_Exit;
#endif

class CNMessageProcess
{
public:
    CNMessageProcess();
    void MainProcess(int nInfoIndex,int nNotifyId,const QByteArray &rInputArray);//���յ���ͨ����Ϣ�Ĵ�����

private:
    //------���Ͷ���------
    typedef bool (CNMessageProcess::*FDealProcess)(int nNotifyId,QDataStream &rQStream);//����Ϣʵ�ֺ�����ԭ�Ͷ��壬����ֵΪtrueʱ������_SurProcess�Żᱻִ��
    //------����------
    static map<int,FDealProcess> ms_oFuncP;//��Ϣ����ָ��
    int m_nKey;//�����������������ļ�
    //------����------
    
    /*
     * ��˽�С���������������Ϣ��ֵ
     * @param int nMessageKey,��Ϊ��Ϣ������ֵ
     * @return void
     */
    void _SetMessageKey(int nMessageKey)
    {
        this->m_nKey = nMessageKey;
    }

    CNMessageProcess(const CNMessageProcess&){};//�ո��ƹ��캯������ֹ�ิ��
    void operator=(const CNMessageProcess&){};//�ո���/��ֵ��������ֹ�ิ��
    bool _PreProcess(int nInfoIndex,int nNotifyId,const QByteArray &rInputArray);//��ϢԤ������
    void _SurProcess(int nInfoIndex,int nNotifyId,const QByteArray &rInputArray);//��Ϣ��������

    ////////////////////�����Զ��屾����ר�õĹ��ܺ���////////////////////////////
    
    ////////////////�ض���Ϣ�����������涨��////////////////////////
    bool _ExitProcess(int nNotifyId,QDataStream &rQStream);
    bool _Layout(int nNotifyId,QDataStream &rQStream);
    bool _MemberLocation(int nNotifyId,QDataStream &rQStream);
    bool _MediaState(int nNotifyId,QDataStream &rQStream);
    bool _AddMemberLocation(int nNotifyId,QDataStream &rQStream);
    bool _RemoveMemeberLocation(int nNotifyId,QDataStream &rQStream);
    bool _ChangeMemberLocation(int nNotifyId,QDataStream &rQStream);
    bool _PostMeetingInfo(int nNotifyId,QDataStream &rQStream);
	bool _ScreenShareState(int nNotifyId,QDataStream &rQStream);
	bool _TerminalInfo(int nNotifyId,QDataStream &rQStream);
	bool _PanelState(int nNotifyId,QDataStream &rQStream);
};

#endif