#include "stdafx.h"
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

#include "QtWengoPhone.h"

#include "ui_WengoPhoneWindow.h"

#include <presentation/PFactory.h>
#include <presentation/qt/chat/QtChatWindow.h>

#include <cutil/global.h>

#include "QtSystray.h"
#include "QtHttpProxyLogin.h"
#include "QtDialpad.h"
#include "QtIdle.h"
#include "QtLanguage.h"
#include "QtToolBar.h"
#include "QtBrowserWidget.h"
#include "QtIMAccountMonitor.h"
#include "callbar/QtCallBar.h"
#include "callbar/QtPhoneComboBox.h"
#include "contactlist/QtContactList.h"
#include "filetransfer/QtFileTransfer.h"
#include "history/QtHistoryWidget.h"
//#include "imaccount/QtIMAccountManager.h"			����zhenHua.sun 2010-09-03
#include "phonecall/QtContactCallListWidget.h"
#include "phonecall/QtPhoneCall.h"
#include "profile/QtProfileDetails.h"
#include "profilebar/QtProfileBar.h"
//#include "profilebar/QtIMProfileWidget.h"			����zhenHua.sun 2010-09-03
#include "statusbar/QtStatusBar.h"
#include "webservices/sms/QtSms.h"
#include "conference/QtConferenceRoomWidget.h"
#include "conference/QtInstantConferenceWidget.h" //Add by hexianfu
#include "conference/QtToolWidget.h"	//zhenHua.sun 2010-08-19
#include "profile/QtUserProfileHandler.h"
#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
	#include "webdirectory/QtWebDirectory.h"
#endif

#include <control/CWengoPhone.h>
#include <control/history/CHistory.h>
#include <control/phoneline/CPhoneLine.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <model/WengoPhone.h>
#include <model/commandserver/ContactInfo.h>
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/connect/ConnectHandler.h>
#include <model/contactlist/ContactList.h>
#include <model/contactlist/Contact.h>
#include <model/history/History.h>
#include <model/phonecall/PhoneCall.h>
#include <model/phonecall/SipAddress.h>
#include <model/phonecall/ConferenceCall.h>
#include <model/phoneline/IPhoneLine.h>
#include <model/profile/AvatarList.h>
#include <model/profile/UserProfile.h>
#include <model/account/EnumSipLoginState.h>

#include <imwrapper/EnumPresenceState.h>

#include <qtutil/CloseEventFilter.h>
#include <qtutil/Widget.h>
#include <qtutil/SafeConnect.h>
#include <qtutil/LanguageChangeEventFilter.h>

#include <cutil/global.h>
#include <thread/Thread.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <webcam/WebcamDriver.h>
#include "ExportUtil.h"

#include <QtGui/QtGui>

#include <RunningProfile.h>
#include <SessionManager.h>
/**
* Add by: hexianfu
* for the telecontroller message analysis
* 2009-10-21
*/
#include "control/MessageFrame.h"
#include "login/ILogin.h"
#include "login/QtAddSIPAccount.h"
#include "login/QtLoginDialog.h"
#include "login/QtLogAccount.h"
#include "login/QtAddAccount.h"
#include <QtCore/QTimer>
/***********************************/

#include <sound/VolumeControl.h>
#include <sound/AudioDevice.h>

#include "conference/QtToolWidget.h"


//zhenHua.sun  2010-10-26
#ifdef DBUS_ENABLE
#include <dbus/core/service/CvCoreService.h>
#include <dbus/telecontroller/CvTelecontrollerClient.h>		//zhenHua.sun 2010-11-02
#endif


#if defined(OS_MACOSX)
	#include <Carbon/Carbon.h>
#endif

static const char * CSS_DIR = "css";

using namespace std;

QtWengoPhone::QtWengoPhone(CWengoPhone & cWengoPhone)
	: QObjectThreadSafe(NULL),
	_cWengoPhone(cWengoPhone) {

		QtWengoPhone::UI_State_Code=UI_S_INIT_BEGIN;//Add By LZY 2010-09-19

		//-------Add By LZY 2010-09-27-------
		//   -   ������Ϣ���������ر�����ʼ��
		QtWengoPhone::InviteList=(InviteListNode *)malloc(sizeof(InviteListNode)*5);
		for (int LoopVar=0;LoopVar<5;++LoopVar)
		{
			QtWengoPhone::InviteList[LoopVar].IdentifyNum=0;
			QtWengoPhone::InviteList[LoopVar].MsgPointer=NULL;
		}
		QtWengoPhone::InviteListCount=5;
		QtWengoPhone::InviteListNextIdentify=1;
		QtWengoPhone::InviteListLockNum=0;
		//-------End of Add By LZY-----------

	_wengoPhoneWindow = NULL;
	_qtSystray = NULL;
	_qtSms = NULL;
	_qtContactList = NULL;
	_qtHistoryWidget = NULL;
	_qtContactCallListWidget = NULL;
	_qtIdle = NULL;
	_qtLanguage = NULL;
	_qtStatusBar = NULL;
	_qtFileTransfer = NULL;
	_qtBrowserWidget = NULL;
	_chatWindow = NULL;
	_activeTabBeforeCall = NULL;
	m_qtConferenceRoomWidget = NULL;

	_dialTabWidget =new QTabWidget();    //huixin.du 2010-09-15
	_toolButton = new QPushButton();
	_qtToolWidget = NULL;

	_qtLoginDialog = NULL;

#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
	_qtWebDirectory = NULL;
#endif

	NetworkProxyDiscovery::getInstance().proxyNeedsAuthenticationEvent +=
		boost::bind(&QtWengoPhone::proxyNeedsAuthenticationEventHandler, this, _1, _2);
	NetworkProxyDiscovery::getInstance().wrongProxyAuthenticationEvent +=
		boost::bind(&QtWengoPhone::wrongProxyAuthenticationEventHandler, this, _1, _2);

	//Check if the event has not already been sent
	if (NetworkProxyDiscovery::getInstance().getState() ==
		NetworkProxyDiscovery::NetworkProxyDiscoveryStateNeedsAuthentication) {
		proxyNeedsAuthenticationEventHandler(NetworkProxyDiscovery::getInstance(),
			NetworkProxyDiscovery::getInstance().getNetworkProxy());
	}

	qRegisterMetaType<QVariant>("QVariant");

	//Initialize libwebcam for the Qt thread.
	WebcamDriver::apiInitialize();
	////

	initUi();

	typedef PostEvent0<void ()> MyPostEvent;
	MyPostEvent * event = new MyPostEvent(boost::bind(&QtWengoPhone::initThreadSafe, this));
	postEvent(event);

	SAFE_CONNECT_RECEIVER(this,SIGNAL(ActionAcceptInviteSignal(int,int)),this,SLOT(RemoveInviteMsg(int,int)));//Add By LZY 2010-09-28 ����ӦAcceptInvite����Signal�ĺ���
	SAFE_CONNECT_RECEIVER(this,SIGNAL(ActionCreateMeetingSignal(QString,QString,int,int,QString)),this,SLOT(CreateMeeting(QString,QString,int,int,QString)));//Add By LZY 2010-09-28 ����ӦCreateMeeting����Signal�ĺ���

#ifdef DBUS_ENABLE
	CvCoreService::getInstance()->setQtWengoPhone( this );
#endif
}

QtWengoPhone::~QtWengoPhone() {

	disconnect(this,SIGNAL(ActionAcceptInviteSignal(int,int)),this,SLOT(RemoveInviteMsg(int,int)));//Add By LZY 2010-09-28 �����ӦAcceptInvite����Signal�ĺ���
	disconnect(this,SIGNAL(ActionCreateMeetingSignal(QString,QString,int,int,QString)),this,SLOT(CreateMeeting(QString,QString,int,int,QString)));//Add By LZY 2010-09-28 �����ӦCreateMeeting����Signal�ĺ���

	free(QtWengoPhone::InviteList);//Add By LZY 2010-09-27
}

void QtWengoPhone::initUi() {
	// Init parts of the UI which do not rely on Control layer
	QApplication::setQuitOnLastWindowClosed(false);

	loadStyleSheets();

	//Translation
	_qtLanguage = new QtLanguage(this);

	_wengoPhoneWindow = new QMainWindow(NULL);
	_wengoPhoneWindow->setWindowFlags(Qt::WindowStaysOnTopHint);  //add by dhx 2010-10-27
	LANGUAGE_CHANGE(_wengoPhoneWindow);

	_ui = new Ui::WengoPhoneWindow();
	_ui->setupUi(_wengoPhoneWindow);

#ifndef CUSTOM_ACCOUNT
	_ui->actionShowWengoAccount->setVisible(false);
#endif

#ifdef OS_MACOSX
	fixMacOSXMenus();
#endif

#ifdef OS_LINUX
	std::string data = AvatarList::getInstance().getDefaultAvatarPicture().getData();
	QPixmap defaultAvatar;
	defaultAvatar.loadFromData((uchar*) data.c_str(), data.size());
	_wengoPhoneWindow->setWindowIcon(QIcon(defaultAvatar));
#endif

	//zhenHua.sun 2010-10-20
	_wengoPhoneWindow->setWindowIcon(QIcon(":/coolview3/application.ico"));

	//Install the close event filter
	CloseEventFilter * closeEventFilter = new CloseEventFilter(this, SLOT(closeWindow()));
	_wengoPhoneWindow->installEventFilter(closeEventFilter);

	//QtCallBar
	_ui->callBar->getQtPhoneComboBox()->setQtWengoPhone(this);

	//QtToolBar
	_qtToolBar = new QtToolBar(*this, _ui, _wengoPhoneWindow);

	//phoneComboBox
	SAFE_CONNECT(_ui->callBar, SIGNAL(phoneComboBoxClicked()), SLOT(phoneComboBoxClicked()));

	//Buttons initialization
	initCallButtons();

	//QtStatusBar
	_qtStatusBar = new QtStatusBar(this, _ui->statusBar);

	/****************SipCall  huixin.du 2010-09-15**********************/
	//_ui->dialWidget->layout()->addWidget(_dialTabWidget);

    _toolButton->setMaximumSize(30,30);  //���ư�ť��С
    _toolButton->setMinimumSize(30,30);

	QIcon *icon = new QIcon(":/coolview3/setting.bmp"); //��ť���icon
    QSize *isize = new QSize(25,25);
    _toolButton->setIcon(*icon);
    _toolButton->setIconSize(*isize);

    QHBoxLayout *mainLayout = new QHBoxLayout();
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget(_toolButton);
	vLayout->addSpacing(_dialTabWidget->height()+300);      //add by huixin.du 2010-09-06
	vLayout->setMargin(0);                     //add by huixin.du 2010-09-06
    mainLayout->addLayout(vLayout);
	mainLayout->addWidget(_dialTabWidget);
	mainLayout->setMargin(0);                 //add by huixin.du 2010-09-06
	_ui->dialDialpad->setLayout(mainLayout);
    _ui->dialWidget->hide();


    /******************************************************************/

#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
	_qtWebDirectory = new QtWebDirectory(0);
#endif

	installQtBrowserWidget();

	Config & config = ConfigManager::getInstance().getCurrentConfig();
	mainWindowGeometry(config);


	//����menubar  zhenHua.sun 2010-08-19
	_ui->menuBar->hide();

	//����statusbar  huixin.du 2010-09-06
	_ui->statusBar->hide();
}

