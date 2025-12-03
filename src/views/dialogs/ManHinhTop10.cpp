#include "views/dialogs/ManHinhTop10.h"
#include "models/QuanLySach.h"
#include "models/DocGia.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QGroupBox>
#include <QMessageBox>
#include <QTextDocument>
#include <QPrinter>
#include <QPrintDialog>
#include <algorithm>
#include <vector>

// Struct để lưu thông tin sách và số lượt mượn
struct SachVaSoLuotMuon {
    PTRDS sach;
    int soLuotMuon;
    int soLuotMuonTruoc;  // Số lượt mượn kỳ trước (để so sánh)
    int thayDoi;  // Chênh lệch so với kỳ trước
};

ManHinhTop10::ManHinhTop10(PTRDS dsDauSach[], int soLuongDauSach, QWidget *parent)
    : QDialog(parent), danhSachDauSach(dsDauSach), soLuong(soLuongDauSach) {
    setWindowTitle("Top 10 Sách Được Mượn Nhiều Nhất");
    setMinimumSize(1100, 750);
    setModal(true);
    
    thietLapGiaoDien();
    apDungPhongCach();
    capNhatBangTop10();
}

ManHinhTop10::~ManHinhTop10() {
}

void ManHinhTop10::thietLapGiaoDien() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(25, 25, 25, 25);
    
    // Tiêu đề
    nhanTieuDe = new QLabel("TOP 10 SÁCH PHỔ BIẾN NHẤT");
    nhanTieuDe->setStyleSheet(
        "font-size: 24px; "
        "font-weight: bold; "
        "color: white; "
        "padding: 20px; "
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1e3a8a, stop:1 #1e40af); "
        "border-radius: 10px;"
    );
    nhanTieuDe->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(nhanTieuDe);
    
    // ========== BỘ LỌC MỚI ==========
    QGroupBox *boxFilter = new QGroupBox("Bộ Lọc Thời Gian");
    boxFilter->setStyleSheet(
        "QGroupBox { "
        "    font-weight: bold; "
        "    border: 2px solid #1e40af; "
        "    border-radius: 8px; "
        "    margin-top: 12px; "
        "    padding-top: 10px; "
        "    background-color: #f1f5f9; "
        "} "
        "QGroupBox::title { "
        "    subcontrol-origin: margin; "
        "    left: 15px; "
        "    padding: 0 5px; "
        "    color: #1e293b; "
        "}"
    );
    
    QHBoxLayout *filterLayout = new QHBoxLayout();
    
    // ComboBox chọn khoảng thời gian
    filterLayout->addWidget(new QLabel("Khoảng:"));
    comboBoLoc = new QComboBox();
    comboBoLoc->addItems({
        "Tất cả thời gian",
        "Tháng này",
        "Tháng trước",
        "Tùy chỉnh"
    });
    comboBoLoc->setMinimumWidth(180);
    filterLayout->addWidget(comboBoLoc);
    
    filterLayout->addSpacing(20);
    
    // Date range pickers
    filterLayout->addWidget(new QLabel("Từ ngày:"));
    dateFrom = new QDateEdit(QDate::currentDate().addMonths(-1));
    dateFrom->setCalendarPopup(true);
    dateFrom->setDisplayFormat("dd/MM/yyyy");
    dateFrom->setEnabled(false);
    filterLayout->addWidget(dateFrom);
    
    filterLayout->addWidget(new QLabel("Đến:"));
    dateTo = new QDateEdit(QDate::currentDate());
    dateTo->setCalendarPopup(true);
    dateTo->setDisplayFormat("dd/MM/yyyy");
    dateTo->setEnabled(false);
    filterLayout->addWidget(dateTo);
    
    filterLayout->addSpacing(20);
    
    // Checkbox so sánh
    checkSoSanh = new QCheckBox("Hiển thị xu hướng");
    checkSoSanh->setChecked(true);
    checkSoSanh->setToolTip("So sánh với kỳ trước đó");
    filterLayout->addWidget(checkSoSanh);
    
    filterLayout->addStretch();
    boxFilter->setLayout(filterLayout);
    mainLayout->addWidget(boxFilter);
    
    // Panel thông tin
    nhanThongTin = new QLabel(
        "<b>Thống kê:</b> Xếp hạng dựa trên tổng số lượt sách đã được mượn. "
        "Bao gồm cả sách đang mượn và đã trả."
    );
    nhanThongTin->setStyleSheet(
        "color: #065f46; "
        "font-size: 12px; "
        "padding: 12px; "
        "background-color: #d1fae5; "
        "border-radius: 6px; "
        "border-left: 4px solid #059669;"
    );
    nhanThongTin->setWordWrap(true);
    mainLayout->addWidget(nhanThongTin);
    
    // Nhãn so sánh xu hướng
    nhanSoSanh = new QLabel();
    nhanSoSanh->setVisible(false);
    nhanSoSanh->setWordWrap(true);
    mainLayout->addWidget(nhanSoSanh);
    
    // Bảng Top 10 (7 cột: thêm "Xu hướng")
    bangTop10 = new QTableWidget(this);
    bangTop10->setColumnCount(7);
    bangTop10->setHorizontalHeaderLabels({
        "Hạng", "ISBN", "Tên Sách", "Tác Giả", 
        "Số Lượt", "Tỷ Lệ", "Xu Hướng"
    });
    
    bangTop10->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangTop10->setSelectionMode(QAbstractItemView::SingleSelection);
    bangTop10->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bangTop10->setAlternatingRowColors(true);
    bangTop10->verticalHeader()->setVisible(false);
    bangTop10->horizontalHeader()->setStretchLastSection(true);
    bangTop10->setShowGrid(true);
    
    // Đặt chiều rộng cột
    bangTop10->setColumnWidth(0, 70);   // Hạng
    bangTop10->setColumnWidth(1, 100);  // ISBN
    bangTop10->setColumnWidth(2, 250);  // Tên sách
    bangTop10->setColumnWidth(3, 160);  // Tác giả
    bangTop10->setColumnWidth(4, 100);  // Số lượt mượn
    bangTop10->setColumnWidth(5, 80);   // Tỷ lệ
    bangTop10->setColumnWidth(6, 140);  // Xu hướng
    
    mainLayout->addWidget(bangTop10);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    nutXuatBaoCao = new QPushButton("Xuất Báo Cáo", this);
    nutXuatBaoCao->setMinimumSize(140, 40);
    nutXuatBaoCao->setToolTip("In hoặc xuất báo cáo (Ctrl+P)");
    nutXuatBaoCao->setShortcut(QKeySequence("Ctrl+P"));
    
    nutLamMoi = new QPushButton("Làm Mới", this);
    nutLamMoi->setMinimumSize(120, 40);
    nutLamMoi->setToolTip("Cập nhật lại bảng xếp hạng (F5)");
    nutLamMoi->setShortcut(QKeySequence("F5"));
    
    nutDong = new QPushButton("Đóng", this);
    nutDong->setMinimumSize(120, 40);
    nutDong->setToolTip("Đóng cửa sổ (Esc)");
    nutDong->setShortcut(QKeySequence(Qt::Key_Escape));
    
    buttonLayout->addWidget(nutXuatBaoCao);
    buttonLayout->addWidget(nutLamMoi);
    buttonLayout->addWidget(nutDong);
    
    mainLayout->addLayout(buttonLayout);
    
    // Kết nối signals
    connect(comboBoLoc, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &ManHinhTop10::onFilterChanged);
    connect(dateFrom, &QDateEdit::dateChanged, this, &ManHinhTop10::capNhatBangTop10);
    connect(dateTo, &QDateEdit::dateChanged, this, &ManHinhTop10::capNhatBangTop10);
    connect(checkSoSanh, &QCheckBox::checkStateChanged, this, &ManHinhTop10::capNhatBangTop10);
    connect(nutLamMoi, &QPushButton::clicked, this, &ManHinhTop10::capNhatBangTop10);
    connect(nutXuatBaoCao, &QPushButton::clicked, this, &ManHinhTop10::onExportReport);
    connect(nutDong, &QPushButton::clicked, this, &QDialog::accept);
}

