#include "views/dialogs/ManHinhDemoPhase1.h"
#include <QGroupBox>
#include <QFormLayout>
#include <QScrollArea>
#include <QRegularExpressionValidator>
#include <QMessageBox>

ManHinhDemoPhase1::ManHinhDemoPhase1(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("🎨 Demo Phase 1: Button System & Input Fields");
    setMinimumSize(900, 700);
    
    khoiTaoGiaoDien();
    khoiTaoConnections();
}

ManHinhDemoPhase1::~ManHinhDemoPhase1() {
}

void ManHinhDemoPhase1::khoiTaoGiaoDien() {
    // Scroll area cho responsive
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    QWidget *scrollContent = new QWidget();
    layoutChinh = new QVBoxLayout(scrollContent);
    layoutChinh->setSpacing(32);
    layoutChinh->setContentsMargins(32, 32, 32, 32);
    
    // Title
    QLabel *title = new QLabel("🎨 Demo Components Phase 1", scrollContent);
    QFont titleFont = title->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setStyleSheet("color: #60A5FA; background: transparent;");
    layoutChinh->addWidget(title);
    
    // Demo button system
    taoNhomNutBam();
    
    // Demo input fields
    taoNhomTruongNhapLieu();
    
    layoutChinh->addStretch();
    
    scrollArea->setWidget(scrollContent);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(scrollArea);
    
    // Apply dark theme
    setStyleSheet(
        "QDialog {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0F172A, stop:1 #1E293B);"
        "}"
        "QScrollArea {"
        "    background: transparent;"
        "    border: none;"
        "}"
        "QGroupBox {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1E293B, stop:1 #334155);"
        "    border: 2px solid #475569;"
        "    border-radius: 12px;"
        "    padding: 20px;"
        "    margin-top: 16px;"
        "    font-size: 16px;"
        "    font-weight: 700;"
        "    color: #F1F5F9;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 16px;"
        "    padding: 0 8px;"
        "}"
    );
}

void ManHinhDemoPhase1::taoNhomNutBam() {
    QGroupBox *groupNut = new QGroupBox("🎯 Hệ Thống Nút Bấm", this);
    QVBoxLayout *layoutNut = new QVBoxLayout(groupNut);
    layoutNut->setSpacing(24);
    
    // Row 1: Primary & Secondary
    QHBoxLayout *row1 = new QHBoxLayout();
    row1->setSpacing(16);
    
    nutLuuChinh = TaoNutBam::taoNutChinh("✓ Lưu Thông Tin");
    connect(nutLuuChinh, &QPushButton::clicked, this, [this]() {
        xuLyNutBamDemo("Primary - Lưu");
    });
    
    nutHuyChinh = TaoNutBam::taoNutPhu("✕ Hủy Bỏ");
    connect(nutHuyChinh, &QPushButton::clicked, this, [this]() {
        xuLyNutBamDemo("Secondary - Hủy");
    });
    
    row1->addWidget(nutLuuChinh);
    row1->addWidget(nutHuyChinh);
    
    // Row 2: Danger & Success
    QHBoxLayout *row2 = new QHBoxLayout();
    row2->setSpacing(16);
    
    nutXoaNguyHiem = TaoNutBam::taoNutNguyHiem("🗑️ Xóa Vĩnh Viễn");
    connect(nutXoaNguyHiem, &QPushButton::clicked, this, [this]() {
        xuLyNutBamDemo("Danger - Xóa");
    });
    
    nutHoanTatThanhCong = TaoNutBam::taoNutThanhCong("✓ Hoàn Tất");
    connect(nutHoanTatThanhCong, &QPushButton::clicked, this, [this]() {
        xuLyNutBamDemo("Success - Hoàn tất");
    });
    
    row2->addWidget(nutXoaNguyHiem);
    row2->addWidget(nutHoanTatThanhCong);
    
    // Row 3: Ghost & Warning
    QHBoxLayout *row3 = new QHBoxLayout();
    row3->setSpacing(16);
    
    QPushButton *nutExport = TaoNutBam::taoNutNhe("📤 Export CSV");
    connect(nutExport, &QPushButton::clicked, this, [this]() {
        xuLyNutBamDemo("Ghost - Export");
    });
    
    QPushButton *nutCanhBao = TaoNutBam::taoNut("⚠️ Cảnh Báo", NUT_WARNING, NUT_TRUNG_BINH);
    connect(nutCanhBao, &QPushButton::clicked, this, [this]() {
        xuLyNutBamDemo("Warning - Cảnh báo");
    });
    
    row3->addWidget(nutExport);
    row3->addWidget(nutCanhBao);
    
    // Row 4: Size variations
    QHBoxLayout *row4 = new QHBoxLayout();
    row4->setSpacing(16);
    
    QPushButton *nutLon = TaoNutBam::taoNut("Nút Lớn (50px)", NUT_PRIMARY, NUT_LON);
    QPushButton *nutTrungBinh = TaoNutBam::taoNut("Nút Trung Bình (42px)", NUT_PRIMARY, NUT_TRUNG_BINH);
    QPushButton *nutNho = TaoNutBam::taoNut("Nút Nhỏ (36px)", NUT_PRIMARY, NUT_NHO);
    
    row4->addWidget(nutLon);
    row4->addWidget(nutTrungBinh);
    row4->addWidget(nutNho);
    
    layoutNut->addLayout(row1);
    layoutNut->addLayout(row2);
    layoutNut->addLayout(row3);
    layoutNut->addLayout(row4);
    
    layoutChinh->addWidget(groupNut);
}

void ManHinhDemoPhase1::taoNhomTruongNhapLieu() {
    QGroupBox *groupInput = new QGroupBox("📝 Trường Nhập Liệu", this);
    QVBoxLayout *layoutInput = new QVBoxLayout(groupInput);
    layoutInput->setSpacing(20);
    
    // Search field
    truongTimKiem = new TruongTimKiem(groupInput);
    truongTimKiem->datPlaceholder("Tìm kiếm độc giả, sách...");
    layoutInput->addWidget(truongTimKiem);
    
    // Họ tên (bắt buộc)
    truongHoTen = new TruongNhapLieu("Họ và tên", groupInput);
    truongHoTen->datPlaceholder("Nhập họ tên đầy đủ");
    truongHoTen->datBatBuoc(true);
    truongHoTen->datTextGiupDo("Ví dụ: Nguyễn Văn A");
    layoutInput->addWidget(truongHoTen);
    
    // Email với validation
    truongEmail = new TruongNhapLieu("Email", groupInput);
    truongEmail->datPlaceholder("example@email.com");
    truongEmail->datBatBuoc(true);
    truongEmail->datTextGiupDo("Email sẽ dùng để nhận thông báo");
    
    // Email validator
    QRegularExpressionValidator *emailValidator = new QRegularExpressionValidator(
        QRegularExpression("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"),
        this
    );
    truongEmail->datValidator(emailValidator);
    layoutInput->addWidget(truongEmail);
    
    // Số điện thoại
    truongSDT = new TruongNhapLieu("Số điện thoại", groupInput);
    truongSDT->datPlaceholder("0xxx xxx xxx");
    truongSDT->datTextGiupDo("10 chữ số, bắt đầu bằng 0");
    truongSDT->datMaxLength(10);
    
    // Phone validator
    QRegularExpressionValidator *phoneValidator = new QRegularExpressionValidator(
        QRegularExpression("^0\\d{9}$"),
        this
    );
    truongSDT->datValidator(phoneValidator);
    layoutInput->addWidget(truongSDT);
    
    // Mật khẩu
    truongMatKhau = new TruongNhapLieu("Mật khẩu", groupInput);
    truongMatKhau->datPlaceholder("Nhập mật khẩu");
    truongMatKhau->datPasswordMode(true);
    truongMatKhau->datTextGiupDo("Tối thiểu 6 ký tự");
    truongMatKhau->datBatBuoc(true);
    layoutInput->addWidget(truongMatKhau);
    
    // Nút validate
    QHBoxLayout *layoutValidate = new QHBoxLayout();
    QPushButton *nutValidate = TaoNutBam::taoNutChinh("✓ Kiểm Tra Form");
    connect(nutValidate, &QPushButton::clicked, this, &ManHinhDemoPhase1::xuLyValidation);
    
    QPushButton *nutReset = TaoNutBam::taoNutPhu("🔄 Reset Form");
    connect(nutReset, &QPushButton::clicked, this, [this]() {
        truongHoTen->datGiaTri("");
        truongEmail->datGiaTri("");
        truongSDT->datGiaTri("");
        truongMatKhau->datGiaTri("");
        truongTimKiem->xoa();
        
        truongHoTen->xoaLoi();
        truongEmail->xoaLoi();
        truongSDT->xoaLoi();
        truongMatKhau->xoaLoi();
    });
    
    layoutValidate->addWidget(nutValidate);
    layoutValidate->addWidget(nutReset);
    layoutInput->addLayout(layoutValidate);
    
    layoutChinh->addWidget(groupInput);
}

void ManHinhDemoPhase1::khoiTaoConnections() {
    // Real-time validation cho email
    connect(truongEmail, &TruongNhapLieu::textThayDoi, this, [this](const QString &text) {
        if (!text.isEmpty()) {
            if (truongEmail->hopLe()) {
                truongEmail->hienThiThanhCong();
            } else {
                truongEmail->hienThiLoi("Email không hợp lệ");
            }
        } else {
            truongEmail->xoaLoi();
        }
    });
    
    // Real-time validation cho SĐT
    connect(truongSDT, &TruongNhapLieu::textThayDoi, this, [this](const QString &text) {
        if (!text.isEmpty()) {
            if (truongSDT->hopLe()) {
                truongSDT->hienThiThanhCong();
            } else {
                truongSDT->hienThiLoi("SĐT phải có 10 chữ số, bắt đầu bằng 0");
            }
        } else {
            truongSDT->xoaLoi();
        }
    });
    
    // Search demo
    connect(truongTimKiem, &TruongTimKiem::timKiem, this, [](const QString &tuKhoa) {
        qDebug() << "Tìm kiếm:" << tuKhoa;
    });
}

void ManHinhDemoPhase1::xuLyNutBamDemo(const QString &ten) {
    QMessageBox msg(this);
    msg.setWindowTitle("Demo Button Click");
    msg.setText(QString("Bạn đã click: %1").arg(ten));
    msg.setIcon(QMessageBox::Information);
    msg.setStandardButtons(QMessageBox::Ok);
    
    // Style for message box
    msg.setStyleSheet(
        "QMessageBox {"
        "    background: #1E293B;"
        "    color: #F1F5F9;"
        "}"
        "QPushButton {"
        "    background: #3B82F6;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 8px 20px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: #2563EB;"
        "}"
    );
    
    msg.exec();
}

void ManHinhDemoPhase1::xuLyValidation() {
    bool hopLe = true;
    QString thongDiep = "Kết quả kiểm tra:\n\n";
    
    // Validate họ tên
    if (truongHoTen->laTrong()) {
        truongHoTen->hienThiLoi("Họ tên là bắt buộc");
        hopLe = false;
        thongDiep += "❌ Họ tên: Chưa nhập\n";
    } else {
        truongHoTen->hienThiThanhCong();
        thongDiep += "✓ Họ tên: Hợp lệ\n";
    }
    
    // Validate email
    if (!truongEmail->hopLe()) {
        if (truongEmail->laTrong()) {
            truongEmail->hienThiLoi("Email là bắt buộc");
        } else {
            truongEmail->hienThiLoi("Email không đúng định dạng");
        }
        hopLe = false;
        thongDiep += "❌ Email: Không hợp lệ\n";
    } else {
        truongEmail->hienThiThanhCong();
        thongDiep += "✓ Email: Hợp lệ\n";
    }
    
    // Validate SĐT
    if (!truongSDT->hopLe() && !truongSDT->laTrong()) {
        truongSDT->hienThiLoi("SĐT phải có 10 chữ số");
        hopLe = false;
        thongDiep += "❌ SĐT: Không đúng định dạng\n";
    } else if (!truongSDT->laTrong()) {
        truongSDT->hienThiThanhCong();
        thongDiep += "✓ SĐT: Hợp lệ\n";
    } else {
        thongDiep += "⚪ SĐT: Chưa nhập (không bắt buộc)\n";
    }
    
    // Validate mật khẩu
    if (truongMatKhau->laTrong()) {
        truongMatKhau->hienThiLoi("Mật khẩu là bắt buộc");
        hopLe = false;
        thongDiep += "❌ Mật khẩu: Chưa nhập\n";
    } else if (truongMatKhau->layGiaTri().length() < 6) {
        truongMatKhau->hienThiLoi("Mật khẩu phải có ít nhất 6 ký tự");
        hopLe = false;
        thongDiep += "❌ Mật khẩu: Quá ngắn\n";
    } else {
        truongMatKhau->hienThiThanhCong();
        thongDiep += "✓ Mật khẩu: Hợp lệ\n";
    }
    
    // Show result
    QMessageBox msg(this);
    msg.setWindowTitle(hopLe ? "✓ Validation Thành Công" : "❌ Có Lỗi Validation");
    msg.setText(thongDiep);
    msg.setIcon(hopLe ? QMessageBox::Information : QMessageBox::Warning);
    msg.setStandardButtons(QMessageBox::Ok);
    
    msg.setStyleSheet(
        "QMessageBox {"
        "    background: #1E293B;"
        "    color: #F1F5F9;"
        "    min-width: 400px;"
        "}"
        "QLabel {"
        "    color: #F1F5F9;"
        "    font-size: 14px;"
        "}"
        "QPushButton {"
        "    background: " + QString(hopLe ? "#10B981" : "#EF4444") + ";"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 10px 24px;"
        "    font-weight: 700;"
        "}"
        "QPushButton:hover {"
        "    background: " + QString(hopLe ? "#059669" : "#DC2626") + ";"
        "}"
    );
    
    msg.exec();
}
