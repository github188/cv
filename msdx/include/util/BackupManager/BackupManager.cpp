#include "BackupManager.h"
#include "log/log.h"

//
//���ڱ��ݵĹ����ڴ�ṹΪ��
// �������Щ������������Щ������������Щ������������Щ���������
// ��int �� BlockData  �� BlockData  �� BlockData  �� ......
// �������ة������������ة������������ة������������ة���������
//����int��ʾ����BlockData������
//
//ÿһ��BlockDataΪ��
// �������Щ������������Щ������������Щ������������Щ������������Щ������������Щ�������������
// �� id �� consistent ��    type    �� data1_size ��   data1    �� data2_size ��   data2    ��
// �������ة������������ة������������ة������������ة������������ة������������ة�������������
//����dataΪ���ݵ����ݣ����ͷ�ʽ��ʹ�������о�������֧�ֱ�����������
//

#define MAX_DATA_PER_BLOCK 2

struct BlockData
{
    bool consistent; //��ʾ��ǰ�����Ƿ�����һ����
    char id[32];
    char type[128];
    struct Data
    {
        int size;
        char data[2048];
    } datas[MAX_DATA_PER_BLOCK];
};

#define DATA_BLOCK_SIZE sizeof(BlockData) //Ϊÿ��Controller׼�������ݱ�������С
#define MAX_BLOCK_COUNT 512 //���֧�ֵı�����
#define BACKUP_SHAREDMEM_NAME "CvChannelBackup" //���ڱ��ݵĹ����ڴ�����


BackupManagerPtr BackupManager::instance_;


BackupManager::BackupManager(void)
    : count_(0)
    , data_(NULL)
{
}

BackupManager::~BackupManager(void)
{
    shared_mem_.detach();
    count_ = 0;
    data_ = NULL;
}

BackupManagerPtr BackupManager::Instance()
{
    //�÷������ڶ��߳��е���,���ض��������߳������ȵ���,�ʲ��ؼ�������
    if (!instance_)
    {
        instance_.reset(new BackupManager());
        instance_->Init();
    }
    return instance_;
}

void BackupManager::Init()
{
    shared_mem_.setKey(BACKUP_SHAREDMEM_NAME);
    if (!shared_mem_.attach(QSharedMemory::ReadWrite))
    {
        //�����ڱ��ݹ����ڴ�,�����µ�
        const int size = DATA_BLOCK_SIZE * MAX_BLOCK_COUNT + sizeof(int); //�����һ��int�������ڴ洢�ѱ��ݵ����ݿ�����
        bool ret = shared_mem_.create(size, QSharedMemory::ReadWrite);
        if (!ret)
        {
            LOG_ERROR("Create backup memory failed: %s", 
                shared_mem_.errorString().toLocal8Bit().data());
            return;
        }
        LOG_PRINTF("Create backup memory succeed.");
        char * ptr = static_cast<char *>(shared_mem_.data());
        memset(ptr, 0, size);
        count_ = reinterpret_cast<int *>(shared_mem_.data());
        data_ = reinterpret_cast<BlockData *>(count_ + 1); //������һ�������ڴ���
    }
    else
    {
        //���ڱ�������,�ؽ�����
        LOG_INFO("Found backup memory created before.");
        count_ = reinterpret_cast<int *>(shared_mem_.data());
        data_ = reinterpret_cast<BlockData *>(count_ + 1); //������һ�������ڴ���
        for (int i = 0; i < *count_; ++i)
        {
            BlockData * cur = data_ + i;
            QString id = cur->id;
            index_table_[id] = i;
        }
    }
}

int BackupManager::GetCount() const
{
    return index_table_.size();
}

BackupManager::IDList BackupManager::GetMediaIDList() const
{
    return index_table_.keys();
}

int BackupManager::Backup( QString id, QString type, int index, const QByteArray &data )
{
    if (!shared_mem_.isAttached() || !data_ || index >= MAX_DATA_PER_BLOCK)
    {
        return -1;
    }
    BlockData * cur = NULL;
    int block_index = 0;
    if (!index_table_.contains(id))
    {
        //����ÿ��controller�ڸ����߳��д���ִ��,�������ط����ʸ������������ؼ���
        //���޸Ĺ�����������ͼ�����ʱ�������,�����п��ܵ����controllerͬʱ����
        //����ʱ,�õ���ͬ������
        QWriteLocker locker(&lock_); //��ֹ���ǽ���
        //�������޸���,����
        block_index = index_table_.size();
        if (block_index >= MAX_BLOCK_COUNT)
        {
            LOG_ERROR("Cannot backup more than %d blocks", MAX_BLOCK_COUNT);
            return -1;
        }
        //�µ����ݽ����뱸���б�ĩβ
        index_table_[id] = block_index;
        //��������
        ++(*count_);
        locker.unlock();

        //��¼ID������
        cur = data_ + block_index;
        if (id.size() > sizeof(cur->id) - 1)
        {
            LOG_ERROR("Backup failed: id %s is too large!", id.toLocal8Bit().data());
            return -2;
        }
        if (type.size() > sizeof(cur->type) - 1)
        {
            LOG_ERROR("Backup failed: type %s is too large!", type.toLocal8Bit().data());
            return -2;
        }
        strcpy_s(cur->id, id.toLocal8Bit().data());
        strcpy_s(cur->type, type.toLocal8Bit().data());
    }
    else
    {
        //����ȡ��������
        QReadLocker locker(&lock_); //��ֹ���ǽ���
        block_index = index_table_[id];
        cur = data_ + block_index;
    }

    BlockData::Data *d = &(cur->datas[index]);

    if (data.size() > sizeof(d->data) - 1)
    {
        LOG_ERROR("Backup failed: data of %d bytes is too large!", data.size());
        return -2;
    }

    cur->consistent = false; //Inconsistent
    memcpy_s(d->data, sizeof(d->data) - 1, data.data(), data.size());
    d->size = data.size();
    cur->consistent = true; //Consistent

    return 0;
}

int BackupManager::FetchType( QString id, QString &type )
{
    if (!shared_mem_.isAttached() || !data_)
    {
        return -1;
    }

    QReadLocker locker(&lock_); //��ʽ���ǽ���
    if (!index_table_.contains(id))
    {
        //�������޸���,����
        return -1;
    }
    const int index = index_table_[id];
    locker.unlock();

    const BlockData * cur = data_ + index;
    type = cur->type;

    return 0;
}

int BackupManager::FetchData( QString id, int index, QByteArray &data )
{
    if (!shared_mem_.isAttached() || !data_ || index >= MAX_DATA_PER_BLOCK)
    {
        return -1;
    }

    QReadLocker locker(&lock_); //��ʽ���ǽ���
    if (!index_table_.contains(id))
    {
        //�������޸���
        return -1;
    }
    const int block_index = index_table_[id];
    locker.unlock(); //���ٳ�������ʱ��

    const BlockData * cur = data_ + block_index;
    if (!cur->consistent)
    {
        LOG_ERROR("Backup data of %s is inconsistent.", id.toLocal8Bit().data());
        return -2;
    }

    data = QByteArray(cur->datas[index].data, cur->datas[index].size);

    return 0;
}
