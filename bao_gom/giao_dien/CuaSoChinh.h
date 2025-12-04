#ifndef CUASOCHINH_H
#define CUASOCHINH_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QStatusBar>
#include <QListWidget>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>

// Forward declarations cho các widget
class BangDieuKhien;
class WidgetQuanLySach;
class WidgetQuanLyDocGia;
class WidgetMuonTraSach;
class AutoSaveManager;

/**
 * @brief Lớp CuaSoChinh - Cửa sổ chính của ứng dụng Quản Lý Thư Viện
 * 
 * Cửa sổ này quản lý tất cả các màn hình con:
 * - Bảng điều khiển (Dashboard)
 * - Quản lý sách
 * - Quản lý độc giả
 * - Mượn trả sách
 * - Các báo cáo thống kê
 */
class CuaSoChinh : public QMainWindow {
    Q_OBJECT

public:
    explicit CuaSoChinh(QWidget *cha = nullptr);
    ~CuaSoChinh();
    
    // Hàm tạo widget trạng thái cho UI hiện đại
    static QWidget* taoTagTrangThai(int trangThai);
    
    // Trigger auto-save from widgets
    void triggerAutoSave();

protected:
    void closeEvent(QCloseEvent *sukien) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void hienThiManHinhChinh();
    void hienThiQuanLySach();
    void hienThiQuanLyDocGia();
    void hienThiMuonTraSach();
    void hienThiGioiThieu();
    void hienThiThongKe();
    void hienThiTop10();
    void hienThiBaoCaoQuaHan();
    void hienThiSaoLuu();
    void hienThiIn();
    void capNhatThanhTrangThai(const QString &thongDiep, int thoiGianHienThi = 3000);
    void toggleSidebar();

private:
    // Thiết lập giao diện
    void thieLapGiaoDien();
    void thieLapMenuBen();
    void thieLapKetNoi();
    void apDungKieuMau();
    
    // Auto-save functions
    void thieLapAutoSave();
    void luuTatCaDuLieu();
    
    // Tạo các thành phần UI
    QPushButton* taoNutMenu(const QString &chuoi, const QString &duongDanBieuTuong);
    QWidget* taoTieuDeManHinh(const QString &tieuDe, const QString &moTa);
    
    // Widgets chính
    QStackedWidget *boChuyenManHinh;
    QWidget *widgetManHinhChinh;
    BangDieuKhien *bangDieuKhien;
    WidgetQuanLySach *widgetSach;
    WidgetQuanLyDocGia *widgetDocGia;
    WidgetMuonTraSach *widgetMuonTra;
    
    // Menu sidebar
    QWidget *khungMenuBen;
    QListWidget *danhSachMenu;
    QVBoxLayout *boTriMenuBen;
    QPushButton *nutToggleSidebar;
    bool sidebarDangMoRong;
    int sidebarWidthMax;
    int sidebarWidthMin;
    
    // Thanh công cụ và trạng thái
    QToolBar *thanhCongCu;
    QLabel *nhanTieuDe;
    QLabel *nhanNguoiDung;
    
    // Auto-save manager
    AutoSaveManager *autoSaveManager;
    
    // Lưu trạng thái
    int chiSoManHinhHienTai;
};

#endif // CUASOCHINH_H
