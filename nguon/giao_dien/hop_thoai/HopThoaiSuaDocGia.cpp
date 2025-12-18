#include "giao_dien/hop_thoai/HopThoaiSuaDocGia.h"
#include "models/DocGia.h"
#include "utils/ResponsiveHelper.h"
#include "utils/XuLyChuoi.h"
#include "utils/EventBus.h"
#include "utils/ToastNotification.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QTimer>

HopThoaiSuaDocGia::HopThoaiSuaDocGia(int maThe, QWidget *cha)
    : QDialog(cha), maDocGia(maThe), docGiaHienTai(nullptr) {
    setWindowTitle("Chỉnh Sửa Độc Giả");
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    setMinimumWidth(ResponsiveHelper::getScaledWidth(550, screenSize));
    
    extern PTRDG rootDocGia;
    docGiaHienTai = timDocGia(rootDocGia, maThe);
    
    // Apply Deep Night Premium Theme
    this->setStyleSheet("QDialog { background-color: #1e1e2d; color: #ffffff; }");

    thieLapGiaoDien();
    taiDuLieuDocGia();
}

void HopThoaiSuaDocGia::thieLapGiaoDien() {
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    QVBoxLayout *boTriChinh = new QVBoxLayout(this);
    int margin = ResponsiveHelper::getScaledMargin(25, screenSize);
    boTriChinh->setContentsMargins(margin, margin, margin, margin);
    
    QLabel *tieuDe = new QLabel("👤 CHỈNH SỬA THÔNG TIN ĐỘC GIẢ");
    int titleSize = ResponsiveHelper::getScaledFontSize(20, screenSize);
    tieuDe->setStyleSheet(QString("font-size: %1px; font-weight: bold; color: #FFFFFF;").arg(titleSize));
    tieuDe->setAlignment(Qt::AlignCenter);
    boTriChinh->addWidget(tieuDe);
    
    QGroupBox *nhomForm = new QGroupBox("Thông Tin Độc Giả", this);
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
    
    oNhapMaThe = new QLineEdit();
    oNhapMaThe->setReadOnly(true);
    oNhapMaThe->setStyleSheet("background-color: #151521; color: #565674; padding: 0 10px; border: 1px solid #323248; border-radius: 6px; min-height: " + QString::number(fieldHeight) + "px;");
    QLabel *lblMaThe = new QLabel("Mã thẻ:");
    lblMaThe->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    boTriForm->addRow(lblMaThe, oNhapMaThe);
    
    oNhapHo = new QLineEdit();
    oNhapHo->setStyleSheet(kieuTruong);
    connect(oNhapHo, &QLineEdit::textChanged, this, &HopThoaiSuaDocGia::kiemTraDuLieu);
    connect(oNhapHo, &QLineEdit::textChanged, this, &HopThoaiSuaDocGia::capNhatPreviewHoTen);
    QLabel *lblHo = new QLabel("Họ đệm *:");
    lblHo->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    boTriForm->addRow(lblHo, oNhapHo);
    
    oNhapTen = new QLineEdit();
    oNhapTen->setStyleSheet(kieuTruong);
    connect(oNhapTen, &QLineEdit::textChanged, this, &HopThoaiSuaDocGia::kiemTraDuLieu);
    connect(oNhapTen, &QLineEdit::textChanged, this, &HopThoaiSuaDocGia::capNhatPreviewHoTen);
    QLabel *lblTen = new QLabel("Tên *:");
    lblTen->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    boTriForm->addRow(lblTen, oNhapTen);
    
    hopChonGioiTinh = new QComboBox();
    hopChonGioiTinh->addItem("Nam", 0);
    hopChonGioiTinh->addItem("Nữ", 1);
    hopChonGioiTinh->setStyleSheet(kieuTruong);
    QLabel *lblGioiTinh = new QLabel("Giới tính:");
    lblGioiTinh->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    boTriForm->addRow(lblGioiTinh, hopChonGioiTinh);
    
    hopChonTrangThai = new QComboBox();
    hopChonTrangThai->addItem("Hoạt động", 0);
    hopChonTrangThai->addItem("Khóa", 1);
    hopChonTrangThai->setStyleSheet(kieuTruong);
    QLabel *lblTrangThai = new QLabel("Trạng thái:");
    lblTrangThai->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    boTriForm->addRow(lblTrangThai, hopChonTrangThai);
    
    boTriChinh->addWidget(nhomForm);
    
    nhanSoSachDangMuon = new QLabel("");
    nhanSoSachDangMuon->setStyleSheet("color: #B5B5C3; padding: 10px; background-color: #252534; border-radius: 8px;");
    nhanSoSachDangMuon->setAlignment(Qt::AlignCenter);
    boTriChinh->addWidget(nhanSoSachDangMuon);
    
    nhanTrangThai = new QLabel("");
    nhanTrangThai->setAlignment(Qt::AlignCenter);
    boTriChinh->addWidget(nhanTrangThai);

    nhanPreviewHoTen = new QLabel("");
    nhanPreviewHoTen->setStyleSheet("color: #A1F0E8; font-weight: 600; padding: 6px; background: transparent;");
    nhanPreviewHoTen->setAlignment(Qt::AlignCenter);
    boTriChinh->addWidget(nhanPreviewHoTen);
    
    QHBoxLayout *boTriNut = new QHBoxLayout();
    int buttonHeight = ResponsiveHelper::getScaledHeight(45, screenSize);
    int buttonWidth = ResponsiveHelper::getScaledWidth(130, screenSize);
    
    nutLuu = new QPushButton("Lưu Thay Đổi");
    nutLuu->setMinimumSize(buttonWidth, buttonHeight);
    nutLuu->setCursor(Qt::PointingHandCursor);
    nutLuu->setStyleSheet(
        "QPushButton {"
        "   background-color: #5D9CEC;"
        "   color: #ffffff;"
        "   border: none;"
        "   border-radius: 6px;"
        "   font-weight: bold;"
        "   font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #4A89DC; }"
        "QPushButton:pressed { background-color: #3b6bbf; }"
    );
    connect(nutLuu, &QPushButton::clicked, this, &HopThoaiSuaDocGia::xuLyNutLuu);
    
    nutHuy = new QPushButton("Hủy");
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

void HopThoaiSuaDocGia::taiDuLieuDocGia() {
    if (!docGiaHienTai) {
        QMessageBox::critical(this, "Lỗi", "Không tìm thấy độc giả!");
        reject();
        return;
    }
    
    oNhapMaThe->setText(QString::number(docGiaHienTai->data.MaThe));
    oNhapHo->setText(QString::fromStdString(docGiaHienTai->data.Ho));
    oNhapTen->setText(QString::fromStdString(docGiaHienTai->data.Ten));
    hopChonGioiTinh->setCurrentIndex(docGiaHienTai->data.Phai ? 1 : 0);
    hopChonTrangThai->setCurrentIndex(docGiaHienTai->data.TrangThai);
    
    nhanSoSachDangMuon->setText(QString("➤ Số sách đang mượn: %1").arg(docGiaHienTai->data.soSachDangMuon));
}

void HopThoaiSuaDocGia::kiemTraDuLieu() {
    bool hopLe = !oNhapHo->text().trimmed().isEmpty() && !oNhapTen->text().trimmed().isEmpty();
    nutLuu->setEnabled(hopLe);
}

void HopThoaiSuaDocGia::xuLyNutLuu() {
    if (!docGiaHienTai) return;
    
    // Disable buttons
    nutLuu->setEnabled(false);
    nutHuy->setEnabled(false);
    nutLuu->setText("Đang xử lý...");
    
    nhanTrangThai->setText("⚙ Đang cập nhật...");
    nhanTrangThai->setStyleSheet("color: #FFA800; font-weight: bold;");
    QApplication::processEvents();
    
    std::string ho = oNhapHo->text().trimmed().toStdString();
    std::string ten = oNhapTen->text().trimmed().toStdString();
    
    // CHUẨN HÓA HỌA TÊN
    std::string hoChuan, tenChuan;
    
    std::string loiHo = ChuanHoaTenUnicode(ho, 1, 50, hoChuan);
    if (!loiHo.empty()) {
        nhanTrangThai->setText("✕ " + QString::fromStdString(loiHo));
        nhanTrangThai->setStyleSheet("color: #E06D6D; font-weight: bold; font-size: 13px;");
        oNhapHo->setFocus();
        nutLuu->setEnabled(true);
        nutHuy->setEnabled(true);
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
        nutLuu->setText("Lưu");
        return;
    }
    
    // Tạo struct mới với dữ liệu đã cập nhật và chuẩn hóa
    TheDocGia duLieuMoi = docGiaHienTai->data;
    duLieuMoi.Ho = hoChuan;
    duLieuMoi.Ten = tenChuan;
    duLieuMoi.Phai = (hopChonGioiTinh->currentIndex() == 1); // 0=Nam, 1=Nữ
    duLieuMoi.TrangThai = hopChonTrangThai->currentIndex();
    
    extern PTRDG rootDocGia;
    hieuChinhDocGia(rootDocGia, duLieuMoi);
    EventBus::instance().docGiaUpdated(duLieuMoi.MaThe); // REALTIME UPDATE
    
    nhanTrangThai->setText(QString("✓ Đã cập nhật: %1 %2")
                          .arg(QString::fromStdString(hoChuan))
                          .arg(QString::fromStdString(tenChuan)));
    nhanTrangThai->setStyleSheet("color: #78C2AD; font-weight: bold; font-size: 13px;");
    
    emit docGiaDaSua();
    
    // Show Toast
    ToastNotification::show(QString("Đã cập nhật độc giả '%1 %2' thành công!")
                           .arg(QString::fromStdString(hoChuan))
                           .arg(QString::fromStdString(tenChuan)), ToastNotification::SUCCESS);
                           
    accept(); // Exit immediately
    // QTimer removed
}

void HopThoaiSuaDocGia::capNhatPreviewHoTen() {
    std::string ho = oNhapHo->text().trimmed().toStdString();
    std::string ten = oNhapTen->text().trimmed().toStdString();
    std::string hoChuan, tenChuan;
    ChuanHoaTenUnicode(ho, 1, 50, hoChuan);
    ChuanHoaTenUnicode(ten, 1, 20, tenChuan);
    std::string full = hoChuan + " " + tenChuan;
    nhanPreviewHoTen->setText(QString::fromStdString(full));
}
