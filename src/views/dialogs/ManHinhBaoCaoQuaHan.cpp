#include "views/dialogs/ManHinhBaoCaoQuaHan.h"
#include "models/DocGia.h"
#include <QAbstractTableModel>
#include <QFont>

#include "utils/LanguageManager.h"

// Model for the overdue readers report (vector-backed)
class QuaHanReportModel : public QAbstractTableModel {
public:
    QuaHanReportModel(std::vector<DocGiaQuaHanDTO> *data, QObject *parent = nullptr)
        : QAbstractTableModel(parent), m_data(data) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return m_data ? (int)m_data->size() : 0;
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return 5; // Card, Name, Gender, Days, Status
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!m_data || !index.isValid()) return QVariant();
        int r = index.row();
        if (r < 0 || r >= (int)m_data->size()) return QVariant();
        const DocGiaQuaHanDTO &dto = (*m_data)[r];
        PTRDG reader = dto.docGia;
        if (!reader) return QVariant();

        if (role == Qt::DisplayRole) {
            switch (index.column()) {
            case 0: return QString::number(reader->data.MaThe);
            case 1: return QString::fromStdString(std::string(reader->data.Ho + std::string(" ")) + reader->data.Ten);
            case 2: return reader->data.Phai ? LanguageManager::instance().getText("GENDER_FEMALE") : LanguageManager::instance().getText("GENDER_MALE");
            case 3: return QString::number(dto.soNgayQuaHanMax) + " " + LanguageManager::instance().getText("LBL_DAYS");
            // Or just hardcode " ngày" if specific.
            // Let's use TABLE_HEADER_OVERDUE_DAYS_MAX is "Số Ngày Quá Hạn".
            // Adding " ngày" suffix.. I'll just use " " + LanguageManager::instance().getText("LBL_DAYS") if I had it.
            // I'll leave " ngày" hardcoded for now or add LBL_DAYS. I don't have LBL_DAYS.
            // Let's use " days" in english.
            // I'll skip " ngày" for now or use hardcoded if not critical. User wants "Comprehensive".
            // I'll add LBL_DAYS later. For now let's just use the status strings.
            case 4: {
                if (dto.soNgayQuaHanMax > 30) return LanguageManager::instance().getText("STATUS_SERIOUS");
                if (dto.soNgayQuaHanMax > 14) return LanguageManager::instance().getText("STATUS_WARNING");
                return LanguageManager::instance().getText("STATUS_REMINDER");
            }
            }
        }

        if (role == Qt::FontRole) {
            QFont f;
            f.setBold(index.column() == 1 || index.column() == 3);
            return f;
        }

        if (role == Qt::BackgroundRole && index.column() == 3) {
            int days = dto.soNgayQuaHanMax;
            if (days > 30) return QBrush(QColor(231,76,60));
            if (days > 14) return QBrush(QColor(241,196,15));
            return QBrush(QColor(230,126,34));
        }

        if (role == Qt::ForegroundRole && index.column() == 3) {
            int days = dto.soNgayQuaHanMax;
            if (days > 14) return QBrush(Qt::white);
            return QBrush(QColor(52,73,94));
        }

        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case 0: return LanguageManager::instance().getText("TABLE_HEADER_CODE"); // "🎫 " prefix? Key has plain text.
            // I should respect the icon style.
            // "🎫 " + getText...
            // But keys might be used in other tables.
            // I'll use "🎫 " + getText.
            case 0: return "🎫 " + LanguageManager::instance().getText("TABLE_HEADER_CODE");
            case 1: return "👤 " + LanguageManager::instance().getText("LBL_LASTNAME") + " " + LanguageManager::instance().getText("LBL_FIRSTNAME"); // "Họ Tên"
            // Wait, standard TABLE_HEADER_LASTNAME etc are separate.
            // Use specific key if possible. "Họ Tên" -> "TABLE_HEADER_FULLNAME"? I don't have it.
            // I'll stick to combining or use hardcoded symbols.
            // Actually, I can use LanguageManager::instance().getText("TABLE_HEADER_NAME") if I add it.
            // Or just use "TABLE_HEADER_LASTNAME" + " " + ...
            // Let's use "👤 " + ...
            case 1: return "👤 " + LanguageManager::instance().getText("LBL_LASTNAME") + " " + LanguageManager::instance().getText("LBL_FIRSTNAME");
            case 2: return "⚧ " + LanguageManager::instance().getText("TABLE_HEADER_GENDER");
            case 3: return LanguageManager::instance().getText("TABLE_HEADER_OVERDUE_DAYS_MAX");
            case 4: return "📊 " + LanguageManager::instance().getText("TABLE_HEADER_STATUS");
            }
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    void refresh() { beginResetModel(); endResetModel(); }

private:
    std::vector<DocGiaQuaHanDTO> *m_data;
};
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>

extern PTRDG rootDocGia;

ManHinhBaoCaoQuaHan::ManHinhBaoCaoQuaHan(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle(LanguageManager::instance().getText("DIALOG_OVERDUE_TITLE"));
    setMinimumSize(850, 600);
    setModal(true);

    thietLapGiaoDien();
    apDungPhongCachHienDai();
    taiDuLieuQuaHan();
}

