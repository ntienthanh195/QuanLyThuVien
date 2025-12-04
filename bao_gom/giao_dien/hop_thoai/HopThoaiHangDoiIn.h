#ifndef HOPTHOAIHANGDOIIN_H
#define HOPTHOAIHANGDOIIN_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QSpinBox>
#include "models/DocGia.h"

/**
 * @brief HopThoaiHangDoiIn - Dialog quản lý hàng đợi in thẻ hàng loạt
 * 
 * Cho phép:
 * - Xem danh sách độc giả cần in
 * - Thêm/xóa độc giả khỏi hàng đợi
 * - In hàng loạt lên giấy A4 (10 thẻ/trang)
 * - Xuất PDF
 */
class HopThoaiHangDoiIn : public QDialog {
    Q_OBJECT

public:
    explicit HopThoaiHangDoiIn(PTRDG rootDocGia, QWidget *parent = nullptr);
    ~HopThoaiHangDoiIn();

private slots:
    void xuLyThemDocGia();
    void xuLyThemNhieuDocGia();
    void xuLyXoaDocGia();
    void xuLyXoaTatCa();
    void xuLyInA4();
    void xuLyXuatPDF();
    void xuLyChonDocGia(int hang);
    void capNhatDanhSach();

private:
    void taoGiaoDien();
    void capNhatThongTin();
    
    // Layouts
    QVBoxLayout* layoutChinh;
    QHBoxLayout* layoutNut;
    QHBoxLayout* layoutThongTin;
    
    // Widgets
    QLabel* nhanTieuDe;
    QListWidget* danhSachDocGia;
    QLabel* nhanThongTin;
    QLabel* nhanSoTrang;
    QSpinBox* spinSoCot;
    
    // Buttons
    QPushButton* nutThemDocGia;
    QPushButton* nutThemNhieu;
    QPushButton* nutXoa;
    QPushButton* nutXoaTatCa;
    QPushButton* nutInA4;
    QPushButton* nutXuatPDF;
    QPushButton* nutDong;
    
    // Data
    PTRDG rootDocGia;
};

#endif // HOPTHOAIHANGDOIIN_H
