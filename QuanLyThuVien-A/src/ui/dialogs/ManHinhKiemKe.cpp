/**
 * @file ManHinhKiemKe.cpp
 * @path src/ui/dialogs/ManHinhKiemKe.cpp
 * @description Dialog quản lý kiểm kê kho sách bằng mã vạch.
 */
#include "ui/dialogs/ManHinhKiemKe.h"
#include "utils/LanguageManager.h"
#include "utils/AuditLogger.h"
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

    // Progress Bar (System-wide)
    QWidget *progressContainer = new QWidget();
    QVBoxLayout *progressLayout = new QVBoxLayout(progressContainer);
    progressLayout->setContentsMargins(0, 10, 0, 10);
    
    QHBoxLayout *progressInfo = new QHBoxLayout();
    QLabel *lblProgress = new QLabel(LanguageManager::instance().getText("LBL_INVENTORY_PROGRESS"));
    lblProgress->setStyleSheet("color: #E0E0E0; font-weight: bold;");
    nhanPhanTram = new QLabel("0%");
    nhanPhanTram->setStyleSheet("color: #5D9CEC; font-weight: bold;");
    progressInfo->addWidget(lblProgress);
    progressInfo->addStretch();
    progressInfo->addWidget(nhanPhanTram);
    
    thanhTienDo = new QProgressBar();
    thanhTienDo->setRange(0, 100);
    thanhTienDo->setValue(0);
    thanhTienDo->setTextVisible(false);
    thanhTienDo->setFixedHeight(8);
    thanhTienDo->setStyleSheet(
        "QProgressBar { background: #252830; border-radius: 4px; border: none; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4facfe, stop:1 #00f2fe); border-radius: 4px; }"
    );
    
    progressLayout->addLayout(progressInfo);
    progressLayout->addWidget(thanhTienDo);
    mainLayout->addWidget(progressContainer);

    // Filter Buttons
    QHBoxLayout *filterLayout = new QHBoxLayout();
    QLabel *lblFilter = new QLabel(LanguageManager::instance().getText("LBL_FILTER") + ":");
    lblFilter->setStyleSheet("color: #888; margin-right: 10px;");
    
    auto createFilterBtn = [this](QString text, QString objName) {
        QPushButton *btn = new QPushButton(text);
        btn->setCheckable(true);
        btn->setObjectName(objName);
        btn->setStyleSheet(
            "QPushButton { background: #2D2D30; border: none; padding: 5px 15px; border-radius: 15px; color: #888; }"
            "QPushButton:checked { background: #5D9CEC; color: white; }"
        );
        return btn;
    };
    
    nutLocTatCa = createFilterBtn(LanguageManager::instance().getText("FILTER_ALL"), "filterAll");
    nutLocTatCa->setChecked(true);
    nutLocKhop = createFilterBtn(LanguageManager::instance().getText("FILTER_MATCHED"), "filterMatched");
    nutLocLoi = createFilterBtn(LanguageManager::instance().getText("FILTER_ERROR"), "filterError");
    
    QButtonGroup *filterGroup = new QButtonGroup(this);
    filterGroup->addButton(nutLocTatCa);
    filterGroup->addButton(nutLocKhop);
    filterGroup->addButton(nutLocLoi);
    
    filterLayout->addWidget(lblFilter);
    filterLayout->addWidget(nutLocTatCa);
    filterLayout->addWidget(nutLocKhop);
    filterLayout->addWidget(nutLocLoi);
    filterLayout->addStretch();
    mainLayout->addLayout(filterLayout);

    // Table
    bangKetQua = new QTableWidget(0, 5);
    bangKetQua->setHorizontalHeaderLabels({
        LanguageManager::instance().getText("COL_BOOK_CODE"),
        LanguageManager::instance().getText("COL_BOOK_TITLE"),
        LanguageManager::instance().getText("COL_SHELF_LOCATION"),
        LanguageManager::instance().getText("COL_STATUS"),
        LanguageManager::instance().getText("COL_NOTE")
    });
    bangKetQua->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    bangKetQua->setColumnWidth(0, 120);
    bangKetQua->setColumnWidth(2, 100);
    bangKetQua->setColumnWidth(3, 100);

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
    
    connect(nutLocTatCa, &QPushButton::clicked, this, &ManHinhKiemKe::xuLyLoc);
    connect(nutLocKhop, &QPushButton::clicked, this, &ManHinhKiemKe::xuLyLoc);
    connect(nutLocLoi, &QPushButton::clicked, this, &ManHinhKiemKe::xuLyLoc);
}

