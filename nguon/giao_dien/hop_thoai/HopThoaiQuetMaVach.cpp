#include "hop_thoai/HopThoaiQuetMaVach.h"
#include "models/DocGia.h"
#include "models/DauSach.h"
#include "models/QuanLySach.h"
#include "models/ThaoTacFile.h"
#include "utils/FilePathManager.h"
#include "utils/TransactionLogger.h"
#include <QMessageBox>
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
    setWindowTitle("Quét Mã (Barcode/QR Code)");
    setMinimumWidth(500);
    setMinimumHeight(400);
    
    QVBoxLayout *boTriChinh = new QVBoxLayout(this);
    boTriChinh->setSpacing(15);
    boTriChinh->setContentsMargins(20, 20, 20, 20);
    
    // === Hướng dẫn ===
    QString huongDan;
    switch(loaiQuet) {
        case QUET_SACH:
            huongDan = "Quét mã sách (Barcode/QR) hoặc nhập thủ công";
            break;
        case QUET_DOC_GIA:
            huongDan = "Quét mã thẻ độc giả (Barcode/QR) hoặc nhập thủ công";
            break;
        case QUET_TAT_CA:
            huongDan = "Quét mã (Barcode/QR - sách hoặc độc giả) hoặc nhập thủ công";
            break;
    }
    
    QLabel *nhanHuongDan = new QLabel(huongDan);
    nhanHuongDan->setStyleSheet(
        "font-size: 14px; "
        "color: #555; "
        "padding: 10px; "
        "background-color: #f0f8ff; "
        "border-radius: 5px;"
    );
    nhanHuongDan->setWordWrap(true);
    boTriChinh->addWidget(nhanHuongDan);
    
    // === Ô nhập mã ===
    QGroupBox *nhomNhap = new QGroupBox("Nhập Mã");
    QVBoxLayout *boTriNhap = new QVBoxLayout(nhomNhap);
    
    oNhapMa = new QLineEdit();
    oNhapMa->setPlaceholderText("Nhập hoặc quét mã vạch...");
    oNhapMa->setStyleSheet(
        "QLineEdit { "
        "   padding: 12px; "
        "   font-size: 16px; "
        "   border: 2px solid #4CAF50; "
        "   border-radius: 5px; "
        "}"
    );
    
    // Khi nhấn Enter trong ô nhập -> tự động quét
    connect(oNhapMa, &QLineEdit::returnPressed, this, &HopThoaiQuetMaVach::xuLyQuetMa);
    
    boTriNhap->addWidget(oNhapMa);
    
    // Nút quét và xóa
    QHBoxLayout *boTriNutPhu = new QHBoxLayout();
    
    nutQuet = new QPushButton("Quét/Tìm Kiếm");
    nutQuet->setStyleSheet(
        "QPushButton { "
        "   background-color: #4CAF50; "
        "   color: white; "
        "   padding: 10px 20px; "
        "   font-size: 14px; "
        "   border: none; "
        "   border-radius: 5px; "
        "} "
        "QPushButton:hover { background-color: #45a049; }"
    );
    connect(nutQuet, &QPushButton::clicked, this, &HopThoaiQuetMaVach::xuLyQuetMa);
    
    nutXoaMa = new QPushButton("🗑️ Xóa");
    nutXoaMa->setStyleSheet(
        "QPushButton { "
        "   background-color: #f44336; "
        "   color: white; "
        "   padding: 10px 20px; "
        "   font-size: 14px; "
        "   border: none; "
        "   border-radius: 5px; "
        "} "
        "QPushButton:hover { background-color: #da190b; }"
    );
    connect(nutXoaMa, &QPushButton::clicked, this, &HopThoaiQuetMaVach::xuLyXoaMa);
    
    boTriNutPhu->addWidget(nutQuet);
    boTriNutPhu->addWidget(nutXoaMa);
    boTriNhap->addLayout(boTriNutPhu);
    
    boTriChinh->addWidget(nhomNhap);
    
    // === Kết quả tìm kiếm ===
    nhanKetQua = new QLabel("Chưa có kết quả");
    nhanKetQua->setStyleSheet(
        "font-size: 13px; "
        "color: #666; "
        "padding: 8px; "
        "background-color: #fafafa; "
        "border-radius: 5px;"
    );
    boTriChinh->addWidget(nhanKetQua);
    
    // === Thông tin chi tiết ===
    nhomThongTin = new QGroupBox("Thông Tin Chi Tiết");
    QVBoxLayout *boTriThongTin = new QVBoxLayout(nhomThongTin);
    
    nhanThongTin = new QLabel("Vui lòng quét hoặc nhập mã để xem thông tin");
    nhanThongTin->setStyleSheet(
        "font-size: 13px; "
        "color: #333; "
        "padding: 15px;"
    );
    nhanThongTin->setWordWrap(true);
    nhanThongTin->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    boTriThongTin->addWidget(nhanThongTin);
    boTriChinh->addWidget(nhomThongTin);
    
    // === Nút hành động ===
    QHBoxLayout *boTriNut = new QHBoxLayout();
    boTriNut->addStretch();
    
    nutXacNhan = new QPushButton("✓ Xác Nhận");
    nutXacNhan->setEnabled(false);
    nutXacNhan->setStyleSheet(
        "QPushButton { "
        "   background-color: #2196F3; "
        "   color: white; "
        "   padding: 10px 30px; "
        "   font-size: 14px; "
        "   border: none; "
        "   border-radius: 5px; "
        "   min-width: 120px; "
        "} "
        "QPushButton:hover { background-color: #0b7dda; } "
        "QPushButton:disabled { background-color: #ccc; }"
    );
    connect(nutXacNhan, &QPushButton::clicked, this, &HopThoaiQuetMaVach::xuLyXacNhan);
    
    nutHuy = new QPushButton("✗ Hủy");
    nutHuy->setStyleSheet(
        "QPushButton { "
        "   background-color: #757575; "
        "   color: white; "
        "   padding: 10px 30px; "
        "   font-size: 14px; "
        "   border: none; "
        "   border-radius: 5px; "
        "   min-width: 120px; "
        "} "
        "QPushButton:hover { background-color: #616161; }"
    );
    connect(nutHuy, &QPushButton::clicked, this, &HopThoaiQuetMaVach::xuLyHuy);
    
    boTriNut->addWidget(nutXacNhan);
    boTriNut->addWidget(nutHuy);
    boTriChinh->addLayout(boTriNut);
}

