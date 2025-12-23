#include "dialogs/HopThoaiCauHinhIn.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QApplication>
#include "utils/EventBus.h"
#include "utils/LanguageManager.h"
#include "utils/SettingsManager.h"

HopThoaiCauHinhIn::HopThoaiCauHinhIn(const PrintHelper::PrintConfig& cfg, QWidget* parent)
    : QDialog(parent), m_config(cfg) {
    setWindowTitle(LanguageManager::instance().getText("DIALOG_CONFIG_PRINT_TITLE"));
    setModal(true);
    setupUi();
    loadFromConfig();

    // Refresh UI if data changes elsewhere while dialog is open
    connect(&EventBus::instance(), &EventBus::dauSachAdded, this, [this](const QString&){ this->update(); });
    connect(&EventBus::instance(), &EventBus::docGiaAdded, this, [this](int){ this->update(); });
}

void HopThoaiCauHinhIn::setupUi() {
    QVBoxLayout* main = new QVBoxLayout(this);

    leTenThuVien = new QLineEdit(this);
    leTenVietTat = new QLineEdit(this);
    cbInMau = new QCheckBox(LanguageManager::instance().getText("CB_PRINT_COLOR"), this);
    cbHienThiLogo = new QCheckBox(LanguageManager::instance().getText("CB_SHOW_LOGO"), this);
    cbSilentPrint = new QCheckBox(LanguageManager::instance().getText("CB_SILENT_PRINT"), this);
    leDuongDanLogo = new QLineEdit(this);
    leQROutput = new QLineEdit(this);
    lePDFOutput = new QLineEdit(this);

    btnChonLogo = new QPushButton(LanguageManager::instance().getText("BTN_CHOOSE"), this);
    btnChonQR = new QPushButton(LanguageManager::instance().getText("BTN_CHOOSE"));
    btnChonPDF = new QPushButton(LanguageManager::instance().getText("BTN_CHOOSE"));

    connect(btnChonLogo, &QPushButton::clicked, this, &HopThoaiCauHinhIn::chonLogo);
    connect(btnChonQR, &QPushButton::clicked, this, &HopThoaiCauHinhIn::chonThuMucQR);
    connect(btnChonPDF, &QPushButton::clicked, this, &HopThoaiCauHinhIn::chonThuMucPDF);

    // Layout rows
    auto addRow = [&](const QString& label, QWidget* widget, QWidget* extra = nullptr){
        QHBoxLayout* row = new QHBoxLayout();
        row->addWidget(new QLabel(label, this));
        row->addWidget(widget, 1);
        if (extra) row->addWidget(extra);
        main->addLayout(row);
    };

    addRow(LanguageManager::instance().getText("LBL_CONFIG_LIB_NAME"), leTenThuVien);
    addRow(LanguageManager::instance().getText("LBL_CONFIG_LIB_ABBR"), leTenVietTat);
    
    QHBoxLayout *cbRow = new QHBoxLayout();
    cbRow->addWidget(cbInMau);
    cbRow->addWidget(cbHienThiLogo);
    cbRow->addWidget(cbSilentPrint);
    main->addLayout(cbRow);

    addRow(LanguageManager::instance().getText("LBL_LOGO_PATH"), leDuongDanLogo, btnChonLogo);
    addRow(LanguageManager::instance().getText("LBL_QR_OUTPUT_DIR"), leQROutput, btnChonQR);
    addRow(LanguageManager::instance().getText("LBL_PDF_OUTPUT_DIR"), lePDFOutput, btnChonPDF);

    // === BARCODE SETTINGS GROUP ===
    QGroupBox *gbBarcode = new QGroupBox(LanguageManager::instance().getText("LBL_CONFIG_BARCODE_TITLE"), this);
    gbBarcode->setStyleSheet("QGroupBox { background: #2b2b40; border-radius: 10px; padding: 15px; margin-top: 20px; }");
    QVBoxLayout *barcodeLayout = new QVBoxLayout(gbBarcode);
    
    // Row 1: Presets
    QHBoxLayout *h1 = new QHBoxLayout();
    h1->addWidget(new QLabel(LanguageManager::instance().getText("LBL_BARCODE_PRESETS")));
    cbPresets = new QComboBox(this);
    cbPresets->addItem(LanguageManager::instance().getText("PRESET_CUSTOM"));
    cbPresets->addItem("Mặc định thư viện - 38x20mm");
    cbPresets->addItem("Mẫu phổ thông - 37x12mm");
    cbPresets->addItem("Tiêu chuẩn (Gọn) - 35x15mm");
    cbPresets->addItem("Nhãn đơn lẻ");
    h1->addWidget(cbPresets, 1);
    barcodeLayout->addLayout(h1);

    // Row 2: W/H (in mm)
    QHBoxLayout *h2 = new QHBoxLayout();
    h2->addWidget(new QLabel(LanguageManager::instance().getText("LBL_BARCODE_WIDTH")));
    sbLabelWidth = new QSpinBox(this);
    sbLabelWidth->setRange(20, 80);  // mm (changed from px)
    sbLabelWidth->setSuffix(" mm");  // Show unit
    h2->addWidget(sbLabelWidth);
    
    h2->addSpacing(20);
    h2->addWidget(new QLabel(LanguageManager::instance().getText("LBL_BARCODE_HEIGHT")));
    sbLabelHeight = new QSpinBox(this);
    sbLabelHeight->setRange(15, 50);  // mm (changed from px)
    sbLabelHeight->setSuffix(" mm");  // Show unit
    h2->addWidget(sbLabelHeight);
    barcodeLayout->addLayout(h2);

    // Row 3: Cols & Checks
    QHBoxLayout *h3 = new QHBoxLayout();
    h3->addWidget(new QLabel(LanguageManager::instance().getText("LBL_BARCODE_COLS")));
    sbLabelCols = new QSpinBox(this);
    sbLabelCols->setRange(2, 6);  // Realistic range for A4
    h3->addWidget(sbLabelCols);
    
    cbShowLibName = new QCheckBox(LanguageManager::instance().getText("CB_SHOW_LIB_NAME"));
    cbShowBookTitle = new QCheckBox(LanguageManager::instance().getText("CB_SHOW_BOOK_TITLE"));
    h3->addWidget(cbShowLibName);
    h3->addWidget(cbShowBookTitle);
    barcodeLayout->addLayout(h3);

    QHBoxLayout *h4 = new QHBoxLayout();
    cbShowPrice = new QCheckBox(LanguageManager::instance().getText("CB_SHOW_PRICE"));
    cbShowBorder = new QCheckBox(LanguageManager::instance().getText("CB_SHOW_BORDER"));
    h4->addStretch();
    h4->addWidget(cbShowPrice);
    h4->addWidget(cbShowBorder);
    barcodeLayout->addLayout(h4);

    // Live Preview
    QLabel *lblPrevTitle = new QLabel(LanguageManager::instance().getText("LBL_LIVE_PREVIEW"));
    lblPrevTitle->setStyleSheet("font-weight: bold; margin-top: 10px;");
    barcodeLayout->addWidget(lblPrevTitle);

    lblPreview = new QLabel(this);
    lblPreview->setAlignment(Qt::AlignCenter);
    lblPreview->setStyleSheet("background: #1e1e2d; border: 1px dashed #555; padding: 10px; min-height: 120px;");
    barcodeLayout->addWidget(lblPreview);

    main->addWidget(gbBarcode);

    // Connect signals for auto-preview
    connect(sbLabelWidth, QOverload<int>::of(&QSpinBox::valueChanged), this, &HopThoaiCauHinhIn::updatePreview);
    connect(sbLabelHeight, QOverload<int>::of(&QSpinBox::valueChanged), this, &HopThoaiCauHinhIn::updatePreview);
    connect(sbLabelCols, QOverload<int>::of(&QSpinBox::valueChanged), this, &HopThoaiCauHinhIn::updatePreview);
    connect(cbShowLibName, &QCheckBox::toggled, this, &HopThoaiCauHinhIn::updatePreview);
    connect(cbShowBookTitle, &QCheckBox::toggled, this, &HopThoaiCauHinhIn::updatePreview);
    connect(cbShowPrice, &QCheckBox::toggled, this, &HopThoaiCauHinhIn::updatePreview);
    connect(cbShowBorder, &QCheckBox::toggled, this, &HopThoaiCauHinhIn::updatePreview);
    connect(cbPresets, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HopThoaiCauHinhIn::applyPreset);

    // Buttons
    QHBoxLayout* btns = new QHBoxLayout();
    btnLuu = new QPushButton(LanguageManager::instance().getText("BTN_SAVE"), this);
    btnHuy = new QPushButton(LanguageManager::instance().getText("BTN_CANCEL"), this);
    connect(btnLuu, &QPushButton::clicked, this, [this]() {
        // Validate and accept
        m_config.tenThuVien = leTenThuVien->text();
        m_config.tenVietTat = leTenVietTat->text();
        m_config.inMauSac = cbInMau->isChecked();
        m_config.hienThiLogo = cbHienThiLogo->isChecked();
        m_config.silentPrint = cbSilentPrint->isChecked();
        m_config.duongDanLogo = leDuongDanLogo->text();
        m_config.qrOutputDir = leQROutput->text();
        m_config.pdfOutputDir = lePDFOutput->text();
        
        m_config.labelWidth = sbLabelWidth->value();
        m_config.labelHeight = sbLabelHeight->value();
        m_config.labelCols = sbLabelCols->value();
        m_config.showLibName = cbShowLibName->isChecked();
        m_config.showBookTitle = cbShowBookTitle->isChecked();
        m_config.showPrice = cbShowPrice->isChecked();
        m_config.showBorder = cbShowBorder->isChecked();

        // Save to SettingsManager
        SettingsManager::instance().set("Print/TenThuVien", m_config.tenThuVien, "string");
        SettingsManager::instance().set("Print/TenVietTat", m_config.tenVietTat, "string");
        SettingsManager::instance().set("Print/Color", m_config.inMauSac, "bool");
        SettingsManager::instance().set("Print/ShowLogo", m_config.hienThiLogo, "bool"); // Fix: Save ShowLogo
        SettingsManager::instance().set("Print/Silent", m_config.silentPrint, "bool");
        SettingsManager::instance().set("Print/BarcodeWidth", m_config.labelWidth, "int");
        SettingsManager::instance().set("Print/BarcodeHeight", m_config.labelHeight, "int");
        SettingsManager::instance().set("Print/BarcodeCols", m_config.labelCols, "int");
        SettingsManager::instance().set("Print/ShowLibName", m_config.showLibName, "bool");
        SettingsManager::instance().set("Print/ShowBookTitle", m_config.showBookTitle, "bool");
        SettingsManager::instance().set("Print/ShowPrice", m_config.showPrice, "bool");
        SettingsManager::instance().set("Print/ShowBorder", m_config.showBorder, "bool");

        // Save Paths
        SettingsManager::instance().set("Print/LogoPath", m_config.duongDanLogo, "string");
        SettingsManager::instance().set("Print/QrPath", m_config.qrOutputDir, "string");
        SettingsManager::instance().set("Print/PdfPath", m_config.pdfOutputDir, "string");
        
        // ═══ SAVE NEW CONFIG FIELDS ═══
        // Label settings
        SettingsManager::instance().set("Print/LabelGapMm", m_config.labelGapMm, "int");
        
        // Card settings (if dialog has these fields in future)
        SettingsManager::instance().set("Print/CardCols", m_config.cardCols, "int");
        SettingsManager::instance().set("Print/CardWidth", m_config.cardWidth, "double");
        SettingsManager::instance().set("Print/CardHeight", m_config.cardHeight, "double");
        SettingsManager::instance().set("Print/CardGapMm", m_config.cardGapMm, "int");
        SettingsManager::instance().set("Print/CardValidityMonths", m_config.cardValidityMonths, "int");
        
        // Barcode settings
        SettingsManager::instance().set("Print/BarcodeDPI", m_config.barcodeDPI, "int");
        SettingsManager::instance().set("Print/BarcodeModuleWidthMm", m_config.barcodeModuleWidthMm, "double");
        SettingsManager::instance().set("Print/BarcodeQuietZoneMm", m_config.barcodeQuietZoneMm, "double");
        
        // Report settings
        SettingsManager::instance().set("Print/ReportShowSummary", m_config.reportShowSummary, "bool");
        SettingsManager::instance().set("Print/LoiChao", m_config.loiChao, "string");

        // Đồng bộ ngược lại PrintHelper toàn cục ngay lập tức
        PrintHelper::setCauHinh(m_config);
        PrintHelper::reloadCauHinh(); // CRITICAL: Force cache refresh from new settings

        // Ensure output dirs exist or attempt to create
        QString pdfBase = m_config.pdfOutputDir.isEmpty() ?
            (QApplication::applicationDirPath() + "/" + m_config.duongDanThuMucIn + "/" + m_config.thuMucPdf) :
            (QDir(m_config.pdfOutputDir).isAbsolute() ? m_config.pdfOutputDir : QApplication::applicationDirPath() + "/" + m_config.pdfOutputDir);
        if (!QDir().mkpath(pdfBase)) {
            QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_ERROR_SYSTEM"), LanguageManager::instance().getText("MSG_ERROR_CREATE_DIR") + pdfBase);
            return;
        }

        accept();
    });
    connect(btnHuy, &QPushButton::clicked, this, &QDialog::reject);
    btns->addStretch();
    btns->addWidget(btnLuu);
    btns->addWidget(btnHuy);
    main->addLayout(btns);
}

