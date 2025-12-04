#include "giao_dien/WidgetQuanLySach.h"
#include "utils/FilePathManager.h"
#include "giao_dien/hop_thoai/HopThoaiThemSach.h"
#include "giao_dien/hop_thoai/HopThoaiSuaSach.h"
#include "giao_dien/hop_thoai/HopThoaiThemBanSao.h"
#include "models/QuanLySach.h"
#include "models/ThaoTacFile.h"
#include "utils/ResponsiveHelper.h"
#include "utils/AnimationHelper.h"
#include "utils/ConfirmationDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include <QGraphicsDropShadowEffect>

WidgetQuanLySach::WidgetQuanLySach(QWidget *cha) 
    : QWidget(cha), dauSachDangChon(nullptr) {
    setStyleSheet(
        "QWidget#WidgetQuanLySach {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "        stop:0 #1a1d29, stop:0.5 #1e2230, stop:1 #1a1d29);"
        "}"
    );
    setObjectName("WidgetQuanLySach");
    thieLapGiaoDien();
    taiLaiDanhSach();
}

WidgetQuanLySach::~WidgetQuanLySach() {}

void WidgetQuanLySach::thieLapGiaoDien() {
    boTriChinh = new QVBoxLayout(this);
    boTriChinh->setContentsMargins(20, 20, 20, 20);
    boTriChinh->setSpacing(15);
    
    // Header với các nút action
    thieLapHeaderBar();
    
    // Search & Filter bar
    thieLapTimKiem();
    
    // Table
    thieLapBangSach();
}

