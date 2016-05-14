#pragma once
#include <QFileInfo>
#include <QStringList>

#include <queue>
#include <vector>
#include <functional>

class FileInfoComparer
{
public:
	inline bool operator()(QFileInfo a, QFileInfo b);
};

typedef std::priority_queue<QFileInfo, std::vector<QFileInfo>, FileInfoComparer> FileInfoQueue;


class FileScanner
{
	FileScanner(void);
	~FileScanner(void);

public:
  // @brief ��ָ��·����ȡ������Ҫ����ļ�����ʱ���Ⱥ�˳��������ȶ���
  // @param queue ���ض���
  // @param path  ·��
  // @param mark  ��������Ҫ��ȡ���ļ�����·��Ӧ������Щ��ǣ�""��ʾ���������ļ�������֮��Ϊor��ϵ
  // @param exclude  ��������Ҫ�ų����ļ�����·����ǣ�""��ʾ�չ�����������֮��Ϊor��ϵ
  // @param recursive �Ƿ�ݹ����
  // @remark exclude����������mark����һ���ļ�����Ⱥ���mark��ǣ�ͬʱ�ֺ���exclude��ǣ������ų�
	static void ScanFiles(FileInfoQueue &queue, const QString &path, 
    QStringList mark, QStringList exclude, bool recursive = true);

  static bool RemoveEmptyDirs(const QString &root, const QStringList &ignore);

};
