#include "Kinect.h" 
#include <stdio.h>
#include <stdlib.h> 


CKinect::CKinect(void)
{
}


CKinect::~CKinect(void)
{
}

// ��Ա�������;
void CKinect::Nui_Zero()
{  
	m_hNextSkeletonEvent = NULL;
  
	m_hThNuiProcess=NULL;
	m_hEvNuiProcessStop=NULL;
 
	m_nThreshold = 4; 

	m_flagPptShowing = 1;
 
	m_nUserIndex = 0; 
	m_bFirstUser = true;

	m_flagPrePptLeftNearToHip   = 0;
	m_flagNextPptRightNearToHip = 0;
 
}


// ��ʼ��Ӧ��;
HRESULT CKinect::Nui_Init()
{

	HRESULT                hr;

	//  ���� �˹���λ���¼��� ����Ǽ�,TRUE��ʾ��Ҫ�˹���λ��FALSE��ʾ��ʼ״̬;
	m_hNextSkeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

   	// ��ʼ��NUI;
	hr = NuiInitialize( 
		NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR);
	if( FAILED( hr ) )
	{
		printf("%s","Kinect initial failed!");
		KinectService::getInstance()->EmitKinectMessage(KM_Error, QString::fromLocal8Bit("Kinect��ʼ����������Kinect�豸") );
		return hr;
	}

	SetHorizon();
	// ����skeleton׷��;
	hr = NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, 0 );
	if( FAILED( hr ) )
	{
		printf("%s\n","can not track the skeleton!");
		KinectService::getInstance()->EmitKinectMessage(KM_Error, QString::fromLocal8Bit("�޷���ʼ������") );
		return hr;
	}
	printf("%s\n","please guarantee there are not more than six person in the kinect  ");
    KinectService::getInstance()->EmitKinectMessage(KM_Error, QString::fromLocal8Bit("�뱣֤������6�˳�����kinect�Ŀ��ӷ�Χ") );
	// ���������߳�;
	m_hEvNuiProcessStop=CreateEvent(NULL,FALSE,FALSE,NULL);
	m_hThNuiProcess=CreateThread(NULL,0,Nui_ProcessThread,this,0,NULL);

	return hr;
}



void CKinect::Nui_UnInit( )
{ 

	// Stop the Nui processing thread
	if(m_hEvNuiProcessStop!=NULL)
	{
		// Signal the thread
		SetEvent(m_hEvNuiProcessStop);

		// Wait for thread to stop
		if(m_hThNuiProcess!=NULL)
		{
			WaitForSingleObject(m_hThNuiProcess,INFINITE);
			CloseHandle(m_hThNuiProcess);
		}
		CloseHandle(m_hEvNuiProcessStop);
	}

	NuiShutdown( );//NUI API

	if( m_hNextSkeletonEvent && ( m_hNextSkeletonEvent != INVALID_HANDLE_VALUE ) )
	{
		CloseHandle( m_hNextSkeletonEvent );
		m_hNextSkeletonEvent = NULL;
	}
  
}

