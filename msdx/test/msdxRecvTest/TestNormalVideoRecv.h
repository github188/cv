#pragma once

#include "Util.h"
void TestNormalVideoRecv()
{
	//��ʼ��
	int result;
	result = msdx_initial();
	int graphid;


	int videoWidth = 320;
	int videoHeight = 240;

	const char* localAddress = "127.0.0.1";

	//���������Ľ��ܶ�
	graphid = createRecvGraph( videoWidth ,videoHeight , 10, 8500,8600 );
	//CDXutil::AddToRot(graphid, &result);
	getch();

	//���ٽ���
	destroyRecvGraph( graphid );

	//����
	msdx_uninitial();
}