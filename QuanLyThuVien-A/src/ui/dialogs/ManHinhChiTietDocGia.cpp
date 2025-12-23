/**
 * @file ManHinhChiTietDocGia.cpp
 * @path src/views/dialogs/ManHinhChiTietDocGia.cpp
 * @description Hộp thoại hiển thị thông tin chi tiết và lịch sử mượn trả của một học viên.
 * Hỗ trợ tra cứu nhanh các giao dịch liên quan đến thẻ học viên này.
 * @project Library Management System (QuanLyThuVien)
 */
#include "ui/dialogs/ManHinhChiTietDocGia.h"

#include "ui/dialogs/HopThoaiSuaDocGia.h" // Add missing include
#include "ui/dialogs/HopThoaiThanhToan.h" // NEW: For Payment
#include "models/QuanLySach.h" // To lookup book titles if needed
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollBar>
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QDate>
#include "utils/PrintHelper.h"
#include <QMap>
#include "utils/XuLyChuoi.h"
#include "utils/SettingsManager.h" // Add missing include
#include "utils/LanguageManager.h"
#include "utils/ToastNotification.h"
#include <QLocale>
#include <QPainter>
#include <QPainterPath>

// Model for Borrowed Books table
class SachMuonTableModel : public QAbstractTableModel {
public:
    SachMuonTableModel(std::vector<ManHinhChiTietDocGia::ThongTinMuon> *data, QObject *parent = nullptr)
        : QAbstractTableModel(parent), m_data(data) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return m_data ? (int)m_data->size() : 0;
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return 5;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!m_data || !index.isValid()) return QVariant();
        int r = index.row();
        if (r < 0 || r >= (int)m_data->size()) return QVariant();
        const auto &item = (*m_data)[r];

        if (role == Qt::DisplayRole) {
            switch (index.column()) {
                case 0: return item.maSach;
                case 1: return item.ngayMuon.toString("dd/MM/yyyy");
                case 2: return item.hanTra.toString("dd/MM/yyyy");
                case 3: return item.trangThai;
                case 4: return (item.soNgayQuaHan > 0) ? QString("%1 ngày").arg(item.soNgayQuaHan) : QString("-");
            }
        }

        if (role == Qt::ForegroundRole) {
             // 1. Debt Logic: Whole row RED
             if ((item.status == 1 || item.status == 2) && item.tienPhat > 0 && !item.daThanhToan) {
                 return QColor("#FF5252");
             }
             // 2. Overdue Logic: Only 'Overdue' column
            if (index.column() == 4 && item.soNgayQuaHan > 0) return QColor("#f64e60"); 
            
            // 3. Status specific colors (if not debt)
            if (index.column() == 3) {
                 if (item.status == 2) return QColor("#f64e60"); // Mat
                 if (item.status == 1) return QColor("#27ae60"); // Tra
            }
        }
        
        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case 0: return LanguageManager::instance().getText("TABLE_HEADER_BOOK_CODE"); // Needs key or generic "Mã Sách"
            case 1: return LanguageManager::instance().getText("TABLE_HEADER_BORROW_DATE");
            case 2: return LanguageManager::instance().getText("TABLE_HEADER_DUE_DATE");
            case 3: return LanguageManager::instance().getText("TABLE_HEADER_STATUS");
            case 4: return LanguageManager::instance().getText("TABLE_HEADER_OVERDUE"); // "Quá Hạn"
            }
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    void refresh() { beginResetModel(); endResetModel(); }

private:
    std::vector<ManHinhChiTietDocGia::ThongTinMuon> *m_data;
};

ManHinhChiTietDocGia::ManHinhChiTietDocGia(PTRDG reader, QWidget *parent)
    : QDialog(parent)
    , docGiaDuocChon(reader)
    , bangSachMuon(nullptr)
    , modelSachMuon(nullptr)
    , nutSua(nullptr)
    , nutThuHoi(nullptr)
    , nutBaoMat(nullptr)
    , nutLamMoi(nullptr)
    , nutInThe(nullptr)
    , nutXuatPDF(nullptr)
    , nutDong(nullptr)
    , dongSachDuocChon(-1)
{
    setWindowTitle(LanguageManager::instance().getText("TITLE_READER_DETAILS"));
    setMinimumSize(800, 600); // Larger to "mở cái lịch sử đó ra"
    thietLapGiaoDien();
    thietLapKetNoi();
    taiThongTinDocGia();
    taiDanhSachSachMuon();
}

