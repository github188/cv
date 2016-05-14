/*
 * ��ң�����������ݺ�ȫ�����ݹ����� cpp�ļ�
 * 2011-12-08   V1.0.0.1    By Lzy 1����DataTreeģ�������˴洢���ƶ��Ա�ģ�����Ӳ�������Ӧ��֧�ֵ���
 *                                 2����Ӳ������Ӷ��������ݵļ�飬��ֹ������������
 *                                 3����ԭ��Add��Modify��Get��Delete��Clear�������������ΪSet��Get�����������
 * 2011-12-07   V1.0.0.0    By Lzy
 * 2011-12-05   V0.0.0.0    By Lzy �����ļ�
 */

#include "DataManager.h"
#include <string>

CDataManager *CDataManager::m_pDataManager = NULL;

//Set��Get����������ͷ��ͨ�ô���
#define OPERATION_PRE_CODE(GLoc,OLoc)\
    if (pszDataName == NULL)\
        return false;\
    if (TryEnterCS(this->m_pSection,this->m_bObjectDestruct) == false)\
        return false;\
    if (pszAccountName == NULL)\
        this->m_szPath = GLoc;\
    else\
    {\
        this->m_szPath = pszAccountName;\
        this->m_szPath.append(OLoc);\
    }\
    this->m_szPath.append(pszDataName);\

/*
 * ������������̬����ȡ������ָ��
 * @return CDataManager *,���ص���ָ��
 */
CDataManager *CDataManager::GetInstance()
{
    if (CDataManager::m_pDataManager == NULL)
        CDataManager::m_pDataManager = new CDataManager;
    return CDataManager::m_pDataManager;
}

/*
 * ������������̬���ͷŵ�������
 * @return void
 */
void CDataManager::ReleaseInstance()
{
    if (CDataManager::m_pDataManager != NULL)
    {
        delete CDataManager::m_pDataManager;
        CDataManager::m_pDataManager = NULL;
    }
}

/*
 * ��˽�С��๹�캯��
 */
CDataManager::CDataManager()
{
    this->m_pDataTree = CDataTree::GetInstance();
    this->m_pDataTree->LoadFromXML(LOAD_SAVE_DATA_FILE_PATH);
    this->m_bObjectDestruct = false;
    this->m_pSection = CreateCS();
}

/*
 * ��˽�С�����������
 */
CDataManager::~CDataManager()
{
    this->m_bObjectDestruct = true;
    EnterCS(this->m_pSection);
    this->m_pDataTree->SaveToXML(LOAD_SAVE_DATA_FILE_PATH);
    CDataTree::ReleaseInstance();
    ReleaseCS(this->m_pSection);
}

/*
 * ������������long������
 * @param long lValue,������long������ֵ
 * @param bool bCanSave,ָ�������������Ƿ�����洢��Ӳ����
 * @param const char *pszDataName,�������ݵ�������
 * @param const char *pszAccountName = NULL,���������������ض�ң������ר������ʹ�ø�ң������sip�ʺ������֣�Ĭ�ϵ�NULLֵ��ʾ��������ң�������������
 * @return bool,���ز����Ƿ�ɹ����ɹ�����true
 */
bool CDataManager::SetDataLong(long lValue,bool bCanSave,const char *pszDataName,const char *pszAccountName)
{
    OPERATION_PRE_CODE("Global/L/","_/L/")
    bool bResult;
    this->m_pDataTree->GetDataLong(this->m_szPath.c_str(),0,&bResult);
    if (bResult == true)
    {
        bResult = this->m_pDataTree->ModifyDataLong(this->m_szPath.c_str(),lValue);
        if (bResult == true)
            this->m_pDataTree->SetSaveFlag(this->m_szPath.c_str(),bCanSave,false);
    }
    else
        bResult = this->m_pDataTree->AddDataLong(this->m_szPath.c_str(),lValue,bCanSave,true);
    ExitCS(this->m_pSection);
    return bResult;
}

/*
 * �����������ø���������
 * @param double fValue,�����ĸ���������ֵ
 * @param bool bCanSave,ָ�������������Ƿ�����洢��Ӳ����
 * @param const char *pszDataName,�������ݵ�������
 * @param const char *pszAccountName = NULL,���������������ض�ң������ר������ʹ�ø�ң������sip�ʺ������֣�Ĭ�ϵ�NULLֵ��ʾ��������ң�������������
 * @return bool,���ز����Ƿ�ɹ����ɹ�����true
 */
