#include "giao_dien/hop_thoai/HopThoaiSuaSach.h"
#include "utils/LanguageManager.h"
#include "utils/FilePathManager.h"
#include "models/QuanLySach.h"
#include "models/ThaoTacFile.h"
#include "utils/ResponsiveHelper.h"
#include "utils/XuLyChuoi.h"
#include "utils/EventBus.h"
#include "utils/ToastNotification.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QScrollArea>
#include <QCompleter>
#include <QDateTime>
#include <QTimer>

HopThoaiSuaSach::HopThoaiSuaSach(const QString& isbn, QWidget *cha)
    : QDialog(cha), isbnGoc(isbn), sachHienTai(nullptr) {
    setWindowTitle(LanguageManager::instance().getText("DIALOG_EDIT_BOOK_TITLE"));
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    setMinimumWidth(ResponsiveHelper::getScaledWidth(600, screenSize));
    
    // Find book
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] && dsDauSach[i]->ISBN == isbn.toStdString()) {
            sachHienTai = dsDauSach[i];
            break;
        }
    }
    
    thieLapGiaoDien();
    
    // Apply Deep Night Premium Theme
    this->setStyleSheet("QDialog { background-color: #1e1e2d; color: #ffffff; }");
    
    taiDuLieuSach();
}

void HopThoaiSuaSach::thieLapGiaoDien() {
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    QVBoxLayout *boTriChinh = new QVBoxLayout(this);
    int margin = ResponsiveHelper::getScaledMargin(25, screenSize);
    boTriChinh->setContentsMargins(margin, margin, margin, margin);
    
    QLabel *tieuDe = new QLabel(LanguageManager::instance().getText("TITLE_EDIT_BOOK_HEADER"));
    int titleSize = ResponsiveHelper::getScaledFontSize(20, screenSize);
    tieuDe->setStyleSheet(QString("font-size: %1px; font-weight: bold; color: #FFFFFF;").arg(titleSize));
    tieuDe->setAlignment(Qt::AlignCenter);
    boTriChinh->addWidget(tieuDe);
    
    // Scroll Area Setup
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; } QScrollBar:vertical { width: 8px; background: #1b1b29; } QScrollBar::handle:vertical { background: #565674; border-radius: 4px; }");

    QWidget *scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;");
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 10, 0); 
    int spacing = ResponsiveHelper::getScaledSpacing(10, screenSize); // Define spacing variable
    scrollLayout->setSpacing(spacing);

    // Group Box
    QGroupBox *nhomForm = new QGroupBox(LanguageManager::instance().getText("GROUP_BOOK_INFO"), scrollContent);
    int fontSize = ResponsiveHelper::getScaledFontSize(14, screenSize);
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
    
    int fieldHeight = ResponsiveHelper::getScaledHeight(38, screenSize);
    QString kieuTruong = QString(
        "QLineEdit, QSpinBox, QComboBox, QDoubleSpinBox {"
        "   padding: 0 10px;"
        "   border: 1px solid #323248;"
        "   border-radius: 6px;"
        "   font-size: %1px;"
        "   background-color: #1b1b29;"
        "   color: #ffffff;"
        "   min-height: %2px;"
        "   selection-background-color: #5D9CEC;"
        "}"
        "QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QDoubleSpinBox:focus {"
        "   border: 1px solid #5D9CEC;"
        "   background-color: #1b1b29;"
        "}"
    ).arg(fontSize).arg(fieldHeight);
    
    oNhapISBN = new QLineEdit();
    oNhapISBN->setReadOnly(true); // ISBN cannot be changed
    oNhapISBN->setStyleSheet("background-color: #151521; color: #565674; padding: 0 10px; border: 1px solid #323248; border-radius: 6px; min-height: " + QString::number(fieldHeight) + "px;");
    QLabel *nhanISBN = new QLabel(LanguageManager::instance().getText("TABLE_HEADER_ISBN") + ":");
    nhanISBN->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanISBN, oNhapISBN);
    
    oNhapTenSach = new QLineEdit();
    oNhapTenSach->setStyleSheet(kieuTruong);
    connect(oNhapTenSach, &QLineEdit::textChanged, this, &HopThoaiSuaSach::kiemTraDuLieu);
    QLabel *nhanTenSach = new QLabel(LanguageManager::instance().getText("TABLE_HEADER_TITLE") + ":");
    nhanTenSach->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanTenSach, oNhapTenSach);
    
    oNhapTacGia = new QLineEdit();
    oNhapTacGia->setStyleSheet(kieuTruong);
    connect(oNhapTacGia, &QLineEdit::textChanged, this, &HopThoaiSuaSach::kiemTraDuLieu);
    QLabel *nhanTacGia = new QLabel(LanguageManager::instance().getText("TABLE_HEADER_AUTHOR") + ":");
    nhanTacGia->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanTacGia, oNhapTacGia);

    // Phase 2: Publisher
    oNhapNhaXuatBan = new QLineEdit();
    oNhapNhaXuatBan->setStyleSheet(kieuTruong);
    QLabel *nhanNXB = new QLabel(LanguageManager::instance().getText("TABLE_HEADER_PUBLISHER") + ":");
    nhanNXB->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanNXB, oNhapNhaXuatBan);
    
    oNhapTheLoai = new QLineEdit();
    oNhapTheLoai->setStyleSheet(kieuTruong);
    connect(oNhapTheLoai, &QLineEdit::textChanged, this, &HopThoaiSuaSach::kiemTraDuLieu);
    
    // Auto complete
    QStringList goiYTheLoai = {"Văn học", "Khoa học", "Công nghệ", "Lịch sử", "Triết học", 
                              "Kinh tế", "Tâm lý", "Nghệ thuật", "Thiếu nhi", "Giáo dục",
                              "Y học", "Thể thao", "Du lịch", "Ẩm thực", "Tôn giáo"};
    QCompleter *completer = new QCompleter(goiYTheLoai, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    oNhapTheLoai->setCompleter(completer);
    
    QLabel *nhanTheLoai = new QLabel(LanguageManager::instance().getText("TABLE_HEADER_CATEGORY") + ":");
    nhanTheLoai->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanTheLoai, oNhapTheLoai);
    
    soNamXuatBan = new QSpinBox();
    soNamXuatBan->setRange(1000, 3000);
    soNamXuatBan->setStyleSheet(kieuTruong);
    soNamXuatBan->setButtonSymbols(QAbstractSpinBox::NoButtons);
    QLabel *nhanNam = new QLabel(LanguageManager::instance().getText("TABLE_HEADER_YEAR") + ":");
    nhanNam->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanNam, nhanNam);
    
    soSoTrang = new QSpinBox();
    soSoTrang->setRange(1, 10000);
    soSoTrang->setStyleSheet(kieuTruong);
    soSoTrang->setButtonSymbols(QAbstractSpinBox::NoButtons);
    QLabel *nhanSoTrang = new QLabel(LanguageManager::instance().getText("TABLE_HEADER_PAGES") + ":");
    nhanSoTrang->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(nhanSoTrang, soSoTrang);

    // Feature: Price Input
    oNhapGiaTien = new QDoubleSpinBox();
    oNhapGiaTien->setRange(0, 1000000000); // 1 billion max
    oNhapGiaTien->setSingleStep(1000);
    oNhapGiaTien->setDecimals(0);
    oNhapGiaTien->setSuffix(" VNĐ");
    oNhapGiaTien->setStyleSheet(kieuTruong);
    oNhapGiaTien->setButtonSymbols(QAbstractSpinBox::NoButtons);
    
    QLabel *lblGia = new QLabel(LanguageManager::instance().getText("LBL_PRICE") + ":");
    lblGia->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(lblGia, oNhapGiaTien);

    // Location
    oNhapViTri = new QLineEdit();
    oNhapViTri->setStyleSheet(kieuTruong);
    QLabel *lblViTri = new QLabel(LanguageManager::instance().getText("LBL_LOCATION") + ":");
    lblViTri->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 90px;");
    boTriForm->addRow(lblViTri, oNhapViTri);
    
    scrollLayout->addWidget(nhomForm);
    
    nhanTrangThai = new QLabel("");
    nhanTrangThai->setAlignment(Qt::AlignCenter);
    nhanTrangThai->setStyleSheet("color: #B5B5C3; font-style: italic;");
    scrollLayout->addWidget(nhanTrangThai);
    
    // FINALIZE SCROLL
    scrollContent->setLayout(scrollLayout);
    scrollArea->setWidget(scrollContent);
    boTriChinh->addWidget(scrollArea);
    
    QHBoxLayout *boTriNut = new QHBoxLayout();
    int buttonHeight = ResponsiveHelper::getScaledHeight(45, screenSize);
    int buttonWidth = ResponsiveHelper::getScaledWidth(130, screenSize);
    
    nutLuu = new QPushButton(LanguageManager::instance().getText("BTN_SAVE_CHANGES"));
    nutLuu->setMinimumSize(buttonWidth, buttonHeight);
    nutLuu->setCursor(Qt::PointingHandCursor);
    nutLuu->setStyleSheet(
        "QPushButton {"
        "   background-color: #5D9CEC;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 6px;"
        "   font-weight: bold;"
        "   font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #4A89DC; }"
        "QPushButton:pressed { background-color: #3b6bbf; }"
    );
    connect(nutLuu, &QPushButton::clicked, this, &HopThoaiSuaSach::xuLyNutLuu);
    
    nutHuy = new QPushButton(LanguageManager::instance().getText("BTN_CANCEL"));
    nutHuy->setMinimumSize(buttonWidth, buttonHeight);
    nutHuy->setCursor(Qt::PointingHandCursor);
    nutHuy->setStyleSheet(
        "QPushButton {"
        "   background-color: #2b2b40;"
        "   color: #ffffff;"
        "   border: 1px solid #323248;"
        "   border-radius: 6px;"
        "   font-weight: 600;"
        "   font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #323248; border-color: #f64e60; }"
        "QPushButton:pressed { background-color: #1b1b29; }"
    );
    connect(nutHuy, &QPushButton::clicked, this, &QDialog::reject);
    
    boTriNut->addStretch();
    boTriNut->addWidget(nutLuu);
    boTriNut->addWidget(nutHuy);
    boTriChinh->addLayout(boTriNut);
}

