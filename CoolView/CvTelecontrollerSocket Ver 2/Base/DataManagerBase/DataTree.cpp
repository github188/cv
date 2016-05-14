/*
 * ����ű��ⲿ���ݣ������XML�ļ��У��� cpp�ļ� By Lzy
 * 2011-12-21   V2.0.1.1    �����������޸��ַ�������ʱ�����������ֵָ��ΪNULL����������⣬�ְѴ���NULL����ָ����Ϊ����ַ����Ĳ���
 * 2011-12-08   V2.0.1.0    1�����ݽڵ�����ָ�����ݽڵ��ָ�룬�����������ϼ����ʺ���
 *                          2�����ӻ�д���ݿ��ƣ��ɿ�����Щ�����ڱ���ʱ��д�뵽xml�ļ��У�
 * 2011-12-04   V2.0.0.0    ����ȵ����洢�ṹ������ʵ�֣�������ɾ�������ͱ������ݵ��ļ��Ĳ���
 * 2011-09-05   V1.0.0.0    �����汾��Ϣ
 */

#include "DataTree.h"
#include <string.h>

#ifdef WIN32
#define snprintf sprintf_s
#endif

using namespace simple_mem_pool;

CDataTree *CDataTree::m_pInstance = NULL;

/*
 * ������������̬����ȡ��ĵ���ָ��
 * @return CDataTree *,������ĵ�ʵ��ָ��
 */
CDataTree *CDataTree::GetInstance()
{
    if (CDataTree::m_pInstance == NULL)
        CDataTree::m_pInstance = new CDataTree();
    return CDataTree::m_pInstance;
}

/*
 * ������������̬����������
 * @return void
 */
void CDataTree::ReleaseInstance()
{
    if (CDataTree::m_pInstance != NULL)
    {
        delete CDataTree::m_pInstance;
        CDataTree::m_pInstance = NULL;
    }
}

/*
 * ������������ͬ���ķ�����ָ������һ����Ч���ݵ���һ�����ݣ����Ѿ�����β��ʱ���������ͣ�����GetType��ã�ֵ�ᱻ����ΪType_None
 * @return void
 */
void CDataTree::SViewNode::MoveNext()
{
    STreeNode *pNext;
    if (this->m_pLink->m_pNext != NULL)
    {
        pNext = this->m_pLink->m_pNext;
        this->m_pLink = pNext;
    }
    else
        pNext = NULL;
    this->_SetData(pNext);
}

/*
 * ������������ͬ���ķ�����ָ������һ����Ч���ݵ���һ�����ݣ����Ѿ�����ͷ��ʱ���������ͣ�����GetType��ã�ֵ�ᱻ����ΪType_None
 * @return void
 */
void CDataTree::SViewNode::MovePre()
{
    STreeNode *pPre;
    if (this->m_pLink->m_pPre != NULL)
    {
        pPre = this->m_pLink->m_pPre;
        this->m_pLink = pPre;
    }
    else
        pPre = NULL;
    this->_SetData(pPre);
}

/*
 * �����������ʵ�ǰ���ݵ��¼��׸����ݣ�ֻ�е�ǰ��������ֵΪType_Tableʱʹ�ô˺�������Ч������¼�û�����ݻ�ǰ�������Ͳ���Type_Table������������ֵ�ᱻ����ΪType_None
 * @return void
 */
void CDataTree::SViewNode::MoveSub()
{
    STreeNode *pSub;
    if (this->m_pLink->m_eDataType == CDataTree::Type_Table && this->m_pLink->m_uData.m_pTable != NULL)
    {
        pSub = (STreeNode *)(this->m_pLink->m_uData.m_pTable);
        this->m_pLink = pSub;
    }
    else
        pSub = NULL;
    this->_SetData(pSub);
}

/*
 * �����������ʵ�ǰ���ݵ��ϼ����ݣ�����ϼ�û�����ݣ�����������ֵ�ᱻ����ΪType_None
 * @return void
 */
void CDataTree::SViewNode::MoveParent()
{
    STreeNode *pParent;
    if (this->m_pLink->m_pParent != NULL && this->m_pLink->m_pParent->m_pParent != NULL)//��һ���ж��Ƿ�ֹ���ʵ������ص�ͷָ��
    {
        pParent = this->m_pLink->m_pParent;
        this->m_pLink = pParent;
    }
    else
        pParent = NULL;
    this->_SetData(pParent);
}

/*
 * ��˽�С����캯������ʼ����������
 */
CDataTree::CDataTree()
{
    this->m_oHeadNode.m_eDataType = CDataTree::Type_Table;
    this->m_oHeadNode.m_uData.m_pTable = NULL;
    this->m_oHeadNode.m_pNext = NULL;
    this->m_oHeadNode.m_pPre = NULL;
    this->m_oHeadNode.m_bCanSave = true;
    this->m_oHeadNode.m_pParent = NULL;
    this->m_oHeadNode.m_szName[0] = '\0';
    this->m_szXMLFilePath[0] = '\0';
    this->m_pDataMen = CreateMemPool(sizeof(STreeNode));
    this->m_pHash = new CStrHashMap(HASH_MAP_SIZE);
}

/*
 * ��˽�С������������ͷŶ���
 */
CDataTree::~CDataTree()
{
    ReleaseMemPool(&this->m_pDataMen);
    delete this->m_pHash;
}

