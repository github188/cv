/**
 * Coolview 3.0 �������  
 * zhenHua.sun 2010-08-10
 */


#include "stdafx.h"

#include "QtToolWidget.h"
#include "QtConferenceList.h"
#include "QtMemberList.h"
#include "QtWengoPhone.h"
#include "..\config\QtVideoSettings.h"
#include "..\config\QtAudioSettings.h"
#include "..\config\QtShowSetting.h"
#include "QtSipCall.h"
#include "QtTextChatWidget.h"
#include "QtRtcpInfoWidget.h"
#include "QtAboutDialog.h"
#include "QtToolBar.h"

QtToolWidget::QtToolWidget(QtWengoPhone& qtWengoPhone , QtConferenceRoomWidget *conferenceRoom) : _qtWengoPhone(qtWengoPhone) , _qtConferenceRoom(conferenceRoom)
{
	_ui = new Ui::toolbar();
	_ui->setupUi( this );
	
 	this->hide();       //add by huixin.du 2010-09-06

    //Ĭ�����ع�����
	//_ui->dockWidget_2->setVisible(false);

	int w = _qtConferenceRoom->width();
	int h = _qtConferenceRoom->height();
	_ui->dockWidget_2->resize(w,h);
	

	

	//�˵�����ʼ��ʾ���鲿�֣�������������
    _ui->sipGroupBox->hide();
    _ui->settingTabWidget->hide();
    _ui->advanceGroupBox->hide();
    _ui->messageGroupBox->hide();
	_ui->helpGroupBox->hide();

    layout()->setSizeConstraint(QLayout::SetFixedSize); //���ڴ�С��������GroupBox�����غ���ʾ���ı�

//����Ƶ����

	//��Ƶ���ô���
	_qtVideoSetting = new QtVideoSettings( _qtWengoPhone.getCWengoPhone() , this);
	if( !_ui->videoSettingTab->layout() )
	{
		Widget::createLayout(_ui->videoSettingTab);
	}
	_qtVideoSetting->setParent( _ui->videoSettingTab );
	_ui->videoSettingTab->layout()->addWidget( _qtVideoSetting );

	connect( _ui->settingButton , SIGNAL(clicked()) , _qtVideoSetting , SLOT(configButtonClicked()) );

	//��Ƶ���ô���
	_qtAudioSetting = new QtAudioSettings( _qtWengoPhone.getCWengoPhone() , this);
	if( !_ui->audioSettingTab->layout() )
	{
		Widget::createLayout(_ui->audioSettingTab);
	}
	//if( _qtAudioSetting->_ui->mainLayout )
	//{
	//	_ui->audioSettingTab->setLayout( _qtAudioSetting->_ui->mainLayout );
	//}
	_qtAudioSetting->setParent( _ui->audioSettingTab );
	_ui->audioSettingTab->layout()->addWidget( _qtAudioSetting );

	connect( _ui->settingButton , SIGNAL(clicked()) , _qtAudioSetting , SLOT(configButtonClicked()));


	//����־�
	_qtShowSetting = new QtShowSetting( this );
	if( !_ui->displaySettingTab->layout() )
	{
		Widget::createLayout(_ui->displaySettingTab );
	}
	_qtShowSetting->setParent( _ui->displaySettingTab );
	_ui->displaySettingTab->layout()->addWidget(_qtShowSetting);

//���鲿��
	_isInConference = false;
	//��ӻ����б�widget��toolwidget
	_confList = new QtConferenceList(_qtWengoPhone,this);
	if (!_ui->confListTab->layout()) {
		Widget::createLayout(_ui->confListTab);
	}
	_confList->setParent(_ui->confListTab);
	_ui->confListTab->layout()->addWidget(_confList);

	//��ӳ�Ա�б�widget��toolwidget
	_memberList = new QtMemberList(_qtWengoPhone,_qtConferenceRoom);
	if (!_ui->memberListTab->layout()) {
		Widget::createLayout(_ui->memberListTab);
	}
	_memberList->setParent(_ui->memberListTab);
	_ui->memberListTab->layout()->addWidget(_memberList);

//���в���
	_qtSipCall = new QtSipCall( this , _qtWengoPhone.getCWengoPhone() );
	if ( !_ui->sipGroupBox->layout() )
	{
		Widget::createLayout(_ui->sipGroupBox);
	}
	_qtSipCall->setParent( _ui->sipGroupBox );
	_ui->sipGroupBox->layout()->addWidget( _qtSipCall );

//��Ϣ����
	_qtMessage = new QtTextChatWidget(_qtWengoPhone,this);
	if( !_ui->messageGroupBox->layout() )
	{
		Widget::createLayout( _ui->messageGroupBox );
	}
	_qtMessage->setParent( _ui->messageGroupBox );
	_ui->messageGroupBox->layout()->addWidget(_qtMessage);

//����״̬����
	_qtNetworkCondition = new QtRtcpInfoWidget(_qtWengoPhone);
	//_qtNetworkCondition = new QtNetworkCondition( this );
	if( !_ui->advanceGroupBox->layout() )
	{
		Widget::createLayout( _ui->advanceGroupBox );
	}
	_qtNetworkCondition->setParent( _ui->advanceGroupBox );
	_ui->advanceGroupBox->layout()->addWidget(_qtNetworkCondition );

//����
	_qtAboutDialog = new QtAboutDialog();
	if( !_ui->helpGroupBox->layout() )
	{
		Widget::createLayout(_ui->helpGroupBox);
	}
	_qtAboutDialog->setParent( _ui->helpGroupBox );
	_ui->helpGroupBox->layout()->addWidget(_qtAboutDialog);

	_qtAboutDialog->_ui->EnterButton->hide();    //����ȷ����ť

	connect( _qtAboutDialog->_ui->LinkLabel , SIGNAL(clicked()) , &(_qtWengoPhone.getQtToolBar()), SLOT(showWengoForum()));

//��widgetָ�������QtWengoPhone��
	_qtWengoPhone.setQtToolWidget( this );

	SAFE_CONNECT_RECEIVER(&_qtWengoPhone,SIGNAL(ActionLocalControlSignal()),this,SLOT(hideToolWidget()));//Add By LZY 2010-09-28 ����ӦLocalControl����Signal�ĺ���
}

