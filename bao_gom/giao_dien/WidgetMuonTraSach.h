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

public:

    // Universal Scanner Support
    void xuLySmartScan(const QString& inputOverride = QString()); // DEPRECATED - Valid for legacy references

    // NEW SIMPLIFIED SCANNER LOGIC
    void xuLyQuetThongMinh(const QString& input);
    
    enum TrangThaiSach {
        SACH_RANH,
        SACH_DOC_GIA_HIEN_TAI_DANG_MUON,
        SACH_NGUOI_KHAC_DANG_MUON,
        SACH_KHONG_TON_TAI
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
    void xuLyNutQuetMaSach();
    void xuLyNutQuetMaDocGia();
    
    // Trả sách
    // Trả sách
    void xuLyNutXacNhanTra(bool skipConfirm = false);
    void xuLyNutBaoMat();
    void xuLyNutThanhToan(); // Feature 4: Thanh Toan
    void xuLyNutGiaHan(); // Extension slot
    void xuLyChonSachTra(int hang, int cot);
    void xuLyInLaiPhieu();
    
    // Legacy (giữ lại cho tương thích)
    void xuLyNhapMaThe(const QString &maThe);
    void onGlobalScanReceived(const QString &code);
    void xuLyInBatch();

    // Multilingual Support
    void capNhatNgonNgu();
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
    
    // Scanner helpers
    bool laSachDocGiaHienTaiDangMuon(const QString& maSach);
    
    void capNhatThongTinDocGia(); // NEW: Update reader info label
    
    // Legacy helpers
    void hienThiThongTinDocGia(PTRDG docGia);
    void hienThiDanhSachSachCoTheMuon();
    void hienThiDanhSachSachDangMuon(PTRDG docGia);
    void xoaThongTinHienThi();
    
    bool kiemTraDieuKienMuon(PTRDG docGia);
    int tinhSoNgayQuaHan(const std::string &ngayMuon);
    int tinhTienPhat(int soNgayQuaHan);
    
    // Auto-Return Helpers
    PTRDG TimDocGiaDangMuonSach(const QString &input, QString &resolvedMaSach);
    void xuLyReturnPressedMaSachTra();
    
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
    QPushButton *nutTimDocGia;
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
    QPushButton *nutQuetMaSach;
    QPushButton *nutQuetMaDocGia;
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
    
    // === TAB TRẢ SÁCH ===
    QWidget *widgetTraSach;
    QLineEdit *oNhapMaSachTra;
    QPushButton *nutTraSachNhanh; // Nút xác nhận trả (hoặc Auto-Submit)
    QPushButton *nutQuetMaSachTra;
    QLabel *nhanTieuDeTraSach; // New: Return Tab Title
    QLabel *nhanHuongDanTraSach; // New: Instruction Label
    
    // === TAB LỊCH SỬ ===
    QWidget *widgetLichSu;
    QLabel *nhanLichSuPlaceholder; // New: History Placeholder
    
    // Dữ liệu hiện tại
    PTRDG docGiaHienTai;
    PTRDS sachDangChonMuon;
    std::string maSachDangChonTra;
    PTRDMS banSaoDangChon;
};

#endif // WIDGETMUONTRASACH_H
