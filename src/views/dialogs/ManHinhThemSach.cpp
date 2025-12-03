#include "views/dialogs/ManHinhThemSach.h"
#include "models/QuanLySach.h"
#include "models/KiemTraDuLieu.h"
#include "utils/AnimationHelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QDateTime>
#include <QShortcut>
#include <QTimer>

ManHinhThemSach::ManHinhThemSach(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Thêm Đầu Sách Mới");
    setMinimumWidth(550);
    thietLapGiaoDien();
    thietLapKetNoi();
    thietLapKiemTra();
    setupKeyboardShortcuts();
    setupTabOrder();
}

ManHinhThemSach::~ManHinhThemSach() {
}

void ManHinhThemSach::thietLapGiaoDien() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(25, 25, 25, 25);
    
    // Title
    QLabel *titleLabel = new QLabel("📚 THÊM ĐẦU SÁCH MỚI", this);
    QFont titleFont;
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);
    
    // Form group
    QGroupBox *formGroup = new QGroupBox("Thông Tin Sách", this);
    formGroup->setStyleSheet(
        "QGroupBox {"
        "   border: 2px solid #3498db;"
        "   border-radius: 8px;"
        "   margin-top: 10px;"
        "   padding-top: 15px;"
        "   font-weight: bold;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   left: 15px;"
        "   padding: 0 5px;"
        "   color: #3498db;"
        "}"
    );
    
    QFormLayout *formLayout = new QFormLayout(formGroup);
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight);
    
    // ISBN field
    nhapISBN = new QLineEdit(this);
    nhapISBN->setPlaceholderText("Nhập 10 hoặc 13 chữ số (VD: 9781234567890)");
    nhapISBN->setMaxLength(13);
    nhapISBN->setToolTip("ISBN phải có 10 hoặc 13 chữ số\nVí dụ: 9781234567890");
    QLabel *isbnLabel = new QLabel("ISBN *:", this);
    isbnLabel->setStyleSheet("font-weight: bold;");
    formLayout->addRow(isbnLabel, nhapISBN);
    
    // Book name field
    nhapTenSach = new QLineEdit(this);
    nhapTenSach->setPlaceholderText("Tên sách (tối đa 100 ký tự)");
    nhapTenSach->setMaxLength(100);
    nhapTenSach->setToolTip("Nhập tên đầy đủ của sách (bắt buộc)\nTối đa 100 ký tự");
    QLabel *nameLabel = new QLabel("Tên sách *:", this);
    nameLabel->setStyleSheet("font-weight: bold;");
    formLayout->addRow(nameLabel, nhapTenSach);
    
    // Author field
    nhapTacGia = new QLineEdit(this);
    nhapTacGia->setPlaceholderText("Tên tác giả (tối đa 50 ký tự)");
    nhapTacGia->setMaxLength(50);
    nhapTacGia->setToolTip("Nhập tên tác giả (bắt buộc)\nVí dụ: Nguyễn Nhật Ánh");
    QLabel *authorLabel = new QLabel("Tác giả *:", this);
    authorLabel->setStyleSheet("font-weight: bold;");
    formLayout->addRow(authorLabel, nhapTacGia);
    
    // Genre combo
    chonTheLoai = new QComboBox(this);
    chonTheLoai->setEditable(true);
    chonTheLoai->setToolTip("Chọn hoặc nhập thể loại sách");
    diemDayTheLoai();
    QLabel *genreLabel = new QLabel("Thể loại *:", this);
    genreLabel->setStyleSheet("font-weight: bold;");
    formLayout->addRow(genreLabel, chonTheLoai);
    
    // Year spinner
    nhapNamXuatBan = new QSpinBox(this);
    nhapNamXuatBan->setRange(1000, QDateTime::currentDateTime().date().year());
    nhapNamXuatBan->setValue(2024);
    nhapNamXuatBan->setSuffix(" năm");
    nhapNamXuatBan->setToolTip("Năm xuất bản sách (1000 - hiện tại)");
    QLabel *yearLabel = new QLabel("Năm xuất bản *:", this);
    yearLabel->setStyleSheet("font-weight: bold;");
    formLayout->addRow(yearLabel, nhapNamXuatBan);
    
    // Pages spinner
    nhapSoTrang = new QSpinBox(this);
    nhapSoTrang->setRange(1, 10000);
    nhapSoTrang->setValue(100);
    nhapSoTrang->setSuffix(" trang");
    nhapSoTrang->setToolTip("Số trang của sách (1-10000)");
    QLabel *pagesLabel = new QLabel("Số trang *:", this);
    pagesLabel->setStyleSheet("font-weight: bold;");
    formLayout->addRow(pagesLabel, nhapSoTrang);
    
    // Copies spinner
    nhapSoBanSao = new QSpinBox(this);
    nhapSoBanSao->setRange(1, 100);
    nhapSoBanSao->setValue(1);
    nhapSoBanSao->setSuffix(" bản sao");
    nhapSoBanSao->setToolTip("Số lượng bản sao muốn thêm cùng lúc (1-100)");
    QLabel *copiesLabel = new QLabel("Số bản sao:", this);
    copiesLabel->setStyleSheet("font-weight: bold;");
    formLayout->addRow(copiesLabel, nhapSoBanSao);
    
    // Location field
    nhapViTri = new QLineEdit(this);
    nhapViTri->setPlaceholderText("VD: Kệ A1, Phòng 201 (tùy chọn)");
    nhapViTri->setMaxLength(30);
    nhapViTri->setToolTip("Vị trí đặt sách trong thư viện (không bắt buộc)\nVí dụ: Kệ A1, Tầng 2");
    QLabel *locationLabel = new QLabel("Vị trí:", this);
    locationLabel->setStyleSheet("font-weight: bold;");
    formLayout->addRow(locationLabel, nhapViTri);
    
    mainLayout->addWidget(formGroup);
    
    // Status label
    nhanTrangThai = new QLabel("", this);
    nhanTrangThai->setWordWrap(true);
    nhanTrangThai->setMinimumHeight(30);
    nhanTrangThai->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(nhanTrangThai);
    
    // Note
    QLabel *noteLabel = new QLabel("(*) Các trường bắt buộc", this);
    noteLabel->setStyleSheet("color: #7f8c8d; font-style: italic; font-size: 10pt;");
    mainLayout->addWidget(noteLabel);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    nutXoaForm = new QPushButton("🔄 Xóa Form", this);
    nutXoaForm->setMinimumWidth(120);
    nutXoaForm->setMinimumHeight(40);
    nutXoaForm->setToolTip("Xóa tất cả dữ liệu đã nhập");
    nutXoaForm->setStyleSheet(
        "QPushButton {"
        "   background-color: #95a5a6;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   font-size: 11pt;"
        "   font-weight: bold;"
        "   padding: 8px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #7f8c8d;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #6c7a89;"
        "}"
    );
    
    buttonLayout->addWidget(nutXoaForm);
    buttonLayout->addStretch();
    
    nutLuu = new QPushButton("💾 Lưu", this);
    nutLuu->setMinimumWidth(120);
    nutLuu->setMinimumHeight(40);
    nutLuu->setToolTip("Lưu thông tin sách mới (Ctrl+S)");
    nutLuu->setStyleSheet(
        "QPushButton {"
        "   background-color: #27ae60;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   font-size: 11pt;"
        "   font-weight: bold;"
        "   padding: 8px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #229954;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #1e8449;"
        "}"
        "QPushButton:disabled {"
        "   background-color: #95a5a6;"
        "}"
    );
    
    nutHuy = new QPushButton("❌ Hủy", this);
    nutHuy->setMinimumWidth(120);
    nutHuy->setMinimumHeight(40);
    nutHuy->setToolTip("Hủy và đóng cửa sổ (ESC)");
    nutHuy->setStyleSheet(
        "QPushButton {"
        "   background-color: #e74c3c;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   font-size: 11pt;"
        "   font-weight: bold;"
        "   padding: 8px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #c0392b;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #a93226;"
        "}"
    );
    
    buttonLayout->addWidget(nutLuu);
    buttonLayout->addWidget(nutHuy);
    mainLayout->addLayout(buttonLayout);
    
    // Set field styles
    QString fieldStyle = 
        "QLineEdit, QSpinBox, QComboBox {"
        "   padding: 8px;"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 5px;"
        "   font-size: 10pt;"
        "}"
        "QLineEdit:focus, QSpinBox:focus, QComboBox:focus {"
        "   border-color: #3498db;"
        "}"
        "QLineEdit:disabled, QSpinBox:disabled, QComboBox:disabled {"
        "   background-color: #ecf0f1;"
        "}";
    
    nhapISBN->setStyleSheet(fieldStyle);
    nhapTenSach->setStyleSheet(fieldStyle);
    nhapTacGia->setStyleSheet(fieldStyle);
    chonTheLoai->setStyleSheet(fieldStyle);
    nhapNamXuatBan->setStyleSheet(fieldStyle);
    nhapSoTrang->setStyleSheet(fieldStyle);
    nhapSoBanSao->setStyleSheet(fieldStyle);
    nhapViTri->setStyleSheet(fieldStyle);
}

