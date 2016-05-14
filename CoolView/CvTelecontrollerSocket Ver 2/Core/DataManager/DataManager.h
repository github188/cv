/*
 * ��ң�����������ݺ�ȫ�����ݹ����� ͷ�ļ�
 * 2011-12-08   V1.0.0.1    By Lzy 1����DataTreeģ�������˴洢���ƶ��Ա�ģ�����Ӳ�������Ӧ��֧�ֵ���
 *                                 2����Ӳ������Ӷ��������ݵļ�飬��ֹ������������
 *                                 3����ԭ��Add��Modify��Get��Delete��Clear�������������ΪSet��Get�����������
 * 2011-12-07   V1.0.0.0    By Lzy
 * 2011-12-05   V0.0.0.0    By Lzy �����ļ�
 */

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#define LOAD_SAVE_DATA_FILE_PATH "cfg\\Data.xml" //�������/��д�����ݵ�xml�ļ���λ��

#include "DataTree.h"
#include "CommonThread.h"

using namespace thread_operation;
using namespace std;

class CDataManager
{
public:
    //------����------
    static CDataManager *GetInstance();//��ȡ������ָ��
    static void ReleaseInstance();//�ͷŵ�������

    long GetDataLong(const char *pszDataName,const char *pszAccountName = NULL,long lDefault = 0);//��ȡlong������
    double GetDataFloat(const char *pszDataName,const char *pszAccountName = NULL,double fDefault = 0.0);//��ȡ����������
    const char *GetDataString(const char *pszDataName,const char *pszAccountName = NULL);//��ȡ�ַ���������

    bool SetDataLong(long lValue,bool bCanSave,const char *pszDataName,const char *pszAccountName = NULL);//����long������
    bool SetDataFloat(double fValue,bool bCanSave,const char *pszDataName,const char *pszAccountName = NULL);//���ø���������
    bool SetDataString(const char *pszValue,bool bCanSave,const char *pszDataName,const char *pszAccountName = NULL);//�����ַ���������

private:
    //------����------
    CDataManager();
    ~CDataManager();
    CDataManager(const CDataManager &){};
    const CDataManager *operator =(const CDataManager &){};
    //------����------
    static CDataManager *m_pDataManager;//������
    CDataTree *m_pDataTree;//���CDataTreeʵ��ָ��
    string m_szPath;//ר��Ϊ����·���Ļ������ʹ��
    CRITICAL_SECTION *m_pSection;//�����ٽ���
    bool m_bObjectDestruct;//ָʾ�����Ƿ�Ҫ�����������ڿ����ٽ������˳�
};

#endif
