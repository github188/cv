/****************************************************************************
** Meta object code from reading C++ file 'QtContactTreeMouseFilter.h'
**
** Created: Thu Jul 29 12:39:41 2010
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../wengophone/src/presentation/qt/contactlist/QtContactTreeMouseFilter.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QtContactTreeMouseFilter.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtContactTreeMouseFilter[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      33,   26,   25,   25, 0x05,
      85,   63,   25,   25, 0x05,

 // slots: signature, parameters, type, tag, flags
     116,   25,   25,   25, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtContactTreeMouseFilter[] = {
    "QtContactTreeMouseFilter\0\0button\0"
    "mouseClicked(Qt::MouseButton)\0"
    "dstContact,srcContact\0"
    "mergeContacts(QString,QString)\0"
    "handleDrop()\0"
};

const QMetaObject QtContactTreeMouseFilter::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QtContactTreeMouseFilter,
      qt_meta_data_QtContactTreeMouseFilter, 0 }
};

const QMetaObject *QtContactTreeMouseFilter::metaObject() const
{
    return &staticMetaObject;
}

void *QtContactTreeMouseFilter::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtContactTreeMouseFilter))
        return static_cast<void*>(const_cast< QtContactTreeMouseFilter*>(this));
    return QObject::qt_metacast(_clname);
}

int QtContactTreeMouseFilter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: mouseClicked((*reinterpret_cast< Qt::MouseButton(*)>(_a[1]))); break;
        case 1: mergeContacts((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 2: handleDrop(); break;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void QtContactTreeMouseFilter::mouseClicked(Qt::MouseButton _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtContactTreeMouseFilter::mergeContacts(QString _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
