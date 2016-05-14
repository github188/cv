	#pragma once
#include "ui_ConferenceRoomWidget.h"
//coolview3.0 �������

class QtWengoPhone;
class QtToolWidget;
class QScrollArea;
class QPushButton;

class QtConferenceRoomWidget:public QWidget
{
	friend class QtWengoPhone;  //huixin.du  2010-09-16
	Q_OBJECT
public:
	QtConferenceRoomWidget(QtWengoPhone & qtWengoPhone);
	~QtConferenceRoomWidget(void);
    
    QPushButton *_toolButton;     //���Ʋ˵�����ʾ���صİ�ť

	//Coolview 3.0 ���淽�� zhenHua.sun 2010-08-11
	QtToolWidget *GetToolWidget();

Q_SIGNALS:

private Q_SLOTS:

	/**
	 * @brief �˳�����
	 */
	void exitProgramSlot();
	/**
	 * @brief �û�ע��
	 */
	void logOutSlot();

private:
	Ui::Form *_ui;
	QtToolWidget *_tools;             //�˵���
	QtWengoPhone & _qtWengoPhone;
};
