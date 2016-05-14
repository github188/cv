#include "video_setting_tab.h"

#include <assert.h>

#include <vector>
#include <string>
using std::vector;
using std::string;

#include <QMessageBox>

#include "config/RunningConfig.h"
#include "util/ini/CVIniConfig.h"
#include "msdx/config.h"
#include "util/MediaManager.h"
#include "CoolviewCommon.h"
#include "DeviceManager/DeviceManager.h"

VideoSettingTab::VideoSettingTab(QWidget *parent)
    : QWidget(parent) {
  ui.setupUi(this);

  //����kinect����
  ui.ckbKinectDevice->hide();

  //���ص���Ƶ���ʿ��ƺ�ͬ��ƫ������checkbox��ò�ƹ���������
  ui.ckbAdaptiveCodeRate->hide();
  ui.ckbRecvAutoResync->hide();

  //Video
  connect(ui.cameraIndexComboBox , SIGNAL(currentIndexChanged ( const QString & )),
    this, SLOT(cameraIndexChangedSlot(const QString& ) ) );
  connect(ui.webcamDeviceComboBox , SIGNAL(currentIndexChanged ( const QString & )),
    this, SLOT(webcamChangedSlot(const QString& ) ) );
  connect(ui.crossbarlistcomboBox, SIGNAL( currentIndexChanged( const QString& )),
    this , SLOT(crossbarChangedSlot( const QString&)) );

  //Advance
  connect(ui.ckbRecvAutoResync, SIGNAL(stateChanged(int)), SLOT( ckbRecvAutoResync_StateChanged(int)) );
  connect(ui.ckbAdaptiveCodeRate, SIGNAL(stateChanged(int)), SLOT( ckbAdaptiveCodeRate_StateChanged(int)) );
  //Save and close
  connect(ui.saveButton , SIGNAL(clicked()) , this , SLOT(SaveConfig()));
}

VideoSettingTab::~VideoSettingTab() {

}

void VideoSettingTab::ReadConfig() {
  RunningConfig* config = RunningConfig::Instance();

  //video
  //cam
  ui.webcamDeviceComboBox->clear();
  {
    VCapDeviceList webcamList;
    DeviceManager::GetInstatnce()->GetVideoCaptureDevices( &webcamList );
    //TransformStringVectorToQStringList( &webcamList , &devices );
    ui.webcamDeviceComboBox->addItem(QString::fromLocal8Bit("������"));
    //ui.webcamDeviceComboBox->addItems( devices );
    for (VCapDeviceList::iterator it = webcamList.begin(); it != webcamList.end(); ++it)
    {
		if (it->friendlyName=="Kinect Camera V2")//��ΪKinect�Ҳ���devicePath,����displayname�����������Kinect��Ӧdevicepath��Ϊdisplayname--hjf 2015.12.22
		{
			QVariant data(QString::fromStdString(it->displayName));
			ui.webcamDeviceComboBox->addItem(QString::fromStdString(it->friendlyName), data);
		}
		else
		{
		    QVariant data(QString::fromStdString(it->devicePath));
			ui.webcamDeviceComboBox->addItem(QString::fromStdString(it->friendlyName), data);
		}
    }
  }

  //crossbar
  ui.crossbarlistcomboBox->clear();
  vector<string> crossbarList;
  DeviceManager::GetInstatnce()->GetCrossbarDeviceList("", &crossbarList); //��һ�������Ƿϵ�

  ui.crossbarlistcomboBox->addItem(QString::fromLocal8Bit("��ʹ��"), QVariant(QString("")));
  ui.crossbarlistcomboBox->addItem(QString::fromLocal8Bit("�Զ�ѡ��"), 
      QVariant(QString::fromLocal8Bit(MSDX_CONF_CROSSBAR_AUTO)));
  for (int i = 0; i < crossbarList.size(); i+=2) {
      //��ʾfriendlyName
      QString item_text = QString::fromLocal8Bit(crossbarList[i].c_str());
      //�ڲ�����displayName
      QString item_data = QString::fromLocal8Bit(crossbarList[i+1].c_str());
      ui.crossbarlistcomboBox->addItem(item_text, QVariant(item_data));
  }
  //TransformStringVectorToQStringList( &crossbarList , &itemList ); //�ɵ�ֻ��ʾfriendlyName
  //ui.crossbarlistcomboBox->addItems( itemList );

  //������cam��crossbar���������Ŷ����Ȼ�޷���ʾ��֮ǰ���õ�ֵ
  QStringList videoCapIndex;
  for (int i = 1; i <= config->VideoCaptureDeviceCount(); ++i)
  {
    videoCapIndex << QString::number(i);
  }
  ui.cameraIndexComboBox->clear();
  ui.cameraIndexComboBox->addItems(videoCapIndex);
  ui.cameraIndexComboBox->setCurrentIndex(0);

  ui.ckbRecvAutoResync->setChecked(config->isEnableRecvAutoResync());
  ui.ckbAdaptiveCodeRate->setChecked(config->isEnableAdaptiveCodeRate());

  ui.ckbKinectDevice->setChecked( config->isEnableKinect() );

  //ͨ����ȡ�����ļ�����ͼ��������������ʱ�������     add by lzb
  QString appDir = QApplication::applicationDirPath();
  appDir.replace("/" , "\\" );
  QString filePath = appDir +  "\\CvX264.ini";
  QSettings *ConfigIni = new QSettings(filePath,QSettings::IniFormat,0);   
  ui.videoQualitySlider->setSliderPosition( ConfigIni->value("/RC/crf","29").toInt());
  delete ConfigIni;
}

