#ifndef RESPONSIVEHELPER_H
#define RESPONSIVEHELPER_H

#include <QObject>
#include <QScreen>
#include <QApplication>
#include <QWidget>
#include <QFont>

/**
 * @brief Helper class cho responsive design
 * Tự động điều chỉnh UI dựa trên kích thước màn hình
 */
class ResponsiveHelper : public QObject {
    Q_OBJECT

public:
    // Screen size categories
    enum class ScreenSize {
        Small,      // < 1366x768 (netbook, small laptop)
        Medium,     // 1366x768 - 1920x1080 (standard laptop)
        Large,      // 1920x1080 - 2560x1440 (desktop, large laptop)
        ExtraLarge  // >= 2560x1440 (4K, high-end)
    };

    explicit ResponsiveHelper(QObject *parent = nullptr);
    
    // Detect screen size category
    static ScreenSize detectScreenSize();
    static QSize getScreenResolution();
    
    // Recommended window size for each screen category
    static QSize getRecommendedWindowSize(ScreenSize size = detectScreenSize());
    static QSize getMinimumWindowSize(ScreenSize size = detectScreenSize());
    
    // Font size scaling
    static int getScaledFontSize(int baseFontSize, ScreenSize size = detectScreenSize());
    static QFont getScaledFont(const QFont &baseFont, ScreenSize size = detectScreenSize());
    
    // Layout spacing scaling
    static int getScaledSpacing(int baseSpacing, ScreenSize size = detectScreenSize());
    static int getScaledMargin(int baseMargin, ScreenSize size = detectScreenSize());
    
    // Widget size scaling
    static int getScaledWidth(int baseWidth, ScreenSize size = detectScreenSize());
    static int getScaledHeight(int baseHeight, ScreenSize size = detectScreenSize());
    
    // DPI awareness
    static qreal getScreenDpiRatio();
    static int applyDpiScaling(int value);
    
    // Apply responsive layout to widget
    static void applyResponsiveLayout(QWidget *widget);
    static void centerWindow(QWidget *widget);
    
    // Debug info
    static QString getScreenInfo();

private:
    static qreal getScaleFactor(ScreenSize size);
};

#endif // RESPONSIVEHELPER_H
