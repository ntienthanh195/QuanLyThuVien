#include "giao_dien/hop_thoai/HopThoaiTop10.h"
#include "models/QuanLySach.h"
#include "models/DocGia.h"
#include "utils/ResponsiveHelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>

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
    
    // Bảng top 10
    bangTop10 = new QTableWidget();
    bangTop10->setColumnCount(6);
    bangTop10->setHorizontalHeaderLabels({"Hạng", "ISBN", "Tên Sách", "Tác Giả", "Thể Loại", "Lượt Mượn"});
    
    int fontSize = ResponsiveHelper::getScaledFontSize(13, screenSize);
    bangTop10->setStyleSheet(QString(
        "QTableWidget { background-color: #1E1E2D; color: #FFFFFF; border: 2px solid #2B2B40; border-radius: 12px; font-size: %1px; }"
        "QTableWidget::item { padding: 10px; }"
        "QTableWidget::item:selected { background-color: #3699FF; }"
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
    
    bangTop10->setRowCount(soLuong);
    
    for (int i = 0; i < soLuong; i++) {
        if (!topSach[i].dauSach) continue;
        
        // Hạng
        QTableWidgetItem *itemHang = new QTableWidgetItem(QString::number(i + 1));
        itemHang->setTextAlignment(Qt::AlignCenter);
        if (i == 0) itemHang->setForeground(QBrush(QColor("#FFD700"))); // Vàng
        else if (i == 1) itemHang->setForeground(QBrush(QColor("#C0C0C0"))); // Bạc
        else if (i == 2) itemHang->setForeground(QBrush(QColor("#CD7F32"))); // Đồng
        bangTop10->setItem(i, 0, itemHang);
        
        // ISBN
        QTableWidgetItem *itemISBN = new QTableWidgetItem(QString::fromStdString(topSach[i].dauSach->ISBN));
        bangTop10->setItem(i, 1, itemISBN);
        
        // Tên sách
        QTableWidgetItem *itemTen = new QTableWidgetItem(QString::fromStdString(topSach[i].dauSach->tenSach));
        itemTen->setFont(QFont("Segoe UI", 10, QFont::Bold));
        bangTop10->setItem(i, 2, itemTen);
        
        // Tác giả
        QTableWidgetItem *itemTacGia = new QTableWidgetItem(QString::fromStdString(topSach[i].dauSach->tacGia));
        bangTop10->setItem(i, 3, itemTacGia);
        
        // Thể loại
        QTableWidgetItem *itemTheLoai = new QTableWidgetItem(QString::fromStdString(topSach[i].dauSach->theLoai));
        bangTop10->setItem(i, 4, itemTheLoai);
        
        // Lượt mượn
        QTableWidgetItem *itemLuotMuon = new QTableWidgetItem(QString::number(topSach[i].dauSach->soLuotMuon));
        itemLuotMuon->setTextAlignment(Qt::AlignCenter);
        itemLuotMuon->setFont(QFont("Segoe UI", 11, QFont::Bold));
        itemLuotMuon->setForeground(QBrush(QColor("#1BC5BD")));
        bangTop10->setItem(i, 5, itemLuotMuon);
    }
    
    // Thêm emoji cho 3 hàng đầu
    if (soLuong > 0) bangTop10->item(0, 0)->setText("🥇");
    if (soLuong > 1) bangTop10->item(1, 0)->setText("🥈");
    if (soLuong > 2) bangTop10->item(2, 0)->setText("🥉");
}
