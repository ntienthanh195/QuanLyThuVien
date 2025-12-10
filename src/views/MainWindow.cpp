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
    setWindowTitle("🏛️ Hệ Thống Quản Lý Thư Viện");
    
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
    
    // Hien thi bang dieu khien
    stackedWidget->setCurrentIndex(0);
    menuSidebar->setCurrentRow(0);
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
    
    // Thêm tiêu đề với icon chuyên nghiệp - Responsive font
    QLabel *tieuDe = new QLabel("📚 THƯ VIỆN", menuSidebar);
    QFont fontTieuDe;
    int titleFontSize = ResponsiveHelper::getScaledFontSize(18, screenSize);
    fontTieuDe.setPointSize(titleFontSize);
    fontTieuDe.setBold(true);
    fontTieuDe.setLetterSpacing(QFont::PercentageSpacing, 102);
    tieuDe->setFont(fontTieuDe);
    tieuDe->setAlignment(Qt::AlignCenter);
    tieuDe->setProperty("class", "SidebarTitle");
    
    // Tạo widget container cho tiêu đề
    QListWidgetItem *itemTieuDe = new QListWidgetItem(menuSidebar);
    int titleHeight = ResponsiveHelper::getScaledHeight(60, screenSize);
    itemTieuDe->setSizeHint(QSize(0, titleHeight));
    itemTieuDe->setFlags(Qt::NoItemFlags); // Không thể chọn
    menuSidebar->setItemWidget(itemTieuDe, tieuDe);
    
    // Thêm các mục menu với Unicode icons hiện đại (đã gom các chức năng vào tabs)
    QStringList danhSachMenu = {
        "🏠  Trang Chủ",           // Dashboard (có Top 10, Backup, About)
        "📖  Quản Lý Sách",        // Books (có tabs: Danh Sách, Top 10, Thống Kê)
        "👥  Quản Lý Độc Giả",    // Readers (có tabs: Danh Sách, Quá Hạn)
        "🔄  Mượn Trả Sách",       // Loan/Return
        "",                        // Separator
        "ℹ️  Giới Thiệu",         // About
        "❌  Thoát"                // Exit
    };
    
    for (const QString &menu : danhSachMenu) {
        QListWidgetItem *item = new QListWidgetItem(menu, menuSidebar);
        item->setSizeHint(QSize(0, 46));
        
        if (menu.isEmpty()) {
            // Separator
            item->setFlags(Qt::NoItemFlags);
            item->setBackground(QColor("#F3F4F6"));
            item->setSizeHint(QSize(0, 1));
        } else {
            item->setFont(QFont("Segoe UI", 13, QFont::Normal));
        }
    }
    
    // Kết nối signal
    connect(menuSidebar, &QListWidget::currentRowChanged, this, [this](int row) {
        // Bỏ qua separators và tiêu đề (row 0)
        if (row <= 0) return;
        
        // Map row sang chức năng
        // Row 1: Trang Chủ
        // Row 2: Quản Lý Sách
        // Row 3: Quản Lý Độc Giả  
        // Row 4: Mượn Trả Sách
        // Row 5: separator
        // Row 6: Giới Thiệu
        // Row 7: Thoát
        
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
            case 6: // Giới thiệu
                showAbout();
                break;
            case 7: // Thoát
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
    QLabel *titleLabel = new QLabel("HỆ THỐNG QUẢN LÝ THƯ VIỆN", homeWidget);
    QFont titleFont;
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #2c3e50; margin-bottom: 30px;");
    
    mainLayout->addWidget(titleLabel);
    mainLayout->addStretch(1);
    
    // Container cho cac nut
    QWidget *buttonContainer = new QWidget(homeWidget);
    QVBoxLayout *buttonLayout = new QVBoxLayout(buttonContainer);
    buttonLayout->setSpacing(15);
    
    // Tao cac nut menu
    QPushButton *btnBooks = createMenuButton("📚 QUẢN LÝ SÁCH", ":/icons/book.png");
    btnBooks->setToolTip("Quản lý danh mục sách trong thư viện (Ctrl+B)");
    btnBooks->setShortcut(QKeySequence("Ctrl+B"));
    
    QPushButton *btnReaders = createMenuButton("👥 QUẢN LÝ ĐỘC GIẢ", ":/icons/reader.png");
    btnReaders->setToolTip("Quản lý thông tin độc giả (Ctrl+R)");
    btnReaders->setShortcut(QKeySequence("Ctrl+R"));
    
    QPushButton *btnLoans = createMenuButton("📖 MƯỢN - TRẢ SÁCH", ":/icons/loan.png");
    btnLoans->setToolTip("Quản lý mượn trả sách (Ctrl+L)");
    btnLoans->setShortcut(QKeySequence("Ctrl+L"));
    
    // Reports section
    QPushButton *btnTop10 = createMenuButton("🏆 TOP 10 SÁCH", ":/icons/top10.png");
    btnTop10->setToolTip("Xem 10 sách được mượn nhiều nhất (Ctrl+T)");
    btnTop10->setShortcut(QKeySequence("Ctrl+T"));
    btnTop10->setProperty("class", "MenuButtonOrange");
    
    QPushButton *btnStats = createMenuButton("📊 THỐNG KÊ HỆ THỐNG", ":/icons/stats.png");
    btnStats->setToolTip("Xem thống kê tổng quan hệ thống (Ctrl+S)");
    btnStats->setShortcut(QKeySequence("Ctrl+S"));
    btnStats->setProperty("class", "MenuButtonPurple");
    
    QPushButton *btnBackup = createMenuButton("💾 SAO LƯU DỮ LIỆU", ":/icons/backup.png");
    btnBackup->setToolTip("Sao lưu và phục hồi dữ liệu (Ctrl+K)");
    btnBackup->setShortcut(QKeySequence("Ctrl+K"));
    btnBackup->setProperty("class", "MenuButtonTeal");
    
    QPushButton *btnOverdue = createMenuButton("⚠️ ĐỘC GIẢ QUÁ HẠN", ":/icons/overdue.png");
    btnOverdue->setToolTip("Danh sách độc giả mượn quá hạn (Ctrl+O)");
    btnOverdue->setShortcut(QKeySequence("Ctrl+O"));
    btnOverdue->setProperty("class", "MenuButtonWarning");
    
    QPushButton *btnAbout = createMenuButton("ℹ️ THÔNG TIN", ":/icons/info.png");
    btnAbout->setToolTip("Thông tin ứng dụng (F1)");
    btnAbout->setShortcut(QKeySequence("F1"));
    
    QPushButton *btnExit = createMenuButton("🚺 THOÁT", "");
    btnExit->setToolTip("Thoát ứng dụng (Ctrl+Q)");
    btnExit->setShortcut(QKeySequence("Ctrl+Q"));
    
    btnExit->setProperty("class", "MenuButtonDanger");
    
    buttonLayout->addWidget(btnBooks);
    buttonLayout->addWidget(btnReaders);
    buttonLayout->addWidget(btnLoans);
    buttonLayout->addWidget(btnTop10);
    buttonLayout->addWidget(btnStats);
    buttonLayout->addWidget(btnBackup);
    buttonLayout->addWidget(btnOverdue);
    buttonLayout->addWidget(btnAbout);
    buttonLayout->addWidget(btnExit);
    
    buttonContainer->setMaximumWidth(600);
    
    mainLayout->addWidget(buttonContainer, 0, Qt::AlignCenter);
    mainLayout->addStretch(2);
    
    // Ket noi signals
    connect(btnBooks, &QPushButton::clicked, this, &MainWindow::showBookManagement);
    connect(btnReaders, &QPushButton::clicked, this, &MainWindow::showReaderManagement);
    connect(btnLoans, &QPushButton::clicked, this, &MainWindow::showLoanManagement);
    connect(btnTop10, &QPushButton::clicked, this, &MainWindow::showTop10Report);
    connect(btnStats, &QPushButton::clicked, this, &MainWindow::showStatistics);
    connect(btnBackup, &QPushButton::clicked, this, &MainWindow::showBackupDialog);
    connect(btnOverdue, &QPushButton::clicked, this, &MainWindow::showOverdueReport);
    connect(btnAbout, &QPushButton::clicked, this, &MainWindow::showAbout);
    connect(btnExit, &QPushButton::clicked, this, &QWidget::close);
}

QPushButton* MainWindow::createMenuButton(const QString &text, const QString & /*iconPath*/) {
    QPushButton *button = new QPushButton(text, homeWidget);
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
        // Trigger thêm độc giả trong WidgetQuanLyDocGia
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
            "💾 Bạn có muốn tạo backup trước khi thoát không?",
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
        );
        
        if (reply == QMessageBox::Yes) {
            // Tạo backup trước khi thoát
            std::string loi = TaoBackup();
            if (loi.empty()) {
                QMessageBox::information(this, "Backup Thành Công ✅",
                    "Đã tạo backup dữ liệu thành công!\n\n"
                    "Hệ thống sẽ thoát.");
            } else {
                QMessageBox::warning(this, "Backup Thất Bại ⚠️",
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
            "💾 Bạn có muốn tạo backup trước khi thoát không?",
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
        );
        
        if (reply == QMessageBox::Yes) {
            std::string loi = TaoBackup();
            if (loi.empty()) {
                QMessageBox::information(this, "Backup Thành Công ✅",
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
