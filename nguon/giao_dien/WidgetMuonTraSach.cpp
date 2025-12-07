#include "giao_dien/WidgetMuonTraSach.h"
#include "giao_dien/hop_thoai/HopThoaiQuetMaVach.h"
#include "utils/FilePathManager.h"
#include "utils/TransactionLogger.h"
#include "utils/PrintHelper.h"
#include "models/DocGia.h"
#include "models/QuanLySach.h"
#include "models/ThaoTacFile.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>
#include <QTabWidget>
#include <QDebug>
#include <QDateTime>
#include <QInputDialog>
#include <QTableView>
#include <QAbstractTableModel>

// Include responsive helpers
#include "utils/ResponsiveHelper.h"
#include "utils/AnimationHelper.h"
#include "utils/EventBus.h"
#include "utils/SafeGui.h"

// Prefer DBBatcher snapshot API when available to avoid buffer overruns
#if defined(__has_include)
#  if __has_include("utils/DBBatcher.h")
#    include "utils/DBBatcher.h"
#  endif
#endif

WidgetMuonTraSach::WidgetMuonTraSach(QWidget *cha) 
    : QWidget(cha), docGiaHienTai(nullptr), sachDangChonMuon(nullptr), banSaoDangChon(nullptr) {
    thieLapGiaoDien();

    // Realtime updates: refresh lists/details when data changes
    connect(&EventBus::instance(), &EventBus::docGiaAdded, this, [this](int){ this->hienThiDanhSachDocGia(); });
    connect(&EventBus::instance(), &EventBus::docGiaUpdated, this, [this](int){ this->hienThiDanhSachDocGia(); });
    connect(&EventBus::instance(), &EventBus::docGiaRemoved, this, [this](int){ this->hienThiDanhSachDocGia(); });

    connect(&EventBus::instance(), &EventBus::muonTraAdded, this, [this](int maThe, const QString &){
        // If current reader affected, update their borrowed list
        if (this->docGiaHienTai && this->docGiaHienTai->data.MaThe == maThe) this->capNhatBangSachDangMuon();
        this->hienThiDanhSachDocGia();
    });
    connect(&EventBus::instance(), &EventBus::muonTraUpdated, this, [this](int maThe, const QString &){
        if (this->docGiaHienTai && this->docGiaHienTai->data.MaThe == maThe) this->capNhatBangSachDangMuon();
        this->hienThiDanhSachDocGia();
    });
    connect(&EventBus::instance(), &EventBus::muonTraRemoved, this, [this](int maThe, const QString &){
        if (this->docGiaHienTai && this->docGiaHienTai->data.MaThe == maThe) this->capNhatBangSachDangMuon();
        this->hienThiDanhSachDocGia();
    });

    // Refresh when global data changes (e.g. background loader finished)
    connect(&EventBus::instance(), &EventBus::dataChanged, this, [this]() {
        this->hienThiDanhSachDocGia();
        this->capNhatBangSachDangMuon();
    });
}

// Lightweight model to back the readers view (virtualized, paged)
class DocGiaTableModel : public QAbstractTableModel {
public:
    DocGiaTableModel(std::vector<PTRDG> *data, QObject *parent = nullptr)
        : QAbstractTableModel(parent), m_data(data), m_page(1), m_pageSize(10) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        if (!m_data) return 0;
        int total = static_cast<int>(m_data->size());
        int start = (m_page - 1) * m_pageSize;
        if (start >= total) return 0;
        return std::min(m_pageSize, total - start);
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return 6; // STT, MaThe, HoTen, Phai, TrangThai, Sach
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!m_data || !index.isValid() || role != Qt::DisplayRole) return QVariant();
        int start = (m_page - 1) * m_pageSize;
        int absIdx = start + index.row();
        if (absIdx < 0 || absIdx >= (int)m_data->size()) return QVariant();
        PTRDG docGia = (*m_data)[absIdx];
        if (!docGia) return QVariant();

        switch (index.column()) {
        case 0: return QString::number(absIdx + 1); // STT (absolute)
        case 1: return QString::number(docGia->data.MaThe);
        case 2: return QString::fromStdString(std::string(docGia->data.Ho + std::string(" ")) + docGia->data.Ten);
        case 3: return docGia->data.Phai ? QStringLiteral("Nữ") : QStringLiteral("Nam");
        case 4: return docGia->data.TrangThai == 1 ? QStringLiteral("Hoạt động") : QStringLiteral("Khoá");
        case 5: return QString("%1/3").arg(docGia->data.soSachDangMuon);
        default: return QVariant();
        }
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case 0: return QStringLiteral("STT");
            case 1: return QStringLiteral("Mã Thẻ");
            case 2: return QStringLiteral("Họ Tên");
            case 3: return QStringLiteral("Phái");
            case 4: return QStringLiteral("Trạng Thái");
            case 5: return QStringLiteral("Sách");
            }
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    void refresh() { beginResetModel(); endResetModel(); }
    void setPage(int p) { if (p < 1) p = 1; if (p != m_page) { m_page = p; refresh(); } }
    void setPageSize(int s) { if (s < 1) s = 1; if (s != m_pageSize) { m_pageSize = s; refresh(); } }
    int page() const { return m_page; }
    int pageSize() const { return m_pageSize; }
    int totalRows() const { return (int)(m_data ? m_data->size() : 0); }

private:
    std::vector<PTRDG> *m_data;
    int m_page;
    int m_pageSize;
};

// Model for sách đang mượn (small DTO list)
class SachDangMuonModel : public QAbstractTableModel {
public:
    SachDangMuonModel(std::vector<ThongTinSachDangMuon_DTO> *data, QObject *parent = nullptr)
        : QAbstractTableModel(parent), m_data(data) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return m_data ? (int)m_data->size() : 0;
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return 3; // maSach, tenSach, ngayMuon
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!m_data || !index.isValid()) return QVariant();
        int r = index.row();
        if (r < 0 || r >= (int)m_data->size()) return QVariant();
        const ThongTinSachDangMuon_DTO &dto = (*m_data)[r];

        if (role == Qt::DisplayRole) {
            switch (index.column()) {
            case 0: return QString::fromStdString(dto.maSach);
            case 1: return QString::fromStdString(dto.tenSach);
            case 2: return QString::fromStdString(dto.ngayMuon);
            }
        }
        if (role == Qt::FontRole) {
            QFont f;
            if (index.column() == 0 || index.column() == 1) { f.setBold(true); return f; }
        }
        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case 0: return QStringLiteral("Mã Sách");
            case 1: return QStringLiteral("Tên Sách");
            case 2: return QStringLiteral("Ngày Mượn");
            }
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    void refresh() { beginResetModel(); endResetModel(); }

