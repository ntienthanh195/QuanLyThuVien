/**
 * @file WidgetLichSuMuonTra.cpp
 * @path src/ui/WidgetLichSuMuonTra.cpp
 * @description Triển khai Widget xem lịch sử mượn trả và báo cáo tài chính.
 */
#include "ui/WidgetLichSuMuonTra.h"
#include "utils/LanguageManager.h"
#include "utils/DatabaseManager.h"
#include "utils/PrintHelper.h"
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>
#include <QMenu>
#include <QAction>
#include "models/UserManager.h"
#include <QLocale>
#include "ui/dialogs/HopThoaiThanhToan.h"
#include "models/DocGia.h"
#include <QDebug>
#include <QMessageBox>
#include <QShowEvent>
#include "utils/EventBus.h"

#include <QIdentityProxyModel>
#include "utils/FinanceHelper.h"

class LichSuProxyModel : public QIdentityProxyModel {
public:
    LichSuProxyModel(QObject *parent = nullptr) : QIdentityProxyModel(parent) {}

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid()) return QVariant();

        // Financial Status Column (index 5)
        if (role == Qt::DisplayRole && index.column() == 5) {
            int status = sourceModel()->data(sourceModel()->index(index.row(), 5)).toInt();
            long long fine = sourceModel()->data(sourceModel()->index(index.row(), 4)).toLongLong();
            bool paid = sourceModel()->data(sourceModel()->index(index.row(), 9)).toInt() == 1;
            long long refund = sourceModel()->data(sourceModel()->index(index.row(), 10)).toLongLong();

            if (status == 1) { // GD_DA_TRA
                if (fine > 0 && !paid) return LanguageManager::instance().getText("STATUS_RETURNED_DEBT").arg(QLocale(QLocale::Vietnamese).toString(fine));
                return LanguageManager::instance().getText("STATUS_RETURNED");
            }
            if (status == 2) return LanguageManager::instance().getText("STATUS_LOST_DEBT").arg(QLocale(QLocale::Vietnamese).toString(fine));
            if (status == 3) return LanguageManager::instance().getText("STATUS_LOST_PAID");
            if (status == 4) return LanguageManager::instance().getText("STATUS_LOST_REFUND_WAIT").arg(QLocale(QLocale::Vietnamese).toString(refund));
            if (status == 5) return LanguageManager::instance().getText("STATUS_LOST_REFUND_DONE");
            return LanguageManager::instance().getText("STATUS_BORROWING");
        }

        // Color Logic
        if (role == Qt::ForegroundRole) {
            int status = sourceModel()->data(sourceModel()->index(index.row(), 5)).toInt();
            long long fine = sourceModel()->data(sourceModel()->index(index.row(), 4)).toLongLong();
            bool paid = sourceModel()->data(sourceModel()->index(index.row(), 9)).toInt() == 1;
            
            if (status == 2 || (status == 1 && fine > 0 && !paid)) return QColor("#FF5252"); // Red
            if (status == 4) return QColor("#FF9800"); // Orange
            if (status == 3 || status == 5) return QColor("#4CAF50"); // Green
        }

        return QIdentityProxyModel::data(index, role);
    }
};

WidgetLichSuMuonTra::WidgetLichSuMuonTra(QWidget *parent) : QWidget(parent) {
    thieLapGiaoDien();
    // lamMoi(); // Chuyển sang showEvent để auto-refresh mỗi khi quay lại tab
    
    connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this, &WidgetLichSuMuonTra::capNhatNgonNgu);
}

WidgetLichSuMuonTra::~WidgetLichSuMuonTra() {}

