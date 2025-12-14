#ifndef TOASTNOTIFICATION_H
#define TOASTNOTIFICATION_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class ToastNotification : public QWidget {
    Q_OBJECT
public:
    enum Type { SUCCESS, ERROR, WARNING, INFO };

    explicit ToastNotification(QWidget *parent = nullptr);
    static void show(const QString &message, Type type = INFO, int duration = 3000);

private:
    QLabel *iconLabel;
    QLabel *textLabel;
    QGraphicsOpacityEffect *opacityEffect;
    QPropertyAnimation *anim;
    QTimer *timer;

    void setup(const QString &message, Type type);
    void centerOnScreen();
    void flyIn();
    void fadeOut();
};

#endif // TOASTNOTIFICATION_H
