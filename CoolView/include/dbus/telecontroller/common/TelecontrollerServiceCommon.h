#pragma once

#define TELECONTROLLER_SERVICE_NAME			"com.dcampus.coolview.controller.telecontroller"
#define TELECONTROLLER_SERVICE_OBJECT_PATH	"/com/dcampus/coolview/controller/telecontroller"

#include <QtCore/QString>
#include <QtCore/QDataStream>
#include <QtCore/QStringList>
#include <QtCore/QVector>

inline QString _GetTelecontrollerServiceName(const QString& type )
{
	return QString(TELECONTROLLER_SERVICE_NAME) + "." + type;
}

inline QString _GetTelecontrollerObjectPath( const QString& type )
{
	return QString(TELECONTROLLER_SERVICE_OBJECT_PATH) + "/" + type;
}


enum TelecontrollerInfoIndex
{
	TELE_ExitProcess = 0,
	TELE_Layout = 1,
	TELE_MemberLocation = 2,		//��������UI�������еĲ�����Ϣ���͸�ң������ͨ����ң�����տ�ʼ�������ն˵�ʱ��ʹ��
	TELE_Devices = 3,
	TELE_QueryLayout = 4,
	TELE_QueryMemberLocation = 5,
	TELE_QueryDevices = 6,
	TELE_MediaState = 7,
	TELE_RemoveMemeberLocation = 8,	//�ն��Ƴ����յĳ�Ա
	TELE_AddMemberLocation = 9,		//�ն���ӽ��ճ�Ա
	TELE_ChangeMemberLocation = 10,	//�ն˸��Ľ��ճ�Ա��λ��
	TELE_CurrentController = 11,     //�ն˵�ǰ�Ŀ���Ȩ������
	TELE_ApplyAccess = 12,           //�ǿ���Ȩ�������������Ȩ��
	TELE_ApplyResult = 13,           //���ؿ���Ȩ��������
	TELE_NewNotifyIndex = 14,        //��ң��������ͨ������������
	TELE_PostMeetingInfo = 15,           //�ն˽������ʱ��TeleController���ݵ�ǰ������Ϣ������ϯ�˺�
	TELE_QueryScreenShareState = 16 , //���ն˲�ѯ��Ļ����״̬ 
	TELE_ScreenShareState =17 ,		  //���յ���Ļ����״̬��Ϣ 0->û�п������ع���  1->�Ѿ������˱��ع���
	TELE_TerminalConfig = 18,        //��ң���������ն�������Ϣ
	TELE_PanelState = 19,           //��ң�������ؿ��������Ϣ
};

//��¼ָ����Աý����״̬�ı��
class MediaStateNotify
{
public:
	QString		_memberName;		//�û�����������������SIP�˺���
	QString		_mediaType;			//ý�����ͣ�Ϊaudio��video
	QString		_mediaState;		//ý��״̬��Ϊstop��run
	friend QDataStream& operator>>(QDataStream& in, MediaStateNotify& data)
	{
		in >> data._memberName >> data._mediaType >> data._mediaState;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, MediaStateNotify& data)
	{
		out << data._memberName << data._mediaType << data._mediaState;
		return out;
	}
};

//��¼�����Ա�ڻ�����UI���λ����Ϣ
class MemberLocation
{
public:
	QString		_memberName;
	int			_screenIndex;
	int			_seet;
	bool		_audioEnable;
	bool		_videoEnable;

	friend QDataStream& operator>>(QDataStream& in, MemberLocation& data)
	{
		in >> data._memberName >> data._screenIndex >> data._seet >> data._audioEnable >> data._videoEnable;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, MemberLocation& data)
	{
		out << data._memberName << data._screenIndex << data._seet << data._audioEnable << data._videoEnable;
		return out;
	}
};

