#ifndef TX_UTIL_H
#define TX_UTIL_H

#include <QString>

namespace tx_util {

extern const char *PathPlaceholderDate; //�ļ������ڣ�yy-MM-dd��ռλ��
extern const char *PathPlaceholderDateTime; //�ļ�������ʱ�䣨yyMMddhhmmss��ռλ��
extern const char *PathPlaceholderVInfo; //�ļ�����Ƶ��ʽռλ��
extern const char *PathPlaceholderCamIndex; //�ļ�����Ƶ����������ռλ��
extern const char *PathPlaceholderConf; //�ļ�����������ռλ��
extern const char *PathPlaceholderConfDate; //�ļ������鿪ʼ����ռλ��
extern const char *PathPlaceholderTerminal; //�ļ����ն�����ռλ��
extern const char *PathPlaceholderTitle; //�ļ�����Ƶ������ռλ��
extern const char *PathPlaceholderParticipant; //�ļ��������ռλ��
extern const char *PathPlaceholderKeywords; //�ļ����ؼ���ռλ��
extern const char *PathPlaceholderDescription; //�ļ�������ռλ������Ҫ����ʹ�ã��������ܺܳ���


struct FileNameParam {
  int camid; // ʵ����PathPlaceholderCamIndex
  QString conf_date;
  QString conf;
  QString terminal;
  QString title;
  QString participant;
  QString keywords;
  QString description;

  FileNameParam();
};
QString GetRecordFileNamePath(const QString &path_with_placeholder, 
                              const FileNameParam &param);

//���¼���ļ���Ŀ¼
QString TXGetRecordRootDir();

//��þ��Բ��ظ��ĸ߾���ʱ���������1970-1-1 00:00 UTC������΢����
long long GetNonrepeatableTimestamp();

template<typename T>
inline QByteArray DBusParamToByteArray(T &param)
{
  QByteArray bytes;
  QDataStream out(&bytes , QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_4_4);
  out << param;
  return bytes;
}

template<typename T>
inline T DBusParamFromByteArray(const QByteArray &bytes)
{
  T param;
  QDataStream in(bytes);
  in.setVersion(QDataStream::Qt_4_4);
  in >> param;
  return param;
}

}

#endif
