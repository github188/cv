/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#ifndef OWCUSERPROFILE_H
#define OWCUSERPROFILE_H

#include <control/chat/CChatHandler.h>
#include <control/contactlist/CContactList.h>
#include <control/wenbox/CWenboxPlugin.h>

#include <model/account/SipAccount.h>
#include <model/phoneline/EnumMakeCallError.h>
#include <model/conference/CVMsgParser.h>

#include <imwrapper/EnumIMProtocol.h>
#include <imwrapper/IMAccountList.h>

#include <util/Trackable.h>

#include <set>
#include <Windows.h>
#include "RtpStat.h"		  //RTPͳ��ͷ�ļ�


class CHistory;
class CPhoneLine;
class CSms;
class CSoftUpdate;
class CWengoPhone;
class CWsCallForward;
class History;
class IMAccount;
class IPhoneLine;
class PhoneCall;
class PUserProfile;
class UserProfile;
class WengoAccount;
class WsCallForward;
class WsSms;
class WsSoftUpdate;
class PresenceHandler;

/**
 * Control layer for UserProfile.
 *
 * FIXME: 'init()' must be called on UserProfile before constructing CUserProfile.
 *
 * @author Philippe Bernery
 */
class CUserProfile : public Trackable {
public:

	Event <void (CUserProfile & sender, EnumMakeCallError::MakeCallError)> makeCallErrorEvent;


	CUserProfile(UserProfile & userProfile, CWengoPhone & cWengoPhone);

	~CUserProfile();

	/**
	 * @see UserProfile::disconnect
	 */
	void disconnect();

	/**
	 * @see UserProfile::makeCall()
	 */
	void makeContactCall(const std::string & contactId);

	/**
	 * @see UserProfile::makeCall()
	 */
	void makeCall(const std::string & phoneNumber);

	/**
	 * @see UserProfile::startIM
	 */
	void startIM(const std::string & contactId);

	/**
	 * @see UserProfile::setSipAccount
	 */
	void setSipAccount(const SipAccount & sipAccount);

	/**
	 * Gets the active phone call.
	 *
	 * Used for playing DTMF.
	 *
	 * @return active phone call or NULL
	 */
	PhoneCall * getActivePhoneCall() const;

	/**
	 * Gets the CHistory.
	 *
	 * @return the CHistory
	 */
	CHistory * getCHistory() const {
		return _cHistory;
	}

	/**
	 * Gets the CContactList.
	 *
	 * @return the CContactList
	 */
	CContactList & getCContactList() {
		return _cContactList;
	}

	/**
	 * @return the CWsCallForward object.
	 */
	CWsCallForward * getCWsCallForward() {
		return _cWsCallForward;
	}

	/**
	 * @return the CWengoPhone object.
	 */
	CWengoPhone & getCWengoPhone() {
		return _cWengoPhone;
	}

	/**
	 * Gets the UserProfile.
	 *
	 * TODO: This method should not exist (because it allows the GUI to access
	 * the model directly) and so should be removed ASAP.
	 */
	UserProfile & getUserProfile() {
		return _userProfile;
	}

	/**
	 * Gets the Presentation layer.
	 */
	PUserProfile * getPresentation() {
		return _pUserProfile;
	}

	CPhoneLine * getCPhoneLine() {
		return _cPhoneLine;
	}

	/************************************************************************/
	/*Start Conference*/
	void startConference(const std::string & confURI,const std::string & szContent);
	void applyConference(const std::string & confURI,const std::string & szContent);

	void exitConference(int &callId);

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
	int makeJoinConfInvite(const int & index);
	int makeJoinConfInvite(const std::string & focusURI);