void ManHinhThemSach::thietLapKetNoi() {
    connect(nutLuu, &QPushButton::clicked, this, &ManHinhThemSach::khiNhanLuu);
    connect(nutHuy, &QPushButton::clicked, this, &ManHinhThemSach::khiNhanHuy);
    connect(nutXoaForm, &QPushButton::clicked, this, &ManHinhThemSach::clearAllFields);
    connect(nhapISBN, &QLineEdit::textChanged, this, &ManHinhThemSach::kiemTraISBN);
    connect(nhapTenSach, &QLineEdit::textChanged, this, &ManHinhThemSach::kiemTraTatCaTruong);
    connect(nhapTacGia, &QLineEdit::textChanged, this, &ManHinhThemSach::kiemTraTatCaTruong);
}

void ManHinhThemSach::thietLapKiemTra() {
    // ISBN validator: only digits
    QRegularExpressionValidator *isbnValidator = 
        new QRegularExpressionValidator(QRegularExpression("^[0-9]{0,13}$"), this);
    nhapISBN->setValidator(isbnValidator);
}

void ManHinhThemSach::diemDayTheLoai() {
    chonTheLoai->addItems({
        "Văn học",
        "Khoa học",
        "Công nghệ",
        "Lịch sử",
        "Triết học",
        "Kinh tế",
        "Tâm lý",
        "Nghệ thuật",
        "Thiếu nhi",
        "Giáo dục",
        "Y học",
        "Thể thao",
        "Du lịch",
        "Ẩm thực",
        "Tôn giáo"
    });
    chonTheLoai->setCurrentIndex(-1);
    chonTheLoai->setPlaceholderText("Chọn hoặc nhập thể loại");
}

