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

#ifndef OWQTWENGOPHONE_H
#define OWQTWENGOPHONE_H

#include <model/phoneline/EnumMakeCallError.h>
#include <model/network/NetworkProxyDiscovery.h>

#include <imwrapper/EnumPresenceState.h>

#include <qtutil/QObjectThreadSafe.h>

#include <cutil/global.h>
#include <util/Trackable.h>

#include <QtGui/QMainWindow>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QTabWidget>
#include <QtGui/QPushButton>

#include <string>
#include <memory>

class CWengoPhone;
class Config;
class ContactInfo;
class IMContact;
class PPhoneLine;
class PhoneCall;
class PresenceHandler;
class QtCallBar;
class QtContactCallListWidget;
class QtContactList;
class QtFileTransfer;
class QtHistoryWidget;
class QtIdle;
class QtLanguage;
class QtPhoneCall;
class QtProfileBar;
class QtSms;
class QtStatusBar;
class QtToolBar;
class QtSystray;
class QtToaster;
class QtBrowserWidget;
class QtDialpad;
class UserProfile;
class QtPhoneComboBox;
class QtIMAccountMonitor;
class QtConferenceRoomWidget;

class QtLoginDialog; //Add by hexianfu  09-11-9
class ILogin;  //Add by hexianfu  09-11-27
class QMessageBox;//Add by hexianfu  10-4-20

class QtToolWidget; //zhenHua.sun 2010-08-19

#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
class QtWebDirectory;
#endif

class QWidget;
class QMenu;


namespace Ui { class WengoPhoneWindow; }

using namespace std;//Add By LZY 2010-09-27

//-------Add By LZY 2010-09-20_2010-09-25-------
//   -   �Ӻ󷵻ض����б����ֵ����
#define ACTION_LIST_LEN 9 //�б���
#define ACTION_LOGIN 0 //Login����
#define ACTION_ENTERMEETING 1 //EnterMeeting����
#define ACTION_CONTACTMEMBER 2 //ContactMember����
#define ACTION_QUITMEETING 3 //QuitMeeting����
#define ACTION_LOGOFF 4 //LogOff����
#define ACTION_RAISEHAND 5 //RaiseHand����
#define ACTION_PERMITSPEAK 6 //PermitSpeak����
#define ACTION_KICKMEMBER 7 //KickMember����
#define ACTION_CREATEMEETING 8 //CreateMeeting����
//-------End of Add By LZY-----------

//-------Add By LZY 2010-09-20-------
//   -   Actionִ�н������
#define ARESULT_SUCCESS 0 //����ִ�гɹ�
#define ARESULT_RUNNING 1 //ͬ���Ķ�������ִ���У��Ӻ󷵻ض������д˷���ֵ��
#define ARESULT_FAIL 2 //����ִ��ʧ�ܣ����½ʧ�ܵȣ�
#define ARESULT_FORBIDDEN 3 //�ڵ�ǰ���ն�״̬�¸ö���������ִ�У��Ӻ󷵻ض������д˷���ֵ����δ��½״̬����ˢ�»����б��᷵�ش���Ϣ��
//-------End of Add By LZY-----------

