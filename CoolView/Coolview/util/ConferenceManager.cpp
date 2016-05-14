#include "ConferenceManager.h"

#include <WinSock2.h>

#include <QtCore/QVariantMap>
#include <QApplication>

#include <util/ProcessManager.h>

#include "MediaManager.h"
#include <dbus/channel/type/streamedMedia/common/StreamedMediaServiceCommon.h>
#include <dbus/telecontroller/client/TelecontrollerIf.h>
#include <dbus/telecontroller/common/TelecontrollerServiceCommon.h>
#include <dbus/channel/type/screenMedia/common/ScreenMediaServiceCommon.h>
#include <dbus/sipwrapper/client/CvSipWrapperIf.h>
#include <dbus/sipwrapper/common/SipWrapperCommonService.h>

#include <dbus/txMonitor/client/TxMonitorIf.h>
#include <dbus/txMonitor/common/TxMonitorServiceCommon.h>

#include "../profile/RunningProfile.h"
#include "../config/RunningConfig.h"

#include <CoolviewCommon.h>
#include <log/Log.h>
#include <msdx/config.h>

#include "STcpSender.h"
#include "util\ini\CVIniConfig.h"
#include "QoSDef.h"
#include "NetworkUtil.h"
#include "HttpWrapper.h"

#include "util/ini/TxConfig.h"
#include "util/report/RecordStat.h"

char __globalMessege[512];

#define DEFAULT_IP_ADDRESS  "0.0.0.0"        //Ĭ�ϵ�һ����ַ��������ʾ���������IP��ַ������graph�ã�ʵ�ʲ���������
//#define DEFAULT_AUDIO_PORT 10000     //Ĭ�ϵ���Ƶ�˿�
//#define DEFAULT_VIDEO_PORT  10002        //Ĭ�ϵ���Ƶ�˿�
#define LOCALPRE_VIDEO_PORT_BASE 8000
#define LOCALPRE_SMALL_PORT_BASE 8100
#define LOCALREC_AUDIO_PORT_BASE 9000
#define LOCALREC_VIDEO_PORT_BASE 9100

#define MEDIA_CONTROL_ENABLE    //��������˸ú꣬��ý�����Ŀ�����Ҫ�ͻ�����Ʒ���������Э��

#define PPT_TERMINAL_URI	"test_t1@sip.ccnl.scut.edu.cn"
#define MULTICAST_URI       "multicast@sip.ccnl.scut.edu.cn"
#define MULTICAST_SMALL_URI "multicasts@sip.ccnl.scut.edu.cn"


ConfInfo* conf_find_by_uri(const ConferenceVector& confList, const string& uri)
{
    for(ConferenceVector::const_iterator iter = confList.begin(); iter != confList.end(); ++iter)
    {
        if(uri == (*iter)->URI)
            return (*iter);
    }
    return  NULL;
}

//����uri�ڳ�Ա�б����ָ���Ļ�����Ϣ��������ָ�룬�Ҳ����򷵻�NULL
MemberInfo* member_find_by_uri(const vector<MemberInfo*>& memberList, const string& uri)
{
    for(vector<MemberInfo*>::const_iterator iter = memberList.begin(); iter != memberList.end(); ++iter)
    {
        if(uri == (*iter)->MemberURI)
            return (*iter);
    }
    return  NULL;
}

//ɾ���б��а���ָ��ip��Ԫ��
void receiver_remove_by_ip( vector<ReceiverInfo>& receiverList, const string& ip)
{
    vector<ReceiverInfo>::iterator iter;
    for( iter = receiverList.begin(); iter != receiverList.end(); iter++ )
    {
        if(ip == (*iter).recvIP)
        {
            //ɾ��Ԫ�أ�ָ����һ��������
            iter = receiverList.erase(iter);
            break;
        }
    }
}

void getMediaParamFromSdp( int* videoWidth , int* videoHeight , int* videoFps, char* audioCodec , const char* sdp )
{
    //char audioPort[10];
    //char videoPort[10];
    //char width_str[10];
    //char height_str[10];

    //char* temp, * temp2;
    //if(!sdp || strcmp("", sdp) == 0)
    //    return;

    //temp = strstr((char*)sdp, "m=audio");
    //if( temp == NULL )
    //{
    //    strcpy( audioCodec , "" );
    //}
    //else
    //{
    //    strcpy( audioCodec , "SPEEX" );
    //}

    //temp = strstr((char*)sdp, "m=video");
    //if( temp == NULL )
    //{
    //    //��������Ƶ�豸
    //    *videoWidth = 0;
    //    *videoHeight = 0;
    //}
    //else
    //{
    //    temp = strstr(temp, "CUSTOM");
    //    if(temp)
    //    {
    //        while(*temp != '\r' && *temp != '\n' && *temp != '\0' && *temp != '=')
    //        {
    //            temp++;
    //        }
    //        temp++;
    //        temp2 = temp;
    //        while(*temp2 != '\r' && *temp2 != '\n' && *temp2 != '\0' && *temp2 != ',')
    //        {
    //            temp2++;
    //        }
    //        temp2++;
    //        strncpy(width_str, temp, temp2 - 1 - temp);

    //        temp = temp2;
    //        while(*temp2 != '\r' && *temp2 != '\n' && *temp2 != '\0' && *temp2 != ' ' && *temp2 != ',')
    //        {
    //            temp2++;
    //        }
    //        strncpy(height_str, temp, temp2 - temp);
    //        *videoWidth = atoi(width_str);
    //        *videoHeight = atoi(height_str);
    //    }
    //}

    QString sdpMsg = QString::fromUtf8( sdp );
    if( sdpMsg.contains("m=video") )
    {
        int index1 = sdpMsg.indexOf("CUSTOM=");
        int index2 = sdpMsg.indexOf(" MaxBR");
        QString temp1 = sdpMsg.remove(0, index1 + 7);
        temp1 = temp1.left(index2 - index1 - 7);
        int index3 = temp1.indexOf(",");
        QString swidth = temp1.left(index3);
        QString sheight = temp1.remove(0, index3 + 1);

        *videoWidth = swidth.toInt();
        *videoHeight = sheight.toInt();

        index1 = sdpMsg.indexOf("FPS=");
        if( index1 > 0 )
        {
            temp1 = sdpMsg.remove( 0 , index1 + 4 );
            index2 = sdpMsg.indexOf( " " );
            QString sFps = temp1.left( index2 );
            *videoFps = sFps.toInt();
        }
        else
        {
            *videoFps = 30;
        }
    }
    else
    {
        *videoHeight = 0;
        *videoWidth = 0;
        *videoFps = 0;
    }

    strcpy(audioCodec , "SPEEX");

}

TerminalInfo* terminal_find_by_port( TerminalMap& terminalList , const int& port, int mediaType )
{
	if (mediaType!=0 && mediaType!=1)
		return NULL;
    for( TerminalMap::const_iterator it = terminalList.begin() ; it != terminalList.end() ; ++it )
    {
		for (int i = 0; i < it->second.size(); ++i)
		{
			if (mediaType == 0 && (it->second[i])->videoPort == port)
				return (it->second[i]);
			else if (mediaType == 1 && (it->second[i])->audioPort == port)
				return (it->second[i]);
		}
    }
    return NULL;
}

TerminalInfo* FindTerminal( TerminalMap& terminalMap , string uri )
{
	// �������virtualUri�������ʵuri
	string virtualUri = uri;
	if (uri.find("#") != -1)
	{
		uri = uri.substr(uri.find("#") + 1);
	}
	if (terminalMap.find(uri) == terminalMap.end())
	{
		return NULL;
	}
	// ������������ʵuri
	if (uri == virtualUri)
	{
		return terminalMap[uri][0];
	}
	// ���򷵻ض�Ӧ�������ն�
    for( TerminalVector::const_iterator it = terminalMap[uri].begin() ; it != terminalMap[uri].end() ; ++it )
    {
        if( virtualUri == (*it)->virtualUri )
        {
            return (*it);
        }
    }
    return NULL;
}

/*
 * ����һ���Ե�uri�ַ�������������ʵuri������uri���������ն˺�
 */
void ParseVirtualUriStr(const string inputUri, string &uri, string &virtualUri, int &virtualIndex)
{
	int nSharpPos = inputUri.find("#");
	if (std::string::npos != nSharpPos)
	{
		// �ҵ�#����
		virtualUri = inputUri;
		uri = inputUri.substr(nSharpPos + 1);
		virtualIndex = atoi(inputUri.substr(1, nSharpPos - 1).c_str()) - 1;
	}
	else
	{
		uri = inputUri;
		virtualUri = "v1#";
		virtualUri += uri;
		virtualIndex = 0;
	}
}


void TransformConferenceForQt( const ConfInfo& conf , QVariantMap* confMap )
{
    confMap->insert("name" , QString::fromUtf8(conf.Title.c_str()));
    confMap->insert("handle-name" , QString::fromStdString(conf.URI));
    confMap->insert("conference-id" , QString::fromStdString( conf.CID ) );
    confMap->insert("start-time" , QString::fromStdString(conf.StartTime));
    confMap->insert("control-mode" , QString::fromStdString(conf.ControlMode));
    confMap->insert("join-mode" , QString::fromStdString(conf.JoinMode) );
    confMap->insert("duration" , QString::fromStdString(conf.Duration));
    confMap->insert("level" , QString::fromStdString(conf.level));
	confMap->insert("chairman" , QString::fromStdString(conf.HostURI));
	confMap->insert("multicastAddress",QString::fromStdString(conf.multicastAddress));
}

void TransformConfListForQt( const ConferenceVector& confList , QList<QVariantMap>* confMapList )
{
    for( ConferenceVector::const_iterator it = confList.begin(); it != confList.end(); it++ )
    {
        QVariantMap conf;
        TransformConferenceForQt( *(*it) , &conf );
        confMapList->push_back( conf );
    }
}

void TransformMemberForQt( const MemberInfo& member , QVariantMap* memberMap )
{
    memberMap->insert("uri", QString::fromUtf8(member.MemberURI.c_str()));
    memberMap->insert("sdp", QString::fromUtf8(member.sdp.c_str()));
    memberMap->insert("floor" , QString::fromUtf8(member.floor.c_str()));
    memberMap->insert("multicastAddress", QString::fromUtf8(member.multicastAddress.c_str()));
    memberMap->insert("videoPort", member.videoPort);
    memberMap->insert("audioPort", member.audioPort);
    memberMap->insert("name" , QString::fromUtf8(member.name.c_str()));
    memberMap->insert("username" , QString::fromUtf8(member.username.c_str()));
    memberMap->insert("location" , QString::fromUtf8(member.location.c_str() ) );
    memberMap->insert("AllowTerminal" , QString::fromUtf8(member.AllowTerminal.c_str()) );
    memberMap->insert("company" , QString::fromUtf8(member.company.c_str()) );
    memberMap->insert("xmpp" , QString::fromUtf8(member.xmpp.c_str()));
    memberMap->insert("status" , QString::fromUtf8(member.status.c_str()) );
}

void TransformMemberListForQt( const MemberVector& memberList , QList<QVariantMap>* memberMapList )
{
    for( MemberVector::const_iterator it = memberList.begin() ; it != memberList.end() ; it++ )
    {
        QVariantMap member;
        TransformMemberForQt( *(*it) , &member );
        memberMapList->push_back( member );
    }
}

void TransformTerminalForQt( const TerminalInfo&terminal , QVariantMap* terminalMap )
{
	terminalMap->insert("uri", QString::fromUtf8(terminal.uri.c_str()));
    terminalMap->insert("virtualIndex", terminal.virtualIndex);
	terminalMap->insert("virtualUri", QString::fromUtf8(terminal.virtualUri.c_str()));
    terminalMap->insert("virtualCount", terminal.virtualCount);
    terminalMap->insert("name" , QString::fromUtf8(terminal.name.c_str()));
    terminalMap->insert("location" , QString::fromUtf8( terminal.location.c_str() ) );
    terminalMap->insert("status" , QString::fromUtf8(terminal.status.c_str()) );
    terminalMap->insert("sdp" , QString::fromUtf8(terminal.sdp.c_str()));
    terminalMap->insert("videoPort" , QString::number(terminal.videoPort ));
    terminalMap->insert("audioPort" , QString::number(terminal.audioPort) );
    terminalMap->insert("multicastAddress" , QString::fromUtf8(terminal.multicastAddress.c_str()));
	terminalMap->insert("isHandUp",QString :: fromUtf8(terminal.isHandUp.c_str()));
	terminalMap->insert("isSpeaking",QString::fromUtf8(terminal.isSpeaking.c_str()));
	terminalMap->insert("isMainSpeaker",QString::fromUtf8(terminal.isMainSpeaker.c_str()));
	terminalMap->insert("isChairmanTerminal",QString::fromUtf8(terminal.isChairmanTerminal.c_str()));

}

void TransformTerminalListForQt( const TerminalVector& terminalList , QList<QVariantMap>* terminalMapList )
{
    for( TerminalVector::const_iterator it = terminalList.begin() ; it != terminalList.end() ; it++ )
    {
        QVariantMap terminalMap;
        TransformTerminalForQt( *(*it) , &terminalMap );
        terminalMapList->push_back( terminalMap );
    }
}

//����һ����Ϣ��NetworkMeasurer����
int sendMessageToNetworkMeasurer(const char * message, int length )
{
    if(message == NULL || length <= 0)
        return -1;
    STcpSender tcpsender;
    bool connected = tcpsender.connect("127.0.0.1", 5150);
    if (!connected)
        return -1;
    unsigned	char* data = new unsigned char[length+4];
    //�ڴ濽��
    int nlen = htonl(length);
    memcpy(data, &nlen, sizeof(nlen));
    memcpy(&data[4], message, length);
    int ret =	tcpsender.send(data, length + 4);
    delete data;
    tcpsender.close();
    return ret;
}


ConferenceManager::ConferenceManager( )
{
    _sipWrapperProxy = new CvSipWrapperIf( SIP_WRAPPER_SERVICE_NAME , SIP_WRAPPER_OBJECT_PATH ,
                                           QDBusConnection::sessionBus() );
    connect( _sipWrapperProxy , SIGNAL(cvMessageReceivedEvent(const QString &, const QString &)) ,
		this , SLOT(cvMessageReceivedSlot(const QString& , const QString&)));

	_httpWrapper = new HttpWrapper();
	connect( _httpWrapper , SIGNAL(RecvTerminalListSignal(TerminalVector)), this , SLOT(RecvTerminalListSlot(TerminalVector)));

	_txMonitor = nullptr; /*new TxMinitorIf(TX_MONITOR_SERVICE_NAME, TX_MONITOR_SERVICE_OBJECT_PATH, 
		QDBusConnection::sessionBus() );*/

    _confState = NotInConference;
    _currentConfInfo = NULL;
	_totalTermianlNum =0;

    //�������ļ�������Ϣ
    const char *strqosuri = CVIniConfig::getInstance().getQosServerSipUri();
    if (strqosuri != NULL && strlen(strqosuri) > 0)
    {
        _qosServerUri = strqosuri;
    }
    else
    {
        //����һ�����ö�ȡ
        _qosServerUri = RunningConfig::Instance()->getQosServerURI().toStdString();
    }

    _onlineInfoTimer = new QTimer(this);
	_serverHeartbeatTimer = new QTimer(this);
	_serverTestTimer = new QTimer(this);

	_serverTestCount.start();

	_isServerTest=false;

    connect( _onlineInfoTimer , SIGNAL(timeout()) , this , SLOT(onlineInfoSlot()));
	connect( _serverTestTimer , SIGNAL(timeout()) , this , SLOT(severTestTimeOutSlot()));

}


ConferenceManager::~ConferenceManager()
{
    SAFE_DELETE( _onlineInfoTimer);
	SAFE_DELETE( _serverHeartbeatTimer);
	SAFE_DELETE( _serverTestTimer);

    CVMsgParser cvMessageParser;
    cvMessageParser.ReleaseConfList(_confList);

}

int ConferenceManager::makeJoinConfInvite( const std::string& focus_URI )
{
    switch(_confState)
    {
    case IsInConference:
    {
        //�û��Ѿ���һ��������
        return kInAConference;
    }
    break;
    case ExitingConference:
    {
        //�û������˳�����
        return kIsLeavingConference;
    }
    break;
    case NotInConference:
    case WaitingPermission:
        //����ԭ����׼��ȴ�����������
    case JoinRejected:
        //�����������һ���µĻ���
    {
        //��ʾ�����Ҵ���
        startConferenceRoom();


        //����uri���һ�����Ϣ
        ConfInfo* conf = conf_find_by_uri(_confList, focus_URI);
        if(!conf)
            return kConferenceNotExisted;
        if("password" == conf->JoinMode)
            return kNeedPassword;
        //���µ�ǰ������Ϣ
        _focusURI = focus_URI;
        _confTitle = conf->Title;
        _currentConfInfo = conf;
        _hostURI = conf->HostURI;
        _confMode = conf->ControlMode;
		_speakerURI = ""; 
		_TerminalChairmanURI="";
		_currentScreenShareUri="";
		_totalTermianlNum=0;
        CVMsgParser::ReleaseMemberList( _memberList );

		for (TerminalMap::iterator it = _terminalList.begin(); it != _terminalList.end(); ++it)
		{
			CVMsgParser::ReleaseTerminalList( it->second );
		}
		_terminalList.clear();


        SDPInfo info;
		//TODO:�Ѳ����ʺ϶�·�ɼ�����ʱ��ȡһ·��Ч����
		info._videoHeight = 0;
		info._videoWidth = 0;
		info._videoFps = 0;
		for (int i = 0; i < RunningConfig::Instance()->VideoCaptureDeviceCount(); ++i)
		{
			VCapSetting setting = RunningConfig::Instance()->VideoCaptureDevice(i);
			if (setting.devicePath == "")
			{
				continue;
			}
			info._videoCodec = setting.videoCodec;
			info._videoHeight = setting.height;
			info._videoWidth = setting.width;
			info._videoFps = setting.fps;
			break;
		}
        info._audioCodec = RunningConfig::Instance()->AudioCodec();
        QByteArray output_bytes;
        QDataStream out(&output_bytes , QIODevice::WriteOnly );
        out.setVersion( QDataStream::Qt_4_4 );
        out << info;
        std::string username = RunningProfile::getInstance()->username();
        RunningProfile::getInstance()->set_current_conference_uri(focus_URI);
        std::string szContent = createJoinConferenceMsg(focus_URI);
        _sipWrapperProxy->makeJoinConfInvite( QString::fromStdString(username) , QString::fromStdString(focus_URI), QString::fromStdString(szContent), output_bytes);
		
        //�ı����״̬Ϊ������֤��
        _confState = WaitingPermission;


        return kOk;
    }
    break;
    default:
        break;
    }
    return kUnknown;
}



void ConferenceManager::cvMessageReceivedSlot( const QString &message, const QString &from )
{
    CVMsgParser cvMessageParser;
	cvMessageParser.InitParser( message.toStdString().c_str() );		//For WengoPhone SipWrapper
    
    const char* command = cvMessageParser.GetCommandType();
	//��ȡ�ն�����ģʽ Patrick
	if (strcmp(command,"TerminalConfMode") == 0)
	{
		recvTerminalConfModeMsgHandler(cvMessageParser, from.toStdString());
	}
    //��û����б�
    if(strcmp(command, "ConfList") == 0)
    {
        recvConfListMsgHandler(cvMessageParser, from.toStdString());
    }
    else if( strcmp(command , "TerminalList") == 0 )
    {
        //��ȡ�ն��б�
        recvTerminalListCommandMsgHandler( cvMessageParser , from.toStdString() ); //TODO:HTTP
    }
    //�������
    else if(strcmp(command, "LoginResult") == 0)
    {
        recvLoginResultCommandMsgHandler(cvMessageParser, from.toStdString());
    }
    //�յ�MemberList��Info��Ϣ
    else if(strcmp(command, "MemberList") == 0)
    {
        recvMemberListCommandMsgHandler(cvMessageParser, from.toStdString());
    }
    //���˼������
    else if(strcmp(command, "LoginNotify") == 0 || strcmp(command, "loginnotify") == 0)
    {
        recvLoginNotifyMsgHandler(cvMessageParser, from.toStdString());
    }
    //�����˳�����
    else if(strcmp(command, "LogoutNotify") == 0 || strcmp(command, "logoutnotify") == 0)
    {
        recvLogoutNotifyMsgHandler(cvMessageParser, from.toStdString());
    }
    //�ܵ�����
    else if(strcmp(command, "Invite") == 0)
    {
        recvInviteMsgHandler(cvMessageParser, from.toStdString());
    }
    //���뼴ʱ����ɹ�
    else if(strcmp(command, "ApplyConfResult") == 0)
    {
        recvApplyConfResultMsgHandler(cvMessageParser, from.toStdString());
    }
    //���벻��ȷ
    else if(strcmp(command, "PasswordIncorrect") == 0)
    {
        recvPasswordIncorrectMsgHandler(cvMessageParser, from.toStdString());
    }
    //���߳�����
    else if(strcmp(command, "Kicked") == 0)
    {
        recvKickedMsgHandler(cvMessageParser, from.toStdString());
    }
    //�յ��ı���Ϣ
    else if(strcmp(command, "Text") == 0)
    {
        recvTextMsgHandler(cvMessageParser, from.toStdString());
    }
    //�յ��װ���Ϣ
    else if(strcmp(command, "WhiteBoard") == 0)
    {
        recvWhiteBoardMsgHandler(cvMessageParser, from.toStdString());
    }
    //���ַ���
    else if(strcmp(command, "HandUp") == 0)
    {
        recvHandUpMsgHandler(cvMessageParser, from.toStdString());
    }
    //ȡ������
    else if(strcmp(command, "HandDown") == 0)
    {
        recvHandDownMsgHandler(cvMessageParser, from.toStdString());
    }
#ifndef MEDIA_CONTROL_ENABLE
    //����һ·����ý����
    else if(strcmp(command, "Receiver") == 0)
    {
        recvReceiverMsgHandler(cvMessageParser, from.toStdString());
    }
    //��������һ·����ý����
    else if(strcmp(command, "Cancel2Receiver") == 0)
    {
        recvCancel2ReceiverMsgHandler(cvMessageParser, from.toStdString());
    }
#else
    //start---------������ý������������----------zhhua.sun
    else if( strcmp( command , "StartMediaReply") == 0 )
    {
        //focus ���صĴ���ý�����Ļظ�
        recvStartMediaReply( cvMessageParser , from.toStdString() );

    }
    else if( strcmp( command , "StartMediaRelay") == 0 )
    {
        ///focus ת�����շ���ý������Ҫ��
        recvStartSendMediaCommand( cvMessageParser , from.toStdString() );

    }
    else if( strcmp(command , "StopMediaReply") == 0 )
    {
        //focus����ֹͣý�����Ļظ�
        recvStopMediaReply( cvMessageParser , from.toStdString() );
    }
    else if( strcmp( command , "StopMediaRelay") == 0 )
    {
        //focusת�����շ�ֹͣý������Ҫ��
        recvStopSendMediaCommand( cvMessageParser , from.toStdString() );
    }
    else if( strcmp( command , "SendMediaControl") == 0 )
    {
        recvSendMediaControlCommand( cvMessageParser , from.toStdString() );
    }
    //end
#endif

    //����һ·��Ļ������
    else if(strcmp(command, "ScreenReceiver") == 0)
    {
        recvScreenReceiverMsgHandler(cvMessageParser, from.toStdString());
    }
    //��������һ·��Ļ������
    else if(strcmp(command, "ScreenCancel2Receiver") == 0)
    {
        recvScreenCancel2ReceiverMsgHandler(cvMessageParser, from.toStdString());
    }
    else if(strcmp(command, "UnShareScreen") == 0)
    {
        recvUnShareScreenMsgHandler(cvMessageParser, from.toStdString());
    }
    else if( strcmp( command , "EndConference") == 0 )
    {
        cvEndConferenceSignal();
    }
    //qos�������
    else if(strcmp(command, "qos") == 0)
    {
        recvQoSMsgHandler(cvMessageParser, from.toStdString());
    }
	//�����˿���
	else if( strcmp( command, "SetSpeakerTerminal")==0 )
	{
		recvPermitSpeakInfo( cvMessageParser , from.toStdString() );
	}
	else if( strcmp( command, "ForbidSpeaker")==0 )
	{
		recvForbidSpeakInfo( cvMessageParser , from.toStdString() );
	}
	else if( strcmp( command, "TerminalLoginNotify")==0 )
	{
		recvSaviInfo( cvMessageParser , from.toStdString() );
	}
	else if( strcmp( command, "SetChairmanTerminal")==0 )
	{
		recvChairmanTerminalHandler( cvMessageParser , from.toStdString() );
	}
	else if( strcmp( command, "MediaControl")==0 )
	{
		recvMediaControlInfo( cvMessageParser , from.toStdString() );
	}
	else if( strcmp( command, "GetSpeakerTerminal")==0 )
	{
		recvMainSpeakerTermianlInfo( cvMessageParser , from.toStdString() );
	}
	else if( strcmp( command, "GetChairmanTerminal")==0 )
	{
		recvChairmanTermianlInfo( cvMessageParser , from.toStdString() );
	}
	else if( strcmp( command, "GetScreenShareTerminal")==0 )
	{
		recvCurrentScreenShareTerminalInfo( cvMessageParser , from.toStdString() );
	}
	else if( strcmp( command, "ShareScreenControl")==0 )
	{
		recvShareScreenControlInfo( cvMessageParser , from.toStdString() );
	}
	else if( strcmp( command, "Online")==0 )
	{
		recvOnlineMessageInfo( cvMessageParser , from.toStdString() );
	}

}

void ConferenceManager::recvTerminalConfModeMsgHandler(CVMsgParser& cvMessageParser, const std::string& fromUri)
{
	string confMode = cvMessageParser.GetTerminalConfMode();
	if (confMode == "720PWEBCAM*1")
	{
		//implementation
	}
	else if (confMode == "1080PHD*1")
	{
		//implementation
	}
	else if (confMode == "1080PHD*3")
	{
		//implementation
	}
	else if (confMode == "1080PHD*3+VGA")
	{
		//implementation
	}

}
void ConferenceManager::recvConfListMsgHandler( CVMsgParser& cvMessageParser, const std::string& from )
{
   //cvMessageParser.ParseConfListMsg(_confList);

    ConferenceVector tmpConfVector;
    cvMessageParser.ParseConfListMsg( tmpConfVector );

    cvConfListReceivedSignal( tmpConfVector );

    for( int i = 0 ; i < tmpConfVector.size() ; i++ )
    {
        _confList.push_back( tmpConfVector.at(i) );
    }

    //_currentConfInfo = conf_find_by_uri(_confList, _focusURI);

}

void ConferenceManager::recvLoginResultCommandMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
{

}
//���յ�MemberList����SIP��Ϣ�����³�Ա�б�
void ConferenceManager::recvMemberListCommandMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
{
    if(!checkIsInConference())
        return;
    if( _currentConfInfo == NULL )
        return;

    MemberVector tmpMemberVector;
    cvMessageParser.ParseMemberListMsg(tmpMemberVector, RunningProfile::getInstance()->user_uri() , _confMode);
    cvMemberListReceivedSignal(tmpMemberVector, _confTitle);

    for( int i = 0 ; i < tmpMemberVector.size() ; i++ )
    {
        _memberList.push_back( tmpMemberVector.at(i) );
    }
}

bool IgnoreTerminal(TerminalInfo & recvTerminal)
{
	if (recvTerminal.name.find("TX") != -1)
	{
		return true;
	}
	else if (recvTerminal.uri == RunningProfile::getInstance()->user_uri() && 
		CVIniConfig::getInstance().getModelCategory() == CVCFG_VALUE_MODEL_CATEGORY_TX)
	{
		return true;
	}
	return false;
}

//���յ��ն��б�
//������ÿ��Ӧ�÷���һ���ն˵�һ�������ն��б�
void ConferenceManager::recvTerminalListCommandMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
{
    if( !checkIsInConference() || _currentConfInfo == NULL)
        return;

	TerminalVector recvTerminals;

	cvMessageParser.ParseTerminalListMsg( recvTerminals , _hostURI , _confMode );
	if (recvTerminals.size() == 0)
	{
		return;
	}

	RecvTerminalListSlot(recvTerminals);
}

void ConferenceManager::RecvTerminalListSlot(TerminalVector recvTerminals)
{
	TerminalVector checkedTerminals;
	std::string recvTerminalUri = recvTerminals[0]->uri;

	// ���Ϸ���
	// 2013-10-29�޶�(Liaokz)�����������Ϊһ�η�һ�������նˣ��ʺϷ��Լ�鲻����Ҫ����Ϊ������յ��ն����Ƿ�ƥ��
	// ����������ն�����֮ǰ��ͬ������ձ��������
	if (_terminalList[recvTerminalUri].size() > 0 &&
		_terminalList[recvTerminalUri][0]->virtualCount != recvTerminals[0]->virtualCount)
	{
		CVMsgParser::ReleaseTerminalList( _terminalList[recvTerminalUri] );
		_terminalList.erase(recvTerminalUri);
	}
	checkedTerminals = recvTerminals;
	//for( int i = 0; i < recvTerminals.size() ; i++ )
	//{
	//	TerminalInfo* recvTerminal = recvTerminals.at(i);

	//	// ���ԷǸ��ն˵������ն�
	//	if (recvTerminal->uri != recvTerminals[0]->uri)
	//	{
	//		continue;
	//	}
	//	checkedTerminals.push_back(recvTerminal);
	//}
	// �Ϸ��Լ���ѭ����֤��checkedTerminals������һ��Ԫ��

	// �����ն���Ϣ�����жϳ�Ա�б����Ƿ�������Ӧ�ĳ�Ա
	bool bFirstTimeReceived = true; // ���ڼ�¼�ն��Ƿ��ظ�����.���ն˽��ղ�ȫ��,���ظ������ն��б�,�����ظ��յ��Լ�����Ϣ
	// ���޼������ظ���ʼ��������,��ɲ���Ԥ�ϵ�����
	for (TerminalVector::iterator it = checkedTerminals.begin(); 
		it != checkedTerminals.end(); ++it)
	{
		for (TerminalVector::iterator it2 = _terminalList[recvTerminalUri].begin();
			it2 != _terminalList[recvTerminalUri].end(); ++it2)
		{
			if ((*it)->virtualUri == (*it2)->virtualUri)
			{
				// �ظ������ն���Ϣʱ,ɾ��ԭ��Ϣ
				delete (*it2);
				_terminalList[recvTerminalUri].erase(it2);
				bFirstTimeReceived = false;
				break;
			}
		}
		_terminalList[recvTerminalUri].push_back(*it);

		_snprintf(__globalMessege , sizeof(__globalMessege) , "Recv terminal %s(%s)\n",
			(*it)->name.c_str(), (*it)->virtualUri.c_str() );
		CV_LOG_DEBUG( __globalMessege );
	}

	// ���Լ��������ն���ȫ��,��¼��ǰ�ն���Ϣ,����ʼ�����ͽ���
	if (recvTerminalUri == RunningProfile::getInstance()->user_uri() && 
		_terminalList[recvTerminalUri].size() == checkedTerminals[0]->virtualCount &&
		bFirstTimeReceived)
	{
		//createSendStreamMedia();
		initSendStreamMedia();
	}

	// ֪ͨ�������
	// �ȼ���ն�����������������棬��֪���ն������������յ�һ���жϺ󣬻��������Ƿ��㹻���������������ÿ���ӻ����¼��һ��
	// TODO�������Ӧ���ɿ��������
	int num = checkedTerminals[0]->totalTermialNum;
	for( TerminalMap::iterator it = _terminalList.begin(); it != _terminalList.end(); it++ )
	{
		if (it->second.size() > 0 && IgnoreTerminal(*(it->second[0])))
		{
			--num;
		}
	}
	// ���������֪ͨ����
	if(num != _totalTermianlNum)
	{
		_totalTermianlNum = num;
		cvTermianlCountChangeNotify(_totalTermianlNum);
	}

	// ������TX�ն˵Ĵ���
	if (_terminalList[recvTerminalUri].size() && 
		!IgnoreTerminal(*_terminalList[recvTerminalUri][0]))
	{
		cvTerminalListReceivedSignal( _terminalList[recvTerminalUri] , _focusURI );
		// ���ڷ��͸����������ն��б���ÿ�յ�һ���µ������ն���Ϣ��֮ǰ�յ����ն���Ϣ���ᱻ�ٴη��ͣ��źŽ��շ����ܴ���
	}
}

