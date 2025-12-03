#include "views/dialogs/ManHinhDemoBang.h"
#include "utils/KieuBangDuLieu.h"
#include "utils/KieuNutBam.h"
#include <QHeaderView>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>

ManHinhDemoBang::ManHinhDemoBang(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("🎨 Demo Table Styling - Phase 1.3");
    resize(1000, 700);
    
    taoCacWidget();
    taoLayout();
    taoKetNoi();
    taiDuLieuMau();
}

void ManHinhDemoBang::taoCacWidget() {
    // Tạo table với kiểu Hiện đại
    bangChinh = TaoBangDuLieu::taoBang(BANG_HIEN_DAI);
    
    // Model
    model = new QStandardItemModel(0, 7, this);
    model->setHorizontalHeaderLabels({
        "Mã sách", "Tên sách", "Tác giả", "Thể loại", 
        "Năm XB", "Số lượng", "Trạng thái"
    });
    bangChinh->setModel(model);
    
    // Thiết lập độ rộng cột
    bangChinh->setColumnWidth(0, 100);  // Mã sách
    bangChinh->setColumnWidth(1, 250);  // Tên sách
    bangChinh->setColumnWidth(2, 150);  // Tác giả
    bangChinh->setColumnWidth(3, 120);  // Thể loại
    bangChinh->setColumnWidth(4, 80);   // Năm XB
    bangChinh->setColumnWidth(5, 100);  // Số lượng
    bangChinh->setColumnWidth(6, 120);  // Trạng thái
    
    // Combo box để chọn kiểu bảng
    comboKieuBang = new QComboBox();
    comboKieuBang->addItem("🌙 Hiện đại (Dark)", BANG_HIEN_DAI);
    comboKieuBang->addItem("☀️ Sáng (Light)", BANG_SANG);
    comboKieuBang->addItem("📐 Compact", BANG_COMPACT);
    comboKieuBang->setStyleSheet(
        "QComboBox {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #3B82F6, stop:0.5 #2563EB, stop:1 #1D4ED8);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 10px 16px;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "    min-width: 200px;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "    width: 30px;"
        "}"
        "QComboBox::down-arrow {"
        "    image: none;"
        "    border-left: 5px solid transparent;"
        "    border-right: 5px solid transparent;"
        "    border-top: 6px solid white;"
        "    margin-right: 10px;"
        "}"
        "QComboBox:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #2563EB, stop:0.5 #1D4ED8, stop:1 #1E40AF);"
        "}"
        "QComboBox QAbstractItemView {"
        "    background: #1E293B;"
        "    color: white;"
        "    border: 2px solid #3B82F6;"
        "    border-radius: 8px;"
        "    selection-background-color: #3B82F6;"
        "    padding: 5px;"
        "}"
        "QComboBox QAbstractItemView::item {"
        "    padding: 8px 12px;"
        "    border-radius: 4px;"
        "}"
        "QComboBox QAbstractItemView::item:hover {"
        "    background: rgba(59, 130, 246, 0.3);"
        "}"
    );
    
    // Label thông tin
    labelThongTin = new QLabel("💡 Hover vào dòng để xem hiệu ứng, click để chọn dòng");
    labelThongTin->setStyleSheet(
        "QLabel {"
        "    color: #94A3B8;"
        "    font-size: 14px;"
        "    padding: 8px 12px;"
        "    background: rgba(148, 163, 184, 0.1);"
        "    border-radius: 6px;"
        "    border-left: 3px solid #60A5FA;"
        "}"
    );
}

