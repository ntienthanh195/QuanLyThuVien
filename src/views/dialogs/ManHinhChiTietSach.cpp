#include "views/dialogs/ManHinhChiTietSach.h"
#include "models/QuanLySach.h"
#include "utils/ToastNotification.h"
#include "utils/LanguageManager.h"
#include <QInputDialog>
#include <QLineEdit>
// Forward declaration of DBBatcher API used here to avoid include path issues
// (declaration matches include/utils/DBBatcher.h)
namespace DBBatcher { std::vector<PTRDMS> layBanSaoTheoISBN(const std::string &isbn); }
#include <QVBoxLayout>
#include <QHBoxLayout>

void ManHinhChiTietSach::khiNhanThemBanSao() {
    if (!sachDuocChon) return;

    bool ok;
    int soLuong = QInputDialog::getInt(this, LanguageManager::instance().getText("DIALOG_ADD_COPY_TITLE"),
                                     LanguageManager::instance().getText("DIALOG_ADD_COPY_MSG"), 1, 1, 100, 1, &ok);
    if (!ok) return;

    QString viTri = QInputDialog::getText(this, LanguageManager::instance().getText("DIALOG_LOCATION_TITLE"),
                                        LanguageManager::instance().getText("DIALOG_LOCATION_MSG"), QLineEdit::Normal, "", &ok);
    if (!ok) return;

    try {
        std::string viTriStr = viTri.toStdString();
        std::string result = ThemBanSaoMoi(sachDuocChon->ISBN, soLuong, viTriStr);

        if (result.empty()) {
            ToastNotification::show(LanguageManager::instance().getText("MSG_ADD_COPY_SUCCESS"), ToastNotification::SUCCESS);
            taiThongTinSach(); // Refresh data
        } else {
            ToastNotification::show(QString::fromStdString(result), ToastNotification::ERROR);
        }
    } catch (const std::exception& e) {
        ToastNotification::show(QString("Lỗi: %1").arg(e.what()), ToastNotification::ERROR);
    }
}
namespace DBBatcher { std::vector<PTRDMS> layBanSaoTheoISBN(const std::string &isbn); }
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollBar>
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QModelIndex>
#include <algorithm>
#include <QInputDialog>
#include "utils/PrintHelper.h"
#include "../../bao_gom/giao_dien/hop_thoai/HopThoaiSuaSach.h" // Relative path fallback

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
                        case CHO_MUON_DUOC: return LanguageManager::instance().getText("LBL_STATUS_AVAILABLE");
                        case DANG_MUON: return LanguageManager::instance().getText("LBL_STATUS_Borrowed");
                        case THANH_LY: return LanguageManager::instance().getText("LBL_STATUS_LIQUIDATED");
                        default: return LanguageManager::instance().getText("LBL_STATUS_UNKNOWN");
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
            case 0: return LanguageManager::instance().getText("TABLE_HEADER_BOOK_CODE"); // Reusing general key if exists, or LBL_BOOK_CODE
            case 1: return LanguageManager::instance().getText("TABLE_HEADER_STATUS");
            case 2: return LanguageManager::instance().getText("TABLE_HEADER_LOCATION"); // Need to check if this key exists or use LBL
            case 3: return LanguageManager::instance().getText("TABLE_HEADER_ACTION");
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
    setWindowTitle(LanguageManager::instance().getText("TITLE_BOOK_DETAILS"));
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
    mainLayout->setSpacing(8); // Compact spacing
    mainLayout->setContentsMargins(10, 10, 10, 10); // Compact margins
    
    // Title
    QLabel *titleLabel = new QLabel(LanguageManager::instance().getText("HEADER_BOOK_DETAILS"), this);
    QFont titleFont;
    titleFont.setPointSize(14); // Reduced size
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #FFFFFF; marginBottom: 5px;");
    mainLayout->addWidget(titleLabel);
    
    // Global background for Dialog
    this->setStyleSheet("QDialog { background-color: #1e1e2d; color: #E0E0E0; }");
    
    // Book info group
    QGroupBox *infoGroup = new QGroupBox(LanguageManager::instance().getText("GROUP_GENERAL_INFO"), this);
    // Dark Modern styling
    infoGroup->setStyleSheet(
        "QGroupBox {"
        "   border: 1px solid #3E3E42;"
        "   border-radius: 6px;"
        "   margin-top: 10px;"
        "   padding-top: 15px;"
        "   font-weight: bold;"
        "   background-color: #252526;"
        "   color: #E0E0E0;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top left;"
        "   left: 10px;"
        "   top: -10px;"
        "   padding: 0 5px;"
        "   color: #5BA3F5;" // Softer Blue
        "   background-color: #1e1e2d;" // Match dialog bg
        "}"
    );
    
    QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);
    nhanTongQuat = new QLabel(this);
    nhanTongQuat->setWordWrap(true);
    nhanTongQuat->setStyleSheet("padding: 10px; font-size: 10pt;");
    infoLayout->addWidget(nhanTongQuat);
    mainLayout->addWidget(infoGroup);
    
    // Copies table group
    QGroupBox *copiesGroup = new QGroupBox(LanguageManager::instance().getText("GROUP_COPIES_LIST"), this);
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
        "   background-color: #1b1b29;"
        "   color: #ffffff;"
        "   border: 1px solid #323248;"
        "   border-radius: 6px;"
        "   gridline-color: #2b2b40;"
        "   font-size: 10pt;"
        "   selection-background-color: #3699ff;"
        "   selection-color: #ffffff;"
        "}"
        "QTableView::item {"
        "   padding: 8px;"
        "}"
        "QTableView::item:selected {"
        "   background-color: #3699ff;"
        "   color: #ffffff;"
        "}"
        "QHeaderView::section {"
        "   background-color: #2b2b40;"
        "   color: #b5b5c3;"
        "   padding: 10px;"
        "   border: none;"
        "   border-bottom: 2px solid #323248;"
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
    
    nutThanhLy = new QPushButton(LanguageManager::instance().getText("BTN_LIQUIDATE"), this);
    nutThanhLy->setMinimumHeight(40);
    nutThanhLy->setCursor(Qt::PointingHandCursor);
    nutThanhLy->setStyleSheet(
        "QPushButton {"
        "   background-color: #C42B1C;" // Destructive Red
        "   color: #ffffff;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "   padding: 6px 16px;"
        "}"
        "QPushButton:hover { background-color: #e69500; }"
        "QPushButton:disabled { background-color: #2b2b40; color: #565674; }"
    );
    
    // START EDIT: ADD EDIT BUTTON
    nutSuaSach = new QPushButton(LanguageManager::instance().getText("BTN_EDIT"), this);
    nutSuaSach->setMinimumHeight(40);
    nutSuaSach->setCursor(Qt::PointingHandCursor);
    nutSuaSach->setStyleSheet(
        "QPushButton {"
        "   background-color: #5D9CEC;" // Blue
        "   color: #ffffff;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "   padding: 6px 16px;"
        "}"
        "QPushButton:hover { background-color: #4A89DC; }"
    );
    // END EDIT
    
    nutThemBanSao = new QPushButton(LanguageManager::instance().getText("BTN_ADD_COPY"), this);
    nutThemBanSao->setMinimumHeight(40);
    nutThemBanSao->setCursor(Qt::PointingHandCursor);
    nutThemBanSao->setStyleSheet(
        "QPushButton {"
        "   background-color: #28a745;" // Green
        "   color: #ffffff;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "   padding: 6px 16px;"
        "}"
        "QPushButton:hover { background-color: #218838; }"
    );
    
    nutLamMoi = new QPushButton(LanguageManager::instance().getText("BTN_REFRESH"), this);
    nutLamMoi->setMinimumHeight(40);
    nutLamMoi->setCursor(Qt::PointingHandCursor);
    nutLamMoi->setStyleSheet(
        "QPushButton {"
        "   background-color: #2D2D30;" // Secondary Dark
        "   color: #E0E0E0;"
        "   border: 1px solid #3E3E42;"
        "   border-radius: 4px;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "   padding: 6px 16px;"
        "}"
        "QPushButton:hover { background-color: #106ebe; }"
    );
    
    nutDong = new QPushButton(LanguageManager::instance().getText("BTN_CLOSE"), this);
    nutDong->setMinimumHeight(40);
    nutDong->setCursor(Qt::PointingHandCursor);
    nutDong->setStyleSheet(
        "QPushButton {"
        "   background-color: #2b2b40;"
        "   color: #ffffff;"
        "   border: 1px solid #323248;"
        "   border-radius: 6px;"
        "   font-size: 10pt;"
        "   font-weight: 600;"
        "   padding: 0 15px;"
        "}"
        "QPushButton:hover { background-color: #323248; border-color: #565674; }"
    );
    
    buttonLayout->addWidget(nutThanhLy);
    buttonLayout->addWidget(nutThemBanSao); // Added Add Copy button
    buttonLayout->addWidget(nutSuaSach); // Add Edit button to layout
    
    // New: delete copy button (permanent removal)
    nutXoaBanSao = new QPushButton(LanguageManager::instance().getText("BTN_DELETE_COPY"), this);
    nutXoaBanSao->setEnabled(false);
    nutXoaBanSao->setMinimumHeight(40);
    nutXoaBanSao->setStyleSheet(
        "QPushButton { background-color: #C42B1C; color: #ffffff; border: none; border-radius: 4px; font-weight: 600; padding: 6px 16px; font-size: 13px; }"
        "QPushButton:hover { background-color: #A80000; }"
        "QPushButton:disabled { background-color: #2b2b40; color: #565674; }"
    );
    buttonLayout->addWidget(nutXoaBanSao);
    buttonLayout->addWidget(nutLamMoi);

    // Nút in tem mã vạch
    nutInTem = new QPushButton(LanguageManager::instance().getText("BTN_PRINT_BARCODE"), this);
    nutInTem->setMinimumHeight(40);
    nutInTem->setCursor(Qt::PointingHandCursor);
    nutInTem->setStyleSheet(
        "QPushButton {"
        "   background-color: #2D2D30;" // Secondary Dark
        "   color: #E0E0E0;"
        "   border: 1px solid #3E3E42;"
        "   border-radius: 6px;"
        "   font-size: 10pt;"
        "   font-weight: 600;"
        "   padding: 0 15px;"
        "}"
        "QPushButton:hover { background-color: #7337ee; }"
        "QPushButton:disabled { background-color: #2b2b40; color: #565674; }"
    );
    buttonLayout->addWidget(nutInTem);

    buttonLayout->addStretch();
    buttonLayout->addWidget(nutDong);
    mainLayout->addLayout(buttonLayout);
}