/*
 * ��˽�С�����XML������ͬ���ڵ㣬���ݴ˴�������
 * @param const TiXmlElement *pXMLElement,ͬ��XML�ڵ��е��׽ڵ㣬��������˲���ֵ����Ч��
 * @param unsigned long lPathLoc,��ǰʹ�õ�������·�����ȣ�Ҳ����һ�ַ����������λ��
 * @param char *pszPath,·���ַ���ָ�룬�������ΪNULL�����ɺ�������
 * @param STreeNode *pParent,�ϼ��ڵ�ָ��
 * @return STreeNode *,�����׸��ɹ����������ݽڵ�ָ��
 */
CDataTree::STreeNode *CDataTree::_AnalyzeXMLElement(const TiXmlElement *pXMLElement,unsigned long lPathLoc,char *pszPath,STreeNode *pParent)
{
    const char *pszXMLValue;
    char *pszOrgPath = pszPath;
    STreeNode *pFirstNode = NULL;//���κ����������׸������Ľڵ�
    STreeNode *pLastNode = NULL;//��һ�δ���Ľڵ�
    STreeNode *pPresentNode = NULL;//ָ��ǰ�������ݽڵ�ĵ�ַ
    //1��δ�л�����ʱ����������
    if (pszOrgPath == NULL)
    {
        pszOrgPath = new char[MAX_DATATREE_PROPERTY_PATH_LENGTH];
        pszOrgPath[0] = '\0';
    }
    if (pszOrgPath == NULL)
        return pFirstNode;
    //2����������·��д��λ��
    char *pszPropertyPath = &(pszOrgPath[lPathLoc]);
    //3��XML�ڵ����
    while (pXMLElement != NULL)
    {
        //3.1�������ڴ�
        if (pPresentNode == NULL)
        {
            pPresentNode = (STreeNode *)AllocMemory(this->m_pDataMen);
            if (pPresentNode == NULL)
            {
                if (pszPath == NULL)
                    delete[] pszOrgPath;
                return pFirstNode;
            }
        }
        //3.2������������
        pszXMLValue = pXMLElement->Attribute("name");
        if (pszXMLValue != NULL)
            strncpy(pPresentNode->m_szName,pszXMLValue,MAX_DATATREE_NAME_LENGTH);
        else
            pPresentNode->m_szName[0] = '\0';
        //3.3���������ͷ���
        pszXMLValue = pXMLElement->Attribute("type");
        if (pszXMLValue != NULL)
        {
            if (strcmp(pszXMLValue,"long") == 0)
            {
                pPresentNode->m_eDataType = CDataTree::Type_Long;
                pszXMLValue = pXMLElement->Attribute("value");
                if (pszXMLValue != NULL)
                    pPresentNode->m_uData.m_lData = atol(pszXMLValue);
                else
                    pPresentNode->m_uData.m_lData = 0;
            }
            else if (strcmp(pszXMLValue,"float") == 0)
            {
                pPresentNode->m_eDataType = CDataTree::Type_Float;
                pszXMLValue = pXMLElement->Attribute("value");
                if (pszXMLValue != NULL)
                    pPresentNode->m_uData.m_fData = atof(pszXMLValue);
                else
                    pPresentNode->m_uData.m_fData = 0.0;
            }
            else if (strcmp(pszXMLValue,"string") == 0)
            {
                pPresentNode->m_eDataType = CDataTree::Type_String;
                pszXMLValue = pXMLElement->Attribute("value");
                if (pszXMLValue != NULL)
                    strncpy(pPresentNode->m_uData.m_szData,pszXMLValue,MAX_DATATREE_STRING_VALUE_LENGTH);
                else
                    pPresentNode->m_uData.m_szData[0] = '\0';
            }
            else if (strcmp(pszXMLValue,"table") == 0)
            {
                pPresentNode->m_eDataType = CDataTree::Type_Table;
                unsigned long lBackupPathLoc = lPathLoc;
                char * pszBackupPath = pszPropertyPath;
                //3.3.t.1����䵱ǰ������
                strncpy(pszPropertyPath,pPresentNode->m_szName,MAX_DATATREE_PROPERTY_PATH_LENGTH - lPathLoc);
                unsigned long nNameLen = (unsigned long)strlen(pPresentNode->m_szName);//����ת������������ʱ�޷����� By Lzy
                lPathLoc += nNameLen;
                pszPropertyPath += nNameLen;
                //3.3.t.2�����·����׺
                *pszPropertyPath = DIV_PATH_CHAR;
                ++pszPropertyPath;
                ++lPathLoc;
                pszPropertyPath[lPathLoc] = '\0';
                //3.3.t.3�������²�����
                pPresentNode->m_uData.m_pTable = this->_AnalyzeXMLElement(pXMLElement->FirstChildElement(),lPathLoc,pszOrgPath,pPresentNode);
                //3.3.t.4����ԭ
                lPathLoc = lBackupPathLoc;
                pszPropertyPath = pszBackupPath;
            }
            else
            {
                printf ("Unknow Type \"%s\" In \"%s\"\n",pszXMLValue,pszOrgPath);
                pXMLElement = pXMLElement->NextSiblingElement();
                continue;//����ʶ������ͣ�����
            }
            if (pLastNode != NULL)
                pLastNode->m_pNext = pPresentNode;
            pPresentNode->m_pPre = pLastNode;
            pPresentNode->m_pNext = NULL;
            pPresentNode->m_pParent = pParent;
            pPresentNode->m_bCanSave = true;
            pLastNode = pPresentNode;
            if (pFirstNode == NULL)
                pFirstNode = pPresentNode;
            pPresentNode = NULL;
            strncpy(pszPropertyPath,pLastNode->m_szName,MAX_DATATREE_PROPERTY_PATH_LENGTH - lPathLoc);
            this->m_pHash->Insert(pszOrgPath,pLastNode);
        }
        else
            printf ("UnSet Type In \"%s\"\n",pszOrgPath);
        pXMLElement = pXMLElement->NextSiblingElement();
    }
    if (pszPath == NULL)
        delete[] pszOrgPath;
    return pFirstNode;
}