void VideoSettingTab::SaveConfig() {
  RunningConfig* config = RunningConfig::Instance();

  VCapSetting vcap;

  const int vIndex = ui.cameraIndexComboBox->currentIndex();
  if (vIndex < 0)
  {
    QMessageBox::information(
      this, QString::fromLocal8Bit("��ʾ") , 
      QString::fromLocal8Bit("��Ч������ͷ���" ), 
      QMessageBox::Ok 
      );
    return;
  }
  vcap.index = vIndex;

  if (0 < ui.webcamDeviceComboBox->currentIndex()) // 0Ϊ���豸
  {
    vcap.friendlyName = ui.webcamDeviceComboBox->currentText();
    vcap.devicePath = ui.webcamDeviceComboBox->itemData(ui.webcamDeviceComboBox->currentIndex()).toString();
  }

  //crossbar
  {
      int index = ui.crossbarlistcomboBox->currentIndex();
      if (index > 0) {
          vcap.crossBarName = ui.crossbarlistcomboBox->currentText();
          vcap.crossBarDisplayName = ui.crossbarlistcomboBox->itemData(index).toString();
          vcap.crossBarType = ui.crossbarclasscomboBox->currentText();
      } else {
          //0Ϊ��ʹ�ã�-1Ϊδѡ��
          vcap.crossBarName = "";
          vcap.crossBarDisplayName = "";
          vcap.crossBarType = "";
      }
  }

  QStringList sizes = ui.videosizecomboBox->currentText().split("X");
  if( sizes.size()>=2 )
  {
    vcap.width = sizes.at(0).toUInt();
    vcap.height = sizes.at(1).toUInt();
    vcap.fps = 30;
    if( sizes.size()==3 )
      vcap.fps = sizes.at(2).toUInt();
  }
  config->VideoCaptureDevice(vIndex, vcap);

  config->EnableKinect( ui.ckbKinectDevice->isChecked() );

  config->saveConfig();

  QMessageBox::information(
    this, QString::fromLocal8Bit("��ʾ") , 
    QString::fromLocal8Bit("����ɹ������ý����´ν������ʱ��Ч��" ), 
    QMessageBox::Ok 
    );

  if( ui.ckbKinectDevice->isChecked() )
  {
    //���֧��kinect����ô����kinect����
    startKinectDaemon();
  }else
  {
    //����ر�kinect����
    stopKinectDaemon();
  }

  //ͨ���޸������ļ�����ͼ��������������ʱ�������     add by lzb
  QString appDir = QApplication::applicationDirPath();
  appDir.replace("/" , "\\" );
  QString filePath = appDir +  "\\cfg\\CvX264.ini";
  QSettings *ConfigIni = new QSettings(filePath,QSettings::IniFormat,0);   
  ConfigIni->setValue("/RC/crf",(ui.videoQualitySlider->value()));  
  delete ConfigIni;
}

void VideoSettingTab::cameraIndexChangedSlot( const QString& indexStr ) {
  const int vIndex = ui.cameraIndexComboBox->currentIndex();
  assert(vIndex == indexStr.toInt() - 1); // ע����ӱ�Ŵ�1��ʼ

  RunningConfig* config = RunningConfig::Instance();

  if (vIndex >= config->VideoCaptureDeviceCount())
  {
    ui.webcamDeviceComboBox->setCurrentIndex( 0 );
  }

  VCapSetting setting = config->VideoCaptureDevice(vIndex);
  // ��ȡ��������á����豸������ʱ�ᱻ�ضϣ��Ӷ��п��ܲ���β�ո�
  // ����INI��ȡAPI�����β�ո񣬹���Qt::MatchStartsWithֻƥ�俪ͷ
  // ���������豸�����ضϺ�Ĳ�����ȫһ�������򲻻��������
  int index = ui.webcamDeviceComboBox->findText( setting.friendlyName, Qt::MatchStartsWith );
  ui.webcamDeviceComboBox->setCurrentIndex( index>0 ? index : 0 );

  index = ui.crossbarlistcomboBox->findText( setting.crossBarName, Qt::MatchStartsWith );
  ui.crossbarlistcomboBox->setCurrentIndex( index>0 ? index: 0 );

  index = ui.crossbarclasscomboBox->findText( setting.crossBarType, Qt::MatchStartsWith );
  ui.crossbarclasscomboBox->setCurrentIndex( index>0?index:0);

  QString videoSize = QString::number( setting.width) + "X" + QString::number(setting.height) + "X"+ QString::number(setting.fps);
  index = ui.videosizecomboBox->findText( videoSize );
  ui.videosizecomboBox->setCurrentIndex( index>0?index:0);
}

void VideoSettingTab::webcamChangedSlot( const QString& webcamName ) {
  //��·�ظ��Լ�� - Liaokz
  if (ui.webcamDeviceComboBox->currentIndex() > 0)
  {
    const int vIndex = ui.cameraIndexComboBox->currentIndex();
    const QString devicePath = ui.webcamDeviceComboBox->itemData(ui.webcamDeviceComboBox->currentIndex()).toString();
    RunningConfig *config = RunningConfig::Instance();

    int i = 0;
    for (; i < config->VideoCaptureDeviceCount(); ++i)
    {
      VCapSetting setting = config->VideoCaptureDevice(i);

      if (setting.index != vIndex && 
        setting.devicePath == devicePath)
      {
        QString msg = QString::fromLocal8Bit("��ǰ��Ƶ�������%1·��Ƶ���ó�ͻ���Ƿ�رյ�%1·��Ƶ��").arg(setting.index);
        if (QMessageBox::Yes == QMessageBox::question(this, QString::fromLocal8Bit("���ó�ͻ"), msg))
        {
          VCapSetting vcap;
          vcap.index = setting.index;
          config->VideoCaptureDevice(vcap.index, vcap); //����ͻ����Ƶͨ������Ϊ���豸,���ݲ�����,ֱ����Ҫʱ�ٱ���
        }
        else
        {
          ui.webcamDeviceComboBox->setCurrentIndex(0);
          return;
        }
      }
    }
  }

  //�豸����
  ui.videosizecomboBox->clear();
  vector<string> videoSizeList;
  DeviceManager::GetInstatnce()->GetVideoDeviceMediaType( webcamName.toLocal8Bit().constData() , &videoSizeList );
  QStringList itemList;
  TransformStringVectorToQStringList( &videoSizeList , &itemList );
  ui.videosizecomboBox->addItems( itemList );

  if( webcamName.contains("webcam", Qt::CaseInsensitive) 
    || webcamName.contains("e2eSoft", Qt::CaseInsensitive ) )
  {
    //�������������ͷ����ղɼ�������
    ui.crossbarlistcomboBox->setCurrentIndex(0); //����Ϊ��ʹ��
    ui.crossbarlistcomboBox->setEnabled(false);
  } else {
    ui.crossbarlistcomboBox->setEnabled(true);
  }
}

void VideoSettingTab::crossbarChangedSlot( const QString& crossbar ) {
  ui.crossbarclasscomboBox->clear();
  int index = ui.crossbarlistcomboBox->currentIndex();
  if (index <= 1) {
      //0Ϊ��ʹ�ã�1Ϊ�Զ�
      return;
  }
  vector<string> crossbarType;
  QString crossbar_displayname = ui.crossbarlistcomboBox->itemData(index).toString();
  DeviceManager::GetInstatnce()->GetCrossbarInputType( crossbar_displayname.toLocal8Bit().constData() , &crossbarType );

  QStringList itemList;
  TransformStringVectorToQStringList( &crossbarType , &itemList );
  ui.crossbarclasscomboBox->addItems( itemList);
}

void VideoSettingTab::ckbRecvAutoResync_StateChanged( int state ) {
  bool enable =( state == Qt::CheckState::Checked);
  if(RunningConfig::Instance()->isEnableRecvAutoResync() !=enable){
    RunningConfig::Instance()->EnableRecvAutoResync(enable);
    MediaManager::getInstance()->EnableRecvAutoResync(enable);
  }
}

void VideoSettingTab::ckbAdaptiveCodeRate_StateChanged( int state ) {
  bool enable =( state == Qt::CheckState::Checked);
  RunningConfig::Instance()->EnableAdaptiveCodeRate(enable);
  MediaManager::getInstance()->EnableAdaptiveCodeRate(enable);
}

void VideoSettingTab::ChangeCameraIndex( const int index ) {
  ui.cameraIndexComboBox->setCurrentIndex(index);
}
