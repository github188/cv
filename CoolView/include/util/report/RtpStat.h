////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief RTPͳ�Ʋ�������RTCP���ϡ���ͷ�ļ����ܱ����Projectʹ�ã������Ҫ������������������Ŀ����
/// ͬʱ�������ʵ��Ҳд�ڱ�ͷ�ļ��С�
/// @author qhf
/// @date   2010-07-15
/// modify history:
/// 2011-6-20  ����鲥��ַ�ֶ�
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef  RTP_STAT_H
#define  RTP_STAT_H

#include <stdio.h>
#include <string.h>

#define MAX_IP_STRING_LENGTH 60

//�ض���һЩ���ݽṹ
typedef unsigned char bool_t;
typedef unsigned __int64 uint64_t;
typedef  unsigned int uint32_t;

/************************************************************************/
/*  ����һЩ�غ����ͣ���ֵҪ��RTPInfo.h�ж����RTP�غ�������ȣ��ɸ���
���������ж��Ƿ�����Ƶ���ݡ�
�����ٽ��ж�����Ϊ�˱�������RTPInfo.h�ļ���
���¶��嶼��ϵͳĿǰ֧�ֵ�ý������
*/
/************************************************************************/
#define MEDIA_PT_VIDEO_MP4T     35  //MPEG4��Ƶ����
#define MEDIA_PT_VIDEO_H264     43  //H264��Ƶ���� 
#define MEDIA_PT_AUDIO              50  //H264��Ƶ���� 
#define MEDIA_PT__IMAGE    44                    //������ͼ��


#ifdef __cplusplus
extern "C"
{
#endif

    //RTP�Ự�����߽�ɫ���ͣ������߻������
    enum RtpEndpointType
    {
        eET_Sender = 1,       //������
        eET_Receiver = 2     //������          
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief  ��������ý�����ͣ����漰�����ʽ�������ʾ����Ƶ����Ƶ��������
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum StreamMediaType
    {
        eMT_Video = 0, //��Ƶ
        eMT_Audio = 1, //��Ƶ
        eMT_Other = 2 //��������PPT���װ��		
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief     rtpͳ����������ͣ����գ����ʡ������ʡ���ʱ�������Ȳ�������ֵȡ��RTP�Ựĳ��ʱ�̵�
    ///  ���ա����ṹһ��ȽϹ̶��������׸ı䡣
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    typedef struct _RtpStatItem
    {
        uint32_t ssrc;                  /*RTP�Ự��ʶ��һ��ȡ�Է��Ͷ˵�SSRC*/
        uint32_t payload_type;              /*RTP�غ����ͣ���Ӧ�������ͣ�����H264��speex*/
        enum StreamMediaType media_type;  /*ý�����ͣ�ָ��Ƶ����Ƶ�ȣ���ϸ�ֵ������ʽ*/

        enum RtpEndpointType rtp_endpoint_type;  /*�����߽�ɫ���Դ�ʶ���Ƿ��Ͷ˻���ն˵�ͳ������*/
        char rem_addr[MAX_IP_STRING_LENGTH];  /*RTP�����Ŀ��IP��ַ������Լ��Ƿ����ߣ���Ŀ���ǽ����ߣ���֮��Ȼ. ע�⣬ԭ�����ֶ���sockaddr_storage���ͣ������ǵ�winsockͷ�ļ��汾���������������⣬���Ըĳ��ַ���*/
        int rem_addr_len;           /* IP��ַ����*/
        int rem_port;       /*Զ�������˿�*/
        char local_addr[MAX_IP_STRING_LENGTH];  /*RTP����ı���IP��ַ����ֵһ����ʵ��IP��ַ����Ҳ�п����ǻػ���ַ�����ַ*/
        int local_addr_len;         /* ����IP��ַ����*/
        int local_port;       /*���������˿�*/
		bool_t  isMulticast;	  //�Ƿ��鲥
		char  mcast_addr[MAX_IP_STRING_LENGTH];   //�鲥��ַ
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
        uint64_t timestamp;  /* ʱ����������������������ʱ�̣���λΪms*/
        uint32_t interval;      /*  �ɼ�ʱ��������λms��Ĭ��ֵΪ5000*/

        int graph_id;   /*directshow graph��id�����ʹ��msdx����rtp���䣬��graphid����rtp�������ڵģ�����/���գ�graph��Id�������directshow�޹أ����ֵһ��Ϊ0������Ϊ�ô�*/
		char member_id[MAX_IP_STRING_LENGTH];	/* ý������Ա���˺����ƣ�����Ƿ���ý������Ϊsend*/

    } RtpStatItem;

    //�ص���������Ҫ�Ĳ���
    typedef struct _RtpStatCallbackParam
    {
        void* pClass;
        RtpStatItem* pRtpStat;
    } RtpStatCallbackParam;

    /*
     * @brief ����ͳ�ƣ������Ƕ�����ӵĴ���ͳ��
     */
    typedef struct  _TranStatItem
    {
        float bandwidth;    /*������λkb/s*/
        float lost;             /*�����ʣ���λ�ٷֱ�%*/
        uint32_t connectionCount;       /*����������RTP�Ự��*/
    } TranStatItem;

    /*
     * @brief �ܴ���ͳ�ƣ�ָӦ�ó������д����ͳ�ƽ��
     */
    typedef struct _TranStat
    {
        TranStatItem totalStat;                 /*��ͳ��*/
        TranStatItem totalSendStat;         /*�ܷ���ͳ��*/
        TranStatItem totalReceiveStat;      /*�ܽ���ͳ��*/
        TranStatItem videoSendStat;     /*��Ƶ����ͳ��*/
        TranStatItem videoReceiveStat;      /*��Ƶ����ͳ��*/
        TranStatItem audioSendStat;     /*��Ƶ����ͳ��*/
        TranStatItem audioReceiveStat;  /*��Ƶ����ͳ��*/
    } TranStat;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief  rtpͳ����ص�����ָ�����ͣ����ڴ���ͳ�ƽ��.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef void (*RtpStatCallback) (void* pClass, const RtpStatItem* rtpstat);

	/**
	 * @brief ������ͳ�ƻص�����ָ�����ͣ�
	 *				pClass���ڱ����ϲ㴫��������һ��ָ�룬�²㽫�����޸ģ��׻ظ��ϲ㡣
	 */
	typedef void (*TranStatCallback)(void * pClass, const TranStat * transtat);

	////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief   Rtp stat item initialise.
    /// @param [in,out] rtpstat If non-null, the rtpstat.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    static inline  void rtpStatItem_init(RtpStatItem* rtpstat)
    {
        memset(rtpstat, 0, sizeof(*rtpstat));
        rtpstat->ssrc = 0;
        rtpstat->payload_type = 0;
        rtpstat->media_type = eMT_Video;

        rtpstat->rtp_endpoint_type = eET_Sender;
        //memset(rtpstat->remaddr, 0, sizeof(rtpstat->remaddr));
        rtpstat->rem_addr_len = 0;
        rtpstat->rem_port = 0;
        //memset(rtpstat->localaddr, 0, sizeof(rtpstat->localaddr));
        rtpstat->local_addr_len = 0;
        rtpstat->local_port = 0;
		rtpstat->isMulticast = 0;
		//memset(rtpstat->mcast_addr, 0, sizeof(rtpstat->mcast_addr));
		rtpstat->mcast_addr_len =0;

        rtpstat->packet_sent = 0;
        rtpstat->sent = 0;
        rtpstat->packet_recv = 0;
        rtpstat->hw_recv = 0;
        rtpstat->recv = 0;
        rtpstat->outoftime = 0;
        rtpstat->cum_packet_loss = 0;
        rtpstat->discarded = 0;

        rtpstat->send_bandwidth = 0;
        rtpstat->recv_bandwidth = 0;

        rtpstat->lost = 0;
        rtpstat->jitter = 0;
        rtpstat->delay = 0;

        rtpstat->seqnumber = 0;
        rtpstat->timestamp = 0;
        rtpstat->interval = 5000;

        rtpstat->graph_id = 0;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief  ��������RtpStatItem��ͳ�Ʋ���ֵ����������ssrc��pt��remaddr�ȹ̶�ֵ����Զ��ԣ�
    /// @param [in,out] rtpstat If non-null, the rtpstat.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    static inline  void rtpStatItem_reset_stat(RtpStatItem* rtpstat)
    {
		rtpstat->isMulticast = 0;
		memset(rtpstat->mcast_addr, 0, sizeof(rtpstat->mcast_addr));
		rtpstat->mcast_addr_len =0;

        rtpstat->packet_sent = 0;
        rtpstat->sent = 0;
        rtpstat->packet_recv = 0;
        rtpstat->hw_recv = 0;
        rtpstat->recv = 0;
        rtpstat->outoftime = 0;
        rtpstat->cum_packet_loss = 0;
        rtpstat->discarded = 0;

        rtpstat->send_bandwidth = 0;
        rtpstat->recv_bandwidth = 0;

        rtpstat->lost = 0;
        rtpstat->jitter = 0;
        rtpstat->delay = 0;

        rtpstat->seqnumber = 0;
        rtpstat->timestamp = 0;
    }


    //��ʼ��TranStatItem�ṹ����
    static inline void tranStatItem_init(TranStatItem* item)
    {
        item->bandwidth = 0;
        item->lost = 0;
        item->connectionCount = 0;
    }

    //��ʼ��TranStat�ṹ����
    static inline void tranStat_init(TranStat* stat)
    {
        tranStatItem_init(&stat->totalStat);
        tranStatItem_init(&stat->totalSendStat);
        tranStatItem_init(&stat->totalReceiveStat);
        tranStatItem_init(&stat->videoSendStat);
        tranStatItem_init(&stat->videoReceiveStat);
        tranStatItem_init(&stat->audioSendStat);
        tranStatItem_init(&stat->audioReceiveStat);
    }

    //��ӡRtpStatItem��Ϣ�����������̨
    static void printRtpStat(void* pClass, const RtpStatItem* rtpstat)
    {
        printf("\n====================================\n");
        printf("current rtp stat:\n");
        printf("rtp session ssrc:\t %0x \n", rtpstat->ssrc);
        printf("endpoint type:\t%s\n", rtpstat->rtp_endpoint_type == eET_Sender ? "sender" : "receiver");
        printf("media type:\t%s\n", rtpstat->media_type == eMT_Video ? "video" : "audio");
        printf("payload type:\t %d \n", rtpstat->payload_type);
        printf("remote ip:\t %s:%d \n", rtpstat->rem_addr, rtpstat->rem_port);
        printf("local ip:\t %s:%d \n", rtpstat->local_addr, rtpstat->local_port);
		printf("is multicast transmission:\t %s \n", rtpstat->isMulticast?"true":"false");
		if (rtpstat->isMulticast)
		{
			printf("multicast ip:\t %s \n", rtpstat->mcast_addr);
		}

        printf("send_bandwidth:\t %.2f kb/s\n", rtpstat->send_bandwidth);
        printf("recv_bandwidth:\t %.2f kb/s\n", rtpstat->recv_bandwidth);

        printf("lost:\t %.2f %% \n", rtpstat->lost);
        printf("jitter:\t %d \n", rtpstat->jitter);
        printf("delay:\t %d \n", rtpstat->delay);

        printf("packet_sent:\t %I64d \n", rtpstat->packet_sent);
        printf("sent:\t %I64d bytes\n", rtpstat->sent);
        printf("packet_recv:\t %I64d \n", rtpstat->packet_recv);
        printf("hw_recv:\t %I64d bytes\n", rtpstat->packet_recv);
        printf("recv:\t %I64d bytes\n", rtpstat->recv);
        printf("outoftime:\t %I64d \n", rtpstat->outoftime);
        printf("cum_packet_loss:\t %I64d \n", rtpstat->cum_packet_loss);
        printf("discarded:\t %I64d \n", rtpstat->discarded);

        printf("stat seqnumber:\t %d \n", rtpstat->seqnumber);
        printf("stat timestamp:\t %I64d \n", rtpstat->timestamp);
        printf("stat interval:\t %d \n", rtpstat->interval);

		printf("identifier:\t %d \n", rtpstat->graph_id);

        printf("====================================\n\n");
    }

    //���ͳ����Ϣ��TranStat��������̨
    static void printTranStat(void* pClass, const TranStat* transtat)
    {
        printf("\n*********************Global Transmit Stat*********************\n");
        printf("1��total stat:\n");
        printf("  bandwidth:\t %.2f kb/s \n", transtat->totalStat.bandwidth);
        printf("  lost:\t  %.2f %% \n", transtat->totalStat.lost);
        printf("  connection count:\t %d \n", transtat->totalStat.connectionCount);

        printf("2��total send stat:\n");
        printf("  bandwidth:\t %.2f kb/s \n", transtat->totalSendStat.bandwidth);
        printf("  lost:\t  %.2f %% \n", transtat->totalSendStat.lost);
        printf("  connection count:\t %d \n", transtat->totalSendStat.connectionCount);

        printf("3��total receive stat:\n");
        printf("  bandwidth:\t %.2f kb/s \n", transtat->totalReceiveStat.bandwidth);
        printf("  lost:\t  %.2f %% \n", transtat->totalReceiveStat.lost);
        printf("  connection count:\t %d \n", transtat->totalReceiveStat.connectionCount);

        printf("4��total video send stat:\n");
        printf("  bandwidth:\t %.2f kb/s \n", transtat->videoSendStat.bandwidth);
        printf("  lost:\t  %.2f %% \n", transtat->videoSendStat.lost);
        printf("  connection count:\t %d \n", transtat->videoSendStat.connectionCount);

        printf("5��total video receive stat:\n");
        printf("  bandwidth:\t %.2f kb/s \n", transtat->videoReceiveStat.bandwidth);
        printf("  lost:\t  %.2f %% \n", transtat->videoReceiveStat.lost);
        printf("  connection count:\t %d \n", transtat->videoReceiveStat.connectionCount);

        printf("6��total audio send stat:\n");
        printf("  bandwidth:\t %.2f kb/s \n", transtat->audioSendStat.bandwidth);
        printf("  lost:\t  %.2f %% \n", transtat->audioSendStat.lost);
        printf("  connection count:\t %d \n", transtat->audioSendStat.connectionCount);

        printf("7��total audio receive stat:\n");
        printf("  bandwidth:\t %.2f kb/s \n", transtat->audioReceiveStat.bandwidth);
        printf("  lost:\t  %.2f %% \n", transtat->audioReceiveStat.lost);
        printf("  connection count:\t %d \n", transtat->audioReceiveStat.connectionCount);

        printf("**************************************************************\n\n");
    }

#ifdef __cplusplus
}
#endif

#endif