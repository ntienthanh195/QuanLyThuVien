#ifndef PRINTHELPER_H
#define PRINTHELPER_H

#include <QString>
#include <QWidget>
#include <QList>
#include "models/DocGia.h"
#include "models/QuanLySach.h"

class QPrinter;
class QTextDocument;

/**
 * @brief PrintHelper - Utility class cho in ấn thẻ thư viện, tem mã vạch, báo cáo
 * Thiết kế tối giản phù hợp với Học viện Phật giáo
 */
class PrintHelper {
public:
    // Cấu hình in
    struct PrintConfig {
        QString tenThuVien = "HỌC VIỆN PHẬT GIÁO NAM TÔNG CẦN THƠ";
        QString tenVietTat = "HVPG NAMTONG CANTHO";
        bool inMauSac = false;
        bool hienThiLogo = true;
        QString duongDanLogo = "";
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
    static bool inTemSachTuDong(PTRDS dauSach, QWidget* parent = nullptr);
    static bool inNhieuTemSach(PTRDS dauSach, int soLuong, QWidget* parent = nullptr);
    // ===== IN BÁO CÁO =====
    static bool inDanhSachDocGia(PTRDG root, QWidget* parent = nullptr);
    static bool inDanhMucSach(PTRDS* dsDauSach, int soLuong, QWidget* parent = nullptr);
    static bool inDanhSachSach(PTRDS* dsDauSach, int soLuong, QWidget* parent = nullptr);
    static bool inBaoCaoThongKe(const QString& tieuDe, const QString& noiDungHTML, 
                                 QWidget* parent = nullptr);
    static bool inPhieuMuonTra(PTRDG docGia, const QList<QString>& danhSachSach,
                                bool laMuon, QWidget* parent = nullptr);
    static bool inPhieuMuonTuDong(PTRDG docGia, const QString& maSach, const QString& tenSach,
                                   const QString& ngayMuon, QWidget* parent = nullptr);

    // ===== HELPER FUNCTIONS =====
    static QString taoMaVachHTML(const QString& maSo, int fontSize = 48);
    static QString taoMaQRHTML(const QString& maSo, int size = 100);
    
    // Tạo tên file tự động
    static QString taoTenFilePhieuMuon(int maThe, const QString& maSach);
    static QString taoTenFilePhieuTra(int maThe, const QString& maSach);
    static QString taoTenFileTheDocGia(int maThe);
    static QString taoTenFileTemSach(const QString& isbn, int soLuong);
    static QString taoTenFileBaoCao(const QString& loaiBaoCao);
    static bool xuatPDF(QTextDocument* doc, const QString& tenFile, 
                        QWidget* parent = nullptr);

    // ===== CẤU HÌNH =====
    static void setCauHinh(const PrintConfig& config);
    static PrintConfig getCauHinh();

private:
    static PrintConfig s_config;
    
    static QString taoHTMLTheDocGia(PTRDG docGia);
    static QString taoHTMLA4NhieuThe(const QList<PTRDG>& danhSach, int soCot);
    static QString taoHTMLTemSach(const QString& maSach, const QString& tenSach);
    static QString taoHTMLDanhSach(const QString& tieuDe, const QString& noiDungBang);
    static bool inDocument(QTextDocument* doc, QWidget* parent);
    static bool kiemTraFontMaVach();
};

#endif // PRINTHELPER_H
