#include "views/WidgetQuanLySach.h"
#include "views/dialogs/ManHinhThemSach.h"
#include "views/dialogs/ManHinhChiTietSach.h"
#include "views/dialogs/ManHinhThemBanSao.h"
#include "views/dialogs/ManHinhSuaSach.h"
#include "views/BadgeStyledItemDelegate.h"
#include "models/QuanLySach.h"
#include "utils/ConfirmationDialog.h"
#include "utils/AnimationHelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QFormLayout>
#include <QDialog>
#include <QSpinBox>
#include <QFileDialog>
#include <QTextStream>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QPageLayout>
#include <QDate>
#include <QTabBar>
#include <vector>

// Set modern stylesheet for book widget - Deep Dark Theme
static const QString BOOK_WIDGET_STYLE = R"(
    WidgetQuanLySach {
        background-color: #151521;
    }
)";

// ==================== BookTableModel Implementation ====================

BookTableModel::BookTableModel(QObject *parent)
    : QAbstractTableModel(parent)
    , books(nullptr)
    , bookCount(0)
    , isSearchMode(false)
    , sortColumn(-1)
    , sortOrder(Qt::AscendingOrder)
{
}

int BookTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return bookCount;
}

int BookTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 8; // ISBN, Ten Sach, Tac Gia, The Loai, Nam XB, So Trang, So Luong, Trang Thai
}

QVariant BookTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= bookCount || !books)
        return QVariant();
    
    PTRDS book = books[index.row()];
    if (!book)
        return QVariant();
    
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return QString::fromStdString(book->ISBN);
            case 1: return QString::fromStdString(book->tenSach);
            case 2: return QString::fromStdString(book->tacGia);
            case 3: return QString::fromStdString(book->theLoai);
            case 4: return book->namXuatBan;
            case 5: return book->soTrang;
            case 6: {
                // Đếm số bản sao có sẵn (CHO_MUON_DUOC)
                int coSan = 0;
                PTRDMS dms = book->dms;
                while (dms) {
                    if (dms->trangThai == CHO_MUON_DUOC) coSan++;
                    dms = dms->next;
                }
                return QString("%1/%2").arg(coSan).arg(book->tongBanSao);
            }
            case 7: {
                // Trạng thái tổng hợp của cuốn sách
                int dangMuon = 0, coSan = 0, thanhLy = 0;
                PTRDMS dms = book->dms;
                while (dms) {
                    if (dms->trangThai == DANG_MUON) dangMuon++;
                    else if (dms->trangThai == CHO_MUON_DUOC) coSan++;
                    else if (dms->trangThai == THANH_LY) thanhLy++;
                    dms = dms->next;
                }
                // Hiển thị trạng thái chính
                if (coSan > 0) return "Có sẵn";
                else if (dangMuon > 0) return "Đang mượn";
                else return "Đã thanh lý";
            }
        }
    } else if (role == Qt::UserRole && index.column() == 7) {
        // Trả về enum để delegate vẽ badge
        int dangMuon = 0, coSan = 0;
        PTRDMS dms = book->dms;
        while (dms) {
            if (dms->trangThai == DANG_MUON) dangMuon++;
            else if (dms->trangThai == CHO_MUON_DUOC) coSan++;
            dms = dms->next;
        }
        if (coSan > 0) return CHO_MUON_DUOC;
        else if (dangMuon > 0) return DANG_MUON;
        else return THANH_LY;
    } else if (role == Qt::TextAlignmentRole) {
        if (index.column() >= 4) {
            return Qt::AlignCenter;
        }
    }
    
    return QVariant();
}

QVariant BookTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();
    
    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0: return "ISBN";
            case 1: return "TÊN SÁCH";
            case 2: return "TÁC GIẢ";
            case 3: return "THỂ LOẠI";
            case 4: return "NĂM XB";
            case 5: return "SỐ TRANG";
            case 6: return "SỐ LƯỢNG";
            case 7: return "TRẠNG THÁI";
        }
    }
    return QVariant();
}

void BookTableModel::refreshData() {
    beginResetModel();
    books = LayDanhSachDauSach();
    bookCount = LaySoLuongDauSach();
    isSearchMode = false;
    endResetModel();
}

void BookTableModel::setSearchResults(PTRDS* results, int count) {
    beginResetModel();
    books = results;
    bookCount = count;
    isSearchMode = true;
    cachedBooks.clear();  // Clear cache khi dùng array mode
    endResetModel();
}

void BookTableModel::setBooks(const std::vector<PTRDS>& bookList) {
    beginResetModel();
    cachedBooks = bookList;
    bookCount = cachedBooks.size();
    books = cachedBooks.empty() ? nullptr : cachedBooks.data();
    isSearchMode = true;
    endResetModel();
}

PTRDS BookTableModel::getBookAt(int row) const {
    if (row >= 0 && row < bookCount && books) {
        return books[row];
    }
    return nullptr;
}

void BookTableModel::clearData() {
    beginResetModel();
    books = nullptr;
    bookCount = 0;
    endResetModel();
}

