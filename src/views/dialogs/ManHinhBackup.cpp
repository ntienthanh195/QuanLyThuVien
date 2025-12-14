#include "views/dialogs/ManHinhBackup.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QDateTime>
#include "utils/LanguageManager.h"

ManHinhBackup::ManHinhBackup(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle(LanguageManager::instance().getText("TITLE_BACKUP_MANAGER"));
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
    QLabel* titleLabel = new QLabel(LanguageManager::instance().getText("HEADER_BACKUP"));
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
        LanguageManager::instance().getText("MSG_BACKUP_GUIDE_HTML")
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
    
    nutTaoBackup = new QPushButton(LanguageManager::instance().getText("BTN_CREATE_BACKUP"));
    nutTaoBackup->setObjectName("btnCreate");
    
    nutPhucHoi = new QPushButton(LanguageManager::instance().getText("BTN_RESTORE_BACKUP"));
    nutPhucHoi->setObjectName("btnRestore");
    nutPhucHoi->setEnabled(false);
    
    nutXoa = new QPushButton(LanguageManager::instance().getText("BTN_DELETE_BACKUP"));
    nutXoa->setObjectName("btnDelete");
    nutXoa->setEnabled(false);
    
    nutXoaCu = new QPushButton(LanguageManager::instance().getText("BTN_CLEANUP_BACKUP"));
    nutXoaCu->setObjectName("btnCleanup");
    
    nutLamMoi = new QPushButton(LanguageManager::instance().getText("BTN_REFRESH"));
    nutLamMoi->setObjectName("btnRefresh");
    
    nutDong = new QPushButton(LanguageManager::instance().getText("BTN_CLOSE"));
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
        nhanThongTin->setText(LanguageManager::instance().getText("MSG_NO_BACKUP"));
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
        
        QString item = QString("%1 | %2 | %3")
            .arg(QString::fromStdString(backup.tenFile))
            .arg(QString::fromStdString(backup.ngayTao))
            .arg(sizeStr);
        
        danhSachBackup->addItem(item);
    }
    
    nhanThongTin->setText(LanguageManager::instance().getText("MSG_BACKUP_FOUND").arg(cacBackup.size()));
    nhanThongTin->setStyleSheet("color: #27ae60; font-weight: bold;");
}

void ManHinhBackup::onTaoBackupClicked() {
    std::string loi = TaoBackup();
    
    if (loi.empty()) {
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_BACKUP_SUCCESS"),
            LanguageManager::instance().getText("MSG_BACKUP_SUCCESS"));
        capNhatDanhSachBackup();
    } else {
        QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_ERROR"),
            LanguageManager::instance().getText("MSG_ERROR_CREATE_BACKUP_FAILED").arg(QString::fromStdString(loi)));
    }
}

void ManHinhBackup::onPhucHoiClicked() {
    int row = danhSachBackup->currentRow();
    if (row < 0 || row >= (int)cacBackup.size()) return;
    
    const ThongTinBackup& backup = cacBackup[row];
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        LanguageManager::instance().getText("BTN_RESTORE_BACKUP"), // Using Button Title as Window Title or generic "Confirm"
        LanguageManager::instance().getText("MSG_CONFIRM_RESTORE")
            .arg(QString::fromStdString(backup.tenFile))
            .arg(QString::fromStdString(backup.ngayTao)),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        std::string loi = PhucHoiBackup(backup.duongDanDayDu);
        
        if (loi.empty()) {
            QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"),
                LanguageManager::instance().getText("MSG_RESTORE_SUCCESS"));
        } else {
            QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_ERROR"),
                LanguageManager::instance().getText("MSG_ERROR_RESTORE_FAILED").arg(QString::fromStdString(loi)));
        }
    }
}

void ManHinhBackup::onXoaClicked() {
    int row = danhSachBackup->currentRow();
    if (row < 0 || row >= (int)cacBackup.size()) return;
    
    const ThongTinBackup& backup = cacBackup[row];
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        LanguageManager::instance().getText("BTN_DELETE_BACKUP"),
        LanguageManager::instance().getText("MSG_CONFIRM_DELETE_BACKUP")
            .arg(QString::fromStdString(backup.tenFile))
            .arg(QString::fromStdString(backup.ngayTao)),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        std::string loi = XoaBackup(backup.duongDanDayDu);
        
        if (loi.empty()) {
            nhanThongTin->setText(LanguageManager::instance().getText("MSG_DELETE_SUCCESS"));
            nhanThongTin->setStyleSheet("color: #27ae60; font-weight: bold;");
            capNhatDanhSachBackup();
        } else {
            QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_ERROR"),
                LanguageManager::instance().getText("MSG_ERROR_DELETE_FAILED").arg(QString::fromStdString(loi)));
        }
    }
}

void ManHinhBackup::onXoaCuClicked() {
    bool ok;
    int soGiuLai = QInputDialog::getInt(this,
        LanguageManager::instance().getText("TITLE_CLEANUP"),
        LanguageManager::instance().getText("MSG_CLEANUP_PROMPT"),
        5, 1, 100, 1, &ok);
    
    if (ok) {
        int soDaXoa = XoaBackupCu(soGiuLai);
        
        if (soDaXoa >= 0) {
            QMessageBox::information(this, LanguageManager::instance().getText("TITLE_CLEANUP"),
                LanguageManager::instance().getText("MSG_CLEANUP_SUCCESS").arg(soDaXoa).arg(soGiuLai));
            capNhatDanhSachBackup();
        } else {
            QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_ERROR"), LanguageManager::instance().getText("MSG_ERROR_CLEANUP_FAILED"));
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
