// msdx.cpp : ���� DLL Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "msdx.h"
#include "log\Log.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	//init logger
	SetLogComponent("msdx");

    return TRUE;
}

// ���ǵ���������һ��ʾ��
MSDX_API int nmsdx=0;

// ���ǵ���������һ��ʾ����
MSDX_API int fnmsdx(void)
{
	return 42;
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� msdx.h
Cmsdx::Cmsdx()
{ 
	return; 
}
