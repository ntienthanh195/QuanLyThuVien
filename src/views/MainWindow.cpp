#include "views/MainWindow.h"
#include "views/WidgetBangDieuKhien.h"
#include "views/WidgetQuanLySach.h"
#include "views/WidgetQuanLyDocGia.h"
#include "views/WidgetMuonTraSach.h"
#include "views/dialogs/ManHinhGioiThieu.h"
#include "views/dialogs/ManHinhTop10Sach.h"
#include "views/dialogs/ManHinhTop10DocGia.h"
#include "views/dialogs/ManHinhBaoCaoQuaHan.h"
#include "views/dialogs/ManHinhThongKe.h"
#include "views/dialogs/ManHinhBackup.h"
#include "views/dialogs/ManHinhCauHinh.h"
#include "utils/PrintHelper.h"
#include "views/dialogs/ManHinhTop10.h"
#include "views/dialogs/ManHinhQuaHan.h"
#include "views/dialogs/ManHinhDemoPhase1.h"
#include "views/dialogs/ManHinhDemoBang.h"
#include "views/dialogs/ManHinhDemoBang.h"
#include "utils/BackupManager.h"
#include "utils/ResponsiveHelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QCloseEvent>
#include <QLabel>
#include <QFont>
#include <QFile>
#include <QSplitter>
#include <QApplication>
#include <QDebug>
#include "models/QuanLySach.h"
#include "utils/LanguageManager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , stackedWidget(new QStackedWidget(this))
    , homeWidget(nullptr)
    , bangDieuKhien(nullptr)
    , bookWidget(nullptr)
    , readerWidget(nullptr)
    , loanWidget(nullptr)
    , menuSidebar(nullptr)
{
    setWindowTitle("Hệ Thống Quản Lý Thư Viện");
    
    // Deep Dark Theme - use centralized stylesheet
    this->setProperty("class", "MainWindow");
    
    // **RESPONSIVE**: Auto-detect screen size and apply optimal window size
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    QSize recommendedSize = ResponsiveHelper::getRecommendedWindowSize(screenSize);
    QSize minimumSize = ResponsiveHelper::getMinimumWindowSize(screenSize);
    
    setMinimumSize(minimumSize);
    resize(recommendedSize);
    
    // Center window on screen
    ResponsiveHelper::centerWindow(this);
    
    // Debug: Print screen info to console
    qDebug() << "Responsive UI:" << ResponsiveHelper::getScreenInfo();
    
    // Khoi tao cac widget
    bangDieuKhien = new WidgetBangDieuKhien(this);
    bookWidget = new WidgetQuanLySach(this);
    readerWidget = new WidgetQuanLyDocGia(this);
    loanWidget = new WidgetMuonTraSach(this);
    
    // Them cac widget vao stacked widget
    stackedWidget->addWidget(bangDieuKhien);    // Index 0: Dashboard
    stackedWidget->addWidget(bookWidget);       // Index 1: Book Management
    stackedWidget->addWidget(readerWidget);     // Index 2: Reader Management
    stackedWidget->addWidget(loanWidget);       // Index 3: Loan Management
    
    // Tạo layout chính với sidebar
    QSplitter *splitterChinh = new QSplitter(Qt::Horizontal, this);
    splitterChinh->setHandleWidth(1); // Đường phân cách mỏng
    
    // Setup sidebar
    setupSidebar();
    splitterChinh->addWidget(menuSidebar);
    splitterChinh->addWidget(stackedWidget);
    
    // **RESPONSIVE**: Sidebar width scales with window size
    int sidebarWidth = ResponsiveHelper::getScaledWidth(240, screenSize);
    splitterChinh->setStretchFactor(0, 0); // Sidebar cố định
    splitterChinh->setStretchFactor(1, 1); // Content co giãn
    splitterChinh->setSizes(QList<int>() << sidebarWidth << (width() - sidebarWidth));
    
    setCentralWidget(splitterChinh);
    
    // Ket noi signals
    setupConnections();
    
    // Load Print Config
    QSettings settings("HVPG", "QuanLyThuVien");
    PrintHelper::PrintConfig cfg = PrintHelper::getCauHinh();
    cfg.printerReceipt = settings.value("Printer/Receipt", "").toString();
    cfg.printerReport = settings.value("Printer/Report", "").toString();
    cfg.silentPrint = settings.value("Printer/Silent", false).toBool();
    cfg.inMauSac = settings.value("Printer/Color", false).toBool();
    PrintHelper::setCauHinh(cfg);
    
    // Hien thi bang dieu khien
    stackedWidget->setCurrentIndex(0);
    menuSidebar->setCurrentRow(0); // Trigger row 0 (title) - effectively selects nothing or first available?
    // Actually row 0 is title (Index 0). Row 1 is Home.
    // The signal handler ignores row <= 0.
    
    // Connect localization signal
    connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this, &MainWindow::capNhatNgonNgu);
    capNhatNgonNgu(); // Initial set (rebuilds sidebar and sets text)
    menuSidebar->setCurrentRow(1); // Select Home by default
}

