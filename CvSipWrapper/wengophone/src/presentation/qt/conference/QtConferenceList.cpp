/** 
 * Coolview3.0 ����Ԫ��
 * zhenHua.sun 2010-08-13
 */

#include "stdafx.h"

#include ".\qtconferencelist.h"
#include <util/SafeDelete.h>
#include <qtutil/SafeConnect.h>
#include "QtWengoPhone.h"
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>
#include <control/CWengoPhone.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include "QtMemberList.h"
#include "QtInstantConferenceWidget.h"
#include "QtPasswordInputWidget.h"
#include <QtGui/QtGui>
#include "ExportUtil.h"
#include "QtToolWidget.h"
#include "control/MessageFrame.h"

QtConferenceList::QtConferenceList(QtWengoPhone & qtWengoPhone,QtToolWidget *tools):_qtWengoPhone(qtWengoPhone),_cWengoPhone(_qtWengoPhone.getCWengoPhone()),_tools(tools)
{
	_ui = new Ui::ConferenceList();
	_proxyModel = new QSortFilterProxyModel(this);
	_ui->setupUi(this);
//	sourceModel = NULL;
	confTreeModel = new QtConfListTree();
	_proxyModel->setSourceModel(confTreeModel);
	_ui->treeView->setModel(_proxyModel);
	_ui->treeView->setRootIsDecorated(false);
	SAFE_CONNECT(_ui->refreshButton, SIGNAL(clicked()), SLOT(refreshButtonClicked()));
	SAFE_CONNECT(_ui->instantConfButton, SIGNAL(clicked()), SLOT(instantConfButtonClicked()));
	SAFE_CONNECT(_ui->treeView,SIGNAL(doubleClicked(const QModelIndex&)),SLOT(JoinConference(const QModelIndex&)));

	SAFE_CONNECT_TYPE(this, SIGNAL(cvConfrenceInviteEventHandlerSinal(ConfInfo *,const QString&)),SLOT(cvConfrenceInviteEventHandlerSlot(ConfInfo *,const QString&)), Qt::QueuedConnection);
	SAFE_CONNECT_TYPE(this, SIGNAL(cvPasswordIncorrectEventHandlerSinal()),SLOT(cvPasswordIncorrectEventHandlerSlot()), Qt::QueuedConnection);

	CUserProfile * cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();
	cUserProfile->cvConfListReceivedEvent +=boost::bind(&QtConferenceList::cvConfListReceivedEventHandler,this,_2);
	cUserProfile->cvConfrenceInviteEvent +=boost::bind(&QtConferenceList::cvConfrenceInviteEventHandler,this,_2,_3);
	cUserProfile->cvPasswordIncorrectEvent +=boost::bind(&QtConferenceList::cvPasswordIncorrectEventHandler,this);

	SendGetConfListMsg();
	//����QoS��ز���
	//sendQosParam();

	SAFE_CONNECT_RECEIVER(&_qtWengoPhone,SIGNAL(ActionRefreshMeetingListSignal()),this,SLOT(refreshButtonClicked()));//Add By LZY 2010-09-28 ����ӦRefreshMeetingList����Signal�ĺ���
	SAFE_CONNECT_RECEIVER(&_qtWengoPhone,SIGNAL(ActionEnterMeetingSignal(QString)),this,SLOT(teleEnterMeetingCommand(QString)));//Add By LZY 2010-09-28 ����ӦEnterMeeting����Signal�ĺ���
}

QtConferenceList::~QtConferenceList(void)
{
	disconnect(&_qtWengoPhone,SIGNAL(ActionRefreshMeetingListSignal()),this,SLOT(refreshButtonClicked()));//Add By LZY 2010-09-28 �����ӦRefreshMeetingList����Signal�ĺ���
	disconnect(&_qtWengoPhone,SIGNAL(ActionEnterMeetingSignal(QString)),this,SLOT(teleEnterMeetingCommand(QString)));//Add By LZY 2010-09-28 �����ӦEnterMeeting����Signal�ĺ���
	
	CUserProfile * cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();
	cUserProfile->cvConfListReceivedEvent -=boost::bind(&QtConferenceList::cvConfListReceivedEventHandler,this,_2);
	cUserProfile->cvConfrenceInviteEvent -=boost::bind(&QtConferenceList::cvConfrenceInviteEventHandler,this,_2,_3);
	OWSAFE_DELETE(_ui);
	OWSAFE_DELETE(_proxyModel);
//	OWSAFE_DELETE(sourceModel);
	OWSAFE_DELETE(confTreeModel);
}

void QtConferenceList::ShowConfList(vector<ConfInfo *> &confList)
{
//	sourceModel = (QStandardItemModel *)createModel(confList);
//	confTreeModel = new QtConfListTree();
	int size = confList.size();
	assert(confTreeModel!=NULL);
	confTreeModel->setupModelData(confList);
}

void QtConferenceList::refreshButtonClicked()
{
	SendGetConfListMsg();
}

void QtConferenceList::SendGetConfListMsg()
{
	CUserProfile * cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	//std::string confUri = "sip:service@ser.scut.edu.cn";
	std::string confUri = config.getGlobalService();
	std::string szContent = "<?xml version=\"1.0\"?><coolview command=\"GetConfList\"></coolview>";
//	cUserProfile->cvConfListReceivedEvent +=boost::bind(&QtConferenceList::ShowConfList,this,_2);
	cUserProfile->startConference(confUri,szContent);
}

//����QoS��ز���
void QtConferenceList::sendQosParam()
{
	CUserProfile * cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();
	//std::string confUri = "sip:zhongkan.li@sipx.scut.edu.cn";
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	std::string confUri = config.getGlobalService();
	std::string szContent = "<?xml version=\"1.0\"?><coolview command=\"Command\">Qos����</coolview>";
	//	cUserProfile->cvConfListReceivedEvent +=boost::bind(&QtConferenceList::ShowConfList,this,_2);
	cUserProfile->startConference(confUri,szContent);
}

//-------Modify By LZY 2010-09-20-------
//   -   ��Ӳ���PassWord���ѷ���ֵ��void��Ϊint
/**
* Add by: hexianfu
* reload the JoinConference(const QModelIndex&proxyIndex);
* 2009-11-3
*/
int QtConferenceList::JoinConference(const int& row,std::string PassWord)
{
#ifndef DBUS_ENABLE
	//Ҫ�ڼ������ǰ�����úû��Դ��ڣ������ʧ��...zhenHua.sun 2010-09-08
    _tools->getMeetingPlaceWidget()->setPreviewWnd();
#endif

	CUserProfile * cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();
	int result = cUserProfile->makeJoinConfInvite(row);
	//��Ҫ����
	if (result==-1)
	{
		if (PassWord!="")
		{
			int RowNum=row;
			result = cUserProfile->checkPassword(RowNum,PassWord);
		}
	}

	//�������֮����ʾ��Ա�б�  zhenHua.sun 2010-08-13
	if( this->_tools )
	{
		this->_tools->_ui->conInfoTabWidget->setCurrentWidget(
			this->_tools->_ui->memberListTab
			);
		this->_tools->setIsInConference( true );
	}

	return result;
}
//-------End of Modify By LZY------------

//-------Modify By LZY 2010-09-20-------
//   -   ���Ӷ�JoinConference(const int& row,std::string PassWord)����ֵ�Ĵ���
void QtConferenceList::JoinConference(const QModelIndex&proxyIndex)
{
	QtConferenceList::_qtWengoPhone.SetUIState(UI_S_JOINMEETING_BEGIN);//Add By LZY 2010-09-30 ����֪ͨң��������״̬�仯
	int row = proxyIndex.row();
	int result = JoinConference(row);
	if (result==-1)//��Ҫ����
	{
		QtPasswordInputWidget passwordInputDialog(_qtWengoPhone.getWidget(), _cWengoPhone,row);
		passwordInputDialog.exec();
	}
	else if (result==-2)//�Ѿ��ڻ�����
	{
		QMessageBox::warning(this, tr("@product@"),
		tr("\346\202\250\345\267\262\347\273\217\350\277\233\345\205\245\344\272\206\344\270\200\344\270\252\344\274\232\350\256\256!"), QMessageBox::Ok);
		QtConferenceList::_qtWengoPhone.SetUIState(UI_S_MEETING);//Add By LZY 2010-09-30 ����֪ͨң��������״̬�仯
	}
	else if (result==-3)
	{
		QMessageBox::warning(this, tr("@product@"),
			QString::fromLocal8Bit("�����˳����飬���Ժ����ԣ�"), QMessageBox::Ok);
		QtConferenceList::_qtWengoPhone.SetUIState(UI_S_EXITMEETING_BEGIN);//Add By LZY 2010-09-30 ����֪ͨң��������״̬�仯
	}
}
//-------End of Modify By LZY-----------

void QtConferenceList::cvConfListReceivedEventHandler(vector<ConfInfo *> ConfList) 
{
	//CVMsgParser cvMessageParser;
	//cvMessageParser.InitParser(message.c_str());
	//const char *command = cvMessageParser.GetCommandType();
	//if (strcmp(command,"ConfList")==0)
	//{
	//	//int size = ConfList.size();
	//	if(ConfList.size()>0)
	//		ReleaseConfList(ConfList);
	//	cvMessageParser.ParserConfListMsg(ConfList);
	//	ShowConfList(ConfList);
	//}
	//typedef PostEvent1<void (vector<ConfInfo *>), vector<ConfInfo *> > MyPostEvent;
	//MyPostEvent * event = new MyPostEvent(boost::bind(&QtConferenceList::ShowConfList, this, _1), ConfList);
	//_qtWengoPhone.postEvent(event);

	ShowConfList(ConfList);
	QtConferenceList::_qtWengoPhone.sendTeleStateChanged(CHANGE_MEETINGLIST);//Add By LZY 2010-10-09 ���������б���֪ͨ
}

