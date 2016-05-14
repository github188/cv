#ifndef CONFERENCE_MANAGER_H
#define	CONFERENCE_MANAGER_H

#include <QtCore/QtCore>
#include <QTimer>
#include <vector>
using namespace std;
#include "CVMsgParser.h"
#include "util/ini/CVIniConfig.h"

#include <QtCore/QVariantMap>
#include <QtCore/QList>
#include "../profile/RunningProfile.h"
#include <util/report/RtpStat.h>
#include <util/udp/UdpSender.h>

TerminalInfo* FindTerminal( const TerminalVector& terminalList , const string& uri );
void TransformConferenceForQt( const ConfInfo& conf , QVariantMap* confMap );
void TransformConfListForQt( const ConferenceVector& confList , QList<QVariantMap>* confMapList );
void TransformMemberForQt( const MemberInfo& member , QVariantMap* memberMap );
void TransformMemberListForQt( const MemberVector& memberList , QList<QVariantMap>* memberMapList );
void TransformTerminalForQt( const TerminalInfo&terminal , QVariantMap* terminalMap );
void TransformTerminalListForQt( const TerminalVector& terminalList , QList<QVariantMap>* terminalMapList );

typedef struct _RecStatItem RecStatItem;
typedef std::map<const std::string, TerminalVector> TerminalMap;

//��¼��ǰ����״̬...zhenHua.sun 2010-09-08
enum ConferenceState
{
	NotInConference,    //û�м����κλ���
	IsInConference,     //������һ�����飬���ڻỰ��
	ExitingConference,  //�û������˳�����
	WaitingPermission,  //���ڵȴ����Ʒ���������ɣ�δ�������
	JoinRejected,               //������鱻�ܾ�
};


class CvSipWrapperIf;
class CvPerfMonitorIf;
class TxMonitorIf;

class HttpWrapper;

class ConferenceManager : public QObject
{
	Q_OBJECT
public:
    enum JoinConferenceError {
      kOk = 0,
      kNeedPassword = -1,
      kInAConference = -2,
      kIsLeavingConference = -3,
      kConferenceNotExisted = -4,
      kJoinRejected = -5,
      kUnknown = -10,
    };

public:
	ConferenceManager( );
	~ConferenceManager();

	///����ַ�Ƿ�֧���鲥�����Ϊ�ջ���0.0.0.0��֧���鲥������Ĭ��֧���鲥
	bool checkSupportMulticast( const std::string& ipaddress );

	///��ȡ��ǰ������鲥��ַ
	bool getCurrentMulticastAddress( string* ipaddress );

	bool checkIsInConference();

	//�жϵ�ǰ�����Ƿ��鲥����
	bool checkCurrentIsMulticastConference();

	//��֤һ����ַ�Ƿ�֧���鲥��������Ϊ�ջ�0.0.0.0��֧���鲥��������ַ��Ĭ�Ͽ�����֧���鲥
	static bool check_support_multicast(const string& ipaddress);

	bool hostOrNot();

	/************************************************************************/
	/* �������
	*/
	/************************************************************************/

	/**
	 * @brief ��ʼʹ�û����նˣ�ͬʱ��û����б�
	 */
	void startConference( const std::string& username , const std::string& sipServer );

	 /**
     * @brief ���ɼ���ָ���������Ϣ�����͸����������
     * @param index �����б��Ӧ��������
     * @param ����URI
     * @return �ɹ��򷵻�0����������ֵ���ʾ����ʧ�ܣ�
							-1  ��Ҫ����
							-2	 �û��Ѿ�������һ�����飬���ڿ����У������Զ��л����飩
							-3  �û������˳�����
							-4  �������󣬱���uri
							-5	 ���Ʒ�������������룬�����Ǵ���ԭ��
							����	 δ֪ԭ��
     */
    int makeJoinConfInvite(const std::string& focus_URI);


	/**
	 * @brief ����ָ�����飬ͨ������QoS������
	 */
	int joinConference(const string& curFocusURI);

	/**
	 * @brief �ܾ�����ָ�����飬ͨ������QoS����
	 */
	int rejectToJoinConference( const string& curFocusURI, const string& reason );

	/**
	 * @brief ��ɲ��ֻ��鹦��
	 */
	void makeInfoCall( const std::string& fousURI , const std::string& szContent );

	/**
	 * @brief ����QoS��Ϣ��QoS������
	 */
	void sendRTPStatToQoSServer(const RtpStatItem& rtpstat);

