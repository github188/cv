/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007 Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef OWUSERPROFILE_H
#define OWUSERPROFILE_H

#include "Profile.h"
#include "IMAccountManager.h"

#include <imwrapper/EnumPresenceState.h>

#include <model/account/wengo/WengoAccount.h>
#include <model/history/History.h>
#include <model/phoneline/IPhoneLine.h>
#include <model/webservices/callforward/WsCallForward.h>
#include <model/conference/CVMsgParser.h>

#include <thread/Condition.h>
#include <thread/RecursiveMutex.h>

#include <util/Event.h>
#include <util/String.h>
#include <util/Trackable.h>

#include <list>
#include <vector>
#include <Windows.h>

class ChatHandler;
class CoIpManager;
class ConnectHandler;
class Contact;
class ContactList;
class History;
class IMAccount;
class IMAccountManager;
class IMContactListHandler;
class IPhoneLine;
class NetworkObserver;
class PhoneCall;
class PresenceHandler;
class WenboxPlugin;
class WsInfo;
class WsSms;
class WsSoftUpdate;

//��¼��ǰ����״̬...zhenHua.sun 2010-09-08
enum ConferenceState
{
    NotInConference,        //û�м����κλ���
    IsInConference,     //������һ�����飬���ڻỰ��
    ExitingConference,   //�û������˳�����
    WaitingPermission,  //���ڵȴ����Ʒ���������ɣ�δ�������
    JoinRejected,               //������鱻�ܾ�
};

/**
 * Handle the profile of a User.
 *
 * @author Philippe Bernery
 */
class UserProfile : public Profile, public Trackable
{
    friend class UserProfileFileStorage;
    friend class UserProfileFileStorage1;
    friend class UserProfileXMLSerializer;
    friend class Connect;
public:
	Event<void ()> ExitMeetingSuccessEvent;//Add By LZY 2010-09-30 �˳�����ɹ�ʱ����֪ͨ�¼�
    /**
     * Login procedure is done, event with the procedure result.
     *
     * @see SipAccount::loginStateChangedEvent
     */
    Event<void (SipAccount&sender, EnumSipLoginState::SipLoginState state)> loginStateChangedEvent;

    /**
     * Network event while trying to connect a SipAccount.
     *
     * @see SipAccount::networkDiscoveryStateChangedEvent
     */
    Event<void (SipAccount&sender, SipAccount::NetworkDiscoveryState state)> networkDiscoveryStateChangedEvent;

    /**
     * Emitted when SipAccount has been connected
     */
    Event<void ()> sipAccountConnectedEvent;

    /**
     * Emitted when SipAccount failed to connect
     */
    Event<void (EnumSipLoginState::SipLoginState)> sipAccountConnectionFailedEvent;

    /**
     * The history has been loaded.
     *
     * @param sender this class
     * @param History History loaded
     */
    Event<void (UserProfile&sender, History&history)> historyLoadedEvent;

    /**
     * A PhoneLine has been created.
     *
     * @param sender this class
     * @param phoneLine PhoneLine created
     */
    Event<void (UserProfile&sender, IPhoneLine&phoneLine)> phoneLineCreatedEvent;

    /**
     * Sms has been created.
     *
     * @param sender this class
     * @param sms Sms created
     */
    Event<void (UserProfile&sender, WsSms&sms)> wsSmsCreatedEvent;

    /**
     * SoftUpdate has been created.
     *
     * @param sender this class
     * @param sms Sms created
     */
    Event<void (UserProfile&sender, WsSoftUpdate&softUpdate)> wsSoftUpdateCreatedEvent;

    /**
     * WsInfo has been created.
     *
     * @param sender this class
     * @param wsInfo WsInfo created
     */
    Event<void (UserProfile&sender, WsInfo&wsInfo)> wsInfoCreatedEvent;

    /**
     * WsCallForward has been created.
     *
     * @param sender this class
     * @param wsCallForward WsCallForward created
     */
    Event<void (UserProfile&sender, WsCallForward&wsCallForward)> wsCallForwardCreatedEvent;

    UserProfile();

    virtual ~UserProfile();

    /**
     * Initializes the UserProfile.
     */
    void init();

    /**
     * Return the profile directory
     * @return profile directory.
     */
    std::string getProfileDirectory() const;

    /**
     * Connects all this UserProfile accounts (SipAccount and IMAccount) set
     * to be automatically connected.
     */
    void connect();

