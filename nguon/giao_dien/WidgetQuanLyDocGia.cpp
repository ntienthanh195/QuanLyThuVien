#include "giao_dien/WidgetQuanLyDocGia.h"
#include "giao_dien/hop_thoai/HopThoaiThemDocGia.h"
#include "giao_dien/hop_thoai/HopThoaiSuaDocGia.h"
#include "models/DocGia.h"
#include "utils/ConfirmationDialog.h"
#include "utils/AnimationHelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>

// Include responsive helpers
#include "utils/ResponsiveHelper.h"
#include "utils/AnimationHelper.h"

WidgetQuanLyDocGia::WidgetQuanLyDocGia(QWidget *cha) 
    : QWidget(cha), docGiaDangChon(nullptr), maTheDangChon(-1) {
    thieLapGiaoDien();
    taiLaiDanhSach();
}

WidgetQuanLyDocGia::~WidgetQuanLyDocGia() {}

void WidgetQuanLyDocGia::thieLapGiaoDien() {
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    
    boTriChinh = new QVBoxLayout(this);
    int margin = ResponsiveHelper::getScaledMargin(30, screenSize);
    int spacing = ResponsiveHelper::getScaledSpacing(20, screenSize);
    boTriChinh->setContentsMargins(margin, margin, margin, margin);
    boTriChinh->setSpacing(spacing);
    
    // Tiêu đề với icon
    QHBoxLayout *layoutTieuDe = new QHBoxLayout();
    QPushButton *iconTieuDe = new QPushButton();
    iconTieuDe->setIcon(QIcon(":/icons/user.svg"));
    iconTieuDe->setIconSize(QSize(28, 28));
    iconTieuDe->setFlat(true);
    iconTieuDe->setStyleSheet("border: none; background: transparent;");
    int titleSize = ResponsiveHelper::getScaledFontSize(28, screenSize);
    QLabel *tieuDe = new QLabel("QUẢN LÝ ĐỘC GIẢ");
    tieuDe->setStyleSheet(QString("font-size: %1px; font-weight: bold; color: #FFFFFF;").arg(titleSize));
    layoutTieuDe->addWidget(iconTieuDe);
    layoutTieuDe->addWidget(tieuDe);
    layoutTieuDe->addStretch();
    boTriChinh->addLayout(layoutTieuDe);
    
    thieLapTimKiem();
    thieLapBangDocGia();
    thieLapNutChucNang();
    
    qDebug() << "WidgetQuanLyDocGia: Responsive layout applied";
}

void WidgetQuanLyDocGia::thieLapTimKiem() {
    nhomTimKiem = new QGroupBox("  Tìm Kiếm & Lọc");
    nhomTimKiem->setStyleSheet(
        "QGroupBox { background-color: #1E1E2D; border: 1px solid #2B2B40; "
        "border-radius: 12px; padding: 15px; font-size: 14px; font-weight: bold; color: #FFFFFF; }"
        "QLineEdit { background-color: #2B2B40; border: 1px solid #3F3F55; border-radius: 8px; "
        "padding: 8px 12px; color: #FFFFFF; font-size: 13px; }"
        "QLineEdit:focus { border: 2px solid #3699FF; background-color: #1E1E2D; }"
        "QComboBox { background-color: #2B2B40; border: 1px solid #3F3F55; border-radius: 8px; "
        "padding: 8px 12px; color: #FFFFFF; font-size: 13px; }"
        "QComboBox:focus { border: 2px solid #3699FF; }"
        "QComboBox::drop-down { border: none; width: 30px; }"
        "QComboBox::down-arrow { image: url(:/icons/filter.svg); width: 12px; height: 12px; }"
    );
    
    QHBoxLayout *boTri = new QHBoxLayout(nhomTimKiem);
    
    oNhapTimKiem = new QLineEdit();
    oNhapTimKiem->setPlaceholderText("Nhập mã thẻ, họ, tên...");
    oNhapTimKiem->setMinimumHeight(40);
    
    hopChonTrangThai = new QComboBox();
    hopChonTrangThai->addItems({"Tất cả", "Hoạt động", "Bị khóa"});
    hopChonTrangThai->setMinimumHeight(40);
    
    hopChonGioiTinh = new QComboBox();
    hopChonGioiTinh->addItems({"Tất cả giới tính", "Nam", "Nữ"});
    hopChonGioiTinh->setMinimumHeight(40);
    
    nutTimKiem = new QPushButton();
    nutTimKiem->setIcon(QIcon(":/icons/search.svg"));
    nutTimKiem->setText(" Tìm");
    nutTimKiem->setIconSize(QSize(18, 18));
    nutTimKiem->setMinimumHeight(40);
    nutTimKiem->setMinimumWidth(100);
    nutTimKiem->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #3699FF, stop:1 #187DE4); color: white; border: none; "
        "border-radius: 8px; font-weight: bold; padding: 8px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #187DE4, stop:1 #0C5EBF); }"
        "QPushButton:pressed { background: #0C5EBF; }"
    );
    
    nutLamMoi = new QPushButton();
    nutLamMoi->setIcon(QIcon(":/icons/refresh.svg"));
    nutLamMoi->setText(" Làm mới");
    nutLamMoi->setIconSize(QSize(18, 18));
    nutLamMoi->setMinimumHeight(40);
    nutLamMoi->setMinimumWidth(100);
    nutLamMoi->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #1BC5BD, stop:1 #0D9488); color: white; border: none; "
        "border-radius: 8px; font-weight: bold; padding: 8px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #0D9488, stop:1 #0A7368); }"
        "QPushButton:pressed { background: #0A7368; }"
    );
    
    boTri->addWidget(oNhapTimKiem, 3);
    boTri->addWidget(hopChonTrangThai, 1);
    boTri->addWidget(hopChonGioiTinh, 1);
    boTri->addWidget(nutTimKiem);
    
    boTriChinh->addWidget(nhomTimKiem);
    
    // Connect signals - tìm kiếm real-time
    connect(oNhapTimKiem, &QLineEdit::textChanged, this, &WidgetQuanLyDocGia::xuLyTimKiem);
    connect(hopChonTrangThai, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &WidgetQuanLyDocGia::xuLyLocTrangThai);
    connect(hopChonGioiTinh, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &WidgetQuanLyDocGia::xuLyLocGioiTinh);
    connect(nutTimKiem, &QPushButton::clicked, this, [this]() { xuLyTimKiem(oNhapTimKiem->text()); });
}