void ManHinhChiTietSach::thietLapKetNoi() {
    connect(nutThanhLy, &QPushButton::clicked, this, &ManHinhChiTietSach::khiNhanThanhLy);
    connect(nutXoaBanSao, &QPushButton::clicked, this, &ManHinhChiTietSach::khiNhanXoaBanSao);
    connect(nutLamMoi, &QPushButton::clicked, this, &ManHinhChiTietSach::khiNhanLamMoi);
    connect(nutInTem, &QPushButton::clicked, this, &ManHinhChiTietSach::khiNhanInTem);
    connect(nutDong, &QPushButton::clicked, this, &ManHinhChiTietSach::khiNhanDong);
    connect(nutSuaSach, &QPushButton::clicked, this, &ManHinhChiTietSach::khiNhanSuaSach); // Connect Edit
    connect(nutThemBanSao, &QPushButton::clicked, this, &ManHinhChiTietSach::khiNhanThemBanSao);
    
    // connect selection change from view
    if (bangBanSao->selectionModel()) {
        connect(bangBanSao->selectionModel(), &QItemSelectionModel::currentChanged,
                this, &ManHinhChiTietSach::khiChonBanSao);
    }
}

void ManHinhChiTietSach::taiThongTinSach() {
    if (!sachDuocChon) return;
    
    QString info = QString(
        "<b>%1:</b> %2<br>"
        "<b>%3:</b> %4<br>"
        "<b>%5:</b> %6<br>"
        "<b>%7:</b> %8<br>"
        "<b>%9:</b> %10<br>"
        "<b>%11:</b> %12"
    )
    .arg(LanguageManager::instance().getText("LBL_TITLE"))
    .arg(QString::fromStdString(sachDuocChon->tenSach))
    .arg(LanguageManager::instance().getText("LBL_AUTHOR"))
    .arg(QString::fromStdString(sachDuocChon->tacGia))
    .arg(LanguageManager::instance().getText("LBL_ISBN"))
    .arg(QString::fromStdString(sachDuocChon->ISBN))
    .arg(LanguageManager::instance().getText("LBL_CATEGORY"))
    .arg(QString::fromStdString(sachDuocChon->theLoai))
    .arg(LanguageManager::instance().getText("LBL_PUBLISH_YEAR"))
    .arg(sachDuocChon->namXuatBan)
    .arg(LanguageManager::instance().getText("LBL_PAGE_COUNT"))
    .arg(sachDuocChon->soTrang);
    
    // Feature: Display Price
    QLocale locale(QLocale::Vietnamese, QLocale::Vietnam);
    QString giaTienStr = locale.toCurrencyString((double)sachDuocChon->giaTien, "VNĐ");
    info += QString("<br><b>%1:</b> <span style='color: #78C2AD; font-weight: bold;'>%2</span>")
        .arg(LanguageManager::instance().getText("LBL_PRICE"))
        .arg(giaTienStr);
    
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
    
    // Use HTML table for better layout control and avoid overlapping
    // Use HTML table for better layout control and avoid overlapping
    QString summary = QString(
        "<table style='width: 100%; border-collapse: collapse;'>"
        "<tr>"
        "<td style='padding: 5px; color: #b5b5c3;'><b>%1:</b></td><td style='padding: 5px; color: #ffffff;'>%2</td>"
        "<td style='padding: 5px; color: #b5b5c3;'><b>%3:</b></td><td style='padding: 5px; color: #27ae60;'>%4</td>"
        "</tr>"
        "<tr>"
        "<td style='padding: 5px; color: #b5b5c3;'><b>%5:</b></td><td style='padding: 5px; color: #3498db;'>%6</td>"
        "<td style='padding: 5px; color: #b5b5c3;'><b>%7:</b></td><td style='padding: 5px; color: #95a5a6;'>%8</td>"
        "</tr>"
        "</table>"
    )
    .arg(LanguageManager::instance().getText("LBL_TOTAL_COPIES"))
    .arg(info.tongBanSao)
    .arg(LanguageManager::instance().getText("LBL_AVAILABLE_COPIES"))
    .arg(info.choMuonDuoc)
    .arg(LanguageManager::instance().getText("LBL_BORROWED_COPIES")) // Need key
    .arg(info.dangMuon)
    .arg(LanguageManager::instance().getText("LBL_LIQUIDATED_COPIES")) // Need key
    .arg(info.thanhLy);
    
    nhanTrangThai->setText(summary);
    // Remove fixed height/padding in stylesheet to let HTML handle it, but keep container style
    nhanTrangThai->setStyleSheet("background-color: #2b2b40; border: 1px solid #323248; border-radius: 6px;");
}