void WidgetLichSuMuonTra::thieLapGiaoDien() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    // --- Filter Bar ---
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(10);

    oNhapTimKiem = new QLineEdit();
    oNhapTimKiem->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_SEARCH_HISTORY"));
    oNhapTimKiem->setMinimumWidth(200);
    oNhapTimKiem->setStyleSheet("QLineEdit { background: #1a1d23; color: white; border: 1px solid #333; border-radius: 4px; padding: 5px; }");

    ngayBatDau = new QDateEdit(QDate::currentDate().addMonths(-1));
    ngayBatDau->setCalendarPopup(true);
    ngayBatDau->setStyleSheet("QDateEdit { background: #1a1d23; color: white; border: 1px solid #333; padding: 5px; }");

    ngayKetThuc = new QDateEdit(QDate::currentDate());
    ngayKetThuc->setCalendarPopup(true);
    ngayKetThuc->setStyleSheet("QDateEdit { background: #1a1d23; color: white; border: 1px solid #333; padding: 5px; }");

    hopChonTrangThai = new QComboBox();
    hopChonTrangThai->addItem(LanguageManager::instance().getText("FILTER_ALL"), -1);
    hopChonTrangThai->addItem(LanguageManager::instance().getText("STATUS_BORROWING"), 0);
    hopChonTrangThai->addItem(LanguageManager::instance().getText("STATUS_RETURNED"), 1);
    hopChonTrangThai->addItem(LanguageManager::instance().getText("STATUS_LOST"), 2);
    hopChonTrangThai->setStyleSheet("QComboBox { background: #1a1d23; color: white; border: 1px solid #333; padding: 5px; }");

    nutTimKiem = new QPushButton(LanguageManager::instance().getText("BTN_SEARCH"));
    nutTimKiem->setStyleSheet("QPushButton { background: #0078D4; color: white; border: none; border-radius: 4px; padding: 6px 15px; font-weight: bold; } QPushButton:hover { background: #106EBE; }");

    chkBaoGomLuuTru = new QCheckBox(LanguageManager::instance().getText("CHK_INCLUDE_ARCHIVE"));
    chkBaoGomLuuTru->setStyleSheet("QCheckBox { color: #888; }");

    filterLayout->addWidget(new QLabel(LanguageManager::instance().getText("LBL_SEARCH") + ":"));
    filterLayout->addWidget(oNhapTimKiem);
    filterLayout->addWidget(new QLabel(LanguageManager::instance().getText("LBL_FROM") + ":"));
    filterLayout->addWidget(ngayBatDau);
    filterLayout->addWidget(new QLabel(LanguageManager::instance().getText("LBL_TO") + ":"));
    filterLayout->addWidget(ngayKetThuc);
    filterLayout->addWidget(hopChonTrangThai);
    filterLayout->addWidget(chkBaoGomLuuTru);
    filterLayout->addWidget(nutTimKiem);
    filterLayout->addStretch();

    mainLayout->addLayout(filterLayout);

    // --- Table ---
    bangLichSu = new QTableView();
    modelLichSu = new QSqlQueryModel(this);
    bangLichSu->setModel(modelLichSu);
    bangLichSu->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangLichSu->setAlternatingRowColors(true);
    bangLichSu->horizontalHeader()->setStretchLastSection(true);
    bangLichSu->setStyleSheet(
        "QTableView { background-color: #1e1e2d; color: #E0E0E0; border: 1px solid #333; gridline-color: #333; }"
        "QHeaderView::section { background-color: #252534; color: #5D9CEC; padding: 5px; border: 1px solid #333; font-weight: bold; }"
    );
    bangLichSu->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(bangLichSu, &QTableView::customContextMenuRequested, this, &WidgetLichSuMuonTra::xuLyMenuChuotPhai);
    
    mainLayout->addWidget(bangLichSu);

    // --- Footer ---
    QHBoxLayout *footerLayout = new QHBoxLayout();
    nhanThongKe = new QLabel();
    nhanThongKe->setStyleSheet("color: #5BA3F5; font-weight: bold; font-size: 14px;");
    
    nutXuatBaoCao = new QPushButton(LanguageManager::instance().getText("BTN_EXPORT_FINANCE"));
    nutXuatBaoCao->setStyleSheet("QPushButton { background: #2D2D30; color: #E0E0E0; border: 1px solid #3E3E42; border-radius: 4px; padding: 8px 15px; } QPushButton:hover { border-color: #0078D4; }");

    nutLuuTruLichSu = new QPushButton(LanguageManager::instance().getText("BTN_ARCHIVE_DATA"));
    nutLuuTruLichSu->setStyleSheet("QPushButton { background: #3c2424; color: #ff9999; border: 1px solid #5a3333; border-radius: 4px; padding: 8px 15px; } QPushButton:hover { background: #4a2b2b; }");

    footerLayout->addWidget(nhanThongKe);
    footerLayout->addStretch();
    footerLayout->addWidget(nutLuuTruLichSu);
    footerLayout->addWidget(nutXuatBaoCao);
    mainLayout->addLayout(footerLayout);
    
    connect(nutTimKiem, &QPushButton::clicked, this, &WidgetLichSuMuonTra::xuLyTimKiem);
    connect(nutXuatBaoCao, &QPushButton::clicked, this, &WidgetLichSuMuonTra::xuLyXuatBaoCao);
    connect(nutLuuTruLichSu, &QPushButton::clicked, this, [this]() {
        if (QMessageBox::question(this, "Archive", LanguageManager::instance().getText("MSG_ARCHIVE_CONFIRM")) == QMessageBox::Yes) {
            if (DatabaseManager::instance().archiveOldRecords(6)) {
                QMessageBox::information(this, "Success", LanguageManager::instance().getText("MSG_ARCHIVE_SUCCESS"));
                lamMoi();
            }
        }
    });
    connect(chkBaoGomLuuTru, &QCheckBox::toggled, this, &WidgetLichSuMuonTra::xuLyTimKiem);

    connect(oNhapTimKiem, &QLineEdit::textChanged, this, &WidgetLichSuMuonTra::xuLyTimKiem);
    connect(ngayBatDau, &QDateEdit::dateChanged, this, &WidgetLichSuMuonTra::xuLyTimKiem);
    connect(ngayKetThuc, &QDateEdit::dateChanged, this, &WidgetLichSuMuonTra::xuLyTimKiem);
    connect(hopChonTrangThai, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WidgetLichSuMuonTra::xuLyTimKiem);
    
    // --- REALTIME EVENT BUS CONNECTIONS ---
    // Update when any borrow/return happens in the system
    connect(&EventBus::instance(), &EventBus::muonTraAdded, this, [this](int, const QString&){ 
        if (this->isVisible()) this->xuLyTimKiem(); 
    });
    connect(&EventBus::instance(), &EventBus::muonTraRemoved, this, [this](int, const QString&){ 
        if (this->isVisible()) this->xuLyTimKiem(); 
    });
    connect(&EventBus::instance(), &EventBus::muonTraUpdated, this, [this](int, const QString&){ 
        if (this->isVisible()) this->xuLyTimKiem(); 
    });
    
    // Also update on global data change
    connect(&EventBus::instance(), &EventBus::dataChanged, this, [this](){
        if (this->isVisible()) this->xuLyTimKiem();
    });

    // Initial Load
    // lamMoi(); // Moved to showEvent
}

