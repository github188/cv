#pragma  once

#define CONF_ROOM_SERVICE_NAME			"com.coolview.ui.conferenceroom"
#define CONF_ROOM_SERVICE_OBJECT_PATH	"/com/coolview/ui/conferenceroom"
#define CONF_ROOM_INTERFACE_NAME		"com.coolview.ui.conferenceroom"

enum ConfRoomDisplayModel
{
	CF_DM_1X1,
	CF_DM_2X2,
	CF_DM_3X3,
	CF_DM_L3X3,				//����3X3�Ĳ��֣����Ͻ�Ϊ����
	CF_DM_ALL,
	CF_DM_L4X4,				//����4X4�Ĳ��֣����Ͻ�Ϊ����
	CF_DM_L5X5				//����5X5�Ĳ��֣����Ͻ�Ϊ����
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