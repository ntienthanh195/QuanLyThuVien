/**
 * @file WidgetQuanLySach.cpp
 * @path src/ui/WidgetQuanLySach.cpp
 * @description Widget quản lý danh mục sách (Phiên bản V2).
 * Hỗ trợ các chức năng: hiển thị danh sách đầu sách, thêm/sửa/xóa sách, tìm kiếm và quản lý bản sao sách.
 * @project Library Management System (QuanLyThuVien)
 */
#include "ui/WidgetQuanLySach.h"
#include "utils/ToastNotification.h"

#include "utils/LanguageManager.h"
#include "utils/FilePathManager.h"
#include "ui/dialogs/HopThoaiThemSach.h"
#include "ui/dialogs/HopThoaiSuaSach.h"
#include "ui/dialogs/HopThoaiThemBanSao.h"
#include "models/QuanLySach.h"
#include "models/ThaoTacFile.h"
#include "utils/ResponsiveHelper.h"
#include "utils/AnimationHelper.h"
#include "utils/ConfirmationDialog.h"
#include "utils/EventBus.h"
#include "utils/SafeGui.h"
// DBBatcher integration is optional for the legacy `src/` build;
// use local fallbacks where DBBatcher header is not available.
// If DBBatcher is present in the include path, include it so callers
// (below) can use `DBBatcher::...`. We guard with __has_include to
// avoid breaking older toolchains that don't provide this header.
#if defined(__has_include)
#  if __has_include("utils/DBBatcher.h")
#    include "utils/DBBatcher.h"
#  endif
#endif
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QHBoxLayout>
#include "utils/PrintHelper.h" // Added include
#include "ui/dialogs/ManHinhPreviewImport.h"
#include "utils/ImportManager.h"
#include "ui/dialogs/ManHinhQuanLyIn.h" 
#include "ui/dialogs/ManHinhKiemKe.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include "utils/EventBus.h"
#include <QGraphicsDropShadowEffect>
#include <QFileDialog>
#include <QProcess>
#include <QDialog>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QFileSystemWatcher>
#include <QProgressDialog>
#include <QTableView>
#include <QAbstractTableModel>
#include <algorithm>

// Lightweight legacy model for the old UI (reads from global arrays)
class LegacyBookModel : public QAbstractTableModel {
public:
    LegacyBookModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {}
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return LaySoLuongDauSach();
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return 9;
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid()) return QVariant();
        PTRDS *arr = LayDanhSachDauSach();
        int n = LaySoLuongDauSach();
        int r = index.row();
        if (r < 0 || r >= n) return QVariant();
        PTRDS d = arr ? arr[r] : nullptr;
        if (!d) return QVariant();
        if (role == Qt::DisplayRole) {
            switch (index.column()) {
                case 0: return QString::number(r + 1);
                case 1: return QString::fromStdString(d->ISBN);
                case 2: return QString::fromStdString(d->tenSach);
                case 3: return QString::fromStdString(d->tacGia);
                case 4: return QString::fromStdString(d->theLoai);
                case 5: // Vi Tri (Phase 2 Requirement)
                    return (d->dms && !d->dms->viTri.empty()) ? QString::fromStdString(d->dms->viTri) : QStringLiteral("—");
                case 6: // Gia Tien (Phase 2 Requirement)
                    return d->giaTien > 0 ? QString::number(d->giaTien) : QStringLiteral("0");
                case 7: {
                    int tong = d->tongBanSao;
                    int daMuon = 0, thanhLy = 0;
                    for (PTRDMS p : d->dms_vector) {
                        if (p->trangThai == DANG_MUON) daMuon++;
                        else if (p->trangThai == THANH_LY) thanhLy++;
                    }
                    int conLai = tong - daMuon - thanhLy;
                    return QString("%1/%2").arg(conLai).arg(tong);
                }
                case 8: return QString("...");
            }
        }
        return QVariant();
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (role != Qt::DisplayRole) return QVariant();
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0: return LanguageManager::instance().getText("TABLE_HEADER_STT");
                case 1: return LanguageManager::instance().getText("TABLE_HEADER_ISBN");
                case 2: return LanguageManager::instance().getText("TABLE_HEADER_TITLE");
                case 3: return LanguageManager::instance().getText("TABLE_HEADER_AUTHOR");
                case 4: return LanguageManager::instance().getText("TABLE_HEADER_CATEGORY");
                case 5: return LanguageManager::instance().getText("TABLE_HEADER_LOCATION"); // Was Year
                case 6: return LanguageManager::instance().getText("TABLE_HEADER_PRICE"); // Was Pages
                case 7: return LanguageManager::instance().getText("TABLE_HEADER_STATUS");
                case 8: return LanguageManager::instance().getText("TABLE_HEADER_ACTIONS");
            }
        }
        return QVariant();
    }
    // Public refresh helper that properly resets the model
    void refresh() {
        beginResetModel();
        endResetModel();
    }
};

