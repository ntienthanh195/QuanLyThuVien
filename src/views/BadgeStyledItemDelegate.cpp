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
    // WidgetQuanLyDocGia uses column 4 for trạng thái; make delegate respond to that column
    if (index.column() == 4) {
        painter->save();
        
        // Vẽ background nếu được chọn hoặc hover - Deep Dark Theme
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, QColor(54, 153, 255, 38)); // #3699FF với alpha 0.15
        } else if (option.state & QStyle::State_MouseOver) {
            painter->fillRect(option.rect, QColor(54, 153, 255, 20)); // Hover lighter
        } else {
            // Alternating row colors adjusted to navy theme
            if (index.row() % 2 == 0) {
                painter->fillRect(option.rect, QColor("#071233"));
            } else {
                painter->fillRect(option.rect, QColor("#08283d"));
            }
        }
        
        // Lấy giá trị trạng thái
        int trangThaiInt = index.data(Qt::UserRole).toInt();
        TrangThaiSach trangThai = static_cast<TrangThaiSach>(trangThaiInt);
        
        // Vẽ badge gọn giữa ô với kích thước tương đồng Quản Lý Sách
        QRect badgeRect = option.rect;
        badgeRect.setWidth(84);
        badgeRect.setHeight(28);
        badgeRect.moveCenter(option.rect.center());
        
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
    if (index.column() == 4) {
        size.setHeight(qMax(36, size.height()));
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
    
    // Vẽ nền badge với bo góc vừa phải và border
    painter->setPen(QPen(borderColor, 1));
    painter->setBrush(bgColor);
    painter->drawRoundedRect(rect, 6, 6);

    // Vẽ text với font vừa phải để cân bằng không gian
    painter->setPen(textColor);
    QFont font = painter->font();
    font.setWeight(QFont::DemiBold);
    font.setPointSize(9);
    painter->setFont(font);
    painter->drawText(rect, Qt::AlignCenter, text);
}
