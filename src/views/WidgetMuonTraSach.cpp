#include "views/WidgetMuonTraSach.h"
#include "models/DocGia.h"
#include "models/QuanLySach.h"
#include "utils/NgayThang.h"
#include "utils/AnimationHelper.h"
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
#include <QShortcut>

extern PTRDG rootDocGia;
extern PTRDS dsDauSach[MAX_DAUSACH];
extern int soLuongDauSach;

// Set modern stylesheet for loan widget
static const QString LOAN_WIDGET_STYLE = R"(
    WidgetMuonTraSach {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0F172A, stop:1 #1E293B);
    }
)";

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
    
    setupUI();
    setupConnections();
    setupKeyboardShortcuts();
    chuyenSangState(STATE_NHAP_MA_THE);
}

WidgetMuonTraSach::~WidgetMuonTraSach() {
}

void WidgetMuonTraSach::setupUI() {
    // Apply modern gradient stylesheet
    setStyleSheet(LOAN_WIDGET_STYLE);
    
    // Main scroll area cho responsive
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("QScrollArea { background-color: transparent; border: none; }");
    
    QWidget *scrollContent = new QWidget();
    scrollContent->setStyleSheet("background-color: transparent;");
    QVBoxLayout *mainLayout = new QVBoxLayout(scrollContent);
    mainLayout->setSpacing(24);
    mainLayout->setContentsMargins(32, 32, 32, 32);
    
    // Modern title with gradient effect
    QLabel *title = new QLabel("📖  MƯỢN - TRẢ SÁCH", scrollContent);
    title->setProperty("class", "title");
    QFont titleFont = title->font();
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    titleFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.5);
    title->setFont(titleFont);
    title->setStyleSheet("color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #60A5FA, stop:1 #F59E0B); padding: 8px 0; background: transparent;");
    mainLayout->addWidget(title);
    
    // Stacked widget cho state machine
    stackedWidget = new QStackedWidget(scrollContent);
    
    // Setup 5 states
    setupStateNhapMaThe();      // State 0
    setupStateThongTinDocGia(); // State 1
    setupStateMuonSach();       // State 2
    setupStateTraSach();        // State 3
    setupStateBaoMat();         // State 4
    
    mainLayout->addWidget(stackedWidget, 1);
    
    // Modern status bar with gradient
    statusLabel = new QLabel(scrollContent);
    statusLabel->setProperty("class", "caption");
    statusLabel->setWordWrap(true);
    statusLabel->setStyleSheet(
        "color: #F1F5F9;"
        "padding: 14px 20px;"
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #334155, stop:1 #475569);"
        "border: 2px solid #475569;"
        "border-radius: 10px;"
        "font-size: 14px;"
        "font-weight: 500;"
    );
    mainLayout->addWidget(statusLabel);
    
    scrollArea->setWidget(scrollContent);
    
    QVBoxLayout *containerLayout = new QVBoxLayout(this);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->addWidget(scrollArea);
}

// ============================================
// SETUP STATES
// ============================================

