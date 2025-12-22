/**
 * @file PrintHelper.h
 * @path include/utils/PrintHelper.h
 * @description Header các hàm tiện ích in ấn.
 * @project Library Management System (QuanLyThuVien)
 */
#ifndef PRINTHELPER_H

#define PRINTHELPER_H

#include <QString>
#include <QWidget>
#include <QList>
#include <QPair>
#include <QDesktopServices>
#include <QUrl>
#include <QMap>
#include <QAbstractItemModel>
#include "models/DocGia.h"
#include "models/QuanLySach.h"

class QPrinter;
class QTextDocument;
class QAbstractItemModel;
template <typename K, typename V> class QMap;

/**
 * @brief PrintHelper - Utility class cho in ấn thẻ thư viện, tem mã vạch, báo cáo
 * Thiết kế tối giản phù hợp với Học viện Phật giáo
 */
class PrintHelper {
public:
    // Cấu hình khối trong tem
    struct LabelBlock {
        QString type;    // "text", "barcode"
        QString field;   // "tenDonVi", "maBanSao", "tenSach", "giaTien"
        QString position; // "top", "bottom", "middle"
        int fontSize = 8;
        bool bold = false;
        QString textAlign = "center"; // "left", "right", "center"
    };

    // Cấu hình in
    struct PrintConfig {
        // ═══ THÔNG TIN THƯ VIỆN ═══
        QString tenThuVien = "HỌC VIỆN PHẬT GIÁO NAM TÔNG KHMER";
        QString tenVietTat = "HVPGNTK";
        QString loiChao = "Xin cảm ơn!";
        
        // ═══ THIẾT LẬP IN ═══
        bool inMauSac = false;
        bool hienThiLogo = true;
        bool silentPrint = false;
        QString printerReceipt = ""; 
        QString printerReport = "";  
        QString duongDanLogo = "";
        QString pdfOutputDir = "";   
        QString qrOutputDir = "";    // Adding back for QR export
        QString duongDanThuMucIn = "in"; // Legacy default
        QString thuMucPdf = "pdf";       // Legacy default
        QString thuMucQR = "qr";         // Legacy default
        
        // ═══ GIẤY & HƯỚNG ═══
        QString pageSize = "A4";      // A4, A5, Letter...
        QString orientation = "Portrait"; // Portrait, Landscape
        
        // ═══ TEM SÁCH (BOOK LABELS) ═══
        int labelWidth = 50;     // mm
        int labelHeight = 30;    // mm
        int labelCols = 0;       // 0 = Auto calculate
        int labelGapMm = 5;      // Gap between labels
        bool showLibName = true;
        bool showBookTitle = true;
        bool showPrice = false;
        bool showBorder = true;
        QString fontFamily = "Times New Roman";
        double barcodeWidthRatio = 0.9;
        
        // Danh sách các khối hiển thị trên tem (Dynamic Layout)
        QList<LabelBlock> blocks;

        // ═══ THẺ HỌC VIÊN (STUDENT CARDS) ═══
        double cardWidth = 80.0;
        double cardHeight = 50.0;
        int cardCols = 2;
        int cardGapMm = 5;
        int cardValidityMonths = 12;
        
        // ═══ MÃ VẠCH (BARCODE) ═══
        int barcodeDPI = 300;
        double barcodeModuleWidthMm = 0.33;
        double barcodeQuietZoneMm = 4.0;
        int defaultBarcodeHeight = 14; // mm
        
        // ═══ BÁO CÁO (REPORTS) ═══
        bool reportShowSummary = true;
    };

    // ===== IN THẺ ĐỘC GIẢ =====
    static bool inTheDocGia(PTRDG docGia, QWidget* parent = nullptr);
    static bool inTheDocGiaTuDong(PTRDG docGia, QWidget* parent = nullptr);
    static bool inNhieuTheDocGia(const QList<PTRDG>& danhSach, QWidget* parent = nullptr);
    static bool inHangLoatTheDocGia(PTRDG root, QWidget* parent = nullptr);
    static bool inA4NhieuThe(const QList<PTRDG>& danhSach, int soCot = 2, QWidget* parent = nullptr);

    // ===== IN TEM MÃ VẠCH SÁCH =====
    static bool inTemMaVachSach(const QString& maSach, const QString& tenSach, 
                                 QWidget* parent = nullptr);
    static bool inTemSachTuDong(PTRDS dauSach, int soLuong = -1, QWidget* parent = nullptr);
    static bool inNhieuTemSach(PTRDS dauSach, int soLuong, QWidget* parent = nullptr);
    static bool inNhieuTemSachBatch(const QList<QPair<PTRDS, int>>& list, QWidget* parent = nullptr);
    static bool inTemSachTuyChon(PTRDS dauSach, const QStringList& codes, QWidget* parent = nullptr);
    // ===== IN BÁO CÁO =====
    // ===== IN BÁO CÁO (REPORTS) =====
    static bool inDanhSachDocGia(PTRDG root, QWidget* parent = nullptr);
    static bool inDanhMucSach(PTRDS* dsDauSach, int soLuong, QWidget* parent = nullptr);
    static bool inDanhSachSach(PTRDS* dsDauSach, int soLuong, QWidget* parent = nullptr);
    static bool inBaoCaoThongKe(const QString& tieuDe, const QString& noiDungHTML, 
                                 QWidget* parent = nullptr);
    static bool inBaoCaoTuyChinh(const QString& tieuDe, const QString& html, QWidget* parent = nullptr);
    