void QtWengoPhone::initThreadSafe() {
	// The code in this method relies on the Control layer
	_qtSystray = new QtSystray(this);

	updatePresentation();
}

void QtWengoPhone::loadStyleSheets() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QDir dir(QString::fromStdString(config.getResourcesDir()) + CSS_DIR);

	QStringList filters;
	filters << "*.css";
	QStringList cssList;
	Q_FOREACH(QFileInfo fileInfo, dir.entryInfoList(filters)) {
		QString path = fileInfo.absoluteFilePath();
		QFile file(path);
		if (!file.open(QIODevice::ReadOnly)) {
			LOG_WARN("Can't open " + path.toStdString());
			continue;
		}
		QByteArray content = file.readAll();
		cssList << QString::fromUtf8(content);
	}
	QString styleSheet = cssList.join("\n");
	qApp->setStyleSheet(styleSheet);
}

void QtWengoPhone::mainWindowGeometry(Config & config) {
	//default position and size given by Qt
	QPoint defaultPos = _wengoPhoneWindow->pos();
	QSize defaultSize = _wengoPhoneWindow->size();

	//int profileWidth = config.getProfileWidth();
	//if (profileWidth == 0) {
	//	// Make sure the toolbar is fully visible
	//	profileWidth = qMax(
	//		_ui->toolBar->sizeHint().width(),
	//		_wengoPhoneWindow->sizeHint().width()
	//		);
	//}

	//int profileHeight = config.getProfileHeight();

	//����ȫ�� zhenHua.sun 2010-08-18
	int x = QApplication::desktop()->width();
	int y = QApplication::desktop()->height();
	int profileWidth = x*0.9;
	int profileHeight = y*0.9;

	//Main window size and position saved
	_wengoPhoneWindow->resize(profileWidth, profileHeight);
	_wengoPhoneWindow->showMaximized();
	//_wengoPhoneWindow->move(QPoint(config.getProfilePosX(), config.getProfilePoxY()));
	_wengoPhoneWindow->move( 0 , 0) ;	//���Ͻ�

	//tests if the Wengophone is visible, if not sends it back to its default position and size
	QDesktopWidget* desktop = QApplication::desktop();
	if (desktop->screenNumber(_wengoPhoneWindow) == -1) {
		LOG_DEBUG("Main window is NOT visible !!");
		_wengoPhoneWindow->resize(defaultSize);
	 	_wengoPhoneWindow->move(defaultPos);
	}
}

QWidget * QtWengoPhone::getWidget() const {
	return _wengoPhoneWindow;
}

QtProfileBar * QtWengoPhone::getQtProfileBar() const {
	return _ui->profileBar;
}

QtBrowserWidget * QtWengoPhone::getQtBrowserWidget() const {
	return _qtBrowserWidget;
}

CWengoPhone & QtWengoPhone::getCWengoPhone() const {
	return _cWengoPhone;
}

void QtWengoPhone::setQtSms(QtSms * qtSms) {
	_qtSms = qtSms;
}

QtSms * QtWengoPhone::getQtSms() const {
	return _qtSms;
}

#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
QtWebDirectory * QtWengoPhone::getQtWebDirectory() const {
	return _qtWebDirectory;
}
#endif

QtToolBar & QtWengoPhone::getQtToolBar() const {
	return *_qtToolBar;
}

QtStatusBar & QtWengoPhone::getQtStatusBar() const {
	return *_qtStatusBar;
}

QtSystray & QtWengoPhone::getQtSystray() const {
	return *_qtSystray;
}

QtCallBar & QtWengoPhone::getQtCallBar() const {
	return *_ui->callBar;
}

void QtWengoPhone::setChatWindow(QWidget * chatWindow) {
	if (!chatWindow) {
		_chatWindow = NULL;
		_ui->actionOpenChatWindow->setEnabled(false);
	}
	else {
		_chatWindow = chatWindow;
		_ui->actionOpenChatWindow->setEnabled(true);
	}
}

QWidget * QtWengoPhone::getChatWindow() const {
	return _chatWindow;
}

void QtWengoPhone::closeChatWindow() {
        if (_chatWindow)
        {
            ((QtChatWindow *)_chatWindow)->closeAllTabs ();
        }
}

void QtWengoPhone::setQtContactList(QtContactList * qtContactList) {
	_qtContactList = qtContactList;
/*
	if (!_ui->tabContactList->layout()) {
		Widget::createLayout(_ui->tabContactList);
	}

	_ui->tabContactList->layout()->addWidget(_qtContactList->getWidget());
	_ui->tabContactList->setHidden(true);
	LOG_DEBUG("QtContactList added");
	*/
}

QtContactList * QtWengoPhone::getQtContactList() const {
	return _qtContactList;
}

void QtWengoPhone::setQtHistoryWidget(QtHistoryWidget * qtHistoryWidget) {
	_qtHistoryWidget = qtHistoryWidget;
	_qtHistoryWidget->setQtToolBar(_qtToolBar);
	_qtHistoryWidget->setQtCallBar(_ui->callBar);

	//if (!_ui->tabHistory->layout()) {
	//	Widget::createLayout(_ui->tabHistory);
	//}

	//_qtHistoryWidget->getWidget()->setParent(_ui->tabHistory);
	//_ui->tabHistory->layout()->addWidget(_qtHistoryWidget->getWidget());

	LOG_DEBUG("QtHistoryWidget added");
}

void QtWengoPhone::setQtDialpad(QtDialpad * qtDialpad) {
//	Widget::createLayout(_ui->tabDialpad)->addWidget(qtDialpad);
	LOG_DEBUG("QtDialpad added");
}

void QtWengoPhone::initCallButtons() {
	//callButton
	SAFE_CONNECT(_ui->callBar, SIGNAL(callButtonClicked()), SLOT(callButtonClicked()));
	enableCallButton();

	//hangUpButton
	SAFE_CONNECT(_ui->callBar, SIGNAL(hangUpButtonClicked()), SLOT(hangUpButtonClicked()));
	_ui->callBar->setEnabledHangUpButton(false);

	//phoneComboBox
	SAFE_CONNECT(_ui->callBar, SIGNAL(phoneComboBoxReturnPressed()), SLOT(callButtonClicked()));
	SAFE_CONNECT(_ui->callBar, SIGNAL(phoneComboBoxEditTextChanged(const QString &)), SLOT(enableCallButton()));
}

void QtWengoPhone::enableCallButton() {
	std::string phoneNumber = _ui->callBar->getPhoneComboBoxCurrentText();
	_ui->callBar->setEnabledCallButton(!phoneNumber.empty());
}

void QtWengoPhone::hangUpButtonClicked() {
    //modified by huixin.du 2010-09-13
	QtContactCallListWidget * widget =
		dynamic_cast<QtContactCallListWidget *>(_ui->dialWidget);//dynamic_cast<QtContactCallListWidget *>(_ui->tabWidget->currentWidget());
	if (widget) {
		widget->hangup();
	}
}

void QtWengoPhone::callButtonClicked() {
	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile()) {
		std::string phoneNumber = _ui->callBar->getPhoneComboBoxCurrentText();
		if (!phoneNumber.empty()) {
			CUserProfile * cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();
			cUserProfile->makeCallErrorEvent += boost::bind(&QtWengoPhone::makeCallErrorEventHandler, this, _2);
			cUserProfile->makeCall(phoneNumber);
		}
		_ui->callBar->clearPhoneComboBoxEditText();
	}
}

void QtWengoPhone::setActiveTabBeforeCallCurrent() {
	//_ui->tabWidget->setCurrentWidget(_activeTabBeforeCall);   modified by huixin.du 2010-09-13
	if(_activeTabBeforeCall == _ui->tabWidget)
	{
		_ui->tabWidget->show();
		_ui->dialWidget->hide();
	}
	else
	{
		_ui->dialWidget->show();
		_ui->tabWidget->hide();
	}

}

void QtWengoPhone::makeCallErrorEventHandler(EnumMakeCallError::MakeCallError error) {
	typedef PostEvent1<void (EnumMakeCallError::MakeCallError), EnumMakeCallError::MakeCallError> MyPostEvent;
	MyPostEvent * event = new MyPostEvent(boost::bind(&QtWengoPhone::makeCallErrorEventHandlerThreadSafe, this, _1), error);
	postEvent(event);
}

void QtWengoPhone::makeCallErrorEventHandlerThreadSafe(EnumMakeCallError::MakeCallError error) {
	QString message;
	switch (error) {
	case EnumMakeCallError::CallNotHeld:
		message = tr("Please hold all the phone calls before placing a new one");
		break;
	case EnumMakeCallError::NotConnected:
		message = tr("Can't place a call, you are not connected");
		break;
	case EnumMakeCallError::EmptyPhoneNumber:
		message = tr("You must enter a phone number");
		break;
	case EnumMakeCallError::SipError:
		message = tr("An error has occured trying to place the call");
		break;
	case EnumMakeCallError::NoError:
		LOG_FATAL("Should not happen");
		break;
	case EnumMakeCallError::RepeatCall:
		message = tr("you are talking with the person");
		break;
	}
	QMessageBox::warning(_wengoPhoneWindow, tr("@product@ - Call Error"),
		message, QMessageBox::Ok);
}

void QtWengoPhone::addPhoneCall(QtPhoneCall * qtPhoneCall) {

	_activeTabBeforeCall = _ui->dialWidget;//_activeTabBeforeCall = _ui->tabWidget->currentWidget();
	_ui->tabWidget->hide();_ui->dialWidget->show();       //added by huixin.du 2010-09-06
	QtContactCallListWidget * qtContactCallListWidget = new QtContactCallListWidget(_cWengoPhone, NULL);
	//SAFE_CONNECT(qtContactCallListWidget, SIGNAL(aboutToClose()), SLOT(setActiveTabBeforeCallCurrent()) );

    connect(qtContactCallListWidget,SIGNAL(aboutToClose()),this,SLOT(hideDialWidget()));       //added by huixin.du 2010-09-13

	if(m_qtConferenceRoomWidget)                                   //added by huixin.du 2010-09-16
	{
		connect(_toolButton,SIGNAL(clicked()),m_qtConferenceRoomWidget->_tools,SLOT(hideToolWidget()));
	}

	/************************************************************************///Add by zhuang 09-01-08
	/*                                                                      */
	QString userName = qtPhoneCall->getNickNameLabelText();
	userName.append("-call");
	//_ui->dialWidget->layout()->addWidget(qtContactCallListWidget);  //added by huixin.du 2010-09-13
	_dialTabWidget->addTab(qtContactCallListWidget, userName);   //added by huixin.du 2010-09-15
	//_ui->tabWidget->addTab(qtContactCallListWidget, userName);
	/************************************************************************/
	//_ui->tabWidget->addTab(qtContactCallListWidget, tr("Call"));
	_dialTabWidget->setCurrentWidget(qtContactCallListWidget);
	qtContactCallListWidget->addPhoneCall(qtPhoneCall);

	_ui->callBar->setEnabledHangUpButton(true);

	if (qtPhoneCall->isIncoming()) {
		_ui->callBar->setEnabledCallButton(true);
	}
}

