#include "views/dialogs/ManHinhSuaSach.h"
#include "models/QuanLySach.h"
#include "utils/Utf8Helper.h"
#include "models/KiemTraDuLieu.h"
#include "utils/XuLyChuoi.h"
#include "utils/LanguageManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QGroupBox>
#include <QRegularExpressionValidator>

extern DauSach* dsDauSach[];
extern int soLuongDauSach;

ManHinhSuaSach::ManHinhSuaSach(const QString& isbn, QWidget *parent)
    : QDialog(parent), isbnGoc(isbn), sachHienTai(nullptr)
    , nhapISBN(nullptr)
    , nhapTenSach(nullptr)
    , nhapTacGia(nullptr)
    , chonTheLoai(nullptr)
    , nhapNamXuatBan(nullptr)
    , nhapSoTrang(nullptr)
    , nutLuu(nullptr)
    , nutHuy(nullptr)
{
    setWindowTitle(LanguageManager::instance().getText("TITLE_EDIT_BOOK"));
    setMinimumWidth(550);
    setModal(true);

    // Find book
    sachHienTai = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn.toStdString());
    
    thietLapGiaoDien();
    apDungPhongCachHienDai();
    taiDuLieuSach();
}

void ManHinhSuaSach::thietLapGiaoDien() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel* titleLabel = new QLabel(LanguageManager::instance().getText("HEADER_EDIT_BOOK"));
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // Form group
    QGroupBox* formGroup = new QGroupBox(LanguageManager::instance().getText("GROUP_BOOK_INFO"), this);
    formGroup->setObjectName("formGroup");
    QFormLayout* formLayout = new QFormLayout(formGroup);
    formLayout->setSpacing(12);
    formLayout->setContentsMargins(15, 15, 15, 15);

    // ISBN (read-only)
    nhapISBN = new QLineEdit();
    nhapISBN->setReadOnly(true);
    nhapISBN->setStyleSheet("background-color: #ecf0f1; color: #7f8c8d;");
    formLayout->addRow(LanguageManager::instance().getText("TABLE_HEADER_ISBN") + ":", nhapISBN);

    QLabel* noteLabel = new QLabel(LanguageManager::instance().getText("LBL_ISBN_READONLY"));
    noteLabel->setStyleSheet("color: #95a5a6; font-size: 11px; font-style: italic;");
    formLayout->addRow("", noteLabel);

    // Book name
    // Book name
    nhapTenSach = new QLineEdit();
    nhapTenSach->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_ENTER_BOOK_CODE")); // Reusing code placeholder or just "..."? Use generic or hardcode "..."
    // Just hardcode "..." for now or generic "..."
    nhapTenSach->setPlaceholderText("...");
    nhapTenSach->setObjectName("inputField");
    connect(nhapTenSach, &QLineEdit::textChanged, this, &ManHinhSuaSach::kiemTraDuLieu);
    formLayout->addRow(LanguageManager::instance().getText("TABLE_HEADER_TITLE") + ": *", nhapTenSach);

    // Author
    // Author
    nhapTacGia = new QLineEdit();
    nhapTacGia->setPlaceholderText("...");
    nhapTacGia->setObjectName("inputField");
    connect(nhapTacGia, &QLineEdit::textChanged, this, &ManHinhSuaSach::kiemTraDuLieu);
    formLayout->addRow(LanguageManager::instance().getText("TABLE_HEADER_AUTHOR") + ": *", nhapTacGia);

    // Genre
    chonTheLoai = new QComboBox();
    chonTheLoai->setEditable(true);
    chonTheLoai->setObjectName("inputField");
    chonTheLoai->addItems({
        LanguageManager::instance().getText("GENRE_LITERATURE"),
        LanguageManager::instance().getText("GENRE_SCIENCE"),
        LanguageManager::instance().getText("GENRE_HISTORY"),
        LanguageManager::instance().getText("GENRE_PHILOSOPHY"),
        LanguageManager::instance().getText("GENRE_PSYCHOLOGY"),
        LanguageManager::instance().getText("GENRE_ECONOMICS"),
        LanguageManager::instance().getText("GENRE_TECHNOLOGY"),
        LanguageManager::instance().getText("GENRE_EDUCATION"),
        LanguageManager::instance().getText("GENRE_MEDICINE"),
        LanguageManager::instance().getText("GENRE_ART"),
        LanguageManager::instance().getText("GENRE_SPORTS"),
        LanguageManager::instance().getText("GENRE_TRAVEL"),
        LanguageManager::instance().getText("GENRE_COOKING"),
        LanguageManager::instance().getText("GENRE_KIDS"),
        LanguageManager::instance().getText("GENRE_OTHER")
    });
    connect(chonTheLoai, &QComboBox::currentTextChanged, this, &ManHinhSuaSach::kiemTraDuLieu);
    formLayout->addRow(LanguageManager::instance().getText("TABLE_HEADER_CATEGORY") + ": *", chonTheLoai);

    // Publication year
    nhapNamXuatBan = new QSpinBox();
    nhapNamXuatBan->setMinimum(1900);
    nhapNamXuatBan->setMaximum(2100);
    nhapNamXuatBan->setValue(2024);
    nhapNamXuatBan->setSuffix("");
    nhapNamXuatBan->setObjectName("inputField");
    formLayout->addRow(LanguageManager::instance().getText("TABLE_HEADER_YEAR") + ":", nhapNamXuatBan);

    // Pages
    nhapSoTrang = new QSpinBox();
    nhapSoTrang->setMinimum(1);
    nhapSoTrang->setMaximum(9999);
    nhapSoTrang->setValue(100);
    nhapSoTrang->setSuffix("");
    nhapSoTrang->setObjectName("inputField");
    formLayout->addRow(LanguageManager::instance().getText("TABLE_HEADER_PAGES") + ":", nhapSoTrang);

    mainLayout->addWidget(formGroup);

    // Info note
    QLabel* infoLabel = new QLabel(LanguageManager::instance().getText("LBL_REQUIRED_FIELDS_NOTE"));
    infoLabel->setStyleSheet("color: #7f8c8d; font-size: 12px; padding: 10px; background-color: #ecf0f1; border-radius: 5px;");
    mainLayout->addWidget(infoLabel);

    mainLayout->addStretch();

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    nutLuu = new QPushButton(LanguageManager::instance().getText("BTN_SAVE"));
    nutLuu->setObjectName("primaryButton");
    nutLuu->setMinimumHeight(40);
    connect(nutLuu, &QPushButton::clicked, this, &ManHinhSuaSach::khiNhanLuu);

    nutHuy = new QPushButton(LanguageManager::instance().getText("BTN_CLOSE"));
    nutHuy->setObjectName("nutHuy");
    nutHuy->setMinimumHeight(40);
    connect(nutHuy, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addStretch();
    buttonLayout->addWidget(nutLuu);
    buttonLayout->addWidget(nutHuy);

    mainLayout->addLayout(buttonLayout);
}

