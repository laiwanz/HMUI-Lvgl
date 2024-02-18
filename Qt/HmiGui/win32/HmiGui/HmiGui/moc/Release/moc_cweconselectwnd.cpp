/****************************************************************************
** Meta object code from reading C++ file 'cweconselectwnd.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.11)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../HmiGui/formula/cweconselectwnd.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cweconselectwnd.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.11. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CWeconSelectWnd_t {
    QByteArrayData data[17];
    char stringdata0[268];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CWeconSelectWnd_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CWeconSelectWnd_t qt_meta_stringdata_CWeconSelectWnd = {
    {
QT_MOC_LITERAL(0, 0, 15), // "CWeconSelectWnd"
QT_MOC_LITERAL(1, 16, 14), // "OnSelectResult"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 5), // "sPath"
QT_MOC_LITERAL(4, 38, 18), // "OnClickCloseWndBtn"
QT_MOC_LITERAL(5, 57, 16), // "OnClickSelectBtn"
QT_MOC_LITERAL(6, 74, 15), // "OnClickListItem"
QT_MOC_LITERAL(7, 90, 16), // "QListWidgetItem*"
QT_MOC_LITERAL(8, 107, 4), // "item"
QT_MOC_LITERAL(9, 112, 17), // "OnDbClickListItem"
QT_MOC_LITERAL(10, 130, 18), // "OnClickSDCardCheck"
QT_MOC_LITERAL(11, 149, 17), // "OnClickUDiskCheck"
QT_MOC_LITERAL(12, 167, 18), // "OnClickPathForward"
QT_MOC_LITERAL(13, 186, 20), // "OnClickPathStandBack"
QT_MOC_LITERAL(14, 207, 17), // "OnclickCreateFile"
QT_MOC_LITERAL(15, 225, 17), // "OnClickAddNewFile"
QT_MOC_LITERAL(16, 243, 24) // "OnClickCancelAddFileName"

    },
    "CWeconSelectWnd\0OnSelectResult\0\0sPath\0"
    "OnClickCloseWndBtn\0OnClickSelectBtn\0"
    "OnClickListItem\0QListWidgetItem*\0item\0"
    "OnDbClickListItem\0OnClickSDCardCheck\0"
    "OnClickUDiskCheck\0OnClickPathForward\0"
    "OnClickPathStandBack\0OnclickCreateFile\0"
    "OnClickAddNewFile\0OnClickCancelAddFileName"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CWeconSelectWnd[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   74,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,   77,    2, 0x0a /* Public */,
       5,    0,   78,    2, 0x0a /* Public */,
       6,    1,   79,    2, 0x0a /* Public */,
       9,    1,   82,    2, 0x0a /* Public */,
      10,    0,   85,    2, 0x0a /* Public */,
      11,    0,   86,    2, 0x0a /* Public */,
      12,    0,   87,    2, 0x0a /* Public */,
      13,    0,   88,    2, 0x0a /* Public */,
      14,    0,   89,    2, 0x0a /* Public */,
      15,    0,   90,    2, 0x0a /* Public */,
      16,    0,   91,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CWeconSelectWnd::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CWeconSelectWnd *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->OnSelectResult((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->OnClickCloseWndBtn(); break;
        case 2: _t->OnClickSelectBtn(); break;
        case 3: _t->OnClickListItem((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 4: _t->OnDbClickListItem((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 5: _t->OnClickSDCardCheck(); break;
        case 6: _t->OnClickUDiskCheck(); break;
        case 7: _t->OnClickPathForward(); break;
        case 8: _t->OnClickPathStandBack(); break;
        case 9: _t->OnclickCreateFile(); break;
        case 10: _t->OnClickAddNewFile(); break;
        case 11: _t->OnClickCancelAddFileName(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CWeconSelectWnd::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CWeconSelectWnd::OnSelectResult)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CWeconSelectWnd::staticMetaObject = { {
    &QDialog::staticMetaObject,
    qt_meta_stringdata_CWeconSelectWnd.data,
    qt_meta_data_CWeconSelectWnd,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CWeconSelectWnd::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CWeconSelectWnd::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CWeconSelectWnd.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int CWeconSelectWnd::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void CWeconSelectWnd::OnSelectResult(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
