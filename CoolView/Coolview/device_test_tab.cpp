#include "device_test_tab.h"

#include <string>
#include <vector>
using std::vector;
using std::string;

#include "config/RunningConfig.h"
#include "profile/RunningProfile.h"
#include "CoolviewCommon.h"
#include "DeviceManager/DeviceManager.h"
#include "msdx/config.h"
#include <dbus/channel/type/testMedia/common/TestMediaServiceCommon.h>
#include <dbus/performance/monitor/client/CvPerfMonitorIf.h>
#include <dbus/performance/monitor/common/CvPerfMonitorServiceCommon.h>
#include <dbus/performance/monitor/common/MonitorConfigCommon.h>

DeviceTestTab::DeviceTestTab(QWidget *parent)
    : QWidget(parent)
{
  ui.setupUi(this);

  ui.UploadSeverAddressEdit->setText("202.38.192.227");
  isVideoPreview=false;

  //Video
  connect( ui.webcamDeviceTestComboBox , SIGNAL(currentIndexChanged ( const QString & )),
  	this, SLOT(preViewWebcamChangedSlot(const QString& ) ) );
  connect( ui.crossbarlistTestcomboBox, SIGNAL( currentIndexChanged( const QString& )),
  	this , SLOT(preViewCrossbarChangedSlot( const QString&)) );
  connect(ui.videoPreViewButton,SIGNAL(clicked()),this,SLOT(videoPreViewSlot()));
  //Audio
  connect(ui.audioInputTestStartButton ,SIGNAL(clicked()),this,SLOT(startAudioInputTest()));
  connect(ui.audioOutputTestStartButton ,SIGNAL(clicked()),this,SLOT(startAudioOutputTest()));
  connect(ui.audioInputTestStopButton ,SIGNAL(clicked()),this,SLOT(stopAudioInputTest()));
  connect(ui.audioOutputTestStopButton ,SIGNAL(clicked()),this,SLOT(stopAudioOutputTest()));
  //Server
  connect(ui.serverTestButton,SIGNAL(clicked()),this,SLOT(serverTestStartSlot()));
  //Log
  connect( ui.uploadLogButton , SIGNAL(clicked()) , this , SLOT(compressANDUploadLogFileSlot()));
  connect(&uploader, SIGNAL(UploadStateChanged(CUploader::UploadState, QString)), 
  	this, SLOT(OnUploadStateChangedSlot(CUploader::UploadState, QString)));
  connect(&zip, SIGNAL(ZipStateChanged(CZipDir::ZipState, QString)), 
  	this, SLOT(OnZipStateChangedSlot(CZipDir::ZipState, QString)));
}

DeviceTestTab::~DeviceTestTab()
{

}

void DeviceTestTab::ReadConfig()
{
  //���Ӽ����Ϣ���õĶ�ȡ  add by lzb
  RunningConfig* config = RunningConfig::Instance();
  QStringList devices;
  //video
  ui.webcamDeviceTestComboBox->clear();
  {
      VCapDeviceList webcamList;
      DeviceManager::GetInstatnce()->GetVideoCaptureDevices( &webcamList );
      //ui.webcamDeviceComboBox->addItems( devices );
      for (VCapDeviceList::iterator it = webcamList.begin(); it != webcamList.end(); ++it)
      {
          QVariant data(QString::fromStdString(it->devicePath));
          ui.webcamDeviceTestComboBox->addItem(QString::fromStdString(it->friendlyName), data);
      }
  }

  //Audio
  vector<std::string> audioInputList;
  DeviceManager::GetInstatnce()->GetAudioCaptureDevices( &audioInputList );

  ui.audioInputTest_InputDeviceBox->clear();
  TransformStringVectorToQStringList(&audioInputList, &devices );
  ui.audioInputTest_InputDeviceBox->addItems( devices);
  
  vector<std::string> audioOutputList;
  DeviceManager::GetInstatnce()->GetAudioOutputDevices( &audioOutputList );

  ui.audioInputTest_OutputDeviceBox->clear();
  TransformStringVectorToQStringList( &audioOutputList , &devices );
  ui.audioInputTest_OutputDeviceBox->addItems( devices );
    
  ui.audioOutputTestBox->clear();
  TransformStringVectorToQStringList( &audioOutputList , &devices );
  ui.audioOutputTestBox->addItems( devices );
  
  int index = ui.audioInputTest_InputDeviceBox->findText( config->AudioCaptureDevice() );
  ui.audioInputTest_InputDeviceBox->setCurrentIndex( index>0 ? index:0 );
  
  index = ui.audioInputTest_OutputDeviceBox->findText( config->AudioOutputDevice() );
  ui.audioInputTest_OutputDeviceBox->setCurrentIndex( index>0? index:0);
  
  ui.audioOutputTestBox->setCurrentIndex( index>0? index:0);
}