// Local helper fallback: lấy danh sách PTRDMS cho một ISBN bằng API cũ.
// Việc này giúp file trong `src/` biên dịch độc lập khi include chung chưa sẵn sàng.
[[maybe_unused]] static std::vector<PTRDMS> local_layBanSaoTheoISBN_fallback(const std::string &isbn) {
    std::vector<PTRDMS> out;
    // dự đoán số lượng tối đa
    PTRDMS buf[MAX_BAN_SAO];
    int cnt = LayDanhSachBanSaoSapXep(isbn, buf, MAX_BAN_SAO);
    if (cnt <= 0) return out;
    out.reserve(cnt);
    for (int i = 0; i < cnt; ++i) if (buf[i]) out.push_back(buf[i]);
    return out;
}

WidgetQuanLySach::WidgetQuanLySach(QWidget *cha) 
    : QWidget(cha), dauSachDangChon(nullptr) {
    setStyleSheet(
        "QWidget#WidgetQuanLySach {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "        stop:0 #1a1d23, stop:0.5 #242933, stop:1 #1a1d23);"
        "}"
    );
    setObjectName("WidgetQuanLySach");
    thieLapGiaoDien();
    taiLaiDanhSach();
    // Realtime: refresh when book data changes
    connect(&EventBus::instance(), &EventBus::dauSachAdded, this, [this](const QString&){ this->taiLaiDanhSach(); });
    connect(&EventBus::instance(), &EventBus::dauSachUpdated, this, [this](const QString&){ this->taiLaiDanhSach(); });
    connect(&EventBus::instance(), &EventBus::dauSachRemoved, this, [this](const QString&){ this->taiLaiDanhSach(); });
    // Update when loans change (affects availability counters)
    connect(&EventBus::instance(), &EventBus::muonTraAdded, this, [this](int, const QString&){ this->taiLaiDanhSach(); });
    connect(&EventBus::instance(), &EventBus::muonTraUpdated, this, [this](int, const QString&){ this->taiLaiDanhSach(); });
    connect(&EventBus::instance(), &EventBus::muonTraRemoved, this, [this](int, const QString&){ this->taiLaiDanhSach(); });

    // Watch data directory and library.db for external changes (imports, backups)
    try {
        FilePathManager::initialize();
        QString filesDir = FilePathManager::getFilesDirectory();
        QString dbPath = QDir(filesDir).filePath("files/library.db");
        fsWatcher = new QFileSystemWatcher(this);
        // watch directory always, watch DB file if exists
        if (QDir(filesDir).exists()) fsWatcher->addPath(filesDir);
        if (QFile::exists(dbPath)) fsWatcher->addPath(dbPath);
        connect(fsWatcher, &QFileSystemWatcher::fileChanged, this, &WidgetQuanLySach::onWatchedFileChanged);
        connect(fsWatcher, &QFileSystemWatcher::directoryChanged, this, &WidgetQuanLySach::onWatchedFileChanged);

        fsDebounceTimer = new QTimer(this);
        fsDebounceTimer->setSingleShot(true);
        fsDebounceTimer->setInterval(700);
        connect(fsDebounceTimer, &QTimer::timeout, this, [this]() { this->taiLaiDanhSach();
            // re-add db path if it was replaced
            QString filesDir2 = FilePathManager::getFilesDirectory();
            QString dbPath2 = QDir(filesDir2).filePath("library.db");
            if (QFile::exists(dbPath2) && !fsWatcher->files().contains(dbPath2)) {
                fsWatcher->addPath(dbPath2);
            }
        });
    } catch (...) {
        // ignore watcher setup failures
    }
    
    // Connect Language Change
    connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this, &WidgetQuanLySach::capNhatNgonNgu);
    QTimer::singleShot(0, this, &WidgetQuanLySach::capNhatNgonNgu);
}

WidgetQuanLySach::~WidgetQuanLySach() {}

void WidgetQuanLySach::thieLapGiaoDien() {
    boTriChinh = new QVBoxLayout(this);
    boTriChinh->setContentsMargins(20, 20, 20, 20);
    boTriChinh->setSpacing(15);
    
    // Header với các nút action
    thieLapHeaderBar();
    
    // Search & Filter bar
    thieLapTimKiem();
    
    // Table
    thieLapBangSach();
}

