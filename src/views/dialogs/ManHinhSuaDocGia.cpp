#include "views/dialogs/ManHinhSuaDocGia.h"
#include "models/DocGia.h"
#include "utils/Utf8Helper.h"
#include "models/KiemTraDuLieu.h"
#include "utils/XuLyChuoi.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QGroupBox>

extern PTRDG rootDocGia;

#include "utils/LanguageManager.h"

ManHinhSuaDocGia::ManHinhSuaDocGia(const QString& readerCode, QWidget *parent)
    : QDialog(parent), maDocGia(readerCode), docGiaHienTai(nullptr)
    , nhapMaThe(nullptr)
    , nhapHo(nullptr)
    , nhapTen(nullptr)
    , chonGioiTinh(nullptr)
    , chonTrangThai(nullptr)
    , nhanSoSachDangMuon(nullptr)
    , nutLuu(nullptr)
    , nutHuy(nullptr)
{
    setWindowTitle(LanguageManager::instance().getText("TITLE_EDIT_READER") + " 👤");
    setMinimumWidth(500);
    setModal(true);

    // Find reader
    bool ok;
    int maThe = readerCode.toInt(&ok);
    if (ok) {
        docGiaHienTai = timDocGia(rootDocGia, maThe);
    }
    
    thietLapGiaoDien();
    apDungPhongCachHienDai();
    taiDuLieuDocGia();
}

void ManHinhSuaDocGia::thietLapGiaoDien() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel* titleLabel = new QLabel(LanguageManager::instance().getText("DIALOG_EDIT_READER_TITLE"));
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // Form group
    QGroupBox* formGroup = new QGroupBox(LanguageManager::instance().getText("GROUP_READER_INFO"), this);
    formGroup->setObjectName("formGroup");
    QFormLayout* formLayout = new QFormLayout(formGroup);
    formLayout->setSpacing(12);
    formLayout->setContentsMargins(15, 15, 15, 15);

    // Reader code (read-only)
    nhapMaThe = new QLineEdit();
    nhapMaThe->setReadOnly(true);
    nhapMaThe->setStyleSheet("background-color: #ecf0f1; color: #7f8c8d; font-weight: bold;");
    formLayout->addRow(LanguageManager::instance().getText("TABLE_HEADER_CODE") + ":", nhapMaThe);

    QLabel* noteLabel = new QLabel(LanguageManager::instance().getText("MSG_CODE_READONLY"));
    noteLabel->setStyleSheet("color: #95a5a6; font-size: 11px; font-style: italic;");
    formLayout->addRow("", noteLabel);

    // Last name
    nhapHo = new QLineEdit();
    nhapHo->setPlaceholderText(LanguageManager::instance().getText("LBL_LASTNAME") + "...");
    nhapHo->setObjectName("inputField");
    connect(nhapHo, &QLineEdit::textChanged, this, &ManHinhSuaDocGia::kiemTraDuLieu);
    formLayout->addRow(LanguageManager::instance().getText("LBL_LASTNAME") + ": *", nhapHo);

    // First name
    nhapTen = new QLineEdit();
    nhapTen->setPlaceholderText(LanguageManager::instance().getText("LBL_FIRSTNAME") + "...");
    nhapTen->setObjectName("inputField");
    connect(nhapTen, &QLineEdit::textChanged, this, &ManHinhSuaDocGia::kiemTraDuLieu);
    formLayout->addRow(LanguageManager::instance().getText("LBL_FIRSTNAME") + ": *", nhapTen);

    // Gender
    chonGioiTinh = new QComboBox();
    chonGioiTinh->setObjectName("inputField");
    chonGioiTinh->addItem(LanguageManager::instance().getText("GENDER_MALE"), 0);
    chonGioiTinh->addItem(LanguageManager::instance().getText("GENDER_FEMALE"), 1);
    formLayout->addRow(LanguageManager::instance().getText("LBL_GENDER") + ":", chonGioiTinh);

    // Status
    chonTrangThai = new QComboBox();
    chonTrangThai->setObjectName("inputField");
    chonTrangThai->addItem(LanguageManager::instance().getText("STATUS_ACTIVE"), 0);
    chonTrangThai->addItem(LanguageManager::instance().getText("STATUS_LOCKED"), 1);
    formLayout->addRow(LanguageManager::instance().getText("LBL_STATUS") + ":", chonTrangThai);

    mainLayout->addWidget(formGroup);

    // Statistics group
    QGroupBox* statsGroup = new QGroupBox(LanguageManager::instance().getText("GROUP_STATS"));
    statsGroup->setObjectName("statsGroup");
    QVBoxLayout* statsLayout = new QVBoxLayout(statsGroup);
    statsLayout->setContentsMargins(15, 15, 15, 15);
    statsLayout->setSpacing(8);

    nhanSoSachDangMuon = new QLabel();
    nhanSoSachDangMuon->setStyleSheet(
        "font-size: 14px; "
        "padding: 10px; "
        "background-color: #e8f4f8; "
        "border-left: 4px solid #3498db; "
        "border-radius: 4px;"
    );
    statsLayout->addWidget(nhanSoSachDangMuon);

    QLabel* statsNote = new QLabel(LanguageManager::instance().getText("MSG_NOTE_CANNOT_LOCK"));
    statsNote->setStyleSheet("color: #7f8c8d; font-size: 11px; font-style: italic;");
    statsLayout->addWidget(statsNote);

    mainLayout->addWidget(statsGroup);

    // Info note
    QLabel* infoLabel = new QLabel(LanguageManager::instance().getText("LBL_REQUIRED_FIELDS_NOTE"));
    infoLabel->setStyleSheet("color: #7f8c8d; font-size: 12px; padding: 10px; background-color: #ecf0f1; border-radius: 5px;");
    mainLayout->addWidget(infoLabel);

    mainLayout->addStretch();

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    nutLuu = new QPushButton(LanguageManager::instance().getText("BTN_SAVE_CHANGES"));
    nutLuu->setObjectName("primaryButton");
    nutLuu->setMinimumHeight(40);
    connect(nutLuu, &QPushButton::clicked, this, &ManHinhSuaDocGia::khiNhanLuu);

    nutHuy = new QPushButton(LanguageManager::instance().getText("BTN_CANCEL_DIALOG"));
    nutHuy->setObjectName("nutHuy");
    nutHuy->setMinimumHeight(40);
    connect(nutHuy, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addStretch();
    buttonLayout->addWidget(nutLuu);
    buttonLayout->addWidget(nutHuy);

    mainLayout->addLayout(buttonLayout);
}

void ManHinhSuaDocGia::apDungPhongCachHienDai() {
    setStyleSheet(R"(
        QDialog {
            background-color: #ffffff;
        }
        
        QGroupBox#formGroup, QGroupBox#statsGroup {
            font-weight: bold;
            border: 2px solid #3498db;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
            background-color: #f8f9fa;
        }
        
        QGroupBox#formGroup::title, QGroupBox#statsGroup::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 5px;
            color: #3498db;
        }
        
        QLineEdit, QComboBox {
            padding: 8px;
            border: 2px solid #bdc3c7;
            border-radius: 5px;
            background-color: white;
            font-size: 13px;
        }
        
        QLineEdit#inputField:focus, QComboBox#inputField:focus {
            border: 2px solid #3498db;
            background-color: #ecf8ff;
        }
        
        QComboBox::drop-down {
            border: none;
            width: 25px;
            background-color: #3498db;
        }
        
        QComboBox::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 6px solid white;
            margin-right: 8px;
        }
        
        QPushButton#primaryButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #f39c12, stop:1 #e67e22);
            color: white;
            border: none;
            border-radius: 5px;
            padding: 10px 30px;
            font-weight: bold;
            font-size: 14px;
            min-width: 150px;
        }
        
        QPushButton#primaryButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #f1c40f, stop:1 #f39c12);
        }
        
        QPushButton#primaryButton:pressed {
            background: #e67e22;
        }
        
        QPushButton#primaryButton:disabled {
            background: #bdc3c7;
            color: #7f8c8d;
        }
        
        QPushButton#nutHuy {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #e74c3c, stop:1 #c0392b);
            color: white;
            border: none;
            border-radius: 5px;
            padding: 10px 30px;
            font-weight: bold;
            font-size: 14px;
            min-width: 100px;
        }
        
        QPushButton#nutHuy:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #ec7063, stop:1 #e74c3c);
        }
        
        QPushButton#nutHuy:pressed {
            background: #c0392b;
        }
    )");
}

void ManHinhSuaDocGia::taiDuLieuDocGia() {
    if (!docGiaHienTai) {
        QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_ERROR"), 
            LanguageManager::instance().getText("MSG_READER_NOT_FOUND"));
        reject();
        return;
    }

    nhapMaThe->setText(QString::number(docGiaHienTai->data.MaThe));
    nhapHo->setText(QString::fromStdString(docGiaHienTai->data.Ho));
    nhapTen->setText(QString::fromStdString(docGiaHienTai->data.Ten));
    
    chonGioiTinh->setCurrentIndex(docGiaHienTai->data.Phai);
    chonTrangThai->setCurrentIndex(docGiaHienTai->data.TrangThai == 1 ? 0 : 1);

    // Update book count label
    int borrowedCount = docGiaHienTai->data.soSachDangMuon;
    nhanSoSachDangMuon->setText(
        LanguageManager::instance().getText("LBL_BORROWED_COUNT").arg(borrowedCount)
    );

    // Disable status change if reader has borrowed books
    if (borrowedCount > 0) {
        chonTrangThai->setEnabled(false);
         chonTrangThai->setToolTip(LanguageManager::instance().getText("MSG_CANNOT_LOCK_BORROWING").arg(borrowedCount));
    }

    kiemTraDuLieu();
}

