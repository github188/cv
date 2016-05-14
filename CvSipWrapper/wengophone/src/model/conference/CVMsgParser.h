#pragma once

#ifndef CVMsgParser_h
#define CVMsgParser_h


#include < iostream>
#include < fstream>
#include <tinyxml.h>
#include <map>
#include <vector>
#include <list>
using namespace std;

typedef struct tagConfInfo
{
    string CID;         //�����id��ȫ��Ψһ
    string URI;
    string Title;
    string Description;
    string HostURI;		//������URI
    string ControlMode;
    string JoinMode;
    string JoinPassword;
    string StartTime;
    string Duration;

	string multicastAddress;	//����������鲥���飬����Ϊ�鲥��ַ������Ϊ0.0.0.0  --ע:���ڵ�ǰ���Ʒ����������ն˵Ļ����б�û�иõ�ַ��Ϣ������multicastAddress�ֶο���Ϊ�գ�ʹ��ʱ��ע�⡣
}ConfInfo;

typedef struct tagMemberInfo
{
    string MemberURI;   //��Աuri
    string Sdp;         //sdp
    int isReceived;     //�Ƿ��Ѿ������˸ó�Ա��ý������0δ���գ�1�ѽ���
    int permitSpeak;    //�Ƿ������ԣ�0������1����2����������
    int hand;           //�Ƿ���֣�0û���֣�1����
    bool isMulticast;   //�Ƿ�֧���鲥
	string multicastAddress;		//�鲥�����IP��ַ��������������0.0.0.0
	int videoPort;		//��Ƶ���ݴ���ı��ض˿ڣ�Ĭ��ֵΪ0
	int audioPort ;		//��Ƶ���ݴ���ı��ض˿ڣ�Ĭ��ֵΪ0
}MemberInfo;

struct ReceiverInfo
{
    string MemberURI;   //��Աuri
    string recvIP;			 //��Ա��IP��ַ
    int videoPort;		//��Ƶ���ݴ���˿�
    int audioPort ;		//��Ƶ���ݴ���˿�   
};

//������������Ϣ����Ϣ����
struct MsgLoginPermissionInfo
{
    string userURI;
    int dscp;
    int rate;
};

//��ֹ���������Ϣ����Ϣ����
struct MsgLoginRejectionInfo
{
    string userURI;
    string description;
    int suggestedRate;
};


struct QoSTestBeginSendInfo
{
	int sessionId;
	string remoteIP;
	int remotePort;
	int sendRate;
	int dscp;
	int duration;
};

struct  QoSTestBeginReceiveInfo
{
	int sessionId;
	int localPort;
	string sourceIP;
	int duration;
};

class CVMsgParser
{
public:
    CVMsgParser(void);
    ~CVMsgParser(void);

    //��sip��Ϣ���ݽ����ɻ����б�
    vector<ConfInfo*> ParseConfListMsg(vector<ConfInfo*>& conf_list);
    //�������ʱ�������Ƶ���͵ĵ�ַ�Ͷ˿�
    void getAddrPort(int& audio_port, int& video_port, string& address);

    ////��sip��Ϣ���ݽ�����Ա�б�������Ҫ�����˼�������
    vector<MemberInfo*> ParseMemberListMsg(vector<MemberInfo*>& member_list,const string& hosturi, const string& confmode);
    vector<MemberInfo*> ParseMemberListMsg(int& audiao_port, int& video_port, string& address);
    vector<MemberInfo*> ParseMemberListMsg(int& audio_port, int& video_port, string& address, const string& hosturi, const string& confmode);

	//���ݳ�Ա����/�˳�������Ϣ���³�Ա�б�
    vector<MemberInfo*> ParseMemberListByNotifyMsg(vector<MemberInfo*>& member_list, const string& hosturi, const string& confmode);

    //������Ϣ�����е���������
    const char* GetCommandType();
    void InitParser(const char* message);

    //��ջ����б�
	static  void ReleaseConfList(vector<ConfInfo*>& conf_list);
    //�����Ա�б�
   static	 void ReleaseMemberList(vector<MemberInfo*>& member_list);

	//����Confinfo����һ��xml����
    string CreateMsgByConfInfo(ConfInfo* conf_info);
    string CreateMsgByConfInfo(ConfInfo* conf_info, string command);

    //��ȡFocusURI
    string GetFocusURI();
    string GetMemberUri();
    string GetInviterUri();
    string GetLoginMemberUri();
    string GetTextMessage();
    string GetWhiteBoardMessage();
    //
    string GetReceiverIP();

    /*
      Add by heixanfu 
         Ϊ�˴�����յ�QOS��Ϣ
      09-9-4
    */
    string GetElementValueByName(const string fatherElementName, const string childElementName);
	
	//��ȡqos�����type����
    string GetQosType();

  
    //����LoginPermission sip ��Ϣ������0��ʾ�����ɹ�
    int ParseLoginPermissionMsg(MsgLoginPermissionInfo& info);

    //����һ��LoginRejection  sip ��Ϣ������0��ʾ�����ɹ�
    int ParseLoginRejectionMsg(MsgLoginRejectionInfo& info);

	//����һ��QoSTestBeginSend  sip ��Ϣ������0��ʾ�����ɹ�
	int ParseQoSTestBeginSendInfo(QoSTestBeginSendInfo& info);

	//����һ��QoSTestBeginReceive  sip ��Ϣ������0��ʾ�����ɹ�
	int ParseQoSTestBeginReceiveInfo(QoSTestBeginReceiveInfo& info);

	string GetQosTestType();

private:
    //��ȡ����ָ�����Ƶĵ�һ��xmlԪ�ص�ֵ���ַ��������Ҳ�����ӦԪ���򷵻ؿ��ַ���
    static string GetFirstChildElementValue(TiXmlElement* parentElement, const char* childElementName);
	
	//����������Ա��Ϣ����xml�����е�<member></member>Ԫ��
	static	int ParseSingleMemberElement( TiXmlElement& memberElement, MemberInfo& info, const string& hosturi, const string& confmode);
private:
    TiXmlDocument* _doc;
    TiXmlElement* _coolviewElement;
};


inline string CVMsgParser::GetFirstChildElementValue(TiXmlElement* parentElement, const char* childElementName)
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