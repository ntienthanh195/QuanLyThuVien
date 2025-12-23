/**
 * @file WidgetMuonTraSach.cpp
 * @path src/ui/WidgetMuonTraSach.cpp
 * @description Widget quản lý mượn trả sách (Phiên bản V2). 
 * Xử lý các thao tác: tìm kiếm học viên, quét mã sách, mượn sách, trả sách, báo mất và gia hạn.
 * @project Library Management System (QuanLyThuVien)
 */
#include "ui/WidgetMuonTraSach.h"

#include <vector>
#include <algorithm>
#include "utils/LanguageManager.h"
#include "ui/dialogs/HopThoaiQuetMaVach.h"
#include "ui/dialogs/HopThoaiThanhToan.h" // Added Logic
#include "utils/FilePathManager.h"
#include "utils/AuditLogger.h"
#include "utils/XuLyChuoi.h"
#include "utils/PrintHelper.h"
#include "models/DocGia.h"
#include "models/QuanLySach.h"
#include "models/ThaoTacFile.h"
#include <QVBoxLayout>
#include <QTimer>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>
#include <QTabWidget>
#include <QDebug>
#include <QDateTime>
#include <QInputDialog>
#include <QTableView>
#include <QAbstractTableModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QSettings> // Include QSettings
#include "utils/SettingsManager.h" // Added SettingsManager
#include "utils/NgayThang.h"       // Added NgayThang

// Include responsive helpers
#include "utils/ResponsiveHelper.h"
#include "utils/EventBus.h"
#include "utils/ToastNotification.h"
#include "utils/SafeGui.h"
#include "utils/FinanceHelper.h"
#include <QKeyEvent>
#include <QApplication>
#include <QLineEdit>
#include <QTextEdit>
#include "ui/WidgetLichSuMuonTra.h"
#include <QListWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPair>

// Prefer DBBatcher snapshot API when available to avoid buffer overruns


WidgetMuonTraSach::WidgetMuonTraSach(QWidget *cha) 
    : QWidget(cha)
    , m_batchPrintTimer(nullptr)
    , docGiaHienTai(nullptr)
    , sachDangChonMuon(nullptr)
    , banSaoDangChon(nullptr) {
    thieLapGiaoDien();
    // thieLapTabTraSach(); // REMOVED: Now merged into Borrow logic

    // Realtime updates: refresh lists/details when data changes
    connect(&EventBus::instance(), &EventBus::docGiaAdded, this, [this](int){ this->hienThiDanhSachDocGia(); });
    connect(&EventBus::instance(), &EventBus::docGiaUpdated, this, [this](int){ this->hienThiDanhSachDocGia(); });
    connect(&EventBus::instance(), &EventBus::docGiaRemoved, this, [this](int){ this->hienThiDanhSachDocGia(); });

    connect(&EventBus::instance(), &EventBus::muonTraAdded, this, [this](int maThe, const QString &){
        // If current reader affected, update their borrowed list AND reader info
        if (this->docGiaHienTai && this->docGiaHienTai->data.MaThe == maThe) {
            this->capNhatBangSachDangMuon();
            this->capNhatThongTinDocGia(); // NEW: Update the info label too
        }
        this->hienThiDanhSachDocGia();
    });
    connect(&EventBus::instance(), &EventBus::muonTraUpdated, this, [this](int maThe, const QString &){
        if (this->docGiaHienTai && this->docGiaHienTai->data.MaThe == maThe) {
            this->capNhatBangSachDangMuon();
            this->capNhatThongTinDocGia(); // NEW: Update the info label too
        }
        this->hienThiDanhSachDocGia();
    });
    connect(&EventBus::instance(), &EventBus::muonTraRemoved, this, [this](int maThe, const QString &){
        if (this->docGiaHienTai && this->docGiaHienTai->data.MaThe == maThe) {
            this->capNhatBangSachDangMuon();
            this->capNhatThongTinDocGia(); // NEW: Update the info label too
        }
        this->hienThiDanhSachDocGia();
    });

    // Re-enabled: Widget-level event filter (ORIGINAL WORKING SOLUTION)
    // This catches scanner input when widget has focus
    qApp->installEventFilter(this);


    // Refresh when global data changes
    // Refresh when global data changes
    connect(&EventBus::instance(), &EventBus::dataChanged, this, [this]() {
        this->hienThiDanhSachDocGia();
        this->capNhatBangSachDangMuon();
    });


    // Language
    connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this, &WidgetMuonTraSach::capNhatNgonNgu);
    QTimer::singleShot(0, this, &WidgetMuonTraSach::capNhatNgonNgu);
}

WidgetMuonTraSach::~WidgetMuonTraSach() {
    qApp->removeEventFilter(this);
}

// onGlobalScanReceived REMOVED - not used (GlobalScannerFilter disabled)
// Scanner input now handled by eventFilter only


// Lightweight model to back the readers view (virtualized, paged)
class DocGiaTableModel : public QAbstractTableModel {
public:
    DocGiaTableModel(std::vector<PTRDG> *data, QObject *parent = nullptr)
        : QAbstractTableModel(parent), m_data(data), m_page(1), m_pageSize(10) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        if (!m_data) return 0;
        int total = static_cast<int>(m_data->size());
        int start = (m_page - 1) * m_pageSize;
        if (start >= total) return 0;
        return std::min(m_pageSize, total - start);
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return 9; // STT, MaThe, Ho, Ten, Lop, SDT, Phai, TrangThai, Sach
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!m_data || !index.isValid() || role != Qt::DisplayRole) return QVariant();
        int start = (m_page - 1) * m_pageSize;
        int absIdx = start + index.row();
        if (absIdx < 0 || absIdx >= (int)m_data->size()) return QVariant();
        PTRDG docGia = (*m_data)[absIdx];
        if (!docGia) return QVariant();

        if (role == Qt::ForegroundRole) {
            // Check for debt
            long long tongNo = TinhTongNo(docGia);
            if (tongNo > 0) return QColor("#FF5252");
        }

        switch (index.column()) {
        case 0: // STT: relative within current page for clearer UX
            return QString::number(index.row() + 1);
        case 1: return QString::number(docGia->data.MaThe);
        case 2: return QString::fromStdString(docGia->data.Ho);
        case 3: return QString::fromStdString(docGia->data.Ten);
        case 4: return QString::fromStdString(docGia->data.Lop); // Class
        case 5: return QString::fromStdString(docGia->data.SDT); // Phone
        case 6: {
            int maxSach = SettingsManager::instance().getInt("SO_SACH_MUON_TOI_DA", 3);
            return QString("%1 / %2").arg(docGia->data.soSachDangMuon).arg(maxSach);
        }
        case 7: return docGia->data.TrangThai == 1 ? LanguageManager::instance().getText("STATUS_ACTIVE") : LanguageManager::instance().getText("STATUS_LOCKED");
        case 8: {
            int soNgay = TinhSoNgayQuaHanLonNhat(docGia);
            return soNgay > 0 ? LanguageManager::instance().getText("LBL_DAYS_COUNT").arg(soNgay) : QStringLiteral("—");
        }
        default: return QVariant();
        }
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case 0: return LanguageManager::instance().getText("TABLE_HEADER_STT");
            case 1: return LanguageManager::instance().getText("TABLE_HEADER_CODE");
            case 2: return LanguageManager::instance().getText("TABLE_HEADER_LASTNAME_SHORT");
            case 3: return LanguageManager::instance().getText("TABLE_HEADER_FIRSTNAME");
            case 4: return LanguageManager::instance().getText("TABLE_HEADER_CLASS");
            case 5: return LanguageManager::instance().getText("TABLE_HEADER_PHONE");
            case 6: return LanguageManager::instance().getText("TABLE_HEADER_BORROWED");
            case 7: return LanguageManager::instance().getText("TABLE_HEADER_STATUS");
            case 8: return LanguageManager::instance().getText("TABLE_HEADER_OVERDUE");
            }
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    void refresh() { beginResetModel(); endResetModel(); }
    void setPage(int p) { if (p < 1) p = 1; if (p != m_page) { m_page = p; refresh(); } }
    void setPageSize(int s) { if (s < 1) s = 1; if (s != m_pageSize) { m_pageSize = s; refresh(); } }
    int page() const { return m_page; }
    int pageSize() const { return m_pageSize; }
    int totalRows() const { return (int)(m_data ? m_data->size() : 0); }

private:
    std::vector<PTRDG> *m_data;
    int m_page;
    int m_pageSize;
};

// Model for sách đang mượn (small DTO list)
class SachDangMuonModel : public QAbstractTableModel {
public:
    SachDangMuonModel(std::vector<ThongTinSachDangMuon_DTO> *data, QObject *parent = nullptr)
        : QAbstractTableModel(parent), m_data(data) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return m_data ? (int)m_data->size() : 0;
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return 4; // maSach, tenSach, ngayMuon, TrangThai
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!m_data || !index.isValid()) return QVariant();
        int r = index.row();
        if (r < 0 || r >= (int)m_data->size()) return QVariant();
        const ThongTinSachDangMuon_DTO &dto = (*m_data)[r];

        if (role == Qt::DisplayRole) {
            switch (index.column()) {
            case 0: return QString::fromStdString(dto.maSach);
            case 1: return QString::fromStdString(dto.tenSach);
            case 2: return QString::fromStdString(dto.ngayMuon);
            case 3: {
                 // Trạng Thái / Phạt logic
                 
                // 1. Sách Đã Trả (GD_DA_TRA)
                if (dto.trangThai == 1) {
                    if (dto.tienPhat > 0 && !dto.daThanhToan) {
                        return LanguageManager::instance().getText("STATUS_RETURNED_DEBT").arg(QLocale(QLocale::Vietnamese).toString((qint64)dto.tienPhat));
                    }
                    return LanguageManager::instance().getText("STATUS_RETURNED");
                }
                
                // 2. Sách Báo Mất - Chờ đền bù (GD_MAT_CHO_THANH_TOAN)
                if (dto.trangThai == 2) {
                     return LanguageManager::instance().getText("STATUS_LOST_DEBT").arg(QLocale(QLocale::Vietnamese).toString((qint64)dto.tienPhat));
                }

                // 3. Sách Báo Mất - Đã đền (GD_MAT_DA_THANH_TOAN)
                if (dto.trangThai == 3) {
                    return LanguageManager::instance().getText("STATUS_LOST_PAID");
                }

                // 4. Tìm thấy sách - Chờ hoàn tiền (GD_MAT_CHO_HOAN_TIEN)
                if (dto.trangThai == 4) {
                    return LanguageManager::instance().getText("STATUS_LOST_REFUND_WAIT").arg(QLocale(QLocale::Vietnamese).toString((qint64)dto.tienHoanTra));
                }

                // 5. Tìm thấy sách - Đã hoàn tiền (GD_MAT_DA_HOAN_TIEN)
                if (dto.trangThai == 5) {
                    return LanguageManager::instance().getText("STATUS_LOST_REFUND_DONE");
                }
                
                // 6. Sách Đang Mượn (GD_DANG_MUON)
                QDate nm = QDate::fromString(QString::fromStdString(dto.ngayMuon), "dd/MM/yyyy");
                if (!nm.isValid()) return "N/A";
                int maxDays = SettingsManager::instance().getInt("SO_NGAY_MUON_TOI_DA", 14); 
                
                qint64 days = nm.daysTo(QDate::currentDate());
                
                if (days > maxDays) {
                    return LanguageManager::instance().getText("STATUS_OVERDUE").arg(days - maxDays);
                } else {
                    return LanguageManager::instance().getText("STATUS_DUE_IN").arg(maxDays - days);
                }
            }
            }
        }
        if (role == Qt::FontRole) {
            QFont f;
            if (index.column() == 0 || index.column() == 1) { f.setBold(true); return f; }
        }
        if (role == Qt::ForegroundRole) {
             const ThongTinSachDangMuon_DTO &dto = (*m_data)[index.row()];
             // Debt Logic (Red)
             if (dto.trangThai == 2 || (dto.trangThai == 1 && dto.tienPhat > 0 && !dto.daThanhToan)) {
                 return QColor("#FF5252"); 
             }
             // Waiting Refund Logic (Gold/Orange)
             if (dto.trangThai == 4) {
                 return QColor("#FF9800"); 
             }
             // Settled / Refunded Logic (Green)
             if (dto.trangThai == 3 || dto.trangThai == 5 || dto.tienHoanTra > 0) {
                 return QColor("#4CAF50"); 
             }
             // Overdue Logic
             if (dto.trangThai == 0) {
                 QDate nm = QDate::fromString(QString::fromStdString(dto.ngayMuon), "dd/MM/yyyy");
                 if (nm.isValid()) {
                     int maxDays = SettingsManager::instance().getInt("SO_NGAY_MUON_TOI_DA", 14);
                     qint64 days = nm.daysTo(QDate::currentDate());
                     if (days > maxDays) return QColor("#FF5252");
                 }
             }
        }
        // [NEW] Feature: Background Color for Debt Rows (Visual Separation)
        if (role == Qt::BackgroundRole) {
             const ThongTinSachDangMuon_DTO &dto = (*m_data)[index.row()];
             // Debt Logic
             if (dto.trangThai == 2 || (dto.trangThai == 1 && dto.tienPhat > 0 && !dto.daThanhToan)) {
                 return QColor("#FFEBEE"); // Light Red
             }
             // Refund Logic
             if (dto.tienHoanTra > 0) {
                 return QColor("#E8F5E9"); // Light Green
             }
        }
        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case 0: return LanguageManager::instance().getText("TABLE_HEADER_BOOK_CODE");
            case 1: return LanguageManager::instance().getText("TABLE_HEADER_TITLE");
            case 2: return LanguageManager::instance().getText("TABLE_HEADER_DATE_BORROW");
            case 3: return LanguageManager::instance().getText("TABLE_HEADER_STATUS");
            }
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    void refresh() { beginResetModel(); endResetModel(); }

