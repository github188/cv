/*
 * ������Ϣ����ģ�鹫�����ֵķ�װ �����ļ�
 * 2011-12-15   V1.0.0.0    By Lzy
 */

#ifndef COMMON_MESSAGE_PROCESS_H
#define COMMON_MESSAGE_PROCESS_H

#include "DataManager.h"
#include "NetServ.h"
#include "QtDbusServ.h"
#include <Qtcore/QtCore>

#include "json.h"

namespace Common_Message_Space
{
    void SetNewMonitor(const char *pszSipAccount,int nNotifyIndex);//�����µ�Ȩ����Ϣ��������ң��������ͨ��
};

#endif