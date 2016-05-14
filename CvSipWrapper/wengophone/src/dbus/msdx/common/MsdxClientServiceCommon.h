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
#ifndef QT_NO_DATASTREAM
	friend QDataStream& operator>>(QDataStream& in, RtpStatItemInfo& data)
	{
		in >> data.cum_packet_loss >> data.delay >> data.discarded >> data.graph_id >> data.hw_recv >> data.interval
			>> data.jitter >> data.local_addr >> data.local_addr_len >> data.local_port >> data.lost >> data.media_type
			>> data.outoftime >> data.packet_recv >> data .packet_sent >> data.payload_type >> data.recv >> data.recv_bandwidth
			>> data.rem_addr >> data.rem_addr_len >> data.rem_port >> data.rtp_endpoint_type >> data.send_bandwidth 
			>> data.sent >> data.seqnumber >> data.ssrc >> data.timestamp;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, RtpStatItemInfo& data)
	{
		out << data.cum_packet_loss << data.delay << data.discarded << data.graph_id << data.hw_recv << data.interval
			<< data.jitter << data.local_addr << data.local_addr_len << data.local_port << data.lost << data.media_type
			<< data.outoftime << data.packet_recv << data .packet_sent << data.payload_type << data.recv << data.recv_bandwidth
			<< data.rem_addr << data.rem_addr_len << data.rem_port << data.rtp_endpoint_type << data.send_bandwidth 
			<< data.sent << data.seqnumber << data.ssrc << data.timestamp;
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