void BookTableModel::sort(int column, Qt::SortOrder order) {
    if (!books || bookCount == 0) return;
    
    sortColumn = column;
    sortOrder = order;
    
    layoutAboutToBeChanged();
    
    // Simple bubble sort for demonstration
    for (int i = 0; i < bookCount - 1; i++) {
        for (int j = 0; j < bookCount - i - 1; j++) {
            bool swap = false;
            
            switch (column) {
                case 0: // ISBN
                    swap = (sortOrder == Qt::AscendingOrder) ? 
                           (books[j]->ISBN > books[j+1]->ISBN) :
                           (books[j]->ISBN < books[j+1]->ISBN);
                    break;
                case 1: // Ten Sach
                    swap = (sortOrder == Qt::AscendingOrder) ? 
                           (books[j]->tenSach > books[j+1]->tenSach) :
                           (books[j]->tenSach < books[j+1]->tenSach);
                    break;
                case 2: // Tac Gia
                    swap = (sortOrder == Qt::AscendingOrder) ? 
                           (books[j]->tacGia > books[j+1]->tacGia) :
                           (books[j]->tacGia < books[j+1]->tacGia);
                    break;
                case 3: // The Loai
                    swap = (sortOrder == Qt::AscendingOrder) ? 
                           (books[j]->theLoai > books[j+1]->theLoai) :
                           (books[j]->theLoai < books[j+1]->theLoai);
                    break;
                case 4: // Nam XB
                    swap = (sortOrder == Qt::AscendingOrder) ? 
                           (books[j]->namXuatBan > books[j+1]->namXuatBan) :
                           (books[j]->namXuatBan < books[j+1]->namXuatBan);
                    break;
                case 5: // So Trang
                    swap = (sortOrder == Qt::AscendingOrder) ? 
                           (books[j]->soTrang > books[j+1]->soTrang) :
                           (books[j]->soTrang < books[j+1]->soTrang);
                    break;
                case 6: // Tong Ban Sao
                    swap = (sortOrder == Qt::AscendingOrder) ? 
                           (books[j]->tongBanSao > books[j+1]->tongBanSao) :
                           (books[j]->tongBanSao < books[j+1]->tongBanSao);
                    break;
            }
            
            if (swap) {
                PTRDS temp = books[j];
                books[j] = books[j+1];
                books[j+1] = temp;
            }
        }
    }
    
    layoutChanged();
}

// ==================== WidgetQuanLySach Implementation ====================

WidgetQuanLySach::WidgetQuanLySach(QWidget *parent)
    : QWidget(parent)
    , tabWidget(nullptr)
    , badgeDelegate(nullptr)
    , filterTheLoaiCombo(nullptr)
    , filterTrangThaiCombo(nullptr)
    , selectedBook(nullptr)
    , tableTop10(nullptr)
    , modelTop10(nullptr)
    , txtThongKe(nullptr)
{
    searchTimer = new QTimer(this);
    searchTimer->setSingleShot(true);
    searchTimer->setInterval(300); // 300ms debounce
    
    badgeDelegate = new BadgeStyledItemDelegate(this);
    
    setupUI();
    setupConnections();
    setupKeyboardShortcuts();
    updateButtonStates();
}

WidgetQuanLySach::~WidgetQuanLySach() {
}

void WidgetQuanLySach::setupUI() {
    // Apply modern gradient stylesheet
    setStyleSheet(BOOK_WIDGET_STYLE);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(32, 32, 32, 32);
    
    // Modern title with gradient effect
    QLabel *titleLabel = new QLabel("📚 QUẢN LÝ SÁCH", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    titleFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.5);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #60A5FA, stop:1 #34D399); padding: 8px 0; background: transparent;");
    mainLayout->addWidget(titleLabel);
    
    // Modern tab widget with rounded corners and shadows
    tabWidget = new QTabWidget(this);
    tabWidget->setDocumentMode(false);
    tabWidget->setStyleSheet(
        "QTabWidget::pane {"
        "    border: 2px solid #475569;"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1E293B, stop:1 #334155);"
        "    border-radius: 12px;"
        "    top: -1px;"
        "}"
        "QTabBar::tab {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #334155, stop:1 #1E293B);"
        "    color: #CBD5E1;"
        "    padding: 12px 24px;"
        "    margin-right: 6px;"
        "    border-top-left-radius: 10px;"
        "    border-top-right-radius: 10px;"
        "    border: 2px solid #475569;"
        "    border-bottom: none;"
        "    font-weight: 600;"
        "    font-size: 14px;"
        "}"
        "QTabBar::tab:selected {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1E293B, stop:1 #334155);"
        "    color: #60A5FA;"
        "    font-weight: 700;"
        "    border-color: #60A5FA;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #475569, stop:1 #334155);"
        "    color: #F1F5F9;"
        "}"
    );
    
    // Setup 3 tabs
    setupTabQuanLy();
    setupTabTop10();
    setupTabThongKe();
    
    mainLayout->addWidget(tabWidget);
}

