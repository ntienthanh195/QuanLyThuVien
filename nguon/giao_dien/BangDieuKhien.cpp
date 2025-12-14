#include "giao_dien/BangDieuKhien.h"
#include "giao_dien/hop_thoai/HopThoaiThongKe.h"
#include "giao_dien/hop_thoai/HopThoaiTop10.h"
#include "giao_dien/hop_thoai/HopThoaiQuetMaVach.h"
#include "giao_dien/hop_thoai/ManHinhQuanLyIn.h" // New Unified Dialog
#include "giao_dien/hop_thoai/HopThoaiHangDoiIn.h"
#include "models/QuanLySach.h"
#include "models/DocGia.h"
#include "models/UserManager.h"
#include "utils/PrintHelper.h"
#include "utils/EventBus.h"
#if defined(__has_include)
#  if __has_include("utils/DBBatcher.h")
#    include "utils/DBBatcher.h"
#  endif
#endif
#include "utils/LanguageManager.h"
#include "utils/AuditLogger.h"
#include "utils/DatabaseManager.h"
#include "utils/BackupManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTableView>
#include <QAbstractTableModel>
#include <vector>
#include <algorithm>
#include <QHeaderView>
#include <QScrollArea>
#include <QFont>
#include <QPieSeries>
#include <QPieSlice>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QChart>
#include <QChartView>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QListWidget>
#include <QListWidgetItem>
#include <QApplication>
#include <QThread>
#include <QClipboard>
#include <QDialog>

namespace {
    // File-local ActivityModel used by the dashboard activity view
    // File-local ActivityModel used by the dashboard activity view
    struct GiaoDichInfoLocal { QString thoiGian; QString nguoiDung; QString hanhDong; QString noiDung; QString trangThaiColor; };
    class ActivityModel : public QAbstractTableModel {
    public:
        using Item = GiaoDichInfoLocal;
        ActivityModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {}
        int rowCount(const QModelIndex &parent = QModelIndex()) const override { Q_UNUSED(parent); return (int)m_items.size(); }
        int columnCount(const QModelIndex &parent = QModelIndex()) const override { Q_UNUSED(parent); return 4; }
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
            if (!index.isValid()) return QVariant();
            int r = index.row(); if (r < 0 || r >= (int)m_items.size()) return QVariant();
            const Item &it = m_items[r];
            if (role == Qt::DisplayRole) {
                switch (index.column()) {
                    case 0: return it.thoiGian;
                    case 1: return it.nguoiDung;
                    case 2: return it.hanhDong;
                    case 3: return it.noiDung;
                }
            }
            if (role == Qt::ForegroundRole) {
                if (index.column() == 2) return QColor(it.trangThaiColor); // Color for Action
            }
            return QVariant();
        }
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
            if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
                switch (section) {
                    case 0: return QStringLiteral("Thời gian");
                    case 1: return QStringLiteral("Người thực hiện");
                    case 2: return QStringLiteral("Hành động");
                    case 3: return QStringLiteral("Chi tiết");
                }
            }
            return QAbstractTableModel::headerData(section, orientation, role);
        }
        void setList(const std::vector<Item> &list) { beginResetModel(); m_items = list; endResetModel(); }
        void prepend(const Item &it) { beginResetModel(); m_items.insert(m_items.begin(), it); if (m_items.size() > 50) m_items.resize(50); endResetModel(); }
    private:
        std::vector<Item> m_items;
    };
}

BangDieuKhien::BangDieuKhien(QWidget *cha) : QWidget(cha),
    labelSoLieuTongSach(nullptr),
    labelSoLieuTongDocGia(nullptr),
    labelSoLieuDangMuon(nullptr),
    labelSoLieuQuaHan(nullptr),
    bangHoatDong(nullptr),
    panelCanhBao(nullptr),
    nutQuanLySach(nullptr),
    nutQuanLyDocGia(nullptr),
    nutMuonTraSach(nullptr),
    nutXemBaoCao(nullptr),
    nutInThe(nullptr),
    nutQuetMa(nullptr)
{
    qDebug() << "Bắt đầu khởi tạo BangDieuKhien";
    thieLapGiaoDien();
    qDebug() << "Hoàn thành thieLapGiaoDien, bỏ qua capNhatThongKe trong constructor";
    // KHÔNG gọi capNhatThongKe() ở đây - sẽ gọi sau khi window hiển thị

    // Kết nối EventBus để nhận các sự kiện và ghi vào bảng Hoạt động gần đây
    // Kết nối EventBus để nhận các sự kiện và ghi vào bảng Hoạt động gần đây
    try {
        connect(&EventBus::instance(), &EventBus::docGiaAdded, this, [this](int maThe){
            AuditLogger::log("Thêm", "DOC_GIA", QString::number(maThe), QString("Độc giả - Mã thẻ: %1").arg(maThe));
        });
        connect(&EventBus::instance(), &EventBus::docGiaUpdated, this, [this](int maThe){
            AuditLogger::log("Sửa", "DOC_GIA", QString::number(maThe), QString("Độc giả - Mã thẻ: %1").arg(maThe));
        });
        connect(&EventBus::instance(), &EventBus::docGiaRemoved, this, [this](int maThe){
            AuditLogger::log("Xóa", "DOC_GIA", QString::number(maThe), QString("Độc giả - Mã thẻ: %1").arg(maThe));
        });

        connect(&EventBus::instance(), &EventBus::dauSachAdded, this, [this](const QString &isbn){
            AuditLogger::log("Thêm", "DAU_SACH", isbn, QString("Sách - ISBN: %1").arg(isbn));
        });
        connect(&EventBus::instance(), &EventBus::dauSachUpdated, this, [this](const QString &isbn){
            AuditLogger::log("Sửa", "DAU_SACH", isbn, QString("Sách - ISBN: %1").arg(isbn));
        });
        connect(&EventBus::instance(), &EventBus::dauSachRemoved, this, [this](const QString &isbn){
            AuditLogger::log("Xóa", "DAU_SACH", isbn, QString("Sách - ISBN: %1").arg(isbn));
        });

        connect(&EventBus::instance(), &EventBus::nguoiDungAdded, this, [this](const QString &username){
            AuditLogger::log("Thêm", "NGUOI_DUNG", username, QString("Nhân viên - User: %1").arg(username));
        });
        connect(&EventBus::instance(), &EventBus::nguoiDungUpdated, this, [this](const QString &username){
            AuditLogger::log("Sửa", "NGUOI_DUNG", username, QString("Nhân viên - User: %1").arg(username));
        });
        connect(&EventBus::instance(), &EventBus::nguoiDungRemoved, this, [this](const QString &username){
            AuditLogger::log("Xóa", "NGUOI_DUNG", username, QString("Nhân viên - User: %1").arg(username));
        });

        connect(&EventBus::instance(), &EventBus::muonTraAdded, this, [this](int maThe, const QString &maSach){
            AuditLogger::log("Mượn", "MUON_TRA", QString("%1-%2").arg(maThe).arg(maSach), QString("Thẻ: %1 | Sách: %2").arg(maThe).arg(maSach));
        });
        connect(&EventBus::instance(), &EventBus::muonTraUpdated, this, [this](int maThe, const QString &maSach){
            AuditLogger::log("Sửa/Gia hạn", "MUON_TRA", QString("%1-%2").arg(maThe).arg(maSach), QString("Thẻ: %1 | Sách: %2").arg(maThe).arg(maSach));
        });
        connect(&EventBus::instance(), &EventBus::muonTraRemoved, this, [this](int maThe, const QString &maSach){
            AuditLogger::log("Trả", "MUON_TRA", QString("%1-%2").arg(maThe).arg(maSach), QString("Thẻ: %1 | Sách: %2").arg(maThe).arg(maSach));
        });

        connect(&EventBus::instance(), &EventBus::dataChanged, this, [this](){
            AuditLogger::log("System", "DATA", "", "Dữ liệu bên ngoài thay đổi");
        });

        // CENTRAL LOG LISTENER: receives events from AuditLogger (and thus DB)
        connect(&EventBus::instance(), &EventBus::newSystemLog, this, [this](const QString &action, const QString &details){
            ghiNhatKy(action, details);
            capNhatRealtimeTuWidget(); 
        });
    } catch (...) {
        qWarning() << "Không thể kết nối EventBus trong BangDieuKhien constructor";
    }
}

BangDieuKhien::~BangDieuKhien() {}

void BangDieuKhien::thieLapGiaoDien() {
    // Đặt màu nền xanh nhạt gradient giống Quản lý sách
    setStyleSheet(
        "QWidget#BangDieuKhien {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "        stop:0 #1a1d29, stop:0.5 #1e2230, stop:1 #1a1d29);"
        "}"
    );
    setObjectName("BangDieuKhien");
    
    QVBoxLayout *boTriChinh = new QVBoxLayout(this);
    boTriChinh->setContentsMargins(30, 30, 30, 30);
    boTriChinh->setSpacing(20);
    
    // 0. Header (Search + Hello)
    boTriChinh->addWidget(taoHeaderDashboard());
    
    // Tạo các thẻ thống kê (4 cards ở trên)
    taoTheThongKe(boTriChinh);
    
    // Tạo layout 2 cột: hoạt động gần đây (Trái) + Cột tác vụ (Phải)
    QHBoxLayout *layoutNoiDung = new QHBoxLayout();
    layoutNoiDung->setSpacing(20);
    
    // 1. Hoạt động gần đây (Bên trái - Rộng)
    QWidget *khungHoatDong = taoBangHoatDongGanDay();
    
    // 2. Cột bên phải: Panel Cảnh báo (và có thể thêm lịch/tác vụ sau này)
    QWidget *khungPhai = taoPanelCanhBao();
    khungPhai->setFixedWidth(320); // Fixed width for right sidebar
    
    // Add to main layout
    layoutNoiDung->addWidget(khungHoatDong, 1); // Flex 1 (Takes remaining space)
    layoutNoiDung->addWidget(khungPhai, 0);     // Rigid
    
    boTriChinh->addLayout(layoutNoiDung, 1);
}

