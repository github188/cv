#pragma  once

#ifdef MSDXCLIENT_EXPORTS
#define MSDX_CLIENT_API _declspec(dllexport)	 //����
#else
#define MSDX_CLIENT_API _declspec(dllimport)   //����
#endif

#ifdef __cplusplus
extern "C"
{
#endif

	/**
	 * @brief ����msdx client�ķ���ӿڡ���Ҫ��main���������
	 */
	int MSDX_CLIENT_API msdx_client_startCallbackService( );

	/**
	 * @brief �ر�MSDX�������
	 * @param waitTime �����ȴ���ʱ�䣬���Ϊ������Զ�ȴ� , Ĭ��ֵΪ500ms
	 * @return �Ǹ���ʾ��������
	 */
	int MSDX_CLIENT_API msdx_client_closeMsdxSvr( const int waitTime );

	/**
	 * @brief ��MSDX����˷��ʹ���SendGraph���¼������ȴ�������
	 * @param audioInputDevice ��Ƶ�����豸
	 * @param audioOutputDevice ��Ƶ����豸
	 * @param AudioIP ��Ƶ���ĵ�Ŀ��IP
	 * @param AudioPort	��Ƶ���ĵ�Ŀ��˿�
	 * @param Audio_SampleRate ��Ƶ������
	 * @param Audio_BitsPerSample ������λ��
	 * @param Audio_Channels ��Ƶ�ĵ�������
	 * @param VideoIP ��Ƶ���ĵ�Ŀ��IP
	 * @param VideoPort ��Ƶ���ĵ�Ŀ��˿�
	 * @param previewHwnd ��Ƶ�طŵĴ���
	 * @param width ��Ƶ�Ŀ�
	 * @param height ��Ƶ�ĸ�
	 * @return ����Ǹ����ʾ�������������ָ���¼���
	 */
	int MSDX_CLIENT_API msdx_client_createSendGraph( const char* videoCaptrueDevice,
		const char* audioInputDevice, const char* audioOutputDevice,
		const char * AudioIP,const int AudioPort, const char* AudioCodec,
		const int Audio_SampleRate , const int Audio_BitsPerSample , const int Audio_Channels,
		const char * VideoIP,const int VideoPort,
		const HWND previewHwnd,const int width, const int height,
		const char* crossbarName , const char* crossbarInputType );

	/**
	 * @brief ��MSDX����˷���ֹͣ��ָ��IP����ý���������ȴ�������
	 * @param AudioIP ��Ƶ���ĵ�Ŀ��IP
	 * @param AudioPort	��Ƶ���ĵ�Ŀ��˿�
	 * @param VideoIP ��Ƶ���ĵ�Ŀ��IP
	 * @param VideoPort ��Ƶ���ĵ�Ŀ��˿�
	  * @return ����Ǹ����ʾ�������������ָ���¼���
	 */
	int MSDX_CLIENT_API msdx_client_stopSend( const char * AudioIP,const int AudioPort,
		const char * VideoIP,const int VideoPort );

	/**
	 * @brief ��MSDX����˷��ʹ���RecvGraph���¼������ȴ�������
	 * @param AudioIP ��Ƶ���ĵ�Ŀ��IP
	 * @param AudioPort	��Ƶ���ĵ�Ŀ��˿�
	 * @param VideoIP ��Ƶ���ĵ�Ŀ��IP
	 * @param VideoPort ��Ƶ���ĵ�Ŀ��˿�
	 * @param previewHwnd ��Ƶ�طŵĴ���
	 * @param width ��Ƶ�Ŀ�
	 * @param height ��Ƶ�ĸ�
	 * @return ����ɹ��򷵻�GraphID �� ���򷵻ظ�����
	 */
	int MSDX_CLIENT_API msdx_client_createRecvGraph( const char * AudioIP,const int AudioPort,
		const char* AudioCodecMIME , const int AudioSampleRate , const int AudioBps , const int AudioChannels,
		const char * VideoIP,const int VideoPort,
		const HWND recvWnd, const int width, const int height );

	/**
	 * @brief ֹͣ������graphID��Ӧ��RecvGraph
	 * @graphID graph id
	 * @return �Ǹ���ʾ��������
	 */
	int MSDX_CLIENT_API msdx_client_stopRecv( const int graphID );

	/**
	 * @brief ��ָ��Ŀ�Ĺ�������
	 * @param ScreenIP Ŀ��IP
	 * @param ScreenPort Ŀ�Ķ˿�
	 * @param x ���Ͻ� x����
	 * @param y ���Ͻ� y����
	 * @param width ������
	 * @param height ����߶�
	 * @return �Ǹ���ʾ��������
	 */
	int MSDX_CLIENT_API msdx_client_startScreenShare( const char* ScreenIP, const int ScreenPort,
		const int x,const int y,const int width,const int height );

	/**
	 * @brief ȡ��ָ��Ŀ�ĵ����湲��
	 * @param ScreenIP Ŀ��IP
	 * @param ScreenPort Ŀ�Ķ˿�
	 * @return �Ǹ���ʾ��������
	 */
	int MSDX_CLIENT_API msdx_client_stopScreenShare( const char* screenIP , const int screenPort );

	/**
	 * @brief ������������RecvGraph
	 * @param ScreenIP Ŀ��IP
	 * @param ScreenPort Ŀ�Ķ˿�
	 * @param remoteHwnd ���Ŵ���
	 * @param videoWidth ���
	 * @param videoHeight �߶�
	 * @return ����ɹ��򷵻�GraphID�����򷵻ظ���
	 */
	int MSDX_CLIENT_API  msdx_client_createShareScreenRecvGraph( const char * ScreenIP, const int ScreenPort, 
		const HWND remoteHwnd, const int videoWidth, const int videoHeight );

	/**
	 * @brief ɾ����������RecvGraph
	 * @param graphID ��Ӧ��graphID
	 * @return �Ǹ���ʾ��������
	 */
	int MSDX_CLIENT_API msdx_client_stopShareScreenRecvGraph( const int graphID );

	/**
	 * @brief ������Ƶ���մ���
	 * @param graphID ��Ҫ������RecvgraphID
	 * @param remoteHwnd ���յĴ��ھ��
	 * @param xpos ���Ͻǵ�x����
	 * @param ypos ���Ͻǵ�y����
	 * @param width ���ڵĿ��
	 * @param height ���ڵĸ߶�
	 * @param fullscreeflag ȫ����־
	 * @return �Ǹ���ʾ��������
	 */
	int MSDX_CLIENT_API msdx_client_setRecvDisplayWindow( const int graphID,const HWND remoteHwnd,
		const long xpos, const long ypos, 
		const int width, const int height, const int fullscreenflag);
		
	/**
	 * @brief ������Ƶ����
	 * @param videoHwnd ���յĴ��ھ��
	 * @param xpos ���Ͻǵ�x����
	 * @param ypos ���Ͻǵ�y����
	 * @param width ���ڵĿ��
	 * @param height ���ڵĸ߶�
	 * @param fullscreeflag ȫ����־
	 * @return �Ǹ���ʾ��������
	 */
	int MSDX_CLIENT_API msdx_client_setDisplayWindow( const HWND videoHwnd,
		const long xpos, const long ypos, 
		const int width, const int height, const int fullscreenflag);

	/**
	 * @brief ֹͣ��ָ���ĵ�ַ������Ƶ��Ϣ
	 * @param info ��Ƶý������������Ϣ
	 * @return �Ǹ���ʾ��������
	 */
	int MSDX_CLIENT_API msdx_client_forbidSpeak( const char* ip , const int port );

	/**
	 * @brief ������ָ���ĵ�ַ������Ƶ��Ϣ
	 * @param info ��Ƶý������������Ϣ
	 * @return �Ǹ���ʾ��������
	 */
	int MSDX_CLIENT_API msdx_client_permitSpeak( const char* ip , const int port );

	/**
	 * @brief �˳����顣����RecvGraph
	 * @return �Ǹ���ʾ��������
	 */
	int MSDX_CLIENT_API msdx_client_exitConference( void );

	/**
	 * @brief ��������Ƶ�ķ���
	 * @param mediaType ý�����ͣ�0Ϊ��Ƶ��1Ϊ��Ƶ
	 * @param state ������Ϣ�����Ϊ0��ֹͣ����������
	 */
	int MSDX_CLIENT_API msdx_client_controlMediaSend( int mediaType , int state );



	/* ���÷���Graph��RTPͳ����Ϣ�ص�������ȫ�ֵģ�
		pRtpStatCallbackFunc��������ΪRtpStatCallback������Ϊ���������㣬ֻ����Ϊvoid*����Ҫ����RtpStatCallback�������͵�ָ�������Ч
		RtpStatCallback�ص������������ɲ鿴RtpStat.h
	*/
	void MSDX_CLIENT_API msdx_client_send_set_rtpstatcallback(void *pClass, void* pRtpStatCallbackFunc);
	void MSDX_CLIENT_API msdx_client_send_set_video_rtpstatcallback(void *pClass, void* pRtpStatCallbackFunc);
	void MSDX_CLIENT_API msdx_client_send_set_audio_rtpstatcallback( void *pClass, void* pRtpStatCallbackFunc );

	/*
		���õ�·����Graph��RTPͳ����Ϣ�ص�����
	*/
	void MSDX_CLIENT_API msdx_client_recv_set_rtpstatcallback(void *pClass, void* pRtpStatCallbackFunc);
	void MSDX_CLIENT_API msdx_client_recv_set_video_rtpstatcallback(const int graphId, void *pClass, void* pRtpStatCallbackFunc);
	void MSDX_CLIENT_API msdx_client_recv_set_audio_rtpstatcallback(const int graphId, void *pClass, void* pRtpStatCallbackFunc );

	/**
	 *	����dscpֵ
	 * @param dscp ��Чֵ��0-63
	 * @return ����0��ʾ���óɹ�
	 */
	int MSDX_CLIENT_API msdx_client_send_set_dscp(const int dscp);

	//���ô���ͳ�ƻص�������pTranStatCallbackFuncҪ�������TranStatCallback���ͣ���μ�RtpStat.h
	void MSDX_CLIENT_API msdx_client_set_transtatcallback(void * pClass, void * pTranStatCallbackFunc);

	
	/**
	*	@brief	��ȡ����ͳ���������������ȡ�������ݲ���ʵʱ�ģ�����ص���ʽ��
	 * @param transtat ����һ��TranStat�ṹָ�������ͳ�ƽ��ֵ�����浽�ñ�����;	TranStat�ṹ�Ķ�����μ�RtpStat.h
	 * @see RtpStat.h
	 */
	void  MSDX_CLIENT_API msdx_client_get_global_transtat(void * transtat);


#ifdef __cplusplus
}
#endif