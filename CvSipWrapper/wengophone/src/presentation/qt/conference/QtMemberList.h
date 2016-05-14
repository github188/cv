/**
 * Coolview 3.0 �������  
 * zhenHua.sun 2010-08-10
 */

#pragma once
#include "ui_MemberListWidget.h"
#include "QtConferenceRoomWidget.h"
#include <model/conference/CVMsgParser.h>
#include "QtMemberListTreeModel.h"
#include <QtGui/QtGui>
class QtWengoPhone;
class CWengoPhone;
class QtConferenceRoomWidget;
class QtToolWidget;

class QtMemberList:public QWidget
{
	friend class QtToolWidget;
	Q_OBJECT
public:
	QtMemberList(QtWengoPhone & qtWengoPhone,QtConferenceRoomWidget *confRoom);
	~QtMemberList(void);

	void ShowMemberList(vector<MemberInfo *> &memberList);
	QAbstractItemModel *createModel(vector<MemberInfo *> &memberList);
	void addMember(QAbstractItemModel *model, const QString &displayName);
	void cvMemberListReceivedEventHandler(vector<MemberInfo *> &MemberList,std::string ConfTitle);
	void setCallIdEventHandler(int callId,HWND &previewHwnd);
	void getMediaParamFromSdp(int &aport,int &vport,char *mediaType,const char *sdp);
	void ReleaseMemberList(vector<MemberInfo *> &member_list);
	void cvKickedEventHandler();
	void cvHandDownEventHandler();

	/**
	* Add by: hexianfu
	* for the teleController to contact a member/quit meeting
	* 2009-11-3
	*/
	int RecvAudioVideo(int &rowIndex,std::string &userId);
	/***********************************************/

Q_SIGNALS:
	void setConfTitleSinal(const QString& conf_title);
	void cvKickedEventHandlerSinal();
	void cvHandDownEventHandlerSinal();

public Q_SLOTS:
	//-------Add By LZY 2010-10-05-------
	void teleInviteMembersCommand(QString InviteMembers);//Add By LZY 2010-10-05 ��ӦInviteMember����Signal��Slot
	void teleLocalVideoSettingCommand();//��ӦLocalVideoSetting������Signal
	void teleLocalAudioSettingCommand();//��ӦLocalAudioSetting������Signal
	void teleStopMemberCommand(QString userId);//��ӦStopMember������Signal
	//-------End of Add By LZY-----------

private Q_SLOTS:

	bool exitButtonClicked();
	void setRecvButtonStatus(const QModelIndex&proxyIndex);
	void RecvAudioVideo(const QModelIndex&proxyIndex);
	void RecvAudioVideo();
	void SendAudioVideo();
	void setConfTitleSlot(const QString& conf_title);
	void showPopupMenuForCurrentIndex();
	void makeInfoCallSlot();
	void cvKickedEventHandlerSlot();
	void setExitButtonEnable();
	void handButtonClicked();
	void cvHandDownEventHandlerSlot();
	void releaseFloorControl();
	void teleRaiseHandCommand();//Add By LZY 2010-10-05 ��Ӧ����RaiseHand��Signal��Slot

	//��ͣ����Ƶ zhenHua.sun 2010-08-05
	
	void audioControlClicked( bool checked );
	void videoControlClicked( bool checked );
	
	void teleContactMemberCommand(QString ContactMemberNameStr);//Add By LZY 2010-09-28 ����ContactMember����Slot
	void exitMeeting();//Add By LZY 2010-09-25 ң�����ͽ��洦��ͨ�õ��˳����鴦����
	void telePermitSpeakCommand(QString userName);//Modify By LZY 2010-09-28 ����PermitSpeak����Slot
	void teleKickMemberCommand(QString userName);//Modify By LZY 2010-09-28 ����KickMember����Slot

private:
	Ui::MemberList *_ui;
	QSortFilterProxyModel *_proxyModel;

	QtWengoPhone & _qtWengoPhone;

	CWengoPhone & _cWengoPhone;

//	vector<MemberInfo *> MemberList;

	int send_video_port;
	int send_audio_port;
	string send_address;

	int _callId;

	QtConferenceRoomWidget *_confRoom;

	bool inConfrence;//�Ƿ��Ѿ��ڻ�������

	QtMemberListTreeModel *memberListTreeModel;

	//��ʱ��
	QTimer *infoTimer;
	QTimer *exitButtonTimer;
	QTimer *releaseFloorControlTimer;
};
