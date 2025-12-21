/**
 * @file ManHinhKiemKe.h
 * @path bao_gom/giao_dien/hop_thoai/ManHinhKiemKe.h
 * @description Dialog quản lý kiểm kê kho sách bằng mã vạch.
 */
#ifndef MANHINHKIEMKE_H
#define MANHINHKIEMKE_H

#include <QDialog>
#include <QLineEdit>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSet>
#include <QStringList>

class ManHinhKiemKe : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhKiemKe(QWidget *parent = nullptr);
    ~ManHinhKiemKe();

private slots:
    void xuLyQuetMa();
    void xuLyKetThuc();
    void xuLyXuatBaoCao();
    void capNhatThongKe();
    void keyPressEvent(QKeyEvent *event) override;

private:
    void thieLapGiaoDien();
    void taiDuLieuHeThong(); // Get all ma_sach from DB

    // UI Elements
    QLineEdit *oNhapMaVach;
    QTableWidget *bangKetQua;
    
    // Statistics Cards (Matching/Missing)
    QLabel *nhanTongHeThong;
    QLabel *nhanDaQuet;
    QLabel *nhanChuaQuet;
    QLabel *nhanSaiViTri;
    QLabel *nhanSaiTrangThai;

    // Overview Labels (General stats)
    QLabel *nhanTongDauSach;
    QLabel *nhanTongBanSao;
    QLabel *nhanDangChoMuon;
    QLabel *nhanDangTrongKho;

    QPushButton *nutKetThuc;
    QPushButton *nutXuatBaoCao;

    // Data handling
    QSet<QString> setMaHeThong;     // All books in DB (not counting lost)
    QSet<QString> setMaDaQuet;      // Successfully scanned
    QSet<QString> setMaDangMuon;    // Validly borrowed (not in stock)
    QStringList listMaLa;           // Scanned but not in DB
    QStringList listSaiTrangThai;   // Scanned but DB says 'Borrowed'
};

#endif // MANHINHKIEMKE_H
