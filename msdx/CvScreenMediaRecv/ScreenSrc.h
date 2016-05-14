#ifndef SCREEN_SRC_H
#define SCREEN_SRC_H

#include <QtCore/QtCore>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <io.h>
typedef unsigned char BYTE;


//class CvCoreIf;
class CvScreenMediaService;
///��Ļ���ķ���Ŀ��
class ScreenSrc : public QObject
{
	Q_OBJECT
public:
	ScreenSrc( QString ip , int port );
	~ScreenSrc();

//	void resetDst( const QString&ip ,const int port );

	//�ͷ�����
//	void release();

private Q_SLOTS:
	void dealError(QAbstractSocket::SocketError error);

	void dealSocketConnection();

	void imageBufferReady();
	int processData( const int key , const BYTE*data, const int dataLength );

public:
	QString		_ip;			//Ŀ��IP
	int			_port;			//Ŀ��˿�
	QTcpServer  _tcpServer;			//����Զ������
	QTcpSocket*	_remoteSocket;		//��Զ���û�����������socket



	///Socketͨ�ŵ�block�Ĵ�С
	quint32					_blockSize;

	//�洢���յ���ͼƬ���ݵ���ʱ�ļ�
	QString					_recvImageTmpFileName;
	FILE*					_recvImageTmpFile;

	//Qt������ʾͼƬ���õ�ͼƬ�ļ���
	QString					_recvImageFileName;

	CvScreenMediaService*	_service;

	///Coolview Core proxy
	//CvCoreIf*		_cvCoreProxy;

};
#endif