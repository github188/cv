#ifndef TX_DISPLAY_CONTROLLER_H
#define TX_DISPLAY_CONTROLLER_H

#include "display_controller.h"

class TXDisplayController : public DisplayController
{
  Q_OBJECT

public:
  TXDisplayController(QObject *parent, MediaManager *media_manager);
  ~TXDisplayController();

 public Q_SLOTS:
  // ���������Ƶ����
  void HandleReceiveVideoRequestSlot(
    const QString &vuri, 
    int screen_index, 
    int seat_index, 
    bool use_small_video) override;

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

  // ������ع����ļ�������
  void HandleOpenDocumentRequestSlot(
    const QString &filename, 
    int screen_index) override;

  // �ı䲼��
  void HandleScreenLayoutRequestSlot( 
    int screen_index, 
    IDisplayController::ScreenLayout layout) override;

private:
  ScreenLayout ValidateScreenLayout(int layout);

private:
  ScreenLayout screen_type_;
};

#endif // TX_DISPLAY_CONTROLLER_H