void QtWengoPhone::addToConference(QString phoneNumber, PhoneCall * targetCall) {
	//FIXME conference has to be 100% rewritten...
	/*bool conferenceAlreadyStarted = false;

	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile()) {
		int nbtab = _ui->tabWidget->count();

		for (int i = 0; i < nbtab; i++) {
			if (_ui->tabWidget->tabText(i) == QString(tr("Conference"))) {
				return;
			}

			for (int j = 0; j < _ui->tabWidget->count(); j++) {
				QtContactCallListWidget * qtContactCallListWidget = dynamic_cast<QtContactCallListWidget *>(_ui->tabWidget->widget(j));
				if (qtContactCallListWidget) {
					if (qtContactCallListWidget->hasPhoneCall(targetCall)) {
						_ui->tabWidget->setTabText(j, tr("Conference"));
						IPhoneLine * phoneLine = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().getActivePhoneLine();

						if (phoneLine != NULL) {
							if (!conferenceAlreadyStarted) {
								conferenceAlreadyStarted = true;
								ConferenceCall * confCall = new ConferenceCall(*phoneLine);
								confCall->addPhoneCall(*targetCall);
								confCall->addPhoneNumber(phoneNumber.toStdString());
							}
						} else {
							LOG_DEBUG("phoneLine is NULL");
						}
					}
				}
			}
		}
	}*/

	if (!_cWengoPhone.getCUserProfileHandler().getCUserProfile())
		return;

	IPhoneLine * phoneLine = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().getActivePhoneLine();
	if(!phoneLine)
		return;

	//for (int i = 0; i < _ui->tabWidget->count(); i++)                   //deleted by huixin.du 2010-09-14
	//{
	//      if (_ui->tabWidget->tabText(i) == QString(tr("Conference")))
	//		return;
	//}

	ConferenceCall * confCall = new ConferenceCall(*phoneLine);
	targetCall->setConferenceCall(confCall);

	PhoneCall * phoneCall2 = confCall->getPhoneCall(phoneNumber.toStdString());

	QtContactCallListWidget * qtContactCallListWidgetTarget = 0;
	QtPhoneCall * qtPhoneTarget = 0;
	QtPhoneCall * qtPhoneCall2 = 0;
	int index2 = -1;

//	for (int j = 0; j < _ui->tabWidget->count(); j++)                  //deleted by huixin.du 2010-09-14
//	{
		QtContactCallListWidget * qtContactCallListWidget = dynamic_cast<QtContactCallListWidget *>(_ui->tabWidget);//->widget(j));
		if (qtContactCallListWidget)
		{
			if (!qtContactCallListWidgetTarget && qtContactCallListWidget->hasPhoneCall(targetCall))
			{
//				_ui->tabWidget->setTabText(j, tr("Conference"));       //deleted by huixin.du 2010-09-14

				qtContactCallListWidgetTarget = qtContactCallListWidget;
				qtPhoneTarget = qtContactCallListWidgetTarget->takeQtPhoneCall(targetCall);
			}
			else if(!qtPhoneCall2 && qtContactCallListWidget->hasPhoneCall(phoneCall2))
			{
				qtPhoneCall2 = qtContactCallListWidget->takeQtPhoneCall(phoneCall2);
//				index2 = j;

			}
		}
//	}

	if( qtContactCallListWidgetTarget)
	{
		if(qtPhoneCall2)
		{
			qtPhoneCall2->hideConfButton();
			qtContactCallListWidgetTarget->addPhoneCall(qtPhoneCall2);     //modified by huixin.du 2010-09-13
//			_ui->tabWidget->removeTab(index2);
		}
		if(qtPhoneTarget)
		{
			qtPhoneTarget->hideConfButton();
			qtContactCallListWidgetTarget->addPhoneCall(qtPhoneTarget);
		}
	}

	QString address = QString::fromStdString(targetCall->getPeerSipAddress().getSipAddress());

	if(address.contains("@"))
		address = address.section("@",0,0);

	if (address.startsWith("sip:")) {
		address = address.mid(4);
	}

	confCall->addPhoneNumber(address.toStdString());
	confCall->addPhoneNumber(phoneNumber.toStdString());

	if(phoneCall2)
		phoneCall2->setConferenceCall(confCall);

	if(qtContactCallListWidgetTarget)
		qtContactCallListWidgetTarget->slotStartedTalking(qtPhoneTarget);
}

void QtWengoPhone::addToConference(QtPhoneCall * qtPhoneCall) {

	QtContactCallListWidget * qtContactCallListWidget;
	_ui->tabWidget->show();_ui->dialWidget->hide();
//	int nbtab = _ui->tabWidget->count();                         //modified by huixin.du 2010-09-13

//	for (int i = 0; i < nbtab; i++) {
//		if (_ui->tabWidget->tabText(i) == QString(tr("Conference"))) {
			//i is the index of the conference tab
			qtContactCallListWidget = (QtContactCallListWidget *) _ui->tabWidget;//->widget(i);
			qtContactCallListWidget->addPhoneCall(qtPhoneCall);
//			_ui->tabWidget->setCurrentWidget(qtContactCallListWidget);
			return;
//		}
//	}

	_activeTabBeforeCall = _ui->tabWidget;//->currentWidget();            //modified by huixin.du 2010-09-13

	//conference tab not found, create a new one                          //modified by huixin.du 2010-09-13
	qtContactCallListWidget = new QtContactCallListWidget(_cWengoPhone,_wengoPhoneWindow);
	SAFE_CONNECT(qtContactCallListWidget, SIGNAL(aboutToClose()), SLOT(setActiveTabBeforeCallCurrent()) );
	qtContactCallListWidget->setParent(_ui->tabWidget);//_ui->tabWidget->addTab(qtContactCallListWidget, tr("Conference"));
//	_ui->tabWidget->setCurrentWidget(qtContactCallListWidget);
	qtContactCallListWidget->addPhoneCall(qtPhoneCall);
	_ui->callBar->setEnabledHangUpButton(true);
}

/************************************************************************/
/*�������*/
void QtWengoPhone::addToConference()
{   //modified by huixin.du 2010-09-13
//	int nbtab = _ui->tabWidget->count();
//	_ui->tabWidget->show();_ui->dialWidget->hide();
//	for (int i = 0; i < nbtab; i++) {
//		if (_ui->tabWidget->tabText(i) == QString(tr("Conference"))) {
			//i is the index of the conference tab
//			_ui->tabWidget->setCurrentWidget(_ui->tabWidget->widget(i));
//			return;
//		}
//	}
	m_qtConferenceRoomWidget = new QtConferenceRoomWidget(*this);
	m_qtConferenceRoomWidget->setParent(_ui->tabWidget);//_ui->tabWidget->addTab((QWidget*)m_qtConferenceRoomWidget, tr("Conference"));
	//_ui->tabWidget->setCurrentWidget(m_qtConferenceRoomWidget);

	//getQtProfileBar()->getQtIMProfileWidget()->getWidget()->setVisible(false);
	getQtProfileBar()->nicknameClicked();
	//_wengoPhoneWindow->showMaximized();
	getQtProfileBar()->hide();
	getQtCallBar().hide();
}
/*ɾ������*/
void QtWengoPhone::deleteConference()
{
//	int nbtab = _ui->tabWidget->count();
//	for (int i = 0; i < nbtab; i++) {
//		if (_ui->tabWidget->tabText(i) == QString(tr("Conference"))) {
			//i is the index of the conference tab
//	_ui->tabWidget->hide();_ui->dialWidget->show();//_ui->tabWidget->removeTab(i);
//			break;
//		}
//	}
	OWSAFE_DELETE(m_qtConferenceRoomWidget);
	//getQtProfileBar()->getQtIMProfileWidget()->getWidget()->setVisible(true);
	getQtProfileBar()->nicknameClicked();
	getQtProfileBar()->show();
	getQtCallBar().show();
}
/************************************************************************/

QtFileTransfer * QtWengoPhone::getFileTransfer() const {
	return _qtFileTransfer;
}

void QtWengoPhone::updatePresentation() {
	typedef PostEvent0<void ()> MyPostEvent;
	MyPostEvent * event = new MyPostEvent(boost::bind(&QtWengoPhone::updatePresentationThreadSafe, this));
	postEvent(event);
}

void QtWengoPhone::updatePresentationThreadSafe() {
	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile()) {
		//disabled some actions if no SIP Account is used
		bool hasSipAccount = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().hasSipAccount();

		_ui->actionShowWengoAccount->setEnabled(_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().hasWengoAccount());
		_ui->actionSendSms->setEnabled(hasSipAccount);
		_ui->actionCreateConferenceCall->setEnabled(hasSipAccount);
		_ui->actionSearchWengoContact->setEnabled(hasSipAccount);
	}
}

void QtWengoPhone::dialpad(const std::string & tone) {
	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile()) {
		_ui->callBar->setPhoneComboBoxEditText(_ui->callBar->getPhoneComboBoxCurrentText() + tone);
	}
}

void QtWengoPhone::prepareToExitApplication() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	// check for pending calls
	CUserProfile *cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();
	if (cUserProfile) {
		CPhoneLine *cPhoneLine = cUserProfile->getCPhoneLine();
		if (cPhoneLine) {
			if (cPhoneLine->hasPendingCalls()) {

				if (QMessageBox::question(
					getWidget(),
					tr("@product@ - Warning"),
					tr("You have unfinished call(s).") + "\n" +
						tr("Are you sure you want to exit the application?"),
					tr("&Exit"),
					tr("&Cancel")
				) == 1)  {
					return;
				}
			}
		}
	}
	////


	_qtSystray->hide();

	//Save the window size
	QSize winsize = _wengoPhoneWindow->size();
	config.set(Config::PROFILE_WIDTH_KEY, winsize.width());
	config.set(Config::PROFILE_HEIGHT_KEY, winsize.height());

	//Save the window position
	QPoint winpos = _wengoPhoneWindow->pos();
	config.set(Config::PROFILE_POSX_KEY, winpos.x());
	config.set(Config::PROFILE_POSY_KEY, winpos.y());

	QApplication::closeAllWindows();
	QCoreApplication::processEvents();

	//destroyed chatWindow so that chats are saved
	OWSAFE_DELETE(_chatWindow);
	////

	_cWengoPhone.terminate();
}

void QtWengoPhone::phoneComboBoxClicked() {

	_ui->callBar->clearPhoneComboBox();
	_ui->callBar->clearPhoneComboBoxEditText();
}

void QtWengoPhone::exitEvent() {
	QCoreApplication::exit(EXIT_SUCCESS);
}

void QtWengoPhone::showHistory() {
//	_ui->tabWidget->setCurrentWidget(_ui->tabHistory);
}

