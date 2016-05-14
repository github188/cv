#pragma  once

#ifdef CVSTREAMEDMEDIACLIENT_EXPORTS    //��Ҫ����Ŀ���������ô�Ԥ������
#define STREAMED_MEDIA_CLIENT_API __declspec(dllexport)
#else
#define STREAMED_MEDIA_CLIENT_API __declspec(dllimport)
#endif

#include <QtCore/QtCore>
#include <dbus/channel/type/streamedMedia/common/StreamedMediaServiceCommon.h>
#include <dbus/channelDispatcher/common/ChannelDispatcherServiceCommon.h>
#include <dbus/conferenceRoom/common/ConferenceRoomServiceCommon.h>
 
class ChannelDispatcherIf;
class CvStreamedMediaIf;
class ConferenceRoomIf;

class STREAMED_MEDIA_CLIENT_API CvStreamedMediaClient
{
public:
	static CvStreamedMediaClient* getInstance()
	{
		static CvStreamedMediaClient client;
		return &client;
	}

	
	/**
	 * @brief �������������������Graph
	 * @param send_info ������ز���
	 * @return �Ǹ�������������
	 */
	int createSendGraph( const QByteArray& send_info );

	/**
	 * @brief ֹͣ����ָ��IP��ý����
	 * @param net_info ָ���˽�ֹͣ��Ŀ��IP��˿�
	 * @return �Ǹ�������������
	 */
	int stopSend( const QByteArray& net_info);

	/**
	 * @brief �������������������Graph
	 * @param userID ���ڱ�ʾý����
	 * @param recv_info ����ý��������ز���
	 * @return ����ɹ������򷵻�graphID �����򷵻ظ�ֵ
	 */
	int createRecvGraph( const QString& userID , const QByteArray& recv_info );

	/**
	 * @brief ����UserID���ٶ�Ӧ��RecvGraph
	 * @param userID RecvGraph��Ӧ��userID
	 * @return �Ǹ���ʾ��������
	 */
	int stopRecv( const QString& userID );

	/**
	 * @brief �˳����顣����RecvGraph
	 * @return �Ǹ���ʾ��������
	 */
	int exitConference( );

	/**
	 * @brief ����Qos��Ϣ
	 * @param userID �û���
	 * @param qos_info qos��Ϣ
	 * @return �Ǹ���ʾ��������
	 */
	int setQosInfo( const QString& userID , const QByteArray& qos_info );

private:
	CvStreamedMediaClient();
	~CvStreamedMediaClient();


	ChannelDispatcherIf*		_channelDispatcherProxy;
	ConferenceRoomIf*		_confRoomProxy;
};