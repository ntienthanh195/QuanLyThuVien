#include "utils/ToastNotification.h"
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QTimer>

ToastNotification::ToastNotification(QWidget *parent) 
    : QWidget(parent, Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint) 
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    
    // Setup UI
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 15, 20, 15);
    layout->setSpacing(15);

    iconLabel = new QLabel(this);
    iconLabel->setFixedSize(24, 24);
    
    textLabel = new QLabel(this);
    textLabel->setStyleSheet("color: white; font-weight: 600; font-size: 14px;");

    layout->addWidget(iconLabel);
    layout->addWidget(textLabel);

    // Animation setup
    opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);
    
    anim = new QPropertyAnimation(opacityEffect, "opacity", this);
    anim->setDuration(300);

    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, &ToastNotification::fadeOut);
}

void ToastNotification::show(const QString &message, Type type, int duration) {
    ToastNotification *toast = new ToastNotification();
    toast->setup(message, type);
    toast->flyIn();
    
    // Auto delete after duration + fade out
    QTimer::singleShot(duration + 600, toast, &QObject::deleteLater);
}

void ToastNotification::setup(const QString &message, Type type) {
    textLabel->setText(message);
    QString bgColor;
    QString icon;

    switch (type) {
        case SUCCESS:
            bgColor = "#2E7D32"; // Green
            icon = "✓"; 
            break;
        case ERROR:
            bgColor = "#D32F2F"; // Red
            icon = "✕";
            break;
        case WARNING:
            bgColor = "#F9A825"; // Yellow
            icon = "⚠️";
            break;
        case INFO:
        default:
            bgColor = "#1976D2"; // Blue
            icon = "ℹ";
            break;
    }

    setStyleSheet(QString(
        "QWidget { background-color: %1; border-radius: 8px; border: 1px solid #FFFFFF20; }"
        "QLabel { color: white; background: transparent; }"
    ).arg(bgColor));
    
    iconLabel->setText(icon);
    iconLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    
    adjustSize();
    centerOnScreen();
}

void ToastNotification::centerOnScreen() {
    if (QScreen *screen = QApplication::primaryScreen()) {
        QRect screenGeom = screen->availableGeometry();
        int x = screenGeom.width() / 2 - width() / 2;
        int y = screenGeom.height() - height() - 100; // Bottom center
        move(x + screenGeom.x(), y + screenGeom.y());
    }
}

void ToastNotification::flyIn() {
    QWidget::show();
    opacityEffect->setOpacity(0);
    anim->setStartValue(0);
    anim->setEndValue(1);
    anim->start();
    timer->start(3000);
}

void ToastNotification::fadeOut() {
    anim->setStartValue(1);
    anim->setEndValue(0);
    anim->start();
}
