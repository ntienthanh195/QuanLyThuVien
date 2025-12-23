#include "utils/ResponsiveHelper.h"
#include <QDebug>
#include <QMainWindow>
#include <QSettings>

ResponsiveHelper::ResponsiveHelper(QObject *parent)
    : QObject(parent)
{
}

ResponsiveHelper::ScreenSize ResponsiveHelper::detectScreenSize() {
    QSize screenSize = getScreenResolution();
    int width = screenSize.width();
    int height = screenSize.height();
    
    if (width >= 2560 || height >= 1440) {
        return ScreenSize::ExtraLarge; // 4K, 2K displays
    } else if (width >= 1920 || height >= 1080) {
        return ScreenSize::Large; // Full HD
    } else if (width >= 1366 || height >= 768) {
        return ScreenSize::Medium; // HD+
    } else {
        return ScreenSize::Small; // HD Ready
    }
}

QSize ResponsiveHelper::getScreenResolution() {
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        return screen->size();
    }
    return QSize(1920, 1080); // Default fallback
}

QSize ResponsiveHelper::getRecommendedWindowSize(ScreenSize size) {
    switch (size) {
        case ScreenSize::ExtraLarge:
            return QSize(1800, 1100); // 4K screens
        case ScreenSize::Large:
            return QSize(1600, 950);  // Full HD screens
        case ScreenSize::Medium:
            return QSize(1300, 800);  // Standard laptop
        case ScreenSize::Small:
            return QSize(1200, 700);  // Small laptop
        default:
            return QSize(1400, 850);
    }
}

QSize ResponsiveHelper::getMinimumWindowSize(ScreenSize size) {
    switch (size) {
        case ScreenSize::ExtraLarge:
            return QSize(1400, 850);
        case ScreenSize::Large:
            return QSize(1280, 720);
        case ScreenSize::Medium:
            return QSize(1200, 680);
        case ScreenSize::Small:
            return QSize(1024, 600);
        default:
            return QSize(1200, 680);
    }
}

qreal ResponsiveHelper::getScaleFactor(ScreenSize size) {
    switch (size) {
        case ScreenSize::ExtraLarge:
            return 1.3;  // 30% larger for 4K
        case ScreenSize::Large:
            return 1.15; // 15% larger for Full HD
        case ScreenSize::Medium:
            return 1.0;  // Base size for standard laptop
        case ScreenSize::Small:
            return 0.9;  // 10% smaller for small screens
        default:
            return 1.0;
    }
}

int ResponsiveHelper::getScaledFontSize(int baseFontSize, ScreenSize size) {
    qreal factor = getScaleFactor(size);
    return static_cast<int>(baseFontSize * factor);
}

QFont ResponsiveHelper::getScaledFont(const QFont &baseFont, ScreenSize size) {
    QFont scaledFont = baseFont;
    int scaledSize = getScaledFontSize(baseFont.pointSize(), size);
    scaledFont.setPointSize(scaledSize);
    return scaledFont;
}

int ResponsiveHelper::getScaledSpacing(int baseSpacing, ScreenSize size) {
    qreal factor = getScaleFactor(size);
    return static_cast<int>(baseSpacing * factor);
}

int ResponsiveHelper::getScaledMargin(int baseMargin, ScreenSize size) {
    qreal factor = getScaleFactor(size);
    return static_cast<int>(baseMargin * factor);
}

int ResponsiveHelper::getScaledWidth(int baseWidth, ScreenSize size) {
    qreal factor = getScaleFactor(size);
    return static_cast<int>(baseWidth * factor);
}

int ResponsiveHelper::getScaledHeight(int baseHeight, ScreenSize size) {
    qreal factor = getScaleFactor(size);
    return static_cast<int>(baseHeight * factor);
}

qreal ResponsiveHelper::getScreenDpiRatio() {
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        return screen->devicePixelRatio();
    }
    return 1.0;
}

int ResponsiveHelper::applyDpiScaling(int value) {
    qreal dpiRatio = getScreenDpiRatio();
    return static_cast<int>(value * dpiRatio);
}

void ResponsiveHelper::applyResponsiveLayout(QWidget *widget) {
    if (!widget) return;
    
    ScreenSize size = detectScreenSize();
    QSize recommendedSize = getRecommendedWindowSize(size);
    
    // Set size hints for the widget
    widget->setMinimumSize(getMinimumWindowSize(size));
    widget->resize(recommendedSize);
    
    // Apply global font scaling based on physical screen size
    if (QApplication *app = qobject_cast<QApplication*>(QApplication::instance())) {
        applyGlobalFontScaling(app);
    }

    // Enable size grip for resizing
    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(widget)) {
        mainWindow->setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowTabbedDocks);
    }
}