//��¼ÿ����Ļ����Ϣ��Ŀǰ��Ҫ�в���
class ScreenInfo
{
public:
	QString		_layout;			//"1"=1X1,"2"=2X2,"3"=3X3,"L1_5"=L(1+5),"L1_20"=L(1+20)
	int			_screenIndex;
	friend QDataStream& operator>>(QDataStream& in, ScreenInfo& data)
	{
		in >> data._screenIndex >> data._layout;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, ScreenInfo& data)
	{
		out << data._screenIndex << data._layout;
		return out;
	}
};

class CrossBarDevice
{
public:
	QString		_crossbarDevice;
	QStringList	_crossbarTypeList;
	friend QDataStream& operator >> (QDataStream& in , CrossBarDevice&data )
	{
		in >> data._crossbarDevice >> data._crossbarTypeList;
		return in;
	}

	friend QDataStream& operator << (QDataStream& out , const CrossBarDevice&data )
	{
		out << data._crossbarDevice << data._crossbarTypeList;
		return out;
	}
};

class VideoCaptureDevice
{
public:
	QString _captureDevice;
	QStringList _mediaTypeList;			
	friend QDataStream& operator >> (QDataStream& in , VideoCaptureDevice&data )
	{
		in >> data._captureDevice >> data._mediaTypeList;
		return in;
	}

	friend QDataStream& operator << (QDataStream& out , const VideoCaptureDevice&data )
	{
		out << data._captureDevice << data._mediaTypeList;
		return out;
	}
};

class VideoDevice
{
public:
	QVector<VideoCaptureDevice>	_videoCaptureDevices;
	QVector<CrossBarDevice>		_crossbarDevices;
	int			_videoQuality;			//��Ƶ������������ֵ
	
	friend QDataStream& operator >> ( QDataStream &in, VideoDevice& data)
	{
		int videoCaptureNum;
		in >> videoCaptureNum;
		for( int i=0 ; i<videoCaptureNum ; i++ )
		{
			VideoCaptureDevice device;
			in >> device;
			data._videoCaptureDevices.push_back(device);
		}

		int crossbarNum;
		in >> crossbarNum;
		for( int i=0 ; i<crossbarNum; i++ )
		{
			CrossBarDevice device;
			in >> device;
			data._crossbarDevices.push_back(device);
		}

		in >> data._videoQuality;

		return in;
	}

	friend QDataStream& operator << ( QDataStream &out, const VideoDevice& data)
	{
		int videoCaptureNum = data._videoCaptureDevices.size();
		out << videoCaptureNum;
		for( int i=0 ; i<videoCaptureNum ; i++ )
		{
			out << data._videoCaptureDevices.at(i);
		}

		int crossbarNum = data._crossbarDevices.size();
		out << crossbarNum;
		for( int i=0 ; i<crossbarNum ; i++ )
		{
			out << data._crossbarDevices.at(i);
		}

		out << data._videoQuality;

		return out;
	}

};

class AudioDevice
{
public:
	QStringList		_audioInputDeviceList;
	int				_audioInputVolumn;
	QStringList		_audioOutputDeviceList;
	int				_audioOutputVolumn;
	friend QDataStream& operator >> ( QDataStream &in, AudioDevice& data)
	{
		in >> data._audioInputDeviceList >> data._audioInputVolumn >> data._audioOutputDeviceList >> data._audioOutputVolumn;
		return in;
	}
	friend QDataStream& operator << ( QDataStream &out, const AudioDevice& data)
	{
		out << data._audioInputDeviceList << data._audioInputVolumn << data._audioOutputDeviceList << data._audioOutputVolumn;
		return out;
	}
};

class TerminalDevice
{
public:
	VideoDevice		_videoDevice;
	CrossBarDevice	_crossbarDevice;
	AudioDevice		_audioDevice;
	friend QDataStream&	operator >> (QDataStream& in , TerminalDevice& data )
	{
		in >> data._videoDevice >> data._crossbarDevice >> data._audioDevice;
		return in;
	}
	friend QDataStream& operator << (QDataStream&out , const TerminalDevice& data )
	{
		out << data._videoDevice << data._crossbarDevice << data._audioDevice;
		return out;
	}
};