void WidgetQuanLyDocGia::thieLapBangDocGia() {
    bangDocGia = new QTableWidget();
    bangDocGia->setColumnCount(9);
    bangDocGia->setHorizontalHeaderLabels({
        "STT", "Mã Thẻ", "Họ", "Tên", "Giới Tính", 
        "Trạng Thái", "Sách Đang Mượn", "Quá Hạn", "Tổng Mượn"
    });
    
    // Styling chuyên nghiệp
    bangDocGia->setStyleSheet(
        "QTableWidget {"
        "    background-color: #1E1E2D;"
        "    border: 1px solid #2B2B40;"
        "    border-radius: 12px;"
        "    gridline-color: #2B2B40;"
        "    color: #FFFFFF;"
        "    selection-background-color: #3699FF;"
        "}"
        "QTableWidget::item {"
        "    padding: 8px;"
        "    border-bottom: 1px solid #2B2B40;"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: #3699FF;"
        "    color: #FFFFFF;"
        "}"
        "QHeaderView::section {"
        "    background-color: #1E1E2D;"
        "    color: #FFFFFF;"
        "    padding: 12px;"
        "    border: none;"
        "    border-bottom: 2px solid #3699FF;"
        "    font-weight: bold;"
        "    font-size: 13px;"
        "}"
    );
    
    bangDocGia->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangDocGia->setSelectionMode(QAbstractItemView::SingleSelection);
    bangDocGia->setAlternatingRowColors(true);
    bangDocGia->verticalHeader()->setVisible(false);
    bangDocGia->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bangDocGia->setMinimumHeight(450);
    
    // Responsive column widths
    bangDocGia->setColumnWidth(0, 50);   // STT
    bangDocGia->setColumnWidth(1, 80);   // Mã Thẻ
    bangDocGia->setColumnWidth(2, 120);  // Họ
    bangDocGia->setColumnWidth(3, 100);  // Tên
    bangDocGia->setColumnWidth(4, 90);   // Giới Tính
    bangDocGia->setColumnWidth(5, 120);  // Trạng Thái
    bangDocGia->setColumnWidth(6, 130);  // Sách Đang Mượn
    bangDocGia->setColumnWidth(7, 100);  // Quá Hạn
    bangDocGia->horizontalHeader()->setStretchLastSection(true);
    
    boTriChinh->addWidget(bangDocGia, 1);
    
    connect(bangDocGia, &QTableWidget::cellClicked, this, &WidgetQuanLyDocGia::xuLyChonDong);
    
    // Double-click to show detail
    connect(bangDocGia, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        if (row >= 0 && row < bangDocGia->rowCount()) {
            int maThe = bangDocGia->item(row, 1)->data(Qt::UserRole).toInt();
            extern PTRDG rootDocGia;
            PTRDG docGia = timDocGia(rootDocGia, maThe);
            if (docGia) {
                HopThoaiSuaDocGia *dialog = new HopThoaiSuaDocGia(maThe, this);
                dialog->exec();
                delete dialog;
                taiLaiDanhSach();
            }
        }
    });
}

