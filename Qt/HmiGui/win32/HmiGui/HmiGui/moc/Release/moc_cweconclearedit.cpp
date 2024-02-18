/****************************************************************************
** Meta object code from reading C++ file 'cweconclearedit.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.11)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../HmiGui/formula/cweconclearedit.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cweconclearedit.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.11. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CWeconClearEdit_t {
    QByteArrayData data[18];
    char stringdata0[207];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CWeconClearEdit_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CWeconClearEdit_t qt_meta_stringdata_CWeconClearEdit = {
    {
QT_MOC_LITERAL(0, 0, 15), // "CWeconClearEdit"
QT_MOC_LITERAL(1, 16, 14), // "textEditFinish"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 11), // "OnClearText"
QT_MOC_LITERAL(4, 44, 14), // "OnShowDownList"
QT_MOC_LITERAL(5, 59, 12), // "OnClickClear"
QT_MOC_LITERAL(6, 72, 17), // "OnShowDownListWnd"
QT_MOC_LITERAL(7, 90, 15), // "OnSelectPopItem"
QT_MOC_LITERAL(8, 106, 11), // "QModelIndex"
QT_MOC_LITERAL(9, 118, 5), // "index"
QT_MOC_LITERAL(10, 124, 14), // "OnHideDownList"
QT_MOC_LITERAL(11, 139, 5), // "sText"
QT_MOC_LITERAL(12, 145, 13), // "keyPressEvent"
QT_MOC_LITERAL(13, 159, 10), // "QKeyEvent*"
QT_MOC_LITERAL(14, 170, 1), // "e"
QT_MOC_LITERAL(15, 172, 15), // "mousePressEvent"
QT_MOC_LITERAL(16, 188, 12), // "QMouseEvent*"
QT_MOC_LITERAL(17, 201, 5) // "event"

    },
    "CWeconClearEdit\0textEditFinish\0\0"
    "OnClearText\0OnShowDownList\0OnClickClear\0"
    "OnShowDownListWnd\0OnSelectPopItem\0"
    "QModelIndex\0index\0OnHideDownList\0sText\0"
    "keyPressEvent\0QKeyEvent*\0e\0mousePressEvent\0"
    "QMouseEvent*\0event"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CWeconClearEdit[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x06 /* Public */,
       3,    0,   60,    2, 0x06 /* Public */,
       4,    0,   61,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   62,    2, 0x0a /* Public */,
       6,    0,   63,    2, 0x0a /* Public */,
       7,    1,   64,    2, 0x0a /* Public */,
      10,    1,   67,    2, 0x0a /* Public */,
      12,    1,   70,    2, 0x0a /* Public */,
      15,    1,   73,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void, 0x80000000 | 13,   14,
    QMetaType::Void, 0x80000000 | 16,   17,

       0        // eod
};

void CWeconClearEdit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CWeconClearEdit *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->textEditFinish(); break;
        case 1: _t->OnClearText(); break;
        case 2: _t->OnShowDownList(); break;
        case 3: _t->OnClickClear(); break;
        case 4: _t->OnShowDownListWnd(); break;
        case 5: _t->OnSelectPopItem((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 6: _t->OnHideDownList((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->keyPressEvent((*reinterpret_cast< QKeyEvent*(*)>(_a[1]))); break;
        case 8: _t->mousePressEvent((*reinterpret_cast< QMouseEvent*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CWeconClearEdit::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CWeconClearEdit::textEditFinish)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CWeconClearEdit::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CWeconClearEdit::OnClearText)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CWeconClearEdit::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CWeconClearEdit::OnShowDownList)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CWeconClearEdit::staticMetaObject = { {
    &QLineEdit::staticMetaObject,
    qt_meta_stringdata_CWeconClearEdit.data,
    qt_meta_data_CWeconClearEdit,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CWeconClearEdit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CWeconClearEdit::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CWeconClearEdit.stringdata0))
        return static_cast<void*>(this);
    return QLineEdit::qt_metacast(_clname);
}

int CWeconClearEdit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLineEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void CWeconClearEdit::textEditFinish()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void CWeconClearEdit::OnClearText()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void CWeconClearEdit::OnShowDownList()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