/*
 * ����������ȡ�����ļ�
 * @param const char *pszXMLFilePath,�����ļ���·��
 * @return bool,�����Ƿ��ȡ�ɹ�
 */
bool CDataTree::LoadFromXML(const char *pszXMLFilePath)
{
    if (pszXMLFilePath == NULL)
        return false;
    if (strcmp(pszXMLFilePath,this->m_szXMLFilePath) != 0)
    {
        //����XML�ļ�
        TiXmlDocument pXMLReader(pszXMLFilePath);
        if (pXMLReader.LoadFile() == false)
            return false;
        TiXmlElement *pXMLRoot = pXMLReader.RootElement();
        if (pXMLRoot == NULL)
            return false;
        strncpy(this->m_szXMLFilePath,pszXMLFilePath,MAX_XML_FILE_PATH);
        //�����ϴμ������ú󴴽�����Դ
        this->m_pHash->Clear();
        RecycleMemory(this->m_pDataMen);//�����ڴ�
        //����XML����������
        this->m_oHeadNode.m_uData.m_pTable = this->_AnalyzeXMLElement(pXMLRoot->FirstChildElement(),0,NULL,&(this->m_oHeadNode));
    }
    return true;
}

/*
 * ���������ѵ�ǰ���Ϊ�ɴ洢�����ݴ��浽XML�ļ��У������������б���ͻ��з����⣩
 * @param const char *pszXMLFilePath = NULL,�洢·����֧�־���·�������·�������ʹ��Ĭ��ֵNULL����д��ԭ������ʱָ����XML�ļ������ԭ��û�����XML�ļ����˲�����û���ã������ض�����false
 * @return bool,����д������д��ɹ�ʱ����true
 */
bool CDataTree::SaveToXML(const char *pszXMLFilePath)
{
    if (pszXMLFilePath == NULL)
    {
        if (this->m_szXMLFilePath[0] != '\0')
            pszXMLFilePath = this->m_szXMLFilePath;
        else
            return false;
    }
    FILE *fHandle = fopen(pszXMLFilePath,"w");
    if (fHandle != NULL)
    {
        char szLineBuffer[MAX_WRITE_XML_LINE_LEN];
        fwrite("<?xml version=\"1.0\" encoding=\"GBK\" standalone=\"yes\" ?>\n<conf>\n",62,1,fHandle);
        STreeNode *pViewNode = (STreeNode *)this->m_oHeadNode.m_uData.m_pTable;
        szLineBuffer[0] = '\t';
        this->_WriteXML(fHandle,pViewNode,szLineBuffer,1);
        fwrite("</conf>\n",8,1,fHandle);
        fclose(fHandle);
        return true;
    }
    return false;
}

/*
 * ����������ȡ����ֵ������
 * @param const char *pszSearchPath,����·�������Ҽ���
 * @param CDataTree::SViewNode &rViewNode,�����������ã��ڸ�·���»�ȡ�ɹ�ʱ������Ϣд���������
 * @return bool,���ػ�ȡ������ɹ���ȡ����true
 */
bool CDataTree::GetDataVisitor(const char *pszSearchPath,CDataTree::SViewNode &rViewNode)
{
    STreeNode *pFindNode = (STreeNode *)this->m_pHash->Find(pszSearchPath);
    if (pFindNode != NULL)
    {
        rViewNode.m_pLink = pFindNode;
        rViewNode._SetData(pFindNode);
        return true;
    }
    else
        return false;
}

/*
 * ����������ȡlong������ֵ
 * @param const char *pszSearchPath,����·�������Ҽ���
 * @param long lDefault = 0,������ʧ��ʱ��ʧ�ܵ����������·���µ��������Ͳ���long�ͣ����ص�ֵ
 * @param bool *bGetResult = NULL,�����Ҫ֪�������Ƿ��ȡ�ɹ��������ﴫ��һ��bool������ַ��������ѻ�ȡ��������ڴ����ַ���ڱ����У���ȡ�ɹ�ʱ����true����������false
 * @return long,��������ֵ�������ȡ����ʧ�ܣ����صĽ���lDefault��ֵ��ֻ�д�����bGetResult�����ſ���ȷ����ǰ��ȡ�������Ƿ�ָ��·���������
 */