void WidgetQuanLySach::setupTabQuanLy() {
    tabQuanLy = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tabQuanLy);
    layout->setSpacing(12);
    layout->setContentsMargins(12, 12, 12, 12);
    
    // Toolbar - Deep Dark Theme với Flat Design
    toolbar = new QToolBar(tabQuanLy);
    toolbar->setIconSize(QSize(20, 20));
    toolbar->setStyleSheet(
        "QToolBar {"
        "    background: #1E1E2D;"
        "    border: 1px solid #2B2B40;"
        "    border-radius: 6px;"
        "    padding: 8px;"
        "    spacing: 6px;"
        "}"
        "QToolButton {"
        "    background: transparent;"
        "    border: 1px solid #2B2B40;"
        "    border-radius: 6px;"
        "    padding: 8px 16px;"
        "    color: #B5B5C3;"
        "    font-weight: 500;"
        "}"
        "QToolButton:hover {"
        "    background: #232334;"
        "    color: #3699FF;"
        "    border-color: #3699FF;"
        "}"
        "QToolButton:pressed {"
        "    background: #1A1A26;"
        "}"
    );
    
    QAction *actImport = toolbar->addAction("◫ Import CSV");
    connect(actImport, &QAction::triggered, this, &WidgetQuanLySach::onImportCSV);
    
    QAction *actExport = toolbar->addAction("◫ Export CSV");
    connect(actExport, &QAction::triggered, this, &WidgetQuanLySach::onExportCSV);
    
    toolbar->addSeparator();
    
    QAction *actPrint = toolbar->addAction("▣ In Danh Sách");
    actPrint->setShortcut(QKeySequence::Print);
    connect(actPrint, &QAction::triggered, this, &WidgetQuanLySach::onPrintList);
    
    layout->addWidget(toolbar);
    
    // Thanh tìm kiếm
    QHBoxLayout *searchLayout = new QHBoxLayout();
    
    searchTypeCombo = new QComboBox(tabQuanLy);
    searchTypeCombo->addItem("◈ Tìm kiếm", 0);
    searchLayout->addWidget(searchTypeCombo);
    
    searchEdit = new QLineEdit(tabQuanLy);
    searchEdit->setPlaceholderText("Tìm theo ISBN, Tên Sách, Tác Giả...");
    searchEdit->setMinimumWidth(300);
    searchLayout->addWidget(searchEdit, 1);
    
    btnSearch = new QPushButton("◈ Tìm", tabQuanLy);
    btnSearch->setProperty("class", "btn-primary");
    
    btnClearSearch = new QPushButton("× Xóa", tabQuanLy);
    btnClearSearch->setProperty("class", "btn-secondary");
    
    searchLayout->addWidget(btnSearch);
    searchLayout->addWidget(btnClearSearch);
    
    layout->addLayout(searchLayout);
    
    // Filter
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("◧ Lọc theo:", tabQuanLy));
    
    filterTheLoaiCombo = new QComboBox(tabQuanLy);
    filterTheLoaiCombo->setMinimumWidth(150);
    filterLayout->addWidget(filterTheLoaiCombo);
    
    filterTrangThaiCombo = new QComboBox(tabQuanLy);
    filterTrangThaiCombo->addItem("Tất cả trạng thái", -1);
    filterTrangThaiCombo->addItem("● Có sẵn", CHO_MUON_DUOC);
    filterTrangThaiCombo->addItem("● Đang mượn", DANG_MUON);
    filterTrangThaiCombo->addItem("● Đã thanh lý", THANH_LY);
    filterTrangThaiCombo->setMinimumWidth(150);
    filterLayout->addWidget(filterTrangThaiCombo);
    
    filterLayout->addStretch();
    layout->addLayout(filterLayout);
    filterTrangThaiCombo->setMinimumWidth(150);
    filterTrangThaiCombo->setToolTip("Lọc sách theo trạng thái");
    filterLayout->addWidget(filterTrangThaiCombo);
    
    filterLayout->addStretch();
    layout->addLayout(filterLayout);
    
    // Bảng hiển thị
    tableView = new QTableView(tabQuanLy);
    bookModel = new BookTableModel(this);
    tableView->setModel(bookModel);
    tableView->setItemDelegateForColumn(7, badgeDelegate);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setAlternatingRowColors(true);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setSortingEnabled(true);
    tableView->setMinimumHeight(350);
    layout->addWidget(tableView);
    
    // Status label
    statusLabel = new QLabel("", tabQuanLy);
    statusLabel->setStyleSheet("color: #6B7280; padding: 4px; background-color: #F9FAFB; border-radius: 4px;");
    layout->addWidget(statusLabel);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    btnAdd = new QPushButton("+ Thêm Sách", tabQuanLy);
    btnAdd->setProperty("class", "btn-primary");
    btnEdit = new QPushButton("✎ Sửa", tabQuanLy);
    btnEdit->setProperty("class", "btn-secondary");
    btnDelete = new QPushButton("× Xóa", tabQuanLy);
    btnDelete->setProperty("class", "btn-danger");
    btnDetails = new QPushButton("◈ Chi Tiết", tabQuanLy);
    btnDetails->setProperty("class", "btn-secondary");
    btnAddCopy = new QPushButton("▣ Thêm Bản Sao", tabQuanLy);
    btnAddCopy->setProperty("class", "btn-success");
    btnBack = new QPushButton("← Quay Lại", tabQuanLy);
    btnBack->setProperty("class", "btn-secondary");
    
    buttonLayout->addWidget(btnAdd);
    buttonLayout->addWidget(btnEdit);
    buttonLayout->addWidget(btnDelete);
    buttonLayout->addWidget(btnDetails);
    buttonLayout->addWidget(btnAddCopy);
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnBack);
    
    layout->addLayout(buttonLayout);
    
    tabWidget->addTab(tabQuanLy, "◫ Danh Sách Sách");
}

void WidgetQuanLySach::setupTabTop10() {
    tabTop10 = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tabTop10);
    layout->setSpacing(12);
    layout->setContentsMargins(12, 12, 12, 12);
    
    QLabel *title = new QLabel("★ TOP 10 SÁCH ĐƯỢC MƯỢN NHIỀU NHẤT", tabTop10);
    QFont font = title->font();
    font.setPointSize(14);
    font.setBold(true);
    title->setFont(font);
    title->setStyleSheet("color: #2D3748; padding: 8px;");
    layout->addWidget(title);
    
    btnRefreshTop10 = new QPushButton("⟳ Làm mới", tabTop10);
    btnRefreshTop10->setProperty("class", "btn-primary");
    btnRefreshTop10->setMaximumWidth(150);
    layout->addWidget(btnRefreshTop10);
    
    tableTop10 = new QTableView(tabTop10);
    tableTop10->setAlternatingRowColors(true);
    tableTop10->horizontalHeader()->setStretchLastSection(true);
    tableTop10->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableTop10->setMinimumHeight(400);
    layout->addWidget(tableTop10);
    
    layout->addStretch();
    
    tabWidget->addTab(tabTop10, "★ Top 10");
}

