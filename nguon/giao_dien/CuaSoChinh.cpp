/**
 * @file CuaSoChinh.cpp
 * @path nguon/giao_dien/CuaSoChinh.cpp
 * @description Lớp Cửa sổ chính (MainWindow) của ứng dụng Library Management V2.
 * Quản lý thanh điều hướng (Sidebar), khu vực nội dung (StackedWidget) và các hiệu ứng chuyển trang.
 * @project Library Management System (QuanLyThuVien)
 */
#include "giao_dien/CuaSoChinh.h"

#include "views/dialogs/ManHinhGioiThieu.h" // Custom About Dialog
#include "giao_dien/WidgetQuanLyNguoiDung.h" // Include header
#include "giao_dien/BangDieuKhien.h"
#include "giao_dien/WidgetQuanLySach.h"
#include "giao_dien/WidgetQuanLyDocGia.h"
#include "giao_dien/WidgetMuonTraSach.h"
#include "giao_dien/WidgetMuonTraSach.h"
#include "giao_dien/WidgetQuanLyNguoiDung.h" // New
#include "hop_thoai/HopThoaiCaiDat.h"    // Legacy Settings Dialog (Restored)
#include "hop_thoai/ManHinhQuanLyIn.h" // Unified Print Dialog

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QMessageBox>
#include <QCloseEvent>
#include <QTimer>
#include <QApplication>
#include <QScreen>
#include <QListWidgetItem>
#include <QIcon>
#include <QDebug>
#include <QPropertyAnimation>
#include "utils/LanguageManager.h"

// Include backend
#include "models/QuanLySach.h"
#include "models/DocGia.h"
#include "models/ThaoTacFile.h"
#include "models/UserManager.h"

// Include helpers
#include "utils/ResponsiveHelper.h"
#include "utils/AnimationHelper.h"
#include "utils/AutoSaveManager.h"
#include "utils/FilePathManager.h"
#include "utils/EventBus.h"
#include <QDir>
#include <QStandardPaths>
#include "utils/BackupManager.h" // Backup support
#include "utils/ClipboardHelper.h" // Global copy/paste support
#include "utils/GlobalScannerFilter.h" // Global scanner support

CuaSoChinh::CuaSoChinh(QWidget *cha) 
    : QMainWindow(cha),
      boChuyenManHinh(nullptr),
      widgetManHinhChinh(nullptr),
      bangDieuKhien(nullptr),
      widgetSach(nullptr),
      widgetDocGia(nullptr),
      widgetMuonTra(nullptr),
      widgetNguoiDung(nullptr),
      khungMenuBen(nullptr),
      danhSachMenu(nullptr),
      nutToggleSidebar(nullptr),
      sidebarDangMoRong(true),
      sidebarWidthMax(280),
      sidebarWidthMin(70),
      autoSaveManager(nullptr),
      chiSoManHinhHienTai(0)
{
    // Log thông tin màn hình
    qDebug() << "=== KHỞI TẠO HỆ THỐNG QUẢN LÝ THƯ VIỆN V2 ===";
    qDebug() << ResponsiveHelper::getScreenInfo();

    // 1. Run Archive Auto-Cleanup on startup (Monthly/Yearly Zip)
    AutoArchiveBackups();
    
    // 2. Setup Periodic Backup Timer (30 mins = 1800000 ms)
    backupTimer = new QTimer(this);
    connect(backupTimer, &QTimer::timeout, this, &CuaSoChinh::onAutoBackup);
    backupTimer->start(1800000); 
    qDebug() << "Started Periodic Backup Timer (30 mins)";
    
    // Thiết lập cửa sổ chính với responsive
    setWindowTitle("HỆ THỐNG QUẢN LÝ THƯ VIỆN");
    
    // Thiết lập window có thể resize và tự động điều chỉnh
    ResponsiveHelper::setupResizableWindow(this);
    
    // Khôi phục kích thước cửa sổ từ lần chạy trước (hoặc auto-detect lần đầu)
    ResponsiveHelper::restoreWindowGeometry(this);
    // Áp dụng global font scaling dựa trên kích thước vật lý (10" - 17")
    ResponsiveHelper::applyGlobalFontScaling(qApp);
    
    // Thiết lập giao diện
    thieLapGiaoDien();
    thieLapMenuBen();
    thieLapKetNoi();
    apDungKieuMau();
    
    // Thiết lập auto-save
    thieLapAutoSave();
    
    // Hiển thị màn hình chính mặc định với animation
    hienThiManHinhChinh();
    
    // GỌI capNhatThongKe SAU KHI tất cả đã sẵn sàng
    QTimer::singleShot(100, bangDieuKhien, &BangDieuKhien::capNhatThongKe);
    
    // Setup Global Copy/Paste
    ClipboardHelper::setupGlobalCopyPaste(this);

    // Cập nhật thanh trạng thái
    capNhatThanhTrangThai("Hệ thống sẵn sàng - " + ResponsiveHelper::getScreenInfo());
    
    qDebug() << "Kích thước cửa sổ:" << size();
    qDebug() << "========================================";
}

CuaSoChinh::~CuaSoChinh() {
    // Qt tự động xóa các widget con
}

