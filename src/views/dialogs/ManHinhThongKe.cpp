#include "views/dialogs/ManHinhThongKe.h"
#include "models/QuanLySach.h"
#include "models/DocGia.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QFrame>

ManHinhThongKe::ManHinhThongKe(PTRDS dsDauSach[], int soLuongDauSach, PTRDG rootDocGia, QWidget *parent)
    : QDialog(parent), danhSachDauSach(dsDauSach), soLuong(soLuongDauSach), cayDocGia(rootDocGia) {
    setWindowTitle("Thống Kê Hệ Thống");
    setMinimumSize(800, 600);
    setModal(true);
    
    thietLapGiaoDien();
    apDungPhongCach();
    capNhatThongKe();
}

void ManHinhThongKe::thietLapGiaoDien() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(25, 25, 25, 25);
    
    // Title
    QLabel* titleLabel = new QLabel("THỐNG KÊ HỆ THỐNG THƯ VIỆN");
    titleLabel->setStyleSheet(
        "font-size: 22px; "
        "font-weight: bold; "
        "color: white; "
        "padding: 15px; "
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1e40af, stop:1 #059669); "
        "border-radius: 8px;"
    );
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Info panel
    QLabel* infoLabel = new QLabel(
        "<b>Lưu ý:</b> Thống kê được cập nhật tự động từ dữ liệu hiện tại trong hệ thống");
    infoLabel->setStyleSheet(
        "color: #7f8c8d; "
        "font-size: 11px; "
        "padding: 8px; "
        "background-color: #ecf0f1; "
        "border-radius: 5px;"
    );
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);
    
    // Horizontal layout for two columns
    QHBoxLayout* columnsLayout = new QHBoxLayout();
    columnsLayout->setSpacing(20);
    
    // === SÁCH COLUMN ===
    QGroupBox* sachGroup = new QGroupBox("Thống Kê Sách");
    sachGroup->setObjectName("statsGroup");
    QGridLayout* sachLayout = new QGridLayout(sachGroup);
    sachLayout->setSpacing(12);
    sachLayout->setContentsMargins(20, 20, 20, 20);
    
    int row = 0;
    
    // Tổng đầu sách
    QLabel* lblTongDauSach = new QLabel("📖 Tổng số đầu sách:");
    lblTongDauSach->setObjectName("labelMetric");
    nhanTongDauSach = new QLabel("0");
    nhanTongDauSach->setObjectName("valueMetric");
    sachLayout->addWidget(lblTongDauSach, row, 0);
    sachLayout->addWidget(nhanTongDauSach, row, 1);
    row++;
    
    // Tổng bản sao
    QLabel* lblTongBanSao = new QLabel("📚 Tổng số bản sao:");
    lblTongBanSao->setObjectName("labelMetric");
    nhanTongBanSao = new QLabel("0");
    nhanTongBanSao->setObjectName("valueMetric");
    sachLayout->addWidget(lblTongBanSao, row, 0);
    sachLayout->addWidget(nhanTongBanSao, row, 1);
    row++;
    
    // Bản sao có thể mượn
    QLabel* lblCoTheMuon = new QLabel("✅ Có thể mượn:");
    lblCoTheMuon->setObjectName("labelMetric");
    nhanBanSaoCoTheMuon = new QLabel("0");
    nhanBanSaoCoTheMuon->setObjectName("valueGreen");
    sachLayout->addWidget(lblCoTheMuon, row, 0);
    sachLayout->addWidget(nhanBanSaoCoTheMuon, row, 1);
    row++;
    
    // Đang được mượn
    QLabel* lblDangMuon = new QLabel("📤 Đang được mượn:");
    lblDangMuon->setObjectName("labelMetric");
    nhanBanSaoDangMuon = new QLabel("0");
    nhanBanSaoDangMuon->setObjectName("valueBlue");
    sachLayout->addWidget(lblDangMuon, row, 0);
    sachLayout->addWidget(nhanBanSaoDangMuon, row, 1);
    row++;
    
    // Thanh lý
    QLabel* lblThanhLy = new QLabel("🗑️ Đã thanh lý:");
    lblThanhLy->setObjectName("labelMetric");
    nhanBanSaoThanhLy = new QLabel("0");
    nhanBanSaoThanhLy->setObjectName("valueRed");
    sachLayout->addWidget(lblThanhLy, row, 0);
    sachLayout->addWidget(nhanBanSaoThanhLy, row, 1);
    row++;
    
    // Tỷ lệ cho mượn
    QLabel* lblTyLe = new QLabel("📊 Tỷ lệ cho mượn:");
    lblTyLe->setObjectName("labelMetric");
    nhanTyLeChoMuon = new QLabel("0.00%");
    nhanTyLeChoMuon->setObjectName("valueMetric");
    sachLayout->addWidget(lblTyLe, row, 0);
    sachLayout->addWidget(nhanTyLeChoMuon, row, 1);
    row++;
    
    // Số thể loại
    QLabel* lblTheLoai = new QLabel("🏷️ Số thể loại khác nhau:");
    lblTheLoai->setObjectName("labelMetric");
    nhanSoTheLoai = new QLabel("0");
    nhanSoTheLoai->setObjectName("valueMetric");
    sachLayout->addWidget(lblTheLoai, row, 0);
    sachLayout->addWidget(nhanSoTheLoai, row, 1);
    
    sachLayout->setColumnStretch(0, 3);
    sachLayout->setColumnStretch(1, 1);
    columnsLayout->addWidget(sachGroup);
    
    // === ĐỘC GIẢ COLUMN ===
    QGroupBox* docGiaGroup = new QGroupBox("Thống Kê Độc Giả");
    docGiaGroup->setObjectName("statsGroup");
    QGridLayout* docGiaLayout = new QGridLayout(docGiaGroup);
    docGiaLayout->setSpacing(12);
    docGiaLayout->setContentsMargins(20, 20, 20, 20);
    
    row = 0;
    
    // Tổng độc giả
    QLabel* lblTongDocGia = new QLabel("👤 Tổng số độc giả:");
    lblTongDocGia->setObjectName("labelMetric");
    nhanTongDocGia = new QLabel("0");
    nhanTongDocGia->setObjectName("valueMetric");
    docGiaLayout->addWidget(lblTongDocGia, row, 0);
    docGiaLayout->addWidget(nhanTongDocGia, row, 1);
    row++;
    
    // Hoạt động
    QLabel* lblHoatDong = new QLabel("✅ Đang hoạt động:");
    lblHoatDong->setObjectName("labelMetric");
    nhanDocGiaHoatDong = new QLabel("0");
    nhanDocGiaHoatDong->setObjectName("valueGreen");
    docGiaLayout->addWidget(lblHoatDong, row, 0);
    docGiaLayout->addWidget(nhanDocGiaHoatDong, row, 1);
    row++;
    
    // Bị khóa
    QLabel* lblBiKhoa = new QLabel("🔒 Bị khóa:");
    lblBiKhoa->setObjectName("labelMetric");
    nhanDocGiaBiKhoa = new QLabel("0");
    nhanDocGiaBiKhoa->setObjectName("valueRed");
    docGiaLayout->addWidget(lblBiKhoa, row, 0);
    docGiaLayout->addWidget(nhanDocGiaBiKhoa, row, 1);
    row++;
    
    // Đang mượn sách
    QLabel* lblDangMuonSach = new QLabel("📖 Đang mượn sách:");
    lblDangMuonSach->setObjectName("labelMetric");
    nhanDocGiaDangMuon = new QLabel("0");
    nhanDocGiaDangMuon->setObjectName("valueBlue");
    docGiaLayout->addWidget(lblDangMuonSach, row, 0);
    docGiaLayout->addWidget(nhanDocGiaDangMuon, row, 1);
    row++;
    
    // Tổng sách đang mượn
    QLabel* lblTongSachMuon = new QLabel("📚 Tổng sách đang mượn:");
    lblTongSachMuon->setObjectName("labelMetric");
    nhanTongSachDangMuon = new QLabel("0");
    nhanTongSachDangMuon->setObjectName("valueMetric");
    docGiaLayout->addWidget(lblTongSachMuon, row, 0);
    docGiaLayout->addWidget(nhanTongSachDangMuon, row, 1);
    row++;
    
    // Có sách quá hạn
    QLabel* lblQuaHan = new QLabel("⚠️ Có sách quá hạn:");
    lblQuaHan->setObjectName("labelMetric");
    nhanDocGiaCoSachQuaHan = new QLabel("0");
    nhanDocGiaCoSachQuaHan->setObjectName("valueOrange");
    docGiaLayout->addWidget(lblQuaHan, row, 0);
    docGiaLayout->addWidget(nhanDocGiaCoSachQuaHan, row, 1);
    
    docGiaLayout->setColumnStretch(0, 3);
    docGiaLayout->setColumnStretch(1, 1);
    columnsLayout->addWidget(docGiaGroup);
    
    mainLayout->addLayout(columnsLayout);
    
    // Button
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    nutDong = new QPushButton("✖️ Đóng");
    nutDong->setObjectName("primaryButton");
    nutDong->setMinimumWidth(150);
    connect(nutDong, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(nutDong);
    
    mainLayout->addLayout(buttonLayout);
}