ManHinhChiTietDocGia::~ManHinhChiTietDocGia() {
}

void ManHinhChiTietDocGia::thietLapGiaoDien() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Global background for Dialog (Deep Night)
    this->setStyleSheet("QDialog { background-color: #1e1e2d; color: #ffffff; }");
    
    // Title
    QLabel *titleLabel = new QLabel(LanguageManager::instance().getText("HEADER_READER_DETAILS"), this);
    QFont titleFont;
    titleFont.setPointSize(14); // Reduced from 16
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #ffffff; margin-bottom: 5px; letter-spacing: 0.5px;");
    mainLayout->addWidget(titleLabel);
    
    // Reader Info Group
    QGroupBox *infoGroup = new QGroupBox(LanguageManager::instance().getText("GROUP_PERSONAL_INFO"), this);
    infoGroup->setStyleSheet(
        "QGroupBox {"
        "   border: 1px solid #323248;"
        "   border-radius: 8px;"
        "   margin-top: 10px;"
        "   padding: 15px;"
        "   font-weight: bold;"
        "   background-color: #2b2b40;"
        "   color: #ffffff;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top left;"
        "   left: 10px;"
        "   top: -10px;"
        "   padding: 0 5px;"
        "   color: #3699ff;"
        "   background-color: #1e1e2d;"
        "}"
    );
    
    QGridLayout *infoLayout = new QGridLayout(infoGroup);
    infoLayout->setSpacing(10);
    
    auto addInfoRow = [&](const QString &lblKey, QLabel* &valLabel, int row, int col) {
        QLabel *l = new QLabel(LanguageManager::instance().getText(lblKey) + ":", this);
        l->setStyleSheet("color: #b5b5c3; font-weight: 500; font-size: 11px;");
        valLabel = new QLabel("--", this);
        valLabel->setStyleSheet("color: #ffffff; font-weight: 600; font-size: 13px;");
        infoLayout->addWidget(l, row, col * 2);
        infoLayout->addWidget(valLabel, row, col * 2 + 1);
    };

    addInfoRow("LBL_READER_ID", lblMaThe, 0, 0);
    addInfoRow("LBL_FULLNAME", lblHoTen, 0, 1);
    addInfoRow("LBL_READER_TYPE", lblLoaiDocGia, 0, 2);
    
    addInfoRow("LBL_CLASS", lblLop, 1, 0);
    addInfoRow("TABLE_HEADER_GENDER", lblGioiTinh, 1, 1);
    addInfoRow("LBL_STATUS", lblTrangThai, 1, 2);

    addInfoRow("LBL_DOB", lblNgaySinh, 2, 0);
    addInfoRow("LBL_PHONE", lblSDT, 2, 1);
    addInfoRow("LBL_EMAIL", lblEmail, 2, 2);

    infoLayout->setColumnStretch(1, 1);
    infoLayout->setColumnStretch(3, 1);
    infoLayout->setColumnStretch(5, 1);

    mainLayout->addWidget(infoGroup);
    
    // Borrowed Books Group
    QGroupBox *booksGroup = new QGroupBox(LanguageManager::instance().getText("GROUP_HISTORY"), this);
    booksGroup->setStyleSheet(infoGroup->styleSheet());
    
    QVBoxLayout *booksLayout = new QVBoxLayout(booksGroup);
    
    bangSachMuon = new QTableView(this);
    modelSachMuon = new SachMuonTableModel(&danhSachMuonVec, this);
    bangSachMuon->setModel(modelSachMuon);
    bangSachMuon->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangSachMuon->setSelectionMode(QAbstractItemView::SingleSelection);
    bangSachMuon->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bangSachMuon->setAlternatingRowColors(true);
    bangSachMuon->horizontalHeader()->setStretchLastSection(true);
    bangSachMuon->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    bangSachMuon->verticalHeader()->setVisible(false);
    bangSachMuon->setMinimumHeight(250);

    bangSachMuon->setStyleSheet(
        "QTableView {"
        "   background-color: #1b1b29;"
        "   color: #ffffff;"
        "   border: 1px solid #323248;"
        "   border-radius: 6px;"
        "   gridline-color: #2b2b40;"
        "   font-size: 9pt;" // Reduced from 10pt
        "   selection-background-color: #3699ff;"
        "   selection-color: #ffffff;"
        "}"
        "QTableView::item {"
        "   padding: 4px;" // Reduced from 8px
        "}"
        "QTableView::item:selected {"
        "   background-color: #3699ff;"
        "   color: #ffffff;"
        "}"
        "QHeaderView::section {"
        "   background-color: #2b2b40;"
        "   color: #b5b5c3;"
        "   padding: 6px;" // Reduced from 10px
        "   border: none;"
        "   border-bottom: 2px solid #323248;"
        "   font-weight: 600;"
        "}"
    );

    booksLayout->addWidget(bangSachMuon);
    mainLayout->addWidget(booksGroup, 1); // Give it weight to expand

    // Advanced History Chart removed by user request ("xấu quá")
    
    // Status label
    nhanTrangThai = new QLabel("", this);
    nhanTrangThai->setAlignment(Qt::AlignCenter);
    nhanTrangThai->setMinimumHeight(30);
    nhanTrangThai->setStyleSheet("color: #CCCCCC; font-weight: 500; padding: 4px; background-color: #2b2b40; border: 1px solid #323248; border-radius: 4px; font-size: 13px;");
    mainLayout->addWidget(nhanTrangThai);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    // Adding Edit Button as a utility
    nutSua = new QPushButton(LanguageManager::instance().getText("BTN_EDIT"), this);
    nutSua->setMinimumHeight(40);
    nutSua->setCursor(Qt::PointingHandCursor);
    nutSua->setStyleSheet(
        "QPushButton {"
        "   background-color: #242933;"
        "   color: #CCCCCC;"
        "   border: 1px solid #3E3E42;"
        "   border-radius: 4px;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "   padding: 0 15px;"
        "}"
        "QPushButton:hover { background-color: #2D2D30; border: 1px solid #5BA3F5; }"
    );

    nutThuHoi = new QPushButton(LanguageManager::instance().getText("BTN_RETURN_BOOK"), this);
    nutThuHoi->setEnabled(false);
    nutThuHoi->setMinimumHeight(40);
    nutThuHoi->setCursor(Qt::PointingHandCursor);
    nutThuHoi->setStyleSheet(
        "QPushButton {"
        "   background-color: #0078D4;" // Primary Blue
        "   color: #ffffff;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "   padding: 0 15px;"
        "}"
        "QPushButton:hover { background-color: #138D13; }"
        "QPushButton:disabled { background-color: #2b2b40; color: #565674; }"
    );
    
    nutBaoMat = new QPushButton(LanguageManager::instance().getText("BTN_REPORT_LOST"), this);
    nutBaoMat->setEnabled(false);
    nutBaoMat->setMinimumHeight(40);
    nutBaoMat->setCursor(Qt::PointingHandCursor);
    nutBaoMat->setStyleSheet(
        "QPushButton {"
        "   background-color: #C42B1C;" // Standard Red
        "   color: #ffffff;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "   padding: 0 15px;"
        "}"
        "QPushButton:hover { background-color: #A80000; }"
        "QPushButton:disabled { background-color: #2b2b40; color: #565674; }"
    );

    nutThanhToan = new QPushButton(LanguageManager::instance().getText("BTN_PAY"), this);
    nutThanhToan->setEnabled(false);
    nutThanhToan->setMinimumHeight(40);
    nutThanhToan->setCursor(Qt::PointingHandCursor);
    nutThanhToan->setStyleSheet(
        "QPushButton {"
        "   background-color: #FFA800;" // Orange/Yellow for Payment
        "   color: #ffffff;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "   padding: 0 15px;"
        "}"
        "QPushButton:hover { background-color: #D98E00; }"
        "QPushButton:disabled { background-color: #2b2b40; color: #565674; }"
    );
    
    nutLamMoi = new QPushButton(LanguageManager::instance().getText("BTN_REFRESH"), this);
    nutLamMoi->setMinimumHeight(40);
    nutLamMoi->setCursor(Qt::PointingHandCursor);
    nutLamMoi->setStyleSheet(
        "QPushButton {"
        "   background-color: #2D2D30;" // Secondary Dark
        "   color: #E0E0E0;"
        "   border: 1px solid #3E3E42;"
        "   border-radius: 6px;"
        "   font-size: 10pt;"
        "   font-weight: 600;"
        "   padding: 0 15px;"
        "}"
        "QPushButton:hover { background-color: #187de4; }"
    );
    
    nutInThe = new QPushButton(LanguageManager::instance().getText("BTN_PRINT_CARD_TV"), this);
    nutInThe->setMinimumHeight(40);
    nutInThe->setCursor(Qt::PointingHandCursor);
    nutInThe->setStyleSheet(
        "QPushButton {"
        "   background-color: #2D2D30;" // Secondary Dark
        "   color: #E0E0E0;"
        "   border: 1px solid #3E3E42;"
        "   border-radius: 4px;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "   padding: 0 15px;"
        "}"
        "QPushButton:hover { background-color: #4A93E3; }"
    );
    
    nutXuatPDF = new QPushButton("📄 " + LanguageManager::instance().getText("BTN_EXPORT_PDF"), this); // Need to check if key exists or use hardcoded
    nutXuatPDF->setMinimumHeight(40);
    nutXuatPDF->setCursor(Qt::PointingHandCursor);
    nutXuatPDF->setStyleSheet(nutInThe->styleSheet());
    
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
    
    buttonLayout->addWidget(nutSua); // Add new button
    buttonLayout->addWidget(nutThuHoi);
    buttonLayout->addWidget(nutBaoMat);
    buttonLayout->addWidget(nutThanhToan); // NEW
    buttonLayout->addWidget(nutLamMoi);
    buttonLayout->addWidget(nutInThe);
    buttonLayout->addWidget(nutXuatPDF);
    buttonLayout->addStretch();
    buttonLayout->addWidget(nutDong);
    mainLayout->addLayout(buttonLayout);
}