void CuaSoChinh::thieLapGiaoDien() {
    // Lấy screen size và tính toán responsive
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    
    // Widget trung tâm
    QWidget *widgetTrungTam = new QWidget(this);
    widgetTrungTam->setObjectName("CentralWidget");
    setCentralWidget(widgetTrungTam);
    
    // Layout chính: Menu bên trái + Nội dung bên phải
    QHBoxLayout *boTriChinh = new QHBoxLayout(widgetTrungTam);
    boTriChinh->setContentsMargins(0, 0, 0, 0);
    boTriChinh->setSpacing(0);
    
    // Khung menu bên trái - responsive width với khả năng thu gọn
    sidebarWidthMax = ResponsiveHelper::getScaledWidth(280, screenSize);
    sidebarWidthMin = ResponsiveHelper::getScaledWidth(80, screenSize); // Increased from 70 to 80 for visibility
    khungMenuBen = new QWidget();
    khungMenuBen->setObjectName("SidebarFrame");
    khungMenuBen->setFixedWidth(sidebarWidthMax);
    boTriChinh->addWidget(khungMenuBen);
    
    // StackedWidget để chuyển đổi các màn hình
    boChuyenManHinh = new QStackedWidget();
    boChuyenManHinh->setObjectName("MainStack");
    boTriChinh->addWidget(boChuyenManHinh, 1);
    
    // Tạo các widget màn hình - WRAP TRONG TRY-CATCH
    qDebug() << "Bắt đầu khởi tạo widgets...";
    
    try {
        widgetManHinhChinh = new QWidget();
        qDebug() << "✓ widgetManHinhChinh";
        
        bangDieuKhien = new BangDieuKhien();
        qDebug() << "✓ bangDieuKhien";
        
        // Defer creation of heavy management widgets to on-demand (lazy load)
        widgetSach = nullptr;
        widgetDocGia = nullptr;
        widgetSach = nullptr;
        widgetDocGia = nullptr;
        widgetMuonTra = nullptr;
        widgetNguoiDung = nullptr; // Initialize to nullptr
        
    } catch (const std::exception& e) {
        qCritical() << "LỖI khi khởi tạo widgets:" << e.what();
        throw;
    } catch (...) {
        qCritical() << "LỖI không xác định khi khởi tạo widgets";
        throw;
    }
    
    qDebug() << "Hoàn thành khởi tạo widgets";

    // Setup a central filesystem watcher to detect external changes
    fsWatcher = new QFileSystemWatcher(this);
    fsDebounceTimer = new QTimer(this);
    fsDebounceTimer->setInterval(700);
    fsDebounceTimer->setSingleShot(true);

    // Paths to watch: prefer `data/` (managed by FilePathManager) but also watch legacy `files/`
    QString appRoot = FilePathManager::getRootPath();
    QString dataDir = FilePathManager::getFilesDirectory();
    QString legacyFiles = appRoot + "/files";

    // Ensure directories exist before adding
    if (!dataDir.isEmpty()) {
        QDir d(dataDir);
        if (d.exists()) fsWatcher->addPath(d.absolutePath());
    }
    QDir legacyDir(legacyFiles);
    if (legacyDir.exists()) fsWatcher->addPath(legacyDir.absolutePath());

    // Also watch the DB file if present under data/ (data/library.db)
    QString dbPath = dataDir + "/files/library.db";
    QFileInfo fi(dbPath);
    if (fi.exists()) fsWatcher->addPath(fi.absoluteFilePath());

    // When the watcher reports changes, start debounce timer
    connect(fsWatcher, &QFileSystemWatcher::directoryChanged, this, [this](const QString& path){
        Q_UNUSED(path);
        fsDebounceTimer->start();
    });
    connect(fsWatcher, &QFileSystemWatcher::fileChanged, this, [this](const QString& path){
        Q_UNUSED(path);
        fsDebounceTimer->start();
    });

    // On debounce timeout, emit a general dataChanged event so all widgets can refresh
    connect(fsDebounceTimer, &QTimer::timeout, this, [this](){
        qDebug() << "Filesystem change detected (debounced). Emitting EventBus::dataChanged";
        try { EventBus::instance().dataChanged(); } catch(...) {}
        // Also update dashboard/status
        if (bangDieuKhien) bangDieuKhien->capNhatRealtimeTuWidget();
    });

    // Global realtime wiring: update widgets/dashboard when EventBus emits changes
    connect(&EventBus::instance(), &EventBus::docGiaAdded, this, [this](int){
        qDebug() << "EventBus: docGiaAdded received";
        if (widgetDocGia) widgetDocGia->taiLaiDanhSach();
        if (bangDieuKhien) bangDieuKhien->capNhatRealtimeTuWidget();
    });
    connect(&EventBus::instance(), &EventBus::docGiaUpdated, this, [this](int){
        qDebug() << "EventBus: docGiaUpdated received";
        if (widgetDocGia) widgetDocGia->taiLaiDanhSach();
        if (bangDieuKhien) bangDieuKhien->capNhatRealtimeTuWidget();
    });
    connect(&EventBus::instance(), &EventBus::docGiaRemoved, this, [this](int){
        qDebug() << "EventBus: docGiaRemoved received";
        if (widgetDocGia) widgetDocGia->taiLaiDanhSach();
        if (bangDieuKhien) bangDieuKhien->capNhatRealtimeTuWidget();
    });

    connect(&EventBus::instance(), &EventBus::muonTraAdded, this, [this](int, const QString&){
        qDebug() << "EventBus: muonTraAdded received";
        if (widgetMuonTra) widgetMuonTra->lamMoi();
        if (bangDieuKhien) bangDieuKhien->capNhatRealtimeTuWidget();
    });
    connect(&EventBus::instance(), &EventBus::muonTraUpdated, this, [this](int, const QString&){
        qDebug() << "EventBus: muonTraUpdated received";
        if (widgetMuonTra) widgetMuonTra->lamMoi();
        if (bangDieuKhien) bangDieuKhien->capNhatRealtimeTuWidget();
    });
    connect(&EventBus::instance(), &EventBus::muonTraRemoved, this, [this](int, const QString&){
        qDebug() << "EventBus: muonTraRemoved received";
        if (widgetMuonTra) widgetMuonTra->lamMoi();
        if (bangDieuKhien) bangDieuKhien->capNhatRealtimeTuWidget();
    });

    connect(&EventBus::instance(), &EventBus::dauSachAdded, this, [this](const QString&){
        qDebug() << "EventBus: dauSachAdded received";
        if (widgetSach) widgetSach->taiLaiDanhSach();
        if (bangDieuKhien) bangDieuKhien->capNhatRealtimeTuWidget();
    });
    connect(&EventBus::instance(), &EventBus::dauSachUpdated, this, [this](const QString&){
        qDebug() << "EventBus: dauSachUpdated received";
        if (widgetSach) widgetSach->taiLaiDanhSach();
        if (bangDieuKhien) bangDieuKhien->capNhatRealtimeTuWidget();
    });
    connect(&EventBus::instance(), &EventBus::dauSachRemoved, this, [this](const QString&){
        qDebug() << "EventBus: dauSachRemoved received";
        if (widgetSach) widgetSach->taiLaiDanhSach();
        if (bangDieuKhien) bangDieuKhien->capNhatRealtimeTuWidget();
    });

    // General external data change: refresh all major lists and dashboard
    connect(&EventBus::instance(), &EventBus::dataChanged, this, [this](){
        qDebug() << "EventBus: dataChanged received - refreshing all main widgets";
        if (widgetSach) widgetSach->taiLaiDanhSach();
        if (widgetDocGia) widgetDocGia->taiLaiDanhSach();
        if (widgetMuonTra) widgetMuonTra->lamMoi();
        if (bangDieuKhien) bangDieuKhien->capNhatRealtimeTuWidget();
    });
    
    // Thêm vào stack - chỉ thêm dashboard lúc khởi tạo
    boChuyenManHinh->addWidget(bangDieuKhien);      // Index 0
    
    // Thanh trạng thái - responsive padding
    int statusPadding = ResponsiveHelper::getScaledSpacing(5, screenSize);
    QStatusBar *thanhTrangThai = statusBar();
    thanhTrangThai->setStyleSheet(QString(
        "QStatusBar {"
        "    background-color: #1E1E2D;"
        "    color: #B5B5C3;"
        "    border-top: 1px solid #2B2B40;"
        "    padding: %1px 15px;"
        "}"
    ).arg(statusPadding));
    
    nhanNguoiDung = new QLabel(" Quản trị viên");
    thanhTrangThai->addPermanentWidget(nhanNguoiDung);
    
    qDebug() << "Sidebar width:" << sidebarWidthMax << "/" << sidebarWidthMin << "| Status padding:" << statusPadding;
}

