/*
 * ����������ڴ�ع���ģ�飨���̰߳�ȫ�� ͷ�ļ� By Lzy
 * 2011-11-28   V2.0.1.0    1������������ʵ�֣�ʹ�û��ն����ʱ��Ч��Ҳ�ǽ���������
 *                          2�������ڴ������������
 *                          3��ɾ��AllocMemory��RecycleMemory���ڲ���1�ӡ�**���ĳɡ�*��������Ĵ���
 *                          4�������ṹ��������������ʹ����˽�к���������
 * 2011-11-27   V2.0.0.1    �Զ���ĳ�ʼ��/�ͷź����������û��Զ�������Ĵ��룬������������Ҳ���������Ӧ�޸�
 * 2011-11-25   V2.0.0.0    �޸ĺ����ӿڣ�ʹ�����������������ڴ���ڵĶ�����䣬ʹ�õ���������ͷų�Ϊ���ܣ�֧���Զ����ó�ʼ������ By Lzy
 * 2011-08-25   V1.0.0.0    By Lzy
 */

#ifndef SIMPLE_MEM_POOL
#define SIMPLE_MEM_POOL

#include <stdlib.h>

/*
 * ������ָ�붨�塿���ڴ�ض����ʼ�����ͷ�ʱ���õĺ����ĺ���ָ��
 * @param void *pObject,Ҫ��ʼ��/�ͷŵĶ����ָ��
 * @param void *pParam,�û��Զ���������ò����ڵ��ñ�ģ�麯��ʱ��ѡ���Ƿ���
 * @return void
 */
typedef void (* ObjectStructFunc)(void *pObject,void *pParam);

namespace simple_mem_pool
{
    void *CreateMemPool(int nObjectSize,ObjectStructFunc fInitializeFunc = NULL,ObjectStructFunc fReleaseFunc = NULL);//�����ڴ��
    void *AllocMemory(void *pManageP,void *pParam = NULL,int *pGetIndex = NULL);//����ָ����С�ڴ�ռ�
    void RecycleMemory(void *pManageP,void *pRecycleObject = NULL,void *pParam = NULL);//�����ڴ�����ѷ���Ķ���
    void RecycleMemory(void *pManageP,int nRecycleIndex,void *pParam = NULL);//�����ڴ�����ѷ���Ķ���
    void *FindMemory(void *pManageP,int nFindIndex);//��������ֵ�����ڴ����
    void ReleaseMemPool(void **pManageP,void *pParam = NULL);//�ͷ��ڴ��
}
#endif
