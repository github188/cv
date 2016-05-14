/*
 * ����ű��ⲿ���ݣ������XML�ļ��У��� ͷ�ļ� By Lzy
 * 2011-12-21   V2.0.1.1    �����������޸��ַ�������ʱ�����������ֵָ��ΪNULL����������⣬�ְѴ���NULL����ָ����Ϊ����ַ����Ĳ���
 * 2011-12-08   V2.0.1.0    1�����ݽڵ�����ָ�����ݽڵ��ָ�룬�����������ϼ����ʺ���
 *                          2�����ӻ�д���ݿ��ƣ��ɿ�����Щ�����ڱ���ʱ��д�뵽xml�ļ��У�
 * 2011-12-04   V2.0.0.0    ����ȵ����洢�ṹ������ʵ�֣�������ɾ�������ͱ������ݵ��ļ��Ĳ���
 * 2011-09-05   V1.0.0.0    �����汾��Ϣ
 */

#ifndef DATATREE_H
#define DATATREE_H

#ifdef WIN32
#pragma warning ( disable : 4996 )
#endif

#include <stdio.h>
#include "tinyxml.h"
#include "StrHashMap.h"
#include "SimpleMemPool.h"

#define DIV_PATH_CHAR '/' //ָ������·������ַ�
#define HASH_MAP_SIZE 1000//���崴����HashMap�Ĵ�С
#define MAX_XML_FILE_PATH 256 //XML���������ļ����ļ�·�����������
#define MAX_DATATREE_STRING_VALUE_LENGTH 256 //�ַ������͵����������������ַ�������
#define MAX_DATATREE_NAME_LENGTH 50 //���������������ַ�������
#define MAX_DATATREE_PROPERTY_PATH_LENGTH 512 //��ȡXMLʱ����·���������󳤶�
#define MAX_WRITE_XML_LINE_LEN 512 //д���ļ�ʱ����XML����󳤶ȣ����������ȣ�

class CDataTree
{
public:
    //------ö������------
    enum EDataType //��������ö��
    {
        Type_Long,//long����
        Type_Float,//��������
        Type_String,//�ַ������ͣ���������ΪMAX_XML_CONFIG_STRING_PROPERTY_VALUE_LENGTH
        Type_Table,//������ͣ����¿ɰ���������������
        Type_None,//��Ч�ڵ㣬��ֵֻ�����ⲿʹ��SViewNode�ṹ�����ͬ������ʱ����֣���ʾ��������
    };
private:
    //------����������------
    union UData //��������ֵ�洢union
    {
       long m_lData;//Type_Long
       double m_fData;//Type_Float
       char m_szData[MAX_DATATREE_STRING_VALUE_LENGTH];//Type_String
       void *m_pTable;//Type_Table��ָ���¼����������һ�����������
    };
    //------�ṹ������------
    struct STreeNode //�������ݴ洢�ڵ�ṹ
    {
        EDataType m_eDataType;//�洢����������
        UData m_uData;//�洢������ֵ
        char m_szName[MAX_DATATREE_NAME_LENGTH];//�洢��ǰ���Ե��������������������Ϊ���ַ���
        bool m_bCanSave;//ָ�����ڵ��Ƿ���ִ��д�뵽�ļ�����ʱ���Ա��洢���ļ�
        STreeNode *m_pParent;//�ϼ��洢�ڵ�
        STreeNode *m_pNext;//ͬ������һ���洢�ڵ�
        STreeNode *m_pPre;//ͬ������һ���洢�ڵ�
    };
public:
    //------����������------
    union UViewData //���ⲿ��������ʱ�õ����ݴ洢union
    {
        long m_lData;
        double m_fData;
        const char *m_pszData;
    };
    //------�ṹ������------
    struct SViewNode
    {
        UViewData m_uViewData;//����ֵ
        const char *m_pzsName;//������
        //------����------

