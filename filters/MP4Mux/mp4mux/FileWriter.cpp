#include "stdafx.h"
#include "FileWriter.h"
#include "logger.h"

#include <process.h>

class BufferBlock
{
public:

	BufferBlock( long cSize)
	{
		_pos     = 0;
		_cSize   = cSize;
		_cBytes  = 0;
		_pBuffer = new BYTE[cSize];
	}

	BufferBlock( LONGLONG pos, const BYTE* pBuffer, long cBytes )
	{
		_pos     = pos;
		_cSize   = cBytes;
		_cBytes  = cBytes;
		_pBuffer = new BYTE[cBytes];
		memcpy(_pBuffer, pBuffer, cBytes);
	}

	~BufferBlock()
	{
		delete [] _pBuffer;
	}

	void Reset(LONGLONG pos)
	{
		_pos = pos;
		_cBytes = 0;
	}

	bool Contains( LONGLONG pos, long cBytes)
	{
		// ��pos >= _pos ʱ���滻д��ַ�ڻ������У�������д���ļ�
		return pos >= _pos; // ignore pos < _pos + BUF_SIZE, this is impossible, see Append
		UNREFERENCED_PARAMETER(cBytes);
	}

	LONGLONG Position()
	{
		return _pos;
	}

	long Length()
	{
		return _cBytes;
	}

	bool IsFull()
	{
		return _cBytes == _cSize;
	}

	BYTE * Buffer()
	{
		return _pBuffer;
	}

	long Append( const BYTE* pBuffer, long cBytes )
	{
		long cFree   = _cSize - _cBytes;
		long cActual = (cFree >= cBytes ? cBytes : cFree);
		memcpy(_pBuffer + _cBytes, pBuffer, cActual);
		_cBytes += cActual;
		return cActual;
	}

	long Replace( LONGLONG pos, const BYTE* pBuffer, long cBytes)
	{
		// ע�⣺ͨ������Mux����ԭ����Զ϶����������滻д���������������д���ֽڷ�Χ�����
		LONGLONG index = pos - _pos;
		memcpy(_pBuffer + index, pBuffer, cBytes);
		return cBytes;
	}

private:
	LONGLONG _pos;      // begin pos in the file
	long     _cSize;    // size of buffer block
	long     _cBytes;   // bytes actually used
	BYTE   * _pBuffer;  // ptr to the mem block
};


FileWriter::FileWriter()
	: m_hFile(INVALID_HANDLE_VALUE)
	, m_hProc(NULL)
	, m_bStop(true)
	, m_pCurrent(NULL)
	, m_llWritenBytes(0)
	, m_lBufCount(0)
{
	BufferBlock * pBlock = NULL;
	// ��ʼ����������������һ��ģ�һ�����˺�ȴ���д����̣�һ�����ڻ��塣
	// ����������Ӧ���㹻�����Ǵ���д������
	for (int i = 0; i < 2; ++i)
	{
		pBlock = new BufferBlock(BUF_SIZE);
		m_queueFree.push(pBlock);
	}
}

FileWriter::~FileWriter(void)
{
	Stop();
	while (!m_queueFree.empty())
	{
		BufferBlock * pBlock = m_queueFree.front();
		m_queueFree.pop();
		delete pBlock;
	}
	// Stopʱִ����Flush��������m_pCurrent����Ϊ��Чָ�룬m_queueFull��m_queuePending��Ϊ��
}

void FileWriter::SetFile(HANDLE hFile)
{
	m_hFile = hFile;
}

void FileWriter::Start()
{
	if (NULL != m_hProc || !m_bStop)
	{
		return;
	}
	m_bStop = false;
	m_hProc = (HANDLE)_beginthreadex(NULL, 0, _WriterProc, (void *)this, 0, NULL);
	printf("[MP4Mux] Start Writer\n");
}

void FileWriter::Stop()
{
	if (m_bStop)
	{
		return;
	}
	m_bStop = true;
	if (NULL != m_hProc)
	{
		WaitForSingleObject(m_hProc, INFINITE);
	}
	Flush();
	printf("[MP4Mux] Stop Writer\n");
}

void FileWriter::Add( LONGLONG pos, const BYTE* pBuffer, long cBytes )
{
	if (m_hFile == INVALID_HANDLE_VALUE || m_hFile == NULL || cBytes == 0) {
		return;
	}

	if (m_pCurrent == NULL)
	{
		_GetBufferBlock();
	}

	// ��ֹ�滻д�������滻�������ڻ�������
	if (m_pCurrent->Contains(pos, cBytes))
	{
		if (pos == m_pCurrent->Position() + m_pCurrent->Length())
		{
			long cWriten = m_pCurrent->Append(pBuffer, cBytes);
			if (cWriten < cBytes) // current buffer full
			{
				CAutoLock lock(&m_csQueue);
				m_queueFull.push(m_pCurrent);
				m_pCurrent = NULL;
				_GetBufferBlock();
				m_pCurrent->Append(pBuffer + cWriten, cBytes - cWriten);
			}
		}
		else
		{
			m_pCurrent->Replace(pos, pBuffer, cBytes);
		}
	}
	else
	{
		CAutoLock lock(&m_csQueue);
		BufferBlock * pBlock = new BufferBlock(pos, pBuffer, cBytes);
		m_queuePending.push(pBlock);
	}
}