void WidgetQuanLySach::thieLapHeaderBar() {
    // Header Bar Container
    QWidget *headerContainer = new QWidget();
    headerContainer->setStyleSheet("background-color: transparent;");
    QHBoxLayout *headerLayout = new QHBoxLayout(headerContainer);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(15);

    // Group 1: DATA (Thêm Sách, Nhập Excel)
    QWidget *groupData = new QWidget();
    QHBoxLayout *layoutData = new QHBoxLayout(groupData);
    layoutData->setContentsMargins(0, 0, 0, 0);
    layoutData->setSpacing(8);
    
    nutThemSach = new QPushButton(" " + LanguageManager::instance().getText("BTN_ADD_BOOK"));
    nutThemSach->setIcon(QIcon(":/icons/plus.svg"));
    nutThemSach->setStyleSheet(
        "QPushButton { background-color: #0078D4; color: white; border: none; border-radius: 4px; padding: 6px 12px; font-weight: 600; }"
        "QPushButton:hover { background-color: #106EBE; }"
        "QPushButton:pressed { background-color: #005A9E; }"
    );
    
    nutNhapFile = new QPushButton(" " + LanguageManager::instance().getText("BTN_IMPORT"));
    nutNhapFile->setIcon(QIcon(":/icons/import.svg")); 
    nutNhapFile->setStyleSheet(
        "QPushButton { background-color: #2D2D30; color: #E0E0E0; border: 1px solid #3E3E42; border-radius: 4px; padding: 6px 12px; }"
        "QPushButton:hover { background-color: #3E3E42; border-color: #0078D4; }"
    );
    
    layoutData->addWidget(nutThemSach);
    layoutData->addWidget(nutNhapFile);
    
    // Group 2: ACTIONS (In Ấn)
    QWidget *groupActions = new QWidget();
    QHBoxLayout *layoutActions = new QHBoxLayout(groupActions);
    layoutActions->setContentsMargins(0, 0, 0, 0);
    layoutActions->setSpacing(8);

    nutXuatFile = new QPushButton(" " + LanguageManager::instance().getText("BTN_EXPORT_PRINT"));
    nutXuatFile->setIcon(QIcon(":/icons/print.svg"));
    nutXuatFile->setStyleSheet(nutNhapFile->styleSheet());
    
    nutKiemKe = new QPushButton(" " + LanguageManager::instance().getText("BTN_INVENTORY"));
    nutKiemKe->setIcon(QIcon(":/icons/search.svg"));
    nutKiemKe->setStyleSheet(nutNhapFile->styleSheet());
    
    layoutActions->addWidget(nutXuatFile);
    layoutActions->addWidget(nutKiemKe);

    // Group 3: SELECTION (Chi Tiết, Sửa, Xóa) - Initially Disabled
    QWidget *groupSelection = new QWidget();
    QHBoxLayout *layoutSelection = new QHBoxLayout(groupSelection);
    layoutSelection->setContentsMargins(0, 0, 0, 0);
    layoutSelection->setSpacing(8);

    auto createSelectBtn = [](QString text, QString icon, QString color) {
        QPushButton *btn = new QPushButton(" " + text);
        btn->setIcon(QIcon(icon));
        btn->setEnabled(false); // Default disabled
        btn->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: white; border: none; border-radius: 4px; padding: 6px 12px; font-weight: 600; }"
            "QPushButton:hover { background-color: %2; }"
            "QPushButton:disabled { background-color: #2D2D30; color: #666; }"
        ).arg(color).arg(color == "#C42B1C" ? "#A80000" : "#2D2D30")); // Simple logic, refine later if needed
        return btn;
    };

    nutXemChiTiet = createSelectBtn(LanguageManager::instance().getText("BTN_DETAILS"), ":/icons/info.svg", "#2D2D30");
    nutXemChiTiet->setStyleSheet(nutNhapFile->styleSheet()); // Use standard neutral style

    nutSuaSach = createSelectBtn(LanguageManager::instance().getText("BTN_EDIT"), ":/icons/edit.svg", "#2D2D30");
    nutSuaSach->setStyleSheet(nutNhapFile->styleSheet());

    nutXoaSach = createSelectBtn(LanguageManager::instance().getText("BTN_DELETE"), ":/icons/delete.svg", "#C42B1C"); // Red for delete

    layoutSelection->addWidget(nutXemChiTiet);
    layoutSelection->addWidget(nutSuaSach);
    layoutSelection->addWidget(nutXoaSach);

    // Add groups to main header layout
    headerLayout->addWidget(groupData);
    // Vertical separator
    QFrame *line1 = new QFrame(); line1->setFrameShape(QFrame::VLine); line1->setStyleSheet("color: #3E3E42;");
    headerLayout->addWidget(line1);
    
    headerLayout->addWidget(groupActions);
    
    QFrame *line2 = new QFrame(); line2->setFrameShape(QFrame::VLine); line2->setStyleSheet("color: #3E3E42;");
    headerLayout->addWidget(line2);

    headerLayout->addWidget(groupSelection);
    headerLayout->addStretch();

    boTriChinh->addWidget(headerContainer);
    
    // Connect standard signals (re-using existing slots)
    connect(nutThemSach, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutThemSach);
    connect(nutNhapFile, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNhapDuLieuTu);
    connect(nutXemChiTiet, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutXemChiTiet);
    connect(nutSuaSach, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutSuaSach);
    connect(nutXoaSach, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutXoaSach);
    connect(nutKiemKe, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutKiemKe);
    
    // Header: In -> Unified Print Dialog
    connect(nutXuatFile, &QPushButton::clicked, this, [this]() {
        // Use the centralized Print Manager Dialog
        ManHinhQuanLyIn dict(nullptr, {}, this);
        
        // Pass context if a book is selected
        if (dauSachDangChon) {
            dict.setBookContext(dauSachDangChon);
        }
        
        // Set Mode
        dict.setMode(ManHinhQuanLyIn::BOOK_CONTEXT);
        
        dict.exec();
    });
}

void WidgetQuanLySach::onWatchedFileChanged(const QString &path) {
    Q_UNUSED(path);
    if (fsDebounceTimer) fsDebounceTimer->start();
}

