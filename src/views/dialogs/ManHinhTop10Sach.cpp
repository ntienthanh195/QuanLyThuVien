#include "views/dialogs/ManHinhTop10Sach.h"
#include "models/DocGia.h"
#include "models/DauSach.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include "utils/LanguageManager.h"
#include <QMessageBox>
#include <QAbstractTableModel>
#include <QFont>
#include <algorithm>

extern DauSach* dsDauSach[];
extern int soLuongDauSach;

namespace {
    class TopSachModel : public QAbstractTableModel {
    public:
        TopSachModel(std::vector<TopSachDTO> *data, QObject *parent = nullptr) : QAbstractTableModel(parent), m_data(data) {}
        int rowCount(const QModelIndex &parent = QModelIndex()) const override { Q_UNUSED(parent); return m_data ? (int)m_data->size() : 0; }
        int columnCount(const QModelIndex &parent = QModelIndex()) const override { Q_UNUSED(parent); return 6; }
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
            if (!m_data || !index.isValid()) return QVariant();
            int r = index.row(); if (r < 0 || r >= (int)m_data->size()) return QVariant();
            const TopSachDTO &t = (*m_data)[r];
            PTRDS book = t.dauSach;
            if (!book) return QVariant();
            if (role == Qt::DisplayRole) {
                switch (index.column()) {
                case 0: return (r < 3) ? QStringLiteral("🥇🥈🥉").mid(r,1) : QString::number(r + 1);
                case 1: return QString::fromStdString(book->ISBN);
                case 2: return QString::fromStdString(book->tenSach);
                case 3: return QString::fromStdString(book->tacGia);
                case 4: return QString::fromStdString(book->theLoai);
                case 5: return QString::number(book->soLuotMuon);
                }
            }
            if (role == Qt::FontRole && (index.column() == 0 || index.column() == 5)) { QFont f; f.setBold(true); return f; }
            if (role == Qt::ForegroundRole && index.column() == 5) return QBrush(QColor(231,76,60));
            if (role == Qt::BackgroundRole && index.column() == 0) return QBrush(QColor(245,245,245));
            return QVariant();
        }
        QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override {
            if (orientation==Qt::Horizontal && role==Qt::DisplayRole) {
                switch (section) {
                case 0: return LanguageManager::instance().getText("TABLE_HEADER_RANK");
                case 1: return LanguageManager::instance().getText("LBL_ISBN");
                case 2: return LanguageManager::instance().getText("TABLE_HEADER_TITLE");
                case 3: return LanguageManager::instance().getText("LBL_AUTHOR");
                case 4: return LanguageManager::instance().getText("LBL_CATEGORY");
                case 5: return LanguageManager::instance().getText("TABLE_HEADER_BORROW_COUNT");
                }
            }
            return QAbstractTableModel::headerData(section, orientation, role);
        }
        void refresh() { beginResetModel(); endResetModel(); }
    private:
        std::vector<TopSachDTO> *m_data;
    };
}

ManHinhTop10Sach::ManHinhTop10Sach(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle(LanguageManager::instance().getText("TITLE_TOP10_BOOKS"));
    setMinimumSize(900, 600);
    setModal(true);

    thietLapGiaoDien();
    apDungPhongCachHienDai();
    taiDuLieuTop10();
}

