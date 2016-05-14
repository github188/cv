
#pragma once

#include <string>
#include <windows.h>
#include <QtCore>

std::wstring str2wstr(const std::string& str);
std::string wstr2str(const std::wstring& wstr);

BOOL PathExist(std::string strRawPath);	// �������·�������·��
BOOL ParentDirExist(const std::string& strFilePath);	// �ж���ʼ�����һ��'\\'λ�õ�·���Ƿ����
BOOL MakeDirRecursive(std::string strDirPath);	// ��'\\'��β���ļ�����Ի����·��
BOOL MakeParentDir(const std::string& strFilePath);	// ��������ʼ�����һ��'\\'λ�õ�·��
BOOL GetApplicationDirPath(char * applicationDirPath= NULL, char * applicationName= NULL);	// �����'\\'�����ĵ�ǰ·��
std::string GetAbsolutePath(const std::string& strRelative);	// �������·�������ؾ���·��
QString GetAbsolutePath(const QString &strRelative);
std::string GetTempFilePath(const std::string& strPath); // �����ļ���Ի����·�������������ʱ�ļ�·��
QString GetRelativePath(const QString &prefix, QString path);
QString GetDBFilePath(const QString &path); //�������·�����������ݿ��д洢�����·����ʽ
time_t SystemTimeToTimet( const SYSTEMTIME &st );

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
