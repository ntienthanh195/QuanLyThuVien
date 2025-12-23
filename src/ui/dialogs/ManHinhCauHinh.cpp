/**
 * @file ManHinhCauHinh.cpp
 * @path src/views/dialogs/ManHinhCauHinh.cpp
 * @description Giao diện cài đặt (Configuration/Settings) của hệ thống.
 * Cho phép chỉnh sửa quy định thư viện: tiền phạt, số ngày mượn tối đa, số sách tối đa...
 * @project Library Management System (QuanLyThuVien)
 */
#include "ui/dialogs/ManHinhCauHinh.h"

#include "utils/AuditLogger.h"
#include "utils/PrintHelper.h"
#include "utils/SettingsManager.h"
#include "utils/LanguageManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QScrollArea>
#include <QFileDialog>
#include <QPrinterInfo>
#include <QSettings>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDebug>
#include "models/QuanLySach.h" // For NapDanhSachTuDB
#include "utils/EventBus.h" // For notifications

ManHinhCauHinh::ManHinhCauHinh(QWidget *parent, DisplayMode mode) 
    : QDialog(parent), currentMode(mode) {
    setWindowTitle(LanguageManager::instance().getText("TITLE_SETTINGS"));
    setMinimumSize(650, 600); // Increased size to show all content
    resize(700, 650); // Comfortable default size
    
    // Center dialog on parent or screen
    if (parent) {
        move(parent->window()->frameGeometry().center() - rect().center());
    }
    
    setModal(true); // Ensure it blocks parent interaction
    
    thietLapGiaoDien();
    taiCauHinh();
}

#include <QFormLayout>
#include <QFrame>

// Helper to create a styled section header
static QLabel* createSectionHeader(QString text) {
    QLabel *lbl = new QLabel(text);
    lbl->setStyleSheet("color: #5D9CEC; font-size: 14px; font-weight: bold; text-transform: uppercase; margin-top: 10px; margin-bottom: 5px;");
    return lbl;
}