void ConferenceManager::createSendStreamMedia()
{
	if (!CVIniConfig::getInstance().IsModelHD())
	{
		// ������������նˣ�����������ý����
		return;
	}

	RunningConfig* pConfig = RunningConfig::Instance();

	bool enableSmallVideo = pConfig->isEnableSmallVideo();

	// ��������ͷ������������ý����
	const int numberOfStreams = pConfig->VideoCaptureDeviceCount();

	for (int i = 0; i < numberOfStreams; i++)
	{
		VCapSetting vcap = pConfig->VideoCaptureDevice(i);
		if (vcap.devicePath != "")
		{
			//����ý��������
			//֮����Ҫ�ڴ�����������ʱ��ָ�����ػ��Զ˿ڣ�����Ϊ�������һ������ͬ������
			//һ��ģ�����Ҫ�ڷ������ɹ����к󣬲�����ӷ��Ͷ˿ڡ�
			//�����ػ��Խ��ս����뷢�ͽ��̼���ͬʱ���������޷���֤addRecvStreamMedia����ʱ�������Ѿ�����
			//��˷�����Ӧ�ڴ�����ɺ�����������Ƶ�������ػ���
			// Liaokz����˵��
			MediaManager::getInstance()->CreateSendMedia(i, 
				"127.0.0.1" ,
				0, //���ͱ��ػ�����Ƶ����������Ƶ����
				(enableSmallVideo ? LOCALPRE_SMALL_PORT_BASE + 10 * i : LOCALPRE_VIDEO_PORT_BASE + 10 * i),
				enableSmallVideo, 0);
		}

		//TODO:��ʱ�ڴ˴������ػ���
		std::string localID = getLocalPreviewMediaID(i).toStdString();
		addRecvLocalMedia(localID, 0, 6 - i, true);
	}
}

void ConferenceManager::initSendStreamMedia()
{
	if (!CVIniConfig::getInstance().IsModelHD())
	{
		// ������������նˣ�����ʼ��
		return;
	}

	TerminalVector myVirtualTerminals = _terminalList[RunningProfile::getInstance()->user_uri()];
	if (myVirtualTerminals.size() == 0)
	{
		return;
	}

	//�ڱ�������У�ֻ�������˿��Է�����Ƶ
	if(myVirtualTerminals[0]->isSpeaking=="false")
	{
		MediaManager::getInstance()->ControlAudioSend(false);
	}
	else
	{
		MediaManager::getInstance()->ControlAudioSend(true);
	}

	RunningConfig* pConfig = RunningConfig::Instance();
	bool enableSmallVideo = pConfig->isEnableSmallVideo();

	for (int i = 0; i < myVirtualTerminals.size(); i++)
	{
		TerminalInfo * curVirtualTerminal = myVirtualTerminals[i];
		VCapSetting vsetting = RunningConfig::Instance()->VideoCaptureDevice(curVirtualTerminal->virtualIndex);
		if (vsetting.devicePath == "")
		{
			continue;
		}

		//���͸��鲥��ַ
		if( checkSupportMulticast( _currentConfInfo->multicastAddress ) )
		{
			_receiverInfo.AddRemoteReceiver(curVirtualTerminal->virtualIndex, 
				MULTICAST_URI, 
				curVirtualTerminal->multicastAddress.c_str(), 
				curVirtualTerminal->videoPort, 
				RunningConfig::Instance()->AudioCodec() == "" ? 0 : curVirtualTerminal->audioPort,
				false);

			MediaManager::getInstance()->AddSendMedia(curVirtualTerminal->virtualIndex, // ��������ն˶�Ӧ�ķ��ͽ�����ӷ��Ͷ�
				curVirtualTerminal->multicastAddress.c_str(), 
				RunningConfig::Instance()->AudioCodec() == "" ? 0 : curVirtualTerminal->audioPort, 
				curVirtualTerminal->videoPort, 
				false, 0);

			if(enableSmallVideo)
			{
				//��С��Ҳ�����鲥��ַ
				_receiverInfo.AddRemoteReceiver(curVirtualTerminal->virtualIndex, 
					MULTICAST_SMALL_URI, 
					curVirtualTerminal->multicastAddress.c_str(), 
					curVirtualTerminal->smallVideoPort, 
					0,
					true);

				MediaManager::getInstance()->AddSendMedia(curVirtualTerminal->virtualIndex, 
					curVirtualTerminal->multicastAddress.c_str(),
					0, // С������Ƶ
					curVirtualTerminal->smallVideoPort,
					true, 0, false);
			}
		}

		//��������¼��
		if (pConfig->isEnableLocalRecord())
		{
			addRecvLocalMedia(getLocalRecordMediaID(curVirtualTerminal->virtualIndex).toStdString(), -1, -1);
		}
	}

	// ����¼�Ʒָ��ʱ��.ע��:���ܱ���¼���Ƿ����ö�Ӧ������ʱ��,ΪԶ��TX�ṩ�ֶ�����
	int sec = CTxConfig::getInstance().GetRecLocalDuration();
	if (sec > 0)
	{
		QObject::connect(&_localVideoAutoCutTimer, SIGNAL(timeout()), this, SLOT(SegmentLocalMedia()));
		_localVideoAutoCutTimer.start(sec * 1000);
	}
}

void ConferenceManager::resetSendStreamMedia()
{
	//ɾ�����˱��ػ�������ķ��Ͷ�
	ReceiverInfoManager::InfoList list = _receiverInfo.ToInfoList();
	for (ReceiverInfoManager::InfoList::iterator it = list.begin();
		 it != list.end(); ++it)
	{
		if (isLocalPreviewMediaID(it->MemberURI.c_str()))
		{
			continue;
		}

		//ֹͣ����
		MediaManager::getInstance()->RemoveSendMedia( it->virtualIndex, it->recvIP.c_str(),
			it->audioPort,
			it->videoPort,
			it->smallVideo);
		//ɾ��������Ϣ - ʹ��URI,�����޷����ֱ��ػ��Ժͱ���¼��
		_receiverInfo.RemoveRemoteReceiverByURI(it->virtualIndex, it->MemberURI.c_str());
	}
}

//���յ�LoginNotify����SIP��Ϣ�����˼������
void ConferenceManager::recvLoginNotifyMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
{
    if(!checkIsInConference())
        return;

    int loginType = cvMessageParser.GetLoginNotifyType();
    if( loginType == 0 )
    {
        MemberInfo* member;
        member = cvMessageParser.ParseMemberLoginNotifyMsg( _memberList , _hostURI , _confMode ) ;
        if( member )
        {
            cvMemberLoginSignal( *member );
            return;
        }
    }
    else if( loginType == 1 )
    {
		// case:�������ᷢ��һ̨��¼�ն˵����������ն˵��б�
		TerminalVector recvTerminals;
		TerminalVector checkedTerminals;

        cvMessageParser.ParseTerminalLoginNotifyMsg( recvTerminals, _hostURI , _confMode );
		if (recvTerminals.size() == 0)
		{
			return;
		}

		// ���Ϸ���
		//for( int i = 0; i < recvTerminals.size() ; i++ )
		//{
		//	TerminalInfo* recvTerminal = recvTerminals.at(i);

		//	// ���ԷǸ��ն˵������ն�
		//	if (recvTerminal->uri != recvTerminals[0]->uri)
		//	{
		//		continue;
		//	}
		//	checkedTerminals.push_back(recvTerminal);
		//}
		// ��ѭ����֤��checkedTerminals������һ��Ԫ��
		checkedTerminals = recvTerminals; //TODO:��������ָĻ�һ�η�һ�������նˣ���ע���ϲ��ִ���

		string loginUri = checkedTerminals[0]->uri;
		if(loginUri == _TerminalChairmanURI)
		{
			checkedTerminals[0]->isChairmanTerminal="true";
			checkedTerminals[0]->isSpeaking="true";
		}
		
		// �����ն���Ϣ
		for (TerminalVector::iterator it = checkedTerminals.begin(); 
			it != checkedTerminals.end(); ++it)
		{
			for (TerminalVector::iterator it2 = _terminalList[loginUri].begin();
				it2 != _terminalList[loginUri].end(); ++it2)
			{
				if ((*it)->virtualUri == (*it2)->virtualUri)
				{
					// �ظ������ն���Ϣʱ,ɾ��ԭ��Ϣ
					delete (*it2);
					_terminalList[loginUri].erase(it2);
					break;
				}
			}
			_terminalList[loginUri].push_back(*it);
		}

		// ֪ͨ����
		if (!IgnoreTerminal(*checkedTerminals[0]))
		{
			cvTerminalLoginSignal( checkedTerminals );
			// Auto recv
			//addRecvStreamMedia( RunningProfile::getInstance()->getUserName() , terminal->uri, 0, 1);
		}
    }

}

//���յ�LogoutNotify����SIP��Ϣ�������˳�����
void ConferenceManager::recvLogoutNotifyMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
{
    if(!checkIsInConference())
        return;

	// ������ֻ�����˳��ն˵�URI����Ҫ�ҵ������������ն˲���������
    string logoutUserUri = cvMessageParser.GetMemberUri();

    //���������£���������˳�����Ƿ��з���Է���ý����������ֹͣ����
    if(!checkCurrentIsMulticastConference())
    {
		const int virtualCount = RunningConfig::Instance()->VideoCaptureDeviceCount();
		//���ÿһ·������
		for (int i = 0; i < virtualCount; ++i)
		{
			ReceiverInfo * receiver  = _receiverInfo.FindRemoteReceiverByURI(i, logoutUserUri);
			if (receiver)
			{
				//�رշ���ý����
				MediaManager::getInstance()->RemoveSendMedia( receiver->virtualIndex, receiver->recvIP.c_str() , receiver->audioPort , receiver->videoPort );
				//ɾ��������Ϣ
				_receiverInfo.RemoveRemoteReceiverByIP(receiver->virtualIndex, receiver->recvIP);
			}
		}
    }

    //�رս��յ���Ļ��
    MediaManager::getInstance()->RemoveRecvScreenMedia(logoutUserUri.c_str() );

	if (_terminalList.find(logoutUserUri) != _terminalList.end())
	{
		TerminalVector & virtualList = _terminalList[logoutUserUri];
		for (TerminalVector::iterator it = virtualList.begin(); it != virtualList.end(); ++it)
		{
			TerminalInfo * terminal = *it;
            //����н�����ý�������رս���ý����
			MediaManager::getInstance()->RemoveRecvMedia( terminal->virtualUri.c_str() );

			//����״̬
			terminal->status = "Not Available";
			if(terminal->uri == _TerminalChairmanURI)
			{
				terminal->isChairmanTerminal="false";
				if(_confMode == "report")
					terminal->isSpeaking="false";
				cvUpgradeTerminalInfoSignal(*terminal);
				_TerminalChairmanURI ="";
			}
		}

		//֪ͨ������״̬
		emit cvTerminalLogoutSignal(QString::fromStdString(virtualList[0]->uri));
	}

    //���³�Ա�б���Ա�б�
    //_SipAccountLogout( logoutUserUri );
    //cvMessageParser.ParseMemberListByNotifyMsg(_memberList, _hostURI, _confMode);
}

//���յ�Invite����SIP��Ϣ���ܵ�����
void ConferenceManager::recvInviteMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
{
    ConfInfo* conf = conf_find_by_uri(_confList, fromUri);
    if(conf)
    {
        string inviter = cvMessageParser.GetInviterUri();
        cvConfrenceInviteSignal( conf, inviter);
    }
}
//���յ�ApplyConfResult����SIP��Ϣ�����뼴ʱ����ɹ�
void ConferenceManager::recvApplyConfResultMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
{
    string uri = cvMessageParser.GetFocusURI();
    if(uri != "")
    {
        makeJoinConfInvite(uri);
    }
}

//���յ�PasswordIncorrect����SIP��Ϣ�����벻��ȷ
void ConferenceManager::recvPasswordIncorrectMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
{
    cvPasswordIncorrectSignal();
}

void ConferenceManager::recvTextMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
{
    string userId = fromUri;
    int index = userId.find('@');
    if(index != std::string::npos)
    {
        userId = userId.substr(0, index);
    }
    string msg = cvMessageParser.GetTextMessage();
    cvTextMsgReceivedSignal( msg, userId);
}

//���յ�LoginResult����SIP��Ϣ�����߳�����
void ConferenceManager::recvKickedMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
{
    cvKickedSignal();
}

//���յ�WhiteBoard����SIP��Ϣ���յ��װ���Ϣ
void ConferenceManager::recvWhiteBoardMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
{
    string userId = fromUri;
    int index = userId.find('@');
    if(index != std::string::npos)
    {
        userId = userId.substr(0, index);
    }
    string msg = cvMessageParser.GetWhiteBoardMessage();
    cvWhiteBoardMsgReceivedSignal( msg, userId);
}

//���յ�HandUp����SIP��Ϣ�����ַ���
void ConferenceManager::recvHandUpMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
{
    QString userUri =  QString::fromStdString(cvMessageParser.GetTerminalUri());
	TerminalInfo* remoteTerminal = FindTerminal( _terminalList , userUri.toStdString() );
	if(remoteTerminal!= NULL)
	{
		remoteTerminal->isHandUp="true";
		cvUpgradeTerminalInfoSignal(*remoteTerminal);
	}
	//emit cvMemberHandUpSignal(userUri);
    //MemberInfo* member =  member_find_by_uri(_memberList,userUri);
    //if (member)
    //{
    //	member->hand = 1;
    //	//���������Զ���
    //	_requestForSpeakList.push_back(member);
    //}
    //cvMemberListReceivedSignal( _memberList, _confTitle);
}

void ConferenceManager::recvHandDownMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
{
    QString userUri =  QString::fromStdString(cvMessageParser.GetTerminalUri());
	TerminalInfo* remoteTerminal = FindTerminal( _terminalList , userUri.toStdString() );
	if(remoteTerminal!= NULL)
	{
		remoteTerminal->isHandUp="false";
		cvUpgradeTerminalInfoSignal(*remoteTerminal);
	}
    //emit cvMemberHandDownSignal( userUri );
}

/*  �ѹ��ڲ���*/
//void ConferenceManager::recvReceiverMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
//{
//    //��ȡ�Է���ַ������/�鲥IP��ַ
//    string recvIP = cvMessageParser.GetReceiverIP();
//    string remoteIPAddr;					//Զ�̵�ַ��ʵ����ֱ��ʹ��recvIP
//    int videoPort = 0, audioPort = 0;
//
//    //�ж��Ƿ��鲥���� , ��������Ŀ��˿��ǶԷ��ģ��鲥����Ŀ��˿����Լ���
//    if(checkCurrentIsMulticastConference())
//    {
//        // �鲥����ʹ���鲥��ַ�����͵��Լ��Ķ˿�
//        getCurrentMulticastAddress(&remoteIPAddr);		//��ʱremoteIPAddr==recvIP
//    }
//    else
//    {
//        // ��������ʹ�öԷ�IP��ַ�����͵��Լ��Ķ˿�
//        remoteIPAddr = recvIP;
//    }
//    //���͵��Լ��Ķ˿�
//    MemberInfo* currentMember = NULL;
//    for( MemberVector::iterator it = _memberList.begin() ; it != _memberList.end() ; it++ )
//    {
//        MemberInfo*  member = (MemberInfo*) * it;
//        if( member->MemberURI == RunningProfile::getInstance()->user_uri() )
//        {
//            currentMember = *it;
//            break;
//        }
//    }
//
//    if (!currentMember)
//        return;
//    videoPort = currentMember->videoPort;
//    audioPort = currentMember->audioPort;
//
//
//
//    if( _confMode == "host" && currentMember->MemberURI != _hostURI && _memberList[0]->permitSpeak == 0)
//    {
//        MediaManager::getInstance()->AddSendMedia( remoteIPAddr.c_str() , 0 , videoPort,false, 0 );
//    }
//    else
//    {
//        MediaManager::getInstance()->AddSendMedia( remoteIPAddr.c_str() , audioPort , videoPort,false, 0 );
//    }
//
//    //��¼�Է���Ϣ���Ա�ɾ��
//	ReceiverInfo::AddRemoteReceiver( _sendIPList, fromUri, remoteIPAddr, videoPort, audioPort );
//}

void ConferenceManager::recvScreenReceiverMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
{

}

void ConferenceManager::recvUnShareScreenMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
{

}