void ManHinhTop10Sach::thietLapGiaoDien() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel* titleLabel = new QLabel(LanguageManager::instance().getText("HEADER_TOP10_BOOKS"));
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 20px; "
        "font-weight: bold; "
        "color: white; "
        "padding: 15px; "
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 #1e3a8a, stop:1 #1e40af); "
        "border-radius: 8px; "
        "margin-bottom: 10px;"
    );
    mainLayout->addWidget(titleLabel);

    // Info label
    QLabel* infoLabel = new QLabel(
        LanguageManager::instance().getText("MSG_STATS_BOOKS")
    );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet(
        "color: #1e293b; "
        "font-size: 13px; "
        "padding: 10px; "
        "background-color: #f1f5f9; "
        "border-left: 4px solid #1e40af; "
        "border-radius: 5px;"
    );
    mainLayout->addWidget(infoLabel);

    // Table (model/view)
    bangKetQua = new QTableView(this);
    // define lightweight model inline
    class TopSachModel : public QAbstractTableModel {
    public:
        TopSachModel(std::vector<TopSachDTO> *data, QObject *parent = nullptr) : QAbstractTableModel(parent), m_data(data) {}
        int rowCount(const QModelIndex &parent = QModelIndex()) const override { Q_UNUSED(parent); return m_data ? (int)m_data->size() : 0; }
        int columnCount(const QModelIndex &parent = QModelIndex()) const override { Q_UNUSED(parent); return 6; }
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
            if (!m_data || !index.isValid()) return QVariant();
            int r = index.row(); if (r < 0 || r >= (int)m_data->size()) return QVariant();
            const TopSachDTO &t = (*m_data)[r];
            PTRDS book = t.dauSach;
            if (!book) return QVariant();
            if (role == Qt::DisplayRole) {
                switch (index.column()) {
                case 0: return QString::number(r + 1);
                case 1: return QString::fromStdString(book->ISBN);
                case 2: return QString::fromStdString(book->tenSach);
                case 3: return QString::fromStdString(book->tacGia);
                case 4: return QString::fromStdString(book->theLoai);
                case 5: return QString::number(book->soLuotMuon);
                }
            }
            if (role == Qt::FontRole && (index.column() == 0 || index.column() == 5)) { QFont f; f.setBold(true); return f; }
            if (role == Qt::ForegroundRole && index.column() == 5) return QBrush(QColor(231,76,60));
            if (role == Qt::BackgroundRole && index.column() == 0) return QBrush(QColor(245,245,245));
            return QVariant();
        }
        QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override {
            if (orientation==Qt::Horizontal && role==Qt::DisplayRole) {
                switch (section) {
                case 0: return LanguageManager::instance().getText("TABLE_HEADER_RANK");
                case 1: return LanguageManager::instance().getText("LBL_ISBN");
                case 2: return LanguageManager::instance().getText("TABLE_HEADER_TITLE");
                case 3: return LanguageManager::instance().getText("LBL_AUTHOR");
                case 4: return LanguageManager::instance().getText("LBL_CATEGORY");
                case 5: return LanguageManager::instance().getText("TABLE_HEADER_BORROW_COUNT");
                }
            }
            return QAbstractTableModel::headerData(section, orientation, role);
        }
        void refresh() { beginResetModel(); endResetModel(); }
    private:
        std::vector<TopSachDTO> *m_data;
    };

    modelKetQua = new TopSachModel(&danhSachTop, this);
    bangKetQua->setModel(modelKetQua);
    bangKetQua->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangKetQua->setSelectionMode(QAbstractItemView::SingleSelection);
    bangKetQua->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bangKetQua->setAlternatingRowColors(true);
    bangKetQua->horizontalHeader()->setStretchLastSection(false);
    bangKetQua->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    bangKetQua->verticalHeader()->setVisible(false);
    // Set column widths
    bangKetQua->setColumnWidth(0, 60);
    bangKetQua->setColumnWidth(1, 130);
    bangKetQua->setColumnWidth(2, 280);
    bangKetQua->setColumnWidth(3, 180);
    bangKetQua->setColumnWidth(4, 120);
    bangKetQua->setColumnWidth(5, 120);
    mainLayout->addWidget(bangKetQua);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    nutLamMoi = new QPushButton(LanguageManager::instance().getText("BTN_REFRESH"));
    nutLamMoi->setObjectName("refreshButton");
    nutLamMoi->setMinimumHeight(40);
    connect(nutLamMoi, &QPushButton::clicked, this, &ManHinhTop10Sach::taiDuLieuTop10);

    nutDong = new QPushButton(LanguageManager::instance().getText("BTN_CLOSE"));
    nutDong->setObjectName("closeButton");
    nutDong->setMinimumHeight(40);
    connect(nutDong, &QPushButton::clicked, this, &QDialog::accept);

    buttonLayout->addStretch();
    buttonLayout->addWidget(nutLamMoi);
    buttonLayout->addWidget(nutDong);

    mainLayout->addLayout(buttonLayout);
}

void ManHinhTop10Sach::apDungPhongCachHienDai() {
    setStyleSheet(R"(
        QDialog {
            background-color: #ffffff;
        }
        
        QTableView {
            background-color: white;
            border: 2px solid #bdc3c7;
            border-radius: 8px;
            gridline-color: #ecf0f1;
            font-size: 13px;
        }
        
        QTableView::item {
            padding: 8px;
            border-bottom: 1px solid #ecf0f1;
        }
        
        QTableView::item:selected {
            background-color: #3498db;
            color: white;
        }
        
        QHeaderView::section {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498db, stop:1 #2980b9);
            color: white;
            padding: 10px;
            border: none;
            font-weight: bold;
            font-size: 13px;
        }
        
        QHeaderView::section:first {
            border-top-left-radius: 6px;
        }
        
        QHeaderView::section:last {
            border-top-right-radius: 6px;
        }
        
        QPushButton#refreshButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498db, stop:1 #2980b9);
            color: white;
            border: none;
            border-radius: 5px;
            padding: 10px 30px;
            font-weight: bold;
            font-size: 14px;
            min-width: 120px;
        }
        
        QPushButton#refreshButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #5dade2, stop:1 #3498db);
        }
        
        QPushButton#refreshButton:pressed {
            background: #2980b9;
        }
        
        QPushButton#closeButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #7f8c8d, stop:1 #6c7a7b);
            color: white;
            border: none;
            border-radius: 5px;
            padding: 10px 30px;
            font-weight: bold;
            font-size: 14px;
            min-width: 100px;
        }
        
        QPushButton#closeButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #95a5a6, stop:1 #7f8c8d);
        }
        
        QPushButton#closeButton:pressed {
            background: #6c7a7b;
        }
    )");
}

void ManHinhTop10Sach::taiDuLieuTop10() {
    // Fill vector and refresh model
    danhSachTop.clear();
    TopSachDTO topBooks[10];
    int count = LayTopSach(dsDauSach, soLuongDauSach, topBooks);
    if (count == 0) {
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_INFO"),
            LanguageManager::instance().getText("MSG_NO_DATA"));
        return;
    }
    for (int i = 0; i < count; ++i) danhSachTop.push_back(topBooks[i]);
    if (modelKetQua) { if (auto m = dynamic_cast<QAbstractTableModel*>(modelKetQua)) { Q_UNUSED(m); /*call refresh if concrete*/ } }
    // If model provides refresh method, call via dynamic_cast
    if (modelKetQua) { if (auto m = dynamic_cast<TopSachModel*>(modelKetQua)) m->refresh(); }
    // Use uniform row height instead of per-row calls for performance
    bangKetQua->verticalHeader()->setDefaultSectionSize(45);
}