private:
    std::vector<ThongTinSachDangMuon_DTO> *m_data;
};

WidgetMuonTraSach::~WidgetMuonTraSach() {}

void WidgetMuonTraSach::thieLapGiaoDien() {
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    
    boTriChinh = new QVBoxLayout(this);
    int margin = ResponsiveHelper::getScaledMargin(30, screenSize);
    int spacing = ResponsiveHelper::getScaledSpacing(20, screenSize);
    boTriChinh->setContentsMargins(margin, margin, margin, margin);
    boTriChinh->setSpacing(spacing);
    
    // Tiêu đề với icon SVG
    QHBoxLayout *layoutTieuDe = new QHBoxLayout();
    QPushButton *iconTieuDe = new QPushButton();
    iconTieuDe->setIcon(QIcon(":/icons/exchange.svg"));
    iconTieuDe->setIconSize(QSize(28, 28));
    iconTieuDe->setFlat(true);
    iconTieuDe->setStyleSheet("border: none; background: transparent;");
    
    int titleSize = ResponsiveHelper::getScaledFontSize(28, screenSize);
    QLabel *tieuDe = new QLabel("MƯỢN TRẢ SÁCH");
    tieuDe->setStyleSheet(QString("font-size: %1px; font-weight: bold; color: #FFFFFF;").arg(titleSize));
    
    layoutTieuDe->addWidget(iconTieuDe);
    layoutTieuDe->addWidget(tieuDe);
    layoutTieuDe->addStretch();
    boTriChinh->addLayout(layoutTieuDe);
    
    // Tab widget với SVG icons
    int tabPadding = ResponsiveHelper::getScaledSpacing(12, screenSize);
    int tabFontSize = ResponsiveHelper::getScaledFontSize(14, screenSize);
    hopTab = new QTabWidget();
    hopTab->setStyleSheet(QString(
        "QTabWidget::pane { background-color: #1E1E2D; border: 1px solid #2B2B40; border-radius: 12px; }"
        "QTabBar::tab { background-color: #252534; color: #B5B5C3; padding: %1px 30px; "
        "border-top-left-radius: 8px; border-top-right-radius: 8px; font-size: %2px; }"
        "QTabBar::tab:selected { background-color: #3699FF; color: white; font-weight: bold; }"
    ).arg(tabPadding).arg(tabFontSize));
    
    thieLapTabMuonSach();
    thieLapTabTraSach();
    thieLapTabLichSu();
    
    boTriChinh->addWidget(hopTab);
    
    qDebug() << "WidgetMuonTraSach: 2-column layout with pagination applied";
}