private:
    std::vector<ThongTinSachDangMuon_DTO> *m_data;
};



void WidgetMuonTraSach::thieLapGiaoDien() {
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    
    boTriChinh = new QVBoxLayout(this);
    int margin = ResponsiveHelper::getScaledMargin(30, screenSize);
    int spacing = ResponsiveHelper::getScaledSpacing(20, screenSize);
    boTriChinh->setContentsMargins(margin, margin, margin, margin);
    boTriChinh->setSpacing(spacing);
    
    // Tiêu đề với icon SVG
    QHBoxLayout *layoutTieuDe = new QHBoxLayout();
    QPushButton *iconTieuDe = new QPushButton();
    iconTieuDe->setIcon(QIcon(":/icons/exchange.svg"));
    iconTieuDe->setIconSize(QSize(28, 28));
    iconTieuDe->setFlat(true);
    iconTieuDe->setProperty("class", "IconTransparent");
    
    int titleSize = ResponsiveHelper::getScaledFontSize(28, screenSize);
    nhanTieuDeChinh = new QLabel(LanguageManager::instance().getText("HEADER_MAIN_TITLE"));
    QFont ft = nhanTieuDeChinh->font();
    ft.setPointSize(titleSize);
    ft.setBold(true);
    nhanTieuDeChinh->setFont(ft);
    nhanTieuDeChinh->setProperty("class", "PageTitle");
    
    layoutTieuDe->addWidget(iconTieuDe);
    layoutTieuDe->addWidget(nhanTieuDeChinh);
    layoutTieuDe->addStretch();
    boTriChinh->addLayout(layoutTieuDe);
    
    // Tab widget với SVG icons
    // Tab padding and font size are now controlled via QSS classes
    hopTab = new QTabWidget();
    hopTab->setProperty("class", "TabCustom");
    
    thieLapTabMuonSach();
    thieLapTabLichSu();
    
    boTriChinh->addWidget(hopTab);
    
    qDebug() << "WidgetMuonTraSach: 2-column layout with pagination applied";
}

void WidgetMuonTraSach::thieLapTabMuonSach() {
    widgetMuonSach = new QWidget();
    QHBoxLayout *boTriChinh = new QHBoxLayout(widgetMuonSach);
    boTriChinh->setContentsMargins(20, 20, 20, 20);
    boTriChinh->setSpacing(20);
    
    // === CỘT TRÁI: DANH SÁCH học viên (50%) ===
    QWidget *cotTrai = new QWidget();
    QVBoxLayout *boTriTrai = new QVBoxLayout(cotTrai);
    boTriTrai->setContentsMargins(0, 0, 0, 0);
    boTriTrai->setSpacing(15);
    
    // Unified Search Toolbar (One Line)
    QWidget *searchContainer = new QWidget();
    searchContainer->setObjectName("SearchContainer");
    QHBoxLayout *boTriTim = new QHBoxLayout(searchContainer);
    boTriTim->setContentsMargins(0, 0, 0, 0);
    boTriTim->setSpacing(10);
    
    oNhapTimDocGia = new QLineEdit();
    oNhapTimDocGia->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_SEARCH_READER"));
    oNhapTimDocGia->setMinimumHeight(40);
    oNhapTimDocGia->setProperty("class", "InputDark");
    oNhapTimDocGia->setClearButtonEnabled(true);
    
    // Nút làm mới (Icon Only)
    nutLamMoiDocGia = new QPushButton();
    nutLamMoiDocGia->setIcon(QIcon(":/icons/refresh_custom.svg")); 
    // Prefer resource path, but use local path fallback if resource not built yet during dev
    // We updated .qrc but re-running rcc might not happen without full build. 
    // Let's assume standard build flow.
    nutLamMoiDocGia->setIcon(QIcon(":/icons/refresh_custom.svg")); 
    nutLamMoiDocGia->setIconSize(QSize(24, 24)); // Slightly larger icon
    nutLamMoiDocGia->setMinimumHeight(40);
    nutLamMoiDocGia->setMinimumWidth(50); // Compact width
    nutLamMoiDocGia->setText(""); // No Text
    nutLamMoiDocGia->setProperty("class", "IconButton");
    nutLamMoiDocGia->setToolTip(LanguageManager::instance().getText("TOOLTIP_REFRESH"));
    nutLamMoiDocGia->setCursor(Qt::PointingHandCursor);
    
    // Add to Layout
    boTriTim->addWidget(oNhapTimDocGia, 1);
    boTriTim->addWidget(nutLamMoiDocGia);
    
    // ✅ CONNECT: Realtime Search (textChanged)
    connect(oNhapTimDocGia, &QLineEdit::textChanged, this, &WidgetMuonTraSach::xuLyTimKiem);
    
    boTriTrai->addWidget(searchContainer);
    
    // Bảng học viên (view + model)
    bangDocGia = new QTableView();
    modelDocGia = new DocGiaTableModel(&danhSachDocGia, this);
    bangDocGia->setModel(modelDocGia);
    
    // Configure table columns - All visible
    bangDocGia->setColumnWidth(0, 50);   // STT
    bangDocGia->setColumnWidth(1, 80);   // Mã thẻ
    bangDocGia->setColumnWidth(2, 80);  // Họ đệm
    bangDocGia->setColumnWidth(3, 100);   // Tên
    bangDocGia->setColumnHidden(4, true); // Ẩn Lớp (không quan trọng)
    bangDocGia->setColumnHidden(5, true); // Ẩn SĐT (không quan trọng)
    bangDocGia->setColumnWidth(6, 100);   // Đang mượn (Books)
    bangDocGia->setColumnWidth(7, 90);   // Trạng thái
    bangDocGia->setColumnWidth(8, 90);   // Quá hạn (Overdue)
    
    // Highlight selected row with blue color
    bangDocGia->setStyleSheet(
        "QTableView {"
        "    selection-background-color: #3b82f6;"  // Blue-500
        "    selection-color: white;"
        "}"
        "QTableView::item:selected {"
        "    background-color: #3b82f6;"
        "    color: white;"
        "}"
    );
    
    // Match row height to Quản Lý Sách
    bangDocGia->verticalHeader()->setDefaultSectionSize(45);
    // Hide the vertical header numbers to avoid duplicate numbering UX
    bangDocGia->verticalHeader()->setVisible(false);
    boTriTrai->addWidget(bangDocGia, 1);
    
    // Thông tin tổng
    nhanTongDocGia = new QLabel(LanguageManager::instance().getText("LBL_TOTAL_READERS").arg(0));
    nhanTongDocGia->setProperty("class", "SubText");
    
    boTriTrai->addWidget(nhanTongDocGia);
    
    // Legacy cancel button removed earlier
    
    // === CỘT PHẢI: THÔNG TIN CHI TIẾT (50%) ===
    QWidget *cotPhai = new QWidget();
    QVBoxLayout *boTriPhai = new QVBoxLayout(cotPhai);
    boTriPhai->setContentsMargins(0, 0, 0, 0);
    boTriPhai->setSpacing(8); // Giảm spacing
    
    // Panel thông tin học viên (COMPACT)
    nhomThongTinDocGia = new QGroupBox(LanguageManager::instance().getText("GROUP_READER_INFO"));
    nhomThongTinDocGia->setProperty("class", "GroupBoxProfile");
    nhomThongTinDocGia->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed); // Fixed vertical size
    
    QVBoxLayout *boTriDocGia = new QVBoxLayout(nhomThongTinDocGia);
    boTriDocGia->setContentsMargins(10, 10, 10, 10); // Compact margins
    
    nhanThongTinDocGia = new QLabel("<i>" + LanguageManager::instance().getText("MSG_SELECT_READER_FIRST") + "</i>");
    nhanThongTinDocGia->setProperty("class", "SelectedInfo");
    nhanThongTinDocGia->setProperty("state", "empty");
    nhanThongTinDocGia->setWordWrap(true);
    nhanThongTinDocGia->setMinimumHeight(50); // Giảm chiều cao tối thiểu đáng kể
    boTriDocGia->addWidget(nhanThongTinDocGia);
    
    boTriPhai->addWidget(nhomThongTinDocGia, 0); // Stretch 0 (Fixed)
    
    // Panel sách đang mượn (EXPAND)
    QGroupBox *nhomSachDangMuon = new QGroupBox(LanguageManager::instance().getText("GROUP_BOOKS_BORROWED"));
    nhomSachDangMuon->setProperty("class", "GroupBoxBooks");
    nhanTieuDeSachDangMuon = nhomSachDangMuon;
    
    QVBoxLayout *boTriSachDangMuon = new QVBoxLayout(nhomSachDangMuon);
    boTriSachDangMuon->setContentsMargins(5, 5, 5, 5); // Tối ưu lề
    boTriSachDangMuon->setSpacing(5);
    
    // --- INTEGRATE INPUT AREA HERE (Top of Book List) ---
    QWidget *inputContainer = new QWidget();
    QHBoxLayout *inputLayout = new QHBoxLayout(inputContainer);
    inputLayout->setContentsMargins(0,0,0,0);
    inputLayout->setSpacing(5);
    
    oNhapISBN = new QLineEdit();
    oNhapISBN->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_SCAN_BORROW"));
    oNhapISBN->setMinimumHeight(40);
    oNhapISBN->setEnabled(false);
    oNhapISBN->setProperty("class", "InputISBN");
    oNhapISBN->setClearButtonEnabled(true);
    
    connect(oNhapISBN, &QLineEdit::returnPressed, this, [this]() {
         if (oNhapISBN) {
             oNhapISBN->setFocus();
             xuLyQuetThongMinh(oNhapISBN->text());
         }
    });

    boTriSachDangMuon->addWidget(oNhapISBN);
    // ----------------------------------------------------

    // TABLE
    bangSachDangMuon = new QTableView();
    modelSachDangMuon = new SachDangMuonModel(&danhSachSachDangMuon, this);
    bangSachDangMuon->setModel(modelSachDangMuon);
    // Attributes
    bangSachDangMuon->horizontalHeader()->setStretchLastSection(true);
    bangSachDangMuon->setColumnWidth(0, 150); 
    bangSachDangMuon->setColumnWidth(1, 230);
    bangSachDangMuon->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangSachDangMuon->setSelectionMode(QAbstractItemView::ExtendedSelection);
    bangSachDangMuon->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    bangSachDangMuon->setMinimumHeight(100); // Allow shrinking to fit if needed
    bangSachDangMuon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // Expand freely
    bangSachDangMuon->setProperty("class", "InnerTable");
    bangSachDangMuon->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    boTriSachDangMuon->addWidget(bangSachDangMuon, 1); // Expand Table
    
    boTriPhai->addWidget(nhomSachDangMuon, 1); // Expand GroupBox (Stretch 1)
    
    
    // --- BOTTOM ACTION BAR (Compact Row) ---
    QWidget *actionBarRight = new QWidget();
    actionBarRight->setProperty("class", "ActionBar");
    QHBoxLayout *boTriNut = new QHBoxLayout(actionBarRight);
    boTriNut->setContentsMargins(0, 5, 0, 0);
    boTriNut->setSpacing(8);
    
    // Create Buttons
    nutXacNhanMuon = new QPushButton();
    nutXacNhanMuon->setProperty("class", "Primary");
    nutXacNhanMuon->setCursor(Qt::PointingHandCursor);
    
    nutXacNhanTra = new QPushButton();
    nutXacNhanTra->setProperty("class", "Success");
    nutXacNhanTra->setCursor(Qt::PointingHandCursor);
    
    nutBaoMat = new QPushButton();
    nutBaoMat->setProperty("class", "Danger");
    nutBaoMat->setCursor(Qt::PointingHandCursor);
    
    nutGiaHan = new QPushButton();
    nutGiaHan->setProperty("class", "Warning");
    nutGiaHan->setCursor(Qt::PointingHandCursor);
    
    nutThanhToan = new QPushButton();
    nutThanhToan->setProperty("class", "Info");
    nutThanhToan->setCursor(Qt::PointingHandCursor);
    
    nutInLaiPhieu = new QPushButton();
    nutInLaiPhieu->setProperty("class", "Secondary");
    nutInLaiPhieu->setCursor(Qt::PointingHandCursor);

    // Set Text
    nutXacNhanMuon->setText(" " + LanguageManager::instance().getText("BTN_BORROW_SHORT"));
    nutXacNhanTra->setText(" " + LanguageManager::instance().getText("BTN_RETURN_SHORT"));
    nutBaoMat->setText(" " + LanguageManager::instance().getText("BTN_LOST_SHORT"));
    nutGiaHan->setText(" " + LanguageManager::instance().getText("BTN_EXTEND_SHORT"));
    nutThanhToan->setText(" " + LanguageManager::instance().getText("BTN_PAY_SHORT"));
    nutInLaiPhieu->setText(" " + LanguageManager::instance().getText("BTN_REPRINT_SHORT"));

    // Add to Layout
    boTriNut->addWidget(nutXacNhanMuon);
    boTriNut->addWidget(nutXacNhanTra);
    boTriNut->addWidget(nutBaoMat);
    boTriNut->addWidget(nutGiaHan);
    boTriNut->addWidget(nutThanhToan);
    boTriNut->addWidget(nutInLaiPhieu);

    boTriPhai->addWidget(actionBarRight, 0); // Fixed Height Action Bar
    boTriPhai->addStretch();

    // Thêm 2 cột vào layout chính - tăng tỷ lệ cho cột trái (danh sách) để tận dụng không gian
    boTriChinh->addWidget(cotTrai, 3);
    boTriChinh->addWidget(cotPhai, 2);
    
    hopTab->addTab(widgetMuonSach, " " + LanguageManager::instance().getText("TAB_BORROW_RETURN") + " ");
    
    // Khởi tạo dữ liệu
    trangHienTai = 1;
    soDocGiaTrenTrang = 10;
    tongSoTrang = 1;
    
    // Connect signals
    connect(nutLamMoiDocGia, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyLamMoiDanhSach);
    connect(bangDocGia, &QTableView::clicked, this, [this](const QModelIndex &idx){ this->xuLyChonDocGia(idx.row(), idx.column()); });
    
    // Auto-enable borrow button when ISBN changes
    connect(oNhapISBN, &QLineEdit::textChanged, this, [this](const QString &text) {
        nutXacNhanMuon->setEnabled(!text.trimmed().isEmpty() && docGiaHienTai);
    });
    connect(nutXacNhanMuon, &QPushButton::clicked, this, [this](){ xuLyNutXacNhanMuon(false); });
    connect(nutXacNhanTra, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyNutXacNhanTra);
    connect(nutBaoMat, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyNutBaoMat);
    connect(nutGiaHan, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyNutGiaHan); // Connect logic
    connect(nutThanhToan, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyNutThanhToan);
    connect(nutInLaiPhieu, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyInLaiPhieu);
    if (bangSachDangMuon->selectionModel()) {
        connect(bangSachDangMuon->selectionModel(), &QItemSelectionModel::currentChanged,
                this, [this](const QModelIndex &current, const QModelIndex &){ this->xuLyChonSachTra(current.row(), current.column()); });
    }
    
    // Load dữ liệu ban đầu
    hienThiDanhSachDocGia();
    
    // Connect language change signal
    connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this, &WidgetMuonTraSach::capNhatNgonNgu);
}