void WidgetQuanLySach::thieLapHeaderBar() {
    QWidget *headerBar = new QWidget();
    headerBar->setStyleSheet("QWidget { background: transparent; }");
    QHBoxLayout *layout = new QHBoxLayout(headerBar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    
    // Create action buttons
    auto createButton = [](const QString &icon, const QString &text, const QString &color) -> QPushButton* {
        QPushButton *btn = new QPushButton(icon + " " + text);
        btn->setFixedHeight(36);
        btn->setMinimumWidth(110);
        
        // Simple hover/pressed colors
        QString hoverColor = (color == "#0078D4") ? "#106EBE" : "#3E3E42";
        QString pressedColor = (color == "#0078D4") ? "#005A9E" : "#2D2D30";
        
        btn->setStyleSheet(QString(
            "QPushButton {"
            "    background-color: %1;"
            "    color: white;"
            "    border: none;"
            "    border-radius: 4px;"
            "    font-size: 13px;"
            "    font-weight: 500;"
            "    padding: 0 16px;"
            "}"
            "QPushButton:hover {"
            "    background-color: %2;"
            "}"
            "QPushButton:pressed {"
            "    background-color: %3;"
            "}"
        ).arg(color, hoverColor, pressedColor));
        return btn;
    };
    
    // Buttons
    nutThemSach = createButton("+", "Thêm Sách", "#0078D4");
    nutNhapFile = createButton("⇩", "Nhập Excel", "#2D2D30");
    nutNhapFile->setStyleSheet(
        "QPushButton {"
        "    background-color: #2D2D30;"
        "    color: #CCCCCC;"
        "    border: 1px solid #3E3E42;"
        "    border-radius: 4px;"
        "    font-size: 13px;"
        "    padding: 0 16px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #3E3E42;"
        "    border: 1px solid #0078D4;"
        "}"
    );
    
    nutXuatFile = createButton("⎙", "In", "#2D2D30");
    nutXuatFile->setStyleSheet(nutNhapFile->styleSheet());
    
    nutSuaSach = createButton("✏", "Sửa", "#2D2D30");
    nutSuaSach->setStyleSheet(nutNhapFile->styleSheet());
    
    nutXemChiTiet = createButton("ⓘ", "Chi Tiết", "#2D2D30");
    nutXemChiTiet->setStyleSheet(nutNhapFile->styleSheet());
    
    nutXoaSach = createButton("✖", "Xóa", "#C42B1C");
    
    layout->addWidget(nutThemSach);
    layout->addWidget(nutNhapFile);
    layout->addWidget(nutXuatFile);
    layout->addWidget(nutSuaSach);
    layout->addWidget(nutXemChiTiet);
    layout->addWidget(nutXoaSach);
    layout->addStretch();
    
    boTriChinh->addWidget(headerBar);
    
    // Connect signals
    connect(nutThemSach, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutThemSach);
    connect(nutSuaSach, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutSuaSach);
    connect(nutXoaSach, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutXoaSach);
    connect(nutXemChiTiet, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutXemChiTiet);
    connect(nutNhapFile, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNhapDuLieuTu);
}

void WidgetQuanLySach::thieLapTimKiem() {
    QWidget *searchBar = new QWidget();
    searchBar->setStyleSheet("QWidget { background: transparent; }");
    QHBoxLayout *layout = new QHBoxLayout(searchBar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    
    // Search input
    oNhapTimKiem = new QLineEdit();
    oNhapTimKiem->setPlaceholderText("Nhập ISBN, tên sách, tác giả...");
    oNhapTimKiem->setFixedHeight(36);
    oNhapTimKiem->setStyleSheet(
        "QLineEdit {"
        "    background-color: #252830;"
        "    color: #E0E0E0;"
        "    border: 1px solid #3E4451;"
        "    border-radius: 5px;"
        "    padding: 0 12px;"
        "    font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "    border: 1px solid #0078D4;"
        "    background-color: #1E1E1E;"
        "}"
    );
    
    // Filter dropdowns
    hopChonTheLoai = new QComboBox();
    hopChonTheLoai->addItem("Tất cả thể loại");
    hopChonTheLoai->setFixedHeight(36);
    hopChonTheLoai->setStyleSheet(
        "QComboBox {"
        "    background-color: #252830;"
        "    color: #E0E0E0;"
        "    border: 1px solid #3E4451;"
        "    border-radius: 5px;"
        "    padding: 0 12px;"
        "    font-size: 13px;"
        "}"
        "QComboBox:hover {"
        "    border: 1px solid #0078D4;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "    width: 20px;"
        "}"
        "QComboBox::down-arrow {"
        "    image: none;"
        "    border-left: 4px solid transparent;"
        "    border-right: 4px solid transparent;"
        "    border-top: 5px solid #858585;"
        "    margin-right: 5px;"
        "}"
        "QComboBox QAbstractItemView {"
        "    background-color: #252526;"
        "    color: #CCCCCC;"
        "    selection-background-color: #37373D;"
        "    selection-color: #CCCCCC;"
        "    border: 1px solid #3E3E42;"
        "    padding: 4px;"
        "}"
    );
    
    hopChonTrangThai = new QComboBox();
    hopChonTrangThai->addItems({"Tất cả trạng thái", "Còn sách", "Hết sách"});
    hopChonTrangThai->setFixedHeight(36);
    hopChonTrangThai->setStyleSheet(hopChonTheLoai->styleSheet());
    
    // Search button
    nutTimKiem = new QPushButton("🔍 Tìm");
    nutTimKiem->setFixedHeight(36);
    nutTimKiem->setFixedWidth(80);
    nutTimKiem->setStyleSheet(
        "QPushButton {"
        "    background-color: #0078D4;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    font-size: 13px;"
        "    font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "    background-color: #106EBE;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #005A9E;"
        "}"
    );
    
    // Refresh button
    nutLamMoi = new QPushButton("🔄 Làm mới");
    nutLamMoi->setFixedHeight(36);
    nutLamMoi->setFixedWidth(100);
    nutLamMoi->setStyleSheet(
        "QPushButton {"
        "    background-color: #2D2D30;"
        "    color: #CCCCCC;"
        "    border: 1px solid #3E3E42;"
        "    border-radius: 4px;"
        "    font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #3E3E42;"
        "    border: 1px solid #0078D4;"
        "}"
    );
    
    layout->addWidget(oNhapTimKiem, 3);
    layout->addWidget(hopChonTheLoai, 1);
    layout->addWidget(hopChonTrangThai, 1);
    layout->addWidget(nutTimKiem);
    layout->addWidget(nutLamMoi);
    
    boTriChinh->addWidget(searchBar);
    
    // Tải danh sách thể loại
    taiDanhSachTheLoai();
    
    connect(oNhapTimKiem, &QLineEdit::textChanged, this, &WidgetQuanLySach::xuLyTimKiem);
    connect(hopChonTheLoai, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WidgetQuanLySach::xuLyLocTheLoai);
    connect(hopChonTrangThai, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WidgetQuanLySach::xuLyLocTrangThai);
    connect(nutTimKiem, &QPushButton::clicked, this, [this]() { xuLyTimKiem(oNhapTimKiem->text()); });
    connect(nutLamMoi, &QPushButton::clicked, this, &WidgetQuanLySach::taiLaiDanhSach);
}

void WidgetQuanLySach::thieLapBangSach() {
    bangSach = new QTableWidget();
    bangSach->setColumnCount(9);
    bangSach->setHorizontalHeaderLabels({
        "STT", "ISBN", "Tên Sách", "Tác Giả", "Thể Loại", "Năm XB", "Số Trang", "Trạng Thái", "Thao Tác"
    });
    bangSach->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangSach->setSelectionMode(QAbstractItemView::SingleSelection);
    bangSach->setAlternatingRowColors(true);
    
    // Natural color table styling
    bangSach->setStyleSheet(
        "QTableWidget {"
        "    background-color: #1e2230;"
        "    color: #E0E0E0;"
        "    gridline-color: #2d3140;"
        "    border: 1px solid #3E4451;"
        "    border-radius: 6px;"
        "    font-size: 12px;"
        "}"
        "QTableWidget::item {"
        "    padding: 10px 8px;"
        "    border-bottom: 1px solid #252830;"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: #2d5f8d;"
        "    color: #FFFFFF;"
        "}"
        "QTableWidget::item:alternate {"
        "    background-color: #252830;"
        "}"
        "QTableWidget::item:alternate:selected {"
        "    background-color: #2d5f8d;"
        "    color: #FFFFFF;"
        "}"
        "QHeaderView::section {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "        stop:0 #2d3140, stop:1 #252830);"
        "    color: #E0E0E0;"
        "    padding: 11px 8px;"
        "    border: none;"
        "    border-right: 1px solid #3E4451;"
        "    border-bottom: 2px solid #3E4451;"
        "    font-weight: 600;"
        "    font-size: 12px;"
        "}"
        "QTableWidget QScrollBar:vertical {"
        "    background: #1E1E1E;"
        "    width: 12px;"
        "    border: none;"
        "}"
        "QTableWidget QScrollBar::handle:vertical {"
        "    background: #3E3E42;"
        "    border-radius: 6px;"
        "    min-height: 20px;"
        "}"
        "QTableWidget QScrollBar::handle:vertical:hover {"
        "    background: #4E4E52;"
        "}"
    );
    
    // Set column widths
    bangSach->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    bangSach->setColumnWidth(0, 50);   // STT
    bangSach->setColumnWidth(1, 130);  // ISBN
    bangSach->setColumnWidth(2, 260);  // Tên Sách
    bangSach->setColumnWidth(3, 140);  // Tác Giả
    bangSach->setColumnWidth(4, 130);  // Thể Loại
    bangSach->setColumnWidth(5, 80);   // Năm XB
    bangSach->setColumnWidth(6, 80);   // Số Trang
    bangSach->setColumnWidth(7, 170);  // Trạng Thái (with badges)
    bangSach->setColumnWidth(8, 110);  // Thao Tác
    
    // Row height
    bangSach->verticalHeader()->setDefaultSectionSize(45);
    bangSach->verticalHeader()->setVisible(false);
    
    // Table height
    bangSach->setMinimumHeight(500);
    
    boTriChinh->addWidget(bangSach, 1);
    
    connect(bangSach, &QTableWidget::cellClicked, this, &WidgetQuanLySach::xuLyChonDong);
    
    // Double-click to show detail
    connect(bangSach, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        if (row >= 0 && row < bangSach->rowCount()) {
            QString isbn = bangSach->item(row, 1)->text();
            dauSachDangChon = nullptr;
            extern PTRDS dsDauSach[];
            extern int soLuongDauSach;
            for (int i = 0; i < soLuongDauSach; i++) {
                if (dsDauSach[i] && dsDauSach[i]->ISBN == isbn.toStdString()) {
                    dauSachDangChon = dsDauSach[i];
                    break;
                }
            }
            if (dauSachDangChon) {
                xuLyNutXemChiTiet();
            }
        }
    });
}

void WidgetQuanLySach::thieLapNutChucNang() {
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    
    nhomChucNang = new QGroupBox("⚙️ Chức Năng");
    int padding = ResponsiveHelper::getScaledSpacing(15, screenSize);
    nhomChucNang->setStyleSheet(QString(
        "QGroupBox {"
        "    background-color: #1E1E2D;"
        "    border: 1px solid #2B2B40;"
        "    border-radius: 12px;"
        "    padding: %1px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    color: #FFFFFF;"
        "}"
    ).arg(padding));
    
    QHBoxLayout *boTri = new QHBoxLayout(nhomChucNang);
    boTri->setSpacing(ResponsiveHelper::getScaledSpacing(15, screenSize));
    
    nutThemSach = new QPushButton(QIcon(":/icons/plus.svg"), " Thêm Sách");
    nutSuaSach = new QPushButton(QIcon(":/icons/edit.svg"), " Sửa");
    nutXoaSach = new QPushButton(QIcon(":/icons/trash.svg"), " Xóa");
    nutThemBanSao = new QPushButton(QIcon(":/icons/copy.svg"), " Thêm Bản Sao");
    nutXemChiTiet = new QPushButton(QIcon(":/icons/info.svg"), " Chi Tiết");
    
    // Tooltips
    nutThemSach->setToolTip("Thêm đầu sách mới vào thư viện");
    nutSuaSach->setToolTip("Sửa thông tin đầu sách đã chọn");
    nutXoaSach->setToolTip("Xóa đầu sách và tất cả bản sao");
    nutThemBanSao->setToolTip("Thêm bản sao cho đầu sách đã chọn");
    nutXemChiTiet->setToolTip("Xem chi tiết đầu sách và danh sách bản sao");
    
    // Responsive button styling
    int buttonPadding = ResponsiveHelper::getScaledSpacing(15, screenSize);
    int fontSize = ResponsiveHelper::getScaledFontSize(14, screenSize);
    
    // Gradient + shadow styles cho từng button
    QString kieuThemSach = QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1BC5BD, stop:1 #0e6c66);"
        "    color: white; border: none; border-radius: 8px;"
        "    padding: %1px 25px; font-weight: bold; font-size: %2px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0e9d95, stop:1 #084c47);"
        "}"
        "QPushButton:disabled { background-color: #2B2B40; color: #6C6C80; }"
    ).arg(buttonPadding).arg(fontSize);
    
    QString kieuSuaSach = QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3699FF, stop:1 #1565c0);"
        "    color: white; border: none; border-radius: 8px;"
        "    padding: %1px 25px; font-weight: bold; font-size: %2px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #187DE4, stop:1 #0d47a1);"
        "}"
        "QPushButton:disabled { background-color: #2B2B40; color: #6C6C80; }"
    ).arg(buttonPadding).arg(fontSize);
    
    QString kieuXoaSach = QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #F64E60, stop:1 #c62828);"
        "    color: white; border: none; border-radius: 8px;"
        "    padding: %1px 25px; font-weight: bold; font-size: %2px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #d32f2f, stop:1 #b71c1c);"
        "}"
        "QPushButton:disabled { background-color: #2B2B40; color: #6C6C80; }"
    ).arg(buttonPadding).arg(fontSize);
    
    QString kieuThemBanSao = QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFA800, stop:1 #e65100);"
        "    color: white; border: none; border-radius: 8px;"
        "    padding: %1px 25px; font-weight: bold; font-size: %2px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #d68910, stop:1 #bf360c);"
        "}"
        "QPushButton:disabled { background-color: #2B2B40; color: #6C6C80; }"
    ).arg(buttonPadding).arg(fontSize);
    
    QString kieuChiTiet = QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #8950FC, stop:1 #5e35b1);"
        "    color: white; border: none; border-radius: 8px;"
        "    padding: %1px 25px; font-weight: bold; font-size: %2px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #6c2bd9, stop:1 #4527a0);"
        "}"
        "QPushButton:disabled { background-color: #2B2B40; color: #6C6C80; }"
    ).arg(buttonPadding).arg(fontSize);
    
    nutThemSach->setStyleSheet(kieuThemSach);
    nutSuaSach->setStyleSheet(kieuSuaSach);
    nutXoaSach->setStyleSheet(kieuXoaSach);
    nutThemBanSao->setStyleSheet(kieuThemBanSao);
    nutXemChiTiet->setStyleSheet(kieuChiTiet);
    
    // Shadow effects
    QGraphicsDropShadowEffect *shadowThem = new QGraphicsDropShadowEffect();
    shadowThem->setBlurRadius(15);
    shadowThem->setOffset(0, 3);
    shadowThem->setColor(QColor(0, 0, 0, 80));
    nutThemSach->setGraphicsEffect(shadowThem);
    
    QGraphicsDropShadowEffect *shadowSua = new QGraphicsDropShadowEffect();
    shadowSua->setBlurRadius(15);
    shadowSua->setOffset(0, 3);
    shadowSua->setColor(QColor(0, 0, 0, 80));
    nutSuaSach->setGraphicsEffect(shadowSua);
    
    QGraphicsDropShadowEffect *shadowXoa = new QGraphicsDropShadowEffect();
    shadowXoa->setBlurRadius(15);
    shadowXoa->setOffset(0, 3);
    shadowXoa->setColor(QColor(0, 0, 0, 80));
    nutXoaSach->setGraphicsEffect(shadowXoa);
    
    QGraphicsDropShadowEffect *shadowBanSao = new QGraphicsDropShadowEffect();
    shadowBanSao->setBlurRadius(15);
    shadowBanSao->setOffset(0, 3);
    shadowBanSao->setColor(QColor(0, 0, 0, 80));
    nutThemBanSao->setGraphicsEffect(shadowBanSao);
    
    QGraphicsDropShadowEffect *shadowChiTiet = new QGraphicsDropShadowEffect();
    shadowChiTiet->setBlurRadius(15);
    shadowChiTiet->setOffset(0, 3);
    shadowChiTiet->setColor(QColor(0, 0, 0, 80));
    nutXemChiTiet->setGraphicsEffect(shadowChiTiet);
    
    boTri->addWidget(nutThemSach);
    boTri->addWidget(nutSuaSach);
    boTri->addWidget(nutXoaSach);
    boTri->addWidget(nutThemBanSao);
    boTri->addWidget(nutXemChiTiet);
    boTri->addStretch();
    
    boTriChinh->addWidget(nhomChucNang);
    
    connect(nutThemSach, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutThemSach);
    connect(nutSuaSach, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutSuaSach);
    connect(nutXoaSach, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutXoaSach);
    connect(nutThemBanSao, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutThemBanSao);
    connect(nutXemChiTiet, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutXemChiTiet);
}

