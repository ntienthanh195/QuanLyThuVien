#include "hop_thoai/HopThoaiQuetMaVach.h"
#include "models/DocGia.h"
#include "models/DauSach.h"
#include "models/QuanLySach.h"
#include "models/ThaoTacFile.h"
#include "utils/FilePathManager.h"
#include "utils/AuditLogger.h"
#include <QMessageBox>
#include "utils/LanguageManager.h"
#include <QTimer>
#include <QDate>

// Biến toàn cục
extern PTRDS dsDauSach[MAX_DAUSACH];
extern int soLuongDauSach;
extern PTRDG rootDocGia;

HopThoaiQuetMaVach::HopThoaiQuetMaVach(LoaiQuet loai, QWidget *cha)
    : QDialog(cha), loaiQuet(loai), loaiMaQuet("") {
    
    thieLapGiaoDien();
    
    // Auto focus vào ô nhập
    oNhapMa->setFocus();
}

HopThoaiQuetMaVach::~HopThoaiQuetMaVach() {
}

QString HopThoaiQuetMaVach::layMaQuet() const {
    return maQuetDuoc;
}

QString HopThoaiQuetMaVach::layLoaiMaQuet() const {
    return loaiMaQuet;
}

void HopThoaiQuetMaVach::thieLapGiaoDien() {
    setWindowTitle(LanguageManager::instance().getText("DIALOG_SCAN_TITLE"));
    setMinimumWidth(500);
    setMinimumHeight(400);
    
    // === DEEP NIGHT THEME SETUP ===
    this->setStyleSheet("background-color: #1e1e2d; color: #e0e0e0;");
    
    QVBoxLayout *boTriChinh = new QVBoxLayout(this);
    boTriChinh->setSpacing(20);
    boTriChinh->setContentsMargins(25, 25, 25, 25);
    
    // Header Label
    QLabel *lblIcon = new QLabel("📷"); 
    lblIcon->setAlignment(Qt::AlignCenter);
    lblIcon->setStyleSheet("font-size: 40px; margin-bottom: 5px;");
    boTriChinh->addWidget(lblIcon);
    
    QString titleText = LanguageManager::instance().getText("HEADER_SCAN_RETURN_SEARCH");
    if (loaiQuet == QUET_SACH) titleText = LanguageManager::instance().getText("HEADER_SCAN_BOOK");
    else if (loaiQuet == QUET_DOC_GIA) titleText = LanguageManager::instance().getText("HEADER_SCAN_READER");
    
    QLabel *lblTitle = new QLabel(titleText);
    lblTitle->setAlignment(Qt::AlignCenter);
    lblTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #ffffff; letter-spacing: 1px;");
    boTriChinh->addWidget(lblTitle);
    
    // === Hướng dẫn (Subtext) ===
    QLabel *nhanHuongDan = new QLabel(LanguageManager::instance().getText("MSG_SCAN_INSTRUCTION"));
    nhanHuongDan->setAlignment(Qt::AlignCenter);
    nhanHuongDan->setStyleSheet("font-size: 13px; color: #8888aa; margin-bottom: 10px;");
    boTriChinh->addWidget(nhanHuongDan);
    
    // === Ô nhập mã ===
    QGroupBox *nhomNhap = new QGroupBox(); // No title for cleaner look
    nhomNhap->setStyleSheet("QGroupBox { border: none; background: transparent; margin: 0; padding: 0; }");
    QVBoxLayout *boTriNhap = new QVBoxLayout(nhomNhap);
    boTriNhap->setContentsMargins(0,0,0,0);
    boTriNhap->setSpacing(10);
    
    oNhapMa = new QLineEdit();
    oNhapMa->setPlaceholderText(LanguageManager::instance().getText("MSG_PLEASE_SCAN"));
    oNhapMa->setAlignment(Qt::AlignCenter);
    oNhapMa->setStyleSheet(
        "QLineEdit { "
        "   background-color: #2b2b40; "
        "   color: #ffffff; "
        "   padding: 12px; "
        "   font-size: 18px; "
        "   font-weight: bold; "
        "   border: 2px solid #3E3E48; "
        "   border-radius: 8px; "
        "   selection-background-color: #5d9cec;"
        "}"
        "QLineEdit:focus { border-color: #5d9cec; }"
    );
    
    // Khi nhấn Enter trong ô nhập -> tự động quét
    connect(oNhapMa, &QLineEdit::returnPressed, this, &HopThoaiQuetMaVach::xuLyQuetMa);
    boTriNhap->addWidget(oNhapMa);
    
    // Nút Action Row
    QHBoxLayout *boTriNutPhu = new QHBoxLayout();
    boTriNutPhu->setSpacing(10);
    
    nutQuet = new QPushButton(LanguageManager::instance().getText("BTN_CONFIRM_SCAN"));
    nutQuet->setCursor(Qt::PointingHandCursor);
    nutQuet->setStyleSheet(
        "QPushButton { "
        "   background-color: #0078d4; "
        "   color: white; "
        "   padding: 10px; "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "   border: none; "
        "   border-radius: 6px; "
        "} "
        "QPushButton:hover { background-color: #106ebe; }"
    );
    connect(nutQuet, &QPushButton::clicked, this, &HopThoaiQuetMaVach::xuLyQuetMa);
    
    nutXoaMa = new QPushButton("🗑️");
    nutXoaMa->setFixedWidth(50);
    nutXoaMa->setCursor(Qt::PointingHandCursor);
    nutXoaMa->setStyleSheet(
        "QPushButton { "
        "   background-color: #3b3b50; "
        "   color: #e0e0e0; "
        "   padding: 10px; "
        "   font-size: 14px; "
        "   border: 1px solid #45455a; "
        "   border-radius: 6px; "
        "} "
        "QPushButton:hover { background-color: #c0392b; border-color: #c0392b; color: white; }"
    );
    connect(nutXoaMa, &QPushButton::clicked, this, &HopThoaiQuetMaVach::xuLyXoaMa);
    
    boTriNutPhu->addWidget(nutQuet);
    boTriNutPhu->addWidget(nutXoaMa);
    boTriNhap->addLayout(boTriNutPhu);
    
    boTriChinh->addWidget(nhomNhap);
    
    // === Kết quả tìm kiếm ===
    nhanKetQua = new QLabel("");
    nhanKetQua->setAlignment(Qt::AlignCenter);
    nhanKetQua->setStyleSheet("font-size: 14px; color: #aaaaaa; padding: 5px;");
    boTriChinh->addWidget(nhanKetQua);
    
    // === Thông tin chi tiết ===
    nhomThongTin = new QGroupBox(LanguageManager::instance().getText("GROUP_SCAN_RESULT"));
    nhomThongTin->setStyleSheet(
        "QGroupBox { "
        "   background-color: #252526; "
        "   border: 1px solid #3E3E42; "
        "   border-radius: 6px; "
        "   margin-top: 20px; "
        "   padding-top: 15px; "
        "   color: #E0E0E0; font-weight: bold;"
        "}"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; color: #5BA3F5; background-color: #1e1e2d; }"
    );
    QVBoxLayout *boTriThongTin = new QVBoxLayout(nhomThongTin);
    boTriThongTin->setContentsMargins(15, 20, 15, 15);
    
    nhanThongTin = new QLabel(LanguageManager::instance().getText("MSG_PLEASE_SCAN"));
    nhanThongTin->setStyleSheet("font-size: 13px; color: #ccc; line-height: 1.4;");
    nhanThongTin->setWordWrap(true);
    nhanThongTin->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    boTriThongTin->addWidget(nhanThongTin);
    boTriChinh->addWidget(nhomThongTin);
    
    // === Footer Buttons ===
    QHBoxLayout *boTriNut = new QHBoxLayout();
    boTriNut->addStretch();
    
    nutXacNhan = new QPushButton(LanguageManager::instance().getText("BTN_USE"));
    nutXacNhan->setEnabled(false);
    nutXacNhan->setCursor(Qt::PointingHandCursor);
    nutXacNhan->setStyleSheet(
        "QPushButton { "
        "   background-color: #28a745; "
        "   color: white; "
        "   padding: 10px 25px; "
        "   font-size: 13px; font-weight: bold; "
        "   border: none; border-radius: 4px; "
        "} "
        "QPushButton:hover { background-color: #218838; } "
        "QPushButton:disabled { background-color: #3a3a4e; color: #666; }"
    );
    connect(nutXacNhan, &QPushButton::clicked, this, &HopThoaiQuetMaVach::xuLyXacNhan);
    
    nutHuy = new QPushButton(LanguageManager::instance().getText("BTN_CLOSE"));
    nutHuy->setCursor(Qt::PointingHandCursor);
    nutHuy->setStyleSheet(
        "QPushButton { "
        "   background-color: transparent; "
        "   color: #aaa; "
        "   padding: 10px 20px; "
        "   font-size: 13px; font-weight: bold; "
        "   border: 1px solid #444; border-radius: 4px; "
        "} "
        "QPushButton:hover { background-color: #333; color: white; border-color: #666; }"
    );
    connect(nutHuy, &QPushButton::clicked, this, &HopThoaiQuetMaVach::xuLyHuy);
    
    boTriNut->addWidget(nutHuy);
    boTriNut->addWidget(nutXacNhan);
    boTriChinh->addLayout(boTriNut);
}