	/**
	 * @brief ����¼����Ϣ�������� - ��δʵ�ַ���
	 */
	void sendRecStatToServer(const RecStatItem& recstat);

	/**
	 * @brief ����û��ľ�������ֹ���
	 * @param handup ���Ϊtrue��֪ͨ�������û������ˣ�����Ϊfalse
	 * @return NULL
	 */
	void MemberHandup( const std::string memberURI , const bool handup );

	///**
	// * @brief ��ָ���û�����ý����
	// * @param localAddr ý������Ŀ���ַ
	// * @param remoteMemberUniformURI Զ�̳�Ա��URI����ʽΪsip:name@realm
	// */
	//void askForStreamMedia( const std::string localAddr , const std::string remoteMemberUniURI  );

	///**
	// * @brief ��֪Զ���û�ֹͣ������Ƶ��������
	// * @param localAddr ý������Ŀ���ַ
	// * @param remoteMemberUniformURI Զ�̳�Ա��URI����ʽΪsip:name@realm
	// */
	//void stopRemoteStreamMedia( const std::string localAddr , const std::string remoteMemberUniURI );

	/**
	 * @brief ��ָ���û�����ý����
	 * @param userName �û���
	 * @param remoteMemberURI Զ�̳�Ա��URI����ʽΪname@realm;�򱾵ؽ��̵�mediaID
	 * @param screenIndex ��Ļѡ��
	 * @param seet ��Ƶ��������λ�ã�1��ʾ��һ�����ڣ��Դ�����
	 */
	void addRecvStreamMedia( const std::string username , const std::string remoteMemberURI
		, const int screenIndex, const int seet);

	/**
	 * @brief �������ػ��Ժͱ���¼�ƽ���. ע��:������������mediaID��Ϊ��ײ㽻���ı�ʶ��,����virtualURI
	 */
	void addRecvLocalMedia( const std::string localMediaID, const int screenIndex, const int seet, bool smallvideo = false);


	/**
	 * @brief ��֪Զ���û�ֹͣ������Ƶ��������
	 * @param userName �û���
	 * @param remoteMemberURI Զ�̳�Ա��URI����ʽΪname@realm
	 */
	void removeRecvStreamMedia( const std::string username, const std::string remoteMemberURI );

	/**
	 * @brief ֹͣ�������Ľ���. ע��:������������mediaID��Ϊ��ײ㽻���ı�ʶ��,����virtualURI
	 */
	void removeRecvLocalMedia( const std::string localMediaID );

	/**
	 * @brief �ı�������ʾλ��
	 */
	void changeMemberSeat( const std::string remoteMemberURI , const int screenIndex , const int seat );

	/**
	 * @brief ����û�����Ϣ����
	 * @param username ��Ա����
	 * @param message ��Ϣ
	 */
	void sendTextMessage( const std::string& membername , const std::string& message );

	void exitConference( );

	std::string createJoinConferenceMsg(const string& confUri);

	//���ò�����QoS��������Ϣ
	void saveQoSServerInfo(const MsgQoSServerInfo & serverInfo);

	/**
	 * @brief �������ط��ͽ���
	 */
	void createSendStreamMedia();

	/**
	 * @brief ��������ĳ�ʼ��������������鲥���飬���͵��鲥��ַ
	 */
	void initSendStreamMedia();

	/**
	 * @brief �˳���������÷��ͽ���
	 */
	void resetSendStreamMedia();

	/**
	 * @brief ��֪���вλ��ն˵�ý�巢��״̬�������ж�ý�������п���
	 * @param type ý�����ͣ�0Ϊ��Ƶ��1Ϊ��Ƶ
	 * @param enable �������ý��������Ϊtrue������Ϊfalse
	 */
	void controlSendMedia( const int&type , const bool enable );

	/**
	 * @brief ��������ý����
	 */
	void createSendScreenMedia( const QRect wnd );

	/**
	 * @brief ��������ý����
	 */
	void createRecvScreenMedia( const QString& memberURI, const int screenIndex );

	/**
	 * @brief ��֪Զ���û�ֹͣ������Ļ��
	 */
	void removeRecvScreenMedia( const QString& memberURI , const int screenIndex);

	/**
	 * @brief ��ȡ��ǰ�Ļ���ģʽ
	 * @return �����᷵��report,���ۻ᷵��discuss�����û�н�����鷵�ؿ��ַ���
	 */
	string GetCurrentConferenceMode() const { return _confMode; }

