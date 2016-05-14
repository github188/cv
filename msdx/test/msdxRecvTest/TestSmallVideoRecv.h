#ifndef TEST_SMALL_VIDEO_RECV_H
#define TEST_SMALL_VIDEO_RECV_H

#include "Util.h"
#include <msdx/config.h>
void TestSmallVideoRecv()
{
	//��ʼ��
	int result;
	result = msdx_initial();
	int graphid;
	int videoWidth;
	int videoHeight;
	int fps = 25;

	int choise;
	cout<<"��ѡ����Ƶ����:\n1.����(1280x720)\n2.ȫ����(1920x1080)";
	cin>>choise;

	if( choise==2 )
	{
		videoWidth = 1920;
		videoHeight = 1080;
	}else
	{
		videoWidth = 1280;
		videoHeight = 720;
	}

	const char* localAddress = "127.0.0.1";
	
	//���������Ľ��ܶ�
	graphid = createRecvGraph( videoWidth ,videoHeight , fps, 8500,8600 );
	//CDXutil::AddToRot(graphid, &result);
	getch();


	//���ٽ��ն�
	destroyRecvGraph( graphid );

	//����С���Ľ��ն�
	graphid = createRecvGraph( MSDX_CONF_SMALL_VIDEO_WIDTH, MSDX_CONF_SMALL_VIDEO_HEIGHT, fps/2, 8500,8700 );
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