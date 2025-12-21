/**
 * @file ManHinhKiemKe.cpp
 * @path nguon/giao_dien/hop_thoai/ManHinhKiemKe.cpp
 * @description Dialog quản lý kiểm kê kho sách bằng mã vạch.
 */
#include "giao_dien/hop_thoai/ManHinhKiemKe.h"
#include "utils/LanguageManager.h"
#include "utils/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include "utils/PrintHelper.h"
#include <QApplication>
#include <QMenu>
#include <QTimer>
#include <QKeyEvent>

ManHinhKiemKe::ManHinhKiemKe(QWidget *parent) : QDialog(parent) {
    setWindowTitle(LanguageManager::instance().getText("TITLE_INVENTORY"));
    resize(1000, 700);
    
    thieLapGiaoDien();
    taiDuLieuHeThong();
    
    QTimer::singleShot(100, oNhapMaVach, SLOT(setFocus()));
}

ManHinhKiemKe::~ManHinhKiemKe() {}

void ManHinhKiemKe::thieLapGiaoDien() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    setStyleSheet("QDialog { background-color: #1a1d23; color: white; }");

    // Header
    QLabel *title = new QLabel(LanguageManager::instance().getText("TITLE_INVENTORY_HEADER"));
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #5D9CEC;");
    mainLayout->addWidget(title);

    // Scan Area
    QHBoxLayout *scanLayout = new QHBoxLayout();
    QLabel *lblScan = new QLabel(LanguageManager::instance().getText("LBL_SCAN_BOOK") + ":");
    oNhapMaVach = new QLineEdit();
    oNhapMaVach->setPlaceholderText(LanguageManager::instance().getText("PLACEHOLDER_SCAN_INVENTORY"));
    oNhapMaVach->setMinimumHeight(40);
    oNhapMaVach->setStyleSheet("QLineEdit { font-size: 16px; background: #252830; border: 2px solid #333; border-radius: 4px; padding: 0 10px; color: white; } QLineEdit:focus { border-color: #0078D4; }");
    
    scanLayout->addWidget(lblScan);
    scanLayout->addWidget(oNhapMaVach);
    mainLayout->addLayout(scanLayout);

    // --- SECTION 1: SYSTEM OVERVIEW ---
    QLabel *lblOverview = new QLabel(LanguageManager::instance().getText("LBL_OVERVIEW"));
    lblOverview->setStyleSheet("color: #888; font-weight: bold;");
    mainLayout->addWidget(lblOverview);

    QFrame *overviewFrame = new QFrame();
    overviewFrame->setStyleSheet("QFrame { background: #1e1e2d; border: 1px solid #333; border-radius: 8px; }");
    QHBoxLayout *overLayout = new QHBoxLayout(overviewFrame);
    
    auto createMiniStat = [](QLabel* &lbl, QString title) {
        QVBoxLayout *v = new QVBoxLayout();
        QLabel *t = new QLabel(title);
        t->setStyleSheet("color: #666; font-size: 10px;");
        lbl = new QLabel("0");
        lbl->setStyleSheet("color: #E0E0E0; font-size: 16px; font-weight: bold;");
        v->addWidget(t);
        v->addWidget(lbl);
        return v;
    };

    overLayout->addLayout(createMiniStat(nhanTongDauSach, LanguageManager::instance().getText("LBL_TOTAL_TITLES_INV")));
    overLayout->addLayout(createMiniStat(nhanTongBanSao, LanguageManager::instance().getText("LBL_TOTAL_COPIES_INV")));
    overLayout->addLayout(createMiniStat(nhanDangChoMuon, LanguageManager::instance().getText("LBL_BORROWED_INV")));
    overLayout->addLayout(createMiniStat(nhanDangTrongKho, LanguageManager::instance().getText("LBL_IN_STOCK_INV")));
    overLayout->addLayout(createMiniStat(nhanChuaQuet, LanguageManager::instance().getText("LBL_MISPLACED_INV"))); // This will be updated later
    mainLayout->addWidget(overviewFrame);

    // --- SECTION 2: AUDIT AREA ---
    QLabel *lblAudit = new QLabel(LanguageManager::instance().getText("LBL_AUDIT_RESULTS"));
    lblAudit->setStyleSheet("color: #888; font-weight: bold; margin-top: 10px;");
    mainLayout->addWidget(lblAudit);

    // Statistics Bar (Visual Dashboard - Actual Results)
    QHBoxLayout *statsLayout = new QHBoxLayout();
    auto createStatCard = [this](QLabel* &lbl, QString title, QString color) {
        QFrame *card = new QFrame();
        card->setStyleSheet(QString("QFrame { background: #252830; border: 1px solid #333; border-radius: 8px; padding: 10px; }"));
        QVBoxLayout *vbox = new QVBoxLayout(card);
        
        QLabel *titleL = new QLabel(title);
        titleL->setStyleSheet("color: #888; font-size: 11px; text-transform: uppercase;");
        
        lbl = new QLabel("0");
        lbl->setStyleSheet(QString("color: %1; font-size: 24px; font-weight: bold;").arg(color));
        
        vbox->addWidget(titleL);
        vbox->addWidget(lbl);
        return card;
    };

    statsLayout->addWidget(createStatCard(nhanTongHeThong, LanguageManager::instance().getText("STAT_TOTAL_SYSTEM"), "#E0E0E0"));
    statsLayout->addWidget(createStatCard(nhanDaQuet, LanguageManager::instance().getText("STAT_SCANNED"), "#4CAF50")); // Khớp
    statsLayout->addWidget(createStatCard(nhanSaiTrangThai, LanguageManager::instance().getText("STAT_WRONG_STATUS"), "#a78bfa")); // Sai trạng thái
    statsLayout->addWidget(createStatCard(nhanSaiViTri, LanguageManager::instance().getText("STAT_UNKNOWN_CODE"), "#fb923c")); // Mã lạ
    mainLayout->addLayout(statsLayout);

    // Table
    bangKetQua = new QTableWidget(0, 3);
    bangKetQua->setHorizontalHeaderLabels({
        LanguageManager::instance().getText("COL_BOOK_CODE"),
        LanguageManager::instance().getText("COL_STATUS"),
        LanguageManager::instance().getText("COL_NOTE")
    });
    bangKetQua->horizontalHeader()->setStretchLastSection(true);
    bangKetQua->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangKetQua->setStyleSheet(
        "QTableWidget { background-color: #1e1e2d; color: #E0E0E0; gridline-color: #333; border: 1px solid #333; }"
        "QHeaderView::section { background-color: #252534; color: #5D9CEC; padding: 5px; border: 1px solid #333; font-weight: bold; }"
    );
    mainLayout->addWidget(bangKetQua);

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    nutKetThuc = new QPushButton(LanguageManager::instance().getText("BTN_FINISH_INVENTORY"));
    nutKetThuc->setStyleSheet("QPushButton { background-color: #2D2D30; color: white; border-radius: 4px; padding: 10px 20px; }");
    
    nutXuatBaoCao = new QPushButton(LanguageManager::instance().getText("BTN_EXPORT_INVENTORY_REPORT"));
    nutXuatBaoCao->setStyleSheet("QPushButton { background-color: #0078D4; color: white; border-radius: 4px; padding: 10px 20px; font-weight: bold; }");

    btnLayout->addStretch();
    btnLayout->addWidget(nutKetThuc);
    btnLayout->addWidget(nutXuatBaoCao);
    mainLayout->addLayout(btnLayout);

    // Connections
    connect(oNhapMaVach, &QLineEdit::returnPressed, this, &ManHinhKiemKe::xuLyQuetMa);
    connect(nutKetThuc, &QPushButton::clicked, this, &ManHinhKiemKe::xuLyKetThuc);
    connect(nutXuatBaoCao, &QPushButton::clicked, this, &ManHinhKiemKe::xuLyXuatBaoCao);
}