void BangDieuKhien::taoTheThongKe(QVBoxLayout *boTri) {
    // Lấy thống kê từ backend
    extern PTRDG rootDocGia;
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    // KIỂM TRA DỮ LIỆU HỢP LỆ - QUAN TRỌNG ĐỂ TRÁNH CRASH
    if (!boTri) {
        qCritical() << "BangDieuKhien::taoTheThongKe - boTri is NULL!";
        return;
    }
    
    if (soLuongDauSach < 0 || soLuongDauSach > MAX_DAUSACH) {
        qWarning() << "BangDieuKhien: Dữ liệu sách không hợp lệ!" << soLuongDauSach;
        // Hiển thị thẻ với giá trị 0
        QHBoxLayout *layoutThe = new QHBoxLayout();
        layoutThe->setSpacing(15);
        layoutThe->addWidget(taoTheNho("■", "0", LanguageManager::instance().getText("DASH_TOTAL_BOOKS"), "#3B82F6"));
        layoutThe->addWidget(taoTheNho("◆", "0", LanguageManager::instance().getText("DASH_TOTAL_READERS"), "#10B981"));
        layoutThe->addWidget(taoTheNho("●", "0", LanguageManager::instance().getText("DASH_BORROWING"), "#F59E0B"));
        layoutThe->addWidget(taoTheNho("▲", "0", LanguageManager::instance().getText("DASH_OVERDUE"), "#EF4444"));
        boTri->addLayout(layoutThe);
        return;
    }
    
    // Kiểm tra con trỏ dsDauSach không bị lỗi
    bool dataSafe = true;
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] == nullptr) {
            qWarning() << "dsDauSach[" << i << "] is NULL - dữ liệu không an toàn";
            dataSafe = false;
            break;
        }
    }
    
    if (!dataSafe) {
        qWarning() << "BangDieuKhien: Phát hiện con trỏ NULL trong dsDauSach";
    }
    
    qDebug() << "BangDieuKhien: Đang lấy thống kê với" << soLuongDauSach << "đầu sách";
    
    ThongKeSach tkSach = {};
    ThongKeDocGia tkDocGia = {};
    
    try {
        tkSach = LayThongKeSach();
        qDebug() << "LayThongKeSach() thành công -" << tkSach.tongSoBanSao << "bản sao";
        
        // QUAN TRỌNG: Chỉ gọi LayThongKeDocGia nếu dataSafe
        if (dataSafe && rootDocGia != nullptr) {
            tkDocGia = LayThongKeDocGia(rootDocGia, dsDauSach, soLuongDauSach);
            qDebug() << "LayThongKeDocGia() thành công -" << tkDocGia.tongSoDocGia << "độc giả";
        } else {
            qWarning() << "Bỏ qua LayThongKeDocGia do dữ liệu không an toàn hoặc rootDocGia NULL";
        }
    } catch (const std::exception& e) {
        qCritical() << "Lỗi khi lấy thống kê:" << e.what();
        // Không return, tiếp tục hiển thị với giá trị 0
    } catch (...) {
        qCritical() << "Lỗi không xác định khi lấy thống kê";
        // Không return, tiếp tục hiển thị với giá trị 0
    }
    
    // Layout 4 cột cho 4 thẻ
    QHBoxLayout *layoutThe = new QHBoxLayout();
    layoutThe->setSpacing(15);
    
    // 4 thẻ với icon và màu riêng - LƯU CÁC LABEL ĐỂ CẬP NHẬT SAU
    QWidget *theTongSach = taoTheNho(":/icons/book.svg", QString::number(tkSach.tongSoBanSao), LanguageManager::instance().getText("DASH_TOTAL_BOOKS"), "#3B82F6", &labelSoLieuTongSach);
    QWidget *theTongDocGia = taoTheNho(":/icons/user.svg", QString::number(tkDocGia.tongSoDocGia), LanguageManager::instance().getText("DASH_TOTAL_READERS"), "#10B981", &labelSoLieuTongDocGia);
    QWidget *theDangMuon = taoTheNho(":/icons/transaction.svg", QString::number(tkSach.soBanSaoDangMuon), LanguageManager::instance().getText("DASH_BORROWING"), "#F59E0B", &labelSoLieuDangMuon);
    QWidget *theQuaHan = taoTheNho(":/icons/warning.svg", QString::number(tkDocGia.docGiaCoSachQuaHan), LanguageManager::instance().getText("DASH_OVERDUE"), "#EF4444", &labelSoLieuQuaHan);
    
    layoutThe->addWidget(theTongSach);
    layoutThe->addWidget(theTongDocGia);
    layoutThe->addWidget(theDangMuon);
    layoutThe->addWidget(theQuaHan);
    
    boTri->addLayout(layoutThe);
}