void HopThoaiQuetMaVach::xuLyQuetMa() {
    QString ma = oNhapMa->text().trimmed();
    
    if (ma.isEmpty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng nhập mã cần quét!");
        oNhapMa->setFocus();
        return;
    }
    
    nhanKetQua->setText(" Đang tìm kiếm...");
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
                        
                        TransactionLogger::ghiLogTra(docGia, maSach, tenSach, true);
                        
                        // Tự động lưu file
                        saveDocGia(rootDocGia);
                        GhiDanhSachDauSach(FilePathManager::getDauSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
                        GhiDanhMucSach(FilePathManager::getDanhMucSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
                        
                        QString hoTen = QString::fromStdString(docGia->data.Ho + " " + docGia->data.Ten);
                        nhanKetQua->setText("✓ Đã trả sách thành công!");
                        nhanKetQua->setStyleSheet("color: #4CAF50; font-size: 13px; font-weight: bold; padding: 8px;");
                        
                        QString thongTin = QString(
                            "<b>TRẢ SÁCH THÀNH CÔNG</b><br><br>"
                            "<b>Độc giả:</b> %1 (Mã thẻ: %2)<br>"
                            "<b>Sách:</b> %3<br>"
                            "<b>Mã sách:</b> %4<br>"
                            "<b>Ngày mượn:</b> %5<br>"
                            "<b>Ngày trả:</b> %6<br>"
                        ).arg(hoTen).arg(maThe).arg(tenSach).arg(maSach).arg(ngayMuon)
                         .arg(QDate::currentDate().toString("dd/MM/yyyy"));
                        
                        nhanThongTin->setText(thongTin);
                        
                        QMessageBox::information(this, "Thành Công", 
                            QString("Đã trả sách thành công!\n\nĐộc giả: %1\nSách: %2")
                            .arg(hoTen).arg(tenSach));
                        
                        // Đóng dialog sau khi trả thành công
                        QTimer::singleShot(1500, this, &QDialog::accept);
                        return;
                    } else {
                        nhanKetQua->setText("❌ Lỗi trả sách: " + QString::fromStdString(ketQua));
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
        nhanKetQua->setText("❌ Không tìm thấy mã: " + ma);
        nhanKetQua->setStyleSheet("color: #f44336; font-size: 13px; padding: 8px;");
        nhanThongTin->setText("Mã vạch không tồn tại trong hệ thống.\n\n"
                             "Vui lòng kiểm tra lại hoặc thử quét lại.");
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
            
            nhanKetQua->setText("✓ Tìm thấy sách: " + QString::fromStdString(sach->tenSach));
            nhanKetQua->setStyleSheet("color: #4CAF50; font-size: 13px; font-weight: bold; padding: 8px;");
            
            QString thongTin = QString(
                "<b>THÔNG TIN SÁCH</b><br><br>"
                "<b>Tên sách:</b> %1<br>"
                "<b>Tác giả:</b> %2<br>"
                "<b>Thể loại:</b> %3<br>"
                "<b>ISBN:</b> %4<br>"
                "<b>Năm xuất bản:</b> %5<br>"
                "<b>Tổng bản sao:</b> %6<br>"
                "<b>Số trang:</b> %7"
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
        
        nhanKetQua->setText("✓ Tìm thấy độc giả: " + tenDayDu);
        nhanKetQua->setStyleSheet("color: #4CAF50; font-size: 13px; font-weight: bold; padding: 8px;");
        
        QString trangThai;
        QString mauTrangThai;
        switch(docGia->data.TrangThai) {
            case 0: trangThai = "🔴 Bị khóa"; mauTrangThai = "red"; break;
            case 1: trangThai = "🟢 Hoạt động"; mauTrangThai = "green"; break;
            default: trangThai = "⚪ Không xác định"; mauTrangThai = "gray"; break;
        }
        
        QString gioiTinh = (docGia->data.Phai == 0) ? "Nam" : "Nữ";
        
        QString thongTin = QString(
            "<b>👤 THÔNG TIN ĐỘC GIẢ</b><br><br>"
            "<b>Họ tên:</b> %1<br>"
            "<b>Mã thẻ:</b> %2<br>"
            "<b>Giới tính:</b> %3<br>"
            "<b>Trạng thái:</b> <span style='color: %4;'>%5</span><br>"
            "<b>Số sách đang mượn:</b> %6 / 3"
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
        QMessageBox::warning(this, "Lỗi", "Chưa quét được mã nào!");
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
    nhanKetQua->setText("Chưa có kết quả");
    nhanKetQua->setStyleSheet("font-size: 13px; color: #666;");
    nhanThongTin->setText("Vui lòng quét hoặc nhập mã để xem thông tin");
    nutXacNhan->setEnabled(false);
    maQuetDuoc.clear();
    loaiMaQuet.clear();
    oNhapMa->setFocus();
}