void HopThoaiQuetMaVach::xuLyQuetMa() {
    QString ma = oNhapMa->text().trimmed();
    
    if (ma.isEmpty()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_ERROR"), LanguageManager::instance().getText("MSG_ENTER_CODE"));
        oNhapMa->setFocus();
        return;
    }
    
    nhanKetQua->setText(LanguageManager::instance().getText("MSG_SEARCHING"));
    nhanKetQua->setStyleSheet("color: #FF9800; font-size: 13px;");
    
    // Delay nhỏ để hiển thị trạng thái đang tìm
    QTimer::singleShot(100, this, [this, ma]() {
        timKiemVaHienThi(ma);
    });
}

void HopThoaiQuetMaVach::timKiemVaHienThi(const QString &ma) {
    bool timThay = false;
    
    // Kiểm tra nếu là QR code chứa thông tin mượn trả (format: MaThe|MaSach|NgayMuon)
    if (ma.contains('|')) {
        QStringList parts = ma.split('|');
        if (parts.size() == 3) {
            QString maTheStr = parts[0];
            QString maSach = parts[1];
            QString ngayMuon = parts[2];
            
            // Tự động trả sách
            bool ok;
            int maThe = maTheStr.toInt(&ok);
            if (ok) {
                PTRDG docGia = timDocGia(rootDocGia, maThe);
                if (docGia) {
                    extern PTRDS dsDauSach[];
                    extern int soLuongDauSach;
                    
                    std::string ketQua = TraSach(docGia, maSach.toStdString(), dsDauSach, soLuongDauSach);
                    
                    if (ketQua.find("Thành công") != std::string::npos || ketQua.find("thành công") != std::string::npos) {
                        // Tìm tên sách để ghi log
                        QString tenSach = "";
                        for (int i = 0; i < soLuongDauSach; i++) {
                            if (dsDauSach[i]) {
                                PTRDMS p = dsDauSach[i]->dms;
                                while (p != nullptr) {
                                    if (QString::fromStdString(p->maSach) == maSach) {
                                        tenSach = QString::fromStdString(dsDauSach[i]->tenSach);
                                        break;
                                    }
                                    p = p->next;
                                }
                                if (!tenSach.isEmpty()) break;
                            }
                        }
                        
                        AuditLogger::log(AuditLogger::ACTION_RETURN, "MUON_TRA", maSach, "Trả sách: " + tenSach);
                        
                        // Dữ liệu đã được đồng bộ SQL bên trong hàm TraSach
                        
                        QString hoTen = QString::fromStdString(docGia->data.Ho + " " + docGia->data.Ten);
                        nhanKetQua->setText(LanguageManager::instance().getText("MSG_RETURN_SUCCESS_ICON"));
                        nhanKetQua->setStyleSheet("color: #4CAF50; font-size: 13px; font-weight: bold; padding: 8px;");
                        
                        QString thongTin = QString(
                            "<b>" + LanguageManager::instance().getText("TITLE_RETURN_SUCCESS_HTML") + "</b><br><br>"
                            "<b>" + LanguageManager::instance().getText("LBL_RETURN_READER") + "</b> %1 (Mã: %2)<br>"
                            "<b>" + LanguageManager::instance().getText("LBL_RETURN_BOOK") + "</b> %3<br>"
                            "<b>" + LanguageManager::instance().getText("LBL_RETURN_BOOK_CODE") + "</b> %4<br>"
                            "<b>" + LanguageManager::instance().getText("LBL_BORROW_DATE") + "</b> %5<br>"
                            "<b>" + LanguageManager::instance().getText("LBL_RETURN_DATE") + "</b> %6<br>"
                        ).arg(hoTen).arg(maThe).arg(tenSach).arg(maSach).arg(ngayMuon)
                         .arg(QDate::currentDate().toString("dd/MM/yyyy"));
                        
                        nhanThongTin->setText(thongTin);
                        
                        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"), 
                            LanguageManager::instance().getText("MSG_RETURN_SUCCESS_BODY")
                            .arg(hoTen).arg(tenSach));
                        
                        // Đóng dialog sau khi trả thành công
                        QTimer::singleShot(1500, this, &QDialog::accept);
                        return;
                    } else {
                        nhanKetQua->setText(LanguageManager::instance().getText("MSG_RETURN_ERROR_ICON") + QString::fromStdString(ketQua));
                        nhanKetQua->setStyleSheet("color: #f44336; font-size: 13px; padding: 8px;");
                        nhanThongTin->setText(QString::fromStdString(ketQua));
                        return;
                    }
                }
            }
        }
    }
    
    // Thử tìm sách trước (nếu cho phép)
    if (loaiQuet == QUET_SACH || loaiQuet == QUET_TAT_CA) {
        for (int i = 0; i < soLuongDauSach; i++) {
            if (dsDauSach[i] && QString::fromStdString(dsDauSach[i]->ISBN) == ma) {
                hienThiThongTinSach(ma);
                maQuetDuoc = ma;
                loaiMaQuet = "SACH";
                timThay = true;
                break;
            }
        }
    }
    
    // Nếu chưa tìm thấy, thử tìm độc giả
    if (!timThay && (loaiQuet == QUET_DOC_GIA || loaiQuet == QUET_TAT_CA)) {
        bool ok;
        int maThe = ma.toInt(&ok);
        if (ok) {
            PTRDG docGia = timDocGia(rootDocGia, maThe);
            
            if (docGia) {
                hienThiThongTinDocGia(ma);
                maQuetDuoc = ma;
                loaiMaQuet = "DOC_GIA";
                timThay = true;
            }
        }
    }
    
    if (!timThay) {
        nhanKetQua->setText(LanguageManager::instance().getText("MSG_CODE_NOT_FOUND") + ma);
        nhanKetQua->setStyleSheet("color: #f44336; font-size: 13px; padding: 8px;");
        nhanThongTin->setText(LanguageManager::instance().getText("MSG_BARCODE_NOT_EXIST"));
        nutXacNhan->setEnabled(false);
        maQuetDuoc.clear();
        loaiMaQuet.clear();
    } else {
        nutXacNhan->setEnabled(true);
        emit maVachDuocQuet(maQuetDuoc, loaiMaQuet);
    }
}

