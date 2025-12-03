/****************************************************************************
** Meta object code from reading C++ file 'WidgetQuanLyDocGia.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/views/WidgetQuanLyDocGia.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'WidgetQuanLyDocGia.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN16ReaderTableModelE_t {};
} // unnamed namespace

template <> constexpr inline auto ReaderTableModel::qt_create_metaobjectdata<qt_meta_tag_ZN16ReaderTableModelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ReaderTableModel"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ReaderTableModel, qt_meta_tag_ZN16ReaderTableModelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ReaderTableModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractTableModel::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16ReaderTableModelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16ReaderTableModelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16ReaderTableModelE_t>.metaTypes,
    nullptr
} };

void ReaderTableModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ReaderTableModel *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *ReaderTableModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ReaderTableModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16ReaderTableModelE_t>.strings))
        return static_cast<void*>(this);
    return QAbstractTableModel::qt_metacast(_clname);
}

int ReaderTableModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractTableModel::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_tag_ZN18WidgetQuanLyDocGiaE_t {};
} // unnamed namespace

template <> constexpr inline auto WidgetQuanLyDocGia::qt_create_metaobjectdata<qt_meta_tag_ZN18WidgetQuanLyDocGiaE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "WidgetQuanLyDocGia",
        "backToHome",
        "",
        "onSearchClicked",
        "onClearSearchClicked",
        "onDeleteReaderClicked",
        "onViewLoansClicked",
        "onBackClicked",
        "onTableDoubleClicked",
        "QModelIndex",
        "index",
        "onSearchTextChanged",
        "performSearch",
        "onToggleStatus",
        "onViewHistory",
        "onPrintCard",
        "onExportCSV",
        "onImportCSV",
        "onShowWarnings",
        "onRefreshQuaHan",
        "onTableQuaHanDoubleClicked"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'backToHome'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onSearchClicked'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onClearSearchClicked'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDeleteReaderClicked'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onViewLoansClicked'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onBackClicked'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onTableDoubleClicked'
        QtMocHelpers::SlotData<void(const QModelIndex &)>(8, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 9, 10 },
        }}),
        // Slot 'onSearchTextChanged'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'performSearch'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onToggleStatus'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onViewHistory'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onPrintCard'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onExportCSV'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onImportCSV'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onShowWarnings'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRefreshQuaHan'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onTableQuaHanDoubleClicked'
        QtMocHelpers::SlotData<void(const QModelIndex &)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 9, 10 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<WidgetQuanLyDocGia, qt_meta_tag_ZN18WidgetQuanLyDocGiaE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject WidgetQuanLyDocGia::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18WidgetQuanLyDocGiaE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18WidgetQuanLyDocGiaE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN18WidgetQuanLyDocGiaE_t>.metaTypes,
    nullptr
} };

void WidgetQuanLyDocGia::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WidgetQuanLyDocGia *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->backToHome(); break;
        case 1: _t->onSearchClicked(); break;
        case 2: _t->onClearSearchClicked(); break;
        case 3: _t->onDeleteReaderClicked(); break;
        case 4: _t->onViewLoansClicked(); break;
        case 5: _t->onBackClicked(); break;
        case 6: _t->onTableDoubleClicked((*reinterpret_cast<std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 7: _t->onSearchTextChanged(); break;
        case 8: _t->performSearch(); break;
        case 9: _t->onToggleStatus(); break;
        case 10: _t->onViewHistory(); break;
        case 11: _t->onPrintCard(); break;
        case 12: _t->onExportCSV(); break;
        case 13: _t->onImportCSV(); break;
        case 14: _t->onShowWarnings(); break;
        case 15: _t->onRefreshQuaHan(); break;
        case 16: _t->onTableQuaHanDoubleClicked((*reinterpret_cast<std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (WidgetQuanLyDocGia::*)()>(_a, &WidgetQuanLyDocGia::backToHome, 0))
            return;
    }
}

const QMetaObject *WidgetQuanLyDocGia::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WidgetQuanLyDocGia::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18WidgetQuanLyDocGiaE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int WidgetQuanLyDocGia::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void WidgetQuanLyDocGia::backToHome()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
