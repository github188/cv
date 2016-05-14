#pragma once

#ifndef LOG_H
#define LOG_H

#include <fstream>
using namespace std;


static char __global_msg[1024];		//��־��Ϣ

enum{
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARN,
	LOG_LEVEL_ERROR
};	//zhenHua.sun 2010-05-26

class Log
{
public:
	Log(void);
	~Log(void);

	void startWrite();//ʱ�䣬�ļ�����������

	void write(const char *param);
	void write(const int param);
	void write(const void *p);
	void endWrite();

private:
	ofstream out;

public:
	static char _logFileName[128];
};

/**
 * zhenHua.sun 2010-05-26
 * @brief ���ڼ�¼������־��¼
 */
class Logger
{
public:

	~Logger(void);
	
	/** Singleton. */
	static Logger* getInstance();

	/**
	 * @brief Debug ��Ϣ��¼����
	 * @param component ���øú��������
	 * @param classname ���øú���������
	 * @param message ��Ҫ��¼����Ϣ
	 */
	void debug( const char *component , const char *classname , const char *message );

	void info( const char *component , const char *classname , const char *message );

	void warn( const char *component , const char *classname , const char *message );

	void error( const char *component , const char *classname , const char *message );
private:

	Logger();
	class CGarbo // ����Ψһ��������������������ɾ��CSingleton��ʵ��  
	{  
		public:  
			~CGarbo()  
			{  
				if (Logger::p_logger)  
				{
					delete Logger::p_logger;
					p_logger = NULL;
				}
			}  
	};   
	static CGarbo Garbo; // ����һ����̬��Ա���ڳ������ʱ��ϵͳ����������������� 

private:
	Log				log;
	static Logger*	p_logger;
	const char*		m_pComponent;
};

void msdxLogFunction(int msdx_log_level, const char* filename, int line,const char* message);

#define MSDX_LOG_ERROR(message) \
    msdxLogFunction(LOG_LEVEL_ERROR, \
            __FILE__, \
            __LINE__,\
            message) 

#define MSDX_LOG_INFO(message)\
	msdxLogFunction(LOG_LEVEL_INFO, \
			__FILE__, \
			__LINE__,\
			message)

//debugģʽ���¼��־��releaseģʽ��ѵ���ȥ����������¼��־
#ifdef _DEBUG 
#define MSDX_LOG_DEBUG(message)\
	msdxLogFunction(LOG_LEVEL_DEBUG, \
	__FILE__, \
	__LINE__,\
	message)
#else
#define MSDX_LOG_DEBUG
#endif // _DEBUG


void logFunction( const char* component , int log_level, const char* filename, int line,const char* message);

#define MONITOR_LOG_ERROR(message) \
	logFunction( "CvMonitor" , LOG_LEVEL_ERROR, \
	__FILE__, \
	__LINE__,\
	message) 

#define MONITOR_LOG_INFO(message)\
	logFunction( "CvMonitor" ,LOG_LEVEL_INFO, \
	__FILE__, \
	__LINE__,\
	message)

//debugģʽ���¼��־��releaseģʽ��ѵ���ȥ����������¼��־
#ifdef _DEBUG 
#define MONITOR_LOG_DEBUG(message)\
	logFunction( "CvMonitor" ,LOG_LEVEL_DEBUG, \
	__FILE__, \
	__LINE__,\
	message)
#else
#define MONITOR_LOG_DEBUG
#endif // _DEBUG



#define CV_LOG_ERROR(message) \
	logFunction( "CoolView" , LOG_LEVEL_ERROR, \
	__FILE__, \
	__LINE__,\
	message) 

#define CV_LOG_INFO(message)\
	logFunction( "CoolView" ,LOG_LEVEL_INFO, \
	__FILE__, \
	__LINE__,\
	message)

//debugģʽ���¼��־��releaseģʽ��ѵ���ȥ����������¼��־
#ifdef _DEBUG 
#define CV_LOG_DEBUG(message)\
	logFunction( "CoolView" ,LOG_LEVEL_DEBUG, \
	__FILE__, \
	__LINE__,\
	message)
#else
#define CV_LOG_DEBUG
#endif // _DEBUG



#define CONFROOM_LOG_ERROR(message) \
	logFunction( "CoolView" , LOG_LEVEL_ERROR, \
	__FILE__, \
	__LINE__,\
	message) 

#define CONFROOM_LOG_INFO(message)\
	logFunction( "CoolView" ,LOG_LEVEL_INFO, \
	__FILE__, \
	__LINE__,\
	message)

//debugģʽ���¼��־��releaseģʽ��ѵ���ȥ����������¼��־
#ifdef _DEBUG 
#define CONFROOM_LOG_DEBUG(message)\
	logFunction( "CoolView" ,LOG_LEVEL_DEBUG, \
	__FILE__, \
	__LINE__,\
	message)
#else
#define CONFROOM_LOG_DEBUG
#endif // _DEBUG


#define TELE_LOG_ERROR(message) \
	logFunction( "Telecontroller" , LOG_LEVEL_ERROR, \
	__FILE__, \
	__LINE__,\
	message) 

#define TELE_LOG_INFO(message)\
	logFunction( "Telecontroller" ,LOG_LEVEL_INFO, \
	__FILE__, \
	__LINE__,\
	message)

//debugģʽ���¼��־��releaseģʽ��ѵ���ȥ����������¼��־
#ifdef _DEBUG 
#define TELE_LOG_DEBUG(message)\
	logFunction( "Telecontroller" ,LOG_LEVEL_DEBUG, \
	__FILE__, \
	__LINE__,\
	message)
#else
#define TELE_LOG_DEBUG
#endif // _DEBUG
#endif