#include "views/WidgetQuanLyDocGia.h"
#include "views/dialogs/ManHinhThemDocGia.h"
#include "views/dialogs/ManHinhSuaDocGia.h"
#include "models/DocGia.h"
#include "models/QuanLySach.h"
#include "models/DauSach.h"
#include "utils/ConfirmationDialog.h"
#include "utils/AnimationHelper.h"
#include "utils/DBBatcher.h"
#include "utils/NgayThang.h"
#include "utils/Utf8Helper.h"
#include "utils/Utf8Helper.h"
#include "utils/SafeGui.h"
#include "utils/LanguageManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QTextStream>

#include <QDate>
#include <QDialog>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QTabBar>
#include <QRegularExpression>
#include <algorithm>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QIcon>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>

extern PTRDG rootDocGia;

// Modern Card-based Design - No hardcoded styles
// All styles are managed by modern-dashboard.qss

// ==================== ReaderTableModel Implementation ====================

ReaderTableModel::ReaderTableModel(QObject *parent)
    : QAbstractTableModel(parent)
    , readers()
    , isSearchMode(false)
    , sortColumn(-1)
    , sortOrder(Qt::AscendingOrder)
{
}

void WidgetQuanLyDocGia::onSearchFinished() {
    if (!searchWatcher) return;
    QFuture<std::vector<PTRDG>> f = searchWatcher->future();
    std::vector<PTRDG> results = f.result();
    readerModel->setSearchResults(results);
    showMessage(QString("Tìm thấy %1 học viên").arg((int)results.size()));
    btnSearch->setEnabled(true);
    searchProgress->setVisible(false);
}

int ReaderTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return static_cast<int>(readers.size());
}

int ReaderTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 5; // Ma The, Ho, Ten, Phai, Trang Thai
}

QVariant ReaderTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(readers.size()))
        return QVariant();
    
    PTRDG reader = readers[index.row()];
    if (!reader)
        return QVariant();
    
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return reader->data.MaThe;
            case 1: return QString::fromStdString(reader->data.Ho);
            case 2: return QString::fromStdString(reader->data.Ten);
            case 3: return reader->data.Phai ? LanguageManager::instance().getText("GENDER_FEMALE") : LanguageManager::instance().getText("GENDER_MALE");
            case 4: {
                QString status = reader->data.TrangThai ? LanguageManager::instance().getText("STATUS_ACTIVE") : LanguageManager::instance().getText("STATUS_LOCKED");
                if (isReaderOverdue(reader)) {
                    status += " " + LanguageManager::instance().getText("STATUS_OVERDUE");
                }
                return status;
            }
        }
    } else if (role == Qt::TextAlignmentRole) {
        if (index.column() == 0 || index.column() >= 3) {
            return Qt::AlignCenter;
        }
    } else if (role == Qt::ForegroundRole) {
        // Highlight overdue readers in orange/red
        if (isReaderOverdue(reader)) {
            return QColor(245, 158, 11); // Orange warning color #F59E0B
        }
        // Highlight locked readers in red
        if (!reader->data.TrangThai) {
            return QColor(239, 68, 68); // Red color #EF4444
        }
    } else if (role == Qt::FontRole) {
        // Bold font for overdue readers
        if (isReaderOverdue(reader)) {
            QFont font;
            font.setBold(true);
            return font;
        }
    }

    // Provide enum for delegate drawing: map reader status -> TrangThaiSach
    if (role == Qt::UserRole && index.column() == 4) {
        // Map: Active (no overdue) -> CHO_MUON_DUOC (green)
        // Overdue -> DANG_MUON (orange)
        // Locked -> THANH_LY (red)
        if (!reader->data.TrangThai) return QVariant::fromValue<int>(THANH_LY);
        if (isReaderOverdue(reader)) return QVariant::fromValue<int>(DANG_MUON);
        return QVariant::fromValue<int>(CHO_MUON_DUOC);
    }
    
    return QVariant();
}

QVariant ReaderTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();
    
    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0: return LanguageManager::instance().getText("TABLE_HEADER_CODE");
            case 1: return LanguageManager::instance().getText("TABLE_HEADER_LASTNAME");
            case 2: return LanguageManager::instance().getText("TABLE_HEADER_FIRSTNAME");
            case 3: return LanguageManager::instance().getText("TABLE_HEADER_GENDER");
            case 4: return LanguageManager::instance().getText("TABLE_HEADER_STATUS");
        }
    }
    return QVariant();
}

void ReaderTableModel::collectReadersFromTree(PTRDG root, std::vector<PTRDG> &out) {
    if (!root) return;
    collectReadersFromTree(root->left, out);
    out.push_back(root);
    collectReadersFromTree(root->right, out);
}

void ReaderTableModel::refreshData() {
    beginResetModel();
    isSearchMode = false;
    // Use DBBatcher snapshot to collect readers efficiently
    readers = DBBatcher::snapshotDocGia();
    endResetModel();
}

void ReaderTableModel::setSearchResults(PTRDG* results, int count) {
    beginResetModel();
    readers.clear();
    readers.reserve(count > 0 ? count : 0);
    for (int i = 0; i < count; ++i) {
        readers.push_back(results[i]);
    }
    isSearchMode = true;
    endResetModel();
}

void ReaderTableModel::setSearchResults(const std::vector<PTRDG> &results) {
    beginResetModel();
    readers = results;
    isSearchMode = true;
    endResetModel();
}

PTRDG ReaderTableModel::getReaderAt(int row) const {
    if (row >= 0 && row < static_cast<int>(readers.size())) {
        return readers[row];
    }
    return nullptr;
}

void ReaderTableModel::sort(int column, Qt::SortOrder order) {
    if (readers.empty()) return;

    sortColumn = column;
    sortOrder = order;

    layoutAboutToBeChanged();

    auto cmp = [column, order](const PTRDG &a, const PTRDG &b) {
        if (!a || !b) return false;
        switch (column) {
            case 0: // Ma The
                return (order == Qt::AscendingOrder) ? (a->data.MaThe < b->data.MaThe)
                                                      : (a->data.MaThe > b->data.MaThe);
            case 1: // Ho
                return (order == Qt::AscendingOrder) ? (a->data.Ho < b->data.Ho)
                                                      : (a->data.Ho > b->data.Ho);
            case 2: // Ten
                return (order == Qt::AscendingOrder) ? (a->data.Ten < b->data.Ten)
                                                      : (a->data.Ten > b->data.Ten);
            case 3: // Phai
                return (order == Qt::AscendingOrder) ? (a->data.Phai < b->data.Phai)
                                                      : (a->data.Phai > b->data.Phai);
            case 4: // Trang Thai
                return (order == Qt::AscendingOrder) ? (a->data.TrangThai < b->data.TrangThai)
                                                      : (a->data.TrangThai > b->data.TrangThai);
            default:
                return false;
        }
    };

    std::sort(readers.begin(), readers.end(), cmp);

    layoutChanged();
}

bool ReaderTableModel::isReaderOverdue(PTRDG reader) const {
    if (!reader) return false;
    
    // Check if reader has overdue books
    MUONTRA mt = reader->data.dsmt;
    while (mt) {
        if (mt->data.TrangThai == 0) { // Đang mượn
            // Parse date string (format: "DD/MM/YYYY")
            QString ngayMuonStr = QString::fromStdString(mt->data.NgayMuon);
            QStringList parts = ngayMuonStr.split('/');
            if (parts.size() == 3) {
                QDate dateMuon(parts[2].toInt(), parts[1].toInt(), parts[0].toInt());
                QDate dateHienTai = QDate::currentDate();
                
                int soNgayMuon = dateMuon.daysTo(dateHienTai);
                if (soNgayMuon > 7) { // Quá hạn nếu mượn > 7 ngày
                    return true;
                }
            }
        }
        mt = mt->next;
    }
    
    return false;
}

int ReaderTableModel::getBorrowedBookCount(PTRDG reader) const {
    if (!reader) return 0;
    
    int count = 0;
    MUONTRA mt = reader->data.dsmt;
    while (mt) {
        if (mt->data.TrangThai == 0) { // Đang mượn
            count++;
        }
        mt = mt->next;
    }
    
    return count;
}

// ==================== WidgetQuanLyDocGia Implementation ====================