void ManHinhSuaDocGia::kiemTraDuLieu() {
    bool valid = true;

    // Check last name
    QString lastName = nhapHo->text().trimmed();
    if (lastName.isEmpty()) {
        nhapHo->setStyleSheet("QLineEdit { border: 2px solid #e74c3c; background-color: #ffe5e5; }");
        valid = false;
    } else {
        nhapHo->setStyleSheet("QLineEdit { border: 2px solid #27ae60; background-color: #e8f8f5; }");
    }

    // Check first name
    QString firstName = nhapTen->text().trimmed();
    if (firstName.isEmpty()) {
        nhapTen->setStyleSheet("QLineEdit { border: 2px solid #e74c3c; background-color: #ffe5e5; }");
        valid = false;
    } else {
        nhapTen->setStyleSheet("QLineEdit { border: 2px solid #27ae60; background-color: #e8f8f5; }");
    }

    nutLuu->setEnabled(valid);
}

void ManHinhSuaDocGia::khiNhanLuu() {
    if (!docGiaHienTai) {
        QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_ERROR"), 
            LanguageManager::instance().getText("MSG_READER_NOT_FOUND"));
        return;
    }

    // Get values
    QString lastName = Utf8Helper::fromStdString(ChuyenThanhTitleCaseUnicode(Utf8Helper::toStdString(nhapHo->text().trimmed())));
    QString firstName = Utf8Helper::fromStdString(ChuyenThanhTitleCaseUnicode(Utf8Helper::toStdString(nhapTen->text().trimmed())));
    int genderValue = chonGioiTinh->currentData().toInt();
    int statusValue = chonTrangThai->currentData().toInt();

    // Validate họ đệm
    if (lastName.isEmpty()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"),
            LanguageManager::instance().getText("MSG_MISSING_LASTNAME"));
        nhapHo->setFocus();
        return;
    }
    
    std::string loiHo = KiemTraTenNguoi(lastName.toStdString(), "Họ đệm");
    if (!loiHo.empty()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_ERROR_NAME"),
            QString::fromStdString(loiHo) + LanguageManager::instance().getText("MSG_ERROR_NAME_FORMAT_HINT"));
        nhapHo->setFocus();
        return;
    }

    // Validate tên
    if (firstName.isEmpty()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"),
            LanguageManager::instance().getText("MSG_MISSING_FIRSTNAME"));
        nhapTen->setFocus();
        return;
    }
    
    std::string loiTen = KiemTraTenNguoi(firstName.toStdString(), "Tên");
    if (!loiTen.empty()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_ERROR_FIRSTNAME"),
            QString::fromStdString(loiTen) + LanguageManager::instance().getText("MSG_ERROR_FIRSTNAME_FORMAT_HINT"));
        nhapTen->setFocus();
        return;
    }

    // Check if trying to lock reader with borrowed books
    int borrowedCount = docGiaHienTai->data.soSachDangMuon;
    int targetStatus = statusValue == 0 ? 1 : 0; // Convert from combo index to actual status
    
    if (targetStatus == 0 && borrowedCount > 0) {
        QMessageBox::warning(this, LanguageManager::instance().getText("MSG_WARNING_LOCK"),
            LanguageManager::instance().getText("MSG_CANNOT_LOCK_BORROWING").arg(borrowedCount));
        return;
    }

    // Update reader data
    try {
        TheDocGia updatedData = docGiaHienTai->data;
        updatedData.Ho = lastName.toStdString();
        updatedData.Ten = firstName.toStdString();
        updatedData.Phai = genderValue;
        updatedData.TrangThai = targetStatus;

        hieuChinhDocGia(rootDocGia, updatedData);
        emit docGiaDaCapNhat();
        
        QString genderStr = genderValue == 0 ? LanguageManager::instance().getText("GENDER_MALE") : LanguageManager::instance().getText("GENDER_FEMALE");
        QString statusStr = targetStatus == 1 ? LanguageManager::instance().getText("STATUS_ACTIVE") : LanguageManager::instance().getText("STATUS_LOCKED");
        
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"),
            QString("✅ %1\n\n"
                   "🎫 %2: %3\n"
                   "👤 %4: %5 %6\n"
                   "⚧ %7: %8\n"
                   "📊 %9: %10")
                .arg(LanguageManager::instance().getText("MSG_SUCCESS_UPDATE"))
                .arg(LanguageManager::instance().getText("TABLE_HEADER_CODE"))
                .arg(docGiaHienTai->data.MaThe)
                .arg(LanguageManager::instance().getText("LBL_LASTNAME") + " " + LanguageManager::instance().getText("LBL_FIRSTNAME"))
                .arg(lastName)
                .arg(firstName)
                .arg(LanguageManager::instance().getText("LBL_GENDER"))
                .arg(genderStr)
                .arg(LanguageManager::instance().getText("LBL_STATUS"))
                .arg(statusStr));
        
        accept();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_ERROR_SYSTEM"),
            LanguageManager::instance().getText("MSG_ERROR_UPDATE_FAILED").arg(e.what()));
    } catch (...) {
        QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_ERROR_UNKNOWN"),
            LanguageManager::instance().getText("MSG_ERROR_UNKNOWN_ACTION"));
    }
}
