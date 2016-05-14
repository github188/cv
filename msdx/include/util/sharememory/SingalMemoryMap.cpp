#include "SingalMemoryMap.h"
#include <log/Log.h>

SingalMemoryMap::SingalMemoryMap( const std::string& mapFileName )
{
	m_memoryMapFileName = mapFileName;
	std::string mutexName = m_memoryMapFileName+"Mutex";
	m_mutex = CreateMutex( NULL , false , mutexName.c_str() );
	if( m_mutex == NULL )
	{
		LOG_ERROR("���������ڴ�Ļ�����[%s]ʱ��������",mutexName.c_str() );
	}

	m_hMemoryMap = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, 
		PAGE_READWRITE | SEC_COMMIT, 0, MAX_SHARE_SIZE, m_memoryMapFileName.c_str());

	m_lpData = (LPBYTE)MapViewOfFile( m_hMemoryMap , FILE_MAP_WRITE|FILE_MAP_READ , 0 , 0 , 0 );

	ZeroMemory( m_lpData, MAX_SHARE_SIZE );

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
		LOG_ERROR( "��ȡ�����ݴ�С���ڹ����ڴ棡" );

	if( m_lpData )
		memcpy(pData,m_lpData+offset,cDataSize);

	if (! ReleaseMutex(m_mutex)) 
		LOG_ERROR( "�ͷŻ�����ʱ��������" );
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
		LOG_ERROR( "д������ݴ�С���ڹ����ڴ棡" );

	if( m_lpData )
		memcpy(m_lpData+offset,pData,cDataSize);

	if (! ReleaseMutex(m_mutex)) 
		LOG_ERROR( "�ͷŻ�����ʱ��������" );
}