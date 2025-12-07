#ifndef MANHINHTOP10DOCGIA_H
#define MANHINHTOP10DOCGIA_H

#include <QDialog>
#include <QTableView>
#include <QAbstractItemModel>
#include <QPushButton>
#include <QLabel>
#include <vector>
#include "models/DocGia.h"

struct DocGiaVaSoLuot {
    PTRDG docGia;
    int soLuotMuon;
    int soDangMuon;
};

class ManHinhTop10DocGia : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhTop10DocGia(QWidget *parent = nullptr);

private:
    void thietLapGiaoDien();
    void apDungPhongCachHienDai();
    void taiDuLieuTop10();

    QTableView* bangKetQua;
    QAbstractItemModel* modelKetQua;
    std::vector<DocGiaVaSoLuot> danhSachTop;
    QPushButton* nutLamMoi;
    QPushButton* nutDong;
};

#endif // MANHINHTOP10DOCGIA_H
