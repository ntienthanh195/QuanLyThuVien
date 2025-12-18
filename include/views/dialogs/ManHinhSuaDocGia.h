#ifndef MANHINH_SUA_DOC_GIA_H
#define MANHINH_SUA_DOC_GIA_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include "models/DocGia.h"

class ManHinhSuaDocGia : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhSuaDocGia(const QString& readerCode, QWidget *parent = nullptr);

signals:
    void docGiaDaCapNhat();

private slots:
    void khiNhanLuu();
    void kiemTraDuLieu();

private:
    void thietLapGiaoDien();
    void apDungPhongCachHienDai();
    void taiDuLieuDocGia();

    QString maDocGia;
    PTRDG docGiaHienTai;

    QLineEdit* nhapMaThe;
    QLineEdit* nhapHo;
    QLineEdit* nhapTen;
    QComboBox* chonGioiTinh;
    QComboBox* chonTrangThai;
    QLabel* nhanSoSachDangMuon;
    QPushButton* nutLuu;
    QPushButton* nutHuy;
};

#endif // MANHINH_SUA_DOC_GIA_H
