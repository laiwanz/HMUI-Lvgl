/****************************************************************************
** Meta object code from reading C++ file 'blinds.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.11)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../HmiGui/screenSaver/blinds/blinds.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'blinds.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.11. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CBlinds_t {
    QByteArrayData data[8];
    char stringdata0[70];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CBlinds_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CBlinds_t qt_meta_stringdata_CBlinds = {
    {
QT_MOC_LITERAL(0, 0, 7), // "CBlinds"
QT_MOC_LITERAL(1, 8, 9), // "slotTimer"
QT_MOC_LITERAL(2, 18, 0), // ""
QT_MOC_LITERAL(3, 19, 11), // "resetDirect"
QT_MOC_LITERAL(4, 31, 8), // "DIRECT_T"
QT_MOC_LITERAL(5, 40, 6), // "direct"
QT_MOC_LITERAL(6, 47, 10), // "ascentLeaf"
QT_MOC_LITERAL(7, 58, 11) // "descentLeaf"

    },
    "CBlinds\0slotTimer\0\0resetDirect\0DIRECT_T\0"
    "direct\0ascentLeaf\0descentLeaf"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CBlinds[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x08 /* Private */,
       3,    1,   35,    2, 0x08 /* Private */,
       6,    0,   38,    2, 0x08 /* Private */,
       7,    0,   39,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CBlinds::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CBlinds *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->slotTimer(); break;
        case 1: _t->resetDirect((*reinterpret_cast< DIRECT_T(*)>(_a[1]))); break;
        case 2: _t->ascentLeaf(); break;
        case 3: _t->descentLeaf(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CBlinds::staticMetaObject = { {
    &CScreenSaver::staticMetaObject,
    qt_meta_stringdata_CBlinds.data,
    qt_meta_data_CBlinds,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CBlinds::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CBlinds::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CBlinds.stringdata0))
        return static_cast<void*>(this);
    return CScreenSaver::qt_metacast(_clname);
}

int CBlinds::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CScreenSaver::qt_metacall(_c, _id, _a);
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
