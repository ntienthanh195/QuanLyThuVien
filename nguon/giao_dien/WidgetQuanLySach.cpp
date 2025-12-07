#include "giao_dien/WidgetQuanLySach.h"
#include "utils/FilePathManager.h"
#include "giao_dien/hop_thoai/HopThoaiThemSach.h"
#include "giao_dien/hop_thoai/HopThoaiSuaSach.h"
#include "giao_dien/hop_thoai/HopThoaiThemBanSao.h"
#include "models/QuanLySach.h"
#include "models/ThaoTacFile.h"
#include "utils/ResponsiveHelper.h"
#include "utils/AnimationHelper.h"
#include "utils/ConfirmationDialog.h"
#include "utils/EventBus.h"
#include "utils/SafeGui.h"
// DBBatcher integration is optional for the legacy `nguon/` build;
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
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
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
                case 5: return d->namXuatBan;
                case 6: return d->soTrang;
                case 7: {
                    int tong = d->tongBanSao;
                    int daMuon = 0, thanhLy = 0;
                    PTRDMS p = d->dms; int dem = 0;
                    while (p && dem < 10000) { if (p->trangThai == DANG_MUON) daMuon++; else if (p->trangThai == THANH_LY) thanhLy++; p = p->next; dem++; }
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
                case 0: return "STT";
                case 1: return "ISBN";
                case 2: return "Tên Sách";
                case 3: return "Tác Giả";
                case 4: return "Thể Loại";
                case 5: return "Năm XB";
                case 6: return "Số Trang";
                case 7: return "Trạng Thái";
                case 8: return "Thao Tác";
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
// Việc này giúp file trong `nguon/` biên dịch độc lập khi include chung chưa sẵn sàng.
static std::vector<PTRDMS> local_layBanSaoTheoISBN_fallback(const std::string &isbn) {
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
        "        stop:0 #1a1d29, stop:0.5 #1e2230, stop:1 #1a1d29);"
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
    QWidget *headerBar = new QWidget();
    headerBar->setStyleSheet("QWidget { background: transparent; }");
    QHBoxLayout *layout = new QHBoxLayout(headerBar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    
    // Create action buttons
    auto createButton = [](const QString &icon, const QString &text, const QString &color) -> QPushButton* {
        QPushButton *btn = new QPushButton(icon + " " + text);
        btn->setFixedHeight(36);
        btn->setMinimumWidth(110);
        
        // Simple hover/pressed colors
        QString hoverColor = (color == "#0078D4") ? "#106EBE" : "#3E3E42";
        QString pressedColor = (color == "#0078D4") ? "#005A9E" : "#2D2D30";
        
        btn->setStyleSheet(QString(
            "QPushButton {"
            "    background-color: %1;"
            "    color: white;"
            "    border: none;"
            "    border-radius: 4px;"
            "    font-size: 13px;"
            "    font-weight: 500;"
            "    padding: 0 16px;"
            "}"
            "QPushButton:hover {"
            "    background-color: %2;"
            "}"
            "QPushButton:pressed {"
            "    background-color: %3;"
            "}"
        ).arg(color, hoverColor, pressedColor));
        return btn;
    };
    
    // Buttons
    nutThemSach = createButton("+", "Thêm Sách", "#0078D4");
    nutNhapFile = createButton("⇩", "Nhập Excel", "#2D2D30");
    nutNhapFile->setStyleSheet(
        "QPushButton {"
        "    background-color: #2D2D30;"
        "    color: #CCCCCC;"
        "    border: 1px solid #3E3E42;"
        "    border-radius: 4px;"
        "    font-size: 13px;"
        "    padding: 0 16px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #3E3E42;"
        "    border: 1px solid #0078D4;"
        "}"
    );
    
    nutXuatFile = createButton("⎙", "In", "#2D2D30");
    nutXuatFile->setStyleSheet(nutNhapFile->styleSheet());
    
    nutSuaSach = createButton("✏", "Sửa", "#2D2D30");
    nutSuaSach->setStyleSheet(nutNhapFile->styleSheet());
    
    nutXemChiTiet = createButton("ⓘ", "Chi Tiết", "#2D2D30");
    nutXemChiTiet->setStyleSheet(nutNhapFile->styleSheet());
    
    nutXoaSach = createButton("✖", "Xóa", "#C42B1C");
    
    layout->addWidget(nutThemSach);
    layout->addWidget(nutNhapFile);
    layout->addWidget(nutXuatFile);
    layout->addWidget(nutSuaSach);
    layout->addWidget(nutXemChiTiet);
    layout->addWidget(nutXoaSach);
    layout->addStretch();
    
    boTriChinh->addWidget(headerBar);
    
    // Connect signals
    connect(nutThemSach, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutThemSach);
    connect(nutSuaSach, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutSuaSach);
    connect(nutXoaSach, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutXoaSach);
    connect(nutXemChiTiet, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutXemChiTiet);
    connect(nutNhapFile, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNhapDuLieuTu);
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
    oNhapTimKiem->setPlaceholderText("Nhập ISBN, tên sách, tác giả...");
    oNhapTimKiem->setFixedHeight(36);
    oNhapTimKiem->setStyleSheet(
        "QLineEdit {"
        "    background-color: #252830;"
        "    color: #E0E0E0;"
        "    border: 1px solid #3E4451;"
        "    border-radius: 5px;"
        "    padding: 0 12px;"
        "    font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "    border: 1px solid #0078D4;"
        "    background-color: #1E1E1E;"
        "}"
    );
    
    // Filter dropdowns
    hopChonTheLoai = new QComboBox();
    hopChonTheLoai->addItem("Tất cả thể loại");
    hopChonTheLoai->setFixedHeight(36);
    hopChonTheLoai->setStyleSheet(
        "QComboBox {"
        "    background-color: #252830;"
        "    color: #E0E0E0;"
        "    border: 1px solid #3E4451;"
        "    border-radius: 5px;"
        "    padding: 0 12px;"
        "    font-size: 13px;"
        "}"
        "QComboBox:hover {"
        "    border: 1px solid #0078D4;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "    width: 20px;"
        "}"
        "QComboBox::down-arrow {"
        "    image: none;"
        "    border-left: 4px solid transparent;"
        "    border-right: 4px solid transparent;"
        "    border-top: 5px solid #858585;"
        "    margin-right: 5px;"
        "}"
        "QComboBox QAbstractItemView {"
        "    background-color: #252526;"
        "    color: #CCCCCC;"
        "    selection-background-color: #37373D;"
        "    selection-color: #CCCCCC;"
        "    border: 1px solid #3E3E42;"
        "    padding: 4px;"
        "}"
    );
    
    hopChonTrangThai = new QComboBox();
    hopChonTrangThai->addItems({"Tất cả trạng thái", "Còn sách", "Hết sách"});
    hopChonTrangThai->setFixedHeight(36);
    hopChonTrangThai->setStyleSheet(hopChonTheLoai->styleSheet());
    
    // Search button
    nutTimKiem = new QPushButton("🔍 Tìm");
    nutTimKiem->setFixedHeight(36);
    nutTimKiem->setFixedWidth(80);
    nutTimKiem->setStyleSheet(
        "QPushButton {"
        "    background-color: #0078D4;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    font-size: 13px;"
        "    font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "    background-color: #106EBE;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #005A9E;"
        "}"
    );
    
    // Refresh button
    nutLamMoi = new QPushButton("🔄 Làm mới");
    nutLamMoi->setFixedHeight(36);
    nutLamMoi->setFixedWidth(100);
    nutLamMoi->setStyleSheet(
        "QPushButton {"
        "    background-color: #2D2D30;"
        "    color: #CCCCCC;"
        "    border: 1px solid #3E3E42;"
        "    border-radius: 4px;"
        "    font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #3E3E42;"
        "    border: 1px solid #0078D4;"
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
    
    // Natural color table styling
    bangSach->setStyleSheet(
        "QTableWidget {"
        "    background-color: #1e2230;"
        "    color: #E0E0E0;"
        "    gridline-color: #2d3140;"
        "    border: 1px solid #3E4451;"
        "    border-radius: 6px;"
        "    font-size: 12px;"
        "}"
        "QTableWidget::item {"
        "    padding: 10px 8px;"
        "    border-bottom: 1px solid #252830;"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: #2d5f8d;"
        "    color: #FFFFFF;"
        "}"
        "QTableWidget::item:alternate {"
        "    background-color: #252830;"
        "}"
        "QTableWidget::item:alternate:selected {"
        "    background-color: #2d5f8d;"
        "    color: #FFFFFF;"
        "}"
        "QHeaderView::section {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "        stop:0 #2d3140, stop:1 #252830);"
        "    color: #E0E0E0;"
        "    padding: 11px 8px;"
        "    border: none;"
        "    border-right: 1px solid #3E4451;"
        "    border-bottom: 2px solid #3E4451;"
        "    font-weight: 600;"
        "    font-size: 12px;"
        "}"
        "QTableWidget QScrollBar:vertical {"
        "    background: #1E1E1E;"
        "    width: 12px;"
        "    border: none;"
        "}"
        "QTableWidget QScrollBar::handle:vertical {"
        "    background: #3E3E42;"
        "    border-radius: 6px;"
        "    min-height: 20px;"
        "}"
        "QTableWidget QScrollBar::handle:vertical:hover {"
        "    background: #4E4E52;"
        "}"
    );
    
    // Set column widths
    bangSach->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    bangSach->horizontalHeader()->setDefaultSectionSize(150);
    bangSach->setColumnWidth(0, 50);   // STT
    bangSach->setColumnWidth(1, 130);  // ISBN
    bangSach->setColumnWidth(2, 260);  // Tên Sách
    bangSach->setColumnWidth(3, 140);  // Tác Giả
    bangSach->setColumnWidth(4, 130);  // Thể Loại
    bangSach->setColumnWidth(5, 80);   // Năm XB
    bangSach->setColumnWidth(6, 80);   // Số Trang
    bangSach->setColumnWidth(7, 170);  // Trạng Thái (with badges)
    bangSach->setColumnWidth(8, 110);  // Thao Tác
    
    // Row height
    bangSach->verticalHeader()->setDefaultSectionSize(45);
    bangSach->verticalHeader()->setVisible(false);
    bangSach->setMinimumHeight(500);
    boTriChinh->addWidget(bangSach, 1);

    // Selection handling
    connect(bangSach->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex &current, const QModelIndex&) {
        if (!current.isValid()) return;
        int row = current.row();
        extern PTRDS* LayDanhSachDauSach();
        extern int LaySoLuongDauSach();
        PTRDS *arr = LayDanhSachDauSach();
        int n = LaySoLuongDauSach();
        if (row < 0 || row >= n) return;
        dauSachDangChon = arr ? arr[row] : nullptr;
    });

    // Double-click to show detail
    connect(bangSach, &QTableView::doubleClicked, this, [this](const QModelIndex &index) {
        if (!index.isValid()) return;
        int row = index.row();
        extern PTRDS* LayDanhSachDauSach();
        extern int LaySoLuongDauSach();
        PTRDS *arr = LayDanhSachDauSach();
        int n = LaySoLuongDauSach();
        if (row < 0 || row >= n) return;
        dauSachDangChon = arr ? arr[row] : nullptr;
        if (dauSachDangChon) xuLyNutXemChiTiet();
    });
}

void WidgetQuanLySach::thieLapNutChucNang() {
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    
    nhomChucNang = new QGroupBox("⚙️ Chức Năng");
    int padding = ResponsiveHelper::getScaledSpacing(15, screenSize);
    nhomChucNang->setStyleSheet(QString(
        "QGroupBox {"
        "    background-color: #1E1E2D;"
        "    border: 1px solid #2B2B40;"
        "    border-radius: 12px;"
        "    padding: %1px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    color: #FFFFFF;"
        "}"
    ).arg(padding));
    
    QHBoxLayout *boTri = new QHBoxLayout(nhomChucNang);
    boTri->setSpacing(ResponsiveHelper::getScaledSpacing(15, screenSize));
    
    nutThemSach = new QPushButton(QIcon(":/icons/plus.svg"), " Thêm Sách");
    nutSuaSach = new QPushButton(QIcon(":/icons/edit.svg"), " Sửa");
    nutXoaSach = new QPushButton(QIcon(":/icons/trash.svg"), " Xóa");
    nutThemBanSao = new QPushButton(QIcon(":/icons/copy.svg"), " Thêm Bản Sao");
    nutXemChiTiet = new QPushButton(QIcon(":/icons/info.svg"), " Chi Tiết");
    
    // Tooltips
    nutThemSach->setToolTip("Thêm đầu sách mới vào thư viện");
    nutSuaSach->setToolTip("Sửa thông tin đầu sách đã chọn");
    nutXoaSach->setToolTip("Xóa đầu sách và tất cả bản sao");
    nutThemBanSao->setToolTip("Thêm bản sao cho đầu sách đã chọn");
    nutXemChiTiet->setToolTip("Xem chi tiết đầu sách và danh sách bản sao");
    
    // Responsive button styling
    int buttonPadding = ResponsiveHelper::getScaledSpacing(15, screenSize);
    int fontSize = ResponsiveHelper::getScaledFontSize(14, screenSize);
    
    // Gradient + shadow styles cho từng button
    QString kieuThemSach = QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1BC5BD, stop:1 #0e6c66);"
        "    color: white; border: none; border-radius: 8px;"
        "    padding: %1px 25px; font-weight: bold; font-size: %2px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0e9d95, stop:1 #084c47);"
        "}"
        "QPushButton:disabled { background-color: #2B2B40; color: #6C6C80; }"
    ).arg(buttonPadding).arg(fontSize);
    
    QString kieuSuaSach = QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3699FF, stop:1 #1565c0);"
        "    color: white; border: none; border-radius: 8px;"
        "    padding: %1px 25px; font-weight: bold; font-size: %2px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #187DE4, stop:1 #0d47a1);"
        "}"
        "QPushButton:disabled { background-color: #2B2B40; color: #6C6C80; }"
    ).arg(buttonPadding).arg(fontSize);
    
    QString kieuXoaSach = QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #F64E60, stop:1 #c62828);"
        "    color: white; border: none; border-radius: 8px;"
        "    padding: %1px 25px; font-weight: bold; font-size: %2px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #d32f2f, stop:1 #b71c1c);"
        "}"
        "QPushButton:disabled { background-color: #2B2B40; color: #6C6C80; }"
    ).arg(buttonPadding).arg(fontSize);
    
    QString kieuThemBanSao = QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFA800, stop:1 #e65100);"
        "    color: white; border: none; border-radius: 8px;"
        "    padding: %1px 25px; font-weight: bold; font-size: %2px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #d68910, stop:1 #bf360c);"
        "}"
        "QPushButton:disabled { background-color: #2B2B40; color: #6C6C80; }"
    ).arg(buttonPadding).arg(fontSize);
    
    QString kieuChiTiet = QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #8950FC, stop:1 #5e35b1);"
        "    color: white; border: none; border-radius: 8px;"
        "    padding: %1px 25px; font-weight: bold; font-size: %2px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #6c2bd9, stop:1 #4527a0);"
        "}"
        "QPushButton:disabled { background-color: #2B2B40; color: #6C6C80; }"
    ).arg(buttonPadding).arg(fontSize);
    
    nutThemSach->setStyleSheet(kieuThemSach);
    nutSuaSach->setStyleSheet(kieuSuaSach);
    nutXoaSach->setStyleSheet(kieuXoaSach);
    nutThemBanSao->setStyleSheet(kieuThemBanSao);
    nutXemChiTiet->setStyleSheet(kieuChiTiet);
    
    // Shadow effects
    QGraphicsDropShadowEffect *shadowThem = new QGraphicsDropShadowEffect();
    shadowThem->setBlurRadius(15);
    shadowThem->setOffset(0, 3);
    shadowThem->setColor(QColor(0, 0, 0, 80));
    nutThemSach->setGraphicsEffect(shadowThem);
    
    QGraphicsDropShadowEffect *shadowSua = new QGraphicsDropShadowEffect();
    shadowSua->setBlurRadius(15);
    shadowSua->setOffset(0, 3);
    shadowSua->setColor(QColor(0, 0, 0, 80));
    nutSuaSach->setGraphicsEffect(shadowSua);
    
    QGraphicsDropShadowEffect *shadowXoa = new QGraphicsDropShadowEffect();
    shadowXoa->setBlurRadius(15);
    shadowXoa->setOffset(0, 3);
    shadowXoa->setColor(QColor(0, 0, 0, 80));
    nutXoaSach->setGraphicsEffect(shadowXoa);
    
    QGraphicsDropShadowEffect *shadowBanSao = new QGraphicsDropShadowEffect();
    shadowBanSao->setBlurRadius(15);
    shadowBanSao->setOffset(0, 3);
    shadowBanSao->setColor(QColor(0, 0, 0, 80));
    nutThemBanSao->setGraphicsEffect(shadowBanSao);
    
    QGraphicsDropShadowEffect *shadowChiTiet = new QGraphicsDropShadowEffect();
    shadowChiTiet->setBlurRadius(15);
    shadowChiTiet->setOffset(0, 3);
    shadowChiTiet->setColor(QColor(0, 0, 0, 80));
    nutXemChiTiet->setGraphicsEffect(shadowChiTiet);
    
    boTri->addWidget(nutThemSach);
    boTri->addWidget(nutSuaSach);
    boTri->addWidget(nutXoaSach);
    boTri->addWidget(nutThemBanSao);
    boTri->addWidget(nutXemChiTiet);
    boTri->addStretch();
    
    boTriChinh->addWidget(nhomChucNang);
    
    connect(nutThemSach, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutThemSach);
    connect(nutSuaSach, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutSuaSach);
    connect(nutXoaSach, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutXoaSach);
    connect(nutThemBanSao, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutThemBanSao);
    connect(nutXemChiTiet, &QPushButton::clicked, this, &WidgetQuanLySach::xuLyNutXemChiTiet);
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
    emit thongBaoCapNhat(QString("Đã tải %1 đầu sách").arg(LaySoLuongDauSach()));
}

