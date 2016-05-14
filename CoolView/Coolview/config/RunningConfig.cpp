#include "RunningConfig.h"
#include "util/ini/CVIniConfig.h"
#include <cassert>

#include <QtCore/QCoreApplication>
#include <QApplication>
#include <QDesktopWidget>

#include "manualconfig.h"
#include "util/QoSDef.h"
#include "Util\ini\Ini.h"
#include "msdx\config.h"

#include "display_controller_interface.h"

//ȡ�� unicode
#ifdef _UNICODE
#undef    _UNICODE
#endif
#ifdef UNICODE
#undef  UNICODE
#endif 

#define MAX_CAMERA_COUNT 4

#define SECTION_NAME_VIDEO   "Video"
#define SECTION_NAME_AUDIO   "Audio"
#define SECTION_NAME_GLOBAL  "Global"
#define SECTION_NAME_UI "UI"
#define SECTION_NAME_MONITOR "Monitor"
#define SECTION_NAME_PTZ "PTZ"

#define KEY_VIDEO_COUNT      "VideoCaptureCount"
#define KEY_VIDEO_CAP_NAME   "VideoCaptureFriendlyName"
#define KEY_VIDEO_CAP_PATH   "VideoCapturePath"
#define KEY_VIDEO_CROSSBAR   "CrossBarDevice"
#define KEY_VIDEO_CROSSBAR_DISPLAYNAME   "CrossBarDisplayName"
#define KEY_VIDEO_CROSSTYPE  "CrossBarType"
#define KEY_VIDEO_WIDTH      "VideoWidth"
#define KEY_VIDEO_HEIGHT     "VideoHeight"
#define KEY_VIDEO_FPS        "VideoFps"
#define KEY_VIDEO_CODEC      "VideoCodec"

#define KEY_AUDIO_CAPTURE    "AudioCaptureDevice"
#define KEY_AUDIO_OUTPUT     "AudioOutputDevice"
#define KEY_AUDIO_INPUT_TYPE "AudioInputType"
#define KEY_AUDIO_CODEC      "AudioCodec"
#define KEY_AUDIO_SAMPLERATE "SampleRate"
#define KEY_AUDIO_CHANNEL    "Channel"
#define KEY_AUDIO_BITSPERSAMPLE  "BitsPerSample"

#define KEY_GLOBAL_SERVICE       "GlobalService"
#define KEY_GLOBAL_QOS_URI       "QosServerURI"
#define KEY_GLOBAL_AUTO_RESYNC   "AudioAutoResync"
#define KEY_GLOBAL_ENABLE_KINECT "EnableKinect"
#define KEY_GLOBAL_ENABLE_SMALL_VIDEO  "EnableSmallVideo"
#define KEY_GLOBAL_ENABLE_LOCAL_RECORD "EnableLocalRecord"
#define KEY_GLOBAL_ENABLE_HTTP   "EnableHttp"

#define KEY_UI_MAIN_SCREEN "MainScreen"
#define KEY_UI_ENABLE_CONTROL_PANEL_BOOTER "EnableControlPanelBooter"

#define KEY_MONITOR_CPU_TEMP_THRES        "CPUTempThreshold"
#define KEY_MONITOR_CPU_TEMP_ACTION       "CPUTempAction"
#define KEY_MONITOR_MAINBOARD_TEMP_THRES  "MainboardTempThreshold"
#define KEY_MONITOR_MAINBOARD_TEMP_ACTION "MainboardTempAction"

#define KEY_PTZ_SERIALPORT "SerialPort"
#define KEY_PTZ_PROTOCOL "Protocol"
#define KEY_PTZ_CAMERA_ADDR "CameraAddr"
#define KEY_PTZ_BUADBITS "BuadBits"
#define KEY_PTZ_DATABITS "DataBits"
#define KEY_PTZ_STOPBITS "StopBits"
#define KEY_PTZ_PARITY "Parity"

static const int UI_DEFAULT_SCREEN_LAYOUT = 
  static_cast<int>(IDisplayController::kLayoutOf2x2);
static const int UI_CONTROL_PANEL_LAYOUT = 
  static_cast<int>(IDisplayController::kLayoutOfTop1x2);

