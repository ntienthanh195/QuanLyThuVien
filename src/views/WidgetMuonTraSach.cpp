#include "views/WidgetMuonTraSach.h"
#include "models/DocGia.h"
#include "models/QuanLySach.h"
#include "utils/NgayThang.h"
#include "utils/AnimationHelper.h"
#include "utils/DBBatcher.h"
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <algorithm>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QLabel>
#include <QFont>
#include <QRegularExpression>
#include <QDebug>
#include <QScrollArea>
#include <QFrame>
#include <QFrame>
#include <QShortcut>
#include <QFile>
#include <QPixmap>

extern PTRDG rootDocGia;
extern PTRDS dsDauSach[MAX_DAUSACH];
extern int soLuongDauSach;

// Modern Card-based Design - No hardcoded styles
// All styles are managed by modern-dashboard.qss

// ============================================
// LOAN TABLE MODEL - Hiển thị sách đang mượn
// ============================================

LoanTableModel::LoanTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int LoanTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return cacSachDangMuon.size();
}

int LoanTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 4; // Mã Sách | Tên Sách | Ngày Mượn | Số Ngày Giữ
}

QVariant LoanTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= cacSachDangMuon.size())
        return QVariant();
    
    const ThongTinSachDangMuon_DTO &info = cacSachDangMuon[index.row()];
    
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return QString::fromStdString(info.maSach);
            case 1: return QString::fromStdString(info.tenSach);
            case 2: return QString::fromStdString(info.ngayMuon);
            case 3: return info.soNgayGiu;
        }
    } else if (role == Qt::ForegroundRole) {
        // Highlight quá hạn (> 7 ngày)
        if (info.soNgayGiu > 7) {
            return QColor("#DC2626"); // Red
        } else if (info.soNgayGiu >= 5) {
            return QColor("#F59E0B"); // Warning
        }
    } else if (role == Qt::FontRole) {
        if (info.soNgayGiu > 7) {
            QFont font;
            font.setBold(true);
            return font;
        }
    }
    
    return QVariant();
}

QVariant LoanTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();
    
    switch (section) {
        case 0: return "Mã Sách";
        case 1: return "Tên Sách";
        case 2: return "Ngày Mượn";
        case 3: return "Số Ngày";
    }
    return QVariant();
}

void LoanTableModel::setLoans(const QVector<ThongTinSachDangMuon_DTO>& loanList) {
    beginResetModel();
    cacSachDangMuon = loanList;
    endResetModel();
}

void LoanTableModel::clear() {
    beginResetModel();
    cacSachDangMuon.clear();
    endResetModel();
}

ThongTinSachDangMuon_DTO LoanTableModel::getLoanAt(int row) const {
    if (row >= 0 && row < cacSachDangMuon.size()) {
        return cacSachDangMuon[row];
    }
    return ThongTinSachDangMuon_DTO();
}

bool LoanTableModel::isLoanOverdue(int row) const {
    if (row >= 0 && row < cacSachDangMuon.size()) {
        return cacSachDangMuon[row].soNgayGiu > 7;
    }
    return false;
}

int LoanTableModel::getDaysUntilDue(int row) const {
    if (row >= 0 && row < cacSachDangMuon.size()) {
        return 7 - cacSachDangMuon[row].soNgayGiu; // Negative if overdue
    }
    return 0;
}

// ============================================
// MAIN WIDGET - WORKFLOW STATE MACHINE
// ============================================

WidgetMuonTraSach::WidgetMuonTraSach(QWidget *parent)
    : QWidget(parent)
    , currentState(STATE_NHAP_MA_THE)
    , docGiaHienTai(nullptr)
    , sachTimThay(nullptr)
{
    // Initialize search timer for debounced search
    searchTimer = new QTimer(this);
    searchTimer->setSingleShot(true);
    searchTimer->setInterval(300); // 300ms debounce
    
    // Initialize async watchers
    bookSearchWatcher = new QFutureWatcher<std::vector<KetQuaTimKiem>>(this);
    banSaoWatcher = new QFutureWatcher<std::vector<PTRDMS>>(this);
        connect(bookSearchWatcher, &QFutureWatcher<std::vector<KetQuaTimKiem>>::finished,
            this, &WidgetMuonTraSach::onBookSearchFinished);
        connect(banSaoWatcher, &QFutureWatcher<std::vector<PTRDMS>>::finished,
            this, &WidgetMuonTraSach::onBanSaoFinished);

        setupUI();
        setupConnections();
        setupKeyboardShortcuts();
        
        // Init FocusGuard
        m_focusGuard = new FocusGuard(this);
        m_autoConfirm = false;
        
        chuyenSangState(STATE_NHAP_MA_THE);
}

WidgetMuonTraSach::~WidgetMuonTraSach() {
}

void WidgetMuonTraSach::setupUI() {
    // MASTER PLAN: State Machine với Split View Design
    this->setProperty("class", "WidgetMuonTraSach");
    
    // Stacked widget cho state machine (toàn bộ màn hình)
    stackedWidget = new QStackedWidget(this);
    
    QVBoxLayout *containerLayout = new QVBoxLayout(this);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->addWidget(stackedWidget);
    
    // Setup 5 states - mỗi state là 1 màn hình độc lập
    setupStateNhapMaThe();      // State 0: Login Card (Centered)
    setupStateThongTinDocGia(); // State 1: Split View (ProfileCard + Workspace)
    setupStateMuonSach();       // State 2: Split View + Mượn sách form
    setupStateTraSach();        // State 3: Split View + Trả sách form
    setupStateBaoMat();         // State 4: Split View + Báo mất form
    
    // Status label cần được tạo tại widget cha, không nằm trong stacked
    statusLabel = new QLabel(this);
    statusLabel->setWordWrap(true);
    // Use centralized InfoBox styling instead of inline stylesheet
    statusLabel->setProperty("class", "InfoBox");
    
    // Thêm statusLabel vào layout chính nếu cần
    containerLayout->addWidget(statusLabel);
}

// ============================================
// SETUP STATES
// ============================================

// ============================================
// STATE 0: LOGIN SCREEN (CENTERED CARD)
// ============================================

