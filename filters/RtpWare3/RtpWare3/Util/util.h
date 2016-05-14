#ifndef UTIL_H
#define UTIL_H

#include "../Core/core_defs.h"
#include "RtpStat.h"

#include <WinSock2.h>
#include <ws2ipdef.h>

//��filter�����RtpPayloadType��ȡRTPͳ�Ʊ����StreamMediaType
StreamMediaType GetRtpStatMedaiTypeFromPayload(RtpPayloadType type);

//��NTPʱ��ת����DShow�ľ���ʱ�䣨����100nsΪ���ȵ�1970-1-1������ʱ�䣩
long long GetDShowTimeFromNTPTime(unsigned long long ntp_ts);

//Unix��׼�Ļ�ȡ1970-1-1������ʱ���API
int gettimeofday(struct timeval *tv, void* tz);

//��Unixʱ��ת��ΪDShowʱ�䣬tv==nullptrΪ��ȡ��ǰʱ��
long long GetDShowTimeFromUnixTime(struct timeval *tv);

//��DShowʱ���ת�ɺ��뾫��
#define Ts2Ms(_ts_) ((_ts_)/10000LL)

//addrinfo���
bool IsIPv6Addr(addrinfo *addr);
bool IsIPv4Addr(addrinfo *addr);

unsigned long GetIPv4Data(addrinfo *addr);
in6_addr *GetIPv6Data(addrinfo *addr);

bool IsLoopbackAddr(addrinfo *addr);
bool IsMulticastAddr(addrinfo *addr);

//���ַ�����ַ��ȡWindows�ڲ���addrinfo�ṹ
int AddrStrToAddrInfo(const char *ip, addrinfo **addr);

bool GetLocalIPv4Addr(char *ip_buf, unsigned short len);
bool GetLocalIPv6Addr(char *ip_buf, unsigned short len);

int GetInfoFromSockAddrStorage(const sockaddr_storage *sock_addr_storage, 
                               unsigned short sock_addr_len, 
                               char *ip_buf, int ip_buf_len, unsigned short *port);

#endif
