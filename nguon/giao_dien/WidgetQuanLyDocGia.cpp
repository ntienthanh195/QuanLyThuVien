/**
 * @file WidgetQuanLyDocGia.cpp
 * @path nguon/giao_dien/WidgetQuanLyDocGia.cpp
 * @description Widget quản lý hồ sơ học viên (độc giả) (Phiên bản V2). 
 * Cung cấp giao diện thêm, sửa, xóa, tìm kiếm và phân loại học viên.
 * @project Library Management System (QuanLyThuVien)
 */
#include "giao_dien/WidgetQuanLyDocGia.h"

#include <vector>
#include "utils/LanguageManager.h"
#include "giao_dien/hop_thoai/HopThoaiThemDocGia.h"
#include "giao_dien/hop_thoai/HopThoaiSuaDocGia.h"
#include "views/dialogs/ManHinhChiTietDocGia.h"
#include "models/DocGia.h"
#include "utils/ConfirmationDialog.h"
#include "giao_dien/hop_thoai/ManHinhQuanLyIn.h"
#include "utils/PrintHelper.h"
#include "views/dialogs/ManHinhPreviewImport.h"
#include "utils/ImportManager.h"
#include "utils/AnimationHelper.h"
#include <QFileDialog>
#include <QDir>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QVBoxLayout>
#if defined(__has_include)
#  if __has_include("utils/DBBatcher.h")
#    include "utils/DBBatcher.h"
#  endif
#endif
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include "models/UserManager.h"
#include <QTableView>
#include <QAbstractTableModel>
#include "utils/SettingsManager.h"
#include <QTimer>
#include <QEvent>

// Lightweight model for `WidgetQuanLyDocGia` (virtualized)
class DocGiaTableModel : public QAbstractTableModel {
public:
    DocGiaTableModel(std::vector<PTRDG> *data, QObject *parent = nullptr)
        : QAbstractTableModel(parent), m_data(data) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return m_data ? (int)m_data->size() : 0;
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return 9;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!m_data || !index.isValid()) return QVariant();
        int r = index.row();
        if (r < 0 || r >= (int)m_data->size()) return QVariant();
        PTRDG dgptr = (*m_data)[r];
        if (!dgptr) return QVariant();
        const TheDocGia &dg = dgptr->data;

        if (role == Qt::DisplayRole) {
            switch (index.column()) {
            case 0: return QString::number(r + 1);
            case 1: return QString::number(dg.MaThe);
            case 2: return QString::fromStdString(dg.Ho);
            case 3: return QString::fromStdString(dg.Ten);
            case 4: return QString::fromStdString(dg.Lop); // Was Phai
            case 5: return QString::fromStdString(dg.SDT); // Was TrangThai (shifted) -> NO, I will swap columns logic
            case 6: {
                int maxSach = SettingsManager::instance().getInt("SO_SACH_MUON_TOI_DA", 3);
                return QString("%1 / %2").arg(dg.soSachDangMuon).arg(maxSach);
            }
            case 7: return dg.TrangThai == 1 ? LanguageManager::instance().getText("STATUS_ACTIVE") : LanguageManager::instance().getText("STATUS_LOCKED");
            case 8: {
                 int soNgay = TinhSoNgayQuaHanLonNhat(dgptr);
                 return soNgay > 0 ? LanguageManager::instance().getText("LBL_DAYS_COUNT").arg(soNgay) : QStringLiteral("—");
            }
            }
            }


        if (role == Qt::UserRole) {
            // expose MaThe for quick lookup
            if (index.column() == 1) return dg.MaThe;
            return QVariant();
        }

        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case 0: return LanguageManager::instance().getText("TABLE_HEADER_STT");
            case 1: return LanguageManager::instance().getText("TABLE_HEADER_CODE");
            case 2: return LanguageManager::instance().getText("TABLE_HEADER_LASTNAME");
            case 3: return LanguageManager::instance().getText("TABLE_HEADER_FIRSTNAME");
            case 4: return LanguageManager::instance().getText("TABLE_HEADER_CLASS"); // New Key
            case 5: return LanguageManager::instance().getText("TABLE_HEADER_PHONE"); // New Key
            case 6: return LanguageManager::instance().getText("TABLE_HEADER_BORROWED");
            case 7: return LanguageManager::instance().getText("TABLE_HEADER_STATUS");
            case 8: return LanguageManager::instance().getText("TABLE_HEADER_OVERDUE");
            }
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    void refresh() { beginResetModel(); endResetModel(); }

private:
    std::vector<PTRDG> *m_data;
};

// Include responsive helpers
#include "utils/ResponsiveHelper.h"
#include "utils/AnimationHelper.h"
#include "utils/EventBus.h"

WidgetQuanLyDocGia::WidgetQuanLyDocGia(QWidget *cha) 
    : QWidget(cha), docGiaDangChon(nullptr), maTheDangChon(-1) {
    // Apply same overall background and object name as the book management UI
    setStyleSheet(
        "QWidget#WidgetQuanLyDocGia {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "        stop:0 #071233, stop:0.5 #08283d, stop:1 #071233);"
        "}"
    );
    setObjectName("WidgetQuanLyDocGia");

    thieLapGiaoDien();
    taiLaiDanhSach();

    // Realtime updates: refresh when EventBus emits relevant events
    connect(&EventBus::instance(), &EventBus::docGiaAdded, this, [this](int){ this->taiLaiDanhSach(); });
    connect(&EventBus::instance(), &EventBus::docGiaUpdated, this, [this](int){ this->taiLaiDanhSach(); });
    connect(&EventBus::instance(), &EventBus::docGiaRemoved, this, [this](int){ this->taiLaiDanhSach(); });

    connect(&EventBus::instance(), &EventBus::muonTraAdded, this, [this](int, const QString&){ this->taiLaiDanhSach(); });
    connect(&EventBus::instance(), &EventBus::muonTraUpdated, this, [this](int, const QString&){ this->taiLaiDanhSach(); });
    connect(&EventBus::instance(), &EventBus::muonTraRemoved, this, [this](int, const QString&){ this->taiLaiDanhSach(); });

    // Refresh when global data changes (e.g. background loader finished)
    connect(&EventBus::instance(), &EventBus::dataChanged, this, [this]() { this->taiLaiDanhSach(); });
    
    // Refresh when Settings change (e.g. max borrowed books limit)
    connect(&EventBus::instance(), &EventBus::settingsUpdated, this, [this]() { this->taiLaiDanhSach(); });

    // Language
    connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this, &WidgetQuanLyDocGia::capNhatNgonNgu);
    QTimer::singleShot(0, this, &WidgetQuanLyDocGia::capNhatNgonNgu);
}

WidgetQuanLyDocGia::~WidgetQuanLyDocGia() {}

void WidgetQuanLyDocGia::thieLapGiaoDien() {

    
    boTriChinh = new QVBoxLayout(this);
    // Standardized margins to match Book tab (20px) to prevent jump
    boTriChinh->setContentsMargins(20, 20, 20, 20);
    boTriChinh->setSpacing(15);


    
    // Tiêu đề đã được loại bỏ theo yêu cầu
    // layoutTieuDe removed
    
    // Header action bar (match style & behavior of WidgetQuanLySach)
    {
        QWidget *headerBar = new QWidget();
        headerBar->setStyleSheet("QWidget { background: transparent; }");
        QHBoxLayout *hLayout = new QHBoxLayout(headerBar);
        hLayout->setContentsMargins(0,0,0,0);
        hLayout->setSpacing(12);

        auto createButton = [](const QString &iconPath, const QString &text, const QString &color) -> QPushButton* {
            QPushButton *btn = new QPushButton(" " + text);
            if (!iconPath.isEmpty()) {
                btn->setIcon(QIcon(iconPath));
                btn->setIconSize(QSize(16, 16));
            }
            btn->setFixedHeight(36);
            
            // Standard Book Manager Styles
            if (color.compare("#0078D4", Qt::CaseInsensitive) == 0) {
                // Primary Blue (Add)
                btn->setStyleSheet(
                    "QPushButton { background-color: #0078D4; color: white; border: none; border-radius: 4px; padding: 6px 12px; font-weight: 600; }"
                    "QPushButton:hover { background-color: #106EBE; }"
                    "QPushButton:pressed { background-color: #005A9E; }"
                );
            } else if (color.compare("#C42B1C", Qt::CaseInsensitive) == 0) {
                // Delete Red
                btn->setStyleSheet(
                    "QPushButton { background-color: #C42B1C; color: white; border: none; border-radius: 4px; padding: 6px 12px; font-weight: 600; }"
                    "QPushButton:hover { background-color: #A80000; }"
                    "QPushButton:disabled { background-color: #2D2D30; color: #666; }"
                );
            } else {
                // Secondary Dark (Standard)
                btn->setStyleSheet(
                    "QPushButton { background-color: #2D2D30; color: #E0E0E0; border: 1px solid #3E3E42; border-radius: 4px; padding: 6px 12px; }"
                    "QPushButton:hover { background-color: #3E3E42; border-color: #0078D4; }"
                    "QPushButton:disabled { background-color: #2D2D30; color: #666; }"
                );
            }
            return btn;
        };

        // assign header buttons to member variables
        nutThemDocGia = createButton(":/icons/plus.svg", LanguageManager::instance().getText("BTN_ADD_READER"), "#0078D4");
        nutNhapFile = createButton(":/icons/import.svg", LanguageManager::instance().getText("BTN_IMPORT_EXCEL"), "#2D2D30");
        
        // header In and Chi Tiết
        nutHeaderIn = createButton(":/icons/print.svg", LanguageManager::instance().getText("BTN_PRINT"), "#2D2D30");
        nutSuaDocGia = createButton(":/icons/edit.svg", LanguageManager::instance().getText("BTN_EDIT"), "#2D2D30");
        nutHeaderChiTiet = createButton(":/icons/info.svg", LanguageManager::instance().getText("BTN_DETAILS"), "#2D2D30");
        
        // Add header lock/unlock/history buttons
        nutHeaderKhoa = createButton(":/icons/lock.svg", LanguageManager::instance().getText("BTN_LOCK_CARD"), "#2D2D30");
        nutHeaderMoKhoa = createButton(":/icons/unlock.svg", LanguageManager::instance().getText("BTN_UNLOCK_CARD"), "#2D2D30");
        nutHeaderLichSu = createButton(":/icons/history.svg", LanguageManager::instance().getText("BTN_HISTORY"), "#2D2D30");
        
        nutXoaDocGia = createButton(":/icons/delete.svg", LanguageManager::instance().getText("BTN_DELETE"), "#C42B1C");

        hLayout->addWidget(nutThemDocGia);
        hLayout->addWidget(nutNhapFile);
        hLayout->addWidget(nutHeaderIn);
        hLayout->addWidget(nutSuaDocGia);
        hLayout->addWidget(nutHeaderChiTiet);
        hLayout->addWidget(nutHeaderKhoa);
        hLayout->addWidget(nutHeaderMoKhoa);
        hLayout->addWidget(nutHeaderLichSu);
        hLayout->addWidget(nutXoaDocGia);
        hLayout->addStretch();

        boTriChinh->addWidget(headerBar);

        // Connect a subset to existing slots
        connect(nutThemDocGia, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNutThemDocGia);
        connect(nutSuaDocGia, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNutSuaDocGia);
        connect(nutXoaDocGia, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNutXoaDocGia);
        connect(nutNhapFile, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNhapDuLieuTu);
        // Header: In -> Open Print Manager (Grid View)
        connect(nutHeaderIn, &QPushButton::clicked, this, [this]() {
            ManHinhQuanLyIn *dialog = new ManHinhQuanLyIn(docGiaDangChon, danhSachDocGia, this);
            dialog->setMode(ManHinhQuanLyIn::READER_CONTEXT);
            dialog->exec();
            delete dialog;
        });
        
        // Header: Chi Tiết -> open NEW ManHinhChiTietDocGia
        connect(nutHeaderChiTiet, &QPushButton::clicked, this, [this]() {
            if (!docGiaDangChon) { QMessageBox::warning(this, LanguageManager::instance().getText("BTN_DETAILS"), LanguageManager::instance().getText("MSG_SELECT_READER_FIRST")); return; }
            ManHinhChiTietDocGia *dialog = new ManHinhChiTietDocGia(docGiaDangChon, this);
            dialog->exec();
            delete dialog;
            taiLaiDanhSach(); // Refresh in case they returned a book inside the details screen
        });
        
        // Header: Khóa / Mở Khóa / Lịch Sử -> connect to existing handlers
        connect(nutHeaderKhoa, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNutKhoaThe);
        connect(nutHeaderMoKhoa, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNutMoKhoaThe);
        connect(nutHeaderLichSu, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNutXemLichSu);
    }

    thieLapTimKiem();
    thieLapBangDocGia();
    
    qDebug() << "WidgetQuanLyDocGia: Responsive layout applied";
}

void WidgetQuanLyDocGia::thieLapTimKiem() {
    // Make search bar visually identical to the book management search bar
    QWidget *searchBar = new QWidget();
    searchBar->setStyleSheet("QWidget { background: transparent; }");
    QHBoxLayout *layout = new QHBoxLayout(searchBar);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(10);

    oNhapTimKiem = new QLineEdit();
    oNhapTimKiem->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_SEARCH_READER_FULL"));
    oNhapTimKiem->setFixedHeight(36);
    oNhapTimKiem->setStyleSheet(
        "QLineEdit {"
        "    background-color: #1a1d23;"
        "    color: #E0E0E0;"
        "    border: 1px solid #151820;"
        "    border-radius: 5px;"
        "    padding: 0 12px;"
        "    font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "    border: 1px solid #4A90E2;"
        "    background-color: #1a1d23;"
        "}"
    );

    hopChonTrangThai = new QComboBox();
    hopChonTrangThai->addItem(LanguageManager::instance().getText("FILTER_ALL"));
    hopChonTrangThai->addItem(LanguageManager::instance().getText("FILTER_ACTIVE"));
    hopChonTrangThai->addItem(LanguageManager::instance().getText("FILTER_LOCKED"));
    hopChonTrangThai->setFixedHeight(36);
    hopChonTrangThai->setStyleSheet(
        "QComboBox { background-color: #1a1d23; color: #E0E0E0; border: 1px solid #151820; border-radius: 5px; padding: 0 12px; font-size: 13px; }"
        "QComboBox:hover { border: 1px solid #4A90E2; }"
        "QComboBox::drop-down { border: none; width: 20px; }"
    );

    hopChonGioiTinh = new QComboBox();
    hopChonGioiTinh->addItem(LanguageManager::instance().getText("FILTER_ALL_GENDER"));
    hopChonGioiTinh->addItem(LanguageManager::instance().getText("GENDER_MALE"));
    hopChonGioiTinh->addItem(LanguageManager::instance().getText("GENDER_FEMALE"));
    hopChonGioiTinh->setFixedHeight(36);
    hopChonGioiTinh->setStyleSheet(hopChonTrangThai->styleSheet());

    nutTimKiem = new QPushButton(LanguageManager::instance().getText("BTN_SEARCH"));
    nutTimKiem->setFixedHeight(36);
    nutTimKiem->setFixedWidth(72);
    nutTimKiem->setStyleSheet(
        "QPushButton {"
        "    background-color: #5ba3f5;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    font-size: 13px;"
        "    font-weight: 500;"
        "}"
        "QPushButton:hover { background-color: #4a93e3; }"
        "QPushButton:pressed { background-color: #3d7dc4; }"
    );

    nutLamMoi = new QPushButton(LanguageManager::instance().getText("BTN_REFRESH"));
    nutLamMoi->setFixedHeight(36);
    nutLamMoi->setFixedWidth(86);
    nutLamMoi->setStyleSheet(
        "QPushButton { background-color: #242933; color: #CCCCCC; border: 1px solid #151820; border-radius: 4px; font-size: 13px; }"
        "QPushButton:hover { background-color: #2d3541; border: 1px solid #4A90E2; }"
    );

    layout->addWidget(oNhapTimKiem, 3);
    layout->addWidget(hopChonTrangThai, 1);
    layout->addWidget(hopChonGioiTinh, 1);
    layout->addWidget(nutTimKiem);
    layout->addWidget(nutLamMoi);

    boTriChinh->addWidget(searchBar);

    // Result label for search feedback (kept from book UI)
    QLabel *nhanKetQuaTimKiem = new QLabel("");
    nhanKetQuaTimKiem->setStyleSheet("color: #CFCFCF; font-size: 12px; padding-left: 6px;");
    nhanKetQuaTimKiem->setFixedHeight(20);
    boTriChinh->addWidget(nhanKetQuaTimKiem);

    // Connect signals - tìm kiếm real-time
    connect(oNhapTimKiem, &QLineEdit::textChanged, this, &WidgetQuanLyDocGia::xuLyTimKiem);
    connect(hopChonTrangThai, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WidgetQuanLyDocGia::xuLyLocTrangThai);
    connect(hopChonGioiTinh, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WidgetQuanLyDocGia::xuLyLocGioiTinh);
    connect(nutTimKiem, &QPushButton::clicked, this, [this]() { xuLyTimKiem(oNhapTimKiem->text()); });
    connect(nutLamMoi, &QPushButton::clicked, this, [this]() { taiLaiDanhSach(); });
}

void WidgetQuanLyDocGia::thieLapBangDocGia() {
    // switch to QTableView + model
    bangDocGia = new QTableView();
    modelDocGia = new DocGiaTableModel(&danhSachDocGia, this);
    bangDocGia->setModel(modelDocGia);
    
    // Use centralized InnerTable styling instead of inline stylesheet
    bangDocGia->setProperty("class", "InnerTable");
    
    bangDocGia->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangDocGia->setSelectionMode(QAbstractItemView::SingleSelection);
    bangDocGia->setAlternatingRowColors(true);
    bangDocGia->verticalHeader()->setVisible(false);
    bangDocGia->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bangDocGia->setMinimumHeight(450);
    
    // Responsive column widths (visual widths)
    bangDocGia->setColumnWidth(0, 50);   // STT
    bangDocGia->setColumnWidth(1, 80);   // Mã Thẻ
    bangDocGia->setColumnWidth(2, 120);  // Họ
    bangDocGia->setColumnWidth(3, 100);  // Tên
    bangDocGia->setColumnWidth(4, 90);   // Lop
    bangDocGia->setColumnWidth(5, 120);  // SDT
    bangDocGia->setColumnWidth(6, 130);  // Sách Đang Mượn
    bangDocGia->setColumnWidth(7, 120);  // Trang Thai
    bangDocGia->setColumnWidth(8, 100);  // Qua Han
    bangDocGia->horizontalHeader()->setStretchLastSection(true);
    // Row height similar to screenshot
    bangDocGia->verticalHeader()->setDefaultSectionSize(45);

    boTriChinh->addWidget(bangDocGia, 1);
    
    // Hint label
    // Hint label
    QLabel *hintLabel = new QLabel(LanguageManager::instance().getText("LBL_HINT_READER_DBLCLICK"));
    hintLabel->setStyleSheet("color: #666; font-style: italic; font-size: 11px; margin-top: 2px;");
    hintLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    boTriChinh->addWidget(hintLabel);

    connect(bangDocGia, &QTableView::clicked, this, [this](const QModelIndex &idx){ this->xuLyChonDong(idx.row(), idx.column()); });

    // Double-click -> View Details
    connect(bangDocGia, &QTableView::doubleClicked, this, [this](const QModelIndex &){
         if (nutHeaderChiTiet && nutHeaderChiTiet->isEnabled()) nutHeaderChiTiet->click();
    });
    
    // Right-click Context Menu
    bangDocGia->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(bangDocGia, &QTableView::customContextMenuRequested, this, [this](const QPoint &pos) {
        QModelIndex index = bangDocGia->indexAt(pos);
        if (!index.isValid()) return;

        QMenu contextMenu(this);
        contextMenu.setStyleSheet(
            "QMenu { background-color: #2D2D30; color: white; border: 1px solid #3E3E42; }"
            "QMenu::item { padding: 8px 20px; }"
            "QMenu::item:selected { background-color: #0078D4; }"
        );

        QAction *actChiTiet = contextMenu.addAction(QIcon(":/icons/info.svg"), LanguageManager::instance().getText("CTX_VIEW_DETAILS"));
        QAction *actSua = contextMenu.addAction(QIcon(":/icons/edit.svg"), LanguageManager::instance().getText("CTX_EDIT_READER"));
        contextMenu.addSeparator();
        QAction *actInThe = contextMenu.addAction(QIcon(":/icons/card.svg"), LanguageManager::instance().getText("CTX_PRINT_CARD"));
        contextMenu.addSeparator();
        // Check lock status to show correct action
        if (docGiaDangChon && docGiaDangChon->data.TrangThai == 1) {
            QAction *actKhoa = contextMenu.addAction(QIcon(":/icons/lock.svg"), LanguageManager::instance().getText("BTN_LOCK_CARD"));
            connect(actKhoa, &QAction::triggered, this, &WidgetQuanLyDocGia::xuLyNutKhoaThe);
        } else {
            QAction *actMo = contextMenu.addAction(QIcon(":/icons/unlock.svg"), LanguageManager::instance().getText("BTN_UNLOCK_CARD"));
            connect(actMo, &QAction::triggered, this, &WidgetQuanLyDocGia::xuLyNutMoKhoaThe);
        }
        
        QAction *actXoa = contextMenu.addAction(QIcon(":/icons/delete.svg"), LanguageManager::instance().getText("CTX_DELETE_READER"));

        connect(actChiTiet, &QAction::triggered, this, [this](){
            if (nutHeaderChiTiet && nutHeaderChiTiet->isEnabled()) nutHeaderChiTiet->click();
        });
        connect(actSua, &QAction::triggered, this, &WidgetQuanLyDocGia::xuLyNutSuaDocGia);
        connect(actInThe, &QAction::triggered, this, [this](){
            if (docGiaDangChon) PrintHelper::inTheDocGia(docGiaDangChon, this);
        });
        connect(actXoa, &QAction::triggered, this, &WidgetQuanLyDocGia::xuLyNutXoaDocGia);

        contextMenu.exec(bangDocGia->viewport()->mapToGlobal(pos));
    });
}

void WidgetQuanLyDocGia::thieLapNutChucNang() {
    // Chức năng đã được chuyển lên Header Bar.
    // Hàm này được giữ lại để tương thích nếu có gọi từ nơi khác, nhưng không làm gì cả.
    nhomChucNang = nullptr;
}
// themDongVaoBang removed as part of migration to model/view.

void WidgetQuanLyDocGia::xuLyNutThemDocGia() {
    HopThoaiThemDocGia *hopThoai = new HopThoaiThemDocGia(this);

    connect(hopThoai, &HopThoaiThemDocGia::docGiaDaThem, this, [this]() {
        // Dữ liệu đã được đồng bộ SQL bên trong backend
        taiLaiDanhSach();
        emit thongBaoCapNhat(LanguageManager::instance().getText("MSG_ADD_READER_SUCCESS"));
        emit duLieuThayDoi();
    });

    hopThoai->exec();
    delete hopThoai;
}

void WidgetQuanLyDocGia::xuLyNutSuaDocGia() {
    if (!docGiaDangChon) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_READER_FIRST"));
        return;
    }

    HopThoaiSuaDocGia *hopThoai = new HopThoaiSuaDocGia(docGiaDangChon->data.MaThe, this);

    connect(hopThoai, &HopThoaiSuaDocGia::docGiaDaSua, this, [this]() {
        // Dữ liệu đã được đồng bộ SQL bên trong backend
        taiLaiDanhSach();
        emit thongBaoCapNhat(LanguageManager::instance().getText("MSG_UPDATE_READER_SUCCESS"));
        emit duLieuThayDoi();
    });

    hopThoai->exec();
    delete hopThoai;
}
void WidgetQuanLyDocGia::xuLyNutXoaDocGia() {
    if (!docGiaDangChon) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_READER_FIRST"));
        return;
    }
    
    // Kiểm tra điều kiện xóa
    if (docGiaDangChon->data.soSachDangMuon > 0) {
        QMessageBox::warning(this, LanguageManager::instance().getText("MSG_CANNOT_DELETE"),
            LanguageManager::instance().getText("MSG_CANNOT_DELETE_HAS_BOOKS")
                .arg(docGiaDangChon->data.soSachDangMuon));
        return;
    }
    
    // Sử dụng ConfirmationDialog đẹp hơn
    QString chiTiet = QString(
        "Họ tên: %1 %2\n"
        "Mã thẻ: %3\n"
        "Trạng thái: %4\n"
        "Giới tính: %5\n\n"
        "Số sách đang mượn: %6"
    )
    .arg(QString::fromStdString(docGiaDangChon->data.Ho))
    .arg(QString::fromStdString(docGiaDangChon->data.Ten))
    .arg(docGiaDangChon->data.MaThe)
    .arg(docGiaDangChon->data.TrangThai == 1 ? "Đang hoạt động" : "Bị khóa")
    .arg(docGiaDangChon->data.Phai ? "Nữ" : "Nam")
    .arg(docGiaDangChon->data.soSachDangMuon);
    
    bool xacNhan = ConfirmationDialog::confirmDelete(
        this,
        QString("%1 %2 (Mã: %3)")
            .arg(QString::fromStdString(docGiaDangChon->data.Ho))
            .arg(QString::fromStdString(docGiaDangChon->data.Ten))
            .arg(docGiaDangChon->data.MaThe),
        LanguageManager::instance().getText("MSG_DELETE_CONFIRM_DETAILS")
            .arg(QString::fromStdString(docGiaDangChon->data.Ho))
            .arg(QString::fromStdString(docGiaDangChon->data.Ten))
            .arg(docGiaDangChon->data.MaThe)
            .arg(docGiaDangChon->data.TrangThai == 1 ? LanguageManager::instance().getText("LBL_STATUS_ACTIVE_LONG") : LanguageManager::instance().getText("STATUS_LOCKED"))
            .arg(docGiaDangChon->data.Phai ? LanguageManager::instance().getText("GENDER_FEMALE") : LanguageManager::instance().getText("GENDER_MALE"))
            .arg(docGiaDangChon->data.soSachDangMuon)
    );
    
    if (xacNhan) {
        // extern declaration removed, available via header
        
        // Xóa khỏi cây AVL
        int maThe = docGiaDangChon->data.MaThe;
        xoaDocGia(rootDocGia, maThe);
        EventBus::instance().docGiaRemoved(maThe); // REALTIME UPDATE
        
        // Dữ liệu đã được đồng bộ SQL bên trong backend
        emit duLieuThayDoi();
        
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"), LanguageManager::instance().getText("MSG_DELETE_SUCCESS"));
        docGiaDangChon = nullptr;
        maTheDangChon = -1;
        taiLaiDanhSach();
        emit thongBaoCapNhat(LanguageManager::instance().getText("MSG_DELETE_SUCCESS"));
    }
}

void WidgetQuanLyDocGia::xuLyNutKhoaThe() {
    if (!docGiaDangChon) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_READER_FIRST"));
        return;
    }
    
    if (docGiaDangChon->data.TrangThai == 0) {
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_INFO"), LanguageManager::instance().getText("MSG_ALREADY_LOCKED"));
        return;
    }
    
    QMessageBox::StandardButton xacNhan = QMessageBox::question(this, LanguageManager::instance().getText("TITLE_CONFIRM_LOCK"),
        LanguageManager::instance().getText("MSG_CONFIRM_LOCK")
            .arg(QString::fromStdString(docGiaDangChon->data.Ho))
            .arg(QString::fromStdString(docGiaDangChon->data.Ten))
            .arg(docGiaDangChon->data.MaThe),
        QMessageBox::Yes | QMessageBox::No);
    
    if (xacNhan == QMessageBox::Yes) {
        // extern declaration removed, available via header
        
        // Tạo struct mới với trạng thái khóa
        TheDocGia dataCapNhat = docGiaDangChon->data;
        dataCapNhat.TrangThai = 0;
        
        hieuChinhDocGia(rootDocGia, dataCapNhat);
        
        // Dữ liệu đã được đồng bộ SQL bên trong backend
        
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"), LanguageManager::instance().getText("MSG_LOCK_SUCCESS"));
        taiLaiDanhSach();
        emit thongBaoCapNhat(LanguageManager::instance().getText("MSG_LOCK_SUCCESS"));
    }
}

void WidgetQuanLyDocGia::xuLyNutMoKhoaThe() {
    if (!docGiaDangChon) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_READER_FIRST"));
        return;
    }
    
    if (docGiaDangChon->data.TrangThai == 1) {
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_INFO"), LanguageManager::instance().getText("MSG_ALREADY_ACTIVE"));
        return;
    }
    
    QMessageBox::StandardButton xacNhan = QMessageBox::question(this, LanguageManager::instance().getText("TITLE_CONFIRM_UNLOCK"),
        LanguageManager::instance().getText("MSG_CONFIRM_UNLOCK")
            .arg(QString::fromStdString(docGiaDangChon->data.Ho))
            .arg(QString::fromStdString(docGiaDangChon->data.Ten))
            .arg(docGiaDangChon->data.MaThe),
        QMessageBox::Yes | QMessageBox::No);
    
    if (xacNhan == QMessageBox::Yes) {
        // extern declaration removed, available via header
        
        // Tạo struct mới với trạng thái hoạt động
        TheDocGia dataCapNhat = docGiaDangChon->data;
        dataCapNhat.TrangThai = 1;
        
        hieuChinhDocGia(rootDocGia, dataCapNhat);
        
        // Dữ liệu đã được đồng bộ SQL bên trong backend
        
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"), LanguageManager::instance().getText("MSG_UNLOCK_SUCCESS"));
        taiLaiDanhSach();
        emit thongBaoCapNhat(LanguageManager::instance().getText("MSG_UNLOCK_SUCCESS"));
    }
}

void WidgetQuanLyDocGia::xuLyNutXemLichSu() {
    if (!docGiaDangChon) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_READER_FIRST"));
        return;
    }
    QMessageBox::information(this, LanguageManager::instance().getText("TITLE_HISTORY"), LanguageManager::instance().getText("MSG_FEATURE_DEVELOPING"));
}

void WidgetQuanLyDocGia::xuLyNutXemSachDangMuon() {
    if (!docGiaDangChon) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_SELECT_READER_FIRST"));
        return;
    }
    QMessageBox::information(this, LanguageManager::instance().getText("TITLE_BORROWED_BOOKS"), 
        LanguageManager::instance().getText("MSG_READER_BORROWING_INFO")
            .arg(QString::fromStdString(docGiaDangChon->data.Ho))
            .arg(QString::fromStdString(docGiaDangChon->data.Ten))
            .arg(docGiaDangChon->data.soSachDangMuon));
}

void WidgetQuanLyDocGia::xuLyTimKiem(const QString &tuKhoa) {
    if (!rootDocGia) {
        emit thongBaoCapNhat(LanguageManager::instance().getText("MSG_NO_READERS"));
        return;
    }

    QString tuKhoaTrimmed = tuKhoa.trimmed();

    // Nếu không có từ khóa, hiển thị tất cả (có áp dụng bộ lọc nếu có)
    if (tuKhoaTrimmed.isEmpty()) {
        xuLyLocKetHopToanBo();
        return;
    }

    std::vector<PTRDG> ketQua(1000);
    int soKetQua = 0;

    // Kiểm tra xem có phải mã thẻ không (số nguyên)
    bool laMaThe = false;
    int maThe = tuKhoaTrimmed.toInt(&laMaThe);

    if (laMaThe) {
        // Tìm theo mã thẻ
        PTRDG docGia = timDocGia(rootDocGia, maThe);
        if (docGia) {
            ketQua[0] = docGia;
            soKetQua = 1;
        }
    } else {
        // Tìm theo Họ + Tên (backend hỗ trợ tìm kiếm không phân biệt hoa/thường)
        soKetQua = TimDocGiaTheoHoTen(rootDocGia, tuKhoaTrimmed.toStdString(), ketQua.data(), (int)ketQua.size());
    }

    // Hiển thị kết quả với bộ lọc
    hienThiKetQuaTimKiem(ketQua.data(), soKetQua);

    emit thongBaoCapNhat(LanguageManager::instance().getText("MSG_SEARCH_RESULT").arg(soKetQua).arg(tuKhoaTrimmed));
}

void WidgetQuanLyDocGia::xuLyLocTrangThai(int chiSo) {
    Q_UNUSED(chiSo);
    xuLyLocKetHopToanBo();
}

void WidgetQuanLyDocGia::xuLyLocGioiTinh(int chiSo) {
    Q_UNUSED(chiSo);
    xuLyLocKetHopToanBo();
}

void WidgetQuanLyDocGia::hienThiDanhSachDocGia() {
    // Reuse combined filter loader to populate model-backed vector
    xuLyLocKetHopToanBo();
}

void WidgetQuanLyDocGia::taiLaiDanhSach() {
    hienThiDanhSachDocGia();
}

void WidgetQuanLyDocGia::capNhatBang() {
    hienThiDanhSachDocGia();
}

void WidgetQuanLyDocGia::xuLyChonDong(int dong, int) {
    if (dong < 0) return;
    if (!modelDocGia) return;

    QModelIndex maTheIdx = modelDocGia->index(dong, 1);
    int maThe = modelDocGia->data(maTheIdx, Qt::UserRole).toInt();

    // extern removed
    docGiaDangChon = timDocGia(rootDocGia, maThe);
    maTheDangChon = maThe;

    if (docGiaDangChon) {
        emit thongBaoCapNhat(LanguageManager::instance().getText("MSG_SELECTED_READER")
            .arg(QString::fromStdString(docGiaDangChon->data.Ho))
            .arg(QString::fromStdString(docGiaDangChon->data.Ten))
            .arg(maThe));
        // Enable actions that require a selection (use header buttons as primary controls)
        nutSuaDocGia->setEnabled(true);
        nutXoaDocGia->setEnabled(true);
        if (nutHeaderChiTiet) nutHeaderChiTiet->setEnabled(true);
        if (nutHeaderIn) nutHeaderIn->setEnabled(true);
        if (nutHeaderKhoa) nutHeaderKhoa->setEnabled(true);
        if (nutHeaderMoKhoa) nutHeaderMoKhoa->setEnabled(true);
        if (nutHeaderLichSu) nutHeaderLichSu->setEnabled(true);
        return;
    }
    // If we reach here, clear selection
    docGiaDangChon = nullptr;
    maTheDangChon = -1;
    nutSuaDocGia->setEnabled(false);
    nutXoaDocGia->setEnabled(false);
    if (nutHeaderChiTiet) nutHeaderChiTiet->setEnabled(false);
    if (nutHeaderIn) nutHeaderIn->setEnabled(false);
    if (nutHeaderKhoa) nutHeaderKhoa->setEnabled(false);
    if (nutHeaderMoKhoa) nutHeaderMoKhoa->setEnabled(false);
    if (nutHeaderLichSu) nutHeaderLichSu->setEnabled(false);
}

void WidgetQuanLyDocGia::xuLyLocKetHopToanBo() {
    // Build filtered list and notify model (model-backed view)
    // extern removed
    danhSachDocGia.clear();
    if (!rootDocGia) {
        emit thongBaoCapNhat(LanguageManager::instance().getText("MSG_NO_READERS"));
        if (modelDocGia) { if (auto m = dynamic_cast<DocGiaTableModel*>(modelDocGia)) m->refresh(); }
        return;
    }

    std::vector<PTRDG> tmp;
    int dem = 0;
#if defined(__has_include) && __has_include("utils/DBBatcher.h")
    std::vector<PTRDG> _tmp = DBBatcher::snapshotDocGia();
    tmp = _tmp;
    dem = (int)tmp.size();
#else
    DuyetCayRaVector(rootDocGia, tmp);
    dem = (int)tmp.size();
#endif

    int trangThaiLoc = hopChonTrangThai->currentIndex();
    int gioiTinhLoc = hopChonGioiTinh->currentIndex();
    for (int i = 0; i < dem; ++i) {
        PTRDG p = tmp[i];
        if (!p) continue;
        bool hienThi = true;
        if (trangThaiLoc == 1 && p->data.TrangThai != 1) hienThi = false;
        if (trangThaiLoc == 2 && p->data.TrangThai != 0) hienThi = false;
        if (gioiTinhLoc == 1 && p->data.Phai != false) hienThi = false;
        if (gioiTinhLoc == 2 && p->data.Phai != true) hienThi = false;
        if (hienThi) danhSachDocGia.push_back(p);
    }

    if (modelDocGia) { if (auto m = dynamic_cast<DocGiaTableModel*>(modelDocGia)) m->refresh(); }
    emit thongBaoCapNhat(LanguageManager::instance().getText("MSG_LOADED_READERS").arg((int)danhSachDocGia.size()));
}