    /**
     * Disconnects all accounts.
     */
    void disconnect();

    /** List of IPhoneLine. */
    typedef List <IPhoneLine*> PhoneLines;

    /**
     * Gets the list of PhoneLine.
     *
     * Only one PhoneLine is active at a time.
     *
     * @return the list of PhoneLine
     */
    const PhoneLines& getPhoneLineList() const
    {
        return _phoneLineList;
    }

    /**
     * Gets the active/current PhoneLine.
     *
     * Only one PhoneLine is active at a time
     *
     * @return the active PhoneLine
     */
    IPhoneLine* getActivePhoneLine() const
    {
        return _activePhoneLine;
    }

    /**
     * Gets the active phone call.
     *
     * Used for playing DTMF.
     *
     * @return active phone call or NULL
     */
    PhoneCall* getActivePhoneCall() const;

    /** Gets the ConnectHandler reference. */
    ConnectHandler& getConnectHandler()
    {
        return *_connectHandler;
    }

    /** Gets the PresenceHandler reference. */
    PresenceHandler& getPresenceHandler()
    {
        return *_presenceHandler;
    }

    /** Gets the ChatHandler reference. */
    ChatHandler& getChatHandler()
    {
        return *_chatHandler;
    }

    /**
     * Gets the IMAccountManager reference. Here we can safely derefence
     * the pointer as the IMAccountHandler is constructed in
     * the UserProfile constructor.
     */
    IMAccountManager& getIMAccountManager()
    {
        return *_imAccountManager;
    }

    /** Gets the IMContactListHandler reference. */
    IMContactListHandler& getIMContactListHandler()
    {
        return *_imContactListHandler;
    }

    /** Gets the ContactList reference. */
    ContactList& getContactList()
    {
        return *_contactList;
    }

    /**
     * Gets the History reference. Here we can safely derefence the pointer
     * as the History is constructed in the UserProfile constructor.
     */
    History& getHistory()
    {
        return *_history;
    }

    /** Gets the WenboxPlugin pointer. */
    WenboxPlugin* getWenboxPlugin()
    {
        return _wenboxPlugin;
    }

    /** Gets the WsInfo pointer. */
    WsInfo* getWsInfo()
    {
        return _wsInfo;
    }

    /** Gets the WengoPhone update WebService pointer. */
    WsSoftUpdate* getWsSoftUpdate()
    {
        return _wsSoftUpdate;
    }

    /** Gets the SMS WebService pointer. */
    WsSms* getWsSms()
    {
        return _wsSms;
    }

    /** Gets the WsCallForward WebService pointer. */
    WsCallForward* getWsCallForward()
    {
        return _wsCallForward;
    }

    /** Gets the CoIpManager. */
    CoIpManager& getCoIpManager()
    {
        return *_coIpManager;
    }

    /** True if history has been loaded. */
    bool isHistoryLoaded() const
    {
        return _historyLoaded;
    }

    /**
     * Loads the history.
     */
    bool loadHistory(const std::string& path);

    /**
     * Saves the history.
     */
    bool saveHistory(const std::string& path);

    /**
     * Sets the SipAccount of this UserProfile.
     *
     * The SipAccount os copied internally.
     *
     * @param SipAccount the SipAccount to set
     */
    void setSipAccount(const SipAccount& sipAccount, bool needInitialization = true);

    /**
     * Return true if a WengoAccount is active.
     *
     * @return true if a WengoAccount is active
     */
    bool hasWengoAccount() const;

    /**
     * Return true if a SipAccount is active.
     *
     * @return true if a SipAccount is active
     */
    bool hasSipAccount() const;

    /**
     * Gets the SipAccount.
     *
     * @return the SipAccount
     */
    SipAccount* getSipAccount() const
    {
        return _sipAccount;
    }

    /**
     * Gets the WengoAccount.
     *
     * @return the WengoAccount
     */
    WengoAccount* getWengoAccount() const
    {
        if(hasWengoAccount())
        {
            return dynamic_cast<WengoAccount*>(_sipAccount);
        }
        else
        {
            return NULL;
        }
    }

    /**
     * Adds an IMAccount to this UserProfile.
     * This method should currently not be called to add a Wengo
     * IMAccount. A Wengo IMAccount is created internally when setWengoAccount
     * is called. There is an assert to prevent that.
     *
     * The IMAccount is copied internally.
     *
     * @param imAccount the IMAccount to add
     */
    void addIMAccount(const IMAccount& imAccount);

