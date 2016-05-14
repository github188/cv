#ifndef __FTPFILETRANS__
#define __FTPFILETRANS__

#include<windows.h>
#include<string>
using namespace std;

struct FtpFile {
	const char *filename;
	FILE *stream;
};

class FtpOperator
{
public:
	static FtpOperator *getInstance()
	{
		if(NULL == FtpOperator::m_pInstance)
			FtpOperator::m_pInstance = new FtpOperator();
		return FtpOperator::m_pInstance;
	}

public:
	virtual ~FtpOperator();

	//����FTP��ַ����Ӧ���˺���Ϣ������ʼ��FTP����
	HRESULT SetFtpConnectInfo(const string &url, const string &name, const string &pwd);
	//�ϴ��ļ�
	HRESULT FtpFileUpload(const string &remotepath, const string &localpath, const long &timeout, const long &tries);
	//�����ļ���targetpathΪ���������·����savepathΪ���ش洢·��
	HRESULT FtpFileDownload(const string &targetpath, const string &savepath);

private:
	FtpOperator();
	class CGarbo			//����Ψһ��������������������ɾ��CSingleton��ʵ��
	{
	public:
		~CGarbo()
		{
			if (FtpOperator::m_pInstance)
				delete FtpOperator::m_pInstance;
		}
	}; 
	static CGarbo Garbo;	//����һ����̬��Ա���ڳ������ʱ��ϵͳ�����������������
private:
	struct dlParam
	{
		string request;
		string userinfo;
		string savepath;
	};

	struct ulParam
	{
		string remotepath;
		string localpath;
		string userinfo;
		long timeout;
		long tries;
	};

	static unsigned __stdcall download_thread(PVOID pParam);
	static unsigned __stdcall upload_thread(PVOID pParam);
	//�����ļ��Ļص�����
	static size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream);

	static size_t getcontentlengthfunc(void *ptr, size_t size, size_t nmemb, void *stream);
	static size_t discardfunc(void *ptr, size_t size, size_t nmemb, void *stream);
	static size_t readfunc(void *ptr, size_t size, size_t nmemb, void *stream);

private:
	static FtpOperator *m_pInstance;
	string m_ftpurl;
	string m_username;
	string m_userpwd;
};

string &string_replace(string &str, const string &old_value, const string &new_value);
#endif