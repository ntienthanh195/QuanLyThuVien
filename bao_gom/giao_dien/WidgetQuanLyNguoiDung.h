/**
 * @file WidgetQuanLyNguoiDung.h
 * @path bao_gom/giao_dien/WidgetQuanLyNguoiDung.h
 * @description Header của Widget quản lý người dùng (V2).
 * @project Library Management System (QuanLyThuVien)
 */
#ifndef WIDGETQUANLYNGUOIDUNG_H

#define WIDGETQUANLYNGUOIDUNG_H

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QStandardItemModel>

class WidgetQuanLyNguoiDung : public QWidget {
    Q_OBJECT

public:
    explicit WidgetQuanLyNguoiDung(QWidget *parent = nullptr);
    void taiLaiDanhSach(); // Refresh list

private slots:
    void xuLyThem();
    void xuLySua();
    void xuLyXoa();
    void xuLyDatLaiMatKhau();
    void xuLyTimKiem(const QString &text);

private:
    void thietLapGiaoDien();
    void taiDuLieu();

    QTableView *bangNguoiDung;
    QStandardItemModel *moHinh;
    QLineEdit *oTimKiem;
    
    QPushButton *nutThem;
    QPushButton *nutSua;
    QPushButton *nutXoa;
    QPushButton *nutDatLaiMatKhau;
};

#endif // WIDGETQUANLYNGUOIDUNG_H