void WidgetQuanLySach::thieLapTimKiem() {
    QWidget *searchBar = new QWidget();
    searchBar->setStyleSheet("QWidget { background: transparent; }");
    QHBoxLayout *layout = new QHBoxLayout(searchBar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    
    // Search input
    oNhapTimKiem = new QLineEdit();
    oNhapTimKiem->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_SEARCH_BOOK"));
    oNhapTimKiem->setFixedHeight(36);
    oNhapTimKiem->setStyleSheet(
        "QLineEdit {"
        "    background-color: #1E1E2D;"
        "    color: #E0E0E0;"
        "    border: 1px solid #151820;"
        "    border-radius: 5px;"
        "    padding: 0 12px;"
        "    font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "    border: 1px solid #4A90E2;"
        "    background-color: #1E1E2D;"
        "}"
    );
    
    // Filter dropdowns
    hopChonTheLoai = new QComboBox();
    hopChonTheLoai->addItem(LanguageManager::instance().getText("FILTER_ALL_CATEGORIES"));
    hopChonTheLoai->setFixedHeight(36);
    hopChonTheLoai->setStyleSheet(
        "QComboBox {"
        "    background-color: #1E1E2D;"
        "    color: #E0E0E0;"
        "    border: 1px solid #151820;"
        "    border-radius: 5px;"
        "    padding: 0 12px;"
        "    font-size: 13px;"
        "}"
        "QComboBox:hover {"
        "    border: 1px solid #4A90E2;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "    width: 20px;"
        "}"
        "QComboBox::down-arrow {"
        "    image: none;"
        "    border-left: 4px solid transparent;"
        "    border-right: 4px solid transparent;"
        "    border-top: 5px solid #8A9AA8;"
        "    margin-right: 5px;"
        "}"
        "QComboBox QAbstractItemView {"
        "    background-color: #242933;"
        "    color: #CCCCCC;"
        "    selection-background-color: #4A90E2;"
        "    selection-color: #CCCCCC;"
        "    border: 1px solid #151820;"
        "    padding: 4px;"
        "}"
    );
    
    hopChonTrangThai = new QComboBox();
    hopChonTrangThai->addItem(LanguageManager::instance().getText("FILTER_ALL_STATUS"));
    hopChonTrangThai->addItem(LanguageManager::instance().getText("STATUS_AVAILABLE"));
    hopChonTrangThai->addItem(LanguageManager::instance().getText("STATUS_OUT_OF_STOCK"));
    hopChonTrangThai->setFixedHeight(36);
    hopChonTrangThai->setStyleSheet(hopChonTheLoai->styleSheet());
    
    // Search button
    nutTimKiem = new QPushButton(LanguageManager::instance().getText("BTN_SEARCH"));
    nutTimKiem->setFixedHeight(36);
    nutTimKiem->setFixedWidth(80);
    nutTimKiem->setStyleSheet(
        "QPushButton {"
        "    background-color: #5ba3f5;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    font-size: 13px;"
        "    font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "    background-color: #4a93e3;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #3d7dc4;"
        "}"
    );
    
    // Refresh button
    nutLamMoi = new QPushButton(LanguageManager::instance().getText("BTN_REFRESH"));
    nutLamMoi->setFixedHeight(36);
    nutLamMoi->setFixedWidth(100);
    nutLamMoi->setStyleSheet(
        "QPushButton {"
        "    background-color: #242933;"
        "    color: #CCCCCC;"
        "    border: 1px solid #151820;"
        "    border-radius: 4px;"
        "    font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2d3541;"
        "    border: 1px solid #4A90E2;"
        "}"
    );
    
    layout->addWidget(oNhapTimKiem, 3);
    layout->addWidget(hopChonTheLoai, 1);
    layout->addWidget(hopChonTrangThai, 1);
    layout->addWidget(nutTimKiem);
    layout->addWidget(nutLamMoi);
    
    boTriChinh->addWidget(searchBar);
    
    // Result label for search feedback
    nhanKetQuaTimKiem = new QLabel("");
    nhanKetQuaTimKiem->setStyleSheet("color: #CFCFCF; font-size: 12px; padding-left: 6px;");
    nhanKetQuaTimKiem->setFixedHeight(20);
    boTriChinh->addWidget(nhanKetQuaTimKiem);
    
    // Tải danh sách thể loại
    taiDanhSachTheLoai();
    
    connect(oNhapTimKiem, &QLineEdit::textChanged, this, &WidgetQuanLySach::xuLyTimKiem);
    connect(hopChonTheLoai, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WidgetQuanLySach::xuLyLocTheLoai);
    connect(hopChonTrangThai, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WidgetQuanLySach::xuLyLocTrangThai);
    connect(nutTimKiem, &QPushButton::clicked, this, [this]() { xuLyTimKiem(oNhapTimKiem->text()); });
    connect(nutLamMoi, &QPushButton::clicked, this, &WidgetQuanLySach::taiLaiDanhSach);
}

void WidgetQuanLySach::thieLapBangSach() {
    bangSach = new QTableView();
    LegacyBookModel *legacyModel = new LegacyBookModel(this);
    bangSach->setModel(legacyModel);
    bangSach->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangSach->setSelectionMode(QAbstractItemView::SingleSelection);
    bangSach->setAlternatingRowColors(true);
    
    // Use centralized InnerTable styling defined in resources/modern-dashboard.qss
    bangSach->setProperty("class", "InnerTable");
    
    // Set column widths
    bangSach->horizontalHeader()->setStretchLastSection(true);
    bangSach->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    bangSach->horizontalHeader()->setDefaultSectionSize(150);
    bangSach->setColumnWidth(0, 50);   // STT
    bangSach->setColumnWidth(1, 130);  // ISBN
    bangSach->setColumnWidth(2, 260);  // Tên Sách
    bangSach->setColumnWidth(3, 140);  // Tác Giả
    bangSach->setColumnWidth(4, 130);  // Thể Loại
    bangSach->setColumnWidth(5, 100);   // Vi Tri
    bangSach->setColumnWidth(6, 100);   // Gia Tien
    bangSach->setColumnWidth(7, 170);  // Trạng Thái (with badges)
    bangSach->setColumnWidth(8, 110);  // Thao Tác
    
    // Row height
    bangSach->verticalHeader()->setDefaultSectionSize(45);
    bangSach->verticalHeader()->setVisible(false);
    bangSach->setMinimumHeight(450);
    boTriChinh->addWidget(bangSach, 1);
    
    // Hint label
    QLabel *hintLabel = new QLabel(LanguageManager::instance().getText("LBL_HINT_BOOK_DBLCLICK"));
    hintLabel->setStyleSheet("color: #666; font-style: italic; font-size: 11px; margin-top: 2px;");
    hintLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    boTriChinh->addWidget(hintLabel);

    // Selection handling
    // Selection handling
    connect(bangSach->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex &current, const QModelIndex&) {
        if (!current.isValid()) {
            xuLyChonDong(-1, -1);
            return;
        }
        xuLyChonDong(current.row(), current.column());
    });

    // Right-click Context Menu
    bangSach->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(bangSach, &QTableView::customContextMenuRequested, this, [this](const QPoint &pos) {
        QModelIndex index = bangSach->indexAt(pos);
        if (!index.isValid()) return;

        QMenu contextMenu(this);
        contextMenu.setStyleSheet("QMenu { background-color: #2D2D30; color: white; border: 1px solid #3E3E42; }"
                                  "QMenu::item { padding: 8px 20px; }"
                                  "QMenu::item:selected { background-color: #0078D4; }");

        QAction *actChiTiet = contextMenu.addAction(QIcon(":/icons/info.svg"), LanguageManager::instance().getText("BTN_DETAILS"));
        QAction *actSua = contextMenu.addAction(QIcon(":/icons/edit.svg"), LanguageManager::instance().getText("CTX_EDIT_BOOK"));
        contextMenu.addSeparator();
        QAction *actInTem = contextMenu.addAction(QIcon(":/icons/barcode.svg"), LanguageManager::instance().getText("CTX_PRINT_BARCODE"));
        contextMenu.addSeparator();
        QAction *actXoa = contextMenu.addAction(QIcon(":/icons/delete.svg"), LanguageManager::instance().getText("CTX_DELETE_BOOK"));

        connect(actChiTiet, &QAction::triggered, this, &WidgetQuanLySach::xuLyNutXemChiTiet);
        connect(actSua, &QAction::triggered, this, &WidgetQuanLySach::xuLyNutSuaSach);
        connect(actInTem, &QAction::triggered, this, [this]() {
            if (dauSachDangChon) {
                // Use the automatic/interactive function            if (dauSachDangChon) {
                PrintHelper::inTemSachTuDong(dauSachDangChon, -1, this);
            }
        });
        connect(actXoa, &QAction::triggered, this, &WidgetQuanLySach::xuLyNutXoaSach);

        contextMenu.exec(bangSach->viewport()->mapToGlobal(pos));
    });

    // Double-click to show detail
    connect(bangSach, &QTableView::doubleClicked, this, [this](const QModelIndex &index) {
        if (!index.isValid()) return;
        if (dauSachDangChon) xuLyNutXemChiTiet();
    });
}

