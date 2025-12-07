#include "views/WidgetQuanLyDocGia.h"
#include "views/dialogs/ManHinhThemDocGia.h"
#include "views/dialogs/ManHinhSuaDocGia.h"
#include "models/DocGia.h"
#include "models/QuanLySach.h"
#include "utils/ConfirmationDialog.h"
#include "utils/AnimationHelper.h"
#include "utils/DBBatcher.h"
#include "utils/NgayThang.h"
#include "utils/Utf8Helper.h"
#include "utils/SafeGui.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QTextStream>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QPageLayout>
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
    showMessage(QString("👤 Tìm thấy %1 độc giả").arg((int)results.size()));
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
            case 3: return reader->data.Phai ? "Nữ" : "Nam";
            case 4: {
                QString status = reader->data.TrangThai ? "Hoạt động" : "Khóa";
                if (isReaderOverdue(reader)) {
                    status += " ⚠ Quá hạn";
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
    
    return QVariant();
}

QVariant ReaderTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();
    
    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0: return "Mã Thẻ";
            case 1: return "Họ";
            case 2: return "Tên";
            case 3: return "Phái";
            case 4: return "Trạng Thái";
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
{
    searchTimer = new QTimer(this);
    searchTimer->setSingleShot(true);
    searchTimer->setInterval(300);
    
    // Async search watcher
    searchWatcher = new QFutureWatcher<std::vector<PTRDG>>(this);
    connect(searchWatcher, &QFutureWatcher<std::vector<PTRDG>>::finished,
            this, &WidgetQuanLyDocGia::onSearchFinished);

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
    
    // --- CARD 1: HEADER & TÌM KIẾM ---
    QFrame *headerCard = new QFrame(this);
    headerCard->setProperty("class", "Card");
    headerCard->setFrameShape(QFrame::NoFrame);
    QVBoxLayout *headerLayout = new QVBoxLayout(headerCard);
    headerLayout->setSpacing(15);
    headerLayout->setContentsMargins(20, 20, 20, 20);
    
    // Dòng 1: Tiêu đề + Nút Thêm
    QHBoxLayout *topRow = new QHBoxLayout();
    
    QLabel *titleLabel = new QLabel("👥 Quản Lý Độc Giả", headerCard);
    titleLabel->setProperty("class", "PageTitle");
    
    btnAdd = new QPushButton("+ Thêm Độc Giả", headerCard);
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
    searchTypeCombo->addItem("◈ Tìm kiếm", 0);
    searchTypeCombo->setMinimumHeight(40);
    searchTypeCombo->setMinimumWidth(120);
    filterRow->addWidget(searchTypeCombo);
    
    searchEdit = new QLineEdit(headerCard);
    searchEdit->setPlaceholderText("🔍 Tìm kiếm theo Mã thẻ, Họ tên...");
    searchEdit->setMinimumHeight(40);
    filterRow->addWidget(searchEdit, 1);
    
    filterStatusCombo = new QComboBox(headerCard);
    filterStatusCombo->addItem("Tất cả trạng thái", -1);
    filterStatusCombo->addItem("● Hoạt động", 1);
    filterStatusCombo->addItem("● Khóa", 0);
    filterStatusCombo->setMinimumHeight(40);
    filterStatusCombo->setMinimumWidth(150);
    filterRow->addWidget(filterStatusCombo);
    
    btnSearch = new QPushButton("Tìm kiếm", headerCard);
    btnSearch->setProperty("class", "Secondary");
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
    
    // --- CARD 2: BẢNG DỮ LIỆU ---
    QFrame *tableCard = new QFrame(this);
    tableCard->setProperty("class", "Card");
    tableCard->setFrameShape(QFrame::NoFrame);
    QVBoxLayout *tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(1, 1, 1, 1);
    
    // Toolbar
    toolbar = new QToolBar(tableCard);
    toolbar->setIconSize(QSize(18, 18));
    toolbar->addAction("◫ Export CSV");
    toolbar->addSeparator();
    QAction *actPrint = toolbar->addAction("▣ In Danh Sách");
    actPrint->setShortcut(QKeySequence::Print);
    tableLayout->addWidget(toolbar);
    
    tableView = new QTableView(tableCard);
    readerModel = new ReaderTableModel(this);
    tableView->setModel(readerModel);
    
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
    tableView->verticalHeader()->setDefaultSectionSize(45);
    tableView->setMinimumHeight(400);
    
    tableLayout->addWidget(tableView);
    
    // Footer Actions
    QHBoxLayout *footerLayout = new QHBoxLayout();
    footerLayout->setContentsMargins(20, 10, 20, 20);
    
    statusLabel = new QLabel("Tổng số: 0 độc giả", tableCard);
    statusLabel->setProperty("class", "SubText");
    
    btnEdit = new QPushButton("✎ Sửa", tableCard);
    btnEdit->setProperty("class", "Secondary");
    
    btnDelete = new QPushButton("× Xóa", tableCard);
    btnDelete->setProperty("class", "Danger");
    
    btnDetails = new QPushButton("◈ Chi Tiết", tableCard);
    btnDetails->setProperty("class", "Success");
    
    btnUnlock = new QPushButton("🔓 Mở Khóa", tableCard);
    btnUnlock->setProperty("class", "Success");
    
    btnBack = new QPushButton("← Quay Lại", tableCard);
    btnBack->setProperty("class", "Secondary");
    
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
    
    // Toolbar
    toolbar = new QToolBar(tabQuanLy);
    toolbar->setIconSize(QSize(20, 20));
    toolbar->setStyleSheet(
        "QToolBar {"
        "    background: #F8FAFC;"
        "    border: 1px solid #E2E8F0;"
        "    border-radius: 6px;"
        "    padding: 8px;"
        "}"
        "QToolButton {"
        "    background: white;"
        "    border: 1px solid #E2E8F0;"
        "    border-radius: 4px;"
        "    padding: 6px 12px;"
        "    color: #475569;"
        "    font-weight: 500;"
        "}"
        "QToolButton:hover {"
        "    background: #5B7FFF;"
        "    color: white;"
        "    border-color: #5B7FFF;"
        "}"
    );
    
    QAction *actToggleStatus = toolbar->addAction("⇄ Đổi Trạng Thái");
    actToggleStatus->setToolTip("Khóa/Mở khóa độc giả được chọn");
    connect(actToggleStatus, &QAction::triggered, this, &WidgetQuanLyDocGia::onToggleStatus);
    
    QAction *actHistory = toolbar->addAction("◫ Lịch Sử Mượn");
    actHistory->setToolTip("Xem lịch sử mượn trả của độc giả");
    connect(actHistory, &QAction::triggered, this, &WidgetQuanLyDocGia::onViewHistory);
    
    QAction *actPrintCard = toolbar->addAction("▣ In Thẻ");
    actPrintCard->setToolTip("In thẻ độc giả (Ctrl+P)");
    actPrintCard->setShortcut(QKeySequence::Print);
    connect(actPrintCard, &QAction::triggered, this, &WidgetQuanLyDocGia::onPrintCard);
    
    toolbar->addSeparator();
    
    QAction *actWarnings = toolbar->addAction("⚠ Cảnh Báo");
    actWarnings->setToolTip("Xem các cảnh báo (quá hạn, khóa đang mượn...)");
    connect(actWarnings, &QAction::triggered, this, &WidgetQuanLyDocGia::onShowWarnings);
    
    toolbar->addSeparator();
    
    QAction *actExport = toolbar->addAction("◫ Export CSV");
    actExport->setToolTip("Export danh sách độc giả ra CSV");
    connect(actExport, &QAction::triggered, this, &WidgetQuanLyDocGia::onExportCSV);
    
    QAction *actImport = toolbar->addAction("◫ Import CSV");
    actImport->setToolTip("Import độc giả từ file CSV");
    connect(actImport, &QAction::triggered, this, &WidgetQuanLyDocGia::onImportCSV);
    
    layout->addWidget(toolbar);
    
    // Thanh tim kiem
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->addWidget(new QLabel("Tìm kiếm:", tabQuanLy));
    searchEdit = new QLineEdit(tabQuanLy);
    searchEdit->setPlaceholderText("Nhập mã thẻ, tên, họ...");
    searchLayout->addWidget(searchEdit);
    
    // Advanced search combo
    searchTypeCombo = new QComboBox(tabQuanLy);
    searchTypeCombo->addItem("🔍 Tìm theo mã", 0);
    searchTypeCombo->addItem("👤 Tìm theo tên", 1);
    searchTypeCombo->addItem("👥 Tìm theo họ", 2);
    searchTypeCombo->addItem("👤👥 Tìm theo họ tên", 3);
    searchTypeCombo->addItem("📊 Lọc theo trạng thái", 4);
    searchTypeCombo->setToolTip(
        "Loại tìm kiếm:\n"
        "• Theo mã: Nhập mã thẻ độc giả (số nguyên)\n"
        "• Theo tên: Tìm theo tên (VD: 'Lan')\n"
        "• Theo họ: Tìm theo họ đệm (VD: 'Nguyễn')\n"
        "• Theo họ tên: Tìm theo họ và tên\n"
        "• Lọc trạng thái: 0=Khóa, 1=Hoạt động"
    );
    searchLayout->addWidget(searchTypeCombo);
    
    btnSearch = new QPushButton("🔍 Tìm", tabQuanLy);
    btnSearch->setToolTip("Tìm kiếm độc giả (Enter hoặc Ctrl+F)");
    btnSearch->setShortcut(QKeySequence(Qt::Key_Return));
    btnClearSearch = new QPushButton("❌ Xóa", tabQuanLy);
    searchLayout->addWidget(btnSearch);
    searchLayout->addWidget(btnClearSearch);
    
    layout->addLayout(searchLayout);
    
    // Filter controls
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(12);
    
    QLabel *filterLabel = new QLabel("🔎 Lọc nhanh:", tabQuanLy);
    filterLabel->setStyleSheet("font-weight: bold; color: #64B5F6;");
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
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setAlternatingRowColors(true);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setSortingEnabled(true); // Enable column sorting
    
    layout->addWidget(tableView);
    
    // Thanh trang thai
    statusLabel = new QLabel("", tabQuanLy);
    statusLabel->setStyleSheet("color: #27ae60; font-weight: bold;");
    layout->addWidget(statusLabel);
    
    // Cac nut chuc nang
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    btnAdd = new QPushButton("➕ Thêm Độc Giả", tabQuanLy);
    btnEdit = new QPushButton("✏️ Sửa", tabQuanLy);
    btnDelete = new QPushButton("🗑️ Xóa", tabQuanLy);
    btnViewLoans = new QPushButton("📖 Xem Mượn Trả", tabQuanLy);
    btnBack = new QPushButton("⬅️ Quay Lại", tabQuanLy);
    
    // Add keyboard shortcut tooltips
    btnAdd->setToolTip("Thêm độc giả mới (Ctrl+N)");
    btnEdit->setToolTip("Sửa thông tin độc giả (Ctrl+E)");
    btnDelete->setToolTip("Xóa độc giả đã chọn (Delete hoặc Ctrl+D)");
    btnViewLoans->setToolTip("Xem lịch sử mượn trả");
    btnBack->setToolTip("Quay lại màn hình chính (ESC)");
    
    btnAdd->setStyleSheet("background-color: #27ae60; color: white; padding: 8px; font-weight: bold;");
    btnEdit->setStyleSheet("background-color: #f39c12; color: white; padding: 8px; font-weight: bold;");
    btnDelete->setStyleSheet("background-color: #e74c3c; color: white; padding: 8px; font-weight: bold;");
    btnViewLoans->setStyleSheet("background-color: #3498db; color: white; padding: 8px; font-weight: bold;");
    btnBack->setStyleSheet("background-color: #7f8c8d; color: white; padding: 8px; font-weight: bold;");
    
    buttonLayout->addWidget(btnAdd);
    buttonLayout->addWidget(btnEdit);
    buttonLayout->addWidget(btnDelete);
    buttonLayout->addWidget(btnViewLoans);
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnBack);
    
    layout->addLayout(buttonLayout);
    
    tabWidget->addTab(tabQuanLy, "◉ Danh Sách Độc Giả");
}

