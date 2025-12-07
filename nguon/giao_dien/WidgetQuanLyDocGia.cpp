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
    
    thieLapTimKiem();
    thieLapBangDocGia();
    thieLapNutChucNang();
    
    qDebug() << "WidgetQuanLyDocGia: Responsive layout applied";
}

void WidgetQuanLyDocGia::thieLapTimKiem() {
    nhomTimKiem = new QGroupBox("  Tìm Kiếm & Lọc");
    nhomTimKiem->setStyleSheet(
        "QGroupBox { background-color: #1E1E2D; border: 1px solid #2B2B40; "
        "border-radius: 12px; padding: 15px; font-size: 14px; font-weight: bold; color: #FFFFFF; }"
        "QLineEdit { background-color: #2B2B40; border: 1px solid #3F3F55; border-radius: 8px; "
        "padding: 8px 12px; color: #FFFFFF; font-size: 13px; }"
        "QLineEdit:focus { border: 2px solid #3699FF; background-color: #1E1E2D; }"
        "QComboBox { background-color: #2B2B40; border: 1px solid #3F3F55; border-radius: 8px; "
        "padding: 8px 12px; color: #FFFFFF; font-size: 13px; }"
        "QComboBox:focus { border: 2px solid #3699FF; }"
        "QComboBox::drop-down { border: none; width: 30px; }"
        "QComboBox::down-arrow { image: url(:/icons/filter.svg); width: 12px; height: 12px; }"
    );
    
    QHBoxLayout *boTri = new QHBoxLayout(nhomTimKiem);
    
    oNhapTimKiem = new QLineEdit();
    oNhapTimKiem->setPlaceholderText("Nhập mã thẻ, họ, tên...");
    oNhapTimKiem->setMinimumHeight(40);
    
    hopChonTrangThai = new QComboBox();
    hopChonTrangThai->addItems({"Tất cả", "Hoạt động", "Bị khóa"});
    hopChonTrangThai->setMinimumHeight(40);
    
    hopChonGioiTinh = new QComboBox();
    hopChonGioiTinh->addItems({"Tất cả giới tính", "Nam", "Nữ"});
    hopChonGioiTinh->setMinimumHeight(40);
    
    nutTimKiem = new QPushButton();
    nutTimKiem->setIcon(QIcon(":/icons/search.svg"));
    nutTimKiem->setText(" Tìm");
    nutTimKiem->setIconSize(QSize(18, 18));
    nutTimKiem->setMinimumHeight(40);
    nutTimKiem->setMinimumWidth(100);
    nutTimKiem->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #3699FF, stop:1 #187DE4); color: white; border: none; "
        "border-radius: 8px; font-weight: bold; padding: 8px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #187DE4, stop:1 #0C5EBF); }"
        "QPushButton:pressed { background: #0C5EBF; }"
    );
    
    nutLamMoi = new QPushButton();
    nutLamMoi->setIcon(QIcon(":/icons/refresh.svg"));
    nutLamMoi->setText(" Làm mới");
    nutLamMoi->setIconSize(QSize(18, 18));
    nutLamMoi->setMinimumHeight(40);
    nutLamMoi->setMinimumWidth(100);
    nutLamMoi->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #1BC5BD, stop:1 #0D9488); color: white; border: none; "
        "border-radius: 8px; font-weight: bold; padding: 8px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #0D9488, stop:1 #0A7368); }"
        "QPushButton:pressed { background: #0A7368; }"
    );
    
    boTri->addWidget(oNhapTimKiem, 3);
    boTri->addWidget(hopChonTrangThai, 1);
    boTri->addWidget(hopChonGioiTinh, 1);
    boTri->addWidget(nutTimKiem);
    
    boTriChinh->addWidget(nhomTimKiem);
    
    // Connect signals - tìm kiếm real-time
    connect(oNhapTimKiem, &QLineEdit::textChanged, this, &WidgetQuanLyDocGia::xuLyTimKiem);
    connect(hopChonTrangThai, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &WidgetQuanLyDocGia::xuLyLocTrangThai);
    connect(hopChonGioiTinh, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &WidgetQuanLyDocGia::xuLyLocGioiTinh);
    connect(nutTimKiem, &QPushButton::clicked, this, [this]() { xuLyTimKiem(oNhapTimKiem->text()); });
}