void ManHinhThemSach::kiemTraISBN() {
    QString isbn = nhapISBN->text().trimmed();
    
    if (isbn.isEmpty()) {
        datPhongCachTruong(nhapISBN, true);
        return;
    }
    
    // Check length
    if (isbn.length() != 10 && isbn.length() != 13) {
        datPhongCachTruong(nhapISBN, false);
        hienThiLoi("ISBN phải có 10 hoặc 13 chữ số");
        return;
    }
    
    // Check if already exists
    if (TimDauSachTheoISBN(LayDanhSachDauSach(), LaySoLuongDauSach(), isbn.toStdString()) != nullptr) {
        datPhongCachTruong(nhapISBN, false);
        hienThiLoi("ISBN này đã tồn tại trong hệ thống");
        return;
    }
    
    datPhongCachTruong(nhapISBN, true);
    kiemTraTatCaTruong();
}

void ManHinhThemSach::kiemTraTatCaTruong() {
    bool allValid = true;
    
    // ISBN
    QString isbn = nhapISBN->text().trimmed();
    if (isbn.length() != 10 && isbn.length() != 13) {
        allValid = false;
    }
    
    // Name
    if (nhapTenSach->text().trimmed().isEmpty()) {
        allValid = false;
    }
    
    // Author
    if (nhapTacGia->text().trimmed().isEmpty()) {
        allValid = false;
    }
    
    // Genre
    if (chonTheLoai->currentText().trimmed().isEmpty()) {
        allValid = false;
    }
    
    nutLuu->setEnabled(allValid);
    
    if (allValid) {
        nhanTrangThai->clear();
    }
}

void ManHinhThemSach::datPhongCachTruong(QLineEdit *field, bool hopLe) {
    if (hopLe) {
        field->setStyleSheet(
            "QLineEdit {"
            "   padding: 8px;"
            "   border: 2px solid #27ae60;"
            "   border-radius: 5px;"
            "   font-size: 10pt;"
            "}"
        );
    } else {
        field->setStyleSheet(
            "QLineEdit {"
            "   padding: 8px;"
            "   border: 2px solid #e74c3c;"
            "   border-radius: 5px;"
            "   font-size: 10pt;"
            "}"
        );
    }
}

void ManHinhThemSach::hienThiLoi(const QString &message) {
    nhanTrangThai->setText("❌ " + message);
    nhanTrangThai->setStyleSheet("color: #e74c3c; font-weight: bold; padding: 5px;");
}

void ManHinhThemSach::hienThiThanhCong(const QString &message) {
    nhanTrangThai->setText("✅ " + message);
    nhanTrangThai->setStyleSheet("color: #27ae60; font-weight: bold; padding: 5px;");
}

