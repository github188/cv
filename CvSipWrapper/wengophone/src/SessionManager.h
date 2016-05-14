/************************************************************************/
/* @author zhenHua.sun
 * @date 2010-09-01
 * @brief �����������н����ĻỰ��������Ƶ���շ�
 */
/************************************************************************/
#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H
#include <string>
using namespace std;
class MediaSession
{
public:
	string			_userId;
	int				_graphId;
	char			_ipAddr[48];
	unsigned short	_audioPort;
	unsigned short	_videoPort;
	HWND			_videoWnd;
};

class ConferenceSession
{
public:
	string			_focusUI;
	string			_password;
	int				_callID;
};

typedef std::vector<MediaSession*>  MediaRecvSessions;

class SessionManager
{
public:
	SessionManager()
	{
		_mediaRecvSessions.clear();
		_confSession = NULL;
	}
	~SessionManager()
	{
		_mediaRecvSessions.clear();
		if( _confSession )
		{
			delete _confSession;
			_confSession = NULL;
		}
	}

	MediaRecvSessions _mediaRecvSessions;

public:
	/**
	 * @brief ���ڲ����������һ��ý����ջỰ
	 */
	void addRecvSession( std::string userId, const int graphId, char * AudioIP,int AudioPort,const char* audioCodecMIME , const int audioSampleRate , const int audioBps , const int audioChannels,char * VideoIP,int VideoPort,char *codecMIME,HWND remoteHwnd,int videoWidth,int videoHeight );
	/**
	 * @brief ���ڲ�������ɾ��һ��ý����ջỰ
	 */
	void removeRecvSession( std::string userId);
	/**
	 * @brief ���ý����ջỰ
	 */
	void clearRecvSessions();
	/**
	 * @brief ��ȡָ��IP��ȡ��Ӧ��session
	 * @return ���û�ҵ��򷵻ؿ�ָ�룬���򷵻�
	 */
	MediaSession* getRecvSession( const int& graphid );

	/**
	 * @brief �������Ự
	 */
	void addConfSession( const std::string& focus_uri , const int& callID, const std::string& password = "" );

	/**
	 * @brief ���ٻ���Ự
	 */
	void removeConfSession( );

	/**
	 * @brief ��ȡ����Ự
	 * @return ���û�н����κλ��齫����NULL
	 */
	ConferenceSession* getConfSession();

private:
	
	//�����û�����Ļ�����Ϣ
	ConferenceSession*		_confSession;
};

#endif