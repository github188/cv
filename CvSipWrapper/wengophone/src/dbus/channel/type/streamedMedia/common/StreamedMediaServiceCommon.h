#pragma once

#define		STREAMED_MEDIA_SERVICE_NAME			"com.dcampus.coolview.channel.type.streamedMedia"
#define		STREAMED_MEDIA_SERVICE_OBJECT_PATH	"/com/dcampus/coolview/channel/type/streamedMedia"

#define STREAMED_MEDIA_PROC_NAME	"CvStreamedMedia"
#define SEND_STREAMED_MEDIA_ID		"send"			//����ý������mediaID

inline QString userID2MediaID( const QString& userID )
{
	QString mediaID = QString("Recv_") + userID;

	mediaID.replace("." , "_");
	mediaID.replace("-" , "_");

	return mediaID;
}

inline QString getServiceName( const QString& mediaID )
{
	return QString(STREAMED_MEDIA_SERVICE_NAME)+ QString(".") + mediaID;
}

inline QString getObjectPath( const QString& mediaID )
{
	return QString(STREAMED_MEDIA_SERVICE_OBJECT_PATH)+ QString("/") + mediaID;
}

enum StreamedMediaState
{
	StreamedMedia_NoInitial,
	StreamedMedia_Running,
	StreamedMedia_Pause,
	StreamedMedia_Stop,
	StreamedMedia_Destroyed
};

enum MediaDirection
{
	MediaDirection_Unknow,
	MediaDirection_In,
	MediaDirection_out
};

enum StreamedMediaActionID
{
	Action_InitialRecvGraph,
	Action_InitialSendGraph,
	Action_SetVideoWindow,		//����ý�����Ļ��Ի򲥷Ŵ���
	Action_AddSendDst,			//������ý���������Ŀ���ַ
	Action_RemoveSendDst,		//������ý�������Ƴ�Ŀ���ַ
	Action_SetQoSInfo				//���÷��ͱ��ĵ�dscp
};

enum QoSNotify
{
	QoSNotify_RtpState,			//Rtcp��Ϣ
	QoSNotify_TranState			//�ܵ�������Ϣ
};

enum MediaInfoType
{
	Info_TranState				//����ͳ����Ϣ
};