void QtWengoPhone::currentUserProfileWillDieEventHandlerSlot() {
	//Signal for re-initializing graphical components
	//when there is no user profile anymore
	userProfileDeleted();

	// Reset _qtIMAccountMonitor so that it does not emit signals anymore
	_qtIMAccountMonitor.reset(0);

	OWSAFE_DELETE(_qtFileTransfer);
	OWSAFE_DELETE(_qtIdle);
	_ui->profileBar->reset();

	//if (_qtContactList) {
	//	_ui->tabContactList->layout()->removeWidget(_qtContactList->getWidget());
	//	OWSAFE_DELETE(_qtContactList);
	//}

	//if (_qtHistoryWidget) {
	//	_ui->tabHistory->layout()->removeWidget(_qtHistoryWidget->getWidget());
	//	OWSAFE_DELETE(_qtHistoryWidget);
	//}
	if(m_qtConferenceRoomWidget)
	{
		_ui->tabDialpad->layout()->removeWidget(m_qtConferenceRoomWidget);
		OWSAFE_DELETE(m_qtConferenceRoomWidget);
	}

	uninstallQtBrowserWidget();
}

void QtWengoPhone::userProfileInitializedEventHandlerSlot() {
	CUserProfile * cUserProfile = _cWengoPhone.getCUserProfileHandler().getCUserProfile();

	//if( !(cUserProfile->getUserProfile().isConnected()))
	//{
	//
	//	SipAccount* sipAccount = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().getSipAccount()->clone();
	//	PUserProfileHandler* pUserprofileHandler = _cWengoPhone.getCUserProfileHandler().getPresentation();
	//	pUserprofileHandler->sipAccountConnectionFailedEventHandler( *sipAccount, EnumSipLoginState::SipLoginStatePasswordError );

	//	return;
	//}
	//Idle detection
	//FIXME: QtIdle must not use UserProfile but CUserProfile
	_qtIdle = new QtIdle(cUserProfile->getUserProfile(), _wengoPhoneWindow);

	// Create account monitor
	_qtIMAccountMonitor.reset( new QtIMAccountMonitor(0, cUserProfile) );

	//Create the profilebar
	_ui->profileBar->init(&_cWengoPhone, cUserProfile, _qtIMAccountMonitor.get());


	// Systray
	SAFE_CONNECT_RECEIVER_TYPE(_qtIMAccountMonitor.get(), SIGNAL(imAccountAdded(QString)),
		_qtSystray, SLOT(updateSystrayIcon()), Qt::QueuedConnection);
	SAFE_CONNECT_RECEIVER_TYPE(_qtIMAccountMonitor.get(), SIGNAL(imAccountUpdated(QString)),
		_qtSystray, SLOT(updateSystrayIcon()), Qt::QueuedConnection);
	SAFE_CONNECT_RECEIVER_TYPE(_qtIMAccountMonitor.get(), SIGNAL(imAccountRemoved(QString)),
		_qtSystray, SLOT(updateSystrayIcon()), Qt::QueuedConnection);

	_qtSystray->setTrayMenu();
	_qtSystray->updateSystrayIcon();


	if( _qtFileTransfer==NULL )		//��ֹ�ڴ�й¶......zhenHua.sun 2010-12-06
		_qtFileTransfer = new QtFileTransfer(this, _cWengoPhone.getWengoPhone().getCoIpManager());

	//menu
	_qtToolBar->userProfileIsInitialized();

	//�����������
	if( m_qtConferenceRoomWidget==NULL )	//��ֹ�ڴ�й¶......zhenHua.sun 2010-12-06
	{
		m_qtConferenceRoomWidget = new QtConferenceRoomWidget(*this);
		//_ui->tabWidget->addTab((QWidget*)m_qtConferenceRoomWidget, tr("Conference"));
		//_ui->tabWidget->setCurrentWidget(m_qtConferenceRoomWidget);
		if (!_ui->tabDialpad->layout()) {
			Widget::createLayout(_ui->tabDialpad);
		}
		m_qtConferenceRoomWidget->setParent(_ui->tabDialpad);
		_ui->tabDialpad->layout()->addWidget(m_qtConferenceRoomWidget);
		/********huixin.du  2010-09-16**********************/
		connect(m_qtConferenceRoomWidget->_tools->_ui->callingButton,SIGNAL(clicked()),this,SLOT(hideDialWidget()));
		//connect(m_qtConferenceRoomWidget->_tools->_ui->callingButton,SIGNAL(clicked()),_ui->dialWidget,SLOT(hideDialWidget()));
		connect(m_qtConferenceRoomWidget->_tools->_ui->conferenceButton,SIGNAL(clicked()),_ui->dialWidget,SLOT(hide()));
		connect(m_qtConferenceRoomWidget->_tools->_ui->conferenceButton,SIGNAL(clicked()),_ui->tabWidget,SLOT(show()));

	#ifdef DBUS_ENABLE
		connect(m_qtConferenceRoomWidget->_tools->_ui->conferenceButton,SIGNAL(clicked()),_wengoPhoneWindow,SLOT(hide()));
	#endif
	}

	//�������Ϊ�쳣�����µ�½����ôӦ�����½������...zhenHua.sun 2010-12-06
	ConferenceSession * confSession = RunningProfile::getInstance().getSessionManager().getConfSession();
	if( confSession )
	{
		//�����˳������쳣֮ǰ����Ļ���
		QtWengoPhone::ActionQuitMeetingSignal();

		Sleep(1000);

		//Ȼ�����¼������
		QString msg = QString::fromStdString(confSession->_focusUI) + "\r\n" + QString::fromStdString(confSession->_password);
		QtWengoPhone::ActionEnterMeetingSignal( msg );
	}
		


	/***************************************************/
	//Widget::createLayout(_ui->tabDialpad)->addWidget(m_qtConferenceRoomWidget);
	//m_qtConferenceRoomWidget->show();

	QtWengoPhone::SetUIState(UI_S_LOGIN);//Add By LZY 2010-09-30 ����֪ͨң��������״̬�仯
}

void QtWengoPhone::proxyNeedsAuthenticationEventHandler(NetworkProxyDiscovery & sender, NetworkProxy networkProxy) {
	typedef PostEvent1<void (NetworkProxy networkProxy), NetworkProxy> MyPostEvent;
	MyPostEvent * event =
		new MyPostEvent(boost::bind(&QtWengoPhone::proxyNeedsAuthenticationEventHandlerThreadSafe, this, _1), networkProxy);
	postEvent(event);
}

void QtWengoPhone::wrongProxyAuthenticationEventHandler(NetworkProxyDiscovery & sender, NetworkProxy networkProxy) {
	typedef PostEvent1<void (NetworkProxy networkProxy), NetworkProxy> MyPostEvent;
	MyPostEvent * event =
		new MyPostEvent(boost::bind(&QtWengoPhone::proxyNeedsAuthenticationEventHandlerThreadSafe, this, _1), networkProxy);
	postEvent(event);
}

void QtWengoPhone::proxyNeedsAuthenticationEventHandlerThreadSafe(NetworkProxy networkProxy) {
	static QtHttpProxyLogin * httpProxy =
		new QtHttpProxyLogin(getWidget(),
			networkProxy.getServer(), networkProxy.getServerPort());

	int ret = httpProxy->show();

	if (ret == QDialog::Accepted) {
		NetworkProxy myNetworkProxy;
		myNetworkProxy.setServer(httpProxy->getProxyAddress());
		myNetworkProxy.setServerPort(httpProxy->getProxyPort());
		myNetworkProxy.setLogin(httpProxy->getLogin());
		myNetworkProxy.setPassword(httpProxy->getPassword());

		NetworkProxyDiscovery::getInstance().setProxySettings(myNetworkProxy);
	}
}

void QtWengoPhone::closeWindow() {
	_wengoPhoneWindow->hide();
}

#if defined(OS_MACOSX)
void QtWengoPhone::fixMacOSXMenus() {
	// Avoids translation of these menus on Mac OS X. Thus Qt
	// will put these under the Application menu
	_ui->actionShowConfig->setText("Preferences");
	_ui->actionShowAbout->setText("About");
}
#endif

void QtWengoPhone::languageChanged() {
	LOG_DEBUG("retranslate main window ui");
	_ui->retranslateUi(_wengoPhoneWindow);
	_qtToolBar->retranslateUi();
#if defined(OS_MACOSX)
	fixMacOSXMenus();
#endif

#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	if (config.getIEActiveXEnable() && _qtBrowserWidget) {
		_ui->tabWidget->setTabText(_qtBrowserWidgetTabIndex, tr("Home"));
	}
#endif

	//if (_qtHistoryWidget) {
	//	_qtHistoryWidget->retranslateUi();
	//}	zhenHua.sun 2010-10-27
}

void QtWengoPhone::showAddContact(ContactInfo contactInfo) {

	ensureVisible();

	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile()) {

		//FIXME this method should not be called if no UserProfile has been set
		ContactProfile contactProfile;
		QtProfileDetails qtProfileDetails(*_cWengoPhone.getCUserProfileHandler().getCUserProfile(),
			contactProfile, _wengoPhoneWindow, tr("Add a Contact"));

		//FIXME to remove when wdeal will be able to handle SIP presence
		if (contactInfo.group == "WDeal") {
			qtProfileDetails.setHomePhone(QUrl::fromPercentEncoding(QByteArray(contactInfo.sip.c_str())));
		} else {
			qtProfileDetails.setWengoName(QUrl::fromPercentEncoding(QByteArray(contactInfo.wengoName.c_str())));
		}
		///

		if (contactInfo.group == "WDeal") {
			qtProfileDetails.setFirstName(QUrl::fromPercentEncoding(QByteArray(contactInfo.wdealServiceTitle.c_str())));
		} else {
			qtProfileDetails.setFirstName(QUrl::fromPercentEncoding(QByteArray(contactInfo.firstname.c_str())));
		}

		qtProfileDetails.setLastName(QUrl::fromPercentEncoding(QByteArray(contactInfo.lastname.c_str())));
		qtProfileDetails.setCountry(QUrl::fromPercentEncoding(QByteArray(contactInfo.country.c_str())));
		qtProfileDetails.setCity(QUrl::fromPercentEncoding(QByteArray(contactInfo.city.c_str())));
		qtProfileDetails.setState(QUrl::fromPercentEncoding(QByteArray(contactInfo.state.c_str())));
		qtProfileDetails.setGroup(QUrl::fromPercentEncoding(QByteArray(contactInfo.group.c_str())));
		qtProfileDetails.setWebsite(QUrl::fromPercentEncoding(QByteArray(contactInfo.website.c_str())));

		if (qtProfileDetails.show()) {
			_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCContactList().addContact(contactProfile);
		}
	}
}

void QtWengoPhone::bringMainWindowToFront() {
	ensureVisible();
}

void QtWengoPhone::ensureVisible() {
	_wengoPhoneWindow->activateWindow();
	_wengoPhoneWindow->showNormal();
	_wengoPhoneWindow->raise();
}

void QtWengoPhone::installQtBrowserWidget() {
	_qtBrowserWidget = NULL;
	/** ����  zhenHua.sun 2010-10-29
	_qtBrowserWidget = new QtBrowserWidget(*this);
#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	if (config.getIEActiveXEnable()) {
		_qtBrowserWidgetTabIndex = _ui->tabWidget->addTab(_qtBrowserWidget->getWidget(), tr("Home"));
	}
#else
	//_ui->tabWidget->show();_ui->dialWidget->hide();//_ui->tabWidget->setCurrentIndex(0);
#endif
	*/
	
}