void ManHinhThongKe::apDungPhongCach() {
    setStyleSheet(R"(
        QGroupBox#statsGroup {
            font-size: 16px;
            font-weight: bold;
            color: #2c3e50;
            border: 2px solid #3498db;
            border-radius: 10px;
            margin-top: 15px;
            padding-top: 10px;
            background-color: white;
        }
        
        QGroupBox#statsGroup::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 5px 15px;
            background-color: #3498db;
            color: white;
            border-radius: 5px;
            margin-left: 10px;
        }
        
        QLabel#labelMetric {
            font-size: 14px;
            color: #34495e;
            padding: 5px;
        }
        
        QLabel#valueMetric {
            font-size: 18px;
            font-weight: bold;
            color: #2c3e50;
            padding: 5px 10px;
            background-color: #ecf0f1;
            border-radius: 5px;
        }
        
        QLabel#valueGreen {
            font-size: 18px;
            font-weight: bold;
            color: #27ae60;
            padding: 5px 10px;
            background-color: #d5f4e6;
            border-radius: 5px;
        }
        
        QLabel#valueBlue {
            font-size: 18px;
            font-weight: bold;
            color: #3498db;
            padding: 5px 10px;
            background-color: #d6eaf8;
            border-radius: 5px;
        }
        
        QLabel#valueRed {
            font-size: 18px;
            font-weight: bold;
            color: #e74c3c;
            padding: 5px 10px;
            background-color: #fadbd8;
            border-radius: 5px;
        }
        
        QLabel#valueOrange {
            font-size: 18px;
            font-weight: bold;
            color: #e67e22;
            padding: 5px 10px;
            background-color: #fdebd0;
            border-radius: 5px;
        }
        
        QPushButton#primaryButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498db, stop:1 #2980b9);
            color: white;
            border: none;
            border-radius: 5px;
            padding: 12px 30px;
            font-weight: bold;
            font-size: 14px;
        }
        
        QPushButton#primaryButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #5dade2, stop:1 #3498db);
        }
        
        QPushButton#primaryButton:pressed {
            background: #2980b9;
        }
    )");
}

