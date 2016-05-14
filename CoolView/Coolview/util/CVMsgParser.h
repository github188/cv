#pragma once

#ifndef CVMsgParser_h
#define CVMsgParser_h

#include <iostream>
//#include <fstream>
//#include <map>
#include <vector>
//#include <list>
#include <QtCore/QtCore>
#include "RemoteReceiver.h"

#include <tinyxml/tinyxml.h>

typedef struct tagConfInfo
{
    std::string CID;         //�����id��ȫ��Ψһ
    std::string URI;
    std::string Title;
    std::string Description;
    std::string HostURI;		//������URI
    std::string ControlMode;	//����ģʽ��discuss��report
    std::string JoinMode;
    std::string JoinPassword;
    std::string StartTime;
    std::string Duration;

	std::string multicastAddress;	//����������鲥���飬����Ϊ�鲥��ַ������Ϊ0.0.0.0  --ע:���ڵ�ǰ���Ʒ����������ն˵Ļ����б�û�иõ�ַ��Ϣ������multicastAddress�ֶο���Ϊ�գ�ʹ��ʱ��ע�⡣

	std::string		level;		//���鼶��
	std::string  Chairman;		//��ϯ�˺�
	int		pptport;		//������Ļ����
}ConfInfo;

typedef struct tagMemberInfo
{
 //   std::string MemberURI;   //��Աuri
 //   std::string Sdp;         //sdp
 //   int isReceived;     //�Ƿ��Ѿ������˸ó�Ա��ý������0δ���գ�1�ѽ���
 //   int permitSpeak;    //�Ƿ������ԣ�0������1����2����������
 //   int hand;           //�Ƿ���֣�0û���֣�1����
 //   bool isMulticast;   //�Ƿ�֧���鲥
	//std::string multicastAddress;		//�鲥�����IP��ַ��������������0.0.0.0
	//int videoPort;		//��Ƶ���ݴ���ı��ض˿ڣ�Ĭ��ֵΪ0
	//int audioPort ;		//��Ƶ���ݴ���ı��ض˿ڣ�Ĭ��ֵΪ0

	//�°汾���������֧�ֵ��ֶ�
	std::string username;		//sipAccount name
	std::string name;			//��Ա����ʵ����
	std::string MemberURI;				//�û�URI
	std::string xmpp;			//IM�˻�
	std::string company;			//��˾����
	std::string sdp;				
	std::string floor;
	std::string multicastAddress;
	int audioPort;
	int videoPort;
	std::string status;
	std::string location;
	std::string AllowTerminal;	//����������նˣ���\t�ָ�����uri
	int	   permitSpeak;
}MemberInfo;

typedef struct tagTerminalInfo
{
	std::string	uri;		//�ն�uri
	std::string  virtualUri; //�ն˴��ڶ�����ͷʱ�������ն˺ţ���ʽ����v1#{uri}
	int     virtualIndex; //���ֱ�ʾ�������ն�����
	int     virtualCount; //���ն��ܵ������ն���
	std::string	name;	//�ն�����
	std::string	dns;		//����������
	std::string	model;	//����ģʽ
	std::string	version;	//�ն˰汾
	std::string	sdp;		//sdp
	bool	isMulticast;   //�Ƿ�֧���鲥
	std::string	multicastAddress;		//�鲥�����IP��ַ��������������0.0.0.0
	int		videoPort;		//��Ƶ���ݴ���ı��ض˿ڣ�Ĭ��ֵΪ0
	int		audioPort ;		//��Ƶ���ݴ���ı��ض˿ڣ�Ĭ��ֵΪ0
	int		smallVideoPort;	//��ƵС���Ĵ���˿ڣ�Ĭ��ֵΪ0
	std::string	status;			//�ն�״̬
	std::string  location;		//�ն�λ��
	std::string	isSpeaking;		//�Ƿ�������
	std::string  isHandUp;       //�Ƿ����
	std::string  isChairmanTerminal;  //�Ƿ�����ϯ�ն�
	std::string  isMainSpeaker;		 //�Ƿ���������
	int     totalTermialNum; //������ն�����

}TerminalInfo;

typedef std::vector<ConfInfo*>		ConferenceVector;
typedef std::vector<MemberInfo*>	MemberVector;
typedef std::vector<TerminalInfo*> TerminalVector;


//������������Ϣ����Ϣ����
struct MsgLoginPermissionInfo
{
    std::string userURI;
    int dscp;
    int rate;
};

//��ֹ���������Ϣ����Ϣ����
struct MsgLoginRejectionInfo
{
    std::string userURI;
    std::string description;
    int suggestedRate;
};

//QoS��������Ϣ
struct MsgQoSServerInfo 
{
	std::string sipUri;	
	std::string ip;
	int tcpPort;
	int simulateTestUdpPort; //Qos����������ģ����Ա���˿�
	int operationUdpPort;		//Qos���������ձ���˿�
};

class CVMsgParser
{
public:
    CVMsgParser(void);
    ~CVMsgParser(void);

	void InitParser(const char* message);

	TiXmlElement* GetCoolViewElement() { return this->_coolviewElement; }

	//��ȡXML��Ϣ���ı�����
	const char * getMessageContent();

    //��sip��Ϣ���ݽ����ɻ����б�
    std::vector<ConfInfo*> ParseConfListMsg(std::vector<ConfInfo*>& conf_list);

	void ParseConfListMsg( QList<QVariantMap>* confList );
	void ParseMemberListMsg(QList<QVariantMap>* memberList,const std::string& hosturi, const std::string& confmode);