void QtWengoPhone::uninstallQtBrowserWidget() {
#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	if (config.getIEActiveXEnable() && _qtBrowserWidget) {
		_ui->tabWidget->widget(_qtBrowserWidgetTabIndex)->layout()->removeWidget(_qtBrowserWidget->getWidget());
		_ui->tabWidget->removeTab(_qtBrowserWidgetTabIndex);
		OWSAFE_DELETE(_qtBrowserWidget);
	}
#endif
}

void QtWengoPhone::setCurrentUserProfileEventHandlerSlot()
{
	_qtToolBar->tryingToConnect();
}

QtConferenceRoomWidget * QtWengoPhone::getQtConferenceRoomWidget()
{
	return m_qtConferenceRoomWidget;
}


/**
* Add by: hexianfu
* for the telecontroller
* 2009-10-21
*/
char *QtWengoPhone::getPrivateMeetPwd()
{
	return privateMeetingPwd;
}

QtLoginDialog & QtWengoPhone::getQtLoginDialog() const
{
	return *_qtLoginDialog;
}

void QtWengoPhone::setQtLoginDialog(QtLoginDialog * qtLoginDialog)
{
	_qtLoginDialog = qtLoginDialog;
}


void QtWengoPhone::setQtToolWidget( QtToolWidget* qtToolWidget )
{
	_qtToolWidget = qtToolWidget;
}
QtToolWidget* QtWengoPhone::getQtToolWidget() const
{
	return _qtToolWidget;
}

/*
 * Add by huixin.du for delete conference tab  2010-09-14
 * �رպ��н��棬���ػ������
 */
void QtWengoPhone::hideDialWidget()
{

	if(_dialTabWidget->count() == 0)
	{
		_ui->dialWidget->hide();
		_ui->tabWidget->show();		
#ifdef DBUS_ENABLE
		_wengoPhoneWindow->hide();	//zhenHua.sun
#endif
	}
	else
	{
		_ui->tabWidget->hide();
		_ui->dialWidget->show();
#ifdef DBUS_ENABLE
		_wengoPhoneWindow->show();	//zhenHua.sun
#endif
	}

}

//-------Add By LZY 2010-09-27-------
/*
 * ����µ�������Ϣ��ָ�뵽��Ϣ��ָ�������
 * @param QMessageBox *MsgBoxPointer,Ҫ�����б��е���Ϣ��ָ��
 * @return int,���ؼ������Ϣ���ڶ����е�Ψһʶ��ţ��Ƴ�ʱ��Ҫ�õ���ʶ���
 */
int QtWengoPhone::AddInviteMsg(QMessageBox *MsgBoxPointer)
{
	long LockNum=(long)GetCurrentThreadId();
	QtWengoPhone::LockInviteList(LockNum);
	int LoopVar;
	for (LoopVar=0;LoopVar<QtWengoPhone::InviteListCount;++LoopVar)
	{
		if (QtWengoPhone::InviteList[LoopVar].MsgPointer==NULL)
		{
			QtWengoPhone::InviteList[LoopVar].MsgPointer=MsgBoxPointer;
			QtWengoPhone::InviteList[LoopVar].IdentifyNum=QtWengoPhone::InviteListNextIdentify;
			++QtWengoPhone::InviteListNextIdentify;
			if (QtWengoPhone::InviteListNextIdentify==0)
				QtWengoPhone::InviteListNextIdentify=1;
			break;
		}
	}
	if (LoopVar>=QtWengoPhone::InviteListCount)
	{
		LoopVar=QtWengoPhone::InviteListCount;
		QtWengoPhone::InviteListCount+=5;
		QtWengoPhone::InviteList=(QtWengoPhone::InviteListNode *)realloc(QtWengoPhone::InviteList,sizeof(QtWengoPhone::InviteListCount));
		QtWengoPhone::InviteList[LoopVar].MsgPointer=MsgBoxPointer;
		QtWengoPhone::InviteList[LoopVar].IdentifyNum=QtWengoPhone::InviteListNextIdentify;
		++QtWengoPhone::InviteListNextIdentify;
		if (QtWengoPhone::InviteListNextIdentify==0)
			QtWengoPhone::InviteListNextIdentify=1;
		for (int LoopVar2=LoopVar+1;LoopVar2<QtWengoPhone::InviteListCount;++LoopVar2)
		{
			QtWengoPhone::InviteList[LoopVar2].IdentifyNum=0;
			QtWengoPhone::InviteList[LoopVar2].MsgPointer=NULL;
		}
	}
	QtWengoPhone::UnLockInviteList(LockNum);
	return QtWengoPhone::InviteList[LoopVar].IdentifyNum;
}

/*
 * �Ƴ�ָ������λ�õ���Ϣ��ָ�룬���WithAcceptΪtrue�������Ƴ�ǰִ�и���Ϣ���accept����
 * @param int IdentifyNum,Ψһʶ��ţ����䶨λ�б��еĶԻ���
 * @param int AdditionMethod=0,��λ�Ի���ɹ���ĸ��Ӳ�����0-�޲��� >0-accept() <0-reject()
 * @return void
 */
void QtWengoPhone::RemoveInviteMsg(int IdentifyNum,int AdditionMethod)
{
	long LockNum=(long)GetCurrentThreadId();
	QtWengoPhone::LockInviteList(LockNum);
	for (int LoopVar=0;LoopVar<QtWengoPhone::InviteListCount;++LoopVar)
	{
		if (QtWengoPhone::InviteList[LoopVar].IdentifyNum==IdentifyNum)
		{
			if (QtWengoPhone::InviteList[LoopVar].MsgPointer!=NULL)
			{
				QtWengoPhone::sendTeleStateChanged(CHANGE_INVITE,"\r\n");
				if (AdditionMethod>0)
					QtWengoPhone::InviteList[LoopVar].MsgPointer->accept();
				else if (AdditionMethod<0)
					QtWengoPhone::InviteList[LoopVar].MsgPointer->reject();
				QtWengoPhone::InviteList[LoopVar].MsgPointer=NULL;
				break;
			}
		}
	}
	QtWengoPhone::UnLockInviteList(LockNum);
}

/*
 * �Ƴ����е�ǰ������ʾ����Ϣ������ȷ�ϣ�ע��ȷ�ϵȣ���������Ϣ��ִ��reject����
 * @return void
 */
void QtWengoPhone::RemoveAllMsg()
{
	//�Ƴ������
	long LockNum=(long)GetCurrentThreadId();
	QtWengoPhone::LockInviteList(LockNum);
	for (int LoopVar=0;LoopVar<QtWengoPhone::InviteListCount;++LoopVar)
	{
		if (QtWengoPhone::InviteList[LoopVar].MsgPointer!=NULL)
			QtWengoPhone::InviteList[LoopVar].MsgPointer->reject();
	}
	QtWengoPhone::UnLockInviteList(LockNum);
	//�Ƴ�������ʾ��
	;
}

/*
 * InviteList�����������������
 * @param long LockNum,����ֵ������ʱ���ṩ��ͬ��ֵ���ܽ����ɹ�
 * @return void
 */
void QtWengoPhone::LockInviteList(long LockNum)
{
	while (QtWengoPhone::InviteListLockNum!=LockNum)
	{
		if (QtWengoPhone::InviteListLockNum==0)
			QtWengoPhone::InviteListLockNum=LockNum;
		Sleep(15);
	}
}

/*
 * InviteList�����������������
 * @param long UnLockNum,����ֵ���ṩ��ֵ���������ʱ�õ�ֵһ�²��ܽ����ɹ�
 * @return bool,���ؽ������
 */
bool QtWengoPhone::UnLockInviteList(long UnLockNum)
{
	if (QtWengoPhone::InviteListLockNum==UnLockNum)
	{
		QtWengoPhone::InviteListLockNum=0;
		return true;
	}
	else
		return false;
}
//-------End of Add By LZY-----------

//-------Add By LZY ��2010-09-19------
/*
 * 2010-11-02 ������Ϣ��teleController��Ŀǰʹ��DBus���ƹ����ͣ�����Ҫ��
 * @param char *Message,ָ��Ҫ�������ݵ�ָ��
 * @return void
 */
void QtWengoPhone::SendInfo(char *Message)
{
	CvTelecontrollerClient::getInstance()->TeleInfo( QString::fromUtf8(Message) );
}