void WidgetQuanLySach::setupTabThongKe() {
    tabThongKe = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tabThongKe);
    layout->setSpacing(12);
    layout->setContentsMargins(12, 12, 12, 12);
    
    QLabel *title = new QLabel("◫ THỐNG KÊ SÁCH THEO THỂ LOẠI", tabThongKe);
    QFont font = title->font();
    font.setPointSize(14);
    font.setBold(true);
    title->setFont(font);
    title->setStyleSheet("color: #2D3748; padding: 8px;");
    layout->addWidget(title);
    
    btnRefreshThongKe = new QPushButton("⟳ Làm mới", tabThongKe);
    btnRefreshThongKe->setProperty("class", "btn-primary");
    btnRefreshThongKe->setMaximumWidth(150);
    layout->addWidget(btnRefreshThongKe);
    
    txtThongKe = new QTextEdit(tabThongKe);
    txtThongKe->setReadOnly(true);
    txtThongKe->setMinimumHeight(400);
    layout->addWidget(txtThongKe);
    
    tabWidget->addTab(tabThongKe, "◫ Thống Kê");
}

void WidgetQuanLySach::setupConnections() {
    connect(btnSearch, &QPushButton::clicked, this, &WidgetQuanLySach::onSearchClicked);
    connect(btnClearSearch, &QPushButton::clicked, this, &WidgetQuanLySach::onClearSearchClicked);
    connect(btnAdd, &QPushButton::clicked, this, &WidgetQuanLySach::onAddBookClicked);
    connect(btnEdit, &QPushButton::clicked, this, &WidgetQuanLySach::onEditBookClicked);
    connect(btnDelete, &QPushButton::clicked, this, &WidgetQuanLySach::onDeleteBookClicked);
    connect(btnDetails, &QPushButton::clicked, this, &WidgetQuanLySach::onViewDetailsClicked);
    connect(btnAddCopy, &QPushButton::clicked, this, &WidgetQuanLySach::onAddCopyClicked);
    connect(btnBack, &QPushButton::clicked, this, &WidgetQuanLySach::onBackClicked);
    connect(tableView, &QTableView::doubleClicked, this, &WidgetQuanLySach::onTableDoubleClicked);
    connect(searchEdit, &QLineEdit::returnPressed, this, &WidgetQuanLySach::onSearchClicked);
    
    connect(searchEdit, &QLineEdit::textChanged, this, &WidgetQuanLySach::onSearchTextChanged);
    connect(searchTimer, &QTimer::timeout, this, &WidgetQuanLySach::performSearch);
    
    if (filterTheLoaiCombo) {
        connect(filterTheLoaiCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &WidgetQuanLySach::performSearch);
    }
    if (filterTrangThaiCombo) {
        connect(filterTrangThaiCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &WidgetQuanLySach::performSearch);
    }
    
    connect(tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            [this]() {
                QModelIndexList selected = tableView->selectionModel()->selectedRows();
                if (!selected.isEmpty()) {
                    selectedBook = bookModel->getBookAt(selected.first().row());
                } else {
                    selectedBook = nullptr;
                }
                updateButtonStates();
            });
    
    // Tab buttons
    connect(btnRefreshTop10, &QPushButton::clicked, this, &WidgetQuanLySach::onRefreshTop10);
    connect(btnRefreshThongKe, &QPushButton::clicked, this, &WidgetQuanLySach::onRefreshThongKe);
}

void WidgetQuanLySach::onRefreshTop10() {
    // TODO: Implement Top 10 logic using backend
    showMessage("Chức năng Top 10 đang được phát triển");
}

void WidgetQuanLySach::onRefreshThongKe() {
    // TODO: Implement statistics logic using backend
    showMessage("Chức năng Thống kê đang được phát triển");
}

void WidgetQuanLySach::refreshData() {
    bookModel->refreshData();
    selectedBook = nullptr;
    
    // Load thể loại động từ backend
    if (filterTheLoaiCombo) {
        filterTheLoaiCombo->clear();
        filterTheLoaiCombo->addItem("Tất cả thể loại", "");
        
        // Sử dụng backend để lấy danh sách thể loại
        std::string cacTheLoai[MAX_DAUSACH];
        int soTheLoai = 0;
        LayDanhSachTheoTheLoai(cacTheLoai, soTheLoai);
        
        for (int i = 0; i < soTheLoai; i++) {
            filterTheLoaiCombo->addItem(
                QString::fromStdString(cacTheLoai[i]),
                QString::fromStdString(cacTheLoai[i])
            );
        }
    }
    
    updateButtonStates();
    showMessage(QString("Đã tải %1 đầu sách").arg(LaySoLuongDauSach()));
}

void WidgetQuanLySach::updateButtonStates() {
    bool hasSelection = (selectedBook != nullptr);
    btnEdit->setEnabled(hasSelection);
    btnDelete->setEnabled(hasSelection);
    btnDetails->setEnabled(hasSelection);
    btnAddCopy->setEnabled(hasSelection);
}

void WidgetQuanLySach::showMessage(const QString &message, bool isError) {
    statusLabel->setText(message);
    statusLabel->setStyleSheet(isError ? 
        "color: #e74c3c; font-weight: bold;" : 
        "color: #27ae60; font-weight: bold;");
}

