
#ifndef HOPTHOAITHANHTOAN_H
#define HOPTHOAITHANHTOAN_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include "models/DocGia.h"

class HopThoaiThanhToan : public QDialog {
    Q_OBJECT
    
public:
    explicit HopThoaiThanhToan(PTRDG docGia, const QString& lyDo, long long soTien, bool isRefund = false, QWidget *parent = nullptr);
    
    // Helper static function for convenience
    static bool hienThiThanhToan(PTRDG docGia, const QString& lyDo, long long soTien, bool isRefund = false, QWidget *parent = nullptr);

private slots:
    void xuLyThanhToan();
    void xuLyHuy();

private:
    void thietLapGiaoDien();
    
    PTRDG m_docGia;
    QString m_lyDo;
    long long m_soTien;
    bool m_isRefund;
    
    QPushButton *nutThanhToan;
    QPushButton *nutHuy;
    QCheckBox *hopInBienLai;
};

#endif // HOPTHOAITHANHTOAN_H
