#ifndef QTOOLWIDGET_H
#define QTOOLWIDGET_H

/*�˵����������˺��У����š���ַ��������أ������飨�����б������Ա����
�豸��������ͷ����Ƶ����Ƶ��ʾ������أ���Ҫ��һЩ����������������Ϣ����
����Ϣ��������δʵ�֣�����6������*/

#include "StatusWidget.h"
#include "ui_tool.h"

class ToolTest : public QWidget , public Ui::toolbar
{
    Q_OBJECT

    public:
        ToolTest(QWidget *parent = 0);
        void addItem(QTreeWidgetItem *parent, QStringList *sl);//��Tree�б������
        void addConfItem(QTreeWidget *treelist,QTreeWidgetItem *parent, QStringList *sl);//�������б������
        void addMemberItem(QTreeWidget *treelist,QTreeWidgetItem *parent, QStringList *sl);//�������Ա�����
        void deleteConference();//ȡ������

    private:
        StatusWidget *status;
        QAction *delConfAction;  //ȡ�������������action

    public Q_SLOTS:
        void enterConference();
        void exitConference();
        void addcontact();
        void deletecontact();
        void okToContinue();

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

    };

#endif // QTOOLWIDGET_H
