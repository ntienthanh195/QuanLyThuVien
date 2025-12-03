#ifndef MANHINH_CHI_TIET_SACH_H
#define MANHINH_CHI_TIET_SACH_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include "models/DauSach.h"

class ManHinhChiTietSach : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhChiTietSach(PTRDS book, QWidget *parent = nullptr);
    ~ManHinhChiTietSach();

signals:
    void sachDaCapNhat();

private slots:
    void khiNhanThanhLy();
    void khiNhanLamMoi();
    void khiNhanDong();
    void khiChonBanSao(int row, int column);

private:
    PTRDS sachDuocChon;
    QTableWidget *bangBanSao;
    QPushButton *nutThanhLy;
    QPushButton *nutLamMoi;
    QPushButton *nutDong;
    QLabel *nhanTongQuat;
    QLabel *nhanTrangThai;
    
    int dongBanSaoDuocChon;
    std::string maBanSaoDuocChon;
    
    void thietLapGiaoDien();
    void thietLapKetNoi();
    void taiThongTinSach();
    void taiDanhSachBanSao();
    void capNhatTongQuat();
    void hienThiThongBao(const QString &message, bool isError = false);
    QString layBieuTuongTrangThai(TrangThaiSach status);
    QString layVanBanTrangThai(TrangThaiSach status);
    QString layMauTrangThai(TrangThaiSach status);
};

#endif // MANHINH_CHI_TIET_SACH_H