void ManHinhThongKe::capNhatThongKe() {
    // Lấy thống kê sách
    ThongKeSach tkSach = LayThongKeSach();
    
    nhanTongDauSach->setText(QString::number(tkSach.tongSoDauSach));
    nhanTongBanSao->setText(QString::number(tkSach.tongSoBanSao));
    nhanBanSaoCoTheMuon->setText(QString::number(tkSach.soBanSaoCoTheMuon));
    nhanBanSaoDangMuon->setText(QString::number(tkSach.soBanSaoDangMuon));
    nhanBanSaoThanhLy->setText(QString::number(tkSach.soBanSaoThanhLy));
    nhanTyLeChoMuon->setText(QString::number(tkSach.tyLeChoMuon, 'f', 2) + "%");
    nhanSoTheLoai->setText(QString::number(tkSach.soTheLoaiKhacNhau));
    
    // Lấy thống kê độc giả
    ThongKeDocGia tkDocGia = LayThongKeDocGia(cayDocGia, danhSachDauSach, soLuong);
    
    nhanTongDocGia->setText(QString::number(tkDocGia.tongSoDocGia));
    nhanDocGiaHoatDong->setText(QString::number(tkDocGia.docGiaHoatDong));
    nhanDocGiaBiKhoa->setText(QString::number(tkDocGia.docGiaBiKhoa));
    nhanDocGiaDangMuon->setText(QString::number(tkDocGia.docGiaDangMuonSach));
    nhanTongSachDangMuon->setText(QString::number(tkDocGia.tongSoSachDangMuon));
    nhanDocGiaCoSachQuaHan->setText(QString::number(tkDocGia.docGiaCoSachQuaHan));
}
