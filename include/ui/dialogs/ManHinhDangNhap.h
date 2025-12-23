#ifndef MANHINHDANGNHAP_H
#define MANHINHDANGNHAP_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFrame>
#include <QMouseEvent>
#include <QPoint>

class ManHinhDangNhap : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhDangNhap(QWidget *parent = nullptr);
    static bool yeuCauDangNhap();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onDangNhap();
    void onThoat();
    void onToggleMode();

private:
    void setupUI();
    void updateResponsiveLayout();

    // UI Elements
    QWidget *m_mainContainer;
    QWidget *m_leftPanel;  // Image side
    QWidget *m_rightPanel; // Form side
    
    QLabel *m_lblImageBackground;
    
    QLineEdit *m_txtUser;
    QLineEdit *m_txtPass;
    QPushButton *m_btnLogin;
    QPushButton *m_btnExit;
    QPushButton *m_btnToggleMode; // Fullscreen/Window toggle
    QLabel *m_lblMessage;
    
    // Internal State
    QPoint m_dragPos;
    bool m_isFullScreen;
};

#endif // MANHINHDANGNHAP_H