    /**
     * Removes an IMAccount from this UserProfile.
     *
     * An IMAccount of type Wengo must currently not be removed by this method.
     * setWengoAccount must be used for this purpose.
     * There is an assert to prevent that.
     *
     * @param imAccount the IMAccount to remove
     */
    void removeIMAccount(const IMAccount& imAccount);

    /**
     * Updates an IMAccount from this UserProfile.
     *
     * An IMAccount of type Wengo must currently not be removed by this method.
     * setWengoAccount must be used for this purpose.
     * There is an assert to prevent that.
     *
     * @param imAccount the IMAccount to remove
     */
    void updateIMAccount(const IMAccount& imAccount);

    /**
     * @see IPhoneLine::makeCall()
     */
    EnumMakeCallError::MakeCallError makeCall(Contact& contact);

    /**
     * @see IPhoneLine::makeCall()
     */
    EnumMakeCallError::MakeCallError makeCall(const std::string& phoneNumber);

    /**
     * Start a instant messaging with a Contact.
     *
     * @param contact the Contact to talk to
     */
    void startIM(Contact& contact);

    /**
     * Changes alias of this user.
     *
     * @param alias the alias to set
     * @param imAccount the IMAccount to apply the alias to;
     *        pass NULL to set the alias to all IMAccount
     */
    void setAlias(const std::string& alias, IMAccount* imAccount);

    /**
     * Changes icon of this user.
     *
     * @param icon the icon to set
     * @param imAccount the IMAccount to apply the icon to
     *        pass NULL to set the icon to all IMAccount
     */
    void setIcon(const OWPicture& icon, IMAccount* account);

    /**
     * Gets the PresenceState of this UserProfile.
     */
    EnumPresenceState::PresenceState getPresenceState() const;

    /**
     * Checks if at least one IMAccount is in connected state.
     *
     * @return true if at least 1 IMAccount is connected; false otherwise
     */
    bool isConnected() const;

    /**
     * Changes the PresenceState of an IMAccount.
     *
     * @param presenceState the PresenceState to set
     * @param imAccountId the id of the IMAccount to apply the PresenceState.
     */
    void setPresenceState(EnumPresenceState::PresenceState presenceState, std::string imAccountId);

    /**
     * Changes the PresenceState of all IMAccount.
     *
     * @param presenceState the PresenceState to set
     */
    void setAllPresenceState(EnumPresenceState::PresenceState presenceState);

    /**
     * Gets the name of this UserProfile.
     *
     * The name is computed from the SipAccount. "Default" is returned if no
     * SipAccount is set. There should be only one Default UserProfile at
     * a time.
     */
    std::string getName() const
    {
        return _name;
    }

    /************************************************************************/
    /*Start Conference*/
    Event <void (UserProfile&sender, vector<ConfInfo*> _confList)> cvConfListReceivedEvent;
    Event <void (UserProfile&sender, vector<MemberInfo*> _memberList, std::string confName)> cvMemberListReceivedEvent;
    Event <void (UserProfile&sender, ConfInfo*conference, const std::string&from)> cvConfrenceInviteEvent;
    Event <void (UserProfile&sender)> cvPasswordIncorrectEvent;
    Event <void (UserProfile&sender)> cvKickedEvent;
    Event <void (UserProfile&sender, const std::string&Message, const std::string&from)> cvTextMsgReceivedEvent;
    Event <void (UserProfile&sender, const std::string&Message, const std::string&from)> cvWhiteBoardMsgReceivedEvent;
    Event <void (UserProfile&sender)> cvPermitSpeakEvent;
    //
    Event <void (UserProfile&sender)> cvHostUnShareScreenEvent;

    Event <void (UserProfile&sender, int callId, HWND&_previewHwnd)> setCallIdEvent;

	//�ܾ���������¼�������uri���ܾ�ԭ��
	Event<void (UserProfile&sender, const string& confFocusURI, const string & reason)> cvJoinRejectionEvent;

    void setCallIdEventHandler(int callId);
    void startConference(const std::string& confURI, const std::string& szContent);
    void exitConference(int& callId);