void WidgetMuonTraSach::hienThiDanhSachDocGia() {
    // ⚡ OPTIMIZED: Fetch reader IDs from SQL (highly efficient sorting/paging)
    // and load corresponding objects from memory tree (log N). 
    // This avoids O(N) full tree dump for large databases.
    QSqlQuery q;
    q.prepare("SELECT ma_the FROM doc_gia ORDER BY ten ASC");
    
    danhSachDocGia.clear();
    if (q.exec()) {
        while (q.next()) {
            PTRDG dg = timDocGia(rootDocGia, q.value(0).toInt());
            if (dg) danhSachDocGia.push_back(dg);
        }
    }
    
    hienThiTrang();
}

void WidgetMuonTraSach::hienThiTrang() {
    // Recompute total pages and clamp current page
    tongSoTrang = (danhSachDocGia.size() + soDocGiaTrenTrang - 1) / soDocGiaTrenTrang;
    if (tongSoTrang < 1) tongSoTrang = 1;
    if (trangHienTai > tongSoTrang) trangHienTai = tongSoTrang;

    if (modelDocGia) {
        modelDocGia->setPage(trangHienTai);
        modelDocGia->setPageSize(soDocGiaTrenTrang);
        modelDocGia->refresh();
    }

    nhanTongDocGia->setText(QString("Tổng: %1 học viên").arg(danhSachDocGia.size()));
}

void WidgetMuonTraSach::xuLyTimKiem() {
    QString tuKhoa = oNhapTimDocGia->text().trimmed();
    
    if (tuKhoa.isEmpty()) {
        hienThiDanhSachDocGia();
        return;
    }
    

    
    // extern removed
    
    // Sanitize Input: Extract first sequence of digits
    QRegularExpression re("(\\d+)");
    QRegularExpressionMatch match = re.match(tuKhoa);
    if (match.hasMatch()) {
        tuKhoa = match.captured(1);
    }

    // Kiểm tra xem là mã thẻ không (số nguyên)
    bool laMaThe = false;
    tuKhoa.toInt(&laMaThe);
    
    danhSachDocGia.clear();
    
    if (laMaThe) {
        int ma = tuKhoa.toInt();
        PTRDG docGia = timDocGia(rootDocGia, ma);
        if (docGia) danhSachDocGia.push_back(docGia);
    } else {
        // ⚡ OPTIMIZED: Use SQL for fuzzy name search (O(log N) indexed)
        // vs O(N) memory tree scan.
        QSqlQuery q;
        q.prepare("SELECT ma_the FROM doc_gia WHERE (ho || ' ' || ten) LIKE :search OR ten LIKE :search ORDER BY ten ASC");
        q.bindValue(":search", "%" + tuKhoa + "%");
        
        if (q.exec()) {
            while (q.next()) {
                PTRDG dg = timDocGia(rootDocGia, q.value(0).toInt());
                if (dg) danhSachDocGia.push_back(dg);
            }
        }
    }
    
    // Cập nhật phân trang
    trangHienTai = 1;
    tongSoTrang = (danhSachDocGia.size() + soDocGiaTrenTrang - 1) / soDocGiaTrenTrang;
    if (tongSoTrang < 1) tongSoTrang = 1;
    
    hienThiTrang();

    // AUTO-SELECT Logic (UX Optimization)
    // Nếu chỉ tìm thấy 1 kết quả (thường là quét mã thẻ chính xác), tự động chọn luôn.
    if (danhSachDocGia.size() == 1) {
        bangDocGia->selectRow(0); // Highlight visually
        xuLyChonDocGia(0, 0);     // Trigger logic
        
        // Focus ngay vào ô nhập sách để mượn luôn
        if (oNhapISBN->isEnabled()) {
            // oNhapISBN->setFocus(); // DISABLE AUTO-FOCUS (Silent Mode)
        }
    }
}

void WidgetMuonTraSach::xuLyLamMoiDanhSach() {
    oNhapTimDocGia->clear();
    trangHienTai = 1;
    hienThiDanhSachDocGia();
}

void WidgetMuonTraSach::xuLyTrangTruoc() {
    if (trangHienTai > 1) {
        trangHienTai--;
        hienThiTrang();
    }
}

void WidgetMuonTraSach::xuLyTrangSau() {
    if (trangHienTai < tongSoTrang) {
        trangHienTai++;
        hienThiTrang();
    }
}

void WidgetMuonTraSach::xuLyChonDocGia(int hang, int) {
    if (hang < 0) return;
    
    int chiSoThucTe = (trangHienTai - 1) * soDocGiaTrenTrang + hang;
    if (chiSoThucTe >= (int)danhSachDocGia.size()) return;
    
    docGiaHienTai = danhSachDocGia[chiSoThucTe];
    
    // Hiển thị thông tin học viên
    // Feature 4: Calculate Total Debt
    long long tongNo = TinhTongNo(docGiaHienTai);
    QString noInfo = "";
    if (tongNo > 0) {
        noInfo = QString(" <span style='color:#FF0000; font-weight:bold'>(Nợ: %1 đ)</span>").arg(QLocale(QLocale::Vietnamese).toString(tongNo));
    }

    int maxSach = SettingsManager::instance().getInt("SO_SACH_MUON_TOI_DA", 3);
    
    // NEW COMPACT FORMAT
    QString thongTin = QString(
        "<table width='100%' cellpadding='0' cellspacing='1' style='line-height:120%'>"
        "<tr>"
        "  <td width='45%'><b>%1</b> %2</td>"
        "  <td width='55%'><b>%3</b> %4 %5</td>"
        "</tr>"
        "<tr>"
        "  <td><b>%6</b> %7</td>"
        "  <td><b>%8</b> <span style='color:%9'>%10</span></td>"
        "</tr>"
        "<tr>"
        "  <td colspan='2'><b>%11</b> <span style='color:%12'>%13/%15 %14</span>%16</td>"
        "</tr>"
        "</table>"
    )
    .arg(LanguageManager::instance().getText("LBL_CODE")) // 1
    .arg(docGiaHienTai->data.MaThe)                       // 2
    .arg(LanguageManager::instance().getText("LBL_NAME")) // 3
    .arg(QString::fromStdString(docGiaHienTai->data.Ho))  // 4
    .arg(QString::fromStdString(docGiaHienTai->data.Ten)) // 5
    .arg(LanguageManager::instance().getText("LBL_GENDER")) // 6
    .arg(docGiaHienTai->data.Phai ? LanguageManager::instance().getText("GENDER_FEMALE") : LanguageManager::instance().getText("GENDER_MALE")) // 7
    .arg(LanguageManager::instance().getText("LBL_STATUS_SHORT")) // 8
    .arg(docGiaHienTai->data.TrangThai == 1 ? "#1BC5BD" : "#F64E60") // 9
    .arg(docGiaHienTai->data.TrangThai == 1 ? LanguageManager::instance().getText("STATUS_ACTIVE") : LanguageManager::instance().getText("STATUS_LOCKED")) // 10
    .arg(LanguageManager::instance().getText("LBL_BORROWING")) // 11
    .arg(docGiaHienTai->data.soSachDangMuon >= maxSach ? "#F64E60" : "#FFA800") // 12
    .arg(docGiaHienTai->data.soSachDangMuon) // 13
    .arg(LanguageManager::instance().getText("BOOK_UNIT")) // 14
    .arg(maxSach) // 15
    .arg(noInfo); // 16
    
    SafeGui::safeSetText(nhanThongTinDocGia, thongTin);
    // Use QSS class instead of inline stylesheet for selected reader info
    nhanThongTinDocGia->setProperty("class", "SelectedInfo");
    // compute stripe (odd/even) based on absolute index (human row number)
    int humanIndex = chiSoThucTe + 1;
    QString stripe = ((humanIndex % 2) == 1) ? QString("odd") : QString("even");
    nhanThongTinDocGia->setProperty("state", "filled");
    nhanThongTinDocGia->setProperty("stripe", stripe);
    if (nhanThongTinDocGia->style()) { nhanThongTinDocGia->style()->unpolish(nhanThongTinDocGia); nhanThongTinDocGia->style()->polish(nhanThongTinDocGia); }
    nhanThongTinDocGia->update();
    
    // Load sách đang mượn
    capNhatBangSachDangMuon();
    
    // Kích hoạt các nút (nút hủy chọn đã bị loại bỏ; các điều khiển khác vẫn được bật)
    // Kiểm tra điều kiện mượn
    if (docGiaHienTai->data.TrangThai == 1 && docGiaHienTai->data.soSachDangMuon < 3) {
        oNhapISBN->setEnabled(true);
        // Focus ISBN immediately for faster workflow
        // oNhapISBN->setFocus(); // DISABLE AUTO-FOCUS (Silent Mode)
    } else {
        oNhapISBN->setEnabled(false);
    }
}

