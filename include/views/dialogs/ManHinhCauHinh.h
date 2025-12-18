#ifndef MANHINHCAUHINH_H
#define MANHINHCAUHINH_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>

class ManHinhCauHinh : public QDialog {
    Q_OBJECT
public:
    enum class DisplayMode {
        Full,           // Show everything (Default)
        PrintConfigOnly // Hide Borrow Rules (Loan Rules)
    };

    explicit ManHinhCauHinh(QWidget *parent = nullptr, DisplayMode mode = DisplayMode::Full);
    
private slots:
    void luuCauHinh();
    void macDinh();
    void xuLyChuanHoaMaSach(); // Data normalization
    void chonLogo();
    void chonThuMucQR();
    void chonThuMucPDF();

private:
    DisplayMode currentMode;

    QComboBox *comboPrinterReceipt;
    QComboBox *comboPrinterReport;
    QCheckBox *checkSilentPrint;
    QCheckBox *checkColor;
    
    // New Config Fields
    QLineEdit *txtTenThuVien;
    QLineEdit *txtTenVietTat;
    QCheckBox *checkHienThiLogo;
    QLineEdit *txtLogoPath;
    QLineEdit *txtQrPath;
    QLineEdit *txtPdfPath;
    
    // Feature: Loan Rules
    QSpinBox *spinHanMuon;
    QSpinBox *spinTienPhat;
    QSpinBox *spinSoLanGiaHan;
    
    void thietLapGiaoDien();
    void taiCauHinh();
};

#endif // MANHINHCAUHINH_H