void ManHinhChiTietDocGia::thietLapKetNoi() {
    connect(nutSua, &QPushButton::clicked, this, &ManHinhChiTietDocGia::khiNhanSua); // Connect Edit
    connect(nutDong, &QPushButton::clicked, this, &ManHinhChiTietDocGia::khiNhanDong);
    connect(nutLamMoi, &QPushButton::clicked, this, &ManHinhChiTietDocGia::khiNhanLamMoi);
    // Future implementation for return/loss:
    connect(nutThuHoi, &QPushButton::clicked, this, &ManHinhChiTietDocGia::khiNhanThuHoi);
    connect(nutBaoMat, &QPushButton::clicked, this, &ManHinhChiTietDocGia::khiNhanBaoMat);
    connect(nutThanhToan, &QPushButton::clicked, this, &ManHinhChiTietDocGia::khiNhanThanhToan); // NEW
    connect(nutInThe, &QPushButton::clicked, this, &ManHinhChiTietDocGia::khiNhanInThe);
    connect(nutXuatPDF, &QPushButton::clicked, this, [this]() {
        if (docGiaDuocChon) {
            PrintHelper::xuatLichSuMuonTraToPDF(docGiaDuocChon, this);
        }
    });
    
    if (bangSachMuon->selectionModel()) {
        connect(bangSachMuon->selectionModel(), &QItemSelectionModel::currentChanged,
                this, &ManHinhChiTietDocGia::khiChonSachMuon);
    }
}

