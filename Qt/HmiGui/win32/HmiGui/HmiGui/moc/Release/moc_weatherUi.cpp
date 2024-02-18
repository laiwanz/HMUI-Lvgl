/****************************************************************************
** Meta object code from reading C++ file 'weatherUi.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.11)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../HmiGui/weatherUi.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'weatherUi.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.11. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CWeatherUi_t {
    QByteArrayData data[12];
    char stringdata0[133];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CWeatherUi_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CWeatherUi_t qt_meta_stringdata_CWeatherUi = {
    {
QT_MOC_LITERAL(0, 0, 10), // "CWeatherUi"
QT_MOC_LITERAL(1, 11, 24), // "sendWeatherOnclickedDown"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 12), // "QMouseEvent*"
QT_MOC_LITERAL(4, 50, 6), // "eventT"
QT_MOC_LITERAL(5, 57, 2), // "nX"
QT_MOC_LITERAL(6, 60, 2), // "nY"
QT_MOC_LITERAL(7, 63, 22), // "sendWeatherOnclickedUp"
QT_MOC_LITERAL(8, 86, 14), // "slotSelectItem"
QT_MOC_LITERAL(9, 101, 11), // "QModelIndex"
QT_MOC_LITERAL(10, 113, 5), // "index"
QT_MOC_LITERAL(11, 119, 13) // "slotBtnCancel"

    },
    "CWeatherUi\0sendWeatherOnclickedDown\0"
    "\0QMouseEvent*\0eventT\0nX\0nY\0"
    "sendWeatherOnclickedUp\0slotSelectItem\0"
    "QModelIndex\0index\0slotBtnCancel"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CWeatherUi[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   34,    2, 0x06 /* Public */,
       7,    3,   41,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    1,   48,    2, 0x08 /* Private */,
      11,    0,   51,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int, QMetaType::Int,    4,    5,    6,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int, QMetaType::Int,    4,    5,    6,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void,

       0        // eod
};

void CWeatherUi::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CWeatherUi *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sendWeatherOnclickedDown((*reinterpret_cast< QMouseEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 1: _t->sendWeatherOnclickedUp((*reinterpret_cast< QMouseEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: _t->slotSelectItem((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 3: _t->slotBtnCancel(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CWeatherUi::*)(QMouseEvent * , int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CWeatherUi::sendWeatherOnclickedDown)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CWeatherUi::*)(QMouseEvent * , int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CWeatherUi::sendWeatherOnclickedUp)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CWeatherUi::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_CWeatherUi.data,
    qt_meta_data_CWeatherUi,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CWeatherUi::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CWeatherUi::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CWeatherUi.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "HMIPart"))
        return static_cast< HMIPart*>(this);
    return QWidget::qt_metacast(_clname);
}

int CWeatherUi::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void CWeatherUi::sendWeatherOnclickedDown(QMouseEvent * _t1, int _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CWeatherUi::sendWeatherOnclickedUp(QMouseEvent * _t1, int _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
