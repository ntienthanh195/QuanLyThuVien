#ifndef BANGDIEUKHIEN_H
#define BANGDIEUKHIEN_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QGroupBox>
#include <QChart>
#include <QChartView>
#include <QGraphicsDropShadowEffect>
#include <QTableView>
#include <QAbstractItemModel>
#include <QSpinBox>
#include "models/DocGia.h" // For PTRDG type

/**
 * @brief Lớp BangDieuKhien - Dashboard tổng quan hệ thống
 * 
 * Hiển thị:
 * - Thống kê tổng quan (số sách, độc giả, sách đang mượn, quá hạn)
 * - Biểu đồ trực quan
 * - Thông báo nhanh
 * - Truy cập nhanh các chức năng
 */
class BangDieuKhien : public QWidget {
    Q_OBJECT

public:
    explicit BangDieuKhien(QWidget *cha = nullptr);
    ~BangDieuKhien();
    
    // Cập nhật dữ liệu dashboard
    void capNhatThongKe();  // Legacy: Full recreate (deprecated)
    void capNhatThongKeNhanh(); // NEW: Lightweight update (only setText)
    void yeuCauCapNhatThongKe(); // NEW: Debounced request
    void capNhatNgonNgu(); // New method
    void capNhatBieuDo();

public slots:
    void capNhatRealtimeTuWidget(); // Nhận update realtime từ các widget khác

signals:
    void yeuCauChuyenDenQuanLySach();
    void yeuCauChuyenDenQuanLyDocGia();
    void yeuCauChuyenDenMuonTraSach();
    void yeuCauXemBaoCao();

private slots:
    void xuLyNutQuanLySach();
    void xuLyNutQuanLyDocGia();
    void xuLyNutMuonTraSach();
    void xuLyNutXemBaoCao();
    void xuLyNutInThe();
    void xuLyNutQuetMa();
    void xuLyMoDialogInAn();
    
    // Print panel slots (kept for compatibility)
    void xuLyInTheDocGiaHienTai();
    void xuLyInHangLoatThe();
    void xuLyInTemSachHienTai();
    void xuLyInNhieuTem();
    void xuLyInDanhSachDocGia();
    void xuLyInDanhMucSach();
    void xuLyInPhieuMuon();
    void xuLyInPhieuTra();
    
    // Export Log
    void xuatBaoCaoLog();

private:
    void thieLapGiaoDien();
    QWidget* taoHeaderDashboard();
    void taoTheThongKe(QVBoxLayout *boTri);
    QWidget* taoBieuDoTronThongKe();
    QWidget* taoBieuDoCotThongKe();
    void taoThongBaoNhanh();
    void taoNutTruyCapNhanh();
    
    QWidget* taoTheTongQuan(const QString &tieuDe, const QString &giaTri, 
                            const QString &bieuTuong, const QColor &mauNen);
    QWidget* taoTheNho(const QString &icon, const QString &soLieu, const QString &nhan, const QString &mau, QLabel **outLabelSoLieu = nullptr);
    QWidget* taoBangHoatDongGanDay();
    QWidget* taoPanelCanhBao();

    // Thêm hoạt động gần đây
    void appendActivity(const QString &text);
    void ghiNhatKy(const QString &hanhDong, const QString &noiDung);
    
    // Helper functions for stats calculation
    int demDocGia(PTRDG root) const;
    int demSachDangMuon() const;
    int demSachQuaHan() const;
    
    // Widgets
    QGridLayout *boTriChinh;
    
    // Các thẻ thống kê - lưu trữ để cập nhật realtime
    QGroupBox *nhomThongKe;
    QLabel *nhanTongSach;
    QLabel *nhanTongDocGia;
    QLabel *nhanSachDangMuon;
    QLabel *nhanSachQuaHan;
    
    // Widgets thẻ thống kê (để cập nhật nhanh)
    QLabel *labelSoLieuTongSach;
    QLabel *labelSoLieuTongDocGia;
    QLabel *labelSoLieuDangMuon;
    QLabel *labelSoLieuQuaHan;
    
    // Bảng hoạt động gần đây (model/view)
    QTableView *bangHoatDong;
    QAbstractItemModel *modelHoatDong;
    
    // Panel cảnh báo
    QWidget *panelCanhBao;
    
    // Biểu đồ
    QGroupBox *nhomBieuDo;
    QChartView *khungBieuDo;
    
    // Thông báo
    QGroupBox *nhomThongBao;
    QLabel *nhanThongBaoGanDay;
    
    // Nút truy cập nhanh
    QPushButton *nutQuanLySach;
    QPushButton *nutQuanLyDocGia;
    QPushButton *nutMuonTraSach;
    QPushButton *nutXemBaoCao;
    QPushButton *nutInThe;
    QPushButton *nutQuetMa;
    
    // Performance optimization
    QTimer *m_updateTimer = nullptr; // Debounce timer
};

#endif // BANGDIEUKHIEN_H
