#ifndef WIDGET_QUAN_LY_SACH_H
#define WIDGET_QUAN_LY_SACH_H

#include <QWidget>
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QAbstractTableModel>
#include <QTimer>
#include <QToolBar>
#include <QAction>
#include <QTabWidget>
#include <QTextEdit>
#include <QShortcut>
#include <QItemSelectionModel>
#include <vector>
#include "models/DauSach.h"
#include "views/BadgeStyledItemDelegate.h"

class QFileSystemWatcher;


class BookTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit BookTableModel(QObject *parent = nullptr);
    
    // Cac ham bat buoc cua QAbstractTableModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    
    // Cac ham tuy chinh
    void refreshData();
    void setSearchResults(PTRDS* results, int count);
    void setBooks(const std::vector<PTRDS>& bookList);  // Overload cho vector
    PTRDS getBookAt(int row) const;
    void clearData();
    
    // Pagination support
    void setPageSize(int size);
    int pageSize() const;
    void setPage(int page);
    int page() const;
    int totalPages() const;
    int totalCount() const;
    
    // Sorting
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private:
    PTRDS* books;
    int bookCount; // total number of books (unpaged)
    bool isSearchMode;
    int sortColumn;
    Qt::SortOrder sortOrder;
    // pagination state
    int m_pageSize;
    int m_page;
    std::vector<PTRDS> cachedBooks;  // Cache cho vector mode
    // Caches to avoid expensive per-cell traversal of linked lists
    std::vector<int> cachedAvailableCount;   // number of copies available (CHO_MUON_DUOC)
    std::vector<int> cachedTotalCopies;      // total copies (tongBanSao)
    std::vector<int> cachedStatusEnum;       // status enum per book for column 7 (CHO_MUON_DUOC/DANG_MUON/THANH_LY)
    void computeCaches(); // recompute caches for current `books` pointer
};

class WidgetQuanLySach : public QWidget {
    Q_OBJECT

public:
    explicit WidgetQuanLySach(QWidget *parent = nullptr);
    ~WidgetQuanLySach();
    
    void refreshData();

signals:
    void backToHome();

private slots:
    void onSearchClicked();
    void onClearSearchClicked();
    void onAddBookClicked();
    void onEditBookClicked();
    void onDeleteBookClicked();
    void onViewDetailsClicked();
    void onAddCopyClicked();
    void onBackClicked();
    void onTableDoubleClicked(const QModelIndex &index);
    void onSearchTextChanged();
    void performSearch();
    void onExportCSV();
    void onImportCSV();
    void onPrintList();
    void onBulkDelete();
    void onFilterChanged();
    
    // Tab Báo Cáo slots
    void onRefreshTop10();
    void onRefreshThongKe();
    void onWatchedFileChanged(const QString &path);

private:
    // Main layout
    QTabWidget *tabWidget;
    
    // TAB 1: Quản lý sách
    QWidget *tabQuanLy;
    QTableView *tableView;
    BookTableModel *bookModel;
    BadgeStyledItemDelegate *badgeDelegate;
    QLineEdit *searchEdit;
    QComboBox *searchTypeCombo;
    QComboBox *filterTheLoaiCombo;
    QComboBox *filterTrangThaiCombo;
    QPushButton *btnSearch;
    QPushButton *btnClearSearch;
    QPushButton *btnAdd;
    QPushButton *btnEdit;
    QPushButton *btnDelete;
    QPushButton *btnDetails;
    QPushButton *btnAddCopy;
    QPushButton *btnBack;
    QLabel *statusLabel;
    QToolBar *toolbar;
    QTimer *searchTimer;
    QFileSystemWatcher *fsWatcher;
    QTimer *fsDebounceTimer;
    // Pagination controls
    QPushButton *btnPrevPage;
    QPushButton *btnNextPage;
    QLabel *lblPageInfo;
    QComboBox *cbPageSize;
    
    // Keyboard shortcuts
    QShortcut *shortcutSearch;
    QShortcut *shortcutAdd;
    QShortcut *shortcutEdit;
    QShortcut *shortcutDelete;
    QShortcut *shortcutRefresh;
    
    // TAB 2: Top 10 Sách
    QWidget *tabTop10;
    QTableView *tableTop10;
    QAbstractTableModel *modelTop10;
    QPushButton *btnRefreshTop10;
    
    // TAB 3: Thống kê
    QWidget *tabThongKe;
    QTextEdit *txtThongKe;
    QPushButton *btnRefreshThongKe;
    
    // Data
    PTRDS selectedBook;
    
    void setupUI();
    void setupTabQuanLy();
    void setupTabTop10();
    void setupTabThongKe();
    void setupConnections();
    void updateButtonStates();
    void showMessage(const QString &message, bool isError = false);
    void setupKeyboardShortcuts();
    void applyAdvancedFilter();
    
    // Validation helpers
    bool validateBookData(const QString &isbn, const QString &tenSach,
                         const QString &tacGia, int namXB, int soTrang);
    
    // Dialog functions
    void showAddBookDialog();
    void showEditBookDialog();
    void showBookDetailsDialog();
    void showAddCopyDialog();
};

#endif // WIDGET_QUAN_LY_SACH_H
