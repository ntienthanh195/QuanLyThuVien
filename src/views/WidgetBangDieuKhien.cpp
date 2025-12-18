#include "views/WidgetBangDieuKhien.h"
#include "models/QuanLySach.h"
#include "models/DocGia.h"
#include "utils/NgayThang.h"
#include "utils/AnimationHelper.h"
#include "utils/ResponsiveHelper.h"
#include "Constants.h"
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QRegularExpression>
#include <QGridLayout>
#include <QComboBox>
#include <QTimer>
#include <QShortcut>
#include <QKeySequence>
#include <functional>
#include "utils/EventBus.h"
#include "utils/EventBus.h"
#include "utils/LanguageManager.h"
#include <QFile>
#include <QPixmap>

// ==================== REFACTORED: Sử dụng Backend API ====================
// Thay vì tự đếm, sử dụng các hàm có sẵn:
// - LayThongKeSach(): lấy thống kê sách toàn hệ thống
// - LayThongKeDocGia(): lấy thống kê độc giả toàn hệ thống  
// - LayDSDocGiaQuaHan(): lấy danh sách độc giả quá hạn
// ==========================================================

WidgetBangDieuKhien::WidgetBangDieuKhien(QWidget *parent)
    : QWidget(parent)
    , layoutChinh(nullptr)
    , btnRefresh(nullptr)
    , comboBoLocThoiGian(nullptr)
    , labelTieuDeTrang(nullptr)
    , khungCanhBao(nullptr)
    , cardSachQuaHan(nullptr)
    , cardTheSapHet(nullptr)
    , khungThongKe(nullptr)
    , khungTop10(nullptr)
    , cardTop10Sach(nullptr)
    , cardTop10DocGia(nullptr)
    , btnXemTop10Sach(nullptr)
    , btnXemTop10DocGia(nullptr)
    , khungTacVuNhanh(nullptr)
    , labelSachQuaHan(nullptr)
    , labelTheSapHetHan(nullptr)
    , labelMoTaSachQuaHan(nullptr)
    , labelMoTaTheSapHet(nullptr)
    , btnXemSachQuaHan(nullptr)
    , btnXemTheSapHet(nullptr)
    , labelTongSoSach(nullptr)
    , labelSoDocGia(nullptr)
    , labelSachDangMuon(nullptr)
    , labelLuotMuonHomNay(nullptr)
    , btnThemSachMoi(nullptr)
    , btnDangKyDocGia(nullptr)
    , btnTaoPhieuMuon(nullptr)
    , labelLoading(nullptr)
    , timerAutoRefresh(nullptr)
    , isLoading(false)
    , shortcutRefresh(nullptr)
    , shortcutAddBook(nullptr)
    , shortcutAddReader(nullptr)
    , shortcutNewLoan(nullptr)
{
    khoiTaoGiaoDien();
    setupShortcuts();
    
    // Auto-refresh every 30 seconds
    timerAutoRefresh = new QTimer(this);
    connect(timerAutoRefresh, &QTimer::timeout, this, &WidgetBangDieuKhien::capNhatThongKe);
    timerAutoRefresh->start(30000); // 30 seconds
    
    // Refresh when global data changes (e.g. background loader finished)
    // NOTE: This widget uses lightweight update already (only setText), so debouncing not critical here
    // But BangDieuKhien (non-Widget version) needs debouncing due to full UI recreate
    connect(&EventBus::instance(), &EventBus::dataChanged, this, [this]() { 
        this->capNhatThongKe(); // Already lightweight (lines 698-742)
    });

    // Localization
    connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this, &WidgetBangDieuKhien::capNhatNgonNgu);
    capNhatNgonNgu(); // Initial set
}

void WidgetBangDieuKhien::capNhatNgonNgu() {
    // Headers
    if (lblHeaderCanhBao) lblHeaderCanhBao->setText(LanguageManager::instance().getText("HEADER_DASHBOARD_WARNING"));
    if (lblHeaderSoLieu) lblHeaderSoLieu->setText(LanguageManager::instance().getText("HEADER_DASHBOARD_STATS"));
    if (lblHeaderTacVu) lblHeaderTacVu->setText(LanguageManager::instance().getText("HEADER_DASHBOARD_QUICK"));
    
    // Stat Cards
    if (lblTitleTongSach) lblTitleTongSach->setText(LanguageManager::instance().getText("CARD_TITLE_TOTAL_BOOKS"));
    if (lblTitleDocGia) lblTitleDocGia->setText(LanguageManager::instance().getText("CARD_TITLE_TOTAL_READERS"));
    if (lblTitleDangMuon) lblTitleDangMuon->setText(LanguageManager::instance().getText("CARD_TITLE_BORROWED"));
    if (lblTitleLuotMuon) lblTitleLuotMuon->setText(LanguageManager::instance().getText("CARD_TITLE_VISITS"));
    
    // Warning Cards
    if (lblTitleSachQuaHan) lblTitleSachQuaHan->setText(LanguageManager::instance().getText("CARD_TITLE_OVERDUE_BOOKS"));
    if (lblTitleTheSapHet) lblTitleTheSapHet->setText(LanguageManager::instance().getText("CARD_TITLE_WARNING_CARDS"));
    
    if (btnXemSachQuaHan) btnXemSachQuaHan->setText(LanguageManager::instance().getText("BTN_VIEW_DETAILS"));
    if (btnXemTheSapHet) btnXemTheSapHet->setText(LanguageManager::instance().getText("BTN_VIEW_DETAILS"));
    
    // Quick Actions
    if (lblBtnThemSach) lblBtnThemSach->setText(LanguageManager::instance().getText("BTN_QUICK_ADD_BOOK"));
    if (lblBtnDangKy) lblBtnDangKy->setText(LanguageManager::instance().getText("BTN_QUICK_ADD_READER"));
    if (lblBtnTaoPhieu) lblBtnTaoPhieu->setText(LanguageManager::instance().getText("BTN_QUICK_CREATE_LOAN"));
    if (lblBtnSaoLuu) lblBtnSaoLuu->setText(LanguageManager::instance().getText("BTN_QUICK_BACKUP"));
    if (lblBtnThongTin) lblBtnThongTin->setText(LanguageManager::instance().getText("BTN_QUICK_SYSTEM_INFO"));
    
    // Re-trigger stats update to refresh dynamic strings
    capNhatThongKe();
}

WidgetBangDieuKhien::~WidgetBangDieuKhien() {
    // Qt tự động giải phóng các widget con
}

void WidgetBangDieuKhien::khoiTaoGiaoDien() {
    // MASTER PLAN: Grid Layout Dashboard
    this->setProperty("class", "WidgetBangDieuKhien");
    
    layoutChinh = new QVBoxLayout(this);
    layoutChinh->setSpacing(25);
    layoutChinh->setContentsMargins(35, 35, 35, 35);
    
    // Header
    taoTieuDeVaBoLoc();
    
    // MASTER PLAN: Hàng 1 - Grid 4 Metrics Cards
    taoKhungThongKe();
    
    // MASTER PLAN: Hàng 2 - Warning Cards
    taoKhungCanhBao();
    
    // MASTER PLAN: Hàng 3 - Quick Actions
    taoKhungTacVuNhanh();
    
    // MASTER PLAN: Hàng 4 - Top 10 / Charts
    taoKhungTop10();
    
    layoutChinh->addStretch();
}

void WidgetBangDieuKhien::taoKhungCanhBao() {
    // Tiêu đề phần
    QLabel *tieuDePhan = new QLabel("Cảnh báo", this);
    QFont fontPhan;
    fontPhan.setPointSize(18);
    fontPhan.setBold(true);
    tieuDePhan->setFont(fontPhan);
    tieuDePhan->setProperty("class", "SectionTitle");
    layoutChinh->addWidget(tieuDePhan);
    lblHeaderCanhBao = tieuDePhan;
    
    // Layout ngang cho 2 card cảnh báo
    QHBoxLayout *layoutCanhBao = new QHBoxLayout();
    layoutCanhBao->setSpacing(16);
    
    // Card 1: Sách quá hạn - tạo card với placeholder, sẽ cập nhật sau
    cardSachQuaHan = new QWidget(this);
    cardSachQuaHan->setProperty("class", "AlertCardDanger");
    cardSachQuaHan->setMinimumHeight(120);
    
    QGraphicsDropShadowEffect *shadow1 = new QGraphicsDropShadowEffect();
    shadow1->setBlurRadius(20);
    shadow1->setXOffset(0);
    shadow1->setYOffset(4);
    shadow1->setColor(QColor(0, 0, 0, 25));
    cardSachQuaHan->setGraphicsEffect(shadow1);
    
    QHBoxLayout *layoutCard1 = new QHBoxLayout(cardSachQuaHan);
    layoutCard1->setSpacing(16);
    layoutCard1->setContentsMargins(20, 16, 20, 16);
    
    // Icon
    QWidget *iconWidget1 = new QWidget();
    iconWidget1->setProperty("class", "IconWidgetDanger");
    iconWidget1->setFixedSize(64, 64);
    QVBoxLayout *iconLayout1 = new QVBoxLayout(iconWidget1);
    iconLayout1->setContentsMargins(0, 0, 0, 0);
    QLabel *iconLabel1 = new QLabel("!");
    QFont iconFont1;
    iconFont1.setPointSize(32);
    iconFont1.setBold(true);
    iconLabel1->setFont(iconFont1);
    iconLabel1->setAlignment(Qt::AlignCenter);
    iconLabel1->setProperty("class", "IconLabelDanger");
    iconLayout1->addWidget(iconLabel1);
    
    // Content
    QVBoxLayout *contentLayout1 = new QVBoxLayout();
    contentLayout1->setSpacing(8);
    
    labelSachQuaHan = new QLabel("0 Sách Quá Hạn");
    QFont fontTieuDe1;
    fontTieuDe1.setPointSize(15);
    fontTieuDe1.setBold(true);
    labelSachQuaHan->setFont(fontTieuDe1);
    labelSachQuaHan->setProperty("class", "DangerStat");
    
    labelMoTaSachQuaHan = new QLabel("Đang kiểm tra dữ liệu...");
    labelMoTaSachQuaHan->setWordWrap(true);
    labelMoTaSachQuaHan->setProperty("class", "CardDescription");
    
    btnXemSachQuaHan = new QPushButton("Xem chi tiết");
    btnXemSachQuaHan->setProperty("class", "Primary");
    btnXemSachQuaHan->setCursor(Qt::PointingHandCursor);
    connect(btnXemSachQuaHan, &QPushButton::clicked, this, [this]() {
        emit yeuCauXemChiTiet("sachQuaHan");
    });
    
    contentLayout1->addWidget(labelSachQuaHan);
    contentLayout1->addWidget(labelMoTaSachQuaHan);
    contentLayout1->addStretch();
    contentLayout1->addWidget(btnXemSachQuaHan, 0, Qt::AlignLeft);
    
    layoutCard1->addWidget(iconWidget1);
    layoutCard1->addLayout(contentLayout1, 1);
    
    // Card 2: Thẻ sắp hết hạn
    cardTheSapHet = new QWidget(this);
    cardTheSapHet->setProperty("class", "AlertCardWarning");
    cardTheSapHet->setMinimumHeight(120);
    
    QGraphicsDropShadowEffect *shadow2 = new QGraphicsDropShadowEffect();
    shadow2->setBlurRadius(20);
    shadow2->setXOffset(0);
    shadow2->setYOffset(4);
    shadow2->setColor(QColor(0, 0, 0, 25));
    cardTheSapHet->setGraphicsEffect(shadow2);
    
    QHBoxLayout *layoutCard2 = new QHBoxLayout(cardTheSapHet);
    layoutCard2->setSpacing(16);
    layoutCard2->setContentsMargins(20, 16, 20, 16);
    
    // Icon
    QWidget *iconWidget2 = new QWidget();
    iconWidget2->setProperty("class", "IconWidgetWarning");
    iconWidget2->setFixedSize(64, 64);
    QVBoxLayout *iconLayout2 = new QVBoxLayout(iconWidget2);
    iconLayout2->setContentsMargins(0, 0, 0, 0);
    QLabel *iconLabel2 = new QLabel("i");
    QFont iconFont2;
    iconFont2.setPointSize(32);
    iconFont2.setBold(true);
    iconLabel2->setFont(iconFont2);
    iconLabel2->setAlignment(Qt::AlignCenter);
    iconLabel2->setStyleSheet("background: transparent; border: none; color: #F59E0B;");
    iconLayout2->addWidget(iconLabel2);
    
    // Content
    QVBoxLayout *contentLayout2 = new QVBoxLayout();
    contentLayout2->setSpacing(8);
    
    labelTheSapHetHan = new QLabel("0 Thẻ Cảnh Báo");
    QFont fontTieuDe2;
    fontTieuDe2.setPointSize(15);
    fontTieuDe2.setBold(true);
    labelTheSapHetHan->setFont(fontTieuDe2);
    labelTheSapHetHan->setStyleSheet("color: #F59E0B; background: transparent;");
    
    labelMoTaTheSapHet = new QLabel("Đang kiểm tra dữ liệu...");
    labelMoTaTheSapHet->setWordWrap(true);
    labelMoTaTheSapHet->setStyleSheet("color: #A0A0B0; font-size: 12px; background: transparent;");
    
    btnXemTheSapHet = new QPushButton("Xem chi tiết");
    btnXemTheSapHet->setStyleSheet(
        "QPushButton {"
        "  background: #3B82F6; color: white; border: none; border-radius: 6px;"
        "  padding: 8px 16px; font-weight: 600; font-size: 12px;"
        "}"
        "QPushButton:hover { background: #2563EB; }"
    );
    btnXemTheSapHet->setCursor(Qt::PointingHandCursor);
    connect(btnXemTheSapHet, &QPushButton::clicked, this, [this]() {
        emit yeuCauXemChiTiet("theSapHetHan");
    });
    
    contentLayout2->addWidget(labelTheSapHetHan);
    contentLayout2->addWidget(labelMoTaTheSapHet);
    contentLayout2->addStretch();
    contentLayout2->addWidget(btnXemTheSapHet, 0, Qt::AlignLeft);
    
    layoutCard2->addWidget(iconWidget2);
    layoutCard2->addLayout(contentLayout2, 1);
    
    layoutCanhBao->addWidget(cardSachQuaHan);
    layoutCanhBao->addWidget(cardTheSapHet);
    layoutChinh->addLayout(layoutCanhBao);
}

