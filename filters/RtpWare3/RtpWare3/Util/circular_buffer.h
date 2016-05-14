/************************************************************************/
/* Declaration of template class                                        */
/************************************************************************/

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <memory.h>

template<typename T>
class CCircularBuffer
{
public:
	CCircularBuffer(void);
	virtual ~CCircularBuffer(void);

	int  Allocate(unsigned int size);
	void Clear();
	void Destroy();

	//ɾ��ͷ������
	void Pop();

	//�����ڲ����鳤��
	unsigned int BufSize() const;

	//[]����������Կ�ʼλ����Ϊ��ʼ�±�0���������ã�ʵ��������ʣ�����BufSize()���׳��쳣
	T & operator[](const unsigned int index);

protected:

	unsigned int GetPos(unsigned int base, unsigned int offset);

private:

	T * _buf;
	unsigned int _size;

	unsigned int _headPos; //��һ����������λ��
	//unsigned int _tailPos; //���һ����������λ�ã�ע�⣺�м���ܲ�����
	//bool _empty;

};


/************************************************************************/
/* Definition of the class members                                      */
/************************************************************************/
template<typename T>
CCircularBuffer<T>::CCircularBuffer(void)
: _buf(nullptr)
, _size(0)
{
}

template<typename T>
CCircularBuffer<T>::~CCircularBuffer(void)
{
	Destroy();
}

template<typename T>
int CCircularBuffer<T>::Allocate( unsigned int size )
{
	Destroy();
	_buf = new T[size];
	if (!_buf)
	{
		return 0;
	}
	_size = size;

	Clear();

	return _size;
}

template<typename T>
void CCircularBuffer<T>::Clear()
{
	memset(_buf, 0, sizeof(T) * _size);
	_headPos = /*_tailPos =*/ 0;
	//_empty = true;
}

template<typename T>
void CCircularBuffer<T>::Destroy()
{
	if (!_buf)
	{
		delete []_buf;
	}
	_size = 0;
}

template<typename T>
void CCircularBuffer<T>::Pop()
{
	memset(_buf + _headPos, 0, sizeof(T));
	_headPos = GetPos(_headPos, 1);
}

template<typename T>
unsigned int CCircularBuffer<T>::BufSize() const
{
	return _size;
}


template<typename T>
T & CCircularBuffer<T>::operator[]( const unsigned int index )
{
	//����λ��
	unsigned int pos = GetPos(_headPos, index);

	return _buf[pos];

}

template<typename T>
unsigned int CCircularBuffer<T>::GetPos( unsigned int base, unsigned int offset )
{
	if (offset > _size)
	{
		throw "Index overflow.";
	}
	base += offset;
	while (base >= _size)
	{
		base -= _size;
	}
	return base;
}

#endif