void WidgetMuonTraSach::thieLapTabMuonSach() {
    widgetMuonSach = new QWidget();
    QHBoxLayout *boTriChinh = new QHBoxLayout(widgetMuonSach);
    boTriChinh->setContentsMargins(20, 20, 20, 20);
    boTriChinh->setSpacing(20);
    
    // === CỘT TRÁI: DANH SÁCH ĐỘC GIẢ (50%) ===
    QWidget *cotTrai = new QWidget();
    QVBoxLayout *boTriTrai = new QVBoxLayout(cotTrai);
    boTriTrai->setContentsMargins(0, 0, 0, 0);
    boTriTrai->setSpacing(15);
    
    // Tìm kiếm độc giả
    QLabel *tieuDeTim = new QLabel("Tìm Kiếm:");
    tieuDeTim->setStyleSheet("color: #B5B5C3; font-size: 13px; font-weight: bold;");
    
    QHBoxLayout *boTriTim = new QHBoxLayout();
    oNhapTimDocGia = new QLineEdit();
    oNhapTimDocGia->setPlaceholderText("Nhập tên, mã thẻ để tìm kiếm...");
    oNhapTimDocGia->setMinimumHeight(40);
    oNhapTimDocGia->setStyleSheet(
        "QLineEdit { background-color: #252534; color: #FFFFFF; border: 1px solid #2B2B40; "
        "border-radius: 8px; padding: 8px 12px; font-size: 13px; }"
        "QLineEdit:focus { border: 1px solid #3699FF; }"
    );
    
    nutTimDocGia = new QPushButton();
    nutTimDocGia->setIcon(QIcon(":/icons/search.svg"));
    nutTimDocGia->setIconSize(QSize(18, 18));
    nutTimDocGia->setMinimumHeight(40);
    nutTimDocGia->setMinimumWidth(80);
    nutTimDocGia->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3699FF, stop:1 #1BC5BD); "
        "color: white; border: none; border-radius: 8px; font-weight: bold; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #187DE4, stop:1 #0F9A94); }"
    );
    
    QPushButton *nutLamMoi = new QPushButton();
    nutLamMoi->setIcon(QIcon(":/icons/refresh.svg"));
    nutLamMoi->setIconSize(QSize(18, 18));
    nutLamMoi->setMinimumHeight(40);
    nutLamMoi->setMinimumWidth(40);
    nutLamMoi->setStyleSheet(
        "QPushButton { background-color: #252534; color: #B5B5C3; border: 1px solid #2B2B40; "
        "border-radius: 8px; }"
        "QPushButton:hover { background-color: #2B2B40; border: 1px solid #3699FF; }"
    );
    
    // Nút quét mã độc giả
    nutQuetMaDocGia = new QPushButton("🔍");
    nutQuetMaDocGia->setMinimumHeight(40);
    nutQuetMaDocGia->setMinimumWidth(40);
    nutQuetMaDocGia->setToolTip("Quét mã thẻ độc giả");
    nutQuetMaDocGia->setStyleSheet(
        "QPushButton { background-color: #1BC5BD; color: white; border: none; "
        "border-radius: 8px; font-size: 18px; }"
        "QPushButton:hover { background-color: #0BB7AF; }"
    );
    
    boTriTim->addWidget(oNhapTimDocGia, 1);
    boTriTim->addWidget(nutTimDocGia);
    boTriTim->addWidget(nutQuetMaDocGia);
    boTriTim->addWidget(nutLamMoi);
    
    boTriTrai->addWidget(tieuDeTim);
    boTriTrai->addLayout(boTriTim);
    
    // Bảng độc giả (view + model)
    bangDocGia = new QTableView();
    modelDocGia = new DocGiaTableModel(&danhSachDocGia, this);
    bangDocGia->setModel(modelDocGia);
    thieLapBangDocGia();
    boTriTrai->addWidget(bangDocGia, 1);
    
    // Thông tin tổng
    nhanTongDocGia = new QLabel("Tổng: 0 độc giả");
    nhanTongDocGia->setStyleSheet("color: #B5B5C3; font-size: 13px; padding: 8px 0;");
    
    boTriTrai->addWidget(nhanTongDocGia);
    
    // Nút hủy chọn
    nutHuyChonDocGia = new QPushButton("HỦY CHỎN ĐỘC GIẢ");
    nutHuyChonDocGia->setMinimumHeight(40);
    nutHuyChonDocGia->setEnabled(false);
    nutHuyChonDocGia->setStyleSheet(
        "QPushButton { background-color: #F64E60; color: white; border: none; "
        "border-radius: 8px; font-weight: bold; font-size: 13px; }"
        "QPushButton:hover { background-color: #D12E40; }"
        "QPushButton:disabled { background-color: #2B2B40; color: #6C6C80; }"
    );
    boTriTrai->addWidget(nutHuyChonDocGia);
    
    // === CỘT PHẢI: THÔNG TIN CHI TIẾT (50%) ===
    QWidget *cotPhai = new QWidget();
    QVBoxLayout *boTriPhai = new QVBoxLayout(cotPhai);
    boTriPhai->setContentsMargins(0, 0, 0, 0);
    boTriPhai->setSpacing(12);
    
    // Panel thông tin độc giả
    QGroupBox *nhomDocGia = new QGroupBox("ĐỘC GIẢ");
    nhomDocGia->setStyleSheet(
        "QGroupBox { background-color: #252534; border: 1px solid #2B2B40; "
        "border-radius: 8px; padding: 15px; font-weight: bold; color: #FFA800; font-size: 13px; }"
    );
    QVBoxLayout *boTriDocGia = new QVBoxLayout(nhomDocGia);
    
    nhanThongTinDocGia = new QLabel("* Click vào danh sách độc giả để chọn");
    nhanThongTinDocGia->setStyleSheet(
        "color: #B5B5C3; font-size: 13px; padding: 15px; "
        "background-color: #1E1E2D; border-radius: 6px;"
    );
    nhanThongTinDocGia->setWordWrap(true);
    nhanThongTinDocGia->setMinimumHeight(140);
    boTriDocGia->addWidget(nhanThongTinDocGia);
    boTriPhai->addWidget(nhomDocGia);
    
    // Panel sÃ¡ch Ä‘ang mÆ°á»£n
    QGroupBox *nhomSachDangMuon = new QGroupBox("SÃCH ÄANG MÆ¯á»¢N (0/3)");
    nhomSachDangMuon->setStyleSheet(
        "QGroupBox { background-color: #252534; border: 1px solid #2B2B40; "
        "border-radius: 8px; padding: 15px; font-weight: bold; color: #3699FF; font-size: 13px; }"
    );
    nhanTieuDeSachDangMuon = nhomSachDangMuon;
    QVBoxLayout *boTriSachDangMuon = new QVBoxLayout(nhomSachDangMuon);
    
    // use model/view for sách đang mượn
    bangSachDangMuon = new QTableView();
    modelSachDangMuon = new SachDangMuonModel(&danhSachSachDangMuon, this);
    bangSachDangMuon->setModel(modelSachDangMuon);
    bangSachDangMuon->horizontalHeader()->setStretchLastSection(true);
    bangSachDangMuon->setColumnWidth(0, 120);
    bangSachDangMuon->setColumnWidth(1, 200);
    bangSachDangMuon->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangSachDangMuon->setSelectionMode(QAbstractItemView::SingleSelection);
    bangSachDangMuon->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bangSachDangMuon->setMinimumHeight(280);
    bangSachDangMuon->setStyleSheet(
        "QTableView { background-color: #1E1E2D; color: #FFFFFF; gridline-color: #2B2B40; "
        "border: 1px solid #2B2B40; border-radius: 6px; font-size: 12px; }"
        "QHeaderView::section { background-color: #2B2B40; color: #B5B5C3; "
        "border: none; padding: 8px; font-weight: bold; }"
        "QTableView::item:selected { background-color: #3699FF; color: white; }"
    );
    boTriSachDangMuon->addWidget(bangSachDangMuon);
    
    // Nút In Lại Phiếu Mượn
    nutInLaiPhieu = new QPushButton("🖨️ In Lại Phiếu Mượn");
    nutInLaiPhieu->setMinimumHeight(35);
    nutInLaiPhieu->setEnabled(false);
    nutInLaiPhieu->setStyleSheet(
        "QPushButton { background-color: #8950FC; color: white; border: none; "
        "border-radius: 6px; font-weight: bold; font-size: 13px; }"
        "QPushButton:hover { background-color: #7337EE; }"
        "QPushButton:disabled { background-color: #2B2B40; color: #6C6C80; }"
    );
    boTriSachDangMuon->addWidget(nutInLaiPhieu);
    
    boTriPhai->addWidget(nhomSachDangMuon);
    
    // Form nhập ISBN/Mã sách
    QGroupBox *nhomNhapSach = new QGroupBox("NHẬP ISBN SÁCH CẦN MƯỢN");
    nhomNhapSach->setStyleSheet(
        "QGroupBox { background-color: #252534; border: 1px solid #2B2B40; "
        "border-radius: 8px; padding: 15px; font-weight: bold; color: #1BC5BD; font-size: 13px; }"
    );
    QHBoxLayout *boTriNhapSach = new QHBoxLayout(nhomNhapSach);
    
    oNhapISBN = new QLineEdit();
    oNhapISBN->setPlaceholderText("Mã Sách (*)");
    oNhapISBN->setMinimumHeight(40);
    oNhapISBN->setEnabled(false);
    oNhapISBN->setStyleSheet(
        "QLineEdit { background-color: #1E1E2D; color: #FFFFFF; border: 1px solid #2B2B40; "
        "border-radius: 8px; padding: 8px 12px; font-size: 13px; }"
        "QLineEdit:focus { border: 1px solid #1BC5BD; }"
        "QLineEdit:disabled { background-color: #1E1E2D; color: #6C6C80; }"
    );
    
    // Tự động submit khi nhấn Enter (cho máy quét barcode/QR)
    connect(oNhapISBN, &QLineEdit::returnPressed, this, [this]() {
        if (docGiaHienTai && !oNhapISBN->text().trimmed().isEmpty()) {
            hopXacNhanMuon->setChecked(true);
            xuLyNutXacNhanMuon();
        }
    });
    
    // Nút quét mã sách
    nutQuetMaSach = new QPushButton("🔍 Quét");
    nutQuetMaSach->setMinimumHeight(40);
    nutQuetMaSach->setEnabled(false);
    nutQuetMaSach->setStyleSheet(
        "QPushButton { background-color: #1BC5BD; color: white; border: none; "
        "border-radius: 8px; padding: 0 15px; font-weight: bold; font-size: 13px; }"
        "QPushButton:hover { background-color: #0BB7AF; }"
        "QPushButton:disabled { background-color: #2B2B40; color: #6C6C80; }"
    );
    
    hopXacNhanMuon = new QCheckBox("Xác Nhận Mượn");
    hopXacNhanMuon->setEnabled(false);
    hopXacNhanMuon->setStyleSheet(
        "QCheckBox { color: #B5B5C3; font-size: 13px; font-weight: bold; spacing: 8px; }"
        "QCheckBox::indicator { width: 20px; height: 20px; }"
        "QCheckBox::indicator:unchecked { border: 2px solid #2B2B40; border-radius: 4px; background-color: #1E1E2D; }"
        "QCheckBox::indicator:checked { border: 2px solid #1BC5BD; border-radius: 4px; background-color: #1BC5BD; "
        "image: url(:/icons/check.svg); }"
        "QCheckBox:disabled { color: #6C6C80; }"
    );
    
    boTriNhapSach->addWidget(oNhapISBN, 1);
    boTriNhapSach->addWidget(nutQuetMaSach);
    boTriNhapSach->addWidget(hopXacNhanMuon);
    boTriPhai->addWidget(nhomNhapSach);
    
    // 3 nút hành động - giảm kích thước
    QHBoxLayout *boTriNut = new QHBoxLayout();
    
    nutXacNhanMuon = new QPushButton("MƯỢN SÁCH");
    nutXacNhanMuon->setMinimumHeight(35);
    nutXacNhanMuon->setEnabled(false);
    nutXacNhanMuon->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1BC5BD, stop:1 #0BB7AF); "
        "color: white; border: none; border-radius: 6px; font-weight: bold; font-size: 13px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0F9A94, stop:1 #0A9A93); }"
        "QPushButton:disabled { background-color: #2B2B40; color: #6C6C80; }"
    );
    
    nutXacNhanTra = new QPushButton("TRẢ SÁCH");
    nutXacNhanTra->setMinimumHeight(35);
    nutXacNhanTra->setEnabled(false);
    nutXacNhanTra->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3699FF, stop:1 #0D7DFF); "
        "color: white; border: none; border-radius: 6px; font-weight: bold; font-size: 13px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #187DE4, stop:1 #0C6FE6); }"
        "QPushButton:disabled { background-color: #2B2B40; color: #6C6C80; }"
    );
    
    nutBaoMat = new QPushButton("BÁO MẤT");
    nutBaoMat->setMinimumHeight(35);
    nutBaoMat->setEnabled(false);
    nutBaoMat->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #F64E60, stop:1 #E63950); "
        "color: white; border: none; border-radius: 6px; font-weight: bold; font-size: 13px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #D12E40, stop:1 #C72838); }"
        "QPushButton:disabled { background-color: #2B2B40; color: #6C6C80; }"
    );
    
    boTriNut->addWidget(nutXacNhanMuon);
    boTriNut->addWidget(nutXacNhanTra);
    boTriNut->addWidget(nutBaoMat);
    
    boTriPhai->addLayout(boTriNut);
    boTriPhai->addStretch();
    
    // ThÃªm 2 cá»™t vÃ o layout chÃ­nh
    boTriChinh->addWidget(cotTrai, 1);
    boTriChinh->addWidget(cotPhai, 1);
    
    hopTab->addTab(widgetMuonSach, "Mượn Sách");
    
    // Khởi tạo dữ liệu
    trangHienTai = 1;
    soDocGiaTrenTrang = 10;
    tongSoTrang = 1;
    
    // Connect signals
    connect(nutTimDocGia, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyTimKiem);
    connect(nutLamMoi, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyLamMoiDanhSach);
    connect(nutQuetMaDocGia, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyNutQuetMaDocGia);
    connect(bangDocGia, &QTableView::clicked, this, [this](const QModelIndex &idx){ this->xuLyChonDocGia(idx.row(), idx.column()); });
    connect(nutHuyChonDocGia, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyHuyChonDocGia);
    connect(hopXacNhanMuon, &QCheckBox::checkStateChanged, this, &WidgetMuonTraSach::xuLyThayDoiCheckbox);
    connect(nutQuetMaSach, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyNutQuetMaSach);
    connect(nutXacNhanMuon, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyNutXacNhanMuon);
    connect(nutXacNhanTra, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyNutXacNhanTra);
    connect(nutBaoMat, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyNutBaoMat);
    connect(nutInLaiPhieu, &QPushButton::clicked, this, &WidgetMuonTraSach::xuLyInLaiPhieu);
    if (bangSachDangMuon->selectionModel()) {
        connect(bangSachDangMuon->selectionModel(), &QItemSelectionModel::currentChanged,
                this, [this](const QModelIndex &current, const QModelIndex &){ this->xuLyChonSachTra(current.row(), current.column()); });
    }
    
    // Load dá»¯ liá»‡u ban Ä‘áº§u
    hienThiDanhSachDocGia();
}