void WidgetLichSuMuonTra::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    lamMoi();
}

QString WidgetLichSuMuonTra::xayDungCauLenhSQL() {
    QString sql = "SELECT mt.ngay_muon, mt.ngay_tra, (dg.ho || ' ' || dg.ten) as doc_gia, ds.ten_sach, mt.tien_phat, "
                  "CASE mt.trang_thai WHEN 0 THEN 'Đang mượn' WHEN 1 THEN 'Đã trả' WHEN 2 THEN 'Làm mất' END as trang_thai, "
                  "CASE mt.da_thanh_toan WHEN 0 THEN 'Chưa' ELSE 'Rồi' END as thanh_toan "
                  "FROM muon_tra mt "
                  "JOIN doc_gia dg ON mt.ma_the = dg.ma_the "
                  "JOIN ban_sao bs ON mt.ban_sao_id = bs.id "
                  "JOIN dau_sach ds ON bs.dau_sach_id = ds.id "
                  "WHERE 1=1 ";

    if (!oNhapTimKiem->text().isEmpty()) {
        sql += QString("AND (dg.ho || ' ' || dg.ten LIKE '%%1%' OR dg.ma_the LIKE '%%1%' OR ds.ten_sach LIKE '%%1%') ")
               .arg(oNhapTimKiem->text());
    }

    sql += QString("AND mt.ngay_muon >= '%1' AND mt.ngay_muon <= '%2' ")
           .arg(ngayBatDau->date().toString("yyyy-MM-dd"))
           .arg(ngayKetThuc->date().toString("yyyy-MM-dd"));

    int status = hopChonTrangThai->currentData().toInt();
    if (status != -1) {
        sql += QString("AND mt.trang_thai = %1 ").arg(status);
    }

    sql += "ORDER BY mt.ngay_muon DESC";
    return sql;
}

