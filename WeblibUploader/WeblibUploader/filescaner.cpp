#include "filescaner.h"
#include "connectionmanager.h"
#include "weblibsetting.h"
#include <QDir>
#include <QDebug>
#include <QStringList>
#include <QEventLoop>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTextCodec>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>

FileScaner::FileScaner()
{
	m_manager = NULL;
	m_scaning = false;
	m_timer = new QTimer();
	m_timer->setInterval(FILE_SCAN_INTERVAL);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(slotFileScan()));
}

FileScaner::~FileScaner()
{
	if(m_timer != NULL)
	{
		m_timer->stop();
		delete m_timer;
		m_timer = NULL;
	}
}

void FileScaner::start()
{
	slotFileScan();
	m_timer->start();
}

void FileScaner::stop()
{
	m_timer->stop();
}

QList<WebLibDirStruct *> *FileScaner::getRootDirList(int parentid)
{
	QString basedir(RELATIVE_FILE_PATH);
	if(basedir.startsWith("../"))
	{
		basedir = basedir.right(basedir.length() - 2);
		QDir curdir = QApplication::applicationDirPath();
		curdir.cdUp();
		basedir = curdir.path() + basedir;
	}
	else if(basedir.startsWith("/"))
	{
		basedir = QApplication::applicationDirPath() + basedir;
	}

	QDir rootdir(basedir);
	if(!rootdir.exists())
	{
		qDebug() << "Root Dir Not Exists";
		return NULL;
	}

	QList<WebLibDirStruct *> *list = new QList<WebLibDirStruct *>();

	QStringList *p = NULL;
	QStringList *dirlist = new QStringList();
	QStringList *tmplist = new QStringList();
	dirlist->append(rootdir.path());
	for(int i=0; i < SCAN_DIR_LEVEL; i++)
	{
		for(int j=0; j<dirlist->size(); j++)
		{
			QDir curdir(dirlist->at(j));
			if(!curdir.exists())
				continue;
			QStringList subdirlist = curdir.entryList(QDir::NoDotAndDotDot | QDir::AllDirs);
			for(int k=0; k<subdirlist.size(); k++)
				tmplist->append(dirlist->at(j) + "/" + subdirlist.at(k));
		}
		p = dirlist;
		dirlist = tmplist;
		tmplist = p;
		tmplist->clear();
	}
	
	for(int i=0; i < dirlist->size(); i++)
		list->append(new WebLibDirStruct(dirlist->at(i), parentid, WEBLIB_GROUPID, parentid));

	delete tmplist;
	delete dirlist;
	return list;
}

