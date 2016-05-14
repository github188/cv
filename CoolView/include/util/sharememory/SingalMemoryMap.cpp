#include "SingalMemoryMap.h"
#include <log/Log.h>


SingalMemoryMap::SingalMemoryMap( const std::string& mapFileName )
{
	m_memoryMapFileName = mapFileName;
	std::string mutexName = m_memoryMapFileName+"Mutex";
	m_mutex = CreateMutexA( NULL , false , mutexName.c_str() );
	if( m_mutex == NULL )
	{
		_snprintf(__global_msg , sizeof(__global_msg), "���������ڴ�Ļ�����[%s]ʱ��������",mutexName.c_str() );
		MSDX_LOG_ERROR( __global_msg);
	}

	m_hMemoryMap = CreateFileMappingA( INVALID_HANDLE_VALUE, NULL, 
		PAGE_READWRITE | SEC_COMMIT, 0, MAX_SHARE_SIZE, m_memoryMapFileName.c_str());

	m_lpData = (LPBYTE)MapViewOfFile( m_hMemoryMap , FILE_MAP_WRITE|FILE_MAP_READ , 0 , 0 , 0 );

	if (m_lpData == NULL)
	{
		throw "invalid situation.";
	}

}

SingalMemoryMap::~SingalMemoryMap()
{
	if (m_lpData != NULL)
	{
		UnmapViewOfFile(m_lpData);
		m_lpData = NULL;
	}
	
	if (m_hMemoryMap != NULL)
	{
		CloseHandle(m_hMemoryMap);
		m_hMemoryMap = NULL;
	}

	if( m_mutex!=NULL )
	{
		CloseHandle( m_mutex );
		m_mutex = NULL;
	}

}



/**
* @brief ��ȡ�����ڴ��е�����
* @param pData ��Ŷ�ȡ���ݵ�����ָ��
* @param cDataSize ��ȡ���ݵĴ�С 
* @return NULL
*/
void SingalMemoryMap::readData( void* pData , const int& cDataSize , const int& offset )
{

	WaitForSingleObject( m_mutex , INFINITE );

	if( cDataSize>MAX_SHARE_SIZE )
		MSDX_LOG_ERROR( "��ȡ�����ݴ�С���ڹ����ڴ棡" );

	if( m_lpData )
		memcpy(pData,m_lpData+offset,cDataSize);

	if (! ReleaseMutex(m_mutex)) 
		MSDX_LOG_ERROR( "�ͷŻ�����ʱ��������" );
}

/**
* @brief �������ڴ���д������
* @param pData ָ��д�빲���ڴ�����ݵ�ָ��
* @param cDataSize д�����ݵĴ�С
* @return NULL
*/
void SingalMemoryMap::writeData( const void*pData , const int& cDataSize , const int& offset)
{
	WaitForSingleObject( m_mutex , INFINITE );

	if( cDataSize>MAX_SHARE_SIZE )
		MSDX_LOG_ERROR( "д������ݴ�С���ڹ����ڴ棡" );

	if( m_lpData )
		memcpy(m_lpData+offset,pData,cDataSize);

	if (! ReleaseMutex(m_mutex)) 
		MSDX_LOG_ERROR( "�ͷŻ�����ʱ��������" );
}
//
//SingalMemoryMapWriter::SingalMemoryMapWriter( const std::string& mapFileName ):SingalMemoryMap(mapFileName)
//{
//	m_writeCount = 0;
//	m_memoryMapFileName = mapFileName;
//	m_hMemoryMap = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, 
//		PAGE_READWRITE | SEC_COMMIT, 0, MAX_SHARE_SIZE, SingalMemoryMap::m_memoryMapFileName.c_str());
//
//	m_lpData = (LPBYTE)MapViewOfFile( m_hMemoryMap , FILE_MAP_WRITE , 0 , 0 , 0 );
//
//	if (m_lpData == NULL)
//	{
//		CloseHandle(m_hMemoryMap);
//		m_hMemoryMap = NULL;
//	}
//
//	if( m_lpData==NULL||m_hMemoryMap==NULL )
//	{
//		_snprintf( __global_msg , sizeof(__global_msg) , "���������ڴ�[%s]ʧ��!", SingalMemoryMap::m_memoryMapFileName.c_str());
//		MSDX_LOG_ERROR(__global_msg);
//	}
//}
//
//void SingalMemoryMapWriter::writeData( const void* pData , const int & cDataSize , const int &offset/*=0 */ )
//{
//	//������ǵ�һ��д���ݣ�����Ҫ�ȴ�д�ź���
//	if( m_writeCount>0 )
//		WaitForSingleObject( m_writerSemaphore , INFINITE );
//
//
//	printf("Write to shareMemory %d times\n" , m_writeCount );
//
//	if( cDataSize>MAX_SHARE_SIZE )
//		MSDX_LOG_ERROR( "д������ݴ�С���ڹ����ڴ棡" );
//
//	if( m_lpData )
//		memcpy(m_lpData+offset,pData,cDataSize);
//
//	if( m_writerSemaphore!=NULL )
//	{
//		if( ReleaseSemaphore(m_writerSemaphore,1,NULL)==0 )
//			MSDX_LOG_ERROR("�ͷ�д�ź�����ʱ��������");
//	}else
//	{
//		MSDX_LOG_ERROR("�쳣�����д�ź��������ڣ�");
//	}
//	m_writeCount++;
//}

//
//SingalMemoryMapReader::SingalMemoryMapReader(const std::string& mapFileName ):SingalMemoryMap(mapFileName)
//{
//	m_memoryMapFileName = mapFileName;
//	m_hMemoryMap = OpenFileMapping(FILE_MAP_READ , FALSE , m_memoryMapFileName.c_str());
//
//	if( m_hMemoryMap==NULL )
//	{
//		_snprintf(__global_msg , sizeof(__global_msg) , "��Ӧ��[%s]writerû�д򿪣�",SingalMemoryMap::m_memoryMapFileName.c_str() );
//		MSDX_LOG_ERROR( __global_msg );
//	}
//	m_lpData = (LPBYTE)MapViewOfFile( m_hMemoryMap , FILE_MAP_READ, 0 , 0 , 0 );
//
//	if (m_lpData == NULL)
//	{
//		CloseHandle(m_hMemoryMap);
//		m_hMemoryMap = NULL;
//	}
//}
//
//void SingalMemoryMapReader::readData( void* pData , const int & cDataSize , const int &offset/*=0 */ )
//{
//	WaitForSingleObject( m_writerSemaphore , INFINITE );
//
//	if( cDataSize>MAX_SHARE_SIZE )
//		MSDX_LOG_ERROR( "��ȡ�����ݴ�С���ڹ����ڴ棡" );
//
//	if( m_lpData )
//		memcpy(pData,m_lpData+offset,cDataSize);
//
//	if( ! ReleaseSemaphore( m_writerSemaphore , 1,NULL) )
//	{
//		MSDX_LOG_ERROR( "�ͷŻ�����ʱ��������" );
//	}
//}