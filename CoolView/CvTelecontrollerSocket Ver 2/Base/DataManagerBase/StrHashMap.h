/*
 * �ַ�����Ϊ��������ָ����Ϊ����ֵ��������    ͷ�ļ� By Lzy
 * 2011-11-26   V1.0.1.1    �ѻ�ȡHashNode�Ĵ������inline����_GetHashNodeByKey�У��Լ��ٴ����ظ�
 * 2011-11-25   V1.0.1.0    ɾ���˶�SimpleMemPool��������ԭ��������ʹ��SimpleMemPoolûʲô���壬���˷��ڴ棩 By Lzy
 * 2011-08-29   V1.0.0.0    By Lzy
 */

#ifndef STR_HASH_MAP
#define STR_HASH_MAP

#include <map>
#include <string>
#include <stdlib.h>
#include <string.h>

using namespace std;

class CStrHashMap
{
public:
    CStrHashMap(unsigned long lcapability);
    ~CStrHashMap();
    bool Insert(const char *pszKey,void *pValue);//�����µļ�ֵ��
    bool Modify(const char *pszKey,void *pNewValue);//�޸�ָ�����µ�ֵ
    bool Delete(const char *pszKey);//ɾ��ָ���ļ�
    void Clear();//��ռ�ֵ��Ϣ
    void *Find(const char *pszKey);//����ָ�����¶�Ӧ��ֵ
private:
    //------����------
    map<string,void *> **m_pHash;//���Hash��
    unsigned long m_lHashSize;//Hash���С
    //------����------
    unsigned long _HashString(const char *pszStr);//���ַ���ת�����ִ���
    inline map<string,void *> *_GetHashNodeByKey(const char *pszKey);//���ݼ�ֵ��ȡ��Ӧ��Hash�ڵ�
};

#endif
