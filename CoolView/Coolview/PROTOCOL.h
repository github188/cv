#pragma once
//2014.4.2.gmlan-������̨���ƹ���
#include <QMainWindow>

class PROTOCOL
{
public:
	PROTOCOL(void);
	~PROTOCOL(void);

	virtual void left(){};//��
	virtual void right(){};//��
	virtual void up(){};//��
	virtual void down(){};//��
	virtual void stop(){};//ֹͣ
	virtual void focusFar(){};//�۽�Զ
	virtual void focusNear(){};//�۽���
	virtual void zoomIn(){};//�Ŵ�
	virtual void zoomOut(){};//��С
	virtual void callPreset1(){};//����Ԥ��λ1
	//virtual void ();

//private:
	PROTOCOL* protocol;
	QByteArray command;

};

class PELCO_D:public PROTOCOL
{
public:
	PELCO_D(){ command.resize(7); command[0] = 0xff; }
	~PELCO_D(){}

	 void left();//��
	 void right();
	 void up();
	 void down();
	 void stop();
	 void focusFar();
	 void focusNear();
	 void zoomIn();//�Ŵ�
	 void zoomOut();//��С
	 void callPreset1();//����Ԥ��λ1

private:
	
};

class PELCO_P:public PROTOCOL
{
public:
	PELCO_P(){ command.resize(8); command[0] = 0xa0; }
	~PELCO_P(){}

	//void left();//��
	//void right();
	//void up();
	//void down();
	//void stop();
	//void focusFar();
	//void focusNear();
	//void zoomIn();//�Ŵ�
	//void zoomOut();//��С
	//void callPreset1();//����Ԥ��λ1

private:

};

class VISCA:public PROTOCOL
{
public:
	VISCA(){};
	~VISCA(){};

	//void left();//��
	//void right();
	//void up();
	//void down();
	//void stop();
	//void focusFar();
	//void focusNear();
	//void zoomIn();//�Ŵ�
	//void zoomOut();//��С
	//void callPreset1();//����Ԥ��λ1

private:

};