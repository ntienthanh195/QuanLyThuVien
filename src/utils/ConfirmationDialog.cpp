#include "utils/ConfirmationDialog.h"
#include "utils/AnimationHelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QFile>
#include <QPixmap>

ConfirmationDialog::ConfirmationDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    applyTheme();
    
    // ESC to cancel
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void ConfirmationDialog::setupUI() {
    setMinimumWidth(500);
    setModal(true);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // Icon and title row
    QHBoxLayout *headerLayout = new QHBoxLayout();
    
    iconLabel = new QLabel();
    QFont iconFont;
    iconFont.setPointSize(32);
    iconLabel->setFont(iconFont);
    iconLabel->setFixedSize(48, 48);
    iconLabel->setAlignment(Qt::AlignCenter);
    
    titleLabel = new QLabel();
    QFont titleFont;
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    
    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(titleLabel, 1);
    
    // Message
    messageLabel = new QLabel();
    messageLabel->setWordWrap(true);
    QFont messageFont;
    messageFont.setPointSize(11);
    messageLabel->setFont(messageFont);
    
    // Details (optional, hidden by default)
    detailsEdit = new QTextEdit();
    detailsEdit->setReadOnly(true);
    detailsEdit->setMaximumHeight(150);
    detailsEdit->hide();
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    cancelButton = new QPushButton("Hủy");
    cancelButton->setMinimumSize(100, 36);
    cancelButton->setCursor(Qt::PointingHandCursor);
    
    confirmButton = new QPushButton("Xác nhận");
    confirmButton->setMinimumSize(100, 36);
    confirmButton->setCursor(Qt::PointingHandCursor);
    confirmButton->setDefault(true);
    
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(confirmButton);
    
    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(messageLabel);
    mainLayout->addWidget(detailsEdit);
    mainLayout->addLayout(buttonLayout);
    
    // Connections
    connect(confirmButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void ConfirmationDialog::applyTheme() {
    setStyleSheet(
        "QDialog {"
        "    background: #1E1E2D;"
        "}"
        "QLabel {"
        "    color: #F1F5F9;"
        "    background: transparent;"
        "}"
        "QTextEdit {"
        "    background: #2B2B40;"
        "    color: #E2E8F0;"
        "    border: 1px solid #3B3B54;"
        "    border-radius: 8px;"
        "    padding: 12px;"
        "    font-family: 'Consolas', monospace;"
        "    font-size: 10pt;"
        "}"
        "QPushButton {"
        "    background: #3B82F6;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 8px 24px;"
        "    font-weight: 600;"
        "    font-size: 11pt;"
        "}"
        "QPushButton:hover {"
        "    background: #2563EB;"
        "}"
        "QPushButton:pressed {"
        "    background: #1E40AF;"
        "}"
        "QPushButton#cancelButton {"
        "    background: #475569;"
        "}"
        "QPushButton#cancelButton:hover {"
        "    background: #334155;"
        "}"
    );
    
    cancelButton->setObjectName("cancelButton");
}

void ConfirmationDialog::setTitle(const QString &title) {
    setWindowTitle(title);
    titleLabel->setText(title);
}

void ConfirmationDialog::setMessage(const QString &message) {
    messageLabel->setText(message);
}

void ConfirmationDialog::setDetailedInfo(const QString &details) {
    if (!details.isEmpty()) {
        detailsEdit->setPlainText(details);
        detailsEdit->show();
    } else {
        detailsEdit->hide();
    }
}

void ConfirmationDialog::setType(Type type) {
    QString iconPath = getIconForType(type);
    if (!iconPath.isEmpty() && QFile::exists(iconPath)) {
        iconLabel->setText("");
        QPixmap pixmap(iconPath);
        iconLabel->setPixmap(pixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        // Fallback or empty
        iconLabel->setText("");
    }
    
    QString color = getColorForType(type);
    
    // Change confirm button color based on type
    if (type == Type::Delete) {
        confirmButton->setStyleSheet(
            "QPushButton {"
            "    background: #EF4444;"
            "    color: white;"
            "    border: none;"
            "    border-radius: 8px;"
            "    padding: 8px 24px;"
            "    font-weight: 600;"
            "}"
            "QPushButton:hover {"
            "    background: #DC2626;"
            "}"
            "QPushButton:pressed {"
            "    background: #B91C1C;"
            "}"
        );
    } else if (type == Type::Warning) {
        confirmButton->setStyleSheet(
            "QPushButton {"
            "    background: #F59E0B;"
            "    color: white;"
            "    border: none;"
            "    border-radius: 8px;"
            "    padding: 8px 24px;"
            "    font-weight: 600;"
            "}"
            "QPushButton:hover {"
            "    background: #D97706;"
            "}"
        );
    }
}

void ConfirmationDialog::setConfirmButtonText(const QString &text) {
    confirmButton->setText(text);
}

void ConfirmationDialog::setCancelButtonText(const QString &text) {
    cancelButton->setText(text);
}

QString ConfirmationDialog::getIconForType(Type type) const {
    switch (type) {
        case Type::Delete:
            return ":/icons/trash.png";
        case Type::Warning:
            return ":/icons/warning.png";
        case Type::Info:
            return ":/icons/info.png";
        case Type::Question:
        default:
            return ":/icons/help.png";
    }
}

QString ConfirmationDialog::getColorForType(Type type) const {
    switch (type) {
        case Type::Delete:
            return "#EF4444";
        case Type::Warning:
            return "#F59E0B";
        case Type::Info:
            return "#3B82F6";
        case Type::Question:
        default:
            return "#8B5CF6";
    }
}

bool ConfirmationDialog::confirm(QWidget *parent, const QString &title,
                                 const QString &message, const QString &details,
                                 Type type) {
    ConfirmationDialog dialog(parent);
    dialog.setTitle(title);
    dialog.setMessage(message);
    dialog.setDetailedInfo(details);
    dialog.setType(type);
    
    // Animation
    AnimationHelper::fadeIn(&dialog, 200);
    
    return dialog.exec() == QDialog::Accepted;
}

bool ConfirmationDialog::confirmDelete(QWidget *parent, const QString &itemName,
                                      const QString &details) {
    return confirm(parent,
                  "Xác nhận xóa",
                  QString("Bạn có chắc chắn muốn xóa:\n\n%1\n\n"
                         "Thao tác này không thể hoàn tác!").arg(itemName),
                  details,
                  Type::Delete);
}
