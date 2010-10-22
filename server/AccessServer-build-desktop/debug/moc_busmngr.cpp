/****************************************************************************
** Meta object code from reading C++ file 'busmngr.h'
**
** Created: Thu Jul 1 22:11:23 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../AccessServer/busmngr.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'busmngr.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_BusMngr[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       9,    8,    8,    8, 0x0a,
      33,   23,    8,    8, 0x0a,
      56,    8,    8,    8, 0x0a,
      68,    8,    8,    8, 0x0a,
      83,    8,    8,    8, 0x0a,
     100,    8,    8,    8, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_BusMngr[] = {
    "BusMngr\0\0onReadyRead()\0byteCount\0"
    "onBytesWritten(qint64)\0onTimeout()\0"
    "onRtsTimeout()\0onAboutToClose()\0"
    "reconnect()\0"
};

const QMetaObject BusMngr::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_BusMngr,
      qt_meta_data_BusMngr, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BusMngr::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BusMngr::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BusMngr::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BusMngr))
        return static_cast<void*>(const_cast< BusMngr*>(this));
    return QObject::qt_metacast(_clname);
}

int BusMngr::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: onReadyRead(); break;
        case 1: onBytesWritten((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 2: onTimeout(); break;
        case 3: onRtsTimeout(); break;
        case 4: onAboutToClose(); break;
        case 5: reconnect(); break;
        default: ;
        }
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