void WidgetQuanLyDocGia::thieLapNutChucNang() {
    nhomChucNang = new QGroupBox("  Chức Năng");
    nhomChucNang->setStyleSheet(
        "QGroupBox { background-color: #1E1E2D; border: 1px solid #2B2B40; "
        "border-radius: 12px; padding: 15px; font-size: 14px; font-weight: bold; color: #FFFFFF; }"
    );
    
    QHBoxLayout *boTri = new QHBoxLayout(nhomChucNang);
    
    nutThemDocGia = new QPushButton();
    nutThemDocGia->setText("+ Thêm Độc Giả");
    nutThemDocGia->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #1BC5BD, stop:1 #0D9488); color: white; border: none; "
        "border-radius: 8px; font-weight: bold; padding: 10px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #0D9488, stop:1 #0A7368); }"
        "QPushButton:pressed { background: #0A7368; }"
    );
    
    nutSuaDocGia = new QPushButton();
    nutSuaDocGia->setText("Sửa");
    nutSuaDocGia->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #FFA800, stop:1 #CC8600); color: white; border: none; "
        "border-radius: 8px; font-weight: bold; padding: 10px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #CC8600, stop:1 #996600); }"
        "QPushButton:pressed { background: #996600; }"
        "QPushButton:disabled { background: #2B2B40; color: #6C6C80; }"
    );
    
    nutXoaDocGia = new QPushButton();
    nutXoaDocGia->setText("Xóa");
    nutXoaDocGia->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #F64E60, stop:1 #C9384A); color: white; border: none; "
        "border-radius: 8px; font-weight: bold; padding: 10px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #C9384A, stop:1 #A32A3A); }"
        "QPushButton:pressed { background: #A32A3A; }"
        "QPushButton:disabled { background: #2B2B40; color: #6C6C80; }"
    );
    
    nutKhoaThe = new QPushButton();
    nutKhoaThe->setText("Khóa Thẻ");
    nutKhoaThe->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #8950FC, stop:1 #6B3FCC); color: white; border: none; "
        "border-radius: 8px; font-weight: bold; padding: 10px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #6B3FCC, stop:1 #5430A3); }"
        "QPushButton:pressed { background: #5430A3; }"
        "QPushButton:disabled { background: #2B2B40; color: #6C6C80; }"
    );
    
    nutMoKhoaThe = new QPushButton();
    nutMoKhoaThe->setText("Mở Khóa");
    nutMoKhoaThe->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #3699FF, stop:1 #187DE4); color: white; border: none; "
        "border-radius: 8px; font-weight: bold; padding: 10px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #187DE4, stop:1 #0C5EBF); }"
        "QPushButton:pressed { background: #0C5EBF; }"
        "QPushButton:disabled { background: #2B2B40; color: #6C6C80; }"
    );
    
    nutXemLichSu = new QPushButton();
    nutXemLichSu->setText("Lịch Sử");
    nutXemLichSu->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #3699FF, stop:1 #187DE4); color: white; border: none; "
        "border-radius: 8px; font-weight: bold; padding: 10px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #187DE4, stop:1 #0C5EBF); }"
        "QPushButton:pressed { background: #0C5EBF; }"
        "QPushButton:disabled { background: #2B2B40; color: #6C6C80; }"
    );
    
    boTri->addWidget(nutThemDocGia);
    boTri->addWidget(nutSuaDocGia);
    boTri->addWidget(nutXoaDocGia);
    boTri->addWidget(nutKhoaThe);
    boTri->addWidget(nutMoKhoaThe);
    boTri->addWidget(nutXemLichSu);
    boTri->addStretch();
    
    boTriChinh->addWidget(nhomChucNang);
    
    connect(nutThemDocGia, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNutThemDocGia);
    connect(nutSuaDocGia, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNutSuaDocGia);
    connect(nutXoaDocGia, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNutXoaDocGia);
    connect(nutKhoaThe, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNutKhoaThe);
    connect(nutMoKhoaThe, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNutMoKhoaThe);
    connect(nutXemLichSu, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNutXemLichSu);
}

void WidgetQuanLyDocGia::taiLaiDanhSach() {
    hienThiDanhSachDocGia();
}

void WidgetQuanLyDocGia::capNhatBang() {
    hienThiDanhSachDocGia();
}