void HopThoaiCauHinhIn::loadFromConfig() {
    leTenThuVien->setText(m_config.tenThuVien);
    leTenVietTat->setText(m_config.tenVietTat);
    cbInMau->setChecked(m_config.inMauSac);
    cbHienThiLogo->setChecked(m_config.hienThiLogo);
    cbSilentPrint->setChecked(m_config.silentPrint);
    leDuongDanLogo->setText(m_config.duongDanLogo);
    leQROutput->setText(m_config.qrOutputDir.isEmpty() ? m_config.duongDanThuMucIn + "/" + m_config.thuMucQR : m_config.qrOutputDir);
    lePDFOutput->setText(m_config.pdfOutputDir.isEmpty() ? m_config.duongDanThuMucIn + "/" + m_config.thuMucPdf : m_config.pdfOutputDir);

    sbLabelWidth->setValue(m_config.labelWidth);
    sbLabelHeight->setValue(m_config.labelHeight);
    sbLabelCols->setValue(m_config.labelCols);
    cbShowLibName->setChecked(m_config.showLibName);
    cbShowBookTitle->setChecked(m_config.showBookTitle);
    cbShowPrice->setChecked(m_config.showPrice);
    cbShowBorder->setChecked(m_config.showBorder);
    
    updatePreview();
}