void WidgetLichSuMuonTra::lamMoi() {
    xuLyTimKiem();
}

void WidgetLichSuMuonTra::xuLyTimKiem() {
    QSqlQuery q;
    QString dateIso = "SUBSTR(%1, 7, 4) || '-' || SUBSTR(%1, 4, 2) || '-' || SUBSTR(%1, 1, 2)";
    
    // Prepare filter clause first
    QString filter = "";
    if (!oNhapTimKiem->text().isEmpty()) {
        filter = "AND (dg.ho || ' ' || dg.ten LIKE :search OR dg.ma_the LIKE :search OR ds.ten_sach LIKE :search) ";
    }
    
    int status = hopChonTrangThai->currentData().toInt();
    if (status != -1) {
        filter += QString("AND mt.trang_thai = %1 ").arg(status);
    }

    auto generateSql = [&](const QString& tableName) {
        // Robust query using LEFT JOIN and redundancy for deleted master data
        return QString("SELECT mt.ngay_muon, mt.ngay_tra, "
                      "COALESCE(dg.ho || ' ' || dg.ten, 'Độc giả #' || mt.ma_the), "
                      "COALESCE(ds.ten_sach, 'Sách #' || COALESCE(NULLIF(mt.ma_sach, ''), bs.ma_sach, 'K.Xác Định')), "
                      "mt.tien_phat, "
                      "mt.trang_thai, " // Original status ID for proxy
                      "mt.da_thanh_toan, " 
                      "mt.ma_the, COALESCE(NULLIF(mt.ma_sach, ''), bs.ma_sach), "
                      "mt.da_thanh_toan as paid_bit, mt.tien_hoan_tra " // Added bits for proxy
                      "FROM %1 mt "
                      "LEFT JOIN doc_gia dg ON mt.ma_the = dg.ma_the "
                      "LEFT JOIN ban_sao bs ON mt.ban_sao_id = bs.id "
                      "LEFT JOIN dau_sach ds ON bs.dau_sach_id = ds.id "
                      "WHERE %2 >= :start AND %2 <= :end %3")
                      .arg(tableName)
                      .arg(dateIso.arg("mt.ngay_muon"))
                      .arg(filter); // Inject filter here
    };

    QString baseSql = generateSql("muon_tra");
    if (chkBaoGomLuuTru->isChecked()) {
        baseSql = "(" + baseSql + ") UNION ALL (" + generateSql("muon_tra_archive") + ")";
    }

    q.prepare(baseSql + " ORDER BY 1 DESC"); // Order by column 1 (ngay_muon)
    q.bindValue(":start", ngayBatDau->date().toString("yyyy-MM-dd"));
    q.bindValue(":end", ngayKetThuc->date().toString("yyyy-MM-dd"));
    if (!oNhapTimKiem->text().isEmpty()) {
        q.bindValue(":search", "%" + oNhapTimKiem->text() + "%");
    }

    if (!q.exec()) {
        qDebug() << "[History] SQL Error:" << q.lastError().text();
        return;
    }

    modelLichSu->setQuery(q);

    // Proxy Setup
    LichSuProxyModel *proxy = new LichSuProxyModel(this);
    proxy->setSourceModel(modelLichSu);
    bangLichSu->setModel(proxy);

    // Set headers
    modelLichSu->setHeaderData(0, Qt::Horizontal, LanguageManager::instance().getText("COL_NGAY_MUON"));
    modelLichSu->setHeaderData(1, Qt::Horizontal, LanguageManager::instance().getText("COL_NGAY_TRA"));
    modelLichSu->setHeaderData(2, Qt::Horizontal, LanguageManager::instance().getText("COL_DOC_GIA"));
    modelLichSu->setHeaderData(3, Qt::Horizontal, LanguageManager::instance().getText("COL_TEN_SACH"));
    modelLichSu->setHeaderData(4, Qt::Horizontal, LanguageManager::instance().getText("COL_TIEN_PHAT"));
    modelLichSu->setHeaderData(5, Qt::Horizontal, LanguageManager::instance().getText("COL_TRANG_THAI"));
    modelLichSu->setHeaderData(6, Qt::Horizontal, LanguageManager::instance().getText("COL_THANH_TOAN"));

    QSqlQuery sumQ;
    auto generateSumSql = [&](const QString& tableName) {
        return QString("SELECT tien_phat FROM %1 mt "
                      "LEFT JOIN doc_gia dg ON mt.ma_the = dg.ma_the "
                      "LEFT JOIN ban_sao bs ON mt.ban_sao_id = bs.id "
                      "LEFT JOIN dau_sach ds ON bs.dau_sach_id = ds.id "
                      "WHERE %2 >= :start AND %2 <= :end " + filter)
                      .arg(tableName).arg(dateIso.arg("mt.ngay_muon"));
    };
    
    QString sumSql = "SELECT SUM(tien_phat) FROM (" + generateSumSql("muon_tra");
    if (chkBaoGomLuuTru->isChecked()) {
        sumSql += " UNION ALL " + generateSumSql("muon_tra_archive");
    }
    sumSql += ") AS sub_total";
    
    sumQ.prepare(sumSql);
    sumQ.bindValue(":start", ngayBatDau->date().toString("yyyy-MM-dd"));
    sumQ.bindValue(":end", ngayKetThuc->date().toString("yyyy-MM-dd"));
    if (!oNhapTimKiem->text().isEmpty()) {
        sumQ.bindValue(":search", "%" + oNhapTimKiem->text() + "%");
    }

    long long totalPhat = 0;
    if (sumQ.exec() && sumQ.next()) {
        totalPhat = sumQ.value(0).toLongLong();
    }
    
    nhanThongKe->setText(QString("%1: %2 VND")
        .arg(LanguageManager::instance().getText("LBL_TOTAL_FINES"))
        .arg(QLocale(QLocale::Vietnamese).toString(totalPhat)));
    
    // UI Polish: Hide internal columns
    bangLichSu->setColumnHidden(6, true); // da_thanh_toan (internal)
    bangLichSu->setColumnHidden(7, true); // ma_the
    bangLichSu->setColumnHidden(8, true); // ma_sach
    bangLichSu->setColumnHidden(9, true); // paid_bit
    bangLichSu->setColumnHidden(10, true); // tien_hoan_tra
}

