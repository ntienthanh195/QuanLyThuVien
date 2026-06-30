#include "ui/dialogs/HopThoaiThemSach.h"
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
#include <QCompleter>
#include <QScrollArea>
#include <QRegularExpressionValidator>
#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <QKeyEvent>

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

void HopThoaiThemSach::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
        // If focus is on a button, let the button handle it
        if (qobject_cast<QPushButton*>(focusWidget())) {
            QDialog::keyPressEvent(e);
            return;
        }
        // Otherwise, move to next field
        focusNextChild();
    } else {
        QDialog::keyPressEvent(e);
    }
}

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
    
    // Scroll Area Setup
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; } QScrollBar:vertical { width: 8px; background: #1b1b29; } QScrollBar::handle:vertical { background: #565674; border-radius: 4px; }");

    QWidget *scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;");
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 10, 0); 
    scrollLayout->setSpacing(spacing);

    // Group 1: Book Info
    QGroupBox *nhomThongTin = new QGroupBox(LanguageManager::instance().getText("GROUP_BOOK_INFO"), this);
    int fontSize = ResponsiveHelper::getScaledFontSize(14, screenSize);
    nhomThongTin->setStyleSheet(QString(
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
    
    QFormLayout *layoutThongTin = new QFormLayout(nhomThongTin);
    layoutThongTin->setHorizontalSpacing(20);
    layoutThongTin->setVerticalSpacing(12);
    layoutThongTin->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    int fieldHeight = ResponsiveHelper::getScaledHeight(40, screenSize);
    QString kieuTruong = QString(
        "QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {"
        "   padding: 0 10px;"
        "   border: 1px solid #323248;"
        "   border-radius: 6px;"
        "   font-size: %1px;"
        "   background-color: #1b1b29;"
        "   color: #ffffff;"
        "   min-height: %2px;"
        "   selection-background-color: #5D9CEC;"
        "}"
        "QComboBox QAbstractItemView {"
        "   background-color: #1b1b29;"
        "   color: #ffffff;"
        "   selection-background-color: #3e3e5e;"
        "   border: 1px solid #323248;"
        "}"
        "QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus {"
        "   border: 1px solid #5D9CEC;"
        "   background-color: #1b1b29;"
        "}"
    ).arg(fontSize).arg(fieldHeight);
    
    // ... Input fields ...
    oNhapISBN = new QLineEdit();
    oNhapISBN->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_ISBN"));
    oNhapISBN->setStyleSheet(kieuTruong);
    // Validator: only numbers and dashes
    QRegularExpression rxISBN("[0-9\\-]+");
    QValidator *validatorISBN = new QRegularExpressionValidator(rxISBN, this);
    oNhapISBN->setValidator(validatorISBN);
    connect(oNhapISBN, &QLineEdit::textChanged, this, &HopThoaiThemSach::kiemTraTatCaTruong); // Validate
    
    oNhapTenSach = new QLineEdit();
    oNhapTenSach->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_BOOK_TITLE"));
    oNhapTenSach->setStyleSheet(kieuTruong);
    connect(oNhapTenSach, &QLineEdit::textChanged, this, &HopThoaiThemSach::kiemTraTatCaTruong); // Validate
    
    oNhapTacGia = new QLineEdit();
    oNhapTacGia->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_AUTHOR"));
    oNhapTacGia->setStyleSheet(kieuTruong);
    connect(oNhapTacGia, &QLineEdit::textChanged, this, &HopThoaiThemSach::kiemTraTatCaTruong); // Validate

    // Phase 2: Publisher
    oNhapNhaXuatBan = new QLineEdit();
    oNhapNhaXuatBan->setPlaceholderText(LanguageManager::instance().getText("TABLE_HEADER_PUBLISHER"));
    oNhapNhaXuatBan->setStyleSheet(kieuTruong);
    
    // ✨ IMPROVED: QComboBox editable - vừa chọn dropdown, vừa nhập tay
    oNhapTheLoai = new QComboBox();
    oNhapTheLoai->setEditable(true); // ← Cho phép gõ tự do
    oNhapTheLoai->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_CATEGORY"));
    oNhapTheLoai->setStyleSheet(kieuTruong);
    
    // Danh sách thể loại chuẩn cho Học viện Phật giáo Nam tông Khmer
    QStringList goiYTheLoai = {
        "294.3821 - Luật Tạng (Vinaya)",
        "294.3822 - Kinh Tạng (Sutta)",
        "294.3823 - Vi Diệu Pháp (Abhidhamma)",
        "294.385 - Chú giải (Atthakatha)",
        "294.385.2 - Phụ chú giải (Tika)",
        "294.381 - Ngữ pháp Pali",
        "294.38T.1 - Giáo trình Sơ cấp",
        "294.38T.2 - Giáo trình Trung cấp",
        "294.38T.3 - Giáo trình Cao cấp",
        "294.3R.1 - Lịch sử Phật giáo",
        "294.3R.2 - Văn hóa Phật giáo Khmer",
        "294.3R.3 - Thiền định",
        "294.3R.4 - Nghi lễ Phật giáo",
        "Kinh Pali", 
        "Kinh Khmer", 
        "Triết học Phật giáo",
        "Nghệ thuật Phật giáo",
        "Kinh lá buông"
    };
    oNhapTheLoai->addItems(goiYTheLoai);
    oNhapTheLoai->setCurrentIndex(-1); // Không chọn mặc định
    
    // AutoComplete khi gõ trong ComboBox
    QCompleter *completer = new QCompleter(goiYTheLoai, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    oNhapTheLoai->setCompleter(completer);
    
    // Validation khi thay đổi (dùng signal của ComboBox)
    connect(oNhapTheLoai->lineEdit(), &QLineEdit::textChanged, this, &HopThoaiThemSach::kiemTraTatCaTruong);
    
    oNhapNamXB = new QSpinBox();
    oNhapNamXB->setRange(1000, 3000);
    oNhapNamXB->setValue(QDate::currentDate().year());
    oNhapNamXB->setStyleSheet(kieuTruong);
    oNhapNamXB->setButtonSymbols(QAbstractSpinBox::NoButtons); // Hide up/down buttons for cleaner look, or Keep them? Let's keep cleaner look
    
    oNhapSoTrang = new QSpinBox();
    oNhapSoTrang->setRange(1, 10000);
    oNhapSoTrang->setValue(100);
    oNhapSoTrang->setStyleSheet(kieuTruong);
    oNhapSoTrang->setButtonSymbols(QAbstractSpinBox::NoButtons);

    // Feature: Price Input
    oNhapGiaTien = new QDoubleSpinBox();
    oNhapGiaTien->setRange(0, 1000000000); // 1 billion max
    oNhapGiaTien->setSingleStep(1000);
    oNhapGiaTien->setDecimals(0);
    oNhapGiaTien->setSuffix(" VNĐ");
    oNhapGiaTien->setStyleSheet(kieuTruong);
    oNhapGiaTien->setButtonSymbols(QAbstractSpinBox::NoButtons);
    
    QLabel *lblISBN = new QLabel(LanguageManager::instance().getText("LBL_ISBN"));
    lblISBN->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    QLabel *lblTen = new QLabel(LanguageManager::instance().getText("LBL_TITLE"));
    lblTen->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    QLabel *lblTacGia = new QLabel(LanguageManager::instance().getText("LBL_AUTHOR"));
    lblTacGia->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    QLabel *lblNhaXuatBan = new QLabel(LanguageManager::instance().getText("TABLE_HEADER_PUBLISHER"));
    lblNhaXuatBan->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    QLabel *lblTheLoai = new QLabel(LanguageManager::instance().getText("LBL_CATEGORY"));
    lblTheLoai->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    QLabel *lblNam = new QLabel(LanguageManager::instance().getText("LBL_PUBLISH_YEAR"));
    lblNam->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    QLabel *lblTrang = new QLabel(LanguageManager::instance().getText("LBL_PAGE_COUNT"));
    lblTrang->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    QLabel *lblGia = new QLabel(LanguageManager::instance().getText("LBL_PRICE"));
    lblGia->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    
    layoutThongTin->addRow(lblISBN, oNhapISBN);
    layoutThongTin->addRow(lblTen, oNhapTenSach);
    layoutThongTin->addRow(lblTacGia, oNhapTacGia);
    layoutThongTin->addRow(lblNhaXuatBan, oNhapNhaXuatBan);
    layoutThongTin->addRow(lblTheLoai, oNhapTheLoai);
    layoutThongTin->addRow(lblNam, oNhapNamXB);
    layoutThongTin->addRow(lblTrang, oNhapSoTrang);
    layoutThongTin->addRow(lblGia, oNhapGiaTien);
    
    scrollLayout->addWidget(nhomThongTin);
    
    // Group 2: Copy Info
    QGroupBox *nhomBanSao = new QGroupBox(LanguageManager::instance().getText("GROUP_COPY_INFO"), this);
    nhomBanSao->setStyleSheet(nhomThongTin->styleSheet());
    QFormLayout *layoutBanSao = new QFormLayout(nhomBanSao);
    layoutBanSao->setHorizontalSpacing(20);
    layoutBanSao->setVerticalSpacing(12);
    layoutBanSao->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    oNhapSoBanSao = new QSpinBox();
    oNhapSoBanSao->setRange(1, 100);
    oNhapSoBanSao->setValue(1);
    oNhapSoBanSao->setStyleSheet(kieuTruong);
    
    oNhapViTri = new QLineEdit();
    oNhapViTri->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_LOCATION")); // e.g., "Kệ A1"
    oNhapViTri->setStyleSheet(kieuTruong);
    
    QLabel *lblSoBanSao = new QLabel(LanguageManager::instance().getText("LBL_COPY_COUNT"));
    lblSoBanSao->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    QLabel *lblViTri = new QLabel(LanguageManager::instance().getText("LBL_LOCATION"));
    lblViTri->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    
    layoutBanSao->addRow(lblSoBanSao, oNhapSoBanSao);
    layoutBanSao->addRow(lblViTri, oNhapViTri);
    
    scrollLayout->addWidget(nhomBanSao);
    
    // Status
    nhanTrangThai = new QLabel("");
    nhanTrangThai->setAlignment(Qt::AlignCenter);
    nhanTrangThai->setStyleSheet("color: #B5B5C3; font-style: italic;");
    scrollLayout->addWidget(nhanTrangThai);

    // FINALIZE SCROLL
    scrollContent->setLayout(scrollLayout);
    scrollArea->setWidget(scrollContent);
    boTriChinh->addWidget(scrollArea);
    
    // Note
    QLabel *nhanChuThich = new QLabel(LanguageManager::instance().getText("LBL_MUST_ENTER"), this);
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
    nutXoaForm->setAutoDefault(false); // Fix: Prevent Enter from triggering clear
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
    setTabOrder(oNhapTacGia, oNhapNhaXuatBan);
    setTabOrder(oNhapNhaXuatBan, oNhapTheLoai);
    setTabOrder(oNhapTheLoai, oNhapNamXB);
    setTabOrder(oNhapNamXB, oNhapSoTrang);
    setTabOrder(oNhapSoTrang, oNhapSoBanSao);
    setTabOrder(oNhapSoBanSao, oNhapGiaTien);
    setTabOrder(oNhapGiaTien, oNhapViTri);
    setTabOrder(oNhapViTri, nutLuu);
}

void HopThoaiThemSach::diemDayTheLoai() {
    // Auto-complete setup handled in thieLapGiaoDien
}

void HopThoaiThemSach::kiemTraISBN() {
    QString isbn = oNhapISBN->text().trimmed();
    if (isbn.isEmpty()) {
        datKieuTruong(oNhapISBN, true); // Normal
        return;
    }
    
    // Check format
    static QRegularExpression regex("^[0-9\\-]+$");
    QRegularExpressionMatch match = regex.match(isbn);
    if (!match.hasMatch()) {
        hienThiLoi(LanguageManager::instance().getText("MSG_ISBN_INVALID_FORMAT"));
        datKieuTruong(oNhapISBN, false);
        return;
    }
    
    // Check duplicate
    // extern PTRDS dsDauSach[]; // REMOVED
    extern int soLuongDauSach;
    for (int i=0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] && dsDauSach[i]->ISBN == isbn.toStdString()) {
            hienThiLoi(LanguageManager::instance().getText("MSG_ISBN_EXISTS"));
            datKieuTruong(oNhapISBN, false);
            return;
        }
    }
    
    nhanTrangThai->setText("");
    datKieuTruong(oNhapISBN, true);
}

