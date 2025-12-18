#ifndef BADGESTYLEDITEMDELEGATE_H
#define BADGESTYLEDITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include "models/DauSach.h"

// Delegate để vẽ badges trạng thái đẹp trong QTableView
class BadgeStyledItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit BadgeStyledItemDelegate(QObject *parent = nullptr);
    
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:
    void veBadgeTrangThai(QPainter *painter, const QRect &rect,
                          TrangThaiSach trangThai) const;
};

#endif // BADGESTYLEDITEMDELEGATE_H