void WidgetQuanLySach::onSearchClicked() {
    QString keyword = searchEdit->text().trimmed();
    int searchType = searchTypeCombo->currentData().toInt();
    
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    
    static PTRDS searchBooks[MAX_DAUSACH];
    int count = 0;
    
    if (searchType == 0) {
        // Normal search
        if (keyword.isEmpty()) {
            onClearSearchClicked();
            return;
        }
        
        static KetQuaTimKiem results[MAX_DAUSACH];
        count = timKiemLogic(dsDauSach, soLuongDauSach, keyword.toStdString(), results);
        
        for (int i = 0; i < count; i++) {
            searchBooks[i] = results[i].sach;
        }
        showMessage(QString("🔍 Tìm thấy %1 kết quả").arg(count));
        
    } else if (searchType == 1) {
        // Search by year
        bool ok;
        int year = keyword.toInt(&ok);
        if (!ok || year < 1000 || year > 9999) {
            showMessage("⚠️ Vui lòng nhập năm hợp lệ (VD: 2020)!", true);
            return;
        }
        
        count = TimSachTheoNamXuatBan(dsDauSach, soLuongDauSach, year, searchBooks);
        showMessage(QString("📅 Tìm thấy %1 sách xuất bản năm %2").arg(count).arg(year));
        
    } else if (searchType == 2) {
        // Search by year range
        QStringList years = keyword.split("-");
        if (years.size() != 2) {
            showMessage("⚠️ Vui lòng nhập khoảng năm (VD: 2015-2020)!", true);
            return;
        }
        
        bool ok1, ok2;
        int yearStart = years[0].trimmed().toInt(&ok1);
        int yearEnd = years[1].trimmed().toInt(&ok2);
        
        if (!ok1 || !ok2 || yearStart > yearEnd) {
            showMessage("⚠️ Khoảng năm không hợp lệ!", true);
            return;
        }
        
        count = TimSachTheoKhoangNam(dsDauSach, soLuongDauSach, yearStart, yearEnd, searchBooks);
        showMessage(QString("📆 Tìm thấy %1 sách từ %2 đến %3").arg(count).arg(yearStart).arg(yearEnd));
        
    } else if (searchType == 3) {
        // Search by author
        if (keyword.isEmpty()) {
            showMessage("⚠️ Vui lòng nhập tên tác giả!", true);
            return;
        }
        
        count = TimSachTheoTacGia(dsDauSach, soLuongDauSach, keyword.toStdString(), searchBooks);
        showMessage(QString("✍️ Tìm thấy %1 sách của tác giả '%2'").arg(count).arg(keyword));
        
    } else if (searchType == 4) {
        // Filter by status
        QStringList options;
        options << "0 - Có thể mượn" << "1 - Đang được mượn" << "2 - Thanh lý";
        
        bool ok;
        QString item = QInputDialog::getItem(this, "Lọc theo trạng thái",
                                            "Chọn trạng thái:", options, 0, false, &ok);
        if (!ok) return;
        
        int status = item.left(1).toInt();
        
        count = LocSachTheoTrangThai(dsDauSach, soLuongDauSach, static_cast<TrangThaiSach>(status), searchBooks);
        showMessage(QString("📊 Tìm thấy %1 đầu sách có bản sao '%2'").arg(count).arg(item.mid(4)));
    }
    
    bookModel->setSearchResults(searchBooks, count);
}

void WidgetQuanLySach::onClearSearchClicked() {
    searchEdit->clear();
    searchTypeCombo->setCurrentIndex(0);
    bookModel->refreshData();
    selectedBook = nullptr;
    updateButtonStates();
    showMessage(QString("Hiển thị tất cả %1 đầu sách").arg(LaySoLuongDauSach()));
}

void WidgetQuanLySach::onAddBookClicked() {
    showAddBookDialog();
}

void WidgetQuanLySach::onEditBookClicked() {
    if (!selectedBook) return;
    showEditBookDialog();
}

void WidgetQuanLySach::onDeleteBookClicked() {
    if (!selectedBook) return;
    
    // Check if can delete
    if (!KiemTraCoTheXoaDauSach(selectedBook)) {
        ConfirmationDialog::confirm(this,
            "Không thể xóa",
            "Không thể xóa đầu sách này vì còn bản sao đang được mượn hoặc chờ mượn!",
            QString("ISBN: %1\nTên sách: %2\n\nVui lòng đợi tất cả bản sao được trả lại trước khi xóa.")
                .arg(QString::fromStdString(selectedBook->ISBN))
                .arg(QString::fromStdString(selectedBook->tenSach)),
            ConfirmationDialog::Type::Warning);
        return;
    }
    
    // Build detailed preview
    QString details;
    details += QString("ISBN: %1\n").arg(QString::fromStdString(selectedBook->ISBN));
    details += QString("Tác giả: %1\n").arg(QString::fromStdString(selectedBook->tacGia));
    details += QString("Thể loại: %1\n").arg(QString::fromStdString(selectedBook->theLoai));
    details += QString("Năm XB: %1\n").arg(selectedBook->namXuatBan);
    details += QString("Số trang: %1\n").arg(selectedBook->soTrang);
    details += QString("Tổng bản sao: %1\n\n").arg(selectedBook->tongBanSao);
    
    // Count copies by status
    int dangMuon = 0, coSan = 0, thanhLy = 0;
    PTRDMS dms = selectedBook->dms;
    while (dms) {
        if (dms->trangThai == DANG_MUON) dangMuon++;
        else if (dms->trangThai == CHO_MUON_DUOC) coSan++;
        else if (dms->trangThai == THANH_LY) thanhLy++;
        dms = dms->next;
    }
    
    details += QString("Chi tiết bản sao:\n");
    details += QString("- Có sẵn: %1\n").arg(coSan);
    details += QString("- Đang mượn: %1\n").arg(dangMuon);
    details += QString("- Đã thanh lý: %1\n").arg(thanhLy);
    
    // Show confirmation with preview
    bool confirmed = ConfirmationDialog::confirmDelete(this,
        QString::fromStdString(selectedBook->tenSach),
        details);
    
    if (confirmed) {
        // Show loading
        statusLabel->setText("⏳ Đang xóa...");
        statusLabel->setStyleSheet("color: #F59E0B; font-weight: 600;");
        
        std::string result = XoaDauSachTheoISBN(selectedBook->ISBN);
        if (result.empty()) {
            showMessage("✅ Xóa đầu sách thành công!");
            AnimationHelper::fadeIn(statusLabel, 300);
            refreshData();
        } else {
            showMessage(QString("❌ ") + QString::fromStdString(result), true);
        }
    }
}

void WidgetQuanLySach::onViewDetailsClicked() {
    if (!selectedBook) return;
    showBookDetailsDialog();
}

