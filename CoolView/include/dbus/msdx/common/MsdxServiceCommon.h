#ifndef MSDX_SERVICE_COMMON_H
#define MSDX_SERVICE_COMMON_H

//#define MSDX_SERVICE_NAME			"com.coolview.msdx.server"
//#define MSDX_SERVICE_OBJECT_PATH	"/com/coolview/msdx/server"

#include <QtCore/QtCore>
#include <QtCore/QDataStream>
#include "util/report/RtpStat.h"

//enum MsdxServiceMethod
//{
//	CreateSendGraph_Method,
//	StopSendGraph_Method,
//	CreateRecvGraph_Method,
//	StopRecvGraphID_Method,
//	StopRecvUserID_Method,
//	SetRecvWnd_Method,
//	SetPreviewWnd_Method,
//	ForbitSpeak_Method,
//	PermitSpeak_Method,
//	ExitConference_Method,
//	ControlMedia_Method,
//	SetDscp_Method
//};


///����Graph��Ҫ��������Ϣ
class NetInfo
{
public:
	QString			ip_addr;		//IP��ַ��v4��v6��
	unsigned short	video_port;		//��Ƶ������Ŀ��˿�
	unsigned short	audio_port;		//��Ƶ������Ŀ��˿�
	unsigned short  screen_port;	//��Ļ��������Ŀ��˿�
	bool			enable_small_video;	//ʹ��С��

#ifndef QT_NO_DATASTREAM
	friend QDataStream& operator>>(QDataStream& in, NetInfo& data)
	{
		in >> data.ip_addr>> data.video_port >> data.audio_port >> data.screen_port >> data.enable_small_video;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, NetInfo& data)
	{
		out << data.ip_addr << data.video_port << data.audio_port << data.screen_port << data.enable_small_video;
		return out;
	}
#endif
};

//end


///����Graph��Ҫ����Ƶ��Ϣ
class VideoInfo
{
public:
	QString			codec_name;		//��/����������
	unsigned short	width;			//��Ƶ֡�Ŀ�ȣ���λΪ����
	unsigned short	height;			//��Ƶ֡�ĸ߶ȣ���λΪ����
	unsigned short	fps;			//��Ƶ��֡�ʣ���λΪ֡/��
	unsigned short	bit_count;		//ÿ��������ռ��λ��
	unsigned short	quality;		//����������ֵ
	//bool			disable_hwcodec;//�ر�Ӳ���� 2013/2/28��
#ifndef QT_NO_DATASTREAM
	friend QDataStream& operator>>(QDataStream& in, VideoInfo& data)
	{
		in >> data.codec_name >> data.width >> data.height >> data.fps >> data.bit_count >> data.quality ;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, VideoInfo& data)
	{
		out << data.codec_name << data.width << data.height << data.fps << data.bit_count << data.quality ;
		return out;
	}
#endif
};

//end


///����Graph��Ҫ����Ƶ��Ϣ
class AudioInfo
{
public:
	QString			codec_name;			//��������
	unsigned short	channels;			//��Ƶ���ŵ���Ŀ
	unsigned short	bits_per_sample;	//������ռ��λ��
	unsigned int	sample_rate;		//��Ƶ������
	bool			enable;				//�Ƿ����/������Ƶ

#ifndef QT_NO_DATASTREAM
	friend QDataStream& operator>>(QDataStream& in, AudioInfo& data)
	{
		in >> data.codec_name >> data.channels >> data.bits_per_sample >> data.sample_rate >> data.enable;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, AudioInfo& data)
	{
		out << data.codec_name << data.channels << data.bits_per_sample << data.sample_rate << data.enable;
		return out;
	}
#endif
};
//end

class UserDevice
{
public:
	QString		video_capture;		//����ͷ����
	QString		video_crossbar;		//crossbar Name������֧�ָ���
	QString		video_crossbar_type;//crossbarInputType,����crossbar����������
	QString		audio_input_device;		//��Ƶ�����豸
    QString		audio_output_device;	//��Ƶ����豸
    int         audio_input_type;    //��Ƶ�����豸����

#ifndef QT_NO_DATASTREAM
	friend QDataStream& operator>>(QDataStream& in, UserDevice& data)
	{
		in >> data.video_capture >> data.video_crossbar >> data.video_crossbar_type 
			>> data.audio_input_device >> data.audio_output_device >> data.audio_input_type;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, UserDevice& data)
	{
		out << data.video_capture << data.video_crossbar << data.video_crossbar_type 
			<< data.audio_input_device << data.audio_output_device << data.audio_input_type;
		return out;
	}
#endif
};