void WidgetQuanLySach::taiLaiDanhSach() {
    hienThiDanhSachSach();
}

void WidgetQuanLySach::capNhatBang() {
    hienThiDanhSachSach();
}

void WidgetQuanLySach::hienThiDanhSachSach() {
    // Maximum performance optimization
    bangSach->setUpdatesEnabled(false);
    bangSach->setSortingEnabled(false);
    bangSach->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    // Kiểm tra dữ liệu hợp lệ với logging chi tiết
    if (soLuongDauSach < 0 || soLuongDauSach > MAX_DAUSACH) {
        qWarning() << "Số lượng đầu sách không hợp lệ:" << soLuongDauSach;
        bangSach->setUpdatesEnabled(true);
        bangSach->setSortingEnabled(true);
        return;
    }
    
    qDebug() << "Hiển thị" << soLuongDauSach << "đầu sách";
    
    // Pre-allocate all rows at once
    bangSach->setRowCount(soLuongDauSach);
    
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] != nullptr) {
            int hang = i;
            
            // Tính số sách còn lại và đang mượn - KIỂM TRA AN TOÀN
            int tongBanSao = dsDauSach[i]->tongBanSao;
            int daMuon = 0;
            int thanhLy = 0;
            
            // Duyệt qua danh sách bản sao để đếm - KIỂM TRA CON TRỎ
            PTRDMS p = dsDauSach[i]->dms;
            int dem = 0; // Giới hạn vòng lặp để tránh infinite loop
            while (p != nullptr && dem < 10000) {
                if (p->trangThai == DANG_MUON) {
                    daMuon++;
                } else if (p->trangThai == THANH_LY) {
                    thanhLy++;
                }
                p = p->next;
                dem++;
            }
            
            if (dem >= 10000) {
                qWarning() << "Phát hiện vòng lặp vô hạn tại sách:" << QString::fromStdString(dsDauSach[i]->ISBN);
            }
            
            int conLai = tongBanSao - daMuon - thanhLy;
            
            // STT column
            QTableWidgetItem *itemSTT = new QTableWidgetItem(QString::number(i + 1));
            itemSTT->setTextAlignment(Qt::AlignCenter);
            itemSTT->setForeground(QBrush(QColor("#B5B5C3")));
            bangSach->setItem(hang, 0, itemSTT);
            
            // Basic info columns
            bangSach->setItem(hang, 1, new QTableWidgetItem(QString::fromStdString(dsDauSach[i]->ISBN)));
            bangSach->setItem(hang, 2, new QTableWidgetItem(QString::fromStdString(dsDauSach[i]->tenSach)));
            bangSach->setItem(hang, 3, new QTableWidgetItem(QString::fromStdString(dsDauSach[i]->tacGia)));
            bangSach->setItem(hang, 4, new QTableWidgetItem(QString::fromStdString(dsDauSach[i]->theLoai)));
            
            // Centered numeric columns
            QTableWidgetItem *namXB = new QTableWidgetItem(QString::number(dsDauSach[i]->namXuatBan));
            namXB->setTextAlignment(Qt::AlignCenter);
            bangSach->setItem(hang, 5, namXB);
            
            QTableWidgetItem *soTrang = new QTableWidgetItem(QString::number(dsDauSach[i]->soTrang));
            soTrang->setTextAlignment(Qt::AlignCenter);
            bangSach->setItem(hang, 6, soTrang);
            
            // Status badges column with natural colors
            QWidget *badgeWidget = new QWidget();
            badgeWidget->setStyleSheet("background: transparent;");
            QHBoxLayout *badgeLayout = new QHBoxLayout(badgeWidget);
            badgeLayout->setContentsMargins(8, 2, 8, 2);
            badgeLayout->setSpacing(12);

            // Create badges text with larger size and more spacing
            auto createBadge = [](const QString &icon, int value, const QString &color, const QString &bg, const QString &border) -> QLabel* {
                QLabel *label = new QLabel(QString("<span style='color:%1; background:%2; border:1.5px solid %3; padding:5px 14px; border-radius:12px; font-size:15px; font-weight:700;'>%4 %5</span>")
                    .arg(color, bg, border, icon, QString::number(value)));
                label->setTextFormat(Qt::RichText);
                label->setAlignment(Qt::AlignCenter);
                return label;
            };
            if (conLai > 0) {
                badgeLayout->addWidget(createBadge("●", conLai, "#10B981", "rgba(16,185,129,0.18)", "rgba(16,185,129,0.35)"));
            }
            if (daMuon > 0) {
                badgeLayout->addWidget(createBadge("◐", daMuon, "#F59E0B", "rgba(245,158,11,0.18)", "rgba(245,158,11,0.35)"));
            }
            if (thanhLy > 0) {
                badgeLayout->addWidget(createBadge("✖", thanhLy, "#EF4444", "rgba(239,68,68,0.18)", "rgba(239,68,68,0.35)"));
            }
            badgeLayout->addStretch();
            bangSach->setCellWidget(hang, 7, badgeWidget);
            
            // Action buttons column - Single widget with all buttons
            QWidget *actionWidget = new QWidget();
            actionWidget->setStyleSheet("background: transparent;");
            QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
            actionLayout->setContentsMargins(8, 4, 8, 4);
            actionLayout->setSpacing(6);
            
            QString btnStyle = 
                "QPushButton {"
                "    background-color: #2d3140;"
                "    color: #E0E0E0;"
                "    border: 1px solid #3E4451;"
                "    border-radius: 4px;"
                "}"
                "QPushButton:hover {"
                "    background-color: #0078D4;"
                "    color: white;"
                "    border: 1px solid #0078D4;"
                "}";
            
            QString delStyle = 
                "QPushButton {"
                "    background-color: #2d3140;"
                "    color: #E0E0E0;"
                "    border: 1px solid #3E4451;"
                "    border-radius: 4px;"
                "}"
                "QPushButton:hover {"
                "    background-color: #C42B1C;"
                "    color: white;"
                "    border: 1px solid #C42B1C;"
                "}";
            
            // Edit button
            QPushButton *btnEdit = new QPushButton("✏");
            btnEdit->setFixedSize(33, 33);
            btnEdit->setCursor(Qt::PointingHandCursor);
            btnEdit->setStyleSheet(btnStyle + "QPushButton { font-size: 20px; }");
            btnEdit->setProperty("isbn", QString::fromStdString(dsDauSach[i]->ISBN));
            connect(btnEdit, &QPushButton::clicked, [this, hang]() {
                bangSach->selectRow(hang);
                xuLyNutSuaSach();
            });
            
            // Delete button
            QPushButton *btnDelete = new QPushButton("🗑");
            btnDelete->setFixedSize(33, 33);
            btnDelete->setCursor(Qt::PointingHandCursor);
            btnDelete->setStyleSheet(delStyle + "QPushButton { font-size: 20px; }");
            btnDelete->setProperty("isbn", QString::fromStdString(dsDauSach[i]->ISBN));
            connect(btnDelete, &QPushButton::clicked, [this, hang]() {
                bangSach->selectRow(hang);
                xuLyNutXoaSach();
            });
            
            // Info button
            QPushButton *btnInfo = new QPushButton("ⓘ");
            btnInfo->setFixedSize(33, 33);
            btnInfo->setCursor(Qt::PointingHandCursor);
            btnInfo->setStyleSheet(btnStyle + "QPushButton { font-size: 22px; font-weight: bold; }");
            btnInfo->setProperty("isbn", QString::fromStdString(dsDauSach[i]->ISBN));
            connect(btnInfo, &QPushButton::clicked, [this, hang]() {
                bangSach->selectRow(hang);
                xuLyNutXemChiTiet();
            });
            
            actionLayout->addWidget(btnEdit);
            actionLayout->addWidget(btnDelete);
            actionLayout->addWidget(btnInfo);
            actionLayout->addStretch();
            bangSach->setCellWidget(hang, 8, actionWidget);
        }
    }
    
    // Re-enable updates and sorting
    bangSach->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    bangSach->setSortingEnabled(true);
    bangSach->setUpdatesEnabled(true);
    bangSach->viewport()->update();
    
    emit thongBaoCapNhat(QString("Đã tải %1 đầu sách").arg(soLuongDauSach));
}