void WidgetQuanLyDocGia::setupTabQuaHan() {
    tabQuaHan = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tabQuaHan);
    layout->setSpacing(16);
    layout->setContentsMargins(16, 16, 16, 16);
    
    // Hướng dẫn
    QLabel *lblHuongDan = new QLabel("⚠ DANH SÁCH ĐỘC GIẢ QUÁ HẠN", tabQuaHan);
    QFont font = lblHuongDan->font();
    font.setPointSize(16);
    font.setBold(true);
    lblHuongDan->setFont(font);
    lblHuongDan->setStyleSheet("color: #DC2626; padding: 12px; background: transparent;");
    layout->addWidget(lblHuongDan);
    
    // Thống kê
    lblThongKeQuaHan = new QLabel("Đang tải dữ liệu...", tabQuaHan);
    lblThongKeQuaHan->setStyleSheet(
        "padding: 12px;"
        "background: #FEE2E2;"
        "border: 1px solid #FCA5A5;"
        "border-radius: 8px;"
        "color: #991B1B;"
    );
    layout->addWidget(lblThongKeQuaHan);
    
    // Placeholder message
    QLabel *placeholderMsg = new QLabel("📊 Chức năng đang được phát triển...\n\n"
        "Sẽ hiển thị danh sách độc giả có sách mượn quá hạn.", tabQuaHan);
    placeholderMsg->setAlignment(Qt::AlignCenter);
    placeholderMsg->setStyleSheet(
        "padding: 40px;"
        "color: #64748B;"
        "font-size: 14px;"
    );
    layout->addWidget(placeholderMsg, 1);
    
    // Refresh button
    QPushButton *btnRefresh = new QPushButton("🔄 Làm Mới Dữ Liệu", tabQuaHan);
    btnRefresh->setStyleSheet(
        "QPushButton {"
        "    background: #EF4444;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 10px 20px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: #DC2626;"
        "}"
    );
    connect(btnRefresh, &QPushButton::clicked, this, &WidgetQuanLyDocGia::onRefreshQuaHan);
    layout->addWidget(btnRefresh, 0, Qt::AlignRight);
    
    tabWidget->addTab(tabQuaHan, "⚠ Độc Giả Quá Hạn");
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
    showMessage(QString("Hiển thị %1 độc giả%2").arg(totalCount).arg(filterMsg));
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
    showMessage(QString("Đã tải %1 độc giả").arg(readerModel->rowCount()));
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
        showMessage("❌ Danh sách độc giả trống!", true);
        return;
    }
    
    if (searchType == 0) {
        // Search by card number
        bool ok;
        int maThe = keyword.toInt(&ok);
        
        if (!ok || maThe <= 0) {
            showMessage("⚠️ Vui lòng nhập mã thẻ hợp lệ!", true);
            return;
        }
        
        PTRDG reader = nullptr;
        try {
            reader = timDocGia(rootDocGia, maThe);
        } catch (...) {
            showMessage("❌ Lỗi khi tìm kiếm độc giả!", true);
            return;
        }

        if (reader) {
            std::vector<PTRDG> v{reader};
            readerModel->setSearchResults(v);
            showMessage(QString("🎫 Tìm thấy độc giả: %1 %2")
                .arg(QString::fromStdString(reader->data.Ho),
                     QString::fromStdString(reader->data.Ten)));
        } else {
            showMessage("❌ Không tìm thấy độc giả!", true);
            // clear model
            readerModel->setSearchResults(std::vector<PTRDG>());
        }
        
    } else if (searchType == 1) {
        // Search by first name
        if (keyword.isEmpty()) {
            showMessage("⚠️ Vui lòng nhập tên!", true);
            return;
        }
        
            // Run search in background
            try {
                showMessage("⏳ Đang tìm kiếm...");
                btnSearch->setEnabled(false);
                searchProgress->setVisible(true);
                QFuture<std::vector<PTRDG>> future = QtConcurrent::run(&DBBatcher::timDocGiaTheoTen, keyword.toStdString(), 1000);
                searchWatcher->setFuture(future);
            } catch (...) {
                showMessage("❌ Lỗi khi tìm kiếm!", true);
                btnSearch->setEnabled(true);
                return;
            }
        
    } else if (searchType == 2) {
        // Search by last name
        if (keyword.isEmpty()) {
            showMessage("⚠️ Vui lòng nhập họ!", true);
            return;
        }
        
        try {
            showMessage("⏳ Đang tìm kiếm...");
            btnSearch->setEnabled(false);
            searchProgress->setVisible(true);
            QFuture<std::vector<PTRDG>> future = QtConcurrent::run(&DBBatcher::timDocGiaTheoHo, keyword.toStdString(), 1000);
            searchWatcher->setFuture(future);
        } catch (...) {
            showMessage("❌ Lỗi khi tìm kiếm!", true);
            btnSearch->setEnabled(true);
            return;
        }
        
    } else if (searchType == 3) {
        // Search by full name
        if (keyword.isEmpty()) {
            showMessage("⚠️ Vui lòng nhập họ tên!", true);
            return;
        }
        
        try {
            showMessage("⏳ Đang tìm kiếm...");
            btnSearch->setEnabled(false);
            searchProgress->setVisible(true);
            QFuture<std::vector<PTRDG>> future = QtConcurrent::run(&DBBatcher::timDocGiaTheoHoTen, keyword.toStdString(), 1000);
            searchWatcher->setFuture(future);
        } catch (...) {
            showMessage("❌ Lỗi khi tìm kiếm!", true);
            btnSearch->setEnabled(true);
            return;
        }
        
    } else if (searchType == 4) {
        // Filter by status
        QStringList options;
        options << "1 - Hoạt động" << "0 - Bị khóa";
        
        bool ok;
        QString item = QInputDialog::getItem(this, "Lọc theo trạng thái",
                                            "Chọn trạng thái:", options, 0, false, &ok);
        if (!ok) return;
        
        int status = item.left(1).toInt();
        
        try {
            showMessage("⏳ Đang lọc...");
            btnSearch->setEnabled(false);
            QFuture<std::vector<PTRDG>> future = QtConcurrent::run(&DBBatcher::timDocGiaTheoTrangThai, status, 1000);
            searchWatcher->setFuture(future);
        } catch (...) {
            showMessage("❌ Lỗi khi tìm kiếm!", true);
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
    detail += QString("👤 <b>Họ tên:</b> %1 %2<br>")
              .arg(QString::fromStdString(selectedReader->data.Ho))
              .arg(QString::fromStdString(selectedReader->data.Ten));
    detail += QString("👤 <b>Giới tính:</b> %1<br>")
              .arg(selectedReader->data.Phai ? "Nam" : "Nữ");
    detail += QString("📊 <b>Trạng thái:</b> %1<br>")
              .arg(selectedReader->data.TrangThai ? "Hoạt động" : "Khóa");
    
    // Borrowing status with warnings
    if (borrowedCount > 0) {
        detail += QString("<br><span style='color: #F59E0B; font-weight: bold;'>⚠ Đang mượn: %1 cuốn</span><br>")
                  .arg(borrowedCount);
        if (isOverdue) {
            detail += QString("<span style='color: #EF4444; font-weight: bold;'>⚠ CÓ SÁCH QUÁ HẠN!</span><br>");
        }
    } else {
        detail += QString("<br><span style='color: #10B981;'>✓ Không có sách đang mượn</span><br>");
    }
    
    // Warning messages
    QString warningMsg;
    if (borrowedCount > 0) {
        warningMsg = "\n\n⚠ LƯU Ý:\n";
        warningMsg += "• Độc giả đang mượn sách, nên xử lý trả sách trước\n";
        warningMsg += "• Xóa sẽ mất toàn bộ lịch sử mượn trả\n";
        warningMsg += "• Hành động này không thể hoàn tác";
    } else {
        warningMsg = "\n\nLịch sử mượn trả của độc giả này sẽ bị xóa vĩnh viễn.";
    }
    
    // Use ConfirmationDialog with warning type if has borrowed books
    ConfirmationDialog::Type dialogType = (borrowedCount > 0) 
        ? ConfirmationDialog::Type::Warning 
        : ConfirmationDialog::Type::Delete;
    
    bool confirmed = ConfirmationDialog::confirm(
        this,
        "Xác nhận xóa độc giả",
        QString("Bạn có chắc chắn muốn xóa độc giả này?%1").arg(warningMsg),
        detail,
        dialogType
    );
    
    if (confirmed) {
        // TODO: Call backend delete function
        // For now, just show placeholder
        AnimationHelper::shake(this);
        QMessageBox::information(this, "Xóa Độc Giả", 
            "⚠ Chức năng xóa độc giả đang được phát triển.\n"
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
            showMessage("❌ Lỗi khi thực hiện tìm kiếm!", true);
        }
    }
}

void WidgetQuanLyDocGia::onToggleStatus() {
    if (!selectedReader) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng chọn độc giả để đổi trạng thái!");
        return;
    }
    
    QString currentStatus = selectedReader->data.TrangThai ? "Hoạt động" : "Khóa";
    QString newStatus = selectedReader->data.TrangThai ? "Khóa" : "Hoạt động";
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Xác nhận đổi trạng thái",
        QString("Độc giả: %1 %2\nTrạng thái hiện tại: %3\n\n"
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
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng chọn độc giả để xem lịch sử!");
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
    html += QString("<p><b>Độc giả:</b> %1 %2 (Mã thẻ: %3)</p>")
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
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng chọn độc giả để in thẻ!");
        return;
    }
    
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize::A7);
    
    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() != QDialog::Accepted) {
        return;
    }
    
    // Card HTML (A7 size - like ID card)
    QString html = "<html><head><style>"
                   "body { font-family: Arial, sans-serif; margin: 20px; }"
                   ".card { border: 3px solid #3498db; padding: 20px; border-radius: 10px; }"
                   ".header { text-align: center; color: #2c3e50; margin-bottom: 20px; }"
                   ".info { margin: 10px 0; font-size: 14pt; }"
                   ".label { font-weight: bold; color: #7f8c8d; }"
                   ".value { color: #2c3e50; }"
                   ".barcode { text-align: center; font-size: 24pt; font-family: monospace; "
                   "           border: 2px dashed #95a5a6; padding: 10px; margin-top: 20px; }"
                   "</style></head><body>";
    
    html += "<div class='card'>";
    html += "<div class='header'>";
    html += "<h1>📚 THƯ VIỆN</h1>";
    html += "<h3>THẺ ĐỘC GIẢ</h3>";
    html += "</div>";
    
    html += QString("<div class='info'><span class='label'>Mã thẻ:</span> "
                   "<span class='value'>%1</span></div>")
            .arg(selectedReader->data.MaThe);
    
    html += QString("<div class='info'><span class='label'>Họ tên:</span> "
                   "<span class='value'>%1 %2</span></div>")
            .arg(QString::fromStdString(selectedReader->data.Ho))
            .arg(QString::fromStdString(selectedReader->data.Ten));
    
    html += QString("<div class='info'><span class='label'>Giới tính:</span> "
                   "<span class='value'>%1</span></div>")
            .arg(selectedReader->data.Phai ? "Nữ" : "Nam");
    
    html += QString("<div class='info'><span class='label'>Trạng thái:</span> "
                   "<span class='value' style='color: %2;'>%1</span></div>")
            .arg(selectedReader->data.TrangThai ? "Hoạt động ✓" : "Khóa ✗")
            .arg(selectedReader->data.TrangThai ? "#27ae60" : "#e74c3c");
    
    html += QString("<div class='barcode'>*%1*</div>")
            .arg(QString::number(selectedReader->data.MaThe).rightJustified(8, '0'));
    
    html += "</div></body></html>";
    
    QTextDocument document;
    document.setHtml(html);
    document.print(&printer);
    
    showMessage("✅ Đã in thẻ độc giả!");
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
                warnings += QString("⚠️ Độc giả KHÓA nhưng còn %1 sách chưa trả: %2 %3 (Mã: %4)\n")
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
            "✓ Tất cả độc giả đều ổn định.");
    } else {
        QMessageBox::warning(this, QString("Cảnh Báo ⚠️ (%1)").arg(warnCount),
            QString("Phát hiện %1 vấn đề:\n\n%2").arg(warnCount).arg(warnings));
    }
}

void WidgetQuanLyDocGia::onExportCSV() {
    QString fileName = QFileDialog::getSaveFileName(this,
        "Export danh sách độc giả", 
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
        QString("Đã export %1 độc giả ra file:\n%2")
        .arg(totalCount).arg(fileName));
}

void WidgetQuanLyDocGia::onImportCSV() {
    QMessageBox::information(this, "Import CSV", 
        "Tính năng import độc giả từ CSV sẽ được triển khai trong phiên bản sau.\n\n"
        "Hiện tại vui lòng sử dụng chức năng 'Thêm Độc Giả' để thêm từng người.");
}

void WidgetQuanLyDocGia::onRefreshQuaHan() {
    // TODO: Refresh overdue readers list
    showMessage("Chức năng Quá hạn đang được phát triển");
}

void WidgetQuanLyDocGia::onTableQuaHanDoubleClicked(const QModelIndex &index) {
    Q_UNUSED(index);
    // TODO: Handle double click on overdue readers table
}
