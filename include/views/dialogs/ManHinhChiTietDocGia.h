#ifndef MANHINH_CHI_TIET_DOC_GIA_H
#define MANHINH_CHI_TIET_DOC_GIA_H

#include <QDialog>
#include <QDate> // Fix QDate error
#include <QTableView>
#include <QAbstractItemModel>
#include <vector>
#include <QPushButton>
#include <QLabel>
#include "models/DocGia.h"

class ManHinhChiTietDocGia : public QDialog {
    Q_OBJECT

public:
    // Helper struct for table items (Moved to public for Model access)
    struct ThongTinMuon {
        QString maSach;
        QDate ngayMuon;
        QDate hanTra;
        QString trangThai;
        int soNgayQuaHan;
        MUONTRA nodeMuonTra;
    };

    explicit ManHinhChiTietDocGia(PTRDG reader, QWidget *parent = nullptr);
    ~ManHinhChiTietDocGia();

signals:
    void docGiaDaCapNhat();

private slots:
    void khiNhanSua(); // Sửa độc giả
    void khiNhanThuHoi(); // Thu hồi sách (Trả sách)
    void khiNhanBaoMat(); // Báo mất sách
    void khiNhanLamMoi();
    void khiNhanDong();
    void khiNhanInThe();
    void khiChonSachMuon(const QModelIndex &index);

private:
    PTRDG docGiaDuocChon;
    QTableView *bangSachMuon;
    QAbstractItemModel *modelSachMuon;
    
    // Helper struct moved to public
    std::vector<ThongTinMuon> danhSachMuonVec;

    QPushButton *nutThanhLy;
    QPushButton *nutSua; // New button
    QPushButton *nutThuHoi;
    QPushButton *nutBaoMat;
    QPushButton *nutXoaBanSao;
    QPushButton *nutLamMoi;
    QPushButton *nutInThe;
    QPushButton *nutDong;
    
    QLabel *nhanTongQuat;
    QLabel *nhanTrangThai;
    
    int dongSachDuocChon;
    
    void thietLapGiaoDien();
    void thietLapKetNoi();
    void taiThongTinDocGia();
    void taiDanhSachSachMuon();
    void capNhatTongQuat();
    void hienThiThongBao(const QString &message, bool isError = false);
};

#endif // MANHINH_CHI_TIET_DOC_GIA_H