void ManHinhKiemKe::taiDuLieuHeThong() {
    setMaHeThong.clear();
    setMaDangMuon.clear();
    
    // 1. Get Summary Stats
    QSqlQuery qStats;
    qStats.exec("SELECT COUNT(*) FROM dau_sach");
    if (qStats.next()) nhanTongDauSach->setText(qStats.value(0).toString());
    
    qStats.exec("SELECT COUNT(*) FROM ban_sao WHERE trang_thai != 2");
    if (qStats.next()) nhanTongBanSao->setText(qStats.value(0).toString());

    // 2. Get All Expected Books (Not lost)
    QSqlQuery q("SELECT ma_sach, trang_thai FROM ban_sao WHERE trang_thai != 2");
    while (q.next()) {
        QString ma = q.value(0).toString();
        int tt = q.value(1).toInt();
        if (tt == 1) { // DANG_MUON
            setMaDangMuon.insert(ma);
        } else {
            setMaHeThong.insert(ma); // Expected in stock
        }
    }
    
    nhanDangChoMuon->setText(QString::number(setMaDangMuon.size()));
    nhanDangTrongKho->setText(QString::number(setMaHeThong.size()));
    
    capNhatThongKe();
}

void ManHinhKiemKe::xuLyQuetMa() {
    QString ma = oNhapMaVach->text().trimmed();
    if (ma.isEmpty()) return;
    oNhapMaVach->clear();

    if (setMaDaQuet.contains(ma)) {
        // Skip duplicate scans
        return;
    }

    int row = bangKetQua->rowCount();
    bangKetQua->insertRow(row);
    bangKetQua->setItem(row, 0, new QTableWidgetItem(ma));

    if (setMaHeThong.contains(ma)) {
        setMaDaQuet.insert(ma);
        bangKetQua->setItem(row, 1, new QTableWidgetItem("OK"));
        bangKetQua->item(row, 1)->setForeground(QBrush(QColor("#4CAF50")));
        bangKetQua->setItem(row, 2, new QTableWidgetItem(LanguageManager::instance().getText("NOTE_MATCHED")));
        QApplication::beep(); // Success sound
    } else if (setMaDangMuon.contains(ma)) {
        listSaiTrangThai.append(ma);
        bangKetQua->setItem(row, 1, new QTableWidgetItem("WARN"));
        bangKetQua->item(row, 1)->setForeground(QBrush(QColor("#a78bfa")));
        bangKetQua->setItem(row, 2, new QTableWidgetItem(LanguageManager::instance().getText("NOTE_WRONG_STATUS")));
        QApplication::beep(); 
    } else {
        listMaLa.append(ma);
        bangKetQua->setItem(row, 1, new QTableWidgetItem("ERR"));
        bangKetQua->item(row, 1)->setForeground(QBrush(QColor("#f43f5e")));
        bangKetQua->setItem(row, 2, new QTableWidgetItem(LanguageManager::instance().getText("NOTE_UNKNOWN_CODE")));
        QApplication::beep(); 
    }
    
    bangKetQua->scrollToBottom();
    capNhatThongKe();
    oNhapMaVach->setFocus();
}