//-------Add By LZY 2010-09-19_2010-10-05-------
//   -   �ն˽���״ֵ̬���壬4�ֽڣ���16λ��λ���䣬��ʾ�����פ̬������ά���ŵ�״̬������16λ��ֵ���䣬��ʾ�����˲̬���������˲�䣩
//    ---פֵ̬���壨��λ��
#define UI_S_ALL 4294901760 //��16λȫ����1����ʾ��������פ̬
#define UI_S_INIT_BEGIN 2147483648 //�ն˳�ʼ����
#define UI_S_LOGOFF 1073741824 //�ն˴���δ��½״̬
#define UI_S_LOGOFF_BEGIN 536870912 //�ն����ڵǳ���
#define UI_S_LOGIN_BEGIN 268435456 //�ն����ڵ�½��
#define UI_S_LOGIN 134217728 //�ն��ѵ�½
#define UI_S_JOINMEETING_BEGIN 67108864 //�ն����ڼ���ĳ������
#define UI_S_MEETING 33554432 //�ն��Ѽ���ĳ������
#define UI_S_EXITMEETING_BEGIN 16777216 //�ն������˳�����
//    ---˲ֵ̬���壨��ֵ����Χ[1-65535]��
#define UI_D_ALL 65535 //��16λȫ����1����ʾ��������˲̬
#define UI_D_CONTACTMEMBER_SUCCESS 1 //����ContactMemberִ�����
#define UI_D_CONTACTMEMBER_ERROR 2 //����ContactMemberִ������������
#define UI_D_RAISEHAND_SUCCESS 3 //����RaiseHandִ�����
#define UI_D_PERMITSPEAK_SUCCESS 4 //����PermitSpeakִ�����
#define UI_D_PERMITSPEAK_ERROR 5 //����PermitSpeakִ������������
#define UI_D_KICKMEMBER_SUCCESS 6 //����KickMemberִ�����
#define UI_D_KICKMEMBER_ERROR 7 //����KickMemberִ������������
#define UI_D_CREATEMEETING_SUCCESS 8 //����CreateMeetingִ�����
#define UI_D_LOCALVIDEO_SEND_START 9 //����LocalVideoSetting��������Ƶ����
#define UI_D_LOCALVIDEO_SEND_STOP 10 //����LocalVideoSetting��ֹ������Ƶ����
#define UI_D_LOCALAUDIO_SEND_START 11 //����LocalAudioSetting��������Ƶ����
#define UI_D_LOCALAUDIO_SEND_STOP 12 //����LocalAudioSetting��ֹ������Ƶ����
//-------End of Add By LZY-----------

//-------Add By LZY 2010-10-06-------------
#define CHANGE_MEETINGLIST 0 //�����б���
#define CHANGE_MEMBERLIST 1 //�����Ա�б���
#define CHANGE_LAYOUT 2 //��Ƶ���ֱ��
#define CHANGE_UI 3 //����פ̬���
#define CHANGE_INVITE 4 //�յ�����
#define CHANGE_KICK 5 //���߳�����
#define CHANGE_CONTACT 6 //��ϵ����Ϣ�����仯
#define CHANGE_LOCALVIDEO 7 //������Ƶ�ķ������÷����˱仯
#define CHANGE_LOCALAUDIO 8 //������Ƶ�ķ������÷����˱仯
//-------End of Add By LZY----------------
/**
 * Qt Presentation component for WengoPhone.
 *
 * @author Tanguy Krotoff
 */
class QtWengoPhone : public QObjectThreadSafe, public Trackable {
	Q_OBJECT

		friend class QtLoginDialog;       //Add by dhx 10-10-25

public:

	QtWengoPhone(CWengoPhone & cWengoPhone);

	~QtWengoPhone();

	/**
	 * @brief ��ȡ������
	 */
	QMainWindow*  getWengoPhoneWindow(){  return _wengoPhoneWindow; }


	void addPhoneCall(QtPhoneCall * qtPhoneCall);

	void addToConference(QtPhoneCall * qtPhoneCall);

	/************************************************************************/
	/*�������ģʽ*/
	void addToConference();
	void deleteConference();
	/************************************************************************/

	void updatePresentation();

	void dialpad(const std::string & tone);

	void connectionStatusEventHandler(int totalSteps, int currentStep, const std::string & infoMsg);

	QWidget * getWidget() const;

	void setQtDialpad(QtDialpad * qtDialpad);

	void setQtContactList(QtContactList * qtContactList);
	QtContactList * getQtContactList() const;

	void setQtHistoryWidget(QtHistoryWidget * qtHistoryWidget);

	void setQtSms(QtSms * qtSms);
	QtSms * getQtSms() const;

	QtStatusBar & getQtStatusBar() const;

	QtFileTransfer * getFileTransfer() const;

	QtToolBar & getQtToolBar() const;

	QtCallBar & getQtCallBar() const;

	QtSystray & getQtSystray() const;

