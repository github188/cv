/**
 * Coolview 3.0 �������  
 * zhenHua.sun 2010-08-10
 */

#ifndef QTTOOLWIDGET_H
#define QTTOOLWIDGET_H

/*�˵����������˺��У����š���ַ��������أ������飨�����б������Ա����
�豸��������ͷ����Ƶ����Ƶ��ʾ������أ���Ҫ��һЩ����������������Ϣ����
����Ϣ��������δʵ�֣�����6������*/

#include "QtStatusWidget.h"
#include "ui_tool.h"
#include <control/CWengoPhone.h>//Add By LZY 2010-09-25


class QtConferenceRoomWidget;
class QtConferenceList;
class QtMemberList;
class QtWengoPhone;
class QtVideoSettings;
class QtAudioSettings;
class QtShowSetting;
class QtSipCall;
class QtTextChatWidget;
class QtRtcpInfoWidget;
class QtAboutDialog;

class QtToolWidget : public QWidget
{
	friend class QtConferenceRoomWidget;

    Q_OBJECT

    public:
        QtToolWidget(QtWengoPhone& qtWengoPhone, QtConferenceRoomWidget *qtConferenceRoom = 0);
		~QtToolWidget( );
		
		/**
		 * @brief ��ȡ��������Ƶ�ķ���״̬
		 * @return �����1��ʾ���ڷ�����Ƶ������Ϊ0
		 */
		int getVideoSendState();

		/**
		 * @brief ��ȡ��������Ƶ�ķ���״̬
		 * @return �����1��ʾ���ڷ�����Ƶ������Ϊ0
		 */
		int getAudioSendState();

		/**
		 * @brief ��ȡ���Դ��ھ��
		 */
		QWidget *getPreviewWindow();

		/**
		 * @brief ��ȡqtWengoPhone
		 */
		QtWengoPhone& getQtWengoPhone(){ return _qtWengoPhone; }

		/**
		 * @brief ��ȡ�������
		 */
		QtSipCall* getQtSipCall(){	return _qtSipCall; }

		/**
		 * @brief ������Ƶ����
		 * @state 0��ʾֹͣ���ͣ� 1��ʾ��������
		 */
		void setVideoSendStateSlot( int state );

		/**
		 * @brief ������Ƶ����
		 * @state 0��ʾֹͣ���ͣ� 1��ʾ��������
		 */
		void setAudioSendStateSlot( int state );

		void setIsInConference( bool inConf ){ _isInConference = inConf; }
		bool isInConference(void){ return _isInConference; }

		/**
		 * @brief �˳�����
		 */
		bool exitConference();

		
		Ui::toolbar* _ui;
		QtConferenceList *_confList;
		QtMemberList *_memberList;
		QtAboutDialog *_qtAboutDialog;
		QtWengoPhone& _qtWengoPhone;
         
	public Q_SLOTS:
		void hideToolWidget();  //add by huixin.du 2010-09-06

    private:
        QtStatusWidget *status;
        QAction *delConfAction;  //ȡ�������������action
		QtConferenceRoomWidget* _qtConferenceRoom;

		///��Ƶ���ô���
		QtVideoSettings* _qtVideoSetting;

		///��Ƶ���ô���
		QtAudioSettings* _qtAudioSetting;

		///��Ƶ���ڸ�����ô���
		QtShowSetting * _qtShowSetting;

		///���д���
		QtSipCall* _qtSipCall;

		//��Ϣ����
		QtTextChatWidget* _qtMessage;

		//����״̬����
		QtRtcpInfoWidget* _qtNetworkCondition;

		//�Ƿ�����˻���
		bool _isInConference;
		
		
};

#endif // QTTOOLWIDGET_H
