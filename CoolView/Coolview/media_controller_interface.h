#ifndef MEDIA_CONTROLLER_INTERFACE_H
#define MEDIA_CONTROLLER_INTERFACE_H

#include <QObject>
#include <QRect>

#include "util/report/RtpStat.h"
#include "util/report/RecordStat.h"

enum MediaControlType;
enum MediaStreamType;

class IMediaController : public QObject {
  Q_OBJECT
 public:
   IMediaController(QObject *parent) : QObject(parent) {}
   virtual ~IMediaController() {}

 Q_SIGNALS:

 public Q_SLOTS:
   // ����QoS��Ϣ��QoS������
   virtual void HandleSendRTPStatToQoSServerRequestSlot(
     const RtpStatItem& rtp_stat) = 0;

   // ����¼����Ϣ�������� - ��δʵ�ַ���
   virtual void HandleSendRecStatToServerRequestSlot(
     const RecStatItem& recstat) = 0;

   // ��ָ���û�����ý����
   // seat: ��Ƶ��������λ�ã�1��ʾ��һ�����ڣ��Դ�����
   virtual void HandleAddRecvStreamMediaRequestSlot(
     const QString &remote_vuri,
     int screen_index, 
     int seat,
     bool use_small_video) = 0;

   // ��֪Զ���û�ֹͣ������Ƶ��������
   virtual void HandleRemoveRecvStreamMediaRequestSlot(
     const QString &remote_vuri) = 0;

   // �ı�������ʾλ��
   // ���治Ӧֱ�ӵ��øú���
   virtual void HandleChangeVideoSeatSlot( 
     const QString &remote_uri,
     int screen_index,
     int seat) = 0;

   // �������ط��ͽ���
   virtual void HandleCreateSendStreamMediaRequestSlot() = 0;

   // ��֪���вλ��ն˵�ý�巢��״̬�������ж�ý�������п���
   // type: ý�����ͣ�ֻ֧��kAudioControlType��kVideoControlType
   // enable: �������ý��������Ϊtrue������Ϊfalse
   virtual void HandleControlSendMediaRequestSlot(
     MediaControlType type,
     bool enable) = 0;

   // �������͹�����Ļ��
   virtual void HandleCreateSendScreenMediaRequestSlot(const QRect &wnd) = 0;

   // �������չ�����Ļ��
   virtual void HandleCreateRecvScreenMediaRequestSlot(
     const QString &remote_uri, 
     int screen_index) = 0;

   // ��֪Զ���û�ֹͣ������Ļ��
   virtual void HandleRemoveRecvScreenMediaRequestSlot(
     const QString &remote_uri,
     int screen_index) = 0;

   // ���Ʊ�����Ļ������/ֹͣ
   // ͬʱ��֪�����ն˴���/���ٵ�ǰ�ն˵���Ļ�����ս���
   virtual void HandleScreenShareControlRequestSlot(bool enable) = 0;
   
   // �ָ�ý�����
   virtual void RecoveryMediaProcess(
     MediaStreamType type, 
     bool is_send, 
     const QString &remove_user_id) = 0;
};

#endif // MEDIA_CONTROLLER_INTERFACE_H