long CDataTree::GetDataLong(const char *pszSearchPath,long lDefault,bool *bGetResult)
{
    STreeNode *pFindNode = (STreeNode *)this->m_pHash->Find(pszSearchPath);
    if (pFindNode != NULL && pFindNode->m_eDataType == CDataTree::Type_Long)
    {
        if (bGetResult != NULL)
            *bGetResult = true;
        return pFindNode->m_uData.m_lData;
    }
    else
    {
        if (bGetResult != NULL)
            *bGetResult = false;
        return lDefault;
    }
}

/*
 * ����������ȡ����������ֵ
 * @param const char *pszSearchPath,����·�������Ҽ���
 * @param double fDefault = 0.0,������ʧ��ʱ��ʧ�ܵ����������·���µ��������Ͳ��Ǹ����ͣ����ص�ֵ
 * @param bool *bGetResult = NULL,�����Ҫ֪�������Ƿ��ȡ�ɹ��������ﴫ��һ��bool������ַ��������ѻ�ȡ��������ڴ����ַ���ڱ����У���ȡ�ɹ�ʱ����true����������false
 * @return double,��������ֵ�������ȡ����ʧ�ܣ����صĽ���fDefault��ֵ��ֻ�д�����bGetResult�����ſ���ȷ����ǰ��ȡ�������Ƿ�ָ��·���������
 */
double CDataTree::GetDataFloat(const char *pszSearchPath,double fDefault,bool *bGetResult)
{
    STreeNode *pFindNode = (STreeNode *)this->m_pHash->Find(pszSearchPath);
    if (pFindNode != NULL && pFindNode->m_eDataType == CDataTree::Type_Float)
    {
        if (bGetResult != NULL)
            *bGetResult = true;
        return pFindNode->m_uData.m_fData;
    }
    else
    {
        if (bGetResult != NULL)
            *bGetResult = false;
        return fDefault;
    }
}

/*
 * ����������ȡ�ַ���������ֵ
 * @parma const char *pszSearchPath,����·�������Ҽ���
 * @return const char *,����ָ���ַ�����constָ�룬����ʧ�ܷ���NULL
 */
const char * CDataTree::GetDataStr(const char *pszSearchPath)
{
    STreeNode *pFindNode = (STreeNode *)this->m_pHash->Find(pszSearchPath);
    if (pFindNode != NULL && pFindNode->m_eDataType == CDataTree::Type_String)
        return pFindNode->m_uData.m_szData;
    else
        return NULL;
}

/*
 * ��������ͨ���������޸��ض����ݵ�ֵ
 * @info ������ֻ���޸�����ΪType_Long,Type_Float,Type_String������ֵ
 * @param SViewNode &rViewNode,���������ã����ݷ�������ǰ�󶨵�����λ�ý�������ֵ�޸�
 * @return bool,�����޸��Ƿ�ɹ����ɹ�����true
 */
bool CDataTree::ModifyDataVisitor(SViewNode &rViewNode)
{
    STreeNode *pFindNode = rViewNode.m_pLink;
    if (pFindNode != NULL && pFindNode->m_eDataType == rViewNode.m_eDataType)
    {
        switch (pFindNode->m_eDataType)
        {
        case CDataTree::Type_Long:
            pFindNode->m_uData.m_lData = rViewNode.m_uViewData.m_lData;
            break;
        case CDataTree::Type_Float:
            pFindNode->m_uData.m_fData = rViewNode.m_uViewData.m_fData;
            break;
        case CDataTree::Type_String:
            if ( (&(pFindNode->m_uData.m_szData[0])) > rViewNode.m_uViewData.m_pszData || (&(pFindNode->m_uData.m_szData[0])) + MAX_DATATREE_STRING_VALUE_LENGTH <= rViewNode.m_uViewData.m_pszData)
                strncpy(pFindNode->m_uData.m_szData,rViewNode.m_uViewData.m_pszData,MAX_DATATREE_STRING_VALUE_LENGTH);
            else
                return false;
            break;
        default:
            return false;
        }
        return true;
    }
    else
        return false;
}

/*
 * ���������޸�long�����ݵ�ֵ
 * @info ��Ҫͨ���������Ķ���������ΪType_Table������ΪType_Long��ԭ�����µ����������ݻᱻɾ��
 * @param const char *pszSearchPath,����·�������Ҽ���
 * @param long lNewLong,Ҫ���õ���long��ֵ
 * @param bool bCanChangeType = false,ָ����Ҫ�޸ĵ����������뵱ǰ�������Ͳ�һ��ʱ���Ƿ���������޸ģ�����ı�ԭ�����ݵ����ͣ���Ĭ�ϲ����ǲ�����
 * @return bool,�����޸��Ƿ�ɹ����ɹ�����true
 */
bool CDataTree::ModifyDataLong(const char *pszSearchPath,long lNewLong,bool bCanChangeType)
{
    STreeNode *pFindNode = (STreeNode *)this->m_pHash->Find(pszSearchPath);
    if (pFindNode != NULL && (bCanChangeType == true || pFindNode->m_eDataType == CDataTree::Type_Long))
    {
        if (pFindNode->m_eDataType == CDataTree::Type_Table)
            this->_DeleteData(pszSearchPath,true);
        pFindNode->m_eDataType = CDataTree::Type_Long;
        pFindNode->m_uData.m_lData = lNewLong;
        return true;
    }
    return false;
}

