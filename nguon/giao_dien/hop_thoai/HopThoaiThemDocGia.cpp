#include "giao_dien/hop_thoai/HopThoaiThemDocGia.h"
#include "models/DocGia.h"
#include "utils/LanguageManager.h"
#include "utils/ResponsiveHelper.h"
#include "utils/XuLyChuoi.h"
#include "utils/EventBus.h"
#include "utils/ToastNotification.h"
#include "utils/PrintHelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QDateTime>
#include <QScrollArea>
#include <QTimer>
#include <QTimer>
#include <functional>
#include <QKeyEvent>

HopThoaiThemDocGia::HopThoaiThemDocGia(QWidget *cha) : QDialog(cha), maDuocTao(0) {
    setWindowTitle(LanguageManager::instance().getText("DIALOG_ADD_READER_TITLE"));
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    setMinimumWidth(ResponsiveHelper::getScaledWidth(550, screenSize));
    
    // Apply Deep Night Premium Theme
    this->setStyleSheet("QDialog { background-color: #1e1e2d; color: #ffffff; }");

    thieLapGiaoDien();
    taoMaDocGia();
}

void HopThoaiThemDocGia::keyPressEvent(QKeyEvent *e) {
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



void HopThoaiThemDocGia::thieLapGiaoDien() {
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    QVBoxLayout *boTriChinh = new QVBoxLayout(this);
    int margin = ResponsiveHelper::getScaledMargin(25, screenSize);
    int spacing = ResponsiveHelper::getScaledSpacing(15, screenSize);
    boTriChinh->setContentsMargins(margin, margin, margin, margin);
    boTriChinh->setSpacing(spacing);
    
    // Title
    int titleSize = ResponsiveHelper::getScaledFontSize(20, screenSize);
    QLabel *tieuDe = new QLabel(LanguageManager::instance().getText("DIALOG_ADD_READER_TITLE").toUpper());
    tieuDe->setStyleSheet(QString("font-size: %1px; font-weight: bold; color: #FFFFFF; background: transparent;").arg(titleSize));
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
    scrollLayout->setContentsMargins(0, 0, 10, 0); // Right padding for scrollbar
    scrollLayout->setSpacing(spacing);

    // MOVE CONTENT INTO SCROLL LAYOUT
    
    // Mã thẻ
    QGroupBox *nhomMa = new QGroupBox(LanguageManager::instance().getText("GROUP_READER_CODE"), this);
    int fontSize = ResponsiveHelper::getScaledFontSize(14, screenSize);
    nhomMa->setStyleSheet(QString(
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
    QVBoxLayout *boTriMa = new QVBoxLayout(nhomMa);
    
    // Initialize nhanMaThe (Fix crash)
    nhanMaThe = new QLabel("-");
    nhanMaThe->setAlignment(Qt::AlignCenter);
    int maSize = ResponsiveHelper::getScaledFontSize(28, screenSize);
    nhanMaThe->setStyleSheet(QString("font-size: %1px; font-weight: bold; color: #5D9CEC; padding: 15px; background-color: #1b1b29; border-radius: 8px; border: 1px dashed #323248;").arg(maSize));
    boTriMa->addWidget(nhanMaThe);
    
    QLabel *chuThich = new QLabel(LanguageManager::instance().getText("LBL_AUTO_CODE_HINT"));
    chuThich->setStyleSheet("color: #6C6C80; font-style: italic; font-size: 11px;");
    chuThich->setAlignment(Qt::AlignCenter);
    boTriMa->addWidget(chuThich);
    scrollLayout->addWidget(nhomMa);
    
    // Form
    QGroupBox *nhomForm = new QGroupBox(LanguageManager::instance().getText("GROUP_READER_INFO"), this);
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
    boTriForm->setHorizontalSpacing(20);
    boTriForm->setVerticalSpacing(12);
    boTriForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    int fieldHeight = ResponsiveHelper::getScaledHeight(40, screenSize);
    QString kieuTruong = QString(
        "QLineEdit, QComboBox {"
        "   padding: 0 10px;"
        "   border: 1px solid #323248;"
        "   border-radius: 6px;"
        "   font-size: %1px;"
        "   background-color: #1b1b29;"
        "   color: #ffffff;"
        "   min-height: %2px;"
        "   selection-background-color: #5D9CEC;"
        "}"
        "QLineEdit:focus, QComboBox:focus {"
        "   border: 1px solid #5D9CEC;"
        "   background-color: #1b1b29;"
        "}"
    ).arg(fontSize).arg(fieldHeight);
    
    oNhapHo = new QLineEdit();
    oNhapHo->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_HO"));
    oNhapHo->setStyleSheet(kieuTruong);
    connect(oNhapHo, &QLineEdit::textChanged, this, &HopThoaiThemDocGia::kiemTraDuLieu);
    connect(oNhapHo, &QLineEdit::textChanged, this, &HopThoaiThemDocGia::capNhatPreviewHoTen);
    
    oNhapTen = new QLineEdit();
    oNhapTen->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_TEN"));
    oNhapTen->setStyleSheet(kieuTruong);
    connect(oNhapTen, &QLineEdit::textChanged, this, &HopThoaiThemDocGia::kiemTraDuLieu);
    connect(oNhapTen, &QLineEdit::textChanged, this, &HopThoaiThemDocGia::capNhatPreviewHoTen);
    
    hopChonGioiTinh = new QComboBox();
    hopChonGioiTinh->addItem(LanguageManager::instance().getText("GENDER_MALE"), 0);
    hopChonGioiTinh->addItem(LanguageManager::instance().getText("GENDER_FEMALE"), 1);
    hopChonGioiTinh->setStyleSheet(kieuTruong);
    
    // Phase 2 New Fields
    hopChonLoaiDocGia = new QComboBox();
    hopChonLoaiDocGia->addItem(LanguageManager::instance().getText("READER_TYPE_STUDENT"), 0);
    hopChonLoaiDocGia->addItem(LanguageManager::instance().getText("READER_TYPE_TEACHER"), 1);
    hopChonLoaiDocGia->setStyleSheet(kieuTruong);

    oNhapLop = new QLineEdit();
    oNhapLop->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_CLASS"));
    oNhapLop->setStyleSheet(kieuTruong);

    oNhapSDT = new QLineEdit();
    oNhapSDT->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_PHONE"));
    oNhapSDT->setStyleSheet(kieuTruong);
    connect(oNhapSDT, &QLineEdit::textChanged, this, &HopThoaiThemDocGia::kiemTraDuLieu);

    oNhapEmail = new QLineEdit();
    oNhapEmail->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_EMAIL"));
    oNhapEmail->setStyleSheet(kieuTruong);

    oNhapNgaySinh = new QDateEdit();
    oNhapNgaySinh->setDisplayFormat("dd/MM/yyyy");
    oNhapNgaySinh->setCalendarPopup(true);
    oNhapNgaySinh->setDate(QDate::currentDate().addYears(-15)); // Default ~15 years old
    oNhapNgaySinh->setStyleSheet(kieuTruong);
    
    QLabel *nhanHo = new QLabel(LanguageManager::instance().getText("LBL_HO_DEM"));
    nhanHo->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    QLabel *nhanTen = new QLabel(LanguageManager::instance().getText("LBL_TEN"));
    nhanTen->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    QLabel *nhanGioiTinh = new QLabel(LanguageManager::instance().getText("TABLE_HEADER_GENDER") + ":");
    nhanGioiTinh->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    
    QLabel *nhanLoai = new QLabel(LanguageManager::instance().getText("LBL_READER_TYPE"));
    nhanLoai->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    QLabel *nhanLop = new QLabel(LanguageManager::instance().getText("LBL_CLASS"));
    nhanLop->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    QLabel *nhanSDT = new QLabel(LanguageManager::instance().getText("LBL_PHONE") + " (*)");
    nhanSDT->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    QLabel *nhanEmail = new QLabel(LanguageManager::instance().getText("LBL_EMAIL"));
    nhanEmail->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    QLabel *nhanNgaySinh = new QLabel(LanguageManager::instance().getText("LBL_DOB"));
    nhanNgaySinh->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");

    boTriForm->addRow(nhanHo, oNhapHo);
    boTriForm->addRow(nhanTen, oNhapTen);
    boTriForm->addRow(nhanLoai, hopChonLoaiDocGia);
    boTriForm->addRow(nhanLop, oNhapLop);
    boTriForm->addRow(nhanNgaySinh, oNhapNgaySinh);
    boTriForm->addRow(nhanGioiTinh, hopChonGioiTinh);
    boTriForm->addRow(nhanSDT, oNhapSDT);
    boTriForm->addRow(nhanEmail, oNhapEmail);
    scrollLayout->addWidget(nhomForm);
    
    // Status
    nhanTrangThai = new QLabel("");
    nhanTrangThai->setAlignment(Qt::AlignCenter);
    nhanTrangThai->setStyleSheet("color: #B5B5C3; font-style: italic;");
    scrollLayout->addWidget(nhanTrangThai);

    // Preview normalized name
    nhanPreviewHoTen = new QLabel("");
    nhanPreviewHoTen->setStyleSheet("color: #A1F0E8; font-weight: 600; padding: 6px; background: transparent;");
    nhanPreviewHoTen->setAlignment(Qt::AlignCenter);
    scrollLayout->addWidget(nhanPreviewHoTen);

    // FINALIZE SCROLL
    scrollContent->setLayout(scrollLayout);
    scrollArea->setWidget(scrollContent);
    boTriChinh->addWidget(scrollArea);
    
    // Buttons
    int buttonHeight = ResponsiveHelper::getScaledHeight(45, screenSize);
    int buttonWidth = ResponsiveHelper::getScaledWidth(130, screenSize);
    int buttonFont = ResponsiveHelper::getScaledFontSize(14, screenSize);
    
    QHBoxLayout *boTriNut = new QHBoxLayout();
    
    nutXoaForm = new QPushButton(LanguageManager::instance().getText("BTN_REFRESH"));
    nutXoaForm->setMinimumSize(buttonWidth, buttonHeight);
    nutXoaForm->setCursor(Qt::PointingHandCursor);
    nutXoaForm->setAutoDefault(false); // Enable enter navigation by disabling auto default on this button
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
    ).arg(buttonFont));
    connect(nutXoaForm, &QPushButton::clicked, this, &HopThoaiThemDocGia::xoaTatCaTruong);
    
    nutLuu = new QPushButton(LanguageManager::instance().getText("BTN_ADD_READER"));
    nutLuu->setMinimumSize(buttonWidth, buttonHeight);
    nutLuu->setCursor(Qt::PointingHandCursor);
    nutLuu->setDefault(true); 
    nutLuu->setAutoDefault(true);
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
    ).arg(buttonFont));
    connect(nutLuu, &QPushButton::clicked, this, &HopThoaiThemDocGia::xuLyNutLuu);
    
    nutHuy = new QPushButton(LanguageManager::instance().getText("BTN_CANCEL_DIALOG"));
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
        "QPushButton:hover { background-color: #323248; border-color: #f64e60; }"
        "QPushButton:pressed { background-color: #1b1b29; }"
    ).arg(buttonFont));
    connect(nutHuy, &QPushButton::clicked, this, &QDialog::reject);
    
    boTriNut->addWidget(nutXoaForm);
    boTriNut->addStretch();
    boTriNut->addWidget(nutLuu);
    boTriNut->addWidget(nutHuy);
    boTriChinh->addLayout(boTriNut);
    
    nutLuu->setEnabled(false);
}