void VerifyVideoSetting(VCapSetting & setting)
{
	//���ȳ���ȡ�ø��豸����ѷֱ���
	vector<string> mediaType;
	DeviceManager::GetInstatnce()->GetVideoDeviceMediaType(setting.friendlyName.toStdString(), &mediaType);

	int default_width = 0;
	int default_height = 0;
	int default_fps = 0;

	int i = 0;
	for (; i < mediaType.size(); ++i)
	{
		int width = 0;
		int height = 0;
		int fps = 0;
		sscanf(mediaType.at(i).c_str(), "%dX%dX%d", &width, &height, &fps);
		if (width == setting.width && height == setting.height)
		{
			//�豸֧���趨�ķֱ���ʱ�������û�����
			return;
		}
		//VGA�ɼ��豸Ĭ��ʹ��720p
		if (setting.friendlyName.indexOf("VGA2USB") != -1 &&
			width == 1280 && height == 720)
		{
			default_width = width;
			default_height = height;
			default_fps = fps;
		}
		//USB����ͷ����720p�ֱ���(���֧��)
		else if (setting.devicePath.indexOf("\\\\?\\usb#", Qt::CaseInsensitive) == 0 &&
			width == 1280 && height == 720)
		{
			default_width = width;
			default_height = height;
			default_fps = fps;
		}
		//PCI�ɼ����豸����1080p�ֱ���
		else if (setting.devicePath.indexOf("\\\\?\\pci#", Qt::CaseInsensitive) == 0 &&
			width == 1920 && height == 1080)
		{
			default_width = width;
			default_height = height;
			default_fps = fps;
		}
		//�����豸������ѷֱ���
		else if (width > default_width)
		{
			default_width = width;
			default_height = height;
			default_fps = fps;
		}
	}

	//û���ҵ�֮ǰ���õķֱ���,����ΪĬ�Ϸֱ���
	// TODO:��VGA�豸û������ʱ,width,hegiht��Ϊ0.Ӧ��ͨ��VGA�豸��SDK��������DSHOW�еĹ̶��ֱ��� - Liaokz
	setting.width = default_width;
	setting.height = default_height;
	setting.fps = default_fps;
}

RunningConfig::RunningConfig()
{
	QString appDir = QCoreApplication::applicationDirPath();
	appDir.replace("/" , "\\" );
	_configFilePath = appDir + "\\cfg\\config.ini";

	//������yyyyyyyd
	loadConfig();
}