void ManHinhChiTietDocGia::taiThongTinDocGia() {
    if (!docGiaDuocChon) return;
    
    lblMaThe->setText(QString::number(docGiaDuocChon->data.MaThe));
    lblHoTen->setText(QString::fromStdString(docGiaDuocChon->data.Ho + " " + docGiaDuocChon->data.Ten));
    
    QString typeStr = (docGiaDuocChon->data.LoaiDocGia == 0) ? LanguageManager::instance().getText("READER_TYPE_STUDENT") : LanguageManager::instance().getText("READER_TYPE_TEACHER");
    lblLoaiDocGia->setText(typeStr);
    
    lblLop->setText(QString::fromStdString(docGiaDuocChon->data.Lop));
    
    QString gender = (docGiaDuocChon->data.Phai) ? LanguageManager::instance().getText("GENDER_FEMALE") : LanguageManager::instance().getText("GENDER_MALE");
    lblGioiTinh->setText(gender);
    
    QString status = (docGiaDuocChon->data.TrangThai == 1) ? LanguageManager::instance().getText("STATUS_ACTIVE") : LanguageManager::instance().getText("STATUS_LOCKED");
    lblTrangThai->setText(status);
    lblTrangThai->setStyleSheet(docGiaDuocChon->data.TrangThai == 1 ? "color: #27ae60; font-weight: bold;" : "color: #f64e60; font-weight: bold;");

    lblNgaySinh->setText(QString::fromStdString(docGiaDuocChon->data.NgaySinh));
    lblSDT->setText(QString::fromStdString(docGiaDuocChon->data.SDT));
    lblEmail->setText(QString::fromStdString(docGiaDuocChon->data.Email));
}

