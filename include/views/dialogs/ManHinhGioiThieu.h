#ifndef MANHINH_GIOI_THIEU_H
#define MANHINH_GIOI_THIEU_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>

class ManHinhGioiThieu : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhGioiThieu(QWidget *parent = nullptr);
    ~ManHinhGioiThieu();

private:
    void setupUI();
};

#endif // MANHINH_GIOI_THIEU_H