void ManHinhDemoBang::taoLayout() {
    QVBoxLayout *layoutChinh = new QVBoxLayout(this);
    layoutChinh->setSpacing(20);
    layoutChinh->setContentsMargins(20, 20, 20, 20);
    
    // Header
    QLabel *tieuDe = new QLabel("📊 Table Component với Zebra Striping & Hover Effects");
    tieuDe->setStyleSheet(
        "QLabel {"
        "    font-size: 24px;"
        "    font-weight: 700;"
        "    color: white;"
        "    padding: 16px;"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #3B82F6, stop:1 #8B5CF6);"
        "    border-radius: 12px;"
        "}"
    );
    layoutChinh->addWidget(tieuDe);
    
    // Controls
    QHBoxLayout *layoutDieuKhien = new QHBoxLayout();
    
    QLabel *labelKieu = new QLabel("Kiểu bảng:");
    labelKieu->setStyleSheet("color: #E2E8F0; font-size: 14px; font-weight: 600;");
    
    layoutDieuKhien->addWidget(labelKieu);
    layoutDieuKhien->addWidget(comboKieuBang);
    layoutDieuKhien->addStretch();
    
    // Buttons
    QPushButton *btnDuplicate = TaoNutBam::taoNut("➕ Duplicate Row", KieuNut::NUT_SUCCESS, KichThuocNut::NUT_TRUNG_BINH);
    QPushButton *btnXoa = TaoNutBam::taoNut("🗑️ Delete Selected", KieuNut::NUT_DANGER, KichThuocNut::NUT_TRUNG_BINH);
    
    layoutDieuKhien->addWidget(btnDuplicate);
    layoutDieuKhien->addWidget(btnXoa);
    
    connect(btnDuplicate, &QPushButton::clicked, this, &ManHinhDemoBang::kichDuplicate);
    connect(btnXoa, &QPushButton::clicked, this, &ManHinhDemoBang::kichXoa);
    
    layoutChinh->addLayout(layoutDieuKhien);
    
    // Info label
    layoutChinh->addWidget(labelThongTin);
    
    // Table trong group box
    QGroupBox *nhomBang = new QGroupBox("Demo Data - Library Books");
    nhomBang->setStyleSheet(
        "QGroupBox {"
        "    border: 2px solid #475569;"
        "    border-radius: 12px;"
        "    margin-top: 12px;"
        "    padding-top: 12px;"
        "    background: #0F172A;"
        "    color: #F1F5F9;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 20px;"
        "    padding: 0 8px;"
        "}"
    );
    
    QVBoxLayout *layoutNhom = new QVBoxLayout(nhomBang);
    layoutNhom->setContentsMargins(0, 8, 0, 0);
    layoutNhom->addWidget(bangChinh);
    
    layoutChinh->addWidget(nhomBang, 1);
    
    // Dialog background
    setStyleSheet(
        "QDialog {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "        stop:0 #0F172A, stop:1 #1E293B);"
        "}"
    );
}

void ManHinhDemoBang::taoKetNoi() {
    connect(comboKieuBang, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ManHinhDemoBang::thayDoiKieuBang);
}

void ManHinhDemoBang::taiDuLieuMau() {
    // Data mẫu cho thư viện sách
    QVector<QStringList> duLieu = {
        {"BK001", "Đắc Nhân Tâm", "Dale Carnegie", "Kỹ năng sống", "2020", "15", "Còn"},
        {"BK002", "Nhà Giả Kim", "Paulo Coelho", "Tiểu thuyết", "2019", "8", "Còn"},
        {"BK003", "Sapiens", "Yuval Noah Harari", "Lịch sử", "2021", "12", "Còn"},
        {"BK004", "Tuổi Trẻ Đáng Giá Bao Nhiêu", "Rosie Nguyễn", "Kỹ năng sống", "2018", "0", "Hết"},
        {"BK005", "Clean Code", "Robert Martin", "Lập trình", "2022", "20", "Còn"},
        {"BK006", "Hoàng Tử Bé", "Antoine de Saint", "Thiếu nhi", "2020", "25", "Còn"},
        {"BK007", "1984", "George Orwell", "Tiểu thuyết", "2019", "3", "Sắp hết"},
        {"BK008", "Khéo Ăn Nói Sẽ Có Được Thiên Hạ", "Trác Nhã", "Kỹ năng sống", "2021", "18", "Còn"},
        {"BK009", "Thinking Fast and Slow", "Daniel Kahneman", "Tâm lý học", "2020", "10", "Còn"},
        {"BK010", "Harry Potter và Hòn Đá Phù Thủy", "J.K. Rowling", "Fantasy", "2018", "5", "Sắp hết"},
        {"BK011", "The Lean Startup", "Eric Ries", "Kinh doanh", "2022", "14", "Còn"},
        {"BK012", "Đời Ngắn Đừng Ngủ Dài", "Robin Sharma", "Kỹ năng sống", "2019", "22", "Còn"},
        {"BK013", "Atomic Habits", "James Clear", "Kỹ năng sống", "2021", "30", "Còn"},
        {"BK014", "The Subtle Art of Not Giving", "Mark Manson", "Kỹ năng sống", "2020", "7", "Còn"},
        {"BK015", "Educated", "Tara Westover", "Hồi ký", "2021", "9", "Còn"}
    };
    
    for (const auto &hang : duLieu) {
        QList<QStandardItem*> items;
        for (const QString &giaTri : hang) {
            QStandardItem *item = new QStandardItem(giaTri);
            
            // Màu cho trạng thái
            if (giaTri == "Hết") {
                item->setForeground(QColor("#EF4444")); // Red
            } else if (giaTri == "Sắp hết") {
                item->setForeground(QColor("#F59E0B")); // Orange
            } else if (giaTri == "Còn") {
                item->setForeground(QColor("#10B981")); // Green
            }
            
            items.append(item);
        }
        model->appendRow(items);
    }
}

