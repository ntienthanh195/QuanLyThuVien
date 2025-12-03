#ifndef ANIMATIONHELPER_H
#define ANIMATIONHELPER_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>

/**
 * @brief Helper class for smooth UI animations
 */
class AnimationHelper : public QObject {
    Q_OBJECT

public:
    explicit AnimationHelper(QObject *parent = nullptr);
    
    // Fade animations
    static void fadeIn(QWidget *widget, int duration = 300);
    static void fadeOut(QWidget *widget, int duration = 300);
    
    // Slide animations
    static void slideIn(QWidget *widget, int duration = 300);
    static void slideUp(QWidget *widget, int duration = 300);
    
    // Scale animations
    static void scaleIn(QWidget *widget, int duration = 200);
    static void pulse(QWidget *widget, int duration = 500);
    
    // Number counter animation
    static void animateNumber(QLabel *label, int from, int to, int duration = 1000);
    
    // Color transition
    static void animateBackgroundColor(QWidget *widget, const QColor &from, const QColor &to, int duration = 300);
    
    // Shake animation (for errors)
    static void shake(QWidget *widget, int duration = 500);
    
    // Loading spinner
    static void startSpinner(QWidget *widget);
    static void stopSpinner(QWidget *widget);

private:
    static QGraphicsOpacityEffect* ensureOpacityEffect(QWidget *widget);
};

#endif // ANIMATIONHELPER_H
