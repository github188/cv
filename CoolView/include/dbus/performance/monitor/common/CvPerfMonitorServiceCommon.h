#pragma  once

#define PERFORMANCE_MONITOR_SERVICE_NAME	"com.dcampus.coolview.performance.monitor"
#define PERFORMANCE_MONITOR_OBJECT_PATH		"/com/dcampus/coolview/performance/monitor"

#include <QtCore/QtCore>

//����������
class MonitorTriggerAction
{
public:
	static const unsigned int NO_ACTION = 0;
	static const unsigned int EMAIL = 0x00000001;				//�ʼ�֪ͨ
	static const unsigned int RECOVER_PROCESS = 0x00000002;	//�ָ�����
	static const unsigned int RESTART_COMPUTOR = 0x00000004;	//��������
};
//�������������˾��������Լ������Ķ�������
class Trigger
{
public:
	static const unsigned short NO_TRIGGER = 0;
	static const unsigned short CPU_USAGE = 0x0001;			//��λΪ%
	static const unsigned short MEM_USAGE = 0x0002;			//��λΪMB
	static const unsigned short TIME_TO_LIVE = 0x0003;		//ͨ������������TTLֵ�Ĵ�С
	static const unsigned short PROCESS_ALIVE = 0x0004;		//�����Ƿ��ڹ���

	//�������ֽ�Ϊ������ͣ��������ֽڴ洢����ֵ
	unsigned int		_monitorValue;					
	unsigned int		_action;

	friend QDataStream& operator>>(QDataStream& in, Trigger& data)
	{
		in >> data._monitorValue >> data._action;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, const Trigger& data)
	{
		out << data._monitorValue << data._action;
		return out;
	}
};

//�ܼ�صĽ�������
class MonitoredProcess
{
public:
	MonitoredProcess()
	{
		_processID = -1;
		_monitorInterval = -1;
		_monitorThread = NULL;
	}

	///�Ƿ�Ҫ��ظý���
	void SetMonitorMark( const bool monitored ){ _isUnderMonitor = monitored; }
	bool GetMonitorMark( ) { return _isUnderMonitor; }


	//start-----ͨ�����л�ֱ�Ӵ��ݵĲ���
	int					_processID;			//����ID
	QString				_processName;		//��������
	QString				_processDirPath;	//��������Ŀ¼�ľ���·��
	QString				_reportEmail;		//��¼���󱨸�ķ��͵�ַ
	int					_monitorInterval;	//��ؼ������λΪms��ÿ��һ�μ�����鴥����
	//end
	MonitoredProcess& operator = ( const MonitoredProcess& rightValue);

	QVector<Trigger>	_triggerVector;		//�������б�
	QByteArray			_processImage;		//����ӳ�����ڻָ�����
	QThread*			_monitorThread;
	bool				_isUnderMonitor;	//������Ϣ�Ƿ񱻼��;

	void AddTrigger( unsigned short monitorType , unsigned short threshhold , const unsigned int actionIndex )
	{
		Trigger trigger;
		trigger._monitorValue = monitorType << 16;
		trigger._monitorValue |= threshhold ;
		trigger._action = actionIndex;
		_triggerVector.push_back( trigger );
	}

	friend QDataStream& operator>>(QDataStream& in, MonitoredProcess& data)
	{
		int triggerSize ;
		in >> data._processID >> data._processName >> data._processDirPath >> data._reportEmail >> data._monitorInterval
			>> triggerSize;
		for( int i=0 ; i<triggerSize ; i++ )
		{
			Trigger trigger;
			in >> trigger;
			data._triggerVector.push_back( trigger);
		}
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, const MonitoredProcess& data)
	{
		int triggerSize = data._triggerVector.size();
		out << data._processID << data._processName << data._processDirPath << data._reportEmail << data._monitorInterval
			<< triggerSize;
		for( int i=0 ; i<triggerSize ; i++ )
		{
			const Trigger& trigger = data._triggerVector.at(i);
			out << trigger;
		}
		return out;
	}
};

inline MonitoredProcess& MonitoredProcess::operator = ( const MonitoredProcess& rightValue )
{
	this->_processID = rightValue._processID;
	this->_processName = rightValue._processName;
	this->_reportEmail = rightValue._reportEmail;
	this->_monitorInterval = rightValue._monitorInterval;
	this->_triggerVector = rightValue._triggerVector;
	return *this;
}

//��ؽ��̵�ý������
enum MeidaMonitorType
{
	STREAM_MEDIA_SEND = 0,
	STREAM_MEDIA_RECV = 1,
	SCREEN_MEIDA_SEND = 2,
	SCREEN_MEIDA_RECV = 3
};


//�ܼ�ص�ý���������
class MediaMonitoredProcess
{
public:
	QString             userID;             //�û�ID
	int					processID;			//����ID
	QString				processName;		//��������
	int                 mediaType     ;     //���̼�ص�ý������
    // TODO: 
    bool                isSmallVideo;       //�Ƿ���С��

	friend QDataStream& operator>>(QDataStream& in, MediaMonitoredProcess& data)
	{
		in >> data.userID >> data.processID >>data.processName >> data.mediaType;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, const MediaMonitoredProcess& data)
	{
		out << data.userID << data.processID << data.processName << data.mediaType;
		return out;
	}

};