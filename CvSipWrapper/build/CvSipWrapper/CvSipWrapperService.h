#ifndef CV_SIP_WRAPPER_H
#define CV_SIP_WRAPPER_H

#include <QtCore/QtCore>
#include <dbus/sipwrapper/common/SipWrapperCommonService.h>

#include <sipwrapper/EnumPhoneCallState.h>
#include <sipwrapper/EnumPhoneLineState.h>


class PhApiWrapper;
class SipWrapper;
class CvPerfMonitorIf;
typedef std::map<QString , int>		ConferenceIDMap;
typedef std::map<QString , int>		UserIDMap;

enum SipWrapperState
{
	SIP_IDLE,		//���κ��û���¼
	SIP_LOGIN,		//�û���¼
	SIP_MEETING		//�û����ڿ���
};

class CvSipWrapperService : public QObject
{
	Q_OBJECT

public:
	static CvSipWrapperService* getInstance()
	{
		static CvSipWrapperService  service;
		return &service;
	}

public Q_SLOTS: // METHODS
	void acceptSubscription(int sid);

	/**
	 * @brief ���sip��Ϣ
	 * @param displayName ���ƣ�username@servername
	 * @param username	������������Ϣ�Ĳ���
	 * @param identity	��usernameһ��
	 * @param password
	 * @param realm			��������ser.scut.edu.cn
	 * @param proxyServer	sip����������ǰΪser.scut.edu.cn
	 * @param registerServer ע�����������ǰΪser.scut.edu.cn
	 * @param address_family	IP����
	 */
	void addVirtualLine(const QString &displayName, const QString &username, const QString &identity, const QString &password, const QString &realm, const QString &proxyServer, const QString &registerServer, int address_family);
	void exitConference(const QString &confURI);
	void init(int ipv6_enable);
	bool isInitialized();
	void makeJoinConfInvite(const QString &username, const QString &focusURI, const QString &szContent, const QByteArray &sdpMessage);

	/**
	 * @brief ��Ҫ��ָ���Ļ��鷢���������ֵ�
	 * @sipURI ��Ӧ�˻����focusURI
	 * @szContent ����
	 */
	void makeInfoCall(const QString &sipURI, const QString &szContent);
	void registerVirtualLine(const QString &username, const QString &content);
	void rejectSubscription(int sid);
	void removeVirtualLine(const QString &username, bool force);
	void sendQosPara(const QString &currentUserID, const QString &focusURI, const QString &szContent);
	void sendSipMsg(const QString &currentUserID, const QString &remoteURI, const QString &msgContent);
	void setNatType(int natType);
	void setProxy(const QString &address, int port, const QString &login, const QString &password);
	void setSIP(const QString &server, int serverPort, int localPort);
	void setSipOptions(const QString &optname, const QString &optval);
	void setTunnel(const QString &address, int port, bool ssl);
	void setUaName(const QString &name);
	void startConference(const QString &username, const QString &confURI, const QString &szContent);
	void terminate();


	/************************************************************************/
	/* SIP call��صķ���ӿ�
	 * @param username SIP�˺�����
	 * @param memberURI	Զ��SIP�˺�
	 * @sdpMessage �����Ự�����ý�����
	 */
	/************************************************************************/
	void acceptCall(const QString &username, const QString &memberURI, bool enableVideo, const QByteArray &sdpMessage);
	void closeCall(const QString &username, const QString &memberURI);
	void holdCall(const QString &username, const QString &memberURI);
	void makeCall(const QString &username, const QString &memberURI, const QByteArray &sdpMessage);
	void rejectCall(const QString &username, const QString &memberURI);
	void resumeCall(const QString &username, const QString &memberURI);

Q_SIGNALS: // SIGNALS
	void cvMessageReceivedEvent(const QString &message, const QString &from);
	void incomingSubscribeSignal(int sid, const QString &from, const QString &evtType);
	void phoneCallStateChangedSignal(const QString &callId, int state, const QString &from);
	void phoneLineStateChangedSignal(const QString &lineId, int state);

public Q_SLOTS: // METHODS
	void ExitProcess();
	void Recover(const QByteArray &processImage);

private Q_SLOTS:
	//-----------------------------------���̼�����----------------------------------
	//ע������Ϣ
	void registerProcess( );
	//ע��
	void unregisterProcess();
	//����Hello����
	void sendHelloInfoToMonitor( );

	//---------------------------------�������-----------------------------------------
	void autoJoinConference();

private:
	CvSipWrapperService();
	~CvSipWrapperService();

	void initialSipWrapper();

	void phoneLineStateChangedEventHandler(SipWrapper & sender, int lineId,
		EnumPhoneLineState::PhoneLineState state);

	void phoneCallStateChangedEventHandler(SipWrapper & sender, int callId,
		EnumPhoneCallState::PhoneCallState state, const std::string & from);

	//�������URI���͹�������Ϣ
	void cvMessageReceivedEventHandler(const std::string & message,const std::string & from);

	PhApiWrapper*		_phapiWrapper;

	///��¼conf focus UI��callID�Ķ�Ӧ��ϵ
	ConferenceIDMap		_confMap;

	///��¼��¼�û�����lineID�Ķ�Ӧ��ϵ
	UserIDMap			_userMap;

	//�������̵�״̬��Ϣ���Ա�ָ�
	CvPerfMonitorIf*	_perfMonitorProxy;

	//��¼�����ʱ����������������֮��ʼ��ʱ�������ʱû���յ��ն��б���ô��Ҫ���·�������
	QTimer*				_confInviteTimer;

	SipWrapperState	_state;			//SipWrapper��״̬
	QString			_username;		//�û���
	QString			_registContent;	//ע����Ϣ�а���������
	QString			_realm;			//����
	QString			_userSipURI;
	QString			_password;
	QString			_confURI;		//����URI
	QString			_joinConfContent;	//����_confURI����Ҫ���͸������������SIp��Ϣ��
	QByteArray		_sdpMsg;		//���ظ������������SDP��Ϣ
	int				_ipFamily;		//�������ͣ�IPv4��IPv6
	QTimer*			_helloTimer;		//��ʱ���ؽ��̷���hello����
};


#endif