	Event <void (CUserProfile & sender, vector<ConfInfo *> ConfList)> cvConfListReceivedEvent;
	Event <void (CUserProfile & sender, vector<MemberInfo *> MemberList,std::string confTitle)> cvMemberListReceivedEvent;
	Event <void (CUserProfile & sender, ConfInfo *conference,const std::string & from)> cvConfrenceInviteEvent;
	Event <void (CUserProfile & sender)> cvPasswordIncorrectEvent;
	Event <void (CUserProfile & sender)> cvKickedEvent;
	Event <void (CUserProfile & sender, int callId,HWND &previewHwnd)> setCallIdEvent;
	Event <void (CUserProfile & sender, const std::string & Message,const std::string & from)> cvTextMsgReceivedEvent;
	Event <void (CUserProfile & sender, const std::string & Message,const std::string & from)> cvWhiteBoardMsgReceivedEvent;
	//�����Ժ�������¼��������µġ����֡���ť����
	Event <void (CUserProfile & sender)> cvPermitSpeakEvent;
	//
	Event <void (CUserProfile & sender)> cvHostUnShareScreenEvent;


	//�ܾ���������¼�������uri���ܾ�ԭ��
	Event<void (CUserProfile&sender, const string& confFocusURI, const string & reason)> cvJoinRejectionEvent;

	void cvJoinRejectionEventHandler( const string& confFocusURI, const string & reason);

	void cvConfListReceivedEventHandler(vector<ConfInfo *> ConfList);
	void cvMemberListReceivedEventHandler(vector<MemberInfo *> MemberList,std::string confTitle);
	void cvConfrenceInviteEventHandler( ConfInfo *conference,const std::string & from);
	void cvPasswordIncorrectEventHandler();
	void cvKickedEventHandler();
	void setCallIdEventHandler(int callId,HWND &previewHwnd);
	void cvTextMsgReceivedEventHandler(const std::string & Message,const std::string & from);
	void cvWhiteBoardMsgReceivedEventHandler(const std::string & Message,const std::string & from);
	void cvPermitSpeakEventHandler();
	void cvHostUnShareScreenEventHandler();
	
	
	int media_send_start(char * AudioIP,int AudioPort,char * VideoIP,int VideoPort,HWND previewHwnd);
	void media_send_stop(char * AudioIP,int AudioPort,char * VideoIP,int VideoPort);
	int media_recv_start(char * AudioIP,int AudioPort,char * VideoIP,int VideoPort,char *codecMIME,HWND remoteHwnd);
	int media_recv_start(int index,HWND remoteHwnd);
	void media_recv_stop(std::string userId);
	//��Ļ����
	int screen_share_send_start(char * ScreenIP,int ScreenPort,int x,int y,int width,int height);
	//ȡ�����湲��
	void screen_share_send_stop(char * ScreenIP,int ScreenPort);
	//�������湲��
	int screen_share_recv_start(char * ScreenIP,int ScreenPort,HWND remoteHwnd,int videoWidth,int videoHeight);
	//ȡ���������湲��
	void screen_share_recv_stop();
	void exit_conference();

	Event <void (CUserProfile & sender, const std::string & userId)> logoutReceivedEvent;
	void logoutReceivedEventHandler( const std::string & userId);

	//���ý��հ�ť��״̬
	bool setRecvButtonStatus(int index);
	//���ñ���Ԥ�����ھ��
	void setPreviewWin(HWND  &Hwnd);
	//video����resize
	void resizeVideoWindow(const std::string &userId,const HWND &remoteHwnd,int fullscreenflag);

	Event <void (CUserProfile & sender, const RtpStatItem &rtpstat)> rtpStatRecvEvent;
	void rtpStatRecvEventHandler(const RtpStatItem &rtpstat);

	Event <void (const TranStat &transtat)> tranStatRecvEvent;
	void tranStatRecvEventHandler( const TranStat & transtat ); 


	//������룬��ȷ����Ϣ��Focus�������򷵻�
	int checkPassword(int &row,std::string &password);
	//��鵱ǰ�û��Ƿ�������
	bool hostOrNot();

	//�����Ա����״̬���Ƿ����ڷ���
	int memberSpeakStatus(int row);
	//ָ��������
	int setSpeaker(int row,bool permitOrForbid,int callId);

	//����Info��Ϣ
	void makeInfoCall(int callId,char * szContent);

	//ɾ�������
	int KickOut(int row,int callId);

