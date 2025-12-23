#ifndef HOPTHOAITHONGKE_H
#define HOPTHOAITHONGKE_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>

class HopThoaiThongKe : public QDialog {
    Q_OBJECT

public:
    explicit HopThoaiThongKe(QWidget *cha = nullptr);

private:
    void thieLapGiaoDien();
    void taiDuLieuThongKe();
    QChartView* taoBieuDoSach();
    QChartView* taoBieuDoDocGia();
    
    // Labels hiển thị số liệu
    QLabel *nhanTongDauSach;
    QLabel *nhanTongBanSao;
    QLabel *nhanCoTheMuon;
    QLabel *nhanDangMuon;
    QLabel *nhanThanhLy;
    QLabel *nhanTyLe;
    QLabel *nhanTheLoai;
    
    QLabel *nhanTongDocGia;
    QLabel *nhanHoatDong;
    QLabel *nhanBiKhoa;
    QLabel *nhanDangMuonSach;
    QLabel *nhanTongSachMuon;
    QLabel *nhanQuaHan;
    
    QPushButton *nutDong;
};

#endif // HOPTHOAITHONGKE_H