void WidgetBangDieuKhien::taoKhungThongKe() {
    // Tiêu đề phần
    QLabel *tieuDePhan = new QLabel("Số liệu chính", this);
    QFont fontPhan;
    fontPhan.setPointSize(18);
    fontPhan.setBold(true);
    tieuDePhan->setFont(fontPhan);
    tieuDePhan->setStyleSheet("color: #F1F5F9; margin-top: 16px; margin-bottom: 12px; background: transparent;");
    layoutChinh->addWidget(tieuDePhan);
    lblHeaderSoLieu = tieuDePhan;
    
    // Grid layout cho 4 card thống kê (2x2)
    QGridLayout *gridThongKe = new QGridLayout();
    gridThongKe->setSpacing(16);
    gridThongKe->setContentsMargins(0, 0, 0, 0);
    
    // Card 1: Tổng số sách
    QWidget *cardTongSach = taoCardThongKe(":/icons/book.png", "Tổng số sách", "12,480", "#3B82F6");
    gridThongKe->addWidget(cardTongSach, 0, 0);
    lblTitleTongSach = cardTongSach->findChild<QLabel*>("CardTitle");
    
    // Card 2: Số học viên
    QWidget *cardDocGia = taoCardThongKe(":/icons/reader.png", "Số học viên", "3,256", "#10B981");
    gridThongKe->addWidget(cardDocGia, 0, 1);
    lblTitleDocGia = cardDocGia->findChild<QLabel*>("CardTitle");
    
    // Card 3: Sách đang mượn
    QWidget *cardDangMuon = taoCardThongKe(":/icons/loan.png", "Sách đang mượn", "789", "#F59E0B");
    gridThongKe->addWidget(cardDangMuon, 1, 0);
    lblTitleDangMuon = cardDangMuon->findChild<QLabel*>("CardTitle");
    
    // Card 4: Lượt truy cập hôm nay
    QWidget *cardLuotMuon = taoCardThongKe(":/icons/stats.png", "Lượt truy cập hôm nay", "121", "#8B5CF6");
    gridThongKe->addWidget(cardLuotMuon, 1, 1);
    lblTitleLuotMuon = cardLuotMuon->findChild<QLabel*>("CardTitle");
    
    layoutChinh->addLayout(gridThongKe);
}

void WidgetBangDieuKhien::taoKhungTacVuNhanh() {
    // Tiêu đề phần
    QLabel *tieuDePhan = new QLabel("Tác vụ nhanh", this);
    QFont fontPhan;
    fontPhan.setPointSize(18);
    fontPhan.setBold(true);
    tieuDePhan->setFont(fontPhan);
    tieuDePhan->setStyleSheet("color: #F1F5F9; margin-top: 16px; margin-bottom: 12px; background: transparent;");
    layoutChinh->addWidget(tieuDePhan);
    lblHeaderTacVu = tieuDePhan;
    
    // Layout lưới cho các nút
    QGridLayout *layoutTacVu = new QGridLayout();
    layoutTacVu->setSpacing(12);
    
    // Hàng 1: 3 nút chính
    btnThemSachMoi = taoNutTacVuNhanh("Thêm Sách Mới", ":/icons/plus.png", "#3B82F6");
    connect(btnThemSachMoi, &QPushButton::clicked, this, &WidgetBangDieuKhien::yeuCauThemSachMoi);
    lblBtnThemSach = btnThemSachMoi->findChild<QLabel*>("BtnLabel");
    
    btnDangKyDocGia = taoNutTacVuNhanh("Đăng Ký Học Viên", ":/icons/reader.png", "#3B82F6");
    connect(btnDangKyDocGia, &QPushButton::clicked, this, &WidgetBangDieuKhien::yeuCauDangKyDocGia);
    lblBtnDangKy = btnDangKyDocGia->findChild<QLabel*>("BtnLabel");
    
    btnTaoPhieuMuon = taoNutTacVuNhanh("Tạo Phiếu Mượn", ":/icons/loan.png", "#3B82F6");
    connect(btnTaoPhieuMuon, &QPushButton::clicked, this, &WidgetBangDieuKhien::yeuCauTaoPhieuMuon);
    lblBtnTaoPhieu = btnTaoPhieuMuon->findChild<QLabel*>("BtnLabel");
    
    // Hàng 2: 2 nút phụ
    btnSaoLuu = taoNutTacVuNhanh("Sao Lưu Dữ Liệu", ":/icons/save.png", "#10B981");
    connect(btnSaoLuu, &QPushButton::clicked, this, &WidgetBangDieuKhien::yeuCauSaoLuu);
    lblBtnSaoLuu = btnSaoLuu->findChild<QLabel*>("BtnLabel");
    
    btnThongTin = taoNutTacVuNhanh("Thông Tin Hệ Thống", ":/icons/info.png", "#3B82F6");
    connect(btnThongTin, &QPushButton::clicked, this, &WidgetBangDieuKhien::yeuCauXemThongTin);
    lblBtnThongTin = btnThongTin->findChild<QLabel*>("BtnLabel");
    
    // Demo buttons - chỉ để development/debugging
    #ifdef QT_DEBUG
    QPushButton *btnDemo = taoNutTacVuNhanh("Demo UI Phase 1", ":/icons/demo.png", "#8B5CF6");
    connect(btnDemo, &QPushButton::clicked, this, [this]() {
        emit yeuCauMoDemo();
    });
    
    QPushButton *btnDemoBang = taoNutTacVuNhanh("Demo Table", ":/icons/table.png", "#8B5CF6");
    connect(btnDemoBang, &QPushButton::clicked, this, [this]() {
        emit yeuCauMoDemoBang();
    });
    #endif
    
    layoutTacVu->addWidget(btnThemSachMoi, 0, 0);
    layoutTacVu->addWidget(btnDangKyDocGia, 0, 1);
    layoutTacVu->addWidget(btnTaoPhieuMuon, 0, 2);
    layoutTacVu->addWidget(btnSaoLuu, 1, 0);
    layoutTacVu->addWidget(btnThongTin, 1, 1);
    
    #ifdef QT_DEBUG
    layoutTacVu->addWidget(btnDemo, 1, 2);
    layoutTacVu->addWidget(btnDemoBang, 2, 0);
    #endif
    
    layoutChinh->addLayout(layoutTacVu);
}

