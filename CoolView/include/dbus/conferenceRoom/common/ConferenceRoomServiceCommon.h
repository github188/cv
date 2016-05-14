#pragma  once

#define CONF_ROOM_SERVICE_NAME			"com.coolview.ui.conferenceroom"
#define CONF_ROOM_SERVICE_OBJECT_PATH	"/com/coolview/ui/conferenceroom"
#define CONF_ROOM_INTERFACE_NAME		"com.coolview.ui.conferenceroom"

//#define LOCAL_PLAY_USERID	"LocalPreview"

#define UiStateTypeAny ""
#define UiStateTypeAudio "audio"
#define UiStateTypeVideo "video"
#define UiStateTypeScreen "screen"
#define UiStateTypeRecord "record"

enum UiMediaState{
	UiMediaState_Initializing,
	UiMediaState_Ready,
	UiMediaState_Run,
	UiMediaState_Stop,
  UiMediaState_Destroy,
  UiMediaState_RecordRun,
  UiMediaState_RecordStop,
  UiMediaState_RecordError, //һ�����
  UiMediaState_RecordSoapError, //Soap���ô���
  UiMediaState_RecordTxError, //Tx���ش���
};

enum RecordCtrlType
{
  RecCtrl_Start,
  RecCtrl_Stop,
};

enum ConfRoomDisplayModel
{
    CF_DM_UNKNOWN = -1,     //δ֪����
	CF_DM_1X1,
	CF_DM_2X2,
	CF_DM_3X3,
	CF_DM_L3X3,				//����3X3�Ĳ��֣����Ͻ�Ϊ����
	CF_DM_ALL,
	CF_DM_L4X4,				//����4X4�Ĳ��֣����Ͻ�Ϊ����
	CF_DM_L5X5,				//����5X5�Ĳ��֣����Ͻ�Ϊ����
	CF_DM_L1_5,				//����3X3�Ĳ��֣����Ͻ�Ϊ����
	CF_DM_L1_20,			//��1_5�Ļ����Ͻ���һ����Ļ֮������д��ڸĳ�4��
	CF_DM_1X1_REAL,			//��Ȼ���õ���1X1���֣����Ǹ�����Ƶ����ʵ�ʴ�С��ʾ��Ƶ����
	CF_DM_DOC,				//�ĵ��Ķ�����
	CF_DM_AUTO,             //�����Զ�����
    CF_DM_TOP_1_2,          //����Ļ���ϰ벿��ʾ1X2�Ĳ���
    CF_DM_4X4,              //����4X4�Ĳ��֣���16·
    CF_DM_AIRPLAY,
};

inline void getPrimaryConferenceRoomService( QString* serviceName , QString* objectPath)
{
	*serviceName = QString(CONF_ROOM_SERVICE_NAME) + ".room0";
	*objectPath = QString(CONF_ROOM_SERVICE_OBJECT_PATH) + "/room0";
}

inline void getSecondConferenceRoomService( QString* serviceName , QString* objectPath)
{
	*serviceName = QString(CONF_ROOM_SERVICE_NAME) + ".room1";
	*objectPath = QString(CONF_ROOM_SERVICE_OBJECT_PATH) + "/room1";
}

enum MediaWindowAction
{
	MWA_ChangeWindowPosition,		//�ƶ�����
	MWA_ChangeMediaState,			//����ý������״̬
	MWA_AddScreenRecv,				//������Ļ��
	MWA_RemoveScreenRecv,			//ֹͣ��Ļ��
	MWA_ChangeMediaSipState,		//����SIP�����״̬
	MWA_ControlScreenShare,			//������Ļ����Ȩ�ޣ�����Ϊtrue�������ˣ�/false���������ˣ���
	MWA_AddMainSpeakerScreenRecv,   //������������Ļ��
	MWA_RemoveSendShareScreen,	    //�Ƴ�������Ļ����
	MWA_PPTControlCommand           //ִ��PPTҳ��仯����
};

#include <dbus/msdx/common/MsdxServiceCommon.h>
class UserMediaHandle
{
public:

	RecvGraphInfo	info;
	int				seet;	//��������λ��
#ifndef QT_NO_DATASTREAM
	friend QDataStream& operator>>(QDataStream& in, UserMediaHandle& data)
	{
		in >> data.info >>data.seet;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, UserMediaHandle& data)
	{
		out <<data.info << data.seet;
		return out;
	}

#endif
};
