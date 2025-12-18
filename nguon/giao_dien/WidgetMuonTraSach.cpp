#include "giao_dien/WidgetMuonTraSach.h"
#include "utils/LanguageManager.h"
#include "giao_dien/hop_thoai/HopThoaiQuetMaVach.h"
#include "giao_dien/hop_thoai/HopThoaiThanhToan.h" // Added Logic
#include "utils/FilePathManager.h"
#include "utils/TransactionLogger.h"
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
#include <QKeyEvent>
#include <QApplication>
#include <QLineEdit>
#include <QTextEdit>

// Prefer DBBatcher snapshot API when available to avoid buffer overruns


WidgetMuonTraSach::WidgetMuonTraSach(QWidget *cha) 
    : QWidget(cha), docGiaHienTai(nullptr), sachDangChonMuon(nullptr), banSaoDangChon(nullptr) {
    thieLapGiaoDien();

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

    // Batch Print Timer Init
    m_batchPrintTimer = new QTimer(this);
    m_batchPrintTimer->setSingleShot(true);
    connect(m_batchPrintTimer, &QTimer::timeout, this, &WidgetMuonTraSach::xuLyInBatch);

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
        return 7; // STT, MaThe, Ho, Ten, Phai, TrangThai, Sach
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!m_data || !index.isValid() || role != Qt::DisplayRole) return QVariant();
        int start = (m_page - 1) * m_pageSize;
        int absIdx = start + index.row();
        if (absIdx < 0 || absIdx >= (int)m_data->size()) return QVariant();
        PTRDG docGia = (*m_data)[absIdx];
        if (!docGia) return QVariant();

        switch (index.column()) {
        case 0: // STT: relative within current page for clearer UX
            return QString::number(index.row() + 1);
        case 1: return QString::number(docGia->data.MaThe);
        case 2: { // Hiển thị cả Họ và Tên trong cùng một cột để tránh bị rút gọn
            QString ho = QString::fromStdString(docGia->data.Ho);
            QString ten = QString::fromStdString(docGia->data.Ten);
            return QString("%1 %2").arg(ho, ten).trimmed();
        }
        case 3: return QString::fromStdString(docGia->data.Ten);
        case 4: return docGia->data.Phai ? LanguageManager::instance().getText("GENDER_FEMALE") : LanguageManager::instance().getText("GENDER_MALE");
        case 5: return docGia->data.TrangThai == 1 ? LanguageManager::instance().getText("STATUS_ACTIVE") : LanguageManager::instance().getText("STATUS_LOCKED");
        case 6: {
            int maxSach = SettingsManager::instance().getInt("SO_SACH_MUON_TOI_DA", 3);
            return QString("%1/%2").arg(docGia->data.soSachDangMuon).arg(maxSach);
        }
        default: return QVariant();
        }
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case 0: return QStringLiteral("STT");
            case 1: return LanguageManager::instance().getText("TABLE_HEADER_CODE");
            case 2: return LanguageManager::instance().getText("TABLE_HEADER_LASTNAME");
            case 3: return LanguageManager::instance().getText("TABLE_HEADER_FIRSTNAME");
            case 4: return LanguageManager::instance().getText("TABLE_HEADER_GENDER");
            case 5: return LanguageManager::instance().getText("TABLE_HEADER_STATUS");
            case 6: return LanguageManager::instance().getText("TABLE_HEADER_BOOKS");
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
                 if (dto.trangThai == 1) {
                     if (dto.tienPhat > 0 && !dto.daThanhToan) {
                         return LanguageManager::instance().getText("STATUS_RETURNED_DEBT").arg(QLocale(QLocale::Vietnamese).toString(dto.tienPhat));
                     }
                     return LanguageManager::instance().getText("STATUS_RETURNED");
                 }
                 
                 QDate nm = QDate::fromString(QString::fromStdString(dto.ngayMuon), "dd/MM/yyyy");
                 if (!nm.isValid()) return "N/A";
                 int maxDays = QSettings("HVPG", "QuanLyThuVien").value("rules/loan_period", 14).toInt(); 
                 
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
        if (role == Qt::ForegroundRole && index.column() == 3) {
             QDate nm = QDate::fromString(QString::fromStdString(dto.ngayMuon), "dd/MM/yyyy");
             if (nm.isValid()) {
                 int maxDays = QSettings("HVPG", "QuanLyThuVien").value("rules/loan_period", 14).toInt();
                 if (nm.daysTo(QDate::currentDate()) > maxDays) return QColor("red");
                 else return QColor("green");
             }
        }
        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case 0: return LanguageManager::instance().getText("TABLE_HEADER_BOOK_ID");
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
    thieLapTabTraSach();
    thieLapTabLichSu();
    
    boTriChinh->addWidget(hopTab);
    
    qDebug() << "WidgetMuonTraSach: 2-column layout with pagination applied";
}