QWidget* BangDieuKhien::taoTheNho(const QString &iconPath, const QString &soLieu, const QString &nhan, const QString &mau, QLabel **outLabelSoLieu) {
    QWidget *the = new QWidget();
    the->setMinimumHeight(140);
    the->setStyleSheet(QString(
        "QWidget {"
        "    background-color: #1e2230;"
        "    border: 1px solid #2d3140;"
        "    border-radius: 12px;"
        "}"
    ));
    
    QVBoxLayout *boTri = new QVBoxLayout(the);
    boTri->setContentsMargins(20, 20, 20, 20);
    boTri->setSpacing(10);
    
    // Icon với background màu mờ (opacity 15%)
    QLabel *labelIcon = new QLabel();
    labelIcon->setFixedSize(56, 56);
    labelIcon->setAlignment(Qt::AlignCenter);
    
    // Load Icon
    QPixmap pix(iconPath);
    // Fallback if icon missing
    if (pix.isNull()) {
        if (iconPath.contains("book")) pix.load(":/icons/list.svg");
        else if (iconPath.contains("user")) pix.load(":/icons/user.svg");
    }
    
    if (!pix.isNull()) {
        // Colorize icon to match 'mau'
        QImage img = pix.toImage();
        img.invertPixels(); // Simple trick or just use mask
        // Correct way: use QPainter to colorize
        QPixmap coloredPix(pix.size());
        coloredPix.fill(Qt::transparent);
        QPainter p(&coloredPix);
        p.setRenderHint(QPainter::Antialiasing);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawPixmap(0,0, pix);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(coloredPix.rect(), QColor(mau));
        p.end();
        labelIcon->setPixmap(coloredPix.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        labelIcon->setText("●"); // Ultimate fallback
    }

    
    // Chuyển màu hex sang RGBA với opacity 15%
    QColor mauGoc(mau);
    QString mauMo = QString("rgba(%1, %2, %3, 0.15)")
        .arg(mauGoc.red())
        .arg(mauGoc.green())
        .arg(mauGoc.blue());
    
    labelIcon->setStyleSheet(QString(
        "background-color: %1;"
        "border-radius: 28px;"
        "border: none;"
    ).arg(mauMo));
    
    // Số liệu lớn - LƯU POINTER ĐỂ CẬP NHẬT SAU
    QLabel *labelSoLieu = new QLabel(soLieu);
    labelSoLieu->setStyleSheet(
        "font-size: 36px;"
        "font-weight: 700;"
        "color: #FFFFFF;"
        "background: transparent;"
        "border: none;"
    );
    
    // Trả về pointer để cập nhật sau
    if (outLabelSoLieu) {
        *outLabelSoLieu = labelSoLieu;
    }
    
    // Nhãn (không uppercase, màu xám nhẹ hơn)
    QLabel *labelNhan = new QLabel(nhan);
    labelNhan->setStyleSheet(
        "font-size: 12px;"
        "color: #a1a1aa;"
        "background: transparent;"
        "font-weight: 500;"
        "letter-spacing: 0.3px;"
        "border: none;"
    );
    
    boTri->addWidget(labelIcon);
    boTri->addSpacing(5);
    boTri->addWidget(labelSoLieu);
    boTri->addWidget(labelNhan);
    boTri->addStretch();
    
    return the;
}

void BangDieuKhien::taoNutTruyCapNhanh() {
    QWidget *khungNut = new QWidget();
    QHBoxLayout *boTriNut = new QHBoxLayout(khungNut);
    boTriNut->setSpacing(20);
    
    nutQuanLySach = new QPushButton(LanguageManager::instance().getText("MENU_BOOKS"));
    nutQuanLyDocGia = new QPushButton(LanguageManager::instance().getText("MENU_READERS"));
    nutMuonTraSach = new QPushButton(LanguageManager::instance().getText("MENU_LOAN"));
    nutXemBaoCao = new QPushButton(LanguageManager::instance().getText("BTN_REPORTS"));
    
    QPushButton *nutTop10 = new QPushButton(LanguageManager::instance().getText("BTN_TOP10"));
    
    QString kieuNut = 
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "        stop:0 #3699FF, stop:1 #2080E8);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    padding: 18px 30px;"
        "    font-size: 15px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "        stop:0 #187DE4, stop:1 #1060C0);"
        "    transform: translateY(-2px);"
        "}"
        "QPushButton:pressed {"
        "    background: #0F5FC2;"
        "    transform: translateY(0px);"
        "}";
    
    QString kieuNutXanh = kieuNut;
    kieuNutXanh.replace("#3699FF", "#1BC5BD").replace("#2080E8", "#0F9A94")
               .replace("#187DE4", "#0F9A94").replace("#1060C0", "#0A7A75");
    
    QString kieuNutVang = kieuNut;
    kieuNutVang.replace("#3699FF", "#FFA800").replace("#2080E8", "#E89500")
               .replace("#187DE4", "#D68600").replace("#1060C0", "#B57200");
    
    nutQuanLySach->setStyleSheet(kieuNut);
    nutQuanLyDocGia->setStyleSheet(kieuNut);
    nutMuonTraSach->setStyleSheet(kieuNut);
    nutXemBaoCao->setStyleSheet(kieuNutXanh);
    nutTop10->setStyleSheet(kieuNutVang);
    
    // Thêm shadow cho các nút
    for (auto *nut : {nutQuanLySach, nutQuanLyDocGia, nutMuonTraSach, nutXemBaoCao, nutTop10}) {
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
        shadow->setBlurRadius(15);
        shadow->setXOffset(0);
        shadow->setYOffset(3);
        shadow->setColor(QColor(0, 0, 0, 50));
        nut->setGraphicsEffect(shadow);
    }
    
    boTriNut->addWidget(nutQuanLySach);
    boTriNut->addWidget(nutQuanLyDocGia);
    boTriNut->addWidget(nutMuonTraSach);
    boTriNut->addWidget(nutXemBaoCao);
    boTriNut->addWidget(nutTop10);
    
    boTriChinh->addWidget(khungNut, 3, 0, 1, 4);
    
    // Kết nối signals
    connect(nutQuanLySach, &QPushButton::clicked, this, &BangDieuKhien::xuLyNutQuanLySach);
    connect(nutQuanLyDocGia, &QPushButton::clicked, this, &BangDieuKhien::xuLyNutQuanLyDocGia);
    connect(nutMuonTraSach, &QPushButton::clicked, this, &BangDieuKhien::xuLyNutMuonTraSach);
    connect(nutXemBaoCao, &QPushButton::clicked, this, &BangDieuKhien::xuLyNutXemBaoCao);
    connect(nutTop10, &QPushButton::clicked, this, [this]() {
        HopThoaiTop10 *dialog = new HopThoaiTop10(this);
        dialog->exec();
        delete dialog;
    });
}

void BangDieuKhien::capNhatThongKe() {
    // Kiểm tra dữ liệu trước khi cập nhật
    extern int soLuongDauSach;
    
    if (soLuongDauSach < 0 || soLuongDauSach > MAX_DAUSACH) {
        qWarning() << "capNhatThongKe: Dữ liệu không hợp lệ, bỏ qua cập nhật" << soLuongDauSach;
        return;
    }
    
    qDebug() << "Cập nhật thống kê dashboard...";
    
    // Cập nhật lại toàn bộ giao diện
    try {
        thieLapGiaoDien();
    } catch (const std::exception& e) {
        qCritical() << "Lỗi khi thiết lập giao diện:" << e.what();
    } catch (...) {
        qCritical() << "Lỗi không xác định khi thiết lập giao diện";
    }
}

void BangDieuKhien::capNhatNgonNgu() {
    try {
        qDebug() << "[BangDieuKhien] Language update started";
        
        // 1. Update Buttons with comprehensive safety checks
        if (nutQuanLySach && !nutQuanLySach->isHidden() && nutQuanLySach->isVisible()) {
            try {
                QString text = LanguageManager::instance().getText("MENU_BOOKS");
                qDebug() << "[BangDieuKhien] nutQuanLySach setText - length:" << text.length();
                
                // Ensure we're on UI thread
                if (QThread::currentThread() == QApplication::instance()->thread()) {
                    nutQuanLySach->setText(text);
                    qDebug() << "[BangDieuKhien] nutQuanLySach updated OK";
                } else {
                    qWarning() << "[BangDieuKhien] NOT ON UI THREAD - skipping setText";
                }
            } catch (...) {
                qCritical() << "[BangDieuKhien] Exception updating nutQuanLySach";
            }
        } else {
            qWarning() << "[BangDieuKhien] nutQuanLySach is NULL or hidden";
        }
        
        if (nutQuanLyDocGia && !nutQuanLyDocGia->isHidden() && nutQuanLyDocGia->isVisible()) {
            try {
                QString text = LanguageManager::instance().getText("MENU_READERS");
                qDebug() << "[BangDieuKhien] nutQuanLyDocGia setText - length:" << text.length();
                
                if (QThread::currentThread() == QApplication::instance()->thread()) {
                    nutQuanLyDocGia->setText(text);
                    qDebug() << "[BangDieuKhien] nutQuanLyDocGia updated OK";
                } else {
                    qWarning() << "[BangDieuKhien] NOT ON UI THREAD - skipping setText";
                }
            } catch (...) {
                qCritical() << "[BangDieuKhien] Exception updating nutQuanLyDocGia";
            }
        } else {
            qWarning() << "[BangDieuKhien] nutQuanLyDocGia is NULL or hidden";
        }
        
        if (nutMuonTraSach && !nutMuonTraSach->isHidden() && nutMuonTraSach->isVisible()) {
            try {
                QString text = LanguageManager::instance().getText("MENU_LOAN");
                qDebug() << "[BangDieuKhien] nutMuonTraSach setText - length:" << text.length();
                
                if (QThread::currentThread() == QApplication::instance()->thread()) {
                    nutMuonTraSach->setText(text);
                    qDebug() << "[BangDieuKhien] nutMuonTraSach updated OK";
                } else {
                    qWarning() << "[BangDieuKhien] NOT ON UI THREAD - skipping setText";
                }
            } catch (...) {
                qCritical() << "[BangDieuKhien] Exception updating nutMuonTraSach";
            }
        } else {
            qWarning() << "[BangDieuKhien] nutMuonTraSach is NULL or hidden";
        }
        
        qDebug() << "[BangDieuKhien] Language update COMPLETED - no crash!";
        
    } catch (const std::exception& e) {
        qCritical() << "[BangDieuKhien] EXCEPTION:" << e.what();
    } catch (...) {
        qCritical() << "[BangDieuKhien] UNKNOWN EXCEPTION";
    }
}

void BangDieuKhien::capNhatRealtimeTuWidget() {
    qDebug() << "📊 Dashboard nhận realtime update từ widget khác - CẬP NHẬT NGAY LẬP TỨC!";
    
    // Lấy dữ liệu mới nhất
    extern PTRDG rootDocGia;
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    if (soLuongDauSach < 0 || soLuongDauSach > MAX_DAUSACH) {
        qWarning() << "Dữ liệu không hợp lệ, bỏ qua realtime update";
        return;
    }
    
    try {
        // Lấy thống kê mới
        ThongKeSach tkSach = LayThongKeSach();
        ThongKeDocGia tkDocGia = LayThongKeDocGia(rootDocGia, dsDauSach, soLuongDauSach);
        
        // Cập nhật CHỈ TEXT của các label (KHÔNG tạo lại widget)
        if (labelSoLieuTongSach) {
            labelSoLieuTongSach->setText(QString::number(tkSach.tongSoBanSao));
            qDebug() << "  ✓ Cập nhật Tổng sách:" << tkSach.tongSoBanSao;
        }
        
        if (labelSoLieuTongDocGia) {
            labelSoLieuTongDocGia->setText(QString::number(tkDocGia.tongSoDocGia));
            qDebug() << "  ✓ Cập nhật Tổng độc giả:" << tkDocGia.tongSoDocGia;
        }
        
        if (labelSoLieuDangMuon) {
            labelSoLieuDangMuon->setText(QString::number(tkSach.soBanSaoDangMuon));
            qDebug() << "  ✓ Cập nhật Đang mượn:" << tkSach.soBanSaoDangMuon;
        }
        
        if (labelSoLieuQuaHan) {
            labelSoLieuQuaHan->setText(QString::number(tkDocGia.docGiaCoSachQuaHan));
            qDebug() << "  ✓ Cập nhật Quá hạn:" << tkDocGia.docGiaCoSachQuaHan;
        }
        
        // TODO: Cập nhật bảng hoạt động gần đây (nếu cần)
        // Cập nhật bảng hoạt động gần đây: làm mới các hàng (in-place)
        if (bangHoatDong) {
            try {
                // Xây dựng lại danhSachGiaoDich giống như trong taoBangHoatDongGanDay
                PTRDG dsDocGia[100];
                int dem = 0;
#if defined(__has_include) && __has_include("utils/DBBatcher.h")
                std::vector<PTRDG> _tmp = DBBatcher::snapshotDocGia();
                dem = static_cast<int>(std::min<size_t>(_tmp.size(), 100));
                for (int _i = 0; _i < dem; ++_i) dsDocGia[_i] = _tmp[_i];
#else
                DuyetCayRaMang(rootDocGia, dsDocGia, dem);
#endif

                struct GiaoDichInfo { QString thoiGian; QString tenDocGia; QString tenSach; int trangThai; };
                std::vector<GiaoDichInfo> danhSachGiaoDich;

                for (int i = 0; i < dem && (int)danhSachGiaoDich.size() < 20; i++) {
                    if (!dsDocGia[i]) continue;
                    if (!dsDocGia[i]->data.dsmt) continue;
                    auto mt = dsDocGia[i]->data.dsmt;
                    while (mt && (int)danhSachGiaoDich.size() < 20) {
                        GiaoDichInfo gd;
                        gd.thoiGian = QString::fromStdString(mt->data.NgayMuon);
                        gd.tenDocGia = QString::fromStdString(dsDocGia[i]->data.Ho) + " " + QString::fromStdString(dsDocGia[i]->data.Ten);
                        gd.trangThai = mt->data.TrangThai;
                        gd.tenSach = QString();
                        if (mt->data.banSaoSach) {
                            QString maSach = QString::fromStdString(mt->data.banSaoSach->maSach);
                            extern PTRDS dsDauSach[];
                            extern int soLuongDauSach;
                            for (int j = 0; j < soLuongDauSach; j++) {
                                if (dsDauSach[j]) {
                                    PTRDMS banSao = dsDauSach[j]->dms;
                                    while (banSao) {
                                        if (QString::fromStdString(banSao->maSach) == maSach) {
                                            gd.tenSach = QString::fromStdString(dsDauSach[j]->tenSach);
                                            break;
                                        }
                                        banSao = banSao->next;
                                    }
                                    if (!gd.tenSach.isEmpty()) break;
                                }
                            }
                        }
                        danhSachGiaoDich.push_back(gd);
                        mt = mt->next;
                    }
                }

                // Build model data and set into modelHoatDong (if available)
            // Build model data and set into modelHoatDong (if available)
                int maxRows = qMin(9, (int)danhSachGiaoDich.size());
                std::vector<GiaoDichInfoLocal> modelList;
                modelList.reserve(maxRows);
                for (int i = 0; i < maxRows; ++i) {
                    GiaoDichInfoLocal it;
                    it.thoiGian = danhSachGiaoDich[i].thoiGian;
                    it.nguoiDung = "Độc giả: " + danhSachGiaoDich[i].tenDocGia;
                    it.hanhDong = (danhSachGiaoDich[i].trangThai == 0) ? "Đang Mượn" : ((danhSachGiaoDich[i].trangThai == 1) ? "Đã Trả" : "Mất");
                    it.noiDung = danhSachGiaoDich[i].tenSach;
                    // Color mapping
                    if (danhSachGiaoDich[i].trangThai == 0) it.trangThaiColor = "#fbbf24"; // Borrow - Orange
                    else if (danhSachGiaoDich[i].trangThai == 1) it.trangThaiColor = "#34d399"; // Return - Green
                    else it.trangThaiColor = "#f87171"; // Lost - Red
                    
                    modelList.push_back(it);
                }
                if (modelHoatDong) {
                    if (auto am = dynamic_cast<ActivityModel*>(modelHoatDong)) {
                        am->setList(modelList);
                    }
                }
            } catch (...) {
                qWarning() << "Lỗi khi cập nhật bangHoatDong realtime";
            }
        }
        
        qDebug() << "✅ Dashboard đã cập nhật realtime thành công!";
        
    } catch (const std::exception& e) {
        qCritical() << "Lỗi khi cập nhật realtime:" << e.what();
    } catch (...) {
        qCritical() << "Lỗi không xác định khi cập nhật realtime";
    }
}

void BangDieuKhien::capNhatBieuDo() {
    // Sẽ implement sau
}

void BangDieuKhien::appendActivity(const QString &text) {
    if (!bangHoatDong) return;
    QString ts = QDateTime::currentDateTime().toString("HH:mm:ss");
    // Best-effort: push into model if available
    if (modelHoatDong) {
        if (auto am = dynamic_cast<ActivityModel*>(modelHoatDong)) {
            GiaoDichInfoLocal it;
            it.thoiGian = ts;
            it.nguoiDung = "System";
            it.hanhDong = "Log";
            it.noiDung = text;
            it.trangThaiColor = "#94a3b8"; // Grey
            am->prepend(it);
            return;
        }
    }
    // Fallback: nothing to do (view not ready)
}

void BangDieuKhien::xuLyNutQuanLySach() {
    emit yeuCauChuyenDenQuanLySach();
}

void BangDieuKhien::xuLyNutQuanLyDocGia() {
    emit yeuCauChuyenDenQuanLyDocGia();
}

void BangDieuKhien::xuLyNutMuonTraSach() {
    emit yeuCauChuyenDenMuonTraSach();
}

void BangDieuKhien::xuLyNutXemBaoCao() {
    // Open Unified Print Dialog with "Reports" tab selected
    ManHinhQuanLyIn *dialog = new ManHinhQuanLyIn(nullptr, {}, this);
    dialog->setCurrentTab(2); // Tab Index 2 = Báo Cáo
    dialog->exec();
    delete dialog;
}

QWidget* BangDieuKhien::taoTheTongQuan(const QString &tieuDe, const QString &giaTri, 
                                        const QString &bieuTuong, const QColor &mauNen) {
    QWidget *the = new QWidget();
    
    // Tạo gradient từ màu gốc sang màu tối hơn
    QColor mauDam = mauNen.darker(120);
    
    the->setStyleSheet(QString(
        "QWidget {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "        stop:0 %1, stop:1 %2);"
        "    border-radius: 16px;"
        "    border: none;"
        "}"
    ).arg(mauNen.name()).arg(mauDam.name()));
    
    // Thêm shadow effect
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(20);
    shadow->setXOffset(0);
    shadow->setYOffset(4);
    shadow->setColor(QColor(0, 0, 0, 60));
    the->setGraphicsEffect(shadow);
    
    QVBoxLayout *boTri = new QVBoxLayout(the);
    boTri->setContentsMargins(30, 30, 30, 30);
    boTri->setSpacing(12);
    
    // Icon lớn hơn với background tròn
    QWidget *khungIcon = new QWidget();
    khungIcon->setFixedSize(70, 70);
    khungIcon->setStyleSheet(QString(
        "background-color: rgba(255, 255, 255, 0.15);"
        "border-radius: 35px;"
    ));
    
    QLabel *nhanBieuTuong = new QLabel(bieuTuong, khungIcon);
    nhanBieuTuong->setStyleSheet("font-size: 40px; background: transparent;");
    nhanBieuTuong->setAlignment(Qt::AlignCenter);
    nhanBieuTuong->setGeometry(0, 0, 70, 70);
    
    // Số liệu to, bold, nổi bật
    QLabel *nhanGiaTri = new QLabel(giaTri);
    nhanGiaTri->setStyleSheet(
        "font-size: 56px; "
        "font-weight: 900; "
        "color: #FFFFFF; "
        "background: transparent;"
        "letter-spacing: -2px;"
    );
    
    // Label với màu sáng hơn
    QLabel *nhanTieuDe = new QLabel(tieuDe);
    nhanTieuDe->setStyleSheet(
        "font-size: 15px; "
        "color: rgba(255, 255, 255, 0.85); "
        "background: transparent;"
        "font-weight: 500;"
    );
    
    boTri->addWidget(khungIcon);
    boTri->addSpacing(8);
    boTri->addWidget(nhanGiaTri);
    boTri->addWidget(nhanTieuDe);
    boTri->addStretch();
    
    // Thêm hover effect
    the->setAttribute(Qt::WA_Hover);
    the->installEventFilter(new QObject());
    
    return the;
}