//end

///����Graph��Ҫ��¼����Ϣ
class RecordInfo
{
public:
    QString file_name;
    int job_id;

#ifndef QT_NO_DATASTREAM
    friend QDataStream& operator>>(QDataStream& in, RecordInfo& data)
    {
        in >> data.file_name >> data.job_id;
        return in;
    }
    friend QDataStream& operator<<(QDataStream& out, RecordInfo& data)
    {
        out << data.file_name << data.job_id;
        return out;
    }
#endif
};
//end

class RecvGraphInfo
{
public:
	void initial( const char* AudioInputDevice , const char* AudioOutputDevice, const char * AudioIP,const int& AudioPort,
		const char* AudioCodecMIME , const int& AudioSampleRate , const int& AudioBps , const int& AudioChannels,
		const char * VideoIP,const int& VideoPort,
		const int recvWnd, const int& width, const int& height , const int& fps, const char* userId , const char* userName, const bool& bAutoResync, 
		const char * recFile = "", const int jobId = 0)
	{
		audio_input_device = QString::fromLocal8Bit( AudioInputDevice );	//zhenHua.sun 20110307
		audio_output_device = QString::fromLocal8Bit( AudioOutputDevice );	//zhenHua.sun 20110307

		net_info.ip_addr = QString::fromLocal8Bit( VideoIP );
		net_info.video_port = VideoPort;
		net_info.audio_port = AudioPort;

		recv_wnd = (int)recvWnd;

		user_id = QString::fromLocal8Bit(userId );
		user_name = QString::fromUtf8( userName );

		video_info.codec_name = QString::fromLocal8Bit("H264");
		video_info.bit_count = 32;
		video_info.fps = fps;
		video_info.width = width;
		video_info.height = height;
		video_info.quality = 32;
		audio_info.bits_per_sample = AudioBps;
		audio_info.channels = AudioChannels;
		audio_info.codec_name = QString::fromLocal8Bit(AudioCodecMIME);
		audio_info.sample_rate = AudioSampleRate;
		autoResync = bAutoResync;
		record_info.file_name = QString::fromLocal8Bit(recFile);
		record_info.job_id = jobId;
	}
	int			recv_wnd;				//������Ƶ�Ĳ��Ŵ���
	NetInfo		net_info;
	VideoInfo	video_info;				
	AudioInfo	audio_info;
	QString		user_id;				//�˺�����������������Ϣ����scut_t2
	QString		user_name;				//�û�������ʶ�˺ŵ����

	QString		audio_input_device;		//��Ƶ�����豸�����ڻ�������
	QString 	audio_output_device;	//��Ƶ����豸�����ڻ�������
	bool        autoResync;          //�Ƿ�����ͬ��ƫ���Զ�����

	RecordInfo  record_info;

#ifndef QT_NO_DATASTREAM
	friend QDataStream& operator>>(QDataStream& in, RecvGraphInfo& data)
	{
		in >> data.recv_wnd >> data.net_info >> data.video_info >> data.audio_info >> data.user_id >> data.audio_input_device >>data.audio_output_device 
			>> data.user_name >> data.autoResync >> data.record_info;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, RecvGraphInfo& data)
	{
		out << data.recv_wnd << data.net_info << data.video_info << data.audio_info << data.user_id << data.audio_input_device << data.audio_output_device
			<< data.user_name << data.autoResync << data.record_info;
		return out;
	}
#endif
};


///¼�Ʒֶ���Ϣ
class CutRecordInfo
{
public:
  int job_id;
  QString file_name;

#ifndef QT_NO_DATASTREAM
  friend QDataStream& operator>>(QDataStream& in, CutRecordInfo& data)
  {
    in >> data.file_name >> data.job_id;
    return in;
  }
  friend QDataStream& operator<<(QDataStream& out, CutRecordInfo& data)
  {
    out << data.file_name << data.job_id;
    return out;
  }
#endif
};
//end


