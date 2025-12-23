#ifndef HOPTHOAITHEMBANSAO_H
#define HOPTHOAITHEMBANSAO_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include "models/DauSach.h"

class HopThoaiThemBanSao : public QDialog {
    Q_OBJECT

public:
    explicit HopThoaiThemBanSao(const QString& isbn, QWidget *cha = nullptr);

signals:
    void banSaoDaThem();

private slots:
    void xuLyNutLuu();

private:
    void thieLapGiaoDien();
    
    QString maISBN;
    DauSach *sachHienTai;
    
    QLineEdit *oNhapISBN;
    QLineEdit *oNhapTenSach;
    QSpinBox *soSoLuong;
    QLineEdit *oNhapViTri;
    QPushButton *nutLuu;
    QPushButton *nutHuy;
    QLabel *nhanTrangThai;
};

#endif // HOPTHOAITHEMBANSAO_H