void WidgetQuanLyDocGia::hienThiDanhSachDocGia() {
    bangDocGia->setRowCount(0);
    
    extern PTRDG rootDocGia;
    if (!rootDocGia) {
        emit thongBaoCapNhat("Chưa có độc giả nào");
        return;
    }
    
    // Chuyển cây AVL thành mảng
    PTRDG danhSach[1000];
    int dem = 0;
    DuyetCayRaMang(rootDocGia, danhSach, dem);
    
    // Hiển thị với 9 cột đầy đủ thông tin
    for (int i = 0; i < dem; i++) {
        if (danhSach[i]) {
            int hang = bangDocGia->rowCount();
            bangDocGia->insertRow(hang);
            
            TheDocGia &dg = danhSach[i]->data;
            
            // Cột 0: STT
            QTableWidgetItem *itemSTT = new QTableWidgetItem(QString::number(i + 1));
            itemSTT->setTextAlignment(Qt::AlignCenter);
            itemSTT->setForeground(QBrush(QColor("#B5B5C3")));
            bangDocGia->setItem(hang, 0, itemSTT);
            
            // Cột 1: Mã Thẻ
            QTableWidgetItem *itemMaThe = new QTableWidgetItem(QString::number(dg.MaThe));
            itemMaThe->setTextAlignment(Qt::AlignCenter);
            itemMaThe->setForeground(QBrush(QColor("#3699FF")));
            itemMaThe->setFont(QFont("Segoe UI", 11, QFont::Bold));
            bangDocGia->setItem(hang, 1, itemMaThe);
            
            // Cột 2: Họ
            QTableWidgetItem *itemHo = new QTableWidgetItem(QString::fromStdString(dg.Ho));
            itemHo->setForeground(QBrush(QColor("#FFFFFF")));
            bangDocGia->setItem(hang, 2, itemHo);
            
            // Cột 3: Tên
            QTableWidgetItem *itemTen = new QTableWidgetItem(QString::fromStdString(dg.Ten));
            itemTen->setForeground(QBrush(QColor("#FFFFFF")));
            itemTen->setFont(QFont("Segoe UI", 10, QFont::Bold));
            bangDocGia->setItem(hang, 3, itemTen);
            
            // Cột 4: Giới Tính
            QString gioiTinh = dg.Phai ? "Nữ" : "Nam";
            QTableWidgetItem *itemGioiTinh = new QTableWidgetItem(gioiTinh);
            itemGioiTinh->setTextAlignment(Qt::AlignCenter);
            itemGioiTinh->setForeground(QBrush(dg.Phai ? QColor("#FF69B4") : QColor("#3699FF")));
            bangDocGia->setItem(hang, 4, itemGioiTinh);
            
            // Cột 5: Trạng Thái với color-coding
            QString trangThai = (dg.TrangThai == 1) ? "Hoạt động" : "Bị khóa";
            QTableWidgetItem *itemTrangThai = new QTableWidgetItem(trangThai);
            itemTrangThai->setTextAlignment(Qt::AlignCenter);
            if (dg.TrangThai == 1) {
                itemTrangThai->setBackground(QBrush(QColor(27, 197, 189, 40)));
                itemTrangThai->setForeground(QBrush(QColor("#1BC5BD")));
            } else {
                itemTrangThai->setBackground(QBrush(QColor(246, 78, 96, 40)));
                itemTrangThai->setForeground(QBrush(QColor("#F64E60")));
            }
            itemTrangThai->setFont(QFont("Segoe UI", 10, QFont::Bold));
            bangDocGia->setItem(hang, 5, itemTrangThai);
            
            // Cột 6: Sách Đang Mượn với badge
            QString sachDangMuon = QString("%1 / 3").arg(dg.soSachDangMuon);
            QTableWidgetItem *itemSachMuon = new QTableWidgetItem(sachDangMuon);
            itemSachMuon->setTextAlignment(Qt::AlignCenter);
            itemSachMuon->setFont(QFont("Segoe UI", 10, QFont::Bold));
            
            if (dg.soSachDangMuon == 0) {
                itemSachMuon->setForeground(QBrush(QColor("#B5B5C3")));
            } else if (dg.soSachDangMuon >= 3) {
                itemSachMuon->setBackground(QBrush(QColor(246, 78, 96, 50)));
                itemSachMuon->setForeground(QBrush(QColor("#F64E60")));
            } else {
                itemSachMuon->setBackground(QBrush(QColor(255, 168, 0, 40)));
                itemSachMuon->setForeground(QBrush(QColor("#FFA800")));
            }
            bangDocGia->setItem(hang, 6, itemSachMuon);
            
            // Cột 7: Quá Hạn (tính toán real-time)
            int soNgayQuaHan = TinhSoNgayQuaHanLonNhat(danhSach[i]);
            QTableWidgetItem *itemQuaHan = new QTableWidgetItem();
            itemQuaHan->setTextAlignment(Qt::AlignCenter);
            
            if (soNgayQuaHan > 0) {
                itemQuaHan->setText(QString("%1 ngày").arg(soNgayQuaHan));
                itemQuaHan->setBackground(QBrush(QColor(246, 78, 96, 50)));
                itemQuaHan->setForeground(QBrush(QColor("#F64E60")));
                itemQuaHan->setFont(QFont("Segoe UI", 10, QFont::Bold));
            } else {
                itemQuaHan->setText("—");
                itemQuaHan->setForeground(QBrush(QColor("#B5B5C3")));
            }
            bangDocGia->setItem(hang, 7, itemQuaHan);
            
            // Cột 8: Tổng Mượn (đếm từ dsmt)
            int tongMuon = 0;
            MUONTRA mt = dg.dsmt;
            while (mt) {
                tongMuon++;
                mt = mt->next;
            }
            QTableWidgetItem *itemTongMuon = new QTableWidgetItem(QString::number(tongMuon));
            itemTongMuon->setTextAlignment(Qt::AlignCenter);
            itemTongMuon->setForeground(QBrush(QColor("#3699FF")));
            bangDocGia->setItem(hang, 8, itemTongMuon);
        }
    }
    
    emit thongBaoCapNhat(QString("Đã tải %1 độc giả").arg(dem));
}