void QtWengoPhone::commandProcess(const char *recvedMess)
{
	CMessageFrame recvedMessFram(recvedMess);
	char CommandName[50];//������������
	int TLIndex;//��Ŷ�����teleController�����б��е�������
	int TLAvailable;//��Ŷ�����teleController�����б��е���Ч���ֵ
	//----BG �������ض�����Ϣ����ؿ��Ʊ���----
	bool ResponseTele=false;//�Ƿ��ں���ĩ��teleController���ͷ�����Ϣ�������������صĶ�����Ӧ�ðѴ�ֵ����Ϊtrue
	CMessageFrame *ResponseBuffer=NULL;//������Ϣ������ָ�룬��������Ҫ����teleController������Ϣʱ���緵�ػ����б�ȣ����Ѵ�ָ��ʵ����
	int RunResult=ARESULT_SUCCESS;//����ִ�н�������ͷ�����Ϣʱ�ã���ֵ���忴QtWengoPhone.h�ļ����ԡ�ARESULT����ͷ��Ԥ����ֵ˵��
	//----ED �������ض�����Ϣ����ر���----

	recvedMessFram.readLine(false);//��1����Ϣ��ǣ�Ŀǰ��Ϊ�������Ķ��ǡ�Action�������Բ�������ļ��
	TLIndex=atoi(recvedMessFram.readLine(true));//��2����ӦteleController�������յȴ��б��������
	TLAvailable=atoi(recvedMessFram.readLine(true));//��3����ӦteleController�������յȴ��б����Ч���ֵ
	strcpy(&(CommandName[0]),recvedMessFram.readLine(true));//��4�������������临�Ƶ���������������
	//------��ʼ_�����������--------
	if(strcmp(CommandName,"Login")==0)//�Ӻ���ӦAction
	{
		if (QtWengoPhone::CheckDelayAction(CommandName,UI_S_LOGOFF,ACTION_LOGIN,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			QtWengoPhone::SetUIState(UI_S_LOGIN_BEGIN);//����֪ͨң��������״̬�仯
			QtWengoPhone::ActionLoginSignal(QString::fromUtf8(recvedMessFram.readLast()));//�����ź�
		}
	}
	else if(strcmp(CommandName,"GetMeetingList")==0)//������ӦAction
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING|UI_S_LOGIN,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			CUserProfile *cUserProfile = QtWengoPhone::_cWengoPhone.getCUserProfileHandler().getCUserProfile();
			vector<ConfInfo *> currentMeetingList = cUserProfile->getCurrentConfInfoList();
			ResponseBuffer=new CMessageFrame();
			int confListSize = currentMeetingList.size();
			for (int LoopVar=0;LoopVar<confListSize;++LoopVar)
			{
				ResponseBuffer->Append((currentMeetingList[LoopVar]->Title).c_str());
				ResponseBuffer->Append((currentMeetingList[LoopVar]->URI).c_str());
				ResponseBuffer->Append((currentMeetingList[LoopVar]->ControlMode).c_str());
				ResponseBuffer->Append((currentMeetingList[LoopVar]->HostURI).c_str());
				if(currentMeetingList[LoopVar]->JoinMode == "password")
					ResponseBuffer->Append("P");
				else
					ResponseBuffer->Append("N");
			}
		}
	}
	else if (strcmp(CommandName,"RefreshMeetingList")==0)//������ӦAction
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_LOGIN|UI_S_MEETING,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			QtWengoPhone::ActionRefreshMeetingListSignal();//�����ź�
		}
	}
	else if(strcmp(CommandName,"EnterMeeting")==0)//�Ӻ���ӦAction
	{
		if (QtWengoPhone::CheckDelayAction(CommandName,UI_S_LOGIN,ACTION_ENTERMEETING,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			QtWengoPhone::SetUIState(UI_S_JOINMEETING_BEGIN);//����֪ͨң��������״̬�仯
			QtWengoPhone::ActionEnterMeetingSignal(QString::fromUtf8(recvedMessFram.readLast()));//�����ź�
		}
	}
	else if(strcmp(CommandName,"GetMemberList")==0)//������ӦAction
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			CUserProfile *cUserProfile = QtWengoPhone::_cWengoPhone.getCUserProfileHandler().getCUserProfile();
			vector<MemberInfo *> currentMemberList = cUserProfile->getCurrentMemberInfoList();
			ResponseBuffer=new CMessageFrame();
			int Size = currentMemberList.size();
			int index;//λ������ֵ
			string nameAbstract;//����������û���
			MemberInfo *member;
			for (int LoopVar=0;LoopVar<Size;++LoopVar)
			{
				member = currentMemberList[LoopVar];
				index = member->MemberURI.find('@',0);
				nameAbstract=member->MemberURI;
				if (index!=std::string::npos)
					nameAbstract = nameAbstract.substr(0,index);
				ResponseBuffer->Append(nameAbstract.c_str());
				ResponseBuffer->Append(member->permitSpeak);
				if((member->Sdp).find("m=video")!=std::string::npos)
					ResponseBuffer->Append(1);
				else
					ResponseBuffer->Append(0);
				ResponseBuffer->Append(member->hand);
			}
		}
	}
	else if(strcmp(CommandName,"ContactMember")==0)//�Ӻ���ӦAction
	{
		if (QtWengoPhone::CheckDelayAction(CommandName,UI_S_MEETING,ACTION_CONTACTMEMBER,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
			QtWengoPhone::ActionContactMemberSignal(QString::fromUtf8(recvedMessFram.readLine(true)));//�����ź�
	}
	else if(strcmp(CommandName,"StopMember" ) == 0)//������ӦAction
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			QtWengoPhone::ActionStopMemberSignal(QString::fromUtf8(recvedMessFram.readLine(true)));//�����ź�
		}
	}
	else if(strcmp(CommandName,"QuitMeeting")==0)//�Ӻ���ӦAction
	{
		if (QtWengoPhone::CheckDelayAction(CommandName,UI_S_MEETING,ACTION_QUITMEETING,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			QtWengoPhone::SetUIState(UI_S_EXITMEETING_BEGIN);//����֪ͨң��������״̬�仯
			QtWengoPhone::ActionQuitMeetingSignal();//�����ź�
		}
	}
	else if(strcmp(CommandName,"LogOff") == 0)//�Ӻ���ӦAction
	{
		if (QtWengoPhone::CheckDelayAction(CommandName,UI_S_MEETING|UI_S_LOGIN,ACTION_LOGOFF,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			QtWengoPhone::SetUIState(UI_S_LOGOFF_BEGIN);//����֪ͨң��������״̬�仯
			QtWengoPhone::ActionLogOffSignal();//�����ź�
		}
	}
	else if(strcmp(CommandName,"ShutDown") == 0)//������ӦAction
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_ALL,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			system("shutdown -s -t 120");
		}
	}
	else if(strcmp(CommandName, "LocalControl") == 0)//������ӦAction
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING|UI_S_LOGIN,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			QtWengoPhone::ActionLocalControlSignal();//�����ź�
		}
	}
	else if (strcmp(CommandName, "GetVolume") == 0)//������ӦAction
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING|UI_S_LOGIN,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			StringList deviceId;
			ResponseBuffer=new CMessageFrame();
			if (strcmp(recvedMessFram.readLine(true),"out") == 0)
			{
				ResponseBuffer->Append("out");
				deviceId = ConfigManager::getInstance().getCurrentConfig().getAudioOutputDeviceId();
			}
			else
			{
				ResponseBuffer->Append("in");
				deviceId = ConfigManager::getInstance().getCurrentConfig().getAudioInputDeviceId();
			}
			AudioDevice audioDevice(deviceId);
			VolumeControl volumeControl(audioDevice);
			ResponseBuffer->Append(volumeControl.getLevel());
		}
	}
	else if(strcmp(CommandName,"SetVolume")==0)//������ӦAction
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING|UI_S_LOGIN,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			if (strcmp(recvedMessFram.readLine(true),"out")==0)
				QtWengoPhone::ActionSetVolumeSignal(atoi(recvedMessFram.readLine(true)),1);//Output�豸������������������
			else
				QtWengoPhone::ActionSetVolumeSignal(atoi(recvedMessFram.readLine(true)),0);//Input�豸����˷磩��������
		}
	}
	else if (strcmp(CommandName, "GetGlobalService") == 0)//������ӦAction
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING|UI_S_LOGIN,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			Config & config = ConfigManager::getInstance().getCurrentConfig();
			ResponseBuffer=new CMessageFrame();
			ResponseBuffer->Append(config.getGlobalService().c_str());  //v4
			ResponseBuffer->Append(config.getGlobalService().c_str());  //v6
		}
	}
	else if(strcmp(CommandName,"SetGlobalService")==0)//������ӦAction
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING|UI_S_LOGIN,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			string Addr=recvedMessFram.readLine(true);
			QtWengoPhone::SaveGlobalSetting(Addr,Addr);
		}
	}
	else if(strcmp(CommandName,"RaiseHand" ) == 0)//�Ӻ���ӦAction
	{
		if (QtWengoPhone::CheckDelayAction(CommandName,UI_S_MEETING,ACTION_RAISEHAND,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
			QtWengoPhone::ActionRaiseHandSignal();//�����ź�
	}
	else if(strcmp(CommandName,"PermitSpeak" ) == 0)//�Ӻ���ӦAction
	{
		if (QtWengoPhone::CheckDelayAction(CommandName,UI_S_MEETING,ACTION_PERMITSPEAK,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
			QtWengoPhone::ActionPermitSpeakSignal(QString::fromUtf8(recvedMessFram.readLine(true)));//�����ź�
	}
	else if(strcmp(CommandName,"KickMember" ) == 0)//�Ӻ���ӦAction
	{
		if (QtWengoPhone::CheckDelayAction(CommandName,UI_S_MEETING,ACTION_KICKMEMBER,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
			QtWengoPhone::ActionKickMemberSignal(QString::fromUtf8(recvedMessFram.readLine(true)));//�����ź�
	}
	else if(strcmp(CommandName,"SetLayoutType" ) == 0)//������ӦAction
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING|UI_S_LOGIN,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			QtWengoPhone::ActionSetLayoutTypeSignal(QString::fromUtf8(recvedMessFram.readLine(true)));//�����ź�
		}
	}
	else if(strcmp(CommandName,"InviteMembers" ) == 0)//������ӦAction
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			QtWengoPhone::ActionInviteMembersSignal(QString::fromUtf8(recvedMessFram.readLine(true)));//�����ź�
		}
	}
	else if(strcmp(CommandName,"AcceptInvite" ) == 0)//������ӦAction
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_ALL,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			int IdentifyNum=atoi(recvedMessFram.readLine(true));
			QtWengoPhone::ActionAcceptInviteSignal(IdentifyNum,atoi(recvedMessFram.readLine(true)));//�����ź�
		}
	}
	else if(strcmp(CommandName,"CreateMeeting" ) == 0)//�Ӻ���ӦAction�����͸�ʽ�ı������
	{
		if (QtWengoPhone::CheckDelayAction(CommandName,UI_S_MEETING|UI_S_LOGIN,ACTION_CREATEMEETING,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			QString Title=QString::fromUtf8(recvedMessFram.readLine(true));//������
			QString Description=QString::fromUtf8(recvedMessFram.readLine(true));//��������
			int ControlModeNum=atoi(recvedMessFram.readLine(true));//��������
			int JoinModeNum=atoi(recvedMessFram.readLine(true));//��������
			QString Psw=QString::fromUtf8(recvedMessFram.readLine(true));//����
			QtWengoPhone::ActionCreateMeetingSignal(Title,Description,ControlModeNum,JoinModeNum,Psw);//�����ź�
		}
	}
	else if (strcmp(CommandName, "LocalVideoSetting") == 0)//������ӦAction����Ӧ��Ϣ����ͨ���ն�����֪ͨʵ�֣�
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			QtWengoPhone::ActionLocalVideoSettingSignal();//�����ź�
		}
	}
	else if (strcmp(CommandName, "LocalAudioSetting") == 0)//������ӦAction����Ӧ��Ϣ����ͨ���ն�����֪ͨʵ�֣�
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			QtWengoPhone::ActionLocalAudioSettingSignal();//�����ź�
		}
	}
	else if(strcmp(CommandName,"GetContactList") == 0)//������ӦAction
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING|UI_S_LOGIN,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			if (QtWengoPhone::_qtContactList)
				ResponseBuffer=new CMessageFrame(QtWengoPhone::_qtContactList->getContacts().c_str());
			else
				RunResult=ARESULT_FORBIDDEN;
		}
	}
	else if(strcmp(CommandName,"GetContactInfo") == 0)//������ӦAction
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING|UI_S_LOGIN,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			if (QtWengoPhone::_qtContactList)
				ResponseBuffer=new CMessageFrame(QtWengoPhone::_qtContactList->getContactInfo(recvedMessFram.readLine(true)).c_str());
			else
				RunResult=ARESULT_FORBIDDEN;
		}
	}
	else if(strcmp(CommandName,"EditContact") == 0)//������ӦAction����Ӧ��Ϣ����ͨ���ն�����֪ͨʵ�֣�
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING|UI_S_LOGIN,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			string ContactId=recvedMessFram.readLine(true);//��ϵ��ID
			if (QtWengoPhone::_qtContactList)
			{
				ContactProfile ContactProfile;
				if (ContactId!="")
					ContactProfile = QtWengoPhone::_qtContactList->getCContactList().getContactProfile(ContactId);
				ContactProfile.setFirstName(recvedMessFram.readLine(true));
				ContactProfile.setLastName(recvedMessFram.readLine(true));
				//Set Group Id
				string GroupName=recvedMessFram.readLine(true);
				string GroupId = QtWengoPhone::_qtContactList->getCContactList().getContactGroupIdFromName(GroupName);
				if(GroupId == "")
				{
					QtWengoPhone::_qtContactList->getCContactList().addContactGroup(GroupName);
					GroupId = QtWengoPhone::_qtContactList->getCContactList().getContactGroupIdFromName(GroupName);
				}
				ContactProfile.setGroupId(GroupId);
				QtWengoPhone::_qtContactList->setSipAccount(recvedMessFram.readLine(true),ContactProfile);
				ContactProfile.setHomePhone(recvedMessFram.readLine(true));
				ContactProfile.setMobilePhone(recvedMessFram.readLine(true));
				ContactProfile.setPersonalEmail(recvedMessFram.readLine(true));
				StreetAddress Address;
				Address.setCity(recvedMessFram.readLine(true));
				ContactProfile.setStreetAddress(Address);
				ContactProfile.setSex((EnumSex::Sex)atoi(recvedMessFram.readLine(true)));
				QString BirthDay = recvedMessFram.readLine(true);
				QDate date = QDate::fromString(BirthDay,"yyyy-M-d");
				ContactProfile.setBirthdate(Date(date.day(),date.month(),date.year()));
				if(ContactId!="")
					QtWengoPhone::_qtContactList->getCContactList().updateContact(ContactProfile);
				else
					QtWengoPhone::_qtContactList->getCContactList().addContact(ContactProfile);
			}
			else
				RunResult=ARESULT_FORBIDDEN;
		}
	}
	else if(strcmp(CommandName,"DelContact") == 0)//������ӦAction����Ӧ��Ϣ����ͨ���ն�����֪ͨʵ�֣�
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING|UI_S_LOGIN,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			if (QtWengoPhone::_qtContactList)
				QtWengoPhone::_qtContactList->getCContactList().removeContact(recvedMessFram.readLine(true));
			else
				RunResult=ARESULT_FORBIDDEN;
		}
	}
	else if(strcmp(CommandName,"DelGroup") == 0)//������ӦAction����Ӧ��Ϣ����ͨ���ն�����֪ͨʵ�֣�
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING|UI_S_LOGIN,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			if (QtWengoPhone::_qtContactList)
			{
				string GroupId = QtWengoPhone::_qtContactList->getCContactList().getContactGroupIdFromName(recvedMessFram.readLine(true));
				if (GroupId!="")
					QtWengoPhone::_qtContactList->getCContactList().removeContactGroup(GroupId);
				else
					RunResult=ARESULT_FAIL;
			}
			else
				RunResult=ARESULT_FORBIDDEN;
		}
	}
	else if(strcmp(CommandName,"ChangeGroupName") == 0)//������ӦAction����Ӧ��Ϣ����ͨ���ն�����֪ͨʵ�֣�
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING|UI_S_LOGIN,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			if (QtWengoPhone::_qtContactList)
			{
				string GroupId = QtWengoPhone::_qtContactList->getCContactList().getContactGroupIdFromName(recvedMessFram.readLine(true));
				if (GroupId!="")
					QtWengoPhone::_qtContactList->getCContactList().renameContactGroup(GroupId,recvedMessFram.readLine(true));
				else
					RunResult=ARESULT_FAIL;
			}
			else
				RunResult=ARESULT_FORBIDDEN;
		}
	}
	else if(strcmp(CommandName,"CallSip") == 0)//������ӦAction
	{
		if (QtWengoPhone::CheckImitAction(CommandName,UI_S_MEETING|UI_S_LOGIN,recvedMessFram.readLine(true),TLIndex,TLAvailable)==true)
		{
			ResponseTele=true;
			QtWengoPhone::_cWengoPhone.getCUserProfileHandler().getCUserProfile()->makeCall(recvedMessFram.readLine(true));
		}
		else
			RunResult=ARESULT_FORBIDDEN;
	}
	//------����_�����������--------
	else//�յ�������֧�ֵĶ���
	{
		ResponseTele=true;
		RunResult=ARESULT_FORBIDDEN;
	}
	if (ResponseTele)
	{
		char *ResponsePointer=NULL;
		if (ResponseBuffer!=NULL)
		{
			QtWengoPhone::sendTeleResult(CommandName,RunResult,TLIndex,TLAvailable,ResponseBuffer->getString());
			delete ResponseBuffer;
		}
		else
			QtWengoPhone::sendTeleResult(CommandName,RunResult,TLIndex,TLAvailable);
	}
}