void WidgetMuonTraSach::setupStateNhapMaThe() {
    widgetNhapMaThe = new QWidget();
    widgetNhapMaThe->setProperty("class", "WidgetMuonTraSach");
    
    // Layout chính - Căn giữa màn hình
    QVBoxLayout *centerLayout = new QVBoxLayout(widgetNhapMaThe);
    centerLayout->setAlignment(Qt::AlignCenter);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    
    // MASTER PLAN: Login Card 500x450px, căn giữa
    QFrame *loginCard = new QFrame();
    loginCard->setFixedSize(500, 450);
    loginCard->setProperty("class", "Card");
    loginCard->setFrameShape(QFrame::NoFrame);
    
    QVBoxLayout *cardLayout = new QVBoxLayout(loginCard);
    cardLayout->setSpacing(20);
    cardLayout->setContentsMargins(40, 40, 40, 40);
    
    // Icon 64px
    QLabel *iconLabel = new QLabel();
    if (QFile::exists(":/icons/user.png")) {
        iconLabel->setPixmap(QPixmap(":/icons/user.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        // Fallback or empty
    }
    // Set size via font rather than inline stylesheet
    QFont iconFont = iconLabel->font();
    iconFont.setPointSize(64);
    iconLabel->setFont(iconFont);
    // Keep background transparent via class
    iconLabel->setProperty("class", "IconTransparent");
    iconLabel->setAlignment(Qt::AlignCenter);
    
    // Title
    QLabel *titleLabel = new QLabel("Chào mừng");
    titleLabel->setProperty("class", "CardTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    
    // Subtitle
    lblHuongDan = new QLabel("Vui lòng nhập mã thẻ học viên để truy cập");
    lblHuongDan->setProperty("class", "SubText");
    lblHuongDan->setAlignment(Qt::AlignCenter);
    
    // Input Field - MASTER PLAN: 50px height, 16px font, bo tròn
    editMaThe = new QLineEdit();
    editMaThe->setPlaceholderText("Nhập mã thẻ...");
    editMaThe->setAlignment(Qt::AlignCenter);
    editMaThe->setMinimumHeight(50);
    QFont fontInput;
    fontInput.setPointSize(16);
    editMaThe->setFont(fontInput);
    
    // Button - MASTER PLAN: Primary class, to, nổi bật
    btnTimDocGia = new QPushButton("Truy Cập Hệ Thống");
    btnTimDocGia->setProperty("class", "Primary");
    btnTimDocGia->setMinimumHeight(50);
    btnTimDocGia->setCursor(Qt::PointingHandCursor);
    QFont fontButton;
    fontButton.setPointSize(14);
    fontButton.setBold(true);
    btnTimDocGia->setFont(fontButton);
    
    cardLayout->addStretch();
    cardLayout->addWidget(iconLabel);
    cardLayout->addSpacing(10);
    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(lblHuongDan);
    cardLayout->addSpacing(20);
    cardLayout->addWidget(editMaThe);
    cardLayout->addWidget(btnTimDocGia);
    cardLayout->addStretch(2);
    
    centerLayout->addWidget(loginCard);
    
    stackedWidget->addWidget(widgetNhapMaThe); // Index 0
}

// ============================================
// STATE 1: MAIN WORKSPACE (SPLIT VIEW)
// ============================================

void WidgetMuonTraSach::setupStateThongTinDocGia() {
    widgetThongTinDocGia = new QWidget();
    
    // MASTER PLAN: Split View - Left 320px (ProfileCard) + Right (Workspace)
    QHBoxLayout *mainLayout = new QHBoxLayout(widgetThongTinDocGia);
    mainLayout->setSpacing(0); // Dính sát nhau
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // ===== LEFT PANEL: PROFILE CARD (Fixed 320px) =====
    QFrame *leftPanel = new QFrame();
    leftPanel->setProperty("class", "ProfileCard");
    leftPanel->setFixedWidth(320);
    leftPanel->setFrameShape(QFrame::NoFrame);
    
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(20);
    leftLayout->setContentsMargins(25, 30, 25, 30);
    
    // Avatar + Tên
    // Avatar + Tên
    QLabel *avatarLabel = new QLabel();
    if (QFile::exists(":/icons/reader.png")) {
        avatarLabel->setPixmap(QPixmap(":/icons/reader.png").scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    avatarLabel->setStyleSheet("background: transparent;");
    avatarLabel->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(avatarLabel);
    
    lblThongTinDocGia = new QLabel();
    lblThongTinDocGia->setWordWrap(true);
    lblThongTinDocGia->setStyleSheet("color: #FFFFFF; font-size: 16px; font-weight: bold; background: transparent;");
    lblThongTinDocGia->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(lblThongTinDocGia);
    
    // Stats (Số sách đang mượn)
    lblSoSachDangMuon = new QLabel();
    lblSoSachDangMuon->setWordWrap(true);
    lblSoSachDangMuon->setStyleSheet("color: #B5B5C3; font-size: 14px; background: transparent;");
    lblSoSachDangMuon->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(lblSoSachDangMuon);
    
    leftLayout->addStretch();
    
    // Logout Button - Use btnQuayLai (declared in header)
    btnQuayLai = new QPushButton("Đăng Xuất");
    btnQuayLai->setProperty("class", "Danger");
    btnQuayLai->setMinimumHeight(45);
    leftLayout->addWidget(btnQuayLai);
    
    mainLayout->addWidget(leftPanel);
    
    // ===== RIGHT PANEL: WORKSPACE =====
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(25);
    rightLayout->setContentsMargins(30, 30, 30, 30);
    
    // Title
    QLabel *workspaceTitle = new QLabel("Sách Đang Mượn");
    workspaceTitle->setProperty("class", "PageTitle");
    rightLayout->addWidget(workspaceTitle);
    
    // Table Card
    QFrame *tableCard = new QFrame();
    tableCard->setProperty("class", "Card");
    tableCard->setFrameShape(QFrame::NoFrame);
    QVBoxLayout *tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(1, 1, 1, 1);
    
    tableSachDangMuon = new QTableView();
    modelSachDangMuon = new LoanTableModel(this);
    tableSachDangMuon->setModel(modelSachDangMuon);
    
    // Cấu hình bảng
    tableSachDangMuon->setShowGrid(false);
    tableSachDangMuon->verticalHeader()->setVisible(false);
    tableSachDangMuon->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableSachDangMuon->setSelectionMode(QAbstractItemView::SingleSelection);
    tableSachDangMuon->horizontalHeader()->setStretchLastSection(true);
    tableSachDangMuon->setSortingEnabled(false);
    tableSachDangMuon->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableSachDangMuon->verticalHeader()->setDefaultSectionSize(45);
    tableSachDangMuon->setMinimumHeight(300);
    // Let table expand to use vertical/horizontal empty space
    tableSachDangMuon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // Make the 'Tên Sách' column expand to fill remaining width
    tableSachDangMuon->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    tableLayout->addWidget(tableSachDangMuon, 1);
    rightLayout->addWidget(tableCard, 1);
    
    // Action Buttons (To, nổi bật)
    QHBoxLayout *actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(15);
    
    btnMuonSachMoi = new QPushButton("Mượn Sách");
    btnMuonSachMoi->setProperty("class", "Primary");
    btnMuonSachMoi->setMinimumHeight(55);
    btnMuonSachMoi->setMinimumWidth(180);
    
    btnTraSach = new QPushButton("Trả Sách");
    btnTraSach->setProperty("class", "Success");
    btnTraSach->setMinimumHeight(55);
    btnTraSach->setMinimumWidth(180);
    
    btnBaoMat = new QPushButton("Báo Mất");
    btnBaoMat->setProperty("class", "Danger");
    btnBaoMat->setMinimumHeight(55);
    btnBaoMat->setMinimumWidth(180);
    
    // Right-align action buttons to use empty space effectively
    actionLayout->addStretch();
    actionLayout->addWidget(btnMuonSachMoi);
    actionLayout->addWidget(btnTraSach);
    actionLayout->addWidget(btnBaoMat);
    
    rightLayout->addLayout(actionLayout);
    
    mainLayout->addWidget(rightPanel, 1); // Giãn hết phần còn lại
    
    stackedWidget->addWidget(widgetThongTinDocGia); // Index 1
}

void WidgetMuonTraSach::setupStateMuonSach() {
    widgetMuonSach = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widgetMuonSach);
    layout->setSpacing(20);
    
    QLabel *titleMuon = new QLabel("MƯỢN SÁCH MỚI", widgetMuonSach);
    QFont font = titleMuon->font();
    font.setPointSize(15);
    font.setBold(true);
    titleMuon->setFont(font);
    titleMuon->setStyleSheet("color: #2D3748; padding: 8px 0;");
    layout->addWidget(titleMuon);
    
    // Tìm sách
    QGroupBox *groupSearch = new QGroupBox("Tìm Sách", widgetMuonSach);
    QVBoxLayout *searchLayout = new QVBoxLayout(groupSearch);
    
    QHBoxLayout *searchBar = new QHBoxLayout();
    editTimSach = new QLineEdit(groupSearch);
    editTimSach->setPlaceholderText("Nhập ISBN hoặc tên sách...");
    editTimSach->setMinimumHeight(40);
    editTimSach->setToolTip("Tìm sách theo ISBN hoặc tên (Ctrl+F để focus)");
    searchBar->addWidget(editTimSach);
    
    btnTimSach = new QPushButton("Tìm", groupSearch);
    btnTimSach->setProperty("class", "btn-primary");
    btnTimSach->setMinimumHeight(42);
    btnTimSach->setMinimumWidth(100);
    btnTimSach->setToolTip("Tìm kiếm sách (Enter)");
    searchBar->addWidget(btnTimSach);
    
    searchLayout->addLayout(searchBar);
    
    lblKetQuaTimSach = new QLabel(groupSearch);
    lblKetQuaTimSach->setWordWrap(true);
    searchLayout->addWidget(lblKetQuaTimSach);
    
    layout->addWidget(groupSearch);
    
    // Chọn bản sao
    QGroupBox *groupSelect = new QGroupBox("Chọn Bản Sao", widgetMuonSach);
    QFormLayout *selectLayout = new QFormLayout(groupSelect);
    
    comboBanSao = new QComboBox(groupSelect);
    comboBanSao->setMinimumHeight(42);
    selectLayout->addRow("Bản sao:", comboBanSao);
    
    lblViTri = new QLabel("", groupSelect);
    lblViTri->setStyleSheet("color: #6B7280; font-style: italic;");
    selectLayout->addRow("Vị trí:", lblViTri);
    
    layout->addWidget(groupSelect);
    
    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnXacNhanMuon = new QPushButton("Xác Nhận Mượn", widgetMuonSach);
    btnXacNhanMuon->setProperty("class", "btn-success");
    btnXacNhanMuon->setMinimumHeight(44);
    btnXacNhanMuon->setEnabled(false);
    btnLayout->addWidget(btnXacNhanMuon);
    
    btnHuyMuon = new QPushButton("Hủy", widgetMuonSach);
    btnHuyMuon->setProperty("class", "btn-secondary");
    btnHuyMuon->setMinimumHeight(45);
    btnLayout->addWidget(btnHuyMuon);
    
    layout->addLayout(btnLayout);
    layout->addStretch();
    
    stackedWidget->addWidget(widgetMuonSach); // Index 2
}

void WidgetMuonTraSach::setupStateTraSach() {
    widgetTraSach = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widgetTraSach);
    layout->setSpacing(20);
    
    QLabel *titleTra = new QLabel("TRẢ SÁCH", widgetTraSach);
    QFont font = titleTra->font();
    font.setPointSize(15);
    font.setBold(true);
    titleTra->setFont(font);
    titleTra->setStyleSheet("color: #2D3748; padding: 8px 0;");
    layout->addWidget(titleTra);
    
    lblChonSachTra = new QLabel("Chọn sách muốn trả từ danh sách bên dưới:", widgetTraSach);
    lblChonSachTra->setStyleSheet("color: #6B7280; padding: 4px 0;");
    layout->addWidget(lblChonSachTra);
    
    // Continuous Scan Input for Return
    QHBoxLayout *scanLayout = new QHBoxLayout();
    QLabel *lblScan = new QLabel("Quét mã sách để trả ngay:", widgetTraSach);
    lblScan->setStyleSheet("font-weight: bold; color: #2563EB;");
    
    editScanTraSach = new QLineEdit(widgetTraSach);
    editScanTraSach->setPlaceholderText("Quét mã vạch sách tại đây...");
    editScanTraSach->setMinimumHeight(40);
    // Focus policy is important
    editScanTraSach->setFocusPolicy(Qt::StrongFocus);
    
    scanLayout->addWidget(lblScan);
    scanLayout->addWidget(editScanTraSach);
    layout->addLayout(scanLayout);
    
    // Filter controls
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(12);
    
    QLabel *filterLabel = new QLabel("Lọc:", widgetTraSach);
    filterLabel->setStyleSheet("font-weight: bold; color: #64B5F6;");
    filterLayout->addWidget(filterLabel);
    
    filterStatusCombo = new QComboBox(widgetTraSach);
    filterStatusCombo->addItem("Tất cả");
    filterStatusCombo->addItem("Quá hạn");
    filterStatusCombo->addItem("Bình thường");
    filterStatusCombo->setMinimumWidth(150);
    filterStatusCombo->setToolTip("Lọc sách theo trạng thái");
    filterLayout->addWidget(filterStatusCombo);
    
    filterLayout->addStretch();
    layout->addLayout(filterLayout);
    
    // Connect filter
    connect(filterStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &WidgetMuonTraSach::applyLoanFilters);
    
    // Table để chọn sách trả
    tableTraSach = new QTableView(widgetTraSach);
    modelTraSach = new LoanTableModel(this);
    tableTraSach->setModel(modelTraSach);
    tableTraSach->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableTraSach->setSelectionMode(QAbstractItemView::SingleSelection);
    tableTraSach->horizontalHeader()->setStretchLastSection(true);
    tableTraSach->setAlternatingRowColors(true);
    tableTraSach->setMinimumHeight(250);
    layout->addWidget(tableTraSach);
    
    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnXacNhanTra = new QPushButton("Xác Nhận Trả", widgetTraSach);
    btnXacNhanTra->setProperty("class", "btn-success");
    btnXacNhanTra->setMinimumHeight(44);
    btnLayout->addWidget(btnXacNhanTra);
    
    btnHuyTra = new QPushButton("Hủy", widgetTraSach);
    btnHuyTra->setProperty("class", "btn-secondary");
    btnHuyTra->setMinimumHeight(45);
    btnLayout->addWidget(btnHuyTra);
    
    layout->addLayout(btnLayout);
    
    stackedWidget->addWidget(widgetTraSach); // Index 3
}

void WidgetMuonTraSach::setupStateBaoMat() {
    widgetBaoMat = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widgetBaoMat);
    layout->setSpacing(20);
    
    QLabel *titleBaoMat = new QLabel("BÁO MẤT SÁCH", widgetBaoMat);
    QFont font = titleBaoMat->font();
    font.setPointSize(15);
    font.setBold(true);
    titleBaoMat->setFont(font);
    titleBaoMat->setStyleSheet("color: #DC2626; padding: 8px 0;");
    layout->addWidget(titleBaoMat);
    
    // Warning
    lblCanhBaoBaoMat = new QLabel(
        "<b>CẢNH BÁO:</b> Khi báo mất sách, học viên sẽ phải bồi thường.<br>"
        "Trạng thái sách sẽ được cập nhật thành <b>Đã mất</b>.",
        widgetBaoMat
    );
    lblCanhBaoBaoMat->setStyleSheet(
        "background-color: #FEF3C7; color: #B45309; "
        "padding: 16px; border-radius: 8px; "
        "border: 1.5px solid #FDE68A; font-size: 13px;"
    );
    lblCanhBaoBaoMat->setWordWrap(true);
    layout->addWidget(lblCanhBaoBaoMat);
    
    // Chọn sách
    QFormLayout *formLayout = new QFormLayout();
    comboSachBaoMat = new QComboBox(widgetBaoMat);
    comboSachBaoMat->setMinimumHeight(42);
    formLayout->addRow("Chọn sách bị mất:", comboSachBaoMat);
    layout->addLayout(formLayout);
    
    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnXacNhanBaoMat = new QPushButton("Xác Nhận Báo Mất", widgetBaoMat);
    btnXacNhanBaoMat->setProperty("class", "btn-danger");
    btnXacNhanBaoMat->setMinimumHeight(44);
    btnLayout->addWidget(btnXacNhanBaoMat);
    
    btnHuyBaoMat = new QPushButton("Hủy", widgetBaoMat);
    btnHuyBaoMat->setProperty("class", "btn-secondary");
    btnHuyBaoMat->setMinimumHeight(45);
    btnLayout->addWidget(btnHuyBaoMat);
    
    layout->addLayout(btnLayout);
    layout->addStretch();
    
    stackedWidget->addWidget(widgetBaoMat); // Index 4
}

// ============================================
// CONNECTIONS
// ============================================

void WidgetMuonTraSach::setupConnections() {
    // State 0
    connect(btnTimDocGia, &QPushButton::clicked, this, &WidgetMuonTraSach::onTimDocGiaClicked);
    connect(editMaThe, &QLineEdit::returnPressed, this, &WidgetMuonTraSach::onMaTheEnterPressed);
    
    // State 1
    connect(btnMuonSachMoi, &QPushButton::clicked, this, &WidgetMuonTraSach::onMuonSachMoiClicked);
    connect(btnTraSach, &QPushButton::clicked, this, &WidgetMuonTraSach::onTraSachClicked);
    connect(btnBaoMat, &QPushButton::clicked, this, &WidgetMuonTraSach::onBaoMatClicked);
    connect(btnQuayLai, &QPushButton::clicked, this, &WidgetMuonTraSach::onResetWorkflow);
    connect(tableSachDangMuon, &QTableView::doubleClicked, this, &WidgetMuonTraSach::onTableDoubleClicked);
    
    // State 2
    connect(btnTimSach, &QPushButton::clicked, this, &WidgetMuonTraSach::onTimSachMuonClicked);
    connect(editTimSach, &QLineEdit::returnPressed, this, &WidgetMuonTraSach::onTimSachMuonClicked);
    connect(comboBanSao, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &WidgetMuonTraSach::onChonBanSaoChanged);
    connect(btnXacNhanMuon, &QPushButton::clicked, this, &WidgetMuonTraSach::onXacNhanMuonClicked);
    connect(btnHuyMuon, &QPushButton::clicked, [this]() { chuyenSangState(STATE_HIEN_THONG_TIN); });
    
    // State 3
    connect(editScanTraSach, &QLineEdit::returnPressed, this, &WidgetMuonTraSach::onScanTraSach);
    connect(tableTraSach, &QTableView::clicked, this, &WidgetMuonTraSach::onChonSachTraChanged);
    connect(btnXacNhanTra, &QPushButton::clicked, this, &WidgetMuonTraSach::onXacNhanTraClicked);
    connect(btnHuyTra, &QPushButton::clicked, [this]() { chuyenSangState(STATE_HIEN_THONG_TIN); });
    
    // State 4
    connect(btnXacNhanBaoMat, &QPushButton::clicked, this, &WidgetMuonTraSach::onXacNhanBaoMatClicked);
    connect(btnHuyBaoMat, &QPushButton::clicked, [this]() { chuyenSangState(STATE_HIEN_THONG_TIN); });
    
    // Search timer for debounced search
    connect(searchTimer, &QTimer::timeout, this, &WidgetMuonTraSach::performBookSearch);
    connect(editTimSach, &QLineEdit::textChanged, [this]() {
        searchTimer->start(); // Restart timer on each keystroke
    });
}

void WidgetMuonTraSach::setupKeyboardShortcuts() {
    // Ctrl+F - Focus search (in State 2)
    shortcutSearch = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(shortcutSearch, &QShortcut::activated, [this]() {
        if (currentState == STATE_MUON_SACH) {
            editTimSach->setFocus();
            editTimSach->selectAll();
        } else if (currentState == STATE_NHAP_MA_THE) {
            editMaThe->setFocus();
            editMaThe->selectAll();
        }
    });
    
    // Ctrl+B - Quick borrow (State 1 → State 2)
    shortcutBorrow = new QShortcut(QKeySequence("Ctrl+B"), this);
    connect(shortcutBorrow, &QShortcut::activated, [this]() {
        if (currentState == STATE_HIEN_THONG_TIN && btnMuonSachMoi->isEnabled()) {
            onMuonSachMoiClicked();
        }
    });
    
    // Ctrl+R - Quick return (State 1 → State 3)
    shortcutReturn = new QShortcut(QKeySequence("Ctrl+R"), this);
    connect(shortcutReturn, &QShortcut::activated, [this]() {
        if (currentState == STATE_HIEN_THONG_TIN && btnTraSach->isEnabled()) {
            onTraSachClicked();
        }
    });
    
    // F5 - Refresh reader info
    shortcutRefresh = new QShortcut(QKeySequence("F5"), this);
    connect(shortcutRefresh, &QShortcut::activated, [this]() {
        if (currentState == STATE_HIEN_THONG_TIN && docGiaHienTai) {
            capNhatDanhSachSachDangMuon();
            hienThiThongBao("✓ Đã làm mới thông tin", false);
        }
    });
    
    // ESC - Reset workflow / Go back
    shortcutReset = new QShortcut(QKeySequence("Escape"), this);
    connect(shortcutReset, &QShortcut::activated, [this]() {
        if (currentState == STATE_HIEN_THONG_TIN) {
            onResetWorkflow();
        } else if (currentState != STATE_NHAP_MA_THE) {
            chuyenSangState(STATE_HIEN_THONG_TIN);
        }
    });
}

// ============================================
// STATE MACHINE
// ============================================

void WidgetMuonTraSach::chuyenSangState(WorkflowState newState) {
    currentState = newState;
    stackedWidget->setCurrentIndex(static_cast<int>(newState));
    
    switch (newState) {
        case STATE_NHAP_MA_THE:
            xuLyNhapMaThe();
            break;
        case STATE_HIEN_THONG_TIN:
            xuLyHienThongTin();
            break;
        case STATE_MUON_SACH:
            xuLyMuonSach();
            break;
        case STATE_TRA_SACH:
            xuLyTraSach();
            break;
        case STATE_BAO_MAT:
            xuLyBaoMat();
            break;
    }
}

void WidgetMuonTraSach::xuLyNhapMaThe() {
    editMaThe->clear();
    editMaThe->setFocus();
    m_focusGuard->setTarget(editMaThe); // Guard Focus
    statusLabel->clear();
    docGiaHienTai = nullptr;
    qDebug() << "State: NHAP_MA_THE";
}

void WidgetMuonTraSach::xuLyHienThongTin() {
    if (!docGiaHienTai) {
        chuyenSangState(STATE_NHAP_MA_THE);
        return;
    }
    
    capNhatLabelThongTinDocGia();
    capNhatDanhSachSachDangMuon();
    
    // Enable/Disable buttons dựa vào số sách đang mượn
    btnMuonSachMoi->setEnabled(docGiaHienTai->data.soSachDangMuon < 3);
    btnTraSach->setEnabled(cacSachDangMuon.size() > 0);
    btnBaoMat->setEnabled(cacSachDangMuon.size() > 0);
    
    m_focusGuard->setTarget(nullptr); // No mandatory focus here (user chooses action)
    qDebug() << "State: HIEN_THONG_TIN";
}

void WidgetMuonTraSach::xuLyMuonSach() {
    editTimSach->clear();
    editTimSach->setFocus();
    comboBanSao->clear();
    lblKetQuaTimSach->clear();
    lblViTri->clear();
    btnXacNhanMuon->setEnabled(false);
    btnXacNhanMuon->setEnabled(false);
    sachTimThay = nullptr;
    m_focusGuard->setTarget(editTimSach); // Guard Focus for continuous scanning
    qDebug() << "State: MUON_SACH";
}

void WidgetMuonTraSach::xuLyTraSach() {
    modelTraSach->setLoans(cacSachDangMuon);
    lblChonSachTra->setText(QString("Chọn sách muốn trả (Tổng: %1 sách):").arg(cacSachDangMuon.size()));
    tableTraSach->selectRow(0);
    tableTraSach->selectRow(0);
    // TODO: Add barcode field for return status? For now standard
    editScanTraSach->clear();
    editScanTraSach->setFocus();
    m_focusGuard->setTarget(editScanTraSach); 
    qDebug() << "State: TRA_SACH";
}

void WidgetMuonTraSach::xuLyBaoMat() {
    comboSachBaoMat->clear();
    
    // Populate combo với sách đang mượn
    for (const auto& loan : cacSachDangMuon) {
        QString display = QString("%1 - %2")
            .arg(QString::fromStdString(loan.maSach))
            .arg(QString::fromStdString(loan.tenSach));
        comboSachBaoMat->addItem(display);
    }
    
    qDebug() << "State: BAO_MAT";
}

// ============================================
// SLOT IMPLEMENTATIONS
// ============================================

void WidgetMuonTraSach::onTimDocGiaClicked() {
    QString maTheStr = editMaThe->text().trimmed();
    
    if (maTheStr.isEmpty()) {
        hienThiThongBao("⚠️ Vui lòng nhập mã thẻ học viên!", true);
        return;
    }
    
    bool ok;
    int maThe = maTheStr.toInt(&ok);
    
    if (!ok) {
        hienThiThongBao("⚠️ Mã thẻ phải là số nguyên!", true);
        return;
    }
    
    if (layThongTinDocGia(maThe)) {
        chuyenSangState(STATE_HIEN_THONG_TIN);
        hienThiThongBao("✅ Tìm thấy độc giả!");
    } else {
        hienThiThongBao("❌ Không tìm thấy độc giả với mã thẻ: " + maTheStr, true);
    }
}

// Helper algorithm to find reader who has the book
PTRDG WidgetMuonTraSach::findReaderByBook(const std::string& maSach) {
    if (!rootDocGia) return nullptr;
    
    PTRDG result = nullptr;
    // Recursive traversal
    std::function<void(PTRDG)> traverse = [&](PTRDG p) {
        if (!p || result) return; // Stop if found
        
        // Check this reader's loans
        MUONTRA mt = p->data.dsmt;
        while (mt) {
            // Check active loans only (TrangThai == 0: Dang Muon) or Lost (2)
            if (mt->data.TrangThai == 0) {
                 // Check maSach (need to check BanSao)
                 if (mt->data.banSaoSach && mt->data.banSaoSach->maSach.length() > 0) {
                     QString currentMa = QString::fromStdString(mt->data.banSaoSach->maSach);
                     if (currentMa.compare(QString::fromStdString(maSach), Qt::CaseInsensitive) == 0) {
                         result = p;
                         return;
                     }
                 }
            }
            mt = mt->next;
        }
        
        traverse(p->left);
        traverse(p->right);
    };
    
    traverse(rootDocGia);
    return result;
}

void WidgetMuonTraSach::onMaTheEnterPressed() {
    QString input = editMaThe->text().trimmed();
    if (input.isEmpty()) return;
    
    // Unified scanning logic
    handleGlobalScan(input);
}

void WidgetMuonTraSach::onResetWorkflow() {
    resetWorkflow();
}

void WidgetMuonTraSach::onMuonSachMoiClicked() {
    if (!docGiaHienTai) return;
    
    // Check giới hạn 3 sách
    if (docGiaHienTai->data.soSachDangMuon >= 3) {
        QMessageBox::warning(this, "Giới Hạn Mượn Sách",
            "⚠️ Học viên đã mượn đủ 3 sách.\nVui lòng trả sách trước khi mượn tiếp!");
        return;
    }
    
    // Check trạng thái thẻ
    if (docGiaHienTai->data.TrangThai == 0) {
        QMessageBox::critical(this, "Thẻ Đã Khóa",
            "🔒 Thẻ độc giả đã bị khóa.\nKhông thể mượn sách!");
        return;
    }
    
    chuyenSangState(STATE_MUON_SACH);
}

void WidgetMuonTraSach::onTraSachClicked() {
    if (!docGiaHienTai || cacSachDangMuon.isEmpty()) {
        QMessageBox::warning(this, "Không Có Sách",
            "⚠️ Học viên không có sách nào đang mượn!");
        return;
    }
    
    chuyenSangState(STATE_TRA_SACH);
}

void WidgetMuonTraSach::onBaoMatClicked() {
    if (!docGiaHienTai || cacSachDangMuon.isEmpty()) {
        QMessageBox::warning(this, "Không Có Sách",
            "⚠️ Học viên không có sách nào đang mượn!");
        return;
    }
    
    chuyenSangState(STATE_BAO_MAT);
}

void WidgetMuonTraSach::onTimSachMuonClicked() {
    QString tuKhoa = editTimSach->text().trimmed();

    if (tuKhoa.isEmpty()) {
        lblKetQuaTimSach->setText("⚠️ Vui lòng nhập từ khóa tìm kiếm!");
        lblKetQuaTimSach->setStyleSheet("color: #DC2626;");
        return;
    }
    
    // Check Special Command "HOAN_TAT" or "F12"
    if (tuKhoa.toUpper() == "HOAN_TAT" || tuKhoa.toUpper() == "CMD_HOAN_TAT") {
        // Trigger Finish Workflow (Print Receipt)
        // Assume finishing borrow session
        // Print Receipt (Silent)
        if (docGiaHienTai && !cacSachDangMuon.isEmpty()) {
             // In tat ca sach vua muon? 
             // Logic: In phieu muon cho cac sach co ngay muon = hom nay
             // For simplicity, print current list
             QList<QString> bookList;
             for (const auto& s : cacSachDangMuon) bookList.append(QString::fromStdString(s.tenSach));
             
             // In Phieu Tong Hop
             // Loop print for now (supporting Receipt Printer)
             int count = 0;
             for (const auto& s : cacSachDangMuon) {
                 // Check if borrowed today? For now print all in session list
                 if (s.ngayMuon == NgayThang::LayNgayHienTai()) {
                     PrintHelper::inPhieuMuonTuDong(docGiaHienTai, QString::fromStdString(s.maSach), QString::fromStdString(s.tenSach), QString::fromStdString(s.ngayMuon), this);
                     count++;
                 }
             }
             
             if (count > 0) {
                 hienThiThongBao(QString("🖨️ Đã gửi lệnh in %1 phiếu mượn...").arg(count), false);
             } else {
                 hienThiThongBao("⚠️ Không có sách nào mượn hôm nay để in.", true);
             }
             
             // Reset to State 1
             capNhatDanhSachSachDangMuon(); // refresh
             chuyenSangState(STATE_HIEN_THONG_TIN);
        }
        return;
    }
    
    // Set auto confirm flag (since Enter was pressed)
    m_autoConfirm = true;
    
    // Run book search asynchronously to avoid blocking UI
    try {
        btnTimSach->setEnabled(false);
        lblKetQuaTimSach->setText("⏳ Đang tìm sách...");
        QFuture<std::vector<KetQuaTimKiem>> future = QtConcurrent::run(&DBBatcher::timKiemDauSach, tuKhoa.toStdString(), 100);
        bookSearchWatcher->setFuture(future);
    } catch (...) {
        lblKetQuaTimSach->setText("❌ Lỗi khi tìm kiếm sách!");
        lblKetQuaTimSach->setStyleSheet("color: #DC2626;");
        btnTimSach->setEnabled(true);
    }
}

void WidgetMuonTraSach::onChonBanSaoChanged(int index) {
    if (index < 0 || !sachTimThay) {
        lblViTri->clear();
        btnXacNhanMuon->setEnabled(false);
        return;
    }
    
    // Lấy vị trí từ bản sao được chọn
    QString maSach = comboBanSao->currentData().toString();
    
    PTRDMS banSao = TimBanSaoTheoMa(sachTimThay, maSach.toStdString());
    if (banSao) {
        lblViTri->setText(QString::fromStdString(banSao->viTri));
        btnXacNhanMuon->setEnabled(true);
    }
}

void WidgetMuonTraSach::onXacNhanMuonClicked() {
    // Validate borrow request
    QString errorMsg = validateBorrowRequest();
    if (!errorMsg.isEmpty()) {
        hienThiThongBao(errorMsg, true);
        AnimationHelper::shake(this);
        return;
    }
    
    QString codeToBorrow = QString::fromStdString(sachTimThay->ISBN);
    
    // Prefer specific copy if selected
    if (comboBanSao->currentIndex() >= 0) {
        codeToBorrow = comboBanSao->currentData().toString();
    }
    
    // ✅ Gọi backend function MuonSach()
    std::string result = MuonSach(docGiaHienTai, codeToBorrow.toStdString(), dsDauSach, soLuongDauSach);
    
    if (result.empty()) {
        // Refresh 
        try { emit duLieuThayDoi(); } catch(...) {}
        capNhatDanhSachSachDangMuon();
        
        // AUTO PRINT RECEIPT
        PrintHelper::inPhieuMuonTuDong(docGiaHienTai, codeToBorrow, QString::fromStdString(sachTimThay->tenSach), NgayThang::LayNgayHienTai(), this);
        
        // NOTIFY & RESET (CONTINUOUS FLOW)
        hienThiThongBao("✓ Đã mượn: " + QString::fromStdString(sachTimThay->tenSach) + 
                        QString(" (%1/3)").arg(docGiaHienTai->data.soSachDangMuon), false);
        QApplication::beep();

        // Check limit
        if (docGiaHienTai->data.soSachDangMuon >= 3) {
            QMessageBox::information(this, "Đã Đủ Sách", 
                "Học viên đã mượn đủ 3 sách!\nQuay lại màn hình thông tin.");
            chuyenSangState(STATE_HIEN_THONG_TIN);
            return;
        }

        // RESET UI FOR NEXT SCAN
        editTimSach->clear();
        comboBanSao->clear();
        lblKetQuaTimSach->clear(); // Clear search result info
        lblViTri->clear();
        btnXacNhanMuon->setEnabled(false);
        sachTimThay = nullptr; // Reset current book pointer
        
        // FOCUS BACK TO SEARCH
        editTimSach->setFocus();
        if (m_focusGuard) m_focusGuard->setTarget(editTimSach);
        
    } else {
        hienThiThongBao(QString::fromStdString(result), true);
        QMessageBox::critical(this, "Lỗi ❌", QString::fromStdString(result));
        // On error, we might want to stay to let user retry or cancel
    }
}

void WidgetMuonTraSach::onChonSachTraChanged() {
    // Mark selected row
}

void WidgetMuonTraSach::onXacNhanTraClicked() {
    if (!docGiaHienTai) return;
    
    QModelIndex index = tableTraSach->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Chưa Chọn Sách",
            "⚠️ Vui lòng chọn sách muốn trả!");
        return;
    }
    
    int row = index.row();
    if (row < 0 || row >= cacSachDangMuon.size()) return;
    
    ThongTinSachDangMuon_DTO sachTra = cacSachDangMuon[row];
    
    // ✅ Gọi backend function TraSach()
    std::string result = TraSach(docGiaHienTai, sachTra.maSach, dsDauSach, soLuongDauSach);
    
    if (result.empty()) {
        QMessageBox::information(this, "Thành Công",
            QString("Đã trả sách:\n%1\n\nHọc viên còn: %2 sách đang mượn")
            .arg(QString::fromStdString(sachTra.tenSach))
            .arg(docGiaHienTai->data.soSachDangMuon));
        
        // Auto Print Return Receipt
        PrintHelper::inPhieuTraTuDong(docGiaHienTai, QString::fromStdString(sachTra.maSach), QString::fromStdString(sachTra.tenSach), NgayThang::LayNgayHienTai(), this);
        
        // Refresh và quay lại state 1
        try { emit duLieuThayDoi(); } catch(...) {}
        capNhatDanhSachSachDangMuon();
        chuyenSangState(STATE_HIEN_THONG_TIN);
    } else {
        QMessageBox::critical(this, "Lỗi ❌", QString::fromStdString(result));
    }
}


void WidgetMuonTraSach::onScanTraSach() {
    QString barcode = editScanTraSach->text().trimmed();
    if (barcode.isEmpty()) return;
    
    // Find book in 'cacSachDangMuon'
    bool found = false;
    int foundIndex = -1;
    
    for (int i = 0; i < cacSachDangMuon.size(); ++i) {
        // Unified Case-Insensitive Match
        QString currentMa = QString::fromStdString(cacSachDangMuon[i].maSach);
        if (currentMa.compare(barcode, Qt::CaseInsensitive) == 0) {
            foundIndex = i;
            found = true;
            break;
        }
    }
    
    if (!found) {
        hienThiThongBao("⚠️ Sách không có trong danh sách mượn của độc giả này!", true);
        QApplication::beep(); 
        editScanTraSach->selectAll();
        return;
    }
    
    tableTraSach->selectRow(foundIndex);
    
    ThongTinSachDangMuon_DTO sachTra = cacSachDangMuon[foundIndex];
    std::string result = TraSach(docGiaHienTai, sachTra.maSach, dsDauSach, soLuongDauSach);
    
    if (result.empty()) {
        hienThiThongBao(QString("✓ Đã trả: %1").arg(QString::fromStdString(sachTra.tenSach)), false);
        QApplication::beep();
        
        PrintHelper::inPhieuTraTuDong(docGiaHienTai, QString::fromStdString(sachTra.maSach), QString::fromStdString(sachTra.tenSach), NgayThang::LayNgayHienTai(), this);
        
        try { emit duLieuThayDoi(); } catch(...) {}
        capNhatDanhSachSachDangMuon();
        
        modelTraSach->setLoans(cacSachDangMuon);
        lblChonSachTra->setText(QString("Còn %1 sách đang mượn:").arg(cacSachDangMuon.size()));
        
        editScanTraSach->clear();
        editScanTraSach->setFocus();
        
        if (cacSachDangMuon.isEmpty()) {
            QMessageBox::information(this, "Hoàn Tất", "Học viên đã trả hết sách!");
            chuyenSangState(STATE_HIEN_THONG_TIN);
        }
        
    } else {
        hienThiThongBao("❌ Lỗi trả sách: " + QString::fromStdString(result), true);
        QApplication::beep();
    }
}