void HopThoaiQuetMaVach::hienThiThongTinSach(const QString &isbn) {
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] && QString::fromStdString(dsDauSach[i]->ISBN) == isbn) {
            PTRDS sach = dsDauSach[i];
            
            nhanKetQua->setText(LanguageManager::instance().getText("MSG_FOUND_BOOK") + QString::fromStdString(sach->tenSach));
            nhanKetQua->setStyleSheet("color: #4CAF50; font-size: 13px; font-weight: bold; padding: 8px;");
            
            QString thongTin = QString(
                "<b>" + LanguageManager::instance().getText("TITLE_BOOK_INFO") + "</b><br><br>"
                "<b>" + LanguageManager::instance().getText("LBL_TITLE") + ":</b> %1<br>"
                "<b>" + LanguageManager::instance().getText("LBL_AUTHOR") + "</b> %2<br>"
                "<b>" + LanguageManager::instance().getText("LBL_CATEGORY") + "</b> %3<br>"
                "<b>ISBN:</b> %4<br>"
                "<b>" + LanguageManager::instance().getText("LBL_PUBLISH_YEAR") + "</b> %5<br>"
                "<b>" + LanguageManager::instance().getText("LBL_TOTAL_COPIES_SCAN") + "</b> %6<br>"
                "<b>" + LanguageManager::instance().getText("LBL_PAGES") + "</b> %7"
            ).arg(QString::fromStdString(sach->tenSach))
             .arg(QString::fromStdString(sach->tacGia))
             .arg(QString::fromStdString(sach->theLoai))
             .arg(QString::fromStdString(sach->ISBN))
             .arg(sach->namXuatBan)
             .arg(sach->tongBanSao)
             .arg(sach->soTrang);
            
            nhanThongTin->setText(thongTin);
            break;
        }
    }
}

