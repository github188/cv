#ifndef TERMINAL_DESCRIPTION_H
#define TERMINAL_DESCRIPTION_H

#include <memory>

#include <QObject>
#include <QString>
#include <QHash>
#include <QList>

#include <cassert>

struct TerminalDescription {

  // ���Ƚ�total_terminal_num
  bool IsSameWithoutTotalNumCompared(const TerminalDescription &terminals) const;
  
  // TODO: ���ն������������
  int total_terminal_num; // ������ն�����
  QString dns;  // ����������

  uint16_t id; // �ն�id�����ڷ���Ȩ����
  QString	uri; // �ն�uri
  QString  virtual_uri; // �ն˴��ڶ�����ͷʱ�������ն˺ţ���ʽ����v1#{uri}����v1��ʼ
  int virtual_index; // ���ֱ�ʾ�������ն���������0��ʼ
  int virtual_count; // ���ն��ܵ������ն���
  QString name;	// �ն�����
  QString location; // �ն�λ��
  QString model; // ����ģʽ
  QString version; // �ն˰汾
 
  QString multicast_address; // �鲥�����IP��ַ��������������0.0.0.0

  QString sdp; // sdp
  bool has_video;
  bool has_audio;
  int video_port; // ��Ƶ���ݴ���ı��ض˿ڣ�Ĭ��ֵΪ0
  int audio_port; // ��Ƶ���ݴ���ı��ض˿ڣ�Ĭ��ֵΪ0
  int small_video_port; // ��ƵС���Ĵ���˿ڣ�Ĭ��ֵΪ0

  bool is_available; // �ն�״̬���Ƿ���뵽������
  bool is_speaking; // �Ƿ�������
  bool is_handup; // �Ƿ����
  bool is_main_speaker; // �Ƿ���������
  bool is_chairman_terminal; // �Ƿ�����ϯ�ն�
};

typedef std::shared_ptr<TerminalDescription> TerminalPointer;
// TODO: ��conference controller��signal�������ConstTerminalPointer
typedef std::shared_ptr<const TerminalDescription> ConstTerminalPointer;

// virtual terminal list
typedef QList<TerminalPointer> TerminalList;
// key: vuri, value: terminal
typedef QHash<QString, TerminalPointer> TerminalDict;
// key: uri, value: terminal list
typedef QHash<QString, TerminalList> TerminalListDict;

class TerminalHelper {
public:
  static void RegisterMetaType() {
    qRegisterMetaType<TerminalDescription>("TerminalDescription");
    qRegisterMetaType<TerminalPointer>("TerminalPointer");
    qRegisterMetaType<ConstTerminalPointer>("ConstTerminalPointer");
    qRegisterMetaType<TerminalList>("TerminalList");
  }

  static QString ConstructDefaultVirtualURI(
      const QString &uri, int virtual_index = 0) {
    return QString("v%1#%2").arg(virtual_index + 1).arg(uri);
  }

  static bool IsVirtualURI(const QString uri_or_vuri) {
    return uri_or_vuri.contains('#');
  }

  static QString GetTerminalURI(const QString &uri_or_vuri) {
    return uri_or_vuri.mid(uri_or_vuri.indexOf('#') + 1);
  }

  static QString GetRealm(const QString &uri_or_vuri) {
    int at_index = uri_or_vuri.indexOf('@');
    if (at_index == -1) {
      return QString();
    }
    return uri_or_vuri.mid(at_index + 1);
  }

  static QString GetUsername(const QString &uri_or_vuri) {
    return uri_or_vuri.mid(0, uri_or_vuri.indexOf('@'));
  }

  static int GetVirtualIndex(const QString &vuri_or_user_id) {
    int virtual_index = 0;
    int sharp_pos = vuri_or_user_id.indexOf("#");
    if (sharp_pos > 1) {
      QString index_str = vuri_or_user_id.mid(1, sharp_pos - 1);
      virtual_index = index_str.toInt() - 1;
    }
    return virtual_index;
  }
  
  static void GetMediaInfoFromSDP(
    const QString &sdp, 
    int &video_width,
    int &video_height,
    int &video_fps,
    QString &audio_codec,
    int &audio_rate, 
    int &audio_bits,
    int &audio_channel);

  static bool IsModelHD(const QString &model) {
    return model.contains("HD");
  }

  static bool IsModelTX(const QString &model) {
    return model.contains("TX");
  }
};

#endif // TERMINAL_DESCRIPTION_H