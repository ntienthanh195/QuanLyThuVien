#include "views/dialogs/ManHinhTop10.h"
#include "models/QuanLySach.h"
#include "models/DocGia.h"
#include <QTableView>
#include <QAbstractTableModel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QGroupBox>
#include <QMessageBox>
#include "utils/PrintHelper.h"
#include <algorithm>
#include <vector>
#include "utils/LanguageManager.h"

// Lightweight model for Top10 table
class Top10TableModel : public QAbstractTableModel {
public:
    Top10TableModel(std::vector<SachVaSoLuotMuon> *data, QObject *parent = nullptr)
        : QAbstractTableModel(parent), m_data(data) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return m_data ? (int)m_data->size() : 0;
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return 7;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!m_data || !index.isValid()) return QVariant();
        int r = index.row();
        if (r < 0 || r >= (int)m_data->size()) return QVariant();
        const SachVaSoLuotMuon &item = (*m_data)[r];

        if (role == Qt::DisplayRole) {
            switch (index.column()) {
            case 0: return QString::number(r + 1);
            case 1: return QString::fromStdString(item.sach->ISBN);
            case 2: return QString::fromStdString(item.sach->tenSach);
            case 3: return QString::fromStdString(item.sach->tacGia);
            case 4: return QString::number(item.soLuotMuon);
            case 5: return QString::number(item.tyLe, 'f', 1) + "%";
            case 6: {
                if (item.thayDoi > 0) return QString("🔼 +%1").arg(item.thayDoi);
                if (item.thayDoi < 0) return QString("🔽 %1").arg(item.thayDoi);
                return QString("-");
            }
            }
        }

        // Styling roles
        if (role == Qt::BackgroundRole) {
            if (index.column() == 0 && r < 3) {
                static QStringList mauHang = {"#FFD700", "#C0C0C0", "#CD7F32"};
                return QColor(mauHang[r]);
            }
        }

        if (role == Qt::FontRole) {
            QFont f;
            if (index.column() == 0 && r < 3) { f.setBold(true); return f; }
            if (index.column() == 2 && r < 3) { f.setBold(true); return f; }
            if (index.column() == 4) { f.setBold(true); return f; }
        }

        if (role == Qt::ForegroundRole) {
            if (index.column() == 4) return QColor("#27ae60");
            if (index.column() == 6) {
                if (item.thayDoi > 0) return QColor("#27ae60");
                if (item.thayDoi < 0) return QColor("#e74c3c");
                return QColor("#95a5a6");
            }
        }

        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case 0: return LanguageManager::instance().getText("TABLE_HEADER_RANK");
            case 1: return LanguageManager::instance().getText("TABLE_HEADER_ISBN");
            case 2: return LanguageManager::instance().getText("TABLE_HEADER_TITLE");
            case 3: return LanguageManager::instance().getText("TABLE_HEADER_AUTHOR");
            case 4: return LanguageManager::instance().getText("TABLE_HEADER_LOANS");
            case 5: return LanguageManager::instance().getText("TABLE_HEADER_RATE");
            case 6: return LanguageManager::instance().getText("TABLE_HEADER_TREND");
            }
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    void refresh() { beginResetModel(); endResetModel(); }

private:
    std::vector<SachVaSoLuotMuon> *m_data;
};

ManHinhTop10::ManHinhTop10(PTRDS dsDauSach[], int soLuongDauSach, QWidget *parent)
    : QDialog(parent), danhSachDauSach(dsDauSach), soLuong(soLuongDauSach) {
    setWindowTitle(LanguageManager::instance().getText("TITLE_TOP10"));
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
    nhanTieuDe = new QLabel(LanguageManager::instance().getText("HEADER_TOP10"));
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
    QGroupBox *boxFilter = new QGroupBox(LanguageManager::instance().getText("GROUP_FILTER_TIME"));
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
    filterLayout->addWidget(new QLabel(LanguageManager::instance().getText("LBL_RANGE")));
    comboBoLoc = new QComboBox();
    comboBoLoc->addItems({
        LanguageManager::instance().getText("CB_TIME_ALL"),
        LanguageManager::instance().getText("CB_TIME_THIS_MONTH"),
        LanguageManager::instance().getText("CB_TIME_LAST_MONTH"),
        LanguageManager::instance().getText("CB_TIME_CUSTOM")
    });
    comboBoLoc->setMinimumWidth(180);
    filterLayout->addWidget(comboBoLoc);
    
    filterLayout->addSpacing(20);
    
    // Date range pickers
    filterLayout->addWidget(new QLabel(LanguageManager::instance().getText("LBL_FROM_DATE")));
    dateFrom = new QDateEdit(QDate::currentDate().addMonths(-1));
    dateFrom->setCalendarPopup(true);
    dateFrom->setDisplayFormat("dd/MM/yyyy");
    dateFrom->setEnabled(false);
    filterLayout->addWidget(dateFrom);
    
    filterLayout->addWidget(new QLabel(LanguageManager::instance().getText("LBL_TO_DATE")));
    dateTo = new QDateEdit(QDate::currentDate());
    dateTo->setCalendarPopup(true);
    dateTo->setDisplayFormat("dd/MM/yyyy");
    dateTo->setEnabled(false);
    filterLayout->addWidget(dateTo);
    
    filterLayout->addSpacing(20);
    
    // Checkbox so sánh
    checkSoSanh = new QCheckBox(LanguageManager::instance().getText("CHECK_SHOW_TREND"));
    checkSoSanh->setChecked(true);
    checkSoSanh->setToolTip("So sánh với kỳ trước đó");
    filterLayout->addWidget(checkSoSanh);
    
    filterLayout->addStretch();
    boxFilter->setLayout(filterLayout);
    mainLayout->addWidget(boxFilter);
    
    // Panel thông tin
    nhanThongTin = new QLabel(
        LanguageManager::instance().getText("MSG_TOP10_NOTE")
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
    
    // Bảng Top 10 (model/view)
    bangTop10 = new QTableView(this);
    modelTop10 = new Top10TableModel(&danhSachTop, this);
    bangTop10->setModel(modelTop10);
    bangTop10->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangTop10->setSelectionMode(QAbstractItemView::SingleSelection);
    bangTop10->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bangTop10->setAlternatingRowColors(true);
    bangTop10->verticalHeader()->setVisible(false);
    bangTop10->horizontalHeader()->setStretchLastSection(true);
    bangTop10->setShowGrid(true);
    
    // Đặt chiều rộng cột (vẫn áp dụng cho QTableView)
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
    
    nutXuatBaoCao = new QPushButton(LanguageManager::instance().getText("BTN_EXPORT_REPORT"), this);
    nutXuatBaoCao->setMinimumSize(140, 40);
    nutXuatBaoCao->setToolTip("In hoặc xuất báo cáo (Ctrl+P)");
    nutXuatBaoCao->setShortcut(QKeySequence("Ctrl+P"));
    
    nutLamMoi = new QPushButton(LanguageManager::instance().getText("BTN_REFRESH"), this);
    nutLamMoi->setMinimumSize(120, 40);
    nutLamMoi->setToolTip("Cập nhật lại bảng xếp hạng (F5)");
    nutLamMoi->setShortcut(QKeySequence("F5"));
    
    nutDong = new QPushButton(LanguageManager::instance().getText("BTN_CLOSE"), this);
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
        "QTableView { "
        "    background-color: white; "
        "    alternate-background-color: #f8f9fa; "
        "    border: 2px solid #bdc3c7; "
        "    border-radius: 5px; "
        "    gridline-color: #ecf0f1; "
        "} "
        "QTableView::item { "
        "    padding: 8px; "
        "} "
        "QTableView::item:selected { "
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
    // Clear model-backed data (model will be refreshed after we repopulate danhSachTop)
    // No direct setRowCount on QTableView.
    
    if (soLuong == 0) {
        nhanThongTin->setText(LanguageManager::instance().getText("MSG_NO_DATA_TOP10"));
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
    
    // Thu thập dữ liệu và tính số lượt mượn -> populate member danhSachTop and refresh model
    danhSachTop.clear();
    int tongLuotMuon = 0;

    for (int i = 0; i < soLuong; i++) {
        if (danhSachDauSach[i]) {
            SachVaSoLuotMuon item;
            item.sach = danhSachDauSach[i];

            if (coBoLoc) {
                item.soLuotMuon = demLuotMuonTheoKhoangThoiGian(danhSachDauSach[i], tuNgay, denNgay);
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
                item.soLuotMuon = danhSachDauSach[i]->soLuotMuon;
                item.soLuotMuonTruoc = 0;
                item.thayDoi = 0;
            }

            if (item.soLuotMuon > 0) {
                danhSachTop.push_back(item);
                tongLuotMuon += item.soLuotMuon;
            }
        }
    }

    if (danhSachTop.empty()) {
        nhanThongTin->setText(LanguageManager::instance().getText("MSG_NO_DATA_PERIOD"));
        nhanThongTin->setStyleSheet(
            "color: #64748b; font-weight: bold; padding: 12px; "
            "background-color: #f1f5f9; border-radius: 6px; border-left: 4px solid #94a3b8;"
        );
        nhanSoSanh->setVisible(false);
        return;
    }
    
    // Sắp xếp theo số lượt mượn giảm dần
    std::sort(danhSachTop.begin(), danhSachTop.end(), 
        [](const SachVaSoLuotMuon& a, const SachVaSoLuotMuon& b) {
            return a.soLuotMuon > b.soLuotMuon;
        });

    // Lấy top 10
    int soLuongHienThi = std::min(10, (int)danhSachTop.size());
    if ((int)danhSachTop.size() > soLuongHienThi) danhSachTop.resize(soLuongHienThi);
    
    // Màu cho huy chương
    QStringList mauHang = {"#FFD700", "#C0C0C0", "#CD7F32"};  // Vàng, Bạc, Đồng
    QStringList iconHang = {"🥇", "🥈", "🥉"};
    
    for (int i = 0; i < soLuongHienThi; i++) {
        const SachVaSoLuotMuon& item = danhSachTop[i];
        // compute and store percentage
        double tyLe = (tongLuotMuon > 0) ? (item.soLuotMuon * 100.0 / tongLuotMuon) : 0;
        danhSachTop[i].tyLe = tyLe;
        // Use uniform row height for visuals (avoid per-row calls)
        Q_UNUSED(i);
    }
    // Ensure uniform row height
    bangTop10->verticalHeader()->setDefaultSectionSize(45);
    // Refresh model
    if (modelTop10) { if (auto m = dynamic_cast<Top10TableModel*>(modelTop10)) m->refresh(); }
    // Cập nhật thông tin
    QString khoangTG = coBoLoc ? 
        QString("từ %1 đến %2").arg(tuNgay.toString("dd/MM/yyyy")).arg(denNgay.toString("dd/MM/yyyy")) :
        "tất cả thời gian";
    
    nhanThongTin->setText(
        LanguageManager::instance().getText("MSG_STATS_SUMMARY_HTML")
            .arg(khoangTG)
            .arg(danhSachTop.size())
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
    
    // Duyệt tất cả học viên để đếm lượt mượn
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
    QAbstractItemModel *m = modelTop10;
    if (m) {
        for (int row = 0; row < m->rowCount(); row++) {
            QModelIndex idx = m->index(row, 6);
            QString xuHuong = m->data(idx, Qt::DisplayRole).toString();
            if (xuHuong.contains("🔼")) tongTang++;
            else if (xuHuong.contains("🔽")) tongGiam++;
            else tongKhongDoi++;
        }
    }
    
    QString thongBao;
    QString mauNen, mauVien;
    
    if (tongTang > tongGiam) {
        thongBao = LanguageManager::instance().getText("MSG_TREND_POSITIVE")
                   .arg(tongTang).arg(tongGiam).arg(tongKhongDoi);
        mauNen = "#d1fae5";
        mauVien = "#059669";
    } else if (tongGiam > tongTang) {
        thongBao = LanguageManager::instance().getText("MSG_TREND_NEGATIVE")
                   .arg(tongGiam).arg(tongTang).arg(tongKhongDoi);
        mauNen = "#fee2e2";
        mauVien = "#dc2626";
    } else {
        thongBao = LanguageManager::instance().getText("MSG_TREND_STABLE")
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
    html += "<h1>" + LanguageManager::instance().getText("HEADER_TOP10") + "</h1>";
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
    html += "<tr><th>" + LanguageManager::instance().getText("TABLE_HEADER_RANK") + 
                   "</th><th>" + LanguageManager::instance().getText("TABLE_HEADER_ISBN") + 
                   "</th><th>" + LanguageManager::instance().getText("TABLE_HEADER_TITLE") + 
                   "</th><th>" + LanguageManager::instance().getText("TABLE_HEADER_AUTHOR") + 
                   "</th><th>" + LanguageManager::instance().getText("TABLE_HEADER_LOANS") + 
                   "</th><th>" + LanguageManager::instance().getText("TABLE_HEADER_RATE") + 
                   "</th><th>" + LanguageManager::instance().getText("TABLE_HEADER_TREND") + "</th></tr>";
    
    QAbstractItemModel *m = modelTop10;
    int rows = m ? m->rowCount() : 0;
    for (int row = 0; row < rows; row++) {
        QString rankClass = (row == 0) ? "rank1" : (row == 1) ? "rank2" : (row == 2) ? "rank3" : "";
        html += QString("<tr class='%1'>").arg(rankClass);
        for (int col = 0; col < 7; col++) {
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
    
    // Use Centralized PrintHelper
    PrintHelper::inBaoCaoTuyChinh("Top 10 Sách", html, this);
}