void WidgetQuanLyDocGia::thieLapBangDocGia() {
    // switch to QTableView + model
    bangDocGia = new QTableView();
    modelDocGia = new DocGiaTableModel(&danhSachDocGia, this);
    bangDocGia->setModel(modelDocGia);
    
    // Styling chuyên nghiệp
    bangDocGia->setStyleSheet(
        "QTableView {"
        "    background-color: #1E1E2D;"
        "    border: 1px solid #2B2B40;"
        "    border-radius: 12px;"
        "    gridline-color: #2B2B40;"
        "    color: #FFFFFF;"
        "    selection-background-color: #3699FF;"
        "}"
        "QTableView::item {"
        "    padding: 8px;"
        "    border-bottom: 1px solid #2B2B40;"
        "}"
        "QTableView::item:selected {"
        "    background-color: #3699FF;"
        "    color: #FFFFFF;"
        "}"
        "QHeaderView::section {"
        "    background-color: #1E1E2D;"
        "    color: #FFFFFF;"
        "    padding: 12px;"
        "    border: none;"
        "    border-bottom: 2px solid #3699FF;"
        "    font-weight: bold;"
        "    font-size: 13px;"
        "}"
    );
    
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
        "QGroupBox { background-color: #1E1E2D; border: 1px solid #2B2B40; "
        "border-radius: 12px; padding: 15px; font-size: 14px; font-weight: bold; color: #FFFFFF; }"
    );
    
    QHBoxLayout *boTri = new QHBoxLayout(nhomChucNang);
    
    nutThemDocGia = new QPushButton();
    nutThemDocGia->setText("+ Thêm Độc Giả");
    nutThemDocGia->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #1BC5BD, stop:1 #0D9488); color: white; border: none; "
        "border-radius: 8px; font-weight: bold; padding: 10px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #0D9488, stop:1 #0A7368); }"
        "QPushButton:pressed { background: #0A7368; }"
    );
    
    nutSuaDocGia = new QPushButton();
    nutSuaDocGia->setText("Sửa");
    nutSuaDocGia->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #FFA800, stop:1 #CC8600); color: white; border: none; "
        "border-radius: 8px; font-weight: bold; padding: 10px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #CC8600, stop:1 #996600); }"
        "QPushButton:pressed { background: #996600; }"
        "QPushButton:disabled { background: #2B2B40; color: #6C6C80; }"
    );
    
    nutXoaDocGia = new QPushButton();
    nutXoaDocGia->setText("Xóa");
    nutXoaDocGia->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #F64E60, stop:1 #C9384A); color: white; border: none; "
        "border-radius: 8px; font-weight: bold; padding: 10px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #C9384A, stop:1 #A32A3A); }"
        "QPushButton:pressed { background: #A32A3A; }"
        "QPushButton:disabled { background: #2B2B40; color: #6C6C80; }"
    );
    
    nutKhoaThe = new QPushButton();
    nutKhoaThe->setText("Khóa Thẻ");
    nutKhoaThe->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #8950FC, stop:1 #6B3FCC); color: white; border: none; "
        "border-radius: 8px; font-weight: bold; padding: 10px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #6B3FCC, stop:1 #5430A3); }"
        "QPushButton:pressed { background: #5430A3; }"
        "QPushButton:disabled { background: #2B2B40; color: #6C6C80; }"
    );
    
    nutMoKhoaThe = new QPushButton();
    nutMoKhoaThe->setText("Mở Khóa");
    nutMoKhoaThe->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #3699FF, stop:1 #187DE4); color: white; border: none; "
        "border-radius: 8px; font-weight: bold; padding: 10px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #187DE4, stop:1 #0C5EBF); }"
        "QPushButton:pressed { background: #0C5EBF; }"
        "QPushButton:disabled { background: #2B2B40; color: #6C6C80; }"
    );
    
    nutXemLichSu = new QPushButton();
    nutXemLichSu->setText("Lịch Sử");
    nutXemLichSu->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #3699FF, stop:1 #187DE4); color: white; border: none; "
        "border-radius: 8px; font-weight: bold; padding: 10px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #187DE4, stop:1 #0C5EBF); }"
        "QPushButton:pressed { background: #0C5EBF; }"
        "QPushButton:disabled { background: #2B2B40; color: #6C6C80; }"
    );
    
    boTri->addWidget(nutThemDocGia);
    boTri->addWidget(nutSuaDocGia);
    boTri->addWidget(nutXoaDocGia);
    boTri->addWidget(nutKhoaThe);
    boTri->addWidget(nutMoKhoaThe);
    boTri->addWidget(nutXemLichSu);
    boTri->addStretch();
    
    boTriChinh->addWidget(nhomChucNang);
    
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
    }
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
