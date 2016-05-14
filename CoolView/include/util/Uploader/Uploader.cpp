#include "Uploader.h"
#include <QtCore\QFile>
#include <QtCore\QDataStream>
#include <QUrl>
#include <QtNetwork\QNetworkRequest>
//#include <QtDebug>
#include <QMessageBox>

CUploader::CUploader(QObject *parent)
	: QObject(parent)
	, _uploadManager(NULL)
	, _reply(NULL)
	, _uploadBuf(NULL)
{

}

CUploader::~CUploader()
{
	if( _uploadBuf != NULL )
	{
		delete[] _uploadBuf;
	}

	if (_uploadManager)
	{
		delete _uploadManager;
	}
}

void CUploader::Upload( const QString &filename, const QString &clientid, const QString &url )
{

	QFile file(filename);
	file.open(QIODevice::ReadOnly);
	int file_len = file.size();

	QDataStream in(&file);
	_uploadBuf = new char[file_len];
	in.readRawData( _uploadBuf, file_len);
	file.close();


	//׼��ͷ��Ϣ
	QNetworkRequest request(url);

	//��ʼ��
	_uploadManager = new QNetworkAccessManager();

	//������Ҫ����Ȼ���������ļ�����������˵��ȫ����
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
	request.setRawHeader("clientname", clientid.toUtf8());

	//׼������
	QByteArray arr = QByteArray( _uploadBuf, file_len );
	_reply = _uploadManager->post( request , arr );
	emit UploadStateChanged(LINKING, "");

	connect(_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(OnUpLoadError(QNetworkReply::NetworkError)));
	connect(_reply, SIGNAL(uploadProgress (qint64, qint64)), this, SLOT(OnUploadProgress(qint64 ,qint64)));
	connect(_reply, SIGNAL(finished()), this, SLOT(OnUploadFinished()));
}

void CUploader::OnUploadFinished()
{
	//QMessageBox::information(NULL, tr("Uploader"), tr("Upload finished!"));
	Reset();
	emit UploadStateChanged(SEND_DONE, "");
}

void CUploader::OnUpLoadError( QNetworkReply::NetworkError errorCode )
{
	//QMessageBox::critical(NULL, tr("Uploader"), QString::number(errorCode));
	Reset();
	emit UploadStateChanged(SEND_ERROR, "");
}

void CUploader::OnUploadProgress( qint64 bytesSent, qint64 bytesTotal )
{
	if (bytesTotal != 0)
	{
		emit UploadStateChanged(SENDING, QString("%1%").arg(100 * bytesSent / bytesTotal));
	}
	else
	{
		emit UploadStateChanged(SENDING, QString("%1%").arg(0));
	}
}

void CUploader::Reset()
{
	//����Է�ֹ�ڴ�й¶
	if (_uploadManager)
	{
		disconnect(_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(OnUpLoadError(QNetworkReply::NetworkError)));
		disconnect(_reply, SIGNAL(uploadProgress (qint64, qint64)), this, SLOT(OnUploadProgress(qint64 ,qint64)));
		disconnect(_reply, SIGNAL(finished()), this, SLOT(OnUploadFinished()));

		if (_reply) _reply->deleteLater(); //��Ҫֱ��Delete
		_reply = NULL;

		_uploadManager->deleteLater(); //��Ҫֱ��Delete
		_uploadManager = NULL;
	}

	if (_uploadBuf)
	{
		delete _uploadBuf;
		_uploadBuf = NULL;
	}
}
