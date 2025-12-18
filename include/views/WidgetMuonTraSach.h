/**
 * @file WidgetMuonTraSach.h
 * @status DEAD / LEGACY
 * @brief File header cũ, KHÔNG ĐƯỢC SỬ DỤNG.
 * @note AI Chú ý: KHÔNG SỬA FILE NÀY. Dùng bao_gom/giao_dien/WidgetMuonTraSach.h
 */
#ifndef WIDGETMUONTRASACH_H
#define WIDGET_MUON_TRA_SACH_H

#include <QWidget>
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QToolBar>
#include <QAction>
#include <QAbstractTableModel>
#include <QStackedWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QShortcut>
#include <QTimer>
#include "models/DocGia.h"
#include "models/DauSach.h"
#include <QFuture>
#include <QFutureWatcher>
#include <vector>
#include "utils/FocusGuard.h"

// ============ WORKFLOW STATE MACHINE ============
enum WorkflowState {
    STATE_NHAP_MA_THE = 0,      // Bước 1: Nhập mã thẻ độc giả
    STATE_HIEN_THONG_TIN = 1,   // Bước 2: Hiển thị thông tin + chọn action
    STATE_MUON_SACH = 2,        // Bước 3a: Workflow mượn sách
    STATE_TRA_SACH = 3,         // Bước 3b: Workflow trả sách
    STATE_BAO_MAT = 4           // Bước 3c: Workflow báo mất
};

// ============ TABLE MODEL CHO SÁCH ĐANG MƯỢN ============
class LoanTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit LoanTableModel(QObject *parent = nullptr);
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    
    void setLoans(const QVector<ThongTinSachDangMuon_DTO>& loanList);
    void clear();
    ThongTinSachDangMuon_DTO getLoanAt(int row) const;
    int getRowCount() const { return cacSachDangMuon.size(); }
    
    // Helper methods for loan status
    bool isLoanOverdue(int row) const;
    int getDaysUntilDue(int row) const; // Negative if overdue

private:
    QVector<ThongTinSachDangMuon_DTO> cacSachDangMuon;
};

// ============ MAIN WIDGET - WORKFLOW ALL-IN-ONE ============
class WidgetMuonTraSach : public QWidget {
    Q_OBJECT

public:
    explicit WidgetMuonTraSach(QWidget *parent = nullptr);
    ~WidgetMuonTraSach();
    
    void refreshData();
    void resetWorkflow(); // Reset về state 0
    void handleGlobalScan(const QString& barcode); // Xử lý quét toàn cục

signals:
    void backToHome();
    void duLieuThayDoi();

private slots:
    // Workflow control
    void onTimDocGiaClicked();
    void onResetWorkflow();
    
    // Action buttons (State 1 → State 2/3/4)
    void onMuonSachMoiClicked();
    void onTraSachClicked();
    void onBaoMatClicked();
    
    // Mượn sách workflow (State 2)
    void onTimSachMuonClicked();
    void onChonBanSaoChanged(int index);
    void onXacNhanMuonClicked();
    
    // Trả sách workflow (State 3)
    void onChonSachTraChanged();
    void onXacNhanTraClicked();
    void onScanTraSach(); // New slot for continuous return scanning
    
    // Báo mất workflow (State 4)
    void onXacNhanBaoMatClicked();
    
    // Helper slots
    void onTableDoubleClicked(const QModelIndex &index);
    void onMaTheEnterPressed();
    void onBookSearchFinished();
    void onBanSaoFinished();

private:
    // ========== UI COMPONENTS ==========
    // Stacked widget cho state machine
    QStackedWidget *stackedWidget;
    
    // STATE 0: Nhập mã thẻ
    QWidget *widgetNhapMaThe;
    QLineEdit *editMaThe;
    QPushButton *btnTimDocGia;
    QLabel *lblHuongDan;
    
    // STATE 1: Hiển thị thông tin độc giả
    QWidget *widgetThongTinDocGia;
    QLabel *lblThongTinDocGia;      // Hiện: Mã, Họ tên, Giới tính
    QLabel *lblSoSachDangMuon;      // "Đang mượn: 2/3"
    QTableView *tableSachDangMuon;   // Danh sách sách đang mượn
    LoanTableModel *modelSachDangMuon;
    QPushButton *btnMuonSachMoi;
    QPushButton *btnTraSach;
    QPushButton *btnBaoMat;
    QPushButton *btnQuayLai;
    
