#ifndef _KINECT_SERVICE_H_
#define _KINECT_SERVICE_H_

#include <Qtcore/QtCore >
#include <dbus/kinect/common/KinectServiceCommon.h>
class CKinect;
class KinectService : public QObject
{
	Q_OBJECT
public:
	static KinectService*	getInstance()
	{
		static KinectService service;
		return &service;
	}

	/**
	 * @brief �����źŸ�ָ���Ľ���
	 * @param to �������ƣ�������н��̶��ܽ��գ���ô��дALL����Ϊ���ַ���""
	 * @param type �������ͣ��ο�KinectGestureö������
	 */
	void EmitGestureSignal( const QString&to , const KinectGesture type );

	/**
	 * @brief ����Kinect��ʾ��Ϣ
	 * @param type ��Ϣ���ͣ��ο�KinectMessageTypeö������
	 * @param message ��Ϣ����
	 */
	void EmitKinectMessage( const KinectMessageType type , const QString&message );

private:
	KinectService();
	~KinectService();  
	CKinect * kinect;
public: // PROPERTIES
	public Q_SLOTS: // METHODS
Q_SIGNALS: // SIGNALS
		void GestureOccur(const QString &to, int type);
		void KinectMessage(int type, const QString &message);
};
#endif