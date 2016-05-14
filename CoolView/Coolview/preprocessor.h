#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <QObject>

class QThread;
class Launcher;

class Preprocessor : public QObject
{
	Q_OBJECT

public:
	Preprocessor(QObject *parent);
	~Preprocessor();

	void Initialize(Launcher *launcher);
	void Start();

protected slots:
	void Preprocess();
	void Launch();

signals:
	void StartedSignal();
	void FinishedSignal();
    void ShowMsgSignal(const QString &msg);

private:
	void ShowPendingWidget();
	void CheckIP();
    void CheckVideoDevice();

private:
	QThread *work_thread_;
	Launcher *launcher_;
    QWidget *pending_widget_;

  bool ip_valid_; //��¼IP�����
  bool video_valid_; //��Ƶ�豸��Ч��
};

#endif // PREPROCESSOR_H
