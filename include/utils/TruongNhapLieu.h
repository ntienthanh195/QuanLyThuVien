#ifndef TRUONGNHAPLIEU_H
#define TRUONGNHAPLIEU_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QString>
#include <QValidator>
#include <QPushButton>
#include <QTimer>

/**
 * @file TruongNhapLieu.h
 * @brief Component trường nhập liệu với floating label và validation
 * 
 * Features:
 * - Floating label (label nhảy lên khi focus/có text)
 * - Success/Error states với icon
 * - Helper text và error message
 * - Custom validators
 * - Touch-friendly height (44px)
 */

enum TrangThaiTruong {
    TRANG_THAI_BINH_THUONG,  // Trạng thái mặc định
    TRANG_THAI_FOCUS,        // Đang focus
    TRANG_THAI_THANH_CONG,   // Validation thành công (✓)
    TRANG_THAI_LOI,          // Validation lỗi (✗)
    TRANG_THAI_DISABLED      // Disabled
};

/**
 * @class TruongNhapLieu
 * @brief Widget tùy chỉnh cho input field với label và validation
 */
class TruongNhapLieu : public QWidget {
    Q_OBJECT
    
public:
    explicit TruongNhapLieu(QWidget *parent = nullptr);
    explicit TruongNhapLieu(const QString &nhanPlaceholder, QWidget *parent = nullptr);
    ~TruongNhapLieu();
    
    // Getters
    QString layGiaTri() const;
    QLineEdit* layLineEdit();
    bool laTrong() const;
    bool hopLe() const;
    
    // Setters
    void datGiaTri(const QString &giaTri);
    void datNhan(const QString &nhan);
    void datPlaceholder(const QString &placeholder);
    void datTextGiupDo(const QString &text);
    void datTextLoi(const QString &text);
    void datValidator(QValidator *validator);
    void datReadOnly(bool readOnly);
    void datPasswordMode(bool isPassword);
    void datMaxLength(int length);
    
    // State management
    void datTrangThai(TrangThaiTruong trangThai);
    void hienThiThanhCong();
    void hienThiLoi(const QString &thongDiepLoi);
    void xoaLoi();
    void datBatBuoc(bool batBuoc);
    
    // Focus
    void focus();
    void clearFocus();
    
signals:
    void textThayDoi(const QString &text);
    void enterDuocNhan();
    void giaTriHopLe(bool hopLe);
    
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    
private:
    void khoiTaoGiaoDien();
    void khoiTaoConnections();
    void capNhatStyle();
    void capNhatViTriNhan();
    QString layStyleInput();
    QString layStyleNhan();
    
private:
    QVBoxLayout *layoutChinh;
    QLabel *labelNhan;
    QLineEdit *inputField;
    QLabel *labelThongBao;  // Helper text hoặc error message
    QLabel *iconTrangThai;  // ✓ hoặc ✗
    
    QString nhanGoc;
    QString textGiupDo;
    TrangThaiTruong trangThaiHienTai;
    bool laBatBuoc;
    bool daValidate;
    
    // Colors
    static const QString MAU_PRIMARY;
    static const QString MAU_SUCCESS;
    static const QString MAU_ERROR;
    static const QString MAU_BORDER_BINH_THUONG;
    static const QString MAU_BORDER_FOCUS;
    static const QString MAU_TEXT;
    static const QString MAU_PLACEHOLDER;
    static const QString MAU_HELPER;
};

/**
 * @class TruongTimKiem
 * @brief Trường tìm kiếm với icon search và clear button
 */
class TruongTimKiem : public QWidget {
    Q_OBJECT
    
public:
    explicit TruongTimKiem(QWidget *parent = nullptr);
    ~TruongTimKiem();
    
    QString layGiaTri() const;
    void datGiaTri(const QString &giaTri);
    void datPlaceholder(const QString &placeholder);
    void xoa();
    void focus();
    
signals:
    void textThayDoi(const QString &text);
    void timKiem(const QString &tuKhoa);
    void daXoa();
    
private:
    void khoiTaoGiaoDien();
    void khoiTaoConnections();
    void capNhatNutXoa();
    
private:
    QHBoxLayout *layoutChinh;
    QLabel *iconTimKiem;
    QLineEdit *inputField;
    QPushButton *nutXoa;
    QTimer *timerDebounce;
};

#endif // TRUONGNHAPLIEU_H
