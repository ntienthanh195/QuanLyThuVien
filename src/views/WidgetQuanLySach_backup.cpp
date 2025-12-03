#include "views/WidgetQuanLySach.h"
#include "views/dialogs/ManHinhThemSach.h"
#include "views/dialogs/ManHinhChiTietSach.h"
#include "views/dialogs/ManHinhThemBanSao.h"
#include "views/dialogs/ManHinhSuaSach.h"
#include "views/BadgeStyledItemDelegate.h"
#include "models/QuanLySach.h"
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
#include <vector>

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
    updateButtonStates();
}

WidgetQuanLySach::~WidgetQuanLySach() {
}

void WidgetQuanLySach::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    
    // Tiêu đề
    QLabel *titleLabel = new QLabel("▤ QUẢN LÝ SÁCH", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #2D3748; padding: 8px 0;");
    mainLayout->addWidget(titleLabel);
    
    // Tab widget
    tabWidget = new QTabWidget(this);
    tabWidget->setDocumentMode(true);
    
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
    QFont titleFont;
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Toolbar
    toolbar = new QToolBar(this);
    toolbar->setIconSize(QSize(24, 24));
    toolbar->setStyleSheet("QToolBar { background: #ecf0f1; border-radius: 5px; padding: 5px; }");
    
    QAction *actImport = toolbar->addAction("📥 Import CSV");
    actImport->setToolTip("Import danh sách sách từ file CSV");
    connect(actImport, &QAction::triggered, this, &WidgetQuanLySach::onImportCSV);
    
    QAction *actExport = toolbar->addAction("📤 Export CSV");
    actExport->setToolTip("Export danh sách sách ra file CSV");
    connect(actExport, &QAction::triggered, this, &WidgetQuanLySach::onExportCSV);
    
    toolbar->addSeparator();
    
    QAction *actPrint = toolbar->addAction("🖨️ In Danh Sách");
    actPrint->setToolTip("In danh sách sách (Ctrl+P)");
    actPrint->setShortcut(QKeySequence::Print);
    connect(actPrint, &QAction::triggered, this, &WidgetQuanLySach::onPrintList);
    
    mainLayout->addWidget(toolbar);
    
    // Thanh tim kiem va filter
    QHBoxLayout *searchLayout = new QHBoxLayout();
    
    // Dropdown loại tìm kiếm
    searchTypeCombo = new QComboBox(this);
    searchTypeCombo->addItem("🔍 Tìm kiếm", 0);
    searchTypeCombo->setToolTip("Tìm theo ISBN, tên sách, tác giả");
    searchLayout->addWidget(searchTypeCombo);
    
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Tìm theo ISBN, Tên Sách, Tác Giả...");
    searchEdit->setMinimumWidth(300);
    searchLayout->addWidget(searchEdit, 1);
    
    btnSearch = new QPushButton("🔍 Tìm", this);
    btnSearch->setToolTip("Tìm kiếm sách (Enter)");
    btnSearch->setProperty("class", "btn-primary");
    
    btnClearSearch = new QPushButton("❌ Xóa", this);
    btnClearSearch->setToolTip("Xóa kết quả tìm kiếm (Esc)");
    btnClearSearch->setProperty("class", "btn-secondary");
    
    searchLayout->addWidget(btnSearch);
    searchLayout->addWidget(btnClearSearch);
    
    mainLayout->addLayout(searchLayout);
    
    // Thanh filter
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("🔧 Lọc theo:", this));
    
    // Filter thể loại (động từ backend)
    filterTheLoaiCombo = new QComboBox(this);
    filterTheLoaiCombo->setMinimumWidth(150);
    filterTheLoaiCombo->setToolTip("Lọc sách theo thể loại");
    filterLayout->addWidget(filterTheLoaiCombo);
    
    // Filter trạng thái
    filterTrangThaiCombo = new QComboBox(this);
    filterTrangThaiCombo->addItem("Tất cả trạng thái", -1);
    filterTrangThaiCombo->addItem("✅ Có sẵn", CHO_MUON_DUOC);
    filterTrangThaiCombo->addItem("📖 Đang mượn", DANG_MUON);
    filterTrangThaiCombo->addItem("🗑️ Đã thanh lý", THANH_LY);
    filterTrangThaiCombo->setMinimumWidth(150);
    filterTrangThaiCombo->setToolTip("Lọc sách theo trạng thái");
    filterLayout->addWidget(filterTrangThaiCombo);
    
    filterLayout->addStretch();
    mainLayout->addLayout(filterLayout);
    
    // Bang hien thi
    tableView = new QTableView(this);
    bookModel = new BookTableModel(this);
    tableView->setModel(bookModel);
    
    // Gán delegate cho cột trạng thái
    tableView->setItemDelegateForColumn(7, badgeDelegate);
    
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setAlternatingRowColors(true);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setSortingEnabled(true); // Enable sorting by clicking column headers
    tableView->setMinimumHeight(400);
    
    mainLayout->addWidget(tableView);
    
    // Thanh trang thai
    statusLabel = new QLabel("", this);
    statusLabel->setStyleSheet("color: #27ae60; font-weight: bold;");
    mainLayout->addWidget(statusLabel);
    
    // Cac nut chuc nang
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    btnAdd = new QPushButton("➕ Thêm Sách", this);
    btnEdit = new QPushButton("✏️ Sửa", this);
    btnDelete = new QPushButton("🗑️ Xóa", this);
    btnDetails = new QPushButton("📋 Chi Tiết", this);
    btnAddCopy = new QPushButton("📑 Thêm Bản Sao", this);
    btnBack = new QPushButton("⬅️ Quay Lại", this);
    
    btnAdd->setStyleSheet("background-color: #27ae60; color: white; padding: 8px; font-weight: bold;");
    btnEdit->setStyleSheet("background-color: #f39c12; color: white; padding: 8px; font-weight: bold;");
    btnDelete->setStyleSheet("background-color: #e74c3c; color: white; padding: 8px; font-weight: bold;");
    btnDetails->setStyleSheet("background-color: #3498db; color: white; padding: 8px; font-weight: bold;");
    btnAddCopy->setStyleSheet("background-color: #9b59b6; color: white; padding: 8px; font-weight: bold;");
    btnBack->setStyleSheet("background-color: #7f8c8d; color: white; padding: 8px; font-weight: bold;");
    
    buttonLayout->addWidget(btnAdd);
    buttonLayout->addWidget(btnEdit);
    buttonLayout->addWidget(btnDelete);
    buttonLayout->addWidget(btnDetails);
    buttonLayout->addWidget(btnAddCopy);
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnBack);
    
    mainLayout->addLayout(buttonLayout);
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
    
    // Real-time search with debounce
    connect(searchEdit, &QLineEdit::textChanged, this, &WidgetQuanLySach::onSearchTextChanged);
    connect(searchTimer, &QTimer::timeout, this, &WidgetQuanLySach::performSearch);
    
    // Filter combo boxes
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
    
    // Kiem tra co the xoa
    if (!KiemTraCoTheXoaDauSach(selectedBook)) {
        QMessageBox::warning(this, "Không thể xóa",
            "Không thể xóa đầu sách này vì còn bản sao đang được mượn hoặc chờ mượn!");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Xác nhận xóa",
        QString("Bạn có chắc muốn xóa đầu sách:\n%1?")
            .arg(QString::fromStdString(selectedBook->tenSach)),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        std::string result = XoaDauSachTheoISBN(selectedBook->ISBN);
        if (result.empty()) {
            showMessage("Xóa đầu sách thành công!");
            refreshData();
        } else {
            showMessage(QString::fromStdString(result), true);
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