WidgetQuanLyDocGia::WidgetQuanLyDocGia(QWidget *parent)
    : QWidget(parent)

    , selectedReader(nullptr)
    , tabWidget(nullptr)
    , tabQuanLy(nullptr)
    , tableView(nullptr)
    , readerModel(nullptr)
    , searchEdit(nullptr)
    , searchTypeCombo(nullptr)
    , filterGioiTinhCombo(nullptr)
    , filterTrangThaiCombo(nullptr)
    , filterStatusCombo(nullptr)
    , btnSearch(nullptr)
    , btnClearSearch(nullptr)
    , btnAdd(nullptr)
    , btnEdit(nullptr)
    , btnDelete(nullptr)
    , btnDetails(nullptr)
    , btnUnlock(nullptr)
    , btnViewLoans(nullptr)
    , btnBack(nullptr)
    , statusLabel(nullptr)
    , toolbar(nullptr)
    , badgeDelegate(nullptr)
    , tabQuaHan(nullptr)
    , tableQuaHan(nullptr)
    , modelQuaHan(nullptr)
    , btnRefreshQuaHan(nullptr)
    , lblThongKeQuaHan(nullptr)
    , searchProgress(nullptr)
    , searchWatcher(nullptr)
{
    // Ensure QSS selectors targeting object name also apply
    this->setObjectName("WidgetQuanLyDocGia");
    searchTimer = new QTimer(this);
    searchTimer->setSingleShot(true);
    searchTimer->setInterval(300);
    
    // Async search watcher
    searchWatcher = new QFutureWatcher<std::vector<PTRDG>>(this);
    connect(searchWatcher, &QFutureWatcher<std::vector<PTRDG>>::finished,
            this, &WidgetQuanLyDocGia::onSearchFinished);

    // Badge delegate for status column (reuse existing badge styling)
    badgeDelegate = new BadgeStyledItemDelegate(this);

    setupUI();
    setupConnections();
    setupKeyboardShortcuts();
    updateButtonStates();
}

WidgetQuanLyDocGia::~WidgetQuanLyDocGia() {
}

void WidgetQuanLyDocGia::setupUI() {
    // MASTER PLAN: Card-Based Design - No hardcoded styles
    this->setProperty("class", "WidgetQuanLyDocGia");
    
    // Layout chính - Clean Spacing
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(25);
    mainLayout->setContentsMargins(35, 35, 35, 35);

    // Initial Language Update (Wait until widgets are created)
    QTimer::singleShot(0, this, &WidgetQuanLyDocGia::capNhatNgonNgu);
    
    // --- CARD 1: HEADER & TÌM KIẾM ---
    QFrame *headerCard = new QFrame(this);
    headerCard->setProperty("class", "Card");
    headerCard->setFrameShape(QFrame::NoFrame);
    QVBoxLayout *headerLayout = new QVBoxLayout(headerCard);
    headerLayout->setSpacing(15);
    headerLayout->setContentsMargins(20, 20, 20, 20);
    
    // Dòng 1: Tiêu đề + Nút Thêm
    QHBoxLayout *topRow = new QHBoxLayout();
    
    QLabel *titleLabel = new QLabel("Quản Lý Học Viên", headerCard);
    titleLabel->setProperty("class", "PageTitle");
    
    btnAdd = new QPushButton("+ Thêm Học Viên", headerCard);
    btnAdd->setProperty("class", "Primary");
    btnAdd->setCursor(Qt::PointingHandCursor);
    btnAdd->setMinimumHeight(40);
    
    topRow->addWidget(titleLabel);
    topRow->addStretch();
    topRow->addWidget(btnAdd);
    headerLayout->addLayout(topRow);
    
    // Dòng 2: Tìm kiếm + Bộ lọc
    QHBoxLayout *filterRow = new QHBoxLayout();
    filterRow->setSpacing(10);
    
    searchTypeCombo = new QComboBox(headerCard);
    searchTypeCombo->addItem("Tìm kiếm", 0);
    searchTypeCombo->setMinimumHeight(40);
    searchTypeCombo->setMinimumWidth(120);
    filterRow->addWidget(searchTypeCombo);
    
    searchEdit = new QLineEdit(headerCard);
    searchEdit->setPlaceholderText("Tìm kiếm theo Mã thẻ, Họ tên...");
    searchEdit->setMinimumHeight(40);
    searchEdit->setProperty("class", "InputDark");
    filterRow->addWidget(searchEdit, 1);
    
    filterStatusCombo = new QComboBox(headerCard);
    filterStatusCombo->addItem("Tất cả trạng thái", -1);
    filterStatusCombo->addItem("Hoạt động", 1);
    filterStatusCombo->addItem("Khóa", 0);
    filterStatusCombo->setMinimumHeight(40);
    filterStatusCombo->setMinimumWidth(150);
    filterRow->addWidget(filterStatusCombo);
    
    btnSearch = new QPushButton("Tìm kiếm", headerCard);
    btnSearch->setProperty("class", "GradientPrimary");
    btnSearch->setMinimumHeight(40);
    filterRow->addWidget(btnSearch);
    
    btnClearSearch = new QPushButton("× Xóa", headerCard);
    btnClearSearch->setProperty("class", "Secondary");
    btnClearSearch->setMinimumHeight(40);
    filterRow->addWidget(btnClearSearch);
    // Inline progress indicator for search (indeterminate)
    searchProgress = new QProgressBar(headerCard);
    searchProgress->setRange(0, 0);
    searchProgress->setVisible(false);
    searchProgress->setFixedWidth(140);
    filterRow->addWidget(searchProgress);

    headerLayout->addLayout(filterRow);
    mainLayout->addWidget(headerCard);
    
    // --- ACTION BAR: Buttons above the table (match WidgetQuanLySach layout) ---
    QFrame *nhomChucNang = new QFrame(this);
    nhomChucNang->setFrameShape(QFrame::NoFrame);
    nhomChucNang->setProperty("class", "ActionBar");
    QHBoxLayout *layChucNang = new QHBoxLayout(nhomChucNang);
    layChucNang->setSpacing(12);
    layChucNang->setContentsMargins(6, 6, 6, 6);

    QPushButton *btnAddReader = new QPushButton(QIcon(":/icons/plus.svg"), " + Thêm Học Viên", nhomChucNang);
    QPushButton *btnImport = new QPushButton(QIcon(":/icons/upload.svg"), " Nhập Excel", nhomChucNang);
    QPushButton *btnPrint = new QPushButton(QIcon(":/icons/print.svg"), " In", nhomChucNang);
    QPushButton *btnEditTop = new QPushButton(QIcon(":/icons/edit.svg"), " Sửa", nhomChucNang);
    QPushButton *btnDetailTop = new QPushButton(QIcon(":/icons/info.svg"), " Chi Tiết", nhomChucNang);
    QPushButton *btnDeleteTop = new QPushButton(QIcon(":/icons/trash.svg"), " Xóa", nhomChucNang);

    // Use QSS classes for consistent look
    btnAddReader->setProperty("class", "Primary");
    btnImport->setProperty("class", "Secondary");
    btnPrint->setProperty("class", "Secondary");
    btnEditTop->setProperty("class", "Secondary");
    btnDetailTop->setProperty("class", "Secondary");
    btnDeleteTop->setProperty("class", "Danger");

    // Set minimum heights to match other controls
    QList<QPushButton*> topButtons = {btnAddReader, btnImport, btnPrint, btnEditTop, btnDetailTop, btnDeleteTop};
    for (QPushButton *b : topButtons) {
        b->setMinimumHeight(40);
        b->setProperty("compact", true);
    }

    // Add subtle shadow to primary button for emphasis
    QGraphicsDropShadowEffect *shadowPrimary = new QGraphicsDropShadowEffect(btnAddReader);
    shadowPrimary->setBlurRadius(12);
    shadowPrimary->setOffset(0, 3);
    shadowPrimary->setColor(QColor(0,0,0,80));
    btnAddReader->setGraphicsEffect(shadowPrimary);

    layChucNang->addWidget(btnAddReader);
    layChucNang->addWidget(btnImport);
    layChucNang->addWidget(btnPrint);
    layChucNang->addWidget(btnEditTop);
    layChucNang->addWidget(btnDetailTop);
    layChucNang->addWidget(btnDeleteTop);
    layChucNang->addStretch();

    mainLayout->addWidget(nhomChucNang);
    
    // --- CARD 2: BẢNG DỮ LIỆU ---
    QFrame *tableCard = new QFrame(this);
    tableCard->setProperty("class", "Card");
    tableCard->setFrameShape(QFrame::NoFrame);
    QVBoxLayout *tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(1, 1, 1, 1);
    
    // Toolbar
    toolbar = new QToolBar(tableCard);
    toolbar->setIconSize(QSize(18, 18));
    QAction *actExportTop = toolbar->addAction(QIcon(":/icons/copy.svg"), "Xuất CSV");
    connect(actExportTop, &QAction::triggered, this, &WidgetQuanLyDocGia::onExportCSV);
    toolbar->addSeparator();
    QAction *actPrintTop = toolbar->addAction(QIcon(":/icons/info.svg"), "In Danh Sách");
    actPrintTop->setShortcut(QKeySequence::Print);
    connect(actPrintTop, &QAction::triggered, this, &WidgetQuanLyDocGia::onPrintList);
    tableLayout->addWidget(toolbar);
    
    tableView = new QTableView(tableCard);
    readerModel = new ReaderTableModel(this);
    tableView->setModel(readerModel);
    // Use badge delegate for status column
    tableView->setItemDelegateForColumn(4, badgeDelegate);
    
    // Cấu hình bảng
    tableView->setShowGrid(false);
    tableView->verticalHeader()->setVisible(false);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setAlternatingRowColors(true);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tableView->setSortingEnabled(true);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // Row height aligned with Quản Lý Sách for visual parity
    tableView->verticalHeader()->setDefaultSectionSize(45);
    tableView->setMinimumHeight(420);
    // Use centralized InnerTable styling
    tableView->setProperty("class", "InnerTable");
    
    tableLayout->addWidget(tableView);
    
    // Footer Actions
    QHBoxLayout *footerLayout = new QHBoxLayout();
    footerLayout->setContentsMargins(20, 10, 20, 20);
    
    statusLabel = new QLabel("Tổng số: 0 học viên", tableCard);
    statusLabel->setProperty("class", "SubText");
    
    btnEdit = new QPushButton("Sửa", tableCard);
    btnEdit->setProperty("class", "Secondary");
    btnEdit->setIcon(QIcon(":/icons/edit.svg"));
    btnEdit->setProperty("compact", true);
    
    btnDelete = new QPushButton("Xóa", tableCard);
    btnDelete->setProperty("class", "Danger");
    btnDelete->setIcon(QIcon(":/icons/trash.svg"));
    btnDelete->setProperty("compact", true);
    
    btnDetails = new QPushButton("Chi Tiết", tableCard);
    btnDetails->setProperty("class", "Secondary");
    btnDetails->setIcon(QIcon(":/icons/info.svg"));
    btnDetails->setProperty("compact", true);
    
    btnUnlock = new QPushButton("Mở Khóa", tableCard);
    btnUnlock->setProperty("class", "Secondary");
    btnUnlock->setIcon(QIcon(":/icons/unlock.svg"));
    btnUnlock->setProperty("compact", true);
    
    btnBack = new QPushButton("Quay Lại", tableCard);
    btnBack->setProperty("class", "Secondary");
    btnBack->setIcon(QIcon(":/icons/refresh.svg"));
    btnBack->setProperty("compact", true);
    
    footerLayout->addWidget(statusLabel);
    footerLayout->addStretch();
    footerLayout->addWidget(btnDetails);
    footerLayout->addWidget(btnEdit);
    footerLayout->addWidget(btnUnlock);
    footerLayout->addWidget(btnDelete);
    footerLayout->addWidget(btnBack);
    
    tableLayout->addLayout(footerLayout);
    mainLayout->addWidget(tableCard);
}

