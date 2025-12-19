#ifndef HOPTHOAITHEMDOCGIA_H
#define HOPTHOAITHEMDOCGIA_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QDateEdit>

class HopThoaiThemDocGia : public QDialog {
    Q_OBJECT

public:
    explicit HopThoaiThemDocGia(QWidget *cha = nullptr);
    
protected:
    void keyPressEvent(QKeyEvent *e) override;

signals:
    void docGiaDaThem();

private slots:
    void xuLyNutLuu();
    void kiemTraDuLieu();
    void taoMaDocGia();
    void xoaTatCaTruong();

private:
    void thieLapGiaoDien();
    
    QLabel *nhanMaThe;
    QLineEdit *oNhapHo;
    QLineEdit *oNhapTen;
    QLabel *nhanPreviewHoTen;

    QComboBox *hopChonGioiTinh;
    // Phase 2 Fields
    QLineEdit *oNhapSDT;
    QLineEdit *oNhapLop;
    QLineEdit *oNhapEmail;
    QDateEdit *oNhapNgaySinh;
    QComboBox *hopChonLoaiDocGia;

    // Feature 5 Dates
    class QDateEdit *inputNgayLap;
    class QDateEdit *inputNgayHetHan;
    QPushButton *nutLuu;
    QPushButton *nutHuy;
    QPushButton *nutXoaForm;
    QLabel *nhanTrangThai;
    
    int maDuocTao;
    void capNhatPreviewHoTen();
};

#endif // HOPTHOAITHEMDOCGIA_H