void HopThoaiCauHinhIn::updatePreview() {
    QString libName = leTenVietTat->text().isEmpty() ? leTenThuVien->text() : leTenVietTat->text();
    QString title = "Tên Sách Mẫu (Sample Book Title)";
    QString isbn = "9786040000644";
    
    // Scaling Factor: 1mm = 3px for visual preview (adjust as needed for visibility)
    const float scale = 3.0f;
    
    // GET VALUES IN MM from spinboxes (already mm!)
    int w_mm = sbLabelWidth->value();
    int h_mm = sbLabelHeight->value();
    
    int w_px = static_cast<int>(w_mm * scale);
    int h_px = static_cast<int>(h_mm * scale);
    
    QString borderStyle = cbShowBorder->isChecked() ? "1px solid #000" : "1px dashed #ccc";
    
    // Wrapper to center the label in the preview box
    QString html = "<div style='display: flex; justify-content: center; padding: 10px; background: #1e1e2d;'>";
    
    // The Label itself
    html += QString(
        "<div style='width:%1px; height:%2px; border:%3; background:#fff; color:#000; "
        "text-align:center; font-family:Arial; padding:%4px; box-sizing:border-box; "
        "display: flex; flex-direction: column; justify-content: space-between;'>"
    ).arg(w_px).arg(h_px).arg(borderStyle).arg(1 * scale);
    
    if (cbShowLibName->isChecked()) {
        html += QString("<div style='font-size:%1pt; font-weight:bold; border-bottom:1px solid #ddd; padding-bottom:1px;'>%2</div>")
            .arg(static_cast<int>(2.5 * scale)).arg(libName);
    }
    
    if (cbShowBookTitle->isChecked()) {
        html += QString("<div style='font-size:%1pt; font-weight:bold; overflow:hidden; margin: 1px 0;'>%2</div>")
            .arg(static_cast<int>(2.8 * scale)).arg(title);
    }
    
    // Calculate barcode height in mm (remaining space in label)
    double barcodeHeightMm = h_mm - 8.0; // Subtract header/footer space
    if (barcodeHeightMm < 5.0) barcodeHeightMm = 5.0; // Minimum height
    
    // Barcode Base64 - NOW USES MM AND DPI!
    QString base64 = PrintHelper::generateBarcodeBase64(isbn, barcodeHeightMm, 300);
    
    // Barcode Container
    html += QString(
        "  <div style='flex-grow: 1; display: flex; align-items: center; justify-content: center; overflow: hidden;'>"
        "    <img src='data:image/png;base64,%1' style='height: 100%; max-width: 95%; image-rendering: pixelated;' />"
        "  </div>"
    ).arg(base64);
    
    // ISBN Label
    html += QString("<div style='font-size:%1pt; font-weight: bold; letter-spacing: 1px;'>%2</div>")
        .arg(static_cast<int>(2.5 * scale)).arg(isbn);
    
    if (cbShowPrice->isChecked()) {
        html += QString("<div style='font-size:%1pt; color:red; font-weight:bold;'>150,000đ</div>")
            .arg(static_cast<int>(2.5 * scale));
    }
    
    html += "</div></div>";
    
    lblPreview->setText(html);
}

