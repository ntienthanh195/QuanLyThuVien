#ifndef MANHINHTHONGKE_H
#define MANHINHTHONGKE_H

#include <QDialog>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include "models/DauSach.h"
#include "models/DocGia.h"

class ManHinhThongKe : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhThongKe(PTRDS dsDauSach[], int soLuongDauSach, PTRDG rootDocGia, QWidget *parent = nullptr);

private:
    void thietLapGiaoDien();
    void apDungPhongCach();
    void capNhatThongKe();
    
    // Thống kê sách
    QLabel* nhanTongDauSach;
    QLabel* nhanTongBanSao;
    QLabel* nhanBanSaoCoTheMuon;
    QLabel* nhanBanSaoDangMuon;
    QLabel* nhanBanSaoThanhLy;
    QLabel* nhanTyLeChoMuon;
    QLabel* nhanSoTheLoai;
    
    // Thống kê độc giả
    QLabel* nhanTongDocGia;
    QLabel* nhanDocGiaHoatDong;
    QLabel* nhanDocGiaBiKhoa;
    QLabel* nhanDocGiaDangMuon;
    QLabel* nhanTongSachDangMuon;
    QLabel* nhanDocGiaCoSachQuaHan;
    
    QPushButton* nutDong;
    
    // Data
    PTRDS* danhSachDauSach;
    int soLuong;
    PTRDG cayDocGia;
};

#endif // MANHINHTHONGKE_H
