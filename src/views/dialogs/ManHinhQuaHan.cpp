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
#include <QTableView>
#include <QAbstractTableModel>
#include <vector>

const int TIEN_PHAT_MOI_NGAY = 5000;  // 5000 VND per day late fee

// Lightweight model for QuaHan table
class QuaHanTableModel : public QAbstractTableModel {
public:
    QuaHanTableModel(std::vector<DocGiaQuaHan> *data, QObject *parent = nullptr)
        : QAbstractTableModel(parent), m_data(data) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return m_data ? (int)m_data->size() : 0;
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return 8;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!m_data || !index.isValid()) return QVariant();
        int r = index.row();
        if (r < 0 || r >= (int)m_data->size()) return QVariant();
        const DocGiaQuaHan &item = (*m_data)[r];

        if (role == Qt::DisplayRole) {
            switch (index.column()) {
            case 0: return QString::number(item.docGia->data.MaThe);
            case 1: return QString::fromStdString(item.docGia->data.Ho + " " + item.docGia->data.Ten);
            case 2: return (item.docGia->data.Phai == 0) ? QString("Nam") : QString("Nữ");
            case 3: return QString::number(item.soSachQuaHan);
            case 4: return QString::number(item.tongSoNgayQuaHan) + " ngày";
            case 5: return QString::number(item.maxNgayQuaHan) + " ngày";
            case 6: return QString::number(item.phiPhat) + " đ";
            case 7: {
                if (item.maxNgayQuaHan >= 30) return QString("🔴 Nghiêm Trọng");
                if (item.maxNgayQuaHan >= 14) return QString("🟠 Cao");
                if (item.maxNgayQuaHan >= 7) return QString("🟡 Trung Bình");
                return QString("🟢 Nhẹ");
            }
            }
        }

        if (role == Qt::BackgroundRole && index.column() == 7) {
            if (item.maxNgayQuaHan >= 30) return QColor("#e74c3c");
            if (item.maxNgayQuaHan >= 14) return QColor("#e67e22");
            if (item.maxNgayQuaHan >= 7) return QColor("#f39c12");
            return QColor("#f1c40f");
        }

        if (role == Qt::ForegroundRole) {
            if (index.column() == 6) return QColor("#c0392b");
            if (index.column() == 7 && item.maxNgayQuaHan < 7) return QColor(Qt::black);
            if (index.column() == 7) return QColor(Qt::white);
        }

        if (role == Qt::FontRole) {
            QFont f;
            if (index.column() == 0 || index.column() == 3 || index.column() == 5 || index.column() == 6) {
                f.setBold(true); return f;
            }
        }

        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case 0: return QStringLiteral("Mã Thẻ");
            case 1: return QStringLiteral("Họ Tên");
            case 2: return QStringLiteral("Giới Tính");
            case 3: return QStringLiteral("Số Sách QH");
            case 4: return QStringLiteral("Tổng Ngày");
            case 5: return QStringLiteral("Ngày Max");
            case 6: return QStringLiteral("Phí Phạt");
            case 7: return QStringLiteral("Mức Độ");
            }
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    void refresh() { beginResetModel(); endResetModel(); }

private:
    std::vector<DocGiaQuaHan> *m_data;
};

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
    
    // Bảng quá hạn (model/view)
    bangQuaHan = new QTableView(this);
    modelQuaHan = new QuaHanTableModel(&danhSachQuaHan, this);
    bangQuaHan->setModel(modelQuaHan);
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
        "QTableView { "
        "    background-color: white; "
        "    alternate-background-color: #fef5e7; "
        "    border: 2px solid #e74c3c; "
        "    border-radius: 5px; "
        "    gridline-color: #f8d7da; "
        "} "
        "QTableView::item { "
        "    padding: 8px; "
        "} "
        "QTableView::item:selected { "
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
    // Prepare model-backed data
    danhSachQuaHan.clear();

    if (!cayDocGia) {
        nhanThongTin->setText("⚠️ Chưa có dữ liệu độc giả trong hệ thống");
        nhanThongTin->setStyleSheet(
            "color: #e74c3c; font-weight: bold; padding: 12px; "
            "background-color: #fadbd8; border-radius: 6px; border-left: 4px solid #e74c3c;"
        );
        return;
    }

    // Thu thập danh sách độc giả quá hạn
    duyetInOrder(cayDocGia, danhSachQuaHan);

    if (danhSachQuaHan.empty()) {
        nhanThongTin->setText("✅ Không có độc giả nào mượn quá hạn!");
        nhanThongTin->setStyleSheet(
            "color: #27ae60; font-weight: bold; padding: 12px; "
            "background-color: #d5f4e6; border-radius: 6px; border-left: 4px solid #27ae60;"
        );
        // ensure model empty
        if (modelQuaHan) if (auto m = dynamic_cast<QuaHanTableModel*>(modelQuaHan)) m->refresh();
        return;
    }

    // Sắp xếp theo số ngày quá hạn tối đa giảm dần
    std::sort(danhSachQuaHan.begin(), danhSachQuaHan.end(), 
        [](const DocGiaQuaHan& a, const DocGiaQuaHan& b) {
            return a.maxNgayQuaHan > b.maxNgayQuaHan;
        });

    // Hiển thị (model provides data)
    int soLuongHienThi = (int)danhSachQuaHan.size();
    int tongSachQuaHan = 0;
    for (const auto &it : danhSachQuaHan) tongSachQuaHan += it.soSachQuaHan;

    // Use uniform row height for visible rows (avoid per-row calls)
    bangQuaHan->verticalHeader()->setDefaultSectionSize(45);

    // Refresh model
    if (modelQuaHan) { if (auto m = dynamic_cast<QuaHanTableModel*>(modelQuaHan)) m->refresh(); }

    // Cập nhật thống kê
    capNhatThongKeDoanhThu(danhSachQuaHan);

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
    int selectedRow = bangQuaHan->currentIndex().row();
    if (selectedRow < 0) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng chọn độc giả để xem chi tiết!");
        return;
    }

    int maThe = 0;
    if (modelQuaHan) {
        QAbstractItemModel *m = modelQuaHan;
        QModelIndex idx = m->index(selectedRow, 0);
        maThe = m->data(idx, Qt::DisplayRole).toInt();
    }
    
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
    
    QAbstractItemModel *m = modelQuaHan;
    int rows = m ? m->rowCount() : 0;
    for (int row = 0; row < rows; row++) {
        html += "<tr>";
        for (int col = 0; col < 8; col++) {
            QModelIndex idx = m->index(row, col);
            QString text = m->data(idx, Qt::DisplayRole).toString();
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
    
    QMessageBox::information(this, "Xuất Báo Cáo", "✅ Đã in báo cáo quá hạn thành công!");
}
