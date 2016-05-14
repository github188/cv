#ifndef DISPLAY_CONTROLLER_H
#define DISPLAY_CONTROLLER_H

#include <QHash>
#include <QTime>
#include <QTimer>

#include "display_controller_interface.h"
#include "conference_controller_interface.h"

class MediaManager;
class TelecontrollerIf;

class DisplayController : public IDisplayController {
  Q_OBJECT

 public:
   DisplayController(QObject *parent, MediaManager *media_manager);

  virtual ~DisplayController();

  void Initialize(IConferenceController *conference_controller);

  int GetScreenCount() const override { return screen_count_; }

 public Q_SLOTS:
  // ���������Ƶ����
  void HandleReceiveVideoRequestSlot(
    const QString &vuri, 
    int screen_index, 
    int seat_index, 
    bool use_small_video) override;

  // �����Զ������ն�����Ƶ
  void HandleRequestStreamMediaAutoReceiveSlot(
    const QString &remote_uri) override;

  // ����ֹͣ������Ƶ����
  void HandleStopVideoRequestSlot(const QString &vuri) override;

  // ������պͶϿ���Ƶ����
  void HandleReceiveAudioRequestSlot(
    const QString &uri, 
    bool enable) override;

  // ������չ�����Ļ����
  void HandleReceiveDocumentRequestSlot(
    const QString &uri,
    int screen_index) override;

  // ����ֹͣ����ָ��uri�Ĺ�����Ļ����
  // ��uriΪ�գ���ֹͣ�������еĹ�����Ļ
  void HandleStopDocumentRequestSlot(
    const QString &uri) override;

  // ������ع����ļ�������
  void HandleOpenDocumentRequestSlot(
    const QString &filename, 
    int screen_index) override;

  void HandlePPTControlSlot(int type) override;

  // ������ʾ�͹ر�RTCP��Ϣ����
  void HandleShowRTCPMessageRequestSlot(
    bool show, 
    int screen_index) override;
 
  // ���Ʊ�����Ƶ����ͣ
  void HandleSendLocalAudioRequestSlot(bool enable) override;

  // ���Ʊ�����Ƶ����ͣ
  void HandleSendLocalVideoRequestSlot(int virtual_index, bool enable) override;

  // �ı�speaker��mic������
  void HandleVoiceChangeRequestSlot(
    AudioDeviceType type,
    int volume) override;

  void HandleVoiceMuteRequestSlot(
    AudioDeviceType type,
    bool isMute) override;

  // ����ͽ�ֹ����
  void HandlePermitSpeakRequestSlot(
    const QString &uri,
    bool allow) override;

  // �ı䲼��
  void HandleScreenLayoutRequestSlot( 
    int screen_index, 
    IDisplayController::ScreenLayout layout) override;

  // ����������״̬�ı��֪ͨ
  void HandlePanelVisibleStateChangedNotifySlot(
    int telecontroller_index, 
    bool visible) override;

  // �������б��ػ���
  void HandleReceiveAllLocalVideoNotifySlot() override;

  //��ջ��������Ƶ�б�
  void HandleClearAudioDisplayDictNotifySlot();

 protected Q_SLOTS:
   // TODO: �Ƴ������Ϊ���յײ������Ƶ��ص�֪ͨ
   // ��Զ���ն��˳���ʱ��ֹͣ��ص���Ƶ��UI��ʾ
  void HandleTerminalUpdateNotifySlot(
    ConstTerminalPointer terminal,
    const QString &conference_uri);

  // TODO: �Ƴ������Ϊ���յײ������Ƶ��ص�֪ͨ
  // ���˳�����ʱ��ֹͣ�����Ƶ��UI��ʾ
  void HandleConferenceStateChangedNotifySlot(
    const QString &conference_uri, 
    IConferenceController::ConferenceState state);

  // ����ɹ����չ�����Ļ���ź�
  void HandleScreenReceiveStartedNotifySlot(
    const QString &remote_uri, 
    int screen_index);

  // ����ɹ�ֹͣ���չ�����Ļ���ź�
  void HandleScreenReceiveStoppedNotifySlot(
    const QString &remote_uri,
    int screen_index);

  // ����Ƿ��������Է�����ý����������ʱ
  void HandleCheckReceiveVideoAckSlot();

  // �յ����Ͷ˵�ȷ�ϣ�����ack״̬
  void HandleStreamMediaReceiveReplyNotifySlot(
    const QString &remote_vuri, bool permission);


 protected:
  struct VideoSeatInfo {
    QString terminal_vuri;
    int screen_index;
    int seat_index;
    bool use_small_video;
    bool is_ack; // �Ƿ��յ����Ͷ�ȷ����Ϣ�����ڳ�ʱ���
    QTime ack_time_counter; // �ӿ�ʼ���󵽵�ǰ��ʱ������������ڳ�ʱ���
  };

  struct SharedScreenSeatInfo {
    QString terminal_uri;
    int screen_index;
  };

  struct AudioSeatInfo {
    QString terminal_vuri;
    bool is_audio_enabled;
    bool is_audio_received;
  };

  typedef QHash<QString, VideoSeatInfo> DisplayDict;
  typedef QHash<QString, QList<AudioSeatInfo>> DisplayAudioDict;
  typedef QList<SharedScreenSeatInfo> SharedScreenList;
  typedef QList<IDisplayController::ScreenLayout> ScreenLayoutVector;

  void AddVideoSeat(
    const QString &vuri, 
    int screen_index, 
    int seat_index, 
    bool use_small_video);

  VideoSeatInfo* FindVideoSeat(const QString &vuri);
  VideoSeatInfo* FindVideoSeat(int screen_index, int seat_index);
  int FindFreeVideoSeat(int screen_index);
  int GetVideoSeatCount(int screen_index);

  void RearrangeAudioRender(const QString &id);
  
  SharedScreenSeatInfo* FindSharedScreenSeat(const QString &uri);
  void RemoveSharedScreenSeat(const QString &uri);

  bool IsLocalTerminalURI(const QString &vuri) const;

 protected:
  MediaManager *media_manager_;
  // key: id(vuri or localpreviewid), value: video seat
  DisplayDict video_display_dict_;
  // key: uri, value: QList<AudioSeatInfo>
  DisplayAudioDict audio_display_dict_;
  SharedScreenList shared_screen_list_;
  IConferenceController *conference_controller_;
  TelecontrollerIf *telecontroller_proxy_;
  int screen_count_;
  ScreenLayoutVector screen_layout_;

  QString current_conference_uri_;
  bool has_initialized_;
  bool is_model_hd_;

  // ���ڼ���Ƿ��������Է�����ý����������ʱ�ļ�ʱ��
  QTimer *check_receive_video_ack_timer_;
};

#endif // DISPLAY_CONTROLLER_H
