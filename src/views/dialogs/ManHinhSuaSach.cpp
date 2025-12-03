#include "views/dialogs/ManHinhSuaSach.h"
#include "models/QuanLySach.h"
#include "models/KiemTraDuLieu.h"
#include "utils/XuLyChuoi.h"
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
    : QDialog(parent), isbnGoc(isbn), sachHienTai(nullptr) {
    setWindowTitle("Chỉnh Sửa Đầu Sách 📝");
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
    QLabel* titleLabel = new QLabel("📝 Chỉnh Sửa Thông Tin Đầu Sách");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // Form group
    QGroupBox* formGroup = new QGroupBox("Thông Tin Đầu Sách");
    formGroup->setObjectName("formGroup");
    QFormLayout* formLayout = new QFormLayout(formGroup);
    formLayout->setSpacing(12);
    formLayout->setContentsMargins(15, 15, 15, 15);

    // ISBN (read-only)
    nhapISBN = new QLineEdit();
    nhapISBN->setReadOnly(true);
    nhapISBN->setStyleSheet("background-color: #ecf0f1; color: #7f8c8d;");
    formLayout->addRow("ISBN:", nhapISBN);

    QLabel* noteLabel = new QLabel("⚠️ ISBN không thể chỉnh sửa");
    noteLabel->setStyleSheet("color: #95a5a6; font-size: 11px; font-style: italic;");
    formLayout->addRow("", noteLabel);

    // Book name
    nhapTenSach = new QLineEdit();
    nhapTenSach->setPlaceholderText("Nhập tên sách...");
    nhapTenSach->setObjectName("inputField");
    connect(nhapTenSach, &QLineEdit::textChanged, this, &ManHinhSuaSach::kiemTraDuLieu);
    formLayout->addRow("Tên Sách: *", nhapTenSach);

    // Author
    nhapTacGia = new QLineEdit();
    nhapTacGia->setPlaceholderText("Nhập tên tác giả...");
    nhapTacGia->setObjectName("inputField");
    connect(nhapTacGia, &QLineEdit::textChanged, this, &ManHinhSuaSach::kiemTraDuLieu);
    formLayout->addRow("Tác Giả: *", nhapTacGia);

    // Genre
    chonTheLoai = new QComboBox();
    chonTheLoai->setEditable(true);
    chonTheLoai->setObjectName("inputField");
    chonTheLoai->addItems({
        "Văn học",
        "Khoa học",
        "Lịch sử",
        "Triết học",
        "Tâm lý học",
        "Kinh tế",
        "Công nghệ",
        "Giáo dục",
        "Y học",
        "Nghệ thuật",
        "Thể thao",
        "Du lịch",
        "Nấu ăn",
        "Thiếu nhi",
        "Khác"
    });
    connect(chonTheLoai, &QComboBox::currentTextChanged, this, &ManHinhSuaSach::kiemTraDuLieu);
    formLayout->addRow("Thể Loại: *", chonTheLoai);

    // Publication year
    nhapNamXuatBan = new QSpinBox();
    nhapNamXuatBan->setMinimum(1900);
    nhapNamXuatBan->setMaximum(2100);
    nhapNamXuatBan->setValue(2024);
    nhapNamXuatBan->setSuffix(" năm");
    nhapNamXuatBan->setObjectName("inputField");
    formLayout->addRow("Năm XB:", nhapNamXuatBan);

    // Pages
    nhapSoTrang = new QSpinBox();
    nhapSoTrang->setMinimum(1);
    nhapSoTrang->setMaximum(9999);
    nhapSoTrang->setValue(100);
    nhapSoTrang->setSuffix(" trang");
    nhapSoTrang->setObjectName("inputField");
    formLayout->addRow("Số Trang:", nhapSoTrang);

    mainLayout->addWidget(formGroup);

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
    connect(nutLuu, &QPushButton::clicked, this, &ManHinhSuaSach::khiNhanLuu);

    nutHuy = new QPushButton("❌ Hủy");
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
        QMessageBox::critical(this, "Lỗi ❌", 
            "Không tìm thấy thông tin đầu sách!");
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
        QMessageBox::critical(this, "Lỗi ❌", 
            "Không tìm thấy thông tin đầu sách!");
        return;
    }

    // Get values
    QString name = QString::fromStdString(ChuanHoaKhoangTrang(nhapTenSach->text().trimmed().toStdString()));
    QString author = QString::fromStdString(ChuanHoaKhoangTrang(nhapTacGia->text().trimmed().toStdString()));
    QString genre = QString::fromStdString(ChuanHoaKhoangTrang(chonTheLoai->currentText().trimmed().toStdString()));
    int year = nhapNamXuatBan->value();
    int pages = nhapSoTrang->value();

    // Validate
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Thiếu Thông Tin ⚠️",
            "Vui lòng nhập tên sách!");
        nhapTenSach->setFocus();
        return;
    }

    if (author.isEmpty()) {
        QMessageBox::warning(this, "Thiếu Thông Tin ⚠️",
            "Vui lòng nhập tên tác giả!");
        nhapTacGia->setFocus();
        return;
    }

    if (genre.isEmpty()) {
        QMessageBox::warning(this, "Thiếu Thông Tin ⚠️",
            "Vui lòng chọn thể loại!");
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
        
        QMessageBox::information(this, "Thành Công 🎉",
            QString("✅ Đã cập nhật thông tin đầu sách thành công!\n\n"
                   "📚 Tên sách: %1\n"
                   "✍️ Tác giả: %2\n"
                   "📖 Thể loại: %3")
                .arg(name)
                .arg(author)
                .arg(genre));
        
        accept();
    } else {
        QMessageBox::critical(this, "Lỗi ❌",
            "Không thể cập nhật thông tin đầu sách. Vui lòng thử lại!");
    }
}