class QosDscpInfo
{
public:
	int video_dscp;
	int audio_dscp;
#ifndef QT_NO_DATASTREAM
	friend QDataStream& operator>>(QDataStream& in, QosDscpInfo& data)
	{
		in >> data.video_dscp >> data.audio_dscp;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, QosDscpInfo& data)
	{
		out << data.video_dscp << data.audio_dscp;
		return out;
	}
	//end
#endif
};

//end

class SendGraphInfo
{
public:

	void initial( const int&ScreenPort ,const char* videoCaptrueDevice,const char* audioInputDevice, const char* audioOutputDevice,
        const int audioInputType,
		const char * AudioIP,const int& AudioPort,const char* Audio_Codec,const int& Audio_SampleRate , const int& Audio_BitsPerSample , 
		const int& Audio_Channels, const bool AudioEnable,
		const char * VideoIP,const int& VideoPort, const int&fps , const bool enableSmallVideo,
		const int previewHwnd,const int& width, const int& height,
		const char* crossbarName , const char* crossbarInputType)
	{
		device.video_capture = QString::fromLocal8Bit( videoCaptrueDevice );
		device.video_crossbar = QString::fromLocal8Bit( crossbarName );
		device.video_crossbar_type = QString::fromLocal8Bit( crossbarInputType);

		device.audio_input_device = QString::fromLocal8Bit( audioInputDevice );
		device.audio_output_device = QString::fromLocal8Bit( audioOutputDevice );
        device.audio_input_type = audioInputType;

		net_info.ip_addr = QString::fromLocal8Bit( VideoIP );
		net_info.video_port = VideoPort;
		net_info.audio_port = AudioPort;
		net_info.screen_port = ScreenPort;
		net_info.enable_small_video = enableSmallVideo;

		preview_wnd = (int)previewHwnd;

		video_info.codec_name = QString::fromLocal8Bit("H264");
		video_info.bit_count = 32;
		video_info.fps = fps;
		video_info.width = width;
		video_info.height = height;
		video_info.quality = 32;
		audio_info.bits_per_sample = Audio_BitsPerSample;
		audio_info.channels = Audio_Channels;
		audio_info.codec_name = QString::fromLocal8Bit(Audio_Codec);
		audio_info.sample_rate = Audio_SampleRate;
		audio_info.enable = AudioEnable;
		dscp_info.video_dscp = 0;
		dscp_info.audio_dscp = 0;
	}

	int		preview_wnd;				//���Դ���
	NetInfo		net_info;
	VideoInfo	video_info;				
	AudioInfo	audio_info;
	UserDevice device;
	QosDscpInfo dscp_info;

#ifndef QT_NO_DATASTREAM
	friend QDataStream& operator>>(QDataStream& in, SendGraphInfo& data)
	{
		in >> data.preview_wnd >> data.net_info >> data.video_info >> data.audio_info >> data.device >> data.dscp_info;
		return in; 
	}
	friend QDataStream& operator<<(QDataStream& out, SendGraphInfo& data)
	{
		out << data.preview_wnd << data.net_info << data.video_info << data.audio_info << data.device << data.dscp_info;
		return out;
	}
#endif
};
//end

///�����ļ�����Graph��Ҫ����Ϣ
class FilePlayInfo
{
public:
  QString vuser_id; //���ڱ�ʾ��·��Ƶ�������û�id
  QString vuser_name; //�����˹�ʶ�������
  QString sync_id; //��������Ƶͬ�������ʶ
  long long initial_delay; //ms
  QVector<QString> file_list;
  NetInfo net_info; //���Ͷ˿���Ϣ����ip������Ƶ�˿���Ч

#ifndef QT_NO_DATASTREAM
  friend QDataStream& operator>>(QDataStream& in, FilePlayInfo& data)
  {
    in >> data.vuser_id >> data.vuser_name >> data.sync_id >> data.initial_delay >> data.file_list >> data.net_info;
    return in;
  }
  friend QDataStream& operator<<(QDataStream& out, FilePlayInfo& data)
  {
    out << data.vuser_id << data.vuser_name << data.sync_id << data.initial_delay << data.file_list << data.net_info;
    return out;
  }
#endif
};
//end