    //�������ʱ�������Ƶ���͵ĵ�ַ�Ͷ˿�
    void getAddrPort(int& audio_port, int& video_port, std::string& address);

    ////��sip��Ϣ���ݽ�����Ա�б�������Ҫ�����˼�������
    std::vector<MemberInfo*> ParseMemberListMsg(std::vector<MemberInfo*>& member_list,const std::string& hosturi, const std::string& confmode);
    std::vector<MemberInfo*> ParseMemberListMsg(int& audiao_port, int& video_port, std::string& address);
    std::vector<MemberInfo*> ParseMemberListMsg(int& audio_port, int& video_port, std::string& address, const std::string& hosturi, const std::string& confmode);

	///��sip��Ϣ���ݽ���Ϊ�ն��б�
	TerminalVector ParseTerminalListMsg( TerminalVector& terminalList , const std::string& hosturi , const std::string& confmode );
//************************************
// Method:    recvScreenCancel2ReceiverMsgHandler
// FullName:  ConferenceManager::recvScreenCancel2ReceiverMsgHandler
// Access:    private 
// Returns:   CvSipWrapperIf*
// Qualifier:
//************************************

	//���ݳ�Ա����/�˳�������Ϣ���³�Ա�б�
    std::vector<MemberInfo*> ParseMemberListByNotifyMsg(std::vector<MemberInfo*>& member_list, const std::string& hosturi, const std::string& confmode);

	void ParseMemberInOrOutByMsg( std::string* memberURI ,bool* memberIn , std::vector<MemberInfo*>& member_list, const std::string& hosturi, const std::string& confmode);

    //������Ϣ�����е���������
    const char* GetCommandType();
    //��ջ����б�
	static  void ReleaseConfList(std::vector<ConfInfo*>& conf_list);
    //�����Ա�б�
	static	 void ReleaseMemberList(std::vector<MemberInfo*>& member_list);
	//����ն��б�
	static void ReleaseTerminalList( TerminalVector& terminalList);
 

	//����Confinfo����һ��xml����
    std::string CreateMsgByConfInfo(ConfInfo* conf_info);
    std::string CreateMsgByConfInfo(ConfInfo* conf_info, std::string command);

    //��ȡFocusURI
    std::string GetFocusURI();
	std::string GetMemberUri();
	std::string GetTerminalUri();
	std::string GetVirtualUri();
    std::string GetInviterUri();
    std::string GetLoginMemberUri();
    std::string GetTextMessage();
    std::string GetWhiteBoardMessage();
	std::string GetResult();
	std::string GetTerminalCount();
    //
    std::string GetReceiverIP();

	//�ն�����ģʽ
	std::string GetTerminalConfMode();

	//��ȡ����ֵ
	std::string GetPermission();

	//��ȡ����
	std::string GetType();

	//��ȡ����ֵ
	std::string GetEnable();

    /*
      Add by heixanfu 
         Ϊ�˴�����յ�QOS��Ϣ
      09-9-4
    */
    std::string GetElementValueByName(const std::string fatherElementName, const std::string childElementName);
	
	//��ȡqos�����type����
    std::string GetQosType();

  
    //����LoginPermission sip ��Ϣ������0��ʾ�����ɹ�
    int ParseLoginPermissionMsg(MsgLoginPermissionInfo& info);

    //����һ��LoginRejection  sip ��Ϣ������0��ʾ�����ɹ�
    int ParseLoginRejectionMsg(MsgLoginRejectionInfo& info);

	//����QoS��������Ϣ
	 int ParseQoSServerInfoMsg(MsgQoSServerInfo& info);

	//---------------------------��¼ͨ��-------------------------------
	/**
	 * @brief ��ȡ��¼ͨ�������
	 * @return ���Ϊ0��˵����member��1����terminal
	 */
	int GetLoginNotifyType( );

	///�����û���¼ͨ��,ʧ�ܷ���0
	MemberInfo* ParseMemberLoginNotifyMsg( MemberVector&memberList , std::string& hostUri , std::string& confMode );

	///�����ն˵�¼ͨ��,ʧ�ܷ���0
	TerminalVector &ParseTerminalLoginNotifyMsg( TerminalVector &terminalList,  std::string& hostUri , std::string& confMode );


private:
    //��ȡ����ָ�����Ƶĵ�һ��xmlԪ�ص�ֵ���ַ��������Ҳ�����ӦԪ���򷵻ؿ��ַ���
    static std::string GetFirstChildElementValue(TiXmlElement* parentElement, const char* childElementName);
	
	//����������Ա��Ϣ����xml�����е�<member></member>Ԫ��
	static	int ParseSingleMemberElement( TiXmlElement& memberElement, MemberInfo& info, const std::string& hosturi, const std::string& confmode);

	static void  ParseSingleMemberElement( TiXmlElement& memberElement, QVariantMap* member, const std::string& hosturi, const std::string& confmode);

	//���������ն���Ϣ����xml�����е�<terminal></terminal>Ԫ��
	static int ParseSingleTerminalElement( TiXmlElement& memberElement, TerminalInfo& info, const std::string& hosturi, const std::string& confmode);


private:
    TiXmlDocument* _doc;
    TiXmlElement* _coolviewElement;
	
	std::string _content;		//xml��Ϣ���ı�����
};


inline std::string CVMsgParser::GetFirstChildElementValue(TiXmlElement* parentElement, const char* childElementName)
{
    TiXmlElement* element = parentElement->FirstChildElement(childElementName);
    if(element)
    {
        TiXmlNode* node = element->FirstChild();
        if(node)
            return node->ValueStr();    
        //return node->ToText()->ValueStr();    
    }
    return "";
}

#endif