void WidgetQuanLySach::thieLapNutChucNang() {
    // Chức năng đã được chuyển lên Header Bar.
    nhomChucNang = nullptr;
}

void WidgetQuanLySach::taiLaiDanhSach() {
    hienThiDanhSachSach();
}

void WidgetQuanLySach::capNhatBang() {
    hienThiDanhSachSach();
}

void WidgetQuanLySach::hienThiDanhSachSach() {
    // Ask the model to refresh itself (it will call beginResetModel()/endResetModel())
    auto legacy = dynamic_cast<LegacyBookModel*>(bangSach->model());
    if (legacy) legacy->refresh();
    emit thongBaoCapNhat(LanguageManager::instance().getText("MSG_LOADED_BOOKS").arg(LaySoLuongDauSach()));
}

void WidgetQuanLySach::xuLyChonDong(int dong, int) {
    if (dong < 0) {
        // Clear selection
        dauSachDangChon = nullptr;
        if (nutXemChiTiet) nutXemChiTiet->setEnabled(false);
        if (nutSuaSach) nutSuaSach->setEnabled(false);
        if (nutXoaSach) nutXoaSach->setEnabled(false);
        return;
    }
    auto m = bangSach->model();
    if (!m) return;
    QString isbn = m->data(m->index(dong, 1), Qt::DisplayRole).toString();

    // extern removed

    
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] && QString::fromStdString(dsDauSach[i]->ISBN) == isbn) {
            dauSachDangChon = dsDauSach[i];
            emit thongBaoCapNhat(LanguageManager::instance().getText("MSG_SELECTED_READER").arg(QString::fromStdString(dauSachDangChon->tenSach))); 
            // Note: Reuse MSG_SELECTED_READER here as it fits "Selected: %1" pattern, or make MSG_SELECTED_BOOK
            
            // Enable buttons
            if (nutXemChiTiet) nutXemChiTiet->setEnabled(true);
            if (nutSuaSach) nutSuaSach->setEnabled(true);
            if (nutXoaSach) nutXoaSach->setEnabled(true);
            
            break;
        }
    }
}

