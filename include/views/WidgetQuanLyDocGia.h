#ifndef WIDGET_QUAN_LY_DOC_GIA_H
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
    PTRDG getReaderAt(int row) const;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    
    // Advanced features
    bool isReaderOverdue(PTRDG reader) const;
    int getBorrowedBookCount(PTRDG reader) const;

private:
    PTRDG* readers;
    int readerCount;
    bool isSearchMode;
    int sortColumn;
    Qt::SortOrder sortOrder;
    
    void collectReadersFromTree(PTRDG root, PTRDG* arr, int &count);
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
    QPushButton *btnSearch;
    QPushButton *btnClearSearch;
    QPushButton *btnAdd;
    QPushButton *btnEdit;
    QPushButton *btnDelete;
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
    
    // TAB 2: Độc giả quá hạn
    QWidget *tabQuaHan;
    QTableView *tableQuaHan;
    QAbstractTableModel *modelQuaHan;
    QPushButton *btnRefreshQuaHan;
    QLabel *lblThongKeQuaHan;
    
    PTRDG selectedReader;
    
    void setupUI();
    void setupTabQuanLy();
    void setupTabQuaHan();
    void setupConnections();
    void setupKeyboardShortcuts();
    void updateButtonStates();
    void showMessage(const QString &message, bool isError = false);
    void showAddReaderDialog();
    void showEditReaderDialog();
    void applyFilters();
    
    // Validation
    QString validateCMND(const QString &cmnd); // Returns error message or empty if valid
};

#endif // WIDGET_QUAN_LY_DOC_GIA_H
