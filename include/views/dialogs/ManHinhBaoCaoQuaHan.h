#ifndef MANHINHBAOCAOQUAHAN_H
#define MANHINHBAOCAOQUAHAN_H

#include <QDialog>
#include <QTableView>
#include <QAbstractItemModel>
#include <QPushButton>
#include <vector>
#include "models/DocGia.h"

class ManHinhBaoCaoQuaHan : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhBaoCaoQuaHan(QWidget *parent = nullptr);

private:
    void thietLapGiaoDien();
    void apDungPhongCachHienDai();
    void taiDuLieuQuaHan();

    QTableView* bangKetQua;
    QAbstractItemModel* modelKetQua;
    std::vector<DocGiaQuaHanDTO> danhSachQuaHan;
    QPushButton* nutLamMoi;
    QPushButton* nutDong;
};

#endif // MANHINHBAOCAOQUAHAN_H
