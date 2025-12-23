#ifndef HOPTHOAISUASACH_H
#define HOPTHOAISUASACH_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include "models/DauSach.h"

class HopThoaiSuaSach : public QDialog {
    Q_OBJECT

public:
    explicit HopThoaiSuaSach(const QString& isbn, QWidget *cha = nullptr);

signals:
    void sachDaSua();

private slots:
    void xuLyNutLuu();
    void kiemTraDuLieu();

private:
    void thieLapGiaoDien();
    void taiDuLieuSach();
    
    QString isbnGoc;
    PTRDS sachHienTai;
    
    QLineEdit *oNhapISBN;
    QLineEdit *oNhapTenSach;
    QLineEdit *oNhapTacGia;
    QLineEdit *oNhapTheLoai; // Changed from QComboBox
    QSpinBox *soNamXuatBan;
    QSpinBox *soSoTrang;
    QDoubleSpinBox *oNhapGiaTien; // Added Price field
    QLineEdit *oNhapViTri; // Added Location field
    QLineEdit *oNhapNhaXuatBan; // Added Publisher field
    QPushButton *nutLuu;
    QPushButton *nutHuy;
    QLabel *nhanTrangThai;
};

#endif // HOPTHOAISUASACH_H
