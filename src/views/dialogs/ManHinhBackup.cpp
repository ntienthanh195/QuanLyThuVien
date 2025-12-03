#include "views/dialogs/ManHinhBackup.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QDateTime>

ManHinhBackup::ManHinhBackup(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Quản Lý Sao Lưu");
    setMinimumSize(700, 500);
    setModal(true);
    
    thietLapGiaoDien();
    apDungPhongCach();
    capNhatDanhSachBackup();
}

void ManHinhBackup::thietLapGiaoDien() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Title
    QLabel* titleLabel = new QLabel("QUẢN LÝ SAO LƯU DỮ LIỆU");
    titleLabel->setStyleSheet(
        "font-size: 20px; "
        "font-weight: bold; "
        "color: white; "
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1e40af, stop:1 #059669); "
        "padding: 15px; "
        "border-radius: 8px;"
    );
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Info label
    QLabel* infoLabel = new QLabel(
        "<b>Hướng dẫn:</b><br>"
        "• <b>Tạo Backup:</b> Sao lưu toàn bộ dữ liệu hiện tại<br>"
        "• <b>Phục hồi:</b> Khôi phục dữ liệu từ bản sao lưu đã chọn<br>"
        "• <b>Xóa:</b> Xóa bản sao lưu đã chọn<br>"
        "• <b>Xóa cũ:</b> Xóa các bản sao lưu cũ, giữ lại n bản mới nhất"
    );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet(
        "background-color: #e8f4f8; "
        "padding: 10px; "
        "border-left: 4px solid #3498db; "
        "border-radius: 4px; "
        "color: #2c3e50;"
    );
    mainLayout->addWidget(infoLabel);
    
    // List widget
    danhSachBackup = new QListWidget();
    danhSachBackup->setAlternatingRowColors(true);
    danhSachBackup->setStyleSheet(
        "QListWidget { "
        "   border: 2px solid #3498db; "
        "   border-radius: 5px; "
        "   background-color: white; "
        "   font-size: 13px; "
        "}"
        "QListWidget::item { "
        "   padding: 8px; "
        "}"
        "QListWidget::item:selected { "
        "   background-color: #3498db; "
        "   color: white; "
        "}"
        "QListWidget::item:hover { "
        "   background-color: #d6eaf8; "
        "}"
    );
    mainLayout->addWidget(danhSachBackup);
    
    // Status label
    nhanThongTin = new QLabel("");
    nhanThongTin->setStyleSheet("color: #27ae60; font-weight: bold; font-size: 12px;");
    nhanThongTin->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(nhanThongTin);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    nutTaoBackup = new QPushButton("Tạo Backup");
    nutTaoBackup->setObjectName("btnCreate");
    
    nutPhucHoi = new QPushButton("Phục Hồi");
    nutPhucHoi->setObjectName("btnRestore");
    nutPhucHoi->setEnabled(false);
    
    nutXoa = new QPushButton("Xóa");
    nutXoa->setObjectName("btnDelete");
    nutXoa->setEnabled(false);
    
    nutXoaCu = new QPushButton("Xóa Cũ");
    nutXoaCu->setObjectName("btnCleanup");
    
    nutLamMoi = new QPushButton("Làm Mới");
    nutLamMoi->setObjectName("btnRefresh");
    
    nutDong = new QPushButton("✖️ Đóng");
    nutDong->setObjectName("btnClose");
    
    buttonLayout->addWidget(nutTaoBackup);
    buttonLayout->addWidget(nutPhucHoi);
    buttonLayout->addWidget(nutXoa);
    buttonLayout->addWidget(nutXoaCu);
    buttonLayout->addWidget(nutLamMoi);
    buttonLayout->addStretch();
    buttonLayout->addWidget(nutDong);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connections
    connect(nutTaoBackup, &QPushButton::clicked, this, &ManHinhBackup::onTaoBackupClicked);
    connect(nutPhucHoi, &QPushButton::clicked, this, &ManHinhBackup::onPhucHoiClicked);
    connect(nutXoa, &QPushButton::clicked, this, &ManHinhBackup::onXoaClicked);
    connect(nutXoaCu, &QPushButton::clicked, this, &ManHinhBackup::onXoaCuClicked);
    connect(nutLamMoi, &QPushButton::clicked, this, &ManHinhBackup::onLamMoiClicked);
    connect(nutDong, &QPushButton::clicked, this, &QDialog::accept);
    connect(danhSachBackup, &QListWidget::itemSelectionChanged, this, &ManHinhBackup::onBackupSelected);
}

void ManHinhBackup::apDungPhongCach() {
    setStyleSheet(R"(
        QPushButton {
            padding: 10px 20px;
            font-weight: bold;
            font-size: 13px;
            border: none;
            border-radius: 5px;
            min-width: 100px;
        }
        
        QPushButton#btnCreate {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #27ae60, stop:1 #229954);
            color: white;
        }
        
        QPushButton#btnCreate:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2ecc71, stop:1 #27ae60);
        }
        
        QPushButton#btnRestore {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3498db, stop:1 #2980b9);
            color: white;
        }
        
        QPushButton#btnRestore:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5dade2, stop:1 #3498db);
        }
        
        QPushButton#btnRestore:disabled {
            background: #bdc3c7;
            color: #7f8c8d;
        }
        
        QPushButton#btnDelete {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e74c3c, stop:1 #c0392b);
            color: white;
        }
        
        QPushButton#btnDelete:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ec7063, stop:1 #e74c3c);
        }
        
        QPushButton#btnDelete:disabled {
            background: #bdc3c7;
            color: #7f8c8d;
        }
        
        QPushButton#btnCleanup, QPushButton#btnRefresh {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f39c12, stop:1 #e67e22);
            color: white;
        }
        
        QPushButton#btnCleanup:hover, QPushButton#btnRefresh:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f5b041, stop:1 #f39c12);
        }
        
        QPushButton#btnClose {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #95a5a6, stop:1 #7f8c8d);
            color: white;
        }
        
        QPushButton#btnClose:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #b2babb, stop:1 #95a5a6);
        }
    )");
}

