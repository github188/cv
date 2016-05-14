#ifndef VOD_SERVICE_WRAPPER_H
#define VOD_SERVICE_WRAPPER_H

#include <QObject>

#include <memory>

class CvTxWebServiceImpl;
class ILogger;

//�������ڽ�SOAP������QObject��װ���Ա��ڵ�����QThread��ִ��
class TxWebServiceWrapper : public QObject
{
  Q_OBJECT

public:
  TxWebServiceWrapper(ILogger *logger, QObject *parent = nullptr);
  ~TxWebServiceWrapper();

  //ֹͣ���񣬻��ʱ������-�̰߳�ȫ��
  void StopService();

public Q_SLOTS:
  //���ô˷������������뽫��ʵ�����ڵ����߳��У���ͨ���źŲ۵���
  void HandleStartServiceSlot(int port);

private:
  std::shared_ptr<CvTxWebServiceImpl> service_;
  ILogger *logger_;
};

#endif // VOD_SERVICE_WRAPPER_H