void WidgetQuanLySach::onAddCopyClicked() {
    if (!selectedBook) return;
    showAddCopyDialog();
}

void WidgetQuanLySach::onBackClicked() {
    emit backToHome();
}

void WidgetQuanLySach::onTableDoubleClicked(const QModelIndex &index) {
    Q_UNUSED(index);
    if (selectedBook) {
        showBookDetailsDialog();
    }
}

void WidgetQuanLySach::showAddBookDialog() {
    // Thử ManHinhThemSach mới, fallback sang AddBookDialog cũ nếu cần
    ManHinhThemSach dialog(this);
    connect(&dialog, &ManHinhThemSach::sachDaThem, this, &WidgetQuanLySach::refreshData);
    dialog.exec();
}

void WidgetQuanLySach::showEditBookDialog() {
    if (!selectedBook) return;
    
    ManHinhSuaSach dialog(QString::fromStdString(selectedBook->ISBN), this);
    connect(&dialog, &ManHinhSuaSach::sachDaCapNhat, this, &WidgetQuanLySach::refreshData);
    dialog.exec();
}

void WidgetQuanLySach::showBookDetailsDialog() {
    if (!selectedBook) return;
    
    ManHinhChiTietSach dialog(selectedBook, this);
    connect(&dialog, &ManHinhChiTietSach::sachDaCapNhat, this, &WidgetQuanLySach::refreshData);
    dialog.exec();
}

void WidgetQuanLySach::showAddCopyDialog() {
    if (!selectedBook) return;
    
    ManHinhThemBanSao dialog(QString::fromStdString(selectedBook->ISBN), this);
    connect(&dialog, &ManHinhThemBanSao::banSaoDaThem, this, &WidgetQuanLySach::refreshData);
    dialog.exec();
}

// ==================== NEW FEATURES ====================

void WidgetQuanLySach::onSearchTextChanged() {
    // Debounce: Reset timer on each keystroke
    searchTimer->stop();
    
    // For normal search type, enable real-time search
    if (searchTypeCombo->currentData().toInt() == 0) {
        searchTimer->start();
    }
}

void WidgetQuanLySach::performSearch() {
    QString keyword = searchEdit->text().trimmed();
    QString theLoaiFilter = filterTheLoaiCombo->currentData().toString();
    int trangThaiFilter = filterTrangThaiCombo->currentData().toInt();
    
    // Nếu không có filter nào, reset về danh sách đầy đủ
    if (keyword.isEmpty() && theLoaiFilter.isEmpty() && trangThaiFilter == -1) {
        onClearSearchClicked();
        return;
    }
    
    // Bước 1: Lấy danh sách ban đầu (từ keyword hoặc toàn bộ)
    std::vector<PTRDS> ketQuaFilter;
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    
    if (!keyword.isEmpty()) {
        // Tìm kiếm theo keyword
        KetQuaTimKiem kqTK[MAX_DAUSACH];
        int soKetQua = 0;
        std::string tk = keyword.toStdString();
        
        LayDanhSachTimKiem(tk, kqTK, soKetQua, MAX_DAUSACH);
        
        for (int i = 0; i < soKetQua; i++) {
            ketQuaFilter.push_back(kqTK[i].sach);
        }
    } else {
        // Lấy toàn bộ danh sách
        for (int i = 0; i < soLuongDauSach; i++) {
            ketQuaFilter.push_back(dsDauSach[i]);
        }
    }
    
    // Bước 2: Lọc theo thể loại
    if (!theLoaiFilter.isEmpty()) {
        std::vector<PTRDS> ketQuaTam;
        std::string theLoai = theLoaiFilter.toStdString();
        
        for (PTRDS sach : ketQuaFilter) {
            if (sach && sach->theLoai == theLoai) {
                ketQuaTam.push_back(sach);
            }
        }
        ketQuaFilter = ketQuaTam;
    }
    
    // Bước 3: Lọc theo trạng thái (kiểm tra danh mục sách)
    if (trangThaiFilter != -1) {
        std::vector<PTRDS> ketQuaTam;
        TrangThaiSach trangThai = static_cast<TrangThaiSach>(trangThaiFilter);
        
        for (PTRDS sach : ketQuaFilter) {
            if (!sach) continue;
            
            // Kiểm tra xem có ít nhất 1 bản sao có trạng thái này không
            bool coTrangThaiNay = false;
            PTRDMS p = sach->dms;
            while (p) {
                if (p->trangThai == trangThai) {
                    coTrangThaiNay = true;
                    break;
                }
                p = p->next;
            }
            
            if (coTrangThaiNay) {
                ketQuaTam.push_back(sach);
            }
        }
        ketQuaFilter = ketQuaTam;
    }
    
    // Cập nhật model với kết quả filter
    bookModel->setBooks(ketQuaFilter);
    
    // Cập nhật label status
    QString statusText = QString("🔍 Tìm thấy %1 kết quả").arg(ketQuaFilter.size());
    
    QStringList filterDetails;
    if (!keyword.isEmpty()) {
        filterDetails << QString("Từ khóa: \"%1\"").arg(keyword);
    }
    if (!theLoaiFilter.isEmpty()) {
        filterDetails << QString("Thể loại: %1").arg(theLoaiFilter);
    }
    if (trangThaiFilter != -1) {
        QString trangThaiText = filterTrangThaiCombo->currentText();
        filterDetails << QString("Trạng thái: %1").arg(trangThaiText);
    }
    
    if (!filterDetails.isEmpty()) {
        statusText += " (" + filterDetails.join(", ") + ")";
    }
    
    statusLabel->setText(statusText);
}

