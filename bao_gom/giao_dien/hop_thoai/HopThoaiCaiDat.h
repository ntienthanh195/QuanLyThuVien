#ifndef HOPTHOAICAIDAT_H
#define HOPTHOAICAIDAT_H

#include <QDialog>
#include <QTabWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPrinter>
#include <QPrinterInfo>

class HopThoaiCaiDat : public QDialog {
    Q_OBJECT

public:
    explicit HopThoaiCaiDat(QWidget *parent = nullptr);

private slots:
    void luuCauHinh();
    void macDinh();
    void capNhatNgonNgu();

private:
    void thietLapGiaoDien();
    void loadData();

    // UI Elements
    QTabWidget *tabs; // Added for dynamic title update
    
    QLabel *lblTenThuVien;
    QLabel *lblDiaChi;
    QLineEdit *oTenThuVien;
    QLineEdit *oDiaChi;
    
    QLabel *lblTienPhat;
    QLabel *lblNgayMuon;
    QLabel *lblLanGiaHan;
    QLabel *lblSachMuon;
    QSpinBox *oTienPhatMoiNgay;
    QSpinBox *oSoNgayMuonToiDa;
    QSpinBox *oSoLanGiaHanToiDa; 
    QSpinBox *oSoSachMuonToiDa;
    
    // New Settings (Phase 8)
    QLabel *lblThoiHanThe;
    QLabel *lblPhiMatSach;
    QSpinBox *oThoiHanThe; // Thoi han the (thang)
    QSpinBox *oPhiMatSach; // Phi mat sach (vnd)
    
    QPushButton *nutLuu;
    QPushButton *nutHuy;
    QPushButton *nutMacDinh;
};

#endif // HOPTHOAICAIDAT_H
