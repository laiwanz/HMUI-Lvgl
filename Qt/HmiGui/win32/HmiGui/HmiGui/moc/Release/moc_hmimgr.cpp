/****************************************************************************
** Meta object code from reading C++ file 'hmimgr.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.11)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../HmiGui/hmimgr.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'hmimgr.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.11. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_HMIMgr_t {
    QByteArrayData data[8];
    char stringdata0[90];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_HMIMgr_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_HMIMgr_t qt_meta_stringdata_HMIMgr = {
    {
QT_MOC_LITERAL(0, 0, 6), // "HMIMgr"
QT_MOC_LITERAL(1, 7, 11), // "sgl_showSys"
QT_MOC_LITERAL(2, 19, 0), // ""
QT_MOC_LITERAL(3, 20, 5), // "nType"
QT_MOC_LITERAL(4, 26, 21), // "Signal_QuicklyDoEvent"
QT_MOC_LITERAL(5, 48, 17), // "hmiproto::hmiact&"
QT_MOC_LITERAL(6, 66, 19), // "Slot_QuicklyDoEvent"
QT_MOC_LITERAL(7, 86, 3) // "act"

    },
    "HMIMgr\0sgl_showSys\0\0nType\0"
    "Signal_QuicklyDoEvent\0hmiproto::hmiact&\0"
    "Slot_QuicklyDoEvent\0act"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_HMIMgr[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,
       4,    1,   32,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   35,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, 0x80000000 | 5,    2,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 5,    7,

       0        // eod
};

void HMIMgr::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<HMIMgr *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sgl_showSys((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->Signal_QuicklyDoEvent((*reinterpret_cast< hmiproto::hmiact(*)>(_a[1]))); break;
        case 2: _t->Slot_QuicklyDoEvent((*reinterpret_cast< hmiproto::hmiact(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (HMIMgr::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&HMIMgr::sgl_showSys)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (HMIMgr::*)(hmiproto::hmiact & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&HMIMgr::Signal_QuicklyDoEvent)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject HMIMgr::staticMetaObject = { {
    &QFrame::staticMetaObject,
    qt_meta_stringdata_HMIMgr.data,
    qt_meta_data_HMIMgr,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *HMIMgr::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *HMIMgr::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_HMIMgr.stringdata0))
        return static_cast<void*>(this);
    return QFrame::qt_metacast(_clname);
}

int HMIMgr::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void HMIMgr::sgl_showSys(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void HMIMgr::Signal_QuicklyDoEvent(hmiproto::hmiact & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