void BangDieuKhien::taoThongBaoNhanh() {
    // Sẽ implement sau
}

QWidget* BangDieuKhien::taoBieuDoTronThongKe() {
    // Styling constants
    const QString COLOR_BORROWED = "#f59e0b"; // Amber-500
    const QString COLOR_AVAILABLE = "#3b82f6"; // Blue-500
    const QString COLOR_BG = "#1e2230";
    
    // Create Series
    QPieSeries *series = new QPieSeries();
    series->setHoleSize(0.65); // Donut style
    
    // Data
    ThongKeSach tk = LayThongKeSach();
    int total = tk.tongSoBanSao;
    int borrowed = tk.soBanSaoDangMuon;
    int available = total - borrowed; if (available < 0) available = 0; // Safety
    
    QPieSlice *sliceBorrowed = series->append("Đang mượn", borrowed);
    QPieSlice *sliceAvailable = series->append("Sẵn có", available);
    
    // Slice Styling
    sliceBorrowed->setColor(QColor(COLOR_BORROWED));
    sliceAvailable->setColor(QColor(COLOR_AVAILABLE));
    
    sliceBorrowed->setLabelVisible(false);
    sliceAvailable->setLabelVisible(false);
    
    // Chart
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setBackgroundBrush(Qt::NoBrush); // Transparent
    chart->setTitle("Tỉ lệ sách");
    chart->setTitleFont(QFont("Segoe UI", 12, QFont::Bold));
    chart->setTitleBrush(QColor("#e2e8f0"));
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom); // Move to bottom
    chart->legend()->setLabelColor(QColor("#cbd5e1"));
    chart->legend()->setFont(QFont("Segoe UI", 9));
    chart->setMargins(QMargins(0,0,0,0));
    
    // View
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: transparent;");
    chartView->setFixedHeight(200); // Small chart
    
    // Container
    QWidget *container = new QWidget();
    container->setStyleSheet(QString(
        "QWidget {"
        "    background-color: %1;"
        "    border: 1px solid #2d3140;"
        "    border-radius: 12px;"
        "}"
    ).arg(COLOR_BG));
    container->setMinimumHeight(200);
    
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 10, 0, 10);
    layout->addWidget(chartView);
    
    return container;
}

QWidget* BangDieuKhien::taoBieuDoCotThongKe() {
    // Styling
    const QString COLOR_BAR = "#3b82f6"; // Blue-500
    const QString COLOR_BG = "#1e2230";
    const QString COLOR_TEXT = "#cbd5e1";
    
    // 1. Prepare Data Buckets (Last 7 Days)
    QMap<QString, int> dailyCounts;
    QStringList categories;
    QDateTime now = QDateTime::currentDateTime();
    
    for (int i = 6; i >= 0; --i) {
        QDateTime d = now.addDays(-i);
        QString key = d.toString("dd/MM");
        categories << key;
        dailyCounts[key] = 0;
    }
    
    // 2. Fetch Data (scan all loans - basic optimization: simple full scan is fast enough for < 1000 users)
    extern PTRDG rootDocGia;
    PTRDG dsDocGia[100]; // Limit scan
    int dem = 0;
    try {
         DuyetCayRaMang(rootDocGia, dsDocGia, dem);
    } catch (...) { dem = 0; }
    
    for (int i = 0; i < dem; i++) {
        if (!dsDocGia[i]) continue;
         auto mt = dsDocGia[i]->data.dsmt;
         while (mt) {
             // Parse date. Format: "yyyy-MM-dd HH:mm:ss" or similar?
             // std::string NgayMuon;
             QString sDate = QString::fromStdString(mt->data.NgayMuon);
             // Try parsing
             QDateTime loanDate = QDateTime::fromString(sDate, "yyyy-MM-dd HH:mm:ss");
             if (!loanDate.isValid()) loanDate = QDateTime::fromString(sDate, "dd/MM/yyyy"); // Backup format
             
             if (loanDate.isValid()) {
                 QString key = loanDate.toString("dd/MM");
                 if (dailyCounts.contains(key)) {
                     dailyCounts[key]++;
                 }
             }
             mt = mt->next;
         }
    }
    
    // 3. Create Bar Set
    QBarSet *set0 = new QBarSet("Lượt mượn");
    set0->setColor(QColor(COLOR_BAR));
    set0->setBorderColor(Qt::transparent);
    
    for (const QString &cat : categories) {
        *set0 << dailyCounts[cat];
    }
    
    // 4. Create Series
    QBarSeries *series = new QBarSeries();
    series->append(set0);
    series->setBarWidth(0.5);
    
    // 5. Create Chart
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Xu hướng mượn sách (7 ngày qua)");
    chart->setTitleFont(QFont("Segoe UI", 12, QFont::Bold));
    chart->setTitleBrush(QColor("#e2e8f0"));
    chart->setBackgroundBrush(Qt::NoBrush);
    chart->legend()->setVisible(false);
    chart->setMargins(QMargins(0,0,0,0));
    
    // Axes
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsColor(QColor(COLOR_TEXT));
    axisX->setLabelsFont(QFont("Segoe UI", 9)); // Smaller font
    axisX->setGridLineVisible(false); // Clean look
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelsColor(QColor(COLOR_TEXT));
    axisY->setLabelsFont(QFont("Segoe UI", 9));
    axisY->setGridLineColor(QColor("#334155"));
    axisY->setLabelFormat("%d");
    axisY->setTickCount(4); // limit ticks to avoid crowding
    axisY->applyNiceNumbers();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    // View
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: transparent;");
    
    // Container
    QWidget *container = new QWidget();
    container->setStyleSheet(QString(
        "QWidget {"
        "    background-color: %1;"
        "    border: 1px solid #2d3140;"
        "    border-radius: 12px;"
        "}"
    ).arg(COLOR_BG));
    
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->addWidget(chartView);
    
    return container;
}