//�ѹ��ڲ��ã����յ�Cancel2Receiver����SIP��Ϣ������ý�����ķ���
//void ConferenceManager::recvCancel2ReceiverMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
//{
//    //�鲥���鲻ֹͣ����
//    if(checkCurrentIsMulticastConference())
//        return;
//
//    string recvIP = cvMessageParser.GetReceiverIP();
//
//	const int virtualCount = RunningConfig::Instance()->VideoCaptureDeviceNumber();
//	for (int i = 0; i < virtualCount; ++i)
//	{
//		ReceiverInfo * receiver  = _receiverInfo.FindRemoteReceiverByIP(i, recvIP);
//		if (receiver)
//		{
//			//�رշ���ý����
//			MediaManager::getInstance()->RemoveSendMedia( receiver->virtualIndex, receiver->recvIP.c_str() , receiver->audioPort , receiver->videoPort );
//			//ɾ��������Ϣ
//			_receiverInfo.RemoveRemoteReceiverByIP(receiver->virtualIndex, receiver->recvIP);
//		}
//	}
//	//ReceiverInfo * receiver  = ReceiverInfo::FindRemoteReceiverByIP(_receiverInfo, recvIP);
// //   if (receiver)
// //   {
// //       MediaManager::getInstance()->RemoveSendMedia(  receiver->recvIP.c_str() , receiver->audioPort , receiver->videoPort );
// //       //ɾ��������Ϣ
//	//	ReceiverInfo::RemoveRemoteReceiverByIP(_receiverInfo, recvIP);
// //   }
//}

void ConferenceManager::recvScreenCancel2ReceiverMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
{

}

void ConferenceManager::recvQoSMsgHandler( CVMsgParser& cvMessageParser, const std::string& fromUri )
{
    RunningConfig* config = RunningConfig::Instance();
    string strType = cvMessageParser.GetQosType();
    const char* type = strType.c_str();
    if(strcmp(type, "LoginResult") == 0)    //������������
    {
        //sucess��ʾ�������ڷ���LoginPermission��Ϣ
        //failure��ʾʧ�ܣ�������qos������Э��
    }
    else if(strcmp(type, "LoginPermission") == 0)   //qos����������
    {
        MsgLoginPermissionInfo info;
        cvMessageParser.ParseLoginPermissionMsg(info);

        if( _confState == WaitingPermission )
        {
            //��ʼ���뵱ǰ����
            joinConference(_focusURI);
            //��¼DSCPֵ
            int videoDscp =  info.dscp;
            int audioDscp =  info.dscp;
#ifdef _DEBUG
            if (CVIniConfig::getInstance().isUseCustomedDscp())  //ǿ��ʹ��ini�����е�DSCPֵ���÷�ʽ�����ڲ��ԣ���������ʱӦʹ�÷����������
            {
                videoDscp = CVIniConfig::getInstance().getVideoDscp();
                audioDscp = CVIniConfig::getInstance().getAudioDscp();
            }
#endif
            MediaManager::getInstance()->SetDSCP(videoDscp, audioDscp);
			
			//�������������ѯ�ն��б�
			GetTermialList();


        }
    }
    else if(strcmp(type, "LoginRejection") == 0)    //qos������������
    {
        if( _confState == WaitingPermission ) //ֻ���ڵȴ���֤��ʱ�����Ч, zhenHua.sun 20110225
        {
            MsgLoginRejectionInfo info;
            cvMessageParser.ParseLoginRejectionMsg(info);
            //��������뵱ǰ����
            rejectToJoinConference(_focusURI, info.description);
        }
    }
    else  if(strcmp(type, "QoSServerInfo") == 0)		//QoS��������Ϣ
    {
        MsgQoSServerInfo info;
        if ( cvMessageParser.ParseQoSServerInfoMsg(info) == 0 )
            saveQoSServerInfo(info);
    }
}

//�жϵ�ǰ�����Ƿ��鲥����
bool ConferenceManager::checkCurrentIsMulticastConference()
{
    if(!checkIsInConference())
        return false;

    return checkSupportMulticast( _currentConfInfo->multicastAddress );
}

std::string ConferenceManager::createJoinConferenceMsg( const string& confUri )
{
    string cid = "";

    //����uri���һ����cid
    ConfInfo* conf = conf_find_by_uri(_confList, confUri);
    if(conf)
        cid = conf->CID;

    string reporterUri = RunningProfile::getInstance()->user_uri();
    string reporterIP = RunningProfile::getInstance()->ip_address();
    char  strGateway[60];
    memset(strGateway, 0, 60);
    GetGatewayByHostIP(reporterIP.c_str(), strGateway);
    string reporterGateway(strGateway);

    //������һ��Ĭ�ϵĴ�����ֵ
    string minRate = "50";
    stringstream ss;
    ss << computeRequiredBandwidth();
    string maxRate = ss.str();

	int virtualLineCount = RunningConfig::Instance()->VideoCaptureDeviceCount();
	char sVirtualLineCount[10] = {0};
	itoa(virtualLineCount, sVirtualLineCount, 10);

    string szContent = "<?xml version=\"1.0\"?><coolview command=\"qos\" type=\"Login\">";
    szContent += ("<cid>" + cid + "</cid>");
    szContent += ("<focusuri>" + confUri + "</focusuri>");
    szContent += ("<uri>" + reporterUri + "</uri>");
    szContent += ("<minRate>" + minRate + "</minRate>");
    szContent += ("<maxRate>" + maxRate + "</maxRate>");
    szContent += ("<ip>" + reporterIP + "</ip>");
	szContent += ("<gateway>" + reporterGateway + "</gateway>");
	szContent += (std::string("<virtualTerminalCount>") + sVirtualLineCount + "</virtualTerminalCount>");
    szContent += "</coolview>";
    return szContent;
}

void ConferenceManager::exitConference()
{
    if( _currentConfInfo )
    {
        if( checkIsInConference() )
		{
			//ֹͣ���ս���
            MediaManager::getInstance()->ExitConference();
			//ֹͣ����
			resetSendStreamMedia();
		}

        _sipWrapperProxy->exitConference( QString::fromStdString(_currentConfInfo->URI) );
        _confState = NotInConference;
        _hostURI = "";
        _confMode = "";
        _confTitle = "";
		_speakerURI = "";
		_TerminalChairmanURI="";
		_currentScreenShareUri="";
		_totalTermianlNum=0;
		CVMsgParser::ReleaseMemberList( _memberList );

		for (TerminalMap::iterator it = _terminalList.begin(); it != _terminalList.end(); ++it)
		{
			CVMsgParser::ReleaseTerminalList( it->second );
		}
		_terminalList.clear();

		CVMsgParser ::ReleaseConfList(_confList);

        ///ֹͣ����״̬��ʱ��
        _onlineInfoTimer->stop();
		_serverHeartbeatTimer->stop();
        _currentConfInfo = NULL;
		_localVideoAutoCutTimer.stop();

        CVIniConfig::getInstance().setCurrentConferenceUri("");
        CVIniConfig::getInstance().setCurrentConferenceCid(-1);

    }
}

void ConferenceManager::MemberHandup( const std::string memberURI , const bool handup )
{
    if( _currentConfInfo )
    {
        std::string command;
        if(handup)
            command = "HandUp";
        else
            command = "HandDown";

        std::string szContent = "<?xml version=\"1.0\"?><coolview command=\"" + command + "\"><uri>" + memberURI + "</uri></coolview>";

        _sipWrapperProxy->makeInfoCall( QString::fromStdString( RunningProfile::getInstance()->current_conference_uri() ) , QString::fromStdString(szContent) );
    }

}

void ConferenceManager::startConference( const std::string& username , const std::string& sipServer )
{
    //����focus����������ȡ�����б�
    QString szContent = "<?xml version=\"1.0\"?><coolview command=\"GetConfList\"></coolview>";
    _sipWrapperProxy->startConference( QString::fromStdString(username) , QString::fromStdString(sipServer), szContent );

}

void ConferenceManager::makeInfoCall( const std::string& fousURI , const std::string& szContent )
{
    _sipWrapperProxy->makeInfoCall( QString::fromStdString(fousURI) , QString::fromStdString(szContent) );
}


void ConferenceManager::addRecvStreamMedia( const std::string username , const std::string remoteMemberURI , const int screenIndex , const int seet)
{
	if( !checkIsInConference() )
	{
		qDebug("û�м����κλ���!");
		return;
	}

	//Step1: �ռ���Ϣ
	bool isRecord = false; //�Ƿ�Ϊ¼�ƽ���
	bool isMulti =false;  //�Ƿ����鲥
	bool usingSmallVideo = seet > 1; //�Ƿ�ʹ��С��
	TerminalInfo* remoteTerminal = FindTerminal( _terminalList , remoteMemberURI );

    if( remoteTerminal == NULL )
    {
		return;
	}
	//����û�Ҫ����ܵ��Ǳ��ػ��ԣ���ת���ô������ؽ��ս��̵ķ���
	if (remoteTerminal->uri == RunningProfile::getInstance()->user_uri())
	{
		addRecvLocalMedia(getLocalPreviewMediaID(remoteTerminal->virtualIndex).toStdString(), 
			screenIndex, seet, usingSmallVideo);
		return;
	}

	if (CVIniConfig::getInstance().IsModelTX())
	{
		isRecord = true;
		usingSmallVideo = false; // TX¼��ֻʹ�ô���
	}
	else if (!RunningConfig::Instance()->isEnableSmallVideo())
	{
		usingSmallVideo = false;
	}

    //�����ն˵�ý����
    //��ȡԶ���ն˵�ý����� - ���ػ��Ժͽ���Զ��ͳһ���� - Liaokz, 2013-9
	int videoWidth;
	int videoHeight;
	int videoFps;
	char audioCodec[20] = {0};
	getMediaParamFromSdp( &videoWidth , &videoHeight , &videoFps, audioCodec, remoteTerminal->sdp.c_str() );
	std::string sAudioCodec( audioCodec );
	int remoteAudioPort = sAudioCodec.size() > 0 ? remoteTerminal->audioPort : 0;
	int remoteVideoPort;

	// ��ʼ��ý������Ϣ
	if( usingSmallVideo) 
	{
		remoteVideoPort = (videoHeight == 0 || videoWidth == 0) ? 0 : remoteTerminal->smallVideoPort;

		//ȫ��������ͷ����Ƶ��������4����460x270�����Ǳ���������֧�ָ���Ƶ��ʽ�����ͳһΪ320x180
		//...zhenhua.sun 2012/2/22
		videoWidth = MSDX_CONF_SMALL_VIDEO_WIDTH;
		videoHeight = MSDX_CONF_SMALL_VIDEO_HEIGHT;
		videoFps /=2;
	}else
	{
		remoteVideoPort = (videoHeight == 0 || videoWidth == 0) ? 0 : remoteTerminal->videoPort;
	}

	std::string multiAddr =_currentConfInfo->multicastAddress;
	string localAddress = "127.0.0.1";

	//Step2: �����Ͷ˷�������
	// ����Զ���鲥
    if( ConferenceManager::checkSupportMulticast( multiAddr ) )
    {
        localAddress = multiAddr;
		isMulti=true;
    }
	// ����Զ�̵���
    else
    {
		localAddress = RunningProfile::getInstance()->ip_address();
        //��������鲥���飬��ô��Ҫ����Sip��ϢҪ��Է��û�����ý����
		QString szContent = "<?xml version=\"1.0\"?><coolview command=\"StartMedia\"><uri>" +
			QString::fromStdString(remoteTerminal->uri) + "</uri>" + 
			"<virtualUri>" + QString::fromStdString(remoteTerminal->virtualUri) + "</virtualUri>" + 
			"<ip>" + localAddress.c_str() + "</ip><type>%1</type></coolview>";
		if( usingSmallVideo )
			szContent = szContent.arg("smallStream");
		else
			szContent = szContent.arg("stream");
        QString remoteUniURI = QString("sip:") + _currentConfInfo->URI.c_str();
        _sipWrapperProxy->makeInfoCall( _currentConfInfo->URI.c_str() , szContent );
    }

	//Step3: ֪ͨý�����������������ս���
	//��������ӽ��ճ�Ա��λ����Ϣ
	MediaManager::getInstance()->AddRecvMemberPosition( remoteTerminal->virtualUri.c_str() , screenIndex , seet, usingSmallVideo);

    RecvGraphInfo info;
    info.initial( qPrintable(RunningConfig::Instance()->AudioCaptureDevice()), qPrintable(RunningConfig::Instance()->AudioOutputDevice()),
                    localAddress.c_str(), remoteAudioPort  , audioCodec , 16000, 16 , 1 , localAddress.c_str() ,
                    remoteVideoPort , 0 , videoWidth , videoHeight , videoFps, remoteTerminal->virtualUri.c_str(), remoteTerminal->name.c_str(),
                    //RunningConfig::getInstance()->isEnableRecvAutoResync());
					false, "", CTxConfig::getInstance().GetRecLocalDuration() );

	if (isRecord)
	{
		QString file = getRecordPath(QString::fromUtf8(remoteTerminal->name.c_str()), remoteTerminal->virtualIndex);
		info.record_info.file_name = file;
	}

    MediaManager::getInstance()->AddRecvMedia( info );

	//Step4: ��������
	//��Ƶͬ��
	if("true"!=remoteTerminal->isSpeaking)
	{
		//���Ļ���������Ƶ�����ϵ���Ƶͼ��
		MediaManager::getInstance()->ChangeMediaState( remoteTerminal->virtualUri.c_str(), "audio" , UiMediaState_Stop ,true );
		//MediaManager::getInstance()->ControlAudioRecv( remoteUsername ,false );
	}
	if(isMulti)
	{
		MediaManager::getInstance()->ChangeMediaState(remoteTerminal->virtualUri.c_str() , "video" , UiMediaState_Ready ,true);
	}

	//֪ͨ�����Ѵ������ս���
	streamMediaReceiveStartedSignal(QString::fromStdString(remoteTerminal->virtualUri));

    _snprintf(__globalMessege , sizeof(__globalMessege) , "@Recv Media From %s, WIDTH:%d , HEIGHT:%d, VideoPort:%d , AudioPort:%d \n",
                remoteTerminal->virtualUri.c_str() , videoWidth , videoHeight , remoteTerminal->videoPort , remoteTerminal->audioPort );
    CV_LOG_DEBUG( __globalMessege );
    
}

void ConferenceManager::addRecvLocalMedia(const std::string localMediaID, const int screenIndex, const int seet, bool smallvideo)
{
	//Step1: �ռ���Ϣ
	std::string localAddress = "127.0.0.1";
	std::string baseMediaID;
	int videoPort = 0;
	int audioPort = 0;
	int virtualIndex = 0;
	int videoWidth = 0;
	int videoHeight = 0;
	int videoFps = 0;
	RunningConfig * config = RunningConfig::Instance();

	//���������ն˺�������Ϣ
	if (isLocalPreviewMediaID(localMediaID.c_str()))
	{
		virtualIndex = getVirtualIndexFromLocalPreviewMediaID(localMediaID.c_str());
		if (!config->isEnableSmallVideo())
		{
			smallvideo = false;
		}
		if (smallvideo)
		{
			videoPort = LOCALPRE_SMALL_PORT_BASE + 10 * virtualIndex;
		} 
		else
		{
			videoPort = LOCALPRE_VIDEO_PORT_BASE + 10 * virtualIndex;
		}
		baseMediaID = LOCAL_PREVIEW_MEDIA_ID;
	}
	else if (isLocalRecordMediaID(localMediaID.c_str()))
	{
		virtualIndex = getVirtualIndexFromLocalRecordMediaID(localMediaID.c_str());
		smallvideo = false;
		videoPort = LOCALREC_VIDEO_PORT_BASE + 10 * virtualIndex;
		audioPort = LOCALREC_AUDIO_PORT_BASE + 10 * virtualIndex;
		baseMediaID = LOCAL_RECORD_MEDIA_ID;
	}
	else
	{
		return;
	}

	//���ݸ�·��Ƶ������Ҫ�޸���Ϣ
	VCapSetting vsetting = config->VideoCaptureDevice(virtualIndex);

	if (vsetting.devicePath == "")
	{
		//�ײ�Ӧ���ݴ���Ϣ�����Ƿ񴴽�ý�������ս���
		videoPort = 0;
		audioPort = 0;
	}
	videoWidth = vsetting.width;
	videoHeight = vsetting.height;
	videoFps = vsetting.fps;

	if (smallvideo)
	{
		videoWidth = MSDX_CONF_SMALL_VIDEO_WIDTH;
		videoHeight = MSDX_CONF_SMALL_VIDEO_HEIGHT;
	}

	//Step2: ��ӷ��Ͷ�
	MediaManager::getInstance()->AddSendMedia(virtualIndex, localAddress.c_str(), audioPort, videoPort, smallvideo, 0);
	_receiverInfo.AddRemoteReceiver(virtualIndex, baseMediaID, localAddress.c_str(), videoPort, audioPort, smallvideo);

	//Step3: ֪ͨý�����������������ս���
	MediaManager::getInstance()->AddRecvMemberPosition( localMediaID.c_str() , screenIndex , seet, smallvideo);
	//TODO: Ӳ�����±��ػ��Բ�������Ƶ��·ʱ�˳��������������ʱ�������һ��û�����ݵĶ˿�
	if (videoPort != 0 && audioPort == 0)
	{
		audioPort = 8500 + 10 * virtualIndex;
	}

	RecvGraphInfo info;
	info.initial( qPrintable(config->AudioCaptureDevice()), qPrintable(config->AudioOutputDevice()),
		localAddress.c_str(), audioPort  , qPrintable(config->AudioCodec()) , config->AudioSampleRate(), config->AudioBitsPerSample(), config->AudioChannel(),
		localAddress.c_str(), videoPort , NULL, videoWidth , videoHeight , videoFps, localMediaID.c_str(), localMediaID.c_str(),
		//RunningConfig::getInstance()->isEnableRecvAutoResync());
		false);

	if (isLocalRecordMediaID(localMediaID.c_str()))
	{
		//�����������¼�ƣ�������ڻ�����
		if (_terminalList.find(RunningProfile::getInstance()->user_uri()) == _terminalList.end())
		{
			return;
		}
		TerminalVector vec = _terminalList[RunningProfile::getInstance()->user_uri()];
		for (TerminalVector::iterator it = vec.begin(); it != vec.end(); ++it)
		{
			if ((*it)->virtualIndex == virtualIndex)
			{
				QString file = getRecordPath(QString::fromUtf8((*it)->name.c_str()), (*it)->virtualIndex);
				info.record_info.file_name = file;
				break;
			}
		}
		if (info.record_info.file_name.isEmpty())
		{
			return;
		}
	}

	MediaManager::getInstance()->AddRecvMedia( info );

	//Step4
	MediaManager::getInstance()->ChangeMediaState(localMediaID.c_str(), "video", UiMediaState_Ready, true);
	MediaManager::getInstance()->ChangeMediaState(localMediaID.c_str(), "audio", UiMediaState_Ready, true);
	if (vsetting.devicePath == "")
	{
		MediaManager::getInstance()->ChangeMediaState(localMediaID.c_str(), "video", UiMediaState_Stop, true);
	}
}