void WidgetMuonTraSach::xuLyHuyChonDocGia() {
    docGiaHienTai = nullptr;
    bangDocGia->clearSelection();
    
    SafeGui::safeSetText(nhanThongTinDocGia, LanguageManager::instance().getText("MSG_NO_READER_SELECTED"));
    nhanThongTinDocGia->setProperty("class", "SelectedInfo");
    nhanThongTinDocGia->setProperty("state", "empty");
    nhanThongTinDocGia->setProperty("stripe", QString());
    if (nhanThongTinDocGia->style()) { nhanThongTinDocGia->style()->unpolish(nhanThongTinDocGia); nhanThongTinDocGia->style()->polish(nhanThongTinDocGia); }
    nhanThongTinDocGia->update();
    
    // Clear model-backed data for borrowed books
    danhSachSachDangMuon.clear();
    if (modelSachDangMuon) { if (auto m = dynamic_cast<SachDangMuonModel*>(modelSachDangMuon)) m->refresh(); }
    int maxSach = SettingsManager::instance().getInt("SO_SACH_MUON_TOI_DA", 3);
    SafeGui::safeSetTitle(nhanTieuDeSachDangMuon, QString("SÁCH ĐANG MƯỢN (0/%1)").arg(maxSach));
    
    oNhapISBN->clear();
    oNhapISBN->setEnabled(false);
    oNhapISBN->clear();
    oNhapISBN->setEnabled(false);
    nutXacNhanMuon->setEnabled(false);
    nutXacNhanTra->setEnabled(false);
    nutBaoMat->setEnabled(false);
}

void WidgetMuonTraSach::capNhatBangSachDangMuon() {
    danhSachSachDangMuon.clear();
    if (!docGiaHienTai) {
        if (modelSachDangMuon) { if (auto m = dynamic_cast<SachDangMuonModel*>(modelSachDangMuon)) m->refresh(); }
        int maxSach = SettingsManager::instance().getInt("SO_SACH_MUON_TOI_DA", 3);
        SafeGui::safeSetTitle(nhanTieuDeSachDangMuon, QString("SÁCH ĐANG MƯỢN (0/%1)").arg(maxSach));
        return;
    }

    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;

    // Use a larger buffer to fetch history if needed
    ThongTinSachDangMuon_DTO dsSach[100]; 
    // Fetch max 100 items (should cover all active + history if implemented)
    int soSach = LayDSSachDangMuon(docGiaHienTai, dsSach, 100, dsDauSach, soLuongDauSach);

    int activeBorrowCount = 0;

    for (int i = 0; i < soSach; i++) {
        bool hienThi = false;
        
        // Use enriched DTO data - no more manual loops!
        // 1. Always show active loans (Status 0)
        if (dsSach[i].trangThai == 0) {
            hienThi = true;
            activeBorrowCount++;
        }
        // 2. Show others if they need action (Unpaid debt OR Wait for refund)
        else if ((dsSach[i].tienPhat > 0 || dsSach[i].tienHoanTra > 0) && !dsSach[i].daThanhToan) {
            hienThi = true;
        }

        if (hienThi) {
            danhSachSachDangMuon.push_back(dsSach[i]);
        }
    }

    // Use uniform row height for performance
    bangSachDangMuon->verticalHeader()->setDefaultSectionSize(40); // Tăng height chút cho thoáng

    // refresh model
    if (modelSachDangMuon) { if (auto m = dynamic_cast<SachDangMuonModel*>(modelSachDangMuon)) m->refresh(); }

    // Ensure column widths persist after refresh 
    // Auto-resize columns to fit content 
    bangSachDangMuon->setColumnWidth(0, 150); // Mã Sách
    bangSachDangMuon->setColumnWidth(1, 230); // Tên Sách
    bangSachDangMuon->setColumnWidth(3, 200); // Trạng Thái (Tăng kích thước theo yêu cầu)
    // Note: Column 0,1 are important.

    int maxSach = SettingsManager::instance().getInt("SO_SACH_MUON_TOI_DA", 3);
    QString title = LanguageManager::instance().getText("GROUP_BOOKS_BORROWED");
    nhanTieuDeSachDangMuon->setTitle(QString("%1 (%2/%3)").arg(title).arg(activeBorrowCount).arg(maxSach));
}

// NEW: Helper to update reader info label (realtime updates after borrow/return)
void WidgetMuonTraSach::capNhatThongTinDocGia() {
    if (!docGiaHienTai) return;
    
    // Refresh soSachDangMuon from tree (in case it changed)
    extern PTRDG rootDocGia;
    PTRDG fresh = timDocGia(rootDocGia, docGiaHienTai->data.MaThe);
    if (fresh) {
        docGiaHienTai = fresh; // Update pointer to latest data
    }
    
    // Feature 4: Calculate Total Debt
    long long tongNo = TinhTongNo(docGiaHienTai);
    QString noInfo = "";
    if (tongNo > 0) {
        noInfo = QString(" <span style='color:#FF0000; font-weight:bold'>%1</span>").arg(LanguageManager::instance().getText("LBL_DEBT").arg(QLocale(QLocale::Vietnamese).toString(tongNo)));
    }
    
    // Hiển thị thông tin học viên (COMPACT)
    int maxSach = SettingsManager::instance().getInt("SO_SACH_MUON_TOI_DA", 3);
    
    QString thongTin = QString(
        "<table width='100%' cellpadding='0' cellspacing='1' style='line-height:120%'>"
        "<tr>"
        "  <td width='45%'><b>%1</b> %2</td>"
        "  <td width='55%'><b>%3</b> %4 %5</td>"
        "</tr>"
        "<tr>"
        "  <td><b>%6</b> %7</td>"
        "  <td><b>%8</b> <span style='color:%9'>%10</span></td>"
        "</tr>"
        "<tr>"
        "  <td colspan='2'><b>%11</b> <span style='color:%12'>%13/%15 %14</span>%16</td>"
        "</tr>"
        "</table>"
    )
    .arg(LanguageManager::instance().getText("LBL_CODE")) // 1
    .arg(docGiaHienTai->data.MaThe)                       // 2
    .arg(LanguageManager::instance().getText("LBL_NAME")) // 3
    .arg(QString::fromStdString(docGiaHienTai->data.Ho))  // 4
    .arg(QString::fromStdString(docGiaHienTai->data.Ten)) // 5
    .arg(LanguageManager::instance().getText("LBL_GENDER")) // 6
    .arg(docGiaHienTai->data.Phai ? LanguageManager::instance().getText("GENDER_FEMALE") : LanguageManager::instance().getText("GENDER_MALE")) // 7
    .arg(LanguageManager::instance().getText("LBL_STATUS_SHORT")) // 8
    .arg(docGiaHienTai->data.TrangThai == 1 ? "#1BC5BD" : "#F64E60") // 9
    .arg(docGiaHienTai->data.TrangThai == 1 ? LanguageManager::instance().getText("STATUS_ACTIVE") : LanguageManager::instance().getText("STATUS_LOCKED")) // 10
    .arg(LanguageManager::instance().getText("LBL_BORROWING")) // 11
    .arg(docGiaHienTai->data.soSachDangMuon >= maxSach ? "#F64E60" : "#FFA800") // 12
    .arg(docGiaHienTai->data.soSachDangMuon) // 13
    .arg(LanguageManager::instance().getText("BOOK_UNIT")) // 14
    .arg(maxSach) // 15
    .arg(noInfo); // 16


    
    SafeGui::safeSetText(nhanThongTinDocGia, thongTin);
}

// Logic checkbox removed




// Helper to extract ISBN from book code (format: ISBN-Sequence)
// e.g. "978-3-16-148410-0-1" -> "978-3-16-148410-0"

QList<QPair<PTRDG, QString>> WidgetMuonTraSach::TimDocGiaDangMuonSach(const QString &input) {
    QList<QPair<PTRDG, QString>> matches;
    if (input.isEmpty()) return matches;
    
    QSqlQuery q;
    q.prepare("SELECT m.ma_the, b.ma_sach FROM muon_tra m JOIN ban_sao b ON m.ban_sao_id = b.id WHERE (b.ma_sach = ? OR b.ma_sach LIKE ?) AND (m.trang_thai = 0 OR m.trang_thai = 2)");
    q.addBindValue(input.trimmed());
    q.addBindValue(input.trimmed() + "-%");
    extern PTRDG rootDocGia;
    if (q.exec()) {
        while (q.next()) {
            PTRDG reader = timDocGia(rootDocGia, q.value(0).toInt());
            if (reader) matches.append({reader, q.value(1).toString()});
        }
    }
    return matches;
}


// Minimal Selection Dialog for ISBN Ambiguity
static QPair<PTRDG, QString> ShowAmbiguityResolver(QWidget* parent, const QList<QPair<PTRDG, QString>>& matches) {
    QDialog dialog(parent);
    dialog.setWindowTitle("Chọn học viên trả sách");
    dialog.setMinimumWidth(400);
    
    QVBoxLayout layout(&dialog);
    layout.addWidget(new QLabel("Phát hiện nhiều học viên đang mượn đầu sách này.\nVui lòng chọn người cần trả:"));
    
    QListWidget listWidget;
    for (const auto& match : matches) {
        QString info = QString("[%1] %2 %3 - Cuốn: %4")
            .arg(match.first->data.MaThe)
            .arg(QString::fromStdString(match.first->data.Ho))
            .arg(QString::fromStdString(match.first->data.Ten))
            .arg(match.second);
        QListWidgetItem* item = new QListWidgetItem(info, &listWidget);
        item->setData(Qt::UserRole, QVariant::fromValue((void*)match.first));
        item->setData(Qt::UserRole + 1, match.second);
    }
    listWidget.setCurrentRow(0);
    layout.addWidget(&listWidget);
    
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout.addWidget(&buttonBox);
    
    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        QListWidgetItem* item = listWidget.currentItem();
        if (item) {
            return {(PTRDG)item->data(Qt::UserRole).value<void*>(), item->data(Qt::UserRole+1).toString()};
        }
    }
    return {nullptr, ""};
}


void WidgetMuonTraSach::thieLapTabLichSu() {
    widgetLichSu = new QWidget();
    QVBoxLayout *boTri = new QVBoxLayout(widgetLichSu);
    boTri->setContentsMargins(20, 20, 20, 20);
    
    widgetLichSuFull = new WidgetLichSuMuonTra(widgetLichSu);
    boTri->addWidget(widgetLichSuFull);
    
    hopTab->addTab(widgetLichSu, LanguageManager::instance().getText("TAB_HISTORY"));
}

void WidgetMuonTraSach::lamMoi() {
    // SMART REFRESH: Preserve current selection if possible
    int currentID = -1;
    if (docGiaHienTai) {
        currentID = docGiaHienTai->data.MaThe;
    }

    // Refresh History Tab if exists
    if (widgetLichSuFull) widgetLichSuFull->lamMoi();

    // 1. Refresh Reader List (updates Model & Vector)
    hienThiDanhSachDocGia();
    
    // 2. Re-select if possible
    if (currentID != -1) {
        bool found = false;
        int foundIndex = -1;
        
        // Search in the refreshed list
        for (int i = 0; i < (int)danhSachDocGia.size(); ++i) {
             if (danhSachDocGia[i] && danhSachDocGia[i]->data.MaThe == currentID) {
                 docGiaHienTai = danhSachDocGia[i]; // Update pointer
                 foundIndex = i;
                 found = true;
                 break;
             }
        }
        
        if (found) {
            // Update Details Panel (Right Side)
            // Re-use logic from xuLyChonDocGia but without needing table click
            // Manually trigger the display update logic
             int maxSach = SettingsManager::instance().getInt("SO_SACH_MUON_TOI_DA", 3);
    
             QString thongTin = QString(
                "<b>Mã Thẻ:</b> %1<br>"
                "<b>Họ Tên:</b> %2 %3<br>"
                "<b>Giới Tính:</b> %4<br>"
                "<b>Trạng Thái:</b> <span style='color:%5'>%6</span><br>"
                "<b>Đang Mượn:</b> <span style='color:%7'>%8/%9 cuốn</span>"
            )
            .arg(docGiaHienTai->data.MaThe)
            .arg(QString::fromStdString(docGiaHienTai->data.Ho))
            .arg(QString::fromStdString(docGiaHienTai->data.Ten))
            .arg(docGiaHienTai->data.Phai ? "Nữ" : "Nam")
            .arg(docGiaHienTai->data.TrangThai == 1 ? "green" : "red")
            .arg(docGiaHienTai->data.TrangThai == 1 ? "Hoạt động" : "Khóa")
            .arg(docGiaHienTai->data.soSachDangMuon >= maxSach ? "red" : "#2E86C1")
            .arg(docGiaHienTai->data.soSachDangMuon)
            .arg(maxSach);

            SafeGui::safeSetText(nhanThongTinDocGia, thongTin);
            SafeGui::safeSetTitle(nhanTieuDeSachDangMuon, QString("SÁCH ĐANG MƯỢN (%1/%2)").arg(docGiaHienTai->data.soSachDangMuon).arg(maxSach));
            
            // Refresh Borrowed Books Table
            capNhatBangSachDangMuon();
            
            // Restore Table Selection if visible
            int start = (trangHienTai - 1) * soDocGiaTrenTrang;
            int end = start + soDocGiaTrenTrang;
            if (foundIndex >= start && foundIndex < end) {
                int visualRow = foundIndex - start;
                bangDocGia->selectRow(visualRow);
            }
        } else {
            // Reader no longer exists
            xoaThongTinHienThi();
        }
    } else {
        // Nothing selected, clear just in case (e.g. search clear)
         xoaThongTinHienThi();
    }
}

