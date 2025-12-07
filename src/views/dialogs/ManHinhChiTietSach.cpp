#include "views/dialogs/ManHinhChiTietSach.h"
#include "models/QuanLySach.h"
// Forward declaration of DBBatcher API used here to avoid include path issues
// (declaration matches include/utils/DBBatcher.h implemented elsewhere).
namespace DBBatcher { std::vector<PTRDMS> layBanSaoTheoISBN(const std::string &isbn); }
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollBar>
#include <QAbstractTableModel>
#include <QModelIndex>
#include <algorithm>

// Model for Bản sao table
class BanSaoTableModel : public QAbstractTableModel {
public:
    BanSaoTableModel(std::vector<PTRDMS> *data, QObject *parent = nullptr)
        : QAbstractTableModel(parent), m_data(data) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return m_data ? (int)m_data->size() : 0;
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return 4;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!m_data || !index.isValid()) return QVariant();
        int r = index.row();
        if (r < 0 || r >= (int)m_data->size()) return QVariant();
        PTRDMS copy = (*m_data)[r];

        if (role == Qt::DisplayRole) {
            switch (index.column()) {
                case 0: return QString::fromStdString(copy->maSach);
                case 1: {
                    switch (copy->trangThai) {
                        case CHO_MUON_DUOC: return QStringLiteral("Có thể mượn");
                        case DANG_MUON: return QStringLiteral("Đang được mượn");
                        case THANH_LY: return QStringLiteral("Đã thanh lý");
                        default: return QStringLiteral("Không rõ");
                    }
                }
                case 2: {
                    QString location = QString::fromStdString(copy->viTri);
                    if (location.isEmpty()) return QString("Chưa xác định");
                    return location;
                }
                case 3: return (copy->trangThai == CHO_MUON_DUOC) ? QString("Có thể thanh lý") : QString("-");
                }
        }

        if (role == Qt::BackgroundRole && index.column() == 1) {
            switch (copy->trangThai) {
                case CHO_MUON_DUOC: return QBrush(QColor("#27ae60"));
                case DANG_MUON: return QBrush(QColor("#3498db"));
                case THANH_LY: return QBrush(QColor("#bdc3c7"));
                default: return QBrush(QColor("#95a5a6"));
            }
        }

        if (role == Qt::FontRole) {
            QFont f;
            if (index.column() == 0 || index.column() == 3) { f.setBold(true); return f; }
        }

        if (role == Qt::ForegroundRole) {
            if (index.column() == 1 && copy->trangThai != THANH_LY) return QColor(Qt::white);
            if (index.column() == 6) return QColor("#c0392b");
        }

        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case 0: return QStringLiteral("Mã Sách");
            case 1: return QStringLiteral("Trạng Thái");
            case 2: return QStringLiteral("Vị Trí");
            case 3: return QStringLiteral("Thao Tác");
            }
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    void refresh() { beginResetModel(); endResetModel(); }

private:
    std::vector<PTRDMS> *m_data;
};

ManHinhChiTietSach::ManHinhChiTietSach(PTRDS book, QWidget *parent)
    : QDialog(parent)
    , sachDuocChon(book)
    , dongBanSaoDuocChon(-1)
{
    setWindowTitle("Chi Tiết Sách");
    setMinimumSize(750, 550);
    thietLapGiaoDien();
    thietLapKetNoi();
    taiThongTinSach();
    taiDanhSachBanSao();
    capNhatTongQuat();
}

ManHinhChiTietSach::~ManHinhChiTietSach() {
}