void CuaSoChinh::thieLapMenuBen() {
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    
    boTriMenuBen = new QVBoxLayout(khungMenuBen);
    boTriMenuBen->setContentsMargins(0, 0, 0, 0);
    boTriMenuBen->setSpacing(0);
    
    // Logo và tiêu đề - responsive height với gradient background
    int headerHeight = ResponsiveHelper::getScaledHeight(120, screenSize);
    QWidget *widgetTieuDe = new QWidget();
    widgetTieuDe->setFixedHeight(headerHeight);
    widgetTieuDe->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 rgba(244, 164, 96, 0.1), stop:1 #1E1E2D); "
        "border-bottom: 2px solid #FFD700;" // Gold accent border
    );
    
    int headerMargin = ResponsiveHelper::getScaledMargin(15, screenSize);
    QVBoxLayout *boTriTieuDe = new QVBoxLayout(widgetTieuDe);
    boTriTieuDe->setContentsMargins(headerMargin, headerMargin, headerMargin, headerMargin);
    boTriTieuDe->setSpacing(8);
    
    // Nút toggle sidebar ở góc trên
    nutToggleSidebar = new QPushButton("☰");
    nutToggleSidebar->setFixedSize(40, 40);
    nutToggleSidebar->setStyleSheet(
        "QPushButton {"
        "    background-color: rgba(244, 164, 96, 0.15);" // Saffron alpha
        "    color: #F4A460;" // Saffron
        "    border: 1px solid rgba(244, 164, 96, 0.3);"
        "    border-radius: 8px;"
        "    font-size: 20px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(244, 164, 96, 0.25);"
        "    border: 1px solid #F4A460;"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgba(244, 164, 96, 0.35);"
        "}"
    );
    connect(nutToggleSidebar, &QPushButton::clicked, this, &CuaSoChinh::toggleSidebar);
    
    // Layout ngang cho nút toggle
    QHBoxLayout *boTriToggle = new QHBoxLayout();
    boTriToggle->addWidget(nutToggleSidebar);
    
    // LANGUAGE TOGGLE BUTTON (Member)
    btnLang = new QPushButton("🇻🇳");
    btnLang->setFixedSize(40, 40);
    btnLang->setCursor(Qt::PointingHandCursor);
    btnLang->setToolTip("Đổi Ngôn Ngữ (Change Language)");
    btnLang->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    font-size: 20px;"
        "    border: none;"
        "}"
        "QPushButton:hover { background-color: rgba(255,255,255,0.1); border-radius: 5px; }"
    );
    connect(btnLang, &QPushButton::clicked, [this](){
        LanguageManager::instance().cycleLanguage();
        // Update Button Text based on new lang
        Language cur = LanguageManager::instance().currentLanguage();
        if (cur == Language::Vietnamese) btnLang->setText("🇻🇳");
        else if (cur == Language::Khmer) btnLang->setText("🇰🇭");
        else btnLang->setText("🇺🇸");
        
        // Refresh UI
        capNhatNgonNgu();
    });

    boTriToggle->addStretch();
    boTriToggle->addWidget(btnLang);
    boTriTieuDe->addLayout(boTriToggle);
    
    // Logo - Đơn giản hơn, icon đơn sắc - CHUYỂN SANG MÀU CAM NGHỆ (Saffron)
    int logoFontSize = ResponsiveHelper::getScaledFontSize(28, screenSize);
    QLabel *nhanLogo = new QLabel("☸"); // Dharmachakra Symbol
    nhanLogo->setObjectName("LogoLabel");
    nhanLogo->setStyleSheet(QString(
        "font-size: %1px;"
        "color: #F4A460;" // Saffron
        "background: transparent;"
        "font-weight: 300;"
    ).arg(logoFontSize));
    nhanLogo->setAlignment(Qt::AlignCenter);
    
    // Tên hệ thống - Segoe UI, simple - THÊM TÊN HỌC VIỆN SONG NGỮ
    int titleFontSize = ResponsiveHelper::getScaledFontSize(11, screenSize);
    nhanTieuDe = new QLabel(LanguageManager::instance().getText("HEADER_SYSTEM_NAME"));
    nhanTieuDe->setObjectName("TitleLabel");
    nhanTieuDe->setStyleSheet(QString(
        "font-size: %1px;"
        "font-weight: 700;"
        "color: #FFD700;" // Gold
        "text-align: center;"
        "letter-spacing: 0.5px;"
        "background: transparent;"
    ).arg(titleFontSize));
    nhanTieuDe->setAlignment(Qt::AlignCenter);
    nhanTieuDe->setWordWrap(true);
    
    boTriTieuDe->addWidget(nhanLogo);
    boTriTieuDe->addWidget(nhanTieuDe);
    boTriTieuDe->addStretch();
    
    boTriMenuBen->addWidget(widgetTieuDe);
    
    // Danh sách menu - responsive padding và font
    int menuPadding = ResponsiveHelper::getScaledSpacing(15, screenSize);
    int menuFontSize = ResponsiveHelper::getScaledFontSize(14, screenSize);
    
    danhSachMenu = new QListWidget();
    danhSachMenu->setObjectName("MenuList");
    danhSachMenu->setProperty("collapsed", false);
    danhSachMenu->setStyleSheet(QString(
        "QListWidget#MenuList {"
        "    background: #1E1E2D;"
        "    border: none;"
        "    outline: none;"
        "    border-right: 1px solid rgba(244, 164, 96, 0.1);"
        "}"
        "QListWidget#MenuList::item {"
        "    padding: %1px 15px;"
        "    margin: 4px 12px;"
        "    color: #A0A0A0;"
        "    border: none;"
        "    border-radius: 10px;"
        "    font-size: %2px;"
        "    font-weight: 500;"
        "}"
        "QListWidget#MenuList[collapsed=\"true\"]::item {"
        "    padding: 12px 0px;"
        "    margin: 4px 8px;"
        "    border-radius: 8px;"
        "}"
        "QListWidget#MenuList::item:hover {"
        "    background-color: rgba(244, 164, 96, 0.08);"
        "    color: #F4A460;"
        "}"
        "QListWidget#MenuList::item:selected {"
        "    background-color: rgba(244, 164, 96, 0.18);"
        "    color: #FFD700;"
        "    font-weight: 700;"
        "}"
    ).arg(menuPadding).arg(menuFontSize));
    
    // Thêm các mục menu với SVG Icon chính thức (thay vì ký tự Unicode)
    danhSachMenu->setIconSize(QSize(20, 20));

    auto themMuc = [&](const QString &ten, const QString &iconPath) {
        QListWidgetItem *item = new QListWidgetItem(ten);
        
        // Load icon with fallback
        QIcon icon(iconPath);
        if (icon.isNull()) {
            if (iconPath.contains("settings")) icon = QIcon(":/icons/settings.svg"); // Explicit ensure
        }
        item->setIcon(icon);
        danhSachMenu->addItem(item);
    };

    themMuc("  " + LanguageManager::instance().getText("MENU_DASHBOARD"), ":/icons/dashboard.svg");
    themMuc("  " + LanguageManager::instance().getText("MENU_BOOKS"), ":/icons/book.svg");
    themMuc("  " + LanguageManager::instance().getText("MENU_READERS"), ":/icons/user.svg");
    themMuc("  " + LanguageManager::instance().getText("MENU_LOANS"), ":/icons/exchange.svg");
    // Removed In An button as per request
    themMuc("  " + LanguageManager::instance().getText("MENU_USERS"), ":/icons/admin.svg");     // New index: 4 (Admin Icon)
    themMuc("  " + LanguageManager::instance().getText("MENU_SETTINGS"), ":/icons/settings.svg"); // New index: 5 (Settings Icon)
    themMuc("  " + LanguageManager::instance().getText("MENU_ABOUT"), ":/icons/info.svg");   // New index: 6
    
    // Áp dụng phân quyền (RBAC)
    // Index 4: Nhân Sự, 5: Cài Đặt
    bool isAdmin = UserManager::instance().hasPermission("ADMIN");
    bool canManageUsers = isAdmin || UserManager::instance().hasPermission("USER_MANAGE");
    bool canSettings = isAdmin || UserManager::instance().hasPermission("SETTINGS");
    
    danhSachMenu->item(4)->setHidden(!canManageUsers);
    danhSachMenu->item(5)->setHidden(!canSettings);
    
    danhSachMenu->setCurrentRow(0);
    
    boTriMenuBen->addWidget(danhSachMenu, 1);
    
    // Thông tin phiên bản - Minimal
    int versionFontSize = ResponsiveHelper::getScaledFontSize(11, screenSize);
    
    // Container cho phần footer
    QWidget *footerWidget = new QWidget();
    footerWidget->setStyleSheet("background-color: #252526; border-top: 1px solid #3E3E42;");
    QVBoxLayout *footerLayout = new QVBoxLayout(footerWidget);
    footerLayout->setContentsMargins(0, 5, 0, 5);
    footerLayout->setSpacing(0);
    
    // Nút Đăng xuất nhỏ
    QPushButton *btnDangXuat = new QPushButton("Đăng xuất");
    btnDangXuat->setCursor(Qt::PointingHandCursor);
    btnDangXuat->setStyleSheet(QString(
        "QPushButton {"
        "    color: #ef4444;" // Red text
        "    background: transparent;"
        "    border: none;"
        "    font-size: %1px;"
        "    font-weight: 500;"
        "    padding: 2px;"
        "    text-align: center;"
        "}"
        "QPushButton:hover { text-decoration: underline; color: #f87171; }"
    ).arg(versionFontSize));
    connect(btnDangXuat, &QPushButton::clicked, [](){
        // Exit with RESTART code (1000)
        qApp->exit(1000); 
    });
    
    QLabel *nhanPhienBan = new QLabel("Phát triển bởi:\nChăm Rốch Thi");
    nhanPhienBan->setStyleSheet(QString(
        "color: #555555;"
        "font-size: %1px;"
        "padding: 0px 10px 5px 10px;"
        "background: transparent;"
        "border: none;"
    ).arg(versionFontSize - 1)); 
    nhanPhienBan->setAlignment(Qt::AlignCenter);
    nhanPhienBan->setWordWrap(true);
    
    footerLayout->addWidget(btnDangXuat);
    footerLayout->addWidget(nhanPhienBan);
    
    boTriMenuBen->addWidget(footerWidget);
    
    qDebug() << "Menu font size:" << menuFontSize << "| Menu padding:" << menuPadding;
}

