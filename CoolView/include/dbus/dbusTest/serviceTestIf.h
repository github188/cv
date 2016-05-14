/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -v -c serviceTestIf -p serviceTestIf.h:serviceTestIf.cpp .\com.dcampus.coolview.dbustest.xml
 *
 * qdbusxml2cpp is Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef SERVICETESTIF_H_1462341898
#define SERVICETESTIF_H_1462341898

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.dcampus.coolview.dbustest
 */
class serviceTestIf: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "com.dcampus.coolview.dbustest"; }

public:
    serviceTestIf(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~serviceTestIf();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<int> DbusFunc()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("DbusFunc"), argumentList);
    }

Q_SIGNALS: // SIGNALS
};

namespace com {
  namespace dcampus {
    namespace coolview {
      typedef ::serviceTestIf dbustest;
    }
  }
}
#endif