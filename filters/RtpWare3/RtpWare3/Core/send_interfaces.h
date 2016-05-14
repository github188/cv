/************************************************************************/
/* �˴����巢��������ģ��Ľӿ�                                         */
/************************************************************************/

#ifndef SEND_INTERFACES_H
#define SEND_INTERFACES_H

#include "core_defs.h"
#include "RtpStat.h"
#include <string>
#include <memory>
#include <queue>

//
// rtp���ص�Ԫ
//
class ISendUnit
{
public:
  virtual ~ISendUnit() {}

  virtual unsigned char * GetData() = 0;
  virtual unsigned long GetSize() = 0;
};

typedef std::shared_ptr<ISendUnit> ISendUnitPtr;
typedef std::queue<ISendUnitPtr> ISendUnitQueue;

//
// ���һ֡�����е�Ԫ
//
class IFrameUnits
{
public:
  virtual ~IFrameUnits() {}

  virtual unsigned long GetTimestamp() = 0;
  virtual void SetTimestamp(unsigned long) = 0;
  virtual void PushUnit(ISendUnitPtr p) = 0;
  virtual ISendUnitPtr PopUnit() = 0;
  virtual bool Empty() = 0; //�Ƿ�Ϊ��
  virtual IFrameUnits *Clone() = 0; //����һ������
};

typedef std::shared_ptr<IFrameUnits> IFrameUnitsPtr;
typedef std::queue<IFrameUnitsPtr> IFrameUnitsQueue;


//
//���ݰ�������
//
class IRtpSender
{
public:
  virtual ~IRtpSender() {}

  virtual int InitSession(std::string ip, 
    unsigned short port, 
    RtpPayloadType payload_type) = 0;
  virtual int DestroySession() = 0;

  virtual int Send(IFrameUnitsPtr p) = 0;
};


//
//֡���������һ֡���ݲ��Ϊ����ɷ��͵����ݰ���Ԫ
//

class IFrameSplitter
{
public:
  virtual ~IFrameSplitter() {}

  virtual IFrameUnitsPtr Split(const unsigned char *buf, 
    unsigned long size,
    long long timestamp) = 0;
};

#endif
