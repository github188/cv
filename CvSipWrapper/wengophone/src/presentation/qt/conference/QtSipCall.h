/************************************************************************/
/*Coolview 3.0 ����
 *zhenHua.sun 2010-08-14
 */
/************************************************************************/
#ifndef SIPCALL_H
#define SIPCALL_H

#include "ui_qtSipCall.h"
class QtToolWidget;
class QtContactList;
class CWengoPhone;
class QtSipCall : public QWidget , public Ui::sipCallForm
{
    Q_OBJECT

    public:
        QtSipCall(QtToolWidget * qtTool , CWengoPhone &cWengoPhone );
//        void addItem(QTreeWidgetItem *parent, QStringList *sl);//��Tree�б������

		HWND getSipCallPreviewWnd();

	private:
		QtToolWidget* _qtTool;
		QtContactList* _qtContactList;
		CWengoPhone& _cWengoPhone;


        //���µĲ۶��Ǻ��й������
    private Q_SLOTS:
        void pressNum1();
        void pressNum2();
        void pressNum3();
        void pressNum4();
        void pressNum5();
        void pressNum6();
        void pressNum7();
        void pressNum8();
        void pressNum9();
        void pressNum0();
        void pressNumstar(); //*��
        void pressNumpound();//#��
        void getPhoneNum();  //ѡ��ͨѶ¼�е�һ�������ʾ���ı�����

		/// @brief �����ϵ��
		void addContactClicked();

		/// @brief ɾ����ϵ��
		void deleteContactClicked();



		/**
		 * @brief Sip����
		 */
		void sipCallClicked();	

		/** 
		 * @brief ������ϵ���б�
		 */
		void updateContactList();

		/**
		 * @brief ���ز�����
		 */
		void hideDiapadSlot( bool checked);


};


#endif // SIPCALL_H