void WidgetLichSuMuonTra::xuLyXuatBaoCao() {
    // 1. Calculate Summary Stats based on CURRENT FILTER
    double totalTx = modelLichSu->rowCount();
    double returned = 0;
    double lost = 0;
    double revenue = 0;
    
    for (int i = 0; i < totalTx; i++) {
         int status = modelLichSu->data(modelLichSu->index(i, 5)).toInt();
         double fine = modelLichSu->data(modelLichSu->index(i, 4)).toDouble();
         bool isPaid = modelLichSu->data(modelLichSu->index(i, 9)).toInt() == 1;
         
         if (status == 1) returned++; // Returned
         if (status == 2 || status == 3) lost++; // Lost
         
         if (isPaid) revenue += fine;
    }
    
    QMap<QString, double> stats;
    stats["total_tx"] = totalTx;
    stats["returned"] = returned;
    stats["lost"] = lost;
    stats["revenue"] = revenue;

    QString range = QString("%1 - %2")
        .arg(ngayBatDau->date().toString("dd/MM/yyyy"))
        .arg(ngayKetThuc->date().toString("dd/MM/yyyy"));

    PrintHelper::inLichSuMuonTra(modelLichSu, range, stats, this);
}

void WidgetLichSuMuonTra::xuLyMenuChuotPhai(const QPoint &pos) {
    QModelIndex index = bangLichSu->indexAt(pos);
    if (!index.isValid()) return;

    int status = modelLichSu->data(modelLichSu->index(index.row(), 5)).toInt();
    long long fine = modelLichSu->data(modelLichSu->index(index.row(), 4)).toLongLong();
    bool isPaid = modelLichSu->data(modelLichSu->index(index.row(), 9)).toInt() == 1;
    long long refundAmount = modelLichSu->data(modelLichSu->index(index.row(), 10)).toLongLong();

    QMenu menu(this);
    
    // Case 1: Need Payment (Lost-Wait-Pay or Returned-Debt)
    bool needPay = (status == 2) || (status == 1 && fine > 0 && !isPaid);
    
    // Case 2: Need Refund (Lost-Found-Wait-Refund)
    bool needRefund = (status == 4);

    if (needPay || needRefund) {
        QString actionText = needRefund ? LanguageManager::instance().getText("BTN_CONFIRM_REFUND") 
                                         : LanguageManager::instance().getText("BTN_PAY");
        
        QAction *actFin = menu.addAction(QIcon(needRefund ? ":/icons/exchange.svg" : ":/icons/payment.svg"), actionText);
        
        connect(actFin, &QAction::triggered, this, [this, index, status, fine, refundAmount, needRefund]() {
            int maThe = modelLichSu->data(modelLichSu->index(index.row(), 7)).toInt();
            QString maSach = modelLichSu->data(modelLichSu->index(index.row(), 8)).toString();
            
            extern PTRDG rootDocGia;
            PTRDG reader = timDocGia(rootDocGia, maThe);
            if (reader) {
                // Bridge SQL model to FinanceHelper Context
                FinanceHelper::PaymentContext ctx;
                ctx.maSach = maSach;
                ctx.amount = needRefund ? refundAmount : fine;
                ctx.isRefund = needRefund;
                ctx.canProcess = true;
                
                if (needRefund) ctx.reason = QString("Hoàn tiền (Tìm thấy sách %1)").arg(maSach);
                else if (status == 2) ctx.reason = QString("Đền bù sách mất: %1").arg(maSach);
                else ctx.reason = QString("Phạt quá hạn: %1").arg(maSach);

                if (FinanceHelper::processPaymentFlow(reader, ctx, this)) {
                    lamMoi();
                }
            }
        });
    }
    
    if (!menu.isEmpty()) menu.exec(bangLichSu->viewport()->mapToGlobal(pos));
}

void WidgetLichSuMuonTra::capNhatNgonNgu() {
    oNhapTimKiem->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_SEARCH_HISTORY"));
    nutTimKiem->setText(LanguageManager::instance().getText("BTN_SEARCH"));
    nutXuatBaoCao->setText(LanguageManager::instance().getText("BTN_EXPORT_FINANCE"));
    nutLuuTruLichSu->setText(LanguageManager::instance().getText("BTN_ARCHIVE_DATA"));
    chkBaoGomLuuTru->setText(LanguageManager::instance().getText("CHK_INCLUDE_ARCHIVE"));
    
    // Update Combobox items (rebuild)
    hopChonTrangThai->clear();
    hopChonTrangThai->addItem(LanguageManager::instance().getText("FILTER_ALL"), -1);
    hopChonTrangThai->addItem(LanguageManager::instance().getText("STATUS_BORROWING"), 0);
    hopChonTrangThai->addItem(LanguageManager::instance().getText("STATUS_RETURNED"), 1);
    hopChonTrangThai->addItem(LanguageManager::instance().getText("STATUS_LOST"), 2);

    xuLyTimKiem();
}
