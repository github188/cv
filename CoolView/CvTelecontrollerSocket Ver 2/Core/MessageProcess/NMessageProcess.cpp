/*
 * ͨ����Ϣ�����ඨ���ļ�
 * V1.0.0.0	2011-12-15	�ض���Ϣ�������������������� By Lzy
 * V0.0.0.0 2011-11-23  By Lzy
 */

#define N_MESSAGE_PROCESS_CPP

#include "NMessageProcess.h"

#include "CommonMessageProcess.h"
using namespace Common_Message_Space;

#define ADD_API(BindKeyNum,BindFuncName) ms_oFuncP.insert(pair<int,FDealProcess>(BindKeyNum,&CNMessageProcess::BindFuncName));

map<int,CNMessageProcess::FDealProcess> CNMessageProcess::ms_oFuncP;
/*
 * ��������CNMessageProcess�๹�캯��
 */
CNMessageProcess::CNMessageProcess()
{
    if (CNMessageProcess::ms_oFuncP.empty() != false)
    {
        /////////////////////////////������ض�ͨ����Ϣ��������ע�����////////////////////////////
            ADD_API(TELE_ExitProcess,           _ExitProcess)
            ADD_API(TELE_Layout,                _Layout)
            ADD_API(TELE_MemberLocation,        _MemberLocation)
            ADD_API(TELE_MediaState,            _MediaState)
            ADD_API(TELE_AddMemberLocation,     _AddMemberLocation)
            ADD_API(TELE_RemoveMemeberLocation, _RemoveMemeberLocation)
            ADD_API(TELE_ChangeMemberLocation,  _ChangeMemberLocation)
            ADD_API(TELE_PostMeetingInfo,       _PostMeetingInfo)
			ADD_API(TELE_ScreenShareState,      _ScreenShareState)
			ADD_API(TELE_TerminalConfig,        _TerminalInfo)
			ADD_API(TELE_PanelState,            _PanelState)
        ///////////////////////////////////////////////////////////////////////////////////////
    }
}

/*
 * �����������յ���ͨ����Ϣ�Ĵ�����
 * @param int nInfoIndex,ͨ����Ϣ��ʶ
 * @param int nNotifyId,ͨ����Ϣ���ն���ң��������ID,��ֵ��ʾͨ������ң����
 * @param const QByteArray &rInputArray ͨ����Ϣ����
 * @return void
 */
void CNMessageProcess::MainProcess(int nInfoIndex,int nNotifyId,const QByteArray &rInputArray)
{
    QDataStream oStreamIn(rInputArray);
	oStreamIn.setVersion(QDataStream::Qt_4_4);
    if (this->_PreProcess(nInfoIndex,nNotifyId,rInputArray) != false)
    {
        map<int,FDealProcess>::iterator iFunI = CNMessageProcess::ms_oFuncP.find(this->m_nKey);
        if (iFunI == CNMessageProcess::ms_oFuncP.end())
            return;
        if ((this->*(iFunI->second))(nNotifyId,oStreamIn) != false)
            this->_SurProcess(nInfoIndex,nNotifyId,rInputArray);
    }
}

//////////////////////////////////�����н��յ���ͨ����Ϣ��ǰ�ڴ���Ĵ�����_PreProcess������ʵ��////////////////////

/*
 * ��˽�С���ϢԤ������
 * @info ���ϣ�������ض���Ϣ��������������ڱ���������_SetMessageKey�������ض���Ϣ��������������Ϣ��
 * @param int nInfoIndex,ͨ����Ϣ��ʶ
 * @param int nNotifyId,ͨ����Ϣ���ն���ң��������ID,��ֵ��ʾͨ������ң����
 * @param const QByteArray &rInputArray ͨ����Ϣ����
 * @return bool,���ִ�п��ƣ�����true��ʾ����ִ�к�������ض���Ϣ�Ĵ������������false�򱾴���Ϣ�������
 */
bool CNMessageProcess::_PreProcess(int nInfoIndex,int nNotifyId,const QByteArray &rInputArray)
{
#ifdef _DEBUG
	qDebug("Info Index: %d",nInfoIndex);
#endif
    this->_SetMessageKey(nInfoIndex);
    return true;
}

//////////////////////////////////�����н��յ���ͨ����Ϣ����������Ĵ�����_SurProcess������ʵ��////////////////////

