// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���ǳ��õ��������ĵ���Ŀ�ض��İ����ļ�
//

#pragma once

// TODO: �ڴ˴����ó���Ҫ��ĸ���ͷ�ļ�
#include <boost/signal.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/visit_each.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/archive_exception.hpp>
#include <boost/serialization/map.hpp>
#include <boost/program_options.hpp>

#include <QtGui/QtGui>
#include <qtutil/SafeConnect.h>
#include <qtutil/Widget.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <cutil/global.h>

#include <WengoPhoneBuildId.h>


#define  DBUS_ENABLE