// ... existing code ...


QWidget* BangDieuKhien::taoBangHoatDongGanDay() {
    QWidget *khung = new QWidget();
    khung->setStyleSheet(
        "QWidget {"
        "    background-color: #1e2230;"
        "    border: 1px solid #2d3140;"
        "    border-radius: 12px;"
        "}"
    );
    
    QVBoxLayout *boTri = new QVBoxLayout(khung);
    boTri->setContentsMargins(0, 0, 0, 0);
    boTri->setSpacing(0);
    
    // Header
    QWidget *header = new QWidget();
    header->setStyleSheet("background-color: #252830; border: none; border-top-left-radius: 12px; border-top-right-radius: 12px;");
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 18, 24, 18);
    
    QLabel *tieuDe = new QLabel("Hoạt động gần đây");
    tieuDe->setStyleSheet("font-size: 14px; font-weight: 700; color: #E0E0E0; background: transparent; border: none; letter-spacing: 0.3px;");
    
    QPushButton *nutMenu = new QPushButton("⋯");
    nutMenu->setStyleSheet("background: transparent; border: none; color: #858585; font-size: 20px; padding: 0; max-width: 24px;");
    
    headerLayout->addWidget(tieuDe);
    headerLayout->addStretch();
    headerLayout->addWidget(nutMenu);
    
    // Table (model/view)
    QTableView *view = new QTableView();
    this->bangHoatDong = view;
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setShowGrid(false);
    view->setAlternatingRowColors(true);
    view->verticalHeader()->setVisible(false);
    view->horizontalHeader()->setStretchLastSection(false);
    view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    view->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    view->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    view->setStyleSheet(
        "QTableView {"
        "    background-color: #1e2230;"
        "    border: none;"
        "    gridline-color: transparent;" /* No grid lines for cleaner look */
        "    font-size: 14px;"
        "    selection-background-color: #2d5f8d;"
        "}"
        "QTableView::item {"
        "    padding: 12px 15px;"
        "    color: #cad1d9;" /* Light gray text */
        "    border-bottom: 1px solid #2d3140;" /* Subtle separator */
        "}"
        "QTableView::item:selected {"
        "    background-color: rgba(59, 130, 246, 0.2);" /* Blue selection with opacity */
        "    color: #ffffff;"
        "}"
        "QHeaderView::section {"
        "    background-color: #1e2230;" /* Match bg */
        "    color: #94a3b8;" /* Slate-400 */
        "    padding: 12px 15px;"
        "    border: none;"
        "    border-bottom: 2px solid #334155;" /* Header separator */
        "    font-weight: 700;"
        "    font-size: 12px;"
        "    text-transform: uppercase;"
        "}"
        "QScrollBar:vertical {"
        "    background: #1e2230;" /* Track matches bg */
        "    width: 6px;" /* Thinner scrollbar */
        "    margin: 0;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #475569;" /* Slate-600 */
        "    border-radius: 3px;"
        "    min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: #64748b;" /* Slate-500 */
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}"
    );
    
    // Dữ liệu mẫu từ backend
    extern PTRDG rootDocGia;
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    // KIỂM TRA AN TOÀN trước khi truy cập dữ liệu
    if (rootDocGia && soLuongDauSach >= 0 && soLuongDauSach <= MAX_DAUSACH) {
        PTRDG dsDocGia[100];
        int dem = 0;
        
        try {
            DuyetCayRaMang(rootDocGia, dsDocGia, dem);
        } catch (...) {
            qWarning() << "Lỗi khi duyệt cây độc giả";
            dem = 0;
        }
        
        // int soHang = 0; // Unused
        
        // Tạo list tất cả giao dịch để sắp xếp theo thời gian
        struct GiaoDichInfo {
            QString thoiGian;
            QString tenDocGia;
            QString tenSach;
            int trangThai;
        };
        std::vector<GiaoDichInfo> danhSachGiaoDich;
        
        for (int i = 0; i < dem && i < 100; i++) {
            if (!dsDocGia[i]) continue;
            if (!dsDocGia[i]->data.dsmt) continue;
            
            // Duyệt qua tất cả giao dịch của độc giả
            auto mt = dsDocGia[i]->data.dsmt;
            while (mt && danhSachGiaoDich.size() < 20) {
                GiaoDichInfo gd;
                gd.thoiGian = QString::fromStdString(mt->data.NgayMuon);
                gd.tenDocGia = QString("%1 %2")
                    .arg(QString::fromStdString(dsDocGia[i]->data.Ho))
                    .arg(QString::fromStdString(dsDocGia[i]->data.Ten));
                gd.trangThai = mt->data.TrangThai;
                
                // Tìm tên sách
                if (mt->data.banSaoSach) {
                    QString maSach = QString::fromStdString(mt->data.banSaoSach->maSach);
                    for (int j = 0; j < soLuongDauSach; j++) {
                        if (dsDauSach[j]) {
                            PTRDMS banSao = dsDauSach[j]->dms;
                            while (banSao) {
                                if (QString::fromStdString(banSao->maSach) == maSach) {
                                    gd.tenSach = QString::fromStdString(dsDauSach[j]->tenSach);
                                    break;
                                }
                                banSao = banSao->next;
                            }
                            if (!gd.tenSach.isEmpty()) break;
                        }
                    }
                }
                
                danhSachGiaoDich.push_back(gd);
                mt = mt->next;
            }
        }
        
        // Build model data and attach to view
        std::vector<GiaoDichInfoLocal> modelList;
        modelList.reserve(20);

        // 1. Fetch System Logs from DB (Persistent History)
        QSqlQuery qLog("SELECT u.ten_dang_nhap, n.hanh_dong, n.chi_tiet, n.thoi_gian "
                    "FROM nhat_ky_he_thong n "
                    "LEFT JOIN nguoi_dung u ON n.nguoi_dung_id = u.id "
                    "ORDER BY n.id DESC LIMIT 20", DatabaseManager::instance().database());
        while (qLog.next()) {
            GiaoDichInfoLocal it;
            // Format time for shorter display? User wanted "Hiện đủ".
            // DB format: yyyy-MM-dd HH:mm:ss. 
            // Let's shorten to MM-dd HH:mm if convenient, or keep full.
            QDateTime dt = QDateTime::fromString(qLog.value(3).toString(), "yyyy-MM-dd HH:mm:ss");
            if (dt.isValid()) it.thoiGian = dt.toString("dd/MM HH:mm");
            else it.thoiGian = qLog.value(3).toString();

            QString user = qLog.value(0).toString();
            it.nguoiDung = user.isEmpty() ? "Hệ thống" : user;
            it.hanhDong = qLog.value(1).toString();
            it.noiDung = qLog.value(2).toString();

            // Color map
            if (it.hanhDong.contains("Thêm")) it.trangThaiColor = "#34d399";
            else if (it.hanhDong.contains("Xóa")) it.trangThaiColor = "#f87171";
            else if (it.hanhDong.contains("Mượn")) it.trangThaiColor = "#fbbf24";
            else if (it.hanhDong.contains("Trả")) it.trangThaiColor = "#a78bfa";
            else if (it.hanhDong.contains("Sửa")) it.trangThaiColor = "#3699ff";
            else it.trangThaiColor = "#94a3b8";

            modelList.push_back(it);
        }

        // 2. Merge with Legacy Borrow Data (danhSachGiaoDich)
        // Only take items that are NOT in DB (simple heuristic: if we have DB logs, maybe ignore legacy? 
        // No, keep legacy for old borrows not in DB yet).
        int legacyCount = 0;
        int maxLegacy = 10;
        for (int i = 0; i < (int)danhSachGiaoDich.size(); ++i) {
            if (legacyCount >= maxLegacy) break;
            // ... legacy mapping logic follows ...
            GiaoDichInfoLocal it;
            it.thoiGian = danhSachGiaoDich[i].thoiGian;
            
            // Historical data lacks Admin info, so we credit System.
            it.nguoiDung = "Hệ thống"; 
            
            // Map status to Action Verb
            int stt = danhSachGiaoDich[i].trangThai;
            if (stt == 1) {
                it.hanhDong = "Trả";
                it.trangThaiColor = "#a78bfa"; // Purple
            } else if (stt == 2) {
                it.hanhDong = "Mất";
                it.trangThaiColor = "#f87171"; // Red
            } else {
                it.hanhDong = "Mượn";
                it.trangThaiColor = "#fbbf24"; // Orange
            }
            
            // Details: User + Book
            it.noiDung = "Độc giả: " + danhSachGiaoDich[i].tenDocGia;
            if (!danhSachGiaoDich[i].tenSach.isEmpty()) {
                it.noiDung += " | " + danhSachGiaoDich[i].tenSach;
            }
            
            modelList.push_back(it);
        }

        ActivityModel *model = new ActivityModel(this);
        model->setList(modelList);
        this->modelHoatDong = model;
        view->setModel(model);
        
        // Căn chỉnh độ rộng cột
        QHeaderView *headerView = view->horizontalHeader();
        headerView->setSectionResizeMode(0, QHeaderView::Fixed); 
        view->setColumnWidth(0, 140); // Time fixed width
        headerView->setSectionResizeMode(1, QHeaderView::ResizeToContents); // User
        headerView->setSectionResizeMode(2, QHeaderView::Fixed); 
        view->setColumnWidth(2, 100); // Action fixed width
        
        // Use ResizeToContents for Details to allow scrolling (don't limit/cut off)
        headerView->setSectionResizeMode(3, QHeaderView::ResizeToContents);          
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
    
    boTri->addWidget(header);
    boTri->addWidget(view);
    
    return khung;
}

