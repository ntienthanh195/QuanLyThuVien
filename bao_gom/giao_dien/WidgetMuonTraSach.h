/**
 * @file WidgetMuonTraSach.h
 * @status ACTIVE (V2)
 * @brief Giao diện Mượn Trả Sách chính thức.
 * @note AI Chú ý: Đây là file HEADER CHÍNH THỨC.
 *       Bỏ qua file trùng lặp tại include/views/WidgetMuonTraSach.h.
 */
/**
 * @file WidgetMuonTraSach.h
 * @path bao_gom/giao_dien/WidgetMuonTraSach.h
 * @description Header của Widget quản lý mượn trả sách (V2).
 * @project Library Management System (QuanLyThuVien)
 */
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
class WidgetLichSuMuonTra;

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

public:


    // NEW SIMPLIFIED SCANNER LOGIC
    void xuLyQuetThongMinh(const QString& input);
    
    enum TrangThaiSach {
        SACH_RANH,
        SACH_DOC_GIA_HIEN_TAI_DANG_MUON,
        SACH_NGUOI_KHAC_DANG_MUON,
        SACH_DANG_BAO_MAT, // NEW: Found a book that was previously reported as lost
        SACH_KHONG_TON_TAI
    };

    // State Machine for Smart Scan
    enum ScanState {
        STATE_IDLE,                 // Waiting for any scan
        STATE_WAITING_FOR_READER    // Scanned book, waiting for reader to borrow
    };



protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
     // New Helper Methods for Scanner
    void xuLyQuetMaDocGia(const QString& maThe);
    void xuLyQuetMaSach(const QString& maSach);
    TrangThaiSach kiemTraTrangThaiSach(const QString& maSach);
    void xuLyTuDongMuon(const QString& maSach);
    void xuLyTuDongTra(const QString& maSach);

    // Scanner deduplication
    QString m_scanBuffer;
    
    // State Machine Members
    ScanState m_currentState = STATE_IDLE;
    QString m_pendingBookCode; // The book waiting to be borrowed
    
    // Batch Print Queue
    QTimer* m_batchPrintTimer;
    QStringList m_batchPrintQueueMa;
    QStringList m_batchPrintQueueTen;
    
    // Batch Return Queue (separate from borrow)
    QStringList m_batchReturnQueueMa;
    QStringList m_batchReturnQueueTen;
    bool m_lastOperationWasBorrow = true; // Track mode to print correct receipt

signals:
    void thongBaoCapNhat(const QString &thongDiep);
    void duLieuThayDoi();  // Signal for auto-save
    void requestShowMuonTra(); // NEW: For Global Scan switch

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
    // Checkbox deleted
    void xuLyNutXacNhanMuon(bool skipConfirm = false);
    
    // Trả sách
    // Trả sách
    void xuLyNutXacNhanTra(bool skipConfirm = false);
    void xuLyNutBaoMat();
    void xuLyNutThanhToan(); // Feature 4: Thanh Toan
    void xuLyNutGiaHan(); // Extension slot
    void xuLyChonSachTra(int hang, int cot);
    void xuLyInLaiPhieu();

    // Multilingual Support
    void capNhatNgonNgu();



private:
    void thieLapGiaoDien();
    void thieLapTabMuonSach();
    void thieLapTabLichSu();
    
    // Thiết lập bảng
    void thieLapBangDocGia();
    
    // Hiển thị dữ liệu
    void hienThiDanhSachDocGia();
    void hienThiTrang();
    void capNhatBangSachDangMuon();
    
    // Scanner helpers
    bool laSachDocGiaHienTaiDangMuon(const QString& maSach);
    
    void capNhatThongTinDocGia(); // NEW: Update reader info label
    void xoaThongTinHienThi();
    int tinhSoNgayQuaHan(const std::string &ngayMuon);
    int tinhTienPhat(int soNgayQuaHan);
    
    // Auto-Return Helpers
    QList<QPair<PTRDG, QString>> TimDocGiaDangMuonSach(const QString &input);
    
    // Layout chính
    QVBoxLayout *boTriChinh;
    QTabWidget *hopTab;
    QLabel *nhanTieuDeChinh; // New: Main Title
    
    // === TAB MƯỢN SÁCH ===
    QWidget *widgetMuonSach;
    
    // Bảng độc giả (cột trái)
    QTableView *bangDocGia;
    DocGiaTableModel *modelDocGia;
    QLineEdit *oNhapTimDocGia;
    QPushButton *nutTrangTruoc;
    QPushButton *nutTrangSau;
    QPushButton *nutLamMoiDocGia; // New: Refresh button member
    QLabel *nhanTrangHienTai;
    QLabel *nhanTongDocGia;
    
    // Panel chi tiết (cột phải)
    QLabel *nhanThongTinDocGia;
    QGroupBox *nhomThongTinDocGia; // New: GroupBox Member
    QGroupBox *nhanTieuDeSachDangMuon;
    QTableView *bangSachDangMuon; // Changed from QTableWidget to QTableView
    QAbstractItemModel *modelSachDangMuon;
    std::vector<ThongTinSachDangMuon_DTO> danhSachSachDangMuon;
    QLineEdit *oNhapISBN;
    // ck hopXacNhanMuon deleted
    QPushButton *nutXacNhanMuon;
    QPushButton *nutXacNhanTra;
    QPushButton *nutBaoMat;
    QPushButton *nutThanhToan; // New button
    QPushButton *nutGiaHan; // Extension button
    QPushButton *nutInLaiPhieu;
    
    // Phân trang
    std::vector<PTRDG> danhSachDocGia;
    int trangHienTai;
    int soDocGiaTrenTrang;
    int tongSoTrang;
    
    // === TAB TRẢ SÁCH (MERGED) ===
    
    // === TAB LỊCH SỬ ===
    QWidget *widgetLichSu;
    WidgetLichSuMuonTra *widgetLichSuFull;
    
    // Dữ liệu hiện tại
    PTRDG docGiaHienTai;
    PTRDS sachDangChonMuon;
    std::string maSachDangChonTra;
    PTRDMS banSaoDangChon;
};

#endif // WIDGETMUONTRASACH_H
