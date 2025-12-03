#include "views/dialogs/ManHinhSuaDocGia.h"
#include "models/DocGia.h"
#include "models/KiemTraDuLieu.h"
#include "utils/XuLyChuoi.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QGroupBox>

extern PTRDG rootDocGia;

ManHinhSuaDocGia::ManHinhSuaDocGia(const QString& readerCode, QWidget *parent)
    : QDialog(parent), maDocGia(readerCode), docGiaHienTai(nullptr) {
    setWindowTitle("Chỉnh Sửa Độc Giả 👤");
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
    QLabel* titleLabel = new QLabel("👤 Chỉnh Sửa Thông Tin Độc Giả");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // Form group
    QGroupBox* formGroup = new QGroupBox("Thông Tin Độc Giả");
    formGroup->setObjectName("formGroup");
    QFormLayout* formLayout = new QFormLayout(formGroup);
    formLayout->setSpacing(12);
    formLayout->setContentsMargins(15, 15, 15, 15);

    // Reader code (read-only)
    nhapMaThe = new QLineEdit();
    nhapMaThe->setReadOnly(true);
    nhapMaThe->setStyleSheet("background-color: #ecf0f1; color: #7f8c8d; font-weight: bold;");
    formLayout->addRow("Mã Thẻ:", nhapMaThe);

    QLabel* noteLabel = new QLabel("⚠️ Mã thẻ không thể chỉnh sửa");
    noteLabel->setStyleSheet("color: #95a5a6; font-size: 11px; font-style: italic;");
    formLayout->addRow("", noteLabel);

    // Last name
    nhapHo = new QLineEdit();
    nhapHo->setPlaceholderText("Nhập họ đệm...");
    nhapHo->setObjectName("inputField");
    connect(nhapHo, &QLineEdit::textChanged, this, &ManHinhSuaDocGia::kiemTraDuLieu);
    formLayout->addRow("Họ Đệm: *", nhapHo);

    // First name
    nhapTen = new QLineEdit();
    nhapTen->setPlaceholderText("Nhập tên...");
    nhapTen->setObjectName("inputField");
    connect(nhapTen, &QLineEdit::textChanged, this, &ManHinhSuaDocGia::kiemTraDuLieu);
    formLayout->addRow("Tên: *", nhapTen);

    // Gender
    chonGioiTinh = new QComboBox();
    chonGioiTinh->setObjectName("inputField");
    chonGioiTinh->addItem("👨 Nam", 0);
    chonGioiTinh->addItem("👩 Nữ", 1);
    formLayout->addRow("Giới Tính:", chonGioiTinh);

    // Status
    chonTrangThai = new QComboBox();
    chonTrangThai->setObjectName("inputField");
    chonTrangThai->addItem("✅ Hoạt động", 0);
    chonTrangThai->addItem("🔒 Khóa", 1);
    formLayout->addRow("Trạng Thái:", chonTrangThai);

    mainLayout->addWidget(formGroup);

    // Statistics group
    QGroupBox* statsGroup = new QGroupBox("Thống Kê");
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

    QLabel* statsNote = new QLabel("💡 Không thể khóa độc giả đang mượn sách");
    statsNote->setStyleSheet("color: #7f8c8d; font-size: 11px; font-style: italic;");
    statsLayout->addWidget(statsNote);

    mainLayout->addWidget(statsGroup);

    // Info note
    QLabel* infoLabel = new QLabel("💡 Các trường đánh dấu (*) là bắt buộc");
    infoLabel->setStyleSheet("color: #7f8c8d; font-size: 12px; padding: 10px; background-color: #ecf0f1; border-radius: 5px;");
    mainLayout->addWidget(infoLabel);

    mainLayout->addStretch();

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    nutLuu = new QPushButton("✅ Lưu Thay Đổi");
    nutLuu->setObjectName("primaryButton");
    nutLuu->setMinimumHeight(40);
    connect(nutLuu, &QPushButton::clicked, this, &ManHinhSuaDocGia::khiNhanLuu);

    nutHuy = new QPushButton("❌ Hủy");
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
        QMessageBox::critical(this, "Lỗi ❌", 
            "Không tìm thấy thông tin độc giả!");
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
        QString("📚 Số sách đang mượn: <b>%1</b>").arg(borrowedCount)
    );

    // Disable status change if reader has borrowed books
    if (borrowedCount > 0) {
        chonTrangThai->setEnabled(false);
        chonTrangThai->setToolTip("Không thể thay đổi trạng thái khi độc giả đang mượn sách");
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
        QMessageBox::critical(this, "Lỗi ❌", 
            "Không tìm thấy thông tin độc giả!");
        return;
    }

    // Get values
    QString lastName = QString::fromStdString(ChuanHoaKhoangTrang(nhapHo->text().trimmed().toStdString()));
    QString firstName = QString::fromStdString(ChuanHoaKhoangTrang(nhapTen->text().trimmed().toStdString()));
    int genderValue = chonGioiTinh->currentData().toInt();
    int statusValue = chonTrangThai->currentData().toInt();

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

    // Check if trying to lock reader with borrowed books
    int borrowedCount = docGiaHienTai->data.soSachDangMuon;
    int targetStatus = statusValue == 0 ? 1 : 0; // Convert from combo index to actual status
    
    if (targetStatus == 0 && borrowedCount > 0) {
        QMessageBox::warning(this, "Không Thể Khóa ⚠️",
            QString("Không thể khóa độc giả đang mượn %1 cuốn sách!\n"
                   "Vui lòng đợi độc giả trả hết sách trước.").arg(borrowedCount));
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
        
        QString genderStr = genderValue == 0 ? "Nam" : "Nữ";
        QString statusStr = targetStatus == 1 ? "Hoạt động" : "Khóa";
        
        QMessageBox::information(this, "Thành Công 🎉",
            QString("✅ Đã cập nhật thông tin độc giả thành công!\n\n"
                   "🎫 Mã thẻ: %1\n"
                   "👤 Họ tên: %2 %3\n"
                   "⚧ Giới tính: %4\n"
                   "📊 Trạng thái: %5")
                .arg(docGiaHienTai->data.MaThe)
                .arg(lastName)
                .arg(firstName)
                .arg(genderStr)
                .arg(statusStr));
        
        accept();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Lỗi Hệ Thống ❌",
            QString("Đã xảy ra lỗi khi cập nhật độc giả:\n%1").arg(e.what()));
    } catch (...) {
        QMessageBox::critical(this, "Lỗi Không Xác Định ❌",
            "Đã xảy ra lỗi không xác định khi cập nhật độc giả!");
    }
}