qreal ResponsiveHelper::getPhysicalDiagonalInches() {
    QScreen *screen = QApplication::primaryScreen();
    if (!screen) return 13.3; // fallback typical laptop

    QSizeF physicalMM = screen->physicalSize(); // in millimeters
    qreal w = physicalMM.width();
    qreal h = physicalMM.height();
    qreal diagonalMM = std::sqrt(w * w + h * h);
    qreal inches = diagonalMM / 25.4; // 1 inch = 25.4 mm
    return inches;
}

void ResponsiveHelper::applyGlobalFontScaling(QApplication *app, int basePointSize) {
    if (!app) return;

    qreal inches = getPhysicalDiagonalInches();
    qreal scale = 1.0;

    // Map typical laptop/desktop diagonal sizes to scale factors
    if (inches <= 10.5) {
        scale = 0.90; // small tablets / netbooks
    } else if (inches <= 11.5) {
        scale = 0.95;
    } else if (inches <= 12.5) {
        scale = 1.00;
    } else if (inches <= 13.5) {
        scale = 1.05;
    } else if (inches <= 14.5) {
        scale = 1.10;
    } else if (inches <= 15.5) {
        scale = 1.15;
    } else if (inches <= 16.5) {
        scale = 1.20;
    } else {
        scale = 1.25; // large desktops/4K
    }

    QFont f = app->font();
    int newPoint = static_cast<int>(std::round(basePointSize * scale));
    f.setPointSize(newPoint);
    app->setFont(f);

    qDebug() << "Applied global font scaling:" << scale << "(inches=" << inches << ") new point size=" << newPoint;
}

void ResponsiveHelper::centerWindow(QWidget *widget) {
    if (!widget) return;
    
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - widget->width()) / 2;
        int y = (screenGeometry.height() - widget->height()) / 2;
        widget->move(x, y);
    }
}

void ResponsiveHelper::saveWindowGeometry(QWidget *widget, const QString &key) {
    if (!widget) return;
    
    QSettings settings("QuanLyThuVien", "WindowState");
    settings.setValue(key + "/geometry", widget->saveGeometry());
    settings.setValue(key + "/maximized", widget->isMaximized());
    
    qDebug() << "Đã lưu trạng thái cửa sổ:" << widget->size() << "| Maximized:" << widget->isMaximized();
}

void ResponsiveHelper::restoreWindowGeometry(QWidget *widget, const QString &key) {
    if (!widget) return;
    
    QSettings settings("QuanLyThuVien", "WindowState");
    
    // Khôi phục geometry nếu có
    QByteArray savedGeometry = settings.value(key + "/geometry").toByteArray();
    if (!savedGeometry.isEmpty()) {
        widget->restoreGeometry(savedGeometry);
        
        // Khôi phục trạng thái maximize
        bool wasMaximized = settings.value(key + "/maximized", false).toBool();
        if (wasMaximized) {
            widget->showMaximized();
        }
        
        qDebug() << "Khôi phục trạng thái cửa sổ:" << widget->size() << "| Maximized:" << wasMaximized;
    } else {
        // Lần đầu chạy - tự động chọn kích thước phù hợp
        qDebug() << "Lần đầu khởi chạy - thiết lập kích thước tự động";
        applyResponsiveLayout(widget);
        centerWindow(widget);
    }
}

void ResponsiveHelper::setupResizableWindow(QWidget *widget) {
    if (!widget) return;
    
    ScreenSize size = detectScreenSize();
    QSize minSize = getMinimumWindowSize(size);
    
    // Cho phép resize với minimum size
    widget->setMinimumSize(minSize);
    
    // Không giới hạn maximum size (cho phép maximize toàn màn hình)
    widget->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    
    // Enable window size grip
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    qDebug() << "Window resizable - Min size:" << minSize;
}

QString ResponsiveHelper::getScreenInfo() {
    QSize screenSize = getScreenResolution();
    ScreenSize category = detectScreenSize();
    qreal dpiRatio = getScreenDpiRatio();
    
    QString categoryStr;
    switch (category) {
        case ScreenSize::Small:
            categoryStr = "Small (< 1366x768)";
            break;
        case ScreenSize::Medium:
            categoryStr = "Medium (1366x768 - 1920x1080)";
            break;
        case ScreenSize::Large:
            categoryStr = "Large (1920x1080 - 2560x1440)";
            break;
        case ScreenSize::ExtraLarge:
            categoryStr = "Extra Large (>= 2560x1440)";
            break;
    }
    
    return QString("Screen: %1x%2 | Category: %3 | DPI Ratio: %4 | Scale Factor: %5x")
        .arg(screenSize.width())
        .arg(screenSize.height())
        .arg(categoryStr)
        .arg(dpiRatio, 0, 'f', 2)
        .arg(getScaleFactor(category), 0, 'f', 2);
}