/*
 * ���������޸ĸ��������ݵ�ֵ
 * @info ��Ҫͨ���������Ķ���������ΪType_Table������ΪType_Float��ԭ�����µ����������ݻᱻɾ��
 * @param const char *pszSearchPath,����·�������Ҽ���
 * @param double fNewFloat,Ҫ���õ��¸�����ֵ
 * @param bool bCanChangeType = false,ָ����Ҫ�޸ĵ����������뵱ǰ�������Ͳ�һ��ʱ���Ƿ���������޸ģ�����ı�ԭ�����ݵ����ͣ���Ĭ�ϲ����ǲ�����
 * @return bool,�����޸��Ƿ�ɹ����ɹ�����true
 */
bool CDataTree::ModifyDataFloat(const char *pszSearchPath,double fNewFloat,bool bCanChangeType)
{
    STreeNode *pFindNode = (STreeNode *)this->m_pHash->Find(pszSearchPath);
    if (pFindNode != NULL && (bCanChangeType == true || pFindNode->m_eDataType == CDataTree::Type_Float))
    {
        if (pFindNode->m_eDataType == CDataTree::Type_Table)
            this->_DeleteData(pszSearchPath,true);
        pFindNode->m_eDataType = CDataTree::Type_Float;
        pFindNode->m_uData.m_fData = fNewFloat;
        return true;
    }
    return false;
}

/*
 * ���������޸��ַ��������ݵ�ֵ
 * @info ��Ҫͨ���������Ķ���������ΪType_Table������ΪType_String��ԭ�����µ����������ݻᱻɾ��
 * @param const char *pszSearchPath,����·�������Ҽ���
 * @param const char *pszNewString,Ҫ���õ����ַ���ֵ
 * @param bool bCanChangeType = false,ָ����Ҫ�޸ĵ����������뵱ǰ�������Ͳ�һ��ʱ���Ƿ���������޸ģ�����ı�ԭ�����ݵ����ͣ���Ĭ�ϲ����ǲ�����
 * @return bool,�����޸��Ƿ�ɹ����ɹ�����true
 */
bool CDataTree::ModifyDataStr(const char *pszSearchPath,const char *pszNewString,bool bCanChangeType)
{
    STreeNode *pFindNode = (STreeNode *)this->m_pHash->Find(pszSearchPath);
    if (pFindNode != NULL && (bCanChangeType == true || pFindNode->m_eDataType == CDataTree::Type_String))
    {
        if (pFindNode->m_eDataType == CDataTree::Type_Table)
            this->_DeleteData(pszSearchPath,true);
        pFindNode->m_eDataType = CDataTree::Type_String;
        if (pszNewString != NULL)
            strncpy(pFindNode->m_uData.m_szData,pszNewString,MAX_DATATREE_STRING_VALUE_LENGTH);
        else
            pFindNode->m_uData.m_szData[0] = '\0';
        return true;
    }
    return false;
}

/*
 * ��˽�С�ɾ��ָ��·�������µ��������ݣ���ָ���Ƿ���ָ��·�����Ľڵ㣬���ڳ���·���ظ������������ָö����޷�ֱ�ӷ��ʣ�ɾ�����޸ģ����������ʱֻ����ɾ����������ʱ���ܽ��ö���ɾ��
 * @param const char *pszSearchPath,����·�������Ҽ���
 * @param bool bReserve,ָ��ָ��·���������ݽڵ��Ƿ����������ڽڵ����ͱ����Ҫ������ɾ������Ӧ�ðѴ˲�������Ϊtrue
 * @return void
 */
void CDataTree::_DeleteData(const char *pszSearchPath,bool bReserve)
{
    STreeNode *pFindNode = (STreeNode *)this->m_pHash->Find(pszSearchPath);
    char szSearchPath[MAX_DATATREE_PROPERTY_PATH_LENGTH];
    if (pFindNode != NULL)
    {
        strncpy(szSearchPath,pszSearchPath,MAX_DATATREE_PROPERTY_PATH_LENGTH);
        if (pFindNode->m_eDataType == CDataTree::Type_Table)
            this->_RecycleRL(szSearchPath,strlen(szSearchPath),(STreeNode *)pFindNode->m_uData.m_pTable);
        if (bReserve == false)
        {
            if (pFindNode->m_pPre != NULL)
                pFindNode->m_pPre->m_pNext = pFindNode->m_pNext;
            else if (pFindNode == (STreeNode *)this->m_oHeadNode.m_uData.m_pTable)
                this->m_oHeadNode.m_uData.m_pTable = pFindNode->m_pNext;
            if (pFindNode->m_pNext != NULL)
                pFindNode->m_pNext->m_pPre = pFindNode->m_pPre;
            this->m_pHash->Delete(pszSearchPath);
            RecycleMemory(this->m_pDataMen,pFindNode);
        }
    }
}

/*
 * ��˽�С��ݹ�����뵱ǰ�ڵ�ͬ���Ľڵ㣨������ǰ�ڵ�������������Щ�ڵ�֮�µ��ӽڵ�
 * @param char *pszParentPath,�ϼ��ڵ��·��������ִ��ʱ�п����޸�������
 * @param int nLen,pszParentPath�����ݵĳ���
 * @param CDataTree::STreeNode *pNode,ָ���������յ����ڵ㣬���뱣֤�����ַָ��һ����Ч�ڵ�
 * @return void
 */
