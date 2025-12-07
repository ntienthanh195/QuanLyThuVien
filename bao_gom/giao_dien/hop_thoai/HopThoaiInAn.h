#ifndef HOPTHOAIINAN_H
#define HOPTHOAIINAN_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QGraphicsDropShadowEffect>
#include <QLineEdit>
#include <QComboBox>
#include "models/DocGia.h"
#include "models/QuanLySach.h"

/**
 * @brief Dialog In Ấn Thư Viện - Thiết kế lại hoàn toàn
 * 
 * Chức năng chính theo 3 nhóm:
 * 1. IN THẺ ĐỘC GIẢ: In thẻ độc giả hiện tại hoặc hàng loạt (CR80: 85.6mm x 54mm)
 * 2. IN TEM MÃ VẠCH SÁCH: In tem dán gáy sách với QR/barcode (50mm x 30mm)
 * 3. IN BÁO CÁO / BIÊN BẢN: Danh sách độc giả, danh mục sách, phiếu mượn/trả (A4)
 * 
 * Backend tích hợp:
 * - PrintHelper: Hỗ trợ in và xuất PDF tự động
 * - QrCodeGenerator: Tạo QR code cho thẻ và phiếu
 * - Auto filename: Tên file tự động theo format chuẩn
 * - Workflow tự động: Quét mã → In phiếu mượn/trả
 */
class HopThoaiInAn : public QDialog {
    Q_OBJECT

public:
    explicit HopThoaiInAn(QWidget *parent = nullptr);
    ~HopThoaiInAn();

    // Thiết lập dữ liệu từ bên ngoài
    void setDocGiaHienTai(PTRDG docGia);
    void setDanhSachDauSach(PTRDS* dsDauSach, int soLuong);

private slots:
    // ========== IN THẺ ĐỘC GIẢ ==========
    void xuLyInTheDocGiaHienTai();      // In thẻ độc giả hiện tại (CR80 với QR)
    void xuLyInHangLoatThe();            // In hàng loạt toàn bộ độc giả

    // ========== IN TEM MÃ VẠCH SÁCH ==========
    void xuLyTimVaInTem();               // Hiển thị dialog tìm và in tem

    // ========== IN BÁO CÁO / BIÊN BẢN ==========
    void xuLyInDanhSachDocGia();         // Xuất PDF danh sách độc giả
    void xuLyInDanhMucSach();            // Xuất PDF danh mục sách
    void xuLyInPhieuMuon();              // In phiếu mượn với QR code auto-return
    void xuLyInPhieuTra();               // In phiếu trả
    void xuLyCauHinhIn();                // Mở dialog cấu hình in

private:
    // ========== UI SETUP ==========
    void thieLapGiaoDien();
    
    // ========== MODERN UI HELPERS ==========
    void apDungPhongCachModern();
    
    // ========== BACKEND HELPERS ==========
    bool kiemTraDocGiaHopLe();                       // Validate độc giả trước khi in

    // ========== DỮ LIỆU ==========
    PTRDG docGiaHienTai;                 // Độc giả được truyền vào
    PTRDS* dsDauSach;                    // Danh sách đầu sách toàn hệ thống
    int soLuongDauSach;
};

#endif // HOPTHOAIINAN_H
