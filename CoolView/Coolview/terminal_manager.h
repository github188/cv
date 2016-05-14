#ifndef TERMINAL_MANAGER_H
#define TERMINAL_MANAGER_H

#include "terminal_description.h"

class TerminalManager {
public:
  enum ActionType {
    kNoAction = 0,     // ��ִ�ж���
    kInsertAction = 1, // ִ�в��붯��
    kUpdateAction = 2, // ִ�и��¶���
  };

public:
  TerminalManager();
  ~TerminalManager();
  
  // ���������ն���Ϣ
  // ����ִ�еĶ���
  // note: ��������ͬ���նˣ�����Ϣ��ͬ���򷵻�kNoAction
  ActionType AddTerminal(TerminalPointer terminal, bool use_FCS);

  // ���������uri���򽫷���virtual indexΪ0�������ն���Ϣ
  // ��û���ҵ����ն˵���Ϣ���򷵻�nullptr
  TerminalPointer FindTerminal(const QString &uri_or_vuri) const;

  // ����user_id����uri��@��ǰ���֡�
  // ��û�и��ն˵���Ϣ���򷵻�nullptr
  TerminalPointer FindTerminalByUserId(const QString &user_id) const;

  TerminalPointer FindTerminalById(uint16_t id) const;

  // ͨ�������ն˵���Ƶ�˿ڲ��������ն���Ϣ
  // ��û���ҵ����ն˵���Ϣ���򷵻�nullptr
  TerminalPointer FindTerminalByVideoPort(int video_port) const;

  // ͨ�������ն˵���Ƶ�˿ڲ��������ն���Ϣ
  // ��û���ҵ����ն˵���Ϣ���򷵻�nullptr
  TerminalPointer FindTerminalByAudioPort(int audio_port) const;

  // ��ȡTx�ն�
  TerminalPointer FindAvailableTxTerminal() const;

  // �Ƿ����uri��Ӧ���ն�
  bool ContainTerminal(const QString &uri) const {
    return terminals_.find(uri) != terminals_.end();
  }

  // ��ȡuri��Ӧ�����������ն�
  TerminalList& operator[](const QString &uri) {
    return terminals_[uri];
  }

  // ��ȡuri��Ӧ�����������ն�
  TerminalList operator[](const QString &uri) const {
    return terminals_[uri];
  }
  
  // ��uri�Ĳ�ͬ�������ն�����
  int GetTerminalCount() const;

  // ���������ն�����
  int GetVirtualTerminalCount() const;

  // ��uri�����ض�Ӧ�������ն�����
  int GetVirtualTerminalCount(const QString &uri) const;

  // ����Ŀǰ��uri���Ƿ����е������ն˶��Ѿ���¼��
  bool IsAllVirtualTerminalReceived() const;
  
  // ����ն��б�
  void Clear() {
    terminals_.clear();
  }

private:
  TerminalListDict terminals_;
};

#endif // TERMINAL_MANAGER_H