void HopThoaiThemSach::kiemTraTatCaTruong() {
    bool hopLe = kiemTraCacTruong();
    nutLuu->setEnabled(hopLe);
    
    if (hopLe) {
        nhanTrangThai->setText(LanguageManager::instance().getText("MSG_READY_TO_SAVE"));
        nhanTrangThai->setStyleSheet("color: #78C2AD;");
    }
}

bool HopThoaiThemSach::kiemTraCacTruong() {
    if (oNhapISBN->text().trimmed().isEmpty()) return false;
    // ... validate other fields
    
    if (oNhapTenSach->text().trimmed().isEmpty()) return false;
    if (oNhapTacGia->text().trimmed().isEmpty()) return false;
    if (oNhapTheLoai->currentText().trimmed().isEmpty()) return false;
    
    return true;
}

void HopThoaiThemSach::xuLyNutLuu() {
    // Disable all buttons để tránh double-click
    nutLuu->setEnabled(false);
    nutHuy->setEnabled(false);
    nutXoaForm->setEnabled(false);
    nutLuu->setText(LanguageManager::instance().getText("BTN_PROCESSING"));
    
    if (!kiemTraCacTruong()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_ERROR"), LanguageManager::instance().getText("ERR_FILL_REQUIRED"));
        nutLuu->setEnabled(true);
        nutHuy->setEnabled(true);
        nutXoaForm->setEnabled(true);
        nutLuu->setText(LanguageManager::instance().getText("BTN_ADD_BOOK_CONFIRM"));
        return;
    }
    
    // Hiển thị progress
    nhanTrangThai->setText(LanguageManager::instance().getText("MSG_CHECKING_DATA"));
    nhanTrangThai->setStyleSheet("color: #FFA800; font-weight: bold;");
    QApplication::processEvents();
    
    // Lấy dữ liệu từ form
    std::string isbn = oNhapISBN->text().trimmed().toStdString();
    std::string ten = oNhapTenSach->text().trimmed().toStdString();
    std::string tacGia = oNhapTacGia->text().trimmed().toStdString();
    std::string theLoai = oNhapTheLoai->currentText().trimmed().toStdString();
    int nam = oNhapNamXB->value();
    int trang = oNhapSoTrang->value();
    int soBanSao = oNhapSoBanSao->value();

    long long giaTien = (long long)oNhapGiaTien->value();
    std::string viTri = oNhapViTri->text().trimmed().toStdString();
    std::string nhaXuatBan = oNhapNhaXuatBan->text().trimmed().toStdString();
    
    // CHUẨN HÓA DỮ LIỆU - TẬN DỤNG BACKEND
    std::string tenChuan, tacGiaChuan, theLoaiChuan, viTriChuan;
    
    // Chuẩn hóa tên sách (Title Case, xóa khoảng trắng thừa)
    std::string loiTen = ChuanHoaTenUnicode(ten, 1, 500, tenChuan);
    if (!loiTen.empty()) {
        hienThiLoi("Lỗi: " + QString::fromStdString(loiTen));
        oNhapTenSach->setFocus();
        nutLuu->setEnabled(true);
        nutLuu->setText(LanguageManager::instance().getText("BTN_ADD_BOOK_CONFIRM"));
        return;
    }
    
    // Chuẩn hóa tác giả (Title Case)
    std::string loiTacGia = ChuanHoaTenUnicode(tacGia, 1, 100, tacGiaChuan);
    if (!loiTacGia.empty()) {
        hienThiLoi("Lỗi: " + QString::fromStdString(loiTacGia));
        oNhapTacGia->setFocus();
        nutLuu->setEnabled(true);
        nutLuu->setText(LanguageManager::instance().getText("BTN_ADD_BOOK_CONFIRM"));
        return;
    }
    
    // Chuẩn hóa thể loại
    theLoaiChuan = ChuyenThanhTitleCase(ChuanHoaKhoangTrang(theLoai));
    
    // Chuẩn hóa Nhà Xuất Bản
    std::string nhaXuatBanChuan;
    ChuanHoaTenUnicode(nhaXuatBan, 1, 100, nhaXuatBanChuan); // Allows empty if min=0, but let's use 1 if we want to enforce or 0 if optional.
    // Publisher implies Name, so Title Case is good.
    if (nhaXuatBanChuan.empty()) nhaXuatBanChuan = "N/A"; // Or leave empty? Let's leave empty if user didn't type.
    if (!nhaXuatBan.empty()) ChuanHoaTenUnicode(nhaXuatBan, 0, 100, nhaXuatBanChuan); 
    else nhaXuatBanChuan = "";
    
    // Chuẩn hóa vị trí
    viTriChuan = ChuanHoaViTri(viTri);
    if (viTriChuan.empty()) {
        viTriChuan = "Chưa xác định";
    }
    
    // DÙNG BACKEND SERVICE LAYER - ThemDauSachMoi() với dữ liệu đã chuẩn hóa
    nhanTrangThai->setText(LanguageManager::instance().getText("MSG_SAVING_TO_DB"));
    QApplication::processEvents();
    
    std::string ketQua = ThemDauSachMoi(isbn, tenChuan, trang, tacGiaChuan, nam, theLoaiChuan, soBanSao, viTriChuan, giaTien, nhaXuatBanChuan);
    
    if (ketQua.empty()) {
        // Thành công
        AnimationHelper::fadeIn(nhanTrangThai, 200);
        
        // Show Toast only
        ToastNotification::show(LanguageManager::instance().getText("MSG_ADD_BOOK_SUCCESS_TITLE").arg(QString::fromStdString(tenChuan)), ToastNotification::SUCCESS);
        
        // Phát tín hiệu để refresh table
        emit sachDaThem();
        EventBus::instance().dauSachAdded(QString::fromStdString(isbn)); // REALTIME UPDATE
        
        // Đánh dấu dữ liệu đã thay đổi
        DanhDauDuLieuThayDoi();
        
        accept(); // Exit immediately instead of reset
    } else {
        // Lỗi
        hienThiLoi("✕ " + QString::fromStdString(ketQua));
        ToastNotification::show(LanguageManager::instance().getText("MSG_ADD_BOOK_FAILED").arg(QString::fromStdString(ketQua)), ToastNotification::ERROR);
        nutLuu->setEnabled(true);
        nutHuy->setEnabled(true);
        nutXoaForm->setEnabled(true);
        nutLuu->setText(LanguageManager::instance().getText("BTN_ADD_BOOK_CONFIRM"));
    }
}

void HopThoaiThemSach::xuLyNutHuy() {
    reject();
}

void HopThoaiThemSach::xoaTatCaTruong() {
    oNhapISBN->clear();
    oNhapTenSach->clear();
    oNhapTenSach->clear();
    oNhapTacGia->clear();
    oNhapNhaXuatBan->clear();
    oNhapTheLoai->clear();
    oNhapNamXB->setValue(2024);
    oNhapSoTrang->setValue(100);
    oNhapSoBanSao->setValue(1);
    oNhapGiaTien->setValue(50000); // Reset price
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