void DeviceTestTab::preViewWebcamChangedSlot( const QString& webcamName )
{
	//�豸����
	ui.videosizeTestcomboBox->clear();
	vector<std::string> videoSizeList;
	DeviceManager::GetInstatnce()->GetVideoDeviceMediaType( webcamName.toLocal8Bit().constData() , &videoSizeList );
	QStringList itemList;
	TransformStringVectorToQStringList( &videoSizeList , &itemList );
	ui.videosizeTestcomboBox->addItems( itemList );

	if( webcamName.contains("webcam", Qt::CaseInsensitive) 
		|| webcamName.contains("e2eSoft", Qt::CaseInsensitive ) )
	{
		//�������������ͷ����ղɼ�������
		ui.crossbarlistTestcomboBox->clear();
		ui.crossbarclassTestcomboBox->clear();
	}else
	{
		//����ɼ���
		ui.crossbarlistTestcomboBox->clear();
		vector<std::string> crossbarList;
		DeviceManager::GetInstatnce()->GetCrossbarDeviceList( webcamName.toLocal8Bit().constData() , &crossbarList );
        ui.crossbarlistTestcomboBox->addItem(QString::fromLocal8Bit("��ʹ��"), QVariant(QString("")));
        ui.crossbarlistTestcomboBox->addItem(QString::fromLocal8Bit("�Զ�ѡ��"), 
            QVariant(QString::fromLocal8Bit(MSDX_CONF_CROSSBAR_AUTO)));
        for (int i = 0; i < crossbarList.size(); i+=2) {
            //��ʾfriendlyName
            QString item_text = QString::fromLocal8Bit(crossbarList[i].c_str());
            //�ڲ�����displayName
            QString item_data = QString::fromLocal8Bit(crossbarList[i+1].c_str());
            ui.crossbarlistTestcomboBox->addItem(item_text, QVariant(item_data));
        }
		//TransformStringVectorToQStringList( &crossbarList , &itemList );
		//ui.crossbarlistTestcomboBox->addItems( itemList );
	}
}


void DeviceTestTab::preViewCrossbarChangedSlot( const QString& crossbar )
{
	ui.crossbarclassTestcomboBox->clear();
    int index = ui.crossbarlistTestcomboBox->currentIndex();
    if (index <= 0) {
        //0Ϊ��ʹ��
        return;
    }

	vector<std::string> crossbarType;
    QString crossbar_displayname = ui.crossbarlistTestcomboBox->itemData(index).toString();
	DeviceManager::GetInstatnce()->GetCrossbarInputType( crossbar_displayname.toLocal8Bit().constData() , &crossbarType );
	QStringList itemList;
	TransformStringVectorToQStringList( &crossbarType , &itemList );
	ui.crossbarclassTestcomboBox->addItems( itemList);
}