void ManHinhChiTietDocGia::taiDanhSachSachMuon() {
    danhSachMuonVec.clear();
    if (!docGiaDuocChon) return;
    
    // Traverse linked list of borrowed books
    MUONTRA p = docGiaDuocChon->data.dsmt;
    while (p) {
        ThongTinMuon item; // Restored declaration
        if (p->data.banSaoSach) {
             item.maSach = QString::fromStdString(p->data.banSaoSach->maSach);
        } else if (!p->data.maSachCached.empty()) {
             item.maSach = QString::fromStdString(p->data.maSachCached);
        } else {
             item.maSach = "???";
        }

        // Parse date strings "dd/MM/yyyy"
        item.ngayMuon = QDate::fromString(QString::fromStdString(p->data.NgayMuon), "dd/MM/yyyy");
        item.hanTra = QDate::fromString(QString::fromStdString(p->data.NgayTra), "dd/MM/yyyy");
        
        // Determine status
        item.status = p->data.TrangThai;
        if (p->data.TrangThai == 0) item.trangThai = LanguageManager::instance().getText("LBL_BORROWING");
        else if (p->data.TrangThai == 1) item.trangThai = LanguageManager::instance().getText("STATUS_RETURNED");
        else if (p->data.TrangThai == 2) item.trangThai = LanguageManager::instance().getText("STATUS_LOST");
        
        // Calculate overdue
        item.soNgayQuaHan = 0;
        if (p->data.TrangThai == 0) {
            QDate today = QDate::currentDate();
            if (today > item.hanTra) {
                item.soNgayQuaHan = item.hanTra.daysTo(today);
            }
        }
        
        // Populate Debt Info
        item.tienPhat = p->data.tienPhat;
        item.daThanhToan = p->data.daThanhToan;

        // FILTER LOGIC:
        // Hide if (Returned OR Lost) AND Paid/NoDebt
        bool hide = false;
        if (p->data.TrangThai != 0) { // Not borrowing
             if (p->data.daThanhToan) hide = true; // Paid -> Hide (Clean History)
             else if (p->data.tienPhat == 0) hide = true; // No fine -> Hide (Clean Return)
        }
        
        if (!hide) {
            danhSachMuonVec.push_back(item);
        }
        item.nodeMuonTra = p;
        p = p->next;
    }
    
    if (modelSachMuon) { if (auto m = dynamic_cast<SachMuonTableModel*>(modelSachMuon)) m->refresh(); }
    bangSachMuon->resizeColumnsToContents();
}

