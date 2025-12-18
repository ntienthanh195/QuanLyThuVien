/**
 * @file HopThoaiCauHinhIn.h
 * @status DUPLICATE / MISPLACED
 * @brief File header bị đặt sai vị trí trong thư mục nguon/.
 * @note AI Chú ý: KHÔNG SỬA FILE NÀY. Dùng bao_gom/giao_dien/hop_thoai/HopThoaiCauHinhIn.h
 */
#ifndef HOPTHOAICAUHINHIN_H
#define HOPTHOAICAUHINHIN_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
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

private:
    void setupUi();
    void loadFromConfig();

    PrintHelper::PrintConfig m_config;

    QLineEdit* leTenThuVien;
    QLineEdit* leTenVietTat;
    QCheckBox* cbInMau;
    QCheckBox* cbHienThiLogo;
    QLineEdit* leDuongDanLogo;
    QLineEdit* leQROutput;
    QLineEdit* lePDFOutput;
    QPushButton* btnChonLogo;
    QPushButton* btnChonQR;
    QPushButton* btnChonPDF;
    QPushButton* btnLuu;
    QPushButton* btnHuy;
};

#endif // HOPTHOAI_CAUIHINH_IN_H