void WidgetQuanLyDocGia::setupTabQuanLy() {
    tabQuanLy = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tabQuanLy);
    layout->setSpacing(12);
    layout->setContentsMargins(12, 12, 12, 12);
    
    // Toolbar - use dark modern style consistent with WidgetQuanLySach
    toolbar = new QToolBar(tabQuanLy);
    toolbar->setIconSize(QSize(20, 20));
    // Use QSS class for toolbar styling to keep styles centralized
    toolbar->setProperty("class", "ToolbarDark");
    
    QAction *actToggleStatus = toolbar->addAction(QIcon(":/icons/lock.svg"), "Đổi Trạng Thái");
    actToggleStatus->setToolTip("Khóa/Mở khóa học viên được chọn");
    connect(actToggleStatus, &QAction::triggered, this, &WidgetQuanLyDocGia::onToggleStatus);
    
    QAction *actHistory = toolbar->addAction(QIcon(":/icons/exchange.svg"), "Lịch Sử Mượn");
    actHistory->setToolTip("Xem lịch sử mượn trả của học viên");
    connect(actHistory, &QAction::triggered, this, &WidgetQuanLyDocGia::onViewHistory);
    
    QAction *actPrintCard = toolbar->addAction(QIcon(":/icons/user.svg"), "In Thẻ");
    actPrintCard->setToolTip("In thẻ học viên (Ctrl+P)");
    actPrintCard->setShortcut(QKeySequence::Print);
    connect(actPrintCard, &QAction::triggered, this, &WidgetQuanLyDocGia::onPrintCard);
    
    toolbar->addSeparator();
    
    QAction *actWarnings = toolbar->addAction(QIcon(":/icons/stats.svg"), "Cảnh Báo");
    actWarnings->setToolTip("Xem các cảnh báo (quá hạn, khóa đang mượn...)");
    connect(actWarnings, &QAction::triggered, this, &WidgetQuanLyDocGia::onShowWarnings);
    
    toolbar->addSeparator();
    
    QAction *actExport = toolbar->addAction(QIcon(":/icons/copy.svg"), "Xuất CSV");
    actExport->setToolTip("Xuất danh sách học viên ra CSV");
    connect(actExport, &QAction::triggered, this, &WidgetQuanLyDocGia::onExportCSV);
    
    QAction *actImport = toolbar->addAction(QIcon(":/icons/copy.svg"), "Nhập CSV");
    actImport->setToolTip("Nhập học viên từ file CSV");
    connect(actImport, &QAction::triggered, this, &WidgetQuanLyDocGia::onImportCSV);
    
    layout->addWidget(toolbar);
    
    // Thanh tim kiem
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->addWidget(new QLabel("Tìm kiếm:", tabQuanLy));
    searchEdit = new QLineEdit(tabQuanLy);
    searchEdit->setPlaceholderText("Nhập mã thẻ, tên, họ...");
    searchEdit->setMinimumHeight(40);
    searchEdit->setProperty("class", "InputDark");
    searchLayout->addWidget(searchEdit);
    
    // Advanced search combo
    searchTypeCombo = new QComboBox(tabQuanLy);
    searchTypeCombo->addItem("Tìm theo mã", 0);
    searchTypeCombo->addItem("Tìm theo tên", 1);
    searchTypeCombo->addItem("Tìm theo họ", 2);
    searchTypeCombo->addItem("Tìm theo họ tên", 3);
    searchTypeCombo->addItem("Lọc theo trạng thái", 4);
    searchTypeCombo->setToolTip(
        "Loại tìm kiếm:\n"
        "• Theo mã: Nhập mã thẻ học viên (số nguyên)\n"
        "• Theo tên: Tìm theo tên (VD: 'Lan')\n"
        "• Theo họ: Tìm theo họ đệm (VD: 'Nguyễn')\n"
        "• Theo họ tên: Tìm theo họ và tên\n"
        "• Lọc trạng thái: 0=Khóa, 1=Hoạt động"
    );
    searchTypeCombo->setMinimumHeight(40);
    searchTypeCombo->setMinimumWidth(180);
    searchLayout->addWidget(searchTypeCombo);

    btnSearch = new QPushButton("Tìm", tabQuanLy);
    btnSearch->setToolTip("Tìm kiếm học viên (Enter hoặc Ctrl+F)");
    btnSearch->setShortcut(QKeySequence(Qt::Key_Return));
    btnSearch->setMinimumHeight(40);
    btnSearch->setProperty("class", "GradientPrimary");
    btnClearSearch = new QPushButton("Xóa", tabQuanLy);
    btnClearSearch->setMinimumHeight(40);
    btnClearSearch->setProperty("class", "Secondary");
    searchLayout->addWidget(btnSearch);
    searchLayout->addWidget(btnClearSearch);
    
    layout->addLayout(searchLayout);
    
    // Filter controls
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(12);
    
    QLabel *filterLabel = new QLabel("Lọc nhanh:", tabQuanLy);
    filterLabel->setProperty("class", "CardTitle");
    filterLayout->addWidget(filterLabel);
    
    filterGioiTinhCombo = new QComboBox(tabQuanLy);
    filterGioiTinhCombo->addItem("Tất cả giới tính");
    filterGioiTinhCombo->addItem("Nam");
    filterGioiTinhCombo->addItem("Nữ");
    filterGioiTinhCombo->setMinimumWidth(150);
    filterGioiTinhCombo->setToolTip("Lọc theo giới tính");
    filterLayout->addWidget(filterGioiTinhCombo);
    
    filterTrangThaiCombo = new QComboBox(tabQuanLy);
    filterTrangThaiCombo->addItem("Tất cả trạng thái");
    filterTrangThaiCombo->addItem("Hoạt động");
    filterTrangThaiCombo->addItem("Khóa");
    filterTrangThaiCombo->addItem("Quá hạn");
    filterTrangThaiCombo->setMinimumWidth(150);
    filterTrangThaiCombo->setToolTip("Lọc theo trạng thái");
    filterLayout->addWidget(filterTrangThaiCombo);
    
    filterLayout->addStretch();
    layout->addLayout(filterLayout);
    
    // Connect filter combo boxes
    connect(filterGioiTinhCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &WidgetQuanLyDocGia::applyFilters);
    connect(filterTrangThaiCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &WidgetQuanLyDocGia::applyFilters);
    
    // Bang hien thi
    tableView = new QTableView(tabQuanLy);
    readerModel = new ReaderTableModel(this);
    tableView->setModel(readerModel);
    // Use badge delegate for status column in tab view as well
    tableView->setItemDelegateForColumn(4, badgeDelegate);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setAlternatingRowColors(true);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setSortingEnabled(true); // Enable column sorting
    // Match visual style to WidgetQuanLySach
    tableView->verticalHeader()->setDefaultSectionSize(45);
    tableView->setProperty("class", "InnerTable");
    
    layout->addWidget(tableView);
    
    // Thanh trang thai
    statusLabel = new QLabel("", tabQuanLy);
    statusLabel->setProperty("class", "SubText");
    layout->addWidget(statusLabel);
    
    // Cac nut chuc nang
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    btnAdd = new QPushButton("+ Thêm Học Viên", tabQuanLy);
    btnAdd->setIcon(QIcon(":/icons/plus.svg"));
    btnAdd->setMinimumHeight(40);
    btnEdit = new QPushButton("Sửa", tabQuanLy);
    btnEdit->setIcon(QIcon(":/icons/edit.svg"));
    btnEdit->setMinimumHeight(40);
    btnDelete = new QPushButton("Xóa", tabQuanLy);
    btnDelete->setIcon(QIcon(":/icons/trash.svg"));
    btnDelete->setMinimumHeight(40);
    btnViewLoans = new QPushButton("Xem Mượn Trả", tabQuanLy);
    btnViewLoans->setIcon(QIcon(":/icons/exchange.svg"));
    btnViewLoans->setMinimumHeight(40);
    btnBack = new QPushButton("Quay Lại", tabQuanLy);
    btnBack->setIcon(QIcon(":/icons/refresh.svg"));
    btnBack->setMinimumHeight(40);

    // Add keyboard shortcut tooltips
    btnAdd->setToolTip("Thêm học viên mới (Ctrl+N)");
    btnEdit->setToolTip("Sửa thông tin học viên (Ctrl+E)");
    btnDelete->setToolTip("Xóa học viên đã chọn (Delete hoặc Ctrl+D)");
    btnViewLoans->setToolTip("Xem lịch sử mượn trả");
    btnBack->setToolTip("Quay lại màn hình chính (ESC)");

    // Use QSS classes instead of inline styles for consistency
    btnAdd->setProperty("class", "Primary");
    btnEdit->setProperty("class", "Secondary");
    btnDelete->setProperty("class", "Danger");
    btnViewLoans->setProperty("class", "Success");
    btnBack->setProperty("class", "Secondary");
    
    buttonLayout->addWidget(btnAdd);
    buttonLayout->addWidget(btnEdit);
    buttonLayout->addWidget(btnDelete);
    buttonLayout->addWidget(btnViewLoans);
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnBack);
    
    layout->addLayout(buttonLayout);
    
    tabWidget->addTab(tabQuanLy, "Danh Sách Học Viên");
}