void ManHinhCauHinh::thietLapGiaoDien() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15); // Increased spacing
    mainLayout->setContentsMargins(0, 0, 0, 0); // Remove margins for scroll area
    
    // Create scroll area to handle overflow
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");
    
    QWidget *scrollWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(scrollWidget);
    contentLayout->setSpacing(15);
    contentLayout->setContentsMargins(25, 25, 25, 25);

    // Global Style
    setStyleSheet("QDialog { background-color: #1e1e2d; color: #E0E0E0; font-family: 'Segoe UI'; font-size: 14px; }"
                  "QFrame#Card { background-color: #2b2b40; border-radius: 8px; border: 1px solid #3E3E42; }"
                  "QLabel { color: #E0E0E0; min-height: 25px; }"
                  "QLineEdit { padding: 5px; border: 1px solid #3E3E42; border-radius: 4px; background: #1e1e2d; color: #E0E0E0; min-height: 30px; }"
                  "QLineEdit:focus { border: 1px solid #5D9CEC; }"
                  "QComboBox { padding: 5px; border: 1px solid #3E3E42; border-radius: 4px; background: #1e1e2d; color: #E0E0E0; min-height: 30px; }"
                  "QCheckBox { padding: 5px; color: #E0E0E0; spacing: 8px; }"
                  "QCheckBox::indicator { width: 20px; height: 20px; }"
                  "QPushButton { background-color: #3E3E42; color: white; border: none; padding: 8px 15px; border-radius: 4px; min-width: 80px; }"
                  "QPushButton:hover { background-color: #555; }");

    // ==========================================
    // SECTION 1: LIBRARY INFO
    // ==========================================
    contentLayout->addWidget(createSectionHeader(LanguageManager::instance().getText("HEADER_LIBRARY_INFO")));
    
    QFrame *cardInfo = new QFrame();
    cardInfo->setObjectName("Card");
    QFormLayout *layoutInfo = new QFormLayout(cardInfo);
    layoutInfo->setContentsMargins(20, 20, 20, 20);
    layoutInfo->setVerticalSpacing(15);
    layoutInfo->setHorizontalSpacing(20);
    
    txtTenThuVien = new QLineEdit();
    txtTenThuVien->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_LIB_NAME"));
    layoutInfo->addRow(LanguageManager::instance().getText("LBL_LIB_NAME"), txtTenThuVien);
    
    txtTenVietTat = new QLineEdit();
    txtTenVietTat->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_LIB_SHORT_NAME"));
    layoutInfo->addRow(LanguageManager::instance().getText("LBL_LIB_SHORT_NAME"), txtTenVietTat);
    
    QHBoxLayout *hBoxChecks = new QHBoxLayout();
    checkColor = new QCheckBox(LanguageManager::instance().getText("LBL_PRINT_COLOR"));
    checkHienThiLogo = new QCheckBox(LanguageManager::instance().getText("LBL_SHOW_LOGO"));
    hBoxChecks->addWidget(checkColor);
    hBoxChecks->addWidget(checkHienThiLogo);
    hBoxChecks->addStretch();
    layoutInfo->addRow("", hBoxChecks);
    
    txtLogoPath = new QLineEdit();
    QPushButton *btnBrowseLogo = new QPushButton(LanguageManager::instance().getText("BTN_BROWSE"));
    connect(btnBrowseLogo, &QPushButton::clicked, this, &ManHinhCauHinh::chonLogo);
    QHBoxLayout *hBoxLogo = new QHBoxLayout();
    hBoxLogo->addWidget(txtLogoPath);
    hBoxLogo->addWidget(btnBrowseLogo);
    layoutInfo->addRow(LanguageManager::instance().getText("LBL_LOGO"), hBoxLogo);
    
    contentLayout->addWidget(cardInfo);

    // ==========================================
    // SECTION 2: PRINTERS
    // ==========================================
    contentLayout->addWidget(createSectionHeader(LanguageManager::instance().getText("HEADER_PRINTER_CONFIG")));
    
    QFrame *cardPrint = new QFrame();
    cardPrint->setObjectName("Card");
    QFormLayout *layoutPrint = new QFormLayout(cardPrint);
    layoutPrint->setContentsMargins(20, 20, 20, 20);
    layoutPrint->setVerticalSpacing(15);
    layoutPrint->setHorizontalSpacing(20);
    
    comboPrinterReceipt = new QComboBox();
    layoutPrint->addRow(LanguageManager::instance().getText("LBL_PRINTER_RECEIPT"), comboPrinterReceipt);
    
    comboPrinterReport = new QComboBox();
    layoutPrint->addRow(LanguageManager::instance().getText("LBL_PRINTER_REPORT"), comboPrinterReport);
    
    checkSilentPrint = new QCheckBox(LanguageManager::instance().getText("LBL_SILENT_PRINT"), this);
    layoutPrint->addRow("", checkSilentPrint);
    
    contentLayout->addWidget(cardPrint);

    // ==========================================
    // SECTION 2.5: BOOK & BARCODE SETTINGS (NEW)
    // ==========================================
    contentLayout->addWidget(createSectionHeader(LanguageManager::instance().getText("HEADER_BARCODE_CONFIG")));

    QFrame *cardBarcode = new QFrame();
    cardBarcode->setObjectName("Card");
    QFormLayout *layoutBarcode = new QFormLayout(cardBarcode);
    layoutBarcode->setContentsMargins(20, 20, 20, 20);
    layoutBarcode->setVerticalSpacing(15);
    layoutBarcode->setHorizontalSpacing(20);

    // Barcode Dimensions
    spinBarcodeWidth = new QSpinBox();
    spinBarcodeWidth->setRange(20, 100);
    spinBarcodeWidth->setSuffix(" mm");
    layoutBarcode->addRow(LanguageManager::instance().getText("LBL_BARCODE_WIDTH"), spinBarcodeWidth);
    
    spinBarcodeHeight = new QSpinBox();
    spinBarcodeHeight->setRange(5, 50);
    spinBarcodeHeight->setSuffix(" mm");
    layoutBarcode->addRow(LanguageManager::instance().getText("LBL_BARCODE_HEIGHT"), spinBarcodeHeight);

    // Checkboxes for Label Content
    checkShowPrice = new QCheckBox(LanguageManager::instance().getText("LBL_SHOW_PRICE"));
    checkShowHeader = new QCheckBox(LanguageManager::instance().getText("LBL_SHOW_HEADER"));
    checkShowTitle = new QCheckBox(LanguageManager::instance().getText("LBL_SHOW_TITLE"));
    
    layoutBarcode->addRow("", checkShowPrice);
    layoutBarcode->addRow("", checkShowHeader);
    layoutBarcode->addRow("", checkShowTitle);

    contentLayout->addWidget(cardBarcode);

    // ==========================================
    // SECTION 3: PATHS
    // ==========================================
    contentLayout->addWidget(createSectionHeader(LanguageManager::instance().getText("HEADER_STORAGE_PATHS")));
    
    QFrame *cardPaths = new QFrame();
    cardPaths->setObjectName("Card");
    QFormLayout *layoutPaths = new QFormLayout(cardPaths);
    layoutPaths->setContentsMargins(20, 20, 20, 20);
    layoutPaths->setVerticalSpacing(15);
    layoutPaths->setHorizontalSpacing(20);
    
    txtQrPath = new QLineEdit();
    QPushButton *btnBrowseQR = new QPushButton(LanguageManager::instance().getText("BTN_BROWSE"));
    connect(btnBrowseQR, &QPushButton::clicked, this, &ManHinhCauHinh::chonThuMucQR);
    QHBoxLayout *hBoxQR = new QHBoxLayout();
    hBoxQR->addWidget(txtQrPath);
    hBoxQR->addWidget(btnBrowseQR);
    layoutPaths->addRow(LanguageManager::instance().getText("LBL_QR_DIR"), hBoxQR);
    
    txtPdfPath = new QLineEdit();
    QPushButton *btnBrowsePDF = new QPushButton(LanguageManager::instance().getText("BTN_BROWSE"));
    connect(btnBrowsePDF, &QPushButton::clicked, this, &ManHinhCauHinh::chonThuMucPDF);
    QHBoxLayout *hBoxPDF = new QHBoxLayout();
    hBoxPDF->addWidget(txtPdfPath);
    hBoxPDF->addWidget(btnBrowsePDF);
    layoutPaths->addRow(LanguageManager::instance().getText("LBL_PDF_DIR"), hBoxPDF);
    
    contentLayout->addWidget(cardPaths);

    // ==========================================
    // SECTION 4: LOAN RULES
    // ==========================================
    contentLayout->addWidget(createSectionHeader(LanguageManager::instance().getText("HEADER_LOAN_RULES")));
    
    QFrame *cardRules = new QFrame();
    cardRules->setObjectName("Card");
    QFormLayout *layoutRules = new QFormLayout(cardRules);
    layoutRules->setContentsMargins(20, 20, 20, 20);
    layoutRules->setVerticalSpacing(15);
    layoutRules->setHorizontalSpacing(20);
    
    // 1. Thoi Han Muon
    spinHanMuon = new QSpinBox();
    spinHanMuon->setRange(1, 365);
    spinHanMuon->setSuffix(LanguageManager::instance().getText("SUFFIX_DAYS"));
    layoutRules->addRow(LanguageManager::instance().getText("LBL_LOAN_DAYS"), spinHanMuon);
    
    // 2. Tien Phat
    spinTienPhat = new QSpinBox();
    spinTienPhat->setRange(0, 1000000);
    spinTienPhat->setSingleStep(1000);
    spinTienPhat->setSuffix(LanguageManager::instance().getText("SUFFIX_MONEY_DAY"));
    layoutRules->addRow(LanguageManager::instance().getText("LBL_FINE_PER_DAY"), spinTienPhat);
    
    // 3. So Lan Gia Han
    spinSoLanGiaHan = new QSpinBox();
    spinSoLanGiaHan->setRange(0, 5);
    spinSoLanGiaHan->setSuffix(LanguageManager::instance().getText("SUFFIX_TIMES"));
    layoutRules->addRow(LanguageManager::instance().getText("LBL_MAX_EXTENSIONS"), spinSoLanGiaHan);
    
    // Only add Loan Rules section if in Full mode
    if (currentMode == DisplayMode::Full) {
        contentLayout->addWidget(createSectionHeader(LanguageManager::instance().getText("HEADER_LOAN_RULES")));
        contentLayout->addWidget(cardRules);
    } else {
        delete cardRules; // Clean up
    }
    
    contentLayout->addStretch(); // Push content up
    
    // Finalize scroll area
    scrollArea->setWidget(scrollWidget);
    mainLayout->addWidget(scrollArea);
    
    // ==========================================
    // FOOTER BUTTONS
    // ==========================================
    mainLayout->addSpacing(10);
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnSave = new QPushButton(LanguageManager::instance().getText("BTN_SAVE_CONFIG"), this);
    btnSave->setCursor(Qt::PointingHandCursor);
    btnSave->setStyleSheet("QPushButton { background-color: #1877f2; color: white; padding: 10px 25px; font-weight: bold; border-radius: 6px; } QPushButton:hover { background-color: #166fe5; }");
    
    QPushButton *btnCancel = new QPushButton(LanguageManager::instance().getText("BTN_CLOSE"), this);
    btnCancel->setCursor(Qt::PointingHandCursor);
    btnCancel->setStyleSheet("QPushButton { background-color: #444; color: white; padding: 10px 25px; border-radius: 6px; } QPushButton:hover { background-color: #555; }");
    
    connect(btnSave, &QPushButton::clicked, this, &ManHinhCauHinh::luuCauHinh);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::accept);
    
    btnLayout->addStretch();
    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnSave);
    mainLayout->addLayout(btnLayout); // Correctly add buttons to main layout
    
    
    // Populate Printers
    QStringList printers = QPrinterInfo::availablePrinterNames();
    comboPrinterReceipt->addItem(LanguageManager::instance().getText("COMBO_SELECT_PRINTER"));
    comboPrinterReport->addItem(LanguageManager::instance().getText("COMBO_SELECT_PRINTER"));
    comboPrinterReceipt->addItems(printers);
    comboPrinterReport->addItems(printers);
}

