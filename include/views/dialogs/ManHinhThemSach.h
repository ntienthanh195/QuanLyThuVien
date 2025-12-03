#ifndef MANHINH_THEM_SACH_H
#define MANHINH_THEM_SACH_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QShortcut>

class ManHinhThemSach : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhThemSach(QWidget *parent = nullptr);
    ~ManHinhThemSach();

signals:
    void sachDaThem();

private slots:
    void khiNhanLuu();
    void khiNhanHuy();
    void kiemTraISBN();
    void kiemTraTatCaTruong();
    void clearAllFields();

private:
    // Form fields
    QLineEdit *nhapISBN;
    QLineEdit *nhapTenSach;
    QLineEdit *nhapTacGia;
    QComboBox *chonTheLoai;
    QSpinBox *nhapNamXuatBan;
    QSpinBox *nhapSoTrang;
    QSpinBox *nhapSoBanSao;
    QLineEdit *nhapViTri;
    
    // Buttons
    QPushButton *nutLuu;
    QPushButton *nutHuy;
    QPushButton *nutXoaForm;
    
    // Keyboard shortcuts
    QShortcut *shortcutSave;
    QShortcut *shortcutCancel;
    
    // Status label
    QLabel *nhanTrangThai;
    
    void thietLapGiaoDien();
    void thietLapKetNoi();
    void thietLapKiemTra();
    void setupKeyboardShortcuts();
    void setupTabOrder();
    bool kiemTraCacTruong();
    void hienThiLoi(const QString &message);
    void hienThiThanhCong(const QString &message);
    
    // Helper methods
    void diemDayTheLoai();
    void datPhongCachTruong(QLineEdit *field, bool hopLe);
};

#endif // MANHINH_THEM_SACH_H
