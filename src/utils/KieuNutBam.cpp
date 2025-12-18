#include "utils/KieuNutBam.h"
#include <QGraphicsDropShadowEffect>

QPushButton* TaoNutBam::taoNut(
    const QString &chuDe,
    KieuNut kieuNut,
    KichThuocNut kichThuoc,
    const QString &iconPath,
    ViTriIcon viTriIcon
) {
    QPushButton *nut = new QPushButton();
    
    // Set text và icon theo vị trí
    if (viTriIcon == ICON_DOC_LAP) {
        // Chỉ có icon
        if (!iconPath.isEmpty()) {
            nut->setIcon(QIcon(iconPath));
            nut->setIconSize(QSize(20, 20));
        }
    } else if (viTriIcon == ICON_TRAI && !iconPath.isEmpty()) {
        nut->setIcon(QIcon(iconPath));
        nut->setIconSize(QSize(18, 18));
        nut->setText(chuDe);
    } else if (viTriIcon == ICON_PHAI && !iconPath.isEmpty()) {
        // Qt doesn't support right icon natively, use text workaround
        nut->setText(chuDe + "  →");
    } else {
        nut->setText(chuDe);
    }
    
    // Apply style theo kiểu nút
    QString style;
    switch (kieuNut) {
        case NUT_PRIMARY:
            style = layStylePrimary(kichThuoc);
            break;
        case NUT_SECONDARY:
            style = layStyleSecondary(kichThuoc);
            break;
        case NUT_DANGER:
            style = layStyleDanger(kichThuoc);
            break;
        case NUT_SUCCESS:
            style = layStyleSuccess(kichThuoc);
            break;
        case NUT_GHOST:
            style = layStyleGhost(kichThuoc);
            break;
        case NUT_WARNING:
            style = layStyleWarning(kichThuoc);
            break;
    }
    
    nut->setStyleSheet(style);
    nut->setCursor(Qt::PointingHandCursor);
    
    // Add shadow effect cho Primary, Danger, Success
    if (kieuNut == NUT_PRIMARY || kieuNut == NUT_DANGER || kieuNut == NUT_SUCCESS) {
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
        shadow->setBlurRadius(12);
        shadow->setXOffset(0);
        shadow->setYOffset(3);
        
        QColor shadowColor;
        if (kieuNut == NUT_PRIMARY) {
            shadowColor = QColor(59, 130, 246, 80); // Blue
        } else if (kieuNut == NUT_DANGER) {
            shadowColor = QColor(239, 68, 68, 80); // Red
        } else {
            shadowColor = QColor(16, 185, 129, 80); // Green
        }
        
        shadow->setColor(shadowColor);
        nut->setGraphicsEffect(shadow);
    }
    
    return nut;
}

QPushButton* TaoNutBam::taoNutChinh(const QString &chuDe, const QString &iconPath) {
    return taoNut(chuDe, NUT_PRIMARY, NUT_TRUNG_BINH, iconPath, 
                  iconPath.isEmpty() ? KHONG_ICON : ICON_TRAI);
}

QPushButton* TaoNutBam::taoNutPhu(const QString &chuDe, const QString &iconPath) {
    return taoNut(chuDe, NUT_SECONDARY, NUT_TRUNG_BINH, iconPath,
                  iconPath.isEmpty() ? KHONG_ICON : ICON_TRAI);
}

QPushButton* TaoNutBam::taoNutNguyHiem(const QString &chuDe, const QString &iconPath) {
    return taoNut(chuDe, NUT_DANGER, NUT_TRUNG_BINH, iconPath,
                  iconPath.isEmpty() ? KHONG_ICON : ICON_TRAI);
}

QPushButton* TaoNutBam::taoNutThanhCong(const QString &chuDe, const QString &iconPath) {
    return taoNut(chuDe, NUT_SUCCESS, NUT_TRUNG_BINH, iconPath,
                  iconPath.isEmpty() ? KHONG_ICON : ICON_TRAI);
}

QPushButton* TaoNutBam::taoNutNhe(const QString &chuDe, const QString &iconPath) {
    return taoNut(chuDe, NUT_GHOST, NUT_TRUNG_BINH, iconPath,
                  iconPath.isEmpty() ? KHONG_ICON : ICON_TRAI);
}

// ==================== STYLESHEETS ====================

QString TaoNutBam::layStylePrimary(KichThuocNut kichThuoc) {
    return QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #3B82F6, stop:0.5 #2563EB, stop:1 #1D4ED8);"
        "    color: white;"
        "    border: none;"
        "    border-radius: %1;"
        "    padding: %2;"
        "    font-weight: 700;"
        "    font-size: %3;"
        "    min-height: %4;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #2563EB, stop:0.5 #1D4ED8, stop:1 #1E40AF);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #1E40AF, stop:0.5 #1E3A8A, stop:1 #1E3A8A);"
        "    padding-top: %5;"
        "}"
        "QPushButton:disabled {"
        "    background: #9CA3AF;"
        "    color: #D1D5DB;"
        "}"
    )
    .arg(kichThuoc == NUT_LON ? "12px" : kichThuoc == NUT_TRUNG_BINH ? "10px" : "8px")
    .arg(layPadding(kichThuoc))
    .arg(layFontSize(kichThuoc))
    .arg(layChieuCao(kichThuoc))
    .arg(kichThuoc == NUT_LON ? "22px" : kichThuoc == NUT_TRUNG_BINH ? "18px" : "15px");
}

