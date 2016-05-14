/************************************************************************/
/* ���ļ�������������Ƶͬ����������                                     */
/* ͬһDLL�У�ȫ�����������Կ�����������Ƶfilterͨ��                  */
/************************************************************************/

#ifndef MEDIA_SYNC_H
#define MEDIA_SYNC_H

#include "streams.h"

class MediaSync
{
public:
  //��ر�֤�����ͷţ���Ҫ��ʱ�������
  static void Lock() { lock_.Lock(); }
  static void Unlock() { lock_.Unlock(); }
  static void Reset();

public:
  //����ͬ��
  static REFERENCE_TIME delivered_stream_ts_; //�ο����Ѿ����͵�����ʱ���
  static long long delivered_stream_ts_wallclock_; //delivered_stream_ts_����ʱ��Ӧ�ľ���ʱ��

private:
  static CCritSec lock_;
};

#endif
