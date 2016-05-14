#ifndef MSDX_CLIENT_SERVICE_COMMON_H
#define MSDX_CLIENT_SERVICE_COMMON_H

#define MSDX_CLIENT_SERVICE_NAME 			"com.coolview.msdx.client"
#define MSDX_CLIENT_SERVICE_OBJECT_PATH 	"/com/coolview/msdx/client"
#include <QtCore/QtCore>
#include <QtCore/QDataStream>
//�ض���һЩ���ݽṹ
typedef unsigned __int64 uint64_t;
typedef unsigned int uint32_t;

#include <RtpStat.h>

enum MsdxClientServiceMethod
{
	DT_SEND_RTP_STAT_INFO,
	DT_RECV_RTP_STAT_INFO,
	DT_TRAN_STAT_INFO
};


//=================================����ֵ============================

#define		EVENT_R_JOB_DONE				1		//���������
#define		EVENT_R_OK						0
#define		EVENT_R_NO_INITIAL				-1		//�¼�û�б�����
#define		EVENT_R_DATA_UNKNOW				-2		//�����ڴ���������޷�����
#define		EVENT_R_TIME_OUT				-3		//�ȴ����¼���ʱ
#define		EVENT_R_THREAD_ERROR			-4		//�����̳߳��ִ���
#define		EVENT_R_THREAD_TERMINATE_FAIL	-5		//�����߳��˳�ʧ��
#define		EVENT_R_JOB_FAIL				-6		//����ʧ��




#endif