void ConferenceManager::removeRecvStreamMedia( const std::string username, const std::string remoteMemberURI )
{
    if( !checkIsInConference() )
    {
        qDebug("û�м����κλ���!");
        return;
    }

    TerminalInfo* remoteTerminal = FindTerminal( _terminalList , remoteMemberURI );
	if (NULL == remoteTerminal)
	{
		return;
	}

	if(remoteTerminal->uri == RunningProfile::getInstance()->user_uri())
	{
		removeRecvLocalMedia(getLocalPreviewMediaID(remoteTerminal->virtualIndex).toStdString());
		return;
	}

	//�ж϶Է��Ƿ�ΪС��
	int index, seet;
	bool isSmallVideo = false;
	MediaManager::getInstance()->GetRecvMemberPositionFromURI(QString::fromStdString(remoteMemberURI), 
		&index, &seet, &isSmallVideo);
	/*if(RunningConfig::Instance()->isEnableSmallVideo() && seet > 1)
	{
		isSmallVideo = true;
	}*/

    std::string multiAddr = _currentConfInfo->multicastAddress;
    string localAddress;

	// ֹͣ���յ���
    if( !ConferenceManager::checkSupportMulticast( multiAddr ) )
    {
        //��������鲥����ʹ�������ն˺ŷ���ֹͣ�Է�����ý����������
        localAddress = RunningProfile::getInstance()->ip_address();

		QString szContent = QString("<?xml version=\"1.0\"?><coolview command=\"StopMedia\"><uri>") + 
			QString::fromStdString(remoteTerminal->uri) + "</uri><virtualUri>" + QString::fromStdString(remoteTerminal->virtualUri) + 
			"</virtualUri><ip>" + localAddress.c_str() + "</ip><type>%1</type></coolview>";
		szContent = szContent.arg(isSmallVideo ? "stream_small" : "stream");

        QString remoteUniURI = QString("sip:") + _currentConfInfo->URI.c_str();
        _sipWrapperProxy->makeInfoCall( _currentConfInfo->URI.c_str() , szContent);
    }
	// else �鲥��ʲôҲ����

    //ֹͣ���ս���
    MediaManager::getInstance()->RemoveRecvMedia( remoteTerminal->virtualUri.c_str() );

    //ɾ��λ����Ϣ
    MediaManager::getInstance()->RemoveRecvMemberPosition( remoteTerminal->virtualUri.c_str() );

	//֪ͨ������Ӧ
	streamMediaReceiveStoppedSignal(QString::fromStdString(remoteTerminal->virtualUri));
    
}

void ConferenceManager::removeRecvLocalMedia( const std::string localMediaID )
{
	std::string localAddress = "127.0.0.1";
	std::string baseMediaID;
	int virtualIndex = 0;
	int videoPort = 0;
	int audioPort = 0;

	if (isLocalPreviewMediaID(localMediaID.c_str()))
	{
		virtualIndex = getVirtualIndexFromLocalPreviewMediaID(localMediaID.c_str());
		baseMediaID = LOCAL_PREVIEW_MEDIA_ID;
	}
	else if (isLocalRecordMediaID(localMediaID.c_str()))
	{
		virtualIndex = getVirtualIndexFromLocalRecordMediaID(localMediaID.c_str());
		baseMediaID = LOCAL_RECORD_MEDIA_ID;
	}
	else
	{
		return;
	}

	//��Ϊ���ڱ��ػ��ԡ�����¼�Ƶ�IP��Ϊ127.0.0.1�Ľ��ս��̣�����Ҫ��URI����
	ReceiverInfo * receiver  = _receiverInfo.FindRemoteReceiverByURI(virtualIndex, baseMediaID);
	if (receiver)
	{
		assert(receiver->virtualIndex == virtualIndex);
		//�Ƴ����Ͷ�
		MediaManager::getInstance()->RemoveSendMedia( receiver->virtualIndex, 
			receiver->recvIP.c_str() , 
			receiver->audioPort , 
			receiver->videoPort,
			receiver->smallVideo);
		//ɾ��������Ϣ
		_receiverInfo.RemoveRemoteReceiverByURI(receiver->virtualIndex, baseMediaID);
	}

	//ֹͣ���ս���
	MediaManager::getInstance()->RemoveRecvMedia( localMediaID.c_str() );

	//ɾ��λ����Ϣ
	MediaManager::getInstance()->RemoveRecvMemberPosition( localMediaID.c_str() );
}

void ConferenceManager::changeMemberSeat( const std::string remoteMemberURI , const int screenIndex , const int seat )
{
	std::string uri = remoteMemberURI;
	if (checkIsInConference())
	{
		//���ڻ�����ʱ,���URI�Ƿ��Ǳ��ն˵�URI,������ת��Ϊ���ػ���ID
		TerminalInfo* remoteTerminal = FindTerminal( _terminalList , remoteMemberURI );
		if (remoteTerminal && remoteTerminal->uri == RunningProfile::getInstance()->user_uri())
		{
			uri = getLocalPreviewMediaID(remoteTerminal->virtualIndex).toStdString();
		}
	}
	MediaManager::getInstance()->ChangeMemberSeat(uri.c_str(), screenIndex, seat);
}

void ConferenceManager::removeRecvScreenMedia(  const QString& memberURI, const int screenIndex)
{
    if( !checkIsInConference() )
    {
        qDebug("û�м����κλ���!");
        return;
    }

    TerminalInfo* remoteTerminal = FindTerminal( _terminalList , memberURI.toStdString());
    if( remoteTerminal != NULL )
    {
        std::string multiAddr = remoteTerminal->multicastAddress;
        string localAddress;

		//������Ļ������TCP����˲�֧���鲥...zhhua.sun 2012/02/28

		//����ֹͣ�Է�����ý����������
		localAddress = RunningProfile::getInstance()->ip_address();

		QString szContent = "<?xml version=\"1.0\"?><coolview command=\"StopMedia\"><uri>" +
			QString::fromStdString(remoteTerminal->uri) + "</uri><ip>" + localAddress.c_str() + "</ip><type>screen</type></coolview>";
		QString remoteUniURI = QString("sip:") + _currentConfInfo->URI.c_str();
		_sipWrapperProxy->makeInfoCall( _currentConfInfo->URI.c_str() , szContent);

        //�ͷ���Ļ��
        MediaManager::getInstance()->RemoveRecvScreenMedia( memberURI );

        _snprintf(__globalMessege , sizeof(__globalMessege) , "@Stop Recv Screen From %s,Port:%d \n", PPT_TERMINAL_URI, _currentConfInfo->pptport );
        CV_LOG_DEBUG( __globalMessege );
    }

}

void ConferenceManager::sendTextMessage( const std::string& membername , const std::string& message )
{
    QString cvMessage = "<?xml version=\"1.0\"?><coolview command=\"Text\"><message>" + QString::fromStdString(message) + "</message></coolview>";
    QString username = QString::fromStdString(RunningProfile::getInstance()->username());
    if( membername == "ALL" )
    {
        for( MemberVector::iterator it  = _memberList.begin() ; it != _memberList.end() ; it++ )
        {
            MemberInfo* member = *it;
            QString memberURI = "sip:" + QString::fromStdString( member->MemberURI);
            _sipWrapperProxy->startConference( username, memberURI , cvMessage );
        }
    }
    else
    {
        for( MemberVector::iterator it  = _memberList.begin() ; it != _memberList.end() ; it++ )
        {
            MemberInfo* member = *it;
            if( member->MemberURI.find(membername) != -1 )
            {
                QString memberURI = "sip:" + QString::fromStdString( member->MemberURI);
                _sipWrapperProxy->startConference( username, memberURI, cvMessage);
                break;
            }
        }
    }
}

bool ConferenceManager::getCurrentMulticastAddress( string* ipaddress )
{
    if(!checkIsInConference())
        return false;

    //�ӻ�����Ϣ�ж�
    if( _currentConfInfo && checkSupportMulticast(_currentConfInfo->multicastAddress ) )
    {
        *ipaddress = _currentConfInfo->multicastAddress;
        return true;
    }

    //�ӳ�Ա�б��ж�
    int memberCount = _memberList.size();
    for( MemberVector::iterator it = _memberList.begin() ; it != _memberList.end(); it++ )
    {
        MemberInfo* member = *it;
        if( checkSupportMulticast(member->multicastAddress) )
        {
            *ipaddress = member->multicastAddress;
            return true;
        }
    }

    return false;


}

int ConferenceManager::joinConference( const string& curFocusURI )
{
    //���Ʒ��������������飬��ʼ����һ���»���

    //��鵱ǰ�û��Ƿ��Ѿ�������һ�����飬�򲻴���
    if(checkIsInConference() && _memberList.size() > 0 && _receiverInfo.ReceiverCount() > 0)
    {
        return	-1;
    }
    //���ݼ�¼�Ļ���URI�ҵ�ָ��������Ϣ��������
    ConfInfo* conf = conf_find_by_uri(_confList, curFocusURI);
    if(!conf)
        return -1;

	//������������ǰ�����˵���Ϣ...zhenHua.sun 2011-08-17
	_speakerURI = "";
	_TerminalChairmanURI ="";
	_currentScreenShareUri="";

    _focusURI = curFocusURI;
    _confTitle = conf->Title;
    _confMode = conf->ControlMode;
    _hostURI = conf->HostURI;
    _currentConfInfo = conf;
	_totalTermianlNum=0;

	_currentConfInfo = conf_find_by_uri(_confList, _focusURI);
	//��������ۻ�Ļ��������κ��û�������Ļ����
	if( _confMode == "discuss" )
	{
		MediaManager::getInstance()->ControlScreenShare( true );
	}else
	{
		MediaManager::getInstance()->ControlScreenShare( false );
	}
	

    //�ͷų�Ա��Ϣ
    if(_memberList.size() > 0)
		CVMsgParser::ReleaseMemberList(_memberList);

	for (TerminalMap::iterator it = _terminalList.begin(); it != _terminalList.end(); ++it)
	{
		CVMsgParser::ReleaseTerminalList( it->second );
	}
	_terminalList.clear();

    //������Ա�б�����¼�
    cvMemberListReceivedSignal(_memberList, _confTitle);

    string currentUser = RunningProfile::getInstance()->user_uri();

    //�ı�״̬
    _confState = IsInConference;
    emit cvJoinConferenceSuccessSignal(_focusURI);

    ////����ý�崫�䣬������������
    //if(_confMode == "host" && currentUser != _hostURI)
    //{
    //    MediaManager::getInstance()->AddSendMedia(DEFAULT_IP_ADDRESS , 0 ,
    //            RunningConfig::getInstance()->VideoCodec() == "" ? 0 : DEFAULT_VIDEO_PORT
    //                                             );
    //}
    //else
    //{
    //    MediaManager::getInstance()->AddSendMedia(DEFAULT_IP_ADDRESS ,
    //            RunningConfig::getInstance()->AudioCodec() == "" ? 0 : DEFAULT_AUDIO_PORT,
    //            RunningConfig::getInstance()->VideoCodec() == "" ? 0 : DEFAULT_VIDEO_PORT
    //                                             );
    //}


    //For Testing:��Qos�����������½��Ϣ�����͵�ģ����
    //sendQosJoinConfReport(curFocusURI);

    //������ʱ������ʱ��������������������Ϣ
    _onlineInfoTimer->start(5000);
	//����������ʱ��ʱ��
	_serverHeartbeatTimer->start(180000);

    //��¼��ǰ�Ļ�����Ϣ������QoS��Ϣ�ķ���..zhenHua.sun 20110315
    CVIniConfig::getInstance().setCurrentConferenceUri(_currentConfInfo->URI.c_str());
    CVIniConfig::getInstance().setCurrentConferenceCid(atoi(_currentConfInfo->CID.c_str()));

    return 0;
}
/**
 * @brief �ܾ�����ָ�����飬ͨ������QoS����
 */
int ConferenceManager::rejectToJoinConference( const string& curFocusURI, const string& reason )
{
    if (_confState != WaitingPermission)
    {
        //û��������룬������ܾ���Ϣ
        return -1;
    }
    //����

    //֪ͨ�ϲ㱻�ܾ��������
    ConfInfo* conf = conf_find_by_uri( _confList , curFocusURI );
    if( conf == NULL )
        return -2;
    
    cvJoinRejectionSignal( curFocusURI , reason );

    //�ı�״̬
    _confState = JoinRejected;
    return 0;
}