QWidget* WidgetBangDieuKhien::taoCardCanhBao(const QString &tieuDe, const QString &moTa,
                                               const QString &mauNen, const QString &mauChu,
                                               const QString &icon, QLabel **labelSoLieu,
                                               QPushButton **btnHanhDong) {
    QWidget *card = new QWidget(this);
    
    // Card với nền tối và border màu
    card->setStyleSheet(QString(
        "QWidget { "
        "  background: #071233;"
        "  border-radius: 12px; "
        "  border-left: 4px solid %1;"
        "}"
    ).arg(mauChu));
    
    card->setMinimumHeight(120);
    
    // Shadow effect
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(20);
    shadow->setXOffset(0);
    shadow->setYOffset(4);
    shadow->setColor(QColor(0, 0, 0, 25));
    card->setGraphicsEffect(shadow);
    
    // Layout chính
    QHBoxLayout *mainLayout = new QHBoxLayout(card);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(20, 16, 20, 16);
    
    // Icon container bên trái
    QWidget *iconWidget = new QWidget(card);
    iconWidget->setStyleSheet(QString(
        "QWidget { "
        "  background-color: %1;"
        "  border-radius: 12px;"
        "  border: none;"
        "}"
    ).arg(mauNen));
    iconWidget->setFixedSize(64, 64);
    
    QVBoxLayout *iconLayout = new QVBoxLayout(iconWidget);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *iconLabel = new QLabel(icon, iconWidget);
    QFont iconFont;
    iconFont.setPointSize(28);
    iconLabel->setFont(iconFont);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("background: transparent; border: none;");
    iconLayout->addWidget(iconLabel);
    
    // Content bên phải
    QVBoxLayout *contentLayout = new QVBoxLayout();
    contentLayout->setSpacing(8);
    
    // Tiêu đề
    QLabel *labelTieuDe = new QLabel(tieuDe, card);
    QFont fontTieuDe;
    fontTieuDe.setPointSize(15);
    fontTieuDe.setBold(true);
    labelTieuDe->setFont(fontTieuDe);
    labelTieuDe->setStyleSheet(QString("color: %1; background: transparent;").arg(mauChu));
    labelTieuDe->setObjectName("CardTitle");
    if (tieuDe == "Sách Quá Hạn") lblTitleSachQuaHan = labelTieuDe;
    else if (tieuDe == "Thẻ Cảnh Báo") lblTitleTheSapHet = labelTieuDe;
    
    // Mô tả
    QLabel *labelMoTa = new QLabel(moTa, card);
    labelMoTa->setWordWrap(true);
    labelMoTa->setStyleSheet("color: #A0A0B0; font-size: 12px; background: transparent;");
    
    // Nút hành động
    *btnHanhDong = new QPushButton(card);
    (*btnHanhDong)->setStyleSheet(
        "QPushButton {"
        "  background: #3B82F6;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 8px 16px;"
        "  font-weight: 600;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "  background: #2563EB;"
        "}"
    );
    // Lưu lại pointer tới label nút (không trực tiếp vì là nút created dynamically), 
    // nhưng nút này được gắn vào btnXemSachQuaHan ở cấp trên rồi.
    // Thực ra btnHanhDong được assign ra ngoài.
    // Chúng ta cần cập nhật text của nút này.
    // Mà nút này là btnXemSachQuaHan hoặc btnXemTheSapHet.
    // Đã có member rồi: btnXemSachQuaHan, btnXemTheSapHet.
    // Nên chỉ cần cập nhật trong capNhatNgonNgu thông qua các member này là được.
    (*btnHanhDong)->setCursor(Qt::PointingHandCursor);
    
    contentLayout->addWidget(labelTieuDe);
    contentLayout->addWidget(labelMoTa);
    contentLayout->addStretch();
    contentLayout->addWidget(*btnHanhDong, 0, Qt::AlignLeft);
    
    mainLayout->addWidget(iconWidget);
    mainLayout->addLayout(contentLayout, 1);
    
    // Không cần label số liệu riêng nữa
    *labelSoLieu = nullptr;
    
    return card;
}

