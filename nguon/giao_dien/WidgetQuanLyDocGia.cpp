#include "giao_dien/WidgetQuanLyDocGia.h"
#include "giao_dien/hop_thoai/HopThoaiThemDocGia.h"
#include "giao_dien/hop_thoai/HopThoaiSuaDocGia.h"
#include "models/DocGia.h"
#include "utils/ConfirmationDialog.h"
#include "utils/AnimationHelper.h"
#include <QVBoxLayout>
#if defined(__has_include)
#  if __has_include("utils/DBBatcher.h")
#    include "utils/DBBatcher.h"
#  endif
#endif
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include <QTableView>
#include <QAbstractTableModel>

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
            case 4: return dg.Phai ? QStringLiteral("Nữ") : QStringLiteral("Nam");
            case 5: return dg.TrangThai == 1 ? QStringLiteral("Hoạt động") : QStringLiteral("Bị khóa");
            case 6: return QString("%1 / 3").arg(dg.soSachDangMuon);
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
            case 1: return QStringLiteral("Mã Thẻ");
            case 2: return QStringLiteral("Họ");
            case 3: return QStringLiteral("Tên");
            case 4: return QStringLiteral("Giới Tính");
            case 5: return QStringLiteral("Trạng Thái");
            case 6: return QStringLiteral("Sách Đang Mượn");
            case 7: return QStringLiteral("Quá Hạn");
            case 8: return QStringLiteral("Tổng Mượn");
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
}

WidgetQuanLyDocGia::~WidgetQuanLyDocGia() {}