void WidgetMuonTraSach::onXacNhanBaoMatClicked() {
    if (!docGiaHienTai || cacSachDangMuon.isEmpty()) return;
    
    int index = comboSachBaoMat->currentIndex();
    if (index < 0) return;
    
    ThongTinSachDangMuon_DTO sachMat = cacSachDangMuon[index];
    
    QMessageBox::StandardButton reply = QMessageBox::warning(
        this,
        "Xác Nhận Báo Mất",
        QString("⚠️ Bạn có chắc chắn muốn báo mất sách:\n\n"
                "%1\n\n"
                "Học viên sẽ phải bồi thường cho sách này!")
        .arg(QString::fromStdString(sachMat.tenSach)),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply != QMessageBox::Yes) return;
    
    // ✅ Gọi backend function BaoMatSach()
    std::string result = BaoMatSach(docGiaHienTai, sachMat.maSach, dsDauSach, soLuongDauSach);
    
    if (result.empty()) {
        QMessageBox::information(this, "Đã Báo Mất ✅",
            QString("Đã cập nhật trạng thái sách:\n%1\n\n"
                    "Trạng thái: Đã mất (cần bồi thường)")
            .arg(QString::fromStdString(sachMat.tenSach)));
        
        // Refresh và quay lại state 1
        try { emit duLieuThayDoi(); } catch(...) {}
        capNhatDanhSachSachDangMuon();
        chuyenSangState(STATE_HIEN_THONG_TIN);
    } else {
        QMessageBox::critical(this, "Lỗi ❌", QString::fromStdString(result));
    }
}

