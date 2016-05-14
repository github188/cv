#ifndef AUDIO_SETTING_TAB_H
#define AUDIO_SETTING_TAB_H

#include <QWidget>
#include "ui_audio_setting_tab.h"

#include "media_description.h"

class IControllerManager;

class AudioSettingTab : public QWidget {
  Q_OBJECT
public:
  AudioSettingTab(QWidget *parent = 0, IControllerManager *controller_manager = nullptr);
  ~AudioSettingTab();

Q_SIGNALS:
  void RequestChangeVolumeSignal(AudioDeviceType type, int volume);

public slots:
  void ReadConfig();
  void SaveConfig();

private slots:
  //Audio
  void outputVolumnChanged( int index );//������С�����ı�
  void inputVolumnChanged( int index );//��˷�¼�����������ı�
  void audioCodecChangedSlot( const QString& codec );//����������
  void aecChangedSlot( const QString&);//�����������

  void HandleOutputVolumeRequestSlot(int volume);
  void HandleInputVolumeRequestSlot(int volume);
  void HandleVolumeChangedNotifySlot(AudioDeviceType type, int volume);

private:
  void ResetAudioFormatOptions();

private:
  Ui::AudioSettingTab ui;
};

#endif // AUDIO_SETTING_TAB_H