void WidgetMuonTraSach::thieLapBangDocGia() {
    // View already has model set (DocGiaTableModel). Configure visual aspects here.
    bangDocGia->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangDocGia->setSelectionMode(QAbstractItemView::SingleSelection);
    bangDocGia->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bangDocGia->horizontalHeader()->setStretchLastSection(true);
    bangDocGia->setAlternatingRowColors(true);
    bangDocGia->setMinimumHeight(350);

    // Default column widths
    bangDocGia->setColumnWidth(0, 50);
    bangDocGia->setColumnWidth(1, 80);
    bangDocGia->setColumnWidth(2, 180);
    bangDocGia->setColumnWidth(3, 70);
    bangDocGia->setColumnWidth(4, 110);
    bangDocGia->setColumnWidth(5, 70);

    bangDocGia->setStyleSheet(
        "QTableView { background-color: #1E1E2D; color: #FFFFFF; gridline-color: #2B2B40; "
        "border: 1px solid #2B2B40; border-radius: 8px; font-size: 13px; }"
        "QHeaderView::section { background-color: #2B2B40; color: #B5B5C3; "
        "border: none; padding: 10px; font-weight: bold; }"
        "QTableView::item { padding: 8px; }"
        "QTableView::item:selected { background-color: #3699FF; color: white; }"
        "QTableView::item:alternate { background-color: #1A1A27; }"
    );

    // Ensure header labels are shown (model provides them via headerData)
    bangDocGia->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
}