void CuaSoChinh::thieLapKetNoi() {
    // Kết nối menu với các màn hình (đã gom chức năng vào tabs)
    connect(danhSachMenu, &QListWidget::currentRowChanged, [this](int hang) {
        switch(hang) {
            case 0: hienThiManHinhChinh(); break;
            case 1: hienThiQuanLySach(); break;
            case 2: hienThiQuanLyDocGia(); break;
            case 3: hienThiMuonTraSach(); break;
            case 4: // Nhan Su
                nhanTieuDe->setText("QUẢN LÝ NHÂN SỰ");
                hienThiQuanLyNguoiDung();
                break;
            case 5: // Cai Dat
                {
                    HopThoaiCaiDat dialog(this);
                    dialog.exec();
                }
                break;
            case 6: hienThiGioiThieu(); break;
        }
    });
    
    // Kết nối signals từ các widget
    connect(bangDieuKhien, &BangDieuKhien::yeuCauChuyenDenQuanLySach, 
            this, &CuaSoChinh::hienThiQuanLySach);
    connect(bangDieuKhien, &BangDieuKhien::yeuCauChuyenDenQuanLyDocGia, 
            this, &CuaSoChinh::hienThiQuanLyDocGia);
    connect(bangDieuKhien, &BangDieuKhien::yeuCauChuyenDenMuonTraSach, 
            this, &CuaSoChinh::hienThiMuonTraSach);
    
    // Kết nối thông báo từ các widget
    // NOTE: Connections for lazy-loaded widgets (Sach, DocGia, MuonTra) 
    // must be made in their respective creation methods (hienThi...), not here.

}

