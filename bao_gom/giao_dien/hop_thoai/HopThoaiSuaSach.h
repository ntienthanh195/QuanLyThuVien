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
    DauSach *sachHienTai;
    
    QLineEdit *oNhapISBN;
    QLineEdit *oNhapTenSach;
    QLineEdit *oNhapTacGia;
    QComboBox *hopChonTheLoai;
    QSpinBox *soNamXuatBan;
    QSpinBox *soSoTrang;
    QDoubleSpinBox *soGiaTien; // Added Price field
    QPushButton *nutLuu;
    QPushButton *nutHuy;
    QLabel *nhanTrangThai;
};

#endif // HOPTHOAISUASACH_H