void ConferenceManager::sendRTPStatToQoSServer( const RtpStatItem& rtpstat )
{
    if( !checkIsInConference() )
        return;
    //���rtpstat�Ƿ�Ϸ�
    if(rtpstat.local_addr_len == 0 ||  rtpstat.rem_addr_len == 0)
        return;

    string cid = _currentConfInfo != NULL ? _currentConfInfo->CID : "0";
    RunningProfile* profile = RunningProfile::getInstance();
    string reporterUri = profile->user_uri();
    string reporterIP = profile->ip_address();
    string senderUri = "";
    string senderIP = "";
    int senderPort = 0;
    string receiverUri = "";
    string receiverIP = "";
    int receiverPort = 0;
    int bandwidth = 0;
    if(rtpstat.rtp_endpoint_type == eET_Sender)  //�����߱���
    {
        senderUri = reporterUri;
        senderIP = reporterIP;
        senderPort = rtpstat.local_port;
        receiverIP = rtpstat.rem_addr;
        receiverPort = rtpstat.rem_port;
		for (int i = 0; i < RunningConfig::Instance()->VideoCaptureDeviceCount(); ++i)
		{
			ReceiverInfo * recvInfo = _receiverInfo.FindRemoteReceiverByIP(i, rtpstat.rem_addr);
			if (recvInfo)
			{
				receiverUri = recvInfo->MemberURI;
				break;
			}
		}
        bandwidth = rtpstat.send_bandwidth;
    }
    else    //�����߱���
    {
        senderIP = rtpstat.rem_addr;
        senderPort = rtpstat.rem_port;
        //���ݶ˿ڲ��ҷ��Ͷ˵�uri
        TerminalInfo * info = terminal_find_by_port(_terminalList, rtpstat.local_port, (int)rtpstat.media_type);
        if (info)
            senderUri  = info->uri;
        receiverUri = reporterUri;
        receiverIP = reporterIP;
        receiverPort = rtpstat.local_port;
        bandwidth = rtpstat.recv_bandwidth;
    }
    string flowClass;
    if(rtpstat.media_type == eMT_Video)
        flowClass = "video";
    else if(rtpstat.media_type == eMT_Audio)
        flowClass = "audio";
    else
        flowClass = "app";

    char msg[1400];
    memset(msg, 0, sizeof(msg));
    const char *fmtQosReport = "<?xml version=\"1.0\"?>"
                               "<coolview command=\"qos\" type=\"clientQosReport\">"
                               "<cid>%s</cid>"
                               "<messageSenderIP>%s</messageSenderIP>"
                               "<messageSenderUri>%s</messageSenderUri>"
                               "<seq>%d</seq>"
                               "<endpointType>%s</endpointType>"
                               "<senderIP>%s</senderIP>"
                               "<senderUri>%s</senderUri>"
                               "<senderPort>%d</senderPort>"
                               "<receiverIP>%s</receiverIP>"
                               "<receiverUri>%s</receiverUri>"
                               "<receiverPort>%d</receiverPort>"
                               "<flowClass>%s</flowClass>"
                               "<bandwidth>%d</bandwidth>"
                               "<lossRate>%.2f</lossRate>"
                               "<delay>%d</delay>"
                               "<jitter>%d</jitter>"
                               "<interval>%d</interval>"
                               "<timestamp>%I64d</timestamp>"
                               "</coolview>";
    int msgLenth =	sprintf(msg, fmtQosReport,
                            cid.c_str(),
                            reporterIP.c_str(),
                            reporterUri.c_str(),
                            rtpstat.seqnumber,
                            rtpstat.rtp_endpoint_type == eET_Sender ? "sender" : "receiver",
                            senderIP.c_str(),
                            senderUri.c_str(),
                            senderPort,
                            receiverIP.c_str(),
                            receiverUri.c_str(),
                            receiverPort,                          
                            flowClass.c_str(),
                            bandwidth,
                            rtpstat.lost,
                            rtpstat.delay,
                            rtpstat.jitter,
                            rtpstat.interval,
                            rtpstat.timestamp);

    //ȡ��sip���ͱ��棬ʹ��UDP��ʽ����
    //_sipWrapperProxy->sendQosPara( QString::fromStdString(profile->getUserName()) ,getUniSipURI(_qosServerUri.c_str()) ,  msg );

    sendUdpMessageToQosServer(msg, msgLenth);

	receiveRtpStatSignal(rtpstat);

}

void ConferenceManager::sendRecStatToServer(const RecStatItem& recstat)
{
	if (recstat.statType == REC_STAT_FILE_CLOSE)
	{
		QString strFileName = QString::fromLocal8Bit(recstat.rec.file);

		QByteArray data;
		QDataStream out( &data , QIODevice::WriteOnly );
		out << QString(strFileName);

		_txMonitor->ReportRecStat(RecStat_FileClose, data);
	}

	receiveRecStatSignal(recstat);
}

void ConferenceManager::recvStartMediaReply( CVMsgParser& cvMessageParser , const std::string& fromUri )
{
    if( !checkIsInConference() )
    {
        qDebug("û�м����κλ���!");
        return;
    }

    //MemberInfo * remoteMember = member_find_by_uri( _memberList, cvMessageParser.GetMemberUri() );
    //if( remoteMember==NULL )
    //{
    //	return;
    //}

    QString remoteURI = cvMessageParser.GetMemberUri().c_str();
    QString type = cvMessageParser.GetType().c_str();
	QString remoteVirtualURI = cvMessageParser.GetVirtualUri().c_str();

    //��ȡ��������ֵ
    string permission = cvMessageParser.GetPermission();
    if( permission != "true" )
	{
		//TODO:����Ϊ����״̬
		MediaManager::getInstance()->ChangeMediaState(remoteVirtualURI , "video" , UiMediaState_Ready,true); //�ȹرյȴ���ʶ
		MediaManager::getInstance()->ChangeMediaState(remoteVirtualURI , "video" , UiMediaState_Stop,true);
		MediaManager::getInstance()->ChangeMediaState(remoteVirtualURI , "audio" , UiMediaState_Stop,true );
        return;
	}

	if (remoteVirtualURI.isEmpty()) // ���ݾɰ�û��virtualUri�����
	{
		remoteVirtualURI = "v1#" + remoteURI;
	}

    //����ý������״̬
    if( type == "screen" )
    {
        MediaManager::getInstance()->ChangeMediaState(remoteVirtualURI , type , UiMediaState_Ready,true);
    }
    if( type == "stream" || type=="smallStream" )
    {
        MediaManager::getInstance()->ChangeMediaState(remoteVirtualURI , "video" , UiMediaState_Ready,true);
		MediaManager::getInstance()->ChangeMediaState(remoteVirtualURI , "audio" , UiMediaState_Ready,true );

		//֪ͨ����Է���ʼ��������
		streamMediaReceiveStartedSignal(QString::fromStdString(remoteVirtualURI.toStdString()));
    }

    //std::string multiAddr = remoteMember->multicastAddress;
    //string localAddress;
    //if( ConferenceManager::checkSupportMulticast( multiAddr ) )
    //{
    //	getCurrentMulticastAddress(&localAddress);
    //}else
    //{
    //	localAddress = RunningProfile::getInstance()->getIpAddress();
    //}

    //int videoWidth;
    //int videoHeight;
    //char audioCodec[20] = {0};
    //getMediaParamFromSdp( &videoWidth , &videoHeight ,audioCodec, remoteMember->Sdp.c_str() );
    //std::string sAudioCodec( audioCodec );
    //int remoteAudioPort = sAudioCodec.size()>0 ? remoteMember->audioPort : 0;
    //int remoteVideoPort = (videoHeight==0||videoWidth==0) ? 0 : remoteMember->videoPort;

    ////��ȡԶ���û�������
    //QString remoteUsername = QString::fromStdString(remoteMember->MemberURI);
    //int index = remoteUsername.indexOf( "@" );
    //if( index>0 )
    //	remoteUsername = remoteUsername.left(index );
    //RecvGraphInfo info;
    //info.initial( localAddress.c_str(), remoteAudioPort  , "SPEEX" , 16000, 16 , 1 , localAddress.c_str() ,
    //	remoteVideoPort , 0 , videoWidth , videoHeight , qPrintable(remoteUsername));
    //MediaManager::getInstance()->AddRecvMedia( info );
}

void ConferenceManager::recvStartSendMediaCommand( CVMsgParser& cvMessageParser , const std::string& fromUri )
{
	string remoteURI = cvMessageParser.GetMemberUri(); // �Է�URI
	string myVirtualURI = cvMessageParser.GetVirtualUri(); // ���������ն˺�
	string type = cvMessageParser.GetType();

	if (myVirtualURI.empty()) // ���ݷǶ�·֧�ֵľɰ�
	{
		myVirtualURI = "v1#" + RunningProfile::getInstance()->user_uri();
	}

	TerminalInfo* currentTerminal = FindTerminal( _terminalList , myVirtualURI );

    string permission = "false";
	if( type=="" || type=="stream" || type=="smallStream" )
	{
		//���type�Ƿ��ܱ�����
		if (currentTerminal && 
			RunningConfig::Instance()->VideoCaptureDevice(currentTerminal->virtualIndex).devicePath != "")
		{
			permission = "true";
		}
		if (type=="smallStream" && !RunningConfig::Instance()->isEnableSmallVideo())
		{
			//���ն˲�֧��С��ʱ
			permission = "false";
		}
	}
	else if (type=="screen")
	{
		permission = "true";
	}

    if( remoteURI.size() > 0 )
    {
        QString szContent = "<?xml version=\"1.0\"?><coolview command=\"StartMediaRelayReply\"><uri>" +
                            QString::fromStdString(remoteURI) + "</uri><virtualUri>" + 
							QString::fromStdString(myVirtualURI) + "</virtualUri><permission>" +
                            QString::fromStdString(permission) + "</permission><type>" +
                            QString::fromStdString(type) + "</type></coolview>";
        _sipWrapperProxy->makeInfoCall( _currentConfInfo->URI.c_str() , szContent );
    }

    if( permission == "false" )
    {
		return;
	}

    //��ȡ�Է���ַ������/�鲥IP��ַ
    string remoteIP = cvMessageParser.GetReceiverIP();

    if( type == "" || type == "stream" || type=="smallStream")
    {
        int videoPort = 0, audioPort = 0;

        //�ж��Ƿ��鲥���� , ��������Ŀ��˿��ǶԷ��ģ��鲥����Ŀ��˿����Լ���
        if(checkCurrentIsMulticastConference())
        {
            // �鲥�����н�����鼴�Ѿ��ڷ���ý��������˲������κβ���
            return;
        }

        // ��������ʹ�öԷ�IP��ַ�����͵��Լ��Ķ˿�
        if (!currentTerminal)
            return;

		audioPort = RunningConfig::Instance()->AudioCodec() == "" ? 0 : currentTerminal->audioPort;

		bool enableSmallStream = false;
		if( type=="smallStream" )
			enableSmallStream = true; // TODO:������ն˲�֧��С���������¶Է��ղ�������

		{
			VCapSetting setting = RunningConfig::Instance()->VideoCaptureDevice(currentTerminal->virtualIndex);
			if( setting.videoCodec == "")
			{
				videoPort = 0;
			}else
			{
				videoPort = enableSmallStream ? currentTerminal->smallVideoPort : currentTerminal->videoPort;
			}
		}

		MediaManager::getInstance()->AddSendMedia( currentTerminal->virtualIndex, remoteIP.c_str(), audioPort , videoPort, enableSmallStream,  0);

        //��¼�Է���Ϣ���Ա�ɾ��
		_receiverInfo.AddRemoteReceiver(currentTerminal->virtualIndex, remoteURI, remoteIP, videoPort, audioPort, enableSmallStream);

		_snprintf( __globalMessege , sizeof(__globalMessege) , "@Start Send Media To:%s , IP:%s , videoPort:%d , audioPort:%d, audioEnable:%d" ,
                    remoteURI.c_str() , remoteIP.c_str() , videoPort , audioPort, true );
        CV_LOG_DEBUG(__globalMessege);

			
		/************************************************************************/  
		//�������д����Ѿ��ķϣ�ԭ��ʵ�֡��ڽ��շ�Ҫ�������Ƶ֮ǰ��ѯ�ʷ��ͷ����Ƿ�Ը�ⷢ�͸��Է����Ĺ���   			
		/*TerminalInfo* terminal = FindTerminal( _terminalList , remoteURI );
		if( terminal )
		{
			cvRemoteTerminalAskForMediaSignal( remoteURI , terminal->name);
		}*/
		/************************************************************************/

    }
    else if( type == "screen" )
    {
		//������Ļ������TCP����˲�֧���鲥...zhhua.sun 2012/02/28
		MediaManager::getInstance()->AddSendScreenMediaDst( _currentConfInfo->pptport , remoteIP.c_str() );
			
		_snprintf( __globalMessege , sizeof(__globalMessege) , "@Start Send Media To:%s , IP:%s , pptPort:%d" ,
			remoteURI.c_str() , _currentConfInfo->multicastAddress.c_str() , _currentConfInfo->pptport );
        CV_LOG_DEBUG(__globalMessege);
    }
}

void ConferenceManager::recvStopMediaReply( CVMsgParser& cvMessageParser , const std::string& fromUri )
{
    ////�ر�ý����
    if( !checkIsInConference() )
    {
        qDebug("û�м����κλ���!");
        return;
    }

    //MemberInfo * remoteMember = member_find_by_uri( _memberList, cvMessageParser.GetMemberUri() );
    //if( remoteMember==NULL )
    //{
    //	return;
    //}

    //��ȡ��������ֵ
    string permission = cvMessageParser.GetPermission();
    if( permission != "true" )
        return;

    //////��ȡԶ���û�������
    //QString remoteUsername = remoteMember->MemberURI.c_str();
    //int index = remoteUsername.indexOf( "@" );
    //if( index>0 )
    //	remoteUsername = remoteUsername.left(index );
    //MediaManager::getInstance()->RemoveRecvMedia( remoteUsername );
    //
    ////ɾ��λ����Ϣ
    //MediaManager::getInstance()->RemoveRecvMemberPosition( remoteMember->MemberURI.c_str() );
}

void ConferenceManager::recvStopSendMediaCommand( CVMsgParser& cvMessageParser , const std::string& fromUri )
{
    //�Ƿ�����
    string permission = "true";

    string remoteURI = cvMessageParser.GetMemberUri();
	string myVirtualURI = cvMessageParser.GetVirtualUri();
	string type = cvMessageParser.GetType();

	if (myVirtualURI.empty()) // ���ݷǶ�·֧�ֵľɰ�
	{
		myVirtualURI = "v1#" + RunningProfile::getInstance()->user_uri();
	}

    if( remoteURI.size() > 0 )
    {
        QString szContent = "<?xml version=\"1.0\"?><coolview command=\"StopMediaRelayReply\"><uri>" +
                            QString::fromStdString(remoteURI) + "</uri><permission>" +
                            QString::fromStdString(permission) + "</permission><type>" +
                            QString::fromStdString(type) + "</type></coolview>";
        _sipWrapperProxy->makeInfoCall( _currentConfInfo->URI.c_str() , szContent );
    }

    if( permission == "false" )
    {
		return;
	}

    string remoteIP = cvMessageParser.GetReceiverIP();

    if( type == "" || type == "stream" || type == "stream_small")
    {
        //�鲥���鲻ֹͣ����
        if(checkCurrentIsMulticastConference())
			return;

		// ��������ʹ�öԷ�IP��ַ��ֹͣ���͵��Լ��Ķ˿�
		TerminalInfo* currentTerminal = FindTerminal( _terminalList , myVirtualURI );
		if (!currentTerminal)
			return;

		ReceiverInfo * receiver = _receiverInfo.FindRemoteReceiverByIP(currentTerminal->virtualIndex, remoteIP);
        if (receiver)
        {
            _snprintf( __globalMessege , sizeof(__globalMessege) , "@Stop Send Media To:%s , IP:%s , videoPort:%d , audioPort:%d, smallVideo:%d" ,
                        receiver->MemberURI.c_str() , receiver->recvIP.c_str() , receiver->videoPort , receiver->audioPort, receiver->smallVideo );
            CV_LOG_DEBUG(__globalMessege);
			assert(receiver->smallVideo == (type == "stream_small"));

            MediaManager::getInstance()->RemoveSendMedia( currentTerminal->virtualIndex, receiver->recvIP.c_str() ,
                    receiver->audioPort ,
                    receiver->videoPort,
					receiver->smallVideo);
            //ɾ��������Ϣ
			_receiverInfo.RemoveRemoteReceiverByIP(currentTerminal->virtualIndex, remoteIP);
        }
    }
    else if( type == "screen" )
	{
		//������Ļ������TCP����˲�֧���鲥...zhhua.sun 2012/02/28
		MediaManager::getInstance()->RemoveSendScreenMediaDst( _currentConfInfo->pptport , remoteIP.c_str() );
    }
}

// �Ѳ�ʹ�� - Liaokz
void ConferenceManager::_SipAccountLogout( const std::string& sipURI )
{
    //���ҳ�Ա�б�
    for( int i = 0 ; i < _memberList.size() ; i++ )
    {
        MemberInfo* member = _memberList.at(i);
        if( sipURI == member->MemberURI )
        {
            member->status = "Not Available";
			//�������ϯ�û��˺��˳��ˣ�Ҫ������ϯ�ն��˺�
			if(sipURI == _currentConfInfo->HostURI)
			{			
				TerminalInfo* remoteTerminal = FindTerminal( _terminalList ,_TerminalChairmanURI );
				if(remoteTerminal!= NULL)
				{
					remoteTerminal->isChairmanTerminal="false";
					if(_confMode == "report")
					remoteTerminal->isSpeaking="false";
					cvUpgradeTerminalInfoSignal(*remoteTerminal);
				}
				_TerminalChairmanURI ="";
			}
			
            return;
        }
    }

    //�����ն��б�
    if (_terminalList.find(sipURI) != _terminalList.end())
    {
		TerminalVector & virtualList = _terminalList[sipURI];
		for (TerminalVector::iterator it = virtualList.begin(); it != virtualList.end(); ++it)
		{
            TerminalInfo* terminal = *it;

            terminal->status = "Not Available";
			if(sipURI == _TerminalChairmanURI)
			{
				
				TerminalInfo* remoteTerminal = FindTerminal( _terminalList ,_TerminalChairmanURI );
				if(remoteTerminal!= NULL)
				{
					remoteTerminal->isChairmanTerminal="false";
					if(_confMode == "report")
						remoteTerminal->isSpeaking="false";
					cvUpgradeTerminalInfoSignal(*remoteTerminal);
				}
				_TerminalChairmanURI ="";
			}
		}
    }
}

