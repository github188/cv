#ifndef _TX_CLIENT_CONTROLLER_INTERFACE_H_
#define _TX_CLIENT_CONTROLLER_INTERFACE_H_

#include <QObject>

#include "msg_manager_interface.h"
#include "conference_controller_interface.h"
#include "util/report/RecordStat.h"


class ITXClientController : public QObject {
  Q_OBJECT

 public:
  ITXClientController(QObject *parent) : QObject(parent) {}
  virtual ~ITXClientController() {}

  virtual QString GenerateLocalRecordFilePath(int virtual_index) = 0;

 Q_SIGNALS:
  //Զ�̿������
  void SendRecordControlRequestSignal(const RecordControlParam &param); //�ѷ���

  //����¼��״̬
  void ReceiveUDPRecReportSignal(const RecStatItem &item);

 public Q_SLOTS: 
   //
   // �����ն˱���¼��ķ���
   //
   // ������Ƶ�ɼ�״̬�ı�ʱ��������Ҫ�����Զ�¼��
   virtual void HandleSendMediaStateChangedNotifySlot(const QString &media_id, int type) = 0;
   // ��������Ƶ¼�Ʒָ�����
   virtual void HandleSegmentLocalRecordVideoRequestSlot() = 0;
   // ����״̬���
   virtual void HandleConferenceStateChangedSlot(
     const QString &conference_uri, 
     IConferenceController::ConferenceState state) = 0;

   //
   // Զ��¼�������ط���
   //
   virtual void HandleOneVideoRecordRequestSlot(const QString &user_id, int type) = 0;
   virtual void HandleWholeVideoRecordRequestSlot(const QString &user_id, int type, QByteArray param) = 0;
   virtual void HandleRecvRecordControlAckNotifySlot(const RecordControlAckMsg &info) = 0;
};

#endif