void WidgetQuanLySach::xuLyNutThemSach() {
    HopThoaiThemSach *hopThoai = new HopThoaiThemSach(this);
    
    connect(hopThoai, &HopThoaiThemSach::sachDaThem, this, [this]() {
        taiLaiDanhSach();
        // Toast for success
        ToastNotification::show(LanguageManager::instance().getText("MSG_ADD_COPY_SUCCESS"), ToastNotification::SUCCESS);
    });
    
    hopThoai->exec();
    delete hopThoai;
}

void WidgetQuanLySach::xuLyNutXoaSach() {
    if (!dauSachDangChon) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_BOOK_DELETE"));
        return;
    }
    
    // Kiểm tra có thể xóa không
    if (!KiemTraCoTheXoaDauSach(dauSachDangChon)) {
        QMessageBox::warning(this, LanguageManager::instance().getText("MSG_CANNOT_DELETE_BOOK"), 
            LanguageManager::instance().getText("MSG_CANNOT_DELETE_BOOK_HAS_COPIES"));
        return;
    }
    
    // Sử dụng ConfirmationDialog đẹp hơn
    ThongTinDauSach info = LayThongTinDauSach(dauSachDangChon->ISBN);
    QString chiTiet = QString(
        "Tên sách: %1\n"
        "Tác giả: %2\n"
        "ISBN: %3\n\n"
        "Tổng bản sao: %4\n"
        "Có thể mượn: %5\n"
        "Đang mượn: %6\n"
        "Đã thanh lý: %7"
    )
    .arg(QString::fromStdString(dauSachDangChon->tenSach))
    .arg(QString::fromStdString(dauSachDangChon->tacGia))
    .arg(QString::fromStdString(dauSachDangChon->ISBN))
    .arg(info.tongBanSao)
    .arg(info.choMuonDuoc)
    .arg(info.dangMuon)
    .arg(info.thanhLy);
    
    bool xacNhan = ConfirmationDialog::confirmDelete(
        this,
        QString::fromStdString(dauSachDangChon->tenSach),
        LanguageManager::instance().getText("MSG_DELETE_BOOK_CONFIRM_DETAILS")
            .arg(QString::fromStdString(dauSachDangChon->tenSach))
            .arg(QString::fromStdString(dauSachDangChon->tacGia))
            .arg(QString::fromStdString(dauSachDangChon->ISBN))
            .arg(info.tongBanSao)
            .arg(info.choMuonDuoc)
            .arg(info.dangMuon)
            .arg(info.thanhLy)
    );
    
    if (xacNhan) {
        // Xóa đầu sách và kiểm tra kết quả
        std::string ketQua = XoaDauSachTheoISBN(dauSachDangChon->ISBN);
        
        if (ketQua.empty()) {
            // Thành công - SQL Sync is handled inside XoaDauSachTheoISBN
            // REMOVED: Blocking QMessageBox
            // QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"), LanguageManager::instance().getText("MSG_DELETE_BOOK_SUCCESS"));
            
            EventBus::instance().dauSachRemoved(QString::fromStdString(dauSachDangChon->ISBN));
            dauSachDangChon = nullptr;
            taiLaiDanhSach();
            
            // Toast Success
            ToastNotification::show(LanguageManager::instance().getText("MSG_DELETE_BOOK_SUCCESS"), ToastNotification::SUCCESS);
            emit duLieuThayDoi();
        } else {
            // Thất bại - hiển thị lỗi
            QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_ERROR"), QString::fromStdString(ketQua));
            qWarning() << "Lỗi xóa sách:" << QString::fromStdString(ketQua);
        }
    }
}

void WidgetQuanLySach::xuLyNutSuaSach() {
    if (!dauSachDangChon) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_BOOK_EDIT"));
        return;
    }
    
    HopThoaiSuaSach *hopThoai = new HopThoaiSuaSach(QString::fromStdString(dauSachDangChon->ISBN), this);
    connect(hopThoai, &HopThoaiSuaSach::sachDaSua, this, [this]() {
        taiLaiDanhSach();
        ToastNotification::show(LanguageManager::instance().getText("MSG_UPDATE_BOOK_SUCCESS"), ToastNotification::SUCCESS);
    });
    hopThoai->exec();
    delete hopThoai;
}