qint64 FileScaner::getWeblibFreeSpace(int groupid)
{
	if(m_manager == NULL)
		return -1;

	QUrl url(WEBLIB_BASEURL + "/group/getResourceSize.action");
	QByteArray param(QString("groupId=%1").arg(groupid).toLatin1());
	QNetworkRequest request(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	QNetworkReply *reply = m_manager->post(request, param);
	QEventLoop loop;
	connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	loop.exec();
	disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	if(reply->error())
	{
		if(!reply->isFinished())
		{
			emit error(QString::fromLocal8Bit("��ȡWeblibʣ��������ʱ"));
		}
		else
		{
			QJsonParseError ok;
			QJsonObject result = QJsonDocument::fromJson(reply->readAll(), &ok).object();
			if(ok.error == QJsonParseError::NoError)
				emit error(result["detail"].toString());
			else
				emit error(reply->errorString());
		}
		reply->deleteLater();
		return -1;
	}

	QJsonParseError ok;
	QJsonObject result = QJsonDocument::fromJson(reply->readAll(), &ok).object();
	reply->deleteLater();
	if(ok.error != QJsonParseError::NoError)
		return -1;

	return (result["totalSize"].toDouble() - result["resourcesSize"].toDouble()) * 1024;
}


qint64 FileScaner::getWeblibSizeLimit(int groupid)
{
	if(m_manager == NULL)
		return -1;

	QUrl url(WEBLIB_BASEURL + "/group/getResourceSizeLimit.action");
	QByteArray param(QString("groupId=%1").arg(groupid).toLatin1());
	QNetworkRequest request(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	QNetworkReply *reply = m_manager->post(request, param);
	QEventLoop loop;
	connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	loop.exec();
	disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	if(reply->error())
	{
		if(!reply->isFinished())
		{
			emit error(QString::fromLocal8Bit("��ȡWeblib���ļ���С���Ƴ�ʱ"));
		}
		else
		{
			QJsonParseError ok;
			QJsonObject result = QJsonDocument::fromJson(reply->readAll(), &ok).object();
			if(ok.error == QJsonParseError::NoError)
				emit error(result["detail"].toString());
			else
				emit error(reply->errorString());
		}
		reply->deleteLater();
		return -1;
	}

	QJsonParseError ok;
	QJsonObject result = QJsonDocument::fromJson(reply->readAll(), &ok).object();
	reply->deleteLater();
	if(ok.error != QJsonParseError::NoError)
		return -1;

	return result["singleSize"].toDouble() * 1024;
}

QList<int> *FileScaner::getWeblibIdList(int parentid)
{
	if(m_manager == NULL)
		return NULL;

	QUrl url(WEBLIB_BASEURL + "/group/getResources.action");
	QByteArray param(QString("parentId=%1&type=tree").arg(parentid).toLatin1());
	QNetworkRequest request(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	QNetworkReply *reply = m_manager->post(request, param);
	QEventLoop loop;
	connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	QTimer::singleShot(CREATE_DIR_TIMEOUT, &loop, SLOT(quit()));
	loop.exec();
	disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	if(reply->error() || !reply->isFinished())
	{
		if(!reply->isFinished())
		{
			emit error(QString::fromLocal8Bit("��ȡĿ¼��Դ�б�ʱ"));
		}
		else
		{
			QJsonParseError ok;
			QJsonObject result = QJsonDocument::fromJson(reply->readAll(), &ok).object();

			if(ok.error == QJsonParseError::NoError)
				emit error(result["detail"].toString());
			else
				emit error(reply->errorString());
		}
		reply->deleteLater();
		return NULL;
	}

	QJsonParseError ok;
	QJsonArray list = QJsonDocument::fromJson(reply->readAll(), &ok).array();
	reply->deleteLater();

	if(ok.error != QJsonParseError::NoError)
		return NULL;

	QList<int> *idlist = new QList<int>();
	for(int i=0; i<list.size(); i++)
	{
		QJsonObject item = list.at(i).toObject();
		idlist->append((int)item["id"].toDouble());
	}

	return idlist;
}

//�������·������Ŀ¼��
int FileScaner::createWeblibDir(int groupid, int parentid, const QString &dirpath)
{
	if(m_manager == NULL)
		return -1;

	QDir dirinfo(dirpath);
	if(!dirinfo.exists())
		return -1;

	qDebug() << "create dir: " << groupid << ""  << parentid << " " << dirpath;
	QUrl url(WEBLIB_BASEURL + "/group/createDir.action");
	QByteArray param(QString("groupId=%1&parentId=%2&name=%3").arg(groupid).arg(parentid).arg(dirinfo.dirName()).toUtf8());
	QNetworkRequest request(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	QNetworkReply *reply = m_manager->post(request, param);
	QEventLoop loop;
	connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	QTimer::singleShot(CREATE_DIR_TIMEOUT, &loop, SLOT(quit()));
	loop.exec();
	disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	if(reply->error() || !reply->isFinished())
	{
		if(!reply->isFinished())
		{
			emit error(QString::fromLocal8Bit("�����ļ��г�ʱ"));
		}
		else
		{
			QJsonParseError ok;
			QJsonObject result = QJsonDocument::fromJson(reply->readAll(), &ok).object();

			if(ok.error == QJsonParseError::NoError)
				emit error(result["detail"].toString());
			else
				emit error(reply->errorString());
		}
		reply->deleteLater();
		return -1;
	}

	QJsonParseError ok;
	QJsonObject obj = QJsonDocument::fromJson(reply->readAll(), &ok).object();
	reply->deleteLater();
	if(ok.error != QJsonParseError::NoError)
		return -1;

	//����id��¼�ļ�
	QFile idfile(dirpath + "/" + WEBLIB_TMP_FILENAME);
	if(!idfile.open(QIODevice::ReadWrite | QIODevice::Truncate))
		return -1;
	idfile.write(QByteArray::number((int)obj["id"].toDouble()));
	idfile.close();

	return (int)obj["id"].toDouble();
}

QMap<QString, QList<int> *> *FileScaner::getLocalDirInfo(QList<WebLibDirStruct *> *rootlist)
{
	if(rootlist == NULL)
		return NULL;

	QMap<QString, QList<int> *> *idlist = new QMap<QString, QList<int> *>();
	QStack<QDir> sublist;
	int rootdirlen = 0;
	int id;
	for(int i=0; i<rootlist->size(); i++)
	{
		QDir dirinfo(rootlist->at(i)->dirPath);
		rootdirlen = dirinfo.path().length();
		sublist.push(dirinfo);
		while(!sublist.isEmpty())
		{
			QDir info = sublist.pop();
			QFile file(info.path() + "/" + WEBLIB_TMP_FILENAME);
			id = -1;
			if(file.exists())
			{
				QString relpath = info.path().right(info.path().length() - rootdirlen);
				file.open(QIODevice::ReadOnly);
				id = file.readAll().toInt();
				file.close();
				if(id >= 0)
				{
					if(!idlist->contains(relpath))
					{
						QList<int> *idset = new QList<int>();
						idlist->insert(relpath, idset);
					}
					(*idlist)[relpath]->append(id);
				}
			}
			QStringList subdirlist = info.entryList(QDir::NoDotAndDotDot | QDir::AllDirs);
			foreach (QString dname, subdirlist)
				sublist.push(info.path() + "/" + dname);
		}
	}
	return idlist;
}

int FileScaner::createUserDir(int groupid, const QString &dirname)
{
	if(m_manager == NULL)
		return -1;

	QUrl url(WEBLIB_BASEURL + "/group/createDir.action");
	QByteArray param(QString("groupId=%1&parentId=0&name=%3").arg(groupid).arg(dirname).toUtf8());
	QNetworkRequest request(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	QNetworkReply *reply = m_manager->post(request, param);
	QEventLoop loop;
	connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	QTimer::singleShot(CREATE_DIR_TIMEOUT, &loop, SLOT(quit()));
	loop.exec();
	disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	if(reply->error() || !reply->isFinished())
	{
		if(!reply->isFinished())
		{
			emit error(QString::fromLocal8Bit("�����û��ļ��г�ʱ"));
		}
		else
		{
			QJsonParseError ok;
			QJsonObject result = QJsonDocument::fromJson(reply->readAll(), &ok).object();
			if(ok.error == QJsonParseError::NoError)
				emit error(result["detail"].toString());
			else
				emit error(reply->errorString());
		}
		reply->deleteLater();
		return -1;
	}

	QJsonParseError ok;
	QJsonObject result = QJsonDocument::fromJson(reply->readAll(), &ok).object();
	reply->deleteLater();
	if(ok.error != QJsonParseError::NoError)
		return -1;

	return (int)result["id"].toDouble();
}

bool FileScaner::checkUserDir(int &dirId)
{
	if(m_manager == NULL)
	{
		dirId = -1;
		return false;
	}

	QUrl url(WEBLIB_BASEURL + "/group/getResources.action");
	QByteArray param(QString("parentId=%1&type=tree").arg(-WEBLIB_GROUPID).toLatin1());
	QNetworkRequest request(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	QNetworkReply *reply = m_manager->post(request, param);
	QEventLoop loop;
	connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	QTimer::singleShot(CREATE_DIR_TIMEOUT, &loop, SLOT(quit()));
	loop.exec();
	disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	if(reply->error() || !reply->isFinished())
	{
		if(!reply->isFinished())
		{
			emit error(QString::fromLocal8Bit("��ȡĿ¼��Դ�б�ʱ"));
		}
		else
		{
			QJsonParseError ok;
			QJsonObject result = QJsonDocument::fromJson(reply->readAll(), &ok).object();
			if(ok.error == QJsonParseError::NoError)
				emit error(result["detail"].toString());
			else
				emit error(reply->errorString());
		}
		reply->deleteLater();
		dirId = -1;
		return false;
	}

	QJsonParseError ok;
	QJsonArray list = QJsonDocument::fromJson(reply->readAll(), &ok).array();
	reply->deleteLater();

	if(ok.error != QJsonParseError::NoError)
	{
		dirId = -1;
		return false;
	}

	//�ҵ��û��ļ���
	for(int i=0; i<list.size(); i++)
	{
		QJsonObject item = list.at(i).toObject();
		if(item["text"].toString() == WEBLIB_ACCOUNT)
		{
			dirId = (int)item["id"].toDouble();
			return true;
		}
	}

	dirId = -1;
	return true;
}

//��������˳�򹹽�Ŀ¼�����ӵ����ϲ���parentid��Ϊ0�ĸ��ڵ㣬�Ӹýڵ㿪ʼ������
//������Ŀ¼Ϊֹparentid��ȻΪ-1�����Ը�Ŀ¼Ϊ�����д�������ʱparentidΪrootid
int FileScaner::checkAndCreateWeblibDirTree(int groupid, int parentid, const QString &dirpath, int rootid, int rootlen, QMap<QString, int> *idlist)
{
	if(idlist == NULL)
		return -1;

	QList<WebLibDirStruct *> dirlist;
	dirlist.append(new WebLibDirStruct(dirpath, parentid, groupid, -1));
	QString curpath = dirpath;
	//�����ҵ�Ŀ¼parentid��Ϊ-1ʱ��ֹͣ
	while(parentid == -1)
	{
		int index = curpath.lastIndexOf("/");
		if(index >= 0 && index > rootlen)
		{
			curpath = curpath.left(index);
			QString prelpath = curpath.right(curpath.length() - rootlen);
			if(idlist->contains(prelpath))
				parentid = (*idlist)[prelpath];
			else
				parentid = -1;
		}
		else
			parentid = rootid;
		
		if(parentid == -1)
			dirlist.append(new WebLibDirStruct(curpath, parentid, groupid, -1));
		else
			dirlist.back()->parentid = parentid;
	}
	
	for(int i=dirlist.size() - 1; i >= 0; i--)
	{
		WebLibDirStruct *item = dirlist.at(i);
		item->weblibid = createWeblibDir(item->groupid, item->parentid, item->dirPath);
		if(item->weblibid == -1)
		{
			emit error(QString::fromLocal8Bit("����Ŀ¼%1ʧ��").arg(item->dirPath));
			break;
		}

		QString relpath = item->dirPath.right(item->dirPath.length() - rootlen);
		QMap<QString, int>::iterator itor = idlist->find(relpath);
		if(itor != idlist->end())
			*itor = item->weblibid;
		else
			idlist->insert(relpath, item->weblibid);

		if(i > 0)
			dirlist.at(i-1)->parentid = item->weblibid;
	}

	int result = dirlist.at(0)->weblibid;
	for(int i=0; i<dirlist.size(); i++)
		delete dirlist.at(i);

	return result;
}

//����������ÿ��ɨ��Ĺ�����weblib��Ŀ¼�ṹ����仯�����ڷ���ִ�й�����ͨ��������
//����ɾ����weblib��ӦĿ¼���򱾷����޷�����
void FileScaner::slotFileScan()
{
	if(m_scaning == true)
		return;

	m_manager = ConnectionManager::getInstance().getAvailableManager();

	qint64 freespace = getWeblibFreeSpace(WEBLIB_GROUPID);
	if(freespace <= 0)
	{
		ConnectionManager::getInstance().releaseManager(m_manager);
		m_manager = NULL;
		emit error(QString::fromLocal8Bit("Weblib�ռ䲻��"));
		return;
	}

	qint64 singlelim = getWeblibSizeLimit(WEBLIB_GROUPID);
	if(singlelim <= 0)
		singlelim = LONG_MAX;

	m_scaning = true;

	int parentId = 0;
	if(WEBLIB_ISCREATEUSERDIR == true)
	{
		if(!checkUserDir(parentId))
		{
			ConnectionManager::getInstance().releaseManager(m_manager);
			m_manager = NULL;
			emit error(QString::fromLocal8Bit("����Weblib�û��ļ���ʧ��"));
			return;
		}

		if(parentId < 0)
			parentId = createUserDir(WEBLIB_GROUPID, WEBLIB_ACCOUNT);

		if(parentId < 0)
		{
			ConnectionManager::getInstance().releaseManager(m_manager);
			m_manager = NULL;
			emit error(QString::fromLocal8Bit("����Weblib�û��ļ���ʧ��"));
			return;
		}
	}

	QList<WebLibDirStruct*> *list = getRootDirList(parentId);
	if(list == NULL)
		return;

	//�Ȼ�ȡ�����ѱ����id��¼����ֹ�ظ��������·����ͬ��Ŀ¼
	QMap<QString, QList<int> *> *localidset = getLocalDirInfo(list);
	
	//�����ϴ�Ŀ¼�õ��б�
	QMap<QString, int> *localidlist = new QMap<QString, int>();

	long curfilesize = 0;
	QList<int> *idlist = NULL;
	QStack<WebLibDirStruct *> dstack;
	int rootlen, rootid;
	//�Ը�Ŀ¼���д�������ͬʱ���ڶ����Ŀ¼
	for(int i=0; i<list->size(); i++)
	{
		//��·�����ȣ�ʹ��������ȱ��������ڼ���Ŀ¼�����·��
		rootlen = list->at(i)->dirPath.length();
		rootid = list->at(i)->weblibid;
		dstack.push(list->at(i));

		while(!dstack.isEmpty())
		{
			WebLibDirStruct *curitem = dstack.pop();
			//��ȡweblib����Ŀ¼id�б�
			if(curitem->weblibid != -1)
			{
				int tagid = curitem->weblibid == 0 ? -curitem->groupid : curitem->weblibid; 
				idlist = getWeblibIdList(tagid);
			}

			QDir curdir(curitem->dirPath);
			//���������Ŀ¼id��Ӧ�����Ŀ¼����Ϊ�����ϴ���Ŀ¼��������Ŀ¼���������ļ�������
			QStringList sublist = curdir.entryList(QDir::NoDotAndDotDot | QDir::AllDirs);
			for(int j=0; j < sublist.size(); j++)
			{
				int weblibid = -1;
				QString curpath = (curitem->dirPath + "/" + sublist.at(j));
				QString relpath = curpath.right(curpath.length() - rootlen);
				//������·����ͬ����
				if(localidlist->contains(relpath))
					weblibid = (*localidlist)[relpath];
				else
				{
					//��weblib�޶�Ӧid��¼��ǰid��Ч�����ؽ�Ŀ¼
					if(idlist != NULL && localidset->contains(relpath))
					{
						QList<int> *idset = (*localidset)[relpath];
						for(int m=0; m<idset->size(); m++)
						{
							if(idlist->contains(idset->at(m)))
							{
								weblibid = idset->at(m);
								localidlist->insert(relpath, weblibid);
								break;
							}
						}
					}
				}

				qDebug() << "add dir list: " << curitem->groupid << " " << curitem->weblibid << " " << weblibid << " " << curitem->dirPath + "/" + sublist.at(j);
				WebLibDirStruct *item = new WebLibDirStruct(curitem->dirPath + "/" + sublist.at(j), curitem->weblibid, curitem->groupid, weblibid);
				dstack.push(item);
			}

			//�ϴ�Ŀ¼�ļ�
			QStringList filelist = curdir.entryList(QDir::Files);
			for(int j=0; j<filelist.size(); j++)
			{
				QFile fileinfo(filelist.at(j));

				if(fileinfo.size() > singlelim)
				{
					emit error(QString::fromLocal8Bit("�������ļ��������"));
					continue;
				}

				if(curfilesize + fileinfo.size() > freespace)
				{
					emit error(QString::fromLocal8Bit("Weblib�ռ䲻��"));
					continue;
				}

				//������ϴ��ļ����
				if(filelist.at(j).indexOf(WEBLIB_TMP_FILENAME) != -1)
				{
					continue;
				}

				//����ϴ�Ŀ¼id�Ƿ���Ч����Ч���贴����Ŀ¼
				if(curitem->weblibid == -1)
				{
					curitem->weblibid = checkAndCreateWeblibDirTree(curitem->groupid, curitem->parentid, curitem->dirPath, rootid, rootlen, localidlist);
					if(curitem->weblibid == -1)
						continue;
				}

				curfilesize += fileinfo.size();
				emit addUploadFile(curitem->dirPath + "/" + filelist.at(j), curitem->groupid, curitem->weblibid);
			}

			delete curitem;

			if(idlist != NULL)
			{
				delete idlist;
				idlist = NULL;
			}
		}
	}
	delete list;

	QMap<QString, QList<int> *>::iterator itor;
	for(itor = localidset->begin() ; itor != localidset->end(); itor++)
		delete *itor;
	delete localidset;

	if(localidlist != NULL)
		delete localidlist;

	ConnectionManager::getInstance().releaseManager(m_manager);
	m_manager = NULL;
	m_scaning = false;
}