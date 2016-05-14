#ifndef STREAM_RECEIVER_INFO_MANAGER_H
#define STREAM_RECEIVER_INFO_MANAGER_H

#include <QString>
#include <QList>
#include <QHash>

struct StreamReceiverInfo {
  StreamReceiverInfo()
      : video_port(0),
        audio_port(0),
        screen_port(0),
        is_small_video(false),
        local_virtual_index(0) {
  }

  QString recv_uri; // �����ߵ�uri
  QString recv_ip;  // �����ߵ�ip
  int video_port;   // ��Ƶ���ݴ���˿�
  int audio_port;   // ��Ƶ���ݴ���˿�
  bool is_small_video; // �Ƿ�С��
  int local_virtual_index; // ���ط��ͽ��̵������ն�����
  int screen_port;  // ��Ļ����˿�
};

class StreamReceiverInfoManager {

public:
  StreamReceiverInfoManager() {}
  ~StreamReceiverInfoManager() { Clear(); }

  void AddRemoteReceiver(const StreamReceiverInfo &info);

  void RemoveReceiverByURI(const QString &recv_uri, int local_virtual_index);

  void RemoveReceiverByIP(const QString &recv_ip, int local_virtual_index);

  StreamReceiverInfo* FindRemoteReceiverByURI(
    const QString &recv_uri, 
    int local_virtual_index);

  StreamReceiverInfo* FindRemoteReceiverByIP(
    const QString &ip,
    int local_virtual_index);

  int GetReceiverCount() const;

  void Clear();

  typedef QList<StreamReceiverInfo> ReceiverList;
  ReceiverList ToList() const;

private:
  typedef QHash<int, StreamReceiverInfo*> VIndexReceiversDict;
  typedef QHash<QString, VIndexReceiversDict> URIReceiversDict;

  URIReceiversDict dict_;
};

#endif // STREAM_RECEIVER_INFO_MANAGER_H