void WidgetQuanLySach::onExportCSV() {
    QString fileName = QFileDialog::getSaveFileName(this,
        "Export danh sách sách", 
        "books_export.csv",
        "CSV Files (*.csv);;All Files (*)");
    
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Lỗi", "Không thể mở file để ghi!");
        return;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    
    // Write header
    out << "ISBN,Tên Sách,Tác Giả,Thể Loại,Năm XB,Số Trang,Tổng Bản Sao,Đang Mượn\n";
    
    // Write data
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    
    for (int i = 0; i < soLuongDauSach; i++) {
        PTRDS book = dsDauSach[i];
        if (!book) continue;
        
        // Count borrowed copies
        int borrowed = 0;
        PTRDMS p = book->dms;
        while (p) {
            if (p->trangThai == 1) borrowed++;
            p = p->next;
        }
        
        out << QString::fromStdString(book->ISBN) << ","
            << "\"" << QString::fromStdString(book->tenSach) << "\","
            << "\"" << QString::fromStdString(book->tacGia) << "\","
            << "\"" << QString::fromStdString(book->theLoai) << "\","
            << book->namXuatBan << ","
            << book->soTrang << ","
            << book->tongBanSao << ","
            << borrowed << "\n";
    }
    
    file.close();
    
    QMessageBox::information(this, "Thành công ✅",
        QString("Đã export %1 đầu sách ra file:\n%2")
        .arg(soLuongDauSach).arg(fileName));
}

void WidgetQuanLySach::onImportCSV() {
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Xác nhận Import",
        "⚠️ Import sẽ THÊM sách mới từ CSV.\n"
        "Sách trùng ISBN sẽ bị bỏ qua.\n\n"
        "Format CSV:\n"
        "ISBN,Tên Sách,Tác Giả,Thể Loại,Năm XB,Số Trang\n\n"
        "Bạn có muốn tiếp tục?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply != QMessageBox::Yes) return;
    
    QString fileName = QFileDialog::getOpenFileName(this,
        "Import danh sách sách", 
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
    
    // Skip header
    if (!in.atEnd()) {
        in.readLine();
    }
    
    int imported = 0;
    int skipped = 0;
    QString errors;
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(",");
        
        if (fields.size() < 6) {
            skipped++;
            continue;
        }
        
        std::string isbn = fields[0].trimmed().toStdString();
        std::string tenSach = fields[1].trimmed().remove("\"").toStdString();
        std::string tacGia = fields[2].trimmed().remove("\"").toStdString();
        std::string theLoai = fields[3].trimmed().remove("\"").toStdString();
        int namXB = fields[4].trimmed().toInt();
        int soTrang = fields[5].trimmed().toInt();
        
        // Check if ISBN already exists
        if (TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn)) {
            skipped++;
            continue;
        }
        
        // Add book using themDauSach: (ds[], n, isbn, ten, trang, theLoai, nam, tacGia)
        bool result = themDauSach(dsDauSach, soLuongDauSach, isbn, tenSach, soTrang, theLoai, namXB, tacGia);
        if (result) {
            imported++;
        } else {
            skipped++;
            errors += QString::fromStdString(isbn) + ": Failed to add\n";
        }
    }
    
    file.close();
    refreshData();
    
    QString message = QString("✅ Đã import %1 đầu sách mới\n").arg(imported);
    if (skipped > 0) {
        message += QString("⚠️ Đã bỏ qua %1 dòng (trùng/lỗi)").arg(skipped);
    }
    if (!errors.isEmpty()) {
        message += "\n\nLỗi:\n" + errors.left(500);
    }
    
    QMessageBox::information(this, "Kết quả Import", message);
}

void WidgetQuanLySach::onPrintList() {
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageOrientation(QPageLayout::Landscape);
    
    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() != QDialog::Accepted) {
        return;
    }
    
    // Build HTML for printing
    QString html = "<html><head><style>"
                   "body { font-family: Arial, sans-serif; }"
                   "h1 { text-align: center; color: #2c3e50; }"
                   "table { width: 100%; border-collapse: collapse; margin-top: 20px; }"
                   "th { background-color: #3498db; color: white; padding: 8px; text-align: left; }"
                   "td { border: 1px solid #ddd; padding: 8px; }"
                   "tr:nth-child(even) { background-color: #f2f2f2; }"
                   "</style></head><body>";
    
    html += "<h1>📚 DANH SÁCH ĐẦU SÁCH</h1>";
    html += QString("<p style='text-align: center; color: #7f8c8d;'>Tổng số: %1 đầu sách</p>")
            .arg(LaySoLuongDauSach());
    
    html += "<table>";
    html += "<tr>"
            "<th>STT</th>"
            "<th>ISBN</th>"
            "<th>Tên Sách</th>"
            "<th>Tác Giả</th>"
            "<th>Thể Loại</th>"
            "<th>Năm XB</th>"
            "<th>Số Trang</th>"
            "<th>Bản Sao</th>"
            "</tr>";
    
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    
    for (int i = 0; i < soLuongDauSach; i++) {
        PTRDS book = dsDauSach[i];
        if (!book) continue;
        
        html += QString("<tr>"
                       "<td>%1</td>"
                       "<td>%2</td>"
                       "<td>%3</td>"
                       "<td>%4</td>"
                       "<td>%5</td>"
                       "<td>%6</td>"
                       "<td>%7</td>"
                       "<td>%8</td>"
                       "</tr>")
                .arg(i + 1)
                .arg(QString::fromStdString(book->ISBN))
                .arg(QString::fromStdString(book->tenSach))
                .arg(QString::fromStdString(book->tacGia))
                .arg(QString::fromStdString(book->theLoai))
                .arg(book->namXuatBan)
                .arg(book->soTrang)
                .arg(book->tongBanSao);
    }
    
    html += "</table>";
    html += QString("<p style='text-align: right; color: #95a5a6; margin-top: 20px;'>"
                   "In ngày: %1</p>")
            .arg(QDate::currentDate().toString("dd/MM/yyyy"));
    html += "</body></html>";
    
    QTextDocument document;
    document.setHtml(html);
    document.print(&printer);
    
    showMessage("✅ Đã in danh sách sách!");
}