void CuaSoChinh::apDungKieuMau() {
    // Áp dụng Enterprise Theme - Professional, Data-First
    QFile tep(":/enterprise-theme.qss");
    if (tep.open(QFile::ReadOnly | QFile::Text)) {
        QString kieuMau = QString::fromUtf8(tep.readAll());
        qApp->setStyleSheet(kieuMau);
        tep.close();
        qDebug() << "✅ Đã tải Enterprise Theme thành công";
    } else {
        qWarning() << "⚠️ Không tải được Enterprise Theme";
    }
}

void CuaSoChinh::hienThiManHinhChinh() {
    boChuyenManHinh->setCurrentWidget(bangDieuKhien);
    bangDieuKhien->capNhatThongKe();
    chiSoManHinhHienTai = 0;
    capNhatThanhTrangThai("Đang xem: Bảng điều khiển");
}

void CuaSoChinh::hienThiQuanLySach() {
    // Lazy-create the widget if needed to speed up startup
    if (!widgetSach) {
        qDebug() << "Lazy-loading WidgetQuanLySach";
        widgetSach = new WidgetQuanLySach();
        ClipboardHelper::setupGlobalCopyPaste(widgetSach);
        boChuyenManHinh->addWidget(widgetSach);
        
        // Setup Connections
        connect(widgetSach, &WidgetQuanLySach::thongBaoCapNhat, this, [this](const QString &msg) { capNhatThanhTrangThai(msg); });
        connect(widgetSach, &WidgetQuanLySach::duLieuThayDoi, this, &CuaSoChinh::triggerAutoSave);
        connect(widgetSach, &WidgetQuanLySach::duLieuThayDoi, bangDieuKhien, &BangDieuKhien::capNhatRealtimeTuWidget);
    }
    boChuyenManHinh->setCurrentWidget(widgetSach);
    widgetSach->taiLaiDanhSach();
    danhSachMenu->setCurrentRow(1);
    chiSoManHinhHienTai = 1;
    capNhatThanhTrangThai("Đang xem: Quản lý sách");
}

