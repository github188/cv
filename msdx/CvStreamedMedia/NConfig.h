#ifndef _NM_NCONFIG_H
#define _NM_NCONFIG_H


#include <string>
#include "NonCopyable.h"
using namespace std;

class CIni;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ���ù����࣬��ʵ��
////////////////////////////////////////////////////////////////////////////////////////////////////
class NConfig: public NonCopyable
{
public:
    static NConfig & getInstance();

    //���������ļ��������Ƿ���سɹ�
    bool loadConfig();

    //���������ļ������ƣ�Ĭ��������NetworkMeasurer.ini
    void setConfigFileName(const char * filename);

	bool isConfigLoaded() const;

    int getLocalSimulatePort() const;
    bool setLocalSimulatePort(int port);

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

	int getConfRoomQosServerUdpPort() const;
	bool setConfRoomQosServerUdpPort( const int port );

	int getCoolviewQosServerUdpPort() const;
	bool setCoolviewQosServerUdpPort( const int port );

	//�������ã���������ļ������ڣ��򱣴������ļ�
	bool saveConfig();

protected:
    NConfig(void);
    ~NConfig(void);

protected:
    CIni	* _ini;
	string _configFileName;
	bool _isConfigLoaded;		


	/************************************************************************/
	/* �����ֶ�                                                                     */
	/************************************************************************/

    int _localSimulatePort;		//���������������������tcp�˿�
    string _qosServerHostIP;		//qos����������IP
     string _qosServerSipUri;		//qos������SIP Uri
    int _qosServerTcpPort;		//qos������tcp�˿ڣ�������QoS�ͻ���ͨ��
    int _qosServerUdpPort;		//qos������udp�˿ڣ����ڽ��ղ���ͳ�Ʊ���
	
	string	_currentConferenceUri;	//��ǰ���ڽ��еĻ���URI
	int		_currentConferenceCid;	//��ǰ���ڽ��еĻ���ID
	int		_confRoomQosServerUdpPort;	//������UI����Qos����Ķ˿�
	int		_coolviewQosServerUdpPort;	//������UI����Qos����Ķ˿�
};

inline bool  NConfig::isConfigLoaded() const 
{ 
	return _isConfigLoaded; 
}

inline int NConfig::getLocalSimulatePort() const
{
	return _localSimulatePort;
}

inline const char * NConfig::getQosServerHostIP() const
{
	return _qosServerHostIP.c_str();
}

inline const char * NConfig::getQosServerSipUri() const
{
	return _qosServerSipUri.c_str();
}


inline int NConfig::getQosServerUdpPort() const
{
	return _qosServerUdpPort;
}

inline int NConfig::getQosServerTcpPort() const
{
	return _qosServerTcpPort;
}

inline const char* NConfig::getCurrentConferenceUri() const
{
	return _qosServerSipUri.c_str();
}

inline int NConfig::getCurrentConferenceCid() const
{
	return _currentConferenceCid;
}

inline int NConfig::getConfRoomQosServerUdpPort() const 
{
	return _confRoomQosServerUdpPort;
}

inline int NConfig::getCoolviewQosServerUdpPort() const 
{
	return _coolviewQosServerUdpPort;
}

#endif