void WidgetMuonTraSach::setupStateNhapMaThe() {
    widgetNhapMaThe = new QWidget(this);
    widgetNhapMaThe->setStyleSheet("background: transparent;");
    QVBoxLayout *layout = new QVBoxLayout(widgetNhapMaThe);
    layout->setSpacing(24);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(20, 40, 20, 40);
    
    // Icon + Hướng dẫn với màu soft
    lblHuongDan = new QLabel("◉ Nhập mã thẻ độc giả để bắt đầu", widgetNhapMaThe);
    QFont font = lblHuongDan->font();
    font.setPointSize(16);
    font.setWeight(QFont::Medium);
    lblHuongDan->setFont(font);
    lblHuongDan->setAlignment(Qt::AlignCenter);
    lblHuongDan->setStyleSheet("color: #475569; padding: 12px; background: transparent;");
    layout->addWidget(lblHuongDan);
    
    // Input group với card container
    QWidget *cardContainer = new QWidget(widgetNhapMaThe);
    cardContainer->setStyleSheet(
        "background-color: white;"
        "border: 1px solid #E2E8F0;"
        "border-radius: 12px;"
        "padding: 32px;"
    );
    QVBoxLayout *cardLayout = new QVBoxLayout(cardContainer);
    cardLayout->setSpacing(16);
    
    QLabel *lblMaThe = new QLabel("Mã thẻ độc giả", cardContainer);
    font = lblMaThe->font();
    font.setPointSize(13);
    font.setBold(true);
    lblMaThe->setFont(font);
    lblMaThe->setStyleSheet("color: #2D3748;");
    cardLayout->addWidget(lblMaThe);
    
    editMaThe = new QLineEdit(cardContainer);
    editMaThe->setPlaceholderText("Nhập mã thẻ...");
    editMaThe->setMinimumHeight(42);
    editMaThe->setMaximumWidth(400);
    font = editMaThe->font();
    font.setPointSize(13);
    editMaThe->setFont(font);
    cardLayout->addWidget(editMaThe);
    
    btnTimDocGia = new QPushButton("▸  Tìm Độc Giả", cardContainer);
    btnTimDocGia->setProperty("class", "btn-primary");
    btnTimDocGia->setMinimumHeight(44);
    btnTimDocGia->setMaximumWidth(400);
    font = btnTimDocGia->font();
    font.setPointSize(13);
    font.setBold(true);
    btnTimDocGia->setFont(font);
    cardLayout->addWidget(btnTimDocGia);
    
    layout->addWidget(cardContainer);
    layout->addStretch();
    
    stackedWidget->addWidget(widgetNhapMaThe); // Index 0
}

void WidgetMuonTraSach::setupStateThongTinDocGia() {
    widgetThongTinDocGia = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widgetThongTinDocGia);
    layout->setSpacing(20);
    
    // Thông tin độc giả card
    QGroupBox *groupInfo = new QGroupBox("◉ THÔNG TIN ĐỘC GIẢ", widgetThongTinDocGia);
    QVBoxLayout *infoLayout = new QVBoxLayout(groupInfo);
    
    lblThongTinDocGia = new QLabel(groupInfo);
    QFont font = lblThongTinDocGia->font();
    font.setPointSize(12);
    lblThongTinDocGia->setFont(font);
    lblThongTinDocGia->setStyleSheet("color: #2D3748; padding: 8px;");
    infoLayout->addWidget(lblThongTinDocGia);
    
    lblSoSachDangMuon = new QLabel(groupInfo);
    font.setBold(true);
    lblSoSachDangMuon->setFont(font);
    lblSoSachDangMuon->setStyleSheet("color: #5B7FFF; padding: 4px;");
    infoLayout->addWidget(lblSoSachDangMuon);
    
    layout->addWidget(groupInfo);
    
    // Bảng sách đang mượn
    QGroupBox *groupLoans = new QGroupBox("▤ SÁCH ĐANG MƯỢN", widgetThongTinDocGia);
    QVBoxLayout *loansLayout = new QVBoxLayout(groupLoans);
    
    tableSachDangMuon = new QTableView(groupLoans);
    modelSachDangMuon = new LoanTableModel(this);
    tableSachDangMuon->setModel(modelSachDangMuon);
    tableSachDangMuon->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableSachDangMuon->setSelectionMode(QAbstractItemView::SingleSelection);
    tableSachDangMuon->horizontalHeader()->setStretchLastSection(true);
    tableSachDangMuon->setAlternatingRowColors(true);
    tableSachDangMuon->setMinimumHeight(200);
    loansLayout->addWidget(tableSachDangMuon);
    
    layout->addWidget(groupLoans);
    
    // Action buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(15);
    
    btnMuonSachMoi = new QPushButton("+ Mượn Sách Mới", widgetThongTinDocGia);
    btnMuonSachMoi->setProperty("class", "btn-primary");
    btnMuonSachMoi->setMinimumHeight(42);
    btnMuonSachMoi->setToolTip("Mượn sách mới (Ctrl+B)");
    btnLayout->addWidget(btnMuonSachMoi);
    
    btnTraSach = new QPushButton("✓ Trả Sách", widgetThongTinDocGia);
    btnTraSach->setProperty("class", "btn-success");
    btnTraSach->setMinimumHeight(42);
    btnTraSach->setToolTip("Trả sách đã mượn (Ctrl+R)");
    btnLayout->addWidget(btnTraSach);
    
    btnBaoMat = new QPushButton("× Báo Mất Sách", widgetThongTinDocGia);
    btnBaoMat->setProperty("class", "btn-danger");
    btnBaoMat->setMinimumHeight(42);
    btnBaoMat->setToolTip("Báo mất sách");
    btnLayout->addWidget(btnBaoMat);
    
    btnQuayLai = new QPushButton("← Quay Lại", widgetThongTinDocGia);
    btnQuayLai->setProperty("class", "btn-secondary");
    btnQuayLai->setMinimumHeight(45);
    btnQuayLai->setToolTip("Quay lại nhập mã thẻ (ESC)");
    btnLayout->addWidget(btnQuayLai);
    
    layout->addLayout(btnLayout);
    
    stackedWidget->addWidget(widgetThongTinDocGia); // Index 1
}