void CDataTree::_RecycleRL(char *pszParentPath,int nLen,CDataTree::STreeNode *pNode)
{
    STreeNode *pTmpNode;
    while (pNode->m_pPre != NULL)
        pNode = pNode->m_pPre;
    char *pOrgLoc = pszParentPath + nLen;
    *pOrgLoc = '/';
    ++pOrgLoc;
    *pOrgLoc = '\0';
    while (pNode != NULL)
    {
        strncpy(pOrgLoc,pNode->m_szName,MAX_DATATREE_PROPERTY_PATH_LENGTH - nLen - 1);
        if (pNode->m_eDataType == CDataTree::Type_Table)
            this->_RecycleRL(pszParentPath,nLen + strlen(pNode->m_szName) + 1,(STreeNode *)pNode->m_uData.m_pTable);
        pTmpNode = pNode;
        pNode = pNode->m_pNext;
        this->m_pHash->Delete(pszParentPath);
        RecycleMemory(this->m_pDataMen,pTmpNode);
    }
    --pOrgLoc;
    *pOrgLoc = '\0';
}

/*
 * ������������µ�long������ֵ
 * @param const char *pszNewPath,�����ݵ�·��
 * @param long lValue,��ӵ�����ֵ
 * @param bool bCanSave,�Ƿ�����ýڵ���д���ļ������б��洢���������Ϊ�ɱ��洢��true������ýڵ�������ϲ�ڵ�Ҳ�ᱻͬʱ��Ϊ�ɱ��洢
 * @param bool bAutoCreatePath = false,ָ������·�����ϲ㲻����ʱ�Ƿ��Զ������ϲ����ݽڵ㣬�������Ϊfalse����pszNewPath������·���е��ϲ㲿�֣���·����a/b/c���У�a/b���ݽڵ���Ϊc���ϲ����ݽڵ㣩�������
 * @return bool,������ӽ�����ɹ�ʱ����true
 */
bool CDataTree::AddDataLong(const char *pszNewPath,long lValue,bool bCanSave,bool bAutoCreatePath)
{
    STreeNode *pPresentNode = this->_PreSetPath(pszNewPath,bAutoCreatePath);
    if (pPresentNode != NULL)
    {
        pPresentNode->m_eDataType = CDataTree::Type_Long;
        pPresentNode->m_uData.m_lData = lValue;
        this->_SetSaveFlag(pszNewPath,bCanSave,false);
        return true;
    }
    return false;
}

/*
 * ������������µĸ���������ֵ
 * @param const char *pszNewPath,�����ݵ�·��
 * @param double fValue,��ӵ�����ֵ
 * @param bool bCanSave,�Ƿ�����ýڵ���д���ļ������б��洢���������Ϊ�ɱ��洢��true������ýڵ�������ϲ�ڵ�Ҳ�ᱻͬʱ��Ϊ�ɱ��洢
 * @param bool bAutoCreatePath = false,ָ������·�����ϲ㲻����ʱ�Ƿ��Զ������ϲ����ݽڵ㣬�������Ϊfalse����pszNewPath������·���е��ϲ㲿�֣���·����a/b/c���У�a/b���ݽڵ���Ϊc���ϲ����ݽڵ㣩�������
 * @return bool,������ӽ�����ɹ�ʱ����true
 */
bool CDataTree::AddDataFloat(const char *pszNewPath,double fValue,bool bCanSave,bool bAutoCreatePath)
{
    STreeNode *pPresentNode = this->_PreSetPath(pszNewPath,bAutoCreatePath);
    if (pPresentNode != NULL)
    {
        pPresentNode->m_eDataType = CDataTree::Type_Float;
        pPresentNode->m_uData.m_fData = fValue;
        this->_SetSaveFlag(pszNewPath,bCanSave,false);
        return true;
    }
    return false;
}

/*
 * ������������µ��ַ���������ֵ
 * @param const char *pszNewPath,�����ݵ�·��
 * @param const char *pszValue,��ӵ�����ֵ
 * @param bool bCanSave,�Ƿ�����ýڵ���д���ļ������б��洢���������Ϊ�ɱ��洢��true������ýڵ�������ϲ�ڵ�Ҳ�ᱻͬʱ��Ϊ�ɱ��洢
 * @param bool bAutoCreatePath = false,ָ������·�����ϲ㲻����ʱ�Ƿ��Զ������ϲ����ݽڵ㣬�������Ϊfalse����pszNewPath������·���е��ϲ㲿�֣���·����a/b/c���У�a/b���ݽڵ���Ϊc���ϲ����ݽڵ㣩�������
 * @return bool,������ӽ�����ɹ�ʱ����true
 */
bool CDataTree::AddDataStr(const char *pszNewPath,const char *pszValue,bool bCanSave,bool bAutoCreatePath)
{
    STreeNode *pPresentNode = this->_PreSetPath(pszNewPath,bAutoCreatePath);
    if (pPresentNode != NULL)
    {
        pPresentNode->m_eDataType = CDataTree::Type_String;
        if (pszValue != NULL)
            strncpy(pPresentNode->m_uData.m_szData,pszValue,MAX_DATATREE_STRING_VALUE_LENGTH);
        else
            pPresentNode->m_uData.m_szData[0] = '\0';
        this->_SetSaveFlag(pszNewPath,bCanSave,false);
        return true;
    }
    return false;
}