void ManHinhChiTietSach::thietLapGiaoDien() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Title
    QLabel *titleLabel = new QLabel("CHI TIẾT ĐẦU SÁCH", this);
    QFont titleFont;
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #2c3e50; margin-bottom: 5px;");
    mainLayout->addWidget(titleLabel);
    
    // Book info group
    QGroupBox *infoGroup = new QGroupBox("Thông Tin Chung", this);
    // Subtle styling: neutral border and light background
    infoGroup->setStyleSheet(
        "QGroupBox {"
        "   border: 1px solid #d1d5db;"
        "   border-radius: 6px;"
        "   margin-top: 8px;"
        "   padding-top: 12px;"
        "   font-weight: bold;"
        "   background-color: #fbfdff;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top left;"
        "   left: 12px;"
        "   top: -8px;"
        "   padding: 0 6px;"
        "   color: #111827;"
        "   background-color: transparent;"
        "}"
    );
    
    QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);
    nhanTongQuat = new QLabel(this);
    nhanTongQuat->setWordWrap(true);
    nhanTongQuat->setStyleSheet("padding: 10px; font-size: 10pt;");
    infoLayout->addWidget(nhanTongQuat);
    mainLayout->addWidget(infoGroup);
    
    // Copies table group
    QGroupBox *copiesGroup = new QGroupBox("Danh Sách Bản Sao", this);
    copiesGroup->setStyleSheet(infoGroup->styleSheet());
    
    QVBoxLayout *copiesLayout = new QVBoxLayout(copiesGroup);
    
    // Use model/view for copies table
    bangBanSao = new QTableView(this);
    modelBanSao = new BanSaoTableModel(&danhSachBanSaoVec, this);
    bangBanSao->setModel(modelBanSao);
    bangBanSao->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangBanSao->setSelectionMode(QAbstractItemView::SingleSelection);
    bangBanSao->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bangBanSao->setAlternatingRowColors(true);
    bangBanSao->horizontalHeader()->setStretchLastSection(true);
    bangBanSao->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    bangBanSao->verticalHeader()->setVisible(false);
    bangBanSao->setMinimumHeight(250);

    bangBanSao->setStyleSheet(
        "QTableView {"
        "   border: 1px solid #e5e7eb;"
        "   border-radius: 6px;"
        "   gridline-color: #f3f4f6;"
        "   font-size: 10pt;"
        "}"
        "QTableView::item {"
        "   padding: 8px;"
        "}"
        "QTableView::item:selected {"
        "   background-color: #e6f0fb;"
        "   color: #0f172a;"
        "}"
        "QHeaderView::section {"
        "   background-color: #f8fafc;"
        "   color: #111827;"
        "   padding: 8px;"
        "   border: none;"
        "   font-weight: 600;"
        "}"
    );

    copiesLayout->addWidget(bangBanSao);
    mainLayout->addWidget(copiesGroup);
    
    // Status label
    nhanTrangThai = new QLabel("", this);
    nhanTrangThai->setAlignment(Qt::AlignCenter);
    nhanTrangThai->setMinimumHeight(30);
    mainLayout->addWidget(nhanTrangThai);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    nutThanhLy = new QPushButton("Thanh Lý Bản Sao", this);
    nutThanhLy->setEnabled(false);
    nutThanhLy->setMinimumHeight(40);
    nutThanhLy->setStyleSheet(
        "QPushButton {"
        "   background-color: #e67e22;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   font-size: 10pt;"
        "   font-weight: bold;"
        "   padding: 8px 15px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #d68910;"
        "}"
        "QPushButton:disabled {"
        "   background-color: #95a5a6;"
        "}"
    );
    
    nutLamMoi = new QPushButton("Làm Mới", this);
    nutLamMoi->setMinimumHeight(40);
    nutLamMoi->setStyleSheet(
        "QPushButton {"
        "   background-color: #3498db;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   font-size: 10pt;"
        "   font-weight: bold;"
        "   padding: 8px 15px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2980b9;"
        "}"
    );
    
    nutDong = new QPushButton("Đóng", this);
    nutDong->setMinimumHeight(40);
    nutDong->setStyleSheet(
        "QPushButton {"
        "   background-color: #7f8c8d;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   font-size: 10pt;"
        "   font-weight: bold;"
        "   padding: 8px 15px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #5d6d7e;"
        "}"
    );
    
    buttonLayout->addWidget(nutThanhLy);
    // New: delete copy button (permanent removal)
    nutXoaBanSao = new QPushButton("Xóa Bản Sao", this);
    nutXoaBanSao->setEnabled(false);
    nutXoaBanSao->setMinimumHeight(40);
    nutXoaBanSao->setStyleSheet(
        "QPushButton { background-color: #ef4444; color: white; border: none; border-radius: 5px; font-weight: bold; padding: 8px 15px;}"
        "QPushButton:disabled { background-color: #95a5a6; }"
    );
    buttonLayout->addWidget(nutXoaBanSao);
    buttonLayout->addWidget(nutLamMoi);
    buttonLayout->addWidget(nutLamMoi);
    buttonLayout->addStretch();
    buttonLayout->addWidget(nutDong);
    mainLayout->addLayout(buttonLayout);
}