	QtLanguage & getQtLanguage() const;

	QtProfileBar * getQtProfileBar() const;

	CWengoPhone & getCWengoPhone() const;

	QtConferenceRoomWidget * getQtConferenceRoomWidget();

#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
	QtWebDirectory * getQtWebDirectory() const;
#endif

	void setChatWindow(QWidget * chatWindow);
	QWidget * getChatWindow() const;
        void closeChatWindow ();

	void showHistory();

	void ensureVisible();

	void showAddContact(ContactInfo contactInfo);

	virtual void bringMainWindowToFront();

	void installQtBrowserWidget();

	void uninstallQtBrowserWidget();

	QtBrowserWidget * getQtBrowserWidget() const;

	/**
	* Add by: hexianfu
	* for the telecontroller commandProcess
	* 2009-10-22
	*/
	QtLoginDialog & getQtLoginDialog() const;

	void setQtLoginDialog(QtLoginDialog * qtLoginDialog);

	/**
	 * @author zhenHua.sun
	 * @date 2010-08-19
	 * @brief ���ù��߲˵���
	 */
	void setQtToolWidget( QtToolWidget* qtToolWidget );
	QtToolWidget* getQtToolWidget() const;
	
	int AddInviteMsg(QMessageBox *MsgBoxPointer);//Add By LZY 2010-09-27 ����µ�������Ϣ��ָ�뵽��Ϣ��ָ�������

	char *getPrivateMeetPwd();

	void exitEvent();//Move By LZY 2010-09-27

	CWengoPhone & _cWengoPhone;//Move By LZY From Private To Public 2010-09-25

	//-------Add By LZY 2010-09-19_2010-09-30-------
	//---����---
	void SendInfo(char *Message);//2010-11-02 ������Ϣ��teleController��Ŀǰʹ��DBus���ƹ����ͣ�����Ҫ��
	void sendTeleStateChanged(int StateIndex,const char *AddInfo=NULL);//�����ն�����ͨ�����Ϣ
	void ExitMeetingSuccess();//��ӦUserProfile������ExitMeetingSuccessEvent�¼�������UI_State_Code����ֵ
	void SetUIState(unsigned long StateCode);//���ý���״̬��ǣ���������Ӧ��Ϣ��teleController��������״̬�仯�ˣ�Ӧ�õ��ñ�����
	void commandProcess(const char *recvedMess);//����������
	void SaveGlobalSetting(string IPv4Addr,string IPv6Addr);//����GlobalSetting��ֵַ��������ң����ͳһ���õĽӿ�
	void RemoveAllMsg();//�Ƴ����е�ǰ������ʾ����Ϣ������ȷ�ϣ�ע��ȷ�ϵȣ���������Ϣ��ִ��reject����

	//---�ź�---
Q_SIGNALS:
	void RefreshAudioOutSliderSignal(int Value);//���¿��ƽ����������Ƶ���������źţ�����QtAudioSetting��û���ض��Ľӿ����ã�ֻ�ü������Signal�����
	void RefreshAudioInSliderSignal(int Value);//���¿��ƽ�����������Ƶ���������źţ�����QtAudioSetting��û���ض��Ľӿ����ã�ֻ�ü������Signal�����
	void ActionLoginSignal(QString LoginParam);//����Login�������ź�
	void ActionRefreshMeetingListSignal();//����RefreshMeetingList�������ź�
	void ActionEnterMeetingSignal(QString EnterMeetingParam);//����EnterMeeting�������ź�
	void ActionContactMemberSignal(QString ContactMemberName);//����ContactMember�������ź�
	void ActionStopMemberSignal(QString userName);//����StopMember�������ź�
	void ActionQuitMeetingSignal();//����QuitMeeting�������ź�
	void ActionLogOffSignal();//����LogOff�������ź�
	void ActionLocalControlSignal();//����LocalControl�������ź�
	void ActionSetVolumeSignal(int VolumeValue,char VolumeType);//����SetVolume�������ź�
	void ActionRaiseHandSignal();//����RaiseHand�������ź�
	void ActionPermitSpeakSignal(QString userName);//����PermitSpeak�������ź�
	void ActionKickMemberSignal(QString userName);//����KickMember�������ź�
	void ActionSetLayoutTypeSignal(QString LayoutType);//����SetLayoutType�������ź�
	void ActionInviteMembersSignal(QString InviteMembers);//����InviteMembers�������ź�
	void ActionAcceptInviteSignal(int InviteIdentifyNum,int MethodNum);//����AcceptInvite�������ź�
	void ActionCreateMeetingSignal(QString Title,QString Description,int ControlModeNum,int JoinModeNum,QString Psw);//����CreateMeeting�������ź�
	void ActionLocalVideoSettingSignal();//����LocalVideoSetting�������ź�
	void ActionLocalAudioSettingSignal();//����LocalAudioSetting�������ź�
	//-------End of Add By LZY----------

public Q_SLOTS:

	void currentUserProfileWillDieEventHandlerSlot();

	void userProfileInitializedEventHandlerSlot();

	void hangUpButtonClicked();

	void prepareToExitApplication();

	void addToConference(QString phoneNumber, PhoneCall * targetCall);

	void setCurrentUserProfileEventHandlerSlot();

	/*
	* Add by huixin.du for delete conference tab  2010-09-14
	* ���dialWidget���Ƿ��к���
	*/
    void hideDialWidget();

	//-------Add By LZY 2010-09-28_2010-10-05-------
	void RemoveInviteMsg(int IdentifyNum,int AdditionMethod=0);//�Ƴ�ָ������λ�õ���Ϣ��ָ�룬���AdditionMethod��Ϊ0�������Ƴ�ǰ�Ը���Ϣ��ִ�н�һ���Ĳ���
	void CreateMeeting(QString Title,QString Description,int ControlModeNum,int JoinModeNum,QString Psw);//���������ú�����ң�����ͽ���ͨ��
	//-------End of Add By LZY-----------

private Q_SLOTS:

	void callButtonClicked();

	void enableCallButton();

	void phoneComboBoxClicked();

	void closeWindow();

	void languageChanged();

	void setActiveTabBeforeCallCurrent();

Q_SIGNALS:

	/**
	 * Current user profile has been deleted.
	 *
	 * Graphical components should be re-initialized:
	 * QtContactList, QtHistoryWidget, QtProfileBar are removed.
	 * QtSystray, QtBrowserWidget have to be re-initialized.
	 */
	void userProfileDeleted();

private:

	//-------Add By LZY 2010-09-19_2010-10-05-------
	//---�ṹ��---
	struct ActionListNode//�������нṹ����ر������壬�����е�ÿһ����һ����Ҫ�Ӻ󷵻صĶ����󶨣���ֵ�ɱ��ļ���ͷ��һ��ֵ����
	{
		int IndexNum;//teleController�ж������յȴ����ж�Ӧ������ֵ
		int AvailableNum;//teleController�ж������յȴ����ж�Ӧ����Чֵ
		bool Using;//ʹ���б�ǣ����ڶԴ˱�����trueֻ����һ���߳��з��������Բ��ÿ��ǻ�������
		ActionListNode()
		{
			IndexNum=0;
			AvailableNum=0;
			Using=false;
		}
	};

	//---����---
	void sendTeleResult(const char* actionName,int Result,int TIndex,int TAvailable,const char * Response=NULL);//�������ص�Action���Ͷ����������
	void ResponseDelayAction(string ActionName,int ActionIndex,int Result=ARESULT_SUCCESS,char * ResponsePointer=NULL);//�����Ӻ���ӦAction�ĺ������������Զ��жϴ����ActionIndex����ӦAction�Ƿ���Ҫ���ͻظ���Ϣ
	bool CheckDelayAction(char *ActionName,unsigned long EnabledStateCode,int ActionListIndex,char * RunMode,int TIndex,int TAvailableNum);//����Ӻ󷵻ض����Ƿ�����ִ����������������ʱ�Զ���teleController���Ͷ�������
	bool CheckImitAction(char *ActionName,unsigned long EnabledStateCode,char *RunMode,int TIndex,int TAvailableNum);//����������ض����Ƿ�����ִ����������������ʱ�Զ���teleController���Ͷ�������

	//---����---
	unsigned int UI_State_Code;//��¼��ǰ����������״̬�����ڷ��������Ƿ�ִ�У���ֵ�����ɱ��ļ�ǰ��ġ�UI_����ͷ��Ԥ����ֵ����
	ActionListNode ActionList[ACTION_LIST_LEN];
	//-------End of Add By LZY----------

	void initUi();

	void initThreadSafe();

	//-------Add By LZY 2010-09-27-------
	void LockInviteList(long LockNum);//InviteList�����������������
	bool UnLockInviteList(long UnLockNum);//InviteList�����������������
	//-------End of Add By LZY-----------

	/**
	 * Set the geometry of the mainwindow (position and size).
	 *
	 * Load last size and position. If the mainwindow is not visible use default values.
	 */
	void mainWindowGeometry(Config & config);

	/**
	 * Initializes pickup and hangup buttons inside the main window.
	 *
	 * This is called by QtPhoneCall to re-initialize the buttons
	 * since QtPhoneCall modifies the behaviour of this buttons.
	 */
	void initCallButtons();

	void updatePresentationThreadSafe();

	void proxyNeedsAuthenticationEventHandler(NetworkProxyDiscovery & sender, NetworkProxy networkProxy);

	void proxyNeedsAuthenticationEventHandlerThreadSafe(NetworkProxy networkProxy);

	void wrongProxyAuthenticationEventHandler(NetworkProxyDiscovery & sender, NetworkProxy networkProxy);

	void makeCallErrorEventHandler(EnumMakeCallError::MakeCallError);

	void makeCallErrorEventHandlerThreadSafe(EnumMakeCallError::MakeCallError);


#ifdef OS_MACOSX
	void fixMacOSXMenus();
#endif

	void loadStyleSheets();

	/** Direct link to the control. */

	Ui::WengoPhoneWindow * _ui;

	QMainWindow * _wengoPhoneWindow;

	QtSystray * _qtSystray;

	QtSms * _qtSms;

	QtContactList * _qtContactList;

	QtHistoryWidget * _qtHistoryWidget;

	QtContactCallListWidget * _qtContactCallListWidget;

	QtIdle * _qtIdle;

	QtLanguage * _qtLanguage;

	QtToolBar * _qtToolBar;

	QtStatusBar * _qtStatusBar;

	QtFileTransfer * _qtFileTransfer;

	QtBrowserWidget * _qtBrowserWidget;

	int _qtBrowserWidgetTabIndex;

	QWidget * _chatWindow;

	QWidget * _activeTabBeforeCall;

	QTabWidget * _dialTabWidget;      //huixin.du 2010-09-15

	QPushButton * _toolButton;

	QString currentActionName; // Add by li.zhenning for Action Timeout at 2010-08-19

	QtLoginDialog *_qtLoginDialog;

	//-------Add By LZY 2010-09-27-------
	struct InviteListNode
	{
		QMessageBox * MsgPointer;//������Ϣ��ָ��
		int IdentifyNum;//�ڵ�Ψһʶ���
	};
	InviteListNode * InviteList;
	int InviteListCount;//ͳ�Ƶ�ǰ������InviteList�ڵ���
	int InviteListNextIdentify;//��һ�����õ�Identifyֵ
	long InviteListLockNum;//InviteList��������ֵ��0��ʾδ����
	//-------End of Add By LZY-----------

	std::string teleInviteConference;

	char *privateMeetingPwd;

	std::auto_ptr<QtIMAccountMonitor> _qtIMAccountMonitor;
	QtConferenceRoomWidget * m_qtConferenceRoomWidget;


#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
	QtWebDirectory * _qtWebDirectory;
#endif

	//���߲˵���  zhenHua.sun 2010-08-19
	QtToolWidget* _qtToolWidget;
};

#endif	//OWQTWENGOPHONE_H
