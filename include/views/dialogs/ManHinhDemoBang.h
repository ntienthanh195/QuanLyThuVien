#ifndef MANHINHDEMBANG_H
#define MANHINHDEMBANG_H

#include <QDialog>
#include <QTableView>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

/**
 * @file ManHinhDemoBang.h
 * @brief Demo dialog để showcase table styling với zebra stripes, hover, và selection
 */
class ManHinhDemoBang : public QDialog {
    Q_OBJECT
    
public:
    explicit ManHinhDemoBang(QWidget *parent = nullptr);
    ~ManHinhDemoBang() = default;
    
private:
    QTableView *bangChinh;
    QStandardItemModel *model;
    QComboBox *comboKieuBang;
    QLabel *labelThongTin;
    
    void taoCacWidget();
    void taoLayout();
    void taoKetNoi();
    void taiDuLieuMau();
    
private slots:
    void thayDoiKieuBang(int index);
    void kichDuplicate();
    void kichXoa();
};

#endif // MANHINHDEMBANG_H
