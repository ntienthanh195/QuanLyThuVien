#ifndef MANHINH_THEM_DOC_GIA_H
#define MANHINH_THEM_DOC_GIA_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QShortcut>

class ManHinhThemDocGia : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhThemDocGia(QWidget *parent = nullptr);

signals:
    void docGiaDaThem();

private slots:
    void khiNhanLuu();
    void kiemTraDuLieu();
    void taoMaDocGia();
    void clearAllFields();

private:
    void thietLapGiaoDien();
    void apDungPhongCachHienDai();
    void setupKeyboardShortcuts();
    void setupTabOrder();

    QLabel* nhanMaThe;
    QLineEdit* nhapHo;
    QLineEdit* nhapTen;
    QComboBox* chonGioiTinh;
    QPushButton* nutLuu;
    QPushButton* nutHuy;
    QPushButton* nutXoaForm;
    
    QShortcut* shortcutSave;
    QShortcut* shortcutCancel;
    
    QString maDuocTao;
};

#endif // MANHINH_THEM_DOC_GIA_H