DWORD WINAPI CKinect::Nui_ProcessThread(LPVOID pParam)
{
	printf("%s\n","The kinect thread begin!");
	CKinect *pthis=(CKinect *) pParam;
	HANDLE              hEvents[2];
	int                 nEventIdx;

	// ���ñ��������¼�;
	hEvents[0]=pthis->m_hEvNuiProcessStop; 
	hEvents[1]=pthis->m_hNextSkeletonEvent;

	// ѭ��;
	while(1)
	{
		// �ȴ��¼���֪ͨ,FALSE��ʾֻҪ��һ���¼�������������У�TRUE�������¼�;
		nEventIdx = WaitForMultipleObjects(sizeof(hEvents)/sizeof(hEvents[0]),hEvents,FALSE,100);

		// ����жϵ��¼�����,����ѭ��;
		if(nEventIdx == 0)
		{	
			break;          
		} 
		else{
			pthis->Nui_GotSkeletonAlert( ); // skeleton;
		}
		 
	}

	return (0);
}

 
// �յ�skeleton�¼��ź�;
void CKinect::Nui_GotSkeletonAlert( )
{ 
	NUI_SKELETON_FRAME SkeletonFrame;

	HRESULT hr = NuiSkeletonGetNextFrame( 0, &SkeletonFrame );
 
	// smooth the skeleton ;
	NUI_TRANSFORM_SMOOTH_PARAMETERS nuiSmoothParams;
	nuiSmoothParams.fCorrection = 0.3f;
	nuiSmoothParams.fJitterRadius = 1.0f;
	nuiSmoothParams.fMaxDeviationRadius = 0.5f;
	nuiSmoothParams.fPrediction = 0.4f;
	nuiSmoothParams.fSmoothing = 0.7f;
	NuiTransformSmooth(&SkeletonFrame,&nuiSmoothParams);

	bool bBlank = true;
	int  nNoticePersonNum = 0;
	int  nCnt = 0;
	// save the x position of all the skeleton detected,then sort them and tell the user
	float  SkeletonTracking[NUI_SKELETON_COUNT+1] ;
	for( int i_skeleton = 0 ;  i_skeleton < NUI_SKELETON_COUNT ; i_skeleton++ )
	{
		MyPoint point;
		NuiTransformSkeletonToDepthImageF(
				SkeletonFrame.SkeletonData[i_skeleton].Position,
				&point.fx,&point.fy,&point.depth
				);

// 		if( SkeletonFrame.SkeletonData[i_skeleton].eTrackingState == NUI_SKELETON_NOT_TRACKED )
// 		{
// 			printf("%s\n","not tracking");
// 			printf("%f %f\n",point.fx,point.fy);
// 		//	return;
// 		}
		if( SkeletonFrame.SkeletonData[i_skeleton].eTrackingState == NUI_SKELETON_POSITION_ONLY )
		{
			// save 
			SkeletonTracking[nCnt] = point.fx;
			nCnt++;
            //printf("%s\n","track position only");
			//printf("%f %f\n",point.fx,point.fy);
		}
		if( SkeletonFrame.SkeletonData[i_skeleton].eTrackingState == NUI_SKELETON_TRACKED )
		{
		
			// save 
			SkeletonTracking[nCnt] = point.fx;
			nCnt++;
			//printf("%s\n","tracking");
			// ȷ�����ٵ��û�����:����һ���������;
			if (m_bFirstUser && SkeletonFrame.SkeletonData[i_skeleton].dwTrackingID != 0)
			{
				m_bFirstUser = false;
				nNoticePersonNum = 1;
				m_nUserIndex = SkeletonFrame.SkeletonData[i_skeleton].dwTrackingID;
			}

			// is the skeleton who has the control power
			if (SkeletonFrame.SkeletonData[i_skeleton].dwTrackingID == m_nUserIndex)
			{ 

				// save the x position of the skeleton who has control power
				SkeletonTracking[NUI_SKELETON_COUNT] = point.fx;

				// tell user
				//printf("%f %f\n",point.fx,point.fy);
				if (point.fx < 0.20)
				{
					printf("%s\n","System suggests you move right!");
					KinectService::getInstance()->EmitKinectMessage(KM_Warning, QString::fromLocal8Bit("���������ƶ����Ա�ϵͳ���ò������Ĺ���") );
				}
				if (point.fx  > 0.80)
				{
					printf("%s\n","System suggests you move left!");
					KinectService::getInstance()->EmitKinectMessage(KM_Warning, QString::fromLocal8Bit("���������ƶ����Ա�ϵͳ���ò������Ĺ���") );
				}

			

				if (point.depth < 11000 )
				{ 
					printf("%s\n","too close");
					KinectService::getInstance()->EmitKinectMessage(KM_Warning, QString::fromLocal8Bit("����Kinect�ľ���̫����") );
					return;
				}else if( point.depth > 18000)
				{
					printf("%s\n","too far");
					KinectService::getInstance()->EmitKinectMessage(KM_Warning, QString::fromLocal8Bit("����Kinect�ľ���̫Զ��") );
					return;
				}

				// move the kinect
				if(point.fy  < 0.48)
				{
					MoveUp();
					printf("%s\n","Moving up");
				}
				
				if(point.fy > 0.62)
				{
					MoveDown();
					printf("%s\n","Moving down");
				}

				//��ʾ��������������������Ӧ�ùر���Ϣ����
		     	KinectService::getInstance()->EmitKinectMessage(KM_Normal, "" );
				bBlank = false;
				// process the skeleton position
				for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
				{
					MyPoint point; 
					NuiTransformSkeletonToDepthImageF(SkeletonFrame.SkeletonData[i_skeleton].SkeletonPositions[i], &point.fx, &point.fy , &point.depth); 
					m_History[i].push_front(point);
					if (m_History[i].size() > m_nThreshold)
						m_History[i].pop_back();
				}// for 
			}// if
		}// if
	}// for

	if(nCnt >= 1 )//if more than one skeleton are tracking ,tell the users.
	{
		// range the skeleton's x coordinate by bubble sort.
		for(int i=0; i<nCnt-1; i++)
		{
			int flag = 0;
			for(int j=0; j<nCnt-i; j++)
			{
				if(SkeletonTracking[j] > SkeletonTracking[j+1])
				{
					float tmp = SkeletonTracking[j];
					SkeletonTracking[j] = SkeletonTracking[j+1];
					SkeletonTracking[j+1] = tmp;
					flag = 1;
				}
			}
			if(flag == 0)
				break;
		}


		int nPersonLeftToRight;
		for(nPersonLeftToRight=0; nPersonLeftToRight<nCnt; nPersonLeftToRight++)
		{
			if(fabs(SkeletonTracking[nPersonLeftToRight] - SkeletonTracking[NUI_SKELETON_COUNT]) < 1e-8)
			{
				// get x coordinate of the skeleton who has control power
				break;
			}
		}

		printf("System has %d people in,the %dth(from Letf to Right) is tracking! If it's unacceptable,please clear the kinect by covering your hands  on the camera!\n",nCnt,nPersonLeftToRight+1);//2  3
		char strMsg[200];
		memset(strMsg,'\0',sizeof(strMsg));
		sprintf(strMsg,"ϵͳ��⵽ %d ����,�����ҵ�%d��ӵ�п���Ȩ!���޿���Ȩ�û��뿪kinect��ⷶΧ,�������������סkinect������ͷ���ɿ�,ʹ֮���²�׽������֤������Ҫ̫����.\n",nCnt,nPersonLeftToRight+1);
		//KinectService::getInstance()->EmitKinectMessage(KM_Information, QString::fromLocal8Bit(strMsg) );

	}
	
	if ( bBlank ) // ˵��֮ǰ�Ǹ����Ѿ�������;
	{
		// ���¿�ʼ����;
		m_bFirstUser = true;
		m_nUserIndex = 0;
		printf("%s\n","you are out of range of kinect");
    	KinectService::getInstance()->EmitKinectMessage(KM_Warning, QString::fromLocal8Bit("������Kinect����Ч��Χ") );
	}else{
		HandlePpt();// detect user's action so that control the ppt.
	}
}

