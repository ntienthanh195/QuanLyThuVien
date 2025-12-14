#include "giao_dien/hop_thoai/HopThoaiThemSach.h"
#include "models/QuanLySach.h"
#include "models/KiemTraDuLieu.h"
#include "models/ThaoTacFile.h"
#include "utils/ResponsiveHelper.h"
#include "utils/AnimationHelper.h"
#include "utils/XuLyChuoi.h"
#include "utils/EventBus.h"
#include "utils/ToastNotification.h"

#include "utils/ToastNotification.h"
#include "utils/LanguageManager.h"

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
    setWindowTitle(LanguageManager::instance().getText("DIALOG_ADD_BOOK_TITLE"));
    
    // Responsive window size
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    int dialogWidth = ResponsiveHelper::getScaledWidth(520, screenSize);
    setMinimumWidth(dialogWidth);
    // Set fixed initial width so dialog doesn't appear too large on screen
    setFixedWidth(dialogWidth);
    
    // Apply Deep Night Premium Theme
    this->setStyleSheet("QDialog { background-color: #1e1e2d; color: #ffffff; }");

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

    QLabel *nhanTieuDe = new QLabel(LanguageManager::instance().getText("DIALOG_ADD_BOOK_TITLE"), this);
    nhanTieuDe->setStyleSheet(QString(
        "font-size: %1px; font-weight: bold; color: #FFFFFF; "
        "margin-bottom: 5px; background: transparent;"
    ).arg(titleSize));
    nhanTieuDe->setAlignment(Qt::AlignCenter);
    boTriChinh->addWidget(nhanTieuDe);
    
    // Form group with Deep Night Theme
    int fontSize = ResponsiveHelper::getScaledFontSize(12, screenSize);

    QGroupBox *nhomForm = new QGroupBox(LanguageManager::instance().getText("GROUP_BOOK_INFO"), this);
    nhomForm->setStyleSheet(QString(
        "QGroupBox {"
        "   background-color: #1e1e2d;"
        "   border: 1px solid #323248;"
        "   border-radius: 6px;"
        "   margin-top: 12px;"
        "   padding-top: 18px;"
        "   font-weight: bold;"
        "   font-size: %1px;"
        "   color: #ffffff;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top left;"
        "   left: 10px;"
        "   top: -10px;"
        "   padding: 0 5px;"
        "   color: #5D9CEC;"
        "   background-color: #1e1e2d;"
        "   font-size: 12px;"
        "}"
    ).arg(fontSize));
    
    QFormLayout *boTriForm = new QFormLayout(nhomForm);
    int formSpacing = ResponsiveHelper::getScaledSpacing(12, screenSize);
    boTriForm->setSpacing(formSpacing);
    boTriForm->setHorizontalSpacing(15);
    boTriForm->setVerticalSpacing(12);
    boTriForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    boTriForm->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    
    // Responsive field height
    int fieldHeight = ResponsiveHelper::getScaledHeight(38, screenSize);
    
    // Deep Night Input Field stylesheet
    QString kieuTruong = QString(
        "QLineEdit, QSpinBox, QComboBox {"
        "   padding: 0 10px;"
        "   border: 1px solid #323248;"
        "   border-radius: 6px;"
        "   font-size: %1px;"
        "   background-color: #1b1b29;"
        "   color: #ffffff;"
        "   min-height: %2px;"
        "   min-width: 300px;"
        "   selection-background-color: #5D9CEC;"
        "}"
        "QLineEdit:focus, QSpinBox:focus, QComboBox:focus {"
        "   border: 1px solid #5D9CEC;"
        "   background-color: #1b1b29;"
        "}"
        "QLineEdit:disabled, QSpinBox:disabled, QComboBox:disabled {"
        "   background-color: #151521;"
        "   color: #565674;"
        "   border-color: #2b2b40;"
        "}"
    ).arg(fontSize).arg(fieldHeight);
    

    // ISBN field
    oNhapISBN = new QLineEdit(this);
    oNhapISBN->setPlaceholderText("VD: 9781234567890"); // Generic?
    oNhapISBN->setMaxLength(13);
    oNhapISBN->setStyleSheet(kieuTruong);
    QLabel *nhanISBN = new QLabel(LanguageManager::instance().getText("TABLE_HEADER_ISBN") + " *:", this);
    nhanISBN->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanISBN, oNhapISBN);
    

    // Book name
    oNhapTenSach = new QLineEdit(this);
    oNhapTenSach->setPlaceholderText("...");
    oNhapTenSach->setMaxLength(100);
    oNhapTenSach->setStyleSheet(kieuTruong);
    QLabel *nhanTenSach = new QLabel(LanguageManager::instance().getText("TABLE_HEADER_TITLE") + " *:", this);
    nhanTenSach->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanTenSach, oNhapTenSach);
    

    // Author
    oNhapTacGia = new QLineEdit(this);
    oNhapTacGia->setPlaceholderText("...");
    oNhapTacGia->setMaxLength(50);
    oNhapTacGia->setStyleSheet(kieuTruong);
    QLabel *nhanTacGia = new QLabel(LanguageManager::instance().getText("TABLE_HEADER_AUTHOR") + " *:", this);
    nhanTacGia->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanTacGia, oNhapTacGia);
    

    // Genre
    hopChonTheLoai = new QComboBox(this);
    hopChonTheLoai->setEditable(true);
    hopChonTheLoai->setStyleSheet(kieuTruong);
    diemDayTheLoai();
    QLabel *nhanTheLoai = new QLabel(LanguageManager::instance().getText("TABLE_HEADER_CATEGORY") + " *:", this);
    nhanTheLoai->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanTheLoai, hopChonTheLoai);
    

    // Year
    soNamXuatBan = new QSpinBox(this);
    soNamXuatBan->setRange(1000, QDateTime::currentDateTime().date().year());
    soNamXuatBan->setValue(2024); // Dynamically?
    soNamXuatBan->setSuffix(" " + LanguageManager::instance().getText("TABLE_HEADER_YEAR")); // " Năm XB"? No, suffix " year".
    // Or just empty suffix?
    soNamXuatBan->setSuffix(""); 
    soNamXuatBan->setStyleSheet(kieuTruong);
    QLabel *nhanNam = new QLabel(LanguageManager::instance().getText("TABLE_HEADER_YEAR") + " *:", this);
    nhanNam->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanNam, soNamXuatBan);
    

    // Pages
    soSoTrang = new QSpinBox(this);
    soSoTrang->setRange(1, 10000);
    soSoTrang->setValue(100);
    soSoTrang->setSuffix("");
    soSoTrang->setStyleSheet(kieuTruong);
    QLabel *nhanSoTrang = new QLabel(LanguageManager::instance().getText("TABLE_HEADER_PAGES") + " *:", this);
    nhanSoTrang->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanSoTrang, soSoTrang);
    

    // Copies
    soSoBanSao = new QSpinBox(this);
    soSoBanSao->setRange(1, 100);
    soSoBanSao->setValue(1);
    soSoBanSao->setSuffix("");
    soSoBanSao->setStyleSheet(kieuTruong);
    QLabel *nhanSoBanSao = new QLabel(LanguageManager::instance().getText("TABLE_HEADER_COPIES") + ":", this);
    nhanSoBanSao->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanSoBanSao, soSoBanSao);
    

    // Price
    soGiaTien = new QDoubleSpinBox(this);
    soGiaTien->setRange(0, 1000000000); // Max 1 billion
    soGiaTien->setValue(0); // Default 0
    soGiaTien->setSuffix("");
    soGiaTien->setSingleStep(1000); // Step 1k
    soGiaTien->setDecimals(0);
    soGiaTien->setStyleSheet(kieuTruong);
    QLabel *nhanGiaTien = new QLabel(LanguageManager::instance().getText("LBL_PRICE") + ":", this);
    nhanGiaTien->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanGiaTien, soGiaTien);


    // Location
    oNhapViTri = new QLineEdit(this);
    oNhapViTri->setPlaceholderText("");
    oNhapViTri->setMaxLength(30);
    oNhapViTri->setStyleSheet(kieuTruong);
    QLabel *nhanViTri = new QLabel(LanguageManager::instance().getText("LBL_LOCATION") + ":", this);
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
    
    nutXoaForm = new QPushButton(LanguageManager::instance().getText("BTN_REFRESH_FORM"), this);
    nutXoaForm->setMinimumSize(buttonWidth, buttonHeight);
    nutXoaForm->setCursor(Qt::PointingHandCursor);
    nutXoaForm->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: #2b2b40;"
        "   color: #ffffff;"
        "   border: 1px solid #323248;"
        "   border-radius: 6px;"
        "   font-size: %1px;"
        "   font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #323248; border-color: #5D9CEC; }"
        "QPushButton:pressed { background-color: #1b1b29; }"
    ).arg(buttonFontSize));
    
    boTriNut->addWidget(nutXoaForm);
    boTriNut->addStretch();
    
    nutLuu = new QPushButton(LanguageManager::instance().getText("BTN_ADD_BOOK_CONFIRM"), this);
    nutLuu->setMinimumSize(buttonWidth, buttonHeight);
    nutLuu->setCursor(Qt::PointingHandCursor);
    nutLuu->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: #5D9CEC;"
        "   color: #ffffff;"
        "   border: none;"
        "   border-radius: 6px;"
        "   font-size: %1px;"
        "   font-weight: 700;"
        "}"
        "QPushButton:hover { background-color: #4A89DC; }"
        "QPushButton:pressed { background-color: #3b6bbf; }"
        "QPushButton:disabled { background-color: #2b2b40; color: #565674; }"
    ).arg(buttonFontSize));
    nutLuu->setDefault(true);
    nutLuu->setAutoDefault(true);
    

    
    nutHuy = new QPushButton(LanguageManager::instance().getText("BTN_CANCEL_DIALOG"), this);
    nutHuy->setMinimumSize(buttonWidth, buttonHeight);
    nutHuy->setCursor(Qt::PointingHandCursor);
    nutHuy->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: #2b2b40;"
        "   color: #ffffff;"
        "   border: 1px solid #323248;"
        "   border-radius: 6px;"
        "   font-size: %1px;"
        "   font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #323248; border-color: #E06D6D; }" // Red hover border
        "QPushButton:pressed { background-color: #1b1b29; }"
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
    setTabOrder(soSoBanSao, soGiaTien);
    setTabOrder(soGiaTien, oNhapViTri);
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
        nhanTrangThai->setStyleSheet("color: #78C2AD;");
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

    long long giaTien = (long long)soGiaTien->value();
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
    
    std::string ketQua = ThemDauSachMoi(isbn, tenChuan, trang, tacGiaChuan, nam, theLoaiChuan, soBanSao, viTriChuan, giaTien);
    
    if (ketQua.find("Loi") == std::string::npos && ketQua.find("ERROR") == std::string::npos) {
        // Thành công
        AnimationHelper::fadeIn(nhanTrangThai, 200);
        
        // Show Toast only
        ToastNotification::show(QString("Đã thêm sách '%1' thành công!").arg(QString::fromStdString(tenChuan)), ToastNotification::SUCCESS);
        
        // Phát tín hiệu để refresh table
        emit sachDaThem();
        EventBus::instance().dauSachAdded(QString::fromStdString(isbn)); // REALTIME UPDATE
        
        // Đánh dấu dữ liệu đã thay đổi
        DanhDauDuLieuThayDoi();
        
        accept(); // Exit immediately instead of reset
    } else {
        // Lỗi
        hienThiLoi("✕ " + QString::fromStdString(ketQua));
        ToastNotification::show("Thêm sách thất bại: " + QString::fromStdString(ketQua), ToastNotification::ERROR);
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
    soGiaTien->setValue(50000); // Reset price
    oNhapViTri->clear();
    nhanTrangThai->clear();
    
    oNhapISBN->setFocus();
}

void HopThoaiThemSach::hienThiLoi(const QString &thongDiep) {
    nhanTrangThai->setText(thongDiep);
    // Error red but slightly muted for dark theme
    nhanTrangThai->setStyleSheet("color: #E06D6D; font-weight: 500; font-size: 13px;");
}

void HopThoaiThemSach::hienThiThanhCong(const QString &thongDiep) {
    nhanTrangThai->setText(thongDiep);
    // Success green but legible on dark
    nhanTrangThai->setStyleSheet("color: #78C2AD; font-weight: 500; font-size: 13px;");
}

void HopThoaiThemSach::datKieuTruong(QLineEdit *truong, bool hopLe) {
    if (hopLe) {
        truong->setStyleSheet(truong->styleSheet() + " QLineEdit { border: 1px solid #3E3E42; }");
    } else {
        truong->setStyleSheet(truong->styleSheet() + " QLineEdit { border: 1px solid #E06D6D; }");
    }
}
