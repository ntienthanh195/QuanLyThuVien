#include "views/BadgeStyledItemDelegate.h"
#include <QPainter>
#include <QApplication>
#include <QStyleOptionViewItem>

BadgeStyledItemDelegate::BadgeStyledItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void BadgeStyledItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    // Kiểm tra xem đây có phải là cột trạng thái không
    if (index.column() == 7) { // Giả sử cột 7 là cột trạng thái
        painter->save();
        
        // Vẽ background nếu được chọn hoặc hover - Deep Dark Theme
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, QColor(54, 153, 255, 38)); // #3699FF với alpha 0.15
        } else if (option.state & QStyle::State_MouseOver) {
            painter->fillRect(option.rect, QColor(54, 153, 255, 20)); // Hover lighter
        } else {
            // Alternating row colors for deep dark theme
            if (index.row() % 2 == 0) {
                painter->fillRect(option.rect, QColor("#1E1E2D"));
            } else {
                painter->fillRect(option.rect, QColor("#232334"));
            }
        }
        
        // Lấy giá trị trạng thái
        int trangThaiInt = index.data(Qt::UserRole).toInt();
        TrangThaiSach trangThai = static_cast<TrangThaiSach>(trangThaiInt);
        
        // Vẽ badge nhỏ gọn hơn ở giữa cell
        QRect badgeRect = option.rect;
        badgeRect.setWidth(90);  // Giảm từ 100 xuống 90
        badgeRect.moveCenter(option.rect.center());
        badgeRect.setHeight(26); // Tăng từ 24 lên 26 để dễ đọc hơn
        
        veBadgeTrangThai(painter, badgeRect, trangThai);
        
        painter->restore();
    } else {
        // Các cột khác dùng paint mặc định
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize BadgeStyledItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    if (index.column() == 7) {
        size.setHeight(qMax(32, size.height()));
    }
    return size;
}

void BadgeStyledItemDelegate::veBadgeTrangThai(QPainter *painter, const QRect &rect,
                                                TrangThaiSach trangThai) const
{
    painter->setRenderHint(QPainter::Antialiasing);
    
    QString text;
    QColor bgColor, textColor, borderColor;
    
    switch (trangThai) {
        case CHO_MUON_DUOC:
            text = "● Có sẵn";      // Thêm dot icon
            bgColor = QColor("#D1FAE5");  // Xanh lá nhạt
            textColor = QColor("#15803D"); // Xanh lá đậm
            borderColor = QColor("#BBF7D0");
            break;
        case DANG_MUON:
            text = "● Đang mượn";   // Thêm dot icon
            bgColor = QColor("#FEF3C7");  // Vàng nhạt
            textColor = QColor("#B45309"); // Vàng đậm
            borderColor = QColor("#FDE68A");
            break;
        case THANH_LY:
            text = "● Thanh lý";    // Thêm dot icon
            bgColor = QColor("#F1F5F9");  // Xám nhạt
            textColor = QColor("#475569"); // Xám đậm
            borderColor = QColor("#E2E8F0");
            break;
    }
    
    // Vẽ nền badge với bo góc nhỏ hơn và border
    painter->setPen(QPen(borderColor, 1));
    painter->setBrush(bgColor);
    painter->drawRoundedRect(rect, 6, 6); // Giảm từ 12 xuống 6 để badge gọn hơn
    
    // Vẽ text với font nhỏ hơn
    painter->setPen(textColor);
    QFont font = painter->font();
    font.setWeight(QFont::DemiBold);
    font.setPointSize(9);
    painter->setFont(font);
    painter->drawText(rect, Qt::AlignCenter, text);
}