void CKinect::HandlePpt()
{
		
	// ����PPT; 
	if (m_History[NUI_SKELETON_POSITION_SHOULDER_LEFT].size() >= m_nThreshold &&
		m_History[NUI_SKELETON_POSITION_SHOULDER_RIGHT].size() >= m_nThreshold &&
		m_History[NUI_SKELETON_POSITION_ELBOW_LEFT].size() >= m_nThreshold &&
		m_History[NUI_SKELETON_POSITION_HIP_CENTER].size() >= m_nThreshold &&
		m_History[NUI_SKELETON_POSITION_WRIST_LEFT].size() >= m_nThreshold )
	{  
		int nFlagPrePpt   = 1; 
		std::list< MyPoint >::iterator iterShoulder =  m_History[NUI_SKELETON_POSITION_SHOULDER_LEFT].begin();
		std::list< MyPoint >::iterator iterShoulderRight =  m_History[NUI_SKELETON_POSITION_SHOULDER_RIGHT].begin();
		std::list< MyPoint >::iterator iterElbow =  m_History[NUI_SKELETON_POSITION_ELBOW_LEFT].begin();
		std::list< MyPoint >::iterator iterWrist =  m_History[NUI_SKELETON_POSITION_WRIST_LEFT].begin();
		std::list< MyPoint >::iterator iterHip =  m_History[NUI_SKELETON_POSITION_HIP_CENTER].begin();

		for ( ;iterShoulder != m_History[NUI_SKELETON_POSITION_SHOULDER_LEFT].end()&&
			iterShoulderRight != m_History[NUI_SKELETON_POSITION_SHOULDER_RIGHT].end()&&
			iterElbow != m_History[NUI_SKELETON_POSITION_ELBOW_LEFT].end()&&
			iterHip != m_History[NUI_SKELETON_POSITION_HIP_CENTER].end()&&
			iterWrist != m_History[NUI_SKELETON_POSITION_WRIST_LEFT].end();
		iterShoulder++,iterElbow++,iterWrist++,iterShoulderRight++)
		{
			float nYShoulderElbow = iterElbow->fy - iterShoulder->fy;// ����;
			float nYWristElbow = iterWrist->fy - iterElbow->fy;
			float nYShoulderWrist = iterWrist->fy - iterShoulder->fy;
			float nXShoulderElbow = iterShoulder->fx - iterElbow->fx;
			float nXWristElbow = iterWrist->fx - iterElbow->fx;
			float nYShouldeRightWrist = iterWrist->fy - iterShoulderRight->fy;
			float nYHipWrist = iterWrist->fy - iterHip->fy;
			float nXHipWrist = iterWrist->fx - iterHip->fx;
			float nXShoulerWrist = iterWrist->fx - iterShoulder->fx;
	 
			if (abs(nXHipWrist) < 0.18
				&& abs(nYHipWrist) < 0.15 )
			{
				m_flagPrePptLeftNearToHip = 1;
			}

			if (abs(nYShoulderElbow) > 0.1 || 
				abs(nYWristElbow) > 0.10 || 
				abs(nYShoulderWrist) > 0.13 ||
			//	abs(nYShouldeRightWrist) > 0.1 ||
				abs(nXShoulerWrist) < 0.18  
				) 
			{
				nFlagPrePpt = 0;
			} 
		}// for
 
		if (m_flagPptShowing == 1 && nFlagPrePpt == 1 && m_flagPrePptLeftNearToHip == 1)
		{ 
			m_History.clear();
			m_flagPrePptLeftNearToHip = 0;

			// pre ppt
			printf("%s\n","pre ppt");
			//���������ź�
			KinectService::getInstance()->EmitGestureSignal("ALL", KG_LeftHandLift );
			////������ʾ��Ϣ
			//KinectService::getInstance()->EmitKinectMessage(KM_Information, QString::fromLocal8Bit("��һҳPPT") );
		}
	 
	}

	if (m_History[NUI_SKELETON_POSITION_SHOULDER_RIGHT].size() >= m_nThreshold &&
		m_History[NUI_SKELETON_POSITION_SHOULDER_LEFT].size() >= m_nThreshold &&
		m_History[NUI_SKELETON_POSITION_ELBOW_RIGHT].size() >= m_nThreshold &&
		m_History[NUI_SKELETON_POSITION_HIP_CENTER].size() >= m_nThreshold &&
		m_History[NUI_SKELETON_POSITION_WRIST_RIGHT].size() >= m_nThreshold )
	{ 	 
		int nFlagEndPpt   = 1;
		int nFlagNextPpt  = 1;

		std::list< MyPoint >::iterator iterShoulderRight =  m_History[NUI_SKELETON_POSITION_SHOULDER_RIGHT].begin();
		std::list< MyPoint >::iterator iterShoulderLeft =  m_History[NUI_SKELETON_POSITION_SHOULDER_LEFT].begin();
		std::list< MyPoint >::iterator iterElbowRight =  m_History[NUI_SKELETON_POSITION_ELBOW_RIGHT].begin();
		std::list< MyPoint >::iterator iterWristRight =  m_History[NUI_SKELETON_POSITION_WRIST_RIGHT].begin();
		std::list< MyPoint >::iterator iterHip =  m_History[NUI_SKELETON_POSITION_HIP_CENTER].begin();

		for ( ;iterShoulderRight != m_History[NUI_SKELETON_POSITION_SHOULDER_RIGHT].end()&&
			iterShoulderLeft != m_History[NUI_SKELETON_POSITION_SHOULDER_LEFT].end()&&
			iterElbowRight != m_History[NUI_SKELETON_POSITION_ELBOW_RIGHT].end()&&
				iterHip != m_History[NUI_SKELETON_POSITION_HIP_CENTER].end()&&
			iterWristRight != m_History[NUI_SKELETON_POSITION_WRIST_RIGHT].end();
		iterShoulderRight++,iterElbowRight++,iterWristRight++,iterShoulderLeft++)
		{
			float nYShoulderElbow = iterElbowRight->fy - iterShoulderRight->fy;
			float nYWristElbow = iterWristRight->fy - iterElbowRight->fy;
			float nYShoulderWrist = iterWristRight->fy - iterShoulderRight->fy;
			float nXShoulderElbow = iterShoulderRight->fx - iterElbowRight->fx;
			float nXWristElbow = iterWristRight->fx - iterElbowRight->fx;
			float nYShoulderLeftWrist = iterShoulderLeft->fy - iterElbowRight->fy;
			float nYHipWrist = iterWristRight->fy - iterHip->fy;
			float nXHipWrist = iterWristRight->fx - iterHip->fx;
			float nXShoulerWrist = iterWristRight->fx - iterShoulderRight->fx;

			if (nYShoulderElbow > 0 || 
				abs(nXWristElbow) > 0.05 ||
				(nYWristElbow) > 0 ||
				nYShoulderWrist > 0 ||
				abs(nXShoulerWrist) > 0.18  
				)  
			{
				nFlagEndPpt = 0;
			}

			if (abs(nXHipWrist) < 0.18
				&& abs(nYHipWrist) < 0.15 )
			{
				m_flagNextPptRightNearToHip = 1;
			}

			if (abs(nYShoulderElbow) > 0.1 || 
				abs(nYWristElbow) > 0.1 || 
				abs(nYShoulderWrist) > 0.13 ||
			//	abs(nYShoulderLeftWrist) > 0.1 ||
				abs(nXShoulerWrist) < 0.18 
				) 
			{
				nFlagNextPpt = 0;   
			}
		}// for

		if (m_flagPptShowing == 1 && nFlagEndPpt == 1)
		{ 
			//m_flagPptShowing = 0; 
			// end ppt
			printf("%s\n","ends the ppt");
						//���������ź�
			KinectService::getInstance()->EmitGestureSignal("ALL", KG_RightHandUp );
			////������ʾ��Ϣ
			//KinectService::getInstance()->EmitKinectMessage(KM_Information, QString::fromLocal8Bit("��������PPT") );
		}
		if (m_flagPptShowing == 1 && nFlagNextPpt == 1 && m_flagNextPptRightNearToHip == 1)
		{ 
			m_History.clear();
			m_flagNextPptRightNearToHip = 0;

			// next ppt
			printf("%s\n","next ppt");
			//���������ź�
			KinectService::getInstance()->EmitGestureSignal("ALL", KG_RightHandLift );
			////������ʾ��Ϣ
			//KinectService::getInstance()->EmitKinectMessage(KM_Information, QString::fromLocal8Bit("��һҳPPT") );
		}

	}// if
}
 