void WidgetMuonTraSach::setupStateMuonSach() {
    widgetMuonSach = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widgetMuonSach);
    layout->setSpacing(20);
    
    QLabel *titleMuon = new QLabel("+ MƯỢN SÁCH MỚI", widgetMuonSach);
    QFont font = titleMuon->font();
    font.setPointSize(15);
    font.setBold(true);
    titleMuon->setFont(font);
    titleMuon->setStyleSheet("color: #2D3748; padding: 8px 0;");
    layout->addWidget(titleMuon);
    
    // Tìm sách
    QGroupBox *groupSearch = new QGroupBox("◈ Tìm Sách", widgetMuonSach);
    QVBoxLayout *searchLayout = new QVBoxLayout(groupSearch);
    
    QHBoxLayout *searchBar = new QHBoxLayout();
    editTimSach = new QLineEdit(groupSearch);
    editTimSach->setPlaceholderText("Nhập ISBN hoặc tên sách...");
    editTimSach->setMinimumHeight(40);
    editTimSach->setToolTip("Tìm sách theo ISBN hoặc tên (Ctrl+F để focus)");
    searchBar->addWidget(editTimSach);
    
    btnTimSach = new QPushButton("◈ Tìm", groupSearch);
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
    QGroupBox *groupSelect = new QGroupBox("▫ Chọn Bản Sao", widgetMuonSach);
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
    btnXacNhanMuon = new QPushButton("✓ Xác Nhận Mượn", widgetMuonSach);
    btnXacNhanMuon->setProperty("class", "btn-success");
    btnXacNhanMuon->setMinimumHeight(44);
    btnXacNhanMuon->setEnabled(false);
    btnLayout->addWidget(btnXacNhanMuon);
    
    btnHuyMuon = new QPushButton("← Hủy", widgetMuonSach);
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
    
    QLabel *titleTra = new QLabel("✓ TRẢ SÁCH", widgetTraSach);
    QFont font = titleTra->font();
    font.setPointSize(15);
    font.setBold(true);
    titleTra->setFont(font);
    titleTra->setStyleSheet("color: #2D3748; padding: 8px 0;");
    layout->addWidget(titleTra);
    
    lblChonSachTra = new QLabel("Chọn sách muốn trả từ danh sách bên dưới:", widgetTraSach);
    lblChonSachTra->setStyleSheet("color: #6B7280; padding: 4px 0;");
    layout->addWidget(lblChonSachTra);
    
    // Filter controls
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(12);
    
    QLabel *filterLabel = new QLabel("🔎 Lọc:", widgetTraSach);
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
    btnXacNhanTra = new QPushButton("✓ Xác Nhận Trả", widgetTraSach);
    btnXacNhanTra->setProperty("class", "btn-success");
    btnXacNhanTra->setMinimumHeight(44);
    btnLayout->addWidget(btnXacNhanTra);
    
    btnHuyTra = new QPushButton("← Hủy", widgetTraSach);
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
    
    QLabel *titleBaoMat = new QLabel("▲ BÁO MẤT SÁCH", widgetBaoMat);
    QFont font = titleBaoMat->font();
    font.setPointSize(15);
    font.setBold(true);
    titleBaoMat->setFont(font);
    titleBaoMat->setStyleSheet("color: #DC2626; padding: 8px 0;");
    layout->addWidget(titleBaoMat);
    
    // Warning
    lblCanhBaoBaoMat = new QLabel(
        "▲ <b>CẢNH BÁO:</b> Khi báo mất sách, độc giả sẽ phải bồi thường.<br>"
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
    btnXacNhanBaoMat = new QPushButton("✓ Xác Nhận Báo Mất", widgetBaoMat);
    btnXacNhanBaoMat->setProperty("class", "btn-danger");
    btnXacNhanBaoMat->setMinimumHeight(44);
    btnLayout->addWidget(btnXacNhanBaoMat);
    
    btnHuyBaoMat = new QPushButton("← Hủy", widgetBaoMat);
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
    
    qDebug() << "State: HIEN_THONG_TIN";
}