void HopThoaiThemDocGia::taoMaDocGia() {
    extern PTRDG rootDocGia;
    // Tạo mã tự động: tìm max + 1
    // Sử dụng hàm taoDocGia sẽ tự động tạo mã
    // Để hiện thị preview, tìm max hiện tại
    int maxMa = 0;
    // Helper lambda để tìm max
    std::function<void(PTRDG)> timMax = [&](PTRDG node) {
        if (!node) return;
        if (node->data.MaThe > maxMa) maxMa = node->data.MaThe;
        timMax(node->left);
        timMax(node->right);
    };
    timMax(rootDocGia);
    maDuocTao = maxMa + 1;
    nhanMaThe->setText(QString::number(maDuocTao));
}

void HopThoaiThemDocGia::kiemTraDuLieu() {
    bool hopLe = !oNhapHo->text().trimmed().isEmpty() && 
                 !oNhapTen->text().trimmed().isEmpty() &&
                 !oNhapSDT->text().trimmed().isEmpty();
    nutLuu->setEnabled(hopLe);
    if (hopLe) {
        nhanTrangThai->setText(LanguageManager::instance().getText("MSG_READY_TO_SAVE"));
        nhanTrangThai->setStyleSheet("color: #78C2AD; font-size: 13px;");
    }
}

