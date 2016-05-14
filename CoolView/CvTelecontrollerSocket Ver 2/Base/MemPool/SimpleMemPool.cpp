/*
 * ����������ڴ�ع���ģ�飨���̰߳�ȫ�� cpp�ļ� By Lzy
 * 2011-11-28   V2.0.1.0    1������������ʵ�֣�ʹ�û��ն����ʱ��Ч��Ҳ�ǽ���������
 *                          2�������ڴ������������
 *                          3��ɾ��AllocMemory��RecycleMemory���ڲ���1�ӡ�**���ĳɡ�*��������Ĵ���
 *                          4�������ṹ��������������ʹ����˽�к���������
 * 2011-11-27   V2.0.0.1    �Զ���ĳ�ʼ��/�ͷź����������û��Զ�������Ĵ��룬������������Ҳ���������Ӧ�޸�
 * 2011-11-25   V2.0.0.0    �޸ĺ����ӿڣ�ʹ�����������������ڴ���ڵĶ�����䣬ʹ�õ���������ͷų�Ϊ���ܣ�֧���Զ����ó�ʼ������ By Lzy
 * 2011-08-25   V1.0.0.0    By Lzy
 */

#include "SimpleMemPool.h"

#define MALLOC_SIZE 50 //����ÿ�δ������ڴ���ܴ�Ŷ��ٸ��ض����ȵĶ����鲻Ҫ����1000

#define NODE_END -1 //���ɸĶ�ֵ����ʾָ���Ŀ��Ϊ��
#define NODE_UNDEFINE -2 //���ɸĶ�ֵ����ʾ����δ����

namespace simple_mem_pool
{

    struct S_MemBlockNode;

    struct S_MemoryManager //�ڴ�������ڵ�
    {
        S_MemBlockNode *m_pHead;//�ڴ�����ṹ�����ͷ�ṹָ��
        S_MemBlockNode *m_pPre;//���һ�γɹ������ڴ�ʱָ����ڴ��ڵ�
        int m_nObjectSize;//�ڴ���ŵĶ���Ĺ̶���С
        int m_nBlockNodeCount;//�����S_MemBlockNode�����������ڸ���ʵ��������λ�����Ĺ���
        int m_nPreBlockNodeIndex;//��ǰm_pPreָ����ڴ�������ֵ�����ڸ���ʵ��������λ�����Ĺ���
        ObjectStructFunc m_pInitializeFunc;//��ʼ������ʱҪ���õĺ�����ָ��
        ObjectStructFunc m_pReleaseFunc;//�ͷŶ���ʱҪ���õĺ�����ָ��
    };

    struct S_MemBlockNode //�ڴ�����ڵ�
    {
        void *m_pBlock;//�����ڴ��ָ��
        void *m_pTail;//�ڴ��β���ַ
        struct _ObjectManagerNode
        {
            int m_nNextAvailable;
            int m_nNextUsing;
            int m_nPreUsing;
        }m_nControlLink[MALLOC_SIZE];//���������
        int m_nNextFreeIndex;//��һ���ɷ�������ֵ
        int m_nFirstUsingIndex;//�׸�ʹ���е�����ֵ
        S_MemBlockNode *m_pNextNode;//��һ������ڵ��ַ
    };

    /*
     * �������ļ��ڴ���ʹ�á��ڴ�����
     * @param S_MemoryManager *pManageNode,��Ч���ڴ����ָ�룬�����������
     * @param S_MemBlockNode *pNode,Ҫ���յ��ڴ���ָ��
     * @param bool bWithLinkAdjust,�Ƿ�ͬʱ����ʹ�ü�¼��
     * @param void *pParam,�û��Զ������������������ͷź���������Ϊ�ú����ĵڶ�����������
     * @return void
     */
    void _BlockRecycle(S_MemoryManager *pManageNode,S_MemBlockNode *pBlockNode,bool bWithLinkAdjust,void *pParam)
    {
        ObjectStructFunc pReleaseFunc = pManageNode->m_pReleaseFunc;
        if (pReleaseFunc != NULL || bWithLinkAdjust)
        {
            int nViewIndex = pBlockNode->m_nFirstUsingIndex;
            while (nViewIndex >= 0)
            {
                if (pReleaseFunc != NULL)
                    pReleaseFunc(&(((char *)(pBlockNode->m_pBlock))[pManageNode->m_nObjectSize * nViewIndex]),pParam);
                if (bWithLinkAdjust)
                {
                    S_MemBlockNode::_ObjectManagerNode * pTmpControlNode = &(pBlockNode->m_nControlLink[nViewIndex]);
                    pTmpControlNode->m_nNextAvailable = pBlockNode->m_nNextFreeIndex;;
                    pBlockNode->m_nNextFreeIndex = nViewIndex;
                    nViewIndex = pTmpControlNode->m_nNextUsing;
                    pTmpControlNode->m_nNextUsing = NODE_UNDEFINE;
                    pTmpControlNode->m_nPreUsing = NODE_UNDEFINE;
                }
                else
                    nViewIndex = pBlockNode->m_nControlLink[nViewIndex].m_nNextUsing;
            }
            pBlockNode->m_nFirstUsingIndex = NODE_END;
        }
    }