void WidgetMuonTraSach::hienThiDanhSachDocGia() {
    // Lấy toàn bộ độc giả (use DBBatcher snapshot when available)
    PTRDG danhSach[1000];
    int tongSo = 0;
#if defined(__has_include) && __has_include("utils/DBBatcher.h")
    std::vector<PTRDG> _tmp = DBBatcher::snapshotDocGia();
    int _cnt = static_cast<int>(std::min<size_t>(_tmp.size(), 1000));
    for (int _i = 0; _i < _cnt; ++_i) danhSach[_i] = _tmp[_i];
    tongSo = _cnt;
#else
    DuyetCayRaMang(rootDocGia, danhSach, tongSo);
#endif
    
    // LÆ°u danh sÃ¡ch Ä‘áº§y Ä‘á»§
    danhSachDocGia.clear();
    for (int i = 0; i < tongSo; i++) {
        danhSachDocGia.push_back(danhSach[i]);
    }
    
    // Hiá»ƒn thá»‹ trang hiá»‡n táº¡i
    hienThiTrang();
}

void WidgetMuonTraSach::hienThiTrang() {
    // Recompute total pages and clamp current page
    tongSoTrang = (danhSachDocGia.size() + soDocGiaTrenTrang - 1) / soDocGiaTrenTrang;
    if (tongSoTrang < 1) tongSoTrang = 1;
    if (trangHienTai > tongSoTrang) trangHienTai = tongSoTrang;

    if (modelDocGia) {
        modelDocGia->setPage(trangHienTai);
        modelDocGia->setPageSize(soDocGiaTrenTrang);
        modelDocGia->refresh();
    }

    nhanTongDocGia->setText(QString("Tổng: %1 độc giả").arg(danhSachDocGia.size()));
}

void WidgetMuonTraSach::xuLyTimKiem() {
    QString tuKhoa = oNhapTimDocGia->text().trimmed();
    
    if (tuKhoa.isEmpty()) {
        hienThiDanhSachDocGia();
        return;
    }
    
    extern PTRDG rootDocGia;
    
    // Kiá»ƒm tra xem lÃ  mÃ£ tháº» hay tÃªn
    bool laMaThe = false;
    int maThe = tuKhoa.toInt(&laMaThe);
    
    danhSachDocGia.clear();
    
    if (laMaThe) {
        // TÃ¬m theo mÃ£ tháº»
        PTRDG docGia = timDocGia(rootDocGia, maThe);
        if (docGia) {
            danhSachDocGia.push_back(docGia);
        }
    } else {
        // TÃ¬m theo tÃªn
        PTRDG ketQua[1000];
        int soKetQua = TimDocGiaTheoHoTen(rootDocGia, tuKhoa.toStdString(), ketQua, 1000);
        for (int i = 0; i < soKetQua; i++) {
            danhSachDocGia.push_back(ketQua[i]);
        }
    }
    
    // Cáº­p nháº­t phÃ¢n trang
    trangHienTai = 1;
    tongSoTrang = (danhSachDocGia.size() + soDocGiaTrenTrang - 1) / soDocGiaTrenTrang;
    if (tongSoTrang < 1) tongSoTrang = 1;
    
    hienThiTrang();
}

void WidgetMuonTraSach::xuLyLamMoiDanhSach() {
    oNhapTimDocGia->clear();
    trangHienTai = 1;
    hienThiDanhSachDocGia();
}

void WidgetMuonTraSach::xuLyTrangTruoc() {
    if (trangHienTai > 1) {
        trangHienTai--;
        hienThiTrang();
    }
}

void WidgetMuonTraSach::xuLyTrangSau() {
    if (trangHienTai < tongSoTrang) {
        trangHienTai++;
        hienThiTrang();
    }
}

void WidgetMuonTraSach::xuLyChonDocGia(int hang, int) {
    if (hang < 0) return;
    
    int chiSoThucTe = (trangHienTai - 1) * soDocGiaTrenTrang + hang;
    if (chiSoThucTe >= (int)danhSachDocGia.size()) return;
    
    docGiaHienTai = danhSachDocGia[chiSoThucTe];
    
    // Hiá»ƒn thá»‹ thÃ´ng tin Ä‘á»™c giáº£
    QString thongTin = QString(
        "<b>MÃ£ Tháº»:</b> %1<br>"
        "<b>Há» TÃªn:</b> %2 %3<br>"
        "<b>Giá»›i TÃ­nh:</b> %4<br>"
        "<b>Tráº¡ng ThÃ¡i:</b> <span style='color:%5'>%6</span><br>"
        "<b>Äang MÆ°á»£n:</b> <span style='color:%7'>%8/3 cuá»‘n</span>"
    )
    .arg(docGiaHienTai->data.MaThe)
    .arg(QString::fromStdString(docGiaHienTai->data.Ho))
    .arg(QString::fromStdString(docGiaHienTai->data.Ten))
    .arg(docGiaHienTai->data.Phai ? "Ná»¯" : "Nam")
    .arg(docGiaHienTai->data.TrangThai == 1 ? "#1BC5BD" : "#F64E60")
    .arg(docGiaHienTai->data.TrangThai == 1 ? "Hoáº¡t Ä‘á»™ng" : "KhÃ³a")
    .arg(docGiaHienTai->data.soSachDangMuon >= 3 ? "#F64E60" : "#FFA800")
    .arg(docGiaHienTai->data.soSachDangMuon);
    
    SafeGui::safeSetText(nhanThongTinDocGia, thongTin);
    SafeGui::safeSetStyleSheet(nhanThongTinDocGia,
        "color: #FFFFFF; font-size: 13px; padding: 15px; "
        "background-color: #1E1E2D; border-radius: 6px;"
    );
    
    // Load sÃ¡ch Ä‘ang mÆ°á»£n
    capNhatBangSachDangMuon();
    
    // KÃ­ch hoáº¡t cÃ¡c nÃºt
    nutHuyChonDocGia->setEnabled(true);
    
    // Kiá»ƒm tra Ä‘iá»u kiá»‡n mÆ°á»£n
    if (docGiaHienTai->data.TrangThai == 1 && docGiaHienTai->data.soSachDangMuon < 3) {
        oNhapISBN->setEnabled(true);
        hopXacNhanMuon->setEnabled(true);
    } else {
        oNhapISBN->setEnabled(false);
        hopXacNhanMuon->setEnabled(false);
    }
}