void WidgetQuanLyDocGia::xuLyChonDong(int dong, int) {
    if (dong < 0) return;
    
    int maThe = bangDocGia->item(dong, 0)->text().toInt();
    
    extern PTRDG rootDocGia;
    docGiaDangChon = timDocGia(rootDocGia, maThe);
    maTheDangChon = maThe;
    
    if (docGiaDangChon) {
        emit thongBaoCapNhat(QString("Đã chọn: %1 %2 (Mã: %3)")
            .arg(QString::fromStdString(docGiaDangChon->data.Ho))
            .arg(QString::fromStdString(docGiaDangChon->data.Ten))
            .arg(maThe));
    }
}

void WidgetQuanLyDocGia::xuLyNutThemDocGia() {
    HopThoaiThemDocGia *hopThoai = new HopThoaiThemDocGia(this);
    
    connect(hopThoai, &HopThoaiThemDocGia::docGiaDaThem, this, [this]() {
        extern PTRDG rootDocGia;
        saveDocGia(rootDocGia); // Tự động lưu file
        taiLaiDanhSach();
        emit thongBaoCapNhat("Đã thêm độc giả mới thành công!");
        emit duLieuThayDoi(); // Realtime update Dashboard
    });
    
    hopThoai->exec();
    delete hopThoai;
}

void WidgetQuanLyDocGia::xuLyNutSuaDocGia() {
    if (!docGiaDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một độc giả!");
        return;
    }
    
    HopThoaiSuaDocGia *hopThoai = new HopThoaiSuaDocGia(docGiaDangChon->data.MaThe, this);
    
    connect(hopThoai, &HopThoaiSuaDocGia::docGiaDaSua, this, [this]() {
        extern PTRDG rootDocGia;
        saveDocGia(rootDocGia); // Tự động lưu file
        taiLaiDanhSach();
        emit thongBaoCapNhat("Đã cập nhật thông tin độc giả!");
        emit duLieuThayDoi(); // Realtime update Dashboard
    });
    
    hopThoai->exec();
    delete hopThoai;
}

void WidgetQuanLyDocGia::xuLyNutXoaDocGia() {
    if (!docGiaDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một độc giả!");
        return;
    }
    
    // Kiểm tra điều kiện xóa
    if (docGiaDangChon->data.soSachDangMuon > 0) {
        QMessageBox::warning(this, "Không Thể Xóa",
            QString("Không thể xóa độc giả này!\n\nĐộc giả đang mượn %1 cuốn sách.\nVui lòng trả hết sách trước khi xóa.")
                .arg(docGiaDangChon->data.soSachDangMuon));
        return;
    }
    
    // Sử dụng ConfirmationDialog đẹp hơn
    QString chiTiet = QString(
        "Họ tên: %1 %2\n"
        "Mã thẻ: %3\n"
        "Trạng thái: %4\n"
        "Giới tính: %5\n\n"
        "Số sách đang mượn: %6"
    )
    .arg(QString::fromStdString(docGiaDangChon->data.Ho))
    .arg(QString::fromStdString(docGiaDangChon->data.Ten))
    .arg(docGiaDangChon->data.MaThe)
    .arg(docGiaDangChon->data.TrangThai == 1 ? "Đang hoạt động" : "Bị khóa")
    .arg(docGiaDangChon->data.Phai ? "Nữ" : "Nam")
    .arg(docGiaDangChon->data.soSachDangMuon);
    
    bool xacNhan = ConfirmationDialog::confirmDelete(
        this,
        QString("%1 %2 (Mã: %3)")
            .arg(QString::fromStdString(docGiaDangChon->data.Ho))
            .arg(QString::fromStdString(docGiaDangChon->data.Ten))
            .arg(docGiaDangChon->data.MaThe),
        chiTiet
    );
    
    if (xacNhan) {
        extern PTRDG rootDocGia;
        
        // Xóa khỏi cây AVL
        int maThe = docGiaDangChon->data.MaThe;
        xoaDocGia(rootDocGia, maThe);
        
        saveDocGia(rootDocGia); // Tự động lưu file
        emit duLieuThayDoi(); // Realtime update Dashboard
        
        QMessageBox::information(this, "Thành Công", "Đã xóa độc giả thành công!");
        docGiaDangChon = nullptr;
        maTheDangChon = -1;
        taiLaiDanhSach();
        emit thongBaoCapNhat("Đã xóa độc giả!");
    }
}

void WidgetQuanLyDocGia::xuLyNutKhoaThe() {
    if (!docGiaDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một độc giả!");
        return;
    }
    
    if (docGiaDangChon->data.TrangThai == 0) {
        QMessageBox::information(this, "Thông Báo", "Thẻ độc giả này đã bị khóa rồi!");
        return;
    }
    
    QMessageBox::StandardButton xacNhan = QMessageBox::question(this, "Xác Nhận Khóa Thẻ",
        QString("Bạn có chắc muốn khóa thẻ độc giả:\n%1 %2 (Mã: %3)?\n\nĐộc giả sẽ không thể mượn sách sau khi bị khóa.")
            .arg(QString::fromStdString(docGiaDangChon->data.Ho))
            .arg(QString::fromStdString(docGiaDangChon->data.Ten))
            .arg(docGiaDangChon->data.MaThe),
        QMessageBox::Yes | QMessageBox::No);
    
    if (xacNhan == QMessageBox::Yes) {
        extern PTRDG rootDocGia;
        
        // Tạo struct mới với trạng thái khóa
        TheDocGia dataCapNhat = docGiaDangChon->data;
        dataCapNhat.TrangThai = 0;
        
        hieuChinhDocGia(rootDocGia, dataCapNhat);
        
        saveDocGia(rootDocGia); // Tự động lưu file
        
        QMessageBox::information(this, "Thành Công", "Đã khóa thẻ độc giả!");
        taiLaiDanhSach();
        emit thongBaoCapNhat("Đã khóa thẻ độc giả!");
    }
}

void WidgetQuanLyDocGia::xuLyNutMoKhoaThe() {
    if (!docGiaDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một độc giả!");
        return;
    }
    
    if (docGiaDangChon->data.TrangThai == 1) {
        QMessageBox::information(this, "Thông Báo", "Thẻ độc giả này đang hoạt động bình thường!");
        return;
    }
    
    QMessageBox::StandardButton xacNhan = QMessageBox::question(this, "Xác Nhận Mở Khóa",
        QString("Bạn có chắc muốn mở khóa thẻ độc giả:\n%1 %2 (Mã: %3)?")
            .arg(QString::fromStdString(docGiaDangChon->data.Ho))
            .arg(QString::fromStdString(docGiaDangChon->data.Ten))
            .arg(docGiaDangChon->data.MaThe),
        QMessageBox::Yes | QMessageBox::No);
    
    if (xacNhan == QMessageBox::Yes) {
        extern PTRDG rootDocGia;
        
        // Tạo struct mới với trạng thái hoạt động
        TheDocGia dataCapNhat = docGiaDangChon->data;
        dataCapNhat.TrangThai = 1;
        
        hieuChinhDocGia(rootDocGia, dataCapNhat);
        
        saveDocGia(rootDocGia); // Tự động lưu file
        
        QMessageBox::information(this, "Thành Công", "Đã mở khóa thẻ độc giả!");
        taiLaiDanhSach();
        emit thongBaoCapNhat("Đã mở khóa thẻ độc giả!");
    }
}

void WidgetQuanLyDocGia::xuLyNutXemLichSu() {
    if (!docGiaDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một độc giả!");
        return;
    }
    QMessageBox::information(this, "Lịch Sử", "Chức năng xem lịch sử đang được phát triển...");
}

void WidgetQuanLyDocGia::xuLyNutXemSachDangMuon() {
    if (!docGiaDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một độc giả!");
        return;
    }
    QMessageBox::information(this, "Sách Đang Mượn", 
        QString("Độc giả %1 %2 đang mượn %3 cuốn sách.\n\nChức năng xem chi tiết đang được phát triển...")
            .arg(QString::fromStdString(docGiaDangChon->data.Ho))
            .arg(QString::fromStdString(docGiaDangChon->data.Ten))
            .arg(docGiaDangChon->data.soSachDangMuon));
}

void WidgetQuanLyDocGia::xuLyTimKiem(const QString &tuKhoa) {
    bangDocGia->setRowCount(0);
    
    extern PTRDG rootDocGia;
    if (!rootDocGia) {
        emit thongBaoCapNhat("Chưa có độc giả nào");
        return;
    }
    
    QString tuKhoaTrimmed = tuKhoa.trimmed();
    
    // Nếu không có từ khóa, hiển thị tất cả (có áp dụng bộ lọc nếu có)
    if (tuKhoaTrimmed.isEmpty()) {
        xuLyLocKetHopToanBo();
        return;
    }
    
    PTRDG ketQua[1000];
    int soKetQua = 0;
    
    // Kiểm tra xem có phải mã thẻ không (số nguyên)
    bool laMaThe = false;
    int maThe = tuKhoaTrimmed.toInt(&laMaThe);
    
    if (laMaThe) {
        // Tìm theo mã thẻ
        PTRDG docGia = timDocGia(rootDocGia, maThe);
        if (docGia) {
            ketQua[0] = docGia;
            soKetQua = 1;
        }
    } else {
        // Tìm theo Họ + Tên (backend hỗ trợ tìm kiếm không phân biệt hoa/thường)
        soKetQua = TimDocGiaTheoHoTen(rootDocGia, tuKhoaTrimmed.toStdString(), ketQua, 1000);
    }
    
    // Hiển thị kết quả với bộ lọc
    hienThiKetQuaTimKiem(ketQua, soKetQua);
    
    emit thongBaoCapNhat(QString("Tìm thấy %1 kết quả cho '%2'").arg(soKetQua).arg(tuKhoaTrimmed));
}

void WidgetQuanLyDocGia::xuLyLocTrangThai(int chiSo) {
    Q_UNUSED(chiSo);
    xuLyLocKetHopToanBo();
}

void WidgetQuanLyDocGia::xuLyLocGioiTinh(int chiSo) {
    Q_UNUSED(chiSo);
    xuLyLocKetHopToanBo();
}

void WidgetQuanLyDocGia::xuLyLocKetHopToanBo() {
    bangDocGia->setRowCount(0);
    
    extern PTRDG rootDocGia;
    if (!rootDocGia) {
        emit thongBaoCapNhat("Chưa có độc giả nào");
        return;
    }
    
    // Lấy toàn bộ độc giả
    PTRDG danhSach[1000];
    int dem = 0;
    DuyetCayRaMang(rootDocGia, danhSach, dem);
    
    // Áp dụng bộ lọc
    int trangThaiLoc = hopChonTrangThai->currentIndex(); // 0: Tất cả, 1: Hoạt động, 2: Bị khóa
    int gioiTinhLoc = hopChonGioiTinh->currentIndex();   // 0: Tất cả, 1: Nam, 2: Nữ
    
    int soHienThi = 0;
    for (int i = 0; i < dem; i++) {
        if (danhSach[i]) {
            bool hienThi = true;
            
            // Lọc trạng thái
            if (trangThaiLoc == 1 && danhSach[i]->data.TrangThai != 1) hienThi = false;
            if (trangThaiLoc == 2 && danhSach[i]->data.TrangThai != 0) hienThi = false;
            
            // Lọc giới tính
            if (gioiTinhLoc == 1 && danhSach[i]->data.Phai != false) hienThi = false; // Nam
            if (gioiTinhLoc == 2 && danhSach[i]->data.Phai != true) hienThi = false;  // Nữ
            
            if (hienThi) {
                themDongVaoBang(danhSach[i], soHienThi);
                soHienThi++;
            }
        }
    }
    
    emit thongBaoCapNhat(QString("Hiển thị %1/%2 độc giả").arg(soHienThi).arg(dem));
}

void WidgetQuanLyDocGia::hienThiKetQuaTimKiem(PTRDG ketQua[], int soKetQua) {
    bangDocGia->setRowCount(0);
    
    // Áp dụng bộ lọc trên kết quả tìm kiếm
    int trangThaiLoc = hopChonTrangThai->currentIndex();
    int gioiTinhLoc = hopChonGioiTinh->currentIndex();
    
    int soHienThi = 0;
    for (int i = 0; i < soKetQua; i++) {
        if (ketQua[i]) {
            bool hienThi = true;
            
            // Lọc trạng thái
            if (trangThaiLoc == 1 && ketQua[i]->data.TrangThai != 1) hienThi = false;
            if (trangThaiLoc == 2 && ketQua[i]->data.TrangThai != 0) hienThi = false;
            
            // Lọc giới tính
            if (gioiTinhLoc == 1 && ketQua[i]->data.Phai != false) hienThi = false;
            if (gioiTinhLoc == 2 && ketQua[i]->data.Phai != true) hienThi = false;
            
            if (hienThi) {
                themDongVaoBang(ketQua[i], soHienThi);
                soHienThi++;
            }
        }
    }
}

void WidgetQuanLyDocGia::themDongVaoBang(PTRDG docGia, int stt) {
    int hang = bangDocGia->rowCount();
    bangDocGia->insertRow(hang);
    
    TheDocGia &dg = docGia->data;
    
    // STT
    QTableWidgetItem *itemSTT = new QTableWidgetItem(QString::number(stt + 1));
    itemSTT->setTextAlignment(Qt::AlignCenter);
    itemSTT->setForeground(QBrush(QColor("#B5B5C3")));
    bangDocGia->setItem(hang, 0, itemSTT);
    
    // Mã Thẻ
    QTableWidgetItem *itemMaThe = new QTableWidgetItem(QString::number(dg.MaThe));
    itemMaThe->setTextAlignment(Qt::AlignCenter);
    itemMaThe->setForeground(QBrush(QColor("#3699FF")));
    itemMaThe->setFont(QFont("Segoe UI", 11, QFont::Bold));
    bangDocGia->setItem(hang, 1, itemMaThe);
    
    // Họ
    QTableWidgetItem *itemHo = new QTableWidgetItem(QString::fromStdString(dg.Ho));
    itemHo->setForeground(QBrush(QColor("#FFFFFF")));
    bangDocGia->setItem(hang, 2, itemHo);
    
    // Tên
    QTableWidgetItem *itemTen = new QTableWidgetItem(QString::fromStdString(dg.Ten));
    itemTen->setForeground(QBrush(QColor("#FFFFFF")));
    itemTen->setFont(QFont("Segoe UI", 10, QFont::Bold));
    bangDocGia->setItem(hang, 3, itemTen);
    
    // Giới Tính
    QString gioiTinh = dg.Phai ? "Nữ" : "Nam";
    QTableWidgetItem *itemGioiTinh = new QTableWidgetItem(gioiTinh);
    itemGioiTinh->setTextAlignment(Qt::AlignCenter);
    itemGioiTinh->setForeground(QBrush(dg.Phai ? QColor("#FF69B4") : QColor("#3699FF")));
    bangDocGia->setItem(hang, 4, itemGioiTinh);
    
    // Trạng Thái
    QString trangThai = (dg.TrangThai == 1) ? "Hoạt động" : "Bị khóa";
    QTableWidgetItem *itemTrangThai = new QTableWidgetItem(trangThai);
    itemTrangThai->setTextAlignment(Qt::AlignCenter);
    if (dg.TrangThai == 1) {
        itemTrangThai->setBackground(QBrush(QColor(27, 197, 189, 40)));
        itemTrangThai->setForeground(QBrush(QColor("#1BC5BD")));
    } else {
        itemTrangThai->setBackground(QBrush(QColor(246, 78, 96, 40)));
        itemTrangThai->setForeground(QBrush(QColor("#F64E60")));
    }
    itemTrangThai->setFont(QFont("Segoe UI", 10, QFont::Bold));
    bangDocGia->setItem(hang, 5, itemTrangThai);
    
    // Sách Đang Mượn
    QString sachDangMuon = QString("%1 / 3").arg(dg.soSachDangMuon);
    QTableWidgetItem *itemSachMuon = new QTableWidgetItem(sachDangMuon);
    itemSachMuon->setTextAlignment(Qt::AlignCenter);
    itemSachMuon->setFont(QFont("Segoe UI", 10, QFont::Bold));
    
    if (dg.soSachDangMuon == 0) {
        itemSachMuon->setForeground(QBrush(QColor("#B5B5C3")));
    } else if (dg.soSachDangMuon >= 3) {
        itemSachMuon->setBackground(QBrush(QColor(246, 78, 96, 50)));
        itemSachMuon->setForeground(QBrush(QColor("#F64E60")));
    } else {
        itemSachMuon->setBackground(QBrush(QColor(255, 168, 0, 40)));
        itemSachMuon->setForeground(QBrush(QColor("#FFA800")));
    }
    bangDocGia->setItem(hang, 6, itemSachMuon);
    
    // Quá Hạn
    int soNgayQuaHan = TinhSoNgayQuaHanLonNhat(docGia);
    QTableWidgetItem *itemQuaHan = new QTableWidgetItem();
    itemQuaHan->setTextAlignment(Qt::AlignCenter);
    
    if (soNgayQuaHan > 0) {
        itemQuaHan->setText(QString("%1 ngày").arg(soNgayQuaHan));
        itemQuaHan->setBackground(QBrush(QColor(246, 78, 96, 50)));
        itemQuaHan->setForeground(QBrush(QColor("#F64E60")));
        itemQuaHan->setFont(QFont("Segoe UI", 10, QFont::Bold));
    } else {
        itemQuaHan->setText("—");
        itemQuaHan->setForeground(QBrush(QColor("#B5B5C3")));
    }
    bangDocGia->setItem(hang, 7, itemQuaHan);
    
    // Tổng Mượn
    int tongMuon = 0;
    MUONTRA mt = dg.dsmt;
    while (mt) {
        tongMuon++;
        mt = mt->next;
    }
    QTableWidgetItem *itemTongMuon = new QTableWidgetItem(QString::number(tongMuon));
    itemTongMuon->setTextAlignment(Qt::AlignCenter);
    itemTongMuon->setForeground(QBrush(QColor("#3699FF")));
    bangDocGia->setItem(hang, 8, itemTongMuon);
}
void WidgetQuanLyDocGia::xuLyNhapDuLieuTu() {}
void WidgetQuanLyDocGia::hienThiThongTinChiTiet(PTRDG) {}

QString WidgetQuanLyDocGia::layTenTrangThai(int trangThai) const {
    return (trangThai == 1) ? "Hoạt động" : "Bị khóa";
}

QString WidgetQuanLyDocGia::layTenGioiTinh(bool phai) const {
    return phai ? "Nữ" : "Nam";
}

QColor WidgetQuanLyDocGia::layMauTrangThai(int trangThai) const {
    return (trangThai == 1) ? QColor("#1BC5BD") : QColor("#F64E60");
}