    /**
     * @brief ����ָ������
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
    int makeJoinConfInvite(const int& index);
    int makeJoinConfInvite(const std::string& focus_URI);

    int media_send_start(char* AudioIP, int AudioPort, char* VideoIP, int VideoPort, HWND previewHwnd);
    void media_send_stop(char* AudioIP, int AudioPort, char* VideoIP, int VideoPort);
    int media_recv_start(string userId, char* AudioIP, int AudioPort, char* VideoIP, int VideoPort, char* codecMIME, HWND remoteHwnd, int videoWidth, int videoHeight);

    /**
     * @author zhenHua.sun
     * @date 2010-08-03
     * @brief ������media_recv_start��������������Ƶ��������ͣ���Ƶ����Ƶ�ʣ�����λ����������
     */
    int media_recv_start(string userId,
                         char* AudioIP,
                         int AudioPort,
                         const char* audioCodecMIME,
                         const int audioSampleRate,
                         const int audioBps,
                         const int audioChannels,
                         char* VideoIP,
                         int VideoPort,
                         char* codecMIME,
                         HWND remoteHwnd,
                         int videoWidth,
                         int videoHeight);


    int media_recv_start(int index, HWND remoteHwnd);
    void media_recv_stop(std::string userId);
    //��Ļ����
    int screen_share_send_start(char* ScreenIP, int ScreenPort, int x, int y, int width, int height);
    //ȡ�����湲��
    void screen_share_send_stop(char* ScreenIP, int ScreenPort);
    //�������湲��
    int screen_share_recv_start(char* ScreenIP, int ScreenPort, HWND remoteHwnd, int videoWidth, int videoHeight);
    //ȡ���������湲��
    void screen_share_recv_stop();
    void exit_conference();
    //���ý��հ�ť��״̬
    bool setRecvButtonStatus(int index);

    /* zhenHua.sun 2010-08-03
    //��SDP��ö˿ڡ�ý�����͵Ȳ���
    void getMediaParamFromSdp(int &aport,int &vport,char *mediaType,int &width,int &height,const char *sdp);
    */

    /**
     * @author zhenHua.sun
     * @date 2010-08-03
     * @brief ��SDP�л�ȡý������
     */
    void UserProfile::getMediaParamFromSdp(int& aport,
                                           int& vport,
                                           char* videoCodecMIME,
                                           int& width,
                                           int& height,
                                           char* audioCodecMIME,
                                           int& audioSampleRate,
                                           int& audioBitsPerSample,
                                           int& audioChannels,
                                           const char* sdp);

    //���ñ���Ԥ�����ھ��
    void setPreviewWin(HWND& Hwnd);

    Event <void (UserProfile&sender, const std::string&userId)> logoutReceivedEvent;

    //video����resize
    void resizeVideoWindow(const std::string& userId, const HWND& remoteHwnd, int fullscreenflag);

    Event <void (UserProfile&sender, const RtpStatItem&rtpstat)> rtpStatRecvEvent;
    void rtpStatRecvEventHandler(const RtpStatItem& rtpstat);

    Event <void (const TranStat&transtat)> tranStatRecvEvent;
    void tranStatRecvEventHandler(const TranStat& transtat);

    //������ĳ·����Ƶ
    void unReceiveMedia(const std::string& userId);
    //��ֹ����
    void forbidSpeak();
    void forbidSpeak(string userId);
    //������
    void permitSpeak();

    //������룬��ȷ����Ϣ��Focus�������򷵻�
    int checkPassword(int& row, std::string& password);

    //��鵱ǰ�û��Ƿ�������
    bool hostOrNot();

    //�����Ա����״̬���Ƿ����ڷ���
    int memberSpeakStatus(int row);

    //ָ��������
    int setSpeaker(int row, bool permitOrForbid, int callId);

    //����Info��Ϣ
    void makeInfoCall(int callId, char* szContent);

    //ɾ�������
    int KickOut(int row, int callId);

    //�����ı���Ϣ
    void sendTextMsg(const int& receiverIndex, const std::string& szContent);
    //���ַ���
    int handForSpeak(int row, bool handUpOrDown, int callId);


    //ȡ����������еĵ�һ����
    string getFirstMember(bool permitOrForbid);

    /**
    * Add by: hexianfu
    * for the telecontroller
    get the latest confinfo and memberinfo
    * 2009-10-29
    */
    vector<ConfInfo*>& getCurrentConfInfoList();

    vector<MemberInfo*>& getCurrentMemberInfoList();



    //��������Ƶ����ͣ...zhenHua.sun 2010-08-06
    /**
     * @brief ������Ƶ�ķ���
     * @param state �����true����������Ƶ�ķ��ͣ�����ֹͣ��Ƶ�ķ���
     */
    void controlAudioSend(bool state);
    /**
     * @brief ������Ƶ�ķ���
     * @param state �����true����������Ƶ�ķ��ͣ�����ֹͣ��Ƶ�ķ���
     */
    void controlVideoSend(bool state);