void ManHinhTop10::apDungPhongCach() {
    bangTop10->setStyleSheet(
        "QTableWidget { "
        "    background-color: white; "
        "    alternate-background-color: #f8f9fa; "
        "    border: 2px solid #bdc3c7; "
        "    border-radius: 5px; "
        "    gridline-color: #ecf0f1; "
        "} "
        "QTableWidget::item { "
        "    padding: 8px; "
        "} "
        "QTableWidget::item:selected { "
        "    background-color: #3498db; "
        "    color: white; "
        "} "
        "QHeaderView::section { "
        "    background-color: #34495e; "
        "    color: white; "
        "    padding: 12px; "
        "    border: none; "
        "    font-weight: bold; "
        "    font-size: 13px; "
        "}"
    );
    
    nutLamMoi->setStyleSheet(
        "QPushButton { "
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3498db, stop:1 #2980b9); "
        "    color: white; "
        "    border: none; "
        "    border-radius: 5px; "
        "    font-weight: bold; "
        "    font-size: 14px; "
        "} "
        "QPushButton:hover { "
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5dade2, stop:1 #3498db); "
        "} "
        "QPushButton:pressed { "
        "    background: #2980b9; "
        "}"
    );
    
    nutDong->setStyleSheet(
        "QPushButton { "
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #95a5a6, stop:1 #7f8c8d); "
        "    color: white; "
        "    border: none; "
        "    border-radius: 5px; "
        "    font-weight: bold; "
        "    font-size: 14px; "
        "} "
        "QPushButton:hover { "
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #b2babb, stop:1 #95a5a6); "
        "} "
        "QPushButton:pressed { "
        "    background: #7f8c8d; "
        "}"
    );
}