void ManHinhCauHinh::taiCauHinh() {
    PrintHelper::PrintConfig cfg = PrintHelper::getCauHinh(); // Load from helper (cached or default)
    // Actually best to load from QSettings to ensure persistence, helper might just be static init
    QSettings settings("HVPG", "QuanLyThuVien");
    
    txtTenThuVien->setText(settings.value("Print/TenThuVien", cfg.tenThuVien).toString());
    txtTenVietTat->setText(settings.value("Print/TenVietTat", cfg.tenVietTat).toString());
    txtLogoPath->setText(settings.value("Print/LogoPath", cfg.duongDanLogo).toString());
    txtQrPath->setText(settings.value("Print/QrPath", cfg.qrOutputDir).toString());
    txtPdfPath->setText(settings.value("Print/PdfPath", cfg.pdfOutputDir).toString());
    
    checkColor->setChecked(settings.value("Print/Color", cfg.inMauSac).toBool());
    checkHienThiLogo->setChecked(settings.value("Print/ShowLogo", cfg.hienThiLogo).toBool());
    checkSilentPrint->setChecked(settings.value("Print/Silent", cfg.silentPrint).toBool());
    
    QString pReceipt = settings.value("Print/Receipt", cfg.printerReceipt).toString();
    QString pReport = settings.value("Print/Report", cfg.printerReport).toString();
    
    if (!pReceipt.isEmpty()) comboPrinterReceipt->setCurrentText(pReceipt);
    if (!pReport.isEmpty()) comboPrinterReport->setCurrentText(pReport);

    // Load Barcode Settings (2.5)
    spinBarcodeWidth->setValue(settings.value("Print/BarcodeWidth", cfg.labelWidth).toInt());
    spinBarcodeHeight->setValue(settings.value("Print/BarcodeHeight", cfg.labelHeight).toInt());
    checkShowPrice->setChecked(settings.value("Print/ShowPrice", cfg.showPrice).toBool());
    checkShowHeader->setChecked(settings.value("Print/ShowLibName", cfg.showLibName).toBool());
    checkShowTitle->setChecked(settings.value("Print/ShowBookTitle", cfg.showBookTitle).toBool());
    
    // Load Loan Rules from DB (SettingsManager)
    // ONLY IF in Full Mode (otherwise widgets don't exist)
    if (currentMode == DisplayMode::Full) {
        spinHanMuon->setValue(SettingsManager::instance().getInt("THOI_HAN_MUON", 14));
        spinTienPhat->setValue(SettingsManager::instance().getInt("TIEN_PHAT_MOI_NGAY", 5000));
        spinSoLanGiaHan->setValue(SettingsManager::instance().getInt("SO_LAN_GIA_HAN_TOI_DA", 1));
    }
}