void WidgetMuonTraSach::xoaThongTinHienThi() {
    oNhapTimDocGia->clear();
    docGiaHienTai = nullptr;
    sachDangChonMuon = nullptr;
    banSaoDangChon = nullptr;
    danhSachSachDangMuon.clear();
    if (modelSachDangMuon) { if (auto m = dynamic_cast<SachDangMuonModel*>(modelSachDangMuon)) m->refresh(); }
    SafeGui::safeSetText(nhanThongTinDocGia, "<i>" + LanguageManager::instance().getText("MSG_SELECT_READER_FIRST") + "</i>");
}

// Helper: Check if current reader has borrowed this book (Supports ISBN or Exact MaSach)
bool WidgetMuonTraSach::laSachDocGiaHienTaiDangMuon(const QString& maSach) {
    if (!docGiaHienTai) return false;
    
    QString qma = maSach.trimmed();
    if (qma.isEmpty()) return false;
    
    // Search in borrowed books list (in-memory DTOs for current reader)
    for (const auto& sach : danhSachSachDangMuon) {
        // Only check active loans
        if (sach.trangThai != 0) continue;
        
        QString itemMa = QString::fromStdString(sach.maSach);
        
        // Match Scenario 1: Exact Code (e.g. Scanned "ISBN-1" matches Borrowed "ISBN-1")
        if (itemMa == qma) return true;
        
        // Match Scenario 2: ISBN Prefix (e.g. Scanned "ISBN" matches Borrowed "ISBN-1")
        if (itemMa.startsWith(qma + "-")) return true;
    }
    return false;
}


// === BORROW/RETURN BUTTON HANDLERS ===

void WidgetMuonTraSach::xuLyNutXacNhanMuon(bool skipConfirm) {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_READER_FIRST"));
        return;
    }
    
    // Sanitize Input: Remove trailing dashes (caused by some scanners/configs)
    QString isbn = oNhapISBN->text().trimmed();
    while (isbn.endsWith('-')) {
        isbn.chop(1);
    }

    if (isbn.isEmpty()) {
        // Silent return to avoid infinite loops with scanners sending extra Enters or after clearing errors
        return;
    }
    
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    // SMART RESOLVE STRATEGY:
    // Priority 1: Check if input matches a SPECIFIC BOOK COPY (Exact Code Match) -> e.g. "978...-1"
    // Priority 2: Check if input matches a HEADER ISBN -> e.g. "978..." (Borrow ANY copy)
    // Priority 3: Try stripping to find a valid ISBN -> e.g. Input "978...-1" (User meant specific, but system treats as ISBN lookup fallback?)
    
    bool foundExactCopy = false;
    bool foundHeaderISBN = false;
    
    for (int i = 0; i < soLuongDauSach; i++) {
        if (!dsDauSach[i]) continue;
        
        // Check Header ISBN
        if (QString::fromStdString(dsDauSach[i]->ISBN) == isbn) {
            foundHeaderISBN = true;
        }
        
        // Check Copies (Deep Scan for Priority 1)
        PTRDMS p = dsDauSach[i]->dms;
        while (p != nullptr) {
            if (QString::fromStdString(p->maSach) == isbn) {
                foundExactCopy = true;
                break;
            }
            p = p->next;
        }
        if (foundExactCopy) break;
    }
    
    QString finalCodeToBorrow = isbn;
    
    // Logic Decision Tree
    if (foundExactCopy) {
        // User scanned a specific book code. Pass it EXACTLY.
        // Backend 'MuonSach' should reject if this specific copy is busy.
        finalCodeToBorrow = isbn; 
    } else if (foundHeaderISBN) {
        // User scanned an ISBN. Backend will pick any available copy.
        finalCodeToBorrow = isbn;
    } else {
        // Not found strict. Try Smart Resolve (Stripping Suffix).
        // Only strip if it looks like it has a suffix (contains '-')
         std::string sIsbn = isbn.toStdString();
         std::string stripped = TachISBNTuMaSach(sIsbn);
         QString qStripped = QString::fromStdString(stripped);
         
         if (qStripped != isbn) { 
             // Check if the STRIPPED version is a valid ISBN
             bool foundStrippedISBN = false;
             for (int i = 0; i < soLuongDauSach; i++) {
                 if (dsDauSach[i] && QString::fromStdString(dsDauSach[i]->ISBN) == qStripped) {
                     foundStrippedISBN = true;
                     break;
                 }
             }
             
             if (foundStrippedISBN) {
                 // Case: User scanned "978...-1", but that copy doesn't exist (maybe deleted?), 
                 // BUT the ISBN "978..." exists.
                 // Should we fall back to generic borrow? 
                 // The user complained about "scanning specific code but borrowing other".
                 // So we should BE CAREFUL here.
                 // If the specific code was NOT found in the loop above (foundExactCopy == false),
                 // it means that specific copy does NOT exist in the system.
                 // So falling back to ISBN is actually a feature (smart recovery), NOT a bug.
                 // The bug happens when the specific copy EXISTS (and is busy), but we ignored it. 
                 // Since we checked foundExactCopy first, this path is safe.
                 finalCodeToBorrow = qStripped;
             }
         }
    }
    
    std::string ketQua = MuonSach(docGiaHienTai, finalCodeToBorrow.toStdString(), dsDauSach, soLuongDauSach);
    
    // Check success case-insensitively
    // Check success case-insensitively (Backend returns "Thanh cong" or "Thành công")
    QString qKetQua = QString::fromStdString(ketQua);
    // FIXED: MuonSach returns "" on success. Old logic only checked for "Thanh cong" string.
    if (qKetQua.isEmpty() || qKetQua.contains("hành công", Qt::CaseInsensitive) || qKetQua.contains("Thanh cong", Qt::CaseInsensitive)) {
        // Success Logic
        QString tenSach = "";
        QString maSach = "";
        for (int i = 0; i < soLuongDauSach; i++) {
            if (!dsDauSach[i]) continue;
            
            // Find the book title based on the code we used
            // Caution: finalCodeToBorrow could be ISBN or MaSach.
            if (QString::fromStdString(dsDauSach[i]->ISBN) == finalCodeToBorrow) {
                // Borrowed by ISBN
                tenSach = QString::fromStdString(dsDauSach[i]->tenSach);
                // Find which copy was actually borrowed (status became borrowing just now?)
                // Since this is single-threaded UI, we can assume the one with DANG_MUON is likely it,
                // OR we can't easily know WHICH one was picked by MuonSach backend without return value.
                // WEAKNESS: Backend returns string "Muon sach thanh cong". Doesn't return Code.
                // We have to guess/find the first borrowed copy.
                PTRDMS p = dsDauSach[i]->dms;
                while (p != nullptr) {
                    if (p->trangThai == DANG_MUON) { 
                        maSach = QString::fromStdString(p->maSach);
                        // Is this the one for THIS user? We can't be 100% sure if multiple users borrowed same book same time?
                        // But for local app single user session, acceptable.
                         break;
                    }
                    p = p->next;
                }
                break;
            } else {
                 // Maybe finalCodeToBorrow matches a specific MaSach inside this ISBN
                 PTRDMS p = dsDauSach[i]->dms;
                 bool match = false;
                 while (p != nullptr) {
                     if (QString::fromStdString(p->maSach) == finalCodeToBorrow) {
                         tenSach = QString::fromStdString(dsDauSach[i]->tenSach);
                         maSach = finalCodeToBorrow;
                         match = true;
                         break;
                     }
                     p = p->next;
                 }
                 if (match) break;
            }
        }
        
        // Ghi log giao dịch mượn
        AuditLogger::log(AuditLogger::ACTION_BORROW, "MUON_TRA", finalCodeToBorrow, "Mượn sách: " + tenSach);
        
        // --- BATCH PRINT LOGIC ---
        // Thay vì in ngay lập tức, ta thêm vào hàng đợi và reset timer 5s
        // Nếu quét tiếp trong 5s -> Timer reset -> Chưa in
        // Nếu ngưng 5s -> Timer timeout -> In danh sách gom lại
        m_lastOperationWasBorrow = true; // Set operation type
        m_batchPrintQueueMa.append(maSach);
        m_batchPrintQueueTen.append(tenSach);
        // m_batchPrintTimer->start(5000); // 5000ms = 5 giây - DISABLED: Auto-print disabled per user request.
        
        /* 
        // OLD LOGIC: Print immediately
        QString ngayMuon = QDate::currentDate().toString("dd/MM/yyyy");
        PrintHelper::inPhieuMuonTuDong(docGiaHienTai, maSach, tenSach, ngayMuon, this);
        */
        
        // Dữ liệu đã được đồng bộ SQL bên trong hàm backend
        
        if (!skipConfirm) {
            QString successMsg = qKetQua.isEmpty() ? LanguageManager::instance().getText("MSG_BORROW_SUCCESS") : qKetQua;
            QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"), successMsg);
        }
        // Use Toast for success
        ToastNotification::show(LanguageManager::instance().getText("MSG_BORROW_SUCCESS"), ToastNotification::SUCCESS);
        emit thongBaoCapNhat(LanguageManager::instance().getText("MSG_BORROW_SUCCESS"));
        emit duLieuThayDoi(); // Realtime update Dashboard
        EventBus::instance().muonTraAdded(docGiaHienTai->data.MaThe, finalCodeToBorrow); // REALTIME UPDATE
        
        // EventBus handles the rest: 
        // muonTraAdded -> capNhatBangSachDangMuon + capNhatThongTinDocGia + hienThiDanhSachDocGia
        
        // Cập nhật lại UI controls
        oNhapISBN->clear();
        // Checked: hopXacNhanMuon removed from UI, so removing usage here to prevent crash
        // hopXacNhanMuon->setChecked(false); 
        nutXacNhanMuon->setEnabled(false);
        
        // Cập nhật thông tin chi tiết học viên (Box Phải)
        int currentRow = bangDocGia->currentIndex().row();
        if (currentRow >= 0) {
             bangDocGia->selectRow(currentRow);
             xuLyChonDocGia(currentRow, 0); 
        }
    } else {
        // FAILURE CASE
        // FIX INFINITE LOOP: Clear input immediately to prevent "Enter" from re-triggering this same error
        oNhapISBN->clear(); 
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_ERROR"), qKetQua);
    }
}