void CuaSoChinh::hienThiQuanLyDocGia() {
    if (!widgetDocGia) {
        qDebug() << "Lazy-loading WidgetQuanLyDocGia";
        widgetDocGia = new WidgetQuanLyDocGia();
        ClipboardHelper::setupGlobalCopyPaste(widgetDocGia);
        boChuyenManHinh->addWidget(widgetDocGia);
        
        // Setup Connections
        connect(widgetDocGia, &WidgetQuanLyDocGia::thongBaoCapNhat, this, [this](const QString &msg) { capNhatThanhTrangThai(msg); });
        connect(widgetDocGia, &WidgetQuanLyDocGia::duLieuThayDoi, this, &CuaSoChinh::triggerAutoSave);
        connect(widgetDocGia, &WidgetQuanLyDocGia::duLieuThayDoi, bangDieuKhien, &BangDieuKhien::capNhatRealtimeTuWidget);
    }
    boChuyenManHinh->setCurrentWidget(widgetDocGia);
    widgetDocGia->taiLaiDanhSach();
    danhSachMenu->setCurrentRow(2);
    chiSoManHinhHienTai = 2;
    capNhatThanhTrangThai("Đang xem: Quản lý độc giả");
}

void CuaSoChinh::hienThiMuonTraSach() {
    if (!widgetMuonTra) {
        qDebug() << "Lazy-loading WidgetMuonTraSach";
        widgetMuonTra = new WidgetMuonTraSach();
        ClipboardHelper::setupGlobalCopyPaste(widgetMuonTra);
        boChuyenManHinh->addWidget(widgetMuonTra);
        
        // DISABLED: GlobalScannerFilter not in use
        /*
        GlobalScannerFilter* globalScanner = qApp->findChild<GlobalScannerFilter*>();
        if (globalScanner) {
            connect(globalScanner, &GlobalScannerFilter::scanDetected,
                    widgetMuonTra, &WidgetMuonTraSach::onGlobalScanReceived);
            qDebug() << "[CONNECT] GlobalScannerFilter -> WidgetMuonTraSach connected!";
        } else {
            qWarning() << "[CONNECT] GlobalScannerFilter not found!";
        }
        */
        
        // Setup Connections
        connect(widgetMuonTra, &WidgetMuonTraSach::thongBaoCapNhat, this, [this](const QString &msg) { capNhatThanhTrangThai(msg); });
        connect(widgetMuonTra, &WidgetMuonTraSach::duLieuThayDoi, this, &CuaSoChinh::triggerAutoSave);
        connect(widgetMuonTra, &WidgetMuonTraSach::duLieuThayDoi, bangDieuKhien, &BangDieuKhien::capNhatRealtimeTuWidget);
        connect(widgetMuonTra, &WidgetMuonTraSach::requestShowMuonTra, this, &CuaSoChinh::hienThiMuonTraSach);
        
        // CROSS-WIDGET REALTIME UPDATES
        connect(widgetMuonTra, &WidgetMuonTraSach::duLieuThayDoi, this, [this](){
            // Refresh Reader List (e.g. Borrow Count column)
            if (widgetDocGia) widgetDocGia->taiLaiDanhSach();
            // Refresh Book List (e.g. Quantity on shelf)
            if (widgetSach) widgetSach->taiLaiDanhSach();
        });
    }
    boChuyenManHinh->setCurrentWidget(widgetMuonTra);
    widgetMuonTra->lamMoi();
    danhSachMenu->setCurrentRow(3);
    chiSoManHinhHienTai = 3;
    capNhatThanhTrangThai("Đang xem: Mượn trả sách");
}
void CuaSoChinh::onAutoBackup() {
    qDebug() << "[AUTO-BACKUP] Triggered by timer...";
    std::string err = AutoBackup();
    if (err.empty()) {
        capNhatThanhTrangThai("✅ Đã sao lưu tự động (30p)", 3000);
        qDebug() << "[AUTO-BACKUP] Success.";
    } else {
        qWarning() << "[AUTO-BACKUP] Failed:" << QString::fromStdString(err);
    }
}

void CuaSoChinh::hienThiQuanLyNguoiDung() {
    if (!widgetNguoiDung) {
        qDebug() << "Lazy-loading WidgetQuanLyNguoiDung";
        widgetNguoiDung = new WidgetQuanLyNguoiDung();
        ClipboardHelper::setupGlobalCopyPaste(widgetNguoiDung);
        boChuyenManHinh->addWidget(widgetNguoiDung);
    }
    boChuyenManHinh->setCurrentWidget(widgetNguoiDung);
    widgetNguoiDung->taiLaiDanhSach();
    
    // Fix: Block signals to prevent recursion, OR just set correct row 4
    danhSachMenu->blockSignals(true);
    danhSachMenu->setCurrentRow(4); 
    danhSachMenu->blockSignals(false);
    
    chiSoManHinhHienTai = 4;
    capNhatThanhTrangThai("Đang xem: Quản lý nhân sự");
}