	/**
	 * @brief ��ǰ�ն��Ƿ�������
	 * @return ������򷵻�true�����򷵻�false
	 */
	bool isSpeaker() const
	{
		if( _speakerURI==RunningProfile::getInstance()->user_uri() )
		{
			return true;
		}else
		{
			return false;
		}
	}

	//��ǰ̨���յ�handUp/handDown��Ϣ
	void HandUpHandler(const bool);
	
	string getConfMode();

	//��ǰ̨���յ���������Ϣ
	void AllowSpeakHandler(QString uri);

	//��ǰ̨���յ���ֹ������Ϣ
	void ForbidSpeakHandler(QString uri);

	//��ǰ̨���յ��������������ʵԴ��ַ��֤��Ϣ
	void TerminalLogin(const std::string& username , const std::string& sipServer,QString ipv4,QString ipv6,QString myVersion,int screenNum);  

	//��ǰ̨���յ���Ҫ�����������ShareScreenControl�������Ϣ
	void ScreenShareControlHandler(const bool enable);
	
	//֪ͨң����������ϯ
	void ChairmanChangeNotify(bool enable);

	//�������ʱ�����������ȡ�ն��б�
	void GetTermialList();
	
	//���ն˻�ȡ�������ն��б�����������ѯ������صĻ�����Ϣ�����������ˣ�������Ļ�����ߣ�
	void QueryCoferenceInfoBeforeFinishTerminal();

	void RecoveryMediaProcess(QString mediaType,bool isSend,QString removeUserID);

public slots:
	//����������Ͳ�����Ϣ
	void StartServerTsetSlot();

	//��Ƶ�ֶΣ��Ӵ�ʱ��ʼһ���µ���ƵƬ�Σ���Ҫ������tx���ͷּ�ָ��
	void SegmentLocalMedia();

private:
	int computeRequiredBandwidth();

	void sendUdpMessageToQosServer( char * msg, int msgLenth );

	// fot tx get record path
	QString getRecordPath(QString terminalName, const int virtualIndex);

public slots:
	/************************************************************************/
	/* Sip��Ϣ����
	*/
	/************************************************************************/
	void cvMessageReceivedSlot(const QString &message, const QString &from);
	
