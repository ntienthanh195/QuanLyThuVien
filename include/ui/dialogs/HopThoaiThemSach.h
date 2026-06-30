#ifndef HOPTHOAITHEMSACH_H
#define HOPTHOAITHEMSACH_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QShortcut>

/**
 * @brief Dialog thêm đầu sách mới vào hệ thống
 * 
 * Chức năng:
 * - Nhập thông tin sách: ISBN, tên, tác giả, thể loại, năm XB, số trang
 * - Thêm nhiều bản sao cùng lúc
 * - Kiểm tra dữ liệu đầu vào
 * - Tích hợp backend QuanLySach
 */
class HopThoaiThemSach : public QDialog {
    Q_OBJECT

public:
    explicit HopThoaiThemSach(QWidget *cha = nullptr);
    ~HopThoaiThemSach();
    
protected:
    void keyPressEvent(QKeyEvent *e) override;

signals:
    void sachDaThem();

private slots:
    void xuLyNutLuu();
    void xuLyNutHuy();
    void kiemTraISBN();
    void kiemTraTatCaTruong();
    void xoaTatCaTruong();

private:
    void thieLapGiaoDien();
    void thieLapKetNoi();
    void thieLapKiemTra();
    void thieLapPhimTat();
    void thieLapThuTuTab();
    
    bool kiemTraCacTruong();
    void hienThiLoi(const QString &thongDiep);
    void hienThiThanhCong(const QString &thongDiep);
    void diemDayTheLoai();
    void datKieuTruong(QLineEdit *truong, bool hopLe);
    
    // Form fields - responsive
    // Form fields - responsive
    QLineEdit *oNhapISBN;
    QLineEdit *oNhapTenSach;
    QLineEdit *oNhapTacGia;
    QComboBox *oNhapTheLoai; // Editable ComboBox cho dropdown + nhập tay
    QSpinBox *oNhapNamXB;
    QSpinBox *oNhapSoTrang;
    QSpinBox *oNhapSoBanSao;
    QLineEdit *oNhapViTri;
    QDoubleSpinBox *oNhapGiaTien; // Using DoubleSpinBox for currency
    QLineEdit *oNhapNhaXuatBan; // Phase 2 Requirement
    
    // Buttons
    QPushButton *nutLuu;
    QPushButton *nutHuy;
    QPushButton *nutXoaForm;
    
    // Shortcuts
    QShortcut *phimTatLuu;
    QShortcut *phimTatHuy;
    
    // Status
    QLabel *nhanTrangThai;
};

#endif // HOPTHOAITHEMSACH_H
