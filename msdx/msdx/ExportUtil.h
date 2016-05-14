#pragma  once
/************************************************************************/
/* ����Graph API���������п��ܰ���һЩȫ�ֺ���                                */
/************************************************************************/

#ifdef MSDX_EXPORTS  //��Ҫ����Ŀ���������ô�Ԥ������
#define MSDXAPI __declspec(dllexport)
#else
#define MSDXAPI __declspec(dllimport)
#endif

//typedef unsigned char bool; //������ʾ�������ͣ�C�����в�û��bool����

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
	/* ȫ��API����                                                          */
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

	//�ж�ϵͳ�Ƿ�֧��Ӳ������
	bool MSDXAPI msdx_check_hardware_acceleration_support();
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

	int MSDXAPI msdx_create_send_graph( bool enableSmallVideo=false, bool disable_hwcodec=false);

	int MSDXAPI msdx_pause_send_graph();
	int MSDXAPI msdx_resume_send_graph();
	int MSDXAPI msdx_run_send_graph();
	int MSDXAPI msdx_stop_send_graph();
	int MSDXAPI msdx_destroy_send_graph();

	//by dsh 10-07-30�����˲����ʵȲ���
	int MSDXAPI msdx_set_send_audio_codec(const char * codecMIME,const int SampleRate,const int Channels,const int BitsPerSample, const bool aec_enable);
	//by dsh 10-08-06
	void MSDXAPI msdx_set_audio_link_state(bool state);

	int MSDXAPI msdx_set_send_video_vodec(const char * codecMIME,const int width,const int height,const int fps);
	int MSDXAPI msdx_set_send_screen_codec(char * codecMIME);

	/**
	 * ��Ӳ���smallVideo�����Ϊ1������ƵС�������Ϊ0���͸�����Ƶ��......zhenHua.sun 2011-08-10
	 */
	int MSDXAPI msdx_add_send_address(const char * audioIP,const int audioport,const char * videoIP,const int videoport,const bool smallVideo = 0);
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

	//ɾ��RTCP Filter������ĳ���˿�
	int MSDXAPI msdx_delete_Rtcp_Recv_address(char *ListenIP,int ListenPort);

	/**
	 * @author zhenHua.sun 
	 * @date 2010-08-06
	 * @brief �趨��Ƶ�������Ĺ���״̬
	 * @param work ���Ϊtrue�����������������������������
	 * @return 0��ʾ��������
	 */
	int MSDXAPI msdx_control_audio_encoder( bool work );
	/**
	 * @author zhenHua.sun 
	 * @date 2010-08-06
	 * @brief �趨��Ƶ�������Ĺ���״̬
	 * @param work ���Ϊtrue�����������������������������
	 * @return 0��ʾ��������
	 */
	int MSDXAPI msdx_control_video_encoder( bool work );


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
	int MSDXAPI msdx_send_set_video_dscp(const int dscp);

	int MSDXAPI msdx_send_set_audio_dscp(const int dscp);

	//�Ƿ���������Ӧ���ʵ���
	void MSDXAPI msdx_send_enable_adaptive_coderate(bool enable);

	/************************************************************************/
	/* ����Graph API����                                                                     */
	/************************************************************************/

	/*recv graph ȫ�ֿ����ж��recv graph*/
	/*return graph id >= 1*/
	int MSDXAPI msdx_create_recv_graph(const char * userId = "");
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

	/**
	 * ¼����ط���
	 * by Liaokz, 2013-5-8
	 */
	int MSDXAPI msdx_set_record_file_name(int graphid, const char * fileName);
    int MSDXAPI msdx_set_record_auto_cut(int graphid, int sec);
    int MSDXAPI msdx_set_record_cut_by_episode_change(int graphid, bool enable);
	int MSDXAPI msdx_set_record_callback(int graphid, void * obj, void * func);
  int MSDXAPI msdx_set_record_jobid(int graphid, long job_id);
	/**
	 * ¼�ƶ������ָ�
	 * by Liaokz, 2013-8-8
	 */
	int MSDXAPI msdx_record_cut_file(int graphid, long job_id, const char * fileName);
	/**
	 * ���Ͷ�Ҫ��ָ�
	 * by Liaokz, 2013-8-8
	 */
	int MSDXAPI msdx_send_cut_file();

	/**
	 * @brief ������Ƶ�Ľ���
	 * @param enable ���Ϊtrue��ָ���Ƶ�Ľ��գ�����ر���Ƶ�Ľ���
	 * @return ����ɹ��򷵻�0
	 */
	int MSDXAPI msdx_control_audio_recv( const int graphid, const bool enable );

	/**
	 * @brief ������Ƶ�Ľ���
	 * @param enable ���Ϊtrue��ָ���Ƶ�Ľ��գ�����ر���Ƶ�Ľ���
	 * @return ����ɹ��򷵻�0
	 */
	int MSDXAPI msdx_control_video_recv( const int graphid,  const bool enable );

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

	/*�Ƿ����Զ���������Ƶͬ��ƫ�Ŀ���Ǽ�С��ʱ�����ٲ��ţ�ע�⿪���Զ�ͬ�����ܻ�������ݵ���Ƶ������*/
	void MSDXAPI msdx_recv_all_enable_auto_resync(bool enable);
	
	void MSDXAPI msdx_recv_enable_auto_resync(int graphid, bool enable);



	/************************************************************************/
	/* ��������Ƶ����                                                         */
	/************************************************************************/

	enum AudioTestType
	{
		MSDX_LOCAL_ATYPE_DEVICE,
		MSDX_LOCAL_ATYPE_FILE
	};

	int MSDXAPI msdx_create_audio_test_graph(const AudioTestType type, const char * inputSrc, const char * outputDevice);
	void MSDXAPI msdx_destroy_audio_test_graph();

	int MSDXAPI msdx_create_video_test_graph(const char * deviceid, const char*crossbar, const char* cb_input, int width, int height, int fps);
	int MSDXAPI msdx_set_video_test_preview_window(HWND WndHandle, long left, long top, long width, long height);
	int MSDXAPI msdx_run_video_test_graph();
	void MSDXAPI msdx_destroy_video_test_graph();
	/************************************************************************/
	/* smallvideo graph                                                     */
	/************************************************************************/
	int MSDXAPI msdx_create_smallvideo_graph(const char * userId);
    int MSDXAPI msdx_destroy_smallvideo_graphs();
	int MSDXAPI msdx_set_preview_video_codec(char * codecMIME,int width,int height,int fps,int BitCount);
	void MSDXAPI msdx_preview_set_video_rtpstatcallback(void *pClass, void* pRtpStatCallbackFunc );
	int MSDXAPI msdx_set_preview_address(const char * audioIP,const int audioport,const char * videoIP,const int videoport);
	/*int MSDXAPI msdx_set_smallvideo_video_vodec(const char * codecMIME, const int width, const int height);*/
	int MSDXAPI msdx_add_smallvideo_address(const char * audioIP, const int audioport, const char * videoIP, const int videoport,const bool smallVideo );
	int MSDXAPI msdx_connect_smallvideo_graph();
	int MSDXAPI msdx_run_smallvideo_graph();
	int MSDXAPI msdx_set_preview_display_window(HWND WndHandle,long left,long top,long width,long height,int fullScreen);
	int MSDXAPI msdx_get_smallvideo_graph();
	int MSDXAPI msdx_delete_smallvideo_address(const char * audioIP, const int audioport, const char * videoIP, const int videoport);

#ifdef __cplusplus
}

#endif

