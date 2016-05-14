/*
 * ��ָ�������ݴ��͸��������뷨���  ͷ�ļ�
 * 2012-05-02   V1.0.1.0    ����CNetManager��ĵ�����������Ӧ���
 * 2012-02-28   V1.0.0.0    ������� By LZY
 * 2012-01-27   V0.0.0.0    �����ļ� By Lzy
 */

#ifndef CONTEXT_INPUT_H
#define CONTEXT_INPUT_H

#define LOOP_CONTEXT_COUNT 15 //ѭ�����еĶ��г���

#include "NetManager.h"

class CContextInput
{
public:
    /*
     * ��������������������̬�����������ʵ��
     * @param in ENUM_IPVersion eIPVer,�������뷨���ӵ�IP�汾
     * @param in unsigned int uPort,�����˿ں�
     * @return CContextInput *,���ش����õĶ���
     */
    static CContextInput *CreateInstance(ENUM_IPVersion eIPVer,unsigned int uPort)
    {
        if (CContextInput::m_pInstanceObject != NULL)
            delete CContextInput::m_pInstanceObject;
        CContextInput::m_pInstanceObject = new CContextInput(eIPVer,uPort);
        return CContextInput::m_pInstanceObject;
    }

    /*
     * ��������������������̬����ȡ�����ʵ��
     * @return CContextInput *,���������ʵ�������ʵ��δ�����򷵻�NULL
     */
    static CContextInput *GetInstance()
    {
        return CContextInput::m_pInstanceObject;
    }

    /*
     * ��������������������̬���ͷ������ʵ��
     * @return void
     */
    static void ReleaseInstance()
    {
        if (CContextInput::m_pInstanceObject != NULL)
            delete CContextInput::m_pInstanceObject;
        CContextInput::m_pInstanceObject = NULL;
    }

    void SendContext(unsigned short usCodePageNum,const char *pszContext,int nContextLen);//����Ҫ��������ݸ����뷨
private:
    //------�ṹ��------
    struct SContextNode
    {
        char m_szContext[256];//���Ҫ���͸����뷨���ַ������Ѱ�������ҳ��Ϣ��
        int m_nContextLen;//m_szContext����Ч�����ܳ�
    };
    //------����------
    static CContextInput *m_pInstanceObject;//��̬����
    /*CRITICAL_SECTION m_oCContextInputSection;//д�����ݵ�ѭ�����е��ٽ���
    SContextNode m_sLoopContext[LOOP_CONTEXT_COUNT];//ѭ������
    int m_nLoopWrite,m_nLoopRead;//ѭ�����е�д��/��ȡָ��*/
    SNodeID m_oTCPListenerID;//�������뷨���ӵ����������ṹ
    //------����------
    CContextInput(ENUM_IPVersion eIPVer,unsigned int uPort);
    ~CContextInput();
    CContextInput(const CContextInput&){;};
};

#endif