#ifndef QOS_DESCRIPTION_H
#define QOS_DESCRIPTION_H

#include <QString>

// ������������Ϣ����Ϣ����
struct LoginPermissionInfo {
  QString user_uri;
  int dscp;
  int rate;
};

// ��ֹ���������Ϣ����Ϣ����
struct LoginRejectionInfo {
  QString user_uri;
  QString description;
  int suggested_rate;
};

// QoS��������Ϣ
struct QoSServerInfo {
  QString sip_uri;	
  QString ip;
  int tcp_port;
  int simulate_test_udp_port; // QoS����������ģ����Ա���˿�
  int operation_udp_port;     // QoS���������ձ���˿�
};

#endif // QOS_DESCRIPTION_H