void CuaSoChinh::hienThiGioiThieu() {
    ManHinhGioiThieu dialog(this);
    dialog.exec();
    
    // Khôi phục highlight menu về tab hiện tại (vì Giới thiệu chỉ là dialog, không phải màn hình chính)
    danhSachMenu->setCurrentRow(chiSoManHinhHienTai);
}

void CuaSoChinh::hienThiIn() {
    PTRDG docGia = nullptr;
    
    // Nếu đang ở màn hình Mượn Trả, truyền độc giả đang chọn
    if (widgetMuonTra && chiSoManHinhHienTai == 3) {
        docGia = widgetMuonTra->property("docGiaHienTai").value<PTRDG>();
    }
    
    // Launch Unified Print Dialog
    ManHinhQuanLyIn *dialog = new ManHinhQuanLyIn(docGia, {}, this);
    dialog->exec();
    delete dialog;
    
    // Khôi phục menu position
    danhSachMenu->setCurrentRow(chiSoManHinhHienTai);
    capNhatThanhTrangThai("Đã đóng trình quản lý in ấn");
}

void CuaSoChinh::hienThiThongKe() {
    QMessageBox::information(this, "Thống Kê", 
        "Chức năng Thống Kê đang được phát triển...\n"
        "Sẽ hiển thị: Biểu đồ cột, tròn, đường về sách, độc giả, mượn trả.");
    danhSachMenu->setCurrentRow(chiSoManHinhHienTai);
}

void CuaSoChinh::hienThiTop10() {
    QMessageBox::information(this, "Top 10", 
        "Chức năng Top 10 đang được phát triển...\n"
        "Sẽ hiển thị: Top 10 sách được mượn nhiều nhất\n"
        "Top 10 độc giả mượn nhiều nhất.");
    danhSachMenu->setCurrentRow(chiSoManHinhHienTai);
}

void CuaSoChinh::hienThiBaoCaoQuaHan() {
    QMessageBox::information(this, "Báo Cáo Quá Hạn", 
        "Chức năng Báo cáo quá hạn đang được phát triển...\n"
        "Sẽ hiển thị: Danh sách độc giả có sách quá hạn\n"
        "Số ngày quá hạn, tiền phạt.");
    danhSachMenu->setCurrentRow(chiSoManHinhHienTai);
}

void CuaSoChinh::hienThiSaoLuu() {
    QMessageBox::information(this, "Sao Lưu", 
        "Chức năng Sao lưu đang được phát triển...\n"
        "Sẽ cho phép: Sao lưu dữ liệu\n"
        "Khôi phục dữ liệu từ bản sao lưu.");
    danhSachMenu->setCurrentRow(chiSoManHinhHienTai);
}

void CuaSoChinh::capNhatThanhTrangThai(const QString &thongDiep, int thoiGianHienThi) {
    statusBar()->showMessage(thongDiep, thoiGianHienThi);
}