void ManHinhCauHinh::luuCauHinh() {
    QSettings settings("HVPG", "QuanLyThuVien");
    
    settings.setValue("Print/TenThuVien", txtTenThuVien->text());
    settings.setValue("Print/TenVietTat", txtTenVietTat->text());
    settings.setValue("Print/Color", checkColor->isChecked());
    settings.setValue("Print/ShowLogo", checkHienThiLogo->isChecked());
    settings.setValue("Print/LogoPath", txtLogoPath->text());
    settings.setValue("Print/QrPath", txtQrPath->text());
    settings.setValue("Print/PdfPath", txtPdfPath->text());
    settings.setValue("Print/PdfPath", txtPdfPath->text());
    settings.setValue("Print/Silent", checkSilentPrint->isChecked());
    
    // Save Barcode Settings
    settings.setValue("Print/BarcodeWidth", spinBarcodeWidth->value());
    settings.setValue("Print/BarcodeHeight", spinBarcodeHeight->value());
    settings.setValue("Print/ShowPrice", checkShowPrice->isChecked());
    settings.setValue("Print/ShowLibName", checkShowHeader->isChecked());
    settings.setValue("Print/ShowBookTitle", checkShowTitle->isChecked());
    
    QString pReceipt = (comboPrinterReceipt->currentIndex() > 0) ? comboPrinterReceipt->currentText() : "";
    QString pReport = (comboPrinterReport->currentIndex() > 0) ? comboPrinterReport->currentText() : "";
    
    settings.setValue("Print/Receipt", pReceipt);
    settings.setValue("Print/Report", pReport);
    
    // Save Loan Rules to DB
    if (currentMode == DisplayMode::Full) {
        SettingsManager::instance().set("THOI_HAN_MUON", spinHanMuon->value(), "int");
        SettingsManager::instance().set("TIEN_PHAT_MOI_NGAY", spinTienPhat->value(), "int");
        SettingsManager::instance().set("SO_LAN_GIA_HAN_TOI_DA", spinSoLanGiaHan->value(), "int");
    }
    
    // Apply to PrintHelper
    PrintHelper::PrintConfig cfg;
    cfg.tenThuVien = txtTenThuVien->text();
    cfg.tenVietTat = txtTenVietTat->text();
    cfg.inMauSac = checkColor->isChecked();
    cfg.hienThiLogo = checkHienThiLogo->isChecked();
    cfg.duongDanLogo = txtLogoPath->text();
    cfg.qrOutputDir = txtQrPath->text();
    cfg.pdfOutputDir = txtPdfPath->text();
    cfg.silentPrint = checkSilentPrint->isChecked();
    
    // Apply Barcode Config to Helper
    cfg.labelWidth = spinBarcodeWidth->value();
    cfg.labelHeight = spinBarcodeHeight->value();
    cfg.showPrice = checkShowPrice->isChecked();
    cfg.showLibName = checkShowHeader->isChecked();
    cfg.showBookTitle = checkShowTitle->isChecked();
    cfg.printerReceipt = pReceipt;
    cfg.printerReport = pReport;
    
    PrintHelper::setCauHinh(cfg);
    
    // Log Activity
    AuditLogger::log(AuditLogger::ACTION_SETTINGS, "CAU_HINH", "", "Cập nhật cấu hình hệ thống (ManHinhCauHinh)");
    
    QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"), LanguageManager::instance().getText("MSG_SAVE_CONFIG_SUCCESS"));
    accept();
}

