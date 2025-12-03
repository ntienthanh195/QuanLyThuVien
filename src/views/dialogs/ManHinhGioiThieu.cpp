#include "views/dialogs/ManHinhGioiThieu.h"
#include <QVBoxLayout>
#include <QFont>
#include <QFrame>

ManHinhGioiThieu::ManHinhGioiThieu(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Về Hệ Thống 📚");
    setMinimumSize(600, 600);
    setupUI();
}

ManHinhGioiThieu::~ManHinhGioiThieu() {
}

void ManHinhGioiThieu::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // Tieu de
    QLabel *titleLabel = new QLabel("📚 HỆ THỐNG QUẢN LÝ THƯ VIỆN", this);
    QFont titleFont;
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);
    
    // Phien ban
    QLabel *versionLabel = new QLabel("🔖 Phiên bản 4.0.0 (Qt 6.10.1)", this);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet(
        "color: #27ae60; "
        "font-size: 13pt; "
        "font-weight: bold; "
        "margin-bottom: 5px;"
    );
    mainLayout->addWidget(versionLabel);
    
    // Ngay xay dung
    QLabel *buildDateLabel = new QLabel("📅 Build: " __DATE__ " " __TIME__, this);
    buildDateLabel->setAlignment(Qt::AlignCenter);
    buildDateLabel->setStyleSheet("color: #95a5a6; margin-bottom: 20px;");
    mainLayout->addWidget(buildDateLabel);
    
    // Khung thong tin chinh
    QFrame *infoFrame = new QFrame(this);
    infoFrame->setStyleSheet(
        "QFrame {"
        "   background-color: #ecf0f1;"
        "   border-radius: 10px;"
        "   padding: 15px;"
        "}"
    );
    QVBoxLayout *infoLayout = new QVBoxLayout(infoFrame);
    
    // Thong tin du an
    QLabel *infoLabel = new QLabel(
        "<p><b>📝 Mô tả:</b></p>"
        "<p>Hệ thống quản lý thư viện chuyên nghiệp với giao diện Qt6 hiện đại.</p>"
        "<br>"
        "<p><b>🏗️ Cấu trúc dữ liệu:</b></p>"
        "<ul style='margin-left: 20px;'>"
        "<li>📖 Đầu sách: Mảng con trỏ (sắp xếp theo tên)</li>"
        "<li>📚 Danh mục sách: Danh sách liên kết đơn</li>"
        "<li>👤 Độc giả: Cây nhị phân tìm kiếm AVL</li>"
        "<li>📝 Mượn trả: Danh sách liên kết đơn</li>"
        "</ul>"
        "<br>"
        "<p><b>✨ Tính năng mới:</b></p>"
        "<ul style='margin-left: 20px;'>"
        "<li>🥇 Top 10 sách được mượn nhiều nhất</li>"
        "<li>⏰ Theo dõi độc giả mượn quá hạn</li>"
        "<li>💾 Backup dữ liệu tự động khi thoát</li>"
        "<li>🎨 Giao diện tối ưu với màu sắc chuyên nghiệp</li>"
        "<li>⌨️ Phím tắt tiện lợi (Ctrl+F, F5, Esc...)</li>"
        "<li>💡 Tooltips hướng dẫn chi tiết</li>"
        "</ul>"
        "<br>"
        "<p><b>🛠️ Công nghệ:</b></p>"
        "<p style='margin-left: 20px;'>C++17, Qt 6.10.1, MinGW 13.1.0</p>"
        "<br>"
        "<p><b>👨‍💻 Phát triển bởi:</b></p>"
        "<p style='margin-left: 20px;'>Nhóm sinh viên PTIT - Lớp D22CQCN01-B</p>"
        "<p style='margin-left: 20px; color: #7f8c8d;'>Môn: Cấu trúc dữ liệu và giải thuật</p>",
        infoFrame
    );
    infoLabel->setWordWrap(true);
    infoLabel->setTextFormat(Qt::RichText);
    infoLayout->addWidget(infoLabel);
    
    mainLayout->addWidget(infoFrame);
    
    mainLayout->addStretch();
    
    // Nut dong
    QPushButton *closeButton = new QPushButton("Đóng", this);
    closeButton->setMinimumHeight(40);
    closeButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #3498db;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2980b9;"
        "}"
    );
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeButton);
}