void WidgetMuonTraSach::onTableDoubleClicked(const QModelIndex &index) {
    // Double click để xem chi tiết sách
    if (!index.isValid()) return;
    
    int row = index.row();
    if (row < 0 || row >= cacSachDangMuon.size()) return;
    
    ThongTinSachDangMuon_DTO loan = cacSachDangMuon[row];
    
    QString info = QString(
        "📖 CHI TIẾT SÁCH ĐANG MƯỢN\n\n"
        "Mã sách: %1\n"
        "Tên sách: %2\n"
        "Ngày mượn: %3\n"
        "Số ngày giữ: %4 ngày\n"
    ).arg(QString::fromStdString(loan.maSach))
     .arg(QString::fromStdString(loan.tenSach))
     .arg(QString::fromStdString(loan.ngayMuon))
     .arg(loan.soNgayGiu);
    
    if (loan.soNgayGiu > 7) {
        info += QString("\n⚠️ QUÁ HẠN: %1 ngày").arg(loan.soNgayGiu - 7);
    }
    
    QMessageBox::information(this, "Chi Tiết Sách", info);
}

// ============================================
// DATA LOADERS (BACKEND INTEGRATION)
// ============================================

bool WidgetMuonTraSach::layThongTinDocGia(int maThe) {
    // ✅ Gọi backend function timDocGia() - O(log n) AVL search
    docGiaHienTai = timDocGia(rootDocGia, maThe);
    
    if (!docGiaHienTai) {
        return false;
    }
    
    // Check trạng thái thẻ
    if (docGiaHienTai->data.TrangThai == 0) {
        hienThiThongBao("⚠️ Thẻ độc giả đã bị khóa!", true);
    }
    
    return true;
}

