#ifndef MANHINH_THEM_BAN_SAO_H
#define MANHINH_THEM_BAN_SAO_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include "models/DauSach.h"

class ManHinhThemBanSao : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhThemBanSao(const QString& isbn, QWidget *parent = nullptr);

signals:
    void banSaoDaThem();

private slots:
    void khiNhanLuu();
    void kiemTraDuLieu();

private:
    void thietLapGiaoDien();
    void apDungPhongCachHienDai();

    QString maISBN;
    DauSach* sachHienTai;

    QLineEdit* nhapISBN;
    QLineEdit* nhapTenSach;
    QSpinBox* nhapSoLuong;
    QComboBox* chonViTri;
    QPushButton* nutLuu;
    QPushButton* nutHuy;
};

#endif // MANHINH_THEM_BAN_SAO_H
