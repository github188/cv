#include "config_helper.h"
#include "msdx/config.h"


ConfigHelper::ConfigHelper(void)
{
}


ConfigHelper::~ConfigHelper(void)
{
}

QStringList ConfigHelper::GetUIAudioInputTypeOptions()
{
  //TODO:�Ժ���ʱ��ĳ�������ʽ����ֱ��д��
  QStringList list;
  list << QString::fromLocal8Bit("��ʹ��") 
    << QString::fromLocal8Bit("Windows AEC");
  return list;
}

bool ConfigHelper::GetAudioInputTypeByUIOption( 
  const QString& option, int &config )
{
  config = -1;
  if (option == QString::fromLocal8Bit("��ʹ��")) {
    config = MSDX_CONF_AUDIO_INPUT_NOAEC;
  } else if (option == QString::fromLocal8Bit("Windows AEC")) {
    config = MSDX_CONF_AUDIO_INPUT_WINAEC;
  }
  if (config > -1) {
    return true;
  }
  config = 0;
  return false;
}