void WidgetMuonTraSach::xuLyMuonSach() {
    editTimSach->clear();
    editTimSach->setFocus();
    comboBanSao->clear();
    lblKetQuaTimSach->clear();
    lblViTri->clear();
    btnXacNhanMuon->setEnabled(false);
    sachTimThay = nullptr;
    qDebug() << "State: MUON_SACH";
}

void WidgetMuonTraSach::xuLyTraSach() {
    modelTraSach->setLoans(cacSachDangMuon);
    lblChonSachTra->setText(QString("Chọn sách muốn trả (Tổng: %1 sách):").arg(cacSachDangMuon.size()));
    tableTraSach->selectRow(0);
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
        hienThiThongBao("⚠️ Vui lòng nhập mã thẻ độc giả!", true);
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

void WidgetMuonTraSach::onMaTheEnterPressed() {
    onTimDocGiaClicked();
}

void WidgetMuonTraSach::onResetWorkflow() {
    resetWorkflow();
}

void WidgetMuonTraSach::onMuonSachMoiClicked() {
    if (!docGiaHienTai) return;
    
    // Check giới hạn 3 sách
    if (docGiaHienTai->data.soSachDangMuon >= 3) {
        QMessageBox::warning(this, "Giới Hạn Mượn Sách",
            "⚠️ Độc giả đã mượn đủ 3 sách.\nVui lòng trả sách trước khi mượn tiếp!");
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
            "⚠️ Độc giả không có sách nào đang mượn!");
        return;
    }
    
    chuyenSangState(STATE_TRA_SACH);
}

