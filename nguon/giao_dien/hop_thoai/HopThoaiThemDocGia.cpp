#include "giao_dien/hop_thoai/HopThoaiThemDocGia.h"
#include "models/DocGia.h"
#include "utils/ResponsiveHelper.h"
#include "utils/XuLyChuoi.h"
#include "utils/EventBus.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QDateTime>
#include <QTimer>
#include <functional>

HopThoaiThemDocGia::HopThoaiThemDocGia(QWidget *cha) : QDialog(cha), maDuocTao(0) {
    setWindowTitle("Thêm Độc Giả Mới");
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    setMinimumWidth(ResponsiveHelper::getScaledWidth(550, screenSize));
    
    thieLapGiaoDien();
    taoMaDocGia();
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
        "   background-color: #1E1E2D;"
        "   border: 2px solid #3699FF;"
        "   border-radius: 12px;"
        "   margin-top: 12px;"
        "   padding-top: 18px;"
        "   font-weight: bold;"
        "   font-size: %1px;"
        "   color: #FFFFFF;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top left;"
        "   left: 12px;"
        "   top: -12px;"
        "   padding: 0 8px;"
        "   color: #3699FF;"
        "   background-color: #1E1E2D;"
        "   font-size: 12px;"
        "}"
    ).arg(fontSize));
    QVBoxLayout *boTriMa = new QVBoxLayout(nhomMa);
    
    nhanMaThe = new QLabel();
    nhanMaThe->setAlignment(Qt::AlignCenter);
    int maSize = ResponsiveHelper::getScaledFontSize(28, screenSize);
    nhanMaThe->setStyleSheet(QString("font-size: %1px; font-weight: bold; color: #1BC5BD; padding: 15px; background-color: #252534; border-radius: 8px;").arg(maSize));
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
        "   background-color: #1E1E2D;"
        "   border: 2px solid #3699FF;"
        "   border-radius: 12px;"
        "   margin-top: 12px;"
        "   padding-top: 18px;"
        "   font-weight: bold;"
        "   font-size: %1px;"
        "   color: #FFFFFF;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top left;"
        "   left: 12px;"
        "   top: -12px;"
        "   padding: 0 8px;"
        "   color: #3699FF;"
        "   background-color: #1E1E2D;"
        "   font-size: 12px;"
        "}"
    ).arg(fontSize));
    QFormLayout *boTriForm = new QFormLayout(nhomForm);
    boTriForm->setHorizontalSpacing(20);
    boTriForm->setVerticalSpacing(12);
    boTriForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    int fieldHeight = ResponsiveHelper::getScaledHeight(40, screenSize);
    QString kieuTruong = QString("padding: 8px; border: 2px solid #2B2B40; border-radius: 8px; font-size: %1px; background-color: #252534; color: #FFFFFF; min-height: %2px;").arg(fontSize).arg(fieldHeight);
    
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
    
    nutXoaForm = new QPushButton("↺ Làm Mới");
    nutXoaForm->setMinimumSize(buttonWidth, buttonHeight);
    nutXoaForm->setStyleSheet(QString("QPushButton { background-color: #6C6C80; color: white; border: none; border-radius: 8px; font-size: %1px; font-weight: bold; } QPushButton:hover { background-color: #5A5A70; }").arg(buttonFont));
    connect(nutXoaForm, &QPushButton::clicked, this, &HopThoaiThemDocGia::xoaTatCaTruong);
    
    nutLuu = new QPushButton("Lưu");
    nutLuu->setMinimumSize(buttonWidth, buttonHeight);
    nutLuu->setStyleSheet(QString("QPushButton { background-color: #1BC5BD; color: white; border: none; border-radius: 8px; font-size: %1px; font-weight: bold; } QPushButton:hover { background-color: #0F9A94; } QPushButton:disabled { background-color: #2B2B40; color: #6C6C80; }").arg(buttonFont));
    connect(nutLuu, &QPushButton::clicked, this, &HopThoaiThemDocGia::xuLyNutLuu);
    
    nutHuy = new QPushButton("Hủy");
    nutHuy->setMinimumSize(buttonWidth, buttonHeight);
    nutHuy->setStyleSheet(QString("QPushButton { background-color: #F64E60; color: white; border: none; border-radius: 8px; font-size: %1px; font-weight: bold; } QPushButton:hover { background-color: #D63447; }").arg(buttonFont));
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
        nhanTrangThai->setStyleSheet("color: #1BC5BD; font-size: 13px;");
    }
}

void HopThoaiThemDocGia::xuLyNutLuu() {
    if (oNhapHo->text().trimmed().isEmpty() || oNhapTen->text().trimmed().isEmpty()) {
        nhanTrangThai->setText("✕ Vui lòng điền đầy đủ thông tin!");
        nhanTrangThai->setStyleSheet("color: #F64E60; font-weight: bold;");
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
        nhanTrangThai->setStyleSheet("color: #F64E60; font-weight: bold; font-size: 13px;");
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
        nhanTrangThai->setStyleSheet("color: #F64E60; font-weight: bold; font-size: 13px;");
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
        // Tự động lưu file ngay sau khi thêm
        saveDocGia(rootDocGia);
        // Global notification
        EventBus::instance().docGiaAdded(newReader->data.MaThe);
    }
    
    nhanTrangThai->setText(QString("✓ Đã thêm độc giả: %1 %2")
                          .arg(QString::fromStdString(hoChuan))
                          .arg(QString::fromStdString(tenChuan)));
    nhanTrangThai->setStyleSheet("color: #1BC5BD; font-weight: bold; font-size: 13px;");
    
    emit docGiaDaThem();
    QTimer::singleShot(1000, this, &HopThoaiThemDocGia::accept);
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