void WidgetQuanLyDocGia::hienThiKetQuaTimKiem(PTRDG ketQua[], int soKetQua) {
    // Build filtered search result list and refresh model
    danhSachDocGia.clear();
    int trangThaiLoc = hopChonTrangThai->currentIndex();
    int gioiTinhLoc = hopChonGioiTinh->currentIndex();
    for (int i = 0; i < soKetQua; ++i) {
        PTRDG p = ketQua[i];
        if (!p) continue;
        bool hienThi = true;
        if (trangThaiLoc == 1 && p->data.TrangThai != 1) hienThi = false;
        if (trangThaiLoc == 2 && p->data.TrangThai != 0) hienThi = false;
        if (gioiTinhLoc == 1 && p->data.Phai != false) hienThi = false;
        if (gioiTinhLoc == 2 && p->data.Phai != true) hienThi = false;
        if (hienThi) danhSachDocGia.push_back(p);
    }
    if (modelDocGia) { if (auto m = dynamic_cast<DocGiaTableModel*>(modelDocGia)) m->refresh(); }
}
void WidgetQuanLyDocGia::xuLyNhapDuLieuTu() {
    QString file = QFileDialog::getOpenFileName(this, LanguageManager::instance().getText("TITLE_SELECT_EXCEL"), QDir::currentPath(), LanguageManager::instance().getText("FILTER_EXCEL_FILES"));
    if (file.isEmpty()) return;

    ImportManager importer(this);


    QProgressDialog progress(LanguageManager::instance().getText("MSG_PROCESSING_EXCEL"), QString(), 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.show();
    QCoreApplication::processEvents();

    auto result = importer.runPreview(file);
    progress.close();

    if (!result.success) {
        QMessageBox::critical(this, tr("Lỗi"), tr("Không thể đọc file Excel:\n") + result.message);
        return;
    }

    // Show Preview Dialog
    ManHinhPreviewImport dlg(result, file, &importer, this);
    if (dlg.exec() == QDialog::Accepted) {
        // Success - RELOAD FROM DB to RAM
        extern PTRDG rootDocGia;
        extern PTRDS dsDauSach[];
        extern int soLuongDauSach;
        NapDocGiaTuDB_Fixed(rootDocGia, dsDauSach, soLuongDauSach);
        
        taiLaiDanhSach();
        // Force refresh table
        hienThiDanhSachDocGia();
    }
}
void WidgetQuanLyDocGia::hienThiThongTinChiTiet(PTRDG) {}

QString WidgetQuanLyDocGia::layTenTrangThai(int trangThai) const {
    return (trangThai == 1) ? "Hoạt động" : "Bị khóa";
}

QString WidgetQuanLyDocGia::layTenGioiTinh(bool phai) const {
    return phai ? "Nữ" : "Nam";
}

QColor WidgetQuanLyDocGia::layMauTrangThai(int trangThai) const {
    return (trangThai == 1) ? QColor("#1BC5BD") : QColor("#F64E60");
}

void WidgetQuanLyDocGia::capNhatNgonNgu() {
    // 1. Buttons
    if(nutThemDocGia) nutThemDocGia->setText(" " + LanguageManager::instance().getText("BTN_ADD_READER"));
    if(nutNhapFile) nutNhapFile->setText(" Import Excel"); 
    if(nutHeaderIn) nutHeaderIn->setText(" " + LanguageManager::instance().getText("BTN_PRINT"));
    if(nutSuaDocGia) nutSuaDocGia->setText(" " + LanguageManager::instance().getText("BTN_EDIT")); 
    if(nutHeaderChiTiet) nutHeaderChiTiet->setText(" " + LanguageManager::instance().getText("BTN_DETAILS"));
    if(nutHeaderKhoa) nutHeaderKhoa->setText(" Khóa Thẻ"); 
    if(nutHeaderMoKhoa) nutHeaderMoKhoa->setText(" " + LanguageManager::instance().getText("BTN_UNLOCK"));
    if(nutXoaDocGia) nutXoaDocGia->setText(" " + LanguageManager::instance().getText("BTN_DELETE")); 
    
    // 2. Search
    if(oNhapTimKiem) oNhapTimKiem->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_SEARCH_READER"));
    if(nutTimKiem) nutTimKiem->setText(LanguageManager::instance().getText("BTN_SEARCH"));
    if(nutLamMoi) nutLamMoi->setText(LanguageManager::instance().getText("BTN_REFRESH")); 
    
    // 4. Update Model Headers
    if (modelDocGia) {
        if (auto m = dynamic_cast<DocGiaTableModel*>(modelDocGia)) m->refresh(); 
        QEvent e(QEvent::LanguageChange);
        if (bangDocGia) QApplication::sendEvent(bangDocGia->horizontalHeader(), &e);
    }
}
