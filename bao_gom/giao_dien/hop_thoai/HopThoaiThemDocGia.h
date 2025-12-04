#ifndef HOPTHOAITHEMDOCGIA_H
#define HOPTHOAITHEMDOCGIA_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class HopThoaiThemDocGia : public QDialog {
    Q_OBJECT

public:
    explicit HopThoaiThemDocGia(QWidget *cha = nullptr);

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
    QPushButton *nutLuu;
    QPushButton *nutHuy;
    QPushButton *nutXoaForm;
    QLabel *nhanTrangThai;
    
    int maDuocTao;
    void capNhatPreviewHoTen();
};

#endif // HOPTHOAITHEMDOCGIA_H
