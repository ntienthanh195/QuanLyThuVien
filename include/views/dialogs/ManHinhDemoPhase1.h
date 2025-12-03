#ifndef MANHINHDEMO_PHASE1_H
#define MANHINHDEMO_PHASE1_H

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include "utils/KieuNutBam.h"
#include "utils/TruongNhapLieu.h"

/**
 * @brief Dialog demo cho Phase 1: Button System & Input Fields
 * Hiển thị tất cả các kiểu nút và trường nhập liệu mới
 */
class ManHinhDemoPhase1 : public QDialog {
    Q_OBJECT
    
public:
    explicit ManHinhDemoPhase1(QWidget *parent = nullptr);
    ~ManHinhDemoPhase1();
    
private:
    void khoiTaoGiaoDien();
    void taoNhomNutBam();
    void taoNhomTruongNhapLieu();
    void khoiTaoConnections();
    
private slots:
    void xuLyNutBamDemo(const QString &ten);
    void xuLyValidation();
    
private:
    QVBoxLayout *layoutChinh;
    
    // Input fields
    TruongNhapLieu *truongHoTen;
    TruongNhapLieu *truongEmail;
    TruongNhapLieu *truongMatKhau;
    TruongNhapLieu *truongSDT;
    TruongTimKiem *truongTimKiem;
    
    // Buttons
    QPushButton *nutLuuChinh;
    QPushButton *nutHuyChinh;
    QPushButton *nutXoaNguyHiem;
    QPushButton *nutHoanTatThanhCong;
};

#endif // MANHINHDEMO_PHASE1_H
