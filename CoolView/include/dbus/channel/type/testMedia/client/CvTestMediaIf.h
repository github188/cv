/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -v -c CvTestMediaIf -p CvTestMediaIf.h:CvTestMediaIf.cpp .\com.dcampus.coolview.channel.type.testMedia.xml
 *
 * qdbusxml2cpp is Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef CVTESTMEDIAIF_H_1342680726
#define CVTESTMEDIAIF_H_1342680726

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.dcampus.coolview.channel.type.testMedia
 */
class CvTestMediaIf: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "com.dcampus.coolview.channel.type.testMedia"; }

public:
    CvTestMediaIf(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~CvTestMediaIf();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> Destroy()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("Destroy"), argumentList);
    }

    inline QDBusPendingReply<QByteArray> GetMediaInfo(int info_type)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(info_type);
        return asyncCallWithArgumentList(QLatin1String("GetMediaInfo"), argumentList);
    }

    inline QDBusPendingReply<> Pause()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("Pause"), argumentList);
    }

    inline QDBusPendingReply<> Run()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("Run"), argumentList);
    }

    inline QDBusPendingReply<> SetMediaInfo(int action_id, const QByteArray &input_garray)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(action_id) << qVariantFromValue(input_garray);
        return asyncCallWithArgumentList(QLatin1String("SetMediaInfo"), argumentList);
    }

    inline QDBusPendingReply<> Stop()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("Stop"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void RunningStateChanged(const QString &media_id, int current_state);
};

namespace com {
  namespace dcampus {
    namespace coolview {
      namespace channel {
        namespace type {
          typedef ::CvTestMediaIf testMedia;
        }
      }
    }
  }
}
#endif
