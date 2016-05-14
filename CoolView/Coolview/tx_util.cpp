#include "tx_util.h"

#include <windows.h>
#include <winsock.h> // for timeval
#include <QApplication>

#include "util/ini/TxConfig.h"
#include "util/report/RecordStat.h"

namespace tx_util {

const char *PathPlaceholderDate = "{date}";
const char *PathPlaceholderDateTime = "{datetime}";
const char *PathPlaceholderVInfo = "{vinfo}";
const char *PathPlaceholderCamIndex = "{camid}";
const char *PathPlaceholderConf = "{conf}";
const char *PathPlaceholderConfDate = "{confdate}";
const char *PathPlaceholderTerminal = "{terminal}";
const char *PathPlaceholderTitle = "{title}";
const char *PathPlaceholderParticipant = "{participant}";
const char *PathPlaceholderKeywords = "{keywords}";
const char *PathPlaceholderDescription = "{description}";

FileNameParam::FileNameParam()
{
  camid = 0;
}

QString GetRecordFileNamePath(const QString &path_with_placeholder, 
                              const FileNameParam &param)
{
  QString src = path_with_placeholder;
  QString tmp;
  QHash<QString, QString> placeholder_dict;

  //PathPlaceholderDate, PathPlaceholderDateTime, PathPlaceholderVInfo��Ҫ�ײ���ʵ������
  //�����滻ΪRecordStat.h�ж����ռλ��
  placeholder_dict.insert(PathPlaceholderDate, TX_FNFMT_DATE);
  placeholder_dict.insert(PathPlaceholderDateTime, TX_FNFMT_DATETIME);
  placeholder_dict.insert(PathPlaceholderVInfo, TX_FNFMT_VINFO);

  //����ռλ��
  tmp = QString::number(param.camid);
  placeholder_dict.insert(PathPlaceholderCamIndex, tmp);

  tmp = (param.conf.isEmpty() ? QString::fromLocal8Bit("[��Ч����]") : param.conf);
  placeholder_dict.insert(PathPlaceholderConf, tmp);

  tmp = (param.conf_date.isEmpty() ? QString::fromLocal8Bit("[��Ч��������]") : param.conf_date);
  placeholder_dict.insert(PathPlaceholderConfDate, tmp);

  tmp = (param.terminal.isEmpty() ? QString::fromLocal8Bit("[��Ч�ն�]") : param.terminal);
  placeholder_dict.insert(PathPlaceholderTerminal, tmp);

  tmp = (param.title.isEmpty() ? QString::fromLocal8Bit("[�ޱ���]") : param.title);
  placeholder_dict.insert(PathPlaceholderTitle, tmp);

  tmp = (param.participant.isEmpty() ? QString::fromLocal8Bit("[�������]") : param.participant);
  placeholder_dict.insert(PathPlaceholderParticipant, tmp);

  tmp = (param.keywords.isEmpty() ? QString::fromLocal8Bit("[�޹ؼ���]") : param.keywords);
  placeholder_dict.insert(PathPlaceholderKeywords, tmp);

  tmp = (param.description.isEmpty() ? QString::fromLocal8Bit("[������]") : param.description);
  placeholder_dict.insert(PathPlaceholderDescription, tmp);

  //ʵ���滻
  for (const QString &key : placeholder_dict.keys()) {
    if (src.indexOf(key) != -1) {
      src.replace(key, placeholder_dict[key]);
    }
  }
  return src;
}

QString TXGetRecordRootDir()
{
  QString rec_dir = CTxConfig::getInstance().GetRecPath();
  if (rec_dir.isEmpty()) {
    //û���������Ĭ��ֵ
    rec_dir = "txcache";
  }
  if (rec_dir.indexOf(":") == -1) {
    //���Ǿ���·����������԰�װĿ¼�µ�·��
    QString app_dir = QCoreApplication::applicationDirPath();
    app_dir.replace("/" , "\\" );
    if (rec_dir[0] == '\\') {
      rec_dir = app_dir + rec_dir;
    } else {
      rec_dir = app_dir + "\\" + rec_dir;
    }
  }
  if (rec_dir[rec_dir.size()-1] == '\\') {
    //�ԡ�\����β��ȥ����
    rec_dir = rec_dir.mid(0, rec_dir.size() - 1);
  }
  return rec_dir;
}


static unsigned __int64 CalculateMicroseconds(unsigned __int64 performancecount,unsigned __int64 performancefrequency)
{
  unsigned __int64 f = performancefrequency;
  unsigned __int64 a = performancecount;
  unsigned __int64 b = a/f;
  unsigned __int64 c = a%f; // a = b*f+c => (a*1000000)/f = b*1000000+(c*1000000)/f

  return b*1000000ui64+(c*1000000ui64)/f;
}

/**   
 *  Returns an timeval struct representing the current wallclock time. This is expressed 
 *  as a number of seconds since 00:00:00 UTC, January 1, 1970.
 */
int gettimeofday(struct timeval *tv, void* tz)
{
	static int inited = 0;
	static unsigned __int64 microseconds, initmicroseconds;
	static LARGE_INTEGER performancefrequency;

	unsigned __int64 emulate_microseconds, microdiff;
	SYSTEMTIME systemtime;
	FILETIME filetime;

	LARGE_INTEGER performancecount;

	QueryPerformanceCounter(&performancecount);
    
	if(!inited){
		inited = 1;
		QueryPerformanceFrequency(&performancefrequency);
		GetSystemTime(&systemtime);
		SystemTimeToFileTime(&systemtime,&filetime);
		microseconds = ( ((unsigned __int64)(filetime.dwHighDateTime) << 32) + (unsigned __int64)(filetime.dwLowDateTime) ) / 10ui64;
		microseconds-= 11644473600000000ui64; // EPOCH
		initmicroseconds = CalculateMicroseconds(performancecount.QuadPart, performancefrequency.QuadPart);
	}
    
	emulate_microseconds = CalculateMicroseconds(performancecount.QuadPart, performancefrequency.QuadPart);

	microdiff = emulate_microseconds - initmicroseconds;
  
    tv->tv_sec = (long)((microseconds + microdiff) / 1000000ui64);
    tv->tv_usec = (long)((microseconds + microdiff) % 1000000ui64);

    tz; //����δ���õı�������
    return (0);
}

long long GetNonrepeatableTimestamp()
{
  static long long last = 0;
  long long cur = 0;
  do {
    struct timeval now;
    gettimeofday(&now, NULL);
    cur = (long long)(now.tv_sec * 1000000LL + now.tv_usec);
  } while (cur == last);
  last = cur;
  return cur;
}

}
