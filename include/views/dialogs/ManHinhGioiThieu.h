/**
 * @file ManHinhGioiThieu.h
 * @path include/views/dialogs/ManHinhGioiThieu.h
 * @description Header của hộp thoại Giới thiệu ứng dụng (About).
 * @project Library Management System (QuanLyThuVien)
 */
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