void ManHinhKiemKe::taiDuLieuHeThong() {
    setMaHeThong.clear();
    setMaDangMuon.clear();
    mapThongTinHeThong.clear();
    
    // 1. Get Summary Stats (Aligned with JOIN)
    QSqlQuery qStats;
    qStats.exec("SELECT COUNT(*) FROM dau_sach");
    if (qStats.next()) nhanTongDauSach->setText(qStats.value(0).toString());
    
    // Fix: Count only copies with valid headers (orphans excluded from valid count)
    qStats.exec("SELECT COUNT(*) FROM ban_sao bs JOIN dau_sach ds ON bs.dau_sach_id = ds.id WHERE bs.trang_thai != 2");
    if (qStats.next()) nhanTongBanSao->setText(qStats.value(0).toString());

    // 2. Orphan Scan (Check for data integrity)
    qStats.exec("SELECT COUNT(*) FROM ban_sao WHERE dau_sach_id NOT IN (SELECT id FROM dau_sach)");
    if (qStats.next()) {
        int orphans = qStats.value(0).toInt();
        if (orphans > 0) {
            // Show warning in Overview or dedicated label
            nhanTongBanSao->setToolTip(QString("Cảnh báo: Có %1 bản sao 'mồ côi' (Không có đầu sách)").arg(orphans));
            nhanTongBanSao->setStyleSheet("color: #FF5252; font-size: 16px; font-weight: bold;"); // Red warning
        }
    }

    // 3. Get All Expected Books (Not lost)
    QSqlQuery q("SELECT bs.ma_sach, bs.trang_thai, ds.ten_sach, bs.vi_tri FROM ban_sao bs "
                "JOIN dau_sach ds ON bs.dau_sach_id = ds.id WHERE bs.trang_thai != 2");
    while (q.next()) {
        QString ma = q.value(0).toString();
        int tt = q.value(1).toInt();
        QString ten = q.value(2).toString();
        QString vt = q.value(3).toString();
        
        mapThongTinHeThong[ma] = {ten, vt, tt};
        
        if (tt == 1) { // DANG_MUON
            setMaDangMuon.insert(ma);
        } else {
            setMaHeThong.insert(ma); // Expected in stock (Available)
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
    
    QString ten = "N/A";
    QString vt = "N/A";
    if (mapThongTinHeThong.contains(ma)) {
        ten = mapThongTinHeThong[ma].ten;
        vt = mapThongTinHeThong[ma].viTri;
    }
    bangKetQua->setItem(row, 1, new QTableWidgetItem(ten));
    bangKetQua->setItem(row, 2, new QTableWidgetItem(vt));

    if (setMaHeThong.contains(ma)) {
        setMaDaQuet.insert(ma);
        bangKetQua->setItem(row, 3, new QTableWidgetItem("OK"));
        bangKetQua->item(row, 3)->setForeground(QBrush(QColor("#4CAF50")));
        bangKetQua->setItem(row, 4, new QTableWidgetItem(LanguageManager::instance().getText("NOTE_MATCHED")));
        QApplication::beep(); // Success sound
        
    } else if (setMaDangMuon.contains(ma)) {
        // IMPROVED: Found a book that technically should be borrowed!
        // This is a "Warning" but also a "Found" event.
        listSaiTrangThai.append(ma);
        bangKetQua->setItem(row, 3, new QTableWidgetItem("FOUND"));
        bangKetQua->item(row, 3)->setForeground(QBrush(QColor("#FF9800"))); // Orange
        bangKetQua->setItem(row, 4, new QTableWidgetItem(LanguageManager::instance().getText("NOTE_BORROWED_BUT_FOUND"))); // New Key Needed
        QApplication::beep(); 
        
    } else {
        // Unknown code (Orphan or Trash)
        listMaLa.append(ma);
        bangKetQua->setItem(row, 3, new QTableWidgetItem("ERR"));
        bangKetQua->item(row, 3)->setForeground(QBrush(QColor("#F44336"))); // Red
        bangKetQua->setItem(row, 4, new QTableWidgetItem(LanguageManager::instance().getText("NOTE_UNKNOWN_CODE")));
        QApplication::beep(); 
    }
    
    bangKetQua->scrollToBottom();
    capNhatThongKe();
    oNhapMaVach->setFocus();
}

void ManHinhKiemKe::capNhatThongKe() {
    int tongPhaiCo = setMaHeThong.size();
    int daQuet = setMaDaQuet.size();
    
    nhanTongHeThong->setText(QString::number(tongPhaiCo));
    nhanDaQuet->setText(QString::number(daQuet));
    nhanChuaQuet->setText(QString::number(tongPhaiCo - daQuet));
    nhanSaiViTri->setText(QString::number(listMaLa.size()));
    nhanSaiTrangThai->setText(QString::number(listSaiTrangThai.size()));
    
    // Update Progress Bar
    if (tongPhaiCo > 0) {
        int pt = (daQuet * 100) / tongPhaiCo;
        thanhTienDo->setValue(pt);
        nhanPhanTram->setText(QString("%1%").arg(pt));
    }
}

void ManHinhKiemKe::xuLyLoc() {
    QString filter = "all";
    if (nutLocKhop->isChecked()) filter = "ok";
    else if (nutLocLoi->isChecked()) filter = "err";
    
    for (int i = 0; i < bangKetQua->rowCount(); ++i) {
        QString status = bangKetQua->item(i, 3)->text();
        if (filter == "all") {
            bangKetQua->setRowHidden(i, false);
        } else if (filter == "ok") {
            bangKetQua->setRowHidden(i, status != "OK");
        } else if (filter == "err") {
            bangKetQua->setRowHidden(i, status == "OK");
        }
    }
}

void ManHinhKiemKe::xuLyKetThuc() {
    if (setMaDaQuet.size() < setMaHeThong.size()) {
        auto res = QMessageBox::question(this, "Xác Nhận", LanguageManager::instance().getText("MSG_CONFIRM_FINISH_INVENTORY_MISSING"));
        if (res != QMessageBox::Yes) return;
    }
    
    AuditLogger::log("INVENTORY", "KHO_SACH", "", 
                     QString("Hoàn hành kiểm kê. Quét %1/%2 cuốn sách.")
                     .arg(setMaDaQuet.size()).arg(setMaHeThong.size()));
                     
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
