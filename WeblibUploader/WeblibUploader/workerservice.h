#ifndef _WORKERSERVICE_H_
#define _WORKERSERVICE_H_

#include <QObject>
#include "commonstruts.h"
#include "dllexports.h"

class FileUploader;

class WEBLIB_DLLSPEC WorkerService : public QObject
{
	Q_OBJECT

public:
	static WorkerService &getInstance()
	{
		static WorkerService g_instance;
		return g_instance;
	}

	~WorkerService();
public slots:
	void slotStartUpload();
	void slotStopUpload();

	WeblibSettingInfo readOrignalSettings();					//��ȡ�����ļ��еĲ���ֵ
	void slotSetWeblibSetting(const WeblibSettingInfo& info);	//�����ϴ�������������д�������ļ�

	//��������������Ҫ����ֵ
	WeblibSettingInfo getWeblibSetting();	//��ȡ��ǰʵʱ���õĲ���ֵ
	bool saveSettings();					//ִ�������ļ�д�����
private slots:
	void loginFailed(const QString &info);

signals:
	void signalItemStateChanged(ItemOperate oper, const UploadItem &item);
	void signalShowErrorInfo(const QString &info);
private:
	WorkerService();
	WorkerService(WorkerService const &param);
	WorkerService &operator = (WorkerService const &param);

private:
	FileUploader *m_uploader;
};

#endif