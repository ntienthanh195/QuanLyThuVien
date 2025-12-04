#ifndef WIDGETQUANLYSACH_H
#define WIDGETQUANLYSACH_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSet>

// Include backend models
#include "models/DauSach.h"
#include "models/QuanLySach.h"

// Include dialogs
#include "views/dialogs/ManHinhChiTietSach.h"

/**
 * @brief Lớp WidgetQuanLySach - Quản lý sách
 * 
 * Chức năng:
 * - Hiển thị danh sách đầu sách và bản sao
 * - Thêm/sửa/xóa đầu sách
 * - Thêm/xóa bản sao
 * - Tìm kiếm theo ISBN, tên sách, tác giả, thể loại
 * - Lọc theo trạng thái
 * - Xem chi tiết sách
 */
class WidgetQuanLySach : public QWidget {
    Q_OBJECT

public:
    explicit WidgetQuanLySach(QWidget *cha = nullptr);
    ~WidgetQuanLySach();
    
    // Cập nhật hiển thị
    void taiLaiDanhSach();
    void capNhatBang();

signals:
    void thongBaoCapNhat(const QString &thongDiep);
    void duLieuThayDoi();  // Signal for auto-save

private slots:
    void xuLyNutThemSach();
    void xuLyNutSuaSach();
    void xuLyNutXoaSach();
    void xuLyNutThemBanSao();
    void xuLyNutXoaBanSao();
    void xuLyNutXemChiTiet();
    void xuLyTimKiem(const QString &tuKhoa);
    void xuLyLocTheLoai(int chiSo);
    void xuLyLocTrangThai(int chiSo);
    void xuLyChonDong(int dong, int cot);
    void xuLyNhapDuLieuTu();

private:
    void thieLapGiaoDien();
    void thieLapHeaderBar();
    void thieLapBangSach();
    void thieLapTimKiem();
    void thieLapNutChucNang();
    void hienThiDanhSachSach();
    void hienThiBanSao(PTRDS dauSach);
    void taiDanhSachTheLoai();
    
    QString layTrangThaiSach(TrangThaiSach trangThai) const;
    QColor layMauTrangThai(TrangThaiSach trangThai) const;
    
    // Widgets chính
    QVBoxLayout *boTriChinh;
    
    // Tìm kiếm và lọc
    QGroupBox *nhomTimKiem;
    QLineEdit *oNhapTimKiem;
    QComboBox *hopChonTheLoai;
    QComboBox *hopChonTrangThai;
    QPushButton *nutTimKiem;
    QPushButton *nutLamMoi;
    QLabel *nhanKetQuaTimKiem;
    
    // Bảng dữ liệu
    QTableWidget *bangSach;
    QTableWidget *bangBanSao;
    QLabel *nhanThongTinChon;
    
    // Nút chức năng
    QGroupBox *nhomChucNang;
    QPushButton *nutThemSach;
    QPushButton *nutSuaSach;
    QPushButton *nutXoaSach;
    QPushButton *nutThemBanSao;
    QPushButton *nutXoaBanSao;
    QPushButton *nutXemChiTiet;
    QPushButton *nutNhapFile;
    QPushButton *nutXuatFile;
    
    // Dữ liệu hiện tại
    PTRDS dauSachDangChon;
    std::string maSachBanSaoChon;
};

#endif // WIDGETQUANLYSACH_H