void WidgetMuonTraSach::xuLyHuyChonDocGia() {
    docGiaHienTai = nullptr;
    bangDocGia->clearSelection();
    
    SafeGui::safeSetText(nhanThongTinDocGia, "* Click vÃ o danh sÃ¡ch Ä‘á»™c giáº£ Ä‘á»ƒ chá»n");
    SafeGui::safeSetStyleSheet(nhanThongTinDocGia,
        "color: #B5B5C3; font-size: 13px; padding: 15px; "
        "background-color: #1E1E2D; border-radius: 6px;"
    );
    
    // Clear model-backed data for borrowed books
    danhSachSachDangMuon.clear();
    if (modelSachDangMuon) { if (auto m = dynamic_cast<SachDangMuonModel*>(modelSachDangMuon)) m->refresh(); }
    SafeGui::safeSetTitle(nhanTieuDeSachDangMuon, "SÁCH ĐANG MƯỢN (0/3)");
    
    oNhapISBN->clear();
    oNhapISBN->setEnabled(false);
    hopXacNhanMuon->setChecked(false);
    hopXacNhanMuon->setEnabled(false);
    nutXacNhanMuon->setEnabled(false);
    nutXacNhanTra->setEnabled(false);
    nutBaoMat->setEnabled(false);
    nutHuyChonDocGia->setEnabled(false);
}

void WidgetMuonTraSach::capNhatBangSachDangMuon() {
    if (!docGiaHienTai) return;
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;

    ThongTinSachDangMuon_DTO dsSach[3];
    int soSach = LayDSSachDangMuon(docGiaHienTai, dsSach, 3, dsDauSach, soLuongDauSach);

    danhSachSachDangMuon.clear();
    for (int i = 0; i < soSach; i++) {
        danhSachSachDangMuon.push_back(dsSach[i]);
    }

    // Use uniform row height for performance
    bangSachDangMuon->verticalHeader()->setDefaultSectionSize(36);

    // refresh model
    if (modelSachDangMuon) { if (auto m = dynamic_cast<SachDangMuonModel*>(modelSachDangMuon)) m->refresh(); }

    nhanTieuDeSachDangMuon->setTitle(QString("SÁCH ĐANG MƯỢN (%1/3)").arg(soSach));
}

void WidgetMuonTraSach::xuLyThayDoiCheckbox(int trangThai) {
    if (trangThai == Qt::Checked && !oNhapISBN->text().trimmed().isEmpty()) {
        nutXacNhanMuon->setEnabled(true);
    } else {
        nutXacNhanMuon->setEnabled(false);
    }
}

void WidgetMuonTraSach::xuLyChonSachTra(int hang, int) {
    if (hang >= 0) {
        nutXacNhanTra->setEnabled(true);
        nutBaoMat->setEnabled(true);
        nutInLaiPhieu->setEnabled(true);
    }
}

void WidgetMuonTraSach::thieLapTabTraSach() {
    widgetTraSach = new QWidget();
    QVBoxLayout *boTri = new QVBoxLayout(widgetTraSach);
    boTri->setContentsMargins(20, 20, 20, 20);
    
    QLabel *thongBao = new QLabel(
        "📖 CHỨC NĂNG TRẢ SÁCH\n\n"
        "Sử dụng tab 'Mượn Sách' để thực hiện trả sách:\n\n"
        "1. Chọn độc giả từ danh sách bên trái\n"
        "2. Xem sách đang mượn trong bảng bên phải\n"
        "3. Click chọn sách cần trả\n"
        "4. Nhấn nút 'TRẢ SÁCH' hoặc 'BÁO MẤT SÁCH'"
    );
    thongBao->setStyleSheet(
        "background-color: #252534; color: #B5B5C3; padding: 40px; "
        "border-radius: 8px; font-size: 13px; line-height: 1.8;"
    );
    thongBao->setAlignment(Qt::AlignCenter);
    boTri->addWidget(thongBao);
    
    hopTab->addTab(widgetTraSach, "Trả Sách");
}

void WidgetMuonTraSach::thieLapTabLichSu() {
    widgetLichSu = new QWidget();
    QVBoxLayout *boTri = new QVBoxLayout(widgetLichSu);
    boTri->setContentsMargins(20, 20, 20, 20);
    
    QLabel *thongBao = new QLabel("📜 Lịch Sử Mượn Trả\n\n"
        "Chức năng xem lịch sử đang được phát triển...\n\n"
        "Sẽ hiển thị:\n"
        "- Tất cả giao dịch mượn/trả\n"
        "- Lọc theo độc giả\n"
        "- Lọc theo thời gian\n"
        "- Trạng thái: Đang mượn / Đã trả / Mất");
    thongBao->setStyleSheet(
        "background-color: #252534; color: #B5B5C3; padding: 40px; "
        "border-radius: 8px; font-size: 13px; line-height: 1.8;"
    );
    thongBao->setAlignment(Qt::AlignCenter);
    boTri->addWidget(thongBao);
    
    hopTab->addTab(widgetLichSu, "📜 Lịch Sử");
}

void WidgetMuonTraSach::lamMoi() {
    xoaThongTinHienThi();
}

void WidgetMuonTraSach::xoaThongTinHienThi() {
    oNhapTimDocGia->clear();
    docGiaHienTai = nullptr;
    sachDangChonMuon = nullptr;
    banSaoDangChon = nullptr;
}