bool ManHinhThemSach::kiemTraCacTruong() {
    QString isbn = nhapISBN->text().trimmed();
    QString name = nhapTenSach->text().trimmed();
    QString author = nhapTacGia->text().trimmed();
    QString genre = chonTheLoai->currentText().trimmed();
    
    if (isbn.isEmpty() || name.isEmpty() || author.isEmpty() || genre.isEmpty()) {
        hienThiLoi("Vui lòng điền đầy đủ các trường bắt buộc!");
        return false;
    }
    
    if (isbn.length() != 10 && isbn.length() != 13) {
        hienThiLoi("ISBN phải có 10 hoặc 13 chữ số!");
        return false;
    }
    
    if (name.length() < 2) {
        hienThiLoi("Tên sách quá ngắn!");
        return false;
    }
    
    if (author.length() < 2) {
        hienThiLoi("Tên tác giả quá ngắn!");
        return false;
    }
    
    return true;
}

void ManHinhThemSach::khiNhanLuu() {
    if (!kiemTraCacTruong()) {
        return;
    }
    
    // Get values
    QString isbn = nhapISBN->text().trimmed();
    QString name = nhapTenSach->text().trimmed();
    QString author = nhapTacGia->text().trimmed();
    QString genre = chonTheLoai->currentText().trimmed();
    int year = nhapNamXuatBan->value();
    int pages = nhapSoTrang->value();
    int copies = nhapSoBanSao->value();
    QString location = nhapViTri->text().trimmed();
    
    // Call backend function
    std::string result = ThemDauSachMoi(
        isbn.toStdString(),
        name.toStdString(),
        pages,
        author.toStdString(),
        year,
        genre.toStdString(),
        copies,
        location.toStdString()
    );
    
    if (result.empty()) {
        hienThiThanhCong(QString("Đã thêm đầu sách '%1' với %2 bản sao!").arg(name).arg(copies));
        
        QMessageBox::information(this, "Thành công", 
            QString("✅ Đã thêm thành công đầu sách:\n\n"
                    "📖 %1\n"
                    "✍️ Tác giả: %2\n"
                    "📚 %3 bản sao")
                .arg(name).arg(author).arg(copies));
        
        emit sachDaThem();
        accept();
    } else {
        hienThiLoi(QString::fromStdString(result));
        QMessageBox::warning(this, "Lỗi", QString::fromStdString(result));
    }
}

void ManHinhThemSach::khiNhanHuy() {
    reject();
}

void ManHinhThemSach::setupKeyboardShortcuts() {
    // Ctrl+S - Save
    shortcutSave = new QShortcut(QKeySequence("Ctrl+S"), this);
    connect(shortcutSave, &QShortcut::activated, [this]() {
        if (nutLuu->isEnabled()) {
            khiNhanLuu();
        }
    });
    
    // ESC - Cancel
    shortcutCancel = new QShortcut(QKeySequence("Escape"), this);
    connect(shortcutCancel, &QShortcut::activated, this, &ManHinhThemSach::khiNhanHuy);
}

void ManHinhThemSach::setupTabOrder() {
    // Set logical tab order for keyboard navigation
    setTabOrder(nhapISBN, nhapTenSach);
    setTabOrder(nhapTenSach, nhapTacGia);
    setTabOrder(nhapTacGia, chonTheLoai);
    setTabOrder(chonTheLoai, nhapNamXuatBan);
    setTabOrder(nhapNamXuatBan, nhapSoTrang);
    setTabOrder(nhapSoTrang, nhapSoBanSao);
    setTabOrder(nhapSoBanSao, nhapViTri);
    setTabOrder(nhapViTri, nutXoaForm);
    setTabOrder(nutXoaForm, nutLuu);
    setTabOrder(nutLuu, nutHuy);
    
    // Set initial focus
    nhapISBN->setFocus();
}

void ManHinhThemSach::clearAllFields() {
    // Confirm before clearing
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Xác nhận xóa form",
        "Bạn có chắc muốn xóa tất cả dữ liệu đã nhập?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        nhapISBN->clear();
        nhapTenSach->clear();
        nhapTacGia->clear();
        chonTheLoai->setCurrentIndex(0);
        nhapNamXuatBan->setValue(QDateTime::currentDateTime().date().year());
        nhapSoTrang->setValue(100);
        nhapSoBanSao->setValue(1);
        nhapViTri->clear();
        nhanTrangThai->clear();
        nhapISBN->setFocus();
        
        // Visual feedback
        hienThiThanhCong("Form đã được xóa");
        QTimer::singleShot(2000, [this]() {
            nhanTrangThai->clear();
        });
    }
}
