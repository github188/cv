/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -c CvScreenMediaAdaptor -a CvScreenMediaAdaptor.h:CvScreenMediaAdaptor.cpp .\com.dcampus.coolview.channel.type.screenMedia.xml
 *
 * qdbusxml2cpp is Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef CVSCREENMEDIAADAPTOR_H_1306997626
#define CVSCREENMEDIAADAPTOR_H_1306997626

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;

/*
 * Adaptor class for interface com.dcampus.coolview.channel.type.screenMedia
 */
class CvScreenMediaAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.dcampus.coolview.channel.type.screenMedia")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.dcampus.coolview.channel.type.screenMedia\">\n"
"    <method name=\"SetMediaInfo\">\n"
"      <annotation value=\"__client_request_cb\" name=\"org.freedesktop.DBus.GLib.CSymbol\"/>\n"
"      <arg direction=\"in\" type=\"i\" name=\"action_id\"/>\n"
"      <arg direction=\"in\" type=\"ay\" name=\"input_garray\"/>\n"
"      <!--set media params-->\n"
"    </method>\n"
"    <method name=\"GetMediaInfo\">\n"
"      <!--get media info-->\n"
"      <arg direction=\"in\" type=\"i\" name=\"info_type\"/>\n"
"      <arg direction=\"out\" type=\"ay\" name=\"output_garray\"/>\n"
"    </method>\n"
"    <method name=\"Run\">\n"
"      <!--run proc : this will cause screen captrue-->\n"
"    </method>\n"
"    <method name=\"Pause\">\n"
"      <!--pause sending image file-->\n"
"    </method>\n"
"    <method name=\"Stop\">\n"
"      <!--stop proc-->\n"
"    </method>\n"
"    <method name=\"Destroy\">\n"
"      <!--destroy proc-->\n"
"    </method>\n"
"    <signal name=\"RunningStateChanged\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"media_id\"/>\n"
"      <arg direction=\"out\" type=\"i\" name=\"current_state\"/>\n"
"    </signal>\n"
"  </interface>\n"
        "")
public:
    CvScreenMediaAdaptor(QObject *parent);
    virtual ~CvScreenMediaAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void Destroy();
    QByteArray GetMediaInfo(int info_type);
    void Pause();
    void Run();
    void SetMediaInfo(int action_id, const QByteArray &input_garray);
    void Stop();
Q_SIGNALS: // SIGNALS
    void RunningStateChanged(const QString &media_id, int current_state);
};

#endif
