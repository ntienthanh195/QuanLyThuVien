#ifndef FOCUSGUARD_H
#define FOCUSGUARD_H

#include <QObject>
#include <QLineEdit>
#include <QEvent>
#include <QTimer>
#include <QDebug>

/**
 * @brief FocusGuard - Lớp tiện ích giúp giữ focus vào một ô nhập liệu.
 * Dùng cho các màn hình sử dụng máy quét mã vạch (Scanner), tránh mất focus khi user click nhầm ra ngoài.
 */
class FocusGuard : public QObject {
    Q_OBJECT
public:
    explicit FocusGuard(QObject *parent = nullptr);
    
    /**
     * @brief Thiết lập widget cần bảo vệ focus
     * @param inputWidget: Ô nhập liệu (QLineEdit) cần giữ focus
     */
    void setTarget(QLineEdit* inputWidget);
    
    /**
     * @brief Tạm dừng/Tiếp tục bảo vệ
     */
    void setEnabled(bool enabled);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QLineEdit* m_target;
    bool m_enabled;
    QTimer* m_refocusTimer;
    
private slots:
    void performRefocus();
};

#endif // FOCUSGUARD_H