/*
 * ��˽�С���Ϣ��������
 * @info ���ǰ��_PreProcessִ�к󷵻�false�����ض���Ϣ������ִ�к󷵻�false���򱾺������ᱻִ��
 * @param int nInfoIndex,ͨ����Ϣ��ʶ
 * @param int nNotifyId,ͨ����Ϣ���ն���ң��������ID,��ֵ��ʾͨ������ң����
 * @param const QByteArray &rInputArray ͨ����Ϣ����
 * @return void
 */
void CNMessageProcess::_SurProcess(int nInfoIndex,int nNotifyId,const QByteArray &rInputArray)
{
}

/////////////////////////////////���ض�ͨ����Ϣ�Ĵ�������д������///////////////////////////////////////////

bool CNMessageProcess::_ExitProcess(int nNotifyId,QDataStream &rQStream)
{
#ifdef _DEBUG
    printf ("Receive TELE_ExitProcess\n");
#endif
    g_bProgramme_Exit = true;
    return false;
}

bool CNMessageProcess::_Layout(int nNotifyId,QDataStream &rQStream)
{
#ifdef _DEBUG
    printf ("Receive TELE_Layout\n");
#endif
    int infoType;
	rQStream >> infoType;		//��ȡͨ������
	Json::Value root;
	root["info_type"] = Json::Value(infoType);

	int screenNum;
	rQStream >> screenNum;
	root["screen_num"] = Json::Value( screenNum);

	for( int i=0 ; i<screenNum ; i++ )
	{
		Json::Value json_obj;

		ScreenInfo screen;
		rQStream >> screen;
		char objKeyName[20] = {0};
		strcpy( objKeyName , "screen" );
		strcat( objKeyName , QString::number(i).toUtf8().constData() );

		json_obj["layout"] = qPrintable(screen._layout );
		json_obj["screen_index"] = screen._screenIndex;
		root[objKeyName] = json_obj;
	}

	Json::FastWriter writer;
	QString params = QString::fromStdString( writer.write(root));

	CNetServ::GetInstance()->AddNotify(qPrintable(params),params.length(),nNotifyId);
    
    return true;
}

bool CNMessageProcess::_MemberLocation(int nNotifyId,QDataStream &rQStream)
{
#ifdef _DEBUG
    printf ("Receive TELE_MemberLocation\n");
#endif
    int infoType;
	rQStream >> infoType;		//��ȡͨ������

	int locationNum;
	rQStream >> locationNum;	//��ȡ���ڵ���Ŀ

	for( int i=0 ; i<locationNum ; )
	{
		Json::Value root;
		root["info_type"] = Json::Value(infoType);

		//ÿ������ܷ���6�����ڵ���Ϣ
		int restNum = locationNum -i;
		int sendNum = 0;
		if( restNum<=0 )
		{
			break;
		}else if( restNum>=6 )
		{
			sendNum = 6;
		}else
		{
			sendNum = restNum;
		}
		root["location_num"] = Json::Value( sendNum );

		for( int count=0; count <sendNum; count++ )
		{
			Json::Value json_obj;

			MemberLocation location;
			rQStream >> location;
			char objKeyName[20] = {0};
			strcpy( objKeyName , "location" );
			strcat( objKeyName , QString::number(count).toUtf8().constData() );

			json_obj["member_name"] = qPrintable(location._memberName);
			json_obj["screen_index"] = location._screenIndex;
			json_obj["seet"] = location._seet;
			json_obj["audioEnable"] = location._audioEnable;
			json_obj["videoEnable"] = location._videoEnable;
			root[objKeyName] = json_obj;
		}

		//�����û���λ����Ϣ
		Json::FastWriter writer;
		QString params = QString::fromStdString( writer.write(root));
		CNetServ::GetInstance()->AddNotify(qPrintable(params),params.length(),nNotifyId);

		i += sendNum;
	}
    
    return true;
}

bool CNMessageProcess::_MediaState(int nNotifyId,QDataStream &rQStream)
{
#ifdef _DEBUG
    printf ("Receive TELE_MediaState\n");
#endif
    int infoType;
	rQStream >> infoType;		//��ȡͨ������
	Json::Value root;
	root["info_type"] = Json::Value(infoType);

	MediaStateNotify notify;
	rQStream >> notify;
	root["member_name"] = Json::Value( qPrintable(notify._memberName));
	root["media_type"] = Json::Value( qPrintable(notify._mediaType) );
	root["media_state"] = Json::Value( qPrintable(notify._mediaState ) );
	
	Json::FastWriter writer;
	QString params = QString::fromStdString( writer.write(root));

	CNetServ::GetInstance()->AddNotify(qPrintable(params),params.length(),nNotifyId);

    return true;
}