void WidgetQuanLySach::xuLyChonDong(int dong, int) {
    if (dong < 0) return;
    auto m = bangSach->model();
    if (!m) return;
    QString isbn = m->data(m->index(dong, 1), Qt::DisplayRole).toString();
    
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] && QString::fromStdString(dsDauSach[i]->ISBN) == isbn) {
            dauSachDangChon = dsDauSach[i];
            emit thongBaoCapNhat(QString("Đã chọn: %1").arg(QString::fromStdString(dauSachDangChon->tenSach)));
            break;
        }
    }
}

void WidgetQuanLySach::xuLyNutThemSach() {
    HopThoaiThemSach *hopThoai = new HopThoaiThemSach(this);
    
    connect(hopThoai, &HopThoaiThemSach::sachDaThem, this, [this]() {
        taiLaiDanhSach();
        emit thongBaoCapNhat("Đã thêm sách mới thành công!");
    });
    
    hopThoai->exec();
    delete hopThoai;
}

void WidgetQuanLySach::xuLyNutXoaSach() {
    if (!dauSachDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một đầu sách để xóa!");
        return;
    }
    
    // Kiểm tra có thể xóa không
    if (!KiemTraCoTheXoaDauSach(dauSachDangChon)) {
        QMessageBox::warning(this, "Không thể xóa", 
            "Không thể xóa đầu sách này!\n"
            "Vẫn còn bản sao đang được mượn hoặc chưa thanh lý.");
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
        chiTiet
    );
    
    if (xacNhan) {
        // Xóa đầu sách và kiểm tra kết quả
        std::string ketQua = XoaDauSachTheoISBN(dauSachDangChon->ISBN);
        
        if (ketQua.empty()) {
            // Thành công - lưu lại vào file
            extern PTRDS dsDauSach[];
            extern int soLuongDauSach;
            GhiDanhSachDauSach(FilePathManager::getDauSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
            GhiDanhMucSach(FilePathManager::getDanhMucSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
            
            QMessageBox::information(this, "Thành công", "Đã xóa đầu sách thành công!");
            dauSachDangChon = nullptr;
            taiLaiDanhSach();
            emit thongBaoCapNhat("Đã xóa đầu sách!");
            emit duLieuThayDoi();
        } else {
            // Thất bại - hiển thị lỗi
            QMessageBox::critical(this, "Lỗi", QString::fromStdString(ketQua));
            qWarning() << "Lỗi xóa sách:" << QString::fromStdString(ketQua);
        }
    }
}

void WidgetQuanLySach::xuLyNutSuaSach() {
    if (!dauSachDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một đầu sách để sửa!");
        return;
    }
    
    HopThoaiSuaSach *hopThoai = new HopThoaiSuaSach(QString::fromStdString(dauSachDangChon->ISBN), this);
    connect(hopThoai, &HopThoaiSuaSach::sachDaSua, this, [this]() {
        taiLaiDanhSach();
        emit thongBaoCapNhat("Đã cập nhật thông tin sách!");
    });
    hopThoai->exec();
    delete hopThoai;
}

void WidgetQuanLySach::xuLyNutThemBanSao() {
    if (!dauSachDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một đầu sách để thêm bản sao!");
        return;
    }
    
    HopThoaiThemBanSao *hopThoai = new HopThoaiThemBanSao(QString::fromStdString(dauSachDangChon->ISBN), this);
    
    connect(hopThoai, &HopThoaiThemBanSao::banSaoDaThem, this, [this]() {
        taiLaiDanhSach();
        emit thongBaoCapNhat("Đã thêm bản sao thành công!");
    });
    
    hopThoai->exec();
    delete hopThoai;
}

void WidgetQuanLySach::xuLyNutXoaBanSao() {}

void WidgetQuanLySach::xuLyNutXemChiTiet() {
    if (!dauSachDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một đầu sách để xem chi tiết!");
        return;
    }
    
    ManHinhChiTietSach *dialog = new ManHinhChiTietSach(dauSachDangChon, this);
    
    connect(dialog, &ManHinhChiTietSach::sachDaCapNhat, this, [this]() {
        taiLaiDanhSach();
        emit thongBaoCapNhat("Đã cập nhật thông tin bản sao!");
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
            QString trangThaiChon = hopChonTrangThai->currentText();
            QString trangThaiDong = m->data(m->index(i, 7), Qt::DisplayRole).toString();
            bool khop = false;
            if (trangThaiChon == "Sẵn sàng" && trangThaiDong.contains("Có sẵn")) khop = true;
            else if (trangThaiChon == "Đang mượn hết" && trangThaiDong.contains("Đang mượn")) khop = true;
            else if (trangThaiChon == "Đã thanh lý" && trangThaiDong.contains("Đã thanh lý")) khop = true;
            if (!khop) hienThi = false;
        }
        bangSach->setRowHidden(i, !hienThi);
        if (hienThi) ++soKetQua;
    }
    
    // Hiển thị kết quả
    if (tuKhoaLowerCase.isEmpty() && hopChonTheLoai->currentIndex() == 0 && hopChonTrangThai->currentIndex() == 0) {
        SafeGui::safeSetText(nhanKetQuaTimKiem, "");
    } else {
        SafeGui::safeSetText(nhanKetQuaTimKiem, QString("📋 Tìm thấy <b>%1</b> kết quả").arg(soKetQua));
    }
}

void WidgetQuanLySach::xuLyLocTheLoai(int) {
    xuLyTimKiem(oNhapTimKiem->text());
}

void WidgetQuanLySach::xuLyLocTrangThai(int) {
    xuLyTimKiem(oNhapTimKiem->text());
}

void WidgetQuanLySach::taiDanhSachTheLoai() {
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
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
    // Ask user to choose an Excel file
    QString file = QFileDialog::getOpenFileName(this, tr("Chọn file Excel để nhập"), QDir::currentPath(), tr("Excel Files (*.xlsx);;All Files (*)"));
    if (file.isEmpty()) return;

    // Find the import script in tools/
    // Prefer repository-relative tools/import_from_excel.py
    QString repoTools = QDir(FilePathManager::getRootPath()).filePath("tools/import_from_excel.py");
    QString pythonExe = "python";
    QString scriptToRun = repoTools;
    if (!QFile::exists(scriptToRun)) {
        // fallback: script next to executable
        scriptToRun = QDir(QCoreApplication::applicationDirPath()).filePath("tools/import_from_excel.py");
        if (!QFile::exists(scriptToRun)) {
            QMessageBox::warning(this, tr("Không tìm thấy script"), tr("Không thể tìm thấy file tools/import_from_excel.py"));
            return;
        }
    }

    // Run preview
    QProcess proc;
    QStringList args;
    args << scriptToRun << "--file" << file << "--preview" << "--out" << "tools";
    proc.start(pythonExe, args);
    bool started = proc.waitForStarted(10000);
    if (!started) {
        QMessageBox::critical(this, tr("Lỗi"), tr("Không thể khởi chạy python. Hãy đảm bảo Python có trong PATH."));
        return;
    }
    proc.waitForFinished(-1);
    QString stdoutStr = QString::fromUtf8(proc.readAllStandardOutput());
    QString stderrStr = QString::fromUtf8(proc.readAllStandardError());

    // Build preview dialog
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Kết quả Import (Preview)"));
    dlg.setModal(true);
    QVBoxLayout *lay = new QVBoxLayout(&dlg);

    QLabel *lbl = new QLabel(tr("Kết quả preview từ script import:"));
    lay->addWidget(lbl);

    QTextEdit *out = new QTextEdit();
    out->setReadOnly(true);
    out->setPlainText(stdoutStr + (stderrStr.isEmpty() ? QString() : QString("\n--- ERRORS ---\n") + stderrStr));
    out->setMinimumSize(700, 380);
    lay->addWidget(out);

    QHBoxLayout *btnLay = new QHBoxLayout();
    btnLay->addStretch();
    QPushButton *btnApply = new QPushButton(tr("Áp dụng (ghi file import)"));
    QPushButton *btnOpenPreviews = new QPushButton(tr("Mở file preview"));
    QPushButton *btnClose = new QPushButton(tr("Đóng"));
    btnLay->addWidget(btnOpenPreviews);
    btnLay->addWidget(btnApply);
    btnLay->addWidget(btnClose);
    lay->addLayout(btnLay);

    connect(btnClose, &QPushButton::clicked, &dlg, &QDialog::reject);

    connect(btnOpenPreviews, &QPushButton::clicked, this, [this]() {
        // Open the generated preview CSVs in the OS default viewer
        QString base = QDir(FilePathManager::getRootPath()).filePath("tools");
        QString p1 = QDir(base).filePath("preview_dausach.csv");
        QString p2 = QDir(base).filePath("preview_bansao.csv");
        if (QFile::exists(p1)) QDesktopServices::openUrl(QUrl::fromLocalFile(p1));
        if (QFile::exists(p2)) QDesktopServices::openUrl(QUrl::fromLocalFile(p2));
    });

    connect(btnApply, &QPushButton::clicked, this, [this, &dlg, pythonExe, scriptToRun, file]() {
        // Ask user whether to apply directly into DB
        QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Áp dụng import"),
            tr("Bạn muốn áp dụng trực tiếp vào SQLite DB (recommended)?\n"
               "Chọn Yes để ghi vào database (tạo backup tự động).\n"
               "Chọn No để chỉ ghi các file import vào thư mục data/ (không thay đổi DB)."),
            QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel, QMessageBox::Yes);

        if (reply == QMessageBox::Cancel) return;

        QProcess *applyProc = new QProcess(this);
        QStringList applyArgs;
        if (reply == QMessageBox::Yes) {
            // Apply directly to DB
            applyArgs << scriptToRun << "--file" << file << "--to-db" << "--mode" << "merge";
        } else {
            // Write import files only
            applyArgs << scriptToRun << "--file" << file << "--apply" << "--out" << "data";
        }

        QProgressDialog *progress = new QProgressDialog(tr("Đang áp dụng import..."), QString(), 0, 0, this);
        progress->setWindowModality(Qt::WindowModal);
        progress->setCancelButton(nullptr);
        progress->setMinimumDuration(200);
        progress->setWindowTitle(tr("Vui lòng chờ"));

        connect(applyProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, applyProc, progress, &dlg]() {
            QString outStr = QString::fromUtf8(applyProc->readAllStandardOutput());
            QString errStr = QString::fromUtf8(applyProc->readAllStandardError());
            QString msg = outStr;
            if (!errStr.isEmpty()) msg += "\n--- ERRORS ---\n" + errStr;
            QMessageBox::information(this, tr("Hoàn thành"), msg);
            progress->close();
            applyProc->deleteLater();
            // reload UI data
            this->taiLaiDanhSach();
            dlg.accept();
        });

        applyProc->start(pythonExe, applyArgs);
        if (!applyProc->waitForStarted(10000)) {
            QMessageBox::critical(this, tr("Lỗi"), tr("Không thể khởi chạy python để áp dụng import."));
            applyProc->deleteLater();
            progress->close();
            return;
        }
        progress->show();
    });

    dlg.exec();
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
        SafeGui::safeSetText(nhanThongTinChon, QString("Bản sao: %1 (Có thể mượn: %2)").arg(count).arg(available));
    }

    // Best-effort: if the concrete model exposes a public refresh() method (as in BanSaoTableModel), try to call it.
    if (modelBanSao) {
        // invokeMethod will fail silently if no such invokable method exists; use queued connection to be safe.
        QMetaObject::invokeMethod(modelBanSao, "refresh", Qt::QueuedConnection);
    }
}

QString WidgetQuanLySach::layTrangThaiSach(TrangThaiSach trangThai) const {
    switch(trangThai) {
        case CHO_MUON_DUOC: return "Sẵn sàng";
        case DANG_MUON: return "Đang mượn";
        case THANH_LY: return "Thanh lý";
        default: return "Không rõ";
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