void ManHinhKiemKe::capNhatThongKe() {
    nhanTongHeThong->setText(QString::number(setMaHeThong.size()));
    nhanDaQuet->setText(QString::number(setMaDaQuet.size()));
    nhanChuaQuet->setText(QString::number(setMaHeThong.size() - setMaDaQuet.size()));
    nhanSaiViTri->setText(QString::number(listMaLa.size()));
    nhanSaiTrangThai->setText(QString::number(listSaiTrangThai.size()));
}

void ManHinhKiemKe::xuLyKetThuc() {
    if (setMaDaQuet.size() < setMaHeThong.size()) {
        auto res = QMessageBox::question(this, "Xác Nhận", LanguageManager::instance().getText("MSG_CONFIRM_FINISH_INVENTORY_MISSING"));
        if (res != QMessageBox::Yes) return;
    }
    accept();
}

void ManHinhKiemKe::xuLyXuatBaoCao() {
    QStringList missing;
    for (const QString& ma : setMaHeThong) {
        if (!setMaDaQuet.contains(ma)) {
            missing.append(ma);
        }
    }
    
    QMap<QString, int> stats;
    stats["titles"] = nhanTongDauSach->text().toInt();
    stats["copies"] = nhanTongBanSao->text().toInt();
    stats["borrowed"] = setMaDangMuon.size();
    stats["stock"] = setMaHeThong.size();

    PrintHelper::inBaoCaoKiemKe(missing, listMaLa, listSaiTrangThai, setMaDangMuon.values(), stats, this);
}

void ManHinhKiemKe::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        reject();
    } else {
        QDialog::keyPressEvent(event);
    }
}
