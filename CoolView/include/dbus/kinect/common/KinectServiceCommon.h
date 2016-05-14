#ifndef _KINECT_SERVICE_COMMON_H_
#define _KINECT_SERVICE_COMMON_H_

#define KINECT_SERVICE_NAME		"com.dcampus.coolview.kinect"
#define KINECT_OBJECT_PATH		"/com/dcampus/coolview/kinect"

enum KinectGesture
{
	/* �������ֵ����� */
	KG_LeftHandUp,

	/* �������ֵ����� */
	KG_RightHandUp,

	/* �������ֵ���� */
	KG_LeftHandLift,

	/* �������ֵ��ұ� */
	KG_RightHandLift,
};

enum KinectMessageType
{
	KM_Information,		//��ʾ��Ϣ
	KM_Warning,			//������Ϣ
	KM_Error,			//������Ϣ
	KM_Normal,			//kinect��������
};

#endif