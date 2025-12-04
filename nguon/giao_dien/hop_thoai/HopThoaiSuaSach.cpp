#include "giao_dien/hop_thoai/HopThoaiSuaSach.h"
#include "utils/FilePathManager.h"
#include "models/QuanLySach.h"
#include "models/ThaoTacFile.h"
#include "utils/ResponsiveHelper.h"
#include "utils/XuLyChuoi.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QDateTime>
#include <QTimer>

HopThoaiSuaSach::HopThoaiSuaSach(const QString& isbn, QWidget *cha)
    : QDialog(cha), isbnGoc(isbn), sachHienTai(nullptr) {
    setWindowTitle("Chỉnh Sửa Đầu Sách");
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    setMinimumWidth(ResponsiveHelper::getScaledWidth(600, screenSize));
    
    // Find book
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] && dsDauSach[i]->ISBN == isbn.toStdString()) {
            sachHienTai = dsDauSach[i];
            break;
        }
    }
    
    thieLapGiaoDien();
    taiDuLieuSach();
}

void HopThoaiSuaSach::thieLapGiaoDien() {
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    QVBoxLayout *boTriChinh = new QVBoxLayout(this);
    int margin = ResponsiveHelper::getScaledMargin(25, screenSize);
    boTriChinh->setContentsMargins(margin, margin, margin, margin);
    
    QLabel *tieuDe = new QLabel("✏ EDIT THÔNG TIN SÁCH");
    int titleSize = ResponsiveHelper::getScaledFontSize(20, screenSize);
    tieuDe->setStyleSheet(QString("font-size: %1px; font-weight: bold; color: #FFFFFF;").arg(titleSize));
    tieuDe->setAlignment(Qt::AlignCenter);
    boTriChinh->addWidget(tieuDe);
    
    QGroupBox *nhomForm = new QGroupBox("Thông Tin Sách", this);
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
    
    oNhapISBN = new QLineEdit();
    oNhapISBN->setReadOnly(true);
    oNhapISBN->setStyleSheet("background-color: #1E1E2D; color: #6C6C80; padding: 8px; border-radius: 8px;");
    QLabel *lblISBN = new QLabel("ISBN:");
    lblISBN->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    boTriForm->addRow(lblISBN, oNhapISBN);
    
    oNhapTenSach = new QLineEdit();
    oNhapTenSach->setStyleSheet(kieuTruong);
    connect(oNhapTenSach, &QLineEdit::textChanged, this, &HopThoaiSuaSach::kiemTraDuLieu);
    QLabel *lblTenSach = new QLabel("Tên sách *:");
    lblTenSach->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    boTriForm->addRow(lblTenSach, oNhapTenSach);
    
    oNhapTacGia = new QLineEdit();
    oNhapTacGia->setStyleSheet(kieuTruong);
    connect(oNhapTacGia, &QLineEdit::textChanged, this, &HopThoaiSuaSach::kiemTraDuLieu);
    QLabel *lblTacGia = new QLabel("Tác giả *:");
    lblTacGia->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    boTriForm->addRow(lblTacGia, oNhapTacGia);
    
    hopChonTheLoai = new QComboBox();
    hopChonTheLoai->setEditable(true);
    hopChonTheLoai->addItems({"Văn học", "Khoa học", "Công nghệ", "Lịch sử", "Triết học", "Kinh tế", "Tâm lý", "Nghệ thuật", "Thiếu nhi"});
    hopChonTheLoai->setStyleSheet(kieuTruong);
    QLabel *lblTheLoai = new QLabel("Thể loại *:");
    lblTheLoai->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    boTriForm->addRow(lblTheLoai, hopChonTheLoai);
    
    soNamXuatBan = new QSpinBox();
    soNamXuatBan->setRange(1000, QDateTime::currentDateTime().date().year());
    soNamXuatBan->setSuffix(" năm");
    soNamXuatBan->setStyleSheet(kieuTruong);
    QLabel *lblNam = new QLabel("Năm XB:");
    lblNam->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    boTriForm->addRow(lblNam, soNamXuatBan);
    
    soSoTrang = new QSpinBox();
    soSoTrang->setRange(1, 10000);
    soSoTrang->setSuffix(" trang");
    soSoTrang->setStyleSheet(kieuTruong);
    QLabel *lblSoTrang = new QLabel("Số trang:");
    lblSoTrang->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    boTriForm->addRow(lblSoTrang, soSoTrang);
    
    boTriChinh->addWidget(nhomForm);
    
    nhanTrangThai = new QLabel("");
    nhanTrangThai->setAlignment(Qt::AlignCenter);
    boTriChinh->addWidget(nhanTrangThai);
    
    QHBoxLayout *boTriNut = new QHBoxLayout();
    int buttonHeight = ResponsiveHelper::getScaledHeight(45, screenSize);
    int buttonWidth = ResponsiveHelper::getScaledWidth(130, screenSize);
    
    nutLuu = new QPushButton("Lưu");
    nutLuu->setMinimumSize(buttonWidth, buttonHeight);
    nutLuu->setStyleSheet("QPushButton { background-color: #1BC5BD; color: white; border: none; border-radius: 8px; font-weight: bold; } QPushButton:hover { background-color: #0F9A94; }");
    connect(nutLuu, &QPushButton::clicked, this, &HopThoaiSuaSach::xuLyNutLuu);
    
    nutHuy = new QPushButton("Hủy");
    nutHuy->setMinimumSize(buttonWidth, buttonHeight);
    nutHuy->setStyleSheet("QPushButton { background-color: #F64E60; color: white; border: none; border-radius: 8px; font-weight: bold; } QPushButton:hover { background-color: #D63447; }");
    connect(nutHuy, &QPushButton::clicked, this, &QDialog::reject);
    
    boTriNut->addStretch();
    boTriNut->addWidget(nutLuu);
    boTriNut->addWidget(nutHuy);
    boTriChinh->addLayout(boTriNut);
}