void ManHinhChiTietDocGia::khiNhanSua() {
    if (!docGiaDuocChon) return;
    
    // Open Edit Dialog
    HopThoaiSuaDocGia *dialog = new HopThoaiSuaDocGia(docGiaDuocChon->data.MaThe, this);
    
    connect(dialog, &HopThoaiSuaDocGia::docGiaDaSua, this, [this]() {
        this->taiThongTinDocGia(); // Reload info
        emit this->docGiaDaCapNhat(); // Notify parent
    });
    
    dialog->exec();
    delete dialog;
}

void ManHinhChiTietDocGia::capNhatTongQuat() {
    // Refresh stats if needed
}

void ManHinhChiTietDocGia::khiChonSachMuon(const QModelIndex &index) {
    int row = index.row();
    if (row < 0 || row >= (int)danhSachMuonVec.size()) {
        dongSachDuocChon = -1;
        nutThuHoi->setEnabled(false);
        nutBaoMat->setEnabled(false);
        return;
    }
    
    dongSachDuocChon = row;
    ThongTinMuon item = danhSachMuonVec[row];
    
    // Check status int from node instead of string
    bool isBorrowing = (item.nodeMuonTra && item.nodeMuonTra->data.TrangThai == 0);
    nutThuHoi->setEnabled(isBorrowing);
    nutBaoMat->setEnabled(isBorrowing);
    
    // Enable Pay if Debt > 0 and Unpaid
    bool canPay = (item.tienPhat > 0 && !item.daThanhToan);
    nutThanhToan->setEnabled(canPay);
}

// Externs for global data access required by MuonTra functions
extern PTRDS dsDauSach[];
extern int soLuongDauSach;

#include "models/MuonTra.h"

void ManHinhChiTietDocGia::khiNhanThuHoi() {
    if (dongSachDuocChon < 0 || dongSachDuocChon >= (int)danhSachMuonVec.size()) return;
    
    ThongTinMuon item = danhSachMuonVec[dongSachDuocChon];
    // Check status int
    if (!item.nodeMuonTra || item.nodeMuonTra->data.TrangThai != 0) {
        ToastNotification::show(LanguageManager::instance().getText("MSG_WARN_RETURN_NOT_BORROWED"), ToastNotification::WARNING);
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, LanguageManager::instance().getText("MSG_CONFIRM_RETURN_TITLE"),
        LanguageManager::instance().getText("MSG_CONFIRM_RETURN_BOOK").arg(item.maSach),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        std::string res = TraSach(docGiaDuocChon, item.maSach.toStdString(), dsDauSach, soLuongDauSach);
        if (res.empty()) {
            hienThiThongBao(LanguageManager::instance().getText("MSG_RETURN_SUCCESS"));
            taiThongTinDocGia();
            taiDanhSachSachMuon();
            emit docGiaDaCapNhat();
            nutThuHoi->setEnabled(false);
            nutBaoMat->setEnabled(false);
        } else {
            hienThiThongBao(QString::fromStdString(res), true);
        }
    }
}

