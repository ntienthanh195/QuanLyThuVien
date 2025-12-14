#include "hop_thoai/HopThoaiCauHinhIn.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QApplication>
#include "utils/EventBus.h"
#include "utils/LanguageManager.h"

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
    main->addWidget(cbInMau);
    main->addWidget(cbHienThiLogo);
    main->addWidget(cbSilentPrint);
    addRow(LanguageManager::instance().getText("LBL_LOGO_PATH"), leDuongDanLogo, btnChonLogo);
    addRow(LanguageManager::instance().getText("LBL_QR_OUTPUT_DIR"), leQROutput, btnChonQR);
    addRow(LanguageManager::instance().getText("LBL_PDF_OUTPUT_DIR"), lePDFOutput, btnChonPDF);

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