void HopThoaiSuaSach::taiDuLieuSach() {
    if (!sachHienTai) {
        QMessageBox::critical(this, "Lỗi", "Không tìm thấy sách!");
        reject();
        return;
    }
    
    oNhapISBN->setText(QString::fromStdString(sachHienTai->ISBN));
    oNhapTenSach->setText(QString::fromStdString(sachHienTai->tenSach));
    oNhapTacGia->setText(QString::fromStdString(sachHienTai->tacGia));
    hopChonTheLoai->setCurrentText(QString::fromStdString(sachHienTai->theLoai));
    soNamXuatBan->setValue(sachHienTai->namXuatBan);
    soSoTrang->setValue(sachHienTai->soTrang);
}

void HopThoaiSuaSach::kiemTraDuLieu() {
    bool hopLe = !oNhapTenSach->text().trimmed().isEmpty() && !oNhapTacGia->text().trimmed().isEmpty();
    nutLuu->setEnabled(hopLe);
}

void HopThoaiSuaSach::xuLyNutLuu() {
    if (!sachHienTai) return;
    
    // Disable buttons để tránh double-click
    nutLuu->setEnabled(false);
    nutHuy->setEnabled(false);
    nutLuu->setText("Đang xử lý...");
    
    // Hiển thị status
    nhanTrangThai->setText("⚙ Đang cập nhật...");
    nhanTrangThai->setStyleSheet("color: #FFA800; font-weight: bold;");
    QApplication::processEvents();
    
    std::string tenSach = oNhapTenSach->text().trimmed().toStdString();
    std::string tacGia = oNhapTacGia->text().trimmed().toStdString();
    std::string theLoai = hopChonTheLoai->currentText().trimmed().toStdString();
    int namXB = soNamXuatBan->value();
    int soTrang = soSoTrang->value();
    
    // CHUẨN HÓA DỮ LIỆU
    std::string tenChuan, tacGiaChuan, theLoaiChuan;
    
    std::string loiTen = ChuanHoaTenUnicode(tenSach, 1, 500, tenChuan);
    if (!loiTen.empty()) {
        nhanTrangThai->setText("✕ " + QString::fromStdString(loiTen));
        nhanTrangThai->setStyleSheet("color: #F64E60; font-weight: bold;");
        nutLuu->setEnabled(true);
        nutHuy->setEnabled(true);
        nutLuu->setText("Lưu");
        return;
    }
    
    std::string loiTacGia = ChuanHoaTenUnicode(tacGia, 1, 100, tacGiaChuan);
    if (!loiTacGia.empty()) {
        nhanTrangThai->setText("✕ " + QString::fromStdString(loiTacGia));
        nhanTrangThai->setStyleSheet("color: #F64E60; font-weight: bold;");
        nutLuu->setEnabled(true);
        nutHuy->setEnabled(true);
        nutLuu->setText("Lưu");
        return;
    }
    
    theLoaiChuan = ChuyenThanhTitleCase(ChuanHoaKhoangTrang(theLoai));
    
    // Sử dụng hàm backend CapNhatThongTinDauSach với dữ liệu đã chuẩn hóa
    std::string ketQua = CapNhatThongTinDauSach(
        sachHienTai->ISBN, tenChuan, soTrang, tacGiaChuan, namXB, theLoaiChuan
    );
    
    if (ketQua.find("Thành công") != std::string::npos || ketQua.find("thành công") != std::string::npos) {
        // Tự động lưu file
        extern PTRDS dsDauSach[];
        extern int soLuongDauSach;
        GhiDanhSachDauSach(FilePathManager::getDauSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
        GhiDanhMucSach(FilePathManager::getDanhMucSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
        
        nhanTrangThai->setText("✓ Đã cập nhật thành công!");
        nhanTrangThai->setStyleSheet("color: #1BC5BD; font-weight: bold; font-size: 13px;");
        emit sachDaSua();
        QTimer::singleShot(1000, this, &HopThoaiSuaSach::accept);
    } else {
        nhanTrangThai->setText("✕ " + QString::fromStdString(ketQua));
        nhanTrangThai->setStyleSheet("color: #F64E60; font-weight: bold; font-size: 13px;");
        nutLuu->setEnabled(true);
        nutHuy->setEnabled(true);
        nutLuu->setText("Lưu");
    }
}
