#ifndef _DISPLAY_CONTROLLER_INTERFACE_H_
#define _DISPLAY_CONTROLLER_INTERFACE_H_

#include <QObject>

#include "media_description.h"

class IDisplayController : public QObject {
  Q_OBJECT
public:
  enum ScreenLayout {
    // TODO: ���kLayoutOfUnknown��ת��
    kLayoutOfUnknown = -1,
    kLayoutOf1x1 = 0,
    kLayoutOf2x2 = 1,
    kLayoutOf3x3 = 2,
    kLayoutOf1plus5 = 3,
    kLayoutOfAuto = 11,
    kLayoutOfDocument = 10,
    kLayoutOfTop1x2 = 12,
    kLayoutOf4x4 = 13,
    kLayoutOfAirplay = 14,
  };

  static ScreenLayout ConvertToScreenLayout(int layout) {
    // TODO: ����ConferenceRoom::ConfRoomDisplayModelת��
    return static_cast<ScreenLayout>(layout);
  }

public:
  IDisplayController(QObject *parent) : QObject(parent) {}
  virtual ~IDisplayController() {}

  static void RegisterMetaType() {
    qRegisterMetaType<IDisplayController::ScreenLayout>(
      "IDisplayController::ScreenLayout");
  }

  virtual int GetScreenCount() const = 0;

Q_SIGNALS:
  // ��֪������vuri��Ƶ
  void NotifyReceiveVideoSignal(
    const QString &vuri,
    int screen_index, 
    int seat_index, 
    bool use_small_video);

  // ��ֹ֪ͣ�˽���vuri��Ƶ
  void NotifyStopVideoSignal(
    const QString &vuri,
    int screen_index,
    int seat_index);

  // ��֪uri��Ƶ�Ľ���״̬
  void NotifyReceiveAudioSignal(const QString &uri, bool enable);

  // ��֪������uri�Ĺ�����Ļ
  void NotifyReceiveDocumentSignal(const QString &uri, int screen_index);
  // ��ֹ֪ͣ��uri�Ĺ�����Ļ
  void NotifyStopDocumentSignal(const QString &uri, int screen_index);

  // ��֪RTCP����ʾ״̬����ʾ��ر�
  void NotifyShowRTCPMessageSignal(bool show, int screen_index);

  // ��֪������Ƶ�ķ���״̬
  void NotifySendLocalAudioSignal(bool enable);
  // ��֪������Ƶ�ķ���״̬
  // ��virtual_indexΪ-1ʱ��Ϊ���б�����Ƶ
  void NotifySendLocalVideoSignal(int virtual_index, bool enable);

  // ��֪���������˱仯
  void NotifyVoiceChangeSignal(AudioDeviceType type, int volume);

  // ��֪���������˱仯
  void NotifyMuteChangeSignal(AudioDeviceType type, bool is_mute);

  // ��֪uri�ն˱������˷���
  void NotifyTerminalSpeakSignal(const QString &uri, bool allow);

  // ��֪��Ļ���ַ����˱仯
  void NotifyScreenLayoutChangedSignal(
    int screen_index, 
    IDisplayController::ScreenLayout layout);

  // ���������ʾ�����ؿ������
  // �����������봦����ź�
  void SetPanelVisibleSignal(bool visible);

  // �����ȡ����visible��״̬
  // �����������봦����ź�
  void CheckPanelVisibleStateSignal(int telecontroller_index);

public Q_SLOTS:
  // ���������Ƶ����
  virtual void HandleReceiveVideoRequestSlot(
    const QString &vuri, 
    int screen_index, 
    int seat_index, 
    bool use_small_video) = 0;

  // �����Զ������ն�����Ƶ
  virtual void HandleRequestStreamMediaAutoReceiveSlot(
    const QString &remote_uri) = 0;

  // ����ֹͣ������Ƶ����
  virtual void HandleStopVideoRequestSlot(const QString &vuri) = 0;

  // ������պͶϿ���Ƶ����
  virtual void HandleReceiveAudioRequestSlot(
    const QString &uri, 
    bool enable) = 0;

  // ������չ�����Ļ����
  virtual void HandleReceiveDocumentRequestSlot(
    const QString &uri,
    int screen_index) = 0;

  // ����ֹͣ����ָ��uri�Ĺ�����Ļ����
  // ��uriΪ�գ���ֹͣ�������еĹ�����Ļ
  virtual void HandleStopDocumentRequestSlot(
    const QString &uri) = 0;

  // ������ع����ļ�����
  virtual void HandleOpenDocumentRequestSlot(
    const QString &filename,
    int screen_index) = 0;

  virtual void HandlePPTControlSlot(int type) = 0;

  // ������ʾ�͹ر�RTCP��Ϣ����
  virtual void HandleShowRTCPMessageRequestSlot(
    bool show, 
    int screen_index) = 0;

  // ���Ʊ�����Ƶ����ͣ
  virtual void HandleSendLocalAudioRequestSlot(bool enable) = 0;

  // ���Ʊ�����Ƶ����ͣ
  virtual void HandleSendLocalVideoRequestSlot(int virtual_index, bool enable) = 0;

  // �ı�speaker��mic������
  virtual void HandleVoiceChangeRequestSlot(
    AudioDeviceType type,
    int volume) = 0;

  // �ı�speaker��mic�ľ���״̬
  virtual void HandleVoiceMuteRequestSlot(
    AudioDeviceType type, 
    bool isMute) = 0;

  // ����ͽ�ֹ����
  virtual void HandlePermitSpeakRequestSlot(
    const QString &uri,
    bool allow) = 0;

  // ������Ļ��������
  virtual void HandleScreenLayoutRequestSlot(
    int screen_index, 
    IDisplayController::ScreenLayout layout) = 0;

  // ����ı���������ʾ״̬����Ҫ���͸��źŵ��˲�
  virtual void HandlePanelVisibleStateChangedNotifySlot(
    int telecontroller_index, bool visible) = 0;

  // �������б��ػ���
  virtual void HandleReceiveAllLocalVideoNotifySlot() = 0;
};


#endif // _DISPLAY_CONTROLLER_INTERFACE_H_