#pragma  once
/************************************************************************/
/* ����Graph API���������п��ܰ���һЩȫ�ֺ���                                */
/************************************************************************/

#ifdef MSDX_EXPORTS  //��Ҫ����Ŀ���������ô�Ԥ������
#define MSDXAPI __declspec(dllexport)
#else
#define MSDXAPI __declspec(dllimport)
#endif

typedef unsigned char bool_t; //������ʾ�������ͣ�C�����в�û��bool����

#ifdef __cplusplus
extern "C"
{
#endif

	enum msdxErrors{
		MSDX_ERROR,
		MSDX_OUTOFMEM,
		MSDX_BADARG,
		MSDX_NOVIDEODEVICE,
		MSDX_NOAUDIODEVICE,
		MSDX_GRAPHFAIL,
		MSDX_CONNECTFAIL,
		MSDX_NOFILTER
	};

	/************************************************************************/
	/* ȫ��API����                                                                     */
	/************************************************************************/

	int MSDXAPI testRoute();

	// Formal Interface

	int MSDXAPI msdx_initial(); 	
	void MSDXAPI msdx_uninitial();
	
	void MSDXAPI msdx_set_default_config();
	/*config*/
	void MSDXAPI msdx_config_set_video_device(const char * deviceid,const char *crossbar,const char* cb_input);   //junLin.chen 2010-07-20
	void MSDXAPI msdx_config_set_audio_format(int SampleRate,int Channels,int BitsPerSample);

	///������Ƶ�豸...zhenHua.sun 2010-10-19
	void MSDXAPI msdx_config_set_audio_device(const char* inputDevice , const char* outputDevice );

	/**
	*	@brief	ͳ�ƴ����������
	 * @param transtat ����һ��TranStat�ṹָ�������ͳ�ƽ��ֵ�����浽�ñ�����;	TranStat�ṹ�Ķ�����μ�RtpStat.h
	 * @see RtpStat.h
	 *	@return 0��ʾͳ�Ƴɹ���������ͳ��ʧ�ܻ����rtpware��Ӧ�ĺ���ʧ��
	 */
	int MSDXAPI msdx_compute_global_transtat(void * transtat);

	/************************************************************************/
	/* ����Graph API����                                                                     */
	/************************************************************************/

	/*send Graph ȫ��ֻ��һ��*/

	/*@ return Graph ID:-1 indicate there is no send graph*/
	int MSDXAPI msdx_get_send_graph();
	/*@return Grapg ID: if fail return -1,else return 0*/

	//��ȡ����Graph״̬
	int MSDXAPI msdx_send_get_graph_state();

	int MSDXAPI msdx_create_send_graph();

	int MSDXAPI msdx_pause_send_graph();
	int MSDXAPI msdx_resume_send_graph();
	int MSDXAPI msdx_run_send_graph();
	int MSDXAPI msdx_stop_send_graph();
	int MSDXAPI msdx_destroy_send_graph();

	//by dsh 10-07-30�����˲����ʵȲ���
	int MSDXAPI msdx_set_send_audio_codec(const char * codecMIME,const int SampleRate,const int Channels,const int BitsPerSample);
	//by dsh 10-08-06
	void MSDXAPI msdx_set_audio_link_state(bool_t state);

	int MSDXAPI msdx_set_send_video_vodec(const char * codecMIME,const int width,const int height);
	int MSDXAPI msdx_set_send_screen_codec(char * codecMIME);

	int MSDXAPI msdx_add_send_address(const char * audioIP,const int audioport,const char * videoIP,const int videoport);
	int MSDXAPI msdx_add_send_address2(const char * audioIP,const int audioport,const char * videoIP,const int videoport,const char *ScreenIP,const int ScreenPort);

	int MSDXAPI msdx_delete_send_address(const char * audioIP,const int audioport,const char * videoIP,const int videoport);
	int MSDXAPI msdx_delete_send_address2(const char * audioIP,const int audioport);
	int MSDXAPI msdx_delete_screen_send_address(const char *ScreenIP,const int ScreenPort);
	int MSDXAPI msdx_set_send_preview_window(HWND WndHandle,long left,long top,long width,long height,int fullScreen);
	int MSDXAPI msdx_set_screen_filter_config(int left,int up,int right,int down);
	int MSDXAPI msdx_connect_send_graph();
	//ֱ��������������
	int MSDXAPI msdx_set_send_video_quantizer(int quantizer);
	//���ݵ�ǰ���ʺ�Ŀ�����ʵ�������
	void MSDXAPI msdx_adapt_encode_rate(float currentRate,float destRate);
	//����RTCP Filter������ַ���˿�
	int MSDXAPI msdx_set_Rtcp_Recv_Filter(char *ListenIP,int ListenPort);
	//����Rtcp�ص�����
	int MSDXAPI msdx_set_Rtcp_Callbak_func(void *pClass,void *func);
	//ɾ��RTCP Filter������ĳ���˿�
	int MSDXAPI msdx_delete_Rtcp_Recv_address(char *ListenIP,int ListenPort);

	/**
	 * @author zhenHua.sun 
	 * @date 2010-08-06
	 * @brief �趨��Ƶ�������Ĺ���״̬
	 * @param work ���Ϊtrue�����������������������������
	 * @return 0��ʾ��������
	 */
	int MSDXAPI msdx_control_audio_encoder( bool_t work );
	/**
	 * @author zhenHua.sun 
	 * @date 2010-08-06
	 * @brief �趨��Ƶ�������Ĺ���״̬
	 * @param work ���Ϊtrue�����������������������������
	 * @return 0��ʾ��������
	 */
	int MSDXAPI msdx_control_video_encoder( bool_t work );


	/************************************************************************/
	/* ���÷���Graph��RTPͳ����Ϣ�ص�����
	pRtpStatCallbackFunc��������ΪRtpStatCallback������Ϊ���������㣬ֻ����Ϊvoid*����Ҫ����RtpStatCallback�������͵�ָ�������Ч
	RtpStatCallback�ص������������ɲ鿴RtpStat.h
	*/
	/************************************************************************/
	void MSDXAPI msdx_send_set_video_rtpstatcallback(void *pClass, void* pRtpStatCallbackFunc);
	void MSDXAPI msdx_send_set_audio_rtpstatcallback( void *pClass, void* pRtpStatCallbackFunc );

	
	/**
	 *	���÷��ͳ�ȥ��RTP����dscpֵ
	 * @param dscp ��Чֵ��0-63
	 * @return ����0��ʾ���óɹ�
	 */
	int MSDXAPI msdx_send_set_dscp(const int dscp);



	/************************************************************************/
	/* ����Graph API����                                                                     */
	/************************************************************************/

	/*recv graph ȫ�ֿ����ж��recv graph*/
	/*return graph id >= 1*/
	int MSDXAPI msdx_create_recv_graph();
	int MSDXAPI msdx_pause_recv_graph(int graphid);
	int MSDXAPI msdx_resume_recv_graph(int graphid);
	int MSDXAPI msdx_stop_recv_graph(int graphid);
	int MSDXAPI msdx_destroy_recv_graph(int graphid);
	/**
	* @author zhenHua.sun
	* @date  2010-07-23
	* @brief �������е�RecvGraph
	* @return ����ɹ��򷵻�0
	*/
	int MSDXAPI msdx_destroy_recv_graphs(  );

	int MSDXAPI msdx_set_recv_audio_codec(int graphid,char * codecMIME,int SampleRate,int Channels,int BitsPerSample);
	int MSDXAPI msdx_set_recv_video_codec(int graphid,char * codecMIME,int width,int height,int fps,int BitCount);
	int MSDXAPI msdx_set_recv_screen_codec(int graphid,char * codecMIME,int width,int height,int fps,int BitCount);
	int MSDXAPI msdx_set_recv_address(const int graphid,const char * audioIP,const int audioport,const char * videoIP,const int videoport);
	int MSDXAPI msdx_set_recv_screen_address(int graphid,const char * ScreenIP,const int ScreenPort);
	int MSDXAPI msdx_set_recv_display_window(int graphid,HWND WndHandle,long left,long top,long width,long height,int fullScreen);
	int MSDXAPI msdx_connect_recv_graph(int graphid);
	int MSDXAPI msdx_run_recv_graph(int graphid);
	int MSDXAPI msdx_clear_recv_buf(int graphid);
	/*@return
	*0:initial
	*1:running
	*2:stop
	*3:pause
	**/
	int MSDXAPI msdx_get_graph_state(int graphid);

	/************************************************************************/
	/* ���ý���Graph��RTPͳ����Ϣ�ص�����
	pRtpStatCallbackFunc��������ΪRtpStatCallback������Ϊ���������㣬ֻ����Ϊvoid*����Ҫ����RtpStatCallback�������͵�ָ�������Ч
	RtpStatCallback�ص������������ɲ鿴RtpStat.h
	*/
	/************************************************************************/
	void MSDXAPI msdx_recv_set_video_rtpstatcallback(int graphid, void *pClass, void* pRtpStatCallbackFunc);
	void MSDXAPI msdx_recv_set_audio_rtpstatcallback(int graphid, void *pClass, void* pRtpStatCallbackFunc );

#ifdef __cplusplus
}
#endif