void WidgetMuonTraSach::capNhatDanhSachSachDangMuon() {
    if (!docGiaHienTai) {
        cacSachDangMuon.clear();
        modelSachDangMuon->clear();
        return;
    }

    // ✅ Gọi backend function LayDSSachDangMuon() với DTO
    ThongTinSachDangMuon_DTO danhSach[100];
    int soLuong = LayDSSachDangMuon(docGiaHienTai, danhSach, 100, dsDauSach, soLuongDauSach);

    // Defensive: ensure soLuong is in a safe range
    if (soLuong < 0) soLuong = 0;
    if (soLuong > 100) soLuong = 100;

    cacSachDangMuon.clear();
    for (int i = 0; i < soLuong; i++) {
        cacSachDangMuon.append(danhSach[i]);
    }

    modelSachDangMuon->setLoans(cacSachDangMuon);
}

void WidgetMuonTraSach::timSachCoTheMuon(const QString& tuKhoa) {
    // This function remains for synchronous usage but is now rarely called
    // Use DBBatcher::timKiemDauSach asynchronously; synchronous fallback below.
    std::vector<KetQuaTimKiem> results = DBBatcher::timKiemDauSach(tuKhoa.toStdString(), 100);

    if (results.empty()) {
        lblKetQuaTimSach->setText("❌ Không tìm thấy sách nào phù hợp!");
        lblKetQuaTimSach->setStyleSheet("color: #DC2626;");
        comboBanSao->clear();
        sachTimThay = nullptr;
        return;
    }

    sachTimThay = results[0].sach;
    if (!sachTimThay) {
        lblKetQuaTimSach->setText("❌ Dữ liệu sách không hợp lệ!");
        lblKetQuaTimSach->setStyleSheet("color: #DC2626;");
        comboBanSao->clear();
        return;
    }

    // Synchronous fallback: compute available copies and load linked-list
    int soLuongSan = DemBanSaoCoTheMuon(sachTimThay);
    lblKetQuaTimSach->setText(QString("✅ Tìm thấy: <b>%1</b><br>"
        "Số bản sao sẵn: <b>%2</b>").arg(QString::fromStdString(sachTimThay->tenSach)).arg(soLuongSan));
    lblKetQuaTimSach->setStyleSheet("color: #10B981;");
    if (soLuongSan == 0) {
}

// ============================================
// UI HELPERS
// ============================================

void WidgetMuonTraSach::capNhatLabelThongTinDocGia() {
    if (!docGiaHienTai) return;
    
    QString gioiTinh = docGiaHienTai->data.Phai ? "Nữ" : "Nam";
    QString trangThai = docGiaHienTai->data.TrangThai == 1 ? "✓ Hoạt động" : "🔒 Đã khóa";
    
    lblThongTinDocGia->setText(QString(
        "Mã thẻ: <b>%1</b><br>"
        "Họ tên: <b>%2 %3</b><br>"
        "Giới tính: %4<br>"
        "Trạng thái: %5"
    ).arg(docGiaHienTai->data.MaThe)
     .arg(QString::fromStdString(docGiaHienTai->data.Ho))
     .arg(QString::fromStdString(docGiaHienTai->data.Ten))
     .arg(gioiTinh)
     .arg(trangThai));
    
    lblSoSachDangMuon->setText(QString(
        "📚 Đang mượn: <b>%1 / 3</b> sách"
    ).arg(docGiaHienTai->data.soSachDangMuon));
    
    // Color coding
    if (docGiaHienTai->data.soSachDangMuon >= 3) {
        lblSoSachDangMuon->setStyleSheet("color: #DC2626;"); // Red
    } else if (docGiaHienTai->data.soSachDangMuon >= 2) {
        lblSoSachDangMuon->setStyleSheet("color: #F59E0B;"); // Warning
    } else {
        lblSoSachDangMuon->setStyleSheet("color: #10B981;"); // Green
    }
}

void WidgetMuonTraSach::onBookSearchFinished() {
    btnTimSach->setEnabled(true);
    if (!bookSearchWatcher) return;
    std::vector<KetQuaTimKiem> results = bookSearchWatcher->future().result();
    if (results.empty()) {
        lblKetQuaTimSach->setText("❌ Không tìm thấy sách nào phù hợp!");
        lblKetQuaTimSach->setStyleSheet("color: #DC2626;");
        comboBanSao->clear();
        sachTimThay = nullptr;
        return;
    }

    sachTimThay = results[0].sach;
    if (!sachTimThay) {
        lblKetQuaTimSach->setText("❌ Dữ liệu sách không hợp lệ!");
        lblKetQuaTimSach->setStyleSheet("color: #DC2626;");
        comboBanSao->clear();
        return;
    }

    // Start loading available copies asynchronously
    lblKetQuaTimSach->setText(QString("✅ Tìm thấy: <b>%1</b><br>Đang tải bản sao...").arg(QString::fromStdString(sachTimThay->tenSach)));
    lblKetQuaTimSach->setStyleSheet("color: #10B981;");
    try {
        QFuture<std::vector<PTRDMS>> future = QtConcurrent::run(&DBBatcher::layBanSaoTheoISBN, std::string(sachTimThay->ISBN));
        banSaoWatcher->setFuture(future);
    } catch (...) {
        // fallback to sync
        loadBanSaoKhaDung(sachTimThay);
    }
}

void WidgetMuonTraSach::onBanSaoFinished() {
    btnTimSach->setEnabled(true);
    if (!banSaoWatcher) return;
    
    // SAFETY GUARD: If user switched state (e.g. Global Return), drop this result
    if (currentState != STATE_MUON_SACH) {
        m_autoConfirm = false;
        return;
    }
    
    std::vector<PTRDMS> copies = banSaoWatcher->future().result();
    comboBanSao->clear();
    int count = 0;
    for (auto &p : copies) {
        if (p && p->trangThai == CHO_MUON_DUOC) {
            QString display = QString("%1 (%2)")
                .arg(QString::fromStdString(p->maSach))
                .arg(QString::fromStdString(p->viTri));
            comboBanSao->addItem(display, QString::fromStdString(p->maSach));
            count++;
        }
    }
    if (count > 0) {
        // Auto-select the scanned book if matches specific copy code
        int selectedIndex = 0;
        QString scanCode = editTimSach->text().trimmed();
        for(int i=0; i<comboBanSao->count(); i++) {
            if (comboBanSao->itemData(i).toString().compare(scanCode, Qt::CaseInsensitive) == 0) {
                selectedIndex = i;
                break;
            }
        }
        comboBanSao->setCurrentIndex(selectedIndex);
        onChonBanSaoChanged(selectedIndex);
        
        // ZERO CLICK LOGIC
        if (m_autoConfirm) {
            // Check warnings before auto-confirming?
            // validateBorrowRequest handles it.
            hienThiThongBao("⚡ Auto-Submit: Đang mượn sách...", false);
            // Small delay or direct call? Direct call is faster.
            onXacNhanMuonClicked();
            
            // Refocus for next scan (onXacNhanMuonClicked might change state, we need to ensure we stay or handle it)
            // But checking output: onXacNhanMuonClicked calls chuyenSangState(STATE_HIEN_THONG_TIN) on success.
            // Requirement: "Quét Sách (Liên tục)". So we should STAY in state STATE_MUON_SACH.
            // I need to modify onXacNhanMuonClicked to NOT change state if Auto Mode?
            // Or just change state back?
            
            // Let's modify onXacNhanMuonClicked to be smarter (see next refactor)
        }
        
    } else {
        lblKetQuaTimSach->setText(lblKetQuaTimSach->text() + "<br>⚠️ Không có bản sao nào sẵn sàng!");
        btnXacNhanMuon->setEnabled(false);
    }
    m_autoConfirm = false; // Reset flag
}

void WidgetMuonTraSach::hienThiThongBao(const QString &message, bool isError) {
    statusLabel->setText(message);
    
    if (isError) {
        statusLabel->setStyleSheet("color: #DC2626; font-weight: bold;");
    } else {
        statusLabel->setStyleSheet("color: #10B981; font-weight: bold;");
    }
}

void WidgetMuonTraSach::vohieuHoaButtons(bool disabled) {
    btnMuonSachMoi->setDisabled(disabled);
    btnTraSach->setDisabled(disabled);
    btnBaoMat->setDisabled(disabled);
}

void WidgetMuonTraSach::refreshData() {
    if (docGiaHienTai) {
        capNhatDanhSachSachDangMuon();
        capNhatLabelThongTinDocGia();
    }
}

void WidgetMuonTraSach::resetWorkflow() {
    docGiaHienTai = nullptr;
    sachTimThay = nullptr;
    cacSachDangMuon.clear();
    modelSachDangMuon->clear();
    chuyenSangState(STATE_NHAP_MA_THE);
}

void WidgetMuonTraSach::handleGlobalScan(const QString& barcode) {
    QString code = barcode.trimmed();
    if (code.isEmpty()) return;
    
    qDebug() << "Global Scan Received:" << code;
    
    // 1. Try as Reader ID (Numeric)
    bool isNumeric;
    int maThe = code.toInt(&isNumeric);
    if (isNumeric) {
        // Assume it's a reader card
        if (layThongTinDocGia(maThe)) {
            chuyenSangState(STATE_HIEN_THONG_TIN);
            hienThiThongBao("✅ Đã nhận diện độc giả: " + QString::fromStdString(docGiaHienTai->data.Ho + " " + docGiaHienTai->data.Ten));
            QApplication::beep();
            return;
        }
    }
    
    // 2. Try as Book Code
    
    // Scenario A: Reader is already login
    if (docGiaHienTai) {
        // Check if book is currently borrowed by this reader (for Return)
        bool isBorrowed = false;
        for (const auto& loan : cacSachDangMuon) {
            // Case-Insensitive Check
            if (QString::fromStdString(loan.maSach).compare(code, Qt::CaseInsensitive) == 0) {
                isBorrowed = true;
                break;
            }
        }
        
        if (isBorrowed) {
            // Auto Trigger Return
            if (currentState != STATE_TRA_SACH) {
                chuyenSangState(STATE_TRA_SACH);
            }
            editScanTraSach->setText(code);
            onScanTraSach(); // Trigger Logic
            return;
        } else {
            // Auto Trigger Borrow
            if (currentState != STATE_MUON_SACH) {
                chuyenSangState(STATE_MUON_SACH);
            }
            editTimSach->setText(code); // Put code into search
            // Force auto confirm flow
            m_autoConfirm = true;
            onTimSachMuonClicked(); // Trigger Search
            return;
        }
    } 
    
    // Scenario B: No reader login
    // Check if book is borrowed by SOMEONE ELSE (Global Return)
    PTRDG borrower = findReaderByBook(code.toStdString());
    if (borrower) {
        // Login as that borrower and go to Return
        docGiaHienTai = borrower;
        chuyenSangState(STATE_HIEN_THONG_TIN);
        chuyenSangState(STATE_TRA_SACH);
        
        hienThiThongBao("⚡ Phát hiện sách của: " + QString::fromStdString(borrower->data.Ho + " " + borrower->data.Ten), false);
        
        // Trigger Return
        editScanTraSach->setText(code);
        onScanTraSach();
        return;
    }
    
    // Scenario C: Book is free / Unknown / Error
    std::stringstream ss; 
    // Check if it is a valid book code?
    PTRDMS validBook = timDanhMucTheoMaSach(code.toStdString(), dsDauSach, soLuongDauSach, ss, true);
    
    if (validBook) {
        hienThiThongBao("📖 Sách có trong thư viện. Quét thẻ độc giả để mượn!", false);
        // Maybe visual cue?
    } else {
        hienThiThongBao("⚠️ Mã vạch không hợp lệ hoặc không có trong hệ thống!", true);
    }
    QApplication::beep();
}

// ============================================
// VALIDATION & FILTER METHODS
// ============================================

bool WidgetMuonTraSach::canReaderBorrowMore() const {
    if (!docGiaHienTai) return false;
    return docGiaHienTai->data.soSachDangMuon < 3;
}

bool WidgetMuonTraSach::isBookAvailable(const QString& isbn) const {
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    
    // Find book by ISBN
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] && QString::fromStdString(dsDauSach[i]->ISBN) == isbn) {
            // Check if has available copies
            PTRDMS p = dsDauSach[i]->dms;
            while (p) {
                if (p->trangThai == CHO_MUON_DUOC) {
                    return true;
                }
                p = p->next;
            }
            return false;
        }
    }
    return false;
}

