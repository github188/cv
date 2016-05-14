#ifndef CONFERENCE_DESCRIPTION_H
#define CONFERENCE_DESCRIPTION_H

#include <memory>

#include <QString>
#include <QHash>
#include <QList>

struct ConferenceDescription {
  QString cid;         //�����id��ȫ��Ψһ
  QString uri;
  QString title;
  QString description;
  QString host_uri;		//������URI
  QString control_mode;	//����ģʽ��discuss��report
  QString join_mode;

  // TODO: change QString to char[]
  QString join_password;

  QString start_time;
  QString duration;

  // ����������鲥���飬����Ϊ�鲥��ַ������Ϊ0.0.0.0  
  // ���ڵ�ǰ���Ʒ����������ն˵Ļ����б�û�иõ�ַ��Ϣ��
  // ����multicast_address�ֶο���Ϊ�գ�ʹ��ʱ��ע�⡣
  QString multicast_address;	
  QString level; //���鼶��
  QString chairman;	//��ϯ�˺�
  int ppt_port;	//������Ļ����

  bool use_FCS; // �Ƿ�ʹ�÷���Ȩ����
};

typedef std::shared_ptr<ConferenceDescription> ConferencePointer;
typedef QHash<QString, ConferencePointer> ConferenceDictionary;
typedef QList<ConferencePointer> ConferenceList;

typedef std::shared_ptr<const ConferenceDescription> ConstConferencePointer;
typedef QList<ConstConferencePointer> ConstConferenceList;
typedef QHash<QString, ConstConferencePointer> ConstConferenceDictionary;

class ConferenceHelper {
public:
  static void RegisterMetaType() {
    qRegisterMetaType<ConferenceDescription>("ConferenceDescription");
    qRegisterMetaType<ConferencePointer>("ConferencePointer");
    qRegisterMetaType<ConstConferencePointer>("ConstConferencePointer");
    qRegisterMetaType<ConferenceList>("ConferenceList");
    qRegisterMetaType<ConstConferenceList>("ConstConferenceList");
  }

  static ConferencePointer GetVodPlayVirtualConference() {
    ConferencePointer vod_play_virtual_conference = ConferencePointer(new ConferenceDescription);
    vod_play_virtual_conference->cid = "2147483647"; //2^31-1������ܸò���ͷ��������ݿ��е�ֵ��ͻ��
    vod_play_virtual_conference->uri = "vod0@local"; //����ʹ��focus0���������ʵ�����ͻ
    vod_play_virtual_conference->title = QString::fromLocal8Bit("�ļ��㲥");
    vod_play_virtual_conference->description = QString::fromLocal8Bit("������飬���ڻؿ���¼�Ƶ���Ƶ");
    vod_play_virtual_conference->use_FCS = false;
    return vod_play_virtual_conference;
  }

  static bool IsVirtualConference(const QString &uri) {
      return !uri.startsWith("focus");
  }

  static bool IsVodVirtualConference(const QString &uri) {
      return uri.startsWith("vod");
  }
};

#endif // CONFERENCE_DESCRIPTION_H