/*
 * ����Ӻ󷵻ض����Ƿ�����ִ����������������ʱ�Զ���teleController���Ͷ������أ�ͬʱ�����������ִ��ģʽ�ǡ�E��ʱ���ͷŸ��Ӻ���������ActionList��Ӧ�ö����ĸ�������Using=false��
 * @param char *ActionName,������ָ�룬���Ӻ������ܱ�ִ��ʱ��Ҫ�ô˲�������sendTeleResult����
 * @param unsigned int EnabledStateCode,������Ӻ󷵻ض���ִ�еĽ���״̬��־���á����������
 * @param int ActionListIndex,���Ӻ󷵻�ָ����ActionList�е�����ֵ
 * @param char * RunMode,����ִ��ģʽ����E��ִ�и��Ӻ���Ӧ��������C��ȡ�����Ӻ���Ӧ����
 * @param int TIndex,teleController�������յȴ��б��������
 * @param int TAvailableNum,teleController�������յȴ��б����Ч���ֵ
 */
bool QtWengoPhone::CheckDelayAction(char *ActionName,unsigned long EnabledStateCode,int ActionListIndex,char * RunMode,int TIndex,int TAvailableNum)
{
	if (strcmp(RunMode,"E")==0)
	{
		if ((QtWengoPhone::UI_State_Code & EnabledStateCode)!=0 && QtWengoPhone::ActionList[ActionListIndex].Using==false)
		{
			QtWengoPhone::ActionList[ActionListIndex].IndexNum=TIndex;
			QtWengoPhone::ActionList[ActionListIndex].AvailableNum=TAvailableNum;
			QtWengoPhone::ActionList[ActionListIndex].Using=true;
			return true;
		}
		else
		{
			if (QtWengoPhone::ActionList[ActionListIndex].Using==true)//ͬ���Ķ�������ִ����
				QtWengoPhone::sendTeleResult(ActionName,ARESULT_RUNNING,TIndex,TAvailableNum);
			else//��ǰ����״̬����ִ�д˶���
				QtWengoPhone::sendTeleResult(ActionName,ARESULT_FORBIDDEN,TIndex,TAvailableNum);
		}
	}
	else
		QtWengoPhone::ActionList[ActionListIndex].Using=false;
	return false;
}

/*
 * ����������ض����Ƿ�����ִ����������������ʱ�Զ���teleController���Ͷ�������
 * @param char *ActionName,������ָ�룬���Ӻ������ܱ�ִ��ʱ��Ҫ�ô˲�������sendTeleResult����
 * @param unsigned int EnabledStateCode,������Ӻ󷵻ض���ִ�еĽ���״̬��־���á����������
 * @param char * RunMode,����ִ��ģʽ����E��ִ�и��Ӻ���Ӧ��������C��ȡ�����Ӻ���Ӧ����
 * @param int TIndex,teleController�������յȴ��б��������
 * @param int TAvailableNum,teleController�������յȴ��б����Ч���ֵ
 */
bool QtWengoPhone::CheckImitAction(char *ActionName,unsigned long EnabledStateCode,char *RunMode,int TIndex,int TAvailableNum)
{
	if (strcmp(RunMode,"E")==0 && (QtWengoPhone::UI_State_Code & EnabledStateCode)!=0)
		return true;
	else
	{
		if (strcmp(RunMode,"E")==0)
			QtWengoPhone::sendTeleResult(ActionName,ARESULT_FORBIDDEN,TIndex,TAvailableNum);
		return false;
	}
}

/*
 * ���ڶ���ִ�к󷵻ظ�teleController��Ϣ�ķ��ͣ��˺�������Action��commandProcess����������ʱ���ã������Ӻ���Ӧ��Action�������sendTeleResult(const char* actionName,int ActionListIndex,int Result,const char * Response)����
 * @param const char* actionName,Ҫ���صĶ�����
 * @param int Result,����ִ�н������ֵ���忴QtWengoPhone.h�ļ����ԡ�ARESULT����ͷ��Ԥ����ֵ������
 * @param int TIndex,���ظ�teleControllerʱָ������teleController�Ķ����ȴ������б��е�����λ�ã�����������ֵ�Ƿ���Ч��������teleControleler����������Ӧ�ļ����룩
 * @param int TAvailable,���ظ�teleControllerʱָ������teleController�Ķ����ȴ������б��е���Ч���ֵ
 * @param const char * Response=NULL,��Ҫ�����ķ�����Ϣ
 * @return void
 */
void QtWengoPhone::sendTeleResult(const char* actionName,int Result,int TIndex,int TAvailable,const char * Response)
{
	CMessageFrame SendMsg;
	SendMsg.Append("Action");//��1����Ϣ����־
	SendMsg.Append(TIndex);//��2��teleController�ж�λ������Ϣ�õ�����ֵ
	SendMsg.Append(TAvailable);//��3��teleController�м���������Ƿ���Ȼ��Ч�ı��
	SendMsg.Append(actionName);//��4��������
	SendMsg.Append(Result);//��5������ִ�н��
	if (Response!=NULL)
		SendMsg.Append(Response);//��6������
	QtWengoPhone::SendInfo(SendMsg.getString());//��������
}

/*
 * ���ý���״̬��ǣ�������õ�״̬�����ԭ����¼�ŵĲ�һ������������Ӧ��Ϣ��teleController������֪ͨң�������£�������״̬�仯�ˣ�Ӧ�õ��ñ�����
 * @param int StateCode,Ҫ���óɵ��µ�״̬��ǣ���ֵ��˵����QtWengoPhone.h�ļ����ԡ�UI_����ͷ��Ԥ����ֵ��˵��
 * @return void
 */