void ManHinhBackup::capNhatDanhSachBackup() {
    danhSachBackup->clear();
    ThongTinBackup cacBackupArr[1000];
    int soBackup = LayDanhSachBackup(cacBackupArr, 1000);
    cacBackup.clear();
    for (int i = 0; i < soBackup; ++i) {
        cacBackup.push_back(cacBackupArr[i]);
    }
    
    if (cacBackup.empty()) {
        nhanThongTin->setText("📭 Chưa có bản sao lưu nào");
        nhanThongTin->setStyleSheet("color: #95a5a6; font-weight: bold;");
        return;
    }
    
    for (const auto& backup : cacBackup) {
        double sizeKB = backup.kichThuoc / 1024.0;
        QString sizeStr;
        if (sizeKB >= 1024) {
            sizeStr = QString::number(sizeKB / 1024.0, 'f', 2) + " MB";
        } else {
            sizeStr = QString::number(sizeKB, 'f', 1) + " KB";
        }
        
        QString item = QString("📁 %1 | 📅 %2 | 💾 %3")
            .arg(QString::fromStdString(backup.tenFile))
            .arg(QString::fromStdString(backup.ngayTao))
            .arg(sizeStr);
        
        danhSachBackup->addItem(item);
    }
    
    nhanThongTin->setText(QString("✅ Tìm thấy %1 bản sao lưu").arg(cacBackup.size()));
    nhanThongTin->setStyleSheet("color: #27ae60; font-weight: bold;");
}

void ManHinhBackup::onTaoBackupClicked() {
    std::string loi = TaoBackup();
    
    if (loi.empty()) {
        QMessageBox::information(this, "Thành công 🎉",
            "✅ Đã tạo bản sao lưu thành công!\n\n"
            "💾 Dữ liệu đã được sao lưu an toàn.");
        capNhatDanhSachBackup();
    } else {
        QMessageBox::critical(this, "Lỗi ❌",
            QString("Không thể tạo backup:\n%1").arg(QString::fromStdString(loi)));
    }
}

void ManHinhBackup::onPhucHoiClicked() {
    int row = danhSachBackup->currentRow();
    if (row < 0 || row >= (int)cacBackup.size()) return;
    
    const ThongTinBackup& backup = cacBackup[row];
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Xác nhận phục hồi ⚠️",
        QString("Bạn có chắc muốn phục hồi từ:\n\n"
               "📁 %1\n"
               "📅 %2\n\n"
               "⚠️ Dữ liệu hiện tại sẽ bị ghi đè!")
            .arg(QString::fromStdString(backup.tenFile))
            .arg(QString::fromStdString(backup.ngayTao)),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        std::string loi = PhucHoiBackup(backup.duongDanDayDu);
        
        if (loi.empty()) {
            QMessageBox::information(this, "Thành công 🎉",
                "✅ Đã phục hồi dữ liệu thành công!\n\n"
                "🔄 Vui lòng tải lại chương trình để áp dụng thay đổi.");
        } else {
            QMessageBox::critical(this, "Lỗi ❌",
                QString("Không thể phục hồi:\n%1").arg(QString::fromStdString(loi)));
        }
    }
}

void ManHinhBackup::onXoaClicked() {
    int row = danhSachBackup->currentRow();
    if (row < 0 || row >= (int)cacBackup.size()) return;
    
    const ThongTinBackup& backup = cacBackup[row];
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Xác nhận xóa ⚠️",
        QString("Bạn có chắc muốn xóa bản sao lưu:\n\n"
               "📁 %1\n"
               "📅 %2")
            .arg(QString::fromStdString(backup.tenFile))
            .arg(QString::fromStdString(backup.ngayTao)),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        std::string loi = XoaBackup(backup.duongDanDayDu);
        
        if (loi.empty()) {
            nhanThongTin->setText("✅ Đã xóa bản sao lưu");
            nhanThongTin->setStyleSheet("color: #27ae60; font-weight: bold;");
            capNhatDanhSachBackup();
        } else {
            QMessageBox::critical(this, "Lỗi ❌",
                QString("Không thể xóa:\n%1").arg(QString::fromStdString(loi)));
        }
    }
}

void ManHinhBackup::onXoaCuClicked() {
    bool ok;
    int soGiuLai = QInputDialog::getInt(this,
        "Xóa backup cũ 🧹",
        "Nhập số bản sao lưu muốn giữ lại (mới nhất):",
        5, 1, 100, 1, &ok);
    
    if (ok) {
        int soDaXoa = XoaBackupCu(soGiuLai);
        
        if (soDaXoa >= 0) {
            QMessageBox::information(this, "Thành công 🎉",
                QString("✅ Đã xóa %1 bản cũ!\n\n"
                       "📦 Giữ lại %2 bản mới nhất.").arg(soDaXoa).arg(soGiuLai));
            capNhatDanhSachBackup();
        } else {
            QMessageBox::critical(this, "Lỗi ❌", "Không thể xóa các bản cũ!");
        }
    }
}

void ManHinhBackup::onLamMoiClicked() {
    capNhatDanhSachBackup();
}

void ManHinhBackup::onBackupSelected() {
    bool hasSelection = (danhSachBackup->currentRow() >= 0);
    nutPhucHoi->setEnabled(hasSelection);
    nutXoa->setEnabled(hasSelection);
}