// ==================== NEW METHODS ====================

void WidgetQuanLySach::setupKeyboardShortcuts() {
    // Ctrl+F: Focus search
    shortcutSearch = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    connect(shortcutSearch, &QShortcut::activated, [this]() {
        searchEdit->setFocus();
        searchEdit->selectAll();
    });
    
    // Ctrl+N: Add new book
    shortcutAdd = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_N), this);
    connect(shortcutAdd, &QShortcut::activated, this, &WidgetQuanLySach::onAddBookClicked);
    
    // Ctrl+E: Edit selected book
    shortcutEdit = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_E), this);
    connect(shortcutEdit, &QShortcut::activated, [this]() {
        if (selectedBook) onEditBookClicked();
    });
    
    // Delete or Ctrl+D: Delete selected book
    shortcutDelete = new QShortcut(QKeySequence::Delete, this);
    connect(shortcutDelete, &QShortcut::activated, [this]() {
        if (selectedBook) onDeleteBookClicked();
    });
    
    QShortcut *shortcutDelete2 = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_D), this);
    connect(shortcutDelete2, &QShortcut::activated, [this]() {
        if (selectedBook) onDeleteBookClicked();
    });
    
    // F5 or Ctrl+R: Refresh
    shortcutRefresh = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(shortcutRefresh, &QShortcut::activated, this, &WidgetQuanLySach::refreshData);
    
    QShortcut *shortcutRefresh2 = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this);
    connect(shortcutRefresh2, &QShortcut::activated, this, &WidgetQuanLySach::refreshData);
    
    // Update tooltips with shortcuts
    btnAdd->setToolTip("Thêm sách mới (Ctrl+N)");
    btnEdit->setToolTip("Sửa thông tin sách (Ctrl+E)");
    btnDelete->setToolTip("Xóa sách (Delete hoặc Ctrl+D)");
    searchEdit->setToolTip("Tìm kiếm sách (Ctrl+F)");
    btnSearch->setToolTip("Tìm kiếm (Enter)");
}

void WidgetQuanLySach::onBulkDelete() {
    QModelIndexList selected = tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        showMessage("⚠️ Vui lòng chọn ít nhất một sách để xóa", true);
        return;
    }
    
    QString details = QString("Số lượng: %1 đầu sách\n\n").arg(selected.count());
    details += "Danh sách:\n";
    
    for (int i = 0; i < qMin(selected.count(), 10); i++) {
        PTRDS book = bookModel->getBookAt(selected[i].row());
        if (book) {
            details += QString("%1. %2\n")
                .arg(i + 1)
                .arg(QString::fromStdString(book->tenSach));
        }
    }
    
    if (selected.count() > 10) {
        details += QString("... và %1 sách khác\n").arg(selected.count() - 10);
    }
    
    bool confirmed = ConfirmationDialog::confirm(this,
        "Xóa nhiều sách",
        QString("Bạn có chắc muốn xóa %1 đầu sách?").arg(selected.count()),
        details,
        ConfirmationDialog::Type::Delete);
    
    if (confirmed) {
        int successCount = 0;
        int failCount = 0;
        
        for (const QModelIndex &index : selected) {
            PTRDS book = bookModel->getBookAt(index.row());
            if (book && KiemTraCoTheXoaDauSach(book)) {
                std::string result = XoaDauSachTheoISBN(book->ISBN);
                if (result.empty()) {
                    successCount++;
                } else {
                    failCount++;
                }
            } else {
                failCount++;
            }
        }
        
        showMessage(QString("✅ Xóa thành công: %1 | ❌ Thất bại: %2")
                   .arg(successCount).arg(failCount));
        refreshData();
    }
}

void WidgetQuanLySach::onFilterChanged() {
    applyAdvancedFilter();
}

void WidgetQuanLySach::applyAdvancedFilter() {
    // Get filter criteria
    QString theLoai = filterTheLoaiCombo->currentData().toString();
    int trangThai = filterTrangThaiCombo->currentData().toInt();
    
    // Apply filters to model
    // TODO: Implement advanced filtering in BookTableModel
    performSearch();
}

bool WidgetQuanLySach::validateBookData(const QString &isbn, const QString &tenSach,
                                        const QString &tacGia, int namXB, int soTrang) {
    QStringList errors;
    
    // ISBN validation (10 or 13 digits)
    if (isbn.isEmpty()) {
        errors << "ISBN không được để trống";
    } else if (!isbn.contains(QRegularExpression("^[0-9]{10}$|^[0-9]{13}$"))) {
        errors << "ISBN phải có 10 hoặc 13 chữ số";
    }
    
    // Book title validation
    if (tenSach.trimmed().isEmpty()) {
        errors << "Tên sách không được để trống";
    } else if (tenSach.length() > 200) {
        errors << "Tên sách quá dài (tối đa 200 ký tự)";
    }
    
    // Author validation
    if (tacGia.trimmed().isEmpty()) {
        errors << "Tác giả không được để trống";
    } else if (tacGia.length() > 100) {
        errors << "Tên tác giả quá dài (tối đa 100 ký tự)";
    }
    
    // Year validation
    int currentYear = QDate::currentDate().year();
    if (namXB < 1000 || namXB > currentYear + 1) {
        errors << QString("Năm xuất bản không hợp lệ (1000-%1)").arg(currentYear + 1);
    }
    
    // Page count validation
    if (soTrang <= 0 || soTrang > 10000) {
        errors << "Số trang phải từ 1-10000";
    }
    
    if (!errors.isEmpty()) {
        ConfirmationDialog::confirm(this,
            "Dữ liệu không hợp lệ",
            "Vui lòng kiểm tra lại thông tin:",
            errors.join("\n"),
            ConfirmationDialog::Type::Warning);
        return false;
    }
    
    return true;
}