void CKinect::MoveUp()
{
	// #define NUI_CAMERA_ELEVATION_MAXIMUM  27
	// #define NUI_CAMERA_ELEVATION_MINIMUM (-27)
 
	LONG  lAngleDegrees = 0; 
	NuiCameraElevationGetAngle(&lAngleDegrees) ;
	if( lAngleDegrees +2 <= NUI_CAMERA_ELEVATION_MAXIMUM )
	{
		lAngleDegrees += 2;
	}else{
		printf("%s\n","Get the up limit");
	}
	HRESULT hr =  NuiCameraElevationSetAngle(lAngleDegrees);
	if( S_OK == hr )
	{
		printf("%s\n","move the kinect is ok");
	}else if(E_NUI_DEVICE_NOT_READY == hr )
	{
			printf("%s\n","the device is not ready when moving the kinect");
	}else{
		printf("%s\n","unknow error when moving the kinect");
	}
}
void CKinect::MoveDown()
{
	LONG  lAngleDegrees = 0; 
	NuiCameraElevationGetAngle(&lAngleDegrees) ;
	if( lAngleDegrees -2 >= NUI_CAMERA_ELEVATION_MINIMUM )
	{
		lAngleDegrees -= 2;
	}else{
		printf("%s\n","Get the up limit");
		return;
	}
	HRESULT hr =  NuiCameraElevationSetAngle(lAngleDegrees);
	if( S_OK == hr )
	{
		printf("%s\n","move the kinect is ok");
	}else if(E_NUI_DEVICE_NOT_READY == hr )
	{
			printf("%s\n","the device is not ready when moving the kinect");
	}else{
		printf("%s\n","unknow error when moving the kinect");
	}
} 
void CKinect::SetHorizon()
{
	LONG  lAngleDegrees = 0; 
	int nCnt = 0;
	while(1  && nCnt < 100)// try 100 times at most
	{
		nCnt++;
	 	printf("ajust the kinect %ld",lAngleDegrees);
		NuiCameraElevationGetAngle(&lAngleDegrees) ;
		if( abs(lAngleDegrees) > 2 )// make it hrozonly almost
		{

			HRESULT hr =  NuiCameraElevationSetAngle(1);
			if( S_OK == hr )
			{
				printf("%s\n","move the kinect is ok");
			}else if(E_NUI_DEVICE_NOT_READY == hr )
			{
				printf("%s\n","the device is not ready when moving the kinect");
			}else{
				printf("%s\n","unknow error when moving the kinect");
			}
		}else{
			break;
		}
	}
}