void WidgetQuanLyDocGia::setupTabQuaHan() {
    tabQuaHan = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tabQuaHan);
    layout->setSpacing(16);
    layout->setContentsMargins(16, 16, 16, 16);
    
    // Hướng dẫn
    QLabel *lblHuongDan = new QLabel("DANH SÁCH HỌC VIÊN QUÁ HẠN", tabQuaHan);
    QFont font = lblHuongDan->font();
    font.setPointSize(16);
    font.setBold(true);
    lblHuongDan->setFont(font);
    lblHuongDan->setProperty("class", "DangerTitle");
    layout->addWidget(lblHuongDan);
    
    // Thống kê
    lblThongKeQuaHan = new QLabel("Đang tải dữ liệu...", tabQuaHan);
    lblThongKeQuaHan->setProperty("class", "DangerStat");
    layout->addWidget(lblThongKeQuaHan);
    
    // Placeholder message
    QLabel *placeholderMsg = new QLabel("Chức năng đang được phát triển...\n\n"
        "Sẽ hiển thị danh sách học viên có sách mượn quá hạn.", tabQuaHan);
    placeholderMsg->setAlignment(Qt::AlignCenter);
    placeholderMsg->setProperty("class", "PlaceholderMsg");
    layout->addWidget(placeholderMsg, 1);
    
    // Refresh button
    QPushButton *btnRefresh = new QPushButton("Làm Mới Dữ Liệu", tabQuaHan);
    btnRefresh->setIcon(QIcon(":/icons/refresh.svg"));
    btnRefresh->setProperty("class", "Danger");
    btnRefresh->setMinimumHeight(36);
    connect(btnRefresh, &QPushButton::clicked, this, &WidgetQuanLyDocGia::onRefreshQuaHan);
    layout->addWidget(btnRefresh, 0, Qt::AlignRight);
    
    tabWidget->addTab(tabQuaHan, "Học Viên Quá Hạn");
}

void WidgetQuanLyDocGia::setupConnections() {
    connect(btnSearch, &QPushButton::clicked, this, &WidgetQuanLyDocGia::onSearchClicked);
    connect(btnClearSearch, &QPushButton::clicked, this, &WidgetQuanLyDocGia::onClearSearchClicked);
    connect(btnAdd, &QPushButton::clicked, this, &WidgetQuanLyDocGia::showAddReaderDialog);
    connect(btnEdit, &QPushButton::clicked, this, &WidgetQuanLyDocGia::showEditReaderDialog);
    connect(btnDelete, &QPushButton::clicked, this, &WidgetQuanLyDocGia::onDeleteReaderClicked);
    connect(btnViewLoans, &QPushButton::clicked, this, &WidgetQuanLyDocGia::onViewLoansClicked);
    connect(btnBack, &QPushButton::clicked, this, &WidgetQuanLyDocGia::onBackClicked);
    connect(tableView, &QTableView::doubleClicked, this, &WidgetQuanLyDocGia::onTableDoubleClicked);
    connect(searchEdit, &QLineEdit::returnPressed, this, &WidgetQuanLyDocGia::onSearchClicked);
    
    // Real-time search
    connect(searchEdit, &QLineEdit::textChanged, this, &WidgetQuanLyDocGia::onSearchTextChanged);
    connect(searchTimer, &QTimer::timeout, this, &WidgetQuanLyDocGia::performSearch);
    
    connect(tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            [this]() {
                QModelIndexList selected = tableView->selectionModel()->selectedRows();
                if (!selected.isEmpty()) {
                    selectedReader = readerModel->getReaderAt(selected.first().row());
                } else {
                    selectedReader = nullptr;
                }
                updateButtonStates();
            });
                updateButtonStates();
            });
            
    // Connect Language Change
    connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this, &WidgetQuanLyDocGia::capNhatNgonNgu);
}

