#include <QtCore/QCoreApplication>

#include "CommonMessageProcess.h"
#include "ContextInput.h"
#include "MouseMove.h"
#include "FileUD.h"
#include "NMessageProcess.h"

#include "floor_manager.h"

#include <process.h>

QCoreApplication *g_pApp;

static void __cdecl ProgrammeThreadSub(void *argument)
{
    //1����ʼ������
    int argc = 1;
    g_pApp = new QCoreApplication(argc,(char **)argument);
    CDataManager::GetInstance();
    CQtDBusServ::GetInstance();
    CNetServ::CreateInstance(5000,CNetServ::Type_Both,5001);
    CContextInput::CreateInstance(Version4,5002);
	CFileUpDownServ::CreateInstance(5003,10,100);
#ifdef USE_FCS
    FloorManager::GetInstance()->Initialize();
    FloorManager::GetInstance()->AddFloor(0);
#endif
    //2������qt�¼�ѭ��
    g_pApp->exec();
    //3���˳�ǰ����
#ifdef USE_FCS
    FloorManager::ReleaseInstance();
#endif // USE_FCS
	CFileUpDownServ::ReleaseInstance();
    CContextInput::ReleaseInstance();
    CNetServ::ReleaseInstance();
    Sleep(10);
    CQtDBusServ::ReleaseInstance();
    CDataManager::ReleaseInstance();
    CMouseMove::ReleaseInstance();
    delete g_pApp;
    //4��֪ͨ���߳��˳�
    g_bProgramme_Exit = false;
}

int main(int argc, char *argv[])
{
    g_bProgramme_Exit = false;
    //1��������������߳�
    _beginthread(ProgrammeThreadSub,0,argv);
    //2���ȴ������ź�

    while (g_bProgramme_Exit == false)
        Sleep(10);
    //���п��Ƴ����˳��õĲ��Դ���
    //int ts;
    //g_bProgramme_Exit = true;
    //scanf("%d",&ts);
    
    //3���˳�qt�¼�ѭ��
    g_pApp->exit();
    //4���ȴ����߳��˳�֪ͨ
    while (g_bProgramme_Exit != false)
        Sleep(5);
    return 0;
}