void HopThoaiCauHinhIn::applyPreset(int index) {
    if (index == 0) return; // Custom
    
    if (index == 1) { // Library Default
        sbLabelWidth->setValue(38);   
        sbLabelHeight->setValue(20);  
        sbLabelCols->setValue(4);
        cbShowBorder->setChecked(true);
    } else if (index == 2) { // Universal
        sbLabelWidth->setValue(37);   
        sbLabelHeight->setValue(12);  
        sbLabelCols->setValue(4);
        cbShowBorder->setChecked(true);
    } else if (index == 3) { // Standard Compact
        sbLabelWidth->setValue(35);   
        sbLabelHeight->setValue(15);  
        sbLabelCols->setValue(4);
        cbShowBorder->setChecked(true);
    } else if (index == 4) { // Single
        sbLabelWidth->setValue(30);   
        sbLabelHeight->setValue(10);  
        sbLabelCols->setValue(1);
        cbShowBorder->setChecked(false);
    }
    updatePreview();
}

void HopThoaiCauHinhIn::chonLogo() {
    QString file = QFileDialog::getOpenFileName(this, LanguageManager::instance().getText("DIALOG_SELECT_LOGO"), QApplication::applicationDirPath(), "Images (*.png *.jpg *.svg)");
    if (!file.isEmpty()) leDuongDanLogo->setText(file);
}

void HopThoaiCauHinhIn::chonThuMucQR() {
    QString dir = QFileDialog::getExistingDirectory(this, LanguageManager::instance().getText("DIALOG_SELECT_QR_DIR"), QApplication::applicationDirPath());
    if (!dir.isEmpty()) leQROutput->setText(dir);
}

void HopThoaiCauHinhIn::chonThuMucPDF() {
    QString dir = QFileDialog::getExistingDirectory(this, LanguageManager::instance().getText("DIALOG_SELECT_PDF_DIR"), QApplication::applicationDirPath());
    if (!dir.isEmpty()) lePDFOutput->setText(dir);
}
