#include "giao_dien/CuaSoChinh.h"
#include "giao_dien/BangDieuKhien.h"
#include "giao_dien/WidgetQuanLySach.h"
#include "giao_dien/WidgetQuanLyDocGia.h"
#include "giao_dien/WidgetMuonTraSach.h"
#include "hop_thoai/HopThoaiInAn.h"

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

// Include backend
#include "models/QuanLySach.h"
#include "models/DocGia.h"
#include "models/ThaoTacFile.h"

// Include helpers
#include "utils/ResponsiveHelper.h"
#include "utils/AnimationHelper.h"
#include "utils/AutoSaveManager.h"
#include "utils/FilePathManager.h"

CuaSoChinh::CuaSoChinh(QWidget *cha) 
    : QMainWindow(cha),
      boChuyenManHinh(nullptr),
      widgetManHinhChinh(nullptr),
      bangDieuKhien(nullptr),
      widgetSach(nullptr),
      widgetDocGia(nullptr),
      widgetMuonTra(nullptr),
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
    
    // Thiết lập cửa sổ chính với responsive
    setWindowTitle("📚 HỆ THỐNG QUẢN LÝ THƯ VIỆN V2.0");
    
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
    sidebarWidthMin = ResponsiveHelper::getScaledWidth(70, screenSize);
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
        
        widgetSach = new WidgetQuanLySach();
        qDebug() << "✓ widgetSach";
        
        widgetDocGia = new WidgetQuanLyDocGia();
        qDebug() << "✓ widgetDocGia";
        
        widgetMuonTra = new WidgetMuonTraSach();
        qDebug() << "✓ widgetMuonTra";
        
    } catch (const std::exception& e) {
        qCritical() << "LỖI khi khởi tạo widgets:" << e.what();
        throw;
    } catch (...) {
        qCritical() << "LỖI không xác định khi khởi tạo widgets";
        throw;
    }
    
    qDebug() << "Hoàn thành khởi tạo widgets";
    
    // Thêm vào stack
    boChuyenManHinh->addWidget(bangDieuKhien);      // Index 0
    boChuyenManHinh->addWidget(widgetSach);         // Index 1
    boChuyenManHinh->addWidget(widgetDocGia);       // Index 2
    boChuyenManHinh->addWidget(widgetMuonTra);      // Index 3
    
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
    
    nhanNguoiDung = new QLabel("👤 Quản trị viên");
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
        "stop:0 rgba(54, 153, 255, 0.1), stop:1 #1E1E2D); "
        "border-bottom: 2px solid rgba(54, 153, 255, 0.3);"
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
        "    background-color: rgba(54, 153, 255, 0.15);"
        "    color: #3699FF;"
        "    border: 1px solid rgba(54, 153, 255, 0.3);"
        "    border-radius: 8px;"
        "    font-size: 20px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(54, 153, 255, 0.25);"
        "    border: 1px solid #3699FF;"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgba(54, 153, 255, 0.35);"
        "}"
    );
    connect(nutToggleSidebar, &QPushButton::clicked, this, &CuaSoChinh::toggleSidebar);
    
    // Layout ngang cho nút toggle
    QHBoxLayout *boTriToggle = new QHBoxLayout();
    boTriToggle->addWidget(nutToggleSidebar);
    boTriToggle->addStretch();
    boTriTieuDe->addLayout(boTriToggle);
    
    // Logo - Đơn giản hơn, icon đơn sắc
    int logoFontSize = ResponsiveHelper::getScaledFontSize(28, screenSize);
    QLabel *nhanLogo = new QLabel("▣");
    nhanLogo->setObjectName("LogoLabel");
    nhanLogo->setStyleSheet(QString(
        "font-size: %1px;"
        "color: #0078D4;"
        "background: transparent;"
        "font-weight: 300;"
    ).arg(logoFontSize));
    nhanLogo->setAlignment(Qt::AlignCenter);
    
    // Tên hệ thống - Segoe UI, simple
    int titleFontSize = ResponsiveHelper::getScaledFontSize(13, screenSize);
    QLabel *nhanTenHeThong = new QLabel("QUẢN LÝ THƯ VIỆN");
    nhanTenHeThong->setObjectName("TitleLabel");
    nhanTenHeThong->setStyleSheet(QString(
        "font-size: %1px;"
        "font-weight: 600;"
        "color: #CCCCCC;"
        "text-align: center;"
        "letter-spacing: 1px;"
        "background: transparent;"
    ).arg(titleFontSize));
    nhanTenHeThong->setAlignment(Qt::AlignCenter);
    
    boTriTieuDe->addWidget(nhanLogo);
    boTriTieuDe->addWidget(nhanTenHeThong);
    boTriTieuDe->addStretch();
    
    boTriMenuBen->addWidget(widgetTieuDe);
    
    // Danh sách menu - responsive padding và font
    int menuPadding = ResponsiveHelper::getScaledSpacing(15, screenSize);
    int menuFontSize = ResponsiveHelper::getScaledFontSize(14, screenSize);
    
    danhSachMenu = new QListWidget();
    danhSachMenu->setObjectName("MenuList");
    danhSachMenu->setStyleSheet(QString(
        "QListWidget#MenuList {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #1E1E2D, stop:1 #1A1A26);"
        "    border: none;"
        "    outline: none;"
        "    border-right: 1px solid rgba(54, 153, 255, 0.1);"
        "}"
        "QListWidget#MenuList::item {"
        "    padding: %1px 20px;"
        "    margin: 6px 12px;"
        "    color: #9CA3AF;"
        "    border-left: 3px solid transparent;"
        "    border-radius: 12px;"
        "    font-size: %2px;"
        "    font-weight: 600;"
        "}"
        "QListWidget#MenuList::item:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 rgba(54, 153, 255, 0.12), stop:1 rgba(54, 153, 255, 0.05));"
        "    color: #3699FF;"
        "    border-left: 3px solid #3699FF;"
        "}"
        "QListWidget#MenuList::item:selected {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 rgba(54, 153, 255, 0.25), stop:1 rgba(54, 153, 255, 0.15));"
        "    color: #FFFFFF;"
        "    border-left: 3px solid #3699FF;"
        "    font-weight: 800;"
        "}"
    ).arg(menuPadding).arg(menuFontSize));
    
    // Thêm các mục menu với icon đơn sắc (không dùng emoji nhiều màu)
    danhSachMenu->addItem("▣  Trang Chủ");
    danhSachMenu->addItem("▤  Quản Lý Sách");        // Có tabs: Danh Sách, Top 10, Thống Kê
    danhSachMenu->addItem("◉  Quản Lý Độc Giả");    // Có tabs: Danh Sách, Quá Hạn
    danhSachMenu->addItem("⇄  Mượn Trả Sách");
    danhSachMenu->addItem("⎙  In Ấn");
    danhSachMenu->addItem("ⓘ  Giới Thiệu");
    
    danhSachMenu->setCurrentRow(0);
    
    boTriMenuBen->addWidget(danhSachMenu, 1);
    
    // Thông tin phiên bản - Minimal
    int versionFontSize = ResponsiveHelper::getScaledFontSize(11, screenSize);
    int versionPadding = ResponsiveHelper::getScaledSpacing(12, screenSize);
    QLabel *nhanPhienBan = new QLabel("v2.0.0");
    nhanPhienBan->setStyleSheet(QString(
        "color: #555555;"
        "font-size: %1px;"
        "padding: %2px 20px;"
        "background-color: #252526;"
        "border-top: 1px solid #3E3E42;"
    ).arg(versionFontSize).arg(versionPadding));
    nhanPhienBan->setAlignment(Qt::AlignCenter);
    boTriMenuBen->addWidget(nhanPhienBan);
    
    qDebug() << "Menu font size:" << menuFontSize << "| Menu padding:" << menuPadding;
}

void CuaSoChinh::thieLapKetNoi() {
    // Kết nối menu với các màn hình (đã gom chức năng vào tabs)
    connect(danhSachMenu, &QListWidget::currentRowChanged, [this](int hang) {
        switch(hang) {
            case 0: hienThiManHinhChinh(); break;
            case 1: hienThiQuanLySach(); break;      // Tab Thống Kê & Top 10 bên trong
            case 2: hienThiQuanLyDocGia(); break;    // Tab Quá Hạn bên trong
            case 3: hienThiMuonTraSach(); break;
            case 4: hienThiIn(); break;
            case 5: hienThiGioiThieu(); break;
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
    connect(widgetSach, &WidgetQuanLySach::thongBaoCapNhat, 
            this, [this](const QString &msg) { capNhatThanhTrangThai(msg); });
    connect(widgetDocGia, &WidgetQuanLyDocGia::thongBaoCapNhat, 
            this, [this](const QString &msg) { capNhatThanhTrangThai(msg); });
    connect(widgetMuonTra, &WidgetMuonTraSach::thongBaoCapNhat, 
            this, [this](const QString &msg) { capNhatThanhTrangThai(msg); });
    
    // Kết nối auto-save từ các widget
    connect(widgetSach, &WidgetQuanLySach::duLieuThayDoi,
            this, &CuaSoChinh::triggerAutoSave);
    connect(widgetDocGia, &WidgetQuanLyDocGia::duLieuThayDoi,
            this, &CuaSoChinh::triggerAutoSave);
    connect(widgetMuonTra, &WidgetMuonTraSach::duLieuThayDoi,
            this, &CuaSoChinh::triggerAutoSave);
    
    // Kết nối realtime update đến dashboard
    connect(widgetSach, &WidgetQuanLySach::duLieuThayDoi,
            bangDieuKhien, &BangDieuKhien::capNhatRealtimeTuWidget);
    connect(widgetDocGia, &WidgetQuanLyDocGia::duLieuThayDoi,
            bangDieuKhien, &BangDieuKhien::capNhatRealtimeTuWidget);
    connect(widgetMuonTra, &WidgetMuonTraSach::duLieuThayDoi,
            bangDieuKhien, &BangDieuKhien::capNhatRealtimeTuWidget);
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
    boChuyenManHinh->setCurrentWidget(widgetSach);
    widgetSach->taiLaiDanhSach();
    danhSachMenu->setCurrentRow(1);
    chiSoManHinhHienTai = 1;
    capNhatThanhTrangThai("Đang xem: Quản lý sách");
}

void CuaSoChinh::hienThiQuanLyDocGia() {
    boChuyenManHinh->setCurrentWidget(widgetDocGia);
    widgetDocGia->taiLaiDanhSach();
    danhSachMenu->setCurrentRow(2);
    chiSoManHinhHienTai = 2;
    capNhatThanhTrangThai("Đang xem: Quản lý độc giả");
}

void CuaSoChinh::hienThiMuonTraSach() {
    boChuyenManHinh->setCurrentWidget(widgetMuonTra);
    widgetMuonTra->lamMoi();
    danhSachMenu->setCurrentRow(3);
    chiSoManHinhHienTai = 3;
    capNhatThanhTrangThai("Đang xem: Mượn trả sách");
}

void CuaSoChinh::hienThiGioiThieu() {
    QMessageBox::about(this, "Giới Thiệu",
        "<h2>HỆ THỐNG QUẢN LÝ THƯ VIỆN</h2>"
        "<p><b>Phiên bản:</b> 2.0.0</p>"
        "<p><b>Ngày phát hành:</b> Tháng 12, 2025</p>"
        "<hr>"
        "<p><b>Tính năng chính:</b></p>"
        "<ul>"
        "<li>✅ Quản lý đầu sách và bản sao</li>"
        "<li>✅ Quản lý độc giả (AVL Tree)</li>"
        "<li>✅ Mượn trả sách tự động</li>"
        "<li>✅ Thống kê và báo cáo</li>"
        "<li>✅ Top 10 sách/độc giả</li>"
        "<li>✅ Cảnh báo quá hạn</li>"
        "<li>✅ Sao lưu dữ liệu</li>"
        "<li>✅ In ấn thẻ/tem/báo cáo</li>"
        "</ul>"
        "<hr>"
        "<p><b>Công nghệ:</b> Qt 6.10.1, C++17</p>"
        "<p><b>Cấu trúc dữ liệu:</b> AVL Tree, Linked List, Array</p>"
        "<p>© 2025 - Hệ thống Quản lý Thư viện</p>"
    );
    danhSachMenu->setCurrentRow(chiSoManHinhHienTai);
}

void CuaSoChinh::hienThiIn() {
    extern PTRDG rootDocGia;
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    HopThoaiInAn *dialog = new HopThoaiInAn(this);
    
    // Thiết lập dữ liệu hiện tại
    // Nếu đang ở màn hình Mượn Trả, truyền độc giả đang chọn
    if (widgetMuonTra && chiSoManHinhHienTai == 3) {
        // Lấy độc giả hiện tại từ widget Mượn Trả
        PTRDG docGiaHienTai = widgetMuonTra->property("docGiaHienTai").value<PTRDG>();
        if (docGiaHienTai) {
            dialog->setDocGiaHienTai(docGiaHienTai);
        }
    }
    
    // Truyền danh sách đầu sách
    dialog->setDanhSachDauSach(dsDauSach, soLuongDauSach);
    
    dialog->exec();
    delete dialog;
    
    // Khôi phục menu position
    danhSachMenu->setCurrentRow(chiSoManHinhHienTai);
    capNhatThanhTrangThai("Đã đóng dialog in ấn");
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
    
    // Set fixed width trước rồi animate
    khungMenuBen->setMinimumWidth(khungMenuBen->width());
    khungMenuBen->setMaximumWidth(khungMenuBen->width());
    
    // Animate width một cách mượt mà
    QPropertyAnimation *widthAnim = new QPropertyAnimation(khungMenuBen, "maximumWidth");
    widthAnim->setDuration(200);
    widthAnim->setStartValue(khungMenuBen->width());
    widthAnim->setEndValue(targetWidth);
    widthAnim->setEasingCurve(QEasingCurve::InOutCubic);
    
    // Update minimum width đồng thời
    connect(widthAnim, &QPropertyAnimation::valueChanged, [this](const QVariant &value) {
        khungMenuBen->setMinimumWidth(value.toInt());
    });
    
    // Xử lý UI sau khi animation xong
    connect(widthAnim, &QPropertyAnimation::finished, [this, targetWidth]() {
        khungMenuBen->setMinimumWidth(targetWidth);
        khungMenuBen->setMaximumWidth(targetWidth);
    });
    
    widthAnim->start(QAbstractAnimation::DeleteWhenStopped);
    
    // Update UI ngay lập tức (không đợi animation)
    QLabel *logoLabel = khungMenuBen->findChild<QLabel*>("LogoLabel");
    QLabel *titleLabel = khungMenuBen->findChild<QLabel*>("TitleLabel");
    
    QStringList menuTexts = {
        "▣  Trang Chủ",
        "▤  Quản Lý Sách",
        "◉  Quản Lý Độc Giả",
        "⇄  Mượn Trả Sách",
        "ⓘ  Giới Thiệu"
    };
    
    if (sidebarDangMoRong) {
        nutToggleSidebar->setText("☰");
        if (logoLabel) logoLabel->show();
        if (titleLabel) titleLabel->show();
        
        // Hiện full text
        for (int i = 0; i < danhSachMenu->count() && i < menuTexts.size(); ++i) {
            danhSachMenu->item(i)->setText(menuTexts[i]);
        }
    } else {
        nutToggleSidebar->setText("»");
        if (logoLabel) logoLabel->hide();
        if (titleLabel) titleLabel->hide();
        
        // Chỉ hiện icon
        for (int i = 0; i < danhSachMenu->count() && i < menuTexts.size(); ++i) {
            danhSachMenu->item(i)->setText(menuTexts[i].left(2));
        }
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
    try {
        // Lưu dữ liệu sách
        extern PTRDS dsDauSach[];
        extern int soLuongDauSach;
        GhiDanhSachDauSach(FilePathManager::getDauSachPathStd().c_str(), dsDauSach, soLuongDauSach);
        GhiDanhMucSach(FilePathManager::getDanhMucSachPathStd().c_str(), dsDauSach, soLuongDauSach);
        
        // Lưu dữ liệu độc giả
        extern PTRDG rootDocGia;
        saveDocGia(rootDocGia);
        
        // Reset flag thay đổi
        extern bool duLieuDaThayDoi;
        duLieuDaThayDoi = false;
        
        qDebug() << "[AUTO-SAVE] Đã lưu thành công tất cả dữ liệu";
    } catch (const std::exception& e) {
        qWarning() << "[AUTO-SAVE] Lỗi khi lưu:" << e.what();
    }
}

void CuaSoChinh::triggerAutoSave() {
    if (autoSaveManager) {
        autoSaveManager->triggerSave();
    }
}


