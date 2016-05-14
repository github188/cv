/************************************************************************/
/*   2011-8-19 created 
/*   @author huangkq
/*   description : used to control the ppt by the guest 
/************************************************************************/
#ifndef KINECT_H
#define KINECT_H
#include <windows.h> 
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <list>
#include <time.h>
#include "MSR_NuiApi.h"
#include "KinectService.h"
using namespace std;
 
#pragma comment(lib,"MSRKinectNUI.lib")

class MyPoint
{
public:
	float fx;
	float fy;
	USHORT depth;
};

class CKinect
{
public:
	CKinect(void);
	virtual ~CKinect(void);

	HRESULT		       Nui_Init();              // ��ʼ�������������߳�ȥ��Ӧ��Ϣ;
	void                    Nui_UnInit( );           // ע��; 
 
	void                    Nui_GotSkeletonAlert( ); // ��Ӧ�Ǽ��¼�;
	void                    Nui_Zero();              // ��ʼ������������;

	void		              HandlePpt();

	void                    MoveUp();
	void                    MoveDown();
	void                    SetHorizon();
private: 
	
	// create a thread to process the NUI
	static DWORD WINAPI     Nui_ProcessThread(LPVOID pParam);   

	// thread handling;
	HANDLE        m_hThNuiProcess;         // �̵߳ľ��;
	HANDLE        m_hEvNuiProcessStop;     // �ж��߳�;

	std::map< int, std::list<MyPoint> > m_History;  
	HANDLE        m_hNextSkeletonEvent;    // �Ǽܵ��¼�;

	int           m_flagPptShowing;
	
	size_t 	      m_nThreshold;
	DWORD	      m_nUserIndex;	 
	bool          m_bFirstUser;

	int           m_flagPrePptLeftNearToHip;
	int           m_flagNextPptRightNearToHip;

};

#endif
