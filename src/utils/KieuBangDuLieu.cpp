#include "utils/KieuBangDuLieu.h"
#include <QApplication>
#include <QPainterPath>

// ==================== TaoBangDuLieu Implementation ====================

QTableView* TaoBangDuLieu::taoBang(KieuBang kieuBang) {
    QTableView *table = new QTableView();
    
    // Cấu hình cơ sở
    cauHinhCoSo(table);
    
    // Apply style theo kiểu
    QString style;
    switch (kieuBang) {
        case BANG_HIEN_DAI:
            style = layStyleBangHienDai();
            break;
        case BANG_SANG:
            style = layStyleBangSang();
            break;
        case BANG_COMPACT:
            style = layStyleBangCompact();
            break;
    }
    
    table->setStyleSheet(style);
    
    // Setup header
    apDungStyleHeader(table->horizontalHeader(), kieuBang);
    
    // Set custom delegate cho zebra striping và hover
    DelegateHangBang *delegate = new DelegateHangBang(table);
    delegate->setKieuBang(kieuBang);
    table->setItemDelegate(delegate);
    
    return table;
}

void TaoBangDuLieu::cauHinhCoSo(QTableView *table) {
    // Selection
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // Alternating row colors (sẽ custom bằng delegate)
    table->setAlternatingRowColors(false);
    
    // Grid
    table->setShowGrid(false);
    
    // Scroll
    table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    
    // Sorting
    table->setSortingEnabled(true);
    
    // Disable edit
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // Mouse tracking cho hover effect
    table->setMouseTracking(true);
    
    // Frame
    table->setFrameShape(QFrame::NoFrame);
}

void TaoBangDuLieu::apDungStyleHeader(QHeaderView *header, KieuBang kieuBang) {
    if (!header) return;
    
    QString style;
    switch (kieuBang) {
        case BANG_HIEN_DAI:
        case BANG_COMPACT:
            style = layStyleHeaderHienDai();
            break;
        case BANG_SANG:
            style = layStyleHeaderSang();
            break;
    }
    
    header->setStyleSheet(style);
    header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    header->setMinimumHeight(StyleBang::CHIEU_CAO_HEADER);
    header->setSectionsClickable(true);
    header->setHighlightSections(false);
    header->setStretchLastSection(true);
}

QString TaoBangDuLieu::layStyleBangHienDai() {
    using namespace StyleBang::HienDai;
    
    return QString(
        "QTableView {"
        "    background: %1;"
        "    color: %2;"
        "    border: 2px solid %3;"
        "    border-radius: 12px;"
        "    padding: 0px;"
        "    outline: none;"
        "}"
        "QTableView::item {"
        "    padding: 12px;"
        "    border: none;"
        "    border-bottom: 1px solid %3;"
        "}"
        "QTableView::item:selected {"
        "    background: %4;"
        "    color: %2;"
        "    border-left: 3px solid %5;"
        "}"
        "QTableView::item:hover {"
        "    background: %6;"
        "}"
        "QScrollBar:vertical {"
        "    background: #1E293B;"
        "    width: 12px;"
        "    border-radius: 6px;"
        "    margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #475569;"
        "    border-radius: 6px;"
        "    min-height: 30px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: #64748B;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}"
        "QScrollBar:horizontal {"
        "    background: #1E293B;"
        "    height: 12px;"
        "    border-radius: 6px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "    background: #475569;"
        "    border-radius: 6px;"
        "    min-width: 30px;"
        "}"
        "QScrollBar::handle:horizontal:hover {"
        "    background: #64748B;"
        "}"
    )
    .arg(MAU_NEN_CHAN)
    .arg(MAU_CHU)
    .arg(MAU_BORDER)
    .arg(MAU_CHON)
    .arg(MAU_VIEN_CHON)
    .arg(MAU_HOVER);
}

QString TaoBangDuLieu::layStyleBangSang() {
    using namespace StyleBang::Sang;
    
    return QString(
        "QTableView {"
        "    background: %1;"
        "    color: %2;"
        "    border: 2px solid %3;"
        "    border-radius: 12px;"
        "    padding: 0px;"
        "    outline: none;"
        "}"
        "QTableView::item {"
        "    padding: 12px;"
        "    border: none;"
        "    border-bottom: 1px solid %3;"
        "}"
        "QTableView::item:selected {"
        "    background: %4;"
        "    color: %2;"
        "    border-left: 3px solid %5;"
        "}"
        "QTableView::item:hover {"
        "    background: %6;"
        "}"
    )
    .arg(MAU_NEN_CHAN)
    .arg(MAU_CHU)
    .arg(MAU_BORDER)
    .arg(MAU_CHON)
    .arg(MAU_VIEN_CHON)
    .arg(MAU_HOVER);
}

QString TaoBangDuLieu::layStyleBangCompact() {
    // Same as modern but smaller padding
    return layStyleBangHienDai();
}