void DeviceTestTab::videoPreViewSlot()
{
	if(!isVideoPreview)
	{
		VideoPreviewInfo videoInfo;
		int select_index = -1;
        //cam
        select_index = ui.webcamDeviceTestComboBox->currentIndex();
        if (select_index < 0) return;
		videoInfo.videoCaptureDevice = ui.webcamDeviceTestComboBox->itemData(select_index).toString();

        //crossbar
        select_index = ui.crossbarlistTestcomboBox->currentIndex();
        if (select_index >= 0) {
            videoInfo.videoCrossbar = ui.crossbarlistTestcomboBox->itemData(select_index).toString();
        }
		videoInfo.videoCrossbarType = ui.crossbarclassTestcomboBox->currentText();
		
		//��Ƶ���
		QStringList sizes = ui.videosizeTestcomboBox->currentText().split("X");
		if( sizes.size()>=2 )
		{
			videoInfo.width = sizes.at(0).toUInt();
			videoInfo.height = sizes.at(1).toUInt();
			videoInfo.fps = 30;
			if( sizes.size()==3 )
				videoInfo.fps = sizes.at(2).toUInt();

		}else
		{
            videoInfo.width =0;
			videoInfo.height = 0;
			videoInfo.fps=0;
		}

		videoInfo.preview_wnd =(int)ui.previewFrame->winId();

		ui.videoPreViewButton->setText(QString::fromLocal8Bit("ֹͣԤ��"));
		isVideoPreview=!isVideoPreview;
		
		QByteArray output_bytes;
		QDataStream out( &output_bytes, QIODevice::WriteOnly );
		out.setVersion(QDataStream::Qt_4_4 );
		out << videoInfo;

		CvPerfMonitorIf cvMonitorProxy( PERFORMANCE_MONITOR_SERVICE_NAME , PERFORMANCE_MONITOR_OBJECT_PATH , QDBusConnection::sessionBus() );
		cvMonitorProxy.CreateTestMedia(VIDEO_PREVIEW_MEDIA_ID,output_bytes);
	}
	else
	{

		CvPerfMonitorIf cvMonitorProxy( PERFORMANCE_MONITOR_SERVICE_NAME , PERFORMANCE_MONITOR_OBJECT_PATH , QDBusConnection::sessionBus() );
		cvMonitorProxy.ReleaseTestMedia(VIDEO_PREVIEW_MEDIA_ID);

		ui.videoPreViewButton->setText(QString::fromLocal8Bit("Ԥ��"));
		isVideoPreview=!isVideoPreview;
	}

}

void DeviceTestTab::startAudioInputTest()
{

  QString outPutDevice = ui.audioInputTest_OutputDeviceBox->currentText();
  QString inputPutDevice = ui.audioInputTest_InputDeviceBox->currentText();
  if(outPutDevice == "" || inputPutDevice == "")
    return;

  ui.audioInputTestStopButton->setEnabled(true);
  ui.audioInputTestStartButton->setEnabled(false);
  ui.audioOutputTestStartButton->setEnabled(false);
  ui.audioOutputTestStopButton->setEnabled(false);

  TestMediaInfo testInfo;

  testInfo.audioIutputDevice = inputPutDevice;
  testInfo.audioOutputDevice = outPutDevice;

  QByteArray output_bytes;
  QDataStream out( &output_bytes, QIODevice::WriteOnly );
  out.setVersion(QDataStream::Qt_4_4 );
  out << testInfo;

  CvPerfMonitorIf cvMonitorProxy( PERFORMANCE_MONITOR_SERVICE_NAME , PERFORMANCE_MONITOR_OBJECT_PATH , QDBusConnection::sessionBus() );
  cvMonitorProxy.CreateTestMedia(DEVICE_TEST_MEDIA_ID,output_bytes);

}

void DeviceTestTab::startAudioOutputTest()
{
  QString outPutDevice = ui.audioOutputTestBox->currentText();
  if(outPutDevice == "")
    return;

  ui.audioInputTestStopButton->setEnabled(false);
  ui.audioInputTestStartButton->setEnabled(false);
  ui.audioOutputTestStartButton->setEnabled(false);
  ui.audioOutputTestStopButton->setEnabled(true);

  QString appDir = QApplication::applicationDirPath();
  appDir.replace("/" , "\\" );
  QString audioFilePath = appDir +  "\\Resources\\AudioTest.mp3";

  TestMediaInfo testInfo;

  testInfo.audioIutputDevice = audioFilePath;
  testInfo.audioOutputDevice = outPutDevice;

  QByteArray output_bytes;
  QDataStream out( &output_bytes, QIODevice::WriteOnly );
  out.setVersion(QDataStream::Qt_4_4 );
  out << testInfo;

  CvPerfMonitorIf cvMonitorProxy( PERFORMANCE_MONITOR_SERVICE_NAME , PERFORMANCE_MONITOR_OBJECT_PATH , QDBusConnection::sessionBus() );

  cvMonitorProxy.CreateTestMedia(FILE_TEST_MEDIA_ID,output_bytes);
}

void DeviceTestTab::stopAudioInputTest()
{

  ui.audioInputTestStopButton->setEnabled(false);
  ui.audioInputTestStartButton->setEnabled(true);
  ui.audioOutputTestStartButton->setEnabled(true);
  ui.audioOutputTestStopButton->setEnabled(false);

  CvPerfMonitorIf cvMonitorProxy( PERFORMANCE_MONITOR_SERVICE_NAME , PERFORMANCE_MONITOR_OBJECT_PATH , QDBusConnection::sessionBus() );
  cvMonitorProxy.ReleaseTestMedia(DEVICE_TEST_MEDIA_ID);
}

