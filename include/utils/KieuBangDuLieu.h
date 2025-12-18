#ifndef KIEUBANGDULIEU_H
#define KIEUBANGDULIEU_H

#include <QString>
#include <QTableView>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QPainter>

/**
 * @file KieuBangDuLieu.h
 * @brief Styled table với zebra striping, hover effects, và modern design
 * 
 * Features:
 * - Zebra striping (dòng chẵn/lẻ khác màu)
 * - Hover row highlight
 * - Selected row styling
 * - Modern header design
 * - Rounded corners
 * - Custom cell padding
 */

enum KieuBang {
    BANG_HIEN_DAI,      // Modern dark theme
    BANG_SANG,          // Light theme
    BANG_COMPACT        // Compact spacing
};

/**
 * @class TaoBangDuLieu
 * @brief Factory class để tạo table với styling nhất quán
 */
class TaoBangDuLieu {
public:
    /**
     * Tạo QTableView với modern styling
     * @param kieuBang Kiểu bảng (Hiện đại/Sáng/Compact)
     * @return QTableView* với style đã áp dụng
     */
    static QTableView* taoBang(KieuBang kieuBang = BANG_HIEN_DAI);
    
    /**
     * Apply styling cho table header
     */
    static void apDungStyleHeader(QHeaderView *header, KieuBang kieuBang = BANG_HIEN_DAI);
    
    /**
     * Lấy stylesheet cho table
     */
    static QString layStyleBangHienDai();
    static QString layStyleBangSang();
    static QString layStyleBangCompact();
    
    /**
     * Lấy stylesheet cho header
     */
    static QString layStyleHeaderHienDai();
    static QString layStyleHeaderSang();
    
private:
    static void cauHinhCoSo(QTableView *table);
};

/**
 * @class DelegateHangBang
 * @brief Custom delegate để vẽ zebra striping và hover effects
 */
class DelegateHangBang : public QStyledItemDelegate {
    Q_OBJECT
    
public:
    explicit DelegateHangBang(QObject *parent = nullptr);
    
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    
    void setKieuBang(KieuBang kieu);
    
private:
    KieuBang kieuBang;
    
    // Colors
    QString mauNenChan;      // Màu nền dòng chẵn
    QString mauNenLe;        // Màu nền dòng lẻ
    QString mauHover;        // Màu khi hover
    QString mauChon;         // Màu khi được chọn
    QString mauChu;          // Màu chữ
    QString mauVien;         // Màu viền
    
    void capNhatMauSac();
};

/**
 * @brief Style constants cho bảng
 */
namespace StyleBang {
    // Deep Dark Theme
    namespace HienDai {
        // Slate Grey palette (applied site-wide)
        const QString MAU_NEN_CHAN = "#1E293B";    // slate background for even rows
        const QString MAU_NEN_LE = "#16202B";      // slightly lighter for odd rows (card surface)
        const QString MAU_HOVER = "rgba(59, 130, 246, 0.08)";  // soft blue subtle hover
        const QString MAU_CHON = "rgba(59, 130, 246, 0.15)";   // selected row highlight
        const QString MAU_VIEN_CHON = "#3B82F6";  // primary soft-blue accent (focus border)
        const QString MAU_CHU = "#E2E8F0";        // readable muted text on dark background
        const QString MAU_HEADER_BG = "#16202B";  // header background
        const QString MAU_HEADER_TEXT = "#94A3B8";
        const QString MAU_BORDER = "#0F172A";    // subtle slate border
    }
    
    // Light Theme
    namespace Sang {
        const QString MAU_NEN_CHAN = "#FFFFFF";
        const QString MAU_NEN_LE = "#F8FAFC";
        const QString MAU_HOVER = "rgba(59, 130, 246, 0.05)";
        const QString MAU_CHON = "rgba(59, 130, 246, 0.1)";
        const QString MAU_VIEN_CHON = "#3B82F6";
        const QString MAU_CHU = "#0F172A";
        const QString MAU_HEADER_BG = "#F1F5F9";
        const QString MAU_HEADER_TEXT = "#1E293B";
        const QString MAU_BORDER = "#E2E8F0";
    }
    
    // Spacing - Tăng để table thoáng hơn
    const int PADDING_CELL = 14;           // Tăng từ 12 lên 14
    const int PADDING_CELL_COMPACT = 10;   // Tăng từ 8 lên 10
    const int CHIEU_CAO_HANG = 52;         // Tăng từ 48 lên 52
    const int CHIEU_CAO_HANG_COMPACT = 44; // Tăng từ 40 lên 44
    const int CHIEU_CAO_HEADER = 48;       // Tăng từ 44 lên 48
}

#endif // KIEUBANGDULIEU_H