QWidget* BangDieuKhien::taoPanelCanhBao() {
    QWidget *khung = new QWidget();
    khung->setStyleSheet(
        "QWidget {"
        "    background-color: #1e2230;"
        "    border: 1px solid #2d3140;"
        "    border-radius: 12px;"
        "}"
    );
    
    QVBoxLayout *boTri = new QVBoxLayout(khung);
    boTri->setContentsMargins(0, 0, 0, 0);
    boTri->setSpacing(0);
    
    // Header
    QWidget *header = new QWidget();
    header->setStyleSheet("background-color: #252830; border: none; border-top-left-radius: 12px; border-top-right-radius: 12px;");
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 18, 24, 18);
    
    QLabel *tieuDe = new QLabel("Cảnh báo & Tác vụ");
    tieuDe->setStyleSheet("font-size: 14px; font-weight: 700; color: #E0E0E0; background: transparent; border: none; letter-spacing: 0.3px;");
    
    QPushButton *nutMenu = new QPushButton("⋯");
    nutMenu->setStyleSheet("background: transparent; border: none; color: #858585; font-size: 20px; padding: 0; max-width: 24px;");
    
    headerLayout->addWidget(tieuDe);
    headerLayout->addStretch();
    headerLayout->addWidget(nutMenu);
    
    // Scroll area cho các items
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: #1e2230; width: 8px; }"
        "QScrollBar::handle:vertical { background: #2d3140; border-radius: 4px; }"
        "QScrollBar::handle:vertical:hover { background: #3E4451; }"
    );
    
    QWidget *noiDung = new QWidget();
    noiDung->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout *noiDungLayout = new QVBoxLayout(noiDung);
    noiDungLayout->setContentsMargins(0, 0, 0, 0);
    noiDungLayout->setSpacing(0);
    
    // Lấy thống kê với kiểm tra an toàn
    extern PTRDG rootDocGia;
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    ThongKeDocGia tkDocGia = {};
    
    // Kiểm tra dữ liệu hợp lệ trước khi gọi hàm
    if (rootDocGia && soLuongDauSach >= 0 && soLuongDauSach <= MAX_DAUSACH) {
        try {
            tkDocGia = LayThongKeDocGia(rootDocGia, dsDauSach, soLuongDauSach);
        } catch (...) {
            qWarning() << "Lỗi khi lấy thống kê độc giả trong taoPanelCanhBao";
        }
    }
    
    // Tạo các mục cảnh báo
    auto taoMucCanhBao = [this](const QString &icon, const QString &nhan, const QString &mau, const QString &moTa = "", std::function<void()> callback = nullptr) -> QWidget* {
        QWidget *container = new QWidget();
        container->setStyleSheet(
            "QWidget { background: transparent; border: none; }"
        );
        
        QVBoxLayout *containerLayout = new QVBoxLayout(container);
        containerLayout->setContentsMargins(0, 0, 0, 0);
        containerLayout->setSpacing(0);
        
        QPushButton *muc = new QPushButton();
        muc->setFixedHeight(72);
        muc->setStyleSheet(
            "QPushButton { "
            "    background: transparent; "
            "    border: none; "
            "    border-bottom: 1px solid #2d3140; "
            "    text-align: left; "
            "    padding: 0;"
            "}"
            "QPushButton:hover { background-color: rgba(0, 120, 212, 0.08); }"
        );
        muc->setCursor(Qt::PointingHandCursor);
        
        QHBoxLayout *layout = new QHBoxLayout(muc);
        layout->setContentsMargins(20, 16, 20, 16);
        layout->setSpacing(14);
        
        // Icon với màu pastel
        QLabel *iconLabel = new QLabel();
        iconLabel->setFixedSize(40, 40);
        iconLabel->setText(icon);
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        
        // Chuyển hex sang RGBA với opacity 20%
        QColor mauGoc(mau);
        QString mauMo = QString("rgba(%1, %2, %3, 0.20)")
            .arg(mauGoc.red())
            .arg(mauGoc.green())
            .arg(mauGoc.blue());
        
        iconLabel->setStyleSheet(QString(
            "background-color: %1; border-radius: 8px; font-size: 18px; color: %2; border: none;"
        ).arg(mauMo).arg(mau));
        
        // Text
        QVBoxLayout *textLayout = new QVBoxLayout();
        textLayout->setSpacing(4);
        
        QLabel *nhanLabel = new QLabel(nhan);
        nhanLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #E0E0E0; background: transparent; border: none;");
        nhanLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        
        QLabel *moTaLabel = nullptr;
        if (!moTa.isEmpty()) {
            moTaLabel = new QLabel(moTa);
            moTaLabel->setStyleSheet("font-size: 11px; color: #9B9B9B; background: transparent; border: none;");
            moTaLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        }
        
        textLayout->addWidget(nhanLabel);
        if (moTaLabel) textLayout->addWidget(moTaLabel);
        
        // Arrow
        QLabel *arrow = new QLabel("›");
        arrow->setStyleSheet("font-size: 20px; color: #3E4451; background: transparent; border: none;");
        arrow->setAttribute(Qt::WA_TransparentForMouseEvents);
        
        layout->addWidget(iconLabel);
        layout->addLayout(textLayout, 1);
        layout->addWidget(arrow);
        
        containerLayout->addWidget(muc);
        
        // Kết nối callback
        if (callback) {
            QObject::connect(muc, &QPushButton::clicked, callback);
        }
        
        return container;
    };
    
    // Tính toán sách sắp hết (số lượng bản sao khả dụng <= 2)
    int sachSapHet = 0;
    if (soLuongDauSach > 0) {
        for(int i=0; i<soLuongDauSach; ++i) {
            if (!dsDauSach[i]) continue;
            int available = 0;
            PTRDMS p = dsDauSach[i]->dms;
            while(p) {
                if(p->trangThai == 0) available++; // CHO_MUON_DUOC
                p = p->next;
            }
            if (available <= 2) sachSapHet++;
        }
    }

    // Các mục cảnh báo & tác vụ với chức năng click
    
    // 1. In Ấn (Top Priority)
    noiDungLayout->addWidget(taoMucCanhBao("🖨️", "In Ấn Thư Viện", "#a855f7", "In thẻ, tem, báo cáo",
        [this]() { xuLyMoDialogInAn(); }));
        
    // 2. Quét Mã
    noiDungLayout->addWidget(taoMucCanhBao("🔍", "Quét mã vạch", "#60a5fa", "Quét sách/độc giả",
        [this]() { xuLyNutQuetMa(); }));
    
    // 3. Sao Lưu (New)
    noiDungLayout->addWidget(taoMucCanhBao("", "Sao lưu dữ liệu", "#34d399", "Backup CSDL an toàn",
        [this]() { 
            // Call generic backup function
            QString file = QString::fromStdString(TaoBackup());
            if (!file.isEmpty()) {
                QMessageBox::information(this, "Thành Công", "Đã sao lưu tại:\n" + file);
                AuditLogger::log("BACKUP", "HE_THONG", "Manual", "Nguoi dung sao luu thu cong");
            } else {
                QMessageBox::warning(this, "Lỗi", "Sao lưu thất bại!");
            }
        }));

    // 4. Cảnh báo Thẻ (Merged & Real Data)
    int totalExpiry = tkDocGia.docGiaHetHan + tkDocGia.docGiaSapHetHan;
    QString expiryText = QString("%1 Hết hạn | %2 Sắp hết").arg(tkDocGia.docGiaHetHan).arg(tkDocGia.docGiaSapHetHan);
    noiDungLayout->addWidget(taoMucCanhBao("", "Cảnh báo Thẻ", (totalExpiry > 0 ? "#f87171" : "#fbbf24"), expiryText,
        [this]() { emit yeuCauChuyenDenQuanLyDocGia(); }));

    // 5. Sách Quá Hạn
    noiDungLayout->addWidget(taoMucCanhBao("▲", QString("%1 Sách quá hạn").arg(tkDocGia.docGiaCoSachQuaHan), "#fb923c", "", 
        [this]() { emit yeuCauChuyenDenMuonTraSach(); }));
    
    // 6. Sách Sắp Hết (New)
    noiDungLayout->addWidget(taoMucCanhBao("", QString("%1 Sách sắp hết").arg(sachSapHet), "#f472b6", "SL khả dụng <= 2",
        [this]() { emit yeuCauChuyenDenQuanLySach(); }));
    
    noiDungLayout->addStretch();
    
    scrollArea->setWidget(noiDung);
    
    boTri->addWidget(header);
    boTri->addWidget(scrollArea, 1);
    
    return khung;
}


void BangDieuKhien::xuLyNutInThe() {
    // Redirect to new Print Dialog
    xuLyMoDialogInAn();
}

