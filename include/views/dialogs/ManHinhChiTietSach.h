#ifndef MANHINH_CHI_TIET_SACH_H
#define MANHINH_CHI_TIET_SACH_H

#include <QDialog>
#include <QTableView>
#include <QAbstractItemModel>
#include <vector>
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
    void khiNhanXoaBanSao();
    void khiNhanLamMoi();
    void khiNhanDong();
    void khiChonBanSao(const QModelIndex &index);

private:
    PTRDS sachDuocChon;
    QTableView *bangBanSao;
    QAbstractItemModel *modelBanSao;
    std::vector<PTRDMS> danhSachBanSaoVec;
    QPushButton *nutThanhLy;
    QPushButton *nutXoaBanSao;
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