void HopThoaiThemDocGia::xuLyNutLuu() {
    if (oNhapHo->text().trimmed().isEmpty() || oNhapTen->text().trimmed().isEmpty() || oNhapSDT->text().trimmed().isEmpty()) {
        nhanTrangThai->setText("✕ " + LanguageManager::instance().getText("ERR_FILL_REQUIRED"));
        nhanTrangThai->setStyleSheet("color: #E06D6D; font-weight: bold;");
        return;
    }
    
    // Disable buttons
    nutLuu->setEnabled(false);
    nutHuy->setEnabled(false);
    nutXoaForm->setEnabled(false);
    nutLuu->setText(LanguageManager::instance().getText("BTN_PROCESSING"));
    
    nhanTrangThai->setText(LanguageManager::instance().getText("MSG_ADD_READER_PROCESSING"));
    nhanTrangThai->setStyleSheet("color: #FFA800; font-weight: bold;");
    QApplication::processEvents();
    
    extern PTRDG rootDocGia;
    
    std::string ho = oNhapHo->text().trimmed().toStdString();
    std::string ten = oNhapTen->text().trimmed().toStdString();
    
    // CHUẨN HÓA HỌA TÊN - Viết hoa chữ cái đầu, chuẩn hóa khoảng trắng
    std::string hoChuan, tenChuan;
    
    std::string loiHo = ChuanHoaTenUnicode(ho, 1, 50, hoChuan);
    if (!loiHo.empty()) {
        nhanTrangThai->setText("✕ " + QString::fromStdString(loiHo));
        nhanTrangThai->setStyleSheet("color: #E06D6D; font-weight: bold; font-size: 13px;");
        oNhapHo->setFocus();
        nutLuu->setEnabled(true);
        nutHuy->setEnabled(true);
        nutXoaForm->setEnabled(true);
        nutLuu->setText(LanguageManager::instance().getText("BTN_ADD_READER"));
        return;
    }
    
    std::string loiTen = ChuanHoaTenUnicode(ten, 1, 20, tenChuan);
    if (!loiTen.empty()) {
        nhanTrangThai->setText("✕ " + QString::fromStdString(loiTen));
        nhanTrangThai->setStyleSheet("color: #E06D6D; font-weight: bold; font-size: 13px;");
        oNhapTen->setFocus();
        nutLuu->setEnabled(true);
        nutHuy->setEnabled(true);
        nutXoaForm->setEnabled(true);
        nutLuu->setText(LanguageManager::instance().getText("BTN_ADD_READER"));
        return;
    }
    
    bool phai = (hopChonGioiTinh->currentIndex() == 1); // 0=Nam, 1=Nữ
    int trangThai = 1; // 1 = hoạt động
    
    // Gọi hàm backend tạo độc giả với dữ liệu đã chuẩn hóa
    PTRDG newReader = taoDocGia(hoChuan, tenChuan, phai, trangThai, rootDocGia);
    if (newReader) {
        // Phase 2: Set additional fields
        newReader->data.LoaiDocGia = hopChonLoaiDocGia->currentIndex();
        // Normalize auxiliary fields
        std::string lopChuan = ChuanHoaKhoangTrang(oNhapLop->text().trimmed().toStdString());
        // For class, typically Uppercase is preferred (e.g. 12A1)
        lopChuan = ChuyenInHoa(lopChuan);
        
        std::string emailChuan = ChuanHoaKhoangTrang(oNhapEmail->text().trimmed().toStdString());
        // Email should be lowercase
        emailChuan = ChuyenInThuong(emailChuan);
        
        std::string sdtChuan = LayChiCacKyTuSo(oNhapSDT->text().trimmed().toStdString());

        newReader->data.LoaiDocGia = hopChonLoaiDocGia->currentIndex();
        newReader->data.Lop = lopChuan;
        newReader->data.SDT = sdtChuan;
        newReader->data.Email = emailChuan;
        newReader->data.NgaySinh = oNhapNgaySinh->date().toString("dd/MM/yyyy").toStdString();
        
        InsertDocGia(rootDocGia, newReader);
        // Dữ liệu đã được đồng bộ SQL bên trong backend
        // Global notification
        EventBus::instance().docGiaAdded(newReader->data.MaThe);
        
        // Auto-Print Card removed per user request (annoying)
        // PrintHelper::inTheDocGiaTuDong(newReader, this);
    }
    
    nhanTrangThai->setText(LanguageManager::instance().getText("MSG_ADD_READER_SUCCESS_LBL")
                          .arg(QString::fromStdString(hoChuan))
                          .arg(QString::fromStdString(tenChuan)));
    nhanTrangThai->setStyleSheet("color: #78C2AD; font-weight: bold; font-size: 13px;");
    
    emit docGiaDaThem();
    
    // Show Toast
    ToastNotification::show(LanguageManager::instance().getText("MSG_ADD_READER_SUCCESS_VAR")
                           .arg(QString::fromStdString(hoChuan))
                           .arg(QString::fromStdString(tenChuan)), ToastNotification::SUCCESS);
    
    accept(); // Exit immediately
    // QTimer removed
}

void HopThoaiThemDocGia::capNhatPreviewHoTen() {
    std::string ho = oNhapHo->text().trimmed().toStdString();
    std::string ten = oNhapTen->text().trimmed().toStdString();
    std::string hoChuan, tenChuan;
    // Use ChuanHoaTenUnicode for validation and normalization
    ChuanHoaTenUnicode(ho, 1, 50, hoChuan);
    ChuanHoaTenUnicode(ten, 1, 20, tenChuan);
    std::string full = hoChuan + " " + tenChuan;
    nhanPreviewHoTen->setText(QString::fromStdString(full));
}

void HopThoaiThemDocGia::xoaTatCaTruong() {
    oNhapHo->clear();
    oNhapTen->clear();
    oNhapLop->clear();
    oNhapSDT->clear();
    oNhapEmail->clear();
    hopChonGioiTinh->setCurrentIndex(0);
    hopChonLoaiDocGia->setCurrentIndex(0);
    oNhapNgaySinh->setDate(QDate::currentDate().addYears(-15));
    nhanTrangThai->clear();
    taoMaDocGia();
    oNhapHo->setFocus();
}
