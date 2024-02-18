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
    QByteArrayData data[13];
    char stringdata0[179];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CWeconTable_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CWeconTable_t qt_meta_stringdata_CWeconTable = {
    {
QT_MOC_LITERAL(0, 0, 11), // "CWeconTable"
QT_MOC_LITERAL(1, 12, 17), // "ClickSearchButton"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 18), // "CLickAddDataButton"
QT_MOC_LITERAL(4, 50, 24), // "ClickChangeFormulaButton"
QT_MOC_LITERAL(5, 75, 15), // "DoubleClickCell"
QT_MOC_LITERAL(6, 91, 11), // "QModelIndex"
QT_MOC_LITERAL(7, 103, 5), // "index"
QT_MOC_LITERAL(8, 109, 13), // "CellCloseEdit"
QT_MOC_LITERAL(9, 123, 8), // "QWidget*"
QT_MOC_LITERAL(10, 132, 6), // "editor"
QT_MOC_LITERAL(11, 139, 34), // "QAbstractItemDelegate::EndEdi..."
QT_MOC_LITERAL(12, 174, 4) // "hint"

    },
    "CWeconTable\0ClickSearchButton\0\0"
    "CLickAddDataButton\0ClickChangeFormulaButton\0"
    "DoubleClickCell\0QModelIndex\0index\0"
    "CellCloseEdit\0QWidget*\0editor\0"
    "QAbstractItemDelegate::EndEditHint\0"
    "hint"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CWeconTable[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x0a /* Public */,
       3,    0,   40,    2, 0x0a /* Public */,
       4,    0,   41,    2, 0x0a /* Public */,
       5,    1,   42,    2, 0x0a /* Public */,
       8,    2,   45,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, 0x80000000 | 9, 0x80000000 | 11,   10,   12,

       0        // eod
};

void CWeconTable::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CWeconTable *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->ClickSearchButton(); break;
        case 1: _t->CLickAddDataButton(); break;
        case 2: _t->ClickChangeFormulaButton(); break;
        case 3: _t->DoubleClickCell((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 4: _t->CellCloseEdit((*reinterpret_cast< QWidget*(*)>(_a[1])),(*reinterpret_cast< QAbstractItemDelegate::EndEditHint(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QWidget* >(); break;
            }
            break;
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
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