void FileWriter::Flush()
{
	if (!m_bStop)
	{
		return;
	}
	printf("[MP4Mux] Flush Writer, queueFull=%d, queuePending=%d\n", m_queueFull.size(), m_queuePending.size());

	if (m_pCurrent)
	{
		m_queueFull.push(m_pCurrent);
		m_pCurrent = NULL;
	}

	while (m_queueFull.size() > 0)
	{
		BufferBlock * pBlock = m_queueFull.front();
		m_queueFull.pop();
		_WriteBufToFile(pBlock);
		m_queueFree.push(pBlock);
	}

	while (m_queuePending.size() > 0)
	{
		BufferBlock * pBlock = m_queuePending.front();
		m_queuePending.pop();
		_WriteToFile(pBlock->Position(), pBlock->Buffer(), pBlock->Length()); // Write Directly!
		delete pBlock;
	}
}

unsigned __stdcall FileWriter::_WriterProc( void * param)
{
	FileWriter * writer = (FileWriter *)param;
	writer->_WriterLoop();
	return 0;
}

void FileWriter::_WriterLoop()
{
	while (!m_bStop)
	{
		// handle full buffer queue
		unsigned int queueSize = 0;
		{
			CAutoLock lock(&m_csQueue);
			queueSize = m_queueFull.size();
		}
		while (queueSize > 0)
		{
			BufferBlock * pBlock = NULL;
			{
				CAutoLock lock(&m_csQueue);
				pBlock = m_queueFull.front();
				m_queueFull.pop();
				queueSize = m_queueFull.size();
			}

			_WriteBufToFile(pBlock);

			{
				CAutoLock lock(&m_csQueue);
				m_queueFree.push(pBlock);
			}
		}
		// handle pending queue
		{
			CAutoLock lock(&m_csQueue);
			queueSize = m_queuePending.size();
		}
		while (queueSize > 0)
		{
			BufferBlock * pBlock = NULL;
			{
				CAutoLock lock(&m_csQueue);
				pBlock = m_queuePending.front();
			}

			if (pBlock->Position() + pBlock->Length() > m_llWritenBytes)
			{
				break;
			}
			_WriteToFile(pBlock->Position(), pBlock->Buffer(), pBlock->Length()); // Write Directly!
			{
				CAutoLock lock(&m_csQueue);
				m_queuePending.pop();
				queueSize = m_queuePending.size();
			}
			delete pBlock;
		}
		// sleep some time
		Sleep(10);
	}
}

void FileWriter::_WriteBufToFile( BufferBlock * pBlock )
{
	if (0 == pBlock->Length())
	{
		return;
	}
	//printf("[MP4Mux] Write file %I64d + %d\n", m_llWritenBytes, m_nBuf);
	_WriteToFile(pBlock->Position(), pBlock->Buffer(), pBlock->Length());
	m_llWritenBytes += pBlock->Length();
}

void FileWriter::_WriteToFile( LONGLONG pos, const BYTE* pBuffer, long cBytes )
{
	if (m_hFile == INVALID_HANDLE_VALUE || m_hFile == NULL || cBytes == 0) {
		return;
	}

	LARGE_INTEGER liTo;
	LARGE_INTEGER liUnused;
	liTo.QuadPart = pos;

	if (SetFilePointerEx(m_hFile, liTo, &liUnused, FILE_BEGIN))
	{
		DWORD dwWritten;

		if (!WriteFile(m_hFile, (PVOID)pBuffer, (DWORD)cBytes,
			&dwWritten, NULL)) 
		{
			DWORD dwErr = GetLastError();
			LOG(("WriteFile error. Code %d.", dwErr));
			printf("[MP4Mux] WriteFile error. Code %d.\n", dwErr);
		}
	}
	else
	{
		DWORD dwErr = GetLastError();
		LOG(("SetFilePointerEx error. Code %d.", dwErr));
		printf("[MP4Mux] SetFilePointerEx error. Code %d.\n", dwErr);
	}
}

void FileWriter::_GetBufferBlock()
{
	if (m_pCurrent != NULL)
	{
		return;
	}

	{
		CAutoLock lock(&m_csQueue);
		// �����ڿ��л�����ʱ
		if (!m_queueFree.empty())
		{
			m_pCurrent = m_queueFree.front();
			m_queueFree.pop();
		} 
		// ���Ǵ���д�����裬���򲻻ᵽ��˴�
		else
		{
			LOG(("Writer buffer go beyond limit.(size=%d)", m_queueFull.size() + 1));
			printf("[MP4Mux] Writer buffer go beyond limit.(size=%d)\n", m_queueFull.size() + 1);
			m_pCurrent = new BufferBlock(BUF_SIZE);
		}
	}
	m_pCurrent->Reset((LONGLONG)m_lBufCount * BUF_SIZE);
	++m_lBufCount;
}