void WidgetQuanLySach::xuLyChonDong(int dong, int) {
    if (dong < 0) return;
    
    QString isbn = bangSach->item(dong, 0)->text();
    
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] && QString::fromStdString(dsDauSach[i]->ISBN) == isbn) {
            dauSachDangChon = dsDauSach[i];
            emit thongBaoCapNhat(QString("Đã chọn: %1").arg(QString::fromStdString(dauSachDangChon->tenSach)));
            break;
        }
    }
}

void WidgetQuanLySach::xuLyNutThemSach() {
    HopThoaiThemSach *hopThoai = new HopThoaiThemSach(this);
    
    connect(hopThoai, &HopThoaiThemSach::sachDaThem, this, [this]() {
        taiLaiDanhSach();
        emit thongBaoCapNhat("Đã thêm sách mới thành công!");
    });
    
    hopThoai->exec();
    delete hopThoai;
}

void WidgetQuanLySach::xuLyNutXoaSach() {
    if (!dauSachDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một đầu sách để xóa!");
        return;
    }
    
    // Kiểm tra có thể xóa không
    if (!KiemTraCoTheXoaDauSach(dauSachDangChon)) {
        QMessageBox::warning(this, "Không thể xóa", 
            "Không thể xóa đầu sách này!\n"
            "Vẫn còn bản sao đang được mượn hoặc chưa thanh lý.");
        return;
    }
    
    // Sử dụng ConfirmationDialog đẹp hơn
    ThongTinDauSach info = LayThongTinDauSach(dauSachDangChon->ISBN);
    QString chiTiet = QString(
        "Tên sách: %1\n"
        "Tác giả: %2\n"
        "ISBN: %3\n\n"
        "Tổng bản sao: %4\n"
        "Có thể mượn: %5\n"
        "Đang mượn: %6\n"
        "Đã thanh lý: %7"
    )
    .arg(QString::fromStdString(dauSachDangChon->tenSach))
    .arg(QString::fromStdString(dauSachDangChon->tacGia))
    .arg(QString::fromStdString(dauSachDangChon->ISBN))
    .arg(info.tongBanSao)
    .arg(info.choMuonDuoc)
    .arg(info.dangMuon)
    .arg(info.thanhLy);
    
    bool xacNhan = ConfirmationDialog::confirmDelete(
        this,
        QString::fromStdString(dauSachDangChon->tenSach),
        chiTiet
    );
    
    if (xacNhan) {
        // Xóa đầu sách và kiểm tra kết quả
        std::string ketQua = XoaDauSachTheoISBN(dauSachDangChon->ISBN);
        
        if (ketQua.empty()) {
            // Thành công - lưu lại vào file
            extern PTRDS dsDauSach[];
            extern int soLuongDauSach;
            GhiDanhSachDauSach(FilePathManager::getDauSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
            GhiDanhMucSach(FilePathManager::getDanhMucSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
            
            QMessageBox::information(this, "Thành công", "Đã xóa đầu sách thành công!");
            dauSachDangChon = nullptr;
            taiLaiDanhSach();
            emit thongBaoCapNhat("Đã xóa đầu sách!");
            emit duLieuThayDoi();
        } else {
            // Thất bại - hiển thị lỗi
            QMessageBox::critical(this, "Lỗi", QString::fromStdString(ketQua));
            qWarning() << "Lỗi xóa sách:" << QString::fromStdString(ketQua);
        }
    }
}

void WidgetQuanLySach::xuLyNutSuaSach() {
    if (!dauSachDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một đầu sách để sửa!");
        return;
    }
    
    HopThoaiSuaSach *hopThoai = new HopThoaiSuaSach(QString::fromStdString(dauSachDangChon->ISBN), this);
    connect(hopThoai, &HopThoaiSuaSach::sachDaSua, this, [this]() {
        taiLaiDanhSach();
        emit thongBaoCapNhat("Đã cập nhật thông tin sách!");
    });
    hopThoai->exec();
    delete hopThoai;
}

void WidgetQuanLySach::xuLyNutThemBanSao() {
    if (!dauSachDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một đầu sách để thêm bản sao!");
        return;
    }
    
    HopThoaiThemBanSao *hopThoai = new HopThoaiThemBanSao(QString::fromStdString(dauSachDangChon->ISBN), this);
    
    connect(hopThoai, &HopThoaiThemBanSao::banSaoDaThem, this, [this]() {
        taiLaiDanhSach();
        emit thongBaoCapNhat("Đã thêm bản sao thành công!");
    });
    
    hopThoai->exec();
    delete hopThoai;
}

void WidgetQuanLySach::xuLyNutXoaBanSao() {}

void WidgetQuanLySach::xuLyNutXemChiTiet() {
    if (!dauSachDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một đầu sách để xem chi tiết!");
        return;
    }
    
    ManHinhChiTietSach *dialog = new ManHinhChiTietSach(dauSachDangChon, this);
    
    connect(dialog, &ManHinhChiTietSach::sachDaCapNhat, this, [this]() {
        taiLaiDanhSach();
        emit thongBaoCapNhat("Đã cập nhật thông tin bản sao!");
    });
    
    dialog->exec();
    delete dialog;
}

void WidgetQuanLySach::xuLyTimKiem(const QString &tuKhoa) {
    QString tuKhoaLowerCase = tuKhoa.trimmed().toLower();
    int soKetQua = 0;
    
    for (int i = 0; i < bangSach->rowCount(); i++) {
        bool hienThi = true;
        
        // Filter theo từ khóa
        if (!tuKhoaLowerCase.isEmpty()) {
            QString isbn = bangSach->item(i, 0)->text().toLower();
            QString tenSach = bangSach->item(i, 1)->text().toLower();
            QString tacGia = bangSach->item(i, 2)->text().toLower();
            
            hienThi = isbn.contains(tuKhoaLowerCase) || 
                     tenSach.contains(tuKhoaLowerCase) || 
                     tacGia.contains(tuKhoaLowerCase);
        }
        
        // Filter theo thể loại
        if (hienThi && hopChonTheLoai->currentIndex() > 0) {
            QString theLoaiChon = hopChonTheLoai->currentText();
            QString theLoaiDong = bangSach->item(i, 3)->text();
            if (theLoaiDong != theLoaiChon) {
                hienThi = false;
            }
        }
        
        // Filter theo trạng thái
        if (hienThi && hopChonTrangThai->currentIndex() > 0) {
            QString trangThaiChon = hopChonTrangThai->currentText();
            QString trangThaiDong = bangSach->item(i, 8)->text();
            
            bool khop = false;
            if (trangThaiChon == "Sẵn sàng" && trangThaiDong.contains("[S]")) {
                khop = true;
            } else if (trangThaiChon == "Đang mượn hết" && trangThaiDong.contains("[M]")) {
                khop = true;
            } else if (trangThaiChon == "Đã thanh lý" && trangThaiDong.contains("[T]")) {
                khop = true;
            }
            
            if (!khop) {
                hienThi = false;
            }
        }
        
        bangSach->setRowHidden(i, !hienThi);
        if (hienThi) soKetQua++;
    }
    
    // Hiển thị kết quả
    if (tuKhoaLowerCase.isEmpty() && hopChonTheLoai->currentIndex() == 0 && hopChonTrangThai->currentIndex() == 0) {
        nhanKetQuaTimKiem->setText("");
    } else {
        nhanKetQuaTimKiem->setText(QString("📋 Tìm thấy <b>%1</b> kết quả").arg(soKetQua));
    }
}

void WidgetQuanLySach::xuLyLocTheLoai(int) {
    xuLyTimKiem(oNhapTimKiem->text());
}

void WidgetQuanLySach::xuLyLocTrangThai(int) {
    xuLyTimKiem(oNhapTimKiem->text());
}

void WidgetQuanLySach::taiDanhSachTheLoai() {
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    QSet<QString> theLoaiSet;
    
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] != nullptr) {
            QString theLoai = QString::fromStdString(dsDauSach[i]->theLoai).trimmed();
            if (!theLoai.isEmpty()) {
                theLoaiSet.insert(theLoai);
            }
        }
    }
    
    // Chuyển sang list và sắp xếp
    QStringList theLoaiList = theLoaiSet.values();
    std::sort(theLoaiList.begin(), theLoaiList.end());
    
    // Thêm vào combobox
    for (const QString &theLoai : theLoaiList) {
        hopChonTheLoai->addItem(theLoai);
    }
}
void WidgetQuanLySach::xuLyNhapDuLieuTu() {}
void WidgetQuanLySach::hienThiBanSao(PTRDS) {}

QString WidgetQuanLySach::layTrangThaiSach(TrangThaiSach trangThai) const {
    switch(trangThai) {
        case CHO_MUON_DUOC: return "Sẵn sàng";
        case DANG_MUON: return "Đang mượn";
        case THANH_LY: return "Thanh lý";
        default: return "Không rõ";
    }
}

QColor WidgetQuanLySach::layMauTrangThai(TrangThaiSach trangThai) const {
    switch(trangThai) {
        case CHO_MUON_DUOC: return QColor("#1BC5BD");
        case DANG_MUON: return QColor("#FFA800");
        case THANH_LY: return QColor("#F64E60");
        default: return QColor("#6C6C80");
    }
}
