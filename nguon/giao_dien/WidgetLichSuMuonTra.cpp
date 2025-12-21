/**
 * @file WidgetLichSuMuonTra.cpp
 * @path nguon/giao_dien/WidgetLichSuMuonTra.cpp
 * @description Triển khai Widget xem lịch sử mượn trả và báo cáo tài chính.
 */
#include "giao_dien/WidgetLichSuMuonTra.h"
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
#include "giao_dien/hop_thoai/HopThoaiThanhToan.h"
#include "models/DocGia.h"
#include <QDebug>
#include <QMessageBox>

WidgetLichSuMuonTra::WidgetLichSuMuonTra(QWidget *parent) : QWidget(parent) {
    thieLapGiaoDien();
    lamMoi();
    
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

    connect(oNhapTimKiem, &QLineEdit::returnPressed, this, &WidgetLichSuMuonTra::xuLyTimKiem);
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
    
    auto generateSql = [&](const QString& tableName) {
        return QString("SELECT mt.ngay_muon, mt.ngay_tra, (dg.ho || ' ' || dg.ten) as doc_gia, ds.ten_sach, mt.tien_phat, "
                      "CASE mt.trang_thai WHEN 0 THEN 'Đang mượn' WHEN 1 THEN 'Đã trả' WHEN 2 THEN 'Làm mất' END as trang_thai_text, "
                      "CASE mt.da_thanh_toan WHEN 0 THEN 'Chưa' ELSE 'Rồi' END as thanh_toan_text, "
                      "mt.ma_the, bs.ma_sach "
                      "FROM %1 mt "
                      "JOIN doc_gia dg ON mt.ma_the = dg.ma_the "
                      "JOIN ban_sao bs ON mt.ban_sao_id = bs.id "
                      "JOIN dau_sach ds ON bs.dau_sach_id = ds.id "
                      "WHERE %2 >= :start AND %2 <= :end ")
                      .arg(tableName).arg(dateIso.arg("mt.ngay_muon"));
    };

    QString baseSql = generateSql("muon_tra");
    if (chkBaoGomLuuTru->isChecked()) {
        baseSql = "(" + baseSql + ") UNION ALL (" + generateSql("muon_tra_archive") + ")";
    }

    QString filter = "";
    if (!oNhapTimKiem->text().isEmpty()) {
        filter = "AND (dg.ho || ' ' || dg.ten LIKE :search OR dg.ma_the LIKE :search OR ds.ten_sach LIKE :search) ";
    }

    int status = hopChonTrangThai->currentData().toInt();
    if (status != -1) {
        filter += QString("AND mt.trang_thai = %1 ").arg(status);
    }

    q.prepare(baseSql + filter + "ORDER BY mt.ngay_muon DESC");
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
                      "JOIN doc_gia dg ON mt.ma_the = dg.ma_the "
                      "JOIN ban_sao bs ON mt.ban_sao_id = bs.id "
                      "JOIN dau_sach ds ON bs.dau_sach_id = ds.id "
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
    
    // UI Polish: Hide internal columns if they are shown
    bangLichSu->setColumnHidden(7, true);
    bangLichSu->setColumnHidden(8, true);
}

void WidgetLichSuMuonTra::xuLyXuatBaoCao() {
    QString start = ngayBatDau->date().toString("dd/MM/yyyy");
    QString end = ngayKetThuc->date().toString("dd/MM/yyyy");
    QString tieuDe = LanguageManager::instance().getText("BTN_EXPORT_FINANCE");
    PrintHelper::inBaoCaoTaiChinh(start, end, tieuDe, modelLichSu, this);
}

void WidgetLichSuMuonTra::xuLyMenuChuotPhai(const QPoint &pos) {
    QModelIndex index = bangLichSu->indexAt(pos);
    if (!index.isValid()) return;

    // Check if Paid (Column 6 contains status text "Chưa" or "Rồi")
    QString thanhToan = modelLichSu->data(modelLichSu->index(index.row(), 6)).toString();
    long long fine = modelLichSu->data(modelLichSu->index(index.row(), 4)).toLongLong();

    QMenu menu(this);
    if (thanhToan.contains("Chưa") && fine > 0) {
        QAction *actPay = menu.addAction(QIcon(":/icons/exchange.svg"), LanguageManager::instance().getText("COL_THANH_TOAN"));
        connect(actPay, &QAction::triggered, this, [this, index]() {
            int maThe = modelLichSu->data(modelLichSu->index(index.row(), 7)).toInt();
            QString maSach = modelLichSu->data(modelLichSu->index(index.row(), 8)).toString();
            long long fineAmount = modelLichSu->data(modelLichSu->index(index.row(), 4)).toLongLong();
            
            // Re-fetch objects for payment dialog
            extern PTRDG rootDocGia;
            PTRDG reader = timDocGia(rootDocGia, maThe);
            if (reader) {
                QString reason = LanguageManager::instance().getText("COL_TIEN_PHAT") + ": " + maSach;
                HopThoaiThanhToan dialog(reader, reason, fineAmount, this);
                if (dialog.exec() == QDialog::Accepted) {
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
