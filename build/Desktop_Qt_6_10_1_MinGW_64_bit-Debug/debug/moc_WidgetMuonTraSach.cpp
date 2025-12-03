/****************************************************************************
** Meta object code from reading C++ file 'WidgetMuonTraSach.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../include/views/WidgetMuonTraSach.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'WidgetMuonTraSach.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN14LoanTableModelE_t {};
} // unnamed namespace

template <> constexpr inline auto LoanTableModel::qt_create_metaobjectdata<qt_meta_tag_ZN14LoanTableModelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "LoanTableModel"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<LoanTableModel, qt_meta_tag_ZN14LoanTableModelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject LoanTableModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractTableModel::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14LoanTableModelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14LoanTableModelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14LoanTableModelE_t>.metaTypes,
    nullptr
} };

void LoanTableModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<LoanTableModel *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *LoanTableModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LoanTableModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14LoanTableModelE_t>.strings))
        return static_cast<void*>(this);
    return QAbstractTableModel::qt_metacast(_clname);
}

int LoanTableModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractTableModel::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_tag_ZN17WidgetMuonTraSachE_t {};
} // unnamed namespace

template <> constexpr inline auto WidgetMuonTraSach::qt_create_metaobjectdata<qt_meta_tag_ZN17WidgetMuonTraSachE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "WidgetMuonTraSach",
        "backToHome",
        "",
        "onTimDocGiaClicked",
        "onResetWorkflow",
        "onMuonSachMoiClicked",
        "onTraSachClicked",
        "onBaoMatClicked",
        "onTimSachMuonClicked",
        "onChonBanSaoChanged",
        "index",
        "onXacNhanMuonClicked",
        "onChonSachTraChanged",
        "onXacNhanTraClicked",
        "onXacNhanBaoMatClicked",
        "onTableDoubleClicked",
        "QModelIndex",
        "onMaTheEnterPressed"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'backToHome'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onTimDocGiaClicked'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onResetWorkflow'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onMuonSachMoiClicked'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onTraSachClicked'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onBaoMatClicked'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onTimSachMuonClicked'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onChonBanSaoChanged'
        QtMocHelpers::SlotData<void(int)>(9, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 10 },
        }}),
        // Slot 'onXacNhanMuonClicked'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onChonSachTraChanged'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onXacNhanTraClicked'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onXacNhanBaoMatClicked'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onTableDoubleClicked'
        QtMocHelpers::SlotData<void(const QModelIndex &)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 16, 10 },
        }}),
        // Slot 'onMaTheEnterPressed'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<WidgetMuonTraSach, qt_meta_tag_ZN17WidgetMuonTraSachE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject WidgetMuonTraSach::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17WidgetMuonTraSachE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17WidgetMuonTraSachE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN17WidgetMuonTraSachE_t>.metaTypes,
    nullptr
} };

void WidgetMuonTraSach::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WidgetMuonTraSach *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->backToHome(); break;
        case 1: _t->onTimDocGiaClicked(); break;
        case 2: _t->onResetWorkflow(); break;
        case 3: _t->onMuonSachMoiClicked(); break;
        case 4: _t->onTraSachClicked(); break;
        case 5: _t->onBaoMatClicked(); break;
        case 6: _t->onTimSachMuonClicked(); break;
        case 7: _t->onChonBanSaoChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->onXacNhanMuonClicked(); break;
        case 9: _t->onChonSachTraChanged(); break;
        case 10: _t->onXacNhanTraClicked(); break;
        case 11: _t->onXacNhanBaoMatClicked(); break;
        case 12: _t->onTableDoubleClicked((*reinterpret_cast<std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 13: _t->onMaTheEnterPressed(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (WidgetMuonTraSach::*)()>(_a, &WidgetMuonTraSach::backToHome, 0))
            return;
    }
}

const QMetaObject *WidgetMuonTraSach::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WidgetMuonTraSach::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17WidgetMuonTraSachE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int WidgetMuonTraSach::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void WidgetMuonTraSach::backToHome()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
