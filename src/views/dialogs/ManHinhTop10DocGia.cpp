#include "views/dialogs/ManHinhTop10DocGia.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include "utils/LanguageManager.h"
#include <QAbstractTableModel>
#include <QFont>
#include <algorithm>

extern PTRDG rootDocGia;

// Model for Top10 học viên (vector-backed)
class Top10DocGiaModel : public QAbstractTableModel {
public:
    Top10DocGiaModel(std::vector<DocGiaVaSoLuot> *data, QObject *parent = nullptr)
        : QAbstractTableModel(parent), m_data(data) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return m_data ? (int)m_data->size() : 0;
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return 6; // Rank, ID, Name, Gender, Total, Current
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!m_data || !index.isValid()) return QVariant();
        int r = index.row();
        if (r < 0 || r >= (int)m_data->size()) return QVariant();
        const DocGiaVaSoLuot &item = (*m_data)[r];
        PTRDG dg = item.docGia;
        if (!dg) return QVariant();

        if (role == Qt::DisplayRole) {
            switch (index.column()) {
            case 0: // rank
                return (r < 3) ? QStringLiteral("🥇🥈🥉").mid(r,1) : QString::number(r + 1);
            case 1: return QString::number(dg->data.MaThe);
            case 2: return QString::fromStdString(std::string(dg->data.Ho + std::string(" ")) + dg->data.Ten);
            case 3: return dg->data.Phai ? LanguageManager::instance().getText("GENDER_MALE") : LanguageManager::instance().getText("GENDER_FEMALE");
            case 4: return QString::number(item.soLuotMuon);
            case 5: return QString::number(item.soDangMuon);
            }
        }

        if (role == Qt::FontRole) {
            QFont f;
            if (index.column() == 0) f.setBold(true);
            if (index.column() == 4 || index.column() == 5) f.setBold(true);
            return f;
        }

        if (role == Qt::ForegroundRole && index.column() == 4) {
            return QBrush(QColor(5,150,105));
        }

        if (role == Qt::ForegroundRole && index.column() == 5) {
            return QBrush(item.soDangMuon > 0 ? QColor(234,88,12) : QColor(107,114,128));
        }

        if (role == Qt::BackgroundRole && index.column() == 0) {
            QColor c;
            if (r == 0) c = QColor(255,215,0);
            else if (r == 1) c = QColor(192,192,192);
            else if (r == 2) c = QColor(205,127,50);
            else c = QColor(52,73,94);
            return QBrush(c.lighter(180));
        }

        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case 0: return LanguageManager::instance().getText("TABLE_HEADER_RANK");
            case 1: return LanguageManager::instance().getText("TABLE_HEADER_CODE"); // Use generic code header if available, or ID
            case 2: return LanguageManager::instance().getText("TABLE_HEADER_FULLNAME"); // Or "Họ và Tên"
            case 3: return LanguageManager::instance().getText("TABLE_HEADER_GENDER");
            case 4: return LanguageManager::instance().getText("TABLE_HEADER_TOTAL_BORROW");
            case 5: return LanguageManager::instance().getText("TABLE_HEADER_CURRENT_BORROW");
            }
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    void refresh() { beginResetModel(); endResetModel(); }

private:
    std::vector<DocGiaVaSoLuot> *m_data;
};

ManHinhTop10DocGia::ManHinhTop10DocGia(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle(LanguageManager::instance().getText("TITLE_TOP10_READERS"));
    setMinimumSize(900, 600);
    setModal(true);

    thietLapGiaoDien();
    apDungPhongCachHienDai();
    taiDuLieuTop10();
}

