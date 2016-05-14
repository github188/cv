#pragma once

#define TXCONTROLLER_SERVICE_NAME			"com.dcampus.coolview.controller.txcontroller"
#define TXCONTROLLER_SERVICE_OBJECT_PATH	"/com/dcampus/coolview/controller/txcontroller"

#include <QtCore/QString>
#include <QtCore/QDataStream>
#include <QtCore/QStringList>
#include <QtCore/QVector>


enum RecordOperation
{
	TXRecordOp_Start = 0,
	TXRecordOp_Stop = 1,
};

//���������� - ����DBus��WebService���˼�����
const int TXRecord_Success = 0;
const int TXRecord_Error = 1; //һ�����
const int TXRecord_UriNotExist = 2;
const int TXRecord_IsRecording = 3;
const int TXRecord_IsNotRecording = 4;
const int TXRecord_NotInFocus = 5;

//�ն�¼��״̬
#define TxRecordStatus_Offline "offline"
#define TxRecordStatus_Online "online"
#define TxRecordStatus_Recording "recording"

//StartRecordParam
//�ò�������ͨ�ն��У�����ConfRoom��CoolView����¼�ƿ��Ʋ�����
//��TX�У�����TxMonitor��SOAP�����CoolView�е�TxController����¼�ƿ���ָ��
class StartRecordParam
{
public:
	QString focus; //Ҫ¼�Ƶ��ն�����focus
  QString requester_uri; //¼������uri
	QString target_uri; //Ҫ¼�Ƶ��ն�uri
  QString title; //¼����Ƶ�����⣨��ѡ��
  QString participants; //�λ��ߣ���ѡ��
  QString keywords; //�ؼ��ʣ���ѡ��
  QString description; //��������ѡ��

	friend QDataStream &operator>>(QDataStream &in, StartRecordParam &param)
	{
		in >> param.focus >> param.requester_uri >> param.target_uri 
      >> param.title >> param.participants >> param.keywords >> param.description;
		return in;
	}
	friend QDataStream &operator<<(QDataStream &out, StartRecordParam &param)
	{
    out << param.focus << param.requester_uri << param.target_uri 
      << param.title << param.participants << param.keywords << param.description;
		return out;
	}
};

class StopRecordParam
{
public:
  QString focus; //Ҫ¼�Ƶ��ն�����focus
  QString target_uri; //Ҫ¼�Ƶ��ն�uri

  friend QDataStream &operator>>(QDataStream &in, StopRecordParam &param)
  {
    in >> param.focus >> param.target_uri;
    return in;
  }
  friend QDataStream &operator<<(QDataStream &out, StopRecordParam &param)
  {
    out << param.focus << param.target_uri;
    return out;
  }
};

class RecordTerminalStatus
{
public:
  QString vuri;
  QString name;
  QString status;
  int virtual_count; // ���ն��ܵ������ն���
  QString requester_uri;
  unsigned int duration;
  unsigned long long recorded_bytes;

  RecordTerminalStatus() :
    virtual_count(0),
    duration(0),
    recorded_bytes(0) {}

  friend QDataStream &operator>>(QDataStream &in, RecordTerminalStatus &param)
  {
    in >> param.vuri >> param.name >> param.status >> param.virtual_count >> 
      param.requester_uri >> param.duration >> param.recorded_bytes;
    return in;
  }
  friend QDataStream &operator<<(QDataStream &out, RecordTerminalStatus &param)
  {
    out << param.vuri << param.name << param.status << param.virtual_count <<
      param.requester_uri << param.duration << param.recorded_bytes;
    return out;
  }
};

class RecordFocusStatus
{
public:
  QVector<RecordTerminalStatus> terminals_status;

  friend QDataStream &operator>>(QDataStream &in, RecordFocusStatus &param)
  {
    int len;
    in >> len;
    for (int i = 0; i < len; ++i) {
      RecordTerminalStatus status;
      in >> status;
      param.terminals_status.push_back(status);
    }
    return in;
  }
  friend QDataStream &operator<<(QDataStream &out, RecordFocusStatus &param)
  {
    out << param.terminals_status.size();
    for (auto t : param.terminals_status) {
      out << t;
    }
    return out;
  }
};