void ManHinhTop10::capNhatBangTop10() {
    bangTop10->setRowCount(0);
    
    if (soLuong == 0) {
        nhanThongTin->setText("Chưa có dữ liệu sách trong hệ thống");
        nhanThongTin->setStyleSheet(
            "color: #dc2626; font-weight: bold; padding: 12px; "
            "background-color: #fee2e2; border-radius: 6px; border-left: 4px solid #dc2626;"
        );
        nhanSoSanh->setVisible(false);
        return;
    }
    
    // Xác định khoảng thời gian lọc
    QDate tuNgay, denNgay;
    bool coBoLoc = false;
    
    int indexFilter = comboBoLoc->currentIndex();
    if (indexFilter == 1) {  // Tháng này
        tuNgay = QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1);
        denNgay = QDate::currentDate();
        coBoLoc = true;
    } else if (indexFilter == 2) {  // Tháng trước
        QDate firstDayThisMonth = QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1);
        denNgay = firstDayThisMonth.addDays(-1);
        tuNgay = QDate(denNgay.year(), denNgay.month(), 1);
        coBoLoc = true;
    } else if (indexFilter == 3) {  // Tùy chỉnh
        tuNgay = dateFrom->date();
        denNgay = dateTo->date();
        coBoLoc = true;
    }
    
    // Thu thập dữ liệu và tính số lượt mượn
    std::vector<SachVaSoLuotMuon> danhSach;
    int tongLuotMuon = 0;
    
    for (int i = 0; i < soLuong; i++) {
        if (danhSachDauSach[i]) {
            SachVaSoLuotMuon item;
            item.sach = danhSachDauSach[i];
            
            if (coBoLoc) {
                // Đếm lượt mượn trong khoảng thời gian
                item.soLuotMuon = demLuotMuonTheoKhoangThoiGian(danhSachDauSach[i], tuNgay, denNgay);
                
                // Nếu có so sánh xu hướng, tính kỳ trước
                if (checkSoSanh->isChecked()) {
                    int soNgay = tuNgay.daysTo(denNgay) + 1;
                    QDate tuNgayTruoc = tuNgay.addDays(-soNgay);
                    QDate denNgayTruoc = tuNgay.addDays(-1);
                    item.soLuotMuonTruoc = demLuotMuonTheoKhoangThoiGian(danhSachDauSach[i], tuNgayTruoc, denNgayTruoc);
                    item.thayDoi = item.soLuotMuon - item.soLuotMuonTruoc;
                } else {
                    item.soLuotMuonTruoc = 0;
                    item.thayDoi = 0;
                }
            } else {
                // Tất cả thời gian
                item.soLuotMuon = danhSachDauSach[i]->soLuotMuon;
                item.soLuotMuonTruoc = 0;
                item.thayDoi = 0;
            }
            
            if (item.soLuotMuon > 0) {
                danhSach.push_back(item);
                tongLuotMuon += item.soLuotMuon;
            }
        }
    }
    
    if (danhSach.empty()) {
        nhanThongTin->setText("Chưa có sách nào được mượn trong khoảng thời gian này");
        nhanThongTin->setStyleSheet(
            "color: #64748b; font-weight: bold; padding: 12px; "
            "background-color: #f1f5f9; border-radius: 6px; border-left: 4px solid #94a3b8;"
        );
        nhanSoSanh->setVisible(false);
        return;
    }
    
    // Sắp xếp theo số lượt mượn giảm dần
    std::sort(danhSach.begin(), danhSach.end(), 
        [](const SachVaSoLuotMuon& a, const SachVaSoLuotMuon& b) {
            return a.soLuotMuon > b.soLuotMuon;
        });
    
    // Lấy top 10
    int soLuongHienThi = std::min(10, (int)danhSach.size());
    bangTop10->setRowCount(soLuongHienThi);
    
    // Màu cho huy chương
    QStringList mauHang = {"#FFD700", "#C0C0C0", "#CD7F32"};  // Vàng, Bạc, Đồng
    QStringList iconHang = {"🥇", "🥈", "🥉"};
    
    for (int i = 0; i < soLuongHienThi; i++) {
        const SachVaSoLuotMuon& item = danhSach[i];
        
        // Hạng
        QTableWidgetItem* itemHang = new QTableWidgetItem();
        if (i < 3) {
            itemHang->setText(iconHang[i] + " " + QString::number(i + 1));
            QFont fontHang = itemHang->font();
            fontHang.setBold(true);
            itemHang->setFont(fontHang);
            itemHang->setBackground(QColor(mauHang[i]));
        } else {
            itemHang->setText(QString::number(i + 1));
        }
        itemHang->setTextAlignment(Qt::AlignCenter);
        bangTop10->setItem(i, 0, itemHang);
        
        // ISBN
        QTableWidgetItem* itemISBN = new QTableWidgetItem(
            QString::fromStdString(item.sach->ISBN));
        itemISBN->setTextAlignment(Qt::AlignCenter);
        bangTop10->setItem(i, 1, itemISBN);
        
        // Tên sách
        QTableWidgetItem* itemTen = new QTableWidgetItem(
            QString::fromStdString(item.sach->tenSach));
        QFont fontTen = itemTen->font();
        if (i < 3) fontTen.setBold(true);
        itemTen->setFont(fontTen);
        bangTop10->setItem(i, 2, itemTen);
        
        // Tác giả
        QTableWidgetItem* itemTacGia = new QTableWidgetItem(
            QString::fromStdString(item.sach->tacGia));
        bangTop10->setItem(i, 3, itemTacGia);
        
        // Số lượt mượn
        QTableWidgetItem* itemSoLuot = new QTableWidgetItem(
            QString::number(item.soLuotMuon));
        itemSoLuot->setTextAlignment(Qt::AlignCenter);
        QFont fontSoLuot = itemSoLuot->font();
        fontSoLuot.setBold(true);
        itemSoLuot->setFont(fontSoLuot);
        itemSoLuot->setForeground(QColor("#27ae60"));
        bangTop10->setItem(i, 4, itemSoLuot);
        
        // Tỷ lệ
        double tyLe = (tongLuotMuon > 0) ? 
            (item.soLuotMuon * 100.0 / tongLuotMuon) : 0;
        QTableWidgetItem* itemTyLe = new QTableWidgetItem(
            QString::number(tyLe, 'f', 1) + "%");
        itemTyLe->setTextAlignment(Qt::AlignCenter);
        bangTop10->setItem(i, 5, itemTyLe);
        
        // Xu hướng (cột mới)
        QTableWidgetItem* itemXuHuong = new QTableWidgetItem();
        if (checkSoSanh->isChecked() && coBoLoc) {
            if (item.thayDoi > 0) {
                itemXuHuong->setText(QString("🔼 +%1").arg(item.thayDoi));
                itemXuHuong->setForeground(QColor("#27ae60"));
            } else if (item.thayDoi < 0) {
                itemXuHuong->setText(QString("🔽 %1").arg(item.thayDoi));
                itemXuHuong->setForeground(QColor("#e74c3c"));
            } else {
                itemXuHuong->setText("➖ 0");
                itemXuHuong->setForeground(QColor("#95a5a6"));
            }
            QFont fontXH = itemXuHuong->font();
            fontXH.setBold(true);
            itemXuHuong->setFont(fontXH);
        } else {
            itemXuHuong->setText("-");
        }
        itemXuHuong->setTextAlignment(Qt::AlignCenter);
        bangTop10->setItem(i, 6, itemXuHuong);
        
        // Đặt chiều cao hàng
        bangTop10->setRowHeight(i, 45);
    }
    
    // Cập nhật thông tin
    QString khoangTG = coBoLoc ? 
        QString("từ %1 đến %2").arg(tuNgay.toString("dd/MM/yyyy")).arg(denNgay.toString("dd/MM/yyyy")) :
        "tất cả thời gian";
    
    nhanThongTin->setText(
        QString("Khoảng: <b>%1</b> | %2 đầu sách | Tổng lượt: %3 | Hiển thị top %4")
            .arg(khoangTG)
            .arg(danhSach.size())
            .arg(tongLuotMuon)
            .arg(soLuongHienThi)
    );
    nhanThongTin->setStyleSheet(
        "color: #065f46; font-weight: bold; padding: 12px; "
        "background-color: #d1fae5; border-radius: 6px; border-left: 4px solid #059669;"
    );
    
    // Hiển thị so sánh xu hướng
    if (checkSoSanh->isChecked() && coBoLoc) {
        hienThiSoSanhXuHuong();
    } else {
        nhanSoSanh->setVisible(false);
    }
}

