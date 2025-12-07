#ifndef MANHINHTOP10SACH_H
#define MANHINHTOP10SACH_H

#include <QDialog>
#include <QTableView>
#include <QAbstractItemModel>
#include <QPushButton>
#include <vector>
#include "models/DauSach.h"

struct TopSachDTO;

class ManHinhTop10Sach : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhTop10Sach(QWidget *parent = nullptr);

private:
    void thietLapGiaoDien();
    void apDungPhongCachHienDai();
    void taiDuLieuTop10();

    QTableView* bangKetQua;
    QAbstractItemModel* modelKetQua;
    std::vector<TopSachDTO> danhSachTop;
    QPushButton* nutLamMoi;
    QPushButton* nutDong;
};

#endif // MANHINHTOP10SACH_H