    /*
     * �������������ڴ��
     * @param int nObjectSize,ָ�����ڴ渺�����Ķ�������Ĵ�С��һ���ڴ����ָ��ֻ�ܹ���һ�ֳ��ȵĶ���ķ���
     * @param ObjectStructFunc fInitializeFunc = NULL,Ϊ�ڴ�ط�������ʼ���ĺ���ָ��
     * @param ObjectStructFunc fReleaseFunc = NULL,�ڴ���ڶ����ͷ�ʱ���õĶ��⴦��������������������������Ҫ���⴦����ά��Ĭ��ֵ
     * @return void *,�����ڴ�ع���ָ�룬����ʧ��ʱ����NULL
     */
    void *CreateMemPool(int nObjectSize,ObjectStructFunc fInitializeFunc,ObjectStructFunc fReleaseFunc)
    {
        if (nObjectSize < 1)
            return NULL;
        S_MemoryManager *pManageNode = new S_MemoryManager;
        if (pManageNode != NULL)//����֤newʧ��ʱһ���׳�bad_alloc�쳣��������ָ����Ч�Լ��
        {
            pManageNode->m_nObjectSize = nObjectSize;
            pManageNode->m_pHead = NULL;
            pManageNode->m_pPre = NULL;
            pManageNode->m_pInitializeFunc = fInitializeFunc;
            pManageNode->m_pReleaseFunc = fReleaseFunc;
            pManageNode->m_nBlockNodeCount = 0;
        }
        return pManageNode;
    }

    /*
     * �����������ڴ�ط���ָ����С�ڴ�ռ�
     * @param void *pManageP,��Ч���ڴ�ع���ָ�룬����CreateMenPool����
     * @param void *pParam = NULL,�û��Զ���Ĳ�������������ó�ʼ������������Ϊ�ڶ�������������ú���
     * @param int *pGetIndex = NULL,�����ڽ��յ�ǰ�����ڴ�������ֵ
     * @return void *,���ط��䵽���ڴ�ĵ�ַ�����ִ��󽫷���NULL
     */
    void *AllocMemory(void *pManageP,void *pParam,int *pGetIndex)
    {
        if (pManageP == NULL)
            return NULL;
        S_MemoryManager *pManageNode = (S_MemoryManager *)pManageP;
        S_MemBlockNode *pMemBlockNode = NULL;
        S_MemBlockNode::_ObjectManagerNode * pTmpControlNode;
        int nPreBlockIndex;    
        //�������㹻�ռ������ڴ��
        pMemBlockNode = pManageNode->m_pPre;
        nPreBlockIndex = pManageNode->m_nPreBlockNodeIndex;
        while (pMemBlockNode != NULL)
        {
            if (pMemBlockNode->m_nNextFreeIndex  == NODE_END)
            {
                pMemBlockNode = pMemBlockNode->m_pNextNode;
                --nPreBlockIndex;
            }
            else
            {
                pManageNode->m_pPre = pMemBlockNode;
                pManageNode->m_nPreBlockNodeIndex = nPreBlockIndex;
                break;
            }
        }
        if (pMemBlockNode == NULL)
        {
            //�������ڴ��
            pMemBlockNode = new S_MemBlockNode;
            if (pMemBlockNode == NULL)
                return NULL;
            pMemBlockNode->m_pBlock = malloc(pManageNode->m_nObjectSize * MALLOC_SIZE);
            if (pMemBlockNode->m_pBlock != NULL)
            {
                for (int nLoopVar = 0;nLoopVar < MALLOC_SIZE;++nLoopVar)
                {
                    pTmpControlNode = &(pMemBlockNode->m_nControlLink[nLoopVar]);
                    pTmpControlNode->m_nNextAvailable = nLoopVar + 1;
                    pTmpControlNode->m_nNextUsing = NODE_UNDEFINE;
                    pTmpControlNode->m_nPreUsing = NODE_UNDEFINE;
                }
                pMemBlockNode->m_nControlLink[MALLOC_SIZE - 1].m_nNextAvailable = NODE_END;
                pMemBlockNode->m_nNextFreeIndex = 0;
                pMemBlockNode->m_pTail = (char *)pMemBlockNode->m_pBlock + pManageNode->m_nObjectSize * (MALLOC_SIZE - 1);
                pMemBlockNode->m_pNextNode = pManageNode->m_pHead;
                pMemBlockNode->m_nFirstUsingIndex = NODE_END;
                pManageNode->m_pHead = pMemBlockNode;
                pManageNode->m_pPre = pMemBlockNode;
                nPreBlockIndex = pManageNode->m_nBlockNodeCount;
                pManageNode->m_nPreBlockNodeIndex = nPreBlockIndex;
                pManageNode->m_nBlockNodeCount++;
            }
            else
            {
                delete pMemBlockNode;
                return NULL;
            }
        }
        //�����ڴ�
        int nTmpIndex = pMemBlockNode->m_nNextFreeIndex;
        void *pReturnBlock = &(((char *)(pMemBlockNode->m_pBlock))[pManageNode->m_nObjectSize * nTmpIndex]);
        pTmpControlNode = &(pMemBlockNode->m_nControlLink[nTmpIndex]);
        pMemBlockNode->m_nNextFreeIndex = pTmpControlNode->m_nNextAvailable;
        pTmpControlNode->m_nNextAvailable = NODE_UNDEFINE;
        pTmpControlNode->m_nPreUsing = NODE_END;
        if (pMemBlockNode->m_nFirstUsingIndex != NODE_END)
            pMemBlockNode->m_nControlLink[pMemBlockNode->m_nFirstUsingIndex].m_nPreUsing = nTmpIndex;
        pTmpControlNode->m_nNextUsing = pMemBlockNode->m_nFirstUsingIndex;
        pMemBlockNode->m_nFirstUsingIndex = nTmpIndex;
        if (pManageNode->m_pInitializeFunc != NULL)
            pManageNode->m_pInitializeFunc(pReturnBlock,pParam);
        if (pGetIndex != NULL)
            *pGetIndex = nPreBlockIndex * MALLOC_SIZE + nTmpIndex;
        return pReturnBlock;
    }