// ==================== NEW METHODS FOR ADVANCED FILTERING ====================

void ManHinhTop10::onFilterChanged() {
    int index = comboBoLoc->currentIndex();
    
    // Enable/disable date pickers based on selection
    bool enableDates = (index == 3);  // Tùy chỉnh
    dateFrom->setEnabled(enableDates);
    dateTo->setEnabled(enableDates);
    
    // Auto-set dates for predefined ranges
    if (index == 1) {  // Tháng này
        QDate firstDay = QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1);
        dateFrom->setDate(firstDay);
        dateTo->setDate(QDate::currentDate());
    } else if (index == 2) {  // Tháng trước
        QDate firstDayThisMonth = QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1);
        QDate lastDayLastMonth = firstDayThisMonth.addDays(-1);
        QDate firstDayLastMonth = QDate(lastDayLastMonth.year(), lastDayLastMonth.month(), 1);
        dateFrom->setDate(firstDayLastMonth);
        dateTo->setDate(lastDayLastMonth);
    }
    
    capNhatBangTop10();
}

int ManHinhTop10::demLuotMuonTheoKhoangThoiGian(PTRDS dauSach, QDate tuNgay, QDate denNgay) {
    if (!dauSach) return 0;
    
    int count = 0;
    extern PTRDG rootDocGia;
    
    // Duyệt tất cả độc giả để đếm lượt mượn
    std::function<void(PTRDG)> duyetAVL = [&](PTRDG node) {
        if (!node) return;
        
        duyetAVL(node->left);
        
        // Duyệt danh sách mượn trả của độc giả
        MUONTRA mt = node->data.dsmt;
        while (mt) {
            // Kiểm tra xem có mượn sách của đầu sách này không
            if (mt->data.banSaoSach) {
                // Tìm xem mã sách có thuộc đầu sách này không
                PTRDMS dms = dauSach->dms;
                while (dms) {
                    if (dms->maSach == mt->data.banSaoSach->maSach) {
                        // Parse ngày mượn
                        QString ngayMuonStr = QString::fromStdString(mt->data.NgayMuon);
                        QDate ngayMuon = QDate::fromString(ngayMuonStr, "dd/MM/yyyy");
                        
                        if (ngayMuon.isValid() && ngayMuon >= tuNgay && ngayMuon <= denNgay) {
                            count++;
                        }
                        break;
                    }
                    dms = dms->next;
                }
            }
            mt = mt->next;
        }
        
        duyetAVL(node->right);
    };
    
    duyetAVL(rootDocGia);
    return count;
}

void ManHinhTop10::hienThiSoSanhXuHuong() {
    // Tính tổng xu hướng
    int tongTang = 0, tongGiam = 0, tongKhongDoi = 0;
    
    for (int row = 0; row < bangTop10->rowCount(); row++) {
        QString xuHuong = bangTop10->item(row, 6)->text();
        if (xuHuong.contains("🔼")) tongTang++;
        else if (xuHuong.contains("🔽")) tongGiam++;
        else tongKhongDoi++;
    }
    
    QString thongBao;
    QString mauNen, mauVien;
    
    if (tongTang > tongGiam) {
        thongBao = QString("<b>Xu hướng tích cực:</b> Có %1 sách tăng, %2 sách giảm, %3 sách không đổi so với kỳ trước")
                   .arg(tongTang).arg(tongGiam).arg(tongKhongDoi);
        mauNen = "#d1fae5";
        mauVien = "#059669";
    } else if (tongGiam > tongTang) {
        thongBao = QString("<b>Xu hướng giảm:</b> Có %1 sách giảm, %2 sách tăng, %3 sách không đổi so với kỳ trước")
                   .arg(tongGiam).arg(tongTang).arg(tongKhongDoi);
        mauNen = "#fee2e2";
        mauVien = "#dc2626";
    } else {
        thongBao = QString("<b>Xu hướng ổn định:</b> Có %1 sách tăng, %2 sách giảm, %3 sách không đổi")
                   .arg(tongTang).arg(tongGiam).arg(tongKhongDoi);
        mauNen = "#fef3c7";
        mauVien = "#f59e0b";
    }
    
    nhanSoSanh->setText(thongBao);
    nhanSoSanh->setStyleSheet(QString(
        "color: %2; font-weight: bold; padding: 12px; "
        "background-color: %1; border-radius: 6px; border-left: 4px solid %2;")
        .arg(mauNen).arg(mauVien));
    nhanSoSanh->setVisible(true);
}

void ManHinhTop10::onExportReport() {
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize::A4);
    
    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() != QDialog::Accepted) {
        return;
    }
    
    QString html = "<html><head><style>"
                   "body { font-family: Arial, sans-serif; margin: 30px; }"
                   ".header { text-align: center; border-bottom: 3px solid #f39c12; padding-bottom: 15px; margin-bottom: 25px; }"
                   ".header h1 { color: #f39c12; margin: 5px; }"
                   ".info { background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 20px; }"
                   "table { width: 100%; border-collapse: collapse; margin-top: 20px; }"
                   "th { background-color: #34495e; color: white; padding: 12px; text-align: left; }"
                   "td { border: 1px solid #bdc3c7; padding: 10px; }"
                   "tr:nth-child(even) { background-color: #f8f9fa; }"
                   ".rank1 { background-color: #FFD700; font-weight: bold; }"
                   ".rank2 { background-color: #C0C0C0; font-weight: bold; }"
                   ".rank3 { background-color: #CD7F32; font-weight: bold; }"
                   ".footer { margin-top: 30px; text-align: right; font-size: 11px; color: #7f8c8d; }"
                   "</style></head><body>";
    
    html += "<div class='header'>";
    html += "<h1>TOP 10 SÁCH PHỔ BIẾN NHẤT</h1>";
    html += "<p>Báo Cáo Thống Kê Hệ Thống Quản Lý Thư Viện</p>";
    html += "</div>";
    
    html += "<div class='info'>";
    html += nhanThongTin->text().replace("📊", "").replace("<b>", "<strong>").replace("</b>", "</strong>");
    if (nhanSoSanh->isVisible()) {
        html += "<br>" + nhanSoSanh->text().replace("📈", "").replace("📉", "").replace("📊", "")
                .replace("<b>", "<strong>").replace("</b>", "</strong>");
    }
    html += "</div>";
    
    html += "<table>";
    html += "<tr><th>Hạng</th><th>ISBN</th><th>Tên Sách</th><th>Tác Giả</th><th>Số Lượt</th><th>Tỷ Lệ</th><th>Xu Hướng</th></tr>";
    
    for (int row = 0; row < bangTop10->rowCount(); row++) {
        QString rankClass = (row == 0) ? "rank1" : (row == 1) ? "rank2" : (row == 2) ? "rank3" : "";
        html += QString("<tr class='%1'>").arg(rankClass);
        for (int col = 0; col < 7; col++) {
            QString text = bangTop10->item(row, col)->text();
            html += QString("<td>%1</td>").arg(text);
        }
        html += "</tr>";
    }
    
    html += "</table>";
    
    html += "<div class='footer'>";
    html += QString("<p>Ngày xuất báo cáo: %1</p>").arg(QDate::currentDate().toString("dd/MM/yyyy"));
    html += "<p>Hệ Thống Quản Lý Thư Viện - Phiên bản 2.0</p>";
    html += "</div>";
    
    html += "</body></html>";
    
    QTextDocument document;
    document.setHtml(html);
    document.print(&printer);
    
    QMessageBox::information(this, "Xuất Báo Cáo", "Đã in báo cáo Top 10 thành công!");
}