QString TaoBangDuLieu::layStyleHeaderHienDai() {
    using namespace StyleBang::HienDai;
    
    return QString(
        "QHeaderView::section {"
        "    background: %1;"
        "    color: %2;"
        "    padding: 12px 16px;"
        "    border: none;"
        "    border-bottom: 2px solid %3;"
        "    border-right: 1px solid %4;"
        "    font-weight: 700;"
        "    font-size: 14px;"
        "    text-align: left;"
        "}"
        "QHeaderView::section:hover {"
        "    background: #3F4C5F;"
        "}"
        "QHeaderView::section:first {"
        "    border-top-left-radius: 10px;"
        "}"
        "QHeaderView::section:last {"
        "    border-top-right-radius: 10px;"
        "    border-right: none;"
        "}"
        "QHeaderView::down-arrow {"
        "    image: url(:/icons/arrow-down.png);"
        "    width: 12px;"
        "    height: 12px;"
        "}"
        "QHeaderView::up-arrow {"
        "    image: url(:/icons/arrow-up.png);"
        "    width: 12px;"
        "    height: 12px;"
        "}"
    )
    .arg(MAU_HEADER_BG)
    .arg(MAU_HEADER_TEXT)
    .arg("#60A5FA")
    .arg(MAU_BORDER);
}

QString TaoBangDuLieu::layStyleHeaderSang() {
    using namespace StyleBang::Sang;
    
    return QString(
        "QHeaderView::section {"
        "    background: %1;"
        "    color: %2;"
        "    padding: 12px 16px;"
        "    border: none;"
        "    border-bottom: 2px solid %3;"
        "    border-right: 1px solid %4;"
        "    font-weight: 700;"
        "    font-size: 14px;"
        "}"
        "QHeaderView::section:hover {"
        "    background: #E2E8F0;"
        "}"
    )
    .arg(MAU_HEADER_BG)
    .arg(MAU_HEADER_TEXT)
    .arg("#3B82F6")
    .arg(MAU_BORDER);
}

// ==================== DelegateHangBang Implementation ====================

DelegateHangBang::DelegateHangBang(QObject *parent)
    : QStyledItemDelegate(parent)
    , kieuBang(BANG_HIEN_DAI)
{
    capNhatMauSac();
}

void DelegateHangBang::setKieuBang(KieuBang kieu) {
    kieuBang = kieu;
    capNhatMauSac();
}

void DelegateHangBang::capNhatMauSac() {
    switch (kieuBang) {
        case BANG_HIEN_DAI:
        case BANG_COMPACT:
            mauNenChan = StyleBang::HienDai::MAU_NEN_CHAN;
            mauNenLe = StyleBang::HienDai::MAU_NEN_LE;
            mauHover = StyleBang::HienDai::MAU_HOVER;
            mauChon = StyleBang::HienDai::MAU_CHON;
            mauChu = StyleBang::HienDai::MAU_CHU;
            mauVien = StyleBang::HienDai::MAU_BORDER;
            break;
        case BANG_SANG:
            mauNenChan = StyleBang::Sang::MAU_NEN_CHAN;
            mauNenLe = StyleBang::Sang::MAU_NEN_LE;
            mauHover = StyleBang::Sang::MAU_HOVER;
            mauChon = StyleBang::Sang::MAU_CHON;
            mauChu = StyleBang::Sang::MAU_CHU;
            mauVien = StyleBang::Sang::MAU_BORDER;
            break;
    }
}

void DelegateHangBang::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const {
    painter->save();
    
    // Vẽ background với zebra striping
    QColor bgColor;
    if (option.state & QStyle::State_Selected) {
        bgColor = QColor(mauChon);
    } else if (option.state & QStyle::State_MouseOver) {
        bgColor = QColor(mauHover);
    } else {
        bgColor = QColor(index.row() % 2 == 0 ? mauNenChan : mauNenLe);
    }
    
    painter->fillRect(option.rect, bgColor);
    
    // Vẽ viền trái cho selected row
    if (option.state & QStyle::State_Selected) {
        QRect borderRect = option.rect;
        borderRect.setWidth(3);
        painter->fillRect(borderRect, QColor("#3B82F6"));
    }
    
    // Vẽ text
    painter->setPen(QColor(mauChu));
    QRect textRect = option.rect.adjusted(
        kieuBang == BANG_COMPACT ? StyleBang::PADDING_CELL_COMPACT : StyleBang::PADDING_CELL,
        0,
        -(kieuBang == BANG_COMPACT ? StyleBang::PADDING_CELL_COMPACT : StyleBang::PADDING_CELL),
        0
    );
    
    QString text = index.data(Qt::DisplayRole).toString();
    QFont font = option.font;
    
    if (option.state & QStyle::State_Selected) {
        font.setWeight(QFont::DemiBold);
    }
    
    painter->setFont(font);
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
    
    // Vẽ viền dưới
    QColor borderColor(mauVien);
    borderColor.setAlpha(50);
    painter->setPen(QPen(borderColor, 1));
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
    
    painter->restore();
}

QSize DelegateHangBang::sizeHint(const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const {
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    
    int height = kieuBang == BANG_COMPACT ? 
                 StyleBang::CHIEU_CAO_HANG_COMPACT : 
                 StyleBang::CHIEU_CAO_HANG;
    
    size.setHeight(height);
    return size;
}