void ManHinhDemoBang::thayDoiKieuBang(int index) {
    KieuBang kieuMoi = static_cast<KieuBang>(comboKieuBang->itemData(index).toInt());
    
    // Xóa table cũ
    QLayout *layout = bangChinh->parentWidget()->layout();
    layout->removeWidget(bangChinh);
    delete bangChinh;
    
    // Tạo table mới với kiểu mới
    bangChinh = TaoBangDuLieu::taoBang(kieuMoi);
    bangChinh->setModel(model);
    
    // Restore column widths
    bangChinh->setColumnWidth(0, 100);
    bangChinh->setColumnWidth(1, 250);
    bangChinh->setColumnWidth(2, 150);
    bangChinh->setColumnWidth(3, 120);
    bangChinh->setColumnWidth(4, 80);
    bangChinh->setColumnWidth(5, 100);
    bangChinh->setColumnWidth(6, 120);
    
    layout->addWidget(bangChinh);
}

void ManHinhDemoBang::kichDuplicate() {
    QModelIndexList selected = bangChinh->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "Thông báo", 
            "Vui lòng chọn một dòng để duplicate!");
        return;
    }
    
    int row = selected.first().row();
    QList<QStandardItem*> items;
    
    for (int col = 0; col < model->columnCount(); ++col) {
        QStandardItem *original = model->item(row, col);
        QStandardItem *duplicate = new QStandardItem(original->text());
        duplicate->setForeground(original->foreground());
        items.append(duplicate);
    }
    
    model->appendRow(items);
    
    // Scroll đến dòng mới
    bangChinh->scrollToBottom();
    
    labelThongTin->setText(QString("✅ Đã duplicate dòng %1").arg(row + 1));
    labelThongTin->setStyleSheet(
        "QLabel {"
        "    color: #10B981;"
        "    font-size: 14px;"
        "    padding: 8px 12px;"
        "    background: rgba(16, 185, 129, 0.1);"
        "    border-radius: 6px;"
        "    border-left: 3px solid #10B981;"
        "}"
    );
}

void ManHinhDemoBang::kichXoa() {
    QModelIndexList selected = bangChinh->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "Thông báo", 
            "Vui lòng chọn một dòng để xóa!");
        return;
    }
    
    int row = selected.first().row();
    QString tenSach = model->item(row, 1)->text();
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, 
        "Xác nhận xóa",
        QString("Bạn có chắc muốn xóa sách '%1'?").arg(tenSach),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        model->removeRow(row);
        
        labelThongTin->setText(QString("🗑️ Đã xóa: %1").arg(tenSach));
        labelThongTin->setStyleSheet(
            "QLabel {"
            "    color: #EF4444;"
            "    font-size: 14px;"
            "    padding: 8px 12px;"
            "    background: rgba(239, 68, 68, 0.1);"
            "    border-radius: 6px;"
            "    border-left: 3px solid #EF4444;"
            "}"
        );
    }
}