void QtConferenceList::cvConfrenceInviteEventHandler(ConfInfo * conference,const std::string & from)
{
	cvConfrenceInviteEventHandlerSinal(conference,QString(from.c_str()));
}

/**
* Modifided by: hexianfu
* for the telecontroller to subscribe
* 2010-4-20
*/
void QtConferenceList::cvConfrenceInviteEventHandlerSlot(ConfInfo * conference,const QString& from)
{
	QMessageBox mb(tr("@CoolView@ - Invite"),
		QString::fromLocal8Bit("%1 �������μӻ��� \"%2\". �Ƿ�μӸû��飿").arg(from,QString::fromUtf8(conference->Title.c_str())),//Modify By LZY 2010-10-06
		QMessageBox::Question,
		QMessageBox::Yes | QMessageBox::Default,
		QMessageBox::No | QMessageBox::Escape,
		QMessageBox::NoButton, this);
	
	int MsgBoxIdentify=_qtWengoPhone.AddInviteMsg(&mb);//Add By LZY 2010-09-27

	//-------Add By LZY 2010-10-06-------
	CMessageFrame ResponseMsg;
	ResponseMsg.Append(MsgBoxIdentify);
	ResponseMsg.Append(from.toUtf8().data());
	ResponseMsg.Append(conference->Title.c_str());
	QtConferenceList::_qtWengoPhone.sendTeleStateChanged(CHANGE_INVITE,ResponseMsg.getString());//֪ͨң���������������
	//-------End of Add By LZY-----------

	int MsgBoxReturn = mb.exec();
	if (MsgBoxReturn == QMessageBox::Yes || MsgBoxReturn == QMessageBox::Accepted) {
		_qtWengoPhone.RemoveInviteMsg(MsgBoxIdentify);//Add By LZY 2010-09-27

		CUserProfile * cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();
		cUserProfile->makeJoinConfInvite(conference->URI);


		//�������֮����ʾ��Ա�б�  zhenHua.sun 2010-10-11
		if( this->_tools )
		{
			this->_tools->_ui->conInfoTabWidget->setCurrentWidget(
				this->_tools->_ui->memberListTab
				);
			this->_tools->setIsInConference( true );
		}
	}
	else
		_qtWengoPhone.RemoveInviteMsg(MsgBoxIdentify);//Add By LZY 2010-09-27
}
/***********************************/

void QtConferenceList::instantConfButtonClicked()
{
	QtInstantConferenceWidget instantConferenceDialog(_qtWengoPhone.getWidget(), _cWengoPhone);

	if( instantConferenceDialog.exec()== QDialog::Accepted )
	{	
		//�������֮����ʾ��Ա�б�  zhenHua.sun 2010-08-13
		if( this->_tools )
		{
			this->_tools->_ui->conInfoTabWidget->setCurrentWidget(
				this->_tools->_ui->memberListTab
				);
			this->_tools->setIsInConference( true );
		}
		this->refreshButtonClicked();
	}
}

void QtConferenceList::cvPasswordIncorrectEventHandler()
{
	cvPasswordIncorrectEventHandlerSinal();
}

void QtConferenceList::cvPasswordIncorrectEventHandlerSlot()
{
	QMessageBox::warning(this, tr("@product@ - Password Error"),
		tr("the password you input is incorrect"), QMessageBox::Ok);
}

//-------Add By LZY 2010-09-20-------
/* ����EnterMeeting�����¼�
 * @param QString EnterMeetingStr,����Ĳ���
 * @return void
 */
void QtConferenceList::teleEnterMeetingCommand(QString EnterMeetingStr)
{
	CMessageFrame EnterMeetingParam(EnterMeetingStr.toStdString().c_str());
	CUserProfile * cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();
	vector<ConfInfo *> currentConfList = cUserProfile->getCurrentConfInfoList();
	string MeetingURI=EnterMeetingParam.readLine(true);//Ҫ����Ļ���ʶ��URI
	int nSize=currentConfList.size();
	int indexRow=-1;//ͨ������URI�������Ļ������б��е�������
	for (int LoopVar=0;LoopVar<nSize;++LoopVar)
	{
		if (currentConfList[LoopVar]->URI==MeetingURI)
		{
			indexRow=LoopVar;
			break;
		}
	}
	if (indexRow==-1)
	{
		QtConferenceList::_qtWengoPhone.SetUIState(UI_S_LOGIN);//Add By LZY 2010-09-30 ����֪ͨң��������״̬�仯���������ʧ��
		return;
	}
	int result=QtConferenceList::JoinConference(indexRow,EnterMeetingParam.readLine(true));
	if (result<0)
		QtConferenceList::_qtWengoPhone.SetUIState(UI_S_LOGIN);//Add By LZY 2010-09-30 ����֪ͨң��������״̬�仯���������ʧ��
}
//-------End of Add By LZY---------