class FilePlayCtrlInfo
{
public:
    enum CtrlType {kCtrlPause = 0, kCtrlResume, kCtrlSeek };

    unsigned int op_id;
    unsigned short ctrl_type;
    union {
        long long seek_time; // in ms
    };

#ifndef QT_NO_DATASTREAM
    friend QDataStream& operator>>(QDataStream& in, FilePlayCtrlInfo& data)
    {
        in >> data.op_id >> data.ctrl_type;
        switch (data.ctrl_type)
        {
        case kCtrlSeek:
            in >> data.seek_time;
            break;
        default:
            break;
        }
        return in;
    }
    friend QDataStream& operator<<(QDataStream& out, FilePlayCtrlInfo& data)
    {
        out << data.op_id << data.ctrl_type;
        switch (data.ctrl_type)
        {
        case kCtrlSeek:
            out << data.seek_time;
            break;
        default:
            break;
        }
        return out;
    }
#endif
};

class WndInfo
{
public:

	void initial( int graphID , const char* userID , int wnd , 
		int xpos , int ypos , int width , int height )
	{
		this->graph_id = graphID;
		this->user_id = userID;
		this->video_hwnd = wnd;
		this->point_x = xpos;
		this->point_y = ypos;
		this->width = width;
		this->height = height;
	}


	int			point_x;			//���Ͻ�x����
	int			point_y;			//���Ͻ�y����
	unsigned short	width;				//���ڵĿ�ȣ���λΪ����
	unsigned short	height;				//���ڵĸ߶ȣ���λΪ����
	int				full_screen_flag;	//ȫ����־
	int				graph_id;			//RecvGraph��Ӧ��ID
	int				video_hwnd;			//��Ƶ����
	QString			user_id;			//��Ƶ���ڵ��û���
#ifndef QT_NO_DATASTREAM
	friend QDataStream& operator>>(QDataStream& in, WndInfo& data)
	{
		in >> data.point_x >> data.point_y >> data.width >> data.height >> data.full_screen_flag
			>> data.graph_id >> data.video_hwnd >>data.user_id;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, WndInfo& data)
	{
		out << data.point_x << data.point_y << data.width << data.height << data.full_screen_flag
			<< data.graph_id << data.video_hwnd << data.user_id;
		return out;
	}
	//end
#endif
};

enum MX_MEDIA_TYPE{
	AUDIO_MEDIA = 0,
	VIDEO_MEDIA
};

class MediaControlInfo
{
public:
	int  media_type;			//���Ƶ�ý������,0ΪAudio��1ΪVideo
	int  state;							//���õ�ý��״̬����ǰ֧����ͣ��0Ϊͣ������Ϊ����
#ifndef QT_NO_DATASTREAM
	friend QDataStream& operator>>(QDataStream& in, MediaControlInfo& data)
	{
		in >> data.media_type >> data.state;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, MediaControlInfo& data)
	{
		out << data.media_type << data.state;
		return out;
	}
	//end
#endif
};

class RtpStatItemInfo
{
public:
	uint32_t ssrc;                  /*RTP�Ự��ʶ��һ��ȡ�Է��Ͷ˵�SSRC*/
	uint32_t payload_type;              /*RTP�غ����ͣ���Ӧ�������ͣ�����H264��speex*/
	int media_type;  /*ý�����ͣ�ָ��Ƶ����Ƶ�ȣ���ϸ�ֵ������ʽ*/

	int rtp_endpoint_type;  /*�����߽�ɫ���Դ�ʶ���Ƿ��Ͷ˻���ն˵�ͳ������*/
	QString  rem_addr;  /*RTP�����Ŀ��IP��ַ������Լ��Ƿ����ߣ���Ŀ���ǽ����ߣ���֮��Ȼ. ע�⣬ԭ�����ֶ���sockaddr_storage���ͣ������ǵ�winsockͷ�ļ��汾���������������⣬���Ըĳ��ַ���*/
	int rem_addr_len;           /* IP��ַ����*/
	int rem_port;       /*Զ�������˿�*/
	QString  local_addr;  /*RTP����ı���IP��ַ����ֵһ����ʵ��IP��ַ����Ҳ�п����ǻػ���ַ�����ַ; �鲥����ʱ��ֵ��IP�鲥��ַ*/
	int local_addr_len;         /* ����IP��ַ����*/
	int local_port;       /*���������˿�*/
	bool  isMulticast;	  //�Ƿ��鲥
	QString  mcast_addr;   //�鲥��ַ
	int  mcast_addr_len;          // mcast_addr����


	/*RTP�Ựͳ�Ʋ������ο�ortp��rtp_stats�ṹ*/
	uint64_t packet_sent;    /*number of rtp packet sent*/
	uint64_t sent;      /* bytes sent */
	uint64_t packet_recv;   /* number of packets received */
	uint64_t hw_recv;       /* bytes of payload received */
	uint64_t recv;      /* bytes of payload received and delivered in time to the application */
	uint64_t outoftime;     /* number of packets that were received too late */
	uint64_t cum_packet_loss;   /* cumulative number of packet lost */
	uint64_t discarded;      /* incoming packets discarded because the queue exceeds its max size */

	/* IP������ƣ���λ Kbit/s*/
	float send_bandwidth;       /*���ʹ�����ƣ���λ Kbit/s*/
	float recv_bandwidth;       /*���մ�����ƣ���λ Kbit/s*/

	/*rtcpͳ�Ʋ���*/
	float lost;         /*�����ʣ���λ�ٷֱ�*/
	uint32_t jitter;        /*��������λms*/
	uint32_t delay;                     /*��ʱ����λms*/

	/*������ز���*/
	uint32_t seqnumber;   /* ͳ�Ʊ�������кţ�ÿ��ͳ��һ�Σ������1����0��ʼ��������ֻ��Ծ����RTP�Ự����*/
	uint64_t timestamp;  /* ʱ����������������������ʱ�̣���λΪs���ɲο�Unixʱ���*/
	uint32_t interval;      /*  �ɼ�ʱ��������λms��Ĭ��ֵΪ5000*/

	int graph_id;   /*directshow graph��id�����ʹ��msdx����rtp���䣬��graphid����rtp�������ڵģ�����/���գ�graph��Id�������directshow�޹أ����ֵΪ0*/

	QString user_id;		//�洢�û���

#ifndef QT_NO_DATASTREAM
	friend QDataStream& operator>>(QDataStream& in, RtpStatItemInfo& data)
	{
		in >> data.cum_packet_loss >> data.delay >> data.discarded >> data.graph_id >> data.hw_recv >> data.interval
			>> data.jitter >> data.local_addr >> data.local_addr_len >> data.local_port >>  data.lost >> data.media_type
			>> data.outoftime >> data.packet_recv >> data .packet_sent >> data.payload_type >> data.recv >> data.recv_bandwidth
			>> data.rem_addr >> data.rem_addr_len >> data.rem_port >> data.rtp_endpoint_type >> data.send_bandwidth 
			>> data.sent >> data.seqnumber >> data.ssrc >> data.timestamp >>data.user_id >> data.isMulticast >> data.mcast_addr
			>> data.mcast_addr_len;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, RtpStatItemInfo& data)
	{
		out << data.cum_packet_loss << data.delay << data.discarded << data.graph_id << data.hw_recv << data.interval
			<< data.jitter << data.local_addr << data.local_addr_len << data.local_port << data.lost << data.media_type
			<< data.outoftime << data.packet_recv << data .packet_sent << data.payload_type << data.recv << data.recv_bandwidth
			<< data.rem_addr << data.rem_addr_len << data.rem_port << data.rtp_endpoint_type << data.send_bandwidth 
			<< data.sent << data.seqnumber << data.ssrc << data.timestamp << data.user_id << data.isMulticast << data.mcast_addr 
			<< data.mcast_addr_len;
		return out;
	}
#endif