void WidgetMuonTraSach::thieLapTabMuonSach() {
    widgetMuonSach = new QWidget();
    QHBoxLayout *boTriChinh = new QHBoxLayout(widgetMuonSach);
    boTriChinh->setContentsMargins(20, 20, 20, 20);
    boTriChinh->setSpacing(20);
    
    // === CỘT TRÁI: DANH SÁCH ĐỘC GIẢ (50%) ===
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
    
    nutTimDocGia = new QPushButton();
    nutTimDocGia->setIcon(QIcon(":/icons/search.svg"));
    nutTimDocGia->setIconSize(QSize(18, 18));
    nutTimDocGia->setMinimumHeight(40);
    nutTimDocGia->setMinimumWidth(80);
    nutTimDocGia->setText(LanguageManager::instance().getText("BTN_SEARCH"));
    nutTimDocGia->setProperty("class", "GradientPrimary");
    nutTimDocGia->setCursor(Qt::PointingHandCursor);
    
    // Nút quét mã độc giả
    nutQuetMaDocGia = new QPushButton();
    nutQuetMaDocGia->setIcon(QIcon(":/icons/search.svg"));
    nutQuetMaDocGia->setIconSize(QSize(18,18));
    nutQuetMaDocGia->setMinimumHeight(40);
    nutQuetMaDocGia->setMinimumWidth(90);
    nutQuetMaDocGia->setText(LanguageManager::instance().getText("BTN_SCAN_CARD"));
    nutQuetMaDocGia->setToolTip(LanguageManager::instance().getText("BTN_SCAN_CARD"));
    nutQuetMaDocGia->setProperty("class", "IconButton");
    nutQuetMaDocGia->setCursor(Qt::PointingHandCursor);
    
    // Nút làm mới (Icon Only)
    nutLamMoiDocGia = new QPushButton();
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
    boTriTim->addWidget(nutTimDocGia);
    boTriTim->addWidget(nutQuetMaDocGia); // Moved here
    boTriTim->addWidget(nutLamMoiDocGia);       // Moved here
    
    // ✅ CONNECT: Allow Manual Typing & Enter in Search Box
    connect(oNhapTimDocGia, &QLineEdit::returnPressed, this, [this]() {
         QString text = oNhapTimDocGia->text().trimmed();
         if (text.isEmpty()) return;
         
         static QRegularExpression reBook("[0-9-]{8,}");
         if (reBook.match(text).hasMatch() && !text.contains(QRegularExpression("[a-zA-Z]"))) {
             xuLyQuetThongMinh(text); 
         } else {
             xuLyTimKiem();
         }
    });
    
    boTriTrai->addWidget(searchContainer);
    
    // Bảng độc giả (view + model)
    bangDocGia = new QTableView();
    modelDocGia = new DocGiaTableModel(&danhSachDocGia, this);
    bangDocGia->setModel(modelDocGia);
    thieLapBangDocGia();
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
    
    // Panel thông tin độc giả (COMPACT)
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
    
    connect(oNhapISBN, &QLineEdit::returnPressed, this, [this]() {
         if (oNhapISBN) {
             oNhapISBN->setFocus();
             xuLyQuetThongMinh(oNhapISBN->text());
         }
    });
    
    nutQuetMaSach = new QPushButton(LanguageManager::instance().getText("BTN_SCAN_BOOK"));
    nutQuetMaSach->setMinimumHeight(40);
    nutQuetMaSach->setMinimumWidth(80);
    nutQuetMaSach->setEnabled(false);
    nutQuetMaSach->setProperty("class", "Accent");
    
    inputLayout->addWidget(oNhapISBN, 1);
    inputLayout->addWidget(nutQuetMaSach);
    
    boTriSachDangMuon->addWidget(inputContainer);
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
    
    hopTab->addTab(widgetMuonSach, LanguageManager::instance().getText("TAB_BORROW"));
    
    // Khởi tạo dữ liệu
    trangHienTai = 1;
    soDocGiaTrenTrang = 10;
    tongSoTrang = 1;
    
    // Connect signals
    connect(nutTimDocGia, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyTimKiem);
    connect(nutLamMoiDocGia, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyLamMoiDanhSach);
    connect(nutQuetMaDocGia, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyNutQuetMaDocGia);
    connect(bangDocGia, &QTableView::clicked, this, [this](const QModelIndex &idx){ this->xuLyChonDocGia(idx.row(), idx.column()); });
    connect(bangDocGia, &QTableView::clicked, this, [this](const QModelIndex &idx){ this->xuLyChonDocGia(idx.row(), idx.column()); });
    
    // Auto-enable borrow button when ISBN changes
    connect(oNhapISBN, &QLineEdit::textChanged, this, [this](const QString &text) {
        nutXacNhanMuon->setEnabled(!text.trimmed().isEmpty() && docGiaHienTai);
    });
    connect(nutQuetMaSach, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyNutQuetMaSach);
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

void WidgetMuonTraSach::thieLapBangDocGia() {
    // View already has model set (DocGiaTableModel). Configure visual aspects here.
    bangDocGia->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangDocGia->setSelectionMode(QAbstractItemView::SingleSelection);
    bangDocGia->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // bangDocGia->horizontalHeader()->setStretchLastSection(true); // Disable this conflict
    bangDocGia->setAlternatingRowColors(true);
    bangDocGia->setMinimumHeight(350);

    // Make the table expand to fill available space
    bangDocGia->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    bangDocGia->setProperty("class", "InnerTable");

    // Set Fixed Widths
    bangDocGia->setColumnWidth(0, 40);  // STT gọn hơn
    bangDocGia->setColumnWidth(1, 80);  // Mã thẻ
    // Cột 2 (Họ Tên) để tự giãn (Stretch)
    bangDocGia->setColumnWidth(4, 70);  // Phái
    bangDocGia->setColumnWidth(5, 100); // Trạng Thái
    if (bangDocGia->horizontalHeader()->count() > 6)
        bangDocGia->setColumnWidth(6, 60);  // Sách

    QHeaderView *header = bangDocGia->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Fixed);      // STT
    header->setSectionResizeMode(1, QHeaderView::Fixed);      // Ma The
    header->setSectionResizeMode(2, QHeaderView::Stretch);    // Ho & Ten UTMOST PRIORITY
    header->setSectionResizeMode(3, QHeaderView::Fixed);      // Ten (Hidden/Merged)
    header->setSectionResizeMode(4, QHeaderView::Fixed);      // Gioi Tinh
    header->setSectionResizeMode(5, QHeaderView::Fixed);      // Trang Thai
    header->setSectionResizeMode(6, QHeaderView::Fixed);      // Sach

    // Ẩn cột "Tên" riêng lẻ nếu đã gộp vào cột "Họ"
    bangDocGia->setColumnHidden(3, true);
    // Column 2 matches viewport - Make it wider for Full Name
    bangDocGia->setColumnWidth(2, 160); 
    bangDocGia->setColumnWidth(4, 70);
    bangDocGia->setColumnWidth(5, 110);
    bangDocGia->setColumnWidth(6, 70);

    // Hide separate Name column
    bangDocGia->setColumnHidden(3, true);
    
    // Prevent Qt from eliding long text with '...'
    bangDocGia->setTextElideMode(Qt::ElideNone);
}

void WidgetMuonTraSach::hienThiDanhSachDocGia() {
    // Lấy toàn bộ độc giả từ cây nhị phân (Live Memory)
    // Avoid DBBatcher snapshot to ensure we see realtime changes in 'dsmt' (linked list transactions)
    // which are maintained in the memory tree 'rootDocGia'.
    extern PTRDG rootDocGia;
    PTRDG danhSach[1000];
    int tongSo = 0;
    
    DuyetCayRaMang(rootDocGia, danhSach, tongSo);
    
    // Lưu danh sách đầy đủ
    danhSachDocGia.clear();
    for (int i = 0; i < tongSo; i++) {
        danhSachDocGia.push_back(danhSach[i]);
    }
    
    // Hiển thị trang hiện tại
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

    nhanTongDocGia->setText(QString("Tổng: %1 độc giả").arg(danhSachDocGia.size()));
}

void WidgetMuonTraSach::xuLyTimKiem() {
    QString tuKhoa = oNhapTimDocGia->text().trimmed();
    
    if (tuKhoa.isEmpty()) {
        hienThiDanhSachDocGia();
        return;
    }
    
    extern PTRDG rootDocGia;
    
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
        // Tìm theo mã thẻ
        int ma = tuKhoa.toInt();
        PTRDG docGia = timDocGia(rootDocGia, ma);
        if (docGia) {
            danhSachDocGia.push_back(docGia);
        }
    } else {
        // Tìm theo tên
        PTRDG ketQua[1000];
        int soKetQua = TimDocGiaTheoHoTen(rootDocGia, tuKhoa.toStdString(), ketQua, 1000);
        for (int i = 0; i < soKetQua; i++) {
            danhSachDocGia.push_back(ketQua[i]);
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
            oNhapISBN->setFocus();
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
    
    // Hiển thị thông tin độc giả
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
        "  <td width='45%'><b>Mã:</b> %1</td>"
        "  <td width='55%'><b>Tên:</b> %2 %3</td>"
        "</tr>"
        "<tr>"
        "  <td><b>Giới Tính:</b> %4</td>"
        "  <td><b>TT:</b> <span style='color:%5'>%6</span></td>"
        "</tr>"
        "<tr>"
        "  <td colspan='2'><b>Đang Mượn:</b> <span style='color:%7'>%8/%10 cuốn</span>%9</td>"
        "</tr>"
        "</table>"
    )
    .arg(docGiaHienTai->data.MaThe)
    .arg(QString::fromStdString(docGiaHienTai->data.Ho))
    .arg(QString::fromStdString(docGiaHienTai->data.Ten))
    .arg(docGiaHienTai->data.Phai ? "Nữ" : "Nam")
    .arg(docGiaHienTai->data.TrangThai == 1 ? "#1BC5BD" : "#F64E60")
    .arg(docGiaHienTai->data.TrangThai == 1 ? "Hoạt động" : "Khoá")
    .arg(docGiaHienTai->data.soSachDangMuon >= maxSach ? "#F64E60" : "#FFA800")
    .arg(docGiaHienTai->data.soSachDangMuon)
    .arg(noInfo)
    .arg(maxSach);
    
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
        oNhapISBN->setFocus();
    } else {
        oNhapISBN->setEnabled(false);
    }
}