void WidgetQuanLySach::xuLyNutThemBanSao() {
    if (!dauSachDangChon) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_BOOK_ADD_COPY"));
        return;
    }
    
    HopThoaiThemBanSao *hopThoai = new HopThoaiThemBanSao(QString::fromStdString(dauSachDangChon->ISBN), this);
    
    connect(hopThoai, &HopThoaiThemBanSao::banSaoDaThem, this, [this]() {
        taiLaiDanhSach();
        ToastNotification::show(LanguageManager::instance().getText("MSG_ADD_COPY_SUCCESS"), ToastNotification::SUCCESS);
    });
    
    hopThoai->exec();
    delete hopThoai;
}

void WidgetQuanLySach::xuLyNutXoaBanSao() {}

void WidgetQuanLySach::xuLyNutXemChiTiet() {
    if (!dauSachDangChon) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_BOOK_DETAILS"));
        return;
    }
    
    ManHinhChiTietSach *dialog = new ManHinhChiTietSach(dauSachDangChon, this);
    
    connect(dialog, &ManHinhChiTietSach::sachDaCapNhat, this, [this]() {
        taiLaiDanhSach();
        ToastNotification::show(LanguageManager::instance().getText("MSG_UPDATE_COPY_SUCCESS"), ToastNotification::SUCCESS);
    });
    
    dialog->exec();
    delete dialog;
}

void WidgetQuanLySach::xuLyTimKiem(const QString &tuKhoa) {
    QString tuKhoaLowerCase = tuKhoa.trimmed().toLower();
    int soKetQua = 0;
    QAbstractItemModel *m = qobject_cast<QAbstractItemModel*>(bangSach->model());
    if (!m) return;
    int rows = m->rowCount();
    for (int i = 0; i < rows; ++i) {
        bool hienThi = true;
        if (!tuKhoaLowerCase.isEmpty()) {
            QString isbn = m->data(m->index(i, 1), Qt::DisplayRole).toString().toLower();
            QString tenSach = m->data(m->index(i, 2), Qt::DisplayRole).toString().toLower();
            QString tacGia = m->data(m->index(i, 3), Qt::DisplayRole).toString().toLower();
            hienThi = isbn.contains(tuKhoaLowerCase) || tenSach.contains(tuKhoaLowerCase) || tacGia.contains(tuKhoaLowerCase);
        }
        if (hienThi && hopChonTheLoai->currentIndex() > 0) {
            QString theLoaiChon = hopChonTheLoai->currentText();
            QString theLoaiDong = m->data(m->index(i, 4), Qt::DisplayRole).toString();
            if (theLoaiDong != theLoaiChon) hienThi = false;
        }
        if (hienThi && hopChonTrangThai->currentIndex() > 0) {
            // Logic based on index, not text (robust for localization)
            // Index 1: Available (Status "X/Y" where X > 0)
            // Index 2: Out of Stock (Status "X/Y" where X == 0)
            QString trangThaiDong = m->data(m->index(i, 7), Qt::DisplayRole).toString();
            QStringList parts = trangThaiDong.split("/");
            if (parts.size() >= 1) {
                int available = parts[0].toInt();
                if (hopChonTrangThai->currentIndex() == 1) {
                    if (available <= 0) hienThi = false;
                } else if (hopChonTrangThai->currentIndex() == 2) {
                    if (available > 0) hienThi = false;
                }
            }
        }
        bangSach->setRowHidden(i, !hienThi);
        if (hienThi) ++soKetQua;
    }
    
    // Hiển thị kết quả
    if (tuKhoaLowerCase.isEmpty() && hopChonTheLoai->currentIndex() == 0 && hopChonTrangThai->currentIndex() == 0) {
        SafeGui::safeSetText(nhanKetQuaTimKiem, "");
    } else {
        SafeGui::safeSetText(nhanKetQuaTimKiem, LanguageManager::instance().getText("MSG_SEARCH_RESULT_BOOKS").arg(soKetQua));
    }
}

void WidgetQuanLySach::xuLyLocTheLoai(int) {
    xuLyTimKiem(oNhapTimKiem->text());
}

void WidgetQuanLySach::xuLyLocTrangThai(int) {
    xuLyTimKiem(oNhapTimKiem->text());
}