void CuaSoChinh::toggleSidebar() {
    sidebarDangMoRong = !sidebarDangMoRong;
    
    int targetWidth = sidebarDangMoRong ? sidebarWidthMax : sidebarWidthMin;
    
    // Web-Standard Easing: OutQuint starts fast and settles very smoothly
    QPropertyAnimation *widthAnim = new QPropertyAnimation(khungMenuBen, "maximumWidth");
    widthAnim->setDuration(300); // 300ms + OutQuint = Perfect Web feel
    widthAnim->setStartValue(khungMenuBen->width());
    widthAnim->setEndValue(targetWidth);
    widthAnim->setEasingCurve(QEasingCurve::OutQuint); 
    
    QLabel *logoLabel = khungMenuBen->findChild<QLabel*>("LogoLabel");
    QLabel *titleLabel = khungMenuBen->findChild<QLabel*>("TitleLabel");
    
    QStringList menuTexts = {
        "  " + LanguageManager::instance().getText("MENU_DASHBOARD"),
        "  " + LanguageManager::instance().getText("MENU_BOOKS"),
        "  " + LanguageManager::instance().getText("MENU_READERS"),
        "  " + LanguageManager::instance().getText("MENU_LOANS"),
        "  " + LanguageManager::instance().getText("MENU_USERS"),
        "  " + LanguageManager::instance().getText("MENU_SETTINGS"),
        "  " + LanguageManager::instance().getText("MENU_ABOUT")
    };

    // Performance: Switch state before animation starts
    danhSachMenu->setProperty("collapsed", !sidebarDangMoRong);
    danhSachMenu->style()->unpolish(danhSachMenu);
    danhSachMenu->style()->polish(danhSachMenu);

    if (sidebarDangMoRong) {
        // When expanding: Start width first, show text halfway for "fade-in" feel
        QTimer::singleShot(150, this, [this, logoLabel, titleLabel, menuTexts]() {
            if (!sidebarDangMoRong) return; // Abort if user toggled back fast
            
            if (logoLabel) logoLabel->show();
            if (titleLabel) titleLabel->show();
            if (btnLang) btnLang->show();
            
            for (int i = 0; i < danhSachMenu->count() && i < menuTexts.size(); ++i) {
               danhSachMenu->item(i)->setText(menuTexts[i]);
            }
        });
        
        connect(widthAnim, &QPropertyAnimation::finished, [this]() {
            khungMenuBen->setMinimumWidth(sidebarWidthMax);
        });
    } else {
        // When collapsing: Hide complex stuff IMMEDIATELY
        if (logoLabel) logoLabel->hide();
        if (titleLabel) titleLabel->hide();
        if (btnLang) btnLang->hide();
        
        for (int i = 0; i < danhSachMenu->count(); ++i) {
             danhSachMenu->item(i)->setText("");
        }
    }
    
    // Smooth Sync: Update min/max together to prevent jitter
    connect(widthAnim, &QPropertyAnimation::valueChanged, [this](const QVariant &value) {
        int w = value.toInt();
        khungMenuBen->setMinimumWidth(w);
    });
    
    widthAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void CuaSoChinh::capNhatNgonNgu() {
    // 1. Update Window Title
    setWindowTitle(LanguageManager::instance().getText("APP_TITLE"));
    if (nhanTieuDe) nhanTieuDe->setText(LanguageManager::instance().getText("APP_TITLE"));

    // 2. Update Side Menu
    if (danhSachMenu && danhSachMenu->count() >= 7) {
        danhSachMenu->item(0)->setText("  " + LanguageManager::instance().getText("MENU_DASHBOARD"));
        danhSachMenu->item(1)->setText("  " + LanguageManager::instance().getText("MENU_BOOKS"));
        danhSachMenu->item(2)->setText("  " + LanguageManager::instance().getText("MENU_READERS"));
        danhSachMenu->item(3)->setText("  " + LanguageManager::instance().getText("MENU_LOAN"));
        danhSachMenu->item(4)->setText("  " + LanguageManager::instance().getText("MENU_USERS"));
        danhSachMenu->item(5)->setText("  " + LanguageManager::instance().getText("MENU_SETTINGS"));
        // Item 6 (About) - leave as is or add to key
    }

    // 3. Update Dashboard if visible
    if (bangDieuKhien) {
        bangDieuKhien->capNhatNgonNgu();
    }
}

QWidget* CuaSoChinh::taoTagTrangThai(int trangThai) {
    QLabel *nhan = new QLabel();
    nhan->setFixedHeight(24);
    nhan->setAlignment(Qt::AlignCenter);
    
    switch(trangThai) {
        case 0: // Cho mượn được
            nhan->setText("✓ Sẵn sàng");
            nhan->setStyleSheet(
                "background-color: #1BC5BD; color: white; "
                "border-radius: 12px; padding: 4px 12px; font-weight: bold;"
            );
            break;
        case 1: // Đang mượn
            nhan->setText("◉ Đang mượn");
            nhan->setStyleSheet(
                "background-color: #FFA800; color: white; "
                "border-radius: 12px; padding: 4px 12px; font-weight: bold;"
            );
            break;
        case 2: // Thanh lý
            nhan->setText("✕ Thanh lý");
            nhan->setStyleSheet(
                "background-color: #F64E60; color: white; "
                "border-radius: 12px; padding: 4px 12px; font-weight: bold;"
            );
            break;
        default:
            nhan->setText("? Không rõ");
            nhan->setStyleSheet(
                "background-color: #6C6C80; color: white; "
                "border-radius: 12px; padding: 4px 12px;"
            );
    }
    
    return nhan;
}

void CuaSoChinh::closeEvent(QCloseEvent *sukien) {
    // Hỏi xác nhận trước khi đóng
    QMessageBox::StandardButton xacNhan = QMessageBox::question(
        this,
        "Xác Nhận Thoát",
        "Bạn có chắc muốn thoát khỏi ứng dụng?\n"
        "Dữ liệu sẽ được tự động lưu.",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (xacNhan == QMessageBox::Yes) {
        // Lưu dữ liệu lần cuối
        capNhatThanhTrangThai("Đang lưu dữ liệu...");
        if (autoSaveManager) {
            autoSaveManager->forceSave();
        }
        
        // Backup database before exit
        capNhatThanhTrangThai("Đang sao lưu Database...");
        AutoBackup();
        
        // Lưu trạng thái cửa sổ (kích thước, vị trí, maximize)
        ResponsiveHelper::saveWindowGeometry(this);
        
        sukien->accept();
    } else {
        sukien->ignore();
    }
}

void CuaSoChinh::resizeEvent(QResizeEvent *sukien) {
    QMainWindow::resizeEvent(sukien);
    
    // Log resize để debug (có thể bỏ sau)
    static int resizeCount = 0;
    if (++resizeCount % 10 == 0) { // Log mỗi 10 lần resize để tránh spam
        qDebug() << "Window resized to:" << size();
    }
}

void CuaSoChinh::thieLapAutoSave() {
    // Tạo auto-save manager
    autoSaveManager = new AutoSaveManager(this);
    
    // Thiết lập callback để lưu tất cả dữ liệu
    autoSaveManager->setSaveCallback([this]() {
        luuTatCaDuLieu();
    });
    
    // Sử dụng chế độ Immediate - lưu ngay khi có thay đổi
    autoSaveManager->setSaveMode(AutoSaveManager::Immediate);
    
    // Kết nối signals
    connect(autoSaveManager, &AutoSaveManager::savingStarted, this, [this]() {
        capNhatThanhTrangThai("💾 Đang tự động lưu dữ liệu...", 1000);
    });
    
    connect(autoSaveManager, &AutoSaveManager::savingCompleted, this, [this]() {
        capNhatThanhTrangThai("✅ Dữ liệu đã được lưu", 2000);
    });
    
    qDebug() << "[AUTO-SAVE] Đã thiết lập auto-save (chế độ: Immediate)";
}

void CuaSoChinh::luuTatCaDuLieu() {
    qDebug() << "[AUTO-SAVE] Dữ liệu đã được đồng bộ với DB. Không cần thao tác lưu file thủ công.";
}

void CuaSoChinh::triggerAutoSave() {
    if (autoSaveManager) {
        autoSaveManager->triggerSave();
    }
}