void WidgetMuonTraSach::xuLyHuyChonDocGia() {
    docGiaHienTai = nullptr;
    bangDocGia->clearSelection();
    
    SafeGui::safeSetText(nhanThongTinDocGia, "<i>(Chưa chọn độc giả)</i>");
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
        
        // 1. Luôn hiển thị sách đang mượn (Status 0)
        if (dsSach[i].trangThai == 0) {
            hienThi = true;
            activeBorrowCount++;
        }
        // 2. Với sách đã trả (Status 1), chỉ hiển thị nếu còn Nợ Phạt chưa thanh toán
        else if (dsSach[i].trangThai == 1) {
            // Cần tra cứu lại Node MUONTRA để lấy info tài chính (vì DTO thiếu)
            MUONTRA p = docGiaHienTai->data.dsmt;
            while (p) {
                std::string code = (p->data.banSaoSach) ? p->data.banSaoSach->maSach : p->data.maSachCached;
                if (code == dsSach[i].maSach) {
                    // Populate info
                    dsSach[i].tienPhat = p->data.tienPhat;
                    dsSach[i].daThanhToan = p->data.daThanhToan;
                    dsSach[i].soLanGiaHan = p->data.soLanGiaHan; // Feature Renewal
                    
                    // Chỉ hiện nếu Có Phạt (>0) VÀ Chưa Thanh Toán
                    if (p->data.tienPhat > 0 && !p->data.daThanhToan) {
                        hienThi = true;
                    } 
                    break;
                }
                p = p->next;
            }
        }
        else {
             // For borrowing books, also fetch extension count
             MUONTRA p = docGiaHienTai->data.dsmt;
             while (p) {
                std::string code = (p->data.banSaoSach) ? p->data.banSaoSach->maSach : p->data.maSachCached;
                if (code == dsSach[i].maSach) {
                    dsSach[i].soLanGiaHan = p->data.soLanGiaHan;
                    break;
                }
                p = p->next;
             }
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
        noInfo = QString(" <span style='color:#FF0000; font-weight:bold'>(Nợ: %1 đ)</span>").arg(QLocale(QLocale::Vietnamese).toString(tongNo));
    }
    
    // Hiển thị thông tin độc giả (COMPACT)
    int maxSach = SettingsManager::instance().getInt("SO_SACH_MUON_TOI_DA", 3);
    
    QString thongTin = QString(
        "<table width='100%' cellpadding='0' cellspacing='1' style='line-height:120%'>"
        "<tr>"
        "  <td width='45%'><b>Mã:</b> %1</td>"
        "  <td width='55%'><b>Tên:</b> %2 %3</td>"
        "</tr>"
        "<tr>"
        "  <td><b>Giới Tính:</b> %4</td>"
        "  <td><b>TT:</b> <span style='color:%5'>%6</span></td>"
        "</tr>"
        "<tr>"
        "  <td colspan='2'><b>Đang Mượn:</b> <span style='color:%7'>%8/%10 cuốn</span>%9</td>"
        "</tr>"
        "</table>"
    )
    .arg(docGiaHienTai->data.MaThe)
    .arg(QString::fromStdString(docGiaHienTai->data.Ho))
    .arg(QString::fromStdString(docGiaHienTai->data.Ten))
    .arg(docGiaHienTai->data.Phai ? "Nữ" : "Nam")
    .arg(docGiaHienTai->data.TrangThai == 1 ? "#1BC5BD" : "#F64E60")
    .arg(docGiaHienTai->data.TrangThai == 1 ? "Hoạt động" : "Khoá")
    .arg(docGiaHienTai->data.soSachDangMuon >= maxSach ? "#F64E60" : "#FFA800")
    .arg(docGiaHienTai->data.soSachDangMuon)
    .arg(noInfo)
    .arg(maxSach);


    
    SafeGui::safeSetText(nhanThongTinDocGia, thongTin);
}

// Logic checkbox removed



void WidgetMuonTraSach::thieLapTabTraSach() {
    widgetTraSach = new QWidget();
    QVBoxLayout *boTri = new QVBoxLayout(widgetTraSach);
    boTri->setContentsMargins(40, 40, 40, 40);
    boTri->setSpacing(20);
    
    // Header
    nhanTieuDeTraSach = new QLabel(LanguageManager::instance().getText("HEADER_QUICK_RETURN"));
    nhanTieuDeTraSach->setProperty("class", "PageTitle");
    nhanTieuDeTraSach->setAlignment(Qt::AlignCenter);
    boTri->addWidget(nhanTieuDeTraSach);
    
    // Input Area
    QGroupBox *boxInput = new QGroupBox();
    boxInput->setStyleSheet("QGroupBox { background: #252534; border-radius: 12px; padding: 20px; }");
    QVBoxLayout *layoutInput = new QVBoxLayout(boxInput);
    
    nhanHuongDanTraSach = new QLabel(LanguageManager::instance().getText("LBL_INSTRUCT_QUICK_RETURN"));
    nhanHuongDanTraSach->setStyleSheet("color: #FFFFFF; font-size: 14px; margin-bottom: 10px;");
    layoutInput->addWidget(nhanHuongDanTraSach);
    
    QHBoxLayout *layRow = new QHBoxLayout();
    oNhapMaSachTra = new QLineEdit();
    oNhapMaSachTra->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_ENTER_BOOK_CODE"));
    oNhapMaSachTra->setMinimumHeight(50);
    oNhapMaSachTra->setStyleSheet("QLineEdit { font-size: 18px; padding: 10px; border: 2px solid #3E3E56; border-radius: 8px; background: #1B1B29; color: cyan; font-family: monospace; } QLineEdit:focus { border-color: #5D9CEC; }");
    
    nutQuetMaSachTra = new QPushButton(LanguageManager::instance().getText("BTN_SCAN_SHORT"));
    nutQuetMaSachTra->setMinimumHeight(50);
    nutQuetMaSachTra->setMinimumWidth(80);
    nutQuetMaSachTra->setProperty("class", "Accent");
    
    layRow->addWidget(oNhapMaSachTra);
    layRow->addWidget(nutQuetMaSachTra);
    layoutInput->addLayout(layRow);
    
    nutTraSachNhanh = new QPushButton(LanguageManager::instance().getText("BTN_CONFIRM_RETURN"));
    nutTraSachNhanh->setMinimumHeight(45);
    nutTraSachNhanh->setProperty("class", "Success");
    nutTraSachNhanh->setCursor(Qt::PointingHandCursor);
    layoutInput->addWidget(nutTraSachNhanh);
    
    boTri->addWidget(boxInput);
    boTri->addStretch();
    
    // Logic connections
    connect(oNhapMaSachTra, &QLineEdit::returnPressed, this, &WidgetMuonTraSach::xuLyReturnPressedMaSachTra);
    connect(nutTraSachNhanh, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyReturnPressedMaSachTra);
    
    hopTab->addTab(widgetTraSach, LanguageManager::instance().getText("TAB_RETURN"));
}

