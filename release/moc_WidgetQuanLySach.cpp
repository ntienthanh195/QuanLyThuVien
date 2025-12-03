/****************************************************************************
** Meta object code from reading C++ file 'WidgetQuanLySach.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/views/WidgetQuanLySach.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'WidgetQuanLySach.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN14BookTableModelE_t {};
} // unnamed namespace

template <> constexpr inline auto BookTableModel::qt_create_metaobjectdata<qt_meta_tag_ZN14BookTableModelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "BookTableModel"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<BookTableModel, qt_meta_tag_ZN14BookTableModelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject BookTableModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractTableModel::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14BookTableModelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14BookTableModelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14BookTableModelE_t>.metaTypes,
    nullptr
} };

void BookTableModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<BookTableModel *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *BookTableModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BookTableModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14BookTableModelE_t>.strings))
        return static_cast<void*>(this);
    return QAbstractTableModel::qt_metacast(_clname);
}

int BookTableModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractTableModel::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_tag_ZN16WidgetQuanLySachE_t {};
} // unnamed namespace

template <> constexpr inline auto WidgetQuanLySach::qt_create_metaobjectdata<qt_meta_tag_ZN16WidgetQuanLySachE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "WidgetQuanLySach",
        "backToHome",
        "",
        "onSearchClicked",
        "onClearSearchClicked",
        "onAddBookClicked",
        "onEditBookClicked",
        "onDeleteBookClicked",
        "onViewDetailsClicked",
        "onAddCopyClicked",
        "onBackClicked",
        "onTableDoubleClicked",
        "QModelIndex",
        "index",
        "onSearchTextChanged",
        "performSearch",
        "onExportCSV",
        "onImportCSV",
        "onPrintList",
        "onBulkDelete",
        "onFilterChanged",
        "onRefreshTop10",
        "onRefreshThongKe"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'backToHome'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onSearchClicked'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onClearSearchClicked'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAddBookClicked'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onEditBookClicked'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDeleteBookClicked'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onViewDetailsClicked'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAddCopyClicked'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onBackClicked'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onTableDoubleClicked'
        QtMocHelpers::SlotData<void(const QModelIndex &)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 12, 13 },
        }}),
        // Slot 'onSearchTextChanged'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'performSearch'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onExportCSV'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onImportCSV'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onPrintList'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onBulkDelete'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFilterChanged'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRefreshTop10'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRefreshThongKe'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<WidgetQuanLySach, qt_meta_tag_ZN16WidgetQuanLySachE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject WidgetQuanLySach::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16WidgetQuanLySachE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16WidgetQuanLySachE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16WidgetQuanLySachE_t>.metaTypes,
    nullptr
} };

void WidgetQuanLySach::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WidgetQuanLySach *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->backToHome(); break;
        case 1: _t->onSearchClicked(); break;
        case 2: _t->onClearSearchClicked(); break;
        case 3: _t->onAddBookClicked(); break;
        case 4: _t->onEditBookClicked(); break;
        case 5: _t->onDeleteBookClicked(); break;
        case 6: _t->onViewDetailsClicked(); break;
        case 7: _t->onAddCopyClicked(); break;
        case 8: _t->onBackClicked(); break;
        case 9: _t->onTableDoubleClicked((*reinterpret_cast<std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 10: _t->onSearchTextChanged(); break;
        case 11: _t->performSearch(); break;
        case 12: _t->onExportCSV(); break;
        case 13: _t->onImportCSV(); break;
        case 14: _t->onPrintList(); break;
        case 15: _t->onBulkDelete(); break;
        case 16: _t->onFilterChanged(); break;
        case 17: _t->onRefreshTop10(); break;
        case 18: _t->onRefreshThongKe(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (WidgetQuanLySach::*)()>(_a, &WidgetQuanLySach::backToHome, 0))
            return;
    }
}

const QMetaObject *WidgetQuanLySach::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WidgetQuanLySach::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16WidgetQuanLySachE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int WidgetQuanLySach::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 19;
    }
    return _id;
}

// SIGNAL 0
void WidgetQuanLySach::backToHome()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
