#include "views/dialogs/ManHinhThemDocGia.h"
#include "models/DocGia.h"
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

extern PTRDG rootDocGia;

ManHinhThemDocGia::ManHinhThemDocGia(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Thêm Độc Giả Mới 👤");
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
    QLabel* titleLabel = new QLabel("👤 Thêm Độc Giả Mới");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // Reader code display
    QGroupBox* codeGroup = new QGroupBox("Mã Thẻ Độc Giả");
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

    QLabel* autoLabel = new QLabel("🤖 Mã thẻ được tạo tự động");
    autoLabel->setAlignment(Qt::AlignCenter);
    autoLabel->setStyleSheet("color: #7f8c8d; font-size: 11px; font-style: italic;");
    codeLayout->addWidget(autoLabel);

    mainLayout->addWidget(codeGroup);

    // Form group
    QGroupBox* formGroup = new QGroupBox("Thông Tin Độc Giả");
    formGroup->setObjectName("formGroup");
    QFormLayout* formLayout = new QFormLayout(formGroup);
    formLayout->setSpacing(12);
    formLayout->setContentsMargins(15, 15, 15, 15);

    // Last name
    nhapHo = new QLineEdit();
    nhapHo->setPlaceholderText("Nhập họ đệm...");
    nhapHo->setObjectName("inputField");
    nhapHo->setToolTip("Nhập họ đệm của độc giả (bắt buộc)\nVí dụ: Nguyễn Văn");
    connect(nhapHo, &QLineEdit::textChanged, this, &ManHinhThemDocGia::kiemTraDuLieu);
    formLayout->addRow("Họ Đệm: *", nhapHo);

    // First name
    nhapTen = new QLineEdit();
    nhapTen->setPlaceholderText("Nhập tên...");
    nhapTen->setObjectName("inputField");
    nhapTen->setToolTip("Nhập tên của độc giả (bắt buộc)\nVí dụ: An");
    connect(nhapTen, &QLineEdit::textChanged, this, &ManHinhThemDocGia::kiemTraDuLieu);
    formLayout->addRow("Tên: *", nhapTen);

    // Gender
    chonGioiTinh = new QComboBox();
    chonGioiTinh->setObjectName("inputField");
    chonGioiTinh->addItem("👨 Nam", 0);
    chonGioiTinh->addItem("👩 Nữ", 1);
    chonGioiTinh->setToolTip("Chọn giới tính của độc giả");
    formLayout->addRow("Giới Tính:", chonGioiTinh);

    mainLayout->addWidget(formGroup);

    // Info notes
    QLabel* infoLabel = new QLabel(
        "💡 <b>Lưu ý:</b><br>"
        "• Các trường đánh dấu (*) là bắt buộc<br>"
        "• Mã thẻ sẽ được tự động tạo theo định dạng: YYYYMMDDXXXXXX<br>"
        "• Độc giả mới có trạng thái hoạt động và chưa mượn sách"
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

    nutXoaForm = new QPushButton("🔄 Xóa Form");
    nutXoaForm->setMinimumHeight(40);
    nutXoaForm->setToolTip("Xóa tất cả dữ liệu đã nhập và tạo mã thẻ mới");
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

    nutLuu = new QPushButton("✅ Thêm Độc Giả");
    nutLuu->setObjectName("primaryButton");
    nutLuu->setMinimumHeight(40);
    nutLuu->setToolTip("Lưu thông tin độc giả mới (Ctrl+S)");
    connect(nutLuu, &QPushButton::clicked, this, &ManHinhThemDocGia::khiNhanLuu);

    nutHuy = new QPushButton("❌ Hủy");
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
    // Get values
    QString lastName = QString::fromStdString(ChuanHoaKhoangTrang(nhapHo->text().trimmed().toStdString()));
    QString firstName = QString::fromStdString(ChuanHoaKhoangTrang(nhapTen->text().trimmed().toStdString()));
    int genderValue = chonGioiTinh->currentData().toInt();

    // Validate họ đệm
    if (lastName.isEmpty()) {
        QMessageBox::warning(this, "Thiếu Thông Tin ⚠️",
            "Vui lòng nhập họ đệm!");
        nhapHo->setFocus();
        return;
    }
    
    std::string loiHo = KiemTraTenNguoi(lastName.toStdString(), "Họ đệm");
    if (!loiHo.empty()) {
        QMessageBox::warning(this, "Lỗi Họ Đệm ⚠️",
            QString::fromStdString(loiHo) + "\n\n💡 Họ đệm chỉ được chứa chữ cái và khoảng trắng, tối đa 50 ký tự.");
        nhapHo->setFocus();
        return;
    }

    // Validate tên
    if (firstName.isEmpty()) {
        QMessageBox::warning(this, "Thiếu Thông Tin ⚠️",
            "Vui lòng nhập tên!");
        nhapTen->setFocus();
        return;
    }
    
    std::string loiTen = KiemTraTenNguoi(firstName.toStdString(), "Tên");
    if (!loiTen.empty()) {
        QMessageBox::warning(this, "Lỗi Tên ⚠️",
            QString::fromStdString(loiTen) + "\n\n💡 Tên chỉ được chứa chữ cái và khoảng trắng, tối đa 50 ký tự.");
        nhapTen->setFocus();
        return;
    }

    // Create reader
    try {
        int maThe = maDuocTao.toInt();
        std::string lastNameStd = lastName.toStdString();
        std::string firstNameStd = firstName.toStdString();
        
        PTRDG newReader = taoDocGia(lastNameStd, firstNameStd, genderValue, 1, maThe);
        
        if (newReader) {
            InsertDocGia(rootDocGia, newReader);
            emit docGiaDaThem();
            
            QString genderStr = genderValue == 0 ? "Nam" : "Nữ";
            QMessageBox::information(this, "Thành Công 🎉",
                QString("✅ Đã thêm độc giả thành công!\n\n"
                       "🎫 Mã thẻ: %1\n"
                       "👤 Họ tên: %2 %3\n"
                       "⚧ Giới tính: %4")
                    .arg(maDuocTao)
                    .arg(lastName)
                    .arg(firstName)
                    .arg(genderStr));
            
            accept();
        } else {
            QMessageBox::critical(this, "Lỗi ❌",
                "Không thể thêm độc giả. Mã thẻ có thể đã tồn tại!\n"
                "Vui lòng thử lại.");
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Lỗi Hệ Thống ❌",
            QString("Đã xảy ra lỗi khi thêm độc giả:\n%1").arg(e.what()));
    } catch (...) {
        QMessageBox::critical(this, "Lỗi Không Xác Định ❌",
            "Đã xảy ra lỗi không xác định khi thêm độc giả!");
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
    // Confirm before clearing
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Xác nhận xóa form",
        "Bạn có chắc muốn xóa tất cả dữ liệu đã nhập?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        nhapHo->clear();
        nhapTen->clear();
        chonGioiTinh->setCurrentIndex(0);
        taoMaDocGia(); // Generate new ID
        nhapHo->setFocus();
        
        // Visual feedback
        AnimationHelper::fadeIn(nhanMaThe, 300);
    }
}
