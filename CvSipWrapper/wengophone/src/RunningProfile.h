/************************************************************************/
/* @author zhenHua.sun
 * @date 2010-08-31
 * @brief �洢��Ƶ��������еļ�ʱ״̬�����������Щ���飬��������Щ�Ự
          ��������Ƶ���շ�����Ϣ�����Թ�����ģ��Ķ�ȡ
 */
/************************************************************************/
#ifndef RUNNING_PROFILE_H
#define RUNNING_PROFILE_H

#include "SessionManager.h"



class RunningProfile
{
public:
	static RunningProfile& getInstance()
	{
		static RunningProfile runningProfile;
		return runningProfile;
	}

	/**
	 * @brief ��ȡ�Ự������
	 */
	SessionManager& getSessionManager()
	{
		return _sessionManager;
	}

private:
	RunningProfile(){}
	~RunningProfile(){}

	////���º���ֻ���岻ʵ�֣���ֹ�����࿽��
	////�ž� CClass object = CClass::getInstance()�ĳ���
	RunningProfile( const RunningProfile& );
	RunningProfile& operator= ( const RunningProfile& );

	SessionManager _sessionManager;


};




#endif