void WidgetMuonTraSach::xuLyNutXacNhanTra(bool skipConfirm) {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_READER_FIRST"));
        return;
    }
    
    int dongChon = bangSachDangMuon->currentIndex().row();
    if (dongChon < 0) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_PLEASE_SELECT_RETURN_BOOK"));
        return;
    }
    if (dongChon < 0 || dongChon >= (int)danhSachSachDangMuon.size()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_INVALID_SELECTION"));
        return;
    }

    const ThongTinSachDangMuon_DTO &dto = danhSachSachDangMuon[dongChon];
    QString maSach = QString::fromStdString(dto.maSach);
    QString tenSach = QString::fromStdString(dto.tenSach);
    
    // Tinh tien phat
    int soNgayQuaHan = tinhSoNgayQuaHan(dto.ngayMuon); // Use class member with QDate logic
    long long tienPhat = 0;
    QString canhBaoPhat = "";
    
    if (soNgayQuaHan > 0) {
        tienPhat = tinhTienPhat(soNgayQuaHan); // Use class member
        canhBaoPhat = LanguageManager::instance().getText("MSG_OVERDUE_WARNING")
                        .arg(soNgayQuaHan)
                        .arg(QLocale(QLocale::Vietnamese).toString(tienPhat));
    }

    QMessageBox::StandardButton xacNhan = QMessageBox::Yes;
    if (!skipConfirm) {
        xacNhan = QMessageBox::question(this, LanguageManager::instance().getText("TITLE_CONFIRM_RETURN"),
            LanguageManager::instance().getText("MSG_CONFIRM_RETURN_BODY")
                .arg(maSach, tenSach, canhBaoPhat),
            QMessageBox::Yes | QMessageBox::No);
    }
    
    if (xacNhan == QMessageBox::Yes) {
        extern PTRDS dsDauSach[];
        extern int soLuongDauSach;
        
        std::string ketQua = TraSach(docGiaHienTai, maSach.toStdString(), dsDauSach, soLuongDauSach);
        
        if (ketQua.empty() || ketQua.find("Thành công") != std::string::npos || ketQua.find("thành công") != std::string::npos) {
            // Ghi log giao dịch trả
            AuditLogger::log(AuditLogger::ACTION_RETURN, "MUON_TRA", maSach, "Trả sách: " + tenSach);
            
            // Xử lý phạt (Feature 4)
            if (tienPhat > 0) {
                 bool daThanhToan = HopThoaiThanhToan::hienThiThanhToan(
                     docGiaHienTai, 
                     LanguageManager::instance().getText("MSG_OVERDUE_DAYS").arg(soNgayQuaHan), 
                     tienPhat, 
                     this
                 );
                 
                 if (daThanhToan) {
                     // Gọi backend đánh dấu đã thanh toán
                     std::string err = ThanhToanPhat(docGiaHienTai, maSach.toStdString());
                     if (err.empty()) {
                         ToastNotification::show(LanguageManager::instance().getText("MSG_RETURN_PAY_SUCCESS"), ToastNotification::SUCCESS);
                     } else {
                         QMessageBox::critical(this, "Lỗi Database", QString::fromStdString(err));
                     }
                 } else {
                     ToastNotification::show(LanguageManager::instance().getText("MSG_RETURN_DEBT_RECORDED"), ToastNotification::WARNING);
                 }
                 // Cập nhật lại thông tin hiển thị (nợ phạt)
                 xuLyChonDocGia(bangDocGia->currentIndex().row(), 0);
            } else {
                 // SUCCESS NO FINE
                 ToastNotification::show(LanguageManager::instance().getText("MSG_RETURN_SUCCESS"), ToastNotification::SUCCESS);
            }
            
            // Tự động lưu file
            extern PTRDG rootDocGia;
            saveDocGia(rootDocGia);
            GhiDanhSachDauSach(FilePathManager::getDauSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
            GhiDanhMucSach(FilePathManager::getDanhMucSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
            
            emit thongBaoCapNhat(LanguageManager::instance().getText("MSG_RETURN_SUCCESS"));
            emit duLieuThayDoi(); // Realtime update Dashboard
            EventBus::instance().muonTraRemoved(docGiaHienTai->data.MaThe, maSach); // REALTIME UPDATE
            
            // EventBus handles the rest:
            // muonTraRemoved -> capNhatBangSachDangMuon + capNhatThongTinDocGia + hienThiDanhSachDocGia
            
            // Cập nhật thông tin chi tiết học viên (Box Phải & Highlight lại dòng)
            int currentRow = bangDocGia->currentIndex().row();
            if (currentRow >= 0) {
                 bangDocGia->selectRow(currentRow);
                 xuLyChonDocGia(currentRow, 0);
            }
        } else {
            // Error case
            ToastNotification::show(QString::fromStdString(ketQua), ToastNotification::ERROR);
            QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_ERROR"), QString::fromStdString(ketQua));
        }
    }
}

// Feature 4: Bao Mat Sach
void WidgetMuonTraSach::xuLyNutBaoMat() {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_READER_FIRST"));
        return;
    }
    
    int dongChon = bangSachDangMuon->currentIndex().row();
    if (dongChon < 0 || dongChon >= (int)danhSachSachDangMuon.size()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_LOST_BOOK"));
        return;
    }
    const ThongTinSachDangMuon_DTO &dto = danhSachSachDangMuon[dongChon];
    QString maSach = QString::fromStdString(dto.maSach);
    QString tenSach = QString::fromStdString(dto.tenSach);
    
    // Tính toán số liệu
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    std::string isbn = TachISBNTuMaSach(maSach.toStdString());
    PTRDS dauSach = nullptr;
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] && dsDauSach[i]->ISBN == isbn) {
            dauSach = dsDauSach[i];
            break;
        }
    }
    
    long long giaSach = dauSach ? dauSach->giaTien : 0;
    int phiPhat = SettingsManager::instance().getInt("PHI_MAT_SACH", 20000);
    long long tongDenBu = giaSach + phiPhat;
    
    // BƯỚC 1: XÁC NHẬN BÁO MẤT (Confirmation)
    // Hỏi người dùng có chắc chắn muốn báo mất không? (Cảnh báo phí)
    QMessageBox::StandardButton xacNhan = QMessageBox::question(this, LanguageManager::instance().getText("TITLE_REPORT_LOST"),
        QString("Bạn có chắc muốn báo mất sách <b>%1</b> không?<br><br>"
                "Giá sách: %2 VNĐ<br>"
                "Phí phạt: %3 VNĐ<br>"
                "<b>Tổng tiền đền bù: %4 VNĐ</b>")
            .arg(tenSach)
            .arg(QLocale(QLocale::Vietnamese).toString((qint64)giaSach))
            .arg(QLocale(QLocale::Vietnamese).toString((qint64)phiPhat))
            .arg(QLocale(QLocale::Vietnamese).toString((qint64)tongDenBu)),
        QMessageBox::Yes | QMessageBox::No);
    
    if (xacNhan != QMessageBox::Yes) return;
    
    // BƯỚC 2: THỰC HIỆN BÁO MẤT (Mặc định GHI NỢ trước)
    // Execute DB update: Status becomes LOST_DEBT (Chờ thanh toán)
    std::string ketQua = BaoMatSach(docGiaHienTai, maSach.toStdString(), dsDauSach, soLuongDauSach);
    
    if (ketQua.empty() || ketQua.find("Thành công") != std::string::npos || ketQua.find("thành công") != std::string::npos) {
        
        // Save DB
        extern PTRDG rootDocGia;
        saveDocGia(rootDocGia);
        GhiDanhSachDauSach(FilePathManager::getDauSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
        GhiDanhMucSach(FilePathManager::getDanhMucSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);

        // BƯỚC 3: MỞ HỘP THOẠI THANH TOÁN (Payment)
        // Hỏi người dùng có muốn thanh toán luôn không?
        bool daThanhToan = HopThoaiThanhToan::hienThiThanhToan(
            docGiaHienTai,
            QString("Đền bù sách mất: %1").arg(tenSach),
            tongDenBu,
            this
        );
        
        if (daThanhToan) {
            // Case A: Đã thanh toán -> Cập nhật trạng thái thành PAID
            std::string err = ThanhToanPhat(docGiaHienTai, maSach.toStdString());
            if (err.empty()) {
                saveDocGia(rootDocGia);
                ToastNotification::show(LanguageManager::instance().getText("MSG_LOST_PAY_SUCCESS"), ToastNotification::SUCCESS);
            } else {
                QMessageBox::critical(this, "Lỗi Database", QString::fromStdString(err));
            }
        } else {
            // Case B: Bấm "Hủy" ở hộp thoại thanh toán -> GIỮ NGUYÊN trạng thái NỢ (DEBT)
            // User request: hiển thị message "Khoản nợ đã được ghi nhận..."
            QMessageBox::information(this, LanguageManager::instance().getText("TITLE_DEBT_RECORDED"), 
                QString("<b>%1</b><br>"
                        "%2<br><br>"
                        "%3: <span style='color:red'>%4 VNĐ</span>")
                        .arg(LanguageManager::instance().getText("MSG_DEBT_RECORDED_HEADER"))
                        .arg(LanguageManager::instance().getText("MSG_DEBT_RECORDED_BODY"))
                        .arg(LanguageManager::instance().getText("LBL_TOTAL_DEBT"))
                        .arg(QLocale(QLocale::Vietnamese).toString((qint64)tongDenBu)));
        }
        
        // Cập nhật UI
        emit thongBaoCapNhat(LanguageManager::instance().getText("MSG_REPORT_LOST_SUCCESS"));
        emit duLieuThayDoi(); 
        capNhatBangSachDangMuon();
        hienThiDanhSachDocGia();
        
        int currentRow = bangDocGia->currentIndex().row();
        if (currentRow >= 0) {
            bangDocGia->selectRow(currentRow);
            xuLyChonDocGia(currentRow, 0); 
        }
        
    } else {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_ERROR"), QString::fromStdString(ketQua));
    }
}

// Feature 4: Thanh Toan No
void WidgetMuonTraSach::xuLyNutThanhToan() {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_READER_FIRST"));
        return;
    }
    
    int dongChon = bangSachDangMuon->currentIndex().row();
    if (dongChon < 0 || dongChon >= (int)danhSachSachDangMuon.size()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_PAY_BOOK"));
        return;
    }
    
    // Unified Finance Logic
    const ThongTinSachDangMuon_DTO &dto = danhSachSachDangMuon[dongChon];
    FinanceHelper::PaymentContext ctx = FinanceHelper::getContextFromDto(dto);
    
    if (FinanceHelper::processPaymentFlow(docGiaHienTai, ctx, this)) {
         capNhatBangSachDangMuon();
         hienThiDanhSachDocGia();
         nutThanhToan->setEnabled(false);
    }
}

// Feature 4: Gia Han Sach
void WidgetMuonTraSach::xuLyNutGiaHan() {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_READER_FIRST"));
        return;
    }
    
    int dongChon = bangSachDangMuon->currentIndex().row();
    if (dongChon < 0 || dongChon >= (int)danhSachSachDangMuon.size()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_EXTEND_BOOK"));
        return;
    }
    
    const ThongTinSachDangMuon_DTO &dto = danhSachSachDangMuon[dongChon];
    QString maSach = QString::fromStdString(dto.maSach);
    
    QMessageBox::StandardButton xacNhan = QMessageBox::question(this, LanguageManager::instance().getText("TITLE_CONFIRM_EXTEND"),
        LanguageManager::instance().getText("MSG_CONFIRM_EXTEND").arg(QString::fromStdString(dto.tenSach)),
        QMessageBox::Yes | QMessageBox::No);
        
    if (xacNhan == QMessageBox::Yes) {
        std::string ketQua = GiaHanMuon(docGiaHienTai, maSach.toStdString());
        
        if (ketQua.empty()) {
             // Dữ liệu đã được đồng bộ SQL bên trong backend
             
             QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"), LanguageManager::instance().getText("MSG_EXTEND_SUCCESS"));
             emit thongBaoCapNhat(LanguageManager::instance().getText("MSG_EXTEND_SUCCESS"));
             emit duLieuThayDoi();
             
             capNhatBangSachDangMuon();
             nutGiaHan->setEnabled(false);
        } else {
             QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_ERROR"), QString::fromStdString(ketQua));
        }
    }
}

// In lại phiếu mượn (Có hỗ trợ in gộp)
void WidgetMuonTraSach::xuLyInLaiPhieu() {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_READER_FIRST"));
        return;
    }

    QList<ThongTinSachDangMuon_DTO> danhSachIn;
    
    // 1. Kiểm tra các dòng được chọn (Extended Selection)
    if (bangSachDangMuon->selectionModel()->hasSelection()) {
        QModelIndexList selectedRows = bangSachDangMuon->selectionModel()->selectedRows();
        for (const QModelIndex& idx : selectedRows) {
            int row = idx.row();
            if (row >= 0 && row < (int)danhSachSachDangMuon.size()) {
                danhSachIn.append(danhSachSachDangMuon[row]);
            }
        }
    }
    
    // 2. Logic xử lý
    if (danhSachIn.isEmpty()) {
        // Trường hợp không chọn dòng nào -> Hỏi in TẤT CẢ
        if (danhSachSachDangMuon.empty()) {
             QMessageBox::warning(this, "Thông Báo", "học viên hiện không mượn sách nào.");
             return;
        }
        
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "In Phiếu Mượn",
            QString("Bạn chưa chọn sách nào.\nBạn có muốn in phiếu cho TẤT CẢ %1 cuốn sách đang mượn không?")
                .arg(danhSachSachDangMuon.size()),
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
             // Copy toàn bộ vector sang list
             for (const auto& item : danhSachSachDangMuon) {
                 danhSachIn.append(item);
             }
        } else {
             return; // Hủy
        }
    } else {
        // Trường hợp có chọn -> In danh sách đã chọn
        // Không cần làm gì thêm vì list đã có dữ liệu
    }
    
    // 3. Gọi hàm in
    if (!danhSachIn.isEmpty()) {
        PrintHelper::inPhieuMuonBatch(docGiaHienTai, danhSachIn, this);
    }
}



// Helper to determine overdue status
int WidgetMuonTraSach::tinhSoNgayQuaHan(const std::string &ngayMuonStr) {
    if (ngayMuonStr.empty()) return 0;
    
    QDate ngayMuon = QDate::fromString(QString::fromStdString(ngayMuonStr), "dd/MM/yyyy");
    if (!ngayMuon.isValid()) return 0;
    
    int loanPeriod = SettingsManager::instance().getInt("SO_NGAY_MUON_TOI_DA", 7);
    QDate dueDate = ngayMuon.addDays(loanPeriod);
    
    QDate today = QDate::currentDate();
    if (today > dueDate) {
        return (int)dueDate.daysTo(today);
    }
    return 0;
}

// Calculate fine based on overdue days
int WidgetMuonTraSach::tinhTienPhat(int soNgayQuaHan) {
    if (soNgayQuaHan <= 0) return 0;
    int finePerDay = SettingsManager::instance().getInt("TIEN_PHAT_MOI_NGAY", 5000);
    return soNgayQuaHan * finePerDay;
}



