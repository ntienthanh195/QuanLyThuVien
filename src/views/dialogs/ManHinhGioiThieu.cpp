/**
 * @file ManHinhGioiThieu.cpp
 * @path src/views/dialogs/ManHinhGioiThieu.cpp
 * @description Hộp thoại giới thiệu ứng dụng (About Dialog).
 * Hiển thị thông tin phiên bản, tác giả và bản quyền của phần mềm QuanLyThuVien V2.
 * @project Library Management System (QuanLyThuVien)
 */
#include "views/dialogs/ManHinhGioiThieu.h"

#include <QVBoxLayout>
#include <QFont>
#include <QFrame>
#include "utils/LanguageManager.h"
#include "utils/NgayThang.h"

ManHinhGioiThieu::ManHinhGioiThieu(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(LanguageManager::instance().getText("TITLE_ABOUT"));
    setMinimumSize(600, 600);
    setupUI();
}

ManHinhGioiThieu::~ManHinhGioiThieu() {
}

void ManHinhGioiThieu::setupUI() {
    setStyleSheet("QDialog { background-color: #1E1E2D; }");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    
    // Tieu de - GOLD
    QLabel *titleLabel = new QLabel(LanguageManager::instance().getText("HEADER_SYSTEM_NAME").isEmpty() ? "HỆ THỐNG QUẢN LÝ THƯ VIỆN" : LanguageManager::instance().getText("HEADER_SYSTEM_NAME"), this);
    
    QFont titleFont;
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #FFD700; margin-bottom: 5px; letter-spacing: 1px;"); // Gold
    titleLabel->setWordWrap(true);
    mainLayout->addWidget(titleLabel);
    
    // Phien ban - SAFFRON
    QLabel *versionLabel = new QLabel(LanguageManager::instance().getText("LBL_VERSION"), this);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet(
        "color: #F4A460; " // Saffron
        "font-size: 14pt; "
        "font-weight: bold; "
        "margin-bottom: 5px;"
    );
    mainLayout->addWidget(versionLabel);

    // Ngay xay dung - Chuyen sang Phat Lich (B.E.)
    QString buildDateStr = LanguageManager::instance().getText("LBL_BUILD_DATE");
    // Neu co chuoi ngay "20/12/2025" trong mo ta, chuyen thanh "20/12/2025 (PL 2568)"
    // O day temp fix lay ngay hien tai de format thu
    QString datePart = buildDateStr.split(": ").last();
    QString formattedBDate = buildDateStr;
    if (datePart.contains("/")) {
        formattedBDate = buildDateStr.split(": ").first() + ": " + QString::fromStdString(ChuyenSangPhatLich(datePart.toStdString()));
    }

    QLabel *buildDateLabel = new QLabel(formattedBDate, this);
    buildDateLabel->setAlignment(Qt::AlignCenter);
    buildDateLabel->setStyleSheet("color: #7E8299; margin-bottom: 25px; font-style: italic;");
    mainLayout->addWidget(buildDateLabel);
    
    // Khung thong tin chinh - Saffron Dark Theme
    QFrame *infoFrame = new QFrame(this);
    infoFrame->setStyleSheet(
        "QFrame {"
        "   background-color: #2D1B10;" // Saffron Dark/Brown
        "   border-radius: 12px;"
        "   padding: 25px;"
        "   border: 1px solid #F4A460;" // Saffron border
        "}"
    );
    QVBoxLayout *infoLayout = new QVBoxLayout(infoFrame);
    
    // Thong tin du an
    QString infoContent = LanguageManager::instance().getText("MSG_ABOUT_FEATURES_HTML") + 
                          "<br>" + 
                          LanguageManager::instance().getText("MSG_ABOUT_TECH_HTML") + 
                          "<br>" + 
                          "<p style='color: #F4A460; font-size: 13px; text-align: center; font-weight: bold; margin-top: 10px;'>" + 
                          "Được phát triển bởi Chăm Rốch Thi" + 
                          "</p>" +
                          "<p style='color: #585b68; font-size: 11px; text-align: center; margin-top: 10px;'>" + 
                          LanguageManager::instance().getText("MSG_ABOUT_CREDIT") + 
                          "</p>";

    QLabel *infoLabel = new QLabel(infoContent, infoFrame);
    infoLabel->setWordWrap(true);
    infoLabel->setTextFormat(Qt::RichText);
    infoLabel->setAlignment(Qt::AlignTop);
    
    infoLayout->addWidget(infoLabel);
    mainLayout->addWidget(infoFrame);
    
    mainLayout->addStretch();
    
    // Nut dong
    QPushButton *closeButton = new QPushButton(LanguageManager::instance().getText("BTN_CLOSE"), this);
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setMinimumHeight(44);
    closeButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #3699FF;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 6px;"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #0078D4;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #005A9E;"
        "}"
    );
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    // Center button container (optional, but full width looks good on modal)
    mainLayout->addWidget(closeButton);
}


