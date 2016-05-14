#ifndef TEST_FULLHD_SMALL_VIDEO_RECV_H
#define TEST_FULLHD_SMALL_VIDEO_RECV_H

#include "Util.h"

void TestFullHDSmallVideoRecv()
{
	//��ʼ��
	int result;
	result = msdx_initial();
	int graphid;


	int videoWidth = 1920;
	int videoHeight = 1080;
	int fps = 25;

	const char* localAddress = "127.0.0.1";

	//���������Ľ��ܶ�
	graphid = createRecvGraph( videoWidth ,videoHeight , fps, 8500,8600 );
	//CDXutil::AddToRot(graphid, &result);
	getch();


	//���ٽ��ն�
	destroyRecvGraph( graphid );

	//����С���Ľ��ն�
	graphid = createRecvGraph( 320, 180 , fps/2, 8500,8700 );
	getch();

	//���ٽ��ն�
	destroyRecvGraph( graphid );

	//���½��մ���
	graphid = createRecvGraph( videoWidth , videoHeight , fps, 8500, 8600 );
	getch();

	//���ٽ���
	destroyRecvGraph( graphid );

	//����
	msdx_uninitial();
}

#endif