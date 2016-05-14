#include <winsock2.h>

#include <QApplication>
#include <QThread>
#include <QFile>
#include <QIcon>

#include "profile/RunningProfile.h"
#include "util/MediaManager.h"
#include "util/ProcessManager.h"
#include "util/ini/CVIniConfig.h"
#include "CoolviewCommon.h"

#include "launcher.h"
#include "preprocessor.h"

int main(int argc, char **argv)
{
	WSADATA wsaData; 
	int rc = WSAStartup(MAKEWORD(2,2),&wsaData);

	// ʹ��װ��������ӵĻ���������Ч - ����DBUSͨ��
	DWORD MsgResult;
	SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, LPARAM("Environment"), SMTO_ABORTIFHUNG, 5000, &MsgResult);

	int procCount = 0;
	if( ProcessManager::isRunning(TEXT(COOLVIEW_PROC_NAME) , &procCount) )
	{
		//��ֹ������������
		if( procCount>1 )
			return 0;
	}

	
	if( argc>1 )
	{
		//����CoolView�Ĳ�����
		for( int i=1; i<argc;i++ )
		{
			string param = argv[i];
			if(param.find("UserInfo=")!=string::npos )
			{
				//UKEY��֤ͨ���Ļ����ڲ����д��ݹܵ�������
				string pipeFullName;
				string pipeName = param;
				DWORD bytesWritten;
				string::size_type startIndex = -1;
				if( (startIndex=pipeName.find("="))!=string::npos )
				{
					pipeName = pipeName.substr( startIndex+1, pipeName.length() );
				}
				pipeFullName = "\\\\.\\pipe\\"+pipeName ;

				if( ::WaitNamedPipeA(pipeFullName.c_str(),NMPWAIT_WAIT_FOREVER) )
				{
					HANDLE hPipe=::CreateFileA(pipeFullName.c_str(),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
					DWORD processID = ::GetCurrentProcessId();
					if( ::WriteFile(hPipe,&processID, sizeof(processID ), &bytesWritten, 0 ) )
					{
						DWORD readBytes;
						INT32 bufferLength;
						if(::ReadFile(hPipe, &bufferLength, sizeof(bufferLength),&readBytes,0  ) )
						{
							char* message = new char[bufferLength+2];
							ZeroMemory( message , bufferLength+2 );
							if( ::ReadFile(hPipe,message,bufferLength+2, &readBytes,0 ) )
							{
								string strMessage = message;
								int startIndex = strMessage.find("|");
								if( startIndex!=string::npos )
								{
									RunningProfile* profile = RunningProfile::getInstance();
									string username = strMessage.substr(0,startIndex );
									string password = strMessage.substr(startIndex+1,strMessage.length() );
									profile->set_ukey_certify(true );
									profile->set_username(username );
									profile->set_password(password);
								}else
								{
									::MessageBoxA(0, "��CoolView��֤�����ж�ȡ���û�������������","����",
										MB_OK );
								}
							}
							delete []message;
						}else
						{
							::MessageBoxA(0, "�޷��������ܵ��ж�ȡ����","����",
								MB_OK );
						}
					}else
					{
						::MessageBoxA(0, "�޷��������ܵ���д������","����",
							MB_OK );
					}
					
					CloseHandle(hPipe );
				}else
				{
					DWORD err = GetLastError();
					printf("%x\n", err );
				}

			}
		}
	}

    QApplication app(argc, argv);

	QTranslator qtTralator;
	qtTralator.load(":/Resources/qt_zh_CN.qm");
	app.installTranslator( &qtTralator );

	//���ò����֧��ͼƬ��ʽ
	QString appDir = QCoreApplication::applicationDirPath();
	appDir.replace("/" , "\\" );
	QString pluginsDir = appDir + "\\plugins";
	app.addLibraryPath( pluginsDir );

	QIcon icon(":/Resources/application.png");
	app.setWindowIcon(icon);

    QFile qss_file(":/qss/dark.qss");
    if (qss_file.open(QIODevice::ReadOnly)) {
      QTextStream s(&qss_file);
      app.setStyleSheet(s.readAll());
      qss_file.close();
    }

  Launcher launcher(nullptr);
  Preprocessor preprocessor(nullptr);
  preprocessor.Initialize(&launcher);
  preprocessor.Start();

  int result = app.exec();

  launcher.Stop();
  MediaManager::getInstance()->ClearStreamedMedia();

  return result;
}
