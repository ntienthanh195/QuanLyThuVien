#include "views/dialogs/ManHinhThemDocGia.h"
#include "models/DocGia.h"
#include "utils/Utf8Helper.h"
#include "models/KiemTraDuLieu.h"
#include "utils/XuLyChuoi.h"
#include "utils/AnimationHelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QGroupBox>
#include <QDateTime>
#include <QShortcut>
#include "utils/ToastNotification.h"

extern PTRDG rootDocGia;

#include "utils/EventBus.h"
#include "utils/LanguageManager.h"

ManHinhThemDocGia::ManHinhThemDocGia(QWidget *parent)
    : QDialog(parent)
    , nhanMaThe(nullptr)
    , nhapHo(nullptr)
    , nhapTen(nullptr)
    , chonGioiTinh(nullptr)
    , nutLuu(nullptr)
    , nutHuy(nullptr)
    , nutXoaForm(nullptr)
    , shortcutSave(nullptr)
    , shortcutCancel(nullptr)
{
    setWindowTitle(LanguageManager::instance().getText("DIALOG_ADD_READER_TITLE") + " 👤");
    setMinimumWidth(500);
    setModal(true);

    thietLapGiaoDien();
    apDungPhongCachHienDai();
    setupKeyboardShortcuts();
    setupTabOrder();
    taoMaDocGia();
}

void ManHinhThemDocGia::thietLapGiaoDien() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel* titleLabel = new QLabel(LanguageManager::instance().getText("DIALOG_ADD_READER_TITLE"));
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // Reader code display
    QGroupBox* codeGroup = new QGroupBox(LanguageManager::instance().getText("LBL_READER_CODE"), this);

    codeGroup->setObjectName("codeGroup");
    QVBoxLayout* codeLayout = new QVBoxLayout(codeGroup);
    codeLayout->setContentsMargins(15, 15, 15, 15);

    nhanMaThe = new QLabel();
    nhanMaThe->setAlignment(Qt::AlignCenter);
    nhanMaThe->setStyleSheet(
        "font-size: 24px; "
        "font-weight: bold; "
        "color: #27ae60; "
        "padding: 15px; "
        "background-color: #e8f8f5; "
        "border: 2px dashed #27ae60; "
        "border-radius: 8px;"
    );
    codeLayout->addWidget(nhanMaThe);

    QLabel* autoLabel = new QLabel(LanguageManager::instance().getText("LBL_AUTO_GENERATED"));
    autoLabel->setAlignment(Qt::AlignCenter);
    autoLabel->setStyleSheet("color: #7f8c8d; font-size: 11px; font-style: italic;");
    codeLayout->addWidget(autoLabel);

    mainLayout->addWidget(codeGroup);

    // Form group
    QGroupBox* formGroup = new QGroupBox(LanguageManager::instance().getText("GROUP_READER_INFO"), this);
    formGroup->setObjectName("formGroup");
    QFormLayout* formLayout = new QFormLayout(formGroup);
    formLayout->setSpacing(12);
    formLayout->setContentsMargins(15, 15, 15, 15);

    // Last name
    nhapHo = new QLineEdit();
    nhapHo->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_SEARCH_READER")); // Reuse or custom? Let's use custom if needed but generic search is close. Actually "Nhập họ đệm..."
    // Better add PLACEHOLDER_LASTNAME
    nhapHo->setPlaceholderText("..."); // Skip for now? No, need placeholder.
    // I added LBL_LASTNAME but not placeholder. Using LBL is okay? "Last Name..."
    nhapHo->setPlaceholderText(LanguageManager::instance().getText("LBL_LASTNAME") + "...");
    nhapHo->setObjectName("inputField");
    nhapHo->setToolTip(LanguageManager::instance().getText("LBL_LASTNAME"));
    connect(nhapHo, &QLineEdit::textChanged, this, &ManHinhThemDocGia::kiemTraDuLieu);
    formLayout->addRow(LanguageManager::instance().getText("LBL_LASTNAME") + ": *", nhapHo);

    // First name
    nhapTen = new QLineEdit();
    nhapTen->setPlaceholderText(LanguageManager::instance().getText("LBL_FIRSTNAME") + "...");
    nhapTen->setObjectName("inputField");
    nhapTen->setToolTip(LanguageManager::instance().getText("LBL_FIRSTNAME"));
    connect(nhapTen, &QLineEdit::textChanged, this, &ManHinhThemDocGia::kiemTraDuLieu);
    formLayout->addRow(LanguageManager::instance().getText("LBL_FIRSTNAME") + ": *", nhapTen);

    // Gender
    chonGioiTinh = new QComboBox();
    chonGioiTinh->setObjectName("inputField");
    chonGioiTinh->addItem(LanguageManager::instance().getText("GENDER_MALE"), 0);
    chonGioiTinh->addItem(LanguageManager::instance().getText("GENDER_FEMALE"), 1);
    chonGioiTinh->setToolTip(LanguageManager::instance().getText("LBL_GENDER"));
    formLayout->addRow(LanguageManager::instance().getText("LBL_GENDER") + ":", chonGioiTinh);

    mainLayout->addWidget(formGroup);

    // Info notes
    QLabel* infoLabel = new QLabel(
        LanguageManager::instance().getText("MSG_NOTE_ADD_READER")
    );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet(
        "color: #34495e; "
        "font-size: 12px; "
        "padding: 12px; "
        "background-color: #ecf0f1; "
        "border-left: 4px solid #3498db; "
        "border-radius: 5px;"
    );
    mainLayout->addWidget(infoLabel);

    mainLayout->addStretch();

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    nutXoaForm = new QPushButton(LanguageManager::instance().getText("BTN_CLEAR_FORM"));
    nutXoaForm->setMinimumHeight(40);
    nutXoaForm->setToolTip(LanguageManager::instance().getText("BTN_CLEAR_FORM"));
    nutXoaForm->setStyleSheet(
        "QPushButton {"
        "    background: #95a5a6;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 10px 20px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover { background: #7f8c8d; }"
        "QPushButton:pressed { background: #6c7a89; }"
    );
    connect(nutXoaForm, &QPushButton::clicked, this, &ManHinhThemDocGia::clearAllFields);

    nutLuu = new QPushButton(LanguageManager::instance().getText("BTN_SAVE_READER"));
    nutLuu->setObjectName("primaryButton");
    nutLuu->setMinimumHeight(40);
    nutLuu->setToolTip(LanguageManager::instance().getText("BTN_SAVE_READER"));
    connect(nutLuu, &QPushButton::clicked, this, &ManHinhThemDocGia::khiNhanLuu);

    nutHuy = new QPushButton(LanguageManager::instance().getText("BTN_CANCEL_DIALOG"));
    nutHuy->setObjectName("nutHuy");
    nutHuy->setMinimumHeight(40);
    nutHuy->setToolTip("Hủy và đóng cửa sổ (ESC)");
    connect(nutHuy, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addWidget(nutXoaForm);
    buttonLayout->addStretch();
    buttonLayout->addWidget(nutLuu);
    buttonLayout->addWidget(nutHuy);

    mainLayout->addLayout(buttonLayout);

    // Initial validation
    kiemTraDuLieu();
}

void ManHinhThemDocGia::apDungPhongCachHienDai() {
    setStyleSheet(R"(
        QDialog {
            background-color: #ffffff;
        }
        
        QGroupBox#formGroup, QGroupBox#codeGroup {
            font-weight: bold;
            border: 2px solid #3498db;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
            background-color: #f8f9fa;
        }
        
        QGroupBox#formGroup::title, QGroupBox#codeGroup::title {
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

void ManHinhThemDocGia::taoMaDocGia() {
    // Generate reader code based on current date/time
    QDateTime now = QDateTime::currentDateTime();
    maDuocTao = now.toString("yyyyMMddhhmmss");
    nhanMaThe->setText(maDuocTao);
}

void ManHinhThemDocGia::kiemTraDuLieu() {
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

void ManHinhThemDocGia::khiNhanLuu() {
    // Get values with auto-capitalization
    QString lastName = Utf8Helper::fromStdString(ChuyenThanhTitleCaseUnicode(Utf8Helper::toStdString(nhapHo->text().trimmed())));
    QString firstName = Utf8Helper::fromStdString(ChuyenThanhTitleCaseUnicode(Utf8Helper::toStdString(nhapTen->text().trimmed())));
    int genderValue = chonGioiTinh->currentData().toInt();

    // Validate họ đệm
    // Validate họ đệm
    if (lastName.isEmpty()) {
        nhapHo->setStyleSheet("QLineEdit { border: 2px solid #e74c3c; background-color: #ffe5e5; }");
        ToastNotification::show(LanguageManager::instance().getText("MSG_MISSING_LASTNAME"), ToastNotification::WARNING);
        nhapHo->setFocus();
        return;
    }
    
    std::string loiHo = KiemTraTenNguoi(lastName.toStdString(), "Họ đệm");
    if (!loiHo.empty()) {
        nhapHo->setStyleSheet("QLineEdit { border: 2px solid #e74c3c; background-color: #ffe5e5; }");
        ToastNotification::show(QString::fromStdString(loiHo), ToastNotification::WARNING);
        nhapHo->setFocus();
        return;
    }

    // Validate tên
    // Validate tên
    if (firstName.isEmpty()) {
        nhapTen->setStyleSheet("QLineEdit { border: 2px solid #e74c3c; background-color: #ffe5e5; }");
        ToastNotification::show(LanguageManager::instance().getText("MSG_MISSING_FIRSTNAME"), ToastNotification::WARNING);
        nhapTen->setFocus();
        return;
    }
    
    std::string loiTen = KiemTraTenNguoi(firstName.toStdString(), "Tên");
    if (!loiTen.empty()) {
        nhapTen->setStyleSheet("QLineEdit { border: 2px solid #e74c3c; background-color: #ffe5e5; }");
        ToastNotification::show(QString::fromStdString(loiTen), ToastNotification::WARNING);
        nhapTen->setFocus();
        return;
    }

    // Create reader
    try {
        // Use backend generator to create a safe unique MaThe (avoid int overflow)
        std::string lastNameStd = lastName.toStdString();
        std::string firstNameStd = firstName.toStdString();

        extern PTRDG rootDocGia; // ensure we use current tree for uniqueness
        PTRDG newReader = taoDocGia(lastNameStd, firstNameStd, genderValue, 1, rootDocGia);
        
        if (newReader) {
            InsertDocGia(rootDocGia, newReader);
            // Global notification for realtime update
            try {
                EventBus::instance().docGiaAdded(newReader->data.MaThe);
            } catch(...) {}
            emit docGiaDaThem();
            
            QString genderStr = genderValue == 0 ? LanguageManager::instance().getText("GENDER_MALE") : LanguageManager::instance().getText("GENDER_FEMALE");
            QString maTheStr = (newReader != nullptr) ? QString::number(newReader->data.MaThe) : maDuocTao;
            
            // Show non-blocking success toast
            ToastNotification::show(
                LanguageManager::instance().getText("MSG_SUCCESS_ADD") + ": " + lastName + " " + firstName,
                ToastNotification::SUCCESS
            );
            
            accept();
        } else {
             ToastNotification::show(LanguageManager::instance().getText("MSG_ERROR_ADD"), ToastNotification::ERROR);
        }
    } catch (const std::exception& e) {
        ToastNotification::show(LanguageManager::instance().getText("MSG_ERROR_ADD_FAILED").arg(e.what()), ToastNotification::ERROR);
    } catch (...) {
        ToastNotification::show(LanguageManager::instance().getText("MSG_ERROR_UNKNOWN_ACTION"), ToastNotification::ERROR);
    }
}

void ManHinhThemDocGia::setupKeyboardShortcuts() {
    // Ctrl+S - Save
    shortcutSave = new QShortcut(QKeySequence("Ctrl+S"), this);
    connect(shortcutSave, &QShortcut::activated, [this]() {
        if (nutLuu->isEnabled()) {
            khiNhanLuu();
        }
    });
    
    // ESC - Cancel
    shortcutCancel = new QShortcut(QKeySequence("Escape"), this);
    connect(shortcutCancel, &QShortcut::activated, this, &QDialog::reject);
}

void ManHinhThemDocGia::setupTabOrder() {
    // Set logical tab order for keyboard navigation
    setTabOrder(nhapHo, nhapTen);
    setTabOrder(nhapTen, chonGioiTinh);
    setTabOrder(chonGioiTinh, nutXoaForm);
    setTabOrder(nutXoaForm, nutLuu);
    setTabOrder(nutLuu, nutHuy);
    
    // Set initial focus
    nhapHo->setFocus();
}

void ManHinhThemDocGia::clearAllFields() {
    // Clear without annoying confirmation
    nhapHo->clear();
    nhapTen->clear();
    chonGioiTinh->setCurrentIndex(0);
    taoMaDocGia(); // Generate new ID
    nhapHo->setFocus();
    
    // Visual feedback
    AnimationHelper::fadeIn(nhanMaThe, 300);
    ToastNotification::show(LanguageManager::instance().getText("MSG_FORM_CLEARED"), ToastNotification::INFO);
}
