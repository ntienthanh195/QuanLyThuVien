/**
 * @file HopThoaiCauHinhIn.h
 * @status ACTIVE
 * @brief Dialog cấu hình in ấn.
 * @note AI Chú ý: Đây là header chính thức.
 */
#ifndef HOPTHOAICAUHINHIN_H
#define HOPTHOAICAUHINHIN_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QLabel>
#include "utils/PrintHelper.h"

class HopThoaiCauHinhIn : public QDialog {
    Q_OBJECT
public:
    explicit HopThoaiCauHinhIn(const PrintHelper::PrintConfig& cfg, QWidget* parent = nullptr);
    ~HopThoaiCauHinhIn() {}

    PrintHelper::PrintConfig getConfig() const { return m_config; }

private slots:
    void chonLogo();
    void chonThuMucQR();
    void chonThuMucPDF();
    void updatePreview();
    void applyPreset(int index);

private:
    void setupUi();
    void loadFromConfig();

    PrintHelper::PrintConfig m_config;

    QLineEdit* leTenThuVien;
    QLineEdit* leTenVietTat;
    QCheckBox* cbInMau;
    QCheckBox* cbHienThiLogo;
    QCheckBox* cbSilentPrint;
    QLineEdit* leDuongDanLogo;
    QLineEdit* leQROutput;
    QLineEdit* lePDFOutput;
    
    // Barcode Layout Elements
    QComboBox* cbPresets;
    QSpinBox* sbLabelWidth;
    QSpinBox* sbLabelHeight;
    QSpinBox* sbLabelCols;
    QCheckBox* cbShowLibName;
    QCheckBox* cbShowBookTitle;
    QCheckBox* cbShowPrice;
    QCheckBox* cbShowBorder;
    QLabel* lblPreview;

    QPushButton* btnChonLogo;
    QPushButton* btnChonQR;
    QPushButton* btnChonPDF;
    QPushButton* btnLuu;
    QPushButton* btnHuy;
};

#endif // HOPTHOAI_CAUIHINH_IN_H