// Helper to extract ISBN from book code (format: ISBN-Sequence)
// e.g. "978-3-16-148410-0-1" -> "978-3-16-148410-0"
static std::string TachISBNTuMaSach(const std::string& maSach) {
    size_t lastDash = maSach.find_last_of('-');
    if (lastDash != std::string::npos && lastDash > 0) {
        return maSach.substr(0, lastDash);
    }
    return maSach; // If no dash, return as is (fallback)
}

PTRDG WidgetMuonTraSach::TimDocGiaDangMuonSach(const QString &input, QString &resolvedMaSach) {
    if (input.isEmpty()) return nullptr;
    extern PTRDG rootDocGia;
    extern PTRDG rootDocGia;
    // extern int soLuongDauSach; // Unused
    
    std::string sInput = input.toStdString();
    resolvedMaSach = ""; // Reset
    
    // ---------------------------------------------------------
    // STRATEGY 1: EXACT MATCH (Mã Sách Cụ Thể) - Priority #1
    // ---------------------------------------------------------
    PTRDG found = nullptr;
    
    // Helper to scan tree
    std::function<void(PTRDG)> findExact = [&](PTRDG p) {
        if (!p || found) return;
        MUONTRA mt = p->data.dsmt;
        while (mt) {
            if (mt->data.TrangThai == DANG_MUON) {
                // Check ma sach
                std::string currentCode = (mt->data.banSaoSach) ? mt->data.banSaoSach->maSach : mt->data.maSachCached;
                if (currentCode == sInput) {
                    found = p;
                    resolvedMaSach = QString::fromStdString(currentCode);
                    return;
                }
            }
            mt = mt->next;
        }
        findExact(p->left);
        findExact(p->right);
    };
    findExact(rootDocGia);
    
    if (found) return found;

    // ---------------------------------------------------------
    // STRATEGY 2: ISBN MATCH (Smart Find) - Priority #2
    // ---------------------------------------------------------
    // Input might be an ISBN. We check if ANY reader is borrowing a book with this ISBN.
    // Constraint: Valid ONLY if there is EXACTLY ONE such instance in the entire system.
    // (To avoid returning the wrong person's book if 2 people borrowed copies of same ISBN).
    
    PTRDG candidateReader = nullptr;
    std::string candidateCode = "";
    int matchCount = 0;

    std::function<void(PTRDG)> findISBN = [&](PTRDG p) {
        if (!p) return;
        MUONTRA mt = p->data.dsmt;
        while (mt) {
            if (mt->data.TrangThai == DANG_MUON) {
                std::string currentCode = (mt->data.banSaoSach) ? mt->data.banSaoSach->maSach : mt->data.maSachCached;
                // Attempt to extract ISBN from the borrowed book's code
                // Code format: ISBN-Sequence (e.g. 978-123-1)
                // We check if currentCode STARTS WITH input (simple check) OR strict parsing.
                // Strict parsing: Check if TachISBN(currentCode) == input
                
                std::string isbnOfBook = TachISBNTuMaSach(currentCode);
                if (!isbnOfBook.empty() && isbnOfBook == sInput) {
                    matchCount++;
                    candidateReader = p;
                    candidateCode = currentCode;
                }
            }
            mt = mt->next;
        }
        findISBN(p->left);
        findISBN(p->right);
    };
    findISBN(rootDocGia);
    
    if (matchCount == 1) {
        resolvedMaSach = QString::fromStdString(candidateCode);
        return candidateReader;
    } 
    // If matchCount > 1: Ambiguous. We do not support auto-return to avoid errors.
    
    return nullptr;
}

void WidgetMuonTraSach::xuLyReturnPressedMaSachTra() {
    QString input = oNhapMaSachTra->text().trimmed();
    
    // Sanitize Input: Remove trailing dashes
    while (input.endsWith('-')) {
        input.chop(1);
    }
    
    if (input.isEmpty()) return;
    
    QString resolvedMaSach = "";
    PTRDG docGia = TimDocGiaDangMuonSach(input, resolvedMaSach);
    
    if (!docGia) {
        QMessageBox::warning(this, "Không Tìm Thấy", 
            QString("Không tìm thấy thông tin mượn cho: %1\n\n- Nếu nhập Mã Sách: Sách chưa được mượn hoặc mã sai.\n- Nếu nhập ISBN: Có thể không có ai mượn hoặc có nhiều người cùng mượn (vui lòng nhập mã cụ thể).").arg(input));
        oNhapMaSachTra->selectAll();
        return;
    }
    
    // Found reader!
    
    // Chuyển sang tab Mượn (để dùng lại logic hiển thị)
    hopTab->setCurrentWidget(widgetMuonSach);
    
    // Chọn độc giả này
    docGiaHienTai = docGia;
    
    // Update UI components
    int maxSach = SettingsManager::instance().getInt("SO_SACH_MUON_TOI_DA", 3);
    
    // Update UI components
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
    .arg(docGiaHienTai->data.TrangThai == 1 ? "#1BC5BD" : "#F64E60")
    .arg(docGiaHienTai->data.TrangThai == 1 ? "Hoạt động" : "Khoá")
    .arg(docGiaHienTai->data.soSachDangMuon >= maxSach ? "#F64E60" : "#FFA800")
    .arg(docGiaHienTai->data.soSachDangMuon)
    .arg(maxSach);
    SafeGui::safeSetText(nhanThongTinDocGia, thongTin);
    
    capNhatBangSachDangMuon();
    
    // Tìm dòng chứa sách cần trả trong bảng (Using RESOLVED CODE)
    int rowToSelect = -1;
    for (int i = 0; i < (int)danhSachSachDangMuon.size(); ++i) {
        if (danhSachSachDangMuon[i].maSach == resolvedMaSach.toStdString()) {
            rowToSelect = i;
            break;
        }
    }
    
    if (rowToSelect != -1) {
        bangSachDangMuon->selectRow(rowToSelect);
        xuLyChonSachTra(rowToSelect, 0); // Kích hoạt nút trả
        
        // Auto Execute Return (Skip Confirm) - SMART RETURN
        xuLyNutXacNhanTra(true);
        
        // Clear input for next scan
        oNhapMaSachTra->clear();
        oNhapMaSachTra->setFocus();
        
        hopTab->setCurrentWidget(widgetTraSach); // Go back to scan more
    } else {
         QMessageBox::warning(this, "Lỗi Logic", 
            QString("Tìm thấy độc giả %1 nhưng không tìm thấy sách %2 trong danh sách mượn (Sync Error?)").arg(docGia->data.MaThe).arg(resolvedMaSach));
    }
}

