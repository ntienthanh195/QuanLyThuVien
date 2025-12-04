#ifndef HOPTHOAITOP10_H
#define HOPTHOAITOP10_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>

class HopThoaiTop10 : public QDialog {
    Q_OBJECT

public:
    explicit HopThoaiTop10(QWidget *cha = nullptr);

private:
    void thieLapGiaoDien();
    void taiDuLieuTop10();
    
    QTableWidget *bangTop10;
    QPushButton *nutDong;
};

#endif // HOPTHOAITOP10_H