MainWindow::~MainWindow() {
    // Widgets se tu dong giai phong vi la con cua MainWindow
}

void MainWindow::setupSidebar() {
    // MASTER PLAN: Sidebar width = 260px, objectName="Sidebar" để CSS nhận
    menuSidebar = new QListWidget(this);
    menuSidebar->setObjectName("Sidebar"); // CSS selector: QListWidget#Sidebar
    
    // **RESPONSIVE**: Sidebar width scales with screen
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    int sidebarWidth = ResponsiveHelper::getScaledWidth(260, screenSize); // Master Plan: 260px
    
    menuSidebar->setFixedWidth(sidebarWidth);
    menuSidebar->setSpacing(2);
    
    // Không set styleSheet ở đây - để modern-dashboard.qss xử lý
    menuSidebar->setFrameShape(QFrame::NoFrame);
    menuSidebar->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    menuSidebar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    // List initialization moved to capNhatNgonNgu() for dynamic localization
    
    // Kết nối signal
    connect(menuSidebar, &QListWidget::currentRowChanged, this, [this](int row) {
        // Bỏ qua separators và tiêu đề (row 0)
        if (row <= 0) return;
        
        // Map row sang chức năng
        // Row 1: Trang Chủ
        // Row 2: Quản Lý Sách
        // Row 3: Quản Lý Học Viên
        // Row 4: Mượn Trả Sách
        // Row 6: Cấu Hình (New)
        // Row 7: Giới Thiệu
        // Row 8: Thoát
        
        switch(row) {
            case 1: // Trang Chủ
                stackedWidget->setCurrentIndex(0);
                if (bangDieuKhien) bangDieuKhien->capNhatThongKe();
                break;
            case 2: // Quản lý sách
                showBookManagement();
                break;
            case 3: // Quản lý độc giả
                showReaderManagement();
                break;
            case 4: // Mượn trả sách
                showLoanManagement();
                break;
            case 6: // Cấu hình
                showSettings();
                break;
            case 7: // Giới thiệu
                showAbout();
                break;
            case 8: // Thoát
                close();
                break;
        }
    });
}

void MainWindow::apDungStylesheet() {
    // Thử load modern stylesheet trước
    QFile modernFile(":/modern-dashboard.qss");
    if (modernFile.open(QFile::ReadOnly | QFile::Text)) {
        QString stylesheet = QString::fromUtf8(modernFile.readAll());
        qobject_cast<QApplication*>(qApp)->setStyleSheet(stylesheet);
        modernFile.close();
        qDebug() << "✅ Loaded modern-dashboard.qss";
        return;
    }
    
    // Fallback về styles.qss
    QFile file(":/styles.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString stylesheet = QString::fromUtf8(file.readAll());
        qobject_cast<QApplication*>(qApp)->setStyleSheet(stylesheet);
        file.close();
        qDebug() << "✅ Loaded styles.qss (fallback)";
    } else {
        qDebug() << "⚠️ No stylesheet loaded";
    }
}

