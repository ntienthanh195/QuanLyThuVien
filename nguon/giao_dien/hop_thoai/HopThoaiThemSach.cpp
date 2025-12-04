#include "giao_dien/hop_thoai/HopThoaiThemSach.h"
#include "models/QuanLySach.h"
#include "models/KiemTraDuLieu.h"
#include "models/ThaoTacFile.h"
#include "utils/ResponsiveHelper.h"
#include "utils/AnimationHelper.h"
#include "utils/XuLyChuoi.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QDateTime>
#include <QDebug>
#include <QTimer>

HopThoaiThemSach::HopThoaiThemSach(QWidget *cha)
    : QDialog(cha)
{
    setWindowTitle("Thêm Đầu Sách Mới");
    
    // Responsive window size
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    int dialogWidth = ResponsiveHelper::getScaledWidth(520, screenSize);
    setMinimumWidth(dialogWidth);
    // Set fixed initial width so dialog doesn't appear too large on screen
    setFixedWidth(dialogWidth);
    
    thieLapGiaoDien();
    thieLapKetNoi();
    thieLapKiemTra();
    thieLapPhimTat();
    thieLapThuTuTab();
    
    qDebug() << "HopThoaiThemSach: Dialog width" << dialogWidth;
}

HopThoaiThemSach::~HopThoaiThemSach() {}

void HopThoaiThemSach::thieLapGiaoDien() {
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    
    QVBoxLayout *boTriChinh = new QVBoxLayout(this);
    int spacing = ResponsiveHelper::getScaledSpacing(10, screenSize);
    int margin = ResponsiveHelper::getScaledMargin(15, screenSize);
    boTriChinh->setSpacing(spacing);
    boTriChinh->setContentsMargins(margin, margin, margin, margin);
    
    // Title - responsive
    int titleSize = ResponsiveHelper::getScaledFontSize(18, screenSize);
    QLabel *nhanTieuDe = new QLabel("THÊM ĐẦU SÁCH MỚI", this);
    nhanTieuDe->setStyleSheet(QString(
        "font-size: %1px; font-weight: bold; color: #FFFFFF; "
        "margin-bottom: 5px; background: transparent;"
    ).arg(titleSize));
    nhanTieuDe->setAlignment(Qt::AlignCenter);
    boTriChinh->addWidget(nhanTieuDe);
    
    // Form group
    int fontSize = ResponsiveHelper::getScaledFontSize(12, screenSize);
    QGroupBox *nhomForm = new QGroupBox("Thông Tin Sách", this);
    nhomForm->setStyleSheet(QString(
        "QGroupBox {"
        "   background-color: #1E1E2D;"
        "   border: 2px solid #3699FF;"
        "   border-radius: 12px;"
        "   margin-top: 12px;"
        "   padding-top: 15px;"
        "   font-weight: bold;"
        "   font-size: %1px;"
        "   color: #FFFFFF;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top left;"
        "   left: 15px;"
        "   top: -12px;"
        "   padding: 0 10px;"
        "   color: #3699FF;"
        "   background-color: #1E1E2D;"
        "}"
    ).arg(fontSize));
    
    QFormLayout *boTriForm = new QFormLayout(nhomForm);
    int formSpacing = ResponsiveHelper::getScaledSpacing(10, screenSize);
    boTriForm->setSpacing(formSpacing);
    boTriForm->setHorizontalSpacing(15);
    boTriForm->setVerticalSpacing(8);
    boTriForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    boTriForm->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    
    // Responsive field height
    int fieldHeight = ResponsiveHelper::getScaledHeight(35, screenSize);
    
    // Field stylesheet
    QString kieuTruong = QString(
        "QLineEdit, QSpinBox, QComboBox {"
        "   padding: 6px;"
        "   border: 2px solid #2B2B40;"
        "   border-radius: 8px;"
        "   font-size: %1px;"
        "   background-color: #252534;"
        "   color: #FFFFFF;"
        "   min-height: %2px;"
        "}"
        "QLineEdit:focus, QSpinBox:focus, QComboBox:focus {"
        "   border-color: #3699FF;"
        "}"
        "QLineEdit:disabled, QSpinBox:disabled, QComboBox:disabled {"
        "   background-color: #1E1E2D;"
        "   color: #6C6C80;"
        "}"
    ).arg(fontSize).arg(fieldHeight);
    
    // ISBN field
    oNhapISBN = new QLineEdit(this);
    oNhapISBN->setPlaceholderText("Nhập 10 hoặc 13 chữ số (VD: 9781234567890)");
    oNhapISBN->setMaxLength(13);
    oNhapISBN->setStyleSheet(kieuTruong);
    QLabel *nhanISBN = new QLabel("ISBN *:", this);
    nhanISBN->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanISBN, oNhapISBN);
    
    // Book name
    oNhapTenSach = new QLineEdit(this);
    oNhapTenSach->setPlaceholderText("Tên sách (tối đa 100 ký tự)");
    oNhapTenSach->setMaxLength(100);
    oNhapTenSach->setStyleSheet(kieuTruong);
    QLabel *nhanTenSach = new QLabel("Tên sách *:", this);
    nhanTenSach->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanTenSach, oNhapTenSach);
    
    // Author
    oNhapTacGia = new QLineEdit(this);
    oNhapTacGia->setPlaceholderText("Tên tác giả (tối đa 50 ký tự)");
    oNhapTacGia->setMaxLength(50);
    oNhapTacGia->setStyleSheet(kieuTruong);
    QLabel *nhanTacGia = new QLabel("Tác giả *:", this);
    nhanTacGia->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanTacGia, oNhapTacGia);
    
    // Genre
    hopChonTheLoai = new QComboBox(this);
    hopChonTheLoai->setEditable(true);
    hopChonTheLoai->setStyleSheet(kieuTruong);
    diemDayTheLoai();
    QLabel *nhanTheLoai = new QLabel("Thể loại *:", this);
    nhanTheLoai->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanTheLoai, hopChonTheLoai);
    
    // Year
    soNamXuatBan = new QSpinBox(this);
    soNamXuatBan->setRange(1000, QDateTime::currentDateTime().date().year());
    soNamXuatBan->setValue(2024);
    soNamXuatBan->setSuffix(" năm");
    soNamXuatBan->setStyleSheet(kieuTruong);
    QLabel *nhanNam = new QLabel("Năm XB *:", this);
    nhanNam->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanNam, soNamXuatBan);
    
    // Pages
    soSoTrang = new QSpinBox(this);
    soSoTrang->setRange(1, 10000);
    soSoTrang->setValue(100);
    soSoTrang->setSuffix(" trang");
    soSoTrang->setStyleSheet(kieuTruong);
    QLabel *nhanSoTrang = new QLabel("Số trang *:", this);
    nhanSoTrang->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanSoTrang, soSoTrang);
    
    // Copies
    soSoBanSao = new QSpinBox(this);
    soSoBanSao->setRange(1, 100);
    soSoBanSao->setValue(1);
    soSoBanSao->setSuffix(" bản");
    soSoBanSao->setStyleSheet(kieuTruong);
    QLabel *nhanSoBanSao = new QLabel("Số bản sao:", this);
    nhanSoBanSao->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanSoBanSao, soSoBanSao);
    
    // Location
    oNhapViTri = new QLineEdit(this);
    oNhapViTri->setPlaceholderText("VD: Kệ A1, Phòng 201");
    oNhapViTri->setMaxLength(30);
    oNhapViTri->setStyleSheet(kieuTruong);
    QLabel *nhanViTri = new QLabel("Vị trí:", this);
    nhanViTri->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanViTri, oNhapViTri);
    
    boTriChinh->addWidget(nhomForm);
    
    // Status label
    nhanTrangThai = new QLabel("", this);
    nhanTrangThai->setWordWrap(true);
    nhanTrangThai->setMinimumHeight(ResponsiveHelper::getScaledHeight(25, screenSize));
    nhanTrangThai->setAlignment(Qt::AlignCenter);
    nhanTrangThai->setStyleSheet("color: #B5B5C3; font-style: italic;");
    boTriChinh->addWidget(nhanTrangThai);
    
    // Note
    QLabel *nhanChuThich = new QLabel("(*) Các trường bắt buộc", this);
    nhanChuThich->setStyleSheet(QString("color: #6C6C80; font-style: italic; font-size: %1px;").arg(fontSize - 2));
    boTriChinh->addWidget(nhanChuThich);
    
    // Buttons
    int buttonHeight = ResponsiveHelper::getScaledHeight(40, screenSize);
    int buttonWidth = ResponsiveHelper::getScaledWidth(120, screenSize);
    int buttonFontSize = ResponsiveHelper::getScaledFontSize(13, screenSize);
    
    QHBoxLayout *boTriNut = new QHBoxLayout();
    
    nutXoaForm = new QPushButton("↺ Làm Mới", this);
    nutXoaForm->setMinimumSize(buttonWidth, buttonHeight);
    nutXoaForm->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: #6C6C80;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   font-size: %1px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #5A5A70; }"
        "QPushButton:pressed { background-color: #4A4A60; }"
    ).arg(buttonFontSize));
    
    boTriNut->addWidget(nutXoaForm);
    boTriNut->addStretch();
    
    nutLuu = new QPushButton("Thêm", this);
    nutLuu->setMinimumSize(buttonWidth, buttonHeight);
    nutLuu->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: #1BC5BD;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   font-size: %1px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #0F9A94; }"
        "QPushButton:pressed { background-color: #0C7A75; }"
        "QPushButton:disabled { background-color: #2B2B40; color: #6C6C80; }"
    ).arg(buttonFontSize));
    nutLuu->setDefault(true);
    nutLuu->setAutoDefault(true);
    
    nutHuy = new QPushButton("Hủy", this);
    nutHuy->setMinimumSize(buttonWidth, buttonHeight);
    nutHuy->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: #F64E60;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   font-size: %1px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #D63447; }"
        "QPushButton:pressed { background-color: #B62838; }"
    ).arg(buttonFontSize));
    
    boTriNut->addWidget(nutLuu);
    boTriNut->addWidget(nutHuy);
    boTriChinh->addLayout(boTriNut);
}

void HopThoaiThemSach::thieLapKetNoi() {
    connect(nutLuu, &QPushButton::clicked, this, &HopThoaiThemSach::xuLyNutLuu);
    connect(nutHuy, &QPushButton::clicked, this, &HopThoaiThemSach::xuLyNutHuy);
    connect(nutXoaForm, &QPushButton::clicked, this, &HopThoaiThemSach::xoaTatCaTruong);
    connect(oNhapISBN, &QLineEdit::textChanged, this, &HopThoaiThemSach::kiemTraISBN);
    connect(oNhapTenSach, &QLineEdit::textChanged, this, &HopThoaiThemSach::kiemTraTatCaTruong);
    connect(oNhapTacGia, &QLineEdit::textChanged, this, &HopThoaiThemSach::kiemTraTatCaTruong);
}

void HopThoaiThemSach::thieLapKiemTra() {
    // ISBN validator: only digits
    QRegularExpressionValidator *kiemTraISBN = 
        new QRegularExpressionValidator(QRegularExpression("^[0-9]{0,13}$"), this);
    oNhapISBN->setValidator(kiemTraISBN);
    
    nutLuu->setEnabled(false);
}

void HopThoaiThemSach::thieLapPhimTat() {
    phimTatLuu = new QShortcut(QKeySequence("Ctrl+S"), this);
    connect(phimTatLuu, &QShortcut::activated, this, &HopThoaiThemSach::xuLyNutLuu);
    
    phimTatHuy = new QShortcut(QKeySequence("Esc"), this);
    connect(phimTatHuy, &QShortcut::activated, this, &HopThoaiThemSach::xuLyNutHuy);
}

void HopThoaiThemSach::thieLapThuTuTab() {
    setTabOrder(oNhapISBN, oNhapTenSach);
    setTabOrder(oNhapTenSach, oNhapTacGia);
    setTabOrder(oNhapTacGia, hopChonTheLoai);
    setTabOrder(hopChonTheLoai, soNamXuatBan);
    setTabOrder(soNamXuatBan, soSoTrang);
    setTabOrder(soSoTrang, soSoBanSao);
    setTabOrder(soSoBanSao, oNhapViTri);
    setTabOrder(oNhapViTri, nutLuu);
}

void HopThoaiThemSach::diemDayTheLoai() {
    hopChonTheLoai->addItems({
        "Văn học", "Khoa học", "Công nghệ", "Lịch sử", "Triết học",
        "Kinh tế", "Tâm lý", "Nghệ thuật", "Thiếu nhi", "Giáo dục",
        "Y học", "Thể thao", "Du lịch", "Ẩm thực", "Tôn giáo"
    });
    hopChonTheLoai->setCurrentIndex(-1);
}

void HopThoaiThemSach::kiemTraISBN() {
    QString isbn = oNhapISBN->text().trimmed();
    
    if (isbn.isEmpty()) {
        datKieuTruong(oNhapISBN, true);
        return;
    }
    
    if (isbn.length() != 10 && isbn.length() != 13) {
        datKieuTruong(oNhapISBN, false);
        hienThiLoi("ISBN phải có 10 hoặc 13 chữ số");
        return;
    }
    
    // Check if exists
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] && dsDauSach[i]->ISBN == isbn.toStdString()) {
            datKieuTruong(oNhapISBN, false);
            hienThiLoi("ISBN này đã tồn tại trong hệ thống");
            return;
        }
    }
    
    datKieuTruong(oNhapISBN, true);
    kiemTraTatCaTruong();
}

void HopThoaiThemSach::kiemTraTatCaTruong() {
    bool hopLe = kiemTraCacTruong();
    nutLuu->setEnabled(hopLe);
    
    if (hopLe) {
        nhanTrangThai->setText("✓ Sẵn sàng lưu");
        nhanTrangThai->setStyleSheet("color: #1BC5BD;");
    }
}

bool HopThoaiThemSach::kiemTraCacTruong() {
    QString isbn = oNhapISBN->text().trimmed();
    if (isbn.length() != 10 && isbn.length() != 13) return false;
    
    if (oNhapTenSach->text().trimmed().isEmpty()) return false;
    if (oNhapTacGia->text().trimmed().isEmpty()) return false;
    if (hopChonTheLoai->currentText().trimmed().isEmpty()) return false;
    
    return true;
}

void HopThoaiThemSach::xuLyNutLuu() {
    // Disable all buttons để tránh double-click
    nutLuu->setEnabled(false);
    nutHuy->setEnabled(false);
    nutXoaForm->setEnabled(false);
    nutLuu->setText("Đang xử lý...");
    
    if (!kiemTraCacTruong()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng điền đầy đủ các trường bắt buộc!");
        nutLuu->setEnabled(true);
        nutHuy->setEnabled(true);
        nutXoaForm->setEnabled(true);
        nutLuu->setText("Lưu");
        return;
    }
    
    // Hiển thị progress
    nhanTrangThai->setText("⚙ Đang kiểm tra dữ liệu...");
    nhanTrangThai->setStyleSheet("color: #FFA800; font-weight: bold;");
    QApplication::processEvents();
    
    // Lấy dữ liệu từ form
    std::string isbn = oNhapISBN->text().trimmed().toStdString();
    std::string ten = oNhapTenSach->text().trimmed().toStdString();
    std::string tacGia = oNhapTacGia->text().trimmed().toStdString();
    std::string theLoai = hopChonTheLoai->currentText().trimmed().toStdString();
    int nam = soNamXuatBan->value();
    int trang = soSoTrang->value();
    int soBanSao = soSoBanSao->value();
    std::string viTri = oNhapViTri->text().trimmed().toStdString();
    
    // CHUẨN HÓA DỮ LIỆU - TẬN DỤNG BACKEND
    std::string tenChuan, tacGiaChuan, theLoaiChuan, viTriChuan;
    
    // Chuẩn hóa tên sách (Title Case, xóa khoảng trắng thừa)
    std::string loiTen = ChuanHoaTenUnicode(ten, 1, 500, tenChuan);
    if (!loiTen.empty()) {
        hienThiLoi("Lỗi: " + QString::fromStdString(loiTen));
        oNhapTenSach->setFocus();
        nutLuu->setEnabled(true);
        nutLuu->setText("Lưu");
        return;
    }
    
    // Chuẩn hóa tác giả (Title Case)
    std::string loiTacGia = ChuanHoaTenUnicode(tacGia, 1, 100, tacGiaChuan);
    if (!loiTacGia.empty()) {
        hienThiLoi("Lỗi: " + QString::fromStdString(loiTacGia));
        oNhapTacGia->setFocus();
        nutLuu->setEnabled(true);
        nutLuu->setText("Lưu");
        return;
    }
    
    // Chuẩn hóa thể loại
    theLoaiChuan = ChuyenThanhTitleCase(ChuanHoaKhoangTrang(theLoai));
    
    // Chuẩn hóa vị trí
    viTriChuan = ChuanHoaViTri(viTri);
    if (viTriChuan.empty()) {
        viTriChuan = "Chưa xác định";
    }
    
    // DÙNG BACKEND SERVICE LAYER - ThemDauSachMoi() với dữ liệu đã chuẩn hóa
    nhanTrangThai->setText("Đang lưu vào cơ sở dữ liệu...");
    QApplication::processEvents();
    
    std::string ketQua = ThemDauSachMoi(isbn, tenChuan, trang, tacGiaChuan, nam, theLoaiChuan, soBanSao, viTriChuan);
    
    if (ketQua.find("Loi") == std::string::npos && ketQua.find("ERROR") == std::string::npos) {
        // Thành công
        AnimationHelper::fadeIn(nhanTrangThai, 200);
        hienThiThanhCong(QString("✓ Đã thêm '%1' với %2 bản sao!")
                        .arg(QString::fromStdString(tenChuan))
                        .arg(soBanSao));
        
        // Phát tín hiệu để refresh table
        emit sachDaThem();
        
        // Đánh dấu dữ liệu đã thay đổi
        DanhDauDuLieuThayDoi();
        
        // Reset form để thêm tiếp
        xoaTatCaTruong();
        oNhapISBN->setFocus();
        
        // Auto-enable buttons sau khi success
        QTimer::singleShot(1500, this, [this]() {
            nutLuu->setEnabled(false);
            nutHuy->setEnabled(true);
            nutXoaForm->setEnabled(true);
            nutLuu->setText("Thêm");
        });
    } else {
        // Lỗi
        hienThiLoi("✕ " + QString::fromStdString(ketQua));
        nutLuu->setEnabled(true);
        nutHuy->setEnabled(true);
        nutXoaForm->setEnabled(true);
        nutLuu->setText("Thêm");
    }
}

void HopThoaiThemSach::xuLyNutHuy() {
    reject();
}

void HopThoaiThemSach::xoaTatCaTruong() {
    oNhapISBN->clear();
    oNhapTenSach->clear();
    oNhapTacGia->clear();
    hopChonTheLoai->setCurrentIndex(-1);
    soNamXuatBan->setValue(2024);
    soSoTrang->setValue(100);
    soSoBanSao->setValue(1);
    oNhapViTri->clear();
    nhanTrangThai->clear();
    
    oNhapISBN->setFocus();
}

void HopThoaiThemSach::hienThiLoi(const QString &thongDiep) {
    nhanTrangThai->setText(thongDiep);
    nhanTrangThai->setStyleSheet("color: #F64E60; font-weight: bold; font-size: 13px;");
    AnimationHelper::shake(nhanTrangThai, 400);
}

void HopThoaiThemSach::hienThiThanhCong(const QString &thongDiep) {
    nhanTrangThai->setText(thongDiep);
    nhanTrangThai->setStyleSheet("color: #1BC5BD; font-weight: bold; font-size: 13px;");
    AnimationHelper::pulse(nhanTrangThai, 500);
}

void HopThoaiThemSach::datKieuTruong(QLineEdit *truong, bool hopLe) {
    if (hopLe) {
        truong->setStyleSheet(truong->styleSheet() + " QLineEdit { border-color: #1BC5BD; }");
    } else {
        truong->setStyleSheet(truong->styleSheet() + " QLineEdit { border-color: #F64E60; }");
    }
}
