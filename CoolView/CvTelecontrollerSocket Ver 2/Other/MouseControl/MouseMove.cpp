/*
 * ר���ڴ�����궯������ĺ��������ļ�
 * V1.0.0.1 2011-11-23  ���̺߳�������Ϊ���Ա˽�к�������RunMove������JSon::Value���͵Ĵ����ָ���Ϊ����
 * V0.0.0.0 2011-04-18 �����ļ�
 */

#include "MouseMove.h"
#include <process.h>
#include <vector>
using namespace std;

CMouseMove *CMouseMove::m_pInstanceObject = NULL;

struct SMouseMoveNode//���������Ϣ�Ľṹ��
{
    int m_nPoint_X;
    int m_nPoint_Y;
    char m_chMethod;
};

/*
 * ��˽�С����̺߳�������궯��ִ�к�������������һ���������߳���ִ��
 * @param void *pMoveData,�����Ϊvector<SMouseMoveNode>��ָ�룬���ں����ڱ�ת����ͬʱ������ɺ��ͷ�
 * @return void
 */
void CMouseMove::_CMouseMoveSub(void *pMoveData)
{
    vector<SMouseMoveNode> *pSMouseMoveArray = (vector<SMouseMoveNode> *)pMoveData;
    int nArraySize = pSMouseMoveArray->size();
    SMouseMoveNode oLastNode,*pPreNode;
    oLastNode.m_chMethod = MOUSE_NONE;
    POINT oCurrentPoint;//��ȡ��굱ǰλ��ֵ��
    int nScale = 2;//�㱶��
    for (int nLoopVar = 0;nLoopVar < nArraySize;++nLoopVar)
    {
        pPreNode = &(pSMouseMoveArray->at(nLoopVar));
        if (oLastNode.m_chMethod != MOUSE_NONE)
        {
            GetCursorPos(&oCurrentPoint);
            SetCursorPos(oCurrentPoint.x + nScale * (pPreNode->m_nPoint_X - oLastNode.m_nPoint_X),oCurrentPoint.y + nScale * (pPreNode->m_nPoint_Y - oLastNode.m_nPoint_Y));
        }
        if (pPreNode->m_chMethod == MOUSE_LEFT_DOWN)
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        else if (pPreNode->m_chMethod == MOUSE_LEFT_UP)
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
        else if (pPreNode->m_chMethod == MOUSE_RIGHT_DOWN)
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
        else if (pPreNode->m_chMethod == MOUSE_RIGHT_UP)
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
        oLastNode.m_chMethod = pPreNode->m_chMethod;
        oLastNode.m_nPoint_X = pPreNode->m_nPoint_X;
        oLastNode.m_nPoint_Y = pPreNode->m_nPoint_Y;
        Sleep(1);
    }
    delete pSMouseMoveArray;
    CMouseMove::GetInstance()->m_bCMouseMoveRunning = false;
}

/*
 * ������������̬����ȡ�����ʵ��
 * @return CMouseMove *,����ʵ��ָ��
 */
CMouseMove *CMouseMove::GetInstance()
{
    if (CMouseMove::m_pInstanceObject == NULL)
        CMouseMove::m_pInstanceObject = new CMouseMove();
    return CMouseMove::m_pInstanceObject;
}

/*
 * ������������̬���ͷ������ʵ��
 * @return void
 */
void CMouseMove::ReleaseInstance()
{
    if (CMouseMove::m_pInstanceObject != NULL)
    {
        EnterCriticalSection(&(CMouseMove::m_pInstanceObject->m_oCMouseMoveSection));
        while (CMouseMove::m_pInstanceObject->m_bCMouseMoveRunning == true)
            Sleep(5);
        CMouseMove::m_pInstanceObject->m_bCMouseMoveRunning = true;
        DeleteCriticalSection(&(CMouseMove::m_pInstanceObject->m_oCMouseMoveSection));
        delete CMouseMove::m_pInstanceObject;
        CMouseMove::m_pInstanceObject = NULL;
    }
}

/*
 * ��������ִ����궯�����ú������������һ�ε�ִ��δ�����᷵��ʧ��
 * @param Json::Value &rJObject,��Ŷ�����Json��
 * @return bool,���ض����Ƿ�ִ��
 */
bool CMouseMove::RunMove(Json::Value &rJObject)
{
    if (this->m_bCMouseMoveRunning != true)
    {
        if (TryEnterCriticalSection(&(this->m_oCMouseMoveSection)) != false)
        {
            Json::Value oDataArray;
            oDataArray = rJObject["MMove"];
            if (oDataArray.isArray() == true)
            {
                int nArraySize = oDataArray.size();
                vector<SMouseMoveNode> *pSMouseMoveArray = new (vector<SMouseMoveNode>);
                SMouseMoveNode oSMouseMoveNode;
                for (int nLoopVar = 0;nLoopVar < nArraySize;++nLoopVar)
                {
                    oSMouseMoveNode.m_nPoint_X = oDataArray[nLoopVar]["X"].asInt();
                    oSMouseMoveNode.m_nPoint_Y = oDataArray[nLoopVar]["Y"].asInt();
                    oSMouseMoveNode.m_chMethod = oDataArray[nLoopVar]["M"].asInt();
                    pSMouseMoveArray->push_back(oSMouseMoveNode);
                }
                this->m_bCMouseMoveRunning = true;
                _beginthread(_CMouseMoveSub,0,pSMouseMoveArray);//�����߳�
            }
            LeaveCriticalSection(&(this->m_oCMouseMoveSection));
            return true;
        }
    }
    return false;
}

/*
 * ��˽�С��๹�캯��������ڲ�������ʼ��
 */
CMouseMove::CMouseMove()
{
    this->m_bCMouseMoveRunning = false;
    InitializeCriticalSection(&(this->m_oCMouseMoveSection));
}