void ManHinhCauHinh::chonLogo() {
    QString path = QFileDialog::getOpenFileName(this, LanguageManager::instance().getText("LBL_LOGO"), "", "Images (*.png *.jpg *.jpeg)");
    if (!path.isEmpty()) txtLogoPath->setText(path);
}

void ManHinhCauHinh::chonThuMucQR() {
    QString path = QFileDialog::getExistingDirectory(this, LanguageManager::instance().getText("LBL_QR_DIR"));
    if (!path.isEmpty()) txtQrPath->setText(path);
}

void ManHinhCauHinh::chonThuMucPDF() {
    QString path = QFileDialog::getExistingDirectory(this, LanguageManager::instance().getText("LBL_PDF_DIR"));
    if (!path.isEmpty()) txtPdfPath->setText(path);
}

void ManHinhCauHinh::macDinh() {
    // defaults
}

void ManHinhCauHinh::xuLyChuanHoaMaSach() {
    QMessageBox::StandardButton reply = QMessageBox::question(this, LanguageManager::instance().getText("TITLE_CONFIRM"), 
        LanguageManager::instance().getText("MSG_CONFIRM_NORMALIZE"),
        QMessageBox::Yes | QMessageBox::No);
        
    if (reply != QMessageBox::Yes) return;
    
    QSqlQuery query;
    if (!query.exec("SELECT id, ma_sach FROM CuonSach")) {
        QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_ERROR"), "Query Error: " + query.lastError().text());
        return;
    }
    
    int processed = 0;
    int fixed = 0;
    
    QSqlDatabase::database().transaction();
    
    while (query.next()) {
        int id = query.value(0).toInt();
        QString oldMa = query.value(1).toString();
        std::string maStr = oldMa.toStdString();
        
        size_t lastDash = maStr.rfind('-');
        if (lastDash != std::string::npos) {
            std::string prefix = maStr.substr(0, lastDash);
            std::string suffix = maStr.substr(lastDash + 1);
            
            // Try parse suffix as int
            try {
                if (!suffix.empty() && std::all_of(suffix.begin(), suffix.end(), ::isdigit)) {
                    int num = std::stoi(suffix);
                    std::string newSuffix = std::to_string(num);
                    
                    if (newSuffix != suffix) {
                         std::string newMa = prefix + "-" + newSuffix;
                         QString qNewMa = QString::fromStdString(newMa);
                         
                         QSqlQuery updateQ;
                         updateQ.prepare("UPDATE CuonSach SET ma_sach = :newMa WHERE id = :id");
                         updateQ.bindValue(":newMa", qNewMa);
                         updateQ.bindValue(":id", id);
                         if (updateQ.exec()) {
                             fixed++;
                         } else {
                             qDebug() << "Update failed for ID" << id << updateQ.lastError().text();
                         }
                    }
                }
            } catch (...) {
                // Ignore parsing errors
            }
        }
        processed++;
    }
    
    if (fixed > 0) {
        if (QSqlDatabase::database().commit()) {
             // Reload Application Data
             NapDanhSachTuDB();
             
             // Notify Views
             EventBus::instance().dataChanged();
             
             QMessageBox::information(this, LanguageManager::instance().getText("TITLE_COMPLETE"), 
                 LanguageManager::instance().getText("MSG_NORMALIZE_COMPLETE").arg(processed).arg(fixed));
        } else {
             QSqlDatabase::database().rollback();
             QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_COMMIT_ERROR"), LanguageManager::instance().getText("MSG_COMMIT_FAILED"));
        }
    } else {
        QSqlDatabase::database().rollback(); // Nothing changed
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_COMPLETE"), LanguageManager::instance().getText("MSG_NORMALIZE_NO_CHANGE"));
    }
}