void RunningConfig::saveConfig()
{
    CIni ini;
    ini.SetPathName(_configFilePath.toLocal8Bit().data());

	ini.WriteString(SECTION_NAME_GLOBAL, KEY_GLOBAL_QOS_URI, _qosServerURI.toLocal8Bit().data());
	ini.WriteBool  (SECTION_NAME_GLOBAL, KEY_GLOBAL_AUTO_RESYNC, _enableRecvAutoResync);
	ini.WriteBool  (SECTION_NAME_GLOBAL, KEY_GLOBAL_ENABLE_KINECT, _enableKinect);
	ini.WriteBool  (SECTION_NAME_GLOBAL, KEY_GLOBAL_ENABLE_SMALL_VIDEO, _enableSmallVideo);
	ini.WriteBool  (SECTION_NAME_GLOBAL, KEY_GLOBAL_ENABLE_LOCAL_RECORD, _enableLocalRecord);
	ini.WriteBool  (SECTION_NAME_GLOBAL, KEY_GLOBAL_ENABLE_HTTP, _enableHttp);

	ini.WriteString(SECTION_NAME_AUDIO, KEY_AUDIO_CAPTURE, _audioCaptureDevice.toLocal8Bit().data());
    ini.WriteString(SECTION_NAME_AUDIO, KEY_AUDIO_OUTPUT, _audioOutputDevice.toLocal8Bit().data());
    ini.WriteString(SECTION_NAME_AUDIO, KEY_AUDIO_INPUT_TYPE, _audioInputType.toLocal8Bit().data());
	ini.WriteString(SECTION_NAME_AUDIO, KEY_AUDIO_CODEC, _audioCodec.toLocal8Bit().data());
	ini.WriteInt   (SECTION_NAME_AUDIO, KEY_AUDIO_SAMPLERATE, _audioSampleRate);
	ini.WriteInt   (SECTION_NAME_AUDIO, KEY_AUDIO_CHANNEL,    _audioChannel);
	ini.WriteInt   (SECTION_NAME_AUDIO, KEY_AUDIO_BITSPERSAMPLE, _audioBitsPerSample);

	//ע����Ƶͨ�����������ڵ�һ����Ƶ���ýڵ���
	ini.WriteInt(SECTION_NAME_VIDEO, KEY_VIDEO_COUNT, _videoCapDeviceCount);
	for (int i = 0; i < _videoCapDeviceCount; ++i)
	{
		if (i >= _videoCapSettings.size())
		{
			break;
		}
		char sec[64] = {0};
		if (0 == i)
		{
			sprintf_s(sec, sizeof(sec), SECTION_NAME_VIDEO);
		}
		else
		{
			sprintf_s(sec, sizeof(sec), SECTION_NAME_VIDEO"%d", i);
		}

		ini.WriteString(sec, KEY_VIDEO_CAP_NAME, _videoCapSettings[i].friendlyName.toLocal8Bit().data());
		ini.WriteString(sec, KEY_VIDEO_CAP_PATH, _videoCapSettings[i].devicePath.toLocal8Bit().data());
        ini.WriteString(sec, KEY_VIDEO_CROSSBAR, _videoCapSettings[i].crossBarName.toLocal8Bit().data());
        ini.WriteString(sec, KEY_VIDEO_CROSSBAR_DISPLAYNAME, _videoCapSettings[i].crossBarDisplayName.toLocal8Bit().data());
		ini.WriteString(sec, KEY_VIDEO_CROSSTYPE, _videoCapSettings[i].crossBarType.toLocal8Bit().data());
		ini.WriteInt   (sec, KEY_VIDEO_WIDTH, _videoCapSettings[i].width);
		ini.WriteInt   (sec, KEY_VIDEO_HEIGHT, _videoCapSettings[i].height);
		ini.WriteInt   (sec, KEY_VIDEO_FPS, _videoCapSettings[i].fps);
		ini.WriteString(sec, KEY_VIDEO_CODEC, _videoCapSettings[i].videoCodec.toLocal8Bit().data());
	}

    // save UI settings
    ini.WriteInt(SECTION_NAME_UI, KEY_UI_MAIN_SCREEN, _mainScreen);
    ini.WriteBool(SECTION_NAME_UI, KEY_UI_ENABLE_CONTROL_PANEL_BOOTER, _enableControlPanelBooter);

    // save monitor settings
    ini.WriteDouble(
      SECTION_NAME_MONITOR, 
      KEY_MONITOR_MAINBOARD_TEMP_THRES, 
      _mainboardTempThreshold);
    ini.WriteInt(
      SECTION_NAME_MONITOR, 
      KEY_MONITOR_MAINBOARD_TEMP_ACTION,
      _mainboardTempAction);

    ini.WriteDouble(
      SECTION_NAME_MONITOR, KEY_MONITOR_CPU_TEMP_THRES, _cpuTempThreshold);
    ini.WriteInt(
      SECTION_NAME_MONITOR, KEY_MONITOR_CPU_TEMP_ACTION, _cpuTempAction);

    // save ptz settings
    ini.WriteString(SECTION_NAME_PTZ, KEY_PTZ_SERIALPORT, _serialportName.toLocal8Bit().data());
    ini.WriteInt(SECTION_NAME_PTZ, KEY_PTZ_PROTOCOL, _protocol);
    ini.WriteInt(SECTION_NAME_PTZ, KEY_PTZ_CAMERA_ADDR, _cameraAddr);
    ini.WriteInt(SECTION_NAME_PTZ, KEY_PTZ_BUADBITS, _buadBits);
    ini.WriteInt(SECTION_NAME_PTZ, KEY_PTZ_DATABITS, _dataBits);
    ini.WriteInt(SECTION_NAME_PTZ, KEY_PTZ_STOPBITS, _stopBits);
    ini.WriteInt(SECTION_NAME_PTZ, KEY_PTZ_PARITY, _parity);
}

