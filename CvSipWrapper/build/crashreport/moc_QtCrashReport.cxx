/****************************************************************************
** Meta object code from reading C++ file 'QtCrashReport.h'
**
** Created: Sat Dec 13 10:42:20 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../crashreport/QtCrashReport.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QtCrashReport.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtCrashReport[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtCrashReport[] = {
    "QtCrashReport\0\0sendButtonClicked()\0"
};

const QMetaObject QtCrashReport::staticMetaObject = {
    { &QObjectThreadSafe::staticMetaObject, qt_meta_stringdata_QtCrashReport,
      qt_meta_data_QtCrashReport, 0 }
};

const QMetaObject *QtCrashReport::metaObject() const
{
    return &staticMetaObject;
}

void *QtCrashReport::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtCrashReport))
        return static_cast<void*>(const_cast< QtCrashReport*>(this));
    return QObjectThreadSafe::qt_metacast(_clname);
}

int QtCrashReport::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObjectThreadSafe::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: sendButtonClicked(); break;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE