/*
 * �ַ�����Ϊ��������ָ����Ϊ����ֵ��������    cpp�ļ� By Lzy
 * 2011-11-26   V1.0.1.1    �ѻ�ȡHashNode�Ĵ������inline����_GetHashNodeByKey�У��Լ��ٴ����ظ�
 * 2011-11-25   V1.0.1.0    ɾ���˶�SimpleMemPool��������ԭ��������ʹ��SimpleMemPoolûʲô���壬���˷��ڴ棩 By Lzy
 * 2011-08-29   V1.0.0.0    By Lzy
 */

#include "StrHashMap.h"

/*
 * �����������캯��
 * @param unsigned long lcapability,Hash��������������ʹ�þ�̬Hash������������ô�С��������Ӱ��
 */
CStrHashMap::CStrHashMap(unsigned long lcapability)
{
    if (lcapability < 10)
        lcapability = 10;
    this->m_lHashSize = lcapability;
    this->m_pHash = (map<string,void *> **)malloc(sizeof(void *) * lcapability);
    if (this->m_pHash != NULL)
        memset(this->m_pHash,0,sizeof(void *) * this->m_lHashSize);
}

/*
 * �������������������ͷ��ڲ���Դ
 */
CStrHashMap::~CStrHashMap()
{
    if (this->m_pHash != NULL)
    {
        for (unsigned int nLoopVar = 0;nLoopVar < this->m_lHashSize;++nLoopVar)
        {
            if (this->m_pHash[nLoopVar] != NULL)
                delete this->m_pHash[nLoopVar];
        }
        free(this->m_pHash);
    }
}

/*
 * �������������µļ�ֵ��
 * @param const char *pszKey,��������ַ�����
 * @param void *pValue,�������ֵ
 * @return bool,���ز����Ƿ�ɹ�
 */
bool CStrHashMap::Insert(const char *pszKey,void *pValue)
{
    if (this->m_pHash != NULL && pszKey != NULL)
    {
        unsigned long lHashNum = this->_HashString(pszKey);
        lHashNum = lHashNum % this->m_lHashSize;
        map<string,void *> *pHashNode = this->m_pHash[lHashNum];
        if (pHashNode == NULL)
        {
            pHashNode = new map<string,void *>;
            this->m_pHash[lHashNum] = pHashNode;
        }
        pHashNode->insert(pair<string,void *>(pszKey,pValue));
        return true;
    }
    return false;
}

/*
 * ���������޸�ָ�����µ�ֵ
 * @param const char *pszKey,Ҫ�޸�ֵ��Ӧ���ַ�����
 * @param void *pNewValue,���õ���ֵ
 * @return bool,�����Ƿ��޸ĳɹ�
 */
bool CStrHashMap::Modify(const char *pszKey,void *pNewValue)
{
    if (this->m_pHash != NULL && pszKey != NULL)
    {
        map<string,void *> *pHashNode = this->_GetHashNodeByKey(pszKey);
        if (pHashNode != NULL)
        {
            map<string,void *>::iterator stMapI = pHashNode->find(pszKey);
            if (stMapI != pHashNode->end())
            {
                stMapI->second = pNewValue;
                return true;
            }
        }
    }
    return false;
}

/*
 * ��������ɾ��ָ���ļ�
 * @param const char *pszKey,Ҫɾ�����ַ�����
 * @return bool,����ɾ���Ƿ�ɹ�
 */
bool CStrHashMap::Delete(const char *pszKey)
{
    if (this->m_pHash != NULL && pszKey != NULL)
    {
        map<string,void *> *pHashNode = this->_GetHashNodeByKey(pszKey);
        if (pHashNode != NULL)
        {
            map<string,void *>::iterator stMapI = pHashNode->find(pszKey);
            if (stMapI != pHashNode->end())
            {
                pHashNode->erase(stMapI);
                return true;
            }
        }
    }
    return false;
}

/*
 * ��������������м�ֵ��Ϣ
 * @return void
 */
void CStrHashMap::Clear()
{
    if (this->m_pHash != NULL)
    {
        for (unsigned int nLoopVar =0;nLoopVar < this->m_lHashSize;++nLoopVar)
        {
            if (this->m_pHash[nLoopVar] != NULL)
                this->m_pHash[nLoopVar]->clear();
        }
    }
}

/*
 * ������������ָ�����¶�Ӧ��ֵ
 * @param const char *pszKey,Ҫ���ҵ��ַ�����
 * @return void *,���ز��ҵ���ֵ������ʧ�ܷ���NULL
 */
void *CStrHashMap::Find(const char *pszKey)
{
    if (this->m_pHash != NULL && pszKey != NULL)
    {
        map<string,void *> *pHashNode = this->_GetHashNodeByKey(pszKey);
        if (pHashNode != NULL)
        {
            map<string,void *>::iterator stMapI = pHashNode->find(pszKey);
            if (stMapI != pHashNode->end())
                return stMapI->second;
        }
    }
    return false;
}

/*
 * ��˽�С����ַ���ת�����ִ���
 * @param const char *pszKey,Ҫת�����ַ���
 * @return unsigned long,����ת�����ֵ ע�⣺��ֵ����Ωһ��
 */
unsigned long CStrHashMap::_HashString(const char *pszStr)
{
    const char *schar = pszStr;
    unsigned long Total = 0;
    while ((*schar) != '\0')
    {
        Total += (*schar);
        schar ++;
    }
    return Total;
}

/*
 * ��˽�С��������������ַ�����ֵ��ȡ��Ӧ��Hash�ڵ�
 * @param const char *pszKey,Ҫƥ����ַ�����ֵ������ֵ���뱣֤��Ч
 * @return map<string,void *> *,���ز��ҵ���Hash�ڵ�
 */
inline map<string,void *> * CStrHashMap::_GetHashNodeByKey(const char *pszKey)
{
    unsigned long lHashNum = this->_HashString(pszKey);
    lHashNum = lHashNum % this->m_lHashSize;
    return this->m_pHash[lHashNum];
}