void RunningConfig::loadConfig()
{
	const int BUF_SIZE = 256;
	char buf[BUF_SIZE] = {0};

    CIni ini;
    ini.SetPathName(_configFilePath.toLocal8Bit().data());

	// Global section
    ini.GetString(SECTION_NAME_GLOBAL, KEY_GLOBAL_QOS_URI, buf, BUF_SIZE, DEFAULT_QOS_SERVER_SIP_URI);
	_qosServerURI = QString::fromLocal8Bit(buf);

	_enableRecvAutoResync = ini.GetBool(SECTION_NAME_GLOBAL, KEY_GLOBAL_AUTO_RESYNC, false);
	_enableKinect = ini.GetBool(SECTION_NAME_GLOBAL, KEY_GLOBAL_ENABLE_KINECT, false);
	_enableSmallVideo = ini.GetBool(SECTION_NAME_GLOBAL, KEY_GLOBAL_ENABLE_SMALL_VIDEO, true);
	_enableLocalRecord = ini.GetBool(SECTION_NAME_GLOBAL, KEY_GLOBAL_ENABLE_LOCAL_RECORD, false);
	_enableHttp = ini.GetBool(SECTION_NAME_GLOBAL, KEY_GLOBAL_ENABLE_HTTP, false);

	// Audio section
	ini.GetString(SECTION_NAME_AUDIO, KEY_AUDIO_CAPTURE, buf, BUF_SIZE, "");
	_audioCaptureDevice = QString::fromLocal8Bit(buf);

	ini.GetString(SECTION_NAME_AUDIO, KEY_AUDIO_OUTPUT, buf, BUF_SIZE, "");
    _audioOutputDevice = QString::fromLocal8Bit(buf);

    ini.GetString(SECTION_NAME_AUDIO, KEY_AUDIO_INPUT_TYPE, buf, BUF_SIZE, "");
    _audioInputType = QString::fromLocal8Bit(buf);

	ini.GetString(SECTION_NAME_AUDIO, KEY_AUDIO_CODEC, buf, BUF_SIZE, MSDX_CONF_AUDIO_CODEC_SPEEX);
	_audioCodec = QString::fromLocal8Bit(buf);
	if (_audioCodec != MSDX_CONF_AUDIO_CODEC_SPEEX && _audioCodec != MSDX_CONF_AUDIO_CODEC_AAC)
	{
		_audioCodec = MSDX_CONF_AUDIO_CODEC_SPEEX;
	}

	_audioSampleRate = ini.GetInt(SECTION_NAME_AUDIO, KEY_AUDIO_SAMPLERATE, MSDX_CONF_AUDIO_SAMPLERATE_DEFAULT);
	if (_audioSampleRate % 22050 != 0 && _audioSampleRate % 16000 != 0) // ��׼�豸������Ϊ8k,16k,...,96k or 11025,22050,44100�����ǲ�ʹ��16k���²�����
	{
		_audioSampleRate = MSDX_CONF_AUDIO_SAMPLERATE_DEFAULT;
	}

	_audioChannel = ini.GetInt(SECTION_NAME_AUDIO, KEY_AUDIO_CHANNEL, MSDX_CONF_AUDIO_CHANNEL_DEFAULT);
	if (_audioChannel <= 0 || _audioChannel >= 3) // 1ch and 2ch only
	{
		_audioChannel = MSDX_CONF_AUDIO_CHANNEL_DEFAULT;
	}
	_audioBitsPerSample = ini.GetInt(SECTION_NAME_AUDIO, KEY_AUDIO_BITSPERSAMPLE, MSDX_CONF_AUDIO_BITSPERSAMPLE_DEFAULT);
	if (_audioBitsPerSample != MSDX_CONF_AUDIO_BITSPERSAMPLE_DEFAULT) // 16bits only
	{
		_audioBitsPerSample = MSDX_CONF_AUDIO_BITSPERSAMPLE_DEFAULT;
	}

	// Video section
	loadVideoConfig();

    // load UI settings
    _mainScreen = ini.GetInt(SECTION_NAME_UI, KEY_UI_MAIN_SCREEN, 0);
    _enableControlPanelBooter = ini.GetBool(SECTION_NAME_UI, KEY_UI_ENABLE_CONTROL_PANEL_BOOTER, true);

    // load monitor settings
    _mainboardTempThreshold = 
      ini.GetDouble(SECTION_NAME_MONITOR, KEY_MONITOR_MAINBOARD_TEMP_THRES, 90.0);
    _mainboardTempAction =
      ini.GetInt(SECTION_NAME_MONITOR, KEY_MONITOR_MAINBOARD_TEMP_ACTION, 0);
    _cpuTempThreshold =
      ini.GetDouble(SECTION_NAME_MONITOR, KEY_MONITOR_CPU_TEMP_THRES, 90.0);
    _cpuTempAction =
      ini.GetInt(SECTION_NAME_MONITOR, KEY_MONITOR_CPU_TEMP_ACTION, 0);

    // load ptz settings
    ini.GetString(SECTION_NAME_PTZ, KEY_PTZ_SERIALPORT, buf, BUF_SIZE, "");
    _serialportName = QString::fromLocal8Bit(buf);
    _protocol = ini.GetInt(SECTION_NAME_PTZ, KEY_PTZ_PROTOCOL, 0);
    _cameraAddr = ini.GetInt(SECTION_NAME_PTZ, KEY_PTZ_CAMERA_ADDR, 0);
    _buadBits = ini.GetInt(SECTION_NAME_PTZ, KEY_PTZ_BUADBITS, 4800);
    _dataBits = ini.GetInt(SECTION_NAME_PTZ, KEY_PTZ_DATABITS, 7);
    _parity = ini.GetInt(SECTION_NAME_PTZ, KEY_PTZ_PARITY, 0);
    _stopBits = ini.GetInt(SECTION_NAME_PTZ, KEY_PTZ_STOPBITS, 1);
}