    // In Lịch Sử Mượn Trả (Refactored Unified)
    static bool inLichSuMuonTra(QAbstractItemModel* model, 
                               const QString& timeRange, 
                               const QMap<QString, double>& stats, 
                               QWidget* parent = nullptr);

    // In Phiếu Mượn/Trả (Receipts)
    static bool inPhieuMuonTra(PTRDG docGia, const QList<QString>& danhSachSach,
                                bool laMuon, QWidget* parent = nullptr);
    static bool inPhieuMuonTuDong(PTRDG docGia, const QString& maSach, const QString& tenSach,
                                   const QString& ngayMuon, QWidget* parent = nullptr, bool forcePreview = false);
    static bool inPhieuMuonBatch(PTRDG docGia, const QList<ThongTinSachDangMuon_DTO>& danhSachSach, QWidget* parent = nullptr);
    static bool inPhieuTraTuDong(PTRDG docGia, const QString& maSach, const QString& tenSach,
                                  const QString& ngayTra, QWidget* parent = nullptr, bool forcePreview = false);
    static bool inBienLaiPhat(PTRDG docGia, const QString& lyDo, long long soTien, bool isRefund = false, QWidget* parent = nullptr);
    
    // Legacy - Remove if unused or redirect
    static bool xuatLichSuMuonTraToPDF(PTRDG docGia, QWidget* parent = nullptr); 
    static bool inBaoCaoTaiChinh(const QString& start, const QString& end, const QString& tieuDe, QAbstractItemModel* model, QWidget* parent = nullptr);
    
    // In báo cáo kiểm kê (Updated Signature)
    static bool inBaoCaoKiemKe(const QStringList& missing, const QStringList& unknown, const QStringList& borrowedAnomalies,
                               const QStringList& borrowed, const QMap<QString, int>& stats, QWidget* parent = nullptr);

    // ===== HELPER FUNCTIONS =====
    static QString taoMaVachHTML(const QString& maSo, int fontSize = 48);
    static QString taoMaQRHTML(const QString& maSo, int size = 100);
    
    // Helper to generate Base64 Barcode Image (Public for Config Preview)
    // Parameters: text, heightMm (millimeters), targetDPI (default 300)
    static QString generateBarcodeBase64(const QString &text, double heightMm, int targetDPI = 300, double maxWidthMm = 0);
    
    // Tạo tên file tự động (Chuẩn tiếng Việt không dấu)
    static QString taoTenFilePhieuMuon(int maThe, const QString& hoTen, const QString& maSach);
    static QString taoTenFilePhieuTra(int maThe, const QString& hoTen, const QString& maSach);
    static QString taoTenFileTheDocGia(int maThe, const QString& hoTen);
    static QString taoTenFileTemSach(const QString& isbn, int soLuong);
    static QString taoTenFileBaoCao(const QString& loaiBaoCao);
    static bool xuatPDF(QTextDocument* doc, const QString& tenFile, 
                        QWidget* parent = nullptr);
    
    // Core printing function (public to allow reuse in other managers)
    static bool inDocument(QTextDocument* doc, QWidget* parent, const QString& printerName = "");


    // ===== CẤU HÌNH =====
    static void setCauHinh(const PrintConfig& config);
    static PrintConfig getCauHinh();
    static void reloadCauHinh(); // Force reload from SettingsManager

private:
    static PrintConfig s_config;
    
    static QString taoHTMLTheDocGia(PTRDG docGia);
    static QString taoHTMLPhieuMuon(PTRDG docGia, const QString& maSach, const QString& tenSach, const QString& ngayGiaoDich);
    static QString taoHTMLA4NhieuThe(const QList<PTRDG>& danhSach, int soCot);
    static QString taoHTMLTemSach(const QString& maSach, const QString& tenSach, const QString& giaSach = "");
    static QString taoHTMLDanhSach(const QString& tieuDe, const QString& noiDungBang);
    
    // Private audit helper (Phase 10)
    static bool logReceiptToPDF(QTextDocument* doc, const QString& type);
    
    static bool kiemTraFontMaVach();
    
    // Helper xử lý logic in chung (Hỏi -> In hoặc PDF)
    static bool xuLyYeuCauIn(QTextDocument* doc, const QString& tieuDe, const QString& cauHoi, const QString& tenFilePdfMacDinh, QWidget* parent, const QString& printerName = "", bool forcePreview = false);
    
    // NEW: Unified print preview function with auto-filename for PDF export
    static bool showPrintPreview(QTextDocument* doc, const QString& title, QWidget* parent = nullptr, const QString& defaultFileName = "");
};

#endif // PRINTHELPER_H