void ManHinhChiTietDocGia::khiNhanBaoMat() {
    if (dongSachDuocChon < 0 || dongSachDuocChon >= (int)danhSachMuonVec.size()) return;
    
    ThongTinMuon item = danhSachMuonVec[dongSachDuocChon];
    // Check status int
    if (!item.nodeMuonTra || item.nodeMuonTra->data.TrangThai != 0) {
        ToastNotification::show(LanguageManager::instance().getText("MSG_WARN_LOST_NOT_BORROWED"), ToastNotification::WARNING);
        return;
    }

    // Lookup book info to show price
    std::string isbn = TachISBNTuMaSach(item.maSach.toStdString());
    long long giaSach = 0;
    PTRDS dauSach = TimDauSach(isbn);
    if (dauSach) giaSach = dauSach->giaTien;

    // Use default fine setting
    int phiPhat = SettingsManager::instance().getInt("PHI_MAT_SACH", 20000);
    long long tongPhat = giaSach + phiPhat;

    QMessageBox::StandardButton reply = QMessageBox::warning(
        this, LanguageManager::instance().getText("MSG_CONFIRM_LOST_TITLE"),
        LanguageManager::instance().getText("MSG_CONFIRM_LOST_HTML")
            .arg(item.maSach)
            .arg(QLocale(QLocale::Vietnamese).toString((qint64)giaSach))
            .arg(QLocale(QLocale::Vietnamese).toString((qint64)phiPhat))
            .arg(QLocale(QLocale::Vietnamese).toString((qint64)tongPhat)),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        std::string res = BaoMatSach(docGiaDuocChon, item.maSach.toStdString(), dsDauSach, soLuongDauSach);
        if (res.empty()) {
            // Success
            ToastNotification::show(LanguageManager::instance().getText("MSG_LOST_RECORDED_SUCCESS")
                .arg(item.maSach)
                .arg(QLocale(QLocale::Vietnamese).toString((qint64)giaSach))
                .arg(QLocale(QLocale::Vietnamese).toString((qint64)phiPhat))
                .arg(QLocale(QLocale::Vietnamese).toString((qint64)tongPhat)), ToastNotification::SUCCESS);
            
            taiThongTinDocGia(); // Refresh reader stats
            taiDanhSachSachMuon(); // Refresh book list
            emit docGiaDaCapNhat(); // Notify parent
            nutThuHoi->setEnabled(false);
            nutBaoMat->setEnabled(false);
        } else {
             // Handle case where fine is calculated but returns success message or error
            hienThiThongBao(QString::fromStdString(res), true);
            // Even if error/warning, reload to show updated state if any partial change occurred
            taiThongTinDocGia();
            taiDanhSachSachMuon();
        }
    }
}

void ManHinhChiTietDocGia::khiNhanLamMoi() {
    taiThongTinDocGia();
    taiDanhSachSachMuon();
}

void ManHinhChiTietDocGia::khiNhanDong() {
    accept();
}

void ManHinhChiTietDocGia::khiNhanInThe() {
    if(docGiaDuocChon) {
        PrintHelper::inTheDocGia(docGiaDuocChon, this);
    }
}

void ManHinhChiTietDocGia::khiNhanThanhToan() {
    if (dongSachDuocChon < 0 || dongSachDuocChon >= (int)danhSachMuonVec.size()) return;
    ThongTinMuon item = danhSachMuonVec[dongSachDuocChon];
    
    if (item.tienPhat <= 0 || item.daThanhToan) {
        hienThiThongBao(LanguageManager::instance().getText("MSG_NO_DEBT_OR_PAID"), true);
        return;
    }

    // Call HopThoaiThanhToan
    QString reason = LanguageManager::instance().getText("REASON_PAY_FINE").arg(item.maSach);
    long long amount = item.tienPhat;
    
    HopThoaiThanhToan dialog(docGiaDuocChon, reason, amount, this);
    if (dialog.exec() == QDialog::Accepted) {
        std::string res = ThanhToanPhat(docGiaDuocChon, item.maSach.toStdString());
        if (res.empty()) {
            hienThiThongBao(LanguageManager::instance().getText("MSG_PAYMENT_SUCCESS"));
            taiThongTinDocGia();
            taiDanhSachSachMuon();
            emit docGiaDaCapNhat();
            nutThanhToan->setEnabled(false);
        } else {
             hienThiThongBao(QString::fromStdString(res), true);
        }
    }
}

void ManHinhChiTietDocGia::hienThiThongBao(const QString &message, bool isError) {
    if (!nhanTrangThai) return;
    
    nhanTrangThai->setText((isError ? "❌ " : "✅ ") + message);
    nhanTrangThai->setStyleSheet(
        QString("color: %1; font-weight: bold; padding: 4px; background-color: %2; border-radius: 4px; font-size: 13px;")
        .arg(isError ? "#e74c3c" : "#27ae60")
        .arg("#2b2b40") // Dark bg
    );
    
    ToastNotification::show(message, isError ? ToastNotification::ERROR : ToastNotification::SUCCESS);
}