void RunningConfig::loadVideoConfig()
{
    const int BUF_SIZE = 256;
    char buf[BUF_SIZE] = {0};

    CIni ini;
    ini.SetPathName(_configFilePath.toLocal8Bit().data());

    _videoCapDeviceCount = ini.GetInt(SECTION_NAME_VIDEO, KEY_VIDEO_COUNT, MAX_CAMERA_COUNT);
    if (0 > _videoCapDeviceCount || CVIniConfig::getInstance().IsModelTX())
    {
        _videoCapDeviceCount = 0;
    }
    if (MAX_CAMERA_COUNT < _videoCapDeviceCount)
    {
        _videoCapDeviceCount = MAX_CAMERA_COUNT;
    }

    //��deviceManager���������Ƶ�����豸,�Ա���֮ǰ�������Ƿ���Ч
    VCapDeviceList setupVCapList;
    DeviceManager::GetInstatnce()->GetVideoCaptureDevices(&setupVCapList);

    _videoCapSettings.clear();

    for (int i = 0; i < _videoCapDeviceCount; ++i)
    {
        char sec[64] = {0};
        if (0 == i)
        {
            sprintf_s(sec, sizeof(sec), SECTION_NAME_VIDEO);
        }
        else
        {
            sprintf_s(sec, sizeof(sec), SECTION_NAME_VIDEO"%d", i);
        }

        VCapSetting vcap;

        ini.GetString(sec, KEY_VIDEO_CAP_PATH, buf, BUF_SIZE, "");//�������ļ���ȡ�ڼ���Video���ֶ�

        //����豸�Ƿ���Ч
        VCapDeviceList::iterator it = setupVCapList.begin();
        for (; it != setupVCapList.end(); ++it)
        {
			if (it->friendlyName=="Kinect Camera V2")//�ǳ�ɵ�Ƶش���Kinect���devicepath����� hjf 2015.12.22
			{
				if (it->displayName ==buf)
				{
					break;
				}
			}
            if (it->devicePath == buf)
            {
                break;
            }
        }

        if (it != setupVCapList.end())
        {
            vcap.devicePath = QString::fromLocal8Bit(buf);

            ini.GetString(sec, KEY_VIDEO_CAP_NAME, buf, BUF_SIZE, "");
            vcap.friendlyName = QString::fromLocal8Bit(buf);

            ini.GetString(sec, KEY_VIDEO_CROSSBAR, buf, BUF_SIZE, "");
            vcap.crossBarName = QString::fromLocal8Bit(buf);

            ini.GetString(sec, KEY_VIDEO_CROSSBAR_DISPLAYNAME, buf, BUF_SIZE, "");
            vcap.crossBarDisplayName = QString::fromLocal8Bit(buf);

            ini.GetString(sec, KEY_VIDEO_CROSSTYPE, buf, BUF_SIZE, "");
            vcap.crossBarType = QString::fromLocal8Bit(buf);

            vcap.width  = ini.GetInt(sec, KEY_VIDEO_WIDTH, 0);

            vcap.height = ini.GetInt(sec, KEY_VIDEO_HEIGHT, 0);

            vcap.fps    = ini.GetInt(sec, KEY_VIDEO_FPS, 0);

            ini.GetString(sec, KEY_VIDEO_CODEC, buf, BUF_SIZE, MSDX_CONF_VIDEO_CODEC_H264);
            vcap.videoCodec = QString::fromLocal8Bit(buf);

//            VerifyVideoSetting(vcap); //��������ʦҪ��,��Ҫ�Զ���������
        }

        vcap.index = i;
        _videoCapSettings.push_back(vcap);
    }
}