void WidgetMuonTraSach::xuLyNutXacNhanMuon() {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, "Cáº£nh BÃ¡o", "Vui lÃ²ng chá»n Ä‘á»™c giáº£ trÆ°á»›c!");
        return;
    }
    
    if (!hopXacNhanMuon->isChecked()) {
        QMessageBox::warning(this, "Cáº£nh BÃ¡o", "Vui lÃ²ng Ä‘Ã¡nh dáº¥u 'XÃ¡c Nháº­n MÆ°á»£n'!");
        return;
    }
    
    QString isbn = oNhapISBN->text().trimmed();
    if (isbn.isEmpty()) {
        QMessageBox::warning(this, "Cáº£nh BÃ¡o", "Vui lÃ²ng nháº­p mÃ£ sÃ¡ch cáº§n mÆ°á»£n!");
        return;
    }
    
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    std::string ketQua = MuonSach(docGiaHienTai, isbn.toStdString(), dsDauSach, soLuongDauSach);
    
    if (ketQua.find("Thành công") != std::string::npos || ketQua.find("thành công") != std::string::npos) {
        // Tìm tên sách và mã sách để ghi log + in phiếu
        QString tenSach = "";
        QString maSach = "";
        for (int i = 0; i < soLuongDauSach; i++) {
            if (dsDauSach[i] && QString::fromStdString(dsDauSach[i]->ISBN) == isbn) {
                tenSach = QString::fromStdString(dsDauSach[i]->tenSach);
                // Tìm mã sách vừa mượn (sách đầu tiên có trạng thái đã mượn)
                PTRDMS p = dsDauSach[i]->dms;
                while (p != nullptr) {
                    if (p->trangThai == DANG_MUON) { // 1 = Đang mượn
                        maSach = QString::fromStdString(p->maSach);
                        break;
                    }
                    p = p->next;
                }
                break;
            }
        }
        
        // Ghi log giao dịch mượn
        TransactionLogger::ghiLogMuon(docGiaHienTai, isbn, tenSach, true);
        
        // Tự động in phiếu mượn với QR code
        QString ngayMuon = QDate::currentDate().toString("dd/MM/yyyy");
        PrintHelper::inPhieuMuonTuDong(docGiaHienTai, maSach, tenSach, ngayMuon, this);
        
        // Tự động lưu file
        extern PTRDG rootDocGia;
        saveDocGia(rootDocGia);
        GhiDanhSachDauSach(FilePathManager::getDauSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
        GhiDanhMucSach(FilePathManager::getDanhMucSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
        
        QMessageBox::information(this, "ThÃ nh CÃ´ng", QString::fromStdString(ketQua));
        emit thongBaoCapNhat("ÄÃ£ mÆ°á»£n sÃ¡ch thÃ nh cÃ´ng!");
        emit duLieuThayDoi(); // Realtime update Dashboard
        
        // Cáº­p nháº­t láº¡i thÃ´ng tin
        capNhatBangSachDangMuon();
        oNhapISBN->clear();
        hopXacNhanMuon->setChecked(false);
        nutXacNhanMuon->setEnabled(false);
        
        // Cáº­p nháº­t thÃ´ng tin Ä‘á»™c giáº£
        int chiSo = (trangHienTai - 1) * soDocGiaTrenTrang + bangDocGia->currentIndex().row();
        if (chiSo >= 0 && chiSo < (int)danhSachDocGia.size()) {
            xuLyChonDocGia(bangDocGia->currentIndex().row(), 0);
        }
    } else {
        QMessageBox::warning(this, "Lá»—i", QString::fromStdString(ketQua));
    }
}

void WidgetMuonTraSach::xuLyNutXacNhanTra() {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, "Cáº£nh BÃ¡o", "Vui lÃ²ng chá»n Ä‘á»™c giáº£ trÆ°á»›c!");
        return;
    }
    
    int dongChon = bangSachDangMuon->currentIndex().row();
    if (dongChon < 0) {
        QMessageBox::warning(this, "Cáº£nh BÃ¡o", "Vui lÃ²ng chá»n sÃ¡ch cáº§n tráº£!");
        return;
    }
    if (dongChon < 0 || dongChon >= (int)danhSachSachDangMuon.size()) {
        QMessageBox::warning(this, "Cảnh Báo", "Dữ liệu chọn không hợp lệ!");
        return;
    }

    const ThongTinSachDangMuon_DTO &dto = danhSachSachDangMuon[dongChon];
    QString maSach = QString::fromStdString(dto.maSach);
    QString tenSach = QString::fromStdString(dto.tenSach);
    
    QMessageBox::StandardButton xacNhan = QMessageBox::question(this, "XÃ¡c Nháº­n Tráº£ SÃ¡ch",
        QString("XÃ¡c nháº­n tráº£ sÃ¡ch:\n\nMÃ£: %1\nTÃªn: %2")
            .arg(maSach)
            .arg(tenSach),
        QMessageBox::Yes | QMessageBox::No);
    
    if (xacNhan == QMessageBox::Yes) {
        extern PTRDS dsDauSach[];
        extern int soLuongDauSach;
        
        std::string ketQua = TraSach(docGiaHienTai, maSach.toStdString(), dsDauSach, soLuongDauSach);
        
        if (ketQua.find("Thành công") != std::string::npos || ketQua.find("thành công") != std::string::npos) {
            // Ghi log giao dịch trả
            TransactionLogger::ghiLogTra(docGiaHienTai, maSach, tenSach, true);
            
            // Tự động lưu file
            extern PTRDG rootDocGia;
            saveDocGia(rootDocGia);
            GhiDanhSachDauSach(FilePathManager::getDauSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
            GhiDanhMucSach(FilePathManager::getDanhMucSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
            
            QMessageBox::information(this, "ThÃ nh CÃ´ng", QString::fromStdString(ketQua));
            emit thongBaoCapNhat("ÄÃ£ tráº£ sÃ¡ch thÃ nh cÃ´ng!");
            emit duLieuThayDoi(); // Realtime update Dashboard
            
            // Cập nhật lại
            capNhatBangSachDangMuon();
            nutXacNhanTra->setEnabled(false);
            nutBaoMat->setEnabled(false);
            nutInLaiPhieu->setEnabled(false);
            
            // Cập nhật thông tin độc giả
            int chiSo = (trangHienTai - 1) * soDocGiaTrenTrang + bangDocGia->currentIndex().row();
            if (chiSo >= 0 && chiSo < (int)danhSachDocGia.size()) {
                xuLyChonDocGia(bangDocGia->currentIndex().row(), 0);
            }
        } else {
            QMessageBox::warning(this, "Lá»—i", QString::fromStdString(ketQua));
        }
    }
}

void WidgetMuonTraSach::xuLyNutBaoMat() {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, "Cáº£nh BÃ¡o", "Vui lÃ²ng chá»n Ä‘á»™c giáº£ trÆ°á»›c!");
        return;
    }
    
    int dongChon = bangSachDangMuon->currentIndex().row();
    if (dongChon < 0) {
        QMessageBox::warning(this, "Cáº£nh BÃ¡o", "Vui lÃ²ng chá»n sÃ¡ch cáº§n bÃ¡o máº¥t!");
        return;
    }
    if (dongChon < 0 || dongChon >= (int)danhSachSachDangMuon.size()) {
        QMessageBox::warning(this, "Cảnh Báo", "Dữ liệu chọn không hợp lệ!");
        return;
    }
    const ThongTinSachDangMuon_DTO &dto = danhSachSachDangMuon[dongChon];
    QString maSach = QString::fromStdString(dto.maSach);
    QString tenSach = QString::fromStdString(dto.tenSach);
    
    QMessageBox::StandardButton xacNhan = QMessageBox::question(this, "âš ï¸ XÃC NHáº¬N BÃO Máº¤T SÃCH",
        QString("âš ï¸ BÃO Máº¤T SÃCH lÃ  thao tÃ¡c NGHIÃŠM TRá»ŒNG!\n\nÄá»™c giáº£ sáº½ bá»‹ khÃ³a tÃ i khoáº£n.\n\nMÃ£: %1\nTÃªn: %2\n\nBáº¡n cÃ³ cháº¯c cháº¯n?")
            .arg(maSach)
            .arg(tenSach),
        QMessageBox::Yes | QMessageBox::No);
    
    if (xacNhan == QMessageBox::Yes) {
        extern PTRDS dsDauSach[];
        extern int soLuongDauSach;
        
        std::string ketQua = BaoMatSach(docGiaHienTai, maSach.toStdString(), dsDauSach, soLuongDauSach);
        
        if (ketQua.find("ThÃ nh cÃ´ng") != std::string::npos || ketQua.find("thÃ nh cÃ´ng") != std::string::npos) {
            // Tá»± Ä‘á»™ng lÆ°u file
            extern PTRDG rootDocGia;
            saveDocGia(rootDocGia);
            GhiDanhSachDauSach(FilePathManager::getDauSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
            GhiDanhMucSach(FilePathManager::getDanhMucSachPathStd().c_str(), dsDauSach, soLuongDauSach, true);
            
            QMessageBox::information(this, "ÄÃ£ BÃ¡o Máº¥t", QString::fromStdString(ketQua));
            emit thongBaoCapNhat("ÄÃ£ bÃ¡o máº¥t sÃ¡ch!");
            emit duLieuThayDoi(); // Realtime update Dashboard
            
            // Cáº­p nháº­t láº¡i
            capNhatBangSachDangMuon();
            nutXacNhanTra->setEnabled(false);
            nutBaoMat->setEnabled(false);
            
            // Cáº­p nháº­t thÃ´ng tin Ä‘á»™c giáº£
            int chiSo = (trangHienTai - 1) * soDocGiaTrenTrang + bangDocGia->currentIndex().row();
            if (chiSo >= 0 && chiSo < (int)danhSachDocGia.size()) {
                xuLyChonDocGia(bangDocGia->currentIndex().row(), 0);
            }
            
        } else {
            QMessageBox::warning(this, "Lá»—i", QString::fromStdString(ketQua));
        }
    }
}