	//�����ı���Ϣ
	void sendTextMsg(const int &receiverIndex,const std::string &szContent);
	//���ַ���
	int handForSpeak(int row,bool handUpOrDown,int callId);
	/************************************************************************/

	/**
	* Add by: hexianfu
	* for the telecontroller
	get the latest confinfo and memberinfo
	* 2009-10-29
	*/
	vector<ConfInfo *>& getCurrentConfInfoList();

	vector<MemberInfo *>& getCurrentMemberInfoList();
	/***********************************/


	//��������Ƶ����ͣ...zhenHua.sun 2010-08-06
	/**
	 * @brief ������Ƶ�ķ���
	 * @param state �����true����������Ƶ�ķ��ͣ�����ֹͣ��Ƶ�ķ���
	 */
	void controlAudioSend( bool state );
	/**
	 * @brief ������Ƶ�ķ���
	 * @param state �����true����������Ƶ�ķ��ͣ�����ֹͣ��Ƶ�ķ���
	 */
	void controlVideoSend( bool state );
	//end

private:

	/**
	 * @see UserProfile::loginStateChangedEvent
	 */
	void loginStateChangedEventHandler(SipAccount & sender,
		EnumSipLoginState::SipLoginState state);

	/**
	 * @see UserProfile::networkDiscoveryStateChangedEvent
	 */
	void networkDiscoveryStateChangedEventHandler(SipAccount & sender,
		SipAccount::NetworkDiscoveryState state);

	/**
	 * @see PresenceHandler::authorizationRequestEvent
	 */
	void authorizationRequestEventHandler(PresenceHandler & sender,
		const IMContact & imContact, const std::string & message);

	/**
	 * @see PresenceHandler::authorizationRequestEvent
	 */
	void incomingSubscribeEventHandler(PresenceHandler & sender,
		const std::string & imaccountId,int sid,const std::string & from,const std::string & evtType);

	/**
	 * @see UserProfile::historyLoadedEvent
	 */
	void historyLoadedEventHandler(History & sender);

	/**
	 * @see UserProfile::phoneLineCreatedEvent
	 */
	void phoneLineCreatedEventHandler(UserProfile & sender, IPhoneLine & phoneLine);

	/**
	 * @see UserProfile::wsSmsCreatedEvent
	 */
	void wsSmsCreatedEventHandler(UserProfile & sender, WsSms & wsSms);

	/**
	 * @see UserProfile::wsSoftUpdateCreatedEvent
	 */
	void wsSoftUpdateCreatedEventHandler(UserProfile & sender, WsSoftUpdate & wsSoftUpdate);

	/**
	 * @see UserProfile::wsCallForwardCreatedEvent
	 */
	void wsCallForwardCreatedEventHandler(UserProfile & sender, WsCallForward & wsCallForward);

	/**
	 * @see disconnect()
	 */
	void disconnectThreadSafe();

	/**
	 * @see makeCall()
	 */
	void makeContactCallThreadSafe(std::string contactId);

	/**
	 * @see makeCall()
	 */
	void makeCallThreadSafe(std::string phoneNumber);

	/**
	 * @see startIM
	 */
	void startIMThreadSafe(std::string contactId);

	/**
	 * @see setSipAccount
	 */
	void setSipAccountThreadSafe(SipAccount sipAccount);

	/************************************************************************/
	/*Start Conference*/
	void startConferenceThreadSafe(std::string & confURI,std::string & szContent);
	void exitConferenceThreadSafe(int &callId);
	void media_send_start_ThreadSafe(char * AudioIP,int AudioPort,char * VideoIP,int VideoPort,HWND previewHwnd);
	/************************************************************************/

	UserProfile & _userProfile;

	CWengoPhone & _cWengoPhone;

	PUserProfile * _pUserProfile;

	CContactList _cContactList;

	CHistory * _cHistory;

	CWsCallForward * _cWsCallForward;

	CWenboxPlugin _cWenboxPlugin;

	CChatHandler _cChatHandler;

	CSms * _cSms;

	CSoftUpdate * _cSoftUpdate;

	CPhoneLine * _cPhoneLine;

};

#endif	//OWCUSERPROFILE_H