void MainWindow::setupHomeScreen() {
    QVBoxLayout *mainLayout = new QVBoxLayout(homeWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(50, 50, 50, 50);
    
    // Tieu de
    // Tieu de
    lblHomeTitle = new QLabel("HỆ THỐNG QUẢN LÝ THƯ VIỆN", homeWidget);
    QFont titleFont;
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    lblHomeTitle->setFont(titleFont);
    lblHomeTitle->setAlignment(Qt::AlignCenter);
    lblHomeTitle->setStyleSheet("color: #2c3e50; margin-bottom: 30px;");
    
    mainLayout->addWidget(lblHomeTitle);
    mainLayout->addStretch(1);
    
    // Container cho cac nut
    QWidget *buttonContainer = new QWidget(homeWidget);
    QVBoxLayout *buttonLayout = new QVBoxLayout(buttonContainer);
    buttonLayout->setSpacing(15);
    
    // Tao cac nut menu
    btnHomeBooks = createMenuButton("QUẢN LÝ SÁCH", ":/icons/book.png");
    btnHomeBooks->setToolTip("Quản lý danh mục sách trong thư viện (Ctrl+B)");
    btnHomeBooks->setShortcut(QKeySequence("Ctrl+B"));
    
    btnHomeReaders = createMenuButton("QUẢN LÝ HỌC VIÊN", ":/icons/reader.png");
    btnHomeReaders->setToolTip("Quản lý thông tin học viên (Ctrl+R)");
    btnHomeReaders->setShortcut(QKeySequence("Ctrl+R"));
    
    btnHomeLoans = createMenuButton("MƯỢN - TRẢ SÁCH", ":/icons/loan.png");
    btnHomeLoans->setToolTip("Quản lý mượn trả sách (Ctrl+L)");
    btnHomeLoans->setShortcut(QKeySequence("Ctrl+L"));
    
    // Reports section
    btnHomeTop10 = createMenuButton("TOP 10 SÁCH", ":/icons/top10.png");
    btnHomeTop10->setToolTip("Xem 10 sách được mượn nhiều nhất (Ctrl+T)");
    btnHomeTop10->setShortcut(QKeySequence("Ctrl+T"));
    btnHomeTop10->setProperty("class", "MenuButtonOrange");
    
    btnHomeStats = createMenuButton("THỐNG KÊ HỆ THỐNG", ":/icons/stats.png");
    btnHomeStats->setToolTip("Xem thống kê tổng quan hệ thống (Ctrl+S)");
    btnHomeStats->setShortcut(QKeySequence("Ctrl+S"));
    btnHomeStats->setProperty("class", "MenuButtonPurple");
    
    btnHomeBackup = createMenuButton("SAO LƯU DỮ LIỆU", ":/icons/backup.png");
    btnHomeBackup->setToolTip("Sao lưu và phục hồi dữ liệu (Ctrl+K)");
    btnHomeBackup->setShortcut(QKeySequence("Ctrl+K"));
    btnHomeBackup->setProperty("class", "MenuButtonTeal");
    
    btnHomeOverdue = createMenuButton("HỌC VIÊN QUÁ HẠN", ":/icons/overdue.png");
    btnHomeOverdue->setToolTip("Danh sách học viên mượn quá hạn (Ctrl+O)");
    btnHomeOverdue->setShortcut(QKeySequence("Ctrl+O"));
    btnHomeOverdue->setProperty("class", "MenuButtonWarning");
    
    btnHomeAbout = createMenuButton("THÔNG TIN", ":/icons/info.png");
    btnHomeAbout->setToolTip("Thông tin ứng dụng (F1)");
    btnHomeAbout->setShortcut(QKeySequence("F1"));
    
    btnHomeExit = createMenuButton("THOÁT", "");
    btnHomeExit->setToolTip("Thoát ứng dụng (Ctrl+Q)");
    btnHomeExit->setShortcut(QKeySequence("Ctrl+Q"));
    
    btnHomeExit->setProperty("class", "MenuButtonDanger");
    
    buttonLayout->addWidget(btnHomeBooks);
    buttonLayout->addWidget(btnHomeReaders);
    buttonLayout->addWidget(btnHomeLoans);
    buttonLayout->addWidget(btnHomeTop10);
    buttonLayout->addWidget(btnHomeStats);
    buttonLayout->addWidget(btnHomeBackup);
    buttonLayout->addWidget(btnHomeOverdue);
    buttonLayout->addWidget(btnHomeAbout);
    buttonLayout->addWidget(btnHomeExit);
    
    buttonContainer->setMaximumWidth(600);
    
    mainLayout->addWidget(buttonContainer, 0, Qt::AlignCenter);
    mainLayout->addStretch(2);
    
    // Ket noi signals
    connect(btnHomeBooks, &QPushButton::clicked, this, &MainWindow::showBookManagement);
    connect(btnHomeReaders, &QPushButton::clicked, this, &MainWindow::showReaderManagement);
    connect(btnHomeLoans, &QPushButton::clicked, this, &MainWindow::showLoanManagement);
    connect(btnHomeTop10, &QPushButton::clicked, this, &MainWindow::showTop10Report);
    connect(btnHomeStats, &QPushButton::clicked, this, &MainWindow::showStatistics);
    connect(btnHomeBackup, &QPushButton::clicked, this, &MainWindow::showBackupDialog);
    connect(btnHomeOverdue, &QPushButton::clicked, this, &MainWindow::showOverdueReport);
    connect(btnHomeAbout, &QPushButton::clicked, this, &MainWindow::showAbout);
    connect(btnHomeExit, &QPushButton::clicked, this, &QWidget::close);
}

QPushButton* MainWindow::createMenuButton(const QString &text, const QString &iconPath) {
    QPushButton *button = new QPushButton(text, homeWidget);
    if (!iconPath.isEmpty()) {
        button->setIcon(QIcon(iconPath));
        button->setIconSize(QSize(32, 32));
    }
    button->setMinimumHeight(60);
    button->setMinimumWidth(500);
    
    QFont font;
    font.setPointSize(14);
    font.setBold(true);
    button->setFont(font);
    
    button->setProperty("class", "MenuButton");
    
    return button;
}

void MainWindow::setupConnections() {
    // Ket noi nut "Quay lai" tu cac widget
    connect(bookWidget, &WidgetQuanLySach::backToHome, this, &MainWindow::returnToHome);
    connect(readerWidget, &WidgetQuanLyDocGia::backToHome, this, &MainWindow::returnToHome);
    connect(loanWidget, &WidgetMuonTraSach::backToHome, this, &MainWindow::returnToHome);
    
    // Kết nối signals từ BangDieuKhien
    connect(bangDieuKhien, &WidgetBangDieuKhien::yeuCauXemChiTiet, this, [this](const QString &loai) {
        if (loai == "sachQuaHan") {
            showOverdueReport();
        } else if (loai == "theSapHetHan") {
            // Hiển thị dialog thẻ sắp hết hạn
            QMessageBox::information(this, "Thẻ Sắp Hết Hạn", 
                "Chức năng xem chi tiết thẻ sắp hết hạn đang được phát triển.");
        }
    });
    
    connect(bangDieuKhien, &WidgetBangDieuKhien::yeuCauThemSachMoi, this, [this]() {
        showBookManagement();
        // Trigger thêm sách trong WidgetQuanLySach
    });
    
    connect(bangDieuKhien, &WidgetBangDieuKhien::yeuCauDangKyDocGia, this, [this]() {
        showReaderManagement();
        // Trigger thêm học viên trong WidgetQuanLyDocGia
    });
    
    connect(bangDieuKhien, &WidgetBangDieuKhien::yeuCauTaoPhieuMuon, this, [this]() {
        showLoanManagement();
    });
    
    connect(bangDieuKhien, &WidgetBangDieuKhien::yeuCauSaoLuu, this, &MainWindow::showBackupDialog);
    
    connect(bangDieuKhien, &WidgetBangDieuKhien::yeuCauXemThongTin, this, &MainWindow::showAbout);
    
    connect(bangDieuKhien, &WidgetBangDieuKhien::yeuCauMoDemo, this, &MainWindow::showDemoPhase1);
    connect(bangDieuKhien, &WidgetBangDieuKhien::yeuCauMoDemoBang, this, &MainWindow::showDemoBang);
    
    // Connect Top 10 signals
    connect(bangDieuKhien, &WidgetBangDieuKhien::yeuCauXemTop10Sach, this, &MainWindow::showTop10Report);
    connect(bangDieuKhien, &WidgetBangDieuKhien::yeuCauXemTop10DocGia, this, [this]() {
        ManHinhTop10DocGia dialog(this);
        dialog.exec();
    });
}

void MainWindow::showBookManagement() {
    stackedWidget->setCurrentIndex(1);
    menuSidebar->setCurrentRow(2);
    bookWidget->refreshData();
}
void MainWindow::showReaderManagement() {
    stackedWidget->setCurrentIndex(2);
    menuSidebar->setCurrentRow(3);
    readerWidget->refreshData();
}

void MainWindow::showLoanManagement() {
    stackedWidget->setCurrentIndex(3);
    menuSidebar->setCurrentRow(4);
    loanWidget->refreshData();
}

void MainWindow::showAbout() {
    ManHinhGioiThieu dialog(this);
    dialog.exec();
}

void MainWindow::showSettings() {
    ManHinhCauHinh dialog(this);
    dialog.exec();
}

void MainWindow::showTop10Report() {
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    ManHinhTop10 dialog(dsDauSach, soLuongDauSach, this);
    dialog.exec();
}

void MainWindow::showStatistics() {
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    extern PTRDG rootDocGia;
    
    ManHinhThongKe dialog(dsDauSach, soLuongDauSach, rootDocGia, this);
    dialog.exec();
}

void MainWindow::showBackupDialog() {
    ManHinhBackup dialog(this);
    dialog.exec();
}

void MainWindow::showDemoPhase1() {
    ManHinhDemoPhase1 dialog(this);
    dialog.exec();
}

void MainWindow::showDemoBang() {
    ManHinhDemoBang dialog(this);
    dialog.exec();
}

void MainWindow::showOverdueReport() {
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    extern PTRDG rootDocGia;
    
    ManHinhQuaHan dialog(rootDocGia, dsDauSach, soLuongDauSach, this);
    dialog.exec();
}

void MainWindow::returnToHome() {
    stackedWidget->setCurrentIndex(0);
    menuSidebar->setCurrentRow(1);
    if (bangDieuKhien) {
        bangDieuKhien->capNhatThongKe();
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Kiểm tra nếu có thay đổi chưa lưu
    if (KiemTraDuLieuThayDoi()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Xác nhận thoát",
            "Dữ liệu đã thay đổi sẽ được lưu tự động.\n\n"
            "Bạn có muốn tạo backup trước khi thoát không?",
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
        );
        
        if (reply == QMessageBox::Yes) {
            // Tạo backup trước khi thoát
            std::string loi = TaoBackup();
            if (loi.empty()) {
                QMessageBox::information(this, "Backup Thành Công",
                    "Đã tạo backup dữ liệu thành công!\n\n"
                    "Hệ thống sẽ thoát.");
            } else {
                QMessageBox::warning(this, "Backup Thất Bại",
                    QString("Không thể tạo backup:\n%1\n\n"
                           "Hệ thống vẫn sẽ thoát.").arg(QString::fromStdString(loi)));
            }
            event->accept();
        } else if (reply == QMessageBox::No) {
            // Thoát không backup
            event->accept();
        } else {
            // Cancel - không thoát
            event->ignore();
        }
    } else {
        // Không có thay đổi, hỏi có muốn backup không
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Thoát Ứng Dụng",
            "Bạn có muốn tạo backup trước khi thoát không?",
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
        );
        
        if (reply == QMessageBox::Yes) {
            std::string loi = TaoBackup();
            if (loi.empty()) {
                QMessageBox::information(this, "Backup Thành Công",
                    "Đã tạo backup dữ liệu thành công!");
            }
            event->accept();
        } else if (reply == QMessageBox::No) {
            event->accept();
        } else {
            event->ignore();
        }
    }
}