void ManHinhChiTietSach::thietLapKetNoi() {
    connect(nutThanhLy, &QPushButton::clicked, this, &ManHinhChiTietSach::khiNhanThanhLy);
    connect(nutXoaBanSao, &QPushButton::clicked, this, &ManHinhChiTietSach::khiNhanXoaBanSao);
    connect(nutLamMoi, &QPushButton::clicked, this, &ManHinhChiTietSach::khiNhanLamMoi);
    connect(nutDong, &QPushButton::clicked, this, &ManHinhChiTietSach::khiNhanDong);
    // connect selection change from view
    if (bangBanSao->selectionModel()) {
        connect(bangBanSao->selectionModel(), &QItemSelectionModel::currentChanged,
                this, &ManHinhChiTietSach::khiChonBanSao);
    }
}

void ManHinhChiTietSach::taiThongTinSach() {
    if (!sachDuocChon) return;
    
    QString info = QString(
        "📖 <b>Tên sách:</b> %1<br>"
        "✍️ <b>Tác giả:</b> %2<br>"
        "📚 <b>ISBN:</b> %3<br>"
        "📂 <b>Thể loại:</b> %4<br>"
        "📅 <b>Năm xuất bản:</b> %5<br>"
        "📄 <b>Số trang:</b> %6"
    )
    .arg(QString::fromStdString(sachDuocChon->tenSach))
    .arg(QString::fromStdString(sachDuocChon->tacGia))
    .arg(QString::fromStdString(sachDuocChon->ISBN))
    .arg(QString::fromStdString(sachDuocChon->theLoai))
    .arg(sachDuocChon->namXuatBan)
    .arg(sachDuocChon->soTrang);
    
    nhanTongQuat->setText(info);
}

void ManHinhChiTietSach::taiDanhSachBanSao() {
    // populate vector backing the model
    danhSachBanSaoVec.clear();

    if (!sachDuocChon) return;

    // Lấy danh sách bản sao qua DBBatcher (snapshot vector) thay vì buffer C-style
    std::vector<PTRDMS> copies = DBBatcher::layBanSaoTheoISBN(sachDuocChon->ISBN);
    int count = static_cast<int>(copies.size());
    for (int i = 0; i < count; ++i) {
        PTRDMS copy = copies[i];
        if (!copy) continue;
        danhSachBanSaoVec.push_back(copy);
    }

    // Use uniform row height for performance
    bangBanSao->verticalHeader()->setDefaultSectionSize(45);

    // refresh model
    if (modelBanSao) { if (auto m = dynamic_cast<BanSaoTableModel*>(modelBanSao)) m->refresh(); }
    bangBanSao->resizeColumnsToContents();
}

void ManHinhChiTietSach::capNhatTongQuat() {
    if (!sachDuocChon) return;
    
    ThongTinDauSach info = LayThongTinDauSach(sachDuocChon->ISBN);
    
    QString summary = QString(
        "📊 <b>Tổng bản sao:</b> %1 | "
        "✅ <b>Có thể mượn:</b> %2 | "
        "📖 <b>Đang mượn:</b> %3 | "
        "🗑️ <b>Đã thanh lý:</b> %4"
    )
    .arg(info.tongBanSao)
    .arg(info.choMuonDuoc)
    .arg(info.dangMuon)
    .arg(info.thanhLy);
    
    nhanTrangThai->setText(summary);
    nhanTrangThai->setStyleSheet("color: #2c3e50; font-weight: bold; padding: 8px; background-color: #ecf0f1; border-radius: 5px;");
}

QString ManHinhChiTietSach::layBieuTuongTrangThai(TrangThaiSach status) {
    switch (status) {
        case CHO_MUON_DUOC: return "✅";
        case DANG_MUON: return "📖";
        case THANH_LY: return "🗑️";
        default: return "❓";
    }
}