bool RunningConfig::IsConfiguredVideoDeviceValid()
{
    CIni ini;
    ini.SetPathName(_configFilePath.toLocal8Bit().data());
    const int BUF_SIZE = 256;
    char buf[BUF_SIZE] = {0};

    for (int i = 0; i < _videoCapDeviceCount; ++i) {
        char sec[64] = {0};
        if (0 == i) {
            sprintf_s(sec, sizeof(sec), SECTION_NAME_VIDEO);
        } else {
            sprintf_s(sec, sizeof(sec), SECTION_NAME_VIDEO"%d", i);
        }

        //�ӱ���������ж�ȡ��Ƶ�豸��Ϣ
        ini.GetString(sec, KEY_VIDEO_CAP_PATH, buf, BUF_SIZE, "");
        //���ҵ�ǰ��Ч���á�
        auto it = std::find_if(_videoCapSettings.begin(), _videoCapSettings.end(),
            [&buf](const VCapSetting &device){
                return device.devicePath == QString::fromLocal8Bit(buf);
        });
        //����Ҳ�����������loadConfig��Ϊ����Ч��˵���豸����û�о���
        if (it == _videoCapSettings.end()) {
            return false;
        }
    }
    return true;
}

void RunningConfig::VideoCaptureDevice(int index, const VCapSetting &val) 
{ 
	if (CVIniConfig::getInstance().IsModelTX())
	{
		return;
	}
	if (index >= _videoCapSettings.size())
	{
		return;
	}
	_videoCapSettings[index] = val;
	_videoCapSettings[index].videoCodec = "H264";

	//_videoCaptureDevice = val; 
	//if( val.trimmed().size()>0 )
	//	VideoCodec("H264" );
	//else
	//	VideoCodec( "" );

	//ManualConfig* mConfig = ManualConfig::getInstance();
	//string fileaddr = QCoreApplication::applicationDirPath().replace("/","\\").toStdString();
	//mConfig->ReadConfig( QCoreApplication::applicationDirPath().replace("/","\\").toStdString()+"\\cfg\\ManualConfig.txt");

	////��deviceManager���������Ƶ�����豸��
	//DeviceManager::GetInstatnce()->GetVideoCaptureDevices(&_videoCaptureDeviceList);

	//for (VCapDeviceList::iterator it = _videoCaptureDeviceList.begin(); it != _videoCaptureDeviceList.end(); it++)
	//{	
	//	QString friendlyname = QString::fromStdString(it->friendlyName);
	//	QString devicePath = QString::fromStdString(it->devicePath);

	//	//���ȳ���ȡ�ø��豸����ѷֱ���
	//	vector<string> mediaType;
	//	DeviceManager::GetInstatnce()->GetVideoDeviceMediaType(it->friendlyName, &mediaType);
	//	it->width = 0;
	//	it->height = 0;

	//	int i = 0;
	//	for (; i < mediaType.size(); ++i)
	//	{
	//		int width;
	//		int height;
	//		int fps;
	//		sscanf(mediaType.at(i).c_str(), "%dX%dX%d", &width, &height, &fps);
	//		if (width == _videoWidth && height == _videoHeight)
	//		{
	//			// �豸֧���趨�ķֱ���ʱ�������û�����
	//			it->width = width;
	//			it->height = height;
	//			it->fps = fps;
	//			break;
	//		}
	//		else if (width > it->width)
	//		{
	//			// ��������Ϊ���ֱ���
	//			it->width = width;
	//			it->height = height;
	//			it->fps = fps;
	//		}
	//	}
	//	if (i < mediaType.size())
	//	{
	//		// �豸֧���û����õķֱ���
	//		continue;
	//	}
	//	if (it->width == 0 || it->height == 0)
	//	{
	//		// �豸û�зֱ�����Ϣʱ����VGA�豸û������ʱ���������������
	//		// TODO:Ӧ��ͨ��VGA�豸��SDK��������DSHOW�еĹ̶��ֱ��� - Liaokz
	//		it->width = 1280;
	//		it->height = 720;
	//		it->fps = 30;
	//		continue;
	//	}
	//	
	//	//Ȼ����ݲ�ͬ�����豸����Ԥ���ֱ���

	//	//������豸��USB Webcam�����豸���豸·��������\\?\USB#��ͷ
	//	//��ǰ���õ�VGA�����豸Ҳ��USB�豸���ɼ��豸FriendlyName����VGA2USB�������޹̶��ֱ��ʣ�����Ĭ����ѷֱ���
	//	if (devicePath.indexOf("\\\\?\\usb#", Qt::CaseInsensitive) == 0 && friendlyname.left(7) != "VGA2USB")
	//	{
	//		//��֧�ֵķֱ��ʴ���720p������Ϊ720p�����������ѷֱ���
	//		if (it->width > 1280)
	//		{
	//			it->width = 1280;
	//			it->height = 720;
	//		}
	//	}
	//	//������豸��PCIe�ɼ��������豸���豸·��������\\?\PCI#��ͷ
	//	else if (devicePath.indexOf("\\\\?\\pci#", Qt::CaseInsensitive) == 0)
	//	{
	//		//---------------���÷ֱ���------------------------
	//		if (mConfig->getResolution() == "1080P" && it->width >= 1920)	//1080P
	//		{
	//			it->width = 1920;
	//			it->height = 1080;
	//		}
	//		else if (it->width >= 1280)
	//		{
	//			it->width = 1280;
	//			it->height = 720;
	//		}
	//		//�����豸ʹ��Ĭ����ѷֱ���

	//		//--------------����crossbar����-------------------
	//		string svideoStr("SVideo");
	//		string compositeStr("Composite");
	//		//��ʱͨ�������ļ�дcrossbar����
	//		if (friendlyname.left(12) == "Timeleak hd2")
	//		{
	//			it->crossBarType = (mConfig->getcrosstype2() == "S" ? svideoStr : compositeStr);
	//		}
	//		else if(friendlyname.left(12) == "Timeleak hd3")
	//		{
	//			it->crossBarType = (mConfig->getcrosstype3() == "S" ? svideoStr : compositeStr);
	//		}
	//		else
	//		{
	//			it->crossBarType = (mConfig->getcrosstype1() == "S" ? svideoStr : compositeStr);
	//		}
	//	}
	//}
	//����в�ֹһ������ͷ����ر�С��
	/*if (_videoCaptureDeviceList.size() > 1)
	{
		_enableSmallVideo = false;
	}*/
}

VCapSetting RunningConfig::VideoCaptureDevice(const int index)
{
	for (int i = 0; i < _videoCapSettings.size(); ++i)
	{
		if (_videoCapSettings[i].index == index)
		{
			return _videoCapSettings[i];
		}
	}
	return VCapSetting();
}

void RunningConfig::AudioSampleRate(int val)
{
	if (_audioCodec == MSDX_CONF_AUDIO_CODEC_SPEEX && (val != 16000 && val != 32000))
	{
		_audioSampleRate = MSDX_CONF_AUDIO_SAMPLERATE_DEFAULT;
		return;
	}
	_audioSampleRate = val;
}

void RunningConfig::AudioChannel(int val)
{
	if (_audioCodec == MSDX_CONF_AUDIO_CODEC_SPEEX)
	{
		_audioChannel = 1;
		return;
	}
	_audioChannel = val;
}

bool RunningConfig::isEnableSmallVideo() const
{
	if (CVIniConfig::getInstance().IsModelTX())
	{
		return false;
	}
	return _enableSmallVideo;
}