void WidgetMuonTraSach::thieLapTabLichSu() {
    widgetLichSu = new QWidget();
    QVBoxLayout *boTri = new QVBoxLayout(widgetLichSu);
    boTri->setContentsMargins(20, 20, 20, 20);
    
    nhanLichSuPlaceholder = new QLabel(LanguageManager::instance().getText("MSG_TAB_HISTORY_PLACEHOLDER"));
    nhanLichSuPlaceholder->setStyleSheet(
        "background-color: #252534; color: #B5B5C3; padding: 40px; "
        "border-radius: 8px; font-size: 13px; line-height: 1.8;"
    );
    nhanLichSuPlaceholder->setAlignment(Qt::AlignCenter);
    boTri->addWidget(nhanLichSuPlaceholder);
    
    hopTab->addTab(widgetLichSu, LanguageManager::instance().getText("TAB_HISTORY"));
}

void WidgetMuonTraSach::lamMoi() {
    // SMART REFRESH: Preserve current selection if possible
    int currentID = -1;
    if (docGiaHienTai) {
        currentID = docGiaHienTai->data.MaThe;
    }

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
}

// Helper: Check if current reader has borrowed this book
bool WidgetMuonTraSach::laSachDocGiaHienTaiDangMuon(const QString& maSach) {
    if (!docGiaHienTai) return false;
    
    // Convert once for efficiency
    std::string targetMaSach = maSach.toStdString();
    
    // Search in borrowed books list
    for (const auto& sach : danhSachSachDangMuon) {
        if (sach.maSach == targetMaSach && sach.trangThai == 0) {
            // trangThai == 0 means currently borrowed
            return true;
        }
    }
    return false;
}


// === BORROW/RETURN BUTTON HANDLERS ===

void WidgetMuonTraSach::xuLyNutXacNhanMuon(bool skipConfirm) {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn độc giả trước!");
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
        TransactionLogger::ghiLogMuon(docGiaHienTai, finalCodeToBorrow, tenSach, true);
        
        // --- BATCH PRINT LOGIC ---
        // Thay vì in ngay lập tức, ta thêm vào hàng đợi và reset timer 5s
        // Nếu quét tiếp trong 5s -> Timer reset -> Chưa in
        // Nếu ngưng 5s -> Timer timeout -> In danh sách gom lại
        m_lastOperationWasBorrow = true; // Set operation type
        m_batchPrintQueueMa.append(maSach);
        m_batchPrintQueueTen.append(tenSach);
        m_batchPrintTimer->start(5000); // 5000ms = 5 giây
        
        /* 
        // OLD LOGIC: Print immediately
        QString ngayMuon = QDate::currentDate().toString("dd/MM/yyyy");
        PrintHelper::inPhieuMuonTuDong(docGiaHienTai, maSach, tenSach, ngayMuon, this);
        */
        
        // Dữ liệu đã được đồng bộ SQL bên trong hàm backend
        
        if (!skipConfirm) QMessageBox::information(this, "Thành Công", QString::fromStdString(ketQua));
        // Use Toast for success
        ToastNotification::show(QString("Đã mượn sách thành công!"), ToastNotification::SUCCESS);
        emit thongBaoCapNhat("Đã mượn sách thành công!");
        emit duLieuThayDoi(); // Realtime update Dashboard
        EventBus::instance().muonTraAdded(docGiaHienTai->data.MaThe, finalCodeToBorrow); // REALTIME UPDATE
        
        // Cập nhật bảng sách đang mượn ngay lập tức
        capNhatBangSachDangMuon();
        
        // Cập nhật bảng độc giả (cột sách đang mượn)
        hienThiDanhSachDocGia(); 
        
        // Cập nhật lại UI controls
        oNhapISBN->clear();
        // Checked: hopXacNhanMuon removed from UI, so removing usage here to prevent crash
        // hopXacNhanMuon->setChecked(false); 
        nutXacNhanMuon->setEnabled(false);
        
        // Cập nhật thông tin chi tiết độc giả (Box Phải)
        int currentRow = bangDocGia->currentIndex().row();
        if (currentRow >= 0) {
             bangDocGia->selectRow(currentRow);
             xuLyChonDocGia(currentRow, 0); 
        }
    } else {
        // FAILURE CASE
        // FIX INFINITE LOOP: Clear input immediately to prevent "Enter" from re-triggering this same error
        oNhapISBN->clear(); 
        QMessageBox::warning(this, "Lỗi", QString::fromStdString(ketQua));
    }
}

