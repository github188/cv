#pragma  once

#ifdef CONFROOMCLIENT_EXPORTS    //��Ҫ����Ŀ���������ô�Ԥ������
#define CONF_ROOM_CLIENT_API __declspec(dllexport)
#else
#define CONF_ROOM_CLIENT_API __declspec(dllimport)
#endif

#include <dbus/msdx/common/MsdxServiceCommon.h>
#include <dbus/conferenceRoom/common/ConferenceRoomServiceCommon.h>

class ConferenceRoomIf;
class CONF_ROOM_CLIENT_API ConfRoomDBusClient
{
public:
	static ConfRoomDBusClient* getInstance()
	{
		static ConfRoomDBusClient client;
		return &client;
	}

	/**
	 * @brief ֪ͨ�������ﴴ��һ·����������ָ�����ػ��ԵĴ���λ��
	 * @param sendInfo ������Ϣ
	 * @param seet	����λ�ã����Ϊ�����������
	 */
	void AddSendMedia( SendGraphInfo& sendInfo , const int seet );

	/**
	 * @brief ֪ͨ�����Ҵ���һ·����������ָ����Ƶ���ڴ���λ��
	 * @param sendInfo ������Ϣ
	 * @param seet	����λ�ã����Ϊ�����������
	 */
	void AddRecvMedia( RecvGraphInfo& recvInfo , const int seet );

	/**
	 * @brief ֪ͨ�����ҹر�ĳһ·��Ƶ��
	 * @param userID �û���
	 */
	void CloseMedia( std::string userID );

	/**
	 * @brief ������ʾģʽ
	 */
	void SetDisplayModel( ConfRoomDisplayModel displayModel);

	/**
	 * @brief ��ʾRTCP��Ϣ��
	 */
	void ShowRtcpMessage( const bool show );

	/**
	 * @brief �˳�����
	 */
	void ExitConference( );

	/**
	 * @brief �ر�Coolview�����Ҵ���
	 */
	void CloseConfRoomWindow();

private:
	ConfRoomDBusClient();
	~ConfRoomDBusClient();

private:
	//client proxy
	ConferenceRoomIf*  _conRoomIf;
	
};
