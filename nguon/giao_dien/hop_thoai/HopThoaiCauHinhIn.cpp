#include "hop_thoai/HopThoaiCauHinhIn.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QApplication>
#include "utils/EventBus.h"

HopThoaiCauHinhIn::HopThoaiCauHinhIn(const PrintHelper::PrintConfig& cfg, QWidget* parent)
    : QDialog(parent), m_config(cfg) {
    setWindowTitle("Cấu Hình In Ấn");
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
    cbInMau = new QCheckBox("In màu sắc", this);
    cbHienThiLogo = new QCheckBox("Hiển thị logo", this);
    leDuongDanLogo = new QLineEdit(this);
    leQROutput = new QLineEdit(this);
    lePDFOutput = new QLineEdit(this);

    btnChonLogo = new QPushButton("Chọn...", this);
    btnChonQR = new QPushButton("Chọn...");
    btnChonPDF = new QPushButton("Chọn...");

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

    addRow("Tên thư viện:", leTenThuVien);
    addRow("Tên viết tắt:", leTenVietTat);
    main->addWidget(cbInMau);
    main->addWidget(cbHienThiLogo);
    addRow("Đường dẫn logo:", leDuongDanLogo, btnChonLogo);
    addRow("Thư mục QR (tương đối/absolute):", leQROutput, btnChonQR);
    addRow("Thư mục PDF (tương đối/absolute):", lePDFOutput, btnChonPDF);

    // Buttons
    QHBoxLayout* btns = new QHBoxLayout();
    btnLuu = new QPushButton("Lưu", this);
    btnHuy = new QPushButton("Hủy", this);
    connect(btnLuu, &QPushButton::clicked, this, [this]() {
        // Validate and accept
        m_config.tenThuVien = leTenThuVien->text();
        m_config.tenVietTat = leTenVietTat->text();
        m_config.inMauSac = cbInMau->isChecked();
        m_config.hienThiLogo = cbHienThiLogo->isChecked();
        m_config.duongDanLogo = leDuongDanLogo->text();
        m_config.qrOutputDir = leQROutput->text();
        m_config.pdfOutputDir = lePDFOutput->text();

        // Ensure output dirs exist or attempt to create
        QString pdfBase = m_config.pdfOutputDir.isEmpty() ?
            (QApplication::applicationDirPath() + "/" + m_config.duongDanThuMucIn + "/" + m_config.thuMucPdf) :
            (QDir(m_config.pdfOutputDir).isAbsolute() ? m_config.pdfOutputDir : QApplication::applicationDirPath() + "/" + m_config.pdfOutputDir);
        if (!QDir().mkpath(pdfBase)) {
            QMessageBox::warning(this, "Lỗi", "Không thể tạo thư mục PDF: " + pdfBase);
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
    leDuongDanLogo->setText(m_config.duongDanLogo);
    leQROutput->setText(m_config.qrOutputDir.isEmpty() ? m_config.duongDanThuMucIn + "/" + m_config.thuMucQR : m_config.qrOutputDir);
    lePDFOutput->setText(m_config.pdfOutputDir.isEmpty() ? m_config.duongDanThuMucIn + "/" + m_config.thuMucPdf : m_config.pdfOutputDir);
}

void HopThoaiCauHinhIn::chonLogo() {
    QString file = QFileDialog::getOpenFileName(this, "Chọn file logo", QApplication::applicationDirPath(), "Images (*.png *.jpg *.svg)");
    if (!file.isEmpty()) leDuongDanLogo->setText(file);
}

void HopThoaiCauHinhIn::chonThuMucQR() {
    QString dir = QFileDialog::getExistingDirectory(this, "Chọn thư mục lưu QR", QApplication::applicationDirPath());
    if (!dir.isEmpty()) leQROutput->setText(dir);
}

void HopThoaiCauHinhIn::chonThuMucPDF() {
    QString dir = QFileDialog::getExistingDirectory(this, "Chọn thư mục lưu PDF", QApplication::applicationDirPath());
    if (!dir.isEmpty()) lePDFOutput->setText(dir);
}
