#ifndef MANHINHBAOCAOQUAHAN_H
#define MANHINHBAOCAOQUAHAN_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>

class ManHinhBaoCaoQuaHan : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhBaoCaoQuaHan(QWidget *parent = nullptr);

private:
    void thietLapGiaoDien();
    void apDungPhongCachHienDai();
    void taiDuLieuQuaHan();

    QTableWidget* bangKetQua;
    QPushButton* nutLamMoi;
    QPushButton* nutDong;
};

#endif // MANHINHBAOCAOQUAHAN_H
