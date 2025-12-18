#ifndef MANHINHTOP10_H
#define MANHINHTOP10_H

#include <QDialog>
#include <QTableView>
#include <QPushButton>
#include <QLabel>
#include <QDateEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>
#include "models/DauSach.h"
#include <vector>

// Struct để lưu thông tin sách và số lượt mượn
struct SachVaSoLuotMuon {
    PTRDS sach;
    int soLuotMuon;
    int soLuotMuonTruoc;  // Số lượt mượn kỳ trước (để so sánh)
    int thayDoi;  // Chênh lệch so với kỳ trước
    double tyLe; // tỷ lệ phần trăm
};

class ManHinhTop10 : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhTop10(PTRDS dsDauSach[], int soLuongDauSach, QWidget *parent = nullptr);
    ~ManHinhTop10();

private slots:
    void onFilterChanged();
    void onExportReport();

private:
    PTRDS* danhSachDauSach;
    int soLuong;
    
    // UI Components
    QTableView *bangTop10;
    QAbstractItemModel *modelTop10;
    std::vector<SachVaSoLuotMuon> danhSachTop;
    QLabel *nhanTieuDe;
    QLabel *nhanThongTin;
    QLabel *nhanSoSanh;  // Nhãn so sánh xu hướng
    QPushButton *nutDong;
    QPushButton *nutLamMoi;
    QPushButton *nutXuatBaoCao;
    
    // Filter Controls
    QComboBox *comboBoLoc;  // All time / Tháng này / Tháng trước / Tùy chỉnh
    QDateEdit *dateFrom;
    QDateEdit *dateTo;
    QCheckBox *checkSoSanh;  // Hiển thị so sánh với kỳ trước
    
    void thietLapGiaoDien();
    void apDungPhongCach();
    void capNhatBangTop10();
    
    // New advanced methods
    int demLuotMuonTheoKhoangThoiGian(PTRDS dauSach, QDate tuNgay, QDate denNgay);
    void hienThiSoSanhXuHuong();
};

#endif // MANHINHTOP10_H
