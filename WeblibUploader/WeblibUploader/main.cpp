#include "mainwindow.h"
#include <QApplication>

//����������ض���������־�ļ����������Ŀ��ͨ��qDebug��qCritical�������Ϣ������
//���Դ�����ʾ����Ҫ�鿴��־�ļ���log.h
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	MainWindow mainwindow;
	mainwindow.show();
	return a.exec();
}
