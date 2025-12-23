#ifndef HOPTHOAISUADOCGIA_H
#define HOPTHOAISUADOCGIA_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QDateEdit>
#include "models/DocGia.h"

class HopThoaiSuaDocGia : public QDialog {
    Q_OBJECT

public:
    explicit HopThoaiSuaDocGia(int maThe, QWidget *cha = nullptr);

signals:
    void docGiaDaSua();

private slots:
    void xuLyNutLuu();
    void kiemTraDuLieu();

private:
    void thieLapGiaoDien();
    void taiDuLieuDocGia();
    
    int maDocGia;
    PTRDG docGiaHienTai;  // Node trong AVL tree
    
    QLineEdit *oNhapMaThe;
    QLineEdit *oNhapHo;
    QLineEdit *oNhapTen;
    QComboBox *hopChonGioiTinh;

    // Phase 2 Fields
    QLineEdit *oNhapSDT;
    QLineEdit *oNhapLop;
    QLineEdit *oNhapEmail;
    QDateEdit *oNhapNgaySinh;
    QComboBox *hopChonLoaiDocGia;

    QComboBox *hopChonTrangThai;
    // Feature 5 Dates
    class QDateEdit *inputNgayLap;
    class QDateEdit *inputNgayHetHan;
    QPushButton *nutLuu;
    QPushButton *nutHuy;
    QLabel *nhanSoSachDangMuon;
    QLabel *nhanTrangThai;
    QLabel *nhanPreviewHoTen;

    void capNhatPreviewHoTen();
};

#endif // HOPTHOAISUADOCGIA_H
