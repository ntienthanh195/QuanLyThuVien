#ifndef MANHINHTOP10SACH_H
#define MANHINHTOP10SACH_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>

class ManHinhTop10Sach : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhTop10Sach(QWidget *parent = nullptr);

private:
    void thietLapGiaoDien();
    void apDungPhongCachHienDai();
    void taiDuLieuTop10();

    QTableWidget* bangKetQua;
    QPushButton* nutLamMoi;
    QPushButton* nutDong;
};

#endif // MANHINHTOP10SACH_H