void WidgetQuanLyDocGia::thieLapGiaoDien() {
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    
    boTriChinh = new QVBoxLayout(this);
    int margin = ResponsiveHelper::getScaledMargin(30, screenSize);
    int spacing = ResponsiveHelper::getScaledSpacing(20, screenSize);
    boTriChinh->setContentsMargins(margin, margin, margin, margin);
    boTriChinh->setSpacing(spacing);
    
    // Tiêu đề với icon
    QHBoxLayout *layoutTieuDe = new QHBoxLayout();
    QPushButton *iconTieuDe = new QPushButton();
    iconTieuDe->setIcon(QIcon(":/icons/user.svg"));
    iconTieuDe->setIconSize(QSize(28, 28));
    iconTieuDe->setFlat(true);
    iconTieuDe->setStyleSheet("border: none; background: transparent;");
    int titleSize = ResponsiveHelper::getScaledFontSize(28, screenSize);
    QLabel *tieuDe = new QLabel("QUẢN LÝ ĐỘC GIẢ");
    tieuDe->setStyleSheet(QString("font-size: %1px; font-weight: bold; color: #FFFFFF;").arg(titleSize));
    layoutTieuDe->addWidget(iconTieuDe);
    layoutTieuDe->addWidget(tieuDe);
    layoutTieuDe->addStretch();
    boTriChinh->addLayout(layoutTieuDe);
    
    // Header action bar (match style & behavior of WidgetQuanLySach)
    {
        QWidget *headerBar = new QWidget();
        headerBar->setStyleSheet("QWidget { background: transparent; }");
        QHBoxLayout *hLayout = new QHBoxLayout(headerBar);
        hLayout->setContentsMargins(0,0,0,0);
        hLayout->setSpacing(12);

        auto createButton = [](const QString &icon, const QString &text, const QString &color) -> QPushButton* {
            QPushButton *btn = new QPushButton(icon + " " + text);
            btn->setFixedHeight(30);
            btn->setMinimumWidth(90);
            // Map legacy neutral gray to navy surface
            QString base = color;
            if (color.compare("#2D2D30", Qt::CaseInsensitive) == 0) base = "#08283d";
            QString hoverColor;
            QString pressedColor;
            if (base.compare("#0078D4", Qt::CaseInsensitive) == 0) {
                hoverColor = "#106EBE"; pressedColor = "#005A9E";
            } else if (base.compare("#C42B1C", Qt::CaseInsensitive) == 0) {
                hoverColor = "#A32218"; pressedColor = "#8A1A12";
            } else {
                hoverColor = "#08314f"; pressedColor = "#041726";
            }
            btn->setStyleSheet(QString(
                "QPushButton { background-color: %1; color: white; border: none; border-radius: 4px; font-size: 12px; font-weight: 500; padding: 0 10px; }"
                "QPushButton:hover { background-color: %2; }"
                "QPushButton:pressed { background-color: %3; }"
            ).arg(base, hoverColor, pressedColor));
            return btn;
        };

        // assign header buttons to member variables so they replace bottom duplicates
        nutThemDocGia = createButton("+", "Thêm Độc Giả", "#0078D4");
        nutNhapFile = createButton("⇩", "Nhập Excel", "#2D2D30");
        nutNhapFile->setStyleSheet(
            "QPushButton { background-color: #08283d; color: #E0E0E0; border: 1px solid #08314f; border-radius: 4px; font-size: 13px; padding: 0 16px; }"
            "QPushButton:hover { background-color: #08314f; border: 1px solid #1076C9; }"
        );
        // header In and Chi Tiết stored as members so we can enable/disable them
        nutHeaderIn = createButton("⎙", "In", "#2D2D30");
        nutHeaderIn->setStyleSheet(nutNhapFile->styleSheet());
        nutSuaDocGia = createButton("✏", "Sửa", "#2D2D30");
        nutSuaDocGia->setStyleSheet(nutNhapFile->styleSheet());
        nutHeaderChiTiet = createButton("ⓘ", "Chi Tiết", "#2D2D30");
        nutHeaderChiTiet->setStyleSheet(nutNhapFile->styleSheet());
        // Add header lock/unlock/history buttons (these will replace bottom duplicates)
        nutHeaderKhoa = createButton("🔒", "Khóa Thẻ", "#2D2D30");
        nutHeaderKhoa->setStyleSheet(nutNhapFile->styleSheet());
        nutHeaderMoKhoa = createButton("🔓", "Mở Khóa", "#2D2D30");
        nutHeaderMoKhoa->setStyleSheet(nutNhapFile->styleSheet());
        nutHeaderLichSu = createButton("⎘", "Lịch Sử", "#2D2D30");
        nutHeaderLichSu->setStyleSheet(nutNhapFile->styleSheet());
        nutXoaDocGia = createButton("✖", "Xóa", "#C42B1C");

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
        // Header: In -> try to delegate to main window (CuaSoChinh::hienThiIn)
        connect(nutHeaderIn, &QPushButton::clicked, this, [this]() {
            QWidget *w = this->window();
            if (!w) { QMessageBox::warning(this, "In", "Không tìm thấy cửa sổ chính để thực hiện in."); return; }
            // Try to call hienThiIn on parent main window via meta-object
            if (!QMetaObject::invokeMethod(w, "hienThiIn")) {
                QMessageBox::information(this, "In", "Chức năng in chưa được hỗ trợ ở đây.");
            }
        });
        // Header: Chi Tiết -> open edit dialog same as double-click
        connect(nutHeaderChiTiet, &QPushButton::clicked, this, [this]() {
            if (!docGiaDangChon) { QMessageBox::warning(this, "Chi Tiết", "Vui lòng chọn một độc giả!"); return; }
            HopThoaiSuaDocGia *dialog = new HopThoaiSuaDocGia(docGiaDangChon->data.MaThe, this);
            dialog->exec();
            delete dialog;
            taiLaiDanhSach();
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
    oNhapTimKiem->setFixedHeight(32);
    oNhapTimKiem->setStyleSheet(
        "QLineEdit {"
        "    background-color: #08283d;"
        "    color: #E0E0E0;"
        "    border: 1px solid #08314f;"
        "    border-radius: 5px;"
        "    padding: 0 12px;"
        "    font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "    border: 1px solid #1076C9;"
        "    background-color: #071233;"
        "}"
    );

    hopChonTrangThai = new QComboBox();
    hopChonTrangThai->addItems({"Tất cả", "Hoạt động", "Bị khóa"});
    hopChonTrangThai->setFixedHeight(32);
    hopChonTrangThai->setStyleSheet(
        "QComboBox { background-color: #08283d; color: #E0E0E0; border: 1px solid #08314f; border-radius: 5px; padding: 0 12px; font-size: 13px; }"
        "QComboBox:hover { border: 1px solid #1076C9; }"
        "QComboBox::drop-down { border: none; width: 20px; }"
    );

    hopChonGioiTinh = new QComboBox();
    hopChonGioiTinh->addItems({"Tất cả giới tính", "Nam", "Nữ"});
    hopChonGioiTinh->setFixedHeight(32);
    hopChonGioiTinh->setStyleSheet(hopChonTrangThai->styleSheet());

    nutTimKiem = new QPushButton("Tìm");
    nutTimKiem->setFixedHeight(32);
    nutTimKiem->setFixedWidth(72);
    nutTimKiem->setStyleSheet(
        "QPushButton {"
        "    background-color: #0078D4;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    font-size: 13px;"
        "    font-weight: 500;"
        "}"
        "QPushButton:hover { background-color: #106EBE; }"
        "QPushButton:pressed { background-color: #005A9E; }"
    );

    nutLamMoi = new QPushButton("Làm mới");
    nutLamMoi->setFixedHeight(32);
    nutLamMoi->setFixedWidth(86);
    nutLamMoi->setStyleSheet(
        "QPushButton { background-color: #08283d; color: #E0E0E0; border: 1px solid #08314f; border-radius: 4px; font-size: 13px; }"
        "QPushButton:hover { background-color: #08314f; border: 1px solid #1076C9; }"
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
    bangDocGia->verticalHeader()->setDefaultSectionSize(48);

    boTriChinh->addWidget(bangDocGia, 1);

    connect(bangDocGia, &QTableView::clicked, this, [this](const QModelIndex &idx){ this->xuLyChonDong(idx.row(), idx.column()); });

    // Double-click -> open edit dialog using MaThe from model's UserRole
    connect(bangDocGia, &QTableView::doubleClicked, this, [this](const QModelIndex &idx){
        int row = idx.row();
        if (row < 0) return;
        QModelIndex maTheIdx = modelDocGia->index(row, 1);
        int maThe = modelDocGia->data(maTheIdx, Qt::UserRole).toInt();
        extern PTRDG rootDocGia;
        PTRDG docGia = timDocGia(rootDocGia, maThe);
        if (docGia) {
            HopThoaiSuaDocGia *dialog = new HopThoaiSuaDocGia(maThe, this);
            dialog->exec();
            delete dialog;
            taiLaiDanhSach();
        }
    });
}

void WidgetQuanLyDocGia::thieLapNutChucNang() {
    nhomChucNang = new QGroupBox("  Chức Năng");
    nhomChucNang->setStyleSheet(
        "QGroupBox { background-color: transparent; border: none; padding: 6px; }"
    );

    QHBoxLayout *boTri = new QHBoxLayout(nhomChucNang);

    auto createButton = [](const QString &icon, const QString &text, const QString &color) -> QPushButton* {
        QPushButton *btn = new QPushButton(icon + " " + text);
        btn->setFixedHeight(30);
        btn->setMinimumWidth(90);
        QString hoverColor = (color == "#0078D4") ? "#106EBE" : "#3E3E42";
        QString pressedColor = (color == "#0078D4") ? "#005A9E" : "#2D2D30";
        btn->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: white; border: none; border-radius: 4px; font-size: 12px; font-weight: 500; padding: 0 10px; }"
            "QPushButton:hover { background-color: %2; }"
            "QPushButton:pressed { background-color: %3; }"
        ).arg(color, hoverColor, pressedColor));
        return btn;
    };

    nutThemDocGia = createButton("+", "Thêm Độc Giả", "#0078D4");
    nutSuaDocGia = createButton("✏", "Sửa", "#2D2D30");
    nutXoaDocGia = createButton("✖", "Xóa", "#C42B1C");
    nutKhoaThe = createButton("🔒", "Khóa Thẻ", "#2D2D30");
    nutMoKhoaThe = createButton("🔓", "Mở Khóa", "#2D2D30");
    nutXemLichSu = createButton("ⓘ", "Lịch Sử", "#2D2D30");

    boTri->addWidget(nutThemDocGia);
    boTri->addWidget(nutSuaDocGia);
    boTri->addWidget(nutXoaDocGia);
    boTri->addWidget(nutKhoaThe);
    boTri->addWidget(nutMoKhoaThe);
    boTri->addWidget(nutXemLichSu);
    boTri->addStretch();

    boTriChinh->addWidget(nhomChucNang);

    // Initially disable actions that require a selected row
    nutSuaDocGia->setEnabled(false);
    nutXoaDocGia->setEnabled(false);
    // header buttons are the primary action controls
    if (nutHeaderKhoa) nutHeaderKhoa->setEnabled(false);
    if (nutHeaderMoKhoa) nutHeaderMoKhoa->setEnabled(false);
    if (nutHeaderLichSu) nutHeaderLichSu->setEnabled(false);
    if (nutHeaderChiTiet) nutHeaderChiTiet->setEnabled(false);
    if (nutHeaderIn) nutHeaderIn->setEnabled(false);

    connect(nutThemDocGia, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNutThemDocGia);
    connect(nutSuaDocGia, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNutSuaDocGia);
    connect(nutXoaDocGia, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNutXoaDocGia);
    connect(nutKhoaThe, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNutKhoaThe);
    connect(nutMoKhoaThe, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNutMoKhoaThe);
    connect(nutXemLichSu, &QPushButton::clicked, this, &WidgetQuanLyDocGia::xuLyNutXemLichSu);
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
void WidgetQuanLyDocGia::xuLyNhapDuLieuTu() {}
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