    /*��������*/

    //�ı�DSCPֵ
    void setSendDSCP(int dscp);

    //��ȡ��ǰ�û���sip�˺ŵ�uri
    string getCurrentUserSipUri();

	//��ȡ��ǰ��¼�û����û�����SIP URI�����������ļ��ж�ȡ����������ļ��������������û���
	string getCurrentUserName();

	//��ȡ��ǰ�û��ĳ�Ա��Ϣ
	MemberInfo * getCurrentUserMemberInfo();

	//��ȡ��ǰ����״̬
	ConferenceState getConferenceState()	const
	{
		return _confState;
	}

	//�鿴�Ƿ����ڿ�����
	bool checkIsInConference() 
	{
		return _confState ==IsInConference;
	}

	//�жϵ�ǰ�����Ƿ��鲥����
	bool checkCurrentIsMulticastConference();

	//��ȡ��ǰ������鲥��ַ������������鲥���飬�������IP��ַ�ȣ�����ture�����򷵻�false
	bool getCurrentMulticastAddress(string& ipaddress);

	std::string joiningMeeting;//Add By LZY 2010-10-09 ��¼��ǰ����Ļ�����

private:    //˽�з���

    //����һ����������sig��Ϣ
    string createJoinConferenceMsg(const  string& confUri);

    //����һ���»��飬�����˳���ǰ���顢��ճ�Ա�б�����ý�崫��Ȳ���
    int joinConference(const string& curFocusURI);

    //�ܾ�������飬������Ʒ�����������
    int rejectToJoinConference(const string& curFocusURI, const string& reason);

    //����һ����������Ƿ�ɹ�����Ϣ�����Ʒ�����
    void sendLoginAckMsg(bool isSuccess);

    //����RTPͳ�Ƹ�QoS������
    void sendRTPStatToQoSServer(const RtpStatItem& rtpstat);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief   ���ͼ��������Ϣ��QoSģ����
    ///
    /// @author qhf
    /// @date   2010-08-18
    ///
    /// @param  curFocusURI URI of the current focus.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void sendQosJoinConfReport(const string& curFocusURI);

    //�����˳�������Ϣ��QoSģ����
    void sendQosExitConfReport(const string& curFocusURI);


private:
    /************************************************************************/
    /*  ���յ���SIP��Ϣ���������ȶ�XML���ݽ������ٽ�����ش���               */
    /************************************************************************/

	//�յ����е�SIP��Ϣ
    void cvMessageReceivedEventHandler(const std::string& message, const std::string& fromUri);

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

    //���յ�SetSpeaker����SIP��Ϣ��ָ��������
    void recvSetSpeakerMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

    //���յ�ForbidSpeaker����SIP��Ϣ��ȡ��������
    void recvForbidSpeakerMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

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
    void recvReceiverMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

    //���յ�ScreenReceiver����SIP��Ϣ������ý����
    void recvScreenReceiverMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

    //���յ�LoginResult����SIP��Ϣ������һ·��Ļ������
    void recvUnShareScreenMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

    //���յ�Cancel2Receiver����SIP��Ϣ����������һ·��Ļ������
    void recvCancel2ReceiverMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

    //���յ�ScreenCancel2Receiver����SIP��Ϣ��ȡ����Ļ����
    void recvScreenCancel2ReceiverMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

    //���յ�QoS����SIP��Ϣ��qos�������
    void recvQoSMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

	//���յ�QoS ģ���������SIP��Ϣ
	void recvQoSTestMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri);

private: //˽�б���

    //�����б�
    vector<ConfInfo*> _confList;
    //�����Ա�б�
    vector<MemberInfo*> _memberList;
 
    //����״̬
    ConferenceState _confState;

	//��ǰ����URI���������ڽ��룬���ڿ����С����ܾ����롢�����˳���״̬
    string _focusURI;

    //������URI
    string _hostURI;

    //�������ģʽ
    string _confMode;

    //����title
    string _confTitle;

    ConfInfo* _currentConfInfo;     //��ǰ������Ϣָ�룬ֻ�����ڿ�����(_confState= IsInConference)��ָ�����Ч������ʱ��Ϊ��

    //����Ԥ�����ھ��
    HWND _previewHwnd;


    //�Ƿ��Ѿ�������Ļ����
    bool _isShareScreen;
    //��¼���͵ĵ�ַ
    vector<ReceiverInfo> _sendIPList;
    //��¼��Ļ�����͵ĵ�ַ
    vector<ReceiverInfo> _screenShareSendIPList;
    //��Ļ���������
    RECT _shareRect;
    //�����Զ���
    list<MemberInfo*>_requestForSpeakList;
    //���Զ���
    list<MemberInfo*>_speakingList;

    /************************************************************************/