	//�յ�TerminalConf����SIP��Ϣ������ն����� by Patrick
	void recvTerminalConfModeMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//�յ�ConfList����SIP��Ϣ�� ��ȡ�����б�
	void recvConfListMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�LoginResult����SIP��Ϣ���������
	void recvLoginResultCommandMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�MemberList����SIP��Ϣ�����³�Ա�б�
	void recvMemberListCommandMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�LoginNotify����SIP��Ϣ�����˼������
	void recvLoginNotifyMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�LogoutNotify����SIP��Ϣ�������˳�����
	void recvLogoutNotifyMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�Invite����SIP��Ϣ���ܵ�����
	void recvInviteMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�ApplyConfResult����SIP��Ϣ�����뼴ʱ����ɹ�
	void recvApplyConfResultMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�PasswordIncorrect����SIP��Ϣ�����벻��ȷ
	void recvPasswordIncorrectMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�Text����SIP��Ϣ���յ��ı���Ϣ
	void recvTextMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�LoginResult����SIP��Ϣ�����߳�����
	void recvKickedMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�WhiteBoard����SIP��Ϣ���յ��װ���Ϣ
	void recvWhiteBoardMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�HandUp����SIP��Ϣ�����ַ���
	void recvHandUpMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�HandDown����SIP��Ϣ��ȡ������
	void recvHandDownMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�Receiver����SIP��Ϣ������ý����
	//void recvReceiverMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�ScreenReceiver����SIP��Ϣ������ý����
	void recvScreenReceiverMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�LoginResult����SIP��Ϣ������һ·��Ļ������
	void recvUnShareScreenMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�Cancel2Receiver����SIP��Ϣ����������һ·��Ļ������
	//void recvCancel2ReceiverMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�ScreenCancel2Receiver����SIP��Ϣ��ȡ����Ļ����
	void recvScreenCancel2ReceiverMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�QoS����SIP��Ϣ��qos�������
	void recvQoSMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ��ն��б�
	void recvTerminalListCommandMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri );
	void RecvTerminalListSlot(TerminalVector);

	//------------------������ý������������----------------zhenHua.sun 2010-12-23
	//���յ�FocusURI�Խ���ý�����Ļظ�
	void recvStartMediaReply(CVMsgParser& cvMessageParser , const std::string& fromUri );

	//���յ�FocusURI����ý������Ҫ��
	void recvStartSendMediaCommand(CVMsgParser& cvMessageParser , const std::string& fromUri );

	//���յ�FocusURI��ֹͣý�����Ļظ�
	void recvStopMediaReply(CVMsgParser& cvMessageParser , const std::string& fromUri );


	//���յ�FocusURIֹͣý������Ҫ��
	void recvStopSendMediaCommand(CVMsgParser& cvMessageParser , const std::string& fromUri );

	//���յ�FocusURI���ض��ն�ý��������״̬��ͨ��
	void recvSendMediaControlCommand( CVMsgParser& cvMessageParser , const std::string& fromUri );

	//-------------------�����˿�������----------------------zhenHua.sun 2011-08-17
	///��ѯ�����˵�URI
	void queryMainSpeaker( );

	//��ѯ��ϯ�ն˵�URI
	void queryChairmanTermianl();

	//��ѯ�ĵ������ն˵�URI
	void queryCurrentScreenShareTermianl();

	///���յ�ѡ�������˵���Ϣ
	void recvPermitSpeakInfo( CVMsgParser& cvMessageParser , const std::string& fromUri );

	///���յ�ȡ�������˵���Ϣ
	void recvForbidSpeakInfo( CVMsgParser& cvMessageParser, const std::string& fromUri );

	//ͨ��URIȡ�������� add by zhongbao.liang
	void forbidSpeakByURI(const std::string& Uri);
	
	///���յ���ʵԴ��ַ��֤��Ϣ
	void recvSaviInfo( CVMsgParser& cvMessageParser, const std::string& fromUri );

	//���յ���ϯ�ն���Ϣ
	void recvChairmanTerminalHandler( CVMsgParser& cvMessageParser, const std::string& fromUri);
	
	//���յ�ý�������Ϣ
	void recvMediaControlInfo( CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ���������Ϣ
	void recvMainSpeakerTermianlInfo( CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ���ϯ�ն���Ϣ
	void recvChairmanTermianlInfo( CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ���ǰ�����ĵ������ն���Ϣ
	void recvCurrentScreenShareTerminalInfo( CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ��򿪻�ر���������Ļ������Ϣ
	void recvShareScreenControlInfo( CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���������˵���Ļ����
	void recvMainSpeakerShareScreen(QString & Uri);

	//���յ���������Online��Ϣ
	void recvOnlineMessageInfo( CVMsgParser& cvMessageParser, const std::string& fromUri);
	
	
	/**
	 * @brief ����uri���³�Ա�б�����ն��б��״̬��Ϣ
	 */
	void _SipAccountLogout( const std::string& sipURI );

	///@brief ��������������������Ϣ
	void onlineInfoSlot();
	
	void TerminalLogOutSlot();
	
	//���͵��������Ĳ�����Ϣ���ڳ�ʱǰû���յ��ظ�
	void severTestTimeOutSlot();

	//�ڹ涨��ʱ�������ӣ��ڣ�û���յ��������������ظ��������ж��ն�������������ӳ�������
	void serverHeartbeatMessageTimeOutSlot();
	

Q_SIGNALS:
	void cvConfListReceivedSignal(  const ConferenceVector& confVector );
	void cvMemberListReceivedSignal( const MemberVector& memberList, const std::string& confTitle);
	void cvTerminalListReceivedSignal( const TerminalVector& terminalList , const std::string& confURI);

	void cvMemberHandUpSignal(QString memberURI );
	void cvMemberHandDownSignal(QString memberURI );

	//��֪ǰ̨������ն����������˱仯
	void cvTermianlCountChangeNotify(int TotalTermianlNum);

    void cvJoinConferenceSuccessSignal(const std::string &confURI);

	///�û��������
	void cvMemberLoginSignal( const MemberInfo& member );
	void cvTerminalLoginSignal( const TerminalVector & member );
	///��֤�������ܾ��û��������
	void cvJoinRejectionSignal( const std::string& confURI , const std::string& reason );
	///�û��˳�����
    void cvTerminalLogoutSignal(QString sip);

	///�����������ֹ����
	void cvEndConferenceSignal( );

	void cvConfrenceInviteSignal( const ConfInfo* conference , const std::string&from );
	void cvPasswordIncorrectSignal();
	void cvKickedSignal(); 
	void cvTextMsgReceivedSignal( const std::string& message , const std::string& from );
	void cvWhiteBoardMsgReceivedSignal( const std::string& message , const std::string& from );
	void cvPermitSpeakSignal();
	//
	void cvHostUnShareScreenSignal();

	///Զ���û���ý��������״̬���������RT��ʶremote terminal
	void cvRTSendMediaStateChanged( const std::string&memberURI , const std::string&type , const bool enable );
	
	///Զ���û����ձ��ն�
	void cvRemoteTerminalAskForMediaSignal( const std::string&terminalURI , const std::string&terminalName);

	void RecvSaviSignal(bool res);

	void cvSetMemberWindowToFirstPositionSignal(QString);

	void cvUpgradeTerminalInfoSignal(const TerminalInfo& terminal);

	void cvModifyStopMyAudioState(bool);

	void cvServerTestResultSignal(bool isTimeOut,int interval);


	/////Զ���û���������Ƶ
	//void cvAskForStreamMediaSignal( const std::string& remoteIP , const int&audioPort , const int&videoPort );

	/////Զ���û���ֹ������Ƶ
	//void cvStopStreamMediaSignal( const std::string& remoteIP , const int&audioPort , const int&videoPort  );

	//void cvSetSendDSCPSignal( const int& dscp );
	
	//Event <void (UserProfile&sender, int callId, HWND&_previewHwnd)> setCallIdEvent;

	//void confRoomListReceivedSignal( const QList<QVariantMap>& confList );


	void receiveRtpStatSignal(const RtpStatItem& rtpstat);
	void receiveRecStatSignal(const RecStatItem &recstat);
	void streamMediaReceiveStartedSignal(QString uri);
	void streamMediaReceiveStoppedSignal(QString uri);

private:
	///�û���Ϣ
	CvSipWrapperIf*		_sipWrapperProxy;

	//TOFO:HTTP
	HttpWrapper * _httpWrapper;

	// for Tx
	TxMonitorIf * _txMonitor;

	// for Tx
	QString			_recordingPath;

	//�����б�
	ConferenceVector _confList;

	//��ǰ������Ϣ
	ConfInfo* _currentConfInfo;     //��ǰ������Ϣָ�룬ֻ�����ڿ�����(_confState= IsInConference)��ָ�����Ч������ʱ��Ϊ��

	//��ǰ����URI���������ڽ��룬���ڿ����С����ܾ����롢�����˳���״̬
	string _focusURI;

	//����״̬
	ConferenceState _confState;

	//�����Ա�б�
	MemberVector	_memberList;

	//�ն��б�
	TerminalMap	_terminalList;

	//�����Զ���
	list<MemberInfo*>_requestForSpeakList;
	//���Զ���
	list<MemberInfo*>_speakingList;

	//��¼���͵ĵ�ַ
	ReceiverInfoManager _receiverInfo;

	//������URI
	string _hostURI;

	//������URI
	string _speakerURI;
	
	//��ǰ�ĵ������ն�URI
	string _currentScreenShareUri;

	//��ϯ�ն�URI
	string _TerminalChairmanURI;
	
	//�������ģʽ
	string _confMode;

	//����title
	string _confTitle;

	//qos������uri
	string _qosServerUri;

	//������Ϣ��ʱ����һ��ʱ������������������������Ϣ
	QTimer*	_onlineInfoTimer;

	//qos���淢����
	UdpSender _qosReportSender;

	//��¼��ǰ���������ն˵�����
	int _totalTermianlNum;
	
	///������������Լ�ʱ��
	QTimer*	_serverTestTimer;
	
	//������������ʱ��ʱ��
	QTimer*	_serverHeartbeatTimer;
	

	QTime _serverTestCount;


	bool _isServerTest;

	// Tx����¼���Զ��ָ��ʱ��
	QTimer  _localVideoAutoCutTimer;

};

inline bool ConferenceManager::checkSupportMulticast( const std::string& ipaddress )
{
	return ipaddress.size()>0 && ipaddress!="0.0.0.0";
}

inline bool ConferenceManager::checkIsInConference()
{
	if( _confState == IsInConference )
	{
		return true;
	}else
	{
		return false;
	}
}

inline bool ConferenceManager::check_support_multicast( const string& ipaddress )
{
	return ipaddress.length() > 0 && ipaddress != "0.0.0.0";
}

inline bool ConferenceManager::hostOrNot()
{
	if(_confMode == "host" && RunningProfile::getInstance()->username() == _hostURI)
	{
		return true;
	}
	return false;
}



#endif