QString ManHinhChiTietSach::layBieuTuongTrangThai(TrangThaiSach status) {
    // Removed emojis
    Q_UNUSED(status);
    return "";
}

QString ManHinhChiTietSach::layVanBanTrangThai(TrangThaiSach status) {
    switch (status) {
        case CHO_MUON_DUOC: return LanguageManager::instance().getText("LBL_STATUS_AVAILABLE");
        case DANG_MUON: return LanguageManager::instance().getText("LBL_STATUS_Borrowed");
        case THANH_LY: return LanguageManager::instance().getText("LBL_STATUS_LIQUIDATED");
        default: return LanguageManager::instance().getText("LBL_STATUS_UNKNOWN");
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
        hienThiThongBao(LanguageManager::instance().getText("MSG_SELECT_COPY_DELETE"), true);
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::warning(
        this,
        LanguageManager::instance().getText("MSG_CONFIRM_DELETE_COPY_TITLE"),
        LanguageManager::instance().getText("MSG_CONFIRM_DELETE_COPY_BODY")
            .arg(QString::fromStdString(maBanSaoDuocChon)),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        std::string result = XoaBanSaoTheoMa(maBanSaoDuocChon);
        if (result.empty()) {
            hienThiThongBao(LanguageManager::instance().getText("MSG_DELETE_COPY_SUCCESS"));
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
        hienThiThongBao(LanguageManager::instance().getText("MSG_SELECT_COPY_LIQUIDATE"), true);
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::warning(
        this,
        LanguageManager::instance().getText("MSG_CONFIRM_LIQUIDATE_TITLE"),
        LanguageManager::instance().getText("MSG_CONFIRM_LIQUIDATE_BODY")
            .arg(QString::fromStdString(maBanSaoDuocChon)),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        std::string result = ThanhLyBanSaoTheoMa(maBanSaoDuocChon);
        
        if (result.empty()) {
            hienThiThongBao(LanguageManager::instance().getText("MSG_LIQUIDATE_SUCCESS"));
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
    hienThiThongBao(LanguageManager::instance().getText("MSG_REFRESH_SUCCESS"));
}

void ManHinhChiTietSach::khiNhanSuaSach() {
    if (!sachDuocChon) return;
    
    HopThoaiSuaSach *dialog = new HopThoaiSuaSach(QString::fromStdString(sachDuocChon->ISBN), this);
    connect(dialog, &HopThoaiSuaSach::sachDaSua, this, [this]() {
        taiThongTinSach();
        capNhatTongQuat(); // Update general info too
        emit sachDaCapNhat();
    });
    dialog->exec();
    delete dialog;
}

void ManHinhChiTietSach::khiNhanInTem() {
    if (!sachDuocChon) return;

    bool ok;
    int soLuong = QInputDialog::getInt(this, LanguageManager::instance().getText("BTN_PRINT_BARCODE"),
                                       LanguageManager::instance().getText("MSG_ENTER_PRINT_QUANTITY"), 1, 1, 100, 1, &ok);
    if (ok) {
        PrintHelper::inNhieuTemSach(sachDuocChon, soLuong, this);
    }
}

void ManHinhChiTietSach::khiNhanDong() {
    accept();
}
