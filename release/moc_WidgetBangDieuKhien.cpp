/****************************************************************************
** Meta object code from reading C++ file 'WidgetBangDieuKhien.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/views/WidgetBangDieuKhien.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'WidgetBangDieuKhien.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN19WidgetBangDieuKhienE_t {};
} // unnamed namespace

template <> constexpr inline auto WidgetBangDieuKhien::qt_create_metaobjectdata<qt_meta_tag_ZN19WidgetBangDieuKhienE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "WidgetBangDieuKhien",
        "yeuCauXemChiTiet",
        "",
        "loaiCanhBao",
        "yeuCauThemSachMoi",
        "yeuCauDangKyDocGia",
        "yeuCauTaoPhieuMuon",
        "yeuCauSaoLuu",
        "yeuCauXemThongTin",
        "yeuCauMoDemo",
        "yeuCauMoDemoBang",
        "yeuCauXemTop10Sach",
        "yeuCauXemTop10DocGia"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'yeuCauXemChiTiet'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'yeuCauThemSachMoi'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'yeuCauDangKyDocGia'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'yeuCauTaoPhieuMuon'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'yeuCauSaoLuu'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'yeuCauXemThongTin'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'yeuCauMoDemo'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'yeuCauMoDemoBang'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'yeuCauXemTop10Sach'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'yeuCauXemTop10DocGia'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<WidgetBangDieuKhien, qt_meta_tag_ZN19WidgetBangDieuKhienE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject WidgetBangDieuKhien::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19WidgetBangDieuKhienE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19WidgetBangDieuKhienE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN19WidgetBangDieuKhienE_t>.metaTypes,
    nullptr
} };

void WidgetBangDieuKhien::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WidgetBangDieuKhien *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->yeuCauXemChiTiet((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->yeuCauThemSachMoi(); break;
        case 2: _t->yeuCauDangKyDocGia(); break;
        case 3: _t->yeuCauTaoPhieuMuon(); break;
        case 4: _t->yeuCauSaoLuu(); break;
        case 5: _t->yeuCauXemThongTin(); break;
        case 6: _t->yeuCauMoDemo(); break;
        case 7: _t->yeuCauMoDemoBang(); break;
        case 8: _t->yeuCauXemTop10Sach(); break;
        case 9: _t->yeuCauXemTop10DocGia(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (WidgetBangDieuKhien::*)(const QString & )>(_a, &WidgetBangDieuKhien::yeuCauXemChiTiet, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (WidgetBangDieuKhien::*)()>(_a, &WidgetBangDieuKhien::yeuCauThemSachMoi, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (WidgetBangDieuKhien::*)()>(_a, &WidgetBangDieuKhien::yeuCauDangKyDocGia, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (WidgetBangDieuKhien::*)()>(_a, &WidgetBangDieuKhien::yeuCauTaoPhieuMuon, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (WidgetBangDieuKhien::*)()>(_a, &WidgetBangDieuKhien::yeuCauSaoLuu, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (WidgetBangDieuKhien::*)()>(_a, &WidgetBangDieuKhien::yeuCauXemThongTin, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (WidgetBangDieuKhien::*)()>(_a, &WidgetBangDieuKhien::yeuCauMoDemo, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (WidgetBangDieuKhien::*)()>(_a, &WidgetBangDieuKhien::yeuCauMoDemoBang, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (WidgetBangDieuKhien::*)()>(_a, &WidgetBangDieuKhien::yeuCauXemTop10Sach, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (WidgetBangDieuKhien::*)()>(_a, &WidgetBangDieuKhien::yeuCauXemTop10DocGia, 9))
            return;
    }
}

const QMetaObject *WidgetBangDieuKhien::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WidgetBangDieuKhien::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19WidgetBangDieuKhienE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int WidgetBangDieuKhien::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void WidgetBangDieuKhien::yeuCauXemChiTiet(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void WidgetBangDieuKhien::yeuCauThemSachMoi()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void WidgetBangDieuKhien::yeuCauDangKyDocGia()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void WidgetBangDieuKhien::yeuCauTaoPhieuMuon()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void WidgetBangDieuKhien::yeuCauSaoLuu()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void WidgetBangDieuKhien::yeuCauXemThongTin()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void WidgetBangDieuKhien::yeuCauMoDemo()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void WidgetBangDieuKhien::yeuCauMoDemoBang()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void WidgetBangDieuKhien::yeuCauXemTop10Sach()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void WidgetBangDieuKhien::yeuCauXemTop10DocGia()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}
QT_WARNING_POP
