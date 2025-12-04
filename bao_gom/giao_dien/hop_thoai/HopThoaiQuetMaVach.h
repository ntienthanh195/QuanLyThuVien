#ifndef HOPTHOAIQUETMAVACH_H
#define HOPTHOAIQUETMAVACH_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

/**
 * @brief Dialog quét mã (barcode và QR code scanner)
 * 
 * Chức năng:
 * - Nhập mã từ bàn phím, scanner barcode, hoặc QR scanner
 * - Tự động nhận diện loại mã (barcode/QR)
 * - Tìm kiếm sách hoặc độc giả theo mã
 * - Hiển thị thông tin tìm được
 * - Trả về mã đã quét để sử dụng trong form khác
 */
class HopThoaiQuetMaVach : public QDialog {
    Q_OBJECT

public:
    enum LoaiQuet {
        QUET_SACH,      // Quét ISBN/mã sách
        QUET_DOC_GIA,   // Quét mã thẻ độc giả
        QUET_TAT_CA     // Quét cả 2 loại
    };

    explicit HopThoaiQuetMaVach(LoaiQuet loai = QUET_TAT_CA, QWidget *cha = nullptr);
    ~HopThoaiQuetMaVach();

    // Lấy mã đã quét
    QString layMaQuet() const;
    
    // Lấy loại mã đã quét (sách hay độc giả)
    QString layLoaiMaQuet() const;

signals:
    void maVachDuocQuet(const QString &ma, const QString &loai);

private slots:
    void xuLyQuetMa();
    void xuLyXacNhan();
    void xuLyHuy();
    void xuLyXoaMa();

private:
    void thieLapGiaoDien();
    void timKiemVaHienThi(const QString &ma);
    void hienThiThongTinSach(const QString &isbn);
    void hienThiThongTinDocGia(const QString &maThe);
    
    // UI Components
    QLineEdit *oNhapMa;
    QPushButton *nutQuet;
    QPushButton *nutXacNhan;
    QPushButton *nutHuy;
    QPushButton *nutXoaMa;
    QLabel *nhanKetQua;
    QGroupBox *nhomThongTin;
    QLabel *nhanThongTin;
    
    // Data
    LoaiQuet loaiQuet;
    QString maQuetDuoc;
    QString loaiMaQuet;  // "SACH" hoặc "DOC_GIA"
};

#endif // HOPTHOAIQUETMAVACH_H