QString TaoNutBam::layStyleSecondary(KichThuocNut kichThuoc) {
    return QString(
        "QPushButton {"
        "    background: transparent;"
        "    color: #F1F5F9;"
        "    border: 2px solid #475569;"
        "    border-radius: %1;"
        "    padding: %2;"
        "    font-weight: 600;"
        "    font-size: %3;"
        "    min-height: %4;"
        "}"
        "QPushButton:hover {"
        "    background: rgba(71, 85, 105, 0.3);"
        "    border-color: #64748B;"
        "    color: #FFFFFF;"
        "}"
        "QPushButton:pressed {"
        "    background: rgba(71, 85, 105, 0.5);"
        "    border-color: #94A3B8;"
        "}"
        "QPushButton:disabled {"
        "    color: #64748B;"
        "    border-color: #334155;"
        "}"
    )
    .arg(kichThuoc == NUT_LON ? "12px" : kichThuoc == NUT_TRUNG_BINH ? "10px" : "8px")
    .arg(layPadding(kichThuoc))
    .arg(layFontSize(kichThuoc))
    .arg(layChieuCao(kichThuoc));
}

QString TaoNutBam::layStyleDanger(KichThuocNut kichThuoc) {
    return QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #EF4444, stop:0.5 #DC2626, stop:1 #B91C1C);"
        "    color: white;"
        "    border: none;"
        "    border-radius: %1;"
        "    padding: %2;"
        "    font-weight: 700;"
        "    font-size: %3;"
        "    min-height: %4;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #DC2626, stop:0.5 #B91C1C, stop:1 #991B1B);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #B91C1C, stop:0.5 #991B1B, stop:1 #7F1D1D);"
        "}"
        "QPushButton:disabled {"
        "    background: #9CA3AF;"
        "    color: #D1D5DB;"
        "}"
    )
    .arg(kichThuoc == NUT_LON ? "12px" : kichThuoc == NUT_TRUNG_BINH ? "10px" : "8px")
    .arg(layPadding(kichThuoc))
    .arg(layFontSize(kichThuoc))
    .arg(layChieuCao(kichThuoc));
}

QString TaoNutBam::layStyleSuccess(KichThuocNut kichThuoc) {
    return QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #10B981, stop:0.5 #059669, stop:1 #047857);"
        "    color: white;"
        "    border: none;"
        "    border-radius: %1;"
        "    padding: %2;"
        "    font-weight: 700;"
        "    font-size: %3;"
        "    min-height: %4;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #059669, stop:0.5 #047857, stop:1 #065F46);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #047857, stop:0.5 #065F46, stop:1 #064E3B);"
        "}"
        "QPushButton:disabled {"
        "    background: #9CA3AF;"
        "    color: #D1D5DB;"
        "}"
    )
    .arg(kichThuoc == NUT_LON ? "12px" : kichThuoc == NUT_TRUNG_BINH ? "10px" : "8px")
    .arg(layPadding(kichThuoc))
    .arg(layFontSize(kichThuoc))
    .arg(layChieuCao(kichThuoc));
}

QString TaoNutBam::layStyleGhost(KichThuocNut kichThuoc) {
    return QString(
        "QPushButton {"
        "    background: transparent;"
        "    color: #94A3B8;"
        "    border: none;"
        "    border-radius: %1;"
        "    padding: %2;"
        "    font-weight: 600;"
        "    font-size: %3;"
        "    min-height: %4;"
        "}"
        "QPushButton:hover {"
        "    background: rgba(148, 163, 184, 0.1);"
        "    color: #CBD5E1;"
        "}"
        "QPushButton:pressed {"
        "    background: rgba(148, 163, 184, 0.2);"
        "    color: #F1F5F9;"
        "}"
        "QPushButton:disabled {"
        "    color: #475569;"
        "}"
    )
    .arg(kichThuoc == NUT_LON ? "12px" : kichThuoc == NUT_TRUNG_BINH ? "10px" : "8px")
    .arg(layPadding(kichThuoc))
    .arg(layFontSize(kichThuoc))
    .arg(layChieuCao(kichThuoc));
}

QString TaoNutBam::layStyleWarning(KichThuocNut kichThuoc) {
    return QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #F59E0B, stop:0.5 #D97706, stop:1 #B45309);"
        "    color: white;"
        "    border: none;"
        "    border-radius: %1;"
        "    padding: %2;"
        "    font-weight: 700;"
        "    font-size: %3;"
        "    min-height: %4;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #D97706, stop:0.5 #B45309, stop:1 #92400E);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #B45309, stop:0.5 #92400E, stop:1 #78350F);"
        "}"
        "QPushButton:disabled {"
        "    background: #9CA3AF;"
        "    color: #D1D5DB;"
        "}"
    )
    .arg(kichThuoc == NUT_LON ? "12px" : kichThuoc == NUT_TRUNG_BINH ? "10px" : "8px")
    .arg(layPadding(kichThuoc))
    .arg(layFontSize(kichThuoc))
    .arg(layChieuCao(kichThuoc));
}

// ==================== HELPER FUNCTIONS ====================

QString TaoNutBam::layChieuCao(KichThuocNut kichThuoc) {
    switch (kichThuoc) {
        case NUT_LON: return "50px";
        case NUT_TRUNG_BINH: return "42px";
        case NUT_NHO: return "36px";
        default: return "42px";
    }
}

QString TaoNutBam::layFontSize(KichThuocNut kichThuoc) {
    switch (kichThuoc) {
        case NUT_LON: return "16px";
        case NUT_TRUNG_BINH: return "15px";
        case NUT_NHO: return "14px";
        default: return "15px";
    }
}

QString TaoNutBam::layPadding(KichThuocNut kichThuoc) {
    switch (kichThuoc) {
        case NUT_LON: return "0 32px";
        case NUT_TRUNG_BINH: return "0 24px";
        case NUT_NHO: return "0 16px";
        default: return "0 24px";
    }
}
