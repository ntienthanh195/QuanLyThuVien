#include "giao_dien/hop_thoai/HopThoaiSuaDocGia.h"
#include "models/DocGia.h"
#include "utils/ResponsiveHelper.h"
#include "utils/XuLyChuoi.h"
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
    
    oNhapMaThe = new QLineEdit();
    oNhapMaThe->setReadOnly(true);
    oNhapMaThe->setStyleSheet("background-color: #1E1E2D; color: #6C6C80; padding: 8px; border-radius: 8px; font-weight: bold;");
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
    
    nutLuu = new QPushButton("Lưu");
    nutLuu->setMinimumSize(buttonWidth, buttonHeight);
    nutLuu->setStyleSheet("QPushButton { background-color: #1BC5BD; color: white; border: none; border-radius: 8px; font-weight: bold; } QPushButton:hover { background-color: #0F9A94; }");
    connect(nutLuu, &QPushButton::clicked, this, &HopThoaiSuaDocGia::xuLyNutLuu);
    
    nutHuy = new QPushButton("Hủy");
    nutHuy->setMinimumSize(buttonWidth, buttonHeight);
    nutHuy->setStyleSheet("QPushButton { background-color: #F64E60; color: white; border: none; border-radius: 8px; font-weight: bold; } QPushButton:hover { background-color: #D63447; }");
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
        nhanTrangThai->setStyleSheet("color: #F64E60; font-weight: bold; font-size: 13px;");
        oNhapHo->setFocus();
        nutLuu->setEnabled(true);
        nutHuy->setEnabled(true);
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
    
    nhanTrangThai->setText(QString("✓ Đã cập nhật: %1 %2")
                          .arg(QString::fromStdString(hoChuan))
                          .arg(QString::fromStdString(tenChuan)));
    nhanTrangThai->setStyleSheet("color: #1BC5BD; font-weight: bold; font-size: 13px;");
    
    emit docGiaDaSua();
    QTimer::singleShot(1000, this, &HopThoaiSuaDocGia::accept);
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