void ManHinhTop10DocGia::thietLapGiaoDien() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel* titleLabel = new QLabel(LanguageManager::instance().getText("HEADER_TOP10_READERS"));
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 20px; "
        "font-weight: bold; "
        "color: white; "
        "padding: 15px; "
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 #065f46, stop:1 #047857); "
        "border-radius: 8px; "
        "margin-bottom: 10px;"
    );
    mainLayout->addWidget(titleLabel);

    // Info label
    QLabel* infoLabel = new QLabel(LanguageManager::instance().getText("MSG_STATS_READERS"));
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet(
        "color: #1e293b; "
        "font-size: 13px; "
        "padding: 10px; "
        "background-color: #f1f5f9; "
        "border-left: 4px solid #047857; "
        "border-radius: 5px;"
    );
    mainLayout->addWidget(infoLabel);

    // Table (model/view)
    bangKetQua = new QTableView(this);
    modelKetQua = new Top10DocGiaModel(&danhSachTop, this);
    bangKetQua->setModel(modelKetQua);
    bangKetQua->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangKetQua->setSelectionMode(QAbstractItemView::SingleSelection);
    bangKetQua->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bangKetQua->setAlternatingRowColors(true);
    bangKetQua->horizontalHeader()->setStretchLastSection(false);
    bangKetQua->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    bangKetQua->verticalHeader()->setVisible(false);
    // Set column widths
    bangKetQua->setColumnWidth(0, 60);  // Rank
    bangKetQua->setColumnWidth(1, 100); // ID
    bangKetQua->setColumnWidth(2, 250); // Name
    bangKetQua->setColumnWidth(3, 100); // Gender
    bangKetQua->setColumnWidth(4, 120); // Total
    bangKetQua->setColumnWidth(5, 120); // Current
    mainLayout->addWidget(bangKetQua);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    nutLamMoi = new QPushButton(LanguageManager::instance().getText("BTN_REFRESH"));
    nutLamMoi->setObjectName("refreshButton");
    nutLamMoi->setMinimumHeight(40);
    connect(nutLamMoi, &QPushButton::clicked, this, &ManHinhTop10DocGia::taiDuLieuTop10);

    nutDong = new QPushButton(LanguageManager::instance().getText("BTN_CLOSE"));
    nutDong->setObjectName("closeButton");
    nutDong->setMinimumHeight(40);
    connect(nutDong, &QPushButton::clicked, this, &QDialog::accept);

    buttonLayout->addStretch();
    buttonLayout->addWidget(nutLamMoi);
    buttonLayout->addWidget(nutDong);

    mainLayout->addLayout(buttonLayout);
}

void ManHinhTop10DocGia::apDungPhongCachHienDai() {
    setStyleSheet(R"(
        QDialog {
            background-color: #ffffff;
        }
        
        QTableView {
            background-color: white;
            border: 2px solid #bdc3c7;
            border-radius: 8px;
            gridline-color: #ecf0f1;
            font-size: 13px;
        }
        
        QTableView::item {
            padding: 8px;
            border-bottom: 1px solid #ecf0f1;
        }
        
        QTableView::item:selected {
            background-color: #047857;
            color: white;
        }
        
        QHeaderView::section {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #047857, stop:1 #065f46);
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
                stop:0 #047857, stop:1 #065f46);
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
                stop:0 #059669, stop:1 #047857);
        }
        
        QPushButton#refreshButton:pressed {
            background: #065f46;
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

void ManHinhTop10DocGia::taiDuLieuTop10() {
    // Build vector of results
    danhSachTop.clear();

    if (!rootDocGia) {
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_INFO"),
            LanguageManager::instance().getText("MSG_NO_READERS"));
        return;
    }

    std::function<void(PTRDG)> duyetCay = [&](PTRDG node) {
        if (!node) return;
        duyetCay(node->left);

        DocGiaVaSoLuot it;
        it.docGia = node;
        it.soLuotMuon = 0;
        it.soDangMuon = 0;
        MUONTRA mt = node->data.dsmt;
        while (mt) {
            it.soLuotMuon++;
            if (mt->data.TrangThai == 0) it.soDangMuon++;
            mt = mt->next;
        }
        if (it.soLuotMuon > 0) danhSachTop.push_back(it);

        duyetCay(node->right);
    };

    duyetCay(rootDocGia);

    if (danhSachTop.empty()) {
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_INFO"),
            LanguageManager::instance().getText("MSG_NO_BORROWS"));
        return;
    }

    std::sort(danhSachTop.begin(), danhSachTop.end(), [](const DocGiaVaSoLuot &a, const DocGiaVaSoLuot &b){
        return a.soLuotMuon > b.soLuotMuon;
    });

    if ((int)danhSachTop.size() > 10) danhSachTop.resize(10);

    // refresh model
    if (modelKetQua) { if (auto m = dynamic_cast<Top10DocGiaModel*>(modelKetQua)) m->refresh(); }

    // Use uniform row height for better performance
    bangKetQua->verticalHeader()->setDefaultSectionSize(45);
}