QString ManHinhChiTietSach::layVanBanTrangThai(TrangThaiSach status) {
    switch (status) {
        case CHO_MUON_DUOC: return "Có thể mượn";
        case DANG_MUON: return "Đang được mượn";
        case THANH_LY: return "Đã thanh lý";
        default: return "Không rõ";
    }
}

QString ManHinhChiTietSach::layMauTrangThai(TrangThaiSach status) {
    switch (status) {
        case CHO_MUON_DUOC: return "#27ae60";  // Green
        case DANG_MUON: return "#3498db";      // Blue
        case THANH_LY: return "#bdc3c7";       // Gray
        default: return "#95a5a6";
    }
}

void ManHinhChiTietSach::hienThiThongBao(const QString &message, bool isError) {
    nhanTrangThai->setText((isError ? "❌ " : "✅ ") + message);
    nhanTrangThai->setStyleSheet(
        QString("color: %1; font-weight: bold; padding: 8px; background-color: %2; border-radius: 5px;")
        .arg(isError ? "#e74c3c" : "#27ae60")
        .arg(isError ? "#fadbd8" : "#d5f4e6")
    );
}

void ManHinhChiTietSach::khiChonBanSao(const QModelIndex &index) {
    int row = index.row();
    if (row < 0 || row >= (int)danhSachBanSaoVec.size()) {
        dongBanSaoDuocChon = -1;
        maBanSaoDuocChon.clear();
        nutThanhLy->setEnabled(false);
        nutXoaBanSao->setEnabled(false);
        return;
    }

    dongBanSaoDuocChon = row;
    PTRDMS copy = danhSachBanSaoVec[row];
    if (copy) {
        maBanSaoDuocChon = copy->maSach;

        bool canRetire = (copy->trangThai == CHO_MUON_DUOC);
        nutThanhLy->setEnabled(canRetire);
        nutXoaBanSao->setEnabled(canRetire);
    }
}

void ManHinhChiTietSach::khiNhanXoaBanSao() {
    if (maBanSaoDuocChon.empty()) {
        hienThiThongBao("Vui lòng chọn bản sao cần xóa!", true);
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::warning(
        this,
        "Xác nhận xóa bản sao",
        QString("Bạn có chắc muốn xóa bản sao:\n%1?\n\n⚠️ Hành động này sẽ xóa vĩnh viễn bản sao khỏi hệ thống!")
            .arg(QString::fromStdString(maBanSaoDuocChon)),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        std::string result = XoaBanSaoTheoMa(maBanSaoDuocChon);
        if (result.empty()) {
            hienThiThongBao("Đã xóa bản sao thành công!");
            taiDanhSachBanSao();
            capNhatTongQuat();
            emit sachDaCapNhat();
            dongBanSaoDuocChon = -1;
            maBanSaoDuocChon.clear();
            nutThanhLy->setEnabled(false);
            nutXoaBanSao->setEnabled(false);
        } else {
            hienThiThongBao(QString::fromStdString(result), true);
        }
    }
}

void ManHinhChiTietSach::khiNhanThanhLy() {
    if (maBanSaoDuocChon.empty()) {
        hienThiThongBao("Vui lòng chọn bản sao cần thanh lý!", true);
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::warning(
        this,
        "Xác nhận thanh lý",
        QString("Bạn có chắc muốn thanh lý bản sao:\n%1?\n\n"
                "⚠️ Hành động này không thể hoàn tác!")
            .arg(QString::fromStdString(maBanSaoDuocChon)),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        std::string result = ThanhLyBanSaoTheoMa(maBanSaoDuocChon);
        
        if (result.empty()) {
            hienThiThongBao("Đã thanh lý bản sao thành công!");
            taiDanhSachBanSao();
            capNhatTongQuat();
            emit sachDaCapNhat();
            dongBanSaoDuocChon = -1;
            maBanSaoDuocChon.clear();
            nutThanhLy->setEnabled(false);
        } else {
            hienThiThongBao(QString::fromStdString(result), true);
        }
    }
}

void ManHinhChiTietSach::khiNhanLamMoi() {
    taiDanhSachBanSao();
    capNhatTongQuat();
    hienThiThongBao("Đã làm mới dữ liệu!");
}

void ManHinhChiTietSach::khiNhanDong() {
    accept();
}