	static void RtpStatItemInfo2RtpStatItem( const RtpStatItemInfo& info ,RtpStatItem& item )
	{
		item.cum_packet_loss = info.cum_packet_loss;
		item.delay = info.delay;
		item.discarded = info.discarded;
		item.hw_recv = info.hw_recv;
		item.graph_id = info.graph_id;
		item.interval = info.interval;
		item.jitter = info.jitter;
		strncpy( item.local_addr ,  qPrintable(info.local_addr),sizeof( item.local_addr) );
		item.local_addr_len =info.local_addr_len;
		item.local_port = info.local_port;
		item.lost = info.lost;
		item.media_type = static_cast<StreamMediaType>(info.media_type);
		item.outoftime = info.outoftime;
		item.packet_recv = info.packet_recv;
		item.packet_sent = info.packet_sent;
		item.payload_type = info.payload_type;
		item.recv = info.recv;
		item.recv_bandwidth = info.recv_bandwidth;
		strncpy( item.rem_addr ,  qPrintable(info.rem_addr) ,sizeof( item.rem_addr) );
		item.rem_addr_len = info.rem_addr_len;
		item.rem_port = info.rem_port;
		item.rtp_endpoint_type = static_cast<RtpEndpointType>(info.rtp_endpoint_type);
		item.send_bandwidth = info.send_bandwidth;
		item.sent = info.sent;
		item.seqnumber = info.seqnumber;
		item.ssrc = info.ssrc;
		item.timestamp = info.timestamp;
		item.isMulticast = info.isMulticast;
		strncpy( item.mcast_addr ,  qPrintable(info.mcast_addr) ,sizeof( item.mcast_addr) );
		item.mcast_addr_len= info.mcast_addr_len;

	}

	static void RtpStatItem2RtpStatItemInfo( const RtpStatItem& item ,RtpStatItemInfo& info )
	{
		info.graph_id = item.graph_id;
		if(item.rtp_endpoint_type == eET_Sender)
			info.graph_id = 0 ; //���Ͷ˵�graph Id��Ϊ0

		info.cum_packet_loss = item.cum_packet_loss;
		info.delay = item.delay;
		info.discarded = item.discarded;
		info.hw_recv = item.hw_recv;
		info.interval = item.interval;
		info.jitter = item.jitter;
		info.local_addr = item.local_addr;
		info.local_addr_len =item.local_addr_len;
		info.local_port = item.local_port;
		info.lost = item.lost;
		info.media_type = item.media_type;
		info.outoftime = item.outoftime;
		info.packet_recv = item.packet_recv;
		info.packet_sent = item.packet_sent;
		info.payload_type = item.payload_type;
		info.recv = item.recv;
		info.recv_bandwidth = item.recv_bandwidth;
		info.rem_addr = item.rem_addr;
		info.rem_addr_len = item.rem_addr_len;
		info.rem_port = item.rem_port;
		info.rtp_endpoint_type = item.rtp_endpoint_type;
		info.send_bandwidth = item.send_bandwidth;
		info.sent = item.sent;
		info.seqnumber = item.seqnumber;
		info.ssrc = item.ssrc;
		info.timestamp = item.timestamp;
		info.isMulticast = item.isMulticast;
		info.mcast_addr = item.mcast_addr;
		info.mcast_addr_len = item.mcast_addr_len;
	}

};

/*
* @brief ����ͳ�ƣ������Ƕ�����ӵĴ���ͳ��
*/
class TranStatElement
{
public:
    float bandwidth;			 /*������λkb/s*/
    float lost;					/*�����ʣ���λ�ٷֱ�%*/
    uint32_t connectionCount;    /*����������RTP�Ự��*/

#ifndef QT_NO_DATASTREAM
	friend QDataStream& operator>>(QDataStream& in, TranStatElement& data)
	{
		in >> data.bandwidth >> data.lost >> data.connectionCount;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, TranStatElement& data)
	{
		out << data.bandwidth << data.lost << data.connectionCount;
		return out;
	}
#endif

	const TranStatElement& operator= ( const TranStatItem& item )
	{
		bandwidth = item.bandwidth;
		lost = item.lost;
		connectionCount = item.connectionCount;
	}
};

/*
* @brief �ܴ���ͳ�ƣ�ָӦ�ó������д����ͳ�ƽ��
*/
class TranStatInfo
{
public:
	TranStatElement totalStat;                 /*��ͳ��*/
	TranStatElement totalSendStat;         /*�ܷ���ͳ��*/
	TranStatElement totalReceiveStat;      /*�ܽ���ͳ��*/
	TranStatElement videoSendStat;     /*��Ƶ����ͳ��*/
	TranStatElement videoReceiveStat;      /*��Ƶ����ͳ��*/
	TranStatElement audioSendStat;     /*��Ƶ����ͳ��*/
	TranStatElement audioReceiveStat;  /*��Ƶ����ͳ��*/
#ifndef QT_NO_DATASTREAM
	friend QDataStream& operator>>(QDataStream& in, TranStatInfo& data)
	{
		in >> data.audioReceiveStat >> data.audioSendStat >> data.totalReceiveStat >> data.totalSendStat
			>> data.totalStat >> data.videoReceiveStat >> data.videoSendStat;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, TranStatInfo& data)
	{
		out << data.audioReceiveStat << data.audioSendStat << data.totalReceiveStat << data.totalSendStat
			<< data.totalStat << data.videoReceiveStat << data.videoSendStat;
		return out;
	}
#endif

