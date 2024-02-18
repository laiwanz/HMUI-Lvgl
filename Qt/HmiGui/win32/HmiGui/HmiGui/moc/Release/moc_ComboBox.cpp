/****************************************************************************
** Meta object code from reading C++ file 'ComboBox.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.11)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../HmiGui/ComboBox.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ComboBox.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.11. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CComboBox_t {
    QByteArrayData data[12];
    char stringdata0[113];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CComboBox_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CComboBox_t qt_meta_stringdata_CComboBox = {
    {
QT_MOC_LITERAL(0, 0, 9), // "CComboBox"
QT_MOC_LITERAL(1, 10, 11), // "onClickdown"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 12), // "QMouseEvent*"
QT_MOC_LITERAL(4, 36, 6), // "eventT"
QT_MOC_LITERAL(5, 43, 2), // "nX"
QT_MOC_LITERAL(6, 46, 2), // "nY"
QT_MOC_LITERAL(7, 49, 9), // "onClickup"
QT_MOC_LITERAL(8, 59, 20), // "DownListIndexChanged"
QT_MOC_LITERAL(9, 80, 6), // "nIndex"
QT_MOC_LITERAL(10, 87, 11), // "SetShowRang"
QT_MOC_LITERAL(11, 99, 13) // "getItemHeight"

    },
    "CComboBox\0onClickdown\0\0QMouseEvent*\0"
    "eventT\0nX\0nY\0onClickup\0DownListIndexChanged\0"
    "nIndex\0SetShowRang\0getItemHeight"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CComboBox[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   39,    2, 0x06 /* Public */,
       7,    3,   46,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    1,   53,    2, 0x08 /* Private */,
      10,    1,   56,    2, 0x08 /* Private */,
      11,    0,   59,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int, QMetaType::Int,    4,    5,    6,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int, QMetaType::Int,    4,    5,    6,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Int,

       0        // eod
};

void CComboBox::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CComboBox *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onClickdown((*reinterpret_cast< QMouseEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 1: _t->onClickup((*reinterpret_cast< QMouseEvent*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: _t->DownListIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->SetShowRang((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: { int _r = _t->getItemHeight();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CComboBox::*)(QMouseEvent * , int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CComboBox::onClickdown)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CComboBox::*)(QMouseEvent * , int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CComboBox::onClickup)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CComboBox::staticMetaObject = { {
    &QComboBox::staticMetaObject,
    qt_meta_stringdata_CComboBox.data,
    qt_meta_data_CComboBox,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CComboBox::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CComboBox::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CComboBox.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "HMIPart"))
        return static_cast< HMIPart*>(this);
    return QComboBox::qt_metacast(_clname);
}

int CComboBox::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QComboBox::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void CComboBox::onClickdown(QMouseEvent * _t1, int _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CComboBox::onClickup(QMouseEvent * _t1, int _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