void QtWengoPhone::SetUIState(unsigned long StateCode)
{
	unsigned long NewSStateCode=StateCode&UI_S_ALL;//�����פֵ̬
	unsigned long NewDStateCode=StateCode&UI_D_ALL;//�����˲ֵ̬
	CMessageFrame *ReturnUIParam=NULL;//���UI�ı�ʱ��Ҫ���ظ�teleController�Ĳ�������פ̬�ţ�
	//����פ̬�仯
	if (NewSStateCode!=0 && NewSStateCode!=QtWengoPhone::UI_State_Code)
	{
		QtWengoPhone::UI_State_Code=NewSStateCode;
		ReturnUIParam=new CMessageFrame();
		ReturnUIParam->Append(NewSStateCode);
		switch (NewSStateCode)
		{
		//case UI_S_INIT_BEGIN:
		case UI_S_LOGIN_BEGIN:
			delete ReturnUIParam;
			ReturnUIParam=NULL;
			break;
		case UI_S_LOGIN:
			QtWengoPhone::_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().ExitMeetingSuccessEvent += boost::bind(&QtWengoPhone::ExitMeetingSuccess,this);//���˳�����ɹ�ʱ�������¼������Ը���UI_State_Codeֵ
			ReturnUIParam->Append(QtWengoPhone::_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().getSipAccount()->getFullIdentity().c_str());
			//-------��ʼ ��Ӧפ̬UI_S_LOGIN�����Action�Ĵ������-------
			QtWengoPhone::ResponseDelayAction("Login",ACTION_LOGIN,ARESULT_SUCCESS);//��ӦLogin�Ӻ�������ִ�гɹ�
			QtWengoPhone::ResponseDelayAction("EnterMeeting",ACTION_ENTERMEETING,ARESULT_FAIL);//��ӦEnterMeeting�Ӻ�������ִ��ʧ��
			QtWengoPhone::ResponseDelayAction("QuitMeeting",ACTION_QUITMEETING,ARESULT_SUCCESS);//��ӦQuitMeeting�Ӻ�������ִ�гɹ�
			//-------���� ��Ӧפ̬UI_S_LOGIN�����Action�Ĵ������-------
			break;
		case UI_S_LOGOFF_BEGIN:
			delete ReturnUIParam;
			ReturnUIParam=NULL;
			break;
		case UI_S_LOGOFF:
			//QtWengoPhone::_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().ExitMeetingSuccessEvent -= boost::bind(&QtWengoPhone::ExitMeetingSuccess,this);//����Ҫ����˳�����ɹ�ʱ�������¼�����Ϊ��ʱ��UserProfile�Ѿ�ʧЧ
			ReturnUIParam->Append(ConfigManager::getInstance().getCurrentConfig().getSipRealm().c_str());
			//-------��ʼ ��Ӧפ̬UI_S_LOGOFF�����Action�Ĵ������-------
			QtWengoPhone::ResponseDelayAction("Login",ACTION_LOGIN,ARESULT_FAIL);//��ӦLogin�Ӻ�������ִ��ʧ��
			QtWengoPhone::ResponseDelayAction("LogOff",ACTION_LOGOFF,ARESULT_SUCCESS);//��ӦLogOff�Ӻ�������ִ�гɹ�
			//-------���� ��Ӧפ̬UI_S_LOGOFF�����Action�Ĵ������-------
			break;
		case UI_S_JOINMEETING_BEGIN:
			delete ReturnUIParam;
			ReturnUIParam=NULL;
			break;
		case UI_S_MEETING:
			ReturnUIParam->Append(QtWengoPhone::_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().getSipAccount()->getFullIdentity().c_str());
			ReturnUIParam->Append(QtWengoPhone::_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getUserProfile().joiningMeeting.c_str());
			//-------��ʼ ��Ӧפ̬UI_S_MEETING�����Action�Ĵ������-------
			QtWengoPhone::ResponseDelayAction("EnterMeeting",ACTION_ENTERMEETING,ARESULT_SUCCESS);//��ӦEnterMeeting�Ӻ�������ִ�гɹ�
			//-------���� ��Ӧפ̬UI_S_MEETING�����Action�Ĵ������-------
			break;
		case UI_S_EXITMEETING_BEGIN:
			delete ReturnUIParam;
			ReturnUIParam=NULL;
			break;
		}
		if (ReturnUIParam!=NULL)
			QtWengoPhone::sendTeleStateChanged(CHANGE_UI,ReturnUIParam->getString());//����״̬�������֪ͨ
	}
	//����˲̬�仯
	if (NewDStateCode!=0)
	{
		//-------��ʼ ˲ֵ̬�������-------
		if (NewDStateCode==UI_D_CONTACTMEMBER_SUCCESS)
			QtWengoPhone::ResponseDelayAction("ContactMember",ACTION_CONTACTMEMBER,ARESULT_SUCCESS);//��ӦContactMember�Ӻ�������ִ�гɹ�
		if (NewDStateCode==UI_D_CONTACTMEMBER_ERROR)
			QtWengoPhone::ResponseDelayAction("ContactMember",ACTION_CONTACTMEMBER,ARESULT_FAIL);//��ӦContactMember�Ӻ�������ִ��ʧ��
		if (NewDStateCode==UI_D_RAISEHAND_SUCCESS)
			QtWengoPhone::ResponseDelayAction("RaiseHand",ACTION_RAISEHAND,ARESULT_SUCCESS);//��ӦRaiseHand�Ӻ�������ִ�гɹ�
		if (NewDStateCode==UI_D_PERMITSPEAK_ERROR)
			QtWengoPhone::ResponseDelayAction("PermitSpeak",ACTION_PERMITSPEAK,ARESULT_FAIL);//��ӦPermitSpeak�Ӻ�������ִ��ʧ��
		if (NewDStateCode==UI_D_PERMITSPEAK_SUCCESS)
			QtWengoPhone::ResponseDelayAction("PermitSpeak",ACTION_PERMITSPEAK,ARESULT_SUCCESS);//��ӦPermitSpeak�Ӻ�������ִ�гɹ�
		if (NewDStateCode==UI_D_KICKMEMBER_SUCCESS)
			QtWengoPhone::ResponseDelayAction("KickMember",ACTION_KICKMEMBER,ARESULT_SUCCESS);//��ӦKickMember�Ӻ�������ִ�гɹ�
		if (NewDStateCode==UI_D_KICKMEMBER_ERROR)
			QtWengoPhone::ResponseDelayAction("KickMember",ACTION_KICKMEMBER,ARESULT_FAIL);//��ӦKickMember�Ӻ�������ִ��ʧ��
		if (NewDStateCode==UI_D_CREATEMEETING_SUCCESS)
			QtWengoPhone::ResponseDelayAction("CreateMeeting",ACTION_CREATEMEETING,ARESULT_SUCCESS);//��ӦCreateMeeting�Ӻ�������ִ�гɹ�
		if (NewDStateCode==UI_D_LOCALAUDIO_SEND_STOP || NewDStateCode==UI_D_LOCALAUDIO_SEND_START || NewDStateCode==UI_D_LOCALVIDEO_SEND_STOP || NewDStateCode==UI_D_LOCALVIDEO_SEND_START)
		{
			string ChangeInfo;
			if (NewDStateCode==UI_D_LOCALAUDIO_SEND_STOP || NewDStateCode==UI_D_LOCALVIDEO_SEND_STOP)
				ChangeInfo="On";
			else
				ChangeInfo="Off";
			if (NewDStateCode==UI_D_LOCALAUDIO_SEND_STOP || NewDStateCode==UI_D_LOCALAUDIO_SEND_START)
				QtWengoPhone::sendTeleStateChanged(CHANGE_LOCALAUDIO,ChangeInfo.c_str());
			else
				QtWengoPhone::sendTeleStateChanged(CHANGE_LOCALVIDEO,ChangeInfo.c_str());
		}
		//-------���� ˲ֵ̬�������-------
	}
}

/*
 * �����Ӻ���ӦAction�ĺ������������Զ��жϴ����ActionIndex����ӦAction�Ƿ���Ҫ���ͻظ���Ϣ
 * @param string ActionName,��������������ActionIndex��Ӧ�Ķ�����һ��
 * @param int ActionIndex,�������Ӻ���Ӧ�����б��е�����ֵ�����������鴫��ֵ�Ƿ���Ч�����ţ�����������Ӧ�Ķ������ȴ��ظ�ʱ�����������sendTeleResult
 * @param int Result=ARESULT_SUCCESS,�������صĽ����Ĭ��Ϊ�ɹ�
 * @param char * ResponsePointer=NULL,�ظ�ʱ��Ҫ��������Ϣ��ָ�룬Ĭ��Ϊ��
 * @return void
 */
void QtWengoPhone::ResponseDelayAction(string ActionName,int ActionIndex,int Result,char * ResponsePointer)
{
	if (QtWengoPhone::ActionList[ActionIndex].Using)
	{
		QtWengoPhone::sendTeleResult(ActionName.c_str(),Result,QtWengoPhone::ActionList[ActionIndex].IndexNum,QtWengoPhone::ActionList[ActionIndex].AvailableNum,ResponsePointer);
		QtWengoPhone::ActionList[ActionIndex].Using=false;
	}
}

/*
 * �����ն�����ͨ�����Ϣ��"State"������Ϣ��
 * @param int StateIndex,����֪ͨ�����ţ���teleController�еĶ�����һ��
 * @param const char *AddInfo=NULL,������Ϣ���������״̬�ı�ʱ���ظı��Ľ���ֵ
 * @return void
 */
void QtWengoPhone::sendTeleStateChanged(int StateIndex,const char *AddInfo)
{
	CMessageFrame SendMsg;
	SendMsg.Append("State");
	SendMsg.Append(StateIndex);
	if (AddInfo)
		SendMsg.Append(AddInfo);
	QtWengoPhone::SendInfo(SendMsg.getString());//��������
}

/*
 * ����GlobalSetting��ֵַ��������ң����ͨ�õ��²㺯��
 * @param string IPv4Addr,IPv4��Ӧ��GlobalSetting��ַ
 * @param string IPv6Addr,IPv6��Ӧ��GlobalSetting��ַ��Ŀǰ�˲�����������ò��ϵģ�ֻʹ��IPv4�ĵ�ַ
 * @return void
 */
void QtWengoPhone::SaveGlobalSetting(string IPv4Addr,string IPv6Addr)
{
	ConfigManager::getInstance().getCurrentConfig().set(Config::SIP_GLOBAL_SERVICE,IPv4Addr);
}

/*
 * ���������ú�����ң�����ͽ���ͨ��
 * @param QString Title,Ҫ��������ı��⣬����������Ч�Լ��
 * @param QString Description,Ҫ�������������
 * @param int ControlModeNum,����ģʽ��0-����ģʽ 1-������ģʽ
 * @param int JoinModeNum,����Ȩ�����ã�0-�������飬1-������֤
 * @param QString Psw,�������룬������Ϊ������֤����ʱ�˲�������Ч�������Զ���������������ֵ��\t��
 */
void QtWengoPhone::CreateMeeting(QString Title,QString Description,int ControlModeNum,int JoinModeNum,QString Psw)
{
	string ControlMode = "";
	string JoinMode = "public";
	if (ControlModeNum==0)
		ControlMode="peer";
	else
		ControlMode="host";
	switch(JoinModeNum)
	{
	case 0:
		JoinMode = "public";
		Psw='\t';
		break;
	case 1:
		JoinMode = "password";
		break;
	default:
		break;
	}
	CUserProfile * cUserProfile = QtWengoPhone::_cWengoPhone.getCUserProfileHandler().getCUserProfile();
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	//std::string confUri = "sip:service@ser.scut.edu.cn";
	std::string confUri = config.getGlobalService();
	std::string szContent = "<?xml version=\"1.0\"?><coolview command=\"ApplyConference\"><conference><uri></uri>";
	szContent+="<title>"+string(Title.toUtf8()) +"</title>";  //modify by li.zhenning
	//szContent+="<title>"+std::string(_ui->phoneNumber1LineEdit->text().toLocal8Bit()) +"</title>";
	szContent+="<description>"+string(Description.toUtf8()) +"</description>";
	szContent +="<hostUri></hostUri>";
	szContent +="<controlMode>"+ControlMode+"</controlMode>";
	szContent +="<joinMode>"+JoinMode+"</joinMode>";
	szContent +="<joinPassword>"+string(Psw.toUtf8())+"</joinPassword>";
	szContent +="<startTime></startTime><duration></duration>";
	szContent+="</conference></coolview>";
	cUserProfile->applyConference(confUri,szContent);
	QtWengoPhone::SetUIState(UI_D_CREATEMEETING_SUCCESS);
}

/*
 * ��ӦUserProfile������ExitMeetingSuccessEvent�¼�������UI_State_Code����ֵ
 * @return void
 */
void QtWengoPhone::ExitMeetingSuccess()
{
	QtWengoPhone::SetUIState(UI_S_LOGIN);
}
//-------End of Add By LZY----------