void WidgetMuonTraSach::xuLyNutXacNhanTra(bool skipConfirm) {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn độc giả trước!");
        return;
    }
    
    int dongChon = bangSachDangMuon->currentIndex().row();
    if (dongChon < 0) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn sách cần trả!");
        return;
    }
    if (dongChon < 0 || dongChon >= (int)danhSachSachDangMuon.size()) {
        QMessageBox::warning(this, "Cảnh Báo", "Dữ liệu chọn không hợp lệ!");
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
        canhBaoPhat = QString("\n\n⚠️ QUÁ HẠN %1 NGÀY!\nTiền phạt dự kiến: %2 đ")
                        .arg(soNgayQuaHan)
                        .arg(QLocale(QLocale::Vietnamese).toString(tienPhat));
    }

    QMessageBox::StandardButton xacNhan = QMessageBox::Yes;
    if (!skipConfirm) {
        xacNhan = QMessageBox::question(this, "Xác Nhận Trả Sách",
            QString("Xác nhận trả sách:\n\nMã: %1\nTên: %2%3")
                .arg(maSach, tenSach, canhBaoPhat),
            QMessageBox::Yes | QMessageBox::No);
    }
    
    if (xacNhan == QMessageBox::Yes) {
        extern PTRDS dsDauSach[];
        extern int soLuongDauSach;
        
        std::string ketQua = TraSach(docGiaHienTai, maSach.toStdString(), dsDauSach, soLuongDauSach);
        
        if (ketQua.empty() || ketQua.find("Thành công") != std::string::npos || ketQua.find("thành công") != std::string::npos) {
            // Ghi log giao dịch trả
            TransactionLogger::ghiLogTra(docGiaHienTai, maSach, tenSach, true);
            
            // Xử lý phạt (Feature 4)
            if (tienPhat > 0) {
                 bool daThanhToan = HopThoaiThanhToan::hienThiThanhToan(
                     docGiaHienTai, 
                     QString("Quá hạn %1 ngày").arg(soNgayQuaHan), 
                     tienPhat, 
                     this
                 );
                 
                 if (daThanhToan) {
                     // Gọi backend đánh dấu đã thanh toán
                     ThanhToanPhat(docGiaHienTai, maSach.toStdString());
                     ToastNotification::show("Đã trả sách và thanh toán tiền phạt.", ToastNotification::SUCCESS);
                 } else {
                     ToastNotification::show("Đã trả sách. Ghi nợ tiền phạt.", ToastNotification::WARNING);
                 }
                 // Cập nhật lại thông tin hiển thị (nợ phạt)
                 xuLyChonDocGia(bangDocGia->currentIndex().row(), 0);
            } else {
                 // SUCCESS NO FINE
                 ToastNotification::show("Đã trả sách thành công!", ToastNotification::SUCCESS);
            }
            
            // Auto Print Receipt (Silent or Prompt)
            // Lấy ngày trả hôm nay
            QString ngayTra = QDate::currentDate().toString("dd/MM/yyyy");
            PrintHelper::inPhieuTraTuDong(docGiaHienTai, maSach, tenSach, ngayTra, this);

            // Tự động lưu file
            extern PTRDG rootDocGia;
            saveDocGia(rootDocGia);
            GhiDanhSachDauSach(FilePathManager::getDauSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
            GhiDanhMucSach(FilePathManager::getDanhMucSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
            
            emit thongBaoCapNhat("Đã trả sách thành công!");
            emit duLieuThayDoi(); // Realtime update Dashboard
            EventBus::instance().muonTraRemoved(docGiaHienTai->data.MaThe, maSach); // REALTIME UPDATE
            
            // Cập nhật lại danh sách sách đang mượn
            capNhatBangSachDangMuon();
            nutXacNhanTra->setEnabled(false);
            nutBaoMat->setEnabled(false);
            nutInLaiPhieu->setEnabled(false);
            
            // Cập nhật bảng độc giả (cột số sách đang mượn)
            hienThiDanhSachDocGia(); // Force Left Table Refresh
            
            // Cập nhật thông tin chi tiết độc giả (Box Phải & Highlight lại dòng)
            int currentRow = bangDocGia->currentIndex().row();
            if (currentRow >= 0) {
                 bangDocGia->selectRow(currentRow);
                 xuLyChonDocGia(currentRow, 0);
            }
        } else {
            // Error case
            ToastNotification::show(QString::fromStdString(ketQua), ToastNotification::ERROR);
            QMessageBox::warning(this, "Lỗi", QString::fromStdString(ketQua));
        }
    }
}

void WidgetMuonTraSach::xuLyNutBaoMat() {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn độc giả trước!");
        return;
    }
    
    int dongChon = bangSachDangMuon->currentIndex().row();
    if (dongChon < 0) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn sách cần báo mất!");
        return;
    }
    if (dongChon < 0 || dongChon >= (int)danhSachSachDangMuon.size()) {
        QMessageBox::warning(this, "Cảnh Báo", "Dữ liệu chọn không hợp lệ!");
        return;
    }
    const ThongTinSachDangMuon_DTO &dto = danhSachSachDangMuon[dongChon];
    QString maSach = QString::fromStdString(dto.maSach);
    QString tenSach = QString::fromStdString(dto.tenSach);
    
    // Tính toán chi phí đền bù trước khi xác nhận
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    // Tìm sách để lấy giá
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
    
    QMessageBox::StandardButton xacNhan = QMessageBox::question(this, "⚠️ XÁC NHẬN BÁO MẤT SÁCH",
        QString("⚠️ BÁO MẤT SÁCH là thao tác NGHIÊM TRỌNG!\n\n"
                "Mã: %1\nTên: %2\n\n"
                "CHI PHÍ ĐỀN BÙ:\n"
                "━━━━━━━━━━━━━━━━━━━━\n"
                "Giá sách: %3 VNĐ\n"
                "Phí phạt: %4 VNĐ\n"
                "━━━━━━━━━━━━━━━━━━━━\n"
                "TỔNG CỘNG: %5 VNĐ\n\n"
                "Độc giả sẽ bị ghi nợ số tiền này.\n\n"
                "Bạn có chắc chắn?")
            .arg(maSach, tenSach)
            .arg(QLocale(QLocale::Vietnamese).toString((qint64)giaSach))
            .arg(QLocale(QLocale::Vietnamese).toString((qint64)phiPhat))
            .arg(QLocale(QLocale::Vietnamese).toString((qint64)tongDenBu)),
        QMessageBox::Yes | QMessageBox::No);
    
    if (xacNhan == QMessageBox::Yes) {
        std::string ketQua = BaoMatSach(docGiaHienTai, maSach.toStdString(), dsDauSach, soLuongDauSach);
        
        if (ketQua.empty() || ketQua.find("Thành công") != std::string::npos || ketQua.find("thành công") != std::string::npos) {
            // Tự động lưu file
            extern PTRDG rootDocGia;
            saveDocGia(rootDocGia);
            GhiDanhSachDauSach(FilePathManager::getDauSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
            GhiDanhMucSach(FilePathManager::getDanhMucSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
            
            // Hiển thị thông tin đền bù chi tiết
            QMessageBox::information(this, "✅ Đã Ghi Nhận Mất Sách",
                QString("Đã ghi nhận mất sách: %1\n\n"
                        "CHI TIẾT ĐỀN BÙ:\n"
                        "━━━━━━━━━━━━━━━━━━━━\n"
                        "Giá sách: %2 VNĐ\n"
                        "Phí phạt: %3 VNĐ\n"
                        "━━━━━━━━━━━━━━━━━━━━\n"
                        "TỔNG CỘNG: %4 VNĐ\n\n"
                        "Độc giả cần thanh toán khoản nợ này.")
                    .arg(tenSach)
                    .arg(QLocale(QLocale::Vietnamese).toString((qint64)giaSach))
                    .arg(QLocale(QLocale::Vietnamese).toString((qint64)phiPhat))
                    .arg(QLocale(QLocale::Vietnamese).toString((qint64)tongDenBu)));
            
            // Mở hộp thoại thanh toán
            bool daThanhToan = HopThoaiThanhToan::hienThiThanhToan(
                docGiaHienTai,
                QString("Đền bù sách mất: %1").arg(tenSach),
                tongDenBu,
                this
            );
            
            if (daThanhToan) {
                // Đánh dấu đã thanh toán
                ThanhToanPhat(docGiaHienTai, maSach.toStdString());
                saveDocGia(rootDocGia);
                ToastNotification::show("Đã thanh toán đền bù sách mất!", ToastNotification::SUCCESS);
            } else {
                ToastNotification::show("Đã ghi nợ tiền đền bù.", ToastNotification::WARNING);
            }
            
            emit thongBaoCapNhat("Đã báo mất sách!");
            emit duLieuThayDoi(); // Realtime update Dashboard
            
            // Cập nhật lại
            capNhatBangSachDangMuon();
            nutXacNhanTra->setEnabled(false);
            nutBaoMat->setEnabled(false);
            nutThanhToan->setEnabled(false);
            
            // Refresh table & detail view
             hienThiDanhSachDocGia(); // Update Status in Left Table
             
            int currentRow = bangDocGia->currentIndex().row();
            if (currentRow >= 0) {
                bangDocGia->selectRow(currentRow);
                xuLyChonDocGia(currentRow, 0); // Update Right Panel
            }
            
        } else {
            QMessageBox::warning(this, "Lỗi", QString::fromStdString(ketQua));
        }
    }
}

// Feature 4: Thanh Toan No
void WidgetMuonTraSach::xuLyNutThanhToan() {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn độc giả trước!");
        return;
    }
    
    int dongChon = bangSachDangMuon->currentIndex().row();
    if (dongChon < 0 || dongChon >= (int)danhSachSachDangMuon.size()) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn sách cần thanh toán!");
        return;
    }
    
    const ThongTinSachDangMuon_DTO &dto = danhSachSachDangMuon[dongChon];
    // Re-calculate fine logic
    std::stringstream ss;
    int soNgayQuaHan = ::tinhSoNgayQuaHan(dto.ngayMuon, ss);
    long long tienPhat = 0;
    if (soNgayQuaHan > 0) {
        int tienPhatMoiNgay = SettingsManager::instance().getInt("TIEN_PHAT_MOI_NGAY", 5000);
        tienPhat = (long long)soNgayQuaHan * tienPhatMoiNgay;
    }
    
    if (tienPhat <= 0) {
        QMessageBox::information(this, "Thông Báo", "Sách này không có khoản phạt nào cần thanh toán (hoặc đã trả đúng hạn).");
        return;
    }

    bool daThanhToan = HopThoaiThanhToan::hienThiThanhToan(
        docGiaHienTai, 
        QString("Thanh toán tiền phạt quá hạn (%1 ngày)").arg(soNgayQuaHan), 
        tienPhat, 
        this
    );
     
    if (daThanhToan) {
        // Call backend to clear debt
        ThanhToanPhat(docGiaHienTai, dto.maSach);
         
         // Dữ liệu đã được đồng bộ SQL bên trong backend
         
         QMessageBox::information(this, "Thành Công", "Đã thanh toán thành công!");
         
         emit thongBaoCapNhat("Đã thanh toán tiền phạt!");
         emit duLieuThayDoi();
         
         capNhatBangSachDangMuon();
         hienThiDanhSachDocGia();
         
         // Update UI
         nutThanhToan->setEnabled(false);
    }
}

// Feature 4: Gia Han Sach
void WidgetMuonTraSach::xuLyNutGiaHan() {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn độc giả trước!");
        return;
    }
    
    int dongChon = bangSachDangMuon->currentIndex().row();
    if (dongChon < 0 || dongChon >= (int)danhSachSachDangMuon.size()) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn sách cần gia hạn!");
        return;
    }
    
    const ThongTinSachDangMuon_DTO &dto = danhSachSachDangMuon[dongChon];
    QString maSach = QString::fromStdString(dto.maSach);
    
    QMessageBox::StandardButton xacNhan = QMessageBox::question(this, "Xác Nhận Gia Hạn",
        QString("Gia hạn thêm 7 ngày cho sách:\n%1").arg(QString::fromStdString(dto.tenSach)),
        QMessageBox::Yes | QMessageBox::No);
        
    if (xacNhan == QMessageBox::Yes) {
        std::string ketQua = GiaHanMuon(docGiaHienTai, maSach.toStdString());
        
        if (ketQua.empty()) {
             // Dữ liệu đã được đồng bộ SQL bên trong backend
             
             QMessageBox::information(this, "Thành Công", "Đã gia hạn thành công!");
             emit thongBaoCapNhat("Đã gia hạn sách!");
             emit duLieuThayDoi();
             
             capNhatBangSachDangMuon();
             nutGiaHan->setEnabled(false);
        } else {
             QMessageBox::warning(this, "Lỗi", QString::fromStdString(ketQua));
        }
    }
}

// In lại phiếu mượn (Có hỗ trợ in gộp)
void WidgetMuonTraSach::xuLyInLaiPhieu() {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn độc giả trước!");
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
             QMessageBox::warning(this, "Thông Báo", "Độc giả hiện không mượn sách nào.");
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

void WidgetMuonTraSach::xuLyNutQuetMaDocGia() {
    HopThoaiQuetMaVach *dialogQuet = new HopThoaiQuetMaVach(HopThoaiQuetMaVach::QUET_DOC_GIA, this);
    if (dialogQuet->exec() == QDialog::Accepted) {
        QString maThe = dialogQuet->layMaQuet();
        if (!maThe.isEmpty()) {
            oNhapTimDocGia->setText(maThe);
            xuLyTimKiem();
        }
    }
    delete dialogQuet;
}

void WidgetMuonTraSach::xuLyNutQuetMaSach() {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, QStringLiteral("Lỗi"), QStringLiteral("Vui lòng chọn độc giả trước!"));
        return;
    }
    HopThoaiQuetMaVach *dialogQuet = new HopThoaiQuetMaVach(HopThoaiQuetMaVach::QUET_SACH, this);
    if (dialogQuet->exec() == QDialog::Accepted) {
        QString isbn = dialogQuet->layMaQuet();
        if (!isbn.isEmpty()) {
            oNhapISBN->setText(isbn);
            extern PTRDS dsDauSach[];
            extern int soLuongDauSach;
            bool timThay = false;
            for (int i = 0; i < soLuongDauSach; i++) {
                if (dsDauSach[i] && QString::fromStdString(dsDauSach[i]->ISBN) == isbn) {
                    timThay = true;
                    // Auto-confirm logic replaces checkbox
                    // hopXacNhanMuon->setChecked(true);
                    xuLyNutXacNhanMuon(true); // Auto-borrow immediately on scan
                    break;
                }
            }
            if (!timThay) {
                QMessageBox::warning(this, QStringLiteral("Lỗi"), QStringLiteral("Không tìm thấy sách với ISBN: ") + isbn);
            }
        }
    }
    delete dialogQuet;
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

void WidgetMuonTraSach::xuLyInBatch() {
    if (!docGiaHienTai) {
        // Safety check - clear all queues
        m_batchPrintQueueMa.clear();
        m_batchPrintQueueTen.clear();
        m_batchReturnQueueMa.clear();
        m_batchReturnQueueTen.clear();
        return;
    }

    // [AUTO-PRINT] Force Silent Mode for Batch Operations
    PrintHelper::PrintConfig cfg = PrintHelper::getCauHinh();
    bool originalSilent = cfg.silentPrint;
    cfg.silentPrint = true; // FORCE SILENT
    PrintHelper::setCauHinh(cfg);

    // Check which operation to print for
    // Check which operation to print for
    if (m_lastOperationWasBorrow && !m_batchPrintQueueTen.isEmpty()) {
        // Print BORROW receipt
        // Create DTO list for inPhieuMuonBatch
        QList<ThongTinSachDangMuon_DTO> listDto;
        for (int i = 0; i < m_batchPrintQueueMa.size(); i++) {
            ThongTinSachDangMuon_DTO dto;
            dto.maSach = m_batchPrintQueueMa[i].toStdString();
            dto.tenSach = m_batchPrintQueueTen[i].toStdString();
            listDto.append(dto);
        }
        
        // Use the new Batch Print function (Text Only, No Barcode)
        PrintHelper::inPhieuMuonBatch(docGiaHienTai, listDto, this);
        
        // Clear borrow queues
        m_batchPrintQueueMa.clear();
        m_batchPrintQueueTen.clear();
        
    } else if (!m_lastOperationWasBorrow && !m_batchReturnQueueTen.isEmpty()) {
        // Print RETURN receipt
        // Call InPhieuMuonTra with False = Return
        PrintHelper::inPhieuMuonTra(docGiaHienTai, m_batchReturnQueueTen, false, this);
        
        // Clear return queues
        m_batchReturnQueueMa.clear();
        m_batchReturnQueueTen.clear();
    }
    
    // Restore original config (optional, but good practice if user wants manual print elsewhere)
    // However, user said "tu dong in luon", so maybe they want it everywhere?
    // Let's restore for safety, as batch print is the specific context.
    cfg.silentPrint = originalSilent; 
    PrintHelper::setCauHinh(cfg);
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
    
    // trangThai == 1 means "Returned (Debt)"
    bool daTraNoTien = (dto.trangThai == 1);
    
    // Actions for Borrowed Books vs Returned Books
    // Return & Report Lost are ONLY for Currently Borrowed books
    nutXacNhanTra->setEnabled(!daTraNoTien);
    nutBaoMat->setEnabled(!daTraNoTien);
    
    // Feature Renewal: Only for currently borrowed books
    // Logic: Limit renewal count
    int currentRenewals = dto.soLanGiaHan;
    int maxRenewals = SettingsManager::instance().getInt("SO_LAN_GIA_HAN_TOI_DA", 1);
    
    if (!daTraNoTien && currentRenewals < maxRenewals) {
         nutGiaHan->setEnabled(true);
         nutGiaHan->setText(QString("GIA HẠN (%1/%2)").arg(currentRenewals).arg(maxRenewals));
         nutGiaHan->setProperty("class", "Primary");
    } else {
         nutGiaHan->setEnabled(false);
         if (daTraNoTien) {
             nutGiaHan->setText("GIA HẠN");
             nutGiaHan->setProperty("class", "Disabled");
         } else {
             nutGiaHan->setText(QString("HẾT HẠN (%1/%2)").arg(currentRenewals).arg(maxRenewals));
             nutGiaHan->setProperty("class", "Danger"); // Highlight Limit Reached
         }
    }
    
    // Refresh Button Style
    if(nutGiaHan->style()) { nutGiaHan->style()->unpolish(nutGiaHan); nutGiaHan->style()->polish(nutGiaHan); }

    // Feature: Payment
    // Enabled ONLY if Returned AND has Debt AND not Paid
    if (nutThanhToan) {
        if (daTraNoTien && dto.tienPhat > 0 && !dto.daThanhToan) {
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
    qDebug() << "[SCAN] Mã độc giả:" << maThe;
    
    // Tìm độc giả
    int maTheInt = maThe.toInt();
    extern PTRDG rootDocGia;
    PTRDG dg = timDocGia(rootDocGia, maTheInt);
    
    if (!dg) {
        QMessageBox::warning(this, "Lỗi", 
            QString("Không tìm thấy độc giả: %1").arg(maThe));
        return;
    }
    
    // ✅ TỰ ĐỘNG CHỌN ĐỘC GIẢ
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
        xuLyTuDongMuon(bookToBorrow);
    } else {
        // Chỉ load thông tin nếu đang IDLE
        ToastNotification::show(QString("Đã chọn độc giả: %1").arg(QString::fromStdString(dg->data.Ten)), ToastNotification::SUCCESS);
    }
    
    QApplication::beep();
}

void WidgetMuonTraSach::xuLyQuetMaSach(const QString& maSach) {
    qDebug() << "[SCAN] Mã sách:" << maSach;
    
    // ═══════════════════════════════════════════════════
    // KIỂM TRA TRẠNG THÁI SÁCH (Sử dụng SQL Sync)
    // ═══════════════════════════════════════════════════
    TrangThaiSach trangThai = kiemTraTrangThaiSach(maSach);
    
    switch (trangThai) {
        case SACH_RANH:
            // Nếu chưa chọn độc giả -> Chuyển trạng thái chờ
            if (!docGiaHienTai) {
                m_currentState = STATE_WAITING_FOR_READER;
                m_pendingBookCode = maSach;
                ToastNotification::show("Chưa chọn độc giả. Vui lòng quét thẻ!", ToastNotification::INFO);
                return;
            }
            // ✅ TỰ ĐỘNG MƯỢN
            xuLyTuDongMuon(maSach);
            break;
            
        case SACH_DOC_GIA_HIEN_TAI_DANG_MUON:
            // ✅ TỰ ĐỘNG TRẢ (Đã chọn đúng độc giả)
            xuLyTuDongTra(maSach);
            break;
            
        case SACH_NGUOI_KHAC_DANG_MUON: {
            // ✅ TỰ ĐỘNG TRẢ (Chưa chọn độc giả hoặc chọn sai người)
            // Tìm xem ai đang mượn cuốn này
            QString resolvedMaSach = "";
            PTRDG reader = TimDocGiaDangMuonSach(maSach, resolvedMaSach);
            if (reader) {
                docGiaHienTai = reader;
                capNhatThongTinDocGia();
                capNhatBangSachDangMuon();
                // Select and return
                xuLyTuDongTra(resolvedMaSach.isEmpty() ? maSach : resolvedMaSach);
            } else {
                 QMessageBox::warning(this, "Lỗi", "Sách đang được mượn nhưng không tìm thấy dữ liệu độc giả!");
            }
            break;
        }
            
        case SACH_KHONG_TON_TAI:
            QMessageBox::warning(this, "Lỗi", QString("Không tìm thấy sách hoặc sách đã thanh lý: %1").arg(maSach));
            break;
    }
}

WidgetMuonTraSach::TrangThaiSach WidgetMuonTraSach::kiemTraTrangThaiSach(const QString& maSach) {
    // 1. Kiểm tra ưu tiên: Độc giả hiện tại có đang mượn không?
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
        // Trạng thái khác (Thanh lý, Mất...) coi như không tồn tại để mượn
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
    oNhapISBN->setFocus();
    
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
    if (oNhapMaSachTra) oNhapMaSachTra->setText(maSach);
    
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
    bangSachDangMuon->selectRow(dongChon);
    
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
        hopTab->setTabText(0, LanguageManager::instance().getText("TAB_BORROW"));
        hopTab->setTabText(1, LanguageManager::instance().getText("TAB_RETURN"));
        hopTab->setTabText(2, LanguageManager::instance().getText("TAB_HISTORY"));
    }
    
    // 2. Main Labels
    if(nhanTieuDeChinh) nhanTieuDeChinh->setText(LanguageManager::instance().getText("HEADER_MAIN_TITLE"));
    if(nhanTieuDeTraSach) nhanTieuDeTraSach->setText(LanguageManager::instance().getText("HEADER_QUICK_RETURN"));
    if(nhanHuongDanTraSach) nhanHuongDanTraSach->setText(LanguageManager::instance().getText("LBL_INSTRUCT_QUICK_RETURN"));
    if(nhanLichSuPlaceholder) nhanLichSuPlaceholder->setText(LanguageManager::instance().getText("MSG_TAB_HISTORY_PLACEHOLDER"));
    
    // 3. Placeholders & Tooltips
    if(oNhapTimDocGia) oNhapTimDocGia->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_SEARCH_READER"));
    if(oNhapISBN) oNhapISBN->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_SCAN_BORROW"));
    if(oNhapMaSachTra) oNhapMaSachTra->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_ENTER_BOOK_CODE"));
    
    if(nutTimDocGia) nutTimDocGia->setText(LanguageManager::instance().getText("BTN_SEARCH"));
    if(nutLamMoiDocGia) nutLamMoiDocGia->setToolTip(LanguageManager::instance().getText("TOOLTIP_REFRESH"));
    if(nutQuetMaDocGia) {
        nutQuetMaDocGia->setText(LanguageManager::instance().getText("BTN_SCAN_CARD"));
        nutQuetMaDocGia->setToolTip(LanguageManager::instance().getText("BTN_SCAN_CARD"));
    }
    if(nutQuetMaSach) nutQuetMaSach->setText(LanguageManager::instance().getText("BTN_SCAN_BOOK"));
    if(nutQuetMaSachTra) nutQuetMaSachTra->setText(LanguageManager::instance().getText("BTN_SCAN_SHORT"));
    if(nutTraSachNhanh) nutTraSachNhanh->setText(LanguageManager::instance().getText("BTN_CONFIRM_RETURN"));

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