void WidgetMuonTraSach::onBaoMatClicked() {
    if (!docGiaHienTai || cacSachDangMuon.isEmpty()) {
        QMessageBox::warning(this, "Không Có Sách",
            "⚠️ Độc giả không có sách nào đang mượn!");
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
    
    timSachCoTheMuon(tuKhoa);
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
    
    QString isbn = QString::fromStdString(sachTimThay->ISBN);
    
    // ✅ Gọi backend function MuonSach()
    std::string result = MuonSach(docGiaHienTai, isbn.toStdString(), dsDauSach, soLuongDauSach);
    
    if (result.empty()) {
        QMessageBox::information(this, "Thành Công ✅",
            QString("Đã mượn sách:\n%1\n\nĐộc giả đang mượn: %2/3 sách\n"
                    "Hạn trả: 7 ngày kể từ hôm nay")
            .arg(QString::fromStdString(sachTimThay->tenSach))
            .arg(docGiaHienTai->data.soSachDangMuon));
        
        // Refresh và quay lại state 1
        capNhatDanhSachSachDangMuon();
        hienThiThongBao("✓ Đã mượn sách thành công", false);
        chuyenSangState(STATE_HIEN_THONG_TIN);
    } else {
        hienThiThongBao(QString::fromStdString(result), true);
        QMessageBox::critical(this, "Lỗi ❌", QString::fromStdString(result));
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
        QMessageBox::information(this, "Thành Công ✅",
            QString("Đã trả sách:\n%1\n\nĐộc giả còn: %2 sách đang mượn")
            .arg(QString::fromStdString(sachTra.tenSach))
            .arg(docGiaHienTai->data.soSachDangMuon));
        
        // Refresh và quay lại state 1
        capNhatDanhSachSachDangMuon();
        chuyenSangState(STATE_HIEN_THONG_TIN);
    } else {
        QMessageBox::critical(this, "Lỗi ❌", QString::fromStdString(result));
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
                "Độc giả sẽ phải bồi thường cho sách này!")
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
    
    cacSachDangMuon.clear();
    for (int i = 0; i < soLuong; i++) {
        cacSachDangMuon.append(danhSach[i]);
    }
    
    modelSachDangMuon->setLoans(cacSachDangMuon);
}

void WidgetMuonTraSach::timSachCoTheMuon(const QString& tuKhoa) {
    // ✅ Gọi backend function LayDanhSachTimKiem()
    KetQuaTimKiem ketQua[100];
    int soKetQua = 0;
    
    LayDanhSachTimKiem(tuKhoa.toStdString(), ketQua, soKetQua, 100);
    
    if (soKetQua == 0) {
        lblKetQuaTimSach->setText("❌ Không tìm thấy sách nào phù hợp!");
        lblKetQuaTimSach->setStyleSheet("color: #DC2626;");
        comboBanSao->clear();
        sachTimThay = nullptr;
        return;
    }
    
    // Lấy kết quả đầu tiên (priority cao nhất)
    sachTimThay = ketQua[0].sach;
    
    // ✅ Gọi backend function DemBanSaoCoTheMuon()
    int soLuongSan = DemBanSaoCoTheMuon(sachTimThay);
    
    lblKetQuaTimSach->setText(QString(
        "✅ Tìm thấy: <b>%1</b><br>"
        "Số bản sao sẵn: <b>%2</b>"
    ).arg(QString::fromStdString(sachTimThay->tenSach))
     .arg(soLuongSan));
    
    lblKetQuaTimSach->setStyleSheet("color: #10B981;");
    
    if (soLuongSan == 0) {
        lblKetQuaTimSach->setText(lblKetQuaTimSach->text() + "<br>⚠️ Không có bản sao nào sẵn sàng!");
        comboBanSao->clear();
        btnXacNhanMuon->setEnabled(false);
        return;
    }
    
    // Load bản sao khả dụng
    loadBanSaoKhaDung(sachTimThay);
}

void WidgetMuonTraSach::loadBanSaoKhaDung(PTRDS dauSach) {
    comboBanSao->clear();
    
    if (!dauSach) return;
    
    // ✅ Duyệt linked list DMS để tìm bản sao CHO_MUON_DUOC
    PTRDMS p = dauSach->dms;
    int count = 0;
    
    while (p) {
        if (p->trangThai == CHO_MUON_DUOC) {
            QString display = QString("%1 (%2)")
                .arg(QString::fromStdString(p->maSach))
                .arg(QString::fromStdString(p->viTri));
            
            comboBanSao->addItem(display, QString::fromStdString(p->maSach));
            count++;
        }
        p = p->next;
    }
    
    if (count > 0) {
        comboBanSao->setCurrentIndex(0);
        onChonBanSaoChanged(0);
    }
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
        return "⚠ Độc giả đã bị khóa, không thể mượn sách";
    }
    
    // Check borrowing limit
    if (!canReaderBorrowMore()) {
        return QString("⚠ Độc giả đã mượn tối đa %1/3 sách")
               .arg(docGiaHienTai->data.soSachDangMuon);
    }
    
    // Check if has overdue books
    for (const auto& loan : cacSachDangMuon) {
        if (loan.soNgayGiu > 7) {
            return "⚠ Độc giả có sách quá hạn, cần trả sách trước khi mượn thêm";
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
    
    // Update status
    hienThiThongBao(QString("Hiển thị %1/%2 sách đang mượn")
                    .arg(filtered.size())
                    .arg(cacSachDangMuon.size()), false);
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