    /*
     * �������������ڴ�����ѷ���Ķ���
     * @param void *pManageP,��Ч���ڴ�ع���ָ��
     * @param void *pRecycleObject = NULL,Ҫ���ն����ָ�룬�������ΪNULL����������ж���
     * @param void *pParam = NULL,�û��Զ������������������ͷź���������Ϊ�ú����ĵڶ�����������
     * @return void
     */
    void RecycleMemory(void *pManageP,void *pRecycleObject,void *pParam)
    {
        if (pManageP == NULL)
            return;
        S_MemoryManager *pManageNode = (S_MemoryManager *)pManageP;
        S_MemBlockNode *pNode = pManageNode->m_pHead;
        while (pNode != NULL)
        {
            if (pRecycleObject != NULL)
            {
                if (pNode->m_pBlock <= pRecycleObject && pNode->m_pTail >= pRecycleObject)
                {
                    int nRecycleIndex = ((char *)pRecycleObject - (char *)pNode->m_pBlock) / pManageNode->m_nObjectSize;
                    S_MemBlockNode::_ObjectManagerNode * pTmpControlNode = &(pNode->m_nControlLink[nRecycleIndex]);
                    if (pTmpControlNode->m_nNextAvailable != NODE_UNDEFINE)
                        return;
                    if (pManageNode->m_pReleaseFunc != NULL)
                        pManageNode->m_pReleaseFunc(pRecycleObject,pParam);
                    if (pTmpControlNode->m_nNextUsing != NODE_END)
                        pNode->m_nControlLink[pTmpControlNode->m_nNextUsing].m_nPreUsing = pTmpControlNode->m_nPreUsing;
                    if (pTmpControlNode->m_nPreUsing != NODE_END)
                        pNode->m_nControlLink[pTmpControlNode->m_nPreUsing].m_nNextUsing = pTmpControlNode->m_nNextUsing;
                    else
                        pNode->m_nFirstUsingIndex = pTmpControlNode->m_nNextUsing;
                    pTmpControlNode->m_nNextUsing = NODE_UNDEFINE;
                    pTmpControlNode->m_nPreUsing = NODE_UNDEFINE;
                    pTmpControlNode->m_nNextAvailable = pNode->m_nNextFreeIndex;
                    pNode->m_nNextFreeIndex = nRecycleIndex;
                    return;
                }
            }
            else
                simple_mem_pool::_BlockRecycle(pManageNode,pNode,true,pParam);
            pNode = pNode->m_pNextNode;
        }
    }