	static void TranStat2TranStatInfo( const TranStat& tranState , TranStatInfo& info )
	{
		info.audioReceiveStat.bandwidth = tranState.audioReceiveStat.bandwidth;
		info.audioReceiveStat.connectionCount = tranState.audioReceiveStat.connectionCount;
		info.audioReceiveStat.lost = tranState.audioReceiveStat.lost;

		info.audioSendStat.bandwidth = tranState.audioSendStat.bandwidth;
		info.audioSendStat.connectionCount = tranState.audioSendStat.connectionCount;
		info.audioSendStat.lost = tranState.audioSendStat.lost;

		info.videoReceiveStat.bandwidth = tranState.videoReceiveStat.bandwidth;
		info.videoReceiveStat.connectionCount = tranState.videoReceiveStat.connectionCount;
		info.videoReceiveStat.lost = tranState.videoReceiveStat.lost;
		info.videoSendStat.bandwidth = tranState.videoSendStat.bandwidth;
		info.videoSendStat.connectionCount = tranState.videoSendStat.connectionCount;
		info.videoSendStat.lost = tranState.videoSendStat.lost;

		info.totalStat.bandwidth = tranState.totalStat.bandwidth;
		info.totalStat.connectionCount = tranState.totalStat.connectionCount;
		info.totalStat.lost = tranState.totalStat.lost;

		info.totalSendStat.bandwidth = tranState.totalSendStat.bandwidth;
		info.totalSendStat.connectionCount = tranState.totalSendStat.connectionCount;
		info.totalSendStat.lost = tranState.totalSendStat.lost;

		info.totalReceiveStat.bandwidth = tranState.totalReceiveStat.bandwidth;
		info.totalReceiveStat.connectionCount = tranState.totalReceiveStat.connectionCount;
		info.totalReceiveStat.lost = tranState.totalReceiveStat.lost;	
	}

	static void TranStatInfo2TranStat( const TranStatInfo& info , TranStat& state )
	{
		state.audioReceiveStat.bandwidth = info.audioReceiveStat.bandwidth;
		state.audioReceiveStat.connectionCount = info.audioReceiveStat.connectionCount;
		state.audioReceiveStat.lost = info.audioReceiveStat.lost;

		state.audioSendStat.bandwidth = info.audioSendStat.bandwidth;
		state.audioSendStat.connectionCount = info.audioSendStat.connectionCount;
		state.audioSendStat.lost = info.audioSendStat.lost;

		state.videoReceiveStat.bandwidth = info.videoReceiveStat.bandwidth;
		state.videoReceiveStat.connectionCount = info.videoReceiveStat.connectionCount;
		state.videoReceiveStat.lost = info.videoReceiveStat.lost;
		state.videoSendStat.bandwidth = info.videoSendStat.bandwidth;
		state.videoSendStat.connectionCount = info.videoSendStat.connectionCount;
		state.videoSendStat.lost = info.videoSendStat.lost;

		state.totalStat.bandwidth = info.totalStat.bandwidth;
		state.totalStat.connectionCount = info.totalStat.connectionCount;
		state.totalStat.lost = info.totalStat.lost;

		state.totalSendStat.bandwidth = info.totalSendStat.bandwidth;
		state.totalSendStat.connectionCount = info.totalSendStat.connectionCount;
		state.totalSendStat.lost = info.totalSendStat.lost;

		state.totalReceiveStat.bandwidth = info.totalReceiveStat.bandwidth;
		state.totalReceiveStat.connectionCount = info.totalReceiveStat.connectionCount;
		state.totalReceiveStat.lost = info.totalReceiveStat.lost;	
	}
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
