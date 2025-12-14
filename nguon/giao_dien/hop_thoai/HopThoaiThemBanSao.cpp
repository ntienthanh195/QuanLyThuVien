#include "giao_dien/hop_thoai/HopThoaiThemBanSao.h"
#include "utils/LanguageManager.h"
#include "utils/FilePathManager.h"
#include "models/QuanLySach.h"
#include "models/ThaoTacFile.h"
#include "utils/ResponsiveHelper.h"
#include "utils/XuLyChuoi.h"
#include "utils/AnimationHelper.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QTimer>

HopThoaiThemBanSao::HopThoaiThemBanSao(const QString& isbn, QWidget *cha)
    : QDialog(cha), maISBN(isbn), sachHienTai(nullptr) {
    setWindowTitle(LanguageManager::instance().getText("DIALOG_ADD_COPY_TITLE"));
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    setMinimumWidth(ResponsiveHelper::getScaledWidth(500, screenSize));
    
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] && dsDauSach[i]->ISBN == isbn.toStdString()) {
            sachHienTai = dsDauSach[i];
            break;
        }
    }
    
    thieLapGiaoDien();
}

void HopThoaiThemBanSao::thieLapGiaoDien() {
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    QVBoxLayout *boTriChinh = new QVBoxLayout(this);
    int margin = ResponsiveHelper::getScaledMargin(25, screenSize);
    boTriChinh->setContentsMargins(margin, margin, margin, margin);
    
    QLabel *tieuDe = new QLabel(LanguageManager::instance().getText("HEADER_ADD_COPY"));
    int titleSize = ResponsiveHelper::getScaledFontSize(20, screenSize);
    tieuDe->setStyleSheet(QString("font-size: %1px; font-weight: bold; color: #FFFFFF;").arg(titleSize));
    tieuDe->setAlignment(Qt::AlignCenter);
    boTriChinh->addWidget(tieuDe);
    
    if (!sachHienTai) {
        QLabel *loi = new QLabel("✕ " + LanguageManager::instance().getText("MSG_BOOK_NOT_FOUND"));
        loi->setStyleSheet("color: #F64E60; font-size: 16px; padding: 20px;");
        loi->setAlignment(Qt::AlignCenter);
        boTriChinh->addWidget(loi);
        return;
    }
    
    QGroupBox *nhomThongTin = new QGroupBox(LanguageManager::instance().getText("GROUP_BOOK_INFO"), this);
    int fontSize = ResponsiveHelper::getScaledFontSize(14, screenSize);
    nhomThongTin->setStyleSheet(QString("QGroupBox { background-color: #1E1E2D; border: 2px solid #3699FF; border-radius: 12px; font-size: %1px; color: #FFFFFF; padding-top: 14px; margin-top: 10px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 12px; top: -10px; padding: 0 6px; color: #3699FF; font-size: 12px; background-color: #1E1E2D; }").arg(fontSize));
    QFormLayout *boTriThongTin = new QFormLayout(nhomThongTin);
    
    oNhapISBN = new QLineEdit(maISBN);
    oNhapISBN->setReadOnly(true);
    oNhapISBN->setStyleSheet("background-color: #1E1E2D; color: #6C6C80; padding: 8px; border-radius: 8px;");
    boTriThongTin->addRow(LanguageManager::instance().getText("LBL_ISBN") + ":", oNhapISBN);
    
    oNhapTenSach = new QLineEdit(QString::fromStdString(sachHienTai->tenSach));
    oNhapTenSach->setReadOnly(true);
    oNhapTenSach->setStyleSheet("background-color: #1E1E2D; color: #6C6C80; padding: 8px; border-radius: 8px;");
    boTriThongTin->addRow(LanguageManager::instance().getText("LBL_BOOK_NAME") + ":", oNhapTenSach);
    
    boTriChinh->addWidget(nhomThongTin);
    
    QGroupBox *nhomBanSao = new QGroupBox(LanguageManager::instance().getText("GROUP_ADD_NEW_COPY"), this);
    nhomBanSao->setStyleSheet(QString("QGroupBox { background-color: #1E1E2D; border: 2px solid #3699FF; border-radius: 12px; font-size: %1px; color: #FFFFFF; padding-top: 14px; margin-top: 10px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 12px; top: -10px; padding: 0 6px; color: #3699FF; font-size: 12px; background-color: #1E1E2D; }").arg(fontSize));
    QFormLayout *boTriBanSao = new QFormLayout(nhomBanSao);
    
    int fieldHeight = ResponsiveHelper::getScaledHeight(40, screenSize);
    QString kieuTruong = QString("padding: 8px; border: 2px solid #2B2B40; border-radius: 8px; font-size: %1px; background-color: #252534; color: #FFFFFF; min-height: %2px;").arg(fontSize).arg(fieldHeight);
    
    soSoLuong = new QSpinBox();
    soSoLuong->setRange(1, 100);
    soSoLuong->setValue(1);
    soSoLuong->setSuffix("");
    soSoLuong->setStyleSheet(kieuTruong);
    boTriBanSao->addRow(LanguageManager::instance().getText("LBL_QUANTITY") + ":", soSoLuong);
    
    oNhapViTri = new QLineEdit();
    oNhapViTri->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_LOCATION_EXAMPLE"));
    oNhapViTri->setStyleSheet(kieuTruong);
    boTriBanSao->addRow(LanguageManager::instance().getText("LBL_LOCATION") + ":", oNhapViTri);
    
    boTriChinh->addWidget(nhomBanSao);
    
    nhanTrangThai = new QLabel("");
    nhanTrangThai->setAlignment(Qt::AlignCenter);
    boTriChinh->addWidget(nhanTrangThai);
    
    QHBoxLayout *boTriNut = new QHBoxLayout();
    int buttonHeight = ResponsiveHelper::getScaledHeight(45, screenSize);
    int buttonWidth = ResponsiveHelper::getScaledWidth(130, screenSize);
    
    nutLuu = new QPushButton(LanguageManager::instance().getText("BTN_ADD_SYMBOL"));
    nutLuu->setMinimumSize(buttonWidth, buttonHeight);
    nutLuu->setStyleSheet("QPushButton { background-color: #1BC5BD; color: white; border: none; border-radius: 8px; font-weight: bold; } QPushButton:hover { background-color: #0F9A94; }");
    connect(nutLuu, &QPushButton::clicked, this, &HopThoaiThemBanSao::xuLyNutLuu);
    
    nutHuy = new QPushButton(LanguageManager::instance().getText("BTN_CANCEL"));
    nutHuy->setMinimumSize(buttonWidth, buttonHeight);
    nutHuy->setStyleSheet("QPushButton { background-color: #F64E60; color: white; border: none; border-radius: 8px; font-weight: bold; } QPushButton:hover { background-color: #D63447; }");
    connect(nutHuy, &QPushButton::clicked, this, &QDialog::reject);
    
    boTriNut->addStretch();
    boTriNut->addWidget(nutLuu);
    boTriNut->addWidget(nutHuy);
    boTriChinh->addLayout(boTriNut);
}