private:        //Wengo��һЩԭ����

    // Inherited from Profile
    virtual void setWengoPhoneId(const std::string & /*wengoPhoneId*/)
    {
    }
    virtual std::string getWengoPhoneId() const
    {
        return String::null;
    }
    ////

    /**
     * @see WsCallForward::wsCallForwardEvent
     */
    void wsCallForwardEventHandler(WsCallForward& sender, int id, WsCallForward::WsCallForwardStatus status);

    /**
     * Handles SipAccount::loginStateChangedEvent.
     */
    void loginStateChangedEventHandler(SipAccount& sender, EnumSipLoginState::SipLoginState state);
    void loginStateChangedEventHandlerThreadSafe(SipAccount& sender, EnumSipLoginState::SipLoginState state);

    /**
     * Handles SipAccount::networkDiscoveryStateChangedEvent
     */
    void networkDiscoveryStateChangedEventHandler(SipAccount& sender, SipAccount::NetworkDiscoveryState state);

    /**
     * Compute the name of the UserProfile from the SipAccount
     * and set the _name variable.
     */
    void computeName();

    /* Inherited from Profile */
    void setAlias(const std::string& alias)
    {
        _alias = alias;
    };

    void setIcon(const OWPicture& icon)
    {
        setIcon(icon, NULL);
    };
    /**/

    /**
     * Actually adds an IMAccount. Used internally and by addIMAccount after
     * checking the given IMAccount.
     */
    void _addIMAccount(const IMAccount& imAccount);

    /**
     * Actually removes an IMAccount. Used internally and by removeIMAccount after
     * checking the given IMAccount.
     */
    void _removeIMAccount(const IMAccount& imAccount);

    /**
     * Connect all IMAccounts.
     */
    void connectIMAccounts();

    /**
     * Disconnect all IMAccounts.
     */
    void disconnectIMAccounts();

    /**
     * Connect all SipAccounts.
     */
    bool connectSipAccounts();

    /**
     * Disconnect all SipAccounts.
     *
         * @param force if true, disconnect without doing any transactions
     */
    void disconnectSipAccounts(bool force = false);

    /**
     * Initializes the SipAccount.
     */
    void sipAccountInit();

    /**
     * Creates and adds a new PhoneLine given a SipAccount.
     *
     * This is a helper method.
     *
     * @param account SipAccount associated with the newly created PhoneLine
     */
    bool addPhoneLine(SipAccount& account);

    /**
     * find the wengo phone line
     */
    IPhoneLine* findWengoPhoneLine();

	/** The active/current PhoneLine. */
    IPhoneLine* _activePhoneLine;

    /** The active PhoneCall. */
    PhoneCall* _activePhoneCall;

    /** List of PhoneLine. */
    PhoneLines _phoneLineList;

    /** History. */
    History* _history;

    /** WengoPhone update system. */
    WsSoftUpdate* _wsSoftUpdate;

    /** SMS. */
    WsSms* _wsSms;

    /** WsInfo. */
    WsInfo* _wsInfo;

    /** WsCallForward. */
    WsCallForward* _wsCallForward;

    //TODO: create a list of SipAccount
    SipAccount* _sipAccount;

    /** Wenbox. */
    WenboxPlugin* _wenboxPlugin;

    RecursiveMutex _mutex;

    /**
     * True if the UserProfile must connect after intialization of the
     * SipAccount
     */
    bool _sipAccountMustConnectAfterInit;

    bool _sipAccountReadyToConnect;

    bool _historyLoaded;

    IMAccountManager* _imAccountManager;

    IMContactListHandler* _imContactListHandler;

    ConnectHandler* _connectHandler;

    PresenceHandler* _presenceHandler;

    ChatHandler* _chatHandler;

    ContactList* _contactList;

    CoIpManager* _coIpManager;

    /** Name of the UserProfile. */
    std::string _name;
};

#endif  //OWUSERPROFILE_H