bool CNMessageProcess::_AddMemberLocation(int nNotifyId,QDataStream &rQStream)
{
#ifdef _DEBUG
    printf ("Receive TELE_AddMemberLocation\n");
#endif
    int infoType;
	rQStream >> infoType;
	Json::Value root;
	root["info_type"] = Json::Value(infoType);

	MemberLocation location;
	rQStream >> location;

	root["member_name"] = qPrintable(location._memberName);
	root["screen_index"] = location._screenIndex;
	root["seet"] = location._seet;
	root["audioEnable"] = location._audioEnable;
	root["videoEnable"] = location._videoEnable;

	//�����û���λ����Ϣ
	Json::FastWriter writer;
	QString params = QString::fromStdString( writer.write(root));
	CNetServ::GetInstance()->AddNotify(qPrintable(params),params.length(),nNotifyId);
    
    return true;
}

bool CNMessageProcess::_RemoveMemeberLocation(int nNotifyId,QDataStream &rQStream)
{
#ifdef _DEBUG
    printf ("Receive TELE_RemoveMemeberLocation\n");
#endif
    int infoType;
	rQStream >> infoType;
	Json::Value root;
	root["info_type"] = Json::Value(infoType );

	QString memberName;
	int screen;
	int seet;
	rQStream >> memberName;
	rQStream >> screen;
	rQStream >> seet;

	root["member_name"] = qPrintable(memberName);
	root["screen_index"] = screen;
	root["seet"] = seet;

	//�����û���λ����Ϣ
	Json::FastWriter writer;
	QString params = QString::fromStdString( writer.write(root));
	CNetServ::GetInstance()->AddNotify(qPrintable(params),params.length(),nNotifyId);

    return true;
}

bool CNMessageProcess::_ChangeMemberLocation(int nNotifyId,QDataStream &rQStream)
{
#ifdef _DEBUG
    printf ("Receive TELE_ChangeMemberLocation\n");
#endif
    int infoType;
	rQStream >> infoType;
	Json::Value root;
	root["info_type"] = Json::Value(infoType );

	QString memberName;
	int screenIndex;
	int seet;
	rQStream >> memberName;
	rQStream >> screenIndex;
	rQStream >> seet;

	root["member_name"] = qPrintable(memberName);
	root["screen_index"] = screenIndex;
	root["seet"] = seet;

	//�����û���λ����Ϣ
	Json::FastWriter writer;
	QString params = QString::fromStdString( writer.write(root));
	CNetServ::GetInstance()->AddNotify(qPrintable(params),params.length(),nNotifyId);

    return true;
}

bool CNMessageProcess::_PostMeetingInfo(int nNotifyId,QDataStream &rQStream)
{
#ifdef _DEBUG
    printf ("Receive TELE_PostMeetingInfo\n");
#endif
    int infoType;
	rQStream >> infoType;
	QString qChairmanURI;
	rQStream >> qChairmanURI;
	CDataManager::GetInstance()->SetDataString(qChairmanURI.toStdString().c_str(),false,"Chairman");

	return true;
}

bool CNMessageProcess::_ScreenShareState(int nNotifyId,QDataStream &rQStream)
{
#ifdef _DEBUG
    printf ("Receive TELE_ScreenShareState\n");
#endif
	int infoType;
	rQStream >> infoType;
	Json::Value root;
	root["info_type"] = Json::Value(infoType );

	bool state;
	rQStream >> state;
	root["state"] = state;

	//�����û���λ����Ϣ
	Json::FastWriter writer;
	QString params = QString::fromStdString( writer.write(root));
	CNetServ::GetInstance()->AddNotify(qPrintable(params),params.length(),nNotifyId);

    return true;
}

bool CNMessageProcess::_TerminalInfo(int nNotifyId,QDataStream &rQStream)
{
	return true;
}

bool CNMessageProcess::_PanelState(int nNotifyId,QDataStream &rQStream)
{
#ifdef _DEBUG
	printf ("Receive TELE_PanelState\n");
#endif
	int infoType;
	rQStream >> infoType;
	Json::Value root;
	root["info_type"] = Json::Value(infoType );

	bool visible;
	rQStream >> visible;
	root["visible"] = visible;

	//�����û���λ����Ϣ
	Json::FastWriter writer;
	QString params = QString::fromStdString( writer.write(root));
	CNetServ::GetInstance()->AddNotify(qPrintable(params),params.length(),nNotifyId);

	return true;
}