        /*
         * ������������������ȡ��ǰ���ݵ�����
         * @return EDataType,���ص�ǰ���ݵ�����ֵ
         */
        EDataType GetType() {return this->m_eDataType;}
        
        void MoveNext();//����ͬ���ķ�����ָ������һ����Ч���ݵ���һ�����ݣ����Ѿ�����β��ʱ���������ͣ�����GetType��ã�ֵ�ᱻ����ΪType_None
        void MovePre();//����ͬ���ķ�����ָ������һ����Ч���ݵ���һ�����ݣ����Ѿ�����ͷ��ʱ���������ͣ�����GetType��ã�ֵ�ᱻ����ΪType_None
        void MoveSub();//���ʵ�ǰ���ݵ��¼��׸����ݣ�ֻ�е�ǰ��������ֵΪType_Tableʱʹ�ô˺�������Ч������¼�û�����ݻ�ǰ�������Ͳ���Type_Table������������ֵ�ᱻ����ΪType_None
        void MoveParent();//���ʵ�ǰ���ݵ��ϼ����ݣ�����ϼ�û�����ݣ�����������ֵ�ᱻ����ΪType_None

    private:
        //------����------
        EDataType m_eDataType;//��������
        STreeNode *m_pLink;//��ǰ���ݹ��������ڵ�
        //------����------

        /*
         * ��˽�С���������������������
         * @param STreeNode *pTarget,���ڵ�ָ��
         * @return void
         */
        void _SetData(STreeNode *pTarget)
        {
            if (pTarget != NULL)
            {
                this->m_eDataType = pTarget->m_eDataType;
                this->m_pzsName = pTarget->m_szName;
                switch (pTarget->m_eDataType)
                {
                case CDataTree::Type_Long:
                    this->m_uViewData.m_lData = pTarget->m_uData.m_lData;
                    break;
                case CDataTree::Type_Float:
                    this->m_uViewData.m_fData = pTarget->m_uData.m_fData;
                    break;
                case CDataTree::Type_String:
                    this->m_uViewData.m_pszData = pTarget->m_uData.m_szData;
                    break;
                }
            }
            else
                this->m_eDataType = CDataTree::Type_None;
        }

        friend class CDataTree;
    };
    //------����------
    static CDataTree *GetInstance();//��ȡ��ĵ���ָ��
    static void ReleaseInstance();//��������
    bool LoadFromXML(const char *pszXMLFilePath);//��XML�ļ���ȡ����
    bool SaveToXML(const char *pszXMLFilePath = NULL);//������Ϊ�ɴ洢�����ݵ�XML�ļ��У������������б���ͻ��з����⣩

    bool GetDataVisitor(const char *pszSearchPath,SViewNode &rViewNode);//��ȡ����ֵ������
    long GetDataLong(const char *pszSearchPath,long lDefault,bool *bGetResult = NULL);//��ȡlong������ֵ
    double GetDataFloat(const char *pszSearchPath,double fDefault,bool *bGetResult = NULL);//��ȡ����������ֵ
    const char * GetDataStr(const char *pszSearchPath);//��ȡ�ַ���������ֵ

    bool ModifyDataVisitor(SViewNode &rViewNode);//ͨ���������޸��ض����ݵ�ֵ
    bool ModifyDataLong(const char *pszSearchPath,long lNewLong,bool bCanChangeType = false);//�޸�long�����ݵ�ֵ
    bool ModifyDataFloat(const char *pszSearchPath,double fNewFloat,bool bCanChangeType = false);//�޸ĸ��������ݵ�ֵ
    bool ModifyDataStr(const char *pszSearchPath,const char *pszNewString,bool bCanChangeType = false);//�޸��ַ��������ݵ�ֵ