// ============ HELPER FUNCTION - Modern UI Status Badge ============
QWidget* MainWindow::TaoTagTrangThai(int trangThai) {
    QLabel* label = new QLabel();
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-weight: 600; font-size: 11px; border-radius: 6px; padding: 4px 10px;");

    if (trangThai == 0) { // CHO_MUON_DUOC
        label->setText("● Có sẵn");
        label->setStyleSheet(label->styleSheet() + "background-color: #D1FAE5; color: #15803D; border: 1px solid #BBF7D0;");
    } else if (trangThai == 1) { // DANG_MUON
        label->setText("● Đang mượn");
        label->setStyleSheet(label->styleSheet() + "background-color: #FEF3C7; color: #B45309; border: 1px solid #FDE68A;");
    } else if (trangThai == 2) { // THANH_LY
        label->setText("● Thanh lý");
        label->setStyleSheet(label->styleSheet() + "background-color: #F1F5F9; color: #475569; border: 1px solid #E2E8F0;");
    } else { // Quá hạn hoặc lỗi
        label->setText("● Quá hạn");
        label->setStyleSheet(label->styleSheet() + "background-color: #FEE2E2; color: #B91C1C; border: 1px solid #FECACA;");
    }

    // Container để căn giữa
    QWidget* container = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    return container;
}