void HopThoaiQuetMaVach::hienThiThongTinDocGia(const QString &maThe) {
    bool ok;
    int maTheInt = maThe.toInt(&ok);
    if (!ok) return;
    
    PTRDG docGia = timDocGia(rootDocGia, maTheInt);
    
    if (docGia) {
        QString tenDayDu = QString::fromStdString(docGia->data.Ho) + " " + 
                          QString::fromStdString(docGia->data.Ten);
        
        nhanKetQua->setText(LanguageManager::instance().getText("MSG_FOUND_READER") + tenDayDu);
        nhanKetQua->setStyleSheet("color: #4CAF50; font-size: 13px; font-weight: bold; padding: 8px;");
        
        QString trangThai;
        QString mauTrangThai;
        switch(docGia->data.TrangThai) {
            case 0: trangThai = LanguageManager::instance().getText("STATUS_LOCKED_ICON"); mauTrangThai = "red"; break;
            case 1: trangThai = LanguageManager::instance().getText("STATUS_ACTIVE_ICON"); mauTrangThai = "green"; break;
            default: trangThai = LanguageManager::instance().getText("STATUS_UNKNOWN_ICON"); mauTrangThai = "gray"; break;
        }
        
        QString gioiTinh = (docGia->data.Phai == 0) ? LanguageManager::instance().getText("GENDER_MALE") : LanguageManager::instance().getText("GENDER_FEMALE");
        
        QString thongTin = QString(
            "<b>" + LanguageManager::instance().getText("TITLE_READER_INFO") + "</b><br><br>"
            "<b>" + LanguageManager::instance().getText("LBL_FULLNAME") + "</b> %1<br>"
            "<b>" + LanguageManager::instance().getText("LBL_CARD_ID") + "</b> %2<br>"
            "<b>" + LanguageManager::instance().getText("LBL_GENDER_TEXT") + "</b> %3<br>"
            "<b>" + LanguageManager::instance().getText("LBL_STATUS_TEXT") + "</b> <span style='color: %4;'>%5</span><br>"
            "<b>" + LanguageManager::instance().getText("LBL_BORROWING_COUNT") + "</b> %6 / 3"
        ).arg(tenDayDu)
         .arg(docGia->data.MaThe)
         .arg(gioiTinh)
         .arg(mauTrangThai)
         .arg(trangThai)
         .arg(docGia->data.soSachDangMuon);
        
        nhanThongTin->setText(thongTin);
    }
}

void HopThoaiQuetMaVach::xuLyXacNhan() {
    if (maQuetDuoc.isEmpty()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_ERROR"), LanguageManager::instance().getText("MSG_NO_CODE_SCANNED"));
        return;
    }
    
    accept();  // Đóng dialog với trạng thái accepted
}

void HopThoaiQuetMaVach::xuLyHuy() {
    maQuetDuoc.clear();
    loaiMaQuet.clear();
    reject();  // Đóng dialog với trạng thái rejected
}

void HopThoaiQuetMaVach::xuLyXoaMa() {
    oNhapMa->clear();
    nhanKetQua->setText(LanguageManager::instance().getText("MSG_NO_RESULT"));
    nhanKetQua->setStyleSheet("font-size: 13px; color: #666;");
    nhanThongTin->setText(LanguageManager::instance().getText("MSG_SCAN_RESET"));
    nutXacNhan->setEnabled(false);
    maQuetDuoc.clear();
    loaiMaQuet.clear();
    oNhapMa->setFocus();
}
