#ifndef MANHINHQUAHAN_H
#define MANHINHQUAHAN_H

#include <QDialog>
#include <QTableView>
#include <QAbstractItemModel>
#include <vector>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QPrinter>
#include <QPrintDialog>
#include "models/DocGia.h"
#include "models/DauSach.h"

struct DocGiaQuaHan {
    PTRDG docGia;
    int soSachQuaHan;
    int tongSoNgayQuaHan;
    int maxNgayQuaHan;
    int phiPhat;  // Tổng tiền phạt (5000đ/ngày)
};

class ManHinhQuaHan : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhQuaHan(PTRDG rootDocGia, PTRDS dsDauSach[], int soLuongDauSach, QWidget *parent = nullptr);
    ~ManHinhQuaHan();

private slots:
    void onViewReaderDetail();
    void onExportReport();

private:
    PTRDG cayDocGia;
    PTRDS* danhSachDauSach;
    int soLuong;
    
    // UI Components
    QTableView *bangQuaHan;
    QAbstractItemModel *modelQuaHan;
    std::vector<DocGiaQuaHan> danhSachQuaHan;
    QLabel *nhanTieuDe;
    QLabel *nhanThongTin;
    QLabel *nhanDoanhThu;  // Revenue statistics panel
    QLabel *nhanHoatDong;  // Active readers panel
    QPushButton *nutDong;
    QPushButton *nutLamMoi;
    QPushButton *nutXemChiTiet;
    QPushButton *nutXuatBaoCao;
    
    void thietLapGiaoDien();
    void apDungPhongCach();
    void capNhatBangQuaHan();
    void duyetInOrder(PTRDG root, std::vector<DocGiaQuaHan>& ds);
    
    // New advanced methods
    int demDocGiaHoatDong(int soNgayGanDay);  // Count active readers
    void capNhatThongKeDoanhThu(const std::vector<DocGiaQuaHan>& ds);
    void hienThiChiTietDocGia(PTRDG docGia);
};

#endif // MANHINHQUAHAN_H
