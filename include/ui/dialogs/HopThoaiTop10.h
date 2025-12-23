#ifndef HOPTHOAITOP10_H
#define HOPTHOAITOP10_H

#include <QDialog>
#include <QTableView>
#include <QAbstractItemModel>
#include <vector>
#include "models/DocGia.h"
#include <QPushButton>

class HopThoaiTop10 : public QDialog {
    Q_OBJECT

public:
    explicit HopThoaiTop10(QWidget *cha = nullptr);

private:
    void thieLapGiaoDien();
    void taiDuLieuTop10();
    
    QTableView *bangTop10;
    QAbstractItemModel *modelTop10;
    std::vector<TopSachDTO> danhSachTop;
    QPushButton *nutDong;
};

#endif // HOPTHOAITOP10_H
