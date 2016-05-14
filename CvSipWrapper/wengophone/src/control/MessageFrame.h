#pragma once
#include <string>
/*
��Ϣ��ʽר����Ϣ���࣬������Ϣ��ȫ���������������
m_sMessageBox�������������message
length  ����Message�ĳ���

ע�⣺ ���캯����char*[] �������ַ���������\0��β���������ɵ�MessageFrame�����
*/
class  CMessageFrame
{
public:
	CMessageFrame(const char *msg=NULL);//Modify By LZY 2010-09-19
	~CMessageFrame(void);

	//ͨ�ú���
public:
	//��ȡ��Ϣ����
	int getLength();

	//ת����UTF-8��ʽ��ת��m_sMessage���ص�
	char* toUtf8();
	char* getString();

	//�����Ϣ���ݴ�
	//void Append(CString str);
	void Append(const char* str);
	void Append(const int nt);
	void Append(const unsigned int nt);//Add By LZY 2010-09-19 ���uint����ֵ��������
	void Append(const unsigned long nt);//Add By LZY 2010-10-05 ���ulong����ֵ��������

	//��ȡһ�������ԣ��ж���Ϊ"\r\n"����
	char* readLine(bool ReceiveReturn);//Modify By LZY 2010-09-10 ���Ӵ��������ԭ��MessageFrame.cpp�иú������޸�˵��

	//��ȡ��Ϣ����
	int getLines();

	//��ȡָ��ǰ�Ƽ���
	void moveForward(int rowsCount);

	//-------Add By LZY 2010-09-10-------
	//   -   ��ȡ�ӵ�ǰ�ڲ�����ָ�뵽��β����������
	char* readLast();
	//-------End of Add By LZY-----------

	////���ö�ȡ���
	void resetIndex(); //Add by hexianfu 09-11-9

protected:
	//��ȡλ��
	int m_nReadIndex;
	//������
	int m_nLength;
	//��
	//CString m_sMessageBox;
	//Dimer 11.26 add
	std::string m_sMessage;

	//-------Add By LZY 2010-09-10-------
	//   -   ��������Ϣ�����õ���ر���
	char *LineBuffer;//����Ϣ������ָ��
	int BufferSize;//����Ϣ��������С
	//-------End of Add By LZY----------

	//-------Add By LZY 2010-09-15-------
	//   -   ����תutf8�����Ϣ�����õ���ر���
	char *UTF8_Buffer;//UTF8��Ϣ������ָ��
	int UTF8_BufferSize;//UTF8��Ϣ��������С
	//-------End of Add By LZY----------
};
