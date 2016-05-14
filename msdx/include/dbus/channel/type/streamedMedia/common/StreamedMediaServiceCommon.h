#pragma once

#include <Qtcore/QString>
#include <QtCore/QDataStream>

#define		STREAMED_MEDIA_SERVICE_NAME			"com.dcampus.coolview.channel.type.streamedMedia"
#define		STREAMED_MEDIA_SERVICE_OBJECT_PATH	"/com/dcampus/coolview/channel/type/streamedMedia"

#define STREAMED_MEDIA_PROC_NAME	"CvStreamedMedia"
#define SEND_STREAMED_MEDIA_ID		"send"			//����ý������mediaID
#define SMALL_VIDEO_MEDIA_ID		"smallvideo"	//����С����mediaID
#define LOCAL_RECORD_MEDIA_ID		"localrecord"	//����¼�Ƶ�mediaID
#define LOCAL_PREVIEW_MEDIA_ID		"localpreview"	//���ػ��Ե�mediaID
#define FILE_PLAY_MEDIA_ID		"fileplay"	//�ļ����ŵ�mediaID


//inner functions
//DO NOT call these function outside this file!
inline QString getMediaID( const int virtualIndex, const QString baseID)
{
	if (0 >= virtualIndex)
	{
		return baseID;
	}
	return baseID + QString::number(virtualIndex);
}

//DO NOT call these function outside this file!
inline int getVirtualIndexFromMediaID(const QString id, const QString baseID)
{
	if (id.length() <= baseID.length())
	{
		return 0;
	}
	QString indexStr = id.mid(baseID.length());
	return indexStr.toInt();
}

//DO NOT call these function outside this file!
inline bool isSpecifiedMediaID(const QString id, const QString baseID)
{
	return id.indexOf(baseID) == 0;
}

//send
inline QString getSendMediaID( const int virtualIndex)
{
	return getMediaID(virtualIndex, SEND_STREAMED_MEDIA_ID);
}

inline int getVirtualIndexFromSendMediaID(const QString id)
{
	return getVirtualIndexFromMediaID(id, SEND_STREAMED_MEDIA_ID);
}

inline bool isSendMediaID(const QString id)
{
	return isSpecifiedMediaID(id, SEND_STREAMED_MEDIA_ID);
}

//small video
inline QString getSmallVideoMediaID( const int virtualIndex)
{
	return getMediaID(virtualIndex, SMALL_VIDEO_MEDIA_ID);
}

inline int getVirtualIndexFromSmallMediaID(const QString id)
{
	return getVirtualIndexFromMediaID(id, SMALL_VIDEO_MEDIA_ID);
}

inline bool isSmallVideoMediaID(const QString id)
{
	return isSpecifiedMediaID(id, SMALL_VIDEO_MEDIA_ID);
}

//local record
inline QString getLocalRecordMediaID( const int virtualIndex)
{
	return getMediaID(virtualIndex, LOCAL_RECORD_MEDIA_ID);
}

inline int getVirtualIndexFromLocalRecordMediaID(const QString id)
{
	return getVirtualIndexFromMediaID(id, LOCAL_RECORD_MEDIA_ID);
}

inline bool isLocalRecordMediaID(const QString id)
{
	return isSpecifiedMediaID(id, LOCAL_RECORD_MEDIA_ID);
}

//local preview
inline QString getLocalPreviewMediaID( const int virtualIndex)
{
	return getMediaID(virtualIndex, LOCAL_PREVIEW_MEDIA_ID);
}

inline int getVirtualIndexFromLocalPreviewMediaID(const QString id)
{
	return getVirtualIndexFromMediaID(id, LOCAL_PREVIEW_MEDIA_ID);
}

inline bool isLocalPreviewMediaID(const QString id)
{
	return isSpecifiedMediaID(id, LOCAL_PREVIEW_MEDIA_ID);
}

//file play
inline QString getFilePlayMediaID( const int virtualIndex, const QString &suffix)
{
  //e.g. fileplay0_first
	return FILE_PLAY_MEDIA_ID + QString::number(virtualIndex) + "_" + suffix;
}

inline int getVirtualIndexFromFilePlayMediaID(const QString id)
{
  unsigned long base_len = QString(FILE_PLAY_MEDIA_ID).length();
  if (id.length() <= base_len)
  {
    return 0;
  }
  unsigned long pos = id.indexOf("_");
  QString indexStr = id.mid(base_len, pos);
  return indexStr.toInt();
}

inline bool isFilePlayMediaID(const QString id)
{
	return isSpecifiedMediaID(id, FILE_PLAY_MEDIA_ID);
}

//recv
inline QString getRecvMediaID( const QString& userID )
{
	QString mediaID = QString("Recv_") + userID;

	mediaID.replace("." , "_");
	mediaID.replace("-" , "_");
	mediaID.replace("#" , "_");

	return mediaID;
}

#define userID2MediaID getRecvMediaID //TODO

//util functions
inline QString getStreamedMediaServiceName( const QString& mediaID )
{
	return QString(STREAMED_MEDIA_SERVICE_NAME)+ QString(".") + mediaID;
}

inline QString getStreamedMediaObjectPath( const QString& mediaID )
{
	return QString(STREAMED_MEDIA_SERVICE_OBJECT_PATH)+ QString("/") + mediaID;
}

/************************************************************************/
/* ע�ͣ�����ö�����������ֵ���Ա���ö�ٵ�ֵ���Ŷ���Ĵ���仯���ı䣬 */
/* ��ÿ��ö�ٴӶ�������и��̶���ֵ                                     */
/*  -- by qhf                                                           */
/************************************************************************/

enum StreamedMediaState
{
	StreamedMedia_NoInitial,
	StreamedMedia_Running,
	StreamedMedia_Pause,
	StreamedMedia_Stop,
	StreamedMedia_Destroyed
};


enum StreamedMediaActionID
{
	Action_InitialRecvGraph,
	Action_InitialSendGraph,
	Action_SetVideoWindow,		//����ý�����Ļ��Ի򲥷Ŵ���
	Action_AddSendDst,			//������ý���������Ŀ���ַ
	Action_RemoveSendDst,		//������ý�������Ƴ�Ŀ���ַ
	Action_SetQoSInfo,			//���÷��ͱ��ĵ�dscp
	Action_ControlVideoSend,	//������Ƶ���ķ���
	Action_ControlAudioSend,	//������Ƶ���ķ���
	Action_EnableAutoResync = 9,   //������رս��ն˵��Զ�ͬ��������Action�ṹֵ�Ǹ�boolֵ
	Action_ControlVideoRecv = 10,		//������Ƶ�Ľ��գ�����Ϊboolֵ
	Action_ControlAudioRecv = 11,		//������Ƶ�Ľ��գ�����Ϊboolֵ
	Action_AdaptiveControlCodeRate = 12, //����Ӧ���ʿ��ƣ�������boolֵ
	Action_InitialSmallVideoGraph,
	Action_InitialFilePlayGraph,
	Action_CutRecordingVideo,
    Action_ShowConsoleWindow,
    Action_PlayControl,
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