void DeviceTestTab::stopAudioOutputTest()
{
  ui.audioInputTestStopButton->setEnabled(false);
  ui.audioInputTestStartButton->setEnabled(true);
  ui.audioOutputTestStartButton->setEnabled(true);
  ui.audioOutputTestStopButton->setEnabled(false);

  CvPerfMonitorIf cvMonitorProxy( PERFORMANCE_MONITOR_SERVICE_NAME , PERFORMANCE_MONITOR_OBJECT_PATH , QDBusConnection::sessionBus() );
  cvMonitorProxy.ReleaseTestMedia(FILE_TEST_MEDIA_ID);

}

void DeviceTestTab::serverTestStartSlot()
{
	ui.serverTestButton->setEnabled(false);
	emit serverTestSignal();
}

void DeviceTestTab::recvServerTestResultSlot(bool isTimeOut,int interval)
{
	ui.serverTestButton->setEnabled(true);
	
	QString infoMsg;
	if(!isTimeOut)
	{
		infoMsg = QString::fromLocal8Bit("���յ����Է������Ļظ���ʱ��Ϊ");
		if(interval/1000<1)
		{
			infoMsg=infoMsg+QString::number(interval)+QString::fromLocal8Bit("����");
		}
		else
		{
			interval=interval/1000;
			infoMsg=infoMsg+QString::number(interval)+QString::fromLocal8Bit("��");
		}
	}
	else
	{
		infoMsg = QString::fromLocal8Bit("�ظ���ʱ��");
	}
	ui.testResultText->setText(infoMsg);
}

bool DeviceTestTab::compressANDUploadLogFileSlot()
{
	//����ļ���
	username = QString::fromStdString((RunningProfile::getInstance()->user_uri()));
	int index = username.indexOf( "@" );
	if( index > 0 )
		username = username.left( index );
	
	QDateTime dt;  
	QTime time;  
	QDate date;  
	dt.setTime(time.currentTime());  
	dt.setDate(date.currentDate());  
	compressFileName = username+"_"+ dt.toString("yyyy_MM_dd_hh_mm_ss")+".zip";
	//QString fileName = username+".zip";

	int result=-1;
	
	ui.testResultText->setText(QString::fromLocal8Bit("����ļ���......"));

	result=zip.Create(compressFileName);
	if(result<0)
	{
		ui.testResultText->setText(QString::fromLocal8Bit("��������ļ�ʧ��"));

		return false;
	}

	result=zip.Add("log", "log");
	if(result<0)
	{
		ui.testResultText->setText(QString::fromLocal8Bit("���ʧ��"));
		return false;
	}
		
	zip.Close();
	return true;
}



void DeviceTestTab::OnUploadStateChangedSlot(CUploader::UploadState state, QString msg)
{
	switch (state)
	{
	case CUploader::SEND_DONE:
		
		ui.testResultText->setText(QString::fromLocal8Bit("������־�ļ��ɹ�"));

		//ɾ����ʱ�ļ�
		{
			QDir dir(QDir::currentPath());
			dir.remove(compressFileName);
		}
		break;

	case CUploader::SEND_ERROR:
		ui.testResultText->setText(QString::fromLocal8Bit("������־����"));
		{
			QDir dir(QDir::currentPath());
			dir.remove(compressFileName);
		}
		break;

	case CUploader::SENDING:
		ui.testResultText->setText(QString::fromLocal8Bit("���ڷ�����־�ļ�... %1").arg(msg));
		break;
	}
}

void DeviceTestTab::OnZipStateChangedSlot(CZipDir::ZipState state, QString msg)
{
	switch (state)
	{
	case CZipDir::ZIP_STATE_DONE:
		{

			QString url = "http://%1:24680/UploadLog.aspx";
			url = url.arg(ui.UploadSeverAddressEdit->text());

			uploader.Upload(compressFileName, username, url);

			ui.testResultText->setText(QString::fromLocal8Bit("��������..."));
		}
		break;

	case CZipDir::ZIP_STATE_ERROR:
		ui.testResultText->setText(msg);
		break;

	case CZipDir::ZIP_STATE_PROGRESS:
		ui.testResultText->setText(QString::fromLocal8Bit("�ļ�����ѹ��... %1").arg(msg));
		break;
	}
}