void ManHinhSuaSach::apDungPhongCachHienDai() {
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

void ManHinhSuaSach::taiDuLieuSach() {
    if (!sachHienTai) {
        QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_ERROR"), 
            LanguageManager::instance().getText("MSG_ERROR_NOT_FOUND")); // Hardcoded error, rare case, leaving or replacing.
        reject();
        return;
    }

    nhapISBN->setText(QString::fromStdString(sachHienTai->ISBN));
    nhapTenSach->setText(QString::fromStdString(sachHienTai->tenSach));
    nhapTacGia->setText(QString::fromStdString(sachHienTai->tacGia));
    
    // Set genre
    QString genre = QString::fromStdString(sachHienTai->theLoai);
    int index = chonTheLoai->findText(genre);
    if (index >= 0) {
        chonTheLoai->setCurrentIndex(index);
    } else {
        chonTheLoai->setCurrentText(genre);
    }
    
    nhapNamXuatBan->setValue(sachHienTai->namXuatBan);
    nhapSoTrang->setValue(sachHienTai->soTrang);

    kiemTraDuLieu();
}

void ManHinhSuaSach::kiemTraDuLieu() {
    bool valid = true;

    // Check book name
    QString name = nhapTenSach->text().trimmed();
    if (name.isEmpty()) {
        nhapTenSach->setStyleSheet("border: 2px solid #e74c3c; background-color: #ffe5e5;");
        valid = false;
    } else {
        nhapTenSach->setStyleSheet("");
    }

    // Check author
    QString author = nhapTacGia->text().trimmed();
    if (author.isEmpty()) {
        nhapTacGia->setStyleSheet("border: 2px solid #e74c3c; background-color: #ffe5e5;");
        valid = false;
    } else {
        nhapTacGia->setStyleSheet("");
    }

    // Check genre
    QString genre = chonTheLoai->currentText().trimmed();
    if (genre.isEmpty()) {
        chonTheLoai->setStyleSheet("border: 2px solid #e74c3c; background-color: #ffe5e5;");
        valid = false;
    } else {
        chonTheLoai->setStyleSheet("");
    }

    nutLuu->setEnabled(valid);
}

void ManHinhSuaSach::khiNhanLuu() {
    if (!sachHienTai) {
        QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_ERROR"), 
            LanguageManager::instance().getText("MSG_ERROR_NOT_FOUND"));
        return;
    }

    // Get values
    QString name = Utf8Helper::fromStdString(ChuanHoaKhoangTrang(Utf8Helper::toStdString(nhapTenSach->text().trimmed())));
    QString author = Utf8Helper::fromStdString(ChuanHoaKhoangTrang(Utf8Helper::toStdString(nhapTacGia->text().trimmed())));
    QString genre = Utf8Helper::fromStdString(ChuanHoaKhoangTrang(Utf8Helper::toStdString(chonTheLoai->currentText().trimmed())));
    int year = nhapNamXuatBan->value();
    int pages = nhapSoTrang->value();

    // Validate
    if (name.isEmpty()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"),
            LanguageManager::instance().getText("MSG_MISSING_INFO"));
        nhapTenSach->setFocus();
        return;
    }

    if (author.isEmpty()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"),
            LanguageManager::instance().getText("MSG_MISSING_INFO"));
        nhapTacGia->setFocus();
        return;
    }

    if (genre.isEmpty()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"),
            LanguageManager::instance().getText("MSG_MISSING_INFO"));
        chonTheLoai->setFocus();
        return;
    }

    // Update book using CapNhatThongTinDauSach
    std::string result = CapNhatThongTinDauSach(
        isbnGoc.toStdString(),
        name.toStdString(),
        pages,
        author.toStdString(),
        year,
        genre.toStdString()
    );

    if (!result.empty()) {
        emit sachDaCapNhat();
        
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"),
            LanguageManager::instance().getText("MSG_UPDATE_BOOK_SUCCESS")
                .arg(name)
                .arg(author)
                .arg(genre));
        
        accept();
    } else {
        QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_ERROR"),
            LanguageManager::instance().getText("MSG_ERROR_UPDATE_BOOK"));
    }
}