// Auto-focus logic for scanner using EventFilter
bool WidgetMuonTraSach::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        
        // CRITICAL FIX: Do not intercept keys if a Modal Dialog (like QMessageBox) is open!
        if (QApplication::activeModalWidget()) {
            return QWidget::eventFilter(obj, event);
        }

        // ✅ GLOBAL SCAN: Removed tab restriction. 
        // We will process scans even from Dashboard/Manage Books.

        // ✅ FIX: CHO PHÉP NHẬP LIỆU THỦ CÔNG (MANUAL TYPING)
        // Nếu focus đang ở ô nhập liệu, ta KHÔNG chặn sự kiện
        QWidget *focusW = QApplication::focusWidget();
        if (focusW && (qobject_cast<QLineEdit*>(focusW) || qobject_cast<QTextEdit*>(focusW))) {
             // Tuy nhiên, ta vẫn muốn bắt Enter để xử lý logic thông minh nếu cần
             // Nhưng các ô nhập liệu (oNhapISBN, oNhapTimDocGia) đã có signal returnPressed xử lý riêng
             // Nên ta cứ để sự kiện đi qua
             return QWidget::eventFilter(obj, event);
        }

        int k = keyEvent->key();
        
        // ENTER -> Process Scan (Global Buffer)
        if (k == Qt::Key_Return || k == Qt::Key_Enter) {
            if (!m_scanBuffer.isEmpty()) {
                QString scannedData = m_scanBuffer.trimmed();
                m_scanBuffer.clear(); // Clear immediately
                
                // Emit request to switch to this module globally
                emit requestShowMuonTra();
                
                // Process the buffered data with NEW LOGIC
                xuLyQuetThongMinh(scannedData);
                return true; // Consume Enter
            }
            return false;
        }
        
        // Accumulate characters (Prevent UI focus jumping)
        if (!keyEvent->text().isEmpty()) {
            m_scanBuffer.append(keyEvent->text());
            return true; // BLOCK QT from processing
        }
    }
    
    return QWidget::eventFilter(obj, event);
}


// Feature 4: Handle Book Selection for Renwal & Payment
void WidgetMuonTraSach::xuLyChonSachTra(int hang, int cot) {
    Q_UNUSED(cot);
    // Safety Check: If no rows or invalid index
    if (hang < 0 || hang >= (int)danhSachSachDangMuon.size()) {
        nutXacNhanTra->setEnabled(false);
        nutBaoMat->setEnabled(false);
        nutGiaHan->setEnabled(false);
        if (nutThanhToan) nutThanhToan->setEnabled(false);
        nutInLaiPhieu->setEnabled(false);
        return;
    }

    const ThongTinSachDangMuon_DTO &dto = danhSachSachDangMuon[hang];
    
    // trangThai == 1: Returned (Debt), 2: Lost (Debt)
    bool coNoCanThanhToan = (dto.trangThai == 1 || dto.trangThai == 2);
    
    // Actions for Borrowed Books vs Returned/Lost Books
    // [MODIFIED] Disable buttons instead of hiding for Debt rows (User Request)
    nutXacNhanTra->setVisible(true); // Always visible
    nutBaoMat->setVisible(true);     // Always visible
    
    nutXacNhanTra->setEnabled(!coNoCanThanhToan);
    nutBaoMat->setEnabled(!coNoCanThanhToan);
    
    // Feature Renewal: Only for currently borrowed books
    // Logic: Limit renewal count
    int currentRenewals = dto.soLanGiaHan;
    int maxRenewals = SettingsManager::instance().getInt("SO_LAN_GIA_HAN_TOI_DA", 1);
    
    nutGiaHan->setVisible(true); // Always visible
    if (!coNoCanThanhToan) {
        if (currentRenewals < maxRenewals) {
             nutGiaHan->setEnabled(true);
             nutGiaHan->setText(QString("GIA HẠN (%1/%2)").arg(currentRenewals).arg(maxRenewals));
             nutGiaHan->setProperty("class", "Primary");
        } else {
             nutGiaHan->setEnabled(false);
             nutGiaHan->setText(QString("HẾT HẠN (%1/%2)").arg(currentRenewals).arg(maxRenewals));
             nutGiaHan->setProperty("class", "Danger"); // Highlight Limit Reached
        }
    } else {
        // Debt mode: Show as disabled
        nutGiaHan->setEnabled(false);
        nutGiaHan->setText("GIA HẠN");
        nutGiaHan->setProperty("class", "Disabled");
    }
    
    // Refresh Button Style
    if(nutGiaHan->style()) { nutGiaHan->style()->unpolish(nutGiaHan); nutGiaHan->style()->polish(nutGiaHan); }

    // Feature: Payment
    // Enabled if (Returned OR Lost) AND has Debt AND not Paid
    if (nutThanhToan) {
        if (coNoCanThanhToan && dto.tienPhat > 0 && !dto.daThanhToan) {
             nutThanhToan->setEnabled(true);
             nutThanhToan->setProperty("class", "Warning");
        } else {
             nutThanhToan->setEnabled(false);
             nutThanhToan->setProperty("class", "Disabled");
        }
        if(nutThanhToan->style()) { nutThanhToan->style()->unpolish(nutThanhToan); nutThanhToan->style()->polish(nutThanhToan); }
    }
    
    // Feature: Re-print
    nutInLaiPhieu->setEnabled(true);
}

// ═══════════════════════════════════════════════════════════════
// NEW SIMPLIFIED SCANNER LOGIC IMPLEMENTATION
// ═══════════════════════════════════════════════════════════════

void WidgetMuonTraSach::xuLyQuetThongMinh(const QString& input) {
    if (input.isEmpty()) return;
    
    qDebug() << "[SMART SCAN]" << input;
    
    // Rule: < 8 chars -> Reader Code
    if (input.length() < 8) {
        xuLyQuetMaDocGia(input);
        return;
    }
    
    // Rule: >= 8 chars -> Book Code
    xuLyQuetMaSach(input);
}

void WidgetMuonTraSach::xuLyQuetMaDocGia(const QString& maThe) {
    qDebug() << "[SCAN] Mã học viên:" << maThe;
    
    // Tìm học viên
    int maTheInt = maThe.toInt();
    extern PTRDG rootDocGia;
    PTRDG dg = timDocGia(rootDocGia, maTheInt);
    
    if (!dg) {
        QMessageBox::warning(this, "Lỗi", 
            QString("Không tìm thấy học viên: %1").arg(maThe));
        return;
    }
    
    // ✅ TỰ ĐỘNG CHỌN học viên
    docGiaHienTai = dg;
    capNhatThongTinDocGia();
    capNhatBangSachDangMuon();
    
    // ✅ CHUYỂN TAB MƯỢN SÁCH
    if (hopTab->currentWidget() != widgetMuonSach) {
        hopTab->setCurrentWidget(widgetMuonSach);
    }
    
    // ✅ KIỂM TRA TRẠNG THÁI CHỜ MƯỢN (STATE MACHINE)
    if (m_currentState == STATE_WAITING_FOR_READER && !m_pendingBookCode.isEmpty()) {
        QString bookToBorrow = m_pendingBookCode;
        m_currentState = STATE_IDLE; // Reset trạng thái
        m_pendingBookCode.clear();
        
        qDebug() << "[SCAN] Đang ở trạng thái chờ -> Tự động mượn sách:" << bookToBorrow;
        ToastNotification::show(QString("Đã chọn học viên: %1. Tiếp tục mượn sách...").arg(QString::fromStdString(dg->data.Ten)), ToastNotification::SUCCESS);
        xuLyTuDongMuon(bookToBorrow);
    } else {
        ToastNotification::show(QString("Đã chọn học viên: %1").arg(QString::fromStdString(dg->data.Ten)), ToastNotification::SUCCESS);
        hienThiTrang(); // Sync table focus if possible
    }
    
    QApplication::beep();
}

void WidgetMuonTraSach::xuLyQuetMaSach(const QString& maSach) {
    TrangThaiSach trangThai = kiemTraTrangThaiSach(maSach);
    qDebug() << "[SCAN] Mã sách:" << maSach << "| Trạng thái:" << trangThai;
    
    switch (trangThai) {
        case SACH_RANH:
            qDebug() << " -> Sách rảnh. Kiểm tra mượn sách.";
            // Nếu chưa chọn học viên -> Chuyển trạng thái chờ
            if (!docGiaHienTai) {
                qDebug() << " -> Sách RẢNH nhưng chưa chọn học viên. Chờ quét thẻ...";
                m_currentState = STATE_WAITING_FOR_READER;
                m_pendingBookCode = maSach;
                ToastNotification::show("Sách sẵn sàng. Vui lòng quét thẻ học viên để mượn!", ToastNotification::INFO);
            } else {
                qDebug() << " -> Sách RẢNH. Thực hiện mượn cho học viên hiện tại.";
                ToastNotification::show("Phát hiện sách trên kệ. Đang thực hiện mượn...", ToastNotification::INFO);
                xuLyTuDongMuon(maSach);
            }
            break;
            
        case SACH_DOC_GIA_HIEN_TAI_DANG_MUON:
            qDebug() << " -> Sách đã được học viên này mượn. Thực hiện trả.";
            ToastNotification::show("Sách học viên đang mượn. Đang thực hiện trả...", ToastNotification::INFO);
            xuLyTuDongTra(maSach);
            break;
            
        case SACH_NGUOI_KHAC_DANG_MUON:
        case SACH_DANG_BAO_MAT: {
            // ✅ TỰ ĐỘNG CHUYỂN NGƯỜI & TRẢ (Yêu cầu của bạn: Chuyển sang người đang mượn dù đang chọn ai đi nữa)
            qDebug() << " -> Sách đang được mượn/báo mất bởi NGƯỜI KHÁC. Đang tìm kiếm học viên...";
            
            auto matches = TimDocGiaDangMuonSach(maSach);
            if (matches.isEmpty()) {
                 qDebug() << " -> [SELF-HEALING] Sách có trạng thái Đang Mượn/Báo Mất nhưng KHÔNG tìm thấy giao dịch nào tương ứng.";
                 
                 // TỰ ĐỘNG SỬA LỖI: Reset trạng thái sách về Trong Kho
                 QSqlQuery qFix;
                 qFix.prepare("UPDATE ban_sao SET trang_thai = 0 WHERE ma_sach = ?");
                 qFix.addBindValue(maSach);
                 if (qFix.exec()) {
                     ToastNotification::show(QString("Phát hiện lỗi dữ liệu: Sách '%1' không có người mượn. Đã tự động reset về trạng thái 'Trong Kho'.").arg(maSach), ToastNotification::WARNING);
                     
                     // Reset trạng thái hiển thị
                     if (docGiaHienTai) {
                         capNhatBangSachDangMuon(); 
                     }
                     
                     // Nếu đang ở trạng thái chờ mượn -> Thực hiện mượn lại ngay
                     if (m_pendingBookCode == maSach) {
                         xuLyTuDongMuon(maSach);
                     }
                 } else {
                     QMessageBox::warning(this, "Lỗi Nghiêm Trọng", "Dữ liệu sách bị lỗi và không thể tự động sửa. Vui lòng kiểm tra lại CSDL!");
                 }
                 break;
            }

            PTRDG reader = nullptr;
            QString resolvedMaSach = "";

            if (matches.size() == 1) {
                reader = matches[0].first;
                resolvedMaSach = matches[0].second;
            } else {
                // ✅ Resolve ambiguity (multiple readers have copies of same ISBN)
                qDebug() << " -> Ambiguity detected! Found" << matches.size() << "matches. Opening resolver...";
                auto chosen = ShowAmbiguityResolver(this, matches);
                reader = chosen.first;
                resolvedMaSach = chosen.second;
                
                if (!reader) {
                    qDebug() << " -> Resolver cancelled by user.";
                    return; // Abort if user cancelled
                }
            }
            
            if (reader) {
                qDebug() << " -> Tìm thấy học viên:" << QString::fromStdString(reader->data.Ten) << "MaThe:" << reader->data.MaThe;
                
                docGiaHienTai = reader;
                capNhatThongTinDocGia();
                capNhatBangSachDangMuon();
                hienThiTrang(); // Sync page
                
                QString msg = (trangThai == SACH_DANG_BAO_MAT) ? 
                    QString("Tìm thấy sách báo mất của %1. Tự động chuyển...").arg(QString::fromStdString(reader->data.Ten)) :
                    QString("Phát hiện học viên %1 đang mượn. Tự động chuyển...").arg(QString::fromStdString(reader->data.Ten));
                
                ToastNotification::show(msg, ToastNotification::INFO);
                
                // REDUCED DELAY: 150ms for snappier experience
                QTimer::singleShot(150, this, [this, resolvedMaSach, maSach]() {
                    xuLyTuDongTra(resolvedMaSach.isEmpty() ? maSach : resolvedMaSach);
                });
            }
            break;
        }
            
        case SACH_KHONG_TON_TAI:
            qDebug() << " -> Sách không tồn tại hoặc đã thanh lý.";
            QMessageBox::warning(this, "Lỗi", QString("Không tìm thấy sách hoặc sách đã thanh lý: %1").arg(maSach));
            break;
    }
}

WidgetMuonTraSach::TrangThaiSach WidgetMuonTraSach::kiemTraTrangThaiSach(const QString& maSach) {
    // 1. Kiểm tra ưu tiên: học viên hiện tại có đang mượn không?
    if (laSachDocGiaHienTaiDangMuon(maSach)) {
        return SACH_DOC_GIA_HIEN_TAI_DANG_MUON;
    }

    // 2. Chế độ đồng bộ SQL thực tế: Kiểm tra trực tiếp bảng ban_sao
    QSqlQuery q;
    q.prepare("SELECT trang_thai FROM ban_sao WHERE ma_sach = :ma");
    q.bindValue(":ma", maSach);
    
    if (q.exec() && q.next()) {
        int status = q.value(0).toInt();
        if (status == CHO_MUON_DUOC) return SACH_RANH;
        if (status == DANG_MUON) return SACH_NGUOI_KHAC_DANG_MUON;
        if (status == 2) return SACH_DANG_BAO_MAT; // 2: Lost/Mất
        return SACH_KHONG_TON_TAI;
    }

    // 3. Fallback: Kiểm tra theo ISBN nếu input là ISBN thuần (không có hậu tố -X)
    q.prepare("SELECT bs.trang_thai FROM ban_sao bs "
              "JOIN dau_sach ds ON bs.dau_sach_id = ds.id "
              "WHERE ds.isbn = :isbn AND bs.trang_thai = 0 LIMIT 1");
    q.bindValue(":isbn", maSach);
    if (q.exec() && q.next()) {
        return SACH_RANH;
    }

    return SACH_KHONG_TON_TAI;
}

void WidgetMuonTraSach::xuLyTuDongMuon(const QString& maSach) {
    qDebug() << "[AUTO BORROW]" << maSach;
    
    // Đảm bảo đang ở tab Mượn
    if (hopTab->currentWidget() != widgetMuonSach) {
        hopTab->setCurrentWidget(widgetMuonSach);
    }
    
    // Điền vào ô ISBN
    oNhapISBN->setText(maSach);
    // oNhapISBN->setFocus(); // DISABLE AUTO-FOCUS (User Request: Silent Scan)
    
    // ✅ TỰ ĐỘNG TRIGGER MƯỢN (KHÔNG CẦN BẤM NÚT)
    QTimer::singleShot(100, this, [this]() {
        // Kiểm tra nút còn enable không (đề phòng)
        if (nutXacNhanMuon && nutXacNhanMuon->isEnabled()) {
             xuLyNutXacNhanMuon(true); // skipConfirm = true
        }
    });
    
    QApplication::beep();
}

void WidgetMuonTraSach::xuLyTuDongTra(const QString& maSach) {
    qDebug() << "[AUTO RETURN]" << maSach;
    
    // NOTE: Logic trả sách dựa trên việc chọn dòng trong bảng "Sách Đang Mượn" (Tab Mượn)
    // Nên ta sẽ KHÔNG chuyển sang Tab Trả, mà giữ ở Tab Mượn để highlight sách cần trả.
    
    // Fill return input (just in case user switches manually)
    
    // Tìm sách trong danh sách đang mượn để select hảng
    // Logic của xuLyNutXacNhanTra dựa trên dòng được chọn trong bangSachDangMuon
    // Nhưng bảng này nằm ở tab Mượn!
    // Wait... WidgetMuonTraSach UI logic:
    // Tab Mượn has "Right Panel" with "Sách Đang Mượn" table.
    // Tab Trả has "oNhapMaSachTra".
    
    // Logic cũ của AutoReturn (xuLySmartScan): 
    // Switch to Return Tab -> Fill Box -> Trigger Return
    // Return Tab logic usually uses `oNhapMaSachTra` OR selected row depending on implementation.
    // Let's check `xuLyNutXacNhanTra` implementation.
    // ... It uses `bangSachDangMuon->currentIndex().row()` which is in BORROW tab!
    // Ah, `widgetTraSach` seems to be a separate tab, but `xuLyNutXacNhanTra` logic (line 1609) 
    // checks `bangSachDangMuon` (Borrow Tab). 
    // This implies Return is done by selecting in Borrow Tab usually?
    // OR Does Return Tab have its own logic?
    
    // Let's re-read `xuLyNutXacNhanTra`.
    // It checks `bangSachDangMuon->currentIndex().row()`.
    // So to auto-return, we MUST select the row in `bangSachDangMuon`.
    // AND keep `widgetMuonSach` (Borrow Tab) active?
    // OR does `widgetTraSach` logic handle things differently?
    
    // Re-reading `xuLyNutXacNhanTra` (line 1609):
    // It strictly uses `bangSachDangMuon` (the table in Right Panel of Borrow Tab).
    // So switching to `widgetTraSach` (Tab 2) might be VISUAL only or for manual input?
    // If we want to reuse `xuLyNutXacNhanTra`, we should probably STAY on Borrow Tab or ensure selection is valid.
    
    // HOWEVER, standard "Smart Return" usually implies showing the user the Return UI.
    // If we select the row, we are effectively in Borrow Tab context.
    
    // Let's stick to Borrow Tab for "Context" but maybe switch to Return Tab if that's what user expects?
    // Actually, if `xuLyNutXacNhanTra` relies on `bangSachDangMuon` selection, 
    // then we MUST ensure that table is visible/active or at least the Model is valid.
    
    // BETTER APPROACH:
    // 1. Find row in `bangSachDangMuon`
    // 2. Select it.
    // 3. Trigger `xuLyNutXacNhanTra`.
    // We don't necessarily need to switch tabs if `xuLyNutXacNhanTra` works on the table.
    // But if user wants to see "Return Tab", we can switch. 
    // But wait, does Return Tab have its OWN confirm button?
    // `nutTraSachNhanh` in `widgetTraSach`. 
    // Let's check `xuLyNutTraSachNhanh` (if it exists) or `xuLyNutXacNhanTra` connection.
    
    // If I look at `thieLapTabTraSach`: 
    // It has `nutTraSachNhanh`.
    // I should check what slot `nutTraSachNhanh` is connected to.
    // It is likely connected to `xuLyNutXacNhanTra` OR a specific slot using `oNhapMaSachTra`.
    
    // Since I cannot grep easily right now, I will assume a safer path:
    // 1. Select row in `bangSachDangMuon` (Borrow Tab)
    // 2. Trigger `xuLyNutXacNhanTra`.
    // This works regardless of which tab is strictly "visible" (as long as widgets exist).
    // But staying on Borrow Tab (`widgetMuonSach`) allows user to see the list.
    
    // Let's try to find the row.
    int dongChon = -1;
    for (int i = 0; i < (int)danhSachSachDangMuon.size(); i++) {
        // Compare with MaSach OR ISBN?
        // Code keeps input "978...-1", so `maSach` here is exact code.
        if (QString::fromStdString(danhSachSachDangMuon[i].maSach) == maSach) {
            dongChon = i;
            break;
        }
        // Fallback: If input was ISBN, we might match multiple?
        // But `kiemTraTrangThaiSach` returned `SACH_DOC_GIA_HIEN_TAI_DANG_MUON`
        // which implies we found A SPECIFIC COPY (if input had suffix) 
        // OR we matched ISBN and found A COPY.
        // If input is ISBN, we should pick ANY copy that is borrowed by this user?
        // But `kiemTraTrangThaiSach` loop logic for ISBN match returns `SACH_DOC_GIA_HIEN_TAI_DANG_MUON` 
        // ONLY if we found a copy borrowed by user.
        // So we should search for that copy.
        
        // Wait, `kiemTraTrangThaiSach` returns a Status, but not WHICH copy if it was loose ISBN match.
        // But `xuLyTuDongTra` takes `maSach` (input text).
        // If input is "978...", we need to find which copy of "978..." the user is borrowing.
    }
    
    // Refined Search Logic:
    if (dongChon == -1) {
        // Try loose match (ISBN)
        std::string inputStr = maSach.toStdString();
        // We need to match ISBN of the borrowed book
        // `ThongTinSachDangMuon_DTO` has `maSach` (e.g. "978...-1")
        // We check if that `maSach` starts with input (if input is ISBN)?
        // Or strictly standard ISBN parsing.
        
        for (int i = 0; i < (int)danhSachSachDangMuon.size(); i++) {
            QString itemMaSach = QString::fromStdString(danhSachSachDangMuon[i].maSach);
            // Check if itemMaSach CONTAINS input (loose) or strictly matches ISBN logic
            // Converting itemMaSach to ISBN:
             std::string itemMaSachStr = itemMaSach.toStdString();
             // Simple heuristic: Does itemMaSach start with input? (e.g. input 978... vs 978...-1)
             if (itemMaSach.startsWith(maSach)) {
                 dongChon = i;
                 break;
             }
        }
    }
    
    if (dongChon < 0) {
        QMessageBox::warning(this, "Lỗi", "Không tìm thấy sách trong danh sách đang mượn!");
        return;
    }
    
    // Ensure we are on Borrow Tab to see the selection (since table is there)
    if (hopTab->currentWidget() != widgetMuonSach) {
       hopTab->setCurrentWidget(widgetMuonSach);
    }
    
    // Select the row
    // Select the row and update Current Index (Critical for xuLyNutXacNhanTra)
    QModelIndex idx = modelSachDangMuon->index(dongChon, 0);
    bangSachDangMuon->setCurrentIndex(idx);
    bangSachDangMuon->selectionModel()->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    
    // Trigger Return
    QTimer::singleShot(100, this, [this]() {
         // Auto-confirm return
         xuLyNutXacNhanTra(true);
    });
    
    QApplication::beep();
}

void WidgetMuonTraSach::capNhatNgonNgu() {
    // 1. Tab Titles
    if(hopTab) {
        hopTab->setTabText(0, " " + LanguageManager::instance().getText("TAB_BORROW_RETURN") + " ");
        hopTab->setTabText(1, LanguageManager::instance().getText("TAB_HISTORY"));
    }
    
    // 2. Main Labels
    if(nhanTieuDeChinh) nhanTieuDeChinh->setText(LanguageManager::instance().getText("HEADER_MAIN_TITLE"));
    // if(nhanLichSuPlaceholder) deleted
    
    if(oNhapTimDocGia) oNhapTimDocGia->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_SEARCH_READER"));
    if(oNhapISBN) oNhapISBN->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_SCAN_BORROW"));
    
    if(nutLamMoiDocGia) nutLamMoiDocGia->setToolTip(LanguageManager::instance().getText("TOOLTIP_REFRESH"));

    // 4. Action Buttons (SHORT keys)
    if(nutXacNhanMuon) nutXacNhanMuon->setText(" " + LanguageManager::instance().getText("BTN_BORROW_SHORT"));
    if(nutXacNhanTra) nutXacNhanTra->setText(" " + LanguageManager::instance().getText("BTN_RETURN_SHORT"));
    if(nutBaoMat) nutBaoMat->setText(" " + LanguageManager::instance().getText("BTN_LOST_SHORT"));
    if(nutGiaHan) nutGiaHan->setText(" " + LanguageManager::instance().getText("BTN_EXTEND_SHORT"));
    if(nutThanhToan) nutThanhToan->setText(" " + LanguageManager::instance().getText("BTN_PAY_SHORT"));
    if(nutInLaiPhieu) nutInLaiPhieu->setText(" " + LanguageManager::instance().getText("BTN_REPRINT_SHORT"));
    
    // 5. Group Boxes & Dynamic Labels
    if(nhanTongDocGia) nhanTongDocGia->setText(LanguageManager::instance().getText("LBL_TOTAL_READERS").arg(danhSachDocGia.size()));
    
    if(nhomThongTinDocGia) nhomThongTinDocGia->setTitle(LanguageManager::instance().getText("GROUP_READER_INFO"));
    // Note: GROUP_BOOKS_BORROWED is handled in capNhatBangSachDangMuon() but we trigger it below.

    // 6. Refresh Data/Models (which updates headers and dynamic titles)
    QEvent e(QEvent::LanguageChange);
    if(bangDocGia) QApplication::sendEvent(bangDocGia->horizontalHeader(), &e);
    if(bangSachDangMuon) QApplication::sendEvent(bangSachDangMuon->horizontalHeader(), &e);
    
    if(modelDocGia) {
         if(auto m = dynamic_cast<DocGiaTableModel*>(modelDocGia)) m->refresh(); 
    }
    
    // Refresh Dynamic Content
    capNhatBangSachDangMuon(); // Updates Title and Table Data
    capNhatThongTinDocGia();   // Updates HTML content with localized strings
}

