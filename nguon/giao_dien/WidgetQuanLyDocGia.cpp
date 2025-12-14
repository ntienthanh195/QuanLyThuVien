#include "giao_dien/WidgetQuanLyDocGia.h"
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
            case 4: return dg.Phai ? LanguageManager::instance().getText("GENDER_FEMALE") : LanguageManager::instance().getText("GENDER_MALE");
            case 5: return dg.TrangThai == 1 ? LanguageManager::instance().getText("STATUS_ACTIVE") : LanguageManager::instance().getText("STATUS_LOCKED");
            case 6: {
                int maxSach = SettingsManager::instance().getInt("SO_SACH_MUON_TOI_DA", 3);
                return QString("%1 / %2").arg(dg.soSachDangMuon).arg(maxSach);
            }
            case 7: {
                int soNgay = TinhSoNgayQuaHanLonNhat(dgptr);
                return soNgay > 0 ? QString("%1 ngày").arg(soNgay) : QStringLiteral("—");
            }
            case 8: {
                int tong = 0; MUONTRA mt = dg.dsmt; while (mt) { tong++; mt = mt->next; }
                return QString::number(tong);
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
            case 0: return QStringLiteral("STT");
            case 1: return LanguageManager::instance().getText("TABLE_HEADER_CODE");
            case 2: return LanguageManager::instance().getText("TABLE_HEADER_LASTNAME");
            case 3: return LanguageManager::instance().getText("TABLE_HEADER_FIRSTNAME");
            case 4: return LanguageManager::instance().getText("TABLE_HEADER_GENDER");
            case 5: return LanguageManager::instance().getText("TABLE_HEADER_STATUS");
            case 6: return QString("Sách Đang Mượn"); // TODO: Add key
            case 7: return QString("Quá Hạn"); // TODO: Add key
            case 8: return QString("Tổng Mượn"); // TODO: Add key
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
        nutThemDocGia = createButton(":/icons/plus.svg", "Thêm Độc Giả", "#0078D4");
        nutNhapFile = createButton(":/icons/import.svg", "Nhập Excel", "#2D2D30");
        
        // header In and Chi Tiết
        nutHeaderIn = createButton(":/icons/print.svg", "In", "#2D2D30");
        nutSuaDocGia = createButton(":/icons/edit.svg", "Sửa", "#2D2D30");
        nutHeaderChiTiet = createButton(":/icons/info.svg", "Chi Tiết", "#2D2D30");
        
        // Add header lock/unlock/history buttons
        nutHeaderKhoa = createButton(":/icons/lock.svg", "Khóa Thẻ", "#2D2D30");
        nutHeaderMoKhoa = createButton(":/icons/unlock.svg", "Mở Khóa", "#2D2D30");
        nutHeaderLichSu = createButton(":/icons/history.svg", "Lịch Sử", "#2D2D30");
        
        nutXoaDocGia = createButton(":/icons/delete.svg", "Xóa", "#C42B1C");

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
            if (!docGiaDangChon) { QMessageBox::warning(this, "Chi Tiết", "Vui lòng chọn một độc giả!"); return; }
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
    oNhapTimKiem->setPlaceholderText("Nhập mã thẻ, họ, tên...");
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
    hopChonTrangThai->addItems({"Tất cả", "Hoạt động", "Bị khóa"});
    hopChonTrangThai->setFixedHeight(36);
    hopChonTrangThai->setStyleSheet(
        "QComboBox { background-color: #1a1d23; color: #E0E0E0; border: 1px solid #151820; border-radius: 5px; padding: 0 12px; font-size: 13px; }"
        "QComboBox:hover { border: 1px solid #4A90E2; }"
        "QComboBox::drop-down { border: none; width: 20px; }"
    );

    hopChonGioiTinh = new QComboBox();
    hopChonGioiTinh->addItems({"Tất cả giới tính", "Nam", "Nữ"});
    hopChonGioiTinh->setFixedHeight(36);
    hopChonGioiTinh->setStyleSheet(hopChonTrangThai->styleSheet());

    nutTimKiem = new QPushButton("Tìm");
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

    nutLamMoi = new QPushButton("Làm mới");
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
    bangDocGia->setColumnWidth(4, 90);   // Giới Tính
    bangDocGia->setColumnWidth(5, 120);  // Trạng Thái
    bangDocGia->setColumnWidth(6, 130);  // Sách Đang Mượn
    bangDocGia->setColumnWidth(7, 100);  // Quá Hạn
    bangDocGia->horizontalHeader()->setStretchLastSection(true);
    // Row height similar to screenshot
    bangDocGia->verticalHeader()->setDefaultSectionSize(45);

    boTriChinh->addWidget(bangDocGia, 1);
    
    // Hint label
    QLabel *hintLabel = new QLabel("Mẹo: Nhấp đúp vào độc giả để xem chi tiết & in thẻ");
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

        QAction *actChiTiet = contextMenu.addAction(QIcon(":/icons/info.svg"), "Xem Chi Tiết");
        QAction *actSua = contextMenu.addAction(QIcon(":/icons/edit.svg"), "Sửa Độc Giả");
        contextMenu.addSeparator();
        QAction *actInThe = contextMenu.addAction(QIcon(":/icons/card.svg"), "In Thẻ TV");
        contextMenu.addSeparator();
        // Check lock status to show correct action
        if (docGiaDangChon && docGiaDangChon->data.TrangThai == 1) {
            QAction *actKhoa = contextMenu.addAction(QIcon(":/icons/lock.svg"), "Khóa Thẻ");
            connect(actKhoa, &QAction::triggered, this, &WidgetQuanLyDocGia::xuLyNutKhoaThe);
        } else {
            QAction *actMo = contextMenu.addAction(QIcon(":/icons/unlock.svg"), "Mở Khóa");
            connect(actMo, &QAction::triggered, this, &WidgetQuanLyDocGia::xuLyNutMoKhoaThe);
        }
        
        QAction *actXoa = contextMenu.addAction(QIcon(":/icons/delete.svg"), "Xóa Độc Giả");

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
        extern PTRDG rootDocGia;
        saveDocGia(rootDocGia);
        taiLaiDanhSach();
        emit thongBaoCapNhat("Đã thêm độc giả mới thành công!");
        emit duLieuThayDoi();
    });

    hopThoai->exec();
    delete hopThoai;
}

void WidgetQuanLyDocGia::xuLyNutSuaDocGia() {
    if (!docGiaDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một độc giả!");
        return;
    }

    HopThoaiSuaDocGia *hopThoai = new HopThoaiSuaDocGia(docGiaDangChon->data.MaThe, this);

    connect(hopThoai, &HopThoaiSuaDocGia::docGiaDaSua, this, [this]() {
        extern PTRDG rootDocGia;
        saveDocGia(rootDocGia);
        taiLaiDanhSach();
        emit thongBaoCapNhat("Đã cập nhật thông tin độc giả!");
        emit duLieuThayDoi();
    });

    hopThoai->exec();
    delete hopThoai;
}
void WidgetQuanLyDocGia::xuLyNutXoaDocGia() {
    if (!docGiaDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một độc giả!");
        return;
    }
    
    // Kiểm tra điều kiện xóa
    if (docGiaDangChon->data.soSachDangMuon > 0) {
        QMessageBox::warning(this, "Không Thể Xóa",
            QString("Không thể xóa độc giả này!\n\nĐộc giả đang mượn %1 cuốn sách.\nVui lòng trả hết sách trước khi xóa.")
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
        chiTiet
    );
    
    if (xacNhan) {
        extern PTRDG rootDocGia;
        
        // Xóa khỏi cây AVL
        int maThe = docGiaDangChon->data.MaThe;
        xoaDocGia(rootDocGia, maThe);
        EventBus::instance().docGiaRemoved(maThe); // REALTIME UPDATE
        
        if (rootDocGia) {
            saveDocGia(rootDocGia); // Tự động lưu file
            emit duLieuThayDoi(); // Realtime update Dashboard
        } else {
            // Nếu rootDocGia rỗng, log để dễ debug
            std::cerr << "Canh bao: rootDocGia is null, skip saveDocGia." << std::endl;
        }
        
        QMessageBox::information(this, "Thành Công", "Đã xóa độc giả thành công!");
        docGiaDangChon = nullptr;
        maTheDangChon = -1;
        taiLaiDanhSach();
        emit thongBaoCapNhat("Đã xóa độc giả!");
    }
}

void WidgetQuanLyDocGia::xuLyNutKhoaThe() {
    if (!docGiaDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một độc giả!");
        return;
    }
    
    if (docGiaDangChon->data.TrangThai == 0) {
        QMessageBox::information(this, "Thông Báo", "Thẻ độc giả này đã bị khóa rồi!");
        return;
    }
    
    QMessageBox::StandardButton xacNhan = QMessageBox::question(this, "Xác Nhận Khóa Thẻ",
        QString("Bạn có chắc muốn khóa thẻ độc giả:\n%1 %2 (Mã: %3)?\n\nĐộc giả sẽ không thể mượn sách sau khi bị khóa.")
            .arg(QString::fromStdString(docGiaDangChon->data.Ho))
            .arg(QString::fromStdString(docGiaDangChon->data.Ten))
            .arg(docGiaDangChon->data.MaThe),
        QMessageBox::Yes | QMessageBox::No);
    
    if (xacNhan == QMessageBox::Yes) {
        extern PTRDG rootDocGia;
        
        // Tạo struct mới với trạng thái khóa
        TheDocGia dataCapNhat = docGiaDangChon->data;
        dataCapNhat.TrangThai = 0;
        
        hieuChinhDocGia(rootDocGia, dataCapNhat);
        
        saveDocGia(rootDocGia); // Tự động lưu file
        
        QMessageBox::information(this, "Thành Công", "Đã khóa thẻ độc giả!");
        taiLaiDanhSach();
        emit thongBaoCapNhat("Đã khóa thẻ độc giả!");
    }
}

void WidgetQuanLyDocGia::xuLyNutMoKhoaThe() {
    if (!docGiaDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một độc giả!");
        return;
    }
    
    if (docGiaDangChon->data.TrangThai == 1) {
        QMessageBox::information(this, "Thông Báo", "Thẻ độc giả này đang hoạt động bình thường!");
        return;
    }
    
    QMessageBox::StandardButton xacNhan = QMessageBox::question(this, "Xác Nhận Mở Khóa",
        QString("Bạn có chắc muốn mở khóa thẻ độc giả:\n%1 %2 (Mã: %3)?")
            .arg(QString::fromStdString(docGiaDangChon->data.Ho))
            .arg(QString::fromStdString(docGiaDangChon->data.Ten))
            .arg(docGiaDangChon->data.MaThe),
        QMessageBox::Yes | QMessageBox::No);
    
    if (xacNhan == QMessageBox::Yes) {
        extern PTRDG rootDocGia;
        
        // Tạo struct mới với trạng thái hoạt động
        TheDocGia dataCapNhat = docGiaDangChon->data;
        dataCapNhat.TrangThai = 1;
        
        hieuChinhDocGia(rootDocGia, dataCapNhat);
        
        saveDocGia(rootDocGia); // Tự động lưu file
        
        QMessageBox::information(this, "Thành Công", "Đã mở khóa thẻ độc giả!");
        taiLaiDanhSach();
        emit thongBaoCapNhat("Đã mở khóa thẻ độc giả!");
    }
}

void WidgetQuanLyDocGia::xuLyNutXemLichSu() {
    if (!docGiaDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một độc giả!");
        return;
    }
    QMessageBox::information(this, "Lịch Sử", "Chức năng xem lịch sử đang được phát triển...");
}

void WidgetQuanLyDocGia::xuLyNutXemSachDangMuon() {
    if (!docGiaDangChon) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn một độc giả!");
        return;
    }
    QMessageBox::information(this, "Sách Đang Mượn", 
        QString("Độc giả %1 %2 đang mượn %3 cuốn sách.\n\nChức năng xem chi tiết đang được phát triển...")
            .arg(QString::fromStdString(docGiaDangChon->data.Ho))
            .arg(QString::fromStdString(docGiaDangChon->data.Ten))
            .arg(docGiaDangChon->data.soSachDangMuon));
}

