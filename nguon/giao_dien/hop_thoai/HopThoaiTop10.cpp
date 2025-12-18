#include "giao_dien/hop_thoai/HopThoaiTop10.h"
#include "models/QuanLySach.h"
#include "models/DocGia.h"
#include "utils/ResponsiveHelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QAbstractTableModel>
#include <QFont>
#include <algorithm>

namespace {
    class Top10Model : public QAbstractTableModel {
    public:
        Top10Model(std::vector<TopSachDTO> *data, QObject *parent=nullptr) : QAbstractTableModel(parent), m_data(data) {}
        int rowCount(const QModelIndex &parent = QModelIndex()) const override { Q_UNUSED(parent); return m_data ? (int)m_data->size() : 0; }
        int columnCount(const QModelIndex &parent = QModelIndex()) const override { Q_UNUSED(parent); return 6; }
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
            if (!m_data || !index.isValid()) return QVariant();
            int r = index.row(); if (r < 0 || r >= (int)m_data->size()) return QVariant();
            PTRDS book = (*m_data)[r].dauSach;
            if (!book) return QVariant();
            if (role == Qt::DisplayRole) {
                switch (index.column()) {
                case 0: return QString::number(r+1);
                case 1: return QString::fromStdString(book->ISBN);
                case 2: return QString::fromStdString(book->tenSach);
                case 3: return QString::fromStdString(book->tacGia);
                case 4: return QString::fromStdString(book->theLoai);
                case 5: return book->soLuotMuon;
                }
            }
            if (role == Qt::FontRole && (index.column() == 0 || index.column() == 5)) { QFont f; f.setBold(true); return f; }
            if (role == Qt::ForegroundRole && index.column() == 5) return QBrush(QColor("#1BC5BD"));
            return QVariant();
        }
        QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override {
            if (orientation==Qt::Horizontal && role==Qt::DisplayRole) {
                switch (section) {
                case 0: return QStringLiteral("Hạng");
                case 1: return QStringLiteral("ISBN");
                case 2: return QStringLiteral("Tên Sách");
                case 3: return QStringLiteral("Tác Giả");
                case 4: return QStringLiteral("Thể Loại");
                case 5: return QStringLiteral("Lượt Mượn");
                }
            }
            return QAbstractTableModel::headerData(section, orientation, role);
        }
        void refresh() { beginResetModel(); endResetModel(); }
    private:
        std::vector<TopSachDTO> *m_data;
    };
}

HopThoaiTop10::HopThoaiTop10(QWidget *cha) : QDialog(cha) {
    setWindowTitle("🏆 Top 10 Sách Mượn Nhiều Nhất");
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    setMinimumSize(ResponsiveHelper::getScaledWidth(800, screenSize), 
                   ResponsiveHelper::getScaledHeight(600, screenSize));
    
    thieLapGiaoDien();
    taiDuLieuTop10();
}

void HopThoaiTop10::thieLapGiaoDien() {
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    QVBoxLayout *boTriChinh = new QVBoxLayout(this);
    int margin = ResponsiveHelper::getScaledMargin(20, screenSize);
    boTriChinh->setContentsMargins(margin, margin, margin, margin);
    
    QLabel *tieuDe = new QLabel("🏆 TOP 10 SÁCH ĐƯỢC MƯỢN NHIỀU NHẤT");
    int titleSize = ResponsiveHelper::getScaledFontSize(22, screenSize);
    tieuDe->setStyleSheet(QString("font-size: %1px; font-weight: bold; color: #FFFFFF; margin-bottom: 20px;").arg(titleSize));
    tieuDe->setAlignment(Qt::AlignCenter);
    boTriChinh->addWidget(tieuDe);
    
    // Bảng top 10 (model/view)
    bangTop10 = new QTableView();
    modelTop10 = new Top10Model(&danhSachTop, this);
    bangTop10->setModel(modelTop10);
    int fontSize = ResponsiveHelper::getScaledFontSize(13, screenSize);
    bangTop10->setStyleSheet(QString(
        "QTableView { background-color: #1E1E2D; color: #FFFFFF; border: 2px solid #2B2B40; border-radius: 12px; font-size: %1px; }"
        "QTableView::item { padding: 10px; }"
        "QTableView::item:selected { background-color: #3699FF; }"
        "QHeaderView::section { background-color: #252534; color: #FFFFFF; padding: 12px; border: none; font-weight: bold; }"
    ).arg(fontSize));
    
    bangTop10->horizontalHeader()->setStretchLastSection(true);
    bangTop10->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    bangTop10->setColumnWidth(0, 80);
    bangTop10->setColumnWidth(1, 150);
    bangTop10->setColumnWidth(2, 250);
    bangTop10->setColumnWidth(3, 180);
    bangTop10->setColumnWidth(4, 120);
    
    bangTop10->verticalHeader()->setVisible(false);
    bangTop10->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangTop10->setSelectionMode(QAbstractItemView::SingleSelection);
    bangTop10->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bangTop10->setAlternatingRowColors(true);
    
    boTriChinh->addWidget(bangTop10);
    
    // Nút đóng
    QHBoxLayout *boTriNut = new QHBoxLayout();
    int buttonHeight = ResponsiveHelper::getScaledHeight(45, screenSize);
    
    nutDong = new QPushButton("✖ Đóng");
    nutDong->setMinimumHeight(buttonHeight);
    nutDong->setStyleSheet("QPushButton { background-color: #3699FF; color: white; border: none; border-radius: 8px; font-weight: bold; padding: 0 30px; } QPushButton:hover { background-color: #187DE4; }");
    connect(nutDong, &QPushButton::clicked, this, &QDialog::accept);
    
    boTriNut->addStretch();
    boTriNut->addWidget(nutDong);
    boTriChinh->addLayout(boTriNut);
}

void HopThoaiTop10::taiDuLieuTop10() {
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    TopSachDTO topSach[10];
    int soLuong = LayTopSach(dsDauSach, soLuongDauSach, topSach);
    
    danhSachTop.clear();
    for (int i = 0; i < soLuong; ++i) if (topSach[i].dauSach) danhSachTop.push_back(topSach[i]);
    if (modelTop10) { if (auto m = dynamic_cast<Top10Model*>(modelTop10)) m->refresh(); }
}