void HopThoaiThemBanSao::xuLyNutLuu() {
    if (!sachHienTai) return;
    
    // Disable nút để tránh double-click
    nutLuu->setEnabled(false);
    nutHuy->setEnabled(false);
    nutLuu->setText(LanguageManager::instance().getText("MSG_PROCESSING"));
    
    int soLuong = soSoLuong->value();
    std::string viTri = oNhapViTri->text().trimmed().toStdString();
    
    // CHUẨN HÓA VỊ TRÍ
    std::string viTriChuan = ChuanHoaViTri(viTri);
    if (viTriChuan.empty()) {
        viTriChuan = LanguageManager::instance().getText("MSG_LOCATION_UNDEFINED").toStdString();
    }
    
    // Hiển thị trạng thái đang kiểm tra
    nhanTrangThai->setText(LanguageManager::instance().getText("MSG_CHECKING_DATA"));
    nhanTrangThai->setStyleSheet("color: #FFA800; font-weight: bold;");
    QApplication::processEvents();
    
    // DÙNG BACKEND SERVICE LAYER - ThemBanSaoMoi()
    std::string isbn = maISBN.toStdString();
    std::string ketQua = ThemBanSaoMoi(isbn, soLuong, viTriChuan);
    
    if (ketQua.empty()) {
        // Thành công
        AnimationHelper::fadeIn(nhanTrangThai, 200);
        nhanTrangThai->setText(LanguageManager::instance().getText("MSG_ADD_COPY_SUCCESS")
                              .arg(soLuong)
                              .arg(QString::fromStdString(viTriChuan)));
        nhanTrangThai->setStyleSheet("color: #1BC5BD; font-weight: bold; font-size: 13px;");
        
        // Tự động lưu file
        extern PTRDS dsDauSach[];
        extern int soLuongDauSach;
        GhiDanhSachDauSach(FilePathManager::getDauSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
        GhiDanhMucSach(FilePathManager::getDanhMucSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
        
        emit banSaoDaThem();
        QTimer::singleShot(1500, this, &HopThoaiThemBanSao::accept);
    } else {
        // Lỗi
        AnimationHelper::shake(nhanTrangThai, 300);
        nhanTrangThai->setText("✕ " + QString::fromStdString(ketQua));
        nhanTrangThai->setStyleSheet("color: #F64E60; font-weight: bold; font-size: 13px;");
        nutLuu->setEnabled(true);
        nutHuy->setEnabled(true);
        nutLuu->setText(LanguageManager::instance().getText("BTN_ADD_SYMBOL"));
    }
}