QWidget* WidgetBangDieuKhien::taoCardThongKe(const QString &icon, const QString &tieuDe,
                                               const QString &giaTri, const QString &mauChu) {
    QWidget *card = new QWidget(this);
    
    // Card với nền tối
    card->setStyleSheet(
        "QWidget {"
        "  background: #071233;"
        "  border-radius: 12px;"
        "  border: 1px solid #08314f;"
        "}"
    );
    
    card->setMinimumHeight(110);
    
    // Shadow effect
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(16);
    shadow->setXOffset(0);
    shadow->setYOffset(3);
    shadow->setColor(QColor(0, 0, 0, 20));
    card->setGraphicsEffect(shadow);
    
    // Layout chính
    QHBoxLayout *mainLayout = new QHBoxLayout(card);
    mainLayout->setSpacing(14);
    mainLayout->setContentsMargins(18, 16, 18, 16);
    
    // Icon container bên trái
    QWidget *iconWidget = new QWidget(card);
    QString iconBg;
    
    // Determine background color based on icon path context
    if (icon.contains("book")) {
        iconBg = "rgba(59, 130, 246, 0.1)";
    } else if (icon.contains("reader")) {
        iconBg = "rgba(16, 185, 129, 0.1)";
    } else if (icon.contains("loan")) {
        iconBg = "rgba(245, 158, 11, 0.1)";
    } else {
        iconBg = "rgba(139, 92, 246, 0.1)";
    }
    
    iconWidget->setStyleSheet(QString(
        "QWidget { "
        "  background-color: %1;"
        "  border-radius: 10px;"
        "  border: none;"
        "}"
    ).arg(iconBg));
    iconWidget->setFixedSize(52, 52);
    
    QVBoxLayout *iconLayout = new QVBoxLayout(iconWidget);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *iconLabel = new QLabel(iconWidget);
    if (icon.startsWith(":/")) {
        QPixmap pixmap(icon);
        iconLabel->setPixmap(pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        iconLabel->setText(icon);
        QFont iconFont;
        iconFont.setPointSize(24);
        iconLabel->setFont(iconFont);
    }
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("background: transparent; border: none;");
    iconLayout->addWidget(iconLabel);
    
    // Content bên phải
    QVBoxLayout *contentLayout = new QVBoxLayout();
    contentLayout->setSpacing(4);
    
    // Tiêu đề nhỏ
    QLabel *labelTieuDe = new QLabel(tieuDe, card);
    QFont fontTieuDe;
    fontTieuDe.setPointSize(11);
    fontTieuDe.setWeight(QFont::Medium);
    labelTieuDe->setFont(fontTieuDe);
    labelTieuDe->setStyleSheet("color: #A0A0B0; background: transparent;");
    labelTieuDe->setObjectName("CardTitle");
    
    // Giá trị lớn
    QLabel *labelGiaTri = new QLabel(giaTri, card);
    QFont fontGiaTri;
    fontGiaTri.setPointSize(26);
    fontGiaTri.setBold(true);
    labelGiaTri->setFont(fontGiaTri);
    labelGiaTri->setStyleSheet(QString("color: %1; background: transparent;").arg(mauChu));
    
    contentLayout->addWidget(labelTieuDe);
    contentLayout->addWidget(labelGiaTri);
    contentLayout->addStretch();
    
    mainLayout->addWidget(iconWidget);
    mainLayout->addLayout(contentLayout, 1);
    
    return card;
}

QPushButton* WidgetBangDieuKhien::taoNutTacVuNhanh(const QString &tieuDe, const QString &icon, const QString &mauNen) {
    QPushButton *btn = new QPushButton(this);
    
    // Layout cho button
    QHBoxLayout *btnLayout = new QHBoxLayout(btn);
    btnLayout->setSpacing(10);
    btnLayout->setContentsMargins(16, 14, 16, 14);
    
    // Icon với background tròn
    QLabel *iconLabel = new QLabel(btn);
    if (icon.startsWith(":/")) {
        QPixmap pixmap(icon);
        if (!pixmap.isNull()) {
            iconLabel->setPixmap(pixmap.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
             // Fallback text if icon load fails or empty
             iconLabel->setText("");
        }
    } else {
        iconLabel->setText(icon);
        QFont fontIcon;
        fontIcon.setPointSize(18);
        iconLabel->setFont(fontIcon);
    }

    iconLabel->setStyleSheet(
        "QLabel {"
        "  background-color: rgba(255, 255, 255, 0.2);"
        "  border-radius: 8px;"
        "  padding: 6px;"
        "  min-width: 36px;"
        "  max-width: 36px;"
        "  min-height: 36px;"
        "  max-height: 36px;"
        "}"
    );
    iconLabel->setAlignment(Qt::AlignCenter);
    
    // Text label
    QLabel *textLabel = new QLabel(tieuDe, btn);
    QFont fontText;
    fontText.setPointSize(13);
    fontText.setBold(true);
    textLabel->setFont(fontText);
    textLabel->setFont(fontText);
    textLabel->setStyleSheet("color: white; background: transparent;");
    textLabel->setObjectName("BtnLabel");
    
    btnLayout->addWidget(iconLabel);
    btnLayout->addWidget(textLabel);
    btnLayout->addStretch();
    
    // Style theo màu
    QString hoverColor;
    if (mauNen == "#3B82F6") {
        hoverColor = "#2563EB";
    } else if (mauNen == "#10B981") {
        hoverColor = "#059669";
    } else if (mauNen == "#8B5CF6") {
        hoverColor = "#7C3AED";
    } else {
        hoverColor = "#2563EB";
    }
    
    btn->setStyleSheet(QString(
        "QPushButton {"
        "  background: %1;"
        "  border: none;"
        "  border-radius: 10px;"
        "  text-align: left;"
        "}"
        "QPushButton:hover {"
        "  background: %2;"
        "}"
        "QPushButton:pressed {"
        "  background: %2;"
        "}"
    ).arg(mauNen, hoverColor));
    
    btn->setMinimumHeight(64);
    btn->setCursor(Qt::PointingHandCursor);
    
    // Shadow effect
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(12);
    shadow->setXOffset(0);
    shadow->setYOffset(3);
    shadow->setColor(QColor(0, 0, 0, 30));
    btn->setGraphicsEffect(shadow);
    
    return btn;
}

void WidgetBangDieuKhien::capNhatThongKe() {
    // Cập nhật số liệu cảnh báo
    int soSachQuaHan = demSachQuaHan();
    int soTheSapHet = demTheSapHetHan();
    
    if (labelSachQuaHan) {
        labelSachQuaHan->setText(LanguageManager::instance().getText("MSG_DASHBOARD_OVERDUE_COUNT").arg(soSachQuaHan));
    }
    
    if (labelMoTaSachQuaHan) {
        QString thongTin = layThongTinSachQuaHanNhieuNhat();
        labelMoTaSachQuaHan->setText(thongTin);
    }
    
    if (labelTheSapHetHan) {
        labelTheSapHetHan->setText(LanguageManager::instance().getText("MSG_DASHBOARD_WARNING_COUNT").arg(soTheSapHet));
    }
    
    if (labelMoTaTheSapHet) {
        QString thongTin = layThongTinDocGiaQuaHan();
        labelMoTaTheSapHet->setText(thongTin);
    }
    
    // Cập nhật số liệu thống kê
    if (labelTongSoSach) {
        int tongSoSach = demTongSoSach();
        labelTongSoSach->setText(QString::number(tongSoSach).replace(
            QRegularExpression("(\\d)(?=(\\d{3})+$)"), "\\1,"));
    }
    
    if (labelSoDocGia) {
        int soDocGia = demSoDocGia();
        labelSoDocGia->setText(QString::number(soDocGia).replace(
            QRegularExpression("(\\d)(?=(\\d{3})+$)"), "\\1,"));
    }
    
    if (labelSachDangMuon) {
        int sachDangMuon = demSachDangMuon();
        labelSachDangMuon->setText(QString::number(sachDangMuon));
    }
    
    if (labelLuotMuonHomNay) {
        int luotMuonHomNay = demLuotMuonHomNay();
        labelLuotMuonHomNay->setText(QString::number(luotMuonHomNay));
    }
}

// Các hàm tính toán thống kê
int WidgetBangDieuKhien::demSachQuaHan() {
    extern PTRDG rootDocGia;
    
    // Sử dụng API có sẵn thay vì tự duyệt cây
    const int MAX_KET_QUA = 1000;
    DocGiaQuaHanDTO danhSach[MAX_KET_QUA];
    int soLuong = LayDSDocGiaQuaHan(rootDocGia, danhSach, MAX_KET_QUA);
    
    // Đếm tổng số sách quá hạn từ danh sách
    int tongSachQuaHan = 0;
    for (int i = 0; i < soLuong; i++) {
        if (danhSach[i].docGia) {
            // Đếm số sách đang mượn quá hạn của độc giả này
            MUONTRA mt = danhSach[i].docGia->data.dsmt;
            while (mt != nullptr) {
                if (mt->data.TrangThai == 0) { // đang mượn
                    int soNgayQuaHan = tinhSoNgayQuaHan(mt->data.NgayMuon);
                    if (soNgayQuaHan > 0) {
                        tongSachQuaHan++;
                    }
                }
                mt = mt->next;
            }
        }
    }
    
    return tongSachQuaHan;
}

int WidgetBangDieuKhien::demTheSapHetHan() {
    extern PTRDG rootDocGia;
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    // Sử dụng API thống kê có sẵn
    ThongKeDocGia thongKe = LayThongKeDocGia(rootDocGia, dsDauSach, soLuongDauSach);
    
    // Thẻ cảnh báo = thẻ bị khóa + độc giả có sách quá hạn
    return thongKe.docGiaBiKhoa + thongKe.docGiaCoSachQuaHan;
}

int WidgetBangDieuKhien::demTongSoSach() {
    ThongKeSach thongKe = LayThongKeSach();
    return thongKe.tongSoBanSao;
}

int WidgetBangDieuKhien::demSoDocGia() {
    extern PTRDG rootDocGia;
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    ThongKeDocGia thongKe = LayThongKeDocGia(rootDocGia, dsDauSach, soLuongDauSach);
    return thongKe.tongSoDocGia;
}

int WidgetBangDieuKhien::demSachDangMuon() {
    ThongKeSach thongKe = LayThongKeSach();
    return thongKe.soBanSaoDangMuon;
}

int WidgetBangDieuKhien::demLuotMuonHomNay() {
    // Đếm số lượt truy cập hôm nay = tổng số độc giả hoạt động
    extern PTRDG rootDocGia;
    
    std::string ngayHienTai = layNgayHienTai();
    int dem = 0;
    
    std::function<void(PTRDG)> demMuonHomNay = [&](PTRDG node) {
        if (node == nullptr) return;
        
        // Duyệt danh sách mượn trả
        MUONTRA mt = node->data.dsmt;
        while (mt != nullptr) {
            // Đếm các lượt mượn có ngày mượn = hôm nay
            if (mt->data.NgayMuon == ngayHienTai) {
                dem++;
            }
            // Hoặc đếm các lượt trả có ngày trả = hôm nay
            if (mt->data.TrangThai == 1 && mt->data.NgayTra == ngayHienTai) {
                dem++;
            }
            mt = mt->next;
        }
        
        demMuonHomNay(node->left);
        demMuonHomNay(node->right);
    };
    
    demMuonHomNay(rootDocGia);
    return dem;
}

// Lấy thông tin sách quá hạn nhiều nhất
QString WidgetBangDieuKhien::layThongTinSachQuaHanNhieuNhat() {
    extern PTRDG rootDocGia;
    
    // Sử dụng API có sẵn
    const int MAX_KET_QUA = 1000;
    DocGiaQuaHanDTO danhSach[MAX_KET_QUA];
    int soLuong = LayDSDocGiaQuaHan(rootDocGia, danhSach, MAX_KET_QUA);
    
    if (soLuong > 0 && danhSach[0].docGia) {
        // Lấy độc giả quá hạn lâu nhất (API đã sắp xếp)
        int soNgay = danhSach[0].soNgayQuaHanMax;
        QString ten = QString::fromStdString(danhSach[0].docGia->data.Ho + " " + 
                                             danhSach[0].docGia->data.Ten);
        return LanguageManager::instance().getText("MSG_DASHBOARD_OVERDUE_DETAIL").arg(ten).arg(soNgay);
    }
    
    return LanguageManager::instance().getText("MSG_DASHBOARD_NO_OVERDUE");
}

// Lấy thông tin độc giả quá hạn
QString WidgetBangDieuKhien::layThongTinDocGiaQuaHan() {
    extern PTRDG rootDocGia;
    
    // Sử dụng API có sẵn - đã sắp xếp theo ngày quá hạn giảm dần
    const int MAX_KET_QUA = 1000;
    DocGiaQuaHanDTO danhSach[MAX_KET_QUA];
    int soLuong = LayDSDocGiaQuaHan(rootDocGia, danhSach, MAX_KET_QUA);
    
    if (soLuong > 0 && danhSach[0].docGia) {
        int soNgay = danhSach[0].soNgayQuaHanMax;
        QString ten = QString::fromStdString(danhSach[0].docGia->data.Ho + " " + 
                                             danhSach[0].docGia->data.Ten);
        return LanguageManager::instance().getText("MSG_DASHBOARD_OVERDUE_DETAIL").arg(ten).arg(soNgay);
    }
    
    return LanguageManager::instance().getText("MSG_DASHBOARD_NO_WARNING");
}

// ==================== NEW METHODS ====================

// Tạo tiêu đề và bộ lọc
void WidgetBangDieuKhien::taoTieuDeVaBoLoc() {
    QHBoxLayout *layoutHeader = new QHBoxLayout();
    layoutHeader->setSpacing(16);
    
    // Tiêu đề trang
    labelTieuDeTrang = new QLabel("📊 Bảng điều khiển", this);
    QFont fontTieuDe;
    fontTieuDe.setPointSize(28);
    fontTieuDe.setBold(true);
    labelTieuDeTrang->setFont(fontTieuDe);
    labelTieuDeTrang->setStyleSheet("color: #60A5FA; background: transparent;");
    
    layoutHeader->addWidget(labelTieuDeTrang);
    layoutHeader->addStretch();
    
    // Bộ lọc thời gian
    comboBoLocThoiGian = new QComboBox(this);
    comboBoLocThoiGian->addItem("📅 Tất cả", "all");
    comboBoLocThoiGian->addItem("📆 Hôm nay", "today");
    comboBoLocThoiGian->addItem("📊 7 ngày", "week");
    comboBoLocThoiGian->addItem("📈 30 ngày", "month");
    comboBoLocThoiGian->setStyleSheet(
        "QComboBox {"
        "  background: #2D3748;"
        "  color: #E2E8F0;"
        "  border: 2px solid #4A5568;"
        "  border-radius: 8px;"
        "  padding: 10px 16px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "  min-width: 150px;"
        "}"
        "QComboBox::drop-down {"
        "  border: none;"
        "  width: 30px;"
        "}"
        "QComboBox::down-arrow {"
        "  image: none;"
        "  border-left: 5px solid transparent;"
        "  border-right: 5px solid transparent;"
        "  border-top: 6px solid #E2E8F0;"
        "  margin-right: 8px;"
        "}"
        "QComboBox:hover {"
        "  background: #374151;"
        "  border-color: #60A5FA;"
        "}"
        "QComboBox QAbstractItemView {"
        "  background: #2D3748;"
        "  color: #E2E8F0;"
        "  border: 2px solid #60A5FA;"
        "  border-radius: 8px;"
        "  selection-background-color: #60A5FA;"
        "}"
    );
    connect(comboBoLocThoiGian, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &WidgetBangDieuKhien::xuLyThayDoiBoLoc);
    
    // Nút refresh
    btnRefresh = new QPushButton("🔄 Làm mới", this);
    btnRefresh->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "    stop:0 #3B82F6, stop:1 #2563EB);"
        "  color: white;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 10px 20px;"
        "  font-weight: 600;"
        "  font-size: 14px;"
        "  min-width: 120px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "    stop:0 #2563EB, stop:1 #1D4ED8);"
        "}"
        "QPushButton:pressed {"
        "  background: #1E40AF;"
        "}"
    );
    btnRefresh->setCursor(Qt::PointingHandCursor);
    connect(btnRefresh, &QPushButton::clicked, this, &WidgetBangDieuKhien::xuLyRefresh);
    
    layoutHeader->addWidget(comboBoLocThoiGian);
    layoutHeader->addWidget(btnRefresh);
    
    layoutChinh->addLayout(layoutHeader);
}

// Tạo khung Top 10 Statistics
void WidgetBangDieuKhien::taoKhungTop10() {
    // Tiêu đề phần
    QLabel *tieuDePhan = new QLabel("Thống Kê Hàng Đầu", this);
    QFont fontPhan;
    fontPhan.setPointSize(18);
    fontPhan.setBold(true);
    tieuDePhan->setFont(fontPhan);
    tieuDePhan->setStyleSheet("color: #F1F5F9; margin-top: 16px; margin-bottom: 12px; background: transparent;");
    layoutChinh->addWidget(tieuDePhan);
    
    // Layout ngang cho 2 card top 10
    QHBoxLayout *layoutTop10 = new QHBoxLayout();
    layoutTop10->setSpacing(16);
    
    // Card 1: Top 10 Sách được mượn nhiều
    cardTop10Sach = new QWidget(this);
    cardTop10Sach->setStyleSheet(
        "QWidget { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #1E3A8A, stop:1 #1E40AF);"
        "  border-radius: 12px;"
        "}"
    );
    cardTop10Sach->setMinimumHeight(200);
    
    QGraphicsDropShadowEffect *shadow1 = new QGraphicsDropShadowEffect();
    shadow1->setBlurRadius(20);
    shadow1->setXOffset(0);
    shadow1->setYOffset(4);
    shadow1->setColor(QColor(0, 0, 0, 40));
    cardTop10Sach->setGraphicsEffect(shadow1);
    
    QVBoxLayout *layoutCard1 = new QVBoxLayout(cardTop10Sach);
    layoutCard1->setSpacing(12);
    layoutCard1->setContentsMargins(24, 20, 24, 20);
    
    QLabel *iconSach = new QLabel("■");
    QFont fontIcon;
    fontIcon.setPointSize(40);
    fontIcon.setBold(true);
    iconSach->setFont(fontIcon);
    iconSach->setAlignment(Qt::AlignCenter);
    iconSach->setStyleSheet("background: transparent; color: white;");
    
    QLabel *labelTieuDeSach = new QLabel("Top 10 Sách Được Mượn");
    QFont fontTieuDe1;
    fontTieuDe1.setPointSize(16);
    fontTieuDe1.setBold(true);
    labelTieuDeSach->setFont(fontTieuDe1);
    labelTieuDeSach->setAlignment(Qt::AlignCenter);
    labelTieuDeSach->setStyleSheet("color: white; background: transparent;");
    
    QLabel *labelMoTaSach = new QLabel("Xem danh sách sách được mượn nhiều nhất");
    labelMoTaSach->setAlignment(Qt::AlignCenter);
    labelMoTaSach->setWordWrap(true);
    labelMoTaSach->setStyleSheet("color: #BFDBFE; font-size: 13px; background: transparent;");
    
    btnXemTop10Sach = new QPushButton("Xem chi tiết");
    btnXemTop10Sach->setStyleSheet(
        "QPushButton {"
        "  background: white;"
        "  color: #1E40AF;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 10px 20px;"
        "  font-weight: 600;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "  background: #F0F9FF;"
        "}"
    );
    btnXemTop10Sach->setCursor(Qt::PointingHandCursor);
    connect(btnXemTop10Sach, &QPushButton::clicked, this, [this]() {
        emit yeuCauXemTop10Sach();
    });
    
    layoutCard1->addWidget(iconSach);
    layoutCard1->addWidget(labelTieuDeSach);
    layoutCard1->addWidget(labelMoTaSach);
    layoutCard1->addStretch();
    layoutCard1->addWidget(btnXemTop10Sach);
    
    // Card 2: Top 10 Độc giả hoạt động
    cardTop10DocGia = new QWidget(this);
    cardTop10DocGia->setStyleSheet(
        "QWidget { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #065F46, stop:1 #047857);"
        "  border-radius: 12px;"
        "}"
    );
    cardTop10DocGia->setMinimumHeight(200);
    
    QGraphicsDropShadowEffect *shadow2 = new QGraphicsDropShadowEffect();
    shadow2->setBlurRadius(20);
    shadow2->setXOffset(0);
    shadow2->setYOffset(4);
    shadow2->setColor(QColor(0, 0, 0, 40));
    cardTop10DocGia->setGraphicsEffect(shadow2);
    
    QVBoxLayout *layoutCard2 = new QVBoxLayout(cardTop10DocGia);
    layoutCard2->setSpacing(12);
    layoutCard2->setContentsMargins(24, 20, 24, 20);
    
    QLabel *iconDocGia = new QLabel("●");
    QFont fontIcon2;
    fontIcon2.setPointSize(40);
    fontIcon2.setBold(true);
    iconDocGia->setFont(fontIcon2);
    iconDocGia->setAlignment(Qt::AlignCenter);
    iconDocGia->setStyleSheet("background: transparent; color: white;");
    
    QLabel *labelTieuDeDocGia = new QLabel("Top 10 Học Viên Hoạt Động");
    QFont fontTieuDe2;
    fontTieuDe2.setPointSize(16);
    fontTieuDe2.setBold(true);
    labelTieuDeDocGia->setFont(fontTieuDe2);
    labelTieuDeDocGia->setAlignment(Qt::AlignCenter);
    labelTieuDeDocGia->setStyleSheet("color: white; background: transparent;");
    
    QLabel *labelMoTaDocGia = new QLabel("Xem học viên mượn sách nhiều nhất");
    labelMoTaDocGia->setAlignment(Qt::AlignCenter);
    labelMoTaDocGia->setWordWrap(true);
    labelMoTaDocGia->setStyleSheet("color: #A7F3D0; font-size: 13px; background: transparent;");
    
    btnXemTop10DocGia = new QPushButton("Xem chi tiết");
    btnXemTop10DocGia->setStyleSheet(
        "QPushButton {"
        "  background: white;"
        "  color: #047857;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 10px 20px;"
        "  font-weight: 600;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "  background: #F0FDF4;"
        "}"
    );
    btnXemTop10DocGia->setCursor(Qt::PointingHandCursor);
    connect(btnXemTop10DocGia, &QPushButton::clicked, this, [this]() {
        emit yeuCauXemTop10DocGia();
    });
    
    layoutCard2->addWidget(iconDocGia);
    layoutCard2->addWidget(labelTieuDeDocGia);
    layoutCard2->addWidget(labelMoTaDocGia);
    layoutCard2->addStretch();
    layoutCard2->addWidget(btnXemTop10DocGia);
    
    layoutTop10->addWidget(cardTop10Sach);
    layoutTop10->addWidget(cardTop10DocGia);
    
    layoutChinh->addLayout(layoutTop10);
}

// Xử lý refresh
void WidgetBangDieuKhien::xuLyRefresh() {
    if (isLoading) return; // Prevent multiple refreshes
    
    setLoadingState(true);
    
    // Cập nhật dữ liệu
    capNhatThongKe();
    
    // Khôi phục sau 800ms để thấy animation
    QTimer::singleShot(800, this, [this]() {
        setLoadingState(false);
    });
}

// Xử lý thay đổi bộ lọc
void WidgetBangDieuKhien::xuLyThayDoiBoLoc(int index) {
    Q_UNUSED(index);
    // TODO: Implement time filter logic
    // Hiện tại chỉ refresh data
    capNhatThongKe();
}

// Loading state management
void WidgetBangDieuKhien::setLoadingState(bool loading) {
    isLoading = loading;
    
    if (loading) {
        if (btnRefresh) {
            btnRefresh->setEnabled(false);
            btnRefresh->setText("⏳ Đang tải...");
        }
        
        // Show loading indicators on cards
        if (labelSachQuaHan) {
            AnimationHelper::pulse(cardSachQuaHan, 500);
        }
        if (labelTheSapHetHan) {
            AnimationHelper::pulse(cardTheSapHet, 500);
        }
    } else {
        if (btnRefresh) {
            btnRefresh->setEnabled(true);
            btnRefresh->setText("🔄 Làm mới");
        }
        
        // Fade in updated cards
        if (cardSachQuaHan) {
            AnimationHelper::fadeIn(cardSachQuaHan, 300);
        }
        if (cardTheSapHet) {
            AnimationHelper::fadeIn(cardTheSapHet, 300);
        }
    }
}

// Setup keyboard shortcuts
void WidgetBangDieuKhien::setupShortcuts() {
    // F5 or Ctrl+R: Refresh
    shortcutRefresh = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(shortcutRefresh, &QShortcut::activated, this, &WidgetBangDieuKhien::xuLyRefresh);
    
    QShortcut *shortcutRefresh2 = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this);
    connect(shortcutRefresh2, &QShortcut::activated, this, &WidgetBangDieuKhien::xuLyRefresh);
    
    // Ctrl+N: Add new book
    shortcutAddBook = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_N), this);
    connect(shortcutAddBook, &QShortcut::activated, this, [this]() {
        emit yeuCauThemSachMoi();
    });
    
    // Ctrl+Shift+N: Add new reader
    shortcutAddReader = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N), this);
    connect(shortcutAddReader, &QShortcut::activated, this, [this]() {
        emit yeuCauDangKyDocGia();
    });
    
    // Ctrl+L: New loan
    shortcutNewLoan = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_L), this);
    connect(shortcutNewLoan, &QShortcut::activated, this, [this]() {
        emit yeuCauTaoPhieuMuon();
    });
    
    // Ctrl+B: Backup
    QShortcut *shortcutBackup = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_B), this);
    connect(shortcutBackup, &QShortcut::activated, this, [this]() {
        emit yeuCauSaoLuu();
    });
    
    // Set tooltips with keyboard shortcuts
    if (btnRefresh) {
        btnRefresh->setToolTip("Làm mới dữ liệu (F5 hoặc Ctrl+R)");
    }
    if (btnThemSachMoi) {
        btnThemSachMoi->setToolTip("Thêm sách mới (Ctrl+N)");
    }
    if (btnDangKyDocGia) {
        btnDangKyDocGia->setToolTip("Đăng ký học viên (Ctrl+Shift+N)");
    }
    if (btnTaoPhieuMuon) {
        btnTaoPhieuMuon->setToolTip("Tạo phiếu mượn (Ctrl+L)");
    }
    if (btnSaoLuu) {
        btnSaoLuu->setToolTip("Sao lưu dữ liệu (Ctrl+B)");
    }
}

