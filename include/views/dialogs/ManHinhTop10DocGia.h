#ifndef MANHINHTOP10DOCGIA_H
#define MANHINHTOP10DOCGIA_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>

class ManHinhTop10DocGia : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhTop10DocGia(QWidget *parent = nullptr);

private:
    void thietLapGiaoDien();
    void apDungPhongCachHienDai();
    void taiDuLieuTop10();

    QTableWidget* bangKetQua;
    QPushButton* nutLamMoi;
    QPushButton* nutDong;
};

#endif // MANHINHTOP10DOCGIA_H