    /*
     * �������������ڴ�����ѷ���Ķ���
     * @param void *pManageP,��Ч���ڴ�ع���ָ��
     * @param int nRecycleIndex,Ҫ���ն��������
     * @param void *pParam = NULL,�û��Զ������������������ͷź���������Ϊ�ú����ĵڶ�����������
     * @return void
     */
    void RecycleMemory(void *pManageP,int nRecycleIndex,void *pParam)
    {
        if (pManageP == NULL)
            return;
        S_MemoryManager *pManageNode = (S_MemoryManager *)pManageP;
        int nBlockIndex = pManageNode->m_nBlockNodeCount - nRecycleIndex / MALLOC_SIZE;
        int nObjectIndex = nRecycleIndex % MALLOC_SIZE;
        if (nBlockIndex < 1 || nBlockIndex > pManageNode->m_nBlockNodeCount)
            return;
        S_MemBlockNode *pNode = pManageNode->m_pHead;
        while (pNode != NULL)
        {
            if (nBlockIndex > 1)
            {
                pNode = pNode->m_pNextNode;
                --nBlockIndex;
            }
            else
                break;
        }
        if (pNode != NULL)
        {
            S_MemBlockNode::_ObjectManagerNode * pTmpControlNode = &(pNode->m_nControlLink[nObjectIndex]);
            if (pTmpControlNode->m_nNextAvailable != NODE_UNDEFINE)
                return;
            if (pManageNode->m_pReleaseFunc != NULL)
                pManageNode->m_pReleaseFunc(&(((char *)(pNode->m_pBlock))[nObjectIndex * pManageNode->m_nObjectSize]),pParam);
            if (pTmpControlNode->m_nNextUsing != NODE_END)
                pNode->m_nControlLink[pTmpControlNode->m_nNextUsing].m_nPreUsing = pTmpControlNode->m_nPreUsing;
            if (pTmpControlNode->m_nPreUsing != NODE_END)
                pNode->m_nControlLink[pTmpControlNode->m_nPreUsing].m_nNextUsing = pTmpControlNode->m_nNextUsing;
            else
                pNode->m_nFirstUsingIndex = pTmpControlNode->m_nNextUsing;
            pTmpControlNode->m_nNextUsing = NODE_UNDEFINE;
            pTmpControlNode->m_nPreUsing = NODE_UNDEFINE;
            pTmpControlNode->m_nNextAvailable = pNode->m_nNextFreeIndex;
            pNode->m_nNextFreeIndex = nRecycleIndex;
        }
    }

    /*
     * ���������ͷ��ڴ��
     * @param void **pManageP,�ڴ�ع���ָ�������
     * @param void *pParam = NULL,�û��Զ������������������ͷź���������Ϊ�ú����ĵڶ�����������
     * @return void
     */
    void ReleaseMemPool(void **pManageP,void *pParam)
    {
        if (pManageP == NULL)
            return;
        S_MemoryManager *pManageNode = (S_MemoryManager *)(*pManageP);
        if (pManageNode == NULL)
            return;
        S_MemBlockNode *pNode = pManageNode->m_pHead;
        S_MemBlockNode *pTmp;
        while (pNode != NULL)
        {
            simple_mem_pool::_BlockRecycle(pManageNode,pNode,false,pParam);
            free(pNode->m_pBlock);
            pTmp = pNode;
            pNode = pNode->m_pNextNode;
            delete pTmp;
        }
        delete pManageNode;
        *pManageP = NULL;
    }

    /*
     * ����������������ֵ�����ڴ����
     * @param void *pManageP,��Ч���ڴ�ع���ָ��
     * @param int nFindIndex
     * @return void
     */
    void *FindMemory(void *pManageP,int nFindIndex)
    {
        if (pManageP == NULL)
            return NULL;
        S_MemoryManager *pManageNode = (S_MemoryManager *)pManageP;
        int nBlockIndex = pManageNode->m_nBlockNodeCount - nFindIndex / MALLOC_SIZE;
        int nObjectIndex = nFindIndex % MALLOC_SIZE;
        if (nBlockIndex < 1 || nBlockIndex > pManageNode->m_nBlockNodeCount)
            return NULL;
        S_MemBlockNode *pNode = pManageNode->m_pHead;
        while (pNode != NULL)
        {
            if (nBlockIndex > 1)
            {
                pNode = pNode->m_pNextNode;
                --nBlockIndex;
            }
            else
                break;
        }
        if (pNode != NULL)
        {
            S_MemBlockNode::_ObjectManagerNode * pTmpControlNode = &(pNode->m_nControlLink[nObjectIndex]);
            if (pTmpControlNode->m_nNextAvailable != NODE_UNDEFINE)
                return NULL;
            else
                return &(((char *)(pNode->m_pBlock))[nObjectIndex * pManageNode->m_nObjectSize]);
        }
        else
            return NULL;
    }
}