void WidgetMuonTraSach::xuLyInLaiPhieu() {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn độc giả trước!");
        return;
    }
    
    int dongChon = bangSachDangMuon->currentIndex().row();
    if (dongChon < 0 || dongChon >= (int)danhSachSachDangMuon.size()) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn sách để in lại phiếu!");
        return;
    }

    const ThongTinSachDangMuon_DTO &dto = danhSachSachDangMuon[dongChon];
    QString maSach = QString::fromStdString(dto.maSach);
    QString tenSach = QString::fromStdString(dto.tenSach);
    QString ngayMuon = QString::fromStdString(dto.ngayMuon);
    
    // In lại phiếu mượn với QR code
    PrintHelper::inPhieuMuonTuDong(docGiaHienTai, maSach, tenSach, ngayMuon, this);
}

void WidgetMuonTraSach::xuLyNhapMaThe(const QString &) {}
void WidgetMuonTraSach::xuLyChonSachMuon(int) {}
void WidgetMuonTraSach::xuLyChuyenTab(int) {}
void WidgetMuonTraSach::hienThiThongTinDocGia(PTRDG) {}
void WidgetMuonTraSach::hienThiDanhSachSachCoTheMuon() {}
void WidgetMuonTraSach::hienThiDanhSachSachDangMuon(PTRDG) {}
bool WidgetMuonTraSach::kiemTraDieuKienMuon(PTRDG) { return false; }
int WidgetMuonTraSach::tinhSoNgayQuaHan(const std::string &) { return 0; }
int WidgetMuonTraSach::tinhTienPhat(int) { return 0; }

void WidgetMuonTraSach::xuLyNutQuetMaDocGia() {
    HopThoaiQuetMaVach *dialogQuet = new HopThoaiQuetMaVach(HopThoaiQuetMaVach::QUET_DOC_GIA, this);
    if (dialogQuet->exec() == QDialog::Accepted) {
        QString maThe = dialogQuet->layMaQuet();
        if (!maThe.isEmpty()) {
            oNhapTimDocGia->setText(maThe);
            xuLyTimKiem();
        }
    }
    delete dialogQuet;
}

void WidgetMuonTraSach::xuLyNutQuetMaSach() {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, QStringLiteral("Lỗi"), QStringLiteral("Vui lòng chọn độc giả trước!"));
        return;
    }
    HopThoaiQuetMaVach *dialogQuet = new HopThoaiQuetMaVach(HopThoaiQuetMaVach::QUET_SACH, this);
    if (dialogQuet->exec() == QDialog::Accepted) {
        QString isbn = dialogQuet->layMaQuet();
        if (!isbn.isEmpty()) {
            oNhapISBN->setText(isbn);
            extern PTRDS dsDauSach[];
            extern int soLuongDauSach;
            bool timThay = false;
            for (int i = 0; i < soLuongDauSach; i++) {
                if (dsDauSach[i] && QString::fromStdString(dsDauSach[i]->ISBN) == isbn) {
                    timThay = true;
                    hopXacNhanMuon->setChecked(true);
                    break;
                }
            }
            if (!timThay) {
                QMessageBox::warning(this, QStringLiteral("Lỗi"), QStringLiteral("Không tìm thấy sách với ISBN: ") + isbn);
            }
        }
    }
    delete dialogQuet;
}