// ============ GLOBAL BARCODE SCANNER ============
#include <QKeyEvent>
#include <QDateTime>

void MainWindow::keyPressEvent(QKeyEvent *event) {
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    
    // Check if input is targeting a text field?
    // If user is typing in a search box, we might interfere.
    // Heuristic: Barcodes are typed extremely fast (<60ms per char).
    // Humans type >100ms per char usually.
    
    if (now - lastKeyTime > 100) {
        // Gap too long, reset buffer (likely manual typing or start of scan)
        barcodeBuffer.clear();
    }
    lastKeyTime = now;
    
    // Handle Enter Key (End of Barcode)
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (!barcodeBuffer.isEmpty() && barcodeBuffer.length() >= 2) { 
            // Valid barcode detected (at least 2 chars, e.g. ID '1')
            qDebug() << "Global Scanner Detected:" << barcodeBuffer;
            
            // Switch to Loan Tab logic
            // 1. Show Loan Widget if not active
            if (stackedWidget->currentIndex() != 3) {
                showLoanManagement(); // Switch tab
            }
            
            // 2. Pass data to Loan Widget
            if (loanWidget) {
                loanWidget->handleGlobalScan(barcodeBuffer);
            }
            
            barcodeBuffer.clear();
            event->accept(); // Consume event so it doesn't trigger buttons
            return;
        }
        barcodeBuffer.clear();
    } else {
        // Accumulate printable keys
        if (event->text().length() > 0) {
            barcodeBuffer.append(event->text());
        }
    }
    
    // Pass to default handler (propagation)
    QMainWindow::keyPressEvent(event);
}

