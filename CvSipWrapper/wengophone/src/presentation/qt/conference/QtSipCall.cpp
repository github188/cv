/************************************************************************/
/*Coolview 3.0 ����
 *zhenHua.sun 2010-08-14
 */
/************************************************************************/
#include "stdafx.h"
#include "QtSipCall.h"
#include "QtToolWidget.h"
#include "..\contactlist\QtContactList.h"
#include "..\QtWengoPhone.h"

#include "profile/QtProfileDetails.h"

//#include "ContactProfile.h"
//#include "ui_tool.h"
#include <model/contactlist/ContactProfile.h>
#include <control/contactlist/CContactList.h>
#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>
#include <model/profile/UserProfile.h>
#include <contactlist/QtContactManager.h>
#include <contactlist/QtContactListManager.h>
QtSipCall::QtSipCall(QtToolWidget* qtToolWidget , CWengoPhone& cWengophone ) : _qtTool( qtToolWidget ),_cWengoPhone(cWengophone)
{

    setupUi(this);


//���в���
    //���ز�����
	hideDiapadSlot(true);
    //addFriendButton->hide();
    //contactTreeWidget->hide();
    //deleteFriendButton->hide();

	//contactList->hide();
    //���Ű�ťʵ��
    connect(pushButton1,SIGNAL(clicked()),this,SLOT(pressNum1()));
    connect(pushButton2,SIGNAL(clicked()),this,SLOT(pressNum2()));
    connect(pushButton3,SIGNAL(clicked()),this,SLOT(pressNum3()));
    connect(pushButton4,SIGNAL(clicked()),this,SLOT(pressNum4()));
    connect(pushButton5,SIGNAL(clicked()),this,SLOT(pressNum5()));
    connect(pushButton6,SIGNAL(clicked()),this,SLOT(pressNum6()));
    connect(pushButton7,SIGNAL(clicked()),this,SLOT(pressNum7()));
    connect(pushButton8,SIGNAL(clicked()),this,SLOT(pressNum8()));
    connect(pushButton9,SIGNAL(clicked()),this,SLOT(pressNum9()));
    connect(pushButton0,SIGNAL(clicked()),this,SLOT(pressNum0()));
    connect(pushButtonstar,SIGNAL(clicked()),this,SLOT(pressNumstar()));
    connect(pushButtonpound,SIGNAL(clicked()),this,SLOT(pressNumpound()));
    connect(confCallButton , SIGNAL(clicked()) , this , SLOT(sipCallClicked()));

//��ϵ�˲���
	//�б�
	if (!contactList->layout()) {
		Widget::createLayout(contactList);
	}

	QtContactList *qtContactList = _qtTool->getQtWengoPhone().getQtContactList();
	contactList->layout()->addWidget(qtContactList->getWidget());

	connect(clearButton,SIGNAL(clicked()),numLineEdit,SLOT(clear()));
    //�����ϵ���е�һ��ı�������ʾ����
    //connect(contactTreeWidget,SIGNAL(clicked(QModelIndex)),this,SLOT(getPhoneNum()));
    //��������ϵ�˰�ť���������ϵ�˿�
    connect(addFriendButton,SIGNAL(clicked()),this,SLOT(addContactClicked()));
    //���ɾ����ϵ�˰�ť��ɾ��ѡ�е���ϵ����
    connect(deleteFriendButton,SIGNAL(clicked()),this,SLOT(deleteContactClicked()));


	updateContactList();
	
	//���ƺ����������롡 zhenHua.sun 2010-08-19
	QRegExp exgExp(tr("^[0-9a-zA-Z\\._-]+$"));
	numLineEdit->setValidator( new QRegExpValidator( exgExp , this ));

}

/*
void QtSipCall::addItem(QTreeWidgetItem *parent, QStringList *sl)//���Բ���
{

    QTreeWidgetItem *item;

    if(!parent)
    {
        parent = contactTreeWidget->invisibleRootItem();
    }
    item = new QTreeWidgetItem(parent,*sl);
	
}
*/
void QtSipCall::getPhoneNum()
{
    //numLineEdit->setText(contactTreeWidget->currentItem()->text(0));
}

void QtSipCall::pressNum1()
{
    QString newnum = numLineEdit->text() + "1";
    numLineEdit->setText(newnum);
}
void QtSipCall::pressNum2()
{
    QString newnum = numLineEdit->text() + "2";
    numLineEdit->setText(newnum);
}
void QtSipCall::pressNum3()
{
    QString newnum = numLineEdit->text() + "3";
    numLineEdit->setText(newnum);
}
void QtSipCall::pressNum4()
{
    QString newnum = numLineEdit->text() + "4";
    numLineEdit->setText(newnum);
}
void QtSipCall::pressNum5()
{
    QString newnum = numLineEdit->text() + "5";
    numLineEdit->setText(newnum);
}
void QtSipCall::pressNum6()
{
    QString newnum = numLineEdit->text() + "6";
    numLineEdit->setText(newnum);
}
void QtSipCall::pressNum7()
{
    QString newnum = numLineEdit->text() + "7";
    numLineEdit->setText(newnum);
}
void QtSipCall::pressNum8()
{
    QString newnum = numLineEdit->text() + "8";
    numLineEdit->setText(newnum);
}
void QtSipCall::pressNum9()
{
    QString newnum = numLineEdit->text() + "9";
    numLineEdit->setText(newnum);
}
void QtSipCall::pressNum0()
{
    QString newnum = numLineEdit->text() + "0";
    numLineEdit->setText(newnum);
}
void QtSipCall::pressNumstar()
{
    QString newnum = numLineEdit->text() + "*";
    numLineEdit->setText(newnum);
}
void QtSipCall::pressNumpound()
{
    QString newnum = numLineEdit->text() + "#";
    numLineEdit->setText(newnum);
}