/*
 * ������������µ������ݽڵ�
 * @param const char *pszNewPath,�����ݵ�·��
 * @param bool bCanSave,�Ƿ�����ýڵ���д���ļ������б��洢���������Ϊ�ɱ��洢��true������ýڵ�������ϲ�ڵ�Ҳ�ᱻͬʱ��Ϊ�ɱ��洢
 * @param bool bAutoCreatePath = false,ָ������·�����ϲ㲻����ʱ�Ƿ��Զ������ϲ����ݽڵ㣬�������Ϊfalse����pszNewPath������·���е��ϲ㲿�֣���·����a/b/c���У�a/b���ݽڵ���Ϊc���ϲ����ݽڵ㣩�������
 * @return bool,������ӽ�����ɹ�ʱ����true
 */
bool CDataTree::AddDataTable(const char *pszNewPath,bool bCanSave,bool bAutoCreatePath)
{
    STreeNode *pPresentNode = this->_PreSetPath(pszNewPath,bAutoCreatePath);
    if (pPresentNode != NULL)
    {
        pPresentNode->m_eDataType = CDataTree::Type_Table;
        pPresentNode->m_uData.m_pTable = NULL;
        this->_SetSaveFlag(pszNewPath,bCanSave,false);
        return true;
    }
    return false;
}

/*
 * ��˽�С�·��Ԥ����ȷ��·���ϸ����Ƿ���ڣ�����ʼ�������ݽڵ�
 * @param const char *pszPath,Ҫ����·���ַ���
 * @param bool bAutoCreatePath,ָ���Ƿ��Զ�������·����ȱ�ٵĲ��֣��������·���ϲ㲿���Ǵ��ڵģ������Ͳ���Type_Table�����޷�����ֻ�ܷ���false
 * @return STreeNode *,���Ԥ����ɹ��������س�����ʼ����������ݽڵ㣨�����ͺ�ֵδ��ʼ���������򷵻�NULL
 */
CDataTree::STreeNode *CDataTree::_PreSetPath(const char *pszPath,bool bAutoCreatePath)
{
    char szPath[MAX_DATATREE_PROPERTY_PATH_LENGTH];
    strncpy(szPath,pszPath,MAX_DATATREE_PROPERTY_PATH_LENGTH);
    char *pszView = szPath - 1;//Ϊ���ѭ����Ҫ�������1��ʵ�ʷ����Ǵ�szPath��ʼ��
    const char *pszLastDiv = NULL;
    STreeNode *pLastFindNode = &(this->m_oHeadNode);
    STreeNode *pPresentFindNode;
    STreeNode *pPresentNode;
    do
    {
        ++pszView;
        if (*pszView == DIV_PATH_CHAR || *pszView == '\0')
        {
            char chBackup = *pszView;
            *pszView = '\0';
            pPresentFindNode = (STreeNode *)this->m_pHash->Find(szPath);
            if (pPresentFindNode != NULL && chBackup != '\0')
                pLastFindNode = pPresentFindNode;
            else
            {
                if (chBackup == '\0' && pPresentFindNode != NULL)
                    return NULL;
                if (bAutoCreatePath == true || chBackup == '\0')
                {
                    if (pLastFindNode->m_eDataType != CDataTree::Type_Table)
                        return NULL;
                    pPresentNode = (STreeNode *)AllocMemory(this->m_pDataMen);
                    if (pPresentNode == NULL)
                        return NULL;
                    pPresentNode->m_eDataType = CDataTree::Type_Table;
                    pPresentNode->m_pNext = (STreeNode *)pLastFindNode->m_uData.m_pTable;
                    if (pLastFindNode->m_uData.m_pTable != NULL)
                        ((STreeNode *)pLastFindNode->m_uData.m_pTable)->m_pPre = pPresentNode;
                    pLastFindNode->m_uData.m_pTable = pPresentNode;
                    pPresentNode->m_pParent = pLastFindNode;
                    pPresentNode->m_pPre = NULL;
                    pPresentNode->m_bCanSave = false;
                    pPresentNode->m_uData.m_pTable = NULL;
                    if (pszLastDiv != NULL)
                        pszLastDiv++;
                    else
                        pszLastDiv = szPath;
                    strncpy(pPresentNode->m_szName,pszLastDiv,MAX_DATATREE_NAME_LENGTH);
                    this->m_pHash->Insert(szPath,pPresentNode);
                    pLastFindNode = pPresentNode;
                }
                else
                    return NULL;
            }
            *pszView = chBackup;
            pszLastDiv = pszView;
        }
    }while (*pszView != '\0');
    return pPresentNode;
}

/*
 * ��˽�С��ݹ�������ݽڵ㲢�ѱ��Ϊ�ɴ洢������д��XML�ļ���
 * @param FILE *fHandle,��Ч���ļ�ָ��
 * @param STreeNode *pNode,�����ڵ��ָ��
 * @param char *pszBuffer,������ָ��
 * @parma int nUsedSize,���������ÿռ�
 * @return void
 */
