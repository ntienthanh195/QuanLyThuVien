#include "giao_dien/hop_thoai/HopThoaiThemDocGia.h"
#include "models/DocGia.h"
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
#include <QTimer>
#include <QTimer>
#include <functional>
#include <QKeyEvent>

HopThoaiThemDocGia::HopThoaiThemDocGia(QWidget *cha) : QDialog(cha), maDuocTao(0) {
    setWindowTitle("Thêm Độc Giả Mới");
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
    QLabel *tieuDe = new QLabel("THÊM ĐỘC GIẢ MỚI");
    tieuDe->setStyleSheet(QString("font-size: %1px; font-weight: bold; color: #FFFFFF; background: transparent;").arg(titleSize));
    tieuDe->setAlignment(Qt::AlignCenter);
    boTriChinh->addWidget(tieuDe);
    
    // Mã thẻ
    QGroupBox *nhomMa = new QGroupBox("Mã Thẻ Độc Giả", this);
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
    
    QLabel *chuThich = new QLabel("* Mã thẻ được tạo tự động");
    chuThich->setStyleSheet("color: #6C6C80; font-style: italic; font-size: 11px;");
    chuThich->setAlignment(Qt::AlignCenter);
    boTriMa->addWidget(chuThich);
    boTriChinh->addWidget(nhomMa);
    
    // Form
    QGroupBox *nhomForm = new QGroupBox("Thông Tin Độc Giả", this);
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
    oNhapHo->setPlaceholderText("Nhập họ đệm...");
    oNhapHo->setStyleSheet(kieuTruong);
    connect(oNhapHo, &QLineEdit::textChanged, this, &HopThoaiThemDocGia::kiemTraDuLieu);
    connect(oNhapHo, &QLineEdit::textChanged, this, &HopThoaiThemDocGia::capNhatPreviewHoTen);
    
    oNhapTen = new QLineEdit();
    oNhapTen->setPlaceholderText("Nhập tên...");
    oNhapTen->setStyleSheet(kieuTruong);
    connect(oNhapTen, &QLineEdit::textChanged, this, &HopThoaiThemDocGia::kiemTraDuLieu);
    connect(oNhapTen, &QLineEdit::textChanged, this, &HopThoaiThemDocGia::capNhatPreviewHoTen);
    
    hopChonGioiTinh = new QComboBox();
    hopChonGioiTinh->addItem("Nam", 0);
    hopChonGioiTinh->addItem("Nữ", 1);
    hopChonGioiTinh->setStyleSheet(kieuTruong);
    
    QLabel *nhanHo = new QLabel("Họ đệm *:");
    nhanHo->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    QLabel *nhanTen = new QLabel("Tên *:");
    nhanTen->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    QLabel *nhanGioiTinh = new QLabel("Giới tính:");
    nhanGioiTinh->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 12px; min-width: 100px;");
    
    boTriForm->addRow(nhanHo, oNhapHo);
    boTriForm->addRow(nhanTen, oNhapTen);
    boTriForm->addRow(nhanGioiTinh, hopChonGioiTinh);
    boTriChinh->addWidget(nhomForm);
    
    // Status
    nhanTrangThai = new QLabel("");
    nhanTrangThai->setAlignment(Qt::AlignCenter);
    nhanTrangThai->setStyleSheet("color: #B5B5C3; font-style: italic;");
    boTriChinh->addWidget(nhanTrangThai);

    // Preview normalized name
    nhanPreviewHoTen = new QLabel("");
    nhanPreviewHoTen->setStyleSheet("color: #A1F0E8; font-weight: 600; padding: 6px; background: transparent;");
    nhanPreviewHoTen->setAlignment(Qt::AlignCenter);
    boTriChinh->addWidget(nhanPreviewHoTen);
    
    // Buttons
    int buttonHeight = ResponsiveHelper::getScaledHeight(45, screenSize);
    int buttonWidth = ResponsiveHelper::getScaledWidth(130, screenSize);
    int buttonFont = ResponsiveHelper::getScaledFontSize(14, screenSize);
    
    QHBoxLayout *boTriNut = new QHBoxLayout();
    
    nutXoaForm = new QPushButton("Làm Mới");
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
    
    nutLuu = new QPushButton("Thêm Độc Giả");
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
    
    nutHuy = new QPushButton("Hủy");
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
    bool hopLe = !oNhapHo->text().trimmed().isEmpty() && !oNhapTen->text().trimmed().isEmpty();
    nutLuu->setEnabled(hopLe);
    if (hopLe) {
        nhanTrangThai->setText("✓ Sẵn sàng lưu");
        nhanTrangThai->setStyleSheet("color: #78C2AD; font-size: 13px;");
    }
}

void HopThoaiThemDocGia::xuLyNutLuu() {
    if (oNhapHo->text().trimmed().isEmpty() || oNhapTen->text().trimmed().isEmpty()) {
        nhanTrangThai->setText("✕ Vui lòng điền đầy đủ thông tin!");
        nhanTrangThai->setStyleSheet("color: #E06D6D; font-weight: bold;");
        return;
    }
    
    // Disable buttons
    nutLuu->setEnabled(false);
    nutHuy->setEnabled(false);
    nutXoaForm->setEnabled(false);
    nutLuu->setText("Đang xử lý...");
    
    nhanTrangThai->setText("⚙ Đang thêm độc giả...");
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
        nutLuu->setText("Lưu");
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
        nutLuu->setText("Lưu");
        return;
    }
    
    bool phai = (hopChonGioiTinh->currentIndex() == 1); // 0=Nam, 1=Nữ
    int trangThai = 1; // 1 = hoạt động
    
    // Gọi hàm backend tạo độc giả với dữ liệu đã chuẩn hóa
    PTRDG newReader = taoDocGia(hoChuan, tenChuan, phai, trangThai, rootDocGia);
    if (newReader) {
        InsertDocGia(rootDocGia, newReader);
        // Dữ liệu đã được đồng bộ SQL bên trong backend
        // Global notification
        EventBus::instance().docGiaAdded(newReader->data.MaThe);
        
        // Auto-Print Card (Silent or Prompt)
        PrintHelper::inTheDocGiaTuDong(newReader, this);
    }
    
    nhanTrangThai->setText(QString("✓ Đã thêm độc giả: %1 %2")
                          .arg(QString::fromStdString(hoChuan))
                          .arg(QString::fromStdString(tenChuan)));
    nhanTrangThai->setStyleSheet("color: #78C2AD; font-weight: bold; font-size: 13px;");
    
    emit docGiaDaThem();
    
    // Show Toast
    ToastNotification::show(QString("Đã thêm độc giả '%1 %2' thành công!")
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
    hopChonGioiTinh->setCurrentIndex(0);
    nhanTrangThai->clear();
    taoMaDocGia();
    oNhapHo->setFocus();
}
