/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -v -c CvCoreIf -p CvCoreIf.h:CvCoreIf.cpp .\com.dcampus.coolview.core.xml
 *
 * qdbusxml2cpp is Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef CVCOREIF_H_1437701768
#define CVCOREIF_H_1437701768

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.dcampus.coolview.core
 */
class CvCoreIf: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "com.dcampus.coolview.core"; }

public:
    CvCoreIf(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~CvCoreIf();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> TeleCommand(int command_index, const QByteArray &intput_array)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(command_index) << QVariant::fromValue(intput_array);
        return asyncCallWithArgumentList(QStringLiteral("TeleCommand"), argumentList);
    }

Q_SIGNALS: // SIGNALS
};

namespace com {
  namespace dcampus {
    namespace coolview {
      typedef ::CvCoreIf core;
    }
  }
}
#endif