void WidgetQuanLySach::taiDanhSachTheLoai() {
    // extern removed

    
    QSet<QString> theLoaiSet;
    
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] != nullptr) {
            QString theLoai = QString::fromStdString(dsDauSach[i]->theLoai).trimmed();
            if (!theLoai.isEmpty()) {
                theLoaiSet.insert(theLoai);
            }
        }
    }
    
    // Chuyển sang list và sắp xếp
    QStringList theLoaiList = theLoaiSet.values();
    std::sort(theLoaiList.begin(), theLoaiList.end());
    
    // Thêm vào combobox
    for (const QString &theLoai : theLoaiList) {
        hopChonTheLoai->addItem(theLoai);
    }
}
void WidgetQuanLySach::xuLyNhapDuLieuTu() {
    QString file = QFileDialog::getOpenFileName(this, LanguageManager::instance().getText("TITLE_SELECT_EXCEL"), QDir::currentPath(), LanguageManager::instance().getText("FILTER_EXCEL_FILES"));
    if (file.isEmpty()) return;

    ImportManager importer(this);


    // Show loading
    QProgressDialog progress(LanguageManager::instance().getText("MSG_PROCESSING_EXCEL"), QString(), 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.show();
    QCoreApplication::processEvents();

    auto result = importer.runPreview(file);
    progress.close();

    if (!result.success) {
        QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_ERROR"), LanguageManager::instance().getText("MSG_IMPORT_ERROR") + result.message);
        return;
    }

    // Show Preview Dialog
    ManHinhPreviewImport dlg(result, file, &importer, this);
    if (dlg.exec() == QDialog::Accepted) {
        // Success - RELOAD FROM DB to RAM
        NapDanhSachTuDB();
        
        taiLaiDanhSach();
        // Force refresh table
        hienThiDanhSachSach();
    }
}

void WidgetQuanLySach::capNhatNgonNgu() {
    // 1. Buttons
    if(nutThemSach) nutThemSach->setText(" " + LanguageManager::instance().getText("BTN_ADD_BOOK"));
    if(nutNhapFile) nutNhapFile->setText(" " + LanguageManager::instance().getText("BTN_IMPORT"));
    if(nutXuatFile) nutXuatFile->setText(" " + LanguageManager::instance().getText("BTN_EXPORT_PRINT"));
    if(nutXemChiTiet) nutXemChiTiet->setText(" " + LanguageManager::instance().getText("BTN_DETAILS"));
    if(nutSuaSach) nutSuaSach->setText(" " + LanguageManager::instance().getText("BTN_EDIT"));
    if(nutXoaSach) nutXoaSach->setText(" " + LanguageManager::instance().getText("BTN_DELETE"));
    if(nutTimKiem) nutTimKiem->setText(LanguageManager::instance().getText("BTN_SEARCH"));
    if(nutLamMoi) nutLamMoi->setText(LanguageManager::instance().getText("BTN_REFRESH"));
    
    // 2. Search
    if(oNhapTimKiem) oNhapTimKiem->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_SEARCH_BOOK"));
    
    // 3. Update Model Headers
    auto legacy = dynamic_cast<LegacyBookModel*>(bangSach->model());
    if (legacy) {
        legacy->refresh(); 
        QEvent e(QEvent::LanguageChange);
        if (bangSach) QApplication::sendEvent(bangSach->horizontalHeader(), &e);
    }
}
void WidgetQuanLySach::hienThiBanSao(PTRDS dauSach) {
    // Clear existing cache
    danhSachBanSaoVec.clear();

    if (!dauSach) {
        SafeGui::safeSetText(nhanThongTinChon, "");
        // try to notify model (best-effort)
        if (modelBanSao) QMetaObject::invokeMethod(modelBanSao, "refresh", Qt::QueuedConnection);
        return;
    }

    // Sử dụng DBBatcher để lấy danh sách bản sao cho ISBN một cách an toàn
    // (trả về vector<PTRDMS> để dễ xử lý trong UI và không cần cấp buffer tay)
    // Try to use DBBatcher if available at compile time, otherwise use local fallback
#if defined(__has_include) && __has_include("utils/DBBatcher.h")
    std::vector<PTRDMS> tmp = DBBatcher::layBanSaoTheoISBN(dauSach->ISBN);
#else
    std::vector<PTRDMS> tmp = local_layBanSaoTheoISBN_fallback(dauSach->ISBN);
#endif
    int count = static_cast<int>(tmp.size());
    if (count > 0) {
        danhSachBanSaoVec.reserve(count);
        for (int i = 0; i < count; ++i) danhSachBanSaoVec.push_back(tmp[i]);
    }

    // Update small info label (if present)
    if (nhanThongTinChon) {
        int available = 0;
        for (auto &p : danhSachBanSaoVec) if (p && p->trangThai == CHO_MUON_DUOC) ++available;
        SafeGui::safeSetText(nhanThongTinChon, LanguageManager::instance().getText("LBL_COPY_INFO").arg(count).arg(available));
    }

    // Best-effort: if the concrete model exposes a public refresh() method (as in BanSaoTableModel), try to call it.
    if (modelBanSao) {
        // invokeMethod will fail silently if no such invokable method exists; use queued connection to be safe.
        QMetaObject::invokeMethod(modelBanSao, "refresh", Qt::QueuedConnection);
    }
}

QString WidgetQuanLySach::layTrangThaiSach(TrangThaiSach trangThai) const {
    switch(trangThai) {
        case CHO_MUON_DUOC: return LanguageManager::instance().getText("STATUS_READY");
        case DANG_MUON: return LanguageManager::instance().getText("STATUS_BORROWED");
        case THANH_LY: return LanguageManager::instance().getText("STATUS_LIQUIDATED");
        default: return LanguageManager::instance().getText("STATUS_UNKNOWN");
    }
}

QColor WidgetQuanLySach::layMauTrangThai(TrangThaiSach trangThai) const {
    switch(trangThai) {
        case CHO_MUON_DUOC: return QColor("#1BC5BD");
        case DANG_MUON: return QColor("#FFA800");
        case THANH_LY: return QColor("#F64E60");
        default: return QColor("#6C6C80");
    }
}
void WidgetQuanLySach::xuLyNutKiemKe() {
    ManHinhKiemKe dialog(this);
    dialog.exec();
}