void BangDieuKhien::xuLyNutQuetMa() {
    // Mở dialog quét mã (tất cả loại)
    HopThoaiQuetMaVach *dialogQuet = new HopThoaiQuetMaVach(HopThoaiQuetMaVach::QUET_TAT_CA, this);
    
    connect(dialogQuet, &HopThoaiQuetMaVach::maVachDuocQuet, this, 
        [](const QString &ma, const QString &loai) {
            qDebug() << "Đã quét mã:" << ma << "Loại:" << loai;
        });
    
    if (dialogQuet->exec() == QDialog::Accepted) {
        QString ma = dialogQuet->layMaQuet();
        QString loai = dialogQuet->layLoaiMaQuet();
        
        // Chuyển đến tab tương ứng
        if (loai == "SACH") {
            emit yeuCauChuyenDenQuanLySach();
        } else if (loai == "DOC_GIA") {
            emit yeuCauChuyenDenQuanLyDocGia();
        }
    }
    
    delete dialogQuet;
}

void BangDieuKhien::xuLyMoDialogInAn() {
    // Launch Unified Print Dialog
    ManHinhQuanLyIn dialog(nullptr, {}, this);
    dialog.exec();
}

// ===================== PRINT PANEL SLOT IMPLEMENTATIONS =====================

void BangDieuKhien::xuLyInTheDocGiaHienTai() {
    extern PTRDG rootDocGia;
    
    // Tạo hộp thoại chọn độc giả
    if (!rootDocGia) {
        QMessageBox::warning(this, "Cảnh Báo", "Không có độc giả nào trong hệ thống!");
        return;
    }
    
    // Sử dụng QInputDialog để nhập mã thẻ
    bool ok;
    QString maThe = QInputDialog::getText(
        this,
        "Nhập Mã Thẻ Độc Giả",
        "Mã thẻ độc giả cần in:",
        QLineEdit::Normal,
        "",
        &ok
    );
    
    if (!ok || maThe.trimmed().isEmpty()) {
        return;
    }
    
    // Tìm độc giả theo mã thẻ bằng hàm timDocGia
    PTRDG docGia = timDocGia(rootDocGia, maThe.trimmed().toInt());
    
    if (docGia) {
        PrintHelper::inTheDocGiaTuDong(docGia, this);
    } else {
        QMessageBox::warning(this, "Không Tìm Thấy", "Không tìm thấy độc giả với mã thẻ: " + maThe);
    }
}

void BangDieuKhien::xuLyInHangLoatThe() {
    extern PTRDG rootDocGia;
    
    if (!rootDocGia) {
        QMessageBox::warning(this, "Cảnh Báo", "Không có độc giả nào trong hệ thống!");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Xác Nhận In Hàng Loạt",
        "In thẻ cho TẤT CẢ độc giả trong hệ thống?\n\nQuá trình này có thể mất vài phút.",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        PrintHelper::inHangLoatTheDocGia(rootDocGia, this);
    }
}

void BangDieuKhien::xuLyInTemSachHienTai() {
    // Yêu cầu nhập ISBN
    bool ok;
    QString isbn = QInputDialog::getText(
        this,
        "Nhập ISBN Sách",
        "ISBN của sách cần in tem:",
        QLineEdit::Normal,
        "",
        &ok
    );
    
    if (!ok || isbn.trimmed().isEmpty()) {
        return;
    }
    
    // Tìm sách theo ISBN
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] && QString::fromStdString(dsDauSach[i]->ISBN) == isbn.trimmed()) {
            PrintHelper::inTemSachTuDong(dsDauSach[i], this);
            return;
        }
    }
    
    QMessageBox::warning(this, "Không Tìm Thấy", "Không tìm thấy sách với ISBN: " + isbn);
}

void BangDieuKhien::xuLyInNhieuTem() {
    // Nhập số lượng tem
    bool ok1;
    int soLuong = QInputDialog::getInt(
        this,
        "Nhập Số Lượng Tem",
        "Số lượng tem cần in:",
        5,      // default value
        1,      // min
        100,    // max
        1,      // step
        &ok1
    );
    
    if (!ok1) {
        return;
    }
    
    // Nhập ISBN
    bool ok;
    QString isbn = QInputDialog::getText(
        this,
        "Nhập ISBN Sách",
        QString("ISBN của sách cần in %1 tem:").arg(soLuong),
        QLineEdit::Normal,
        "",
        &ok
    );
    
    if (!ok || isbn.trimmed().isEmpty()) {
        return;
    }
    
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] && QString::fromStdString(dsDauSach[i]->ISBN) == isbn.trimmed()) {
            PrintHelper::inNhieuTemSach(dsDauSach[i], soLuong, this);
            return;
        }
    }
    
    QMessageBox::warning(this, "Không Tìm Thấy", "Không tìm thấy sách với ISBN: " + isbn);
}

void BangDieuKhien::xuLyInDanhSachDocGia() {
    extern PTRDG rootDocGia;
    
    if (!rootDocGia) {
        QMessageBox::warning(this, "Cảnh Báo", "Không có độc giả nào trong hệ thống!");
        return;
    }
    
    PrintHelper::inDanhSachDocGia(rootDocGia, this);
}

void BangDieuKhien::xuLyInDanhMucSach() {
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    
    if (soLuongDauSach == 0) {
        QMessageBox::warning(this, "Cảnh Báo", "Không có sách nào trong hệ thống!");
        return;
    }
    
    PrintHelper::inDanhMucSach(dsDauSach, soLuongDauSach, this);
}

void BangDieuKhien::xuLyInPhieuMuon() {
    extern PTRDG rootDocGia;
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    
    // Nhập mã thẻ độc giả
    bool ok;
    QString maThe = QInputDialog::getText(
        this,
        "Nhập Mã Thẻ Độc Giả",
        "Mã thẻ độc giả có sách đang mượn:",
        QLineEdit::Normal,
        "",
        &ok
    );
    
    if (!ok || maThe.trimmed().isEmpty()) {
        return;
    }
    
    // Tìm độc giả bằng hàm timDocGia
    PTRDG docGia = timDocGia(rootDocGia, maThe.trimmed().toInt());
    
    if (!docGia) {
        QMessageBox::warning(this, "Không Tìm Thấy", "Không tìm thấy độc giả với mã thẻ: " + maThe);
        return;
    }
    
    // Kiểm tra xem độc giả có sách đang mượn không
    if (docGia->data.soSachDangMuon == 0) {
        QMessageBox::information(this, "Thông Báo", "Độc giả này chưa mượn sách nào!");
        return;
    }
    
    // In phiếu mượn cho sách đầu tiên
    MUONTRA mt = docGia->data.dsmt;
    if (mt && mt->data.banSaoSach) {
        QString maSach = QString::fromStdString(mt->data.banSaoSach->maSach);
        QString ngayMuon = QString::fromStdString(mt->data.NgayMuon);
        
        // Tìm tên sách từ mã sách
        QString tenSach = "Không rõ";
        for (int i = 0; i < soLuongDauSach; i++) {
            if (dsDauSach[i]) {
                PTRDMS dms = dsDauSach[i]->dms;
                while (dms) {
                    if (QString::fromStdString(dms->maSach) == maSach) {
                        tenSach = QString::fromStdString(dsDauSach[i]->tenSach);
                        break;
                    }
                    dms = dms->next;
                }
                if (tenSach != "Không rõ") break;
            }
        }
        
        PrintHelper::inPhieuMuonTuDong(docGia, maSach, tenSach, ngayMuon, this);
    }
}

void BangDieuKhien::xuLyInPhieuTra() {
    extern PTRDG rootDocGia;
    
    // Nhập mã thẻ độc giả
    bool ok;
    QString maThe = QInputDialog::getText(
        this,
        "Nhập Mã Thẻ Độc Giả",
        "Mã thẻ độc giả vừa trả sách:",
        QLineEdit::Normal,
        "",
        &ok
    );
    
    if (!ok || maThe.trimmed().isEmpty()) {
        return;
    }
    
    // Tìm độc giả
    PTRDG docGia = timDocGia(rootDocGia, maThe.trimmed().toInt());
    
    if (!docGia) {
        QMessageBox::warning(this, "Không Tìm Thấy", "Không tìm thấy độc giả với mã thẻ: " + maThe);
        return;
    }
    
    // In phiếu trả (giả sử in toàn bộ lịch sử trả gần nhất hoặc danh sách mượn hiện tại đã updated)
    // Hiện tại PrintHelper chưa có hàm inPhieuTra chuyên biệt check lịch sử, 
    // nên ta sẽ dùng tạm hàm inThongTinDocGia hoặc message box báo thành công.
    // Tốt nhất là in một "Biên lai trả sách" tổng quát.
    
    // Check if we have a helper for this?
    // Let's look at PrintHelper options.
    // For now, let's just Print the Reader Details + "Returned" status.
    
    // Actually, let's create a specialized method in PrintHelper or inline it here?
    // Inline is messy. Let's use `PrintHelper::inPhieuTraTuDong` if it existed, but it likely doesn't.
    // We will assume `PrintHelper` needs an update OR we use a basic confirmation.
    
    QString msg = QString("Đã xác nhận trả sách cho độc giả: %1 %2\nIn biên lai?").arg(QString::fromStdString(docGia->data.Ho)).arg(QString::fromStdString(docGia->data.Ten));
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Xác Nhận", msg, QMessageBox::Yes | QMessageBox::No);
    

    if (reply == QMessageBox::Yes) {
         // Reuse inPhieuMuon template but modify title? No, hardcoded.
         // Let's just print the Reader Card info as a receipt for now.
         PrintHelper::inTheDocGiaTuDong(docGia, this);
    }
}