void QtSipCall::addContactClicked()
{
	//�����ϵ��....zhenHua.sun 2010-08-16
	if (_cWengoPhone.getCUserProfileHandler().getCUserProfile()) {

		QWidget * parent = qobject_cast<QWidget *>(sender()->parent());

		//FIXME this method should not be called if no UserProfile has been set
		ContactProfile contactProfile;
		QtProfileDetails qtProfileDetails( *(_cWengoPhone.getCUserProfileHandler().getCUserProfile()),
		contactProfile, parent, tr("Add a Contact") );
		if (qtProfileDetails.show()) {
			_cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCContactList().addContact(contactProfile);
			updateContactList();
		}
	}
	
}


void QtSipCall::updateContactList()
{
	CContactList & currentCContactList = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCContactList();
	StringList currentContactsIds = currentCContactList.getContactIds();

	for (StringList::const_iterator it = currentContactsIds.begin(); it != currentContactsIds.end(); ++it) {

		ContactProfile tmpContactProfile = currentCContactList.getContactProfile(*it);
		QString displayName = QString::fromUtf8(tmpContactProfile.getDisplayName().c_str());

		if (tmpContactProfile.hasAvailableSIPNumber() ) {

			QString freePhoneNumber = QString::fromStdString(tmpContactProfile.getFirstFreePhoneNumber());
			//QAction * tmpAction;
			//tmpAction = menu->addAction(displayName);
			//tmpAction->setData(QVariant(freePhoneNumber));
			EnumPresenceState::PresenceState presenceState = tmpContactProfile.getPresenceState();
			//setPresenceIcon(tmpAction, presenceState);

			//if (presenceState == EnumPresenceState::PresenceStateUnknown) {
			//	setPresenceIcon(tmpAction, QIcon(":/pics/status/unknown-sip.png"));
			//} else {
			//	setPresenceIcon(tmpAction, presenceState);
			//}
		}
	}
}

void QtSipCall::sipCallClicked()
{
	QTextCodec::setCodecForTr( QTextCodec::codecForName("utf-8"));
	if( _qtTool->isInConference() )
	{
		QMessageBox::information(this, 
			QString::fromLocal8Bit("��ʾ"),
			QString::fromLocal8Bit("���Ѿ������˻��飬���ܽ���Sip����"),
			QMessageBox::Ok);
		return;
	}

	if( this->numLineEdit->isVisible() )
	{
		//�����������ʾ�����ˣ���ô������������еĺ���
		if( this->numLineEdit->text().trimmed().length() == 0 )
		{
			QMessageBox::information(this, 
				QString::fromLocal8Bit("��ʾ"),
				QString::fromLocal8Bit("����������Ϊ�գ�"),
				QMessageBox::Ok);
			return;
		}else
		{
			QString inputValue = this->numLineEdit->text().trimmed();

			CContactList & currentCContactList = _cWengoPhone.getCUserProfileHandler().getCUserProfile()->getCContactList();
			StringList currentContactsIds = currentCContactList.getContactIds();
			for (StringList::const_iterator it = currentContactsIds.begin(); it != currentContactsIds.end(); ++it) {

				ContactProfile tmpContactProfile = currentCContactList.getContactProfile(*it);
				QString displayName = QString::fromUtf8(tmpContactProfile.getDisplayName().c_str());
				QString mobilePhone = QString::fromUtf8(tmpContactProfile.getMobilePhone().c_str());
				if( inputValue == displayName ||
					inputValue == mobilePhone ) 
				{

					QtContactListManager* clManager = QtContactListManager::getInstance();
					clManager->startFreeCall( QString::fromStdString(*it) );
					return;
				}				
			}
		}
		QMessageBox::information(this, 
			QString::fromLocal8Bit("��ʾ"),
			QString::fromLocal8Bit("û���ҵ���Ӧ��Ϣ����ϵ��"),
			QMessageBox::Ok);
		return;
	}else
	{
		//����ͨ����ϵ���б���к���
		QtContactManager* manager = _qtTool->getQtWengoPhone().getQtContactList()->getContactManager();
		manager->startFreeCall();
	}

}

void QtSipCall::hideDiapadSlot( bool checked )
{
	if( checked )
	{
		this->pushButton0->setVisible(false);
		this->pushButton1->setVisible(false);
		this->pushButton2->setVisible(false);
		this->pushButton3->setVisible(false);
		this->pushButton4->setVisible(false);
		this->pushButton5->setVisible(false);
		this->pushButton6->setVisible(false);
		this->pushButton7->setVisible(false);
		this->pushButton8->setVisible(false);
		this->pushButton9->setVisible(false);
		this->pushButtonstar->setVisible(false);
		this->pushButtonpound->setVisible(false);
		this->numLineEdit->setVisible(false);
		this->clearButton->setVisible(false);
	}else
	{
		this->pushButton0->setVisible(true);
		this->pushButton1->setVisible(true);
		this->pushButton2->setVisible(true);
		this->pushButton3->setVisible(true);
		this->pushButton4->setVisible(true);
		this->pushButton5->setVisible(true);
		this->pushButton6->setVisible(true);
		this->pushButton7->setVisible(true);
		this->pushButton8->setVisible(true);
		this->pushButton9->setVisible(true);
		this->pushButtonstar->setVisible(true);
		this->pushButtonpound->setVisible(true);
		this->numLineEdit->setVisible(true);
		this->clearButton->setVisible(true);
	}

}

void QtSipCall::deleteContactClicked()
{
	QtContactManager* manager = _qtTool->getQtWengoPhone().getQtContactList()->getContactManager();
	manager->deleteContact();
}

HWND QtSipCall::getSipCallPreviewWnd()
{
	return (HWND)this->localPreviewWidget->winId();
}