QString WidgetMuonTraSach::validateBorrowRequest() const {
    // Check reader exists
    if (!docGiaHienTai) {
        return "⚠ Vui lòng chọn độc giả";
    }
    
    // Check reader is active
    if (docGiaHienTai->data.TrangThai != 1) {
        return "⚠ Học viên đã bị khóa, không thể mượn sách";
    }
    
    // Check borrowing limit
    if (!canReaderBorrowMore()) {
        return QString("⚠ Học viên đã mượn tối đa %1/3 sách")
               .arg(docGiaHienTai->data.soSachDangMuon);
    }
    
    // Check if has overdue books
    for (const auto& loan : cacSachDangMuon) {
        if (loan.soNgayGiu > 7) {
            return "⚠ Học viên có sách quá hạn, cần trả sách trước khi mượn thêm";
        }
    }
    
    // Check book selected
    if (!sachTimThay) {
        return "⚠ Vui lòng chọn sách muốn mượn";
    }
    
    // Check book availability
    if (comboBanSao->count() == 0) {
        return "⚠ Không có bản sao nào sẵn sàng cho mượn";
    }
    
    return ""; // Valid
}

void WidgetMuonTraSach::applyLoanFilters() {
    if (currentState != STATE_TRA_SACH || !filterStatusCombo) return;
    
    QString filter = filterStatusCombo->currentText();
    
    // Build filtered list
    QVector<ThongTinSachDangMuon_DTO> filtered;
    for (const auto& loan : cacSachDangMuon) {
        if (filter == "Tất cả") {
            filtered.append(loan);
        } else if (filter == "Quá hạn" && loan.soNgayGiu > 7) {
            filtered.append(loan);
        } else if (filter == "Bình thường" && loan.soNgayGiu <= 7) {
            filtered.append(loan);
        }
    }
    
    modelTraSach->setLoans(filtered);
    
void WidgetMuonTraSach::capNhatNgonNgu() {
    try {
        if (btnXacNhanMuon) btnXacNhanMuon->setText(" " + LanguageManager::instance().getText("BTN_BORROW_SHORT"));
        if (btnXacNhanTra) btnXacNhanTra->setText(" " + LanguageManager::instance().getText("BTN_RETURN_SHORT"));
        if (btnBaoMat) btnBaoMat->setText(" " + LanguageManager::instance().getText("BTN_LOST_SHORT"));
        
        // Update Table Headers safely
        if (tableSachDangMuon) {
            QAbstractItemModel* model = tableSachDangMuon->model();
            if (model) {
                model->setHeaderData(0, Qt::Horizontal, LanguageManager::instance().getText("TABLE_HEADER_BOOK_ID"));
                model->setHeaderData(1, Qt::Horizontal, LanguageManager::instance().getText("TABLE_HEADER_TITLE"));
                model->setHeaderData(2, Qt::Horizontal, LanguageManager::instance().getText("TABLE_HEADER_DATE_BORROW"));
            }
        }
        
        if (tableTraSach) {
             QAbstractItemModel* model = tableTraSach->model();
             if (model) {
                 model->setHeaderData(0, Qt::Horizontal, LanguageManager::instance().getText("TABLE_HEADER_BOOK_ID"));
                 model->setHeaderData(1, Qt::Horizontal, LanguageManager::instance().getText("TABLE_HEADER_TITLE"));
                 model->setHeaderData(2, Qt::Horizontal, LanguageManager::instance().getText("TABLE_HEADER_DATE_BORROW"));
                 model->setHeaderData(3, Qt::Horizontal, LanguageManager::instance().getText("TABLE_HEADER_STATUS"));
             }
        }
    } catch (...) {
        qWarning() << "WidgetMuonTraSach::capNhatNgonNgu exception";
    }
}

void WidgetMuonTraSach::performBookSearch() {
    QString keyword = editTimSach->text().trimmed();
    if (keyword.isEmpty()) {
        lblKetQuaTimSach->clear();
        comboBanSao->clear();
        lblViTri->clear();
        btnXacNhanMuon->setEnabled(false);
        return;
    }
    
    // Perform search
    onTimSachMuonClicked();
}
