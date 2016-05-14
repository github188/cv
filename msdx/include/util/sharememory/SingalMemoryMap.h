
#include <string>
#include <windows.h>

/**
 * @biref �����ڴ�����࣬�������ڴ�Ĵ�������д���Լ�����
 */
class SingalMemoryMap
{
public:

	/**
	 * @brief �������ڴ���д������
	 * @param pData ָ��д�빲���ڴ�����ݵ�ָ��
	 * @param cDataSize д�����ݵĴ�С
	 * @param offset д���ڴ��ƫ����
	 * @return NULL
	 */
	virtual void writeData( const void* pData , const int & cDataSize , const int &offset=0 );

	/**
	 * @brief ��ȡ�����ڴ��е�����
	 * @param pData ��Ŷ�ȡ���ݵ�����ָ��
	 * @param cDataSize ��ȡ���ݵĴ�С
	 * @param offset ��ȡ�ڴ��ƫ����
	 * @return NULL
	 */
	virtual void readData( void* pData , const int & cDataSize , const int &offset=0 );

	///���캯��
	SingalMemoryMap(const std::string& memoryMapFileName );
	~SingalMemoryMap();

protected:
	HANDLE				m_hMemoryMap;		//д��Ĺ����ڴ�����Ҳ���û��˶�ȡ�Ĺ����ڴ���
	
	LPBYTE				m_lpData;			//�ڴ�ӳ����ڴ�ָ�룬
	static const int	MAX_SHARE_SIZE = 2048;	//�����ڴ�����ֵ����λΪB

	///�����������ڲ�ͬ����֮��Ķ�дͬ��
	HANDLE				m_mutex;

	///�����ڴ��ӳ���ļ�����
	std::string			m_memoryMapFileName;
	
};
