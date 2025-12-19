/**
 * @file WidgetQuanLyDocGia.h
 * @status DEAD / LEGACY
 * @brief File header cũ, KHÔNG ĐƯỢC SỬ DỤNG.
 * @note AI Chú ý: KHÔNG SỬA FILE NÀY. Dùng bao_gom/giao_dien/WidgetQuanLyDocGia.h
 */
#ifndef WIDGETQUANLYDOCGIA_H
#define WIDGET_QUAN_LY_DOC_GIA_H

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
#include <QShortcut>
#include <QItemSelectionModel>
#include "models/DocGia.h"
#include "views/BadgeStyledItemDelegate.h"
#include <vector>

class ReaderTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit ReaderTableModel(QObject *parent = nullptr);
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    
    void refreshData();
    void setSearchResults(PTRDG* results, int count);
    void setSearchResults(const std::vector<PTRDG> &results);
    PTRDG getReaderAt(int row) const;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    
    // Advanced features
    bool isReaderOverdue(PTRDG reader) const;
    int getBorrowedBookCount(PTRDG reader) const;

private:
    std::vector<PTRDG> readers;
    bool isSearchMode;
    int sortColumn;
    Qt::SortOrder sortOrder;
    
    void collectReadersFromTree(PTRDG root, std::vector<PTRDG> &out);
};

class WidgetQuanLyDocGia : public QWidget {
    Q_OBJECT

public:
    explicit WidgetQuanLyDocGia(QWidget *parent = nullptr);
    ~WidgetQuanLyDocGia();
    
    void refreshData();

signals:
    void backToHome();

private slots:
    void onSearchClicked();
    void onClearSearchClicked();
    void onDeleteReaderClicked();
    void onViewLoansClicked();
    void onBackClicked();
    void onTableDoubleClicked(const QModelIndex &index);
    void onSearchTextChanged();
    void performSearch();
    void onToggleStatus();
    void onViewHistory();
    void onPrintCard();
    void onPrintList();
    void onExportCSV();
    void onImportCSV();
    void onShowWarnings();
    
    // Tab Quá hạn slots
    void onRefreshQuaHan();
    void onTableQuaHanDoubleClicked(const QModelIndex &index);

private:
    // Main layout
    QTabWidget *tabWidget;
    
    // TAB 1: Quản lý độc giả
    QWidget *tabQuanLy;
    QTableView *tableView;
    ReaderTableModel *readerModel;
    QLineEdit *searchEdit;
    QComboBox *searchTypeCombo;
    QComboBox *filterGioiTinhCombo;
    QComboBox *filterTrangThaiCombo;
    QComboBox *filterStatusCombo;
    QPushButton *btnSearch;
    QPushButton *btnClearSearch;
    QPushButton *btnAdd;
    QPushButton *btnEdit;
    QPushButton *btnDelete;
    QPushButton *btnDetails;
    QPushButton *btnUnlock;
    QPushButton *btnViewLoans;
    QPushButton *btnBack;
    QLabel *statusLabel;
    QToolBar *toolbar;
    QTimer *searchTimer;
    
    // Keyboard shortcuts
    QShortcut *shortcutSearch;
    QShortcut *shortcutAdd;
    QShortcut *shortcutEdit;
    QShortcut *shortcutDelete;
    QShortcut *shortcutRefresh;
    BadgeStyledItemDelegate *badgeDelegate;
    
    // TAB 2: Độc giả quá hạn
    QWidget *tabQuaHan;
    QTableView *tableQuaHan;
    QAbstractTableModel *modelQuaHan;
    QPushButton *btnRefreshQuaHan;
    QLabel *lblThongKeQuaHan;
    
    PTRDG selectedReader;
    QProgressBar *searchProgress;
    // Async search watcher
    QFutureWatcher<std::vector<PTRDG>> *searchWatcher;
    
    void setupUI();
    void setupTabQuanLy();
    void setupTabQuaHan();
    void setupConnections();
    void setupKeyboardShortcuts();
    void onSearchFinished();
    void updateButtonStates();
    void showMessage(const QString &message, bool isError = false);
    void showAddReaderDialog();
    void showEditReaderDialog();
    void applyFilters();
    
    // Validation
    QString validateCMND(const QString &cmnd); // Returns error message or empty if valid
};

#endif // WIDGET_QUAN_LY_DOC_GIA_H