void WidgetQuanLyDocGia::xuLyTimKiem(const QString &tuKhoa) {
    extern PTRDG rootDocGia;
    if (!rootDocGia) {
        emit thongBaoCapNhat("Chưa có độc giả nào");
        return;
    }

    QString tuKhoaTrimmed = tuKhoa.trimmed();

    // Nếu không có từ khóa, hiển thị tất cả (có áp dụng bộ lọc nếu có)
    if (tuKhoaTrimmed.isEmpty()) {
        xuLyLocKetHopToanBo();
        return;
    }

    PTRDG ketQua[1000];
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
        soKetQua = TimDocGiaTheoHoTen(rootDocGia, tuKhoaTrimmed.toStdString(), ketQua, 1000);
    }

    // Hiển thị kết quả với bộ lọc
    hienThiKetQuaTimKiem(ketQua, soKetQua);

    emit thongBaoCapNhat(QString("Tìm thấy %1 kết quả cho '%2'").arg(soKetQua).arg(tuKhoaTrimmed));
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

    extern PTRDG rootDocGia;
    docGiaDangChon = timDocGia(rootDocGia, maThe);
    maTheDangChon = maThe;

    if (docGiaDangChon) {
        emit thongBaoCapNhat(QString("Đã chọn: %1 %2 (Mã: %3)")
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
    extern PTRDG rootDocGia;
    danhSachDocGia.clear();
    if (!rootDocGia) {
        emit thongBaoCapNhat("Chưa có độc giả nào");
        if (modelDocGia) { if (auto m = dynamic_cast<DocGiaTableModel*>(modelDocGia)) m->refresh(); }
        return;
    }

    PTRDG tmp[1000];
    int dem = 0;
#if defined(__has_include) && __has_include("utils/DBBatcher.h")
    std::vector<PTRDG> _tmp = DBBatcher::snapshotDocGia();
    dem = static_cast<int>(std::min<size_t>(_tmp.size(), 1000));
    for (int i = 0; i < dem; ++i) tmp[i] = _tmp[i];
#else
    DuyetCayRaMang(rootDocGia, tmp, dem);
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
    emit thongBaoCapNhat(QString("Đã tải %1 độc giả").arg((int)danhSachDocGia.size()));
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
    QString file = QFileDialog::getOpenFileName(this, tr("Chọn file Excel để nhập"), QDir::currentPath(), tr("Excel Files (*.xlsx);;All Files (*)"));
    if (file.isEmpty()) return;

    ImportManager importer(this);
    if (!importer.ensureDependencies(this)) return;

    QProgressDialog progress(tr("Đang xử lý tập tin Excel..."), QString(), 0, 0, this);
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
        // Success
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