bool CDataManager::SetDataFloat(double fValue,bool bCanSave,const char *pszDataName,const char *pszAccountName)
{
    OPERATION_PRE_CODE("Global/F/","_/F/")
    bool bResult;
    this->m_pDataTree->GetDataFloat(this->m_szPath.c_str(),0.0,&bResult);
    if (bResult == true)
    {
        bResult = this->m_pDataTree->ModifyDataFloat(this->m_szPath.c_str(),fValue);
        if (bResult == true)
            this->m_pDataTree->SetSaveFlag(this->m_szPath.c_str(),bCanSave,false);
    }
    else
        bResult = this->m_pDataTree->AddDataFloat(this->m_szPath.c_str(),fValue,bCanSave,true);
    ExitCS(this->m_pSection);
    return bResult;
}

/*
 * �������������ַ���������
 * @param const char *pszValue,�������ַ���������ֵ
 * @param bool bCanSave,ָ�������������Ƿ�����洢��Ӳ����
 * @param const char *pszDataName,�������ݵ�������
 * @param const char *pszAccountName = NULL,���������������ض�ң������ר������ʹ�ø�ң������sip�ʺ������֣�Ĭ�ϵ�NULLֵ��ʾ��������ң�������������
 * @return bool,���ز����Ƿ�ɹ����ɹ�����true
 */
bool CDataManager::SetDataString(const char *pszValue,bool bCanSave,const char *pszDataName,const char *pszAccountName)
{
    OPERATION_PRE_CODE("Global/S/","_/S/")
    bool bResult;
    const char *pszResult = this->m_pDataTree->GetDataStr(this->m_szPath.c_str());
    if (pszResult != NULL)
    {
        bResult = this->m_pDataTree->ModifyDataStr(this->m_szPath.c_str(),pszValue);
        if (bResult == true)
            this->m_pDataTree->SetSaveFlag(this->m_szPath.c_str(),bCanSave,false);
    }
    else
        bResult = this->m_pDataTree->AddDataStr(this->m_szPath.c_str(),pszValue,bCanSave,true);
    ExitCS(this->m_pSection);
    return bResult;
}

/*
 * ����������ȡlong������
 * @param const char *pszDataName,Ҫ��ȡ���ݵ�������
 * @param const char *pszAccountName = NULL,Ҫ��ȡ���ݵ������������ض�ң������ר������ʹ�ø�ң������sip�ʺ������֣�Ĭ�ϵ�NULLֵ��ʾ��������ң�������������
 * @param long lDefault = 0,����ȡʧ��ʱ���ص�Ĭ��ֵ
 * @return long,���ػ�ȡ����ֵ����ȡʧ��ʱ������lDefault��ֵ
 */
long CDataManager::GetDataLong(const char *pszDataName,const char *pszAccountName,long lDefault)
{
    OPERATION_PRE_CODE("Global/L/","_/L/")
    long lResult = this->m_pDataTree->GetDataLong(this->m_szPath.c_str(),lDefault);
    ExitCS(this->m_pSection);
    return lResult;
}

/*
 * ����������ȡ����������
 * @param const char *pszDataName,Ҫ��ȡ���ݵ�������
 * @param const char *pszAccountName = NULL,Ҫ��ȡ���ݵ������������ض�ң������ר������ʹ�ø�ң������sip�ʺ������֣�Ĭ�ϵ�NULLֵ��ʾ��������ң�������������
 * @param double fDefault = 0.0,����ȡʧ��ʱ���ص�Ĭ��ֵ
 * @return double,���ػ�ȡ����ֵ����ȡʧ��ʱ������fDefault��ֵ
 */
double CDataManager::GetDataFloat(const char *pszDataName,const char *pszAccountName,double fDefault)
{
    OPERATION_PRE_CODE("Global/F/","_/F/")
    double fResult = this->m_pDataTree->GetDataFloat(this->m_szPath.c_str(),fDefault);
    ExitCS(this->m_pSection);
    return fResult;
}

/*
 * ����������ȡ�ַ���������
 * @param const char *pszDataName,Ҫ��ȡ���ݵ�������
 * @param const char *pszAccountName = NULL,Ҫ��ȡ���ݵ������������ض�ң������ר������ʹ�ø�ң������sip�ʺ������֣�Ĭ�ϵ�NULLֵ��ʾ��������ң�������������
 * @return const char *,���ػ�ȡ�����ַ���ָ�룬��ȡʧ��ʱ������NULL
 */
const char *CDataManager::GetDataString(const char *pszDataName,const char *pszAccountName)
{
    OPERATION_PRE_CODE("Global/S/","_/S/")
    const char *pszResult = this->m_pDataTree->GetDataStr(this->m_szPath.c_str());
    ExitCS(this->m_pSection);
    return pszResult;
}