int ConferenceManager::computeRequiredBandwidth()
{
    //���ݵ�ǰ��Ƶ���ü�������Ҫ�Ĵ������ڲο���������ɢֵ�����ȡ�ֱ��ʺ�֡�ʲ�С�ڵ�ǰ�ֱ��ʺ�֡�ʵ���С����
    static int resolutionTable[] = {1279 * 719, 1280 * 720, 1280 * 720, 1280 * 720, 1280 * 720, 1920 * 1080};
    static int fpsTable[] = {50, 10, 15, 30, 50, 25};
    static int bandwidthTable[] = {1024, 1.5 * 1024, 2.5 * 1024, 4 * 1024, 6 * 1024};

    int requiredBandwidth = 0;
    int tableSize =  sizeof(bandwidthTable) / sizeof(bandwidthTable[0]);
    //int resolution = RunningConfig::Instance()->VideoHeight() * RunningConfig::Instance()->VideoWidth(); //�Ѳ����ʺ϶�·�ɼ�
	int resolution = 1920 * 1080; // TODO:��ʱʹ��֧�ֵ����ֱ���
    int fps = 30;//RunningConfig::Instance()->VideoFps(); // TODO:��ʱʹ��֧�ֵ����ֱ���

    for(int i = 0; i < tableSize; i++)
        if(resolutionTable[i] >= resolution && fpsTable[i] >= fps)
        {
            requiredBandwidth = bandwidthTable[i];
            break;
        };
    if(requiredBandwidth == 0)
        requiredBandwidth = bandwidthTable[tableSize - 1];
    return requiredBandwidth;

}

void ConferenceManager::saveQoSServerInfo( const MsgQoSServerInfo & serverInfo )
{
    //����qos server sip uri
    if (!serverInfo.sipUri.empty())
    {
        _qosServerUri = serverInfo.sipUri;
        CVIniConfig &inst = CVIniConfig::getInstance();
        inst.setQosServerHostIP(serverInfo.ip.c_str());
        inst.setQosServerSipUri(serverInfo.sipUri.c_str());
        inst.setQosServerTcpPort(serverInfo.tcpPort);
        inst.setQosServerUdpPort(serverInfo.operationUdpPort);
        inst.saveConfig();

        //CVIniConfig::getInstance().setQosServerSipUri(_qosServerUri.c_str());
        RunningConfig::Instance()->setQosServerURI(QString::fromStdString(_qosServerUri));
    }

    //����һ��xml��Ϣ������������̣�֪ͨ���޸�QoS��������Ϣ
    char content[500];
    memset(content, 0, sizeof(content));
    const char* formatBuffer = "<?xml version=\"1.0\"?>"
                               "<coolview command=\"qos\" type=\"QoSServerInfo\">"
                               "<ip>%s</ip>"
                               "<simulatetestTCPPort>%d</simulatetestTCPPort>"
                               "<simulatetestUDPPort>%d</simulatetestUDPPort>"
                               "</coolview>";
    sprintf(content,
            formatBuffer,
            serverInfo.ip.c_str(),
            serverInfo.tcpPort,
            serverInfo.simulateTestUdpPort);

    sendMessageToNetworkMeasurer(content, strlen(content));
}

void ConferenceManager::onlineInfoSlot()
{
    char* szContent = "<?xml version=\"1.0\"?><coolview command=\"Online\"><type>heartbeat</type></coolview>";
    if( checkIsInConference() )
    {
        makeInfoCall( RunningProfile::getInstance()->current_conference_uri() , szContent );
    }
}

/**
 * @brief ��֪���вλ��ն˵�ý�巢��״̬�������ж�ý�������п���
 * @param type ý�����ͣ�0Ϊ��Ƶ��1Ϊ��Ƶ
 * @param enable �������ý��������Ϊtrue������Ϊfalse
 */
void ConferenceManager::controlSendMedia( const int&type , const bool enable )
{
    QString sEnable;
    if( enable )
        sEnable = "true";
    else
        sEnable = "false";

		if( type == 0 )
		{
			
			MediaManager::getInstance()->ControlAudioSend( enable );
			QString szContent = "<?xml version=\"1.0\"?><coolview command=\"SendMediaControl\"><uri>" +
				QString::fromStdString(RunningProfile::getInstance()->user_uri()) + "</uri><type>audio</type><enable>" + sEnable + "</enable></coolview>";
			_sipWrapperProxy->makeInfoCall( _currentConfInfo->URI.c_str() , szContent );
			//if(RunningProfile:: getInstance()->getUserURI() == _TerminalChairmanURI)
			//{

			//}
			//else
			//{
			//	cvModifyStopMyAudioState(false);
			//}
		}
		else
		{
			
			MediaManager::getInstance()->ControlVideoSend(enable);
			QString szContent = "<?xml version=\"1.0\"?><coolview command=\"SendMediaControl\"><uri>" +
								QString::fromStdString(RunningProfile::getInstance()->user_uri()) + "</uri><type>video</type><enable>" + sEnable + "</enable></coolview>";
			_sipWrapperProxy->makeInfoCall( _currentConfInfo->URI.c_str() , szContent );
		}


}

void ConferenceManager::recvSendMediaControlCommand( CVMsgParser& cvMessageParser , const std::string& fromUri )
{
    string uri = cvMessageParser.GetMemberUri();
    string mediaType = cvMessageParser.GetType();
    string enable  = cvMessageParser.GetEnable();

    if( uri.size() > 0 && mediaType.size() > 0 && enable.size() > 0 )
    {
        //if( enable=="true" )
        //	cvRTSendMediaStateChanged( uri , mediaType , true );
        //else
        //	cvRTSendMediaStateChanged(uri , mediaType , false );
        if( enable == "true" )
            MediaManager::getInstance()->ChangeMediaState( uri.c_str() , mediaType.c_str() , UiMediaState_Run,true);
        else
            MediaManager::getInstance()->ChangeMediaState( uri.c_str() , mediaType.c_str(), UiMediaState_Stop,true);
    }
}

void ConferenceManager::createSendScreenMedia( const QRect wnd )
{
    if( this->checkIsInConference())
    {
        MediaManager::getInstance()->CreateSendScreenMedia( _currentConfInfo->pptport , _currentConfInfo->multicastAddress.c_str(), wnd );
        _snprintf( __globalMessege , sizeof(__globalMessege ), "Screen Send Media:%d , %s,%x", _currentConfInfo->pptport , _currentConfInfo->multicastAddress.c_str() , wnd );
        CV_LOG_DEBUG(__globalMessege );
    }
}

void ConferenceManager::createRecvScreenMedia( const QString& memberURI, const int screenIndex  )
{
    //���ڷ���PPT���ն��޷�������Ļ��
    if( this->checkIsInConference() )
    {
        string localAddress;
        TerminalInfo* remoteTerminal = FindTerminal( _terminalList , memberURI.toStdString() );
        if( remoteTerminal != NULL )
        {
            if( ConferenceManager::checkSupportMulticast( _currentConfInfo->multicastAddress ) )
            {
                localAddress = _currentConfInfo->multicastAddress;
            }
            else
            {
                localAddress = RunningProfile::getInstance()->ip_address();

                //��������鲥���飬��ô��Ҫ����Sip��ϢҪ��Է��û�����ý����

                QString szContent = "<?xml version=\"1.0\"?><coolview command=\"StartMedia\"><uri>" +
                                    QString::fromStdString(remoteTerminal->uri) + "</uri><ip>" + localAddress.c_str() + "</ip><type>screen</type></coolview>";
                QString remoteUniURI = QString("sip:") + _currentConfInfo->URI.c_str();
                _sipWrapperProxy->makeInfoCall( _currentConfInfo->URI.c_str() , szContent );
            }

            MediaManager::getInstance()->CreateRecvScreenMedia( memberURI, _currentConfInfo->pptport , localAddress.c_str(), 0 , screenIndex );
        }

    }
}

void ConferenceManager::sendUdpMessageToQosServer( char * msg, int msgLenth )
{
    if(!_qosReportSender.isConnected())
    {
        CVIniConfig &inst = CVIniConfig::getInstance();
        if(!_qosReportSender.connect(inst.getQosServerHostIP(), inst.getQosServerUdpPort()))
        {
            return;
        }
    }
    _qosReportSender.send(msg, msgLenth);
}

///���յ�ѡ�������˵���Ϣ
void ConferenceManager::recvPermitSpeakInfo( CVMsgParser& cvMessageParser , const std::string& fromUri )
{
	
	string uri = cvMessageParser.GetTerminalUri();
	
	if (_speakerURI==uri)
	{
		return;
	}

	TerminalInfo* remoteTerminal = FindTerminal( _terminalList , uri );
	if(remoteTerminal!= NULL)
	{
		remoteTerminal->isSpeaking="true";
		remoteTerminal->isMainSpeaker="true";
	}
	if( uri==RunningProfile::getInstance()->user_uri() )
	{
		MediaManager::getInstance()->ControlAudioSend( true );
		MediaManager::getInstance()->ControlScreenShare(true);
	} 
	else
	{
		
		MediaManager::getInstance()->ChangeMediaState(uri.c_str(),"audio",UiMediaState_Run,true);
	}
	cvSetMemberWindowToFirstPositionSignal(QString ::fromStdString(uri));
	if(remoteTerminal!= NULL)
	{
		cvUpgradeTerminalInfoSignal(*remoteTerminal);
	}
	forbidSpeakByURI(_speakerURI);
	_speakerURI=uri;
	

}

///���յ�ȡ�������˵���Ϣ
void ConferenceManager::recvForbidSpeakInfo( CVMsgParser& cvMessageParser, const std::string& fromUri )
{
	string uri = cvMessageParser.GetTerminalUri();
	TerminalInfo* remoteTerminal = FindTerminal( _terminalList , uri );
	if(remoteTerminal!= NULL)
	{
		remoteTerminal->isMainSpeaker="false";
		remoteTerminal->isHandUp="false";
	}

	if(_confMode=="report")
	{
		if( uri==RunningProfile::getInstance()->user_uri() )
		{
			MediaManager::getInstance()->ControlScreenShare(false);
		}

		if(remoteTerminal!= NULL)
		{
			remoteTerminal->isSpeaking="false";
		}
		if( uri==RunningProfile::getInstance()->user_uri() )
		{
			MediaManager::getInstance()->ControlAudioSend( false );
		}
		else
		{
			
			MediaManager::getInstance()->ChangeMediaState(uri.c_str(),"audio",UiMediaState_Stop,true);
		}
	}
	
	if(remoteTerminal!= NULL)
	{
		cvUpgradeTerminalInfoSignal(*remoteTerminal);
	}
}

void ConferenceManager:: forbidSpeakByURI(const std::string& Uri)
{
	string uri = Uri;

	TerminalInfo* remoteTerminal = FindTerminal( _terminalList , uri );
	if(remoteTerminal!= NULL)
	{
		remoteTerminal->isMainSpeaker="false";
		remoteTerminal->isHandUp="false";
	}
	if( uri==RunningProfile::getInstance()->user_uri() )
	{
		MediaManager::getInstance()->ControlScreenShare(false);
	}
	if(_confMode=="report")
	{

		if(remoteTerminal!= NULL)
		{
			remoteTerminal->isSpeaking="false";
		}
		if( uri==RunningProfile::getInstance()->user_uri() )
		{
			MediaManager::getInstance()->ControlAudioSend( false );
		}
		else
		{

			MediaManager::getInstance()->ChangeMediaState(uri.c_str(),"audio",UiMediaState_Stop,true);
		}
	}

	if(remoteTerminal!= NULL)
	{
		cvUpgradeTerminalInfoSignal(*remoteTerminal);
	}
	
}

void ConferenceManager::queryMainSpeaker()
{
	QString szContent = "<?xml version=\"1.0\"?><coolview command=\"GetSpeakerTerminal\"></coolview>";
	_sipWrapperProxy->makeInfoCall(_currentConfInfo->URI.c_str(),szContent);
}


void ConferenceManager::queryChairmanTermianl()
{
	QString szContent = "<?xml version=\"1.0\"?><coolview command=\"GetChairmanTerminal\"></coolview>";
	_sipWrapperProxy->makeInfoCall(_currentConfInfo->URI.c_str(),szContent);
}

void ConferenceManager::queryCurrentScreenShareTermianl()
{
	QString szContent = "<?xml version=\"1.0\"?><coolview command=\"GetScreenShareTerminal\"></coolview>";
	_sipWrapperProxy->makeInfoCall(_currentConfInfo->URI.c_str(),szContent);
}


void ConferenceManager:: HandUpHandler(const bool handup)
{
	if(handup)
	{
		QString szContent = "<?xml version=\"1.0\"?><coolview command=\"HandUp\"><terminalUri>"+ QString::fromStdString(RunningProfile::getInstance()->user_uri())+"</terminalUri></coolview>";
		_sipWrapperProxy->makeInfoCall(_currentConfInfo->URI.c_str(),szContent);
	}
	else
	{
		QString szContent = "<?xml version=\"1.0\"?><coolview command=\"HandDown\"><terminalUri>"+ QString::fromStdString(RunningProfile::getInstance()->user_uri())+"</terminalUri></coolview>";
		_sipWrapperProxy->makeInfoCall(_currentConfInfo->URI.c_str(),szContent);
	}
}

string ConferenceManager:: getConfMode()
{

	return _confMode;
}

void ConferenceManager:: AllowSpeakHandler(QString uri)
{
	QString szContent = "<?xml version=\"1.0\"?><coolview command=\"SetSpeaker\"><terminalUri>"+uri+"</terminalUri></coolview>";
	_sipWrapperProxy->makeInfoCall(_currentConfInfo->URI.c_str(),szContent);
}

void ConferenceManager:: ForbidSpeakHandler(QString uri)
{
	QString szContent = "<?xml version=\"1.0\"?><coolview command=\"ForbidSpeaker\"><terminalUri>"+uri+"</terminalUri></coolview>";
	_sipWrapperProxy->makeInfoCall(_currentConfInfo->URI.c_str(),szContent);
}

void ConferenceManager:: TerminalLogin(const std::string& username , const std::string& sipServer,QString ipv4 ,QString ipv6,QString myVersion,int screenNum)
{
	QString myIp= QString::fromStdString(RunningProfile::getInstance()->ip_address());
	QString uri=QString::fromStdString(RunningProfile::getInstance()->user_uri());
	QString cvMessage = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><coolview command=\"TerminalLogin\"><ip>"+myIp+"</ip><ipv4>"
		+ipv4+"</ipv4><ipv6>"+ipv6+"</ipv6><version>"+myVersion+"</version><screenCount>"+QString::number(screenNum)+"</screenCount></coolview>";
	_sipWrapperProxy->startConference( QString::fromStdString(username) , QString::fromStdString(sipServer), cvMessage );
}


void ConferenceManager:: recvSaviInfo( CVMsgParser& cvMessageParser, const std::string& fromUri )
{
	string res = cvMessageParser.GetResult();
		if(res== "true")
			RecvSaviSignal(true);
		else
			RecvSaviSignal(false);

	startConference( RunningProfile::getInstance()->username() , 
			RunningConfig::Instance()->GlobalService().toStdString());

}


void ConferenceManager:: recvChairmanTerminalHandler( CVMsgParser& cvMessageParser, const std::string& fromUri)
{

		string uri = cvMessageParser.GetTerminalUri();

		if(_TerminalChairmanURI == uri)
			return;

		TerminalInfo* remoteTerminal = FindTerminal( _terminalList , uri );
		if(remoteTerminal!= NULL)
		{
			remoteTerminal->isChairmanTerminal="true";
			remoteTerminal->isSpeaking="true";
			cvUpgradeTerminalInfoSignal(*remoteTerminal);
		}

		TerminalInfo* oldChairmanTerminal = FindTerminal( _terminalList , _TerminalChairmanURI );
		if(oldChairmanTerminal!= NULL)
		{
			oldChairmanTerminal->isChairmanTerminal="false";
			if(_confMode == "report")
			oldChairmanTerminal->isSpeaking="false";
			cvUpgradeTerminalInfoSignal(*oldChairmanTerminal);
		}

		_TerminalChairmanURI=uri;

}

