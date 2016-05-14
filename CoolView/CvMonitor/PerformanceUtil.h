#ifndef PERFORMANCE_UTIL_H
#define PERFORMANCE_UTIL_H
#include <Windows.h>
#include <vector>
#include <Iphlpapi.h>
#include "CpuUsage.h"


using namespace std;



class PerformanceUtil
{
public:

	PerformanceUtil();
	//��ȡָ�����Ƶ��ڴ�ʹ��������λΪKB
	int getProcMemoryUsage( const WCHAR* pProcessName  );

	//��ȡϵͳ���õ��ڴ�ʹ��������λΪKB //add by lzb
	int getToalMemoryUsage();

	//��ȡϵͳ���õ��ڴ�ʹ��������λΪKB //add by lzb
	int getAvailableMemoryUsage();

	//��ȡָ�����Ƶ�CPUʹ����
	int getProcCpuUsage( LPCTSTR pProcessName );
	
	int getTotalCPUUsage(){return _cpuUsage.GetCpuUsage();}

	bool isProcessRunning( const LPCTSTR pProcessName );
	
	//��ȡ�ϴη��ʵ����ڵ����к����д��� ��СΪKB
	bool getNetworkFlow(double & outBandWidth,double & inBandWidth);

	bool isProcessRunningByID(int processID);

private:
	CCpuUsage		_cpuUsage;
	DWORD           _lastInFlow;
	DWORD           _lastOutFlow;
};

#endif