void CDataTree::_WriteXML(FILE *fHandle,STreeNode *pNode,char *pszBuffer,int nUsedSize)
{
    char *pszBgUseBuf = pszBuffer + nUsedSize;
    int nLeftSize = MAX_WRITE_XML_LINE_LEN - nUsedSize;
    if (nLeftSize <= 0)
        return;
    while (pNode != NULL)
    {
        if (pNode->m_bCanSave == true)
        {
            switch (pNode->m_eDataType)
            {
            case CDataTree::Type_Long:
                if (pNode->m_szName[0] != '\0')
                    snprintf(pszBgUseBuf,nLeftSize,"<property name=\"%s\" type=\"long\" value=\"%ld\"/>\n",pNode->m_szName,pNode->m_uData.m_lData);
                else
                    snprintf(pszBgUseBuf,nLeftSize,"<property type=\"long\" value=\"%ld\"/>\n",pNode->m_uData.m_lData);
                break;
            case CDataTree::Type_Float:
                if (pNode->m_szName[0] != '\0')
                    snprintf(pszBgUseBuf,nLeftSize,"<property name=\"%s\" type=\"float\" value=\"%f\"/>\n",pNode->m_szName,pNode->m_uData.m_fData);
                else
                    snprintf(pszBgUseBuf,nLeftSize,"<property type=\"float\" value=\"%f\"/>\n",pNode->m_uData.m_fData);
                break;
            case CDataTree::Type_String:
                if (pNode->m_szName[0] != '\0')
                    snprintf(pszBgUseBuf,nLeftSize,"<property name=\"%s\" type=\"string\" value=\"%s\"/>\n",pNode->m_szName,pNode->m_uData.m_szData);
                else
                    snprintf(pszBgUseBuf,nLeftSize,"<property type=\"string\" value=\"%s\"/>\n",pNode->m_uData.m_szData);
                break;
            case CDataTree::Type_Table:
                if (pNode->m_szName[0] != '\0')
                    snprintf(pszBgUseBuf,nLeftSize,"<property name=\"%s\" type=\"table\">\n",pNode->m_szName);
                else
                    snprintf(pszBgUseBuf,nLeftSize,"<property type=\"table\">\n");
                fwrite(pszBuffer,strlen(pszBuffer),1,fHandle);
                pszBgUseBuf[0] = '\t';
                this->_WriteXML(fHandle,(STreeNode *)pNode->m_uData.m_pTable,pszBuffer,nUsedSize + 1);
                snprintf(pszBgUseBuf,nLeftSize,"</property>\n");
                fwrite(pszBuffer,strlen(pszBuffer),1,fHandle);
                pNode = pNode->m_pNext;
                continue;
                break;
            }
            fwrite(pszBuffer,strlen(pszBuffer),1,fHandle);
        }
        pNode = pNode->m_pNext;
    }
}

/*
 * ��˽�С����ô洢���
 * @param const char *pszPath,Ҫ�޸ĵĴ洢�ڵ�·��������޸�Ϊ�ɴ洢���ļ���bSavaFlag=true����
 *                            ���·�������нڵ���ᱻ��Ϊ�ɴ洢����������޸�Ϊ���ɴ洢����ֻ��ָ���ڵ�ᱻ�޸ġ�
 *                            �˲�����������ΪNULL�������ʱbSetSub����Ϊtrue�����ʾ�޸��漰���нڵ㣬������NULLû���壬������ֱ�ӷ���true
 * @param bool bSaveFlag,�µĴ洢���
 * @param bool bSetSub,ָ���Ƿ��ָ��·���������ӽڵ�ҲӦ���µĴ洢���
 * @return bool,�������ý�������óɹ�����true
 */
bool CDataTree::_SetSaveFlag(const char *pszPath,bool bSaveFlag,bool bSetSub)
{
    STreeNode *pFindNode = (STreeNode *)this->m_pHash->Find(pszPath);
    if (pFindNode != NULL)
    {
        if (bSaveFlag == false)
        {
            pFindNode->m_bCanSave = false;
            return true;
        }
        else
        {
            STreeNode *pViewNode = pFindNode;
            while (pViewNode != NULL)
            {
                pViewNode->m_bCanSave = true;
                pViewNode = pViewNode->m_pParent;
            }
        }
    }
    else if (pszPath == NULL && bSetSub == true)
        pFindNode = &(this->m_oHeadNode);
    else
        return false;
    if (bSetSub == true && pFindNode->m_eDataType == CDataTree::Type_Table)
        this->_SetSubSaveFlag(pFindNode,bSaveFlag);
    return true;
}

/*
 * ��˽�С��ݹ�����ָ���ڵ���ӽڵ�洢���
 * @param STreeNode *pNode,Ҫ���õĽڵ�ĸ��ڵ㣬���뱣֤�ýڵ�Ϊ��Ч��Type_Table���ͽڵ�
 * @param bool bSaveFlag,Ҫ���õĴ洢���
 * @return void
 */
void CDataTree::_SetSubSaveFlag(STreeNode *pNode,bool bSaveFlag)
{
    STreeNode *pSubNode = (STreeNode *)pNode->m_uData.m_pTable;
    while (pSubNode->m_pPre != NULL)
        pSubNode = pSubNode->m_pPre;
    while (pSubNode != NULL)
    {
        pSubNode->m_bCanSave = bSaveFlag;
        if (pSubNode->m_eDataType == CDataTree::Type_Table)
            this->_SetSubSaveFlag(pSubNode,bSaveFlag);
        pSubNode = pSubNode->m_pNext;
    }
}