void ConferenceManager::recvMediaControlInfo( CVMsgParser& cvMessageParser, const std::string& fromUri)
{
	string uri = cvMessageParser.GetTerminalUri();
	string type =cvMessageParser.GetType();
	string enable =cvMessageParser.GetEnable();


	if(enable =="false")
	{
		if(type =="audio")
		{
			TerminalInfo* remoteTerminal = FindTerminal( _terminalList ,uri );
			if(remoteTerminal!= NULL)
			{
				remoteTerminal->isSpeaking="false";
				cvUpgradeTerminalInfoSignal(*remoteTerminal);
			}
			if( uri==RunningProfile::getInstance()->user_uri() )
			{
				MediaManager::getInstance()->ControlAudioSend( false );

			}
			else
			{			

				MediaManager::getInstance()->ChangeMediaState(uri.c_str(),"audio",UiMediaState_Stop,true);
			}

		}		

	}
	else
	{
		TerminalInfo* remoteTerminal = FindTerminal( _terminalList , uri );
		if(type =="audio")
		{
			if(remoteTerminal!= NULL)
			{
				remoteTerminal->isSpeaking="true";
				cvUpgradeTerminalInfoSignal(*remoteTerminal);
			}
			if( uri==RunningProfile::getInstance()->user_uri() )
			{
				MediaManager::getInstance()->ControlAudioSend( true );
			} 
			else
			{			

				MediaManager::getInstance()->ChangeMediaState(uri.c_str(),"audio",UiMediaState_Run,true);
			}

		}
	}
	
	

}

//��֪�����ն˴���/���ٵ�ǰ�ն˵���Ļ�����ս���
void ConferenceManager:: ScreenShareControlHandler(const bool enable)
{
	QString Enable="true";
	if(!enable)
	Enable="false";

	QString uri=QString::fromStdString(RunningProfile::getInstance()->user_uri());
	int port =_currentConfInfo->pptport;
	
	string localAddress="";
	TerminalInfo* remoteTerminal = FindTerminal( _terminalList , uri.toStdString() );
	if( remoteTerminal != NULL )
	{
		//������Ļ������TCPʵ�֣���˲���֧���鲥...zhenhua.sun 2012/02/28
		localAddress = RunningProfile::getInstance()->ip_address();
	}

	QString cvMessage = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><coolview command=\"ShareScreenControl\"><terminalUri>"+uri+"</terminalUri><enable>"+
		Enable+"</enable><ip>"+QString::fromStdString(localAddress)+"</ip><port>"+QString::number(port)+"</port></coolview>";
	_sipWrapperProxy->makeInfoCall(_currentConfInfo->URI.c_str(),cvMessage);

	if(!enable)
	{
		MediaManager::getInstance()->RemoveScreenShareSend();
	}
}



void ConferenceManager::ChairmanChangeNotify(bool enable) {
	TelecontrollerIf *teleControlProxy = new TelecontrollerIf( _GetTelecontrollerServiceName("sock"),
		_GetTelecontrollerObjectPath("sock") , QDBusConnection::sessionBus() );;
	
	QString uri= "";
	if(enable)
		uri=QString ::fromStdString(_currentConfInfo->Chairman);

	QByteArray output_array;
	QDataStream out(&output_array , QIODevice::WriteOnly );
	out.setVersion( QDataStream::Qt_4_4 );
	out << TELE_PostMeetingInfo;
	out << uri;
	if( teleControlProxy )
	{
		teleControlProxy->TeleInfo(TELE_PostMeetingInfo,-1,output_array );
	}
}


void ConferenceManager::recvMainSpeakerTermianlInfo( CVMsgParser& cvMessageParser, const std::string& fromUri)
{
	string uri = cvMessageParser.GetTerminalUri();
	_speakerURI=uri;

}

void ConferenceManager::recvChairmanTermianlInfo( CVMsgParser& cvMessageParser, const std::string& fromUri)
{
	string uri = cvMessageParser.GetTerminalUri();
	_TerminalChairmanURI=uri;

}

void ConferenceManager::recvCurrentScreenShareTerminalInfo(CVMsgParser& cvMessageParser, const std::string& fromUri)
{
	string uri = cvMessageParser.GetTerminalUri();
	_currentScreenShareUri=uri;

	if(_currentScreenShareUri != "" && _currentScreenShareUri != RunningProfile::getInstance()->user_uri())
	{
		recvMainSpeakerShareScreen(QString::fromStdString(_currentScreenShareUri));
	}	
}

void ConferenceManager::recvShareScreenControlInfo( CVMsgParser& cvMessageParser, const std::string& fromUri)
{
	
	string uri = cvMessageParser.GetTerminalUri();
	string enable =cvMessageParser.GetEnable();

	if(enable == "true")
	{
		//���ڷ���PPT���ն��޷�������Ļ��
		if( this->checkIsInConference() )
		{
			string localAddress;
			TerminalInfo* remoteTerminal = FindTerminal( _terminalList , uri);
			if( remoteTerminal != NULL )
			{
				//������Ļ������TCP����˲�֧���鲥...zhhua.sun 2012/02/28
				localAddress = RunningProfile::getInstance()->ip_address();

				//�ȴ������ս��̣���Ҫ����Ļ���ͷ����ӵ���ǰ�Ľ��ս���
				MediaManager::getInstance()->CreateMainSpeakerRecvScreenMedia( QString:: fromStdString(uri), _currentConfInfo->pptport , localAddress.c_str(), 0 );

				Sleep(2000);


				////��Sip��ϢҪ��Է��û�����ý����
				QString szContent = "<?xml version=\"1.0\"?><coolview command=\"StartMedia\"><uri>" +
					QString::fromStdString(remoteTerminal->uri) + "</uri><ip>" + localAddress.c_str() + "</ip><type>screen</type></coolview>";
				QString remoteUniURI = QString("sip:") + _currentConfInfo->URI.c_str();
				_sipWrapperProxy->makeInfoCall( _currentConfInfo->URI.c_str() , szContent );

				
			}

		}
	}
	else
	{
		MediaManager::getInstance()->RemoveRecvScreenMedia();
	}
}



void ConferenceManager::recvMainSpeakerShareScreen(QString & Uri)
{

	//���ڷ���PPT���ն��޷�������Ļ��
	if( this->checkIsInConference() )
	{
		string localAddress;
		TerminalInfo* remoteTerminal = FindTerminal( _terminalList , Uri.toStdString());
		if( remoteTerminal != NULL )
		{
			//������Ļ������TCP����˲�֧���鲥...zhhua.sun 2012/02/28
			localAddress = RunningProfile::getInstance()->ip_address();
			
			//�ȴ������ս��̣���Ҫ����Ļ���ͷ����ӵ���ǰ�Ľ��ս���
			MediaManager::getInstance()->CreateMainSpeakerRecvScreenMedia( Uri, _currentConfInfo->pptport , localAddress.c_str(), 0 );

			Sleep(2000);


			////��������鲥���飬��ô��Ҫ����Sip��ϢҪ��Է��û�����ý����
			QString szContent = "<?xml version=\"1.0\"?><coolview command=\"StartMedia\"><uri>" +
				QString::fromStdString(remoteTerminal->uri) + "</uri><ip>" + localAddress.c_str() + "</ip><type>screen</type></coolview>";
			QString remoteUniURI = QString("sip:") + _currentConfInfo->URI.c_str();
			_sipWrapperProxy->makeInfoCall( _currentConfInfo->URI.c_str() , szContent );
		}


	}


}


void ConferenceManager::TerminalLogOutSlot()
{
	string username=RunningProfile::getInstance()->username();
	QString cvMessage = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><coolview command=\"TerminalLogOut\"></coolview>";
	_sipWrapperProxy->startConference( QString::fromStdString(username) , RunningConfig::Instance()->GlobalService(), cvMessage );
}


void ConferenceManager:: GetTermialList()
{
	if (RunningConfig::Instance()->isEnableHttp())
	{
		_httpWrapper->RequestTerminalList(QString::fromStdString(_currentConfInfo->CID));
	}
	else
	{
		QString szContent = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><coolview command=\"GetTerminalList\"></coolview>";
		_sipWrapperProxy->makeInfoCall( _currentConfInfo->URI.c_str() , szContent );
	}
}

void ConferenceManager:: QueryCoferenceInfoBeforeFinishTerminal()
{
	queryMainSpeaker();
	queryChairmanTermianl();
	queryCurrentScreenShareTermianl();
}

void ConferenceManager:: StartServerTsetSlot()
{
	_isServerTest=true;
	
	string username=RunningProfile::getInstance()->username();
	QString cvMessage = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><coolview command=\"Online\"><type>test</type></coolview>";
	_sipWrapperProxy->startConference( QString::fromStdString(username) , RunningConfig::Instance()->GlobalService(), cvMessage );

	_serverTestCount.restart();
	_serverTestTimer->start(30000);

}

void ConferenceManager::recvOnlineMessageInfo(CVMsgParser& cvMessageParser, const std::string& fromUri)
{
	string type = cvMessageParser.GetType();

	//������ֶ������ն�����������ӳ٣�������֧
	if(type == "test")
	{  
		//û�г�ʱ
		if(_isServerTest)
		{
			int intervalTime = _serverTestCount.elapsed();
			_serverTestTimer->stop();
			_isServerTest=false;
			cvServerTestResultSignal(false,intervalTime);
		}
		else
		{
			return;
		}
		

	}
	else
	{
		//һ��������ظ�
		
		QString fromStr = RunningConfig::Instance()->GlobalService();
		int index = fromStr.indexOf( ":" );
		if( index > 0 )
			fromStr = fromStr.right( index );
		//�յ�����GlobalService�������ظ�
		if(fromUri == fromStr.toStdString() )
		{
			_serverHeartbeatTimer->stop();
			//��������������ʱ��ʱ��,����ȴ����ζ�û���յ���������ʾ�ظ���ʱ
			_serverHeartbeatTimer->start(180000);
		}
			
	}
}

void ConferenceManager:: severTestTimeOutSlot()
{
	_serverTestTimer->stop();
	_isServerTest=false;
	cvServerTestResultSignal(true,30000);
}

void ConferenceManager:: serverHeartbeatMessageTimeOutSlot()
{
	_serverHeartbeatTimer->stop();
	
	//��ʱ����
}

void ConferenceManager::RecoveryMediaProcess(QString mediaType, bool isSend, QString removeUserID)
{
	if("stream" != mediaType)
	{
		return;
	}

	if(!isSend)
	{
		bool isMulti =false;  //�Ƿ����鲥
		bool usingSmallVideo = false; //�Ƿ�ʹ��С��
		int seet = -1;
		int screenIndex = -1;
			
		//MediaManager::getInstance()->AddRecvMemberPosition( remoteMemberURI.c_str() , screenIndex , seet );
		MediaManager::getInstance()->GetRecvMemberPositionFromName(removeUserID, &screenIndex, &seet, &usingSmallVideo);
			
		if(seet<0 || screenIndex<0)
			return;
			
		removeUserID =removeUserID+"@sip.ccnl.scut.edu.cn";
		//�����ն˵�ý����
		TerminalInfo* remoteTerminal = FindTerminal( _terminalList , removeUserID.toStdString() );
		if( remoteTerminal == NULL )
		{
			return;
		}

		//��ȡԶ���ն˵�ý�����
		int videoWidth;
		int videoHeight;
		int videoFps;
		char audioCodec[20] = {0};
		getMediaParamFromSdp( &videoWidth , &videoHeight , &videoFps, audioCodec, remoteTerminal->sdp.c_str() );
		std::string sAudioCodec( audioCodec );
		int remoteAudioPort = sAudioCodec.size() > 0 ? remoteTerminal->audioPort : 0;
		int remoteVideoPort;

		//������ǵ�һ�����ڣ���ʹ��С��
		if( RunningConfig::Instance()->isEnableSmallVideo() && 
			seet>1 )
			usingSmallVideo = true;

		std::string multiAddr =_currentConfInfo->multicastAddress;
		string localAddress;
		if( ConferenceManager::checkSupportMulticast( multiAddr ) )
		{
			localAddress = multiAddr;
			isMulti=true;
		}
		else
		{
			localAddress = RunningProfile::getInstance()->ip_address();
		}

		//����ý����
		if( usingSmallVideo) 
		{
			remoteVideoPort = (videoHeight == 0 || videoWidth == 0) ? 0 : remoteTerminal->smallVideoPort;

			//ȫ��������ͷ����Ƶ��������4����460x270�����Ǳ���������֧�ָ���Ƶ��ʽ�����ͳһΪ320x180
			//...zhenhua.sun 2012/2/22
			videoWidth = MSDX_CONF_SMALL_VIDEO_WIDTH;
			videoHeight = MSDX_CONF_SMALL_VIDEO_HEIGHT;
			videoFps /=2;
		}else
		{
			remoteVideoPort = (videoHeight == 0 || videoWidth == 0) ? 0 : remoteTerminal->videoPort;
		}
		//��ȡԶ���û�������
		QString remoteUsername = QString::fromStdString(remoteTerminal->uri);
		int index = remoteUsername.indexOf( "@" );
		if( index > 0 )
			remoteUsername = remoteUsername.left(index );
		RecvGraphInfo info;
		info.initial( qPrintable(RunningConfig::Instance()->AudioCaptureDevice()), qPrintable(RunningConfig::Instance()->AudioOutputDevice()),
			localAddress.c_str(), remoteAudioPort  , "SPEEX" , 16000, 16 , 1 , localAddress.c_str() ,
			remoteVideoPort , 0 , videoWidth , videoHeight , videoFps, qPrintable(remoteUsername), remoteTerminal->name.c_str(),
			//RunningConfig::getInstance()->isEnableRecvAutoResync());
			false );
				
		MediaManager::getInstance()->RecoveryRecvMedia( info );
	}
}

void ConferenceManager::SegmentLocalMedia()
{
	RunningConfig* pConfig = RunningConfig::Instance();
	const int numberOfStreams = pConfig->VideoCaptureDeviceCount();
	for (int i = 0; i < numberOfStreams; i++)
	{
		QString sendID = getSendMediaID(i);
		MediaManager::getInstance()->SegmentMedia(sendID, 0);
	}
}

QString ConferenceManager::getRecordPath(QString terminalName, const int virtualIndex)
{
	// init file path
	if (_recordingPath.size() == 0)
	{
		QString recDir = CTxConfig::getInstance().GetRecPath();
		if (recDir.indexOf(":") == -1)
		{
			QString appDir = QCoreApplication::applicationDirPath();
			appDir.replace("/" , "\\" );
			if (recDir[0] == '\\')
			{
				recDir = appDir + recDir;
			}
			else
			{
				recDir = appDir + "\\" + recDir;
			}
		}
		if (recDir[recDir.size()-1] == '\\')
		{
			recDir = recDir.mid(0, recDir.size() - 1);
		}
		_recordingPath = recDir;
	}

	QString confTimeTitle;
	int pos = _currentConfInfo->StartTime.find(' ');
	if (pos != -1)
	{
		confTimeTitle = QString("%1_%2").arg(QString::fromUtf8(_currentConfInfo->StartTime.substr(0, pos).c_str())).arg(QString::fromUtf8(_confTitle.c_str()));
	}
	else
	{
		confTimeTitle = QString::fromUtf8(_confTitle.c_str());
	}

	QString curTerminalName = "DefaultName";
	TerminalVector currentTerminal = _terminalList[RunningProfile::getInstance()->user_uri()];
	if (currentTerminal.size() > 0)
	{
		curTerminalName = QString::fromUtf8(currentTerminal[0]->name.c_str());
	}
	if (curTerminalName[curTerminalName.length()-1] == ')') // TODO:��·����ͷʱ��ʱȥ�����ն���ĩβ�ı��
	{
		curTerminalName = curTerminalName.mid(0, curTerminalName.length()-3);
	}
	if (terminalName[terminalName.length()-1] == ')') // TODO:��·����ͷʱ��ʱȥ����¼���ն���ĩβ�ı��
	{
		terminalName = terminalName.mid(0, terminalName.length()-3);
	}

	// ·����{¼�ƻ���λ��}/{tx�ն���}/{������}/{����}/{�ն���}/{�ļ���}
	// �ļ�����{�ն���}-{����ʱ��}-{Ƭ�κ�}-{ý�������}-{��Ƶ��ʽ}.mp4
	QString file = QString("%1\\%2\\%3\\%4\\%5\\%5_%6_%7_%8_%9.mp4")
		.arg(_recordingPath).arg(curTerminalName)
		.arg(confTimeTitle).arg(TX_FNFMT_DATE).arg(terminalName)
		.arg(TX_FNFMT_DATETIME).arg(TX_FNFMT_EPISODE).arg(virtualIndex).arg(TX_FNFMT_VINFO);

	return file;
}