QtToolWidget::~QtToolWidget()
{
	disconnect(&_qtWengoPhone,SIGNAL(ActionLocalControlSignal()),this,SLOT(hideToolWidget()));//Add By LZY 2010-09-19 �����ӦLocalControl����Signal�ĺ���

	OWSAFE_DELETE( _confList );
	OWSAFE_DELETE( _memberList );	
	OWSAFE_DELETE(_qtVideoSetting);
	OWSAFE_DELETE(_qtAudioSetting);
	OWSAFE_DELETE(_qtShowSetting);
	OWSAFE_DELETE(_qtSipCall);
	OWSAFE_DELETE(_qtMessage);
	OWSAFE_DELETE(_qtNetworkCondition);
	OWSAFE_DELETE(_qtAboutDialog);
	OWSAFE_DELETE(_ui);
}


bool QtToolWidget::exitConference()
{
	//�����˳�����
	if( _isInConference )
	{
		return this->_memberList->exitButtonClicked();
	}	

	return true;
}
/**
* @brief ��ȡ��������Ƶ�ķ���״̬
* @return �����1��ʾ���ڷ�����Ƶ������Ϊ0
*/
int QtToolWidget::getVideoSendState()
{
	if( _memberList->_ui->videoControlButton->isChecked() )
	{
		return 0;
	}else
	{
		return 1;
	}
}

/**
* @brief ��ȡ��������Ƶ�ķ���״̬
* @return �����1��ʾ���ڷ�����Ƶ������Ϊ0
*/
int QtToolWidget::getAudioSendState()
{
	if( _memberList->_ui->audioControlButton->isChecked() )
	{
		return 0;
	}else
	{
		return 1;
	}
}

void QtToolWidget::setVideoSendStateSlot( int state )
{
	if( _isInConference )
	{
		_memberList->videoControlClicked( state );
	}
}

void QtToolWidget::setAudioSendStateSlot( int state )
{
	if( _isInConference )
	{
		_memberList->audioControlClicked( state );
	}

}

//-------Modify By LZY 2010-09-26-------
/**
 * add by huixin.du 2010-09-06
 */	
void QtToolWidget::hideToolWidget()
{
	if(_ui->dockWidget_2->isHidden() == true)
	{
		_ui->dockWidget_2->setFloating(true);
		_ui->dockWidget_2->resize(_ui->dockWidget_2->maximumSize());
		_ui->dockWidget_2->show();
	}
	else
	{
		_ui->dockWidget_2->setFloating(false);
		_ui->dockWidget_2->setVisible(false);
	}
}
//-------End of Modify By LZY-----------