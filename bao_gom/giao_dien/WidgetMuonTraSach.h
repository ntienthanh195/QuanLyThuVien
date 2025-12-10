#ifndef WIDGETMUONTRASACH_H
#define WIDGETMUONTRASACH_H

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDateEdit>
#include <QListWidget>
#include <QCheckBox>
#include <vector>

class QAbstractItemModel;
class DocGiaTableModel;

// Include backend models
#include "models/DocGia.h"
#include "models/DauSach.h"
#include "models/QuanLySach.h"

/**
 * @brief Lớp WidgetMuonTraSach - Quản lý mượn trả sách
 * 
 * Chức năng:
 * - Mượn sách: nhập mã thẻ, chọn sách, xác nhận
 * - Trả sách: nhập mã thẻ, chọn sách đang mượn, xác nhận trả
 * - Báo mất sách
 * - Xem danh sách sách đang mượn của độc giả
 * - Xem lịch sử mượn trả
 * - Tính tiền phạt quá hạn
 */
class WidgetMuonTraSach : public QWidget {
    Q_OBJECT

public:
    explicit WidgetMuonTraSach(QWidget *cha = nullptr);
    ~WidgetMuonTraSach();
    
    // Cập nhật hiển thị
    void lamMoi();

signals:
    void thongBaoCapNhat(const QString &thongDiep);
    void duLieuThayDoi();  // Signal for auto-save

private slots:
    // Tìm kiếm và lọc
    void xuLyTimKiem();
    void xuLyLamMoiDanhSach();
    
    // Phân trang
    void xuLyTrangTruoc();
    void xuLyTrangSau();
    
    // Chọn độc giả
    void xuLyChonDocGia(int hang, int cot);
    void xuLyHuyChonDocGia();
    
    // Mượn sách
    void xuLyThayDoiCheckbox(int trangThai);
    void xuLyNutXacNhanMuon();
    void xuLyNutQuetMaSach();
    void xuLyNutQuetMaDocGia();
    
    // Trả sách
    void xuLyNutXacNhanTra();
    void xuLyNutBaoMat();
    void xuLyChonSachTra(int hang, int cot);
    void xuLyInLaiPhieu();
    
    // Legacy (giữ lại cho tương thích)
    void xuLyNhapMaThe(const QString &maThe);
    void xuLyChonSachMuon(int dong);
    void xuLyChuyenTab(int chiSo);

private:
    void thieLapGiaoDien();
    void thieLapTabMuonSach();
    void thieLapTabTraSach();
    void thieLapTabLichSu();
    
    // Thiết lập bảng
    void thieLapBangDocGia();
    
    // Hiển thị dữ liệu
    void hienThiDanhSachDocGia();
    void hienThiTrang();
    void capNhatBangSachDangMuon();
    
    // Legacy helpers
    void hienThiThongTinDocGia(PTRDG docGia);
    void hienThiDanhSachSachCoTheMuon();
    void hienThiDanhSachSachDangMuon(PTRDG docGia);
    void xoaThongTinHienThi();
    
    bool kiemTraDieuKienMuon(PTRDG docGia);
    int tinhSoNgayQuaHan(const std::string &ngayMuon);
    int tinhTienPhat(int soNgayQuaHan);
    
    // Layout chính
    QVBoxLayout *boTriChinh;
    QTabWidget *hopTab;
    
    // === TAB MƯỢN SÁCH ===
    QWidget *widgetMuonSach;
    
    // Bảng độc giả (cột trái)
    QTableView *bangDocGia;
    DocGiaTableModel *modelDocGia;
    QLineEdit *oNhapTimDocGia;
    QPushButton *nutTimDocGia;
    QPushButton *nutTrangTruoc;
    QPushButton *nutTrangSau;
    QLabel *nhanTrangHienTai;
    QLabel *nhanTongDocGia;
    
    // Panel chi tiết (cột phải)
    QLabel *nhanThongTinDocGia;
    QGroupBox *nhanTieuDeSachDangMuon;
    QTableView *bangSachDangMuon; // Changed from QTableWidget to QTableView
    QAbstractItemModel *modelSachDangMuon;
    std::vector<ThongTinSachDangMuon_DTO> danhSachSachDangMuon;
    QLineEdit *oNhapISBN;
    QPushButton *nutQuetMaSach;
    QPushButton *nutQuetMaDocGia;
    QCheckBox *hopXacNhanMuon;
    QPushButton *nutXacNhanMuon;
    QPushButton *nutXacNhanTra;
    QPushButton *nutBaoMat;
    QPushButton *nutInLaiPhieu;
    
    // Phân trang
    std::vector<PTRDG> danhSachDocGia;
    int trangHienTai;
    int soDocGiaTrenTrang;
    int tongSoTrang;
    
    // === TAB TRẢ SÁCH ===
    QWidget *widgetTraSach;
    
    // === TAB LỊCH SỬ ===
    QWidget *widgetLichSu;
    
    // Dữ liệu hiện tại
    PTRDG docGiaHienTai;
    PTRDS sachDangChonMuon;
    std::string maSachDangChonTra;
    PTRDMS banSaoDangChon;
};

#endif // WIDGETMUONTRASACH_H
