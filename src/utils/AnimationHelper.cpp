#include "utils/AnimationHelper.h"
#include <QLabel>
#include <QEasingCurve>
#include <QVariantAnimation>

AnimationHelper::AnimationHelper(QObject *parent)
    : QObject(parent)
{
}

QGraphicsOpacityEffect* AnimationHelper::ensureOpacityEffect(QWidget *widget) {
    QGraphicsOpacityEffect *effect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(widget);
        widget->setGraphicsEffect(effect);
    }
    return effect;
}

void AnimationHelper::fadeIn(QWidget *widget, int duration) {
    if (!widget) return;
    
    QGraphicsOpacityEffect *effect = ensureOpacityEffect(widget);
    
    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity", widget);
    animation->setDuration(duration);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    
    widget->show();
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationHelper::fadeOut(QWidget *widget, int duration) {
    if (!widget) return;
    
    QGraphicsOpacityEffect *effect = ensureOpacityEffect(widget);
    
    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity", widget);
    animation->setDuration(duration);
    animation->setStartValue(1.0);
    animation->setEndValue(0.0);
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    
    QObject::connect(animation, &QPropertyAnimation::finished, widget, &QWidget::hide);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationHelper::slideIn(QWidget *widget, int duration) {
    if (!widget) return;
    
    QPropertyAnimation *animation = new QPropertyAnimation(widget, "geometry", widget);
    animation->setDuration(duration);
    
    QRect startGeometry = widget->geometry();
    startGeometry.moveLeft(-widget->width());
    QRect endGeometry = widget->geometry();
    
    animation->setStartValue(startGeometry);
    animation->setEndValue(endGeometry);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    
    widget->show();
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationHelper::slideUp(QWidget *widget, int duration) {
    if (!widget) return;
    
    QPropertyAnimation *animation = new QPropertyAnimation(widget, "geometry", widget);
    animation->setDuration(duration);
    
    QRect startGeometry = widget->geometry();
    startGeometry.moveTop(startGeometry.top() + 50);
    QRect endGeometry = widget->geometry();
    
    animation->setStartValue(startGeometry);
    animation->setEndValue(endGeometry);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    
    widget->show();
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationHelper::scaleIn(QWidget *widget, int duration) {
    if (!widget) return;
    
    QGraphicsOpacityEffect *effect = ensureOpacityEffect(widget);
    
    QPropertyAnimation *opacityAnim = new QPropertyAnimation(effect, "opacity", widget);
    opacityAnim->setDuration(duration);
    opacityAnim->setStartValue(0.0);
    opacityAnim->setEndValue(1.0);
    
    widget->show();
    opacityAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationHelper::pulse(QWidget *widget, int duration) {
    if (!widget) return;
    
    QGraphicsOpacityEffect *effect = ensureOpacityEffect(widget);
    
    QSequentialAnimationGroup *group = new QSequentialAnimationGroup(widget);
    
    QPropertyAnimation *fadeOut = new QPropertyAnimation(effect, "opacity");
    fadeOut->setDuration(duration / 2);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.6);
    fadeOut->setEasingCurve(QEasingCurve::InOutQuad);
    
    QPropertyAnimation *fadeIn = new QPropertyAnimation(effect, "opacity");
    fadeIn->setDuration(duration / 2);
    fadeIn->setStartValue(0.6);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::InOutQuad);
    
    group->addAnimation(fadeOut);
    group->addAnimation(fadeIn);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationHelper::animateNumber(QLabel *label, int from, int to, int duration) {
    if (!label) return;
    
    QVariantAnimation *animation = new QVariantAnimation(label);
    animation->setDuration(duration);
    animation->setStartValue(from);
    animation->setEndValue(to);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    
    QObject::connect(animation, &QVariantAnimation::valueChanged, [label](const QVariant &value) {
        label->setText(QString::number(value.toInt()));
    });
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationHelper::animateBackgroundColor(QWidget *widget, const QColor &from, const QColor &to, int duration) {
    if (!widget) return;
    
    QVariantAnimation *animation = new QVariantAnimation(widget);
    animation->setDuration(duration);
    animation->setStartValue(from);
    animation->setEndValue(to);
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    
    QObject::connect(animation, &QVariantAnimation::valueChanged, [widget](const QVariant &value) {
        QColor color = value.value<QColor>();
        widget->setStyleSheet(QString("background-color: %1;").arg(color.name()));
    });
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationHelper::shake(QWidget *widget, int duration) {
    if (!widget) return;
    
    QSequentialAnimationGroup *group = new QSequentialAnimationGroup(widget);
    
    QRect originalGeometry = widget->geometry();
    int shakeAmount = 10;
    int steps = 4;
    int stepDuration = duration / (steps * 2);
    
    for (int i = 0; i < steps; i++) {
        QPropertyAnimation *shakeRight = new QPropertyAnimation(widget, "geometry");
        shakeRight->setDuration(stepDuration);
        QRect rightRect = originalGeometry;
        rightRect.moveLeft(originalGeometry.left() + shakeAmount);
        shakeRight->setEndValue(rightRect);
        group->addAnimation(shakeRight);
        
        QPropertyAnimation *shakeLeft = new QPropertyAnimation(widget, "geometry");
        shakeLeft->setDuration(stepDuration);
        QRect leftRect = originalGeometry;
        leftRect.moveLeft(originalGeometry.left() - shakeAmount);
        shakeLeft->setEndValue(leftRect);
        group->addAnimation(shakeLeft);
        
        shakeAmount /= 2;
    }
    
    QPropertyAnimation *restore = new QPropertyAnimation(widget, "geometry");
    restore->setDuration(stepDuration);
    restore->setEndValue(originalGeometry);
    group->addAnimation(restore);
    
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationHelper::startSpinner(QWidget *widget) {
    if (!widget) return;
    
    QGraphicsOpacityEffect *effect = ensureOpacityEffect(widget);
    
    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity", widget);
    animation->setDuration(800);
    animation->setStartValue(0.3);
    animation->setEndValue(1.0);
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->setLoopCount(-1); // Infinite loop
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationHelper::stopSpinner(QWidget *widget) {
    if (!widget) return;
    
    QGraphicsOpacityEffect *effect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    if (effect) {
        effect->setOpacity(1.0);
    }
}
