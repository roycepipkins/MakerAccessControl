/****************************************************************************
** Meta object code from reading C++ file 'adminrfid.h'
**
** Created: Wed Apr 21 18:10:28 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "adminrfid.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'adminrfid.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_AdminRFID[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x0a,
      23,   10,   10,   10, 0x0a,
      37,   10,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_AdminRFID[] = {
    "AdminRFID\0\0onConnect()\0onReadyRead()\0"
    "scanPorts()\0"
};

const QMetaObject AdminRFID::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_AdminRFID,
      qt_meta_data_AdminRFID, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &AdminRFID::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *AdminRFID::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *AdminRFID::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AdminRFID))
        return static_cast<void*>(const_cast< AdminRFID*>(this));
    return QDialog::qt_metacast(_clname);
}

int AdminRFID::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: onConnect(); break;
        case 1: onReadyRead(); break;
        case 2: scanPorts(); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