void HopThoaiSuaSach::taiDuLieuSach() {
    if (!sachHienTai) {
        QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_ERROR"), LanguageManager::instance().getText("MSG_BOOK_NOT_FOUND"));
        reject();
        return;
    }
    
    oNhapISBN->setText(QString::fromStdString(sachHienTai->ISBN));
    oNhapTenSach->setText(QString::fromStdString(sachHienTai->tenSach));
    oNhapTacGia->setText(QString::fromStdString(sachHienTai->tacGia));
    oNhapTheLoai->setText(QString::fromStdString(sachHienTai->theLoai));
    soNamXuatBan->setValue(sachHienTai->namXuatBan);
    soSoTrang->setValue(sachHienTai->soTrang);
    oNhapGiaTien->setValue((double)sachHienTai->giaTien);
    oNhapNhaXuatBan->setText(QString::fromStdString(sachHienTai->NhaXuatBan));
    if (sachHienTai->dms && !sachHienTai->dms->viTri.empty()) {
        oNhapViTri->setText(QString::fromStdString(sachHienTai->dms->viTri));
    } else {
        oNhapViTri->clear();
    }
}

void HopThoaiSuaSach::kiemTraDuLieu() {
    bool hopLe = !oNhapTenSach->text().trimmed().isEmpty() && !oNhapTacGia->text().trimmed().isEmpty();
    nutLuu->setEnabled(hopLe);
}

void HopThoaiSuaSach::xuLyNutLuu() {
    if (!sachHienTai) return;
    
    // Disable buttons để tránh double-click
    nutLuu->setEnabled(false);
    nutHuy->setEnabled(false);
    nutLuu->setText(LanguageManager::instance().getText("MSG_PROCESSING"));
    
    // Hiển thị status
    nhanTrangThai->setText(LanguageManager::instance().getText("MSG_UPDATING"));
    nhanTrangThai->setStyleSheet("color: #FFA800; font-weight: bold;");
    QApplication::processEvents();
    
    std::string tenSach = oNhapTenSach->text().trimmed().toStdString();
    std::string tacGia = oNhapTacGia->text().trimmed().toStdString();
    std::string theLoai = oNhapTheLoai->text().trimmed().toStdString();
    int namXB = soNamXuatBan->value();
    int soTrang = soSoTrang->value();
    long long giaTien = (long long)oNhapGiaTien->value();
    std::string nhaXuatBan = oNhapNhaXuatBan->text().trimmed().toStdString();
    std::string viTri = oNhapViTri->text().trimmed().toStdString();
    
    // CHUẨN HÓA DỮ LIỆU
    std::string tenChuan, tacGiaChuan, theLoaiChuan;
    
    std::string loiTen = ChuanHoaTenUnicode(tenSach, 1, 500, tenChuan);
    if (!loiTen.empty()) {
        nhanTrangThai->setText("✕ " + QString::fromStdString(loiTen));
        nhanTrangThai->setStyleSheet("color: #E06D6D; font-weight: bold;");
        nutLuu->setEnabled(true);
        nutHuy->setEnabled(true);
        nutLuu->setText(LanguageManager::instance().getText("BTN_SAVE"));
        return;
    }
    
    std::string loiTacGia = ChuanHoaTenUnicode(tacGia, 1, 100, tacGiaChuan);
    if (!loiTacGia.empty()) {
        nhanTrangThai->setText("✕ " + QString::fromStdString(loiTacGia));
        nhanTrangThai->setStyleSheet("color: #E06D6D; font-weight: bold;");
        nutLuu->setEnabled(true);
        nutHuy->setEnabled(true);
        nutLuu->setText(LanguageManager::instance().getText("BTN_SAVE"));
        return;
    }
    
    theLoaiChuan = ChuyenThanhTitleCase(ChuanHoaKhoangTrang(theLoai));
    
    // Normalize new fields
    std::string nhaXuatBanChuan;
    if (!nhaXuatBan.empty()) ChuanHoaTenUnicode(nhaXuatBan, 0, 100, nhaXuatBanChuan);
    else nhaXuatBanChuan = "";
    
    std::string viTriChuan = ChuanHoaViTri(viTri);
    
    // Sử dụng hàm backend CapNhatThongTinDauSach với dữ liệu đã chuẩn hóa
    std::string ketQua = CapNhatThongTinDauSach(
        sachHienTai->ISBN, tenChuan, soTrang, tacGiaChuan, namXB, theLoaiChuan, viTriChuan, giaTien, nhaXuatBanChuan
    );
    
    if (ketQua.find("Thành công") != std::string::npos || ketQua.find("thành công") != std::string::npos) {
        // Dữ liệu đã được đồng bộ SQL bên trong CapNhatThongTinDauSach
        // Show Toast only
        ToastNotification::show(LanguageManager::instance().getText("MSG_UPDATE_SUCCESS_VAR").arg(QString::fromStdString(tenChuan)), ToastNotification::SUCCESS);
        
        emit sachDaSua();
        EventBus::instance().dauSachUpdated(QString::fromStdString(sachHienTai->ISBN)); // REALTIME UPDATE
        accept(); // Exit immediately
    } else {
        nhanTrangThai->setText("✕ " + QString::fromStdString(ketQua));
        ToastNotification::show(LanguageManager::instance().getText("MSG_ERROR_UPDATE_FAILED") + ": " + QString::fromStdString(ketQua), ToastNotification::ERROR);
        nhanTrangThai->setStyleSheet("color: #E06D6D; font-weight: bold; font-size: 13px;");
        nutLuu->setEnabled(true);
        nutHuy->setEnabled(true);
        nutLuu->setText(LanguageManager::instance().getText("BTN_SAVE"));
    }
}
