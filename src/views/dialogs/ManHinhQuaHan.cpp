#include "views/dialogs/ManHinhQuaHan.h"
#include "utils/NgayThang.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QMessageBox>
#include <QTextDocument>
#include <QTextEdit>
#include <QDate>
#include <QPrinter>
#include <QPrintDialog>
#include <algorithm>

const int TIEN_PHAT_MOI_NGAY = 5000;  // 5000 VND per day late fee

ManHinhQuaHan::ManHinhQuaHan(PTRDG rootDocGia, PTRDS dsDauSach[], int soLuongDauSach, QWidget *parent)
    : QDialog(parent), cayDocGia(rootDocGia), danhSachDauSach(dsDauSach), soLuong(soLuongDauSach) {
    setWindowTitle("Độc Giả Mượn Quá Hạn");
    setMinimumSize(1200, 800);
    setModal(true);
    
    thietLapGiaoDien();
    apDungPhongCach();
    capNhatBangQuaHan();
}

ManHinhQuaHan::~ManHinhQuaHan() {
}

void ManHinhQuaHan::thietLapGiaoDien() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(25, 25, 25, 25);
    
    // Tiêu đề
    nhanTieuDe = new QLabel("DANH SÁCH ĐỘC GIẢ MƯỢN QUÁ HẠN");
    nhanTieuDe->setStyleSheet(
        "font-size: 24px; "
        "font-weight: bold; "
        "color: white; "
        "padding: 20px; "
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #dc2626, stop:1 #b91c1c); "
        "border-radius: 10px;"
    );
    nhanTieuDe->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(nhanTieuDe);
    
    // ========== STATISTICS PANELS ==========
    QHBoxLayout *statsLayout = new QHBoxLayout();
    
    // Revenue panel
    QGroupBox *boxRevenue = new QGroupBox("Thống Kê Doanh Thu");
    boxRevenue->setStyleSheet(
        "QGroupBox { "
        "    font-weight: bold; "
        "    border: 2px solid #e74c3c; "
        "    border-radius: 8px; "
        "    margin-top: 12px; "
        "    padding-top: 10px; "
        "    background-color: #fadbd8; "
        "} "
        "QGroupBox::title { "
        "    subcontrol-origin: margin; "
        "    left: 15px; "
        "    padding: 0 5px; "
        "    color: #c0392b; "
        "}"
    );
    QVBoxLayout *revenueLayout = new QVBoxLayout();
    nhanDoanhThu = new QLabel("Đang tải...");
    nhanDoanhThu->setWordWrap(true);
    nhanDoanhThu->setStyleSheet("padding: 10px; font-size: 13px;");
    revenueLayout->addWidget(nhanDoanhThu);
    boxRevenue->setLayout(revenueLayout);
    statsLayout->addWidget(boxRevenue);
    
    // Active readers panel
    QGroupBox *boxActive = new QGroupBox("Độc Giả Hoạt Động");
    boxActive->setStyleSheet(
        "QGroupBox { "
        "    font-weight: bold; "
        "    border: 2px solid #1e40af; "
        "    border-radius: 8px; "
        "    margin-top: 12px; "
        "    padding-top: 10px; "
        "    background-color: #dbeafe; "
        "} "
        "QGroupBox::title { "
        "    subcontrol-origin: margin; "
        "    left: 15px; "
        "    padding: 0 5px; "
        "    color: #1e40af; "
        "}"
    );
    QVBoxLayout *activeLayout = new QVBoxLayout();
    nhanHoatDong = new QLabel("Đang tải...");
    nhanHoatDong->setWordWrap(true);
    nhanHoatDong->setStyleSheet("padding: 10px; font-size: 13px;");
    activeLayout->addWidget(nhanHoatDong);
    boxActive->setLayout(activeLayout);
    statsLayout->addWidget(boxActive);
    
    mainLayout->addLayout(statsLayout);
    
    // Panel thông tin
    nhanThongTin = new QLabel(
        "📌 Danh sách độc giả có sách mượn quá hạn. "
        "Phí phạt: 5,000đ/ngày sau 7 ngày mượn."
    );
    nhanThongTin->setStyleSheet(
        "color: #34495e; "
        "font-size: 12px; "
        "padding: 12px; "
        "background-color: #ecf0f1; "
        "border-radius: 6px; "
        "border-left: 4px solid #e67e22;"
    );
    nhanThongTin->setWordWrap(true);
    mainLayout->addWidget(nhanThongTin);
    
    // Bảng quá hạn (8 cột - thêm "Phí Phạt")
    bangQuaHan = new QTableWidget(this);
    bangQuaHan->setColumnCount(8);
    bangQuaHan->setHorizontalHeaderLabels({
        "Mã Thẻ", "👤 Họ Tên", "📱 Giới Tính",
        "📚 Số Sách QH", "⏰ Tổng Ngày QH", 
        "🔥 Ngày QH Max", "💰 Phí Phạt", "⚠️ Mức Độ"
    });
    
    bangQuaHan->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangQuaHan->setSelectionMode(QAbstractItemView::SingleSelection);
    bangQuaHan->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bangQuaHan->setAlternatingRowColors(true);
    bangQuaHan->verticalHeader()->setVisible(false);
    bangQuaHan->horizontalHeader()->setStretchLastSection(true);
    bangQuaHan->setShowGrid(true);
    
    // Đặt chiều rộng cột
    bangQuaHan->setColumnWidth(0, 80);   // Mã thẻ
    bangQuaHan->setColumnWidth(1, 200);  // Họ tên
    bangQuaHan->setColumnWidth(2, 90);   // Giới tính
    bangQuaHan->setColumnWidth(3, 120);  // Số sách QH
    bangQuaHan->setColumnWidth(4, 120);  // Tổng ngày QH
    bangQuaHan->setColumnWidth(5, 120);  // Ngày QH max
    bangQuaHan->setColumnWidth(6, 120);  // Phí phạt
    
    mainLayout->addWidget(bangQuaHan);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    nutXemChiTiet = new QPushButton("👁️ Xem Chi Tiết", this);
    nutXemChiTiet->setMinimumSize(140, 40);
    nutXemChiTiet->setToolTip("Xem chi tiết độc giả đã chọn");
    
    nutXuatBaoCao = new QPushButton("📄 Xuất Báo Cáo", this);
    nutXuatBaoCao->setMinimumSize(140, 40);
    nutXuatBaoCao->setToolTip("In hoặc xuất báo cáo (Ctrl+P)");
    nutXuatBaoCao->setShortcut(QKeySequence("Ctrl+P"));
    
    nutLamMoi = new QPushButton("🔄 Làm Mới", this);
    nutLamMoi->setMinimumSize(120, 40);
    nutLamMoi->setToolTip("Cập nhật lại danh sách (F5)");
    nutLamMoi->setShortcut(QKeySequence("F5"));
    
    nutDong = new QPushButton("❌ Đóng", this);
    nutDong->setMinimumSize(120, 40);
    nutDong->setToolTip("Đóng cửa sổ (Esc)");
    nutDong->setShortcut(QKeySequence(Qt::Key_Escape));
    
    buttonLayout->addWidget(nutXemChiTiet);
    buttonLayout->addWidget(nutXuatBaoCao);
    buttonLayout->addWidget(nutLamMoi);
    buttonLayout->addWidget(nutDong);
    
    mainLayout->addLayout(buttonLayout);
    
    // Kết nối signals
    connect(nutXemChiTiet, &QPushButton::clicked, this, &ManHinhQuaHan::onViewReaderDetail);
    connect(nutXuatBaoCao, &QPushButton::clicked, this, &ManHinhQuaHan::onExportReport);
    connect(nutLamMoi, &QPushButton::clicked, this, &ManHinhQuaHan::capNhatBangQuaHan);
    connect(nutDong, &QPushButton::clicked, this, &QDialog::accept);
}

void ManHinhQuaHan::apDungPhongCach() {
    bangQuaHan->setStyleSheet(
        "QTableWidget { "
        "    background-color: white; "
        "    alternate-background-color: #fef5e7; "
        "    border: 2px solid #e74c3c; "
        "    border-radius: 5px; "
        "    gridline-color: #f8d7da; "
        "} "
        "QTableWidget::item { "
        "    padding: 8px; "
        "} "
        "QTableWidget::item:selected { "
        "    background-color: #e74c3c; "
        "    color: white; "
        "} "
        "QHeaderView::section { "
        "    background-color: #c0392b; "
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

void ManHinhQuaHan::duyetInOrder(PTRDG root, std::vector<DocGiaQuaHan>& ds) {
    if (!root) return;
    
    duyetInOrder(root->left, ds);
    
    // Kiểm tra độc giả có sách quá hạn không
    if (root->data.TrangThai == 1 && root->data.soSachDangMuon > 0) {
        int tongNgayQuaHan = 0;
        int maxNgayQuaHan = 0;
        int soSachQuaHan = 0;
        int tongPhiPhat = 0;
        
        // Duyệt danh sách mượn trả (linked list)
        MUONTRA p = root->data.dsmt;
        while (p) {
            // Chỉ tính sách đang mượn (TrangThai = 0)
            if (p->data.TrangThai == 0) {
                // Tính số ngày quá hạn từ ngày mượn
                std::string ngayMuon = p->data.NgayMuon;
                int ngayQuaHan = tinhSoNgayQuaHan(ngayMuon);
                
                if (ngayQuaHan > 0) {
                    soSachQuaHan++;
                    tongNgayQuaHan += ngayQuaHan;
                    if (ngayQuaHan > maxNgayQuaHan) {
                        maxNgayQuaHan = ngayQuaHan;
                    }
                    // Calculate late fee: 5000 VND per day
                    tongPhiPhat += ngayQuaHan * TIEN_PHAT_MOI_NGAY;
                }
            }
            p = p->next;
        }
        
        if (soSachQuaHan > 0) {
            DocGiaQuaHan item;
            item.docGia = root;
            item.soSachQuaHan = soSachQuaHan;
            item.tongSoNgayQuaHan = tongNgayQuaHan;
            item.maxNgayQuaHan = maxNgayQuaHan;
            item.phiPhat = tongPhiPhat;
            ds.push_back(item);
        }
    }
    
    duyetInOrder(root->right, ds);
}

void ManHinhQuaHan::capNhatBangQuaHan() {
    bangQuaHan->setRowCount(0);
    
    if (!cayDocGia) {
        nhanThongTin->setText("⚠️ Chưa có dữ liệu độc giả trong hệ thống");
        nhanThongTin->setStyleSheet(
            "color: #e74c3c; font-weight: bold; padding: 12px; "
            "background-color: #fadbd8; border-radius: 6px; border-left: 4px solid #e74c3c;"
        );
        return;
    }
    
    // Thu thập danh sách độc giả quá hạn
    std::vector<DocGiaQuaHan> danhSach;
    duyetInOrder(cayDocGia, danhSach);
    
    if (danhSach.empty()) {
        nhanThongTin->setText("✅ Không có độc giả nào mượn quá hạn!");
        nhanThongTin->setStyleSheet(
            "color: #27ae60; font-weight: bold; padding: 12px; "
            "background-color: #d5f4e6; border-radius: 6px; border-left: 4px solid #27ae60;"
        );
        return;
    }
    
    // Sắp xếp theo số ngày quá hạn tối đa giảm dần
    std::sort(danhSach.begin(), danhSach.end(), 
        [](const DocGiaQuaHan& a, const DocGiaQuaHan& b) {
            return a.maxNgayQuaHan > b.maxNgayQuaHan;
        });
    
    // Hiển thị
    int soLuongHienThi = danhSach.size();
    bangQuaHan->setRowCount(soLuongHienThi);
    
    int tongSachQuaHan = 0;
    
    for (int i = 0; i < soLuongHienThi; i++) {
        const DocGiaQuaHan& item = danhSach[i];
        tongSachQuaHan += item.soSachQuaHan;
        
        // Mã thẻ
        QTableWidgetItem* itemMaThe = new QTableWidgetItem(
            QString::number(item.docGia->data.MaThe));
        itemMaThe->setTextAlignment(Qt::AlignCenter);
        QFont fontMaThe = itemMaThe->font();
        fontMaThe.setBold(true);
        itemMaThe->setFont(fontMaThe);
        bangQuaHan->setItem(i, 0, itemMaThe);
        
        // Họ tên
        QString hoTen = QString::fromStdString(item.docGia->data.Ho + " " + item.docGia->data.Ten);
        QTableWidgetItem* itemHoTen = new QTableWidgetItem(hoTen);
        bangQuaHan->setItem(i, 1, itemHoTen);
        
        // Giới tính
        QString gioiTinh = (item.docGia->data.Phai == 0) ? "Nam" : "Nữ";
        QTableWidgetItem* itemGioiTinh = new QTableWidgetItem(gioiTinh);
        itemGioiTinh->setTextAlignment(Qt::AlignCenter);
        bangQuaHan->setItem(i, 2, itemGioiTinh);
        
        // Số sách quá hạn
        QTableWidgetItem* itemSoSach = new QTableWidgetItem(
            QString::number(item.soSachQuaHan));
        itemSoSach->setTextAlignment(Qt::AlignCenter);
        QFont fontSoSach = itemSoSach->font();
        fontSoSach.setBold(true);
        itemSoSach->setFont(fontSoSach);
        itemSoSach->setForeground(QColor("#e74c3c"));
        bangQuaHan->setItem(i, 3, itemSoSach);
        
        // Tổng ngày quá hạn
        QTableWidgetItem* itemTongNgay = new QTableWidgetItem(
            QString::number(item.tongSoNgayQuaHan) + " ngày");
        itemTongNgay->setTextAlignment(Qt::AlignCenter);
        bangQuaHan->setItem(i, 4, itemTongNgay);
        
        // Ngày quá hạn tối đa
        QTableWidgetItem* itemMaxNgay = new QTableWidgetItem(
            QString::number(item.maxNgayQuaHan) + " ngày");
        itemMaxNgay->setTextAlignment(Qt::AlignCenter);
        QFont fontMaxNgay = itemMaxNgay->font();
        fontMaxNgay.setBold(true);
        itemMaxNgay->setFont(fontMaxNgay);
        bangQuaHan->setItem(i, 5, itemMaxNgay);
        
        // Phí phạt (cột mới)
        QTableWidgetItem* itemPhiPhat = new QTableWidgetItem(
            QString::number(item.phiPhat).rightJustified(10, ' ') + " đ");
        itemPhiPhat->setTextAlignment(Qt::AlignCenter);
        QFont fontPhiPhat = itemPhiPhat->font();
        fontPhiPhat.setBold(true);
        itemPhiPhat->setFont(fontPhiPhat);
        itemPhiPhat->setForeground(QColor("#c0392b"));
        bangQuaHan->setItem(i, 6, itemPhiPhat);
        
        // Mức độ với màu
        QString mucDo;
        QColor mauNen;
        QColor mauChu(Qt::white);
        
        if (item.maxNgayQuaHan >= 30) {
            mucDo = "🔴 Nghiêm Trọng";
            mauNen = QColor("#e74c3c");
        } else if (item.maxNgayQuaHan >= 14) {
            mucDo = "🟠 Cao";
            mauNen = QColor("#e67e22");
        } else if (item.maxNgayQuaHan >= 7) {
            mucDo = "🟡 Trung Bình";
            mauNen = QColor("#f39c12");
        } else {
            mucDo = "🟢 Nhẹ";
            mauNen = QColor("#f1c40f");
            mauChu = QColor(Qt::black);
        }
        
        QTableWidgetItem* itemMucDo = new QTableWidgetItem(mucDo);
        itemMucDo->setTextAlignment(Qt::AlignCenter);
        itemMucDo->setBackground(mauNen);
        itemMucDo->setForeground(mauChu);
        QFont fontMucDo = itemMucDo->font();
        fontMucDo.setBold(true);
        itemMucDo->setFont(fontMucDo);
        bangQuaHan->setItem(i, 7, itemMucDo);
        
        // Đặt chiều cao hàng
        bangQuaHan->setRowHeight(i, 45);
    }
    
    // Cập nhật thống kê
    capNhatThongKeDoanhThu(danhSach);
    
    // Cập nhật thông tin
    nhanThongTin->setText(
        QString("⚠️ Có %1 độc giả mượn quá hạn | "
               "Tổng %2 cuốn sách quá hạn | "
               "Cần xử lý ngay!")
            .arg(soLuongHienThi)
            .arg(tongSachQuaHan)
    );
    nhanThongTin->setStyleSheet(
        "color: #e74c3c; font-weight: bold; padding: 12px; "
        "background-color: #fadbd8; border-radius: 6px; border-left: 4px solid #e74c3c;"
    );
}

// ==================== NEW ADVANCED METHODS ====================

int ManHinhQuaHan::demDocGiaHoatDong(int soNgayGanDay) {
    int count = 0;
    QDate ngayHienTai = QDate::currentDate();
    QDate ngayBatDau = ngayHienTai.addDays(-soNgayGanDay);
    
    std::function<void(PTRDG)> duyetAVL = [&](PTRDG node) {
        if (!node) return;
        
        duyetAVL(node->left);
        
        // Check if reader borrowed any book in the last N days
        MUONTRA mt = node->data.dsmt;
        while (mt) {
            QString ngayMuonStr = QString::fromStdString(mt->data.NgayMuon);
            QDate ngayMuon = QDate::fromString(ngayMuonStr, "dd/MM/yyyy");
            
            if (ngayMuon.isValid() && ngayMuon >= ngayBatDau) {
                count++;
                break;  // Count each reader only once
            }
            mt = mt->next;
        }
        
        duyetAVL(node->right);
    };
    
    duyetAVL(cayDocGia);
    return count;
}

void ManHinhQuaHan::capNhatThongKeDoanhThu(const std::vector<DocGiaQuaHan>& ds) {
    // Calculate revenue statistics
    int tongPhiPhat = 0;
    int docGiaNghiemTrong = 0;
    int docGiaCao = 0;
    
    for (const auto& item : ds) {
        tongPhiPhat += item.phiPhat;
        if (item.maxNgayQuaHan >= 30) docGiaNghiemTrong++;
        else if (item.maxNgayQuaHan >= 14) docGiaCao++;
    }
    
    nhanDoanhThu->setText(QString(
        "<b>💰 Tổng phí phạt:</b> %1 đ<br>"
        "<b>🔴 Nghiêm trọng:</b> %2 độc giả<br>"
        "<b>🟠 Mức cao:</b> %3 độc giả<br>"
        "<i>Phí phạt: 5,000đ/ngày sau 7 ngày mượn</i>")
        .arg(QString::number(tongPhiPhat).rightJustified(12, ' '))
        .arg(docGiaNghiemTrong)
        .arg(docGiaCao));
    
    // Calculate active readers statistics
    int hoatDong7Ngay = demDocGiaHoatDong(7);
    int hoatDong30Ngay = demDocGiaHoatDong(30);
    
    // Count total readers
    int tongDocGia = 0;
    std::function<void(PTRDG)> demTong = [&](PTRDG node) {
        if (!node) return;
        demTong(node->left);
        tongDocGia++;
        demTong(node->right);
    };
    demTong(cayDocGia);
    
    double tyLeHoatDong = (tongDocGia > 0) ? (hoatDong30Ngay * 100.0 / tongDocGia) : 0;
    
    nhanHoatDong->setText(QString(
        "<b>📊 Tổng độc giả:</b> %1<br>"
        "<b>🔵 Hoạt động 7 ngày:</b> %2<br>"
        "<b>🔷 Hoạt động 30 ngày:</b> %3<br>"
        "<b>📈 Tỷ lệ hoạt động:</b> %4%")
        .arg(tongDocGia)
        .arg(hoatDong7Ngay)
        .arg(hoatDong30Ngay)
        .arg(QString::number(tyLeHoatDong, 'f', 1)));
}

void ManHinhQuaHan::onViewReaderDetail() {
    int selectedRow = bangQuaHan->currentRow();
    if (selectedRow < 0) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng chọn độc giả để xem chi tiết!");
        return;
    }
    
    int maThe = bangQuaHan->item(selectedRow, 0)->text().toInt();
    
    // Find reader in tree
    PTRDG reader = nullptr;
    std::function<void(PTRDG)> timDocGia = [&](PTRDG node) {
        if (!node) return;
        if (node->data.MaThe == maThe) {
            reader = node;
            return;
        }
        if (maThe < node->data.MaThe) timDocGia(node->left);
        else timDocGia(node->right);
    };
    timDocGia(cayDocGia);
    
    if (reader) {
        hienThiChiTietDocGia(reader);
    }
}

void ManHinhQuaHan::hienThiChiTietDocGia(PTRDG docGia) {
    QString html = "<html><head><style>"
                   "body { font-family: Arial; }"
                   "h2 { color: #e74c3c; }"
                   "table { border-collapse: collapse; width: 100%; }"
                   "th { background-color: #e74c3c; color: white; padding: 10px; }"
                   "td { border: 1px solid #ddd; padding: 8px; }"
                   ".overdue { color: red; font-weight: bold; }"
                   "</style></head><body>";
    
    html += QString("<h2>👤 Chi Tiết Độc Giả: %1 %2</h2>")
            .arg(QString::fromStdString(docGia->data.Ho))
            .arg(QString::fromStdString(docGia->data.Ten));
    
    html += QString("<p><b>Mã thẻ:</b> %1 | <b>Giới tính:</b> %2 | <b>Số sách đang mượn:</b> %3</p>")
            .arg(docGia->data.MaThe)
            .arg(docGia->data.Phai == 0 ? "Nam" : "Nữ")
            .arg(docGia->data.soSachDangMuon);
    
    html += "<h3>📚 Danh Sách Sách Đang Mượn:</h3>";
    html += "<table><tr><th>STT</th><th>Mã Sách</th><th>Tên Sách</th><th>Ngày Mượn</th><th>Ngày Quá Hạn</th><th>Phí Phạt</th></tr>";
    
    MUONTRA mt = docGia->data.dsmt;
    int stt = 1;
    int tongPhiPhat = 0;
    
    while (mt) {
        if (mt->data.TrangThai == 0 && mt->data.banSaoSach) {
            QString maSach = QString::fromStdString(mt->data.banSaoSach->maSach);
            QString tenSach = "N/A";
            
            // Find book name
            for (int i = 0; i < soLuong; i++) {
                PTRDMS dms = danhSachDauSach[i]->dms;
                while (dms) {
                    if (dms->maSach == mt->data.banSaoSach->maSach) {
                        tenSach = QString::fromStdString(danhSachDauSach[i]->tenSach);
                        break;
                    }
                    dms = dms->next;
                }
                if (tenSach != "N/A") break;
            }
            
            int ngayQuaHan = tinhSoNgayQuaHan(mt->data.NgayMuon);
            int phiPhat = ngayQuaHan * TIEN_PHAT_MOI_NGAY;
            tongPhiPhat += phiPhat;
            
            QString rowClass = (ngayQuaHan > 0) ? " class='overdue'" : "";
            html += QString("<tr%1><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6 ngày</td><td>%7 đ</td></tr>")
                    .arg(rowClass)
                    .arg(stt++)
                    .arg(maSach)
                    .arg(tenSach)
                    .arg(QString::fromStdString(mt->data.NgayMuon))
                    .arg(ngayQuaHan)
                    .arg(phiPhat);
        }
        mt = mt->next;
    }
    
    html += "</table>";
    html += QString("<h3 style='color: #c0392b;'>💰 Tổng Phí Phạt: %1 đ</h3>").arg(tongPhiPhat);
    html += "</body></html>";
    
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Chi Tiết Độc Giả");
    dialog->setMinimumSize(800, 500);
    
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    QTextEdit *textEdit = new QTextEdit(dialog);
    textEdit->setHtml(html);
    textEdit->setReadOnly(true);
    layout->addWidget(textEdit);
    
    QPushButton *closeButton = new QPushButton("Đóng", dialog);
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::accept);
    layout->addWidget(closeButton);
    
    dialog->exec();
    delete dialog;
}

void ManHinhQuaHan::onExportReport() {
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize::A4);
    
    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() != QDialog::Accepted) {
        return;
    }
    
    QString html = "<html><head><style>"
                   "body { font-family: Arial; margin: 30px; }"
                   ".header { text-align: center; border-bottom: 3px solid #e74c3c; padding-bottom: 15px; }"
                   ".header h1 { color: #e74c3c; margin: 5px; }"
                   ".stats { background-color: #fef5e7; padding: 15px; border-radius: 8px; margin: 20px 0; }"
                   "table { width: 100%; border-collapse: collapse; margin-top: 20px; }"
                   "th { background-color: #c0392b; color: white; padding: 12px; text-align: left; }"
                   "td { border: 1px solid #bdc3c7; padding: 10px; }"
                   "tr:nth-child(even) { background-color: #fef5e7; }"
                   ".footer { margin-top: 30px; text-align: right; font-size: 11px; color: #7f8c8d; }"
                   "</style></head><body>";
    
    html += "<div class='header'>";
    html += "<h1>⚠️ BÁO CÁO ĐỘC GIẢ MƯỢN QUÁ HẠN</h1>";
    html += "<p>Hệ Thống Quản Lý Thư Viện</p>";
    html += "</div>";
    
    html += "<div class='stats'>";
    html += nhanThongTin->text().replace("<b>", "<strong>").replace("</b>", "</strong>");
    html += "<br><hr>";
    html += nhanDoanhThu->text();
    html += " | " + nhanHoatDong->text();
    html += "</div>";
    
    html += "<table>";
    html += "<tr><th>Mã Thẻ</th><th>Họ Tên</th><th>Giới Tính</th><th>Số Sách QH</th><th>Tổng Ngày</th><th>Ngày Max</th><th>Phí Phạt</th><th>Mức Độ</th></tr>";
    
    for (int row = 0; row < bangQuaHan->rowCount(); row++) {
        html += "<tr>";
        for (int col = 0; col < 8; col++) {
            html += QString("<td>%1</td>").arg(bangQuaHan->item(row, col)->text());
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
    
    QMessageBox::information(this, "Xuất Báo Cáo", "✅ Đã in báo cáo quá hạn thành công!");
}