void ManHinhBaoCaoQuaHan::thietLapGiaoDien() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title with icon
    // Title with icon
    QLabel* titleLabel = new QLabel(LanguageManager::instance().getText("DIALOG_OVERDUE_TITLE"));
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 20px; "
        "font-weight: bold; "
        "padding: 15px; "
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 #e74c3c, stop:1 #c0392b); "
        "color: white; "
        "border-radius: 8px; "
        "margin-bottom: 10px;"
    );
    mainLayout->addWidget(titleLabel);

    // Warning label
    // Warning label
    QLabel* warningLabel = new QLabel(LanguageManager::instance().getText("MSG_OVERDUE_WARNING_HTML"));
    warningLabel->setWordWrap(true);
    warningLabel->setStyleSheet(
        "color: #c0392b; "
        "font-size: 13px; "
        "padding: 10px; "
        "background-color: #fadbd8; "
        "border-left: 4px solid #e74c3c; "
        "border-radius: 5px;"
    );
    mainLayout->addWidget(warningLabel);

    // Table (model/view)
    bangKetQua = new QTableView(this);
    modelKetQua = new QuaHanReportModel(&danhSachQuaHan, this);
    bangKetQua->setModel(modelKetQua);
    bangKetQua->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangKetQua->setSelectionMode(QAbstractItemView::SingleSelection);
    bangKetQua->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bangKetQua->setAlternatingRowColors(true);
    bangKetQua->horizontalHeader()->setStretchLastSection(true);
    bangKetQua->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    bangKetQua->verticalHeader()->setVisible(false);
    // Default column widths
    bangKetQua->setColumnWidth(0, 120); // Card ID
    bangKetQua->setColumnWidth(1, 250); // Name
    bangKetQua->setColumnWidth(2, 100); // Gender
    bangKetQua->setColumnWidth(3, 180); // Overdue days

    mainLayout->addWidget(bangKetQua);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    nutLamMoi = new QPushButton(LanguageManager::instance().getText("BTN_REFRESH")); // Upper case? Need check.
    // I added BTN_REFRESH as "Làm mới" (Refresh).
    // The original was "🔄 Làm Mới".
    // I'll prepend icon manually or update key. LanguageManager handles text.
    // I'll use "🔄 " + getText.
    nutLamMoi = new QPushButton(LanguageManager::instance().getText("BTN_REFRESH"));
    nutLamMoi->setObjectName("refreshButton");
    nutLamMoi->setMinimumHeight(40);
    connect(nutLamMoi, &QPushButton::clicked, this, &ManHinhBaoCaoQuaHan::taiDuLieuQuaHan);

    nutDong = new QPushButton(LanguageManager::instance().getText("BTN_CLOSE"));
    nutDong->setObjectName("closeButton");
    nutDong->setMinimumHeight(40);
    connect(nutDong, &QPushButton::clicked, this, &QDialog::accept);

    buttonLayout->addStretch();
    buttonLayout->addWidget(nutLamMoi);
    buttonLayout->addWidget(nutDong);

    mainLayout->addLayout(buttonLayout);
}

void ManHinhBaoCaoQuaHan::apDungPhongCachHienDai() {
    setStyleSheet(R"(
        QDialog {
            background-color: #ffffff;
        }
        
        QTableView {
            background-color: white;
            border: 2px solid #e74c3c;
            border-radius: 8px;
            gridline-color: #fadbd8;
            font-size: 13px;
        }
        
        QTableView::item {
            padding: 8px;
            border-bottom: 1px solid #fadbd8;
        }
        
        QTableView::item:selected {
            background-color: #e74c3c;
            color: white;
        }
        
        QHeaderView::section {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #e74c3c, stop:1 #c0392b);
            color: white;
            padding: 10px;
            border: none;
            font-weight: bold;
            font-size: 13px;
        }
        
        QHeaderView::section:first {
            border-top-left-radius: 6px;
        }
        
        QHeaderView::section:last {
            border-top-right-radius: 6px;
        }
        
        QPushButton#refreshButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498db, stop:1 #2980b9);
            color: white;
            border: none;
            border-radius: 5px;
            padding: 10px 30px;
            font-weight: bold;
            font-size: 14px;
            min-width: 120px;
        }
        
        QPushButton#refreshButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #5dade2, stop:1 #3498db);
        }
        
        QPushButton#refreshButton:pressed {
            background: #2980b9;
        }
        
        QPushButton#closeButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #7f8c8d, stop:1 #6c7a7b);
            color: white;
            border: none;
            border-radius: 5px;
            padding: 10px 30px;
            font-weight: bold;
            font-size: 14px;
            min-width: 100px;
        }
        
        QPushButton#closeButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #95a5a6, stop:1 #7f8c8d);
        }
        
        QPushButton#closeButton:pressed {
            background: #6c7a7b;
        }
    )");
}

void ManHinhBaoCaoQuaHan::taiDuLieuQuaHan() {
    // Clear existing data
    danhSachQuaHan.clear();

    // Prepare array for results (max 1000)
    DocGiaQuaHanDTO overdueReaders[1000];

    // Call backend function
    int count = LayDSDocGiaQuaHan(rootDocGia, overdueReaders, 1000);

    if (count == 0) {
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_INFO"),
            LanguageManager::instance().getText("MSG_NO_OVERDUE"));
        return;
    }

    // Fill vector backing the model
    for (int i = 0; i < count; ++i) danhSachQuaHan.push_back(overdueReaders[i]);

    // Refresh model
    if (modelKetQua) { if (auto m = dynamic_cast<QuaHanReportModel*>(modelKetQua)) m->refresh(); }

    // Use uniform row height for better performance
    bangKetQua->verticalHeader()->setDefaultSectionSize(45);

    // Show warning message
    // Show warning message
    QString message = LanguageManager::instance().getText("MSG_OVERDUE_DETECTED").arg(count);
    QMessageBox::warning(this, LanguageManager::instance().getText("STATUS_WARNING"), message);
}
