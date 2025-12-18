/**
 * @file WidgetQuanLyDocGia.h
 * @status ACTIVE (V2)
 * @brief Giao diện Quản lý Độc Giả chính thức.
 * @note AI Chú ý: Đây là file HEADER CHÍNH THỨC.
 *       Bỏ qua file trùng lặp tại include/views/WidgetQuanLyDocGia.h.
 */
#ifndef WIDGETQUANLYDOCGIA_H
#define WIDGETQUANLYDOCGIA_H

#include <QWidget>
#include <QTableView>
#include <vector>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>

// Include backend models
#include "models/DocGia.h"

/**
 * @brief Lớp WidgetQuanLyDocGia - Quản lý độc giả
 * 
 * Chức năng:
 * - Hiển thị danh sách độc giả (cây AVL -> bảng)
 * - Thêm/sửa/xóa độc giả
 * - Khóa/mở khóa thẻ độc giả
 * - Tìm kiếm theo mã thẻ, họ, tên
 * - Lọc theo trạng thái (hoạt động/bị khóa)
 * - Xem lịch sử mượn trả
 * - Xem sách đang mượn
 */
class WidgetQuanLyDocGia : public QWidget {
    Q_OBJECT

public:
    explicit WidgetQuanLyDocGia(QWidget *cha = nullptr);
    ~WidgetQuanLyDocGia();
    
    // Cập nhật hiển thị
    void taiLaiDanhSach();
    void capNhatBang();

signals:
    void thongBaoCapNhat(const QString &thongDiep);
    void duLieuThayDoi();  // Signal for auto-save

private slots:
    void xuLyNutThemDocGia();
    void xuLyNutSuaDocGia();
    void xuLyNutXoaDocGia();
    void xuLyNutKhoaThe();
    void xuLyNutMoKhoaThe();
    void xuLyNutXemLichSu();
    void xuLyNutXemSachDangMuon();
    void xuLyTimKiem(const QString &tuKhoa);
    void xuLyLocTrangThai(int chiSo);
    void xuLyLocGioiTinh(int chiSo);
    void xuLyChonDong(int dong, int cot);
    void xuLyNhapDuLieuTu(); // Restored
    
    // Multilingual Support
    void capNhatNgonNgu();

private:
    void thieLapGiaoDien();
    void setupConnections();
    void thieLapTimKiem();
    void thieLapNutChucNang();
    void thieLapBangDocGia(); // Added missing declaration
    void hienThiDanhSachDocGia();
    void hienThiThongTinChiTiet(PTRDG docGia);
    
    // Hàm hỗ trợ tìm kiếm và lọc
    void xuLyLocKetHopToanBo();
    void hienThiKetQuaTimKiem(PTRDG ketQua[], int soKetQua);
    // (migration) per-row insertion replaced by model-backed view
    
    QString layTenTrangThai(int trangThai) const;
    QString layTenGioiTinh(bool phai) const;
    QColor layMauTrangThai(int trangThai) const;
    
    // Widgets chính
    QVBoxLayout *boTriChinh;
    
    // Tìm kiếm và lọc
    QGroupBox *nhomTimKiem;
    QLineEdit *oNhapTimKiem;
    QComboBox *hopChonTrangThai;
    QComboBox *hopChonGioiTinh;
    QPushButton *nutTimKiem;
    QPushButton *nutLamMoi;
    
    // Bảng dữ liệu
    QTableView *bangDocGia;
    QAbstractItemModel *modelDocGia;
    std::vector<PTRDG> danhSachDocGia;
    QLabel *nhanThongTinChon;
    
    // Thông tin chi tiết độc giả được chọn
    QGroupBox *nhomThongTinChiTiet;
    QLabel *nhanMaThe;
    QLabel *nhanHoTen;
    QLabel *nhanGioiTinh;
    QLabel *nhanTrangThai;
    QLabel *nhanSoSachDangMuon;
    
    // Nút chức năng
    QGroupBox *nhomChucNang;
    QPushButton *nutThemDocGia;
    QPushButton *nutSuaDocGia;
    QPushButton *nutXoaDocGia;
    QPushButton *nutKhoaThe;
    QPushButton *nutMoKhoaThe;
    QPushButton *nutXemLichSu;
    QPushButton *nutXemSachDangMuon;
    QPushButton *nutNhapFile;
    QPushButton *nutXuatFile;
    // Header action buttons (top bar)
    QPushButton *nutHeaderIn;
    QPushButton *nutHeaderChiTiet;
    QPushButton *nutHeaderKhoa;
    QPushButton *nutHeaderMoKhoa;
    QPushButton *nutHeaderLichSu;
    
    // Dữ liệu hiện tại
    PTRDG docGiaDangChon;
    int maTheDangChon;
};

#endif // WIDGETQUANLYDOCGIA_H
