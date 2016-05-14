#pragma once
/*
��Ϣ��ʽר����Ϣ���࣬������Ϣ��ȫ���������������
m_sMessageBox�������������message
length  ����Message�ĳ���
*/
#include <string>
using namespace std;
class  CVMessageFrame
{
public:
	CVMessageFrame(void);
	CVMessageFrame(const char *msg);
	~CVMessageFrame(void);

	//ͨ�ú���
public:
	//��ȡ��Ϣ����
	int getLength();

	//ת����UTF-8��ʽ��ת��m_sMessageBox���ص�, ���ش��ĳ��ȵ�nLen
	char* toString(int &nLen);
	//��ȥ��Ϣ����
	std::string &getMessage();

	//�����Ϣ���ݴ�
	void Append(string str);
	void Append(int nt);

	//��ȡһ�������ԣ��ж���Ϊ"\r\n"����
	string ReadLine();

	//��ȡ��Ϣ����
	int GetLines();

	//��ȡָ��ǰ�Ƽ���
	void SetReadIndex(int rowsCount);

	//��ȡ����ָ����С���ڴ�
	void *read(int size);



protected:
	//��ȡλ��
	int m_nReadIndex;
	//������
	int m_nLength;
	//��
	std::string m_sMessageBox;
	

};
