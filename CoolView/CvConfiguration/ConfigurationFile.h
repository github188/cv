#ifndef CONFIGURATION_FILE_H
#define CONFIGURATION_FILE_H

#include <QtCore/QtCore>
#include <QtXml/QtXml>
#include <json/json.h>

/**
 * @brief ����������õ��ļ�
 */
class ConfigurationFile{
public:
	ConfigurationFile();
	~ConfigurationFile();

	/**
	 * @brief ��ʼ�����ñ�
	 */
	void initilizeConfiguration( const QString& fileName );

	/**
	 * @brief ��ʼ�����ñ���Ĳ���
	 */
	void initilizeCameraList( QDomElement& element );
	void initilizeSpeakerList( QDomElement speakerListNode );
	void initilizeMicList( QDomElement micListNode );
	void initilizeIPList( QDomElement ipListNode );

	/**
	 * @brief ��JSON��ʽ��ȡָ�����豸����
	 */
	Json::Value queryCameraConfigJSON( );
	Json::Value querySpeakerConfigJSON();
	Json::Value queryIPConfigJSON();
	Json::Value queryMicConfigJSON();


	/**
	 * @brief �޸�������Ϣ
	 */
	void saveCameraConfig( const QString &camera_name, const QString &crossbar_name, const QString &crossbar_type, int video_width, int video_height );

private:
	/**
	 * @brief ����parentԪ�ؽڵ��µ�һ��TAG�ڵ��ֵ
	 */
	void updateElement( QDomElement& parent, const QString& tag, const QString& nodeValue );

	/**
	 * @brief ����COM�ļ�
	 */
	void loadDOM( QDomDocument* doc , const QString& fileName );
	/**
	 * @brief ����DOM�ļ�
	 */
	void saveDOM( QDomDocument& doc, const QString& fileName );

public:
	QString _CONFIG_FILE_NAME;
	QString _CURRENT_CONFIG_FILE_NAME;

	//ȫ��������Ϣ
	QDomDocument _configDoc;
	QDomElement	 _configRoot;

	//�ն���ѡ��������Ϣ
	QDomDocument _currentConfigDoc;
	QDomElement  _currentConfigRoot;
};

#endif