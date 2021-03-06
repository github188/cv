/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -c TxMonitorAdaptor -a TxMonitorAdaptor.h:TxMonitorAdaptor.cpp .\com.dcampus.coolview.txMonitor.xml
 *
 * qdbusxml2cpp is Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#include "TxMonitorAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class TxMonitorAdaptor
 */

TxMonitorAdaptor::TxMonitorAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

TxMonitorAdaptor::~TxMonitorAdaptor()
{
    // destructor
}

void TxMonitorAdaptor::ReportRecStat(int stat_type, const QByteArray &data_garray)
{
    // handle method call com.dcampus.coolview.txMonitor.ReportRecStat
    QMetaObject::invokeMethod(parent(), "ReportRecStat", Q_ARG(int, stat_type), Q_ARG(QByteArray, data_garray));
}