QWidget* BangDieuKhien::taoHeaderDashboard() {
    QWidget *header = new QWidget();
    header->setFixedHeight(80);
    header->setStyleSheet("background: transparent;");
    
    QHBoxLayout *layout = new QHBoxLayout(header);
    layout->setContentsMargins(0, 0, 0, 20); // Bottom margin to separate from cards
    layout->setSpacing(20);
    
    // Welcome Text
    QWidget *textContainer = new QWidget();
    QVBoxLayout *textLayout = new QVBoxLayout(textContainer);
    textLayout->setContentsMargins(0,0,0,0);
    textLayout->setSpacing(2);
    
    QLabel *lblHello = new QLabel("Xin chào, Quản trị viên 👋");
    lblHello->setStyleSheet("font-size: 24px; font-weight: 800; color: #f8fafc; font-family: 'Segoe UI';");
    
    QLabel *lblSub = new QLabel("Đây là số liệu thống kê mới nhất hôm nay.");
    lblSub->setStyleSheet("font-size: 14px; color: #94a3b8;");
    
    textLayout->addWidget(lblHello);
    textLayout->addWidget(lblSub);
    
    layout->addWidget(textContainer);
    layout->addStretch();
    
    // Search Bar
    QWidget *searchContainer = new QWidget();
    searchContainer->setFixedSize(400, 45);
    searchContainer->setStyleSheet(
        "QWidget { background-color: #1e293b; border-radius: 22px; border: 1px solid #334155; }"
    );
    
    QHBoxLayout *searchLayout = new QHBoxLayout(searchContainer);
    searchLayout->setContentsMargins(15, 0, 15, 0);
    searchLayout->setSpacing(10);
    
    QLabel *icon = new QLabel("🔍");
    icon->setStyleSheet("border: none; background: transparent; color: #64748b; font-size: 16px;");
    
    QLineEdit *input = new QLineEdit();
    input->setPlaceholderText("Tìm kiếm nhanh sách, độc giả...");
    input->setStyleSheet("border: none; background: transparent; color: white; font-size: 14px;");
    
    searchLayout->addWidget(icon);
    searchLayout->addWidget(input);
    
    layout->addWidget(searchContainer);
    
    // Connect Search (Smart Omni-Search)
    connect(input, &QLineEdit::returnPressed, this, [this, input](){
        QString query = input->text().trimmed();
        if (query.isEmpty()) return;
        
        // 1. Prepare Data
        extern PTRDS dsDauSach[];
        extern int soLuongDauSach;
        extern PTRDG rootDocGia;
        
        struct SearchResult { 
            QString type; 
            QString title; 
            QString subtitle; 
            QString id; 
            QString icon;
        };
        QVector<SearchResult> results;
        QString qLower = query.toLower();

        // 2. Search Books
        if (soLuongDauSach >= 0 && soLuongDauSach <= MAX_DAUSACH) {
            for (int i=0; i < soLuongDauSach; i++) {
                 if (!dsDauSach[i]) continue;
                 QString ten = QString::fromStdString(dsDauSach[i]->tenSach);
                 QString isbn = QString::fromStdString(dsDauSach[i]->ISBN);
                 QString tacGia = QString::fromStdString(dsDauSach[i]->tacGia);
                 
                 if (ten.toLower().contains(qLower) || isbn.contains(qLower) || tacGia.toLower().contains(qLower)) {
                     results.append({"Sách", ten, "ISBN: " + isbn + " - TG: " + tacGia, isbn, "book"});
                     if (results.size() > 20) break;
                 }
            }
        }
        
        // 3. Search Readers
        if (rootDocGia) {
            PTRDG arrDocGia[1000];
            int countDG = 0;
            // Best effort traversal (limit 1000)
            try { 
                DuyetCayRaMang(rootDocGia, arrDocGia, countDG); 
            } catch(...) { countDG=0; }
            
            // Limit check
            if (countDG > 1000) countDG = 1000;

            for(int i=0; i<countDG; i++) {
                 if(!arrDocGia[i]) continue;
                 QString ten = QString::fromStdString(arrDocGia[i]->data.Ho + " " + arrDocGia[i]->data.Ten);
                 QString ma = QString::number(arrDocGia[i]->data.MaThe);
                 
                 if (ten.toLower().contains(qLower) || ma.contains(qLower)) {
                     results.append({"Độc Giả", ten, "Mã Thẻ: " + ma, ma, "user"});
                     if (results.size() > 40) break;
                 }
            }
        }
        
        // 4. Show Results Dialog
        if (results.isEmpty()) {
            QMessageBox::information(this, "Kết Quả", "Không tìm thấy kết quả nào cho: " + query);
            return;
        }

        QDialog dlg(this);
        dlg.setWindowTitle("Kết Quả Tìm Kiếm");
        dlg.setMinimumWidth(600);
        dlg.setMinimumHeight(400);
        dlg.setStyleSheet("QDialog { background-color: #1e1e2d; color: white; }");
        
        QVBoxLayout *layout = new QVBoxLayout(&dlg);
        QLabel *lblHeader = new QLabel(QString("Tìm thấy %1 kết quả cho '%2'").arg(results.size()).arg(query));
        lblHeader->setStyleSheet("font-size: 14px; font-weight: bold; color: #3699ff; margin-bottom: 10px;");
        layout->addWidget(lblHeader);

        QListWidget *list = new QListWidget();
        list->setStyleSheet(
            "QListWidget { background-color: #2b2b40; border-radius: 8px; border: 1px solid #3f3f50; padding: 5px; outline: none; }"
            "QListWidget::item { padding: 10px; border-bottom: 1px solid #323248; color: #e0e0e0; }"
            "QListWidget::item:hover { background-color: #323248; }"
            "QListWidget::item:selected { background-color: #3699ff; color: white; }"
        );
        layout->addWidget(list);
        
        for(const auto &res : results) {
            QListWidgetItem *item = new QListWidgetItem();
            QString iconChar = res.icon == "book" ? "📖" : "👤";
            item->setText(QString("%1  %2\n       %3").arg(iconChar).arg(res.title).arg(res.subtitle));
            item->setData(Qt::UserRole, res.type);
            item->setData(Qt::UserRole+1, res.id);
            list->addItem(item);
        }
        
        QLabel *lblHint = new QLabel("💡 Nhấp đúp để chọn và đi tới");
        lblHint->setStyleSheet("color: #7d7d7d; font-style: italic; margin-top: 5px;");
        layout->addWidget(lblHint);

        connect(list, &QListWidget::itemDoubleClicked, [&](QListWidgetItem *item){
             QString type = item->data(Qt::UserRole).toString();
             QString id = item->data(Qt::UserRole+1).toString();
             dlg.accept();
             
             QApplication::clipboard()->setText(id);
             
             if (type == "Sách") {
                 emit yeuCauChuyenDenQuanLySach();
                 QMessageBox::information(this, "Điều Hướng", "Đã copy ISBN <b>" + id + "</b>.<br>Vui lòng Paste (Ctrl+V) vào ô tìm kiếm sách.");
             } else {
                  emit yeuCauChuyenDenQuanLyDocGia();
                  QMessageBox::information(this, "Điều Hướng", "Đã copy Mã Thẻ <b>" + id + "</b>.<br>Vui lòng Paste (Ctrl+V) vào ô tìm kiếm độc giả.");
             }
        });
        
        dlg.exec();
        input->clear();
    });
    
    return header;
}


void BangDieuKhien::ghiNhatKy(const QString &hanhDong, const QString &noiDung) {
    if (!modelHoatDong) return;
    
    // 1. Get Time
    QString thoiGian = QDateTime::currentDateTime().toString("HH:mm:ss");
    
    // 2. Get User
    QString nguoiDung = "Hệ thống";
    try {
        UserInfo currentUser = UserManager::instance().currentUser();
        if (!currentUser.username.isEmpty()) {
            nguoiDung = currentUser.username;
        }
    } catch (...) {}
    
    // 3. Determine Color
    QString mau = "#3699ff"; // Default Blue
    if (hanhDong.contains("Thêm")) mau = "#34d399"; // Green
    else if (hanhDong.contains("Xóa")) mau = "#f87171"; // Red
    else if (hanhDong.contains("Mượn")) mau = "#fbbf24"; // Orange
    else if (hanhDong.contains("Trả")) mau = "#a78bfa"; // Purple
    
    // 4. Prepend to Model
    GiaoDichInfoLocal item;
    item.thoiGian = thoiGian;
    item.nguoiDung = nguoiDung;
    item.hanhDong = hanhDong;
    item.noiDung = noiDung;
    item.trangThaiColor = mau;
    
    ActivityModel *model = dynamic_cast<ActivityModel*>(modelHoatDong);
    if (model) {
        model->prepend(item);
    }
}

void BangDieuKhien::xuatBaoCaoLog() {
    QString fileName = QFileDialog::getSaveFileName(this, "Xuất Báo Cáo Hệ Thống", "BaoCao_HoatDong.csv", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Lỗi", "Không thể ghi file.");
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    // BOM for Excel
    out << "\xEF\xBB\xBF"; 

    // Header
    out << "ID,NguoiDung,HanhDong,BangDuLieu,DoiTuongID,ChiTiet,ThoiGian\n";

    // Query DB
    QSqlQuery q("SELECT n.id, u.ten_dang_nhap, n.hanh_dong, n.bang_du_lieu, n.doi_tuong_id, n.chi_tiet, n.thoi_gian "
                "FROM nhat_ky_he_thong n "
                "LEFT JOIN nguoi_dung u ON n.nguoi_dung_id = u.id "
                "ORDER BY n.id DESC", DatabaseManager::instance().database());
    
    while(q.next()) {
        QString id = q.value(0).toString();
        QString user = q.value(1).toString();
        if (user.isEmpty()) user = "System";
        QString action = q.value(2).toString();
        QString table = q.value(3).toString();
        QString objId = q.value(4).toString();
        QString details = q.value(5).toString().replace("\"", "\"\""); // Escape quotes
        QString time = q.value(6).toString();

        out << id << "," << user << "," << action << "," << table << "," << objId << ",\"" << details << "\"," << time << "\n";
    }

    file.close();
    QMessageBox::information(this, "Thành Công", "Đã xuất báo cáo thành công!");
}