    bool AddDataLong(const char *pszNewPath,long lValue,bool bCanSave,bool bAutoCreatePath = false);//����µ�long������ֵ
    bool AddDataFloat(const char *pszNewPath,double fValue,bool bCanSave,bool bAutoCreatePath = false);//����µĸ���������ֵ
    bool AddDataStr(const char *pszNewPath,const char *pszValue,bool bCanSave,bool bAutoCreatePath = false);//����µ��ַ���������ֵ
    bool AddDataTable(const char *pszNewPath,bool bCanSave,bool bAutoCreatePath = false);//����µ������ݽڵ�
    
    /*
     * ����������������ɾ��ָ��·�������µ���������
     * @param const char *pszSearchPath,Ҫɾ������������·�������Ҽ���
     * @return void
     */
    void DeleteData(const char *pszSearchPath)
    {
        this->_DeleteData(pszSearchPath,false);
    }

    /*
     * �������������������ȫ������
     * @return void
     */
    void ClearAllData()
    {
        this->m_pHash->Clear();
        simple_mem_pool::RecycleMemory(this->m_pDataMen);//�����ڴ�
        this->m_oHeadNode.m_uData.m_pTable = NULL;
    }

    /*
     * �������������������ô洢���
     * @param const char *pszPath,Ҫ�޸ĵĴ洢�ڵ�·��������޸�Ϊ�ɴ洢���ļ���bSavaFlag=true����
     *                            ���·�������нڵ���ᱻ��Ϊ�ɴ洢����������޸�Ϊ���ɴ洢����ֻ��ָ���ڵ�ᱻ�޸ġ�
     *                            �˲�����������ΪNULL�������ʱbSetSub����Ϊtrue�����ʾ�޸��漰���нڵ㣬������NULLû���壬������ֱ�ӷ���true
     * @param bool bSaveFlag,�µĴ洢���
     * @param bool bSetSub,ָ���Ƿ��ָ��·���������ӽڵ�ҲӦ���µĴ洢���
     * @return bool,�������ý�������óɹ�����true
     */
    bool SetSaveFlag(const char *pszPath,bool bSaveFlag,bool bSetSub)
    {
        return this->_SetSaveFlag(pszPath,bSaveFlag,bSetSub);
    }

private:
    //------����------
    ~CDataTree();
    CDataTree();
    CDataTree(const CDataTree&){};
    STreeNode *_AnalyzeXMLElement(const TiXmlElement *pXMLElement,unsigned long lPathLoc,char *pszPath,STreeNode *pParent);//����XML������ͬ���ڵ㣬���ݴ˴�������
    void _DeleteData(const char *pszSearchPath,bool bReserve);//ɾ��ָ��·�������µ��������ݣ���ָ���Ƿ���ָ��·�����Ľڵ�
    void _RecycleRL(char *pszParentPath,int nLen,STreeNode *pNode);//�ݹ�����뵱ǰ�ڵ�ͬ���Ľڵ㣨������ǰ�ڵ�������������Щ�ڵ�֮�µ��ӽڵ�
    STreeNode *_PreSetPath(const char *pszPath,bool bAutoCreatePath);//��������·���Ƿ�������·���ϸ��ڵ���ڣ�����������Զ�����·��������ִ�й����л��Զ����������ݽڵ㣩
    void _WriteXML(FILE *fHandle,STreeNode *pNode,char *pszBuffer,int nUsedSize);//�ݹ�������ݽڵ㲢�ѱ��Ϊ�ɴ洢������д��XML�ļ���
    bool _SetSaveFlag(const char *pszPath,bool bSaveFlag,bool bSetSub);//���ô洢���
    void _SetSubSaveFlag(STreeNode *pNode,bool bSaveFlag);//�ݹ�����ָ���ڵ���ӽڵ�洢���
    
    //------����------
    static CDataTree *m_pInstance;//��ʵ��
    char m_szXMLFilePath[MAX_XML_FILE_PATH];//��ŵ�ǰ�Ѷ����XML�ļ����ļ�����·��
    CStrHashMap *m_pHash;//����������ָ��
    void *m_pDataMen;//���ݴ����ڴ����ָ��
    STreeNode m_oHeadNode;//�׽ڵ�
};

#endif
