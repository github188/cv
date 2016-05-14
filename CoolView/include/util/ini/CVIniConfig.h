#ifndef _NM_NCONFIG_H
#define _NM_NCONFIG_H

#include <string>
using namespace std;

#define CVCFG_VALUE_MODEL_CATEGORY_HD      0 
#define CVCFG_VALUE_MODEL_CATEGORY_TX      1 

class CIni;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ini���ù����࣬��ʵ��
////////////////////////////////////////////////////////////////////////////////////////////////////
class CVIniConfig
{
public:
    static CVIniConfig & getInstance();

    //���������ļ��������Ƿ���سɹ�
    bool loadConfig();

    //���������ļ������ƣ�Ĭ��������NetworkMeasurer.ini
    void setConfigFileName(const char * filename);

	bool isConfigLoaded() const;

    const char * getQosServerHostIP() const;
    bool setQosServerHostIP(const char * ip);

	const char * getQosServerSipUri() const;
	bool setQosServerSipUri(const char * uri);

    int getQosServerTcpPort() const;
    bool setQosServerTcpPort(int port);

    int getQosServerUdpPort() const;
    bool setQosServerUdpPort(int port);

	const char* getCurrentConferenceUri() const;
	bool setCurrentConferenceUri( const char* uri );

	int getCurrentConferenceCid() const;
	bool setCurrentConferenceCid( const int Cid );

    // �Ƿ�ǿ��ʹ���Զ���DSCP����QoS����������ģ�
    bool isUseCustomedDscp() const { return _useCustomedDscp; }
    void setUseCustomedDscp(bool val);

    int getVideoDscp() const { return _videoDscp; }
    void setVideoDscp(int dscp);

    int getAudioDscp() const { return _audioDscp; }
    void setAudioDscp(int dscp);


	//�������ã���������ļ������ڣ��򱣴������ļ�
	bool saveConfig();


	//��ȡCoolview����udp���ݵĶ˿�
	int getCoolviewQosServerUdpPort() const;
  bool setCoolviewQosServerUdpPort( const int port );

  int getConfRoomQosServerUdpPort() const;
  bool setConfRoomQosServerUdpPort( const int port );

	int getModelCategory() const;
	bool setModelCategory(int cate);

    bool IsModelTX() const;
    bool IsModelHD() const;

    bool useMultiThread() const { return _useMultiThread; }
    void setMultiThread(bool use_multi_thread);

    bool isVirtualConferenceEnable() const { return _enableVirtualConference; }
    void setVirtualConferenceEnable(bool val);

    bool useTransparentSubtitle() const { return _useTransparentSubtitle; }
    void setTransparentSubtitle(bool val);

    bool isAutoSetPrimaryScreen() const { return _isAutoSetPrimaryScreen; }
    void setAutoSetPrimaryScreen(bool val);

protected:
    CVIniConfig(void);
    ~CVIniConfig(void);

	//������������ֻ���岻ʵ�֣���ֹ�����࿽��
	CVIniConfig(const CVIniConfig&);
	CVIniConfig& operator=(const CVIniConfig&);

protected:
  CIni	* _ini;
	string _configFileName;
	bool _isConfigLoaded;		

	/************************************************************************/
	/* �����ֶ�                                                                     */
	/************************************************************************/
  string _qosServerHostIP;		//qos����������IP
  string _qosServerSipUri;		//qos������SIP Uri
  int _qosServerTcpPort;		//qos������tcp�˿ڣ�������QoS�ͻ���ͨ��
  int _qosServerUdpPort;		//qos������udp�˿ڣ����ڽ��ղ���ͳ�Ʊ���
    
	string	_currentConferenceUri;	//��ǰ���ڽ��еĻ���URI
	int		_currentConferenceCid;	//��ǰ���ڽ��еĻ���ID
	bool _useCustomedDscp;     //�Ƿ�ǿ��ʹ���Զ���DSCP����QoS����������ģ�
  int  _videoDscp;
  int _audioDscp;  

  int _coolviewQosServerUdpPort;	//Coolview����Qos����Ķ˿�
  int		_confRoomQosServerUdpPort;	//������UI����Qos����Ķ˿�

	int _modelCategory;  //�ͺ����
  bool _useMultiThread;
  bool _enableVirtualConference; //�Ƿ��������������

  //TODO:ConfRoom������Ƶ����������ļ��У�
  bool _useTransparentSubtitle;

  bool _isAutoSetPrimaryScreen;
};

inline bool  CVIniConfig::isConfigLoaded() const 
{ 
	return _isConfigLoaded; 
}


inline const char * CVIniConfig::getQosServerHostIP() const
{
	return _qosServerHostIP.c_str();
}

inline const char * CVIniConfig::getQosServerSipUri() const
{
	return _qosServerSipUri.c_str();
}


inline int CVIniConfig::getQosServerUdpPort() const
{
	return _qosServerUdpPort;
}

inline int CVIniConfig::getQosServerTcpPort() const
{
	return _qosServerTcpPort;
}

inline const char* CVIniConfig::getCurrentConferenceUri() const
{
	return _qosServerSipUri.c_str();
}

inline int CVIniConfig::getCurrentConferenceCid() const
{
	return _currentConferenceCid;
}

inline int CVIniConfig::getCoolviewQosServerUdpPort() const 
{
	return _coolviewQosServerUdpPort;
}

inline int CVIniConfig::getConfRoomQosServerUdpPort() const 
{
  return _confRoomQosServerUdpPort;
}

inline int CVIniConfig::getModelCategory() const
{
	return _modelCategory;
}

inline bool CVIniConfig::IsModelTX() const {
  return _modelCategory == CVCFG_VALUE_MODEL_CATEGORY_TX;
}

inline bool CVIniConfig::IsModelHD() const {
  return _modelCategory == CVCFG_VALUE_MODEL_CATEGORY_HD;
}

#endif