    // STATE 2: Mượn sách mới
    QWidget *widgetMuonSach;
    QLineEdit *editTimSach;          // Tìm theo ISBN/Tên
    QTimer *searchTimer;             // Debounce timer for search
    QPushButton *btnTimSach;
    QLabel *lblKetQuaTimSach;        // "Tìm thấy: Lập Trình C++ - 3 bản sao sẵn"
    QComboBox *comboBanSao;          // Dropdown chọn bản sao
    QLabel *lblViTri;                // "Vị trí: Kệ A1"
    QPushButton *btnXacNhanMuon;
    QPushButton *btnHuyMuon;
    
    // STATE 3: Trả sách
    QWidget *widgetTraSach;
    QTableView *tableTraSach;        // Hiện sách đang mượn với checkbox
    LoanTableModel *modelTraSach;
    QLabel *lblChonSachTra;
    QComboBox *filterStatusCombo;    // Filter: All/Overdue/Normal
    QPushButton *btnXacNhanTra;
    QPushButton *btnHuyTra;
    QLineEdit *editScanTraSach;      // Add hidden scanner input for State 3
    
    // STATE 4: Báo mất
    QWidget *widgetBaoMat;
    QComboBox *comboSachBaoMat;      // Chọn sách bị mất
    QLabel *lblCanhBaoBaoMat;        // "⚠️ Độc giả sẽ phải bồi thường"
    QPushButton *btnXacNhanBaoMat;
    QPushButton *btnHuyBaoMat;
    
    // Common status bar
    QLabel *statusLabel;
    
    // Keyboard shortcuts
    QShortcut *shortcutSearch;
    QShortcut *shortcutBorrow;
    QShortcut *shortcutReturn;
    QShortcut *shortcutRefresh;
    QShortcut *shortcutReset;
    
    // ========== DATA ==========
    WorkflowState currentState;
    PTRDG docGiaHienTai;             // Độc giả đang thao tác
    QVector<ThongTinSachDangMuon_DTO> cacSachDangMuon; // Cache sách đang mượn
    PTRDS sachTimThay;               // Sách tìm được (state 2)
    ThongTinSachDangMuon_DTO sachDuocChon; // Sách được chọn để trả/báo mất
    bool m_autoConfirm; // Flag to trigger zero-click borrow
    // Async search watchers
    QFutureWatcher<std::vector<KetQuaTimKiem>> *bookSearchWatcher;
    QFutureWatcher<std::vector<PTRDMS>> *banSaoWatcher;
    
    // Focus Guard
    FocusGuard *m_focusGuard;
    
    // ========== METHODS ==========
    void setupUI();
    void setupConnections();
    void setupKeyboardShortcuts();
    void chuyenSangState(WorkflowState newState);
    
    // Setup individual states
    void setupStateNhapMaThe();
    void setupStateThongTinDocGia();
    void setupStateMuonSach();
    void setupStateTraSach();
    void setupStateBaoMat();
    
    // State handlers
    void xuLyNhapMaThe();
    void xuLyHienThongTin();
    void xuLyMuonSach();
    void xuLyTraSach();
    void xuLyBaoMat();
    
    // Data loaders (Using backend API)
    bool layThongTinDocGia(int maThe);
    void capNhatDanhSachSachDangMuon();
    void timSachCoTheMuon(const QString& tuKhoa);
    void loadBanSaoKhaDung(PTRDS dauSach);
    
    // UI updaters
    void capNhatLabelThongTinDocGia();
    void hienThiThongBao(const QString &message, bool isError = false);
    void vohieuHoaButtons(bool disabled);
    
    // Validation methods
    bool canReaderBorrowMore() const;  // Check if reader can borrow (< 3 books)
    bool isBookAvailable(const QString& isbn) const; // Check if book has available copies
    QString validateBorrowRequest() const; // Returns error message or empty if valid
    void applyLoanFilters(); // Apply status filter to loan table
    void performBookSearch(); // Debounced search
    
    // Helper to find reader by book (Global Return)
    PTRDG findReaderByBook(const std::string& maSach);
};

#endif // WIDGET_MUON_TRA_SACH_H
