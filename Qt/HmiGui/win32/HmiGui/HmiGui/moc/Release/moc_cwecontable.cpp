/****************************************************************************
** Meta object code from reading C++ file 'cwecontable.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.11)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../HmiGui/formula/cwecontable.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cwecontable.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.11. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CWeconTable_t {
    QByteArrayData data[24];
    char stringdata0[336];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CWeconTable_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CWeconTable_t qt_meta_stringdata_CWeconTable = {
    {
QT_MOC_LITERAL(0, 0, 11), // "CWeconTable"
QT_MOC_LITERAL(1, 12, 14), // "showMessageWnd"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 8), // "QString&"
QT_MOC_LITERAL(4, 37, 8), // "sMsgText"
QT_MOC_LITERAL(5, 46, 22), // "updateOnePageShowCount"
QT_MOC_LITERAL(6, 69, 16), // "OnClickSearchBtn"
QT_MOC_LITERAL(7, 86, 14), // "OnTurnToPageUp"
QT_MOC_LITERAL(8, 101, 17), // "ClickPageUpButton"
QT_MOC_LITERAL(9, 119, 19), // "ClickPageDownButton"
QT_MOC_LITERAL(10, 139, 17), // "ClickExpandButton"
QT_MOC_LITERAL(11, 157, 20), // "ChangeShowPageNumber"
QT_MOC_LITERAL(12, 178, 9), // "nCurIndex"
QT_MOC_LITERAL(13, 188, 15), // "DoubleClickCell"
QT_MOC_LITERAL(14, 204, 11), // "QModelIndex"
QT_MOC_LITERAL(15, 216, 5), // "index"
QT_MOC_LITERAL(16, 222, 13), // "CellCloseEdit"
QT_MOC_LITERAL(17, 236, 8), // "QWidget*"
QT_MOC_LITERAL(18, 245, 6), // "editor"
QT_MOC_LITERAL(19, 252, 34), // "QAbstractItemDelegate::EndEdi..."
QT_MOC_LITERAL(20, 287, 4), // "hint"
QT_MOC_LITERAL(21, 292, 12), // "OnClickVHead"
QT_MOC_LITERAL(22, 305, 13), // "nLogicalIndex"
QT_MOC_LITERAL(23, 319, 16) // "OnShowMessageWnd"

    },
    "CWeconTable\0showMessageWnd\0\0QString&\0"
    "sMsgText\0updateOnePageShowCount\0"
    "OnClickSearchBtn\0OnTurnToPageUp\0"
    "ClickPageUpButton\0ClickPageDownButton\0"
    "ClickExpandButton\0ChangeShowPageNumber\0"
    "nCurIndex\0DoubleClickCell\0QModelIndex\0"
    "index\0CellCloseEdit\0QWidget*\0editor\0"
    "QAbstractItemDelegate::EndEditHint\0"
    "hint\0OnClickVHead\0nLogicalIndex\0"
    "OnShowMessageWnd"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CWeconTable[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   74,    2, 0x06 /* Public */,
       5,    0,   77,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    0,   78,    2, 0x0a /* Public */,
       7,    0,   79,    2, 0x0a /* Public */,
       8,    0,   80,    2, 0x0a /* Public */,
       9,    0,   81,    2, 0x0a /* Public */,
      10,    0,   82,    2, 0x0a /* Public */,
      11,    1,   83,    2, 0x0a /* Public */,
      13,    1,   86,    2, 0x0a /* Public */,
      16,    2,   89,    2, 0x0a /* Public */,
      21,    1,   94,    2, 0x0a /* Public */,
      23,    1,   97,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void, 0x80000000 | 17, 0x80000000 | 19,   18,   20,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

void CWeconTable::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CWeconTable *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->showMessageWnd((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->updateOnePageShowCount(); break;
        case 2: _t->OnClickSearchBtn(); break;
        case 3: _t->OnTurnToPageUp(); break;
        case 4: _t->ClickPageUpButton(); break;
        case 5: _t->ClickPageDownButton(); break;
        case 6: _t->ClickExpandButton(); break;
        case 7: _t->ChangeShowPageNumber((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->DoubleClickCell((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 9: _t->CellCloseEdit((*reinterpret_cast< QWidget*(*)>(_a[1])),(*reinterpret_cast< QAbstractItemDelegate::EndEditHint(*)>(_a[2]))); break;
        case 10: _t->OnClickVHead((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->OnShowMessageWnd((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 9:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QWidget* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CWeconTable::*)(QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CWeconTable::showMessageWnd)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CWeconTable::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CWeconTable::updateOnePageShowCount)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CWeconTable::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_CWeconTable.data,
    qt_meta_data_CWeconTable,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CWeconTable::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CWeconTable::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CWeconTable.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CWeconTable::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void CWeconTable::showMessageWnd(QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CWeconTable::updateOnePageShowCount()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
