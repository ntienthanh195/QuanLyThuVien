#ifndef KIENUTBAM_H
#define KIENUTBAM_H

#include <QString>
#include <QPushButton>
#include <QIcon>

/**
 * @file KieuNutBam.h
 * @brief Định nghĩa các kiểu nút bấm và style cho hệ thống
 * 
 * Hệ thống phân cấp nút bấm:
 * - Primary: Hành động chính (Thêm, Lưu, Xác nhận)
 * - Secondary: Hành động phụ (Hủy, Quay lại)
 * - Danger: Hành động nguy hiểm (Xóa, Thanh lý)
 * - Success: Hành động thành công (Hoàn tất, Duyệt)
 * - Ghost: Hành động nhẹ (Export, Print)
 */

enum KieuNut {
    NUT_PRIMARY,      // Xanh gradient - Hành động chính
    NUT_SECONDARY,    // Xám outline - Hành động phụ  
    NUT_DANGER,       // Đỏ gradient - Xóa, nguy hiểm
    NUT_SUCCESS,      // Xanh lá gradient - Thành công
    NUT_GHOST,        // Trong suốt - Tertiary actions
    NUT_WARNING       // Vàng/cam - Cảnh báo
};

enum KichThuocNut {
    NUT_LON,          // 50px height - Primary actions
    NUT_TRUNG_BINH,   // 42px height - Standard
    NUT_NHO           // 36px height - Compact areas
};

enum ViTriIcon {
    ICON_TRAI,        // Icon ở bên trái text
    ICON_PHAI,        // Icon ở bên phải text
    ICON_DOC_LAP,     // Chỉ có icon, không có text
    KHONG_ICON        // Không có icon
};

/**
 * @class TaoNutBam
 * @brief Factory class để tạo các nút bấm với style nhất quán
 */
class TaoNutBam {
public:
    /**
     * Tạo nút bấm với đầy đủ tùy chỉnh
     * @param chuDe Text hiển thị trên nút
     * @param kieuNut Loại nút (Primary/Secondary/Danger...)
     * @param kichThuoc Kích thước nút (Lớn/Trung bình/Nhỏ)
     * @param iconPath Đường dẫn đến icon (optional)
     * @param viTriIcon Vị trí icon (Trái/Phải/Độc lập/Không)
     * @return QPushButton* với style đã được áp dụng
     */
    static QPushButton* taoNut(
        const QString &chuDe,
        KieuNut kieuNut = NUT_PRIMARY,
        KichThuocNut kichThuoc = NUT_TRUNG_BINH,
        const QString &iconPath = QString(),
        ViTriIcon viTriIcon = ICON_TRAI
    );
    
    /**
     * Tạo nút Primary (hành động chính)
     */
    static QPushButton* taoNutChinh(const QString &chuDe, const QString &iconPath = QString());
    
    /**
     * Tạo nút Secondary (hành động phụ)
     */
    static QPushButton* taoNutPhu(const QString &chuDe, const QString &iconPath = QString());
    
    /**
     * Tạo nút Danger (xóa, nguy hiểm)
     */
    static QPushButton* taoNutNguyHiem(const QString &chuDe, const QString &iconPath = QString());
    
    /**
     * Tạo nút Success (hoàn tất)
     */
    static QPushButton* taoNutThanhCong(const QString &chuDe, const QString &iconPath = QString());
    
    /**
     * Tạo nút Ghost (tertiary)
     */
    static QPushButton* taoNutNhe(const QString &chuDe, const QString &iconPath = QString());
    
    /**
     * Lấy stylesheet cho từng loại nút
     */
    static QString layStylePrimary(KichThuocNut kichThuoc);
    static QString layStyleSecondary(KichThuocNut kichThuoc);
    static QString layStyleDanger(KichThuocNut kichThuoc);
    static QString layStyleSuccess(KichThuocNut kichThuoc);
    static QString layStyleGhost(KichThuocNut kichThuoc);
    static QString layStyleWarning(KichThuocNut kichThuoc);
    
private:
    static QString layChieuCao(KichThuocNut kichThuoc);
    static QString layFontSize(KichThuocNut kichThuoc);
    static QString layPadding(KichThuocNut kichThuoc);
};

/**
 * @brief Các style constants cho nút bấm
 */
namespace StyleNutBam {
    // Colors
    const QString MAU_PRIMARY = "#3B82F6";
    const QString MAU_PRIMARY_HOVER = "#2563EB";
    const QString MAU_PRIMARY_PRESSED = "#1D4ED8";
    
    const QString MAU_DANGER = "#EF4444";
    const QString MAU_DANGER_HOVER = "#DC2626";
    const QString MAU_DANGER_PRESSED = "#B91C1C";
    
    const QString MAU_SUCCESS = "#10B981";
    const QString MAU_SUCCESS_HOVER = "#059669";
    const QString MAU_SUCCESS_PRESSED = "#047857";
    
    const QString MAU_WARNING = "#F59E0B";
    const QString MAU_WARNING_HOVER = "#D97706";
    const QString MAU_WARNING_PRESSED = "#B45309";
    
    const QString MAU_SECONDARY = "#6B7280";
    const QString MAU_SECONDARY_HOVER = "#4B5563";
    
    // Border radius
    const QString BORDER_RADIUS_LON = "12px";
    const QString BORDER_RADIUS_TRUNG = "10px";
    const QString BORDER_RADIUS_NHO = "8px";
    
    // Shadows
    const QString BOX_SHADOW_PRIMARY = "0 4px 12px rgba(59, 130, 246, 0.3)";
    const QString BOX_SHADOW_DANGER = "0 4px 12px rgba(239, 68, 68, 0.3)";
    const QString BOX_SHADOW_SUCCESS = "0 4px 12px rgba(16, 185, 129, 0.3)";
}

#endif // KIENUTBAM_H
