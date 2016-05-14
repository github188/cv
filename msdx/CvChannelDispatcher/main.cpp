#include <QtCore/QCoreApplication>
#include "CvChannelDispatcherService.h"

#include <Windows.h>
#include "util/ProcessManager.h"
#include "log/Log.h"

int main(int argc, char *argv[])
{
    //���ڵ�ChannelDispatcher��DBus��Service��ʽ�Զ�����,����Ҫ���
    //int procCount = 0;
    //if( ProcessManager::isRunning(TEXT("CvChannelDispatcher.exe") , &procCount) )
    //{
    //    //��ֹ������������
    //    if( procCount>1 )
    //        return 0;
    //}

    //SetConsoleTitleA( "CvChannelDispatcher.exe" );

    //����log�ļ���
    SetLogComponent("CvChannel" );

    QCoreApplication a(argc, argv);

    CvChannelDispatcherService service;
    if (0 != service.Initial())
    {
        return 0;
    }

    //::SetPriorityClass( ::GetCurrentProcess() , HIGH_PRIORITY_CLASS );

    return a.exec();
}