void MainWindow::capNhatNgonNgu() {
    //! [1] Window Title & App Title
    setWindowTitle(LanguageManager::instance().getText("APP_WINDOW_TITLE"));
    if (lblHomeTitle) lblHomeTitle->setText(LanguageManager::instance().getText("APP_TITLE"));

    //! [2] Rebuild Sidebar
    if (menuSidebar) {
        menuSidebar->clear();
        
        // Responsive size
        ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
        
        // Row 0: Title
        QLabel *tieuDe = new QLabel(LanguageManager::instance().getText("SIDEBAR_TITLE"), menuSidebar);
        QFont fontTieuDe;
        int titleFontSize = ResponsiveHelper::getScaledFontSize(18, screenSize);
        fontTieuDe.setPointSize(titleFontSize);
        fontTieuDe.setBold(true);
        fontTieuDe.setLetterSpacing(QFont::PercentageSpacing, 102);
        tieuDe->setFont(fontTieuDe);
        tieuDe->setAlignment(Qt::AlignCenter);
        tieuDe->setProperty("class", "SidebarTitle");
        
        QListWidgetItem *itemTieuDe = new QListWidgetItem(menuSidebar);
        int titleHeight = ResponsiveHelper::getScaledHeight(60, screenSize);
        itemTieuDe->setSizeHint(QSize(0, titleHeight));
        itemTieuDe->setFlags(Qt::NoItemFlags);
        menuSidebar->setItemWidget(itemTieuDe, tieuDe);
        
        // Menu Items Structure (Must match rows in setupConnections)
        QStringList danhSachMenu = {
            LanguageManager::instance().getText("MENU_HOME"),        // Row 1
            LanguageManager::instance().getText("MENU_BOOKS"),       // Row 2
            LanguageManager::instance().getText("MENU_READERS"),     // Row 3
            LanguageManager::instance().getText("MENU_LOANS"),       // Row 4
            "",                                                      // Row 5 (Separator)
            LanguageManager::instance().getText("MENU_SETTINGS"),    // Row 6
            LanguageManager::instance().getText("MENU_ABOUT"),       // Row 7
            LanguageManager::instance().getText("MENU_EXIT")         // Row 8
        };
        
        for (const QString &menu : danhSachMenu) {
            QListWidgetItem *item = new QListWidgetItem(menu, menuSidebar);
            item->setSizeHint(QSize(0, 46));
            
            if (menu.isEmpty()) {
                item->setFlags(Qt::NoItemFlags);
                item->setBackground(QColor("#F3F4F6"));
                item->setSizeHint(QSize(0, 1));
            } else {
                item->setFont(QFont("Segoe UI", 13, QFont::Normal));
            }
        }
        
        // Bottom Credit
        QListWidgetItem *itemBottom = new QListWidgetItem(menuSidebar);
        itemBottom->setSizeHint(QSize(0, 80));
        itemBottom->setFlags(Qt::NoItemFlags);
        QWidget *bottomWidget = new QWidget(menuSidebar);
        QVBoxLayout *bottomLayout = new QVBoxLayout(bottomWidget);
        bottomLayout->setContentsMargins(8, 0, 8, 8);
        bottomLayout->setSpacing(0);
        bottomLayout->addStretch();
        
        QLabel *hiddenCredit = new QLabel("CHĂM RỐCH THI", bottomWidget);
        hiddenCredit->setStyleSheet("color: transparent; font-size:1px; line-height:1px; margin:0;");
        hiddenCredit->setAlignment(Qt::AlignCenter);
        bottomLayout->addWidget(hiddenCredit);
        
        menuSidebar->setItemWidget(itemBottom, bottomWidget);
    }
    
    //! [3] Update Home Buttons
    if (btnHomeBooks) btnHomeBooks->setText(LanguageManager::instance().getText("BTN_MENU_BOOKS"));
    if (btnHomeReaders) btnHomeReaders->setText(LanguageManager::instance().getText("BTN_MENU_READERS"));
    if (btnHomeLoans) btnHomeLoans->setText(LanguageManager::instance().getText("BTN_MENU_LOANS"));
    if (btnHomeTop10) btnHomeTop10->setText(LanguageManager::instance().getText("BTN_MENU_TOP10"));
    if (btnHomeStats) btnHomeStats->setText(LanguageManager::instance().getText("BTN_MENU_STATS"));
    if (btnHomeBackup) btnHomeBackup->setText(LanguageManager::instance().getText("BTN_MENU_BACKUP"));
    if (btnHomeOverdue) btnHomeOverdue->setText(LanguageManager::instance().getText("BTN_MENU_OVERDUE"));
    if (btnHomeAbout) btnHomeAbout->setText(LanguageManager::instance().getText("BTN_MENU_ABOUT"));
    if (btnHomeExit) btnHomeExit->setText(LanguageManager::instance().getText("BTN_MENU_EXIT"));
}
