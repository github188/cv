/*
 * ר���ڴ�����궯������ĺ��������ļ�
 * V1.0.0.1 2011-11-23  ���̺߳�������Ϊ���Ա˽�к�������RunMove������JSon::Value���͵Ĵ����ָ���Ϊ����
 * V0.0.0.0 2011-04-18  �����ļ�
 */

#ifndef MOUSE_MOVE_H
#define MOUSE_MOVE_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include "windows.h"
#include "json.h"

//------��궯������------
#define MOUSE_LEFT_DOWN 1//����������
#define MOUSE_LEFT_UP 2//�������ͷ�
#define MOUSE_RIGHT_DOWN 3//����Ҽ�����
#define MOUSE_RIGHT_UP 4//����Ҽ��ͷ�
#define MOUSE_MOVE 5//����ƶ�
#define MOUSE_NONE 0//����ʾ�κ���궯��������Ϊ��Ч�ڵ㿴��
//----------------------

class CMouseMove
{
public:
    static CMouseMove *GetInstance();//��ȡ�����ʵ��
    static void ReleaseInstance();//�ͷ������ʵ��
    bool RunMove(Json::Value &rJObject);//ִ����궯�����ú������������һ�ε�ִ��δ�����᷵��ʧ��
private:
    static CMouseMove *m_pInstanceObject;//��̬����
    CRITICAL_SECTION m_oCMouseMoveSection;//����CMouseMoveRunning���ٽ���
    bool m_bCMouseMoveRunning;//����ִ�б��
    //------------
    CMouseMove();
    CMouseMove(const CMouseMove&){;};
    static void _CMouseMoveSub(void *pMoveData);//��궯�������̺߳���
};

#endif