void WidgetQuanLyDocGia::setupKeyboardShortcuts() {
    // Ctrl+F - Focus search
    shortcutSearch = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(shortcutSearch, &QShortcut::activated, this, [this]() {
        searchEdit->setFocus();
        searchEdit->selectAll();
    });
    
    // Ctrl+N - Add reader
    shortcutAdd = new QShortcut(QKeySequence("Ctrl+N"), this);
    connect(shortcutAdd, &QShortcut::activated, this, &WidgetQuanLyDocGia::showAddReaderDialog);
    
    // Ctrl+E - Edit reader
    shortcutEdit = new QShortcut(QKeySequence("Ctrl+E"), this);
    connect(shortcutEdit, &QShortcut::activated, this, [this]() {
        if (selectedReader) {
            showEditReaderDialog();
        }
    });
    
    // Delete / Ctrl+D - Delete reader
    shortcutDelete = new QShortcut(QKeySequence("Delete"), this);
    connect(shortcutDelete, &QShortcut::activated, this, [this]() {
        if (selectedReader) {
            onDeleteReaderClicked();
        }
    });
    
    QShortcut *shortcutDeleteAlt = new QShortcut(QKeySequence("Ctrl+D"), this);
    connect(shortcutDeleteAlt, &QShortcut::activated, this, [this]() {
        if (selectedReader) {
            onDeleteReaderClicked();
        }
    });
    
    // F5 / Ctrl+R - Refresh
    shortcutRefresh = new QShortcut(QKeySequence("F5"), this);
    connect(shortcutRefresh, &QShortcut::activated, this, &WidgetQuanLyDocGia::refreshData);
    
    QShortcut *shortcutRefreshAlt = new QShortcut(QKeySequence("Ctrl+R"), this);
    connect(shortcutRefreshAlt, &QShortcut::activated, this, &WidgetQuanLyDocGia::refreshData);
}

void WidgetQuanLyDocGia::applyFilters() {
    // Get filter values
    QString gioiTinh = filterGioiTinhCombo->currentText();
    QString trangThai = filterTrangThaiCombo->currentText();
    
    // Store original search text
    QString searchText = searchEdit->text();
    
    // Apply filtering logic in performSearch
    performSearch();
    
    // Update status message
    int totalCount = readerModel->rowCount();
    QString filterMsg;
    if (gioiTinh != "Tất cả" || trangThai != "Tất cả") {
        filterMsg = QString(" (Lọc: %1, %2)").arg(gioiTinh).arg(trangThai);
    }
    showMessage(QString("Hiển thị %1 học viên%2").arg(totalCount).arg(filterMsg));
}

QString WidgetQuanLyDocGia::validateCMND(const QString &cmnd) {
    // Check empty
    if (cmnd.trimmed().isEmpty()) {
        return "CMND/CCCD không được để trống";
    }
    
    // Check format: 9 or 12 digits
    QRegularExpression regex("^[0-9]{9}$|^[0-9]{12}$");
    QRegularExpressionMatch match = regex.match(cmnd.trimmed());
    
    if (!match.hasMatch()) {
        return "CMND/CCCD phải có 9 hoặc 12 chữ số";
    }
    
    return ""; // Valid
}

void WidgetQuanLyDocGia::refreshData() {
    readerModel->refreshData();
    selectedReader = nullptr;
    updateButtonStates();
    showMessage(QString("Đã tải %1 học viên").arg(readerModel->rowCount()));
}

void WidgetQuanLyDocGia::updateButtonStates() {
    bool hasSelection = (selectedReader != nullptr);
    btnEdit->setEnabled(hasSelection);
    btnDelete->setEnabled(hasSelection);
    btnViewLoans->setEnabled(hasSelection);
}

void WidgetQuanLyDocGia::showMessage(const QString &message, bool isError) {
    SafeGui::safeSetText(statusLabel, message);
    SafeGui::safeSetStyleSheet(statusLabel, isError ? "color: #e74c3c; font-weight: bold;" : "color: #27ae60; font-weight: bold;");
}

void WidgetQuanLyDocGia::onSearchClicked() {
    QString keyword = searchEdit->text().trimmed();
    int searchType = searchTypeCombo->currentData().toInt();
    
    extern PTRDG rootDocGia;
    int count = 0;
    
    // Validate root pointer
    if (!rootDocGia) {
        showMessage("Danh sách học viên trống!", true);
        return;
    }
    
    if (searchType == 0) {
        // Search by card number
        bool ok;
        int maThe = keyword.toInt(&ok);
        
        if (!ok || maThe <= 0) {
            showMessage("Vui lòng nhập mã thẻ hợp lệ!", true);
            return;
        }
        
        PTRDG reader = nullptr;
        try {
            reader = timDocGia(rootDocGia, maThe);
        } catch (...) {
            showMessage(LanguageManager::instance().getText("MSG_ERROR_SEARCH_READER"), true);
            return;
        }

        if (reader) {
            std::vector<PTRDG> v{reader};
            readerModel->setSearchResults(v);
            showMessage(QString(" Tìm thấy học viên: %1 %2")
                .arg(QString::fromStdString(reader->data.Ho),
                     QString::fromStdString(reader->data.Ten)));
        } else {
            showMessage(LanguageManager::instance().getText("MSG_READER_NOT_FOUND_SIMPLE"), true);
            // clear model
            readerModel->setSearchResults(std::vector<PTRDG>());
        }
        
    } else if (searchType == 1) {
        // Search by first name
        if (keyword.isEmpty()) {
            showMessage(LanguageManager::instance().getText("MSG_ENTER_NAME"), true);
            return;
        }
        
            // Run search in background
            try {
                showMessage(LanguageManager::instance().getText("MSG_SEARCHING_PROGRESS"));
                btnSearch->setEnabled(false);
                searchProgress->setVisible(true);
                QFuture<std::vector<PTRDG>> future = QtConcurrent::run(&DBBatcher::timDocGiaTheoTen, keyword.toStdString(), 1000);
                searchWatcher->setFuture(future);
            } catch (...) {
                showMessage(LanguageManager::instance().getText("MSG_ERROR_SEARCH"), true);
                btnSearch->setEnabled(true);
                return;
            }
        
    } else if (searchType == 2) {
        // Search by last name
        if (keyword.isEmpty()) {
            showMessage(LanguageManager::instance().getText("MSG_ENTER_LASTNAME"), true);
            return;
        }
        
        try {
            showMessage(LanguageManager::instance().getText("MSG_SEARCHING_PROGRESS"));
            btnSearch->setEnabled(false);
            searchProgress->setVisible(true);
            QFuture<std::vector<PTRDG>> future = QtConcurrent::run(&DBBatcher::timDocGiaTheoHo, keyword.toStdString(), 1000);
            searchWatcher->setFuture(future);
        } catch (...) {
            showMessage(LanguageManager::instance().getText("MSG_ERROR_SEARCH"), true);
            btnSearch->setEnabled(true);
            return;
        }
        
    } else if (searchType == 3) {
        // Search by full name
        if (keyword.isEmpty()) {
            showMessage(LanguageManager::instance().getText("MSG_ENTER_FULLNAME"), true);
            return;
        }
        
        try {
            showMessage(LanguageManager::instance().getText("MSG_SEARCHING_PROGRESS"));
            btnSearch->setEnabled(false);
            searchProgress->setVisible(true);
            QFuture<std::vector<PTRDG>> future = QtConcurrent::run(&DBBatcher::timDocGiaTheoHoTen, keyword.toStdString(), 1000);
            searchWatcher->setFuture(future);
        } catch (...) {
            showMessage(LanguageManager::instance().getText("MSG_ERROR_SEARCH"), true);
            btnSearch->setEnabled(true);
            return;
        }
        
    } else if (searchType == 4) {
        // Filter by status
        QStringList options;
        options << LanguageManager::instance().getText("OPTION_STATUS_ACTIVE") << LanguageManager::instance().getText("OPTION_STATUS_LOCKED");
        
        bool ok;
        QString item = QInputDialog::getItem(this, LanguageManager::instance().getText("TITLE_FILTER_STATUS"),
                                            LanguageManager::instance().getText("LBL_CHOOSE_STATUS"), options, 0, false, &ok);
        if (!ok) return;
        
        int status = item.left(1).toInt();
        
        try {
            showMessage(LanguageManager::instance().getText("MSG_FILTERING_PROGRESS"));
            btnSearch->setEnabled(false);
            QFuture<std::vector<PTRDG>> future = QtConcurrent::run(&DBBatcher::timDocGiaTheoTrangThai, status, 1000);
            searchWatcher->setFuture(future);
        } catch (...) {
            showMessage(LanguageManager::instance().getText("MSG_ERROR_SEARCH"), true);
            btnSearch->setEnabled(true);
            return;
        }
        // onSearchFinished will display result count
    }
}

void WidgetQuanLyDocGia::onClearSearchClicked() {
    searchEdit->clear();
    searchTypeCombo->setCurrentIndex(0);
    refreshData();
}

void WidgetQuanLyDocGia::showAddReaderDialog() {
    ManHinhThemDocGia dialog(this);
    connect(&dialog, &ManHinhThemDocGia::docGiaDaThem, this, &WidgetQuanLyDocGia::refreshData);
    dialog.exec();
}

void WidgetQuanLyDocGia::showEditReaderDialog() {
    if (!selectedReader) return;
    
    ManHinhSuaDocGia dialog(QString::number(selectedReader->data.MaThe), this);
    connect(&dialog, &ManHinhSuaDocGia::docGiaDaCapNhat, this, &WidgetQuanLyDocGia::refreshData);
    dialog.exec();
}

void WidgetQuanLyDocGia::onDeleteReaderClicked() {
    if (!selectedReader) return;
    
    // Build detailed info with borrowing status
    QString detail;
    int borrowedCount = readerModel->getBorrowedBookCount(selectedReader);
    bool isOverdue = readerModel->isReaderOverdue(selectedReader);
    
    detail += QString("📇 <b>Mã thẻ:</b> %1<br>").arg(selectedReader->data.MaThe);
    detail += QString("<b>Họ tên:</b> %1 %2<br>")
              .arg(QString::fromStdString(selectedReader->data.Ho))
              .arg(QString::fromStdString(selectedReader->data.Ten));
    detail += QString("<b>Giới tính:</b> %1<br>")
              .arg(selectedReader->data.Phai ? "Nam" : "Nữ");
    detail += QString("<b>Trạng thái:</b> %1<br>")
              .arg(selectedReader->data.TrangThai ? "Hoạt động" : "Khóa");
    
    // Borrowing status with warnings
    if (borrowedCount > 0) {
        detail += QString("<br><span style='color: #F59E0B; font-weight: bold;'>Đang mượn: %1 cuốn</span><br>")
                  .arg(borrowedCount);
        if (isOverdue) {
            detail += QString("<span style='color: #EF4444; font-weight: bold;'>CÓ SÁCH QUÁ HẠN!</span><br>");
        }
    } else {
        detail += QString("<br><span style='color: #10B981;'>✓ Không có sách đang mượn</span><br>");
    }
    
    // Warning messages
    QString warningMsg;
    if (borrowedCount > 0) {
        warningMsg = "\n\nLƯU Ý:\n";
        warningMsg += "• Học viên đang mượn sách, nên xử lý trả sách trước\n";
        warningMsg += "• Xóa sẽ mất toàn bộ lịch sử mượn trả\n";
        warningMsg += "• Hành động này không thể hoàn tác";
    } else {
        warningMsg = "\n\nLịch sử mượn trả của học viên này sẽ bị xóa vĩnh viễn.";
    }
    
    // Use ConfirmationDialog with warning type if has borrowed books
    ConfirmationDialog::Type dialogType = (borrowedCount > 0) 
        ? ConfirmationDialog::Type::Warning 
        : ConfirmationDialog::Type::Delete;
    
    bool confirmed = ConfirmationDialog::confirm(
        this,
        "Xác nhận xóa độc giả",
        QString("Bạn có chắc chắn muốn xóa học viên này?%1").arg(warningMsg),
        detail,
        dialogType
    );
    
    if (confirmed) {
        // TODO: Call backend delete function
        // For now, just show placeholder
        AnimationHelper::shake(this);
        QMessageBox::information(this, "Xóa Học Viên", 
            "Chức năng xóa học viên đang được phát triển.\n"
            "Cần implement hàm XoaDocGia() trong backend.");
        
        // When implemented:
        // extern NODEDG* rootDocGia;
        // XoaDocGia(rootDocGia, selectedReader->data.MaThe);
        // extern bool duLieuDaThayDoi;
        // duLieuDaThayDoi = true;
        // refreshData();
        // showMessage(QString("✅ Đã xóa độc giá: %1 %2")
        //     .arg(QString::fromStdString(selectedReader->data.Ho))
        //     .arg(QString::fromStdString(selectedReader->data.Ten)));
    }
}

void WidgetQuanLyDocGia::onViewLoansClicked() {
    if (!selectedReader) return;
    QMessageBox::information(this, "Xem Mượn Trả", "Chức năng đang phát triển...");
}

void WidgetQuanLyDocGia::onBackClicked() {
    emit backToHome();
}

void WidgetQuanLyDocGia::onTableDoubleClicked(const QModelIndex &index) {
    Q_UNUSED(index);
    if (selectedReader) {
        onViewHistory(); // Changed to view history instead
    }
}

// ==================== NEW ADVANCED FEATURES ====================

void WidgetQuanLyDocGia::onSearchTextChanged() {
    searchTimer->stop();
    if (searchTypeCombo->currentData().toInt() == 0) {
        searchTimer->start();
    }
}

void WidgetQuanLyDocGia::performSearch() {
    QString keyword = searchEdit->text().trimmed();
    if (keyword.isEmpty()) {
        onClearSearchClicked();
        return;
    }
    if (searchTypeCombo->currentData().toInt() == 0) {
        try {
            onSearchClicked();
        } catch (...) {
            showMessage("Lỗi khi thực hiện tìm kiếm!", true);
        }
    }
}

void WidgetQuanLyDocGia::onToggleStatus() {
    if (!selectedReader) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng chọn học viên để đổi trạng thái!");
        return;
    }
    
    QString currentStatus = selectedReader->data.TrangThai ? "Hoạt động" : "Khóa";
    QString newStatus = selectedReader->data.TrangThai ? "Khóa" : "Hoạt động";
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Xác nhận đổi trạng thái",
        QString("Học viên: %1 %2\nTrạng thái hiện tại: %3\n\n"
                "Bạn có muốn chuyển sang trạng thái: %4?")
            .arg(QString::fromStdString(selectedReader->data.Ho))
            .arg(QString::fromStdString(selectedReader->data.Ten))
            .arg(currentStatus)
            .arg(newStatus),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        selectedReader->data.TrangThai = !selectedReader->data.TrangThai;
        extern bool duLieuDaThayDoi;
        duLieuDaThayDoi = true;
        refreshData();
        showMessage(QString("✅ Đã chuyển trạng thái sang: %1").arg(newStatus));
    }
}

void WidgetQuanLyDocGia::onViewHistory() {
    if (!selectedReader) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng chọn học viên để xem lịch sử!");
        return;
    }
    
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    
    // Build history HTML
    QString html = "<html><head><style>"
                   "body { font-family: Arial, sans-serif; }"
                   "h2 { color: #2c3e50; }"
                   "table { width: 100%; border-collapse: collapse; margin-top: 10px; }"
                   "th { background-color: #3498db; color: white; padding: 8px; text-align: left; }"
                   "td { border: 1px solid #ddd; padding: 8px; }"
                   "tr:nth-child(even) { background-color: #f2f2f2; }"
                   ".returned { color: #27ae60; }"
                   ".borrowing { color: #e74c3c; font-weight: bold; }"
                   "</style></head><body>";
    
    html += QString("<h2>📚 Lịch Sử Mượn Trả</h2>");
    html += QString("<p><b>Học viên:</b> %1 %2 (Mã thẻ: %3)</p>")
            .arg(QString::fromStdString(selectedReader->data.Ho))
            .arg(QString::fromStdString(selectedReader->data.Ten))
            .arg(selectedReader->data.MaThe);
    
    MUONTRA mt = selectedReader->data.dsmt;
    int total = 0;
    int borrowing = 0;
    
    // Count first
    MUONTRA temp = mt;
    while (temp) {
        total++;
        if (temp->data.TrangThai == 0) borrowing++;
        temp = temp->next;
    }
    
    html += QString("<p><b>Tổng số giao dịch:</b> %1 | <b class='borrowing'>Đang mượn:</b> %2</p>")
            .arg(total, borrowing);
    
    if (total == 0) {
        html += "<p style='color: #95a5a6;'>Chưa có giao dịch nào.</p>";
    } else {
        html += "<table>";
        html += "<tr>"
                "<th>STT</th>"
                "<th>Mã Sách</th>"
                "<th>Tên Sách</th>"
                "<th>Ngày Mượn</th>"
                "<th>Ngày Trả</th>"
                "<th>Trạng Thái</th>"
                "</tr>";
        
        int stt = 1;
        while (mt) {
            // Find book name
            QString bookName = "N/A";
            QString maSach = "N/A";
            if (mt->data.banSaoSach) {
                maSach = QString::fromStdString(mt->data.banSaoSach->maSach);
                for (int i = 0; i < soLuongDauSach; i++) {
                    PTRDMS dms = dsDauSach[i]->dms;
                    while (dms) {
                        if (dms->maSach == mt->data.banSaoSach->maSach) {
                            bookName = QString::fromStdString(dsDauSach[i]->tenSach);
                            break;
                        }
                        dms = dms->next;
                    }
                    if (bookName != "N/A") break;
                }
            }
            
            QString status = mt->data.TrangThai == 0 ? 
                "<span class='borrowing'>Đang mượn</span>" : 
                "<span class='returned'>Đã trả</span>";
            
            QString ngayTra = mt->data.TrangThai == 0 ? "-" : QString::fromStdString(mt->data.NgayTra);
            
            html += QString("<tr>"
                           "<td>%1</td>"
                           "<td>%2</td>"
                           "<td>%3</td>"
                           "<td>%4</td>"
                           "<td>%5</td>"
                           "<td>%6</td>"
                           "</tr>")
                    .arg(stt++)
                    .arg(maSach)
                    .arg(bookName)
                    .arg(QString::fromStdString(mt->data.NgayMuon))
                    .arg(ngayTra)
                    .arg(status);
            
            mt = mt->next;
        }
        html += "</table>";
    }
    
    html += "</body></html>";
    
    // Show dialog
    QDialog dialog(this);
    dialog.setWindowTitle("Lịch Sử Mượn Trả");
    dialog.resize(800, 500);
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QTextEdit *textEdit = new QTextEdit(&dialog);
    textEdit->setHtml(html);
    textEdit->setReadOnly(true);
    layout->addWidget(textEdit);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    layout->addWidget(buttonBox);
    
    dialog.exec();
}

void WidgetQuanLyDocGia::onPrintCard() {
    if (!selectedReader) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng chọn học viên để in thẻ!");
        return;
    }
    
    // Use centralized PrintHelper
    // Correctly handles formatting, barcodes, and print dialogs
    PrintHelper::inTheDocGia(selectedReader, this);
    
    // Success message is handled by user interaction or implicit success
}

void WidgetQuanLyDocGia::onShowWarnings() {
    // extern PTRDS dsDauSach[MAX_DAUSACH];  // Unused variables - commented out
    // extern int soLuongDauSach;
    
    QString warnings;
    int warnCount = 0;
    
    // Collect warnings from all readers
    PTRDG tempReaders[1000];
    int count = 0;
    
    std::function<void(PTRDG)> collectAll = [&](PTRDG root) {
        if (!root || count >= 1000) return;
        collectAll(root->left);
        tempReaders[count++] = root;
        collectAll(root->right);
    };
    
    collectAll(rootDocGia);
    
    for (int i = 0; i < count; i++) {
        PTRDG reader = tempReaders[i];
        
        // Warning: Locked but has borrowed books
        if (!reader->data.TrangThai) {
            int borrowing = 0;
            MUONTRA mt = reader->data.dsmt;
            while (mt) {
                if (mt->data.TrangThai == 0) borrowing++;
                mt = mt->next;
            }
            
            if (borrowing > 0) {
                warnings += QString("Học viên KHÓA nhưng còn %1 sách chưa trả: %2 %3 (Mã: %4)\n")
                            .arg(borrowing)
                            .arg(QString::fromStdString(reader->data.Ho))
                            .arg(QString::fromStdString(reader->data.Ten))
                            .arg(reader->data.MaThe);
                warnCount++;
            }
        }
        
        // Warning: Overdue books
        int maxDays = TinhSoNgayQuaHanLonNhat(reader);
        
        if (maxDays >= 7) {
            int overdue = 0;
            MUONTRA mt = reader->data.dsmt;
            while (mt) {
                if (mt->data.TrangThai == 0) {
                    overdue++;
                }
                mt = mt->next;
            }
            
            warnings += QString("🔴 Quá hạn %1 ngày: %2 %3 (Mã: %4) - %5 sách\n")
                        .arg(maxDays)
                        .arg(QString::fromStdString(reader->data.Ho))
                        .arg(QString::fromStdString(reader->data.Ten))
                        .arg(reader->data.MaThe)
                        .arg(overdue);
            warnCount++;
        }
    }
    
    if (warnCount == 0) {
        QMessageBox::information(this, "Cảnh Báo ✅", 
            "Không có cảnh báo nào!\n\n"
            "✓ Tất cả học viên đều ổn định.");
    } else {
        QMessageBox::warning(this, QString("Cảnh Báo (%1)").arg(warnCount),
            QString("Phát hiện %1 vấn đề:\n\n%2").arg(warnCount).arg(warnings));
    }
}

void WidgetQuanLyDocGia::onPrintList() {
    extern PTRDG rootDocGia;
    if (!rootDocGia) {
        showMessage("Danh sách học viên trống!", true);
        return;
    }
    
    // Use centralized PrintHelper
    PrintHelper::inDanhSachDocGia(rootDocGia, this);
}

void WidgetQuanLyDocGia::onExportCSV() {
    QString fileName = QFileDialog::getSaveFileName(this,
        "Export danh sách học viên", 
        "readers_export.csv",
        "CSV Files (*.csv);;All Files (*)");
    
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Lỗi", "Không thể mở file để ghi!");
        return;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    
    // Header
    out << "Mã Thẻ,Họ,Tên,Giới Tính,Trạng Thái,Số Sách Đang Mượn\n";
    
    // Collect all readers
    PTRDG allReaders[1000];
    int totalCount = 0;
    
    std::function<void(PTRDG)> collect = [&](PTRDG root) {
        if (!root || totalCount >= 1000) return;
        collect(root->left);
        allReaders[totalCount++] = root;
        collect(root->right);
    };
    
    collect(rootDocGia);
    
    for (int i = 0; i < totalCount; i++) {
        PTRDG reader = allReaders[i];
        
        // Count borrowing books
        int borrowing = 0;
        MUONTRA mt = reader->data.dsmt;
        while (mt) {
            if (mt->data.TrangThai == 0) borrowing++;
            mt = mt->next;
        }
        
        out << reader->data.MaThe << ","
            << "\"" << QString::fromStdString(reader->data.Ho) << "\","
            << "\"" << QString::fromStdString(reader->data.Ten) << "\","
            << (reader->data.Phai ? "Nữ" : "Nam") << ","
            << (reader->data.TrangThai ? "Hoạt động" : "Khóa") << ","
            << borrowing << "\n";
    }
    
    file.close();
    
    QMessageBox::information(this, "Thành công ✅",
        QString("Đã export %1 học viên ra file:\n%2")
        .arg(totalCount).arg(fileName));
}

void WidgetQuanLyDocGia::onImportCSV() {
    // Ask whether user wants native CSV import or Excel (.xlsx via tools script)
    QMessageBox msg(this);
    msg.setWindowTitle("Chọn nguồn Import");
    msg.setText("Bạn muốn import từ file CSV (nội bộ) hay Excel (.xlsx) (cần script tools/import_from_excel.py)?");
    QPushButton *btnCsv = msg.addButton("CSV (nội bộ)", QMessageBox::AcceptRole);
    QPushButton *btnXlsx = msg.addButton("Excel (.xlsx)", QMessageBox::AcceptRole);
    msg.addButton(QMessageBox::Cancel);
    msg.exec();

    if (msg.clickedButton() == nullptr) return;

    // --- CSV (native) flow ---
    if (msg.clickedButton() == btnCsv) {
        QMessageBox::StandardButton reply = QMessageBox::question(this,
            "Xác nhận Import",
            "Import sẽ THÊM độc giả mới từ CSV.\n"
            "Học viên trùng Mã Thẻ sẽ bị bỏ qua.\n\n"
            "Format CSV: ma_the,ho,ten,phai,trang_thai,so_dien_thoai,email,ngay_dang_ky\n\n"
            "Bạn có muốn tiếp tục?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) return;

        QString fileName = QFileDialog::getOpenFileName(this,
            "Import danh sách học viên",
            "",
            "CSV Files (*.csv);;All Files (*)");
        if (fileName.isEmpty()) return;

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Lỗi", "Không thể mở file để đọc!");
            return;
        }

        QTextStream in(&file);
        in.setEncoding(QStringConverter::Utf8);

        // Skip header if present
        if (!in.atEnd()) in.readLine();

        int imported = 0;
        int skipped = 0;
        QString errors;

        extern PTRDG rootDocGia;

        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.trimmed().isEmpty()) continue;
            // Split by comma but allow quoted fields - simple implementation: split and trim quotes
            QStringList fields = line.split(',');
            if (fields.size() < 3) { skipped++; continue; }

            int maThe = fields[0].trimmed().toInt();
            QString ho = fields[1].trimmed().remove('"');
            QString ten = fields[2].trimmed().remove('"');
            bool phai = true;
            if (fields.size() > 3) phai = (fields[3].trimmed().toInt() != 0);
            int trangThai = 1;
            if (fields.size() > 4) trangThai = fields[4].trimmed().toInt();

            // Skip if maThe provided and already exists
            if (maThe > 0 && timDocGia(rootDocGia, maThe)) { skipped++; continue; }

            try {
                std::string hoStd = Utf8Helper::toStdString(ho);
                std::string tenStd = Utf8Helper::toStdString(ten);
                PTRDG node = nullptr;
                if (maThe > 0) {
                    node = taoDocGia(hoStd, tenStd, phai, trangThai, maThe);
                } else {
                    node = taoDocGia(hoStd, tenStd, phai, trangThai, rootDocGia);
                }
                if (node) {
                    InsertDocGia(rootDocGia, node);
                    imported++;
                } else {
                    skipped++;
                    errors += QString("%1 %2: Failed to create\n").arg(ho, ten);
                }
            } catch (...) {
                skipped++;
                errors += QString("%1 %2: Exception\n").arg(ho, ten);
            }
        }

        file.close();
        // Persist changes
        try { saveDocGia(rootDocGia); } catch(...) {}
        refreshData();

        QString message = QString("Đã import %1 học viên mới\n").arg(imported);
        if (skipped > 0) message += QString("Đã bỏ qua %1 dòng (trùng/lỗi)").arg(skipped);
        if (!errors.isEmpty()) message += "\n\nLỗi:\n" + errors.left(1000);
        QMessageBox::information(this, "Kết quả Import", message);
        return;
    }

    // --- Excel (.xlsx) flow via tools/import_from_excel.py ---
    QString fileName = QFileDialog::getOpenFileName(this,
        "Chọn file Excel để nhập", QDir::currentPath(), "Excel Files (*.xlsx);;All Files (*)");
    if (fileName.isEmpty()) return;

    // Find import script in multiple locations
    QString repoRoot = QDir(FilePathManager::getRootPath()).absolutePath();
    QStringList candidates;
    candidates << QDir(repoRoot).filePath("tools/import_from_excel.py");
    candidates << QDir(QCoreApplication::applicationDirPath()).filePath("tools/import_from_excel.py");
    candidates << QDir(QCoreApplication::applicationDirPath()).filePath("..\\tools/import_from_excel.py");
    candidates << QDir(QDir::currentPath()).filePath("tools/import_from_excel.py");
    QString scriptToRun;
    for (const QString &cand : candidates) { if (QFile::exists(cand)) { scriptToRun = cand; break; } }
    if (scriptToRun.isEmpty()) {
        QMessageBox::warning(this, tr("Không tìm thấy script"), tr("Không thể tìm thấy file tools/import_from_excel.py trong thư mục dự án hoặc thư mục của ứng dụng. Hãy đảm bảo file tồn tại tại 'tools/import_from_excel.py'."));
        return;
    }

    // Prefer repo venv python
    QString pythonExe;
    QString venvPython = QDir(repoRoot).filePath(".venv\\Scripts\\python.exe");
    if (QFile::exists(venvPython)) pythonExe = venvPython;
    else {
        QString appVenv = QDir(QCoreApplication::applicationDirPath()).filePath(".venv\\Scripts\\python.exe");
        if (QFile::exists(appVenv)) pythonExe = appVenv; else pythonExe = "python";
    }

    // Run preview
    QProcess proc;
    QStringList args;
    args << scriptToRun << "--file" << fileName << "--preview" << "--out" << "tools";
    proc.start(pythonExe, args);
    bool started = proc.waitForStarted(10000);
    if (!started) {
        QMessageBox::critical(this, tr("Lỗi"), tr("Không thể khởi chạy python. Hãy đảm bảo Python có trong PATH hoặc .venv được cài dependencies."));
        return;
    }
    proc.waitForFinished(-1);
    QString stdoutStr = QString::fromUtf8(proc.readAllStandardOutput());
    QString stderrStr = QString::fromUtf8(proc.readAllStandardError());

    // Build preview dialog
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Kết quả Import (Preview)"));
    dlg.setModal(true);
    QVBoxLayout *lay = new QVBoxLayout(&dlg);
    QLabel *lbl = new QLabel(tr("Kết quả preview từ script import:")); lay->addWidget(lbl);
    QTextEdit *out = new QTextEdit(); out->setReadOnly(true);
    out->setPlainText(stdoutStr + (stderrStr.isEmpty() ? QString() : QString("\n--- ERRORS ---\n") + stderrStr));
    out->setMinimumSize(700, 380); lay->addWidget(out);
    QHBoxLayout *btnLay = new QHBoxLayout(); btnLay->addStretch();
    QPushButton *btnApply = new QPushButton(tr("Áp dụng (ghi file import)"));
    QPushButton *btnOpenPreviews = new QPushButton(tr("Mở file preview"));
    QPushButton *btnClose = new QPushButton(tr("Đóng"));
    btnLay->addWidget(btnOpenPreviews); btnLay->addWidget(btnApply); btnLay->addWidget(btnClose); lay->addLayout(btnLay);
    connect(btnClose, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(btnOpenPreviews, &QPushButton::clicked, this, [this]() {
        QString folder = QDir(FilePathManager::getRootPath()).filePath("tools");
        QDesktopServices::openUrl(QUrl::fromLocalFile(folder));
    });
    connect(btnApply, &QPushButton::clicked, this, [this, &dlg, pythonExe, scriptToRun, fileName]() {
        QProcess p2;
        QStringList a2; a2 << scriptToRun << "--file" << fileName << "--apply" << "--mode" << "merge";
        p2.start(pythonExe, a2);
        if (!p2.waitForStarted(10000)) { QMessageBox::critical(this, tr("Lỗi"), tr("Không thể khởi chạy python để áp dụng import.")); return; }
        p2.waitForFinished(-1);
        QString out2 = QString::fromUtf8(p2.readAllStandardOutput());
        QString err2 = QString::fromUtf8(p2.readAllStandardError());
        QString msg = "Import applied.\n" + out2 + (err2.isEmpty()?QString():QString("\n--- ERR ---\n") + err2);
        QMessageBox::information(this, tr("Kết quả Áp dụng"), msg.left(2000));
        dlg.accept();
    });
    dlg.exec();
    // after apply/close, refresh
    refreshData();
    return;
}

void WidgetQuanLyDocGia::onRefreshQuaHan() {
    // TODO: Refresh overdue readers list
    showMessage("Chức năng Quá hạn đang được phát triển");
}

void WidgetQuanLyDocGia::onTableQuaHanDoubleClicked(const QModelIndex &index) {
    Q_UNUSED(index);
    // TODO: Handle double click on overdue readers table
}

void WidgetQuanLyDocGia::capNhatNgonNgu() {
    // 1. Header
    QList<QLabel*> labels = this->findChildren<QLabel*>();
    for(auto l : labels) {
        if(l->property("class") == "PageTitle") l->setText(LanguageManager::instance().getText("PAGE_TITLE_READERS"));
    }
    
    // 2. Buttons
    if(btnAdd) btnAdd->setText(LanguageManager::instance().getText("BTN_ADD_READER"));
    if(btnSearch) btnSearch->setText(LanguageManager::instance().getText("BTN_SEARCH"));
    if(btnClearSearch) btnClearSearch->setText(LanguageManager::instance().getText("BTN_CLEAR"));
    if(btnEdit) btnEdit->setText(LanguageManager::instance().getText("BTN_EDIT"));
    if(btnDelete) btnDelete->setText(LanguageManager::instance().getText("BTN_DELETE"));
    if(btnUnlock) btnUnlock->setText(LanguageManager::instance().getText("BTN_UNLOCK"));
    if(btnDetails) btnDetails->setText(LanguageManager::instance().getText("BTN_DETAILS"));
    if(btnBack) btnBack->setText(LanguageManager::instance().getText("BTN_Back"));
    
    // 3. Search Placeholder
    if(searchEdit) searchEdit->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_SEARCH_READER"));
    
    // 4. Refresh Table Headers and Data (to update Status text)
    if(readerModel) {
        readerModel->refreshData(); // Triggers data change which updates views
        // Force header update
        QEvent e(QEvent::LanguageChange);
        QApplication::sendEvent(tableView->horizontalHeader(), &e);
    }
}
