/****************************************************************************
** Meta object code from reading C++ file 'TruongNhapLieu.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../include/utils/TruongNhapLieu.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TruongNhapLieu.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN14TruongNhapLieuE_t {};
} // unnamed namespace

template <> constexpr inline auto TruongNhapLieu::qt_create_metaobjectdata<qt_meta_tag_ZN14TruongNhapLieuE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "TruongNhapLieu",
        "textThayDoi",
        "",
        "text",
        "enterDuocNhan",
        "giaTriHopLe",
        "hopLe"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'textThayDoi'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'enterDuocNhan'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'giaTriHopLe'
        QtMocHelpers::SignalData<void(bool)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 6 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<TruongNhapLieu, qt_meta_tag_ZN14TruongNhapLieuE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject TruongNhapLieu::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14TruongNhapLieuE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14TruongNhapLieuE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14TruongNhapLieuE_t>.metaTypes,
    nullptr
} };

void TruongNhapLieu::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TruongNhapLieu *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->textThayDoi((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->enterDuocNhan(); break;
        case 2: _t->giaTriHopLe((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (TruongNhapLieu::*)(const QString & )>(_a, &TruongNhapLieu::textThayDoi, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (TruongNhapLieu::*)()>(_a, &TruongNhapLieu::enterDuocNhan, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (TruongNhapLieu::*)(bool )>(_a, &TruongNhapLieu::giaTriHopLe, 2))
            return;
    }
}

const QMetaObject *TruongNhapLieu::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TruongNhapLieu::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14TruongNhapLieuE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int TruongNhapLieu::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void TruongNhapLieu::textThayDoi(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void TruongNhapLieu::enterDuocNhan()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void TruongNhapLieu::giaTriHopLe(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
namespace {
struct qt_meta_tag_ZN13TruongTimKiemE_t {};
} // unnamed namespace

template <> constexpr inline auto TruongTimKiem::qt_create_metaobjectdata<qt_meta_tag_ZN13TruongTimKiemE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "TruongTimKiem",
        "textThayDoi",
        "",
        "text",
        "timKiem",
        "tuKhoa",
        "daXoa"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'textThayDoi'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'timKiem'
        QtMocHelpers::SignalData<void(const QString &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Signal 'daXoa'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<TruongTimKiem, qt_meta_tag_ZN13TruongTimKiemE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject TruongTimKiem::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13TruongTimKiemE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13TruongTimKiemE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13TruongTimKiemE_t>.metaTypes,
    nullptr
} };

void TruongTimKiem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TruongTimKiem *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->textThayDoi((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->timKiem((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->daXoa(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (TruongTimKiem::*)(const QString & )>(_a, &TruongTimKiem::textThayDoi, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (TruongTimKiem::*)(const QString & )>(_a, &TruongTimKiem::timKiem, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (TruongTimKiem::*)()>(_a, &TruongTimKiem::daXoa, 2))
            return;
    }
}

const QMetaObject *TruongTimKiem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TruongTimKiem::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13TruongTimKiemE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int TruongTimKiem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void TruongTimKiem::textThayDoi(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void TruongTimKiem::timKiem(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void TruongTimKiem::daXoa()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
