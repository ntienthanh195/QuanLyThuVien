#ifndef MANHINH_SUA_SACH_H
#define MANHINH_SUA_SACH_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include "models/DauSach.h"

class ManHinhSuaSach : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhSuaSach(const QString& isbn, QWidget *parent = nullptr);

signals:
    void sachDaCapNhat();

private slots:
    void khiNhanLuu();
    void kiemTraDuLieu();

private:
    void thietLapGiaoDien();
    void apDungPhongCachHienDai();
    void taiDuLieuSach();

    QString isbnGoc;
    DauSach* sachHienTai;

    QLineEdit* nhapISBN;
    QLineEdit* nhapTenSach;
    QLineEdit* nhapTacGia;
    QComboBox* chonTheLoai;
    QSpinBox* nhapNamXuatBan;
    QSpinBox* nhapSoTrang;
    QPushButton* nutLuu;
    QPushButton* nutHuy;
};

#endif // MANHINH_SUA_SACH_H
