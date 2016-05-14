#include "fileplay_ctrl_widget.h"
#include "fileplay_controller_interface.h"

#include <QMouseEvent>

#include "assert.h"

FilePlayCtrlWidget::FilePlayCtrlWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    duration_in_sec_ = 0;
    paused_ = false;
}

FilePlayCtrlWidget::~FilePlayCtrlWidget()
{

}

void FilePlayCtrlWidget::Initialize( IFilePlayController *fileplay_controller )
{
    assert(fileplay_controller);
    fileplay_controller_ = fileplay_controller;

    connect(fileplay_controller_, &IFilePlayController::NotifyPlayProgressSignal,
        this, &FilePlayCtrlWidget::HandleNotifyPlayProgressSlot);

    // pause button
    connect(ui.pause_button, &QPushButton::clicked, 
        this, &FilePlayCtrlWidget::HandlePauseButtonClickedSlot);

    // slider
    ui.seek_slider->installEventFilter(this);
}

void FilePlayCtrlWidget::Reset()
{
    duration_in_sec_ = 0;
    paused_ = false;
    ui.time_label->setText("00:00:00/00:00:00");
    is_dragging_progressbar_ = false;
    is_seeking_ = false;
}

void FilePlayCtrlWidget::HandleNotifyPlayProgressSlot( 
    unsigned long position, unsigned long duration )
{
    if (is_seeking_) return;
    // save duration
    duration_in_sec_ = duration;
    SetPosition(position);
}

void FilePlayCtrlWidget::SetPosition( unsigned long position )
{
    // update label text
    struct {
        unsigned long hour;
        unsigned long min;
        unsigned long sec;
    } pos, dur;
    pos.hour = position / 3600;
    pos.min = (position % 3600) / 60;
    pos.sec = position % 60;
    dur.hour = duration_in_sec_ / 3600;
    dur.min = (duration_in_sec_ % 3600) / 60;
    dur.sec = duration_in_sec_ % 60;
    QString time_string = QString("%1:%2:%3/%4:%5:%6").
        arg(pos.hour, 2, 10, QChar('0')).arg(pos.min, 2, 10, QChar('0')).arg(pos.sec, 2, 10, QChar('0')).
        arg(dur.hour, 2, 10, QChar('0')).arg(dur.min, 2, 10, QChar('0')).arg(dur.sec, 2, 10, QChar('0'));
    ui.time_label->setText(time_string);

    //update slider
    if (!ui.seek_slider->isSliderDown()) {
        int slider_value = ((double)position) / duration_in_sec_ * ui.seek_slider->maximum();
        ui.seek_slider->setValue(slider_value);
    }
}

void FilePlayCtrlWidget::HandlePauseButtonClickedSlot()
{
    paused_ = !paused_;
    if (paused_) fileplay_controller_->HandlePauseSlot();
    else fileplay_controller_->HandlePlaySlot();
}

bool FilePlayCtrlWidget::eventFilter( QObject *watched, QEvent *event )
{
  if (watched == ui.seek_slider) {
    QSlider *slider = static_cast<QSlider *>(watched);
    const int max = slider->maximum();
    const int min = slider->minimum();
    const int handler_width = 10; //������
    const int width = slider->width() - handler_width; //��ϸ�۲��֪��ΪʲôҪ����������

    if (event->type() == QEvent::MouseButtonPress && 
        ((QMouseEvent *)event)->button() == Qt::LeftButton) {
      is_seeking_ = true; //������ڶ�λ����ֹ���ȱ�����½�������ɳ�ͻ
      int x = ((QMouseEvent *)event)->x();
      if (x > width) x = width;
      const int dest_pos = (double)x / width * (max - min) + min;
      const int handler_range = (double)handler_width / width * (max - min);
      if(dest_pos - slider->value() < 0 || dest_pos - slider->value() > handler_range) {
        //��갴���Ҳ��ڻ����ϣ���ֱ�Ӷ�λ
        slider->setValue(dest_pos);
        is_dragging_progressbar_ = false;
        return true;
      } else {
        //����ڻ����ϣ���Ϊ��ק
        is_dragging_progressbar_ = true;
        dragging_value_ = slider->value();
      }
    }
    else if (event->type() == QEvent::MouseButtonRelease && 
             ((QMouseEvent *)event)->button() == Qt::LeftButton) {
      int value = slider->value();
      if (is_dragging_progressbar_) {
        value = dragging_value_; //�϶�ʱ���ɿ���˲�������ܻ�΢΢�ƶ�����ʱ���û�����ϵ���ֵΪ׼
      }
      int length = max - min;
      if (length > 0) {
        unsigned long seek_sec = ((double)value)/length * duration_in_sec_; 
        fileplay_controller_->HandleSeekSlot(seek_sec);
        SetPosition(seek_sec);
      }
      is_seeking_ = false; //������ڶ�λ���
    }
    else if (event->type() == QEvent::MouseMove && is_dragging_progressbar_) {
      int value = slider->value();
      int length = max - min;
      if (length > 0) {
        unsigned long seek_sec = ((double)value)/length * duration_in_sec_; 
        SetPosition(seek_sec);
        dragging_value_ = value;
      }
    }
  }
  // pass the event on to the parent class
  return QObject::eventFilter(watched, event);
}
