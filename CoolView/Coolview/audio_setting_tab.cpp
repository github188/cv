#include "audio_setting_tab.h"

#include <cassert>
#include <vector>
#include <string>
using std::vector;
using std::string;

#include <QMessageBox>

#include "msdx/config.h"
#include "config/RunningConfig.h"
#include "config/config_helper.h"

#include "controller_manager_interface.h"
#include "display_controller_interface.h"
#include "CoolviewCommon.h"


const char *dsound_renderer_prefix = "DirectSound: ";


AudioSettingTab::AudioSettingTab(
    QWidget *parent, IControllerManager *controller_manager)
    : QWidget(parent) {
  ui.setupUi(this);

  //Audio
  if (controller_manager) {
    // Ϊ��ͬ������UI����Ƶ�Ĳ�����ʹ���ܹ��ڲ�ͬUI����ʾ��ͬ�Ĳ������
    IDisplayController *controller = controller_manager->GetDisplayController();
    assert(controller);
    connect(this, &AudioSettingTab::RequestChangeVolumeSignal,
      controller, &IDisplayController::HandleVoiceChangeRequestSlot);
    connect(controller, &IDisplayController::NotifyVoiceChangeSignal,
      this, &AudioSettingTab::HandleVolumeChangedNotifySlot);
    connect(ui.outputVolumnSlider, &QSlider::valueChanged, 
      this, &AudioSettingTab::HandleOutputVolumeRequestSlot);
    connect(ui.inputVolumnSlider, &QSlider::valueChanged,
      this, &AudioSettingTab::HandleInputVolumeRequestSlot);
  } else {
    // �����и���UI����Ƶ��ͬ������
    connect(ui.outputVolumnSlider, SIGNAL(valueChanged(int)), SLOT(outputVolumnChanged(int)));
    connect(ui.inputVolumnSlider,  SIGNAL(valueChanged(int)), SLOT(inputVolumnChanged(int)));
  }
  connect(ui.audioCodecComboBox, SIGNAL( currentIndexChanged( const QString& )),
    this, SLOT(audioCodecChangedSlot( const QString&)) );
  connect(ui.aecComboBox, SIGNAL( currentIndexChanged( const QString& )),
    this, SLOT(aecChangedSlot( const QString&)) );

  //Save and close
  connect(ui.saveButton , SIGNAL(clicked()) , this , SLOT(SaveConfig()));
}

AudioSettingTab::~AudioSettingTab() {
}

void AudioSettingTab::ReadConfig() {
  RunningConfig* config = RunningConfig::Instance();
  int index = 0;
  //audio input
  QStringList devices;
  ui.inputComboBox_2->clear();
  vector<string> audioInputList;
  DeviceManager::GetInstatnce()->GetAudioCaptureDevices( &audioInputList );
  TransformStringVectorToQStringList(&audioInputList, &devices );
  ui.inputComboBox_2->addItem(QString::fromLocal8Bit("������")); // ע����������Ĭ����
  ui.inputComboBox_2->addItems( devices);

  // ��ȡ��������á����豸������ʱ�ᱻ�ضϣ��Ӷ��п��ܲ���β�ո�
  // ����INI��ȡAPI�����β�ո񣬹���Qt::MatchStartsWithֻƥ�俪ͷ
  // ���������豸�����ضϺ�Ĳ�����ȫһ�������򲻻��������
  index = ui.inputComboBox_2->findText( config->AudioCaptureDevice(),
    Qt::MatchStartsWith );
  ui.inputComboBox_2->setCurrentIndex( index>0 ? index:0 );
  if( ui.inputComboBox_2->currentIndex() > 0 )
  {
    // ��ǰѡ�����0˵������Ч�豸
    int inputVolumnIndex = DeviceManager::GetInstatnce()->GetAudioInputVolumn( 
      ui.inputComboBox_2->currentText().toLocal8Bit().data());
    if( inputVolumnIndex <0 ) inputVolumnIndex=0;
    if( inputVolumnIndex >=100 ) inputVolumnIndex=99;
    ui.inputVolumnSlider->setSliderPosition( inputVolumnIndex );
  }else
  {
    ui.inputVolumnSlider->setSliderPosition(0);
  }

  //audio output
  {
      ui.outputComboBox_2->clear();
      vector<string> audioOutputList;
      DeviceManager::GetInstatnce()->GetAudioOutputDevices( &audioOutputList );

      for (auto device_friendly_name : audioOutputList) {
          //ֻ��ʾ֧�ֻ�����DirectSound�豸�����ش�ͳ�����豸���Լ�ӳ�䵽ϵͳĬ���豸��Default�豸
          //DirectSound�豸��"DirectSound: "(ע������и��ո�)��ͷ
          if (device_friendly_name.find(dsound_renderer_prefix) != 0) 
              continue;
          QString item_data = QString::fromLocal8Bit(device_friendly_name.c_str());
          //Ϊ�˺ÿ�������"DirectSound: "ǰ׺
          QString item_text = item_data.mid(strlen(dsound_renderer_prefix));
          ui.outputComboBox_2->addItem(item_text, QVariant(item_data));
      }
      //TransformStringVectorToQStringList( &audioOutputList , &devices );
      //ui.outputComboBox_2->addItems( devices );
 
      // ��ȡ���������
      QString saved_device_name = config->AudioOutputDevice();
      if (saved_device_name.startsWith(dsound_renderer_prefix)) {
          saved_device_name = saved_device_name.mid(strlen(dsound_renderer_prefix));
      }
      index = ui.outputComboBox_2->findText( saved_device_name, Qt::MatchStartsWith );
      ui.outputComboBox_2->setCurrentIndex( index>0 ? index : 0);

      //��ȡ��������
      if( ui.outputComboBox_2->currentIndex() >= 0 ) //��Ƶ��������ڡ����豸��ѡ��
      {
          int volumnIndex = DeviceManager::GetInstatnce()->GetAudioOutputVolumn(
              config->AudioOutputDevice().toLocal8Bit().data());
          if( 0<volumnIndex && volumnIndex <= 100 )
          {
              //����������
              ui.outputVolumnSlider->setSliderPosition( volumnIndex );
          }
      }else
      {
          ui.outputVolumnSlider->setSliderPosition(0);
      }
  }

  //AEC��������ѡ��
  {
    QStringList aec_type = ConfigHelper::GetUIAudioInputTypeOptions();
    ui.aecComboBox->clear();
    ui.aecComboBox->addItems(aec_type);
    index = ui.aecComboBox->findText( config->AudioInputType() );
    ui.aecComboBox->setCurrentIndex( index>0? index:0);
  }

  //audio codec
  ui.audioCodecComboBox->clear();
  if (audioInputList.size() > 0)
  {
    QStringList audioCodec;
    audioCodec << MSDX_CONF_AUDIO_CODEC_SPEEX << MSDX_CONF_AUDIO_CODEC_AAC;
    ui.audioCodecComboBox->addItems(audioCodec);
  }
  index = ui.audioCodecComboBox->findText( config->AudioCodec() );//��ȡ�����ļ�
  ui.audioCodecComboBox->setCurrentIndex( index>0? index:0); // �����ʺ��������ø��ݴ����ã����audioCodecChangeSlot
}

void AudioSettingTab::SaveConfig() {
  RunningConfig* config = RunningConfig::Instance();
  if (ui.inputComboBox_2->currentIndex() > 0) //0Ϊ������
  {
    config->AudioCaptureDevice(ui.inputComboBox_2->currentText() );
  }
  else
  {
    config->AudioCaptureDevice("");
  }
  {
      int index = ui.outputComboBox_2->currentIndex();
      if (index >= 0) {
          QString audio_output_device = ui.outputComboBox_2->itemData(index).toString();
          config->AudioOutputDevice(audio_output_device);
      }
  }
  config->AudioInputType(ui.aecComboBox->currentText());
  config->AudioCodec(ui.audioCodecComboBox->currentText());
  config->AudioSampleRate(ui.audioSampleRateComboBox->currentText().toInt());
  config->AudioChannel(ui.audioChannelComboBox->currentText().toInt());

  config->saveConfig();
  QMessageBox::information(
    this, QString::fromLocal8Bit("��ʾ") , 
    QString::fromLocal8Bit("����ɹ������ý����´ν������ʱ��Ч��" ), 
    QMessageBox::Ok);
}

void AudioSettingTab::HandleOutputVolumeRequestSlot( int volume ) {
  emit RequestChangeVolumeSignal(kSpeaker, volume);
}

void AudioSettingTab::HandleInputVolumeRequestSlot( int volume ) {
  emit RequestChangeVolumeSignal(kMicrophone, volume);
}

void AudioSettingTab::HandleVolumeChangedNotifySlot( AudioDeviceType type, int volume ) {
  switch (type) {
    case kSpeaker:
      if (ui.outputVolumnSlider->value() != volume)
        ui.outputVolumnSlider->setValue(volume); 
      break;
    case kMicrophone:
      if (ui.inputVolumnSlider->value() != volume)
        ui.inputVolumnSlider->setValue(volume); 
      break;
  }
}

void AudioSettingTab::audioCodecChangedSlot( const QString& codec ) {
  ResetAudioFormatOptions();
}

void AudioSettingTab::outputVolumnChanged( int index ) {
  DeviceManager::GetInstatnce()->SetAudioOutputVolumn( index );
}

void AudioSettingTab::inputVolumnChanged( int index ) {
  DeviceManager::GetInstatnce()->SetAudioInputVolumn( 
    ui.inputComboBox_2->currentText().toStdString() , index );
}

void AudioSettingTab::aecChangedSlot( const QString& )
{
  ResetAudioFormatOptions();
}

void AudioSettingTab::ResetAudioFormatOptions() {
  const QString codec = ui.audioCodecComboBox->currentText();
  QStringList audioSampleRate;
  QStringList audioChannel;
  if (0 != ui.aecComboBox->currentIndex()) {
    //����AECֻ��ʹ�õ���Ƶ��ʽ
    audioSampleRate << "16000";
    audioChannel << "1";
  }
  else if (codec == MSDX_CONF_AUDIO_CODEC_SPEEX) {
    audioSampleRate << "16000" << "32000";
    audioChannel << "1";
  } 
  else if (codec == MSDX_CONF_AUDIO_CODEC_AAC) {
    audioSampleRate << "16000" << "32000" /*<< "44100" << "48000" << "96000"*/;
    audioChannel << "1" << "2";
  }
  else {
    return;
  }

  ui.audioSampleRateComboBox->clear();
  ui.audioSampleRateComboBox->addItems(audioSampleRate);
  ui.audioChannelComboBox->clear();
  ui.audioChannelComboBox->addItems(audioChannel);

  RunningConfig* config = RunningConfig::Instance();
  int index = -1;

  index = ui.audioSampleRateComboBox->findText(QString::number(config->AudioSampleRate()));
  ui.audioSampleRateComboBox->setCurrentIndex(index > 0 ? index : 0);

  index = ui.audioChannelComboBox->findText(QString::number(config->AudioChannel()));
  ui.audioChannelComboBox->setCurrentIndex(index > 0 ? index : 0);
}
