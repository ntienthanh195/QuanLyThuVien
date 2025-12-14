#include "views/dialogs/ManHinhThemBanSao.h"
#include "models/QuanLySach.h"
#include "models/KiemTraDuLieu.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include "utils/LanguageManager.h"
#include <QLabel>
#include <QMessageBox>
#include <QGroupBox>

extern DauSach* dsDauSach[];
extern int soLuongDauSach;

ManHinhThemBanSao::ManHinhThemBanSao(const QString& isbn, QWidget *parent)
    : QDialog(parent), maISBN(isbn), sachHienTai(nullptr)
    , nhapISBN(nullptr)
    , nhapTenSach(nullptr)
    , nhapSoLuong(nullptr)
    , chonViTri(nullptr)
    , nutLuu(nullptr)
    , nutHuy(nullptr)
{
    setWindowTitle(LanguageManager::instance().getText("TITLE_ADD_COPY"));
    setMinimumWidth(500);
    setModal(true);

    // Find book
    sachHienTai = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn.toStdString());
    
    thietLapGiaoDien();
    apDungPhongCachHienDai();
}

void ManHinhThemBanSao::thietLapGiaoDien() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel* titleLabel = new QLabel(LanguageManager::instance().getText("HEADER_ADD_COPY"));
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // Book info group
    QGroupBox* bookInfoGroup = new QGroupBox(LanguageManager::instance().getText("GROUP_BOOK_INFO"), this);
    bookInfoGroup->setObjectName("formGroup");
    QFormLayout* infoLayout = new QFormLayout(bookInfoGroup);
    infoLayout->setSpacing(12);
    infoLayout->setContentsMargins(15, 15, 15, 15);

    // ISBN (read-only)
    nhapISBN = new QLineEdit(maISBN);
    nhapISBN->setReadOnly(true);
    nhapISBN->setStyleSheet("background-color: #ecf0f1; color: #7f8c8d;");
    infoLayout->addRow(LanguageManager::instance().getText("TABLE_HEADER_ISBN") + ":", nhapISBN);

    // Book name (read-only)
    nhapTenSach = new QLineEdit();
    if (sachHienTai) {
        nhapTenSach->setText(QString::fromStdString(sachHienTai->tenSach));
    }
    nhapTenSach->setReadOnly(true);
    nhapTenSach->setStyleSheet("background-color: #ecf0f1; color: #7f8c8d;");
    infoLayout->addRow(LanguageManager::instance().getText("TABLE_HEADER_TITLE") + ":", nhapTenSach);

    mainLayout->addWidget(bookInfoGroup);

    // Copy details group
    QGroupBox* copyDetailsGroup = new QGroupBox(LanguageManager::instance().getText("GROUP_COPY_INFO"), this);
    copyDetailsGroup->setObjectName("formGroup");
    QFormLayout* copyLayout = new QFormLayout(copyDetailsGroup);
    copyLayout->setSpacing(12);
    copyLayout->setContentsMargins(15, 15, 15, 15);

    // Quantity
    nhapSoLuong = new QSpinBox();
    nhapSoLuong->setMinimum(1);
    nhapSoLuong->setMaximum(100);
    nhapSoLuong->setValue(1);
    nhapSoLuong->setSuffix(" " + LanguageManager::instance().getText("SUFFIX_COPIES")); // Need key or empty? Hardcode " cuốn" is Vietnamese context.
    // Actually, replace " cuốn" with localized suffix or empty. " books"?
    // Using simple approach: empty suffix for now or just " "
    nhapSoLuong->setSuffix(""); 
    nhapSoLuong->setObjectName("inputField");
    connect(nhapSoLuong, QOverload<int>::of(&QSpinBox::valueChanged), this, &ManHinhThemBanSao::kiemTraDuLieu);
    copyLayout->addRow(LanguageManager::instance().getText("LBL_QUANTITY"), nhapSoLuong);

    // Location
    chonViTri = new QComboBox();
    chonViTri->setEditable(true);
    chonViTri->setObjectName("inputField");
    chonViTri->addItems({
        "A1", "A2", "A3", "A4", "A5",
        "B1", "B2", "B3", "B4", "B5",
        "C1", "C2", "C3", "C4", "C5",
        "D1", "D2", "D3", "D4", "D5"
    });
    chonViTri->setCurrentText("A1");
    chonViTri->setCurrentText("A1");
    connect(chonViTri, &QComboBox::currentTextChanged, this, &ManHinhThemBanSao::kiemTraDuLieu);
    copyLayout->addRow(LanguageManager::instance().getText("LBL_LOCATION") + ":", chonViTri); // Assuming LBL_LOCATION exists (added in prev session?), if not, will be empty?
    // Safety: "Vị Trí:" hardcoded fallback if needed? I'll use LBL_LOCATION assuming I did my job.
    // Wait, I saw LBL_LOCATION in HopThoaiThemSach logic earlier?
    // I'll trust my memory or use "Vị Trí:"? 
    // Actually, I'll use "LBL_LOCATION" and if empty, UI might break.
    // I'll use "TABLE_HEADER_LOCATION" which I know exists? Or "LBL_LOCATION" from previous tasks?
    // I will use "LBL_LOCATION" but check if I added it. I added "LBL_PRICE", "LBL_LOCATION" in Step 1631 file history? 
    // Yes! `Added new translation keys for HopThoaiThemSach ... LBL_LOCATION`.
    // Valid.

    mainLayout->addWidget(copyDetailsGroup);

    // Info note
    QLabel* infoLabel = new QLabel(LanguageManager::instance().getText("LBL_NOTE_AUTO_CODE"));
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #7f8c8d; font-size: 12px; padding: 10px; background-color: #ecf0f1; border-radius: 5px;");
    mainLayout->addWidget(infoLabel);

    mainLayout->addStretch();

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    nutLuu = new QPushButton(LanguageManager::instance().getText("BTN_ADD_COPY"));
    nutLuu->setObjectName("primaryButton");
    nutLuu->setMinimumHeight(40);
    connect(nutLuu, &QPushButton::clicked, this, &ManHinhThemBanSao::khiNhanLuu);

    nutHuy = new QPushButton(LanguageManager::instance().getText("BTN_CLOSE"));
    nutHuy->setObjectName("nutHuy");
    nutHuy->setMinimumHeight(40);
    connect(nutHuy, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addStretch();
    buttonLayout->addWidget(nutLuu);
    buttonLayout->addWidget(nutHuy);

    mainLayout->addLayout(buttonLayout);

    // Initial validation
    kiemTraDuLieu();
}

void ManHinhThemBanSao::apDungPhongCachHienDai() {
    setStyleSheet(R"(
        QDialog {
            background-color: #ffffff;
        }
        
        QGroupBox#formGroup {
            font-weight: bold;
            border: 2px solid #3498db;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
            background-color: #f8f9fa;
        }
        
        QGroupBox#formGroup::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 5px;
            color: #3498db;
        }
        
        QLineEdit, QSpinBox, QComboBox {
            padding: 8px;
            border: 2px solid #bdc3c7;
            border-radius: 5px;
            background-color: white;
            font-size: 13px;
        }
        
        QLineEdit#inputField:focus, QSpinBox#inputField:focus, QComboBox#inputField:focus {
            border: 2px solid #3498db;
            background-color: #ecf8ff;
        }
        
        QSpinBox::up-button, QSpinBox::down-button {
            width: 20px;
            border: none;
            background-color: #3498db;
        }
        
        QSpinBox::up-button:hover, QSpinBox::down-button:hover {
            background-color: #2980b9;
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
                stop:0 #27ae60, stop:1 #229954);
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
                stop:0 #2ecc71, stop:1 #27ae60);
        }
        
        QPushButton#primaryButton:pressed {
            background: #229954;
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

void ManHinhThemBanSao::kiemTraDuLieu() {
    bool valid = true;

    // Check book exists
    if (!sachHienTai) {
        valid = false;
    }

    // Check quantity
    if (nhapSoLuong->value() < 1) {
        valid = false;
    }

    // Check location
    QString location = chonViTri->currentText().trimmed();
    if (location.isEmpty()) {
        chonViTri->setStyleSheet("border: 2px solid #e74c3c; background-color: #ffe5e5;");
        valid = false;
    } else {
        chonViTri->setStyleSheet("");
    }

    nutLuu->setEnabled(valid);
}

void ManHinhThemBanSao::khiNhanLuu() {
    if (!sachHienTai) {
        QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_ERROR"), 
            LanguageManager::instance().getText("MSG_ERROR_NOT_FOUND"));
        return;
    }

    QString location = chonViTri->currentText().trimmed();
    if (location.isEmpty()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"),
            LanguageManager::instance().getText("MSG_MISSING_INFO"));
        chonViTri->setFocus();
        return;
    }

    int quantity = nhapSoLuong->value();

    // Add copies using ThemBanSaoMoi function
    std::string result = ThemBanSaoMoi(maISBN.toStdString(), quantity, location.toStdString());

    if (!result.empty()) {
        emit banSaoDaThem();
        
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"),  LanguageManager::instance().getText("MSG_ADD_COPY_SUCCESS")
            .arg(quantity)
            .arg(QString::fromStdString(sachHienTai->tenSach))
            .arg(location)
            .arg(QString::fromStdString(result)));
        accept();
    } else {
        QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_ERROR"),
            LanguageManager::instance().getText("MSG_ERROR_ADD_COPY_FAILED"));
    }
}
