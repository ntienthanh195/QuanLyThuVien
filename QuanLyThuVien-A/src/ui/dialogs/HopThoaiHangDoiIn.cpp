#include "dialogs/HopThoaiHangDoiIn.h"
#include "utils/PrintQueueManager.h"
#include "utils/PrintHelper.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QApplication>
#include "utils/LanguageManager.h"

HopThoaiHangDoiIn::HopThoaiHangDoiIn(PTRDG root, QWidget *parent)
    : QDialog(parent), rootDocGia(root)
{
    setWindowTitle(LanguageManager::instance().getText("DIALOG_PRINT_QUEUE_TITLE"));
    resize(600, 500);
    
    taoGiaoDien();
    
    // Connect PrintQueueManager signal
    connect(PrintQueueManager::instance(), &PrintQueueManager::hangDoiThayDoi,
            this, &HopThoaiHangDoiIn::capNhatDanhSach);
    
    capNhatDanhSach();
}

HopThoaiHangDoiIn::~HopThoaiHangDoiIn()
{
}

void HopThoaiHangDoiIn::taoGiaoDien() {
    layoutChinh = new QVBoxLayout(this);
    
    // Tiêu đề
    nhanTieuDe = new QLabel(LanguageManager::instance().getText("HEADER_PRINT_QUEUE"), this);
    nhanTieuDe->setAlignment(Qt::AlignCenter);
    nhanTieuDe->setStyleSheet("font-size: 16pt; font-weight: bold; padding: 10px; background: #3498db; color: white;");
    layoutChinh->addWidget(nhanTieuDe);
    
    // Thông tin
    layoutThongTin = new QHBoxLayout();
    nhanThongTin = new QLabel(LanguageManager::instance().getText("LBL_QUEUE_COUNT").arg(0), this);
    nhanSoTrang = new QLabel(LanguageManager::instance().getText("LBL_QUEUE_PAGES").arg(0).arg(0), this);
    nhanThongTin->setStyleSheet("font-size: 11pt; padding: 5px;");
    nhanSoTrang->setStyleSheet("font-size: 11pt; padding: 5px;");
    
    QLabel* nhanSoCot = new QLabel(LanguageManager::instance().getText("LBL_QUEUE_COLUMNS"), this);
    spinSoCot = new QSpinBox(this);
    spinSoCot->setRange(1, 3);
    spinSoCot->setValue(2);
    spinSoCot->setToolTip(LanguageManager::instance().getText("TOOLTIP_QUEUE_COLUMNS"));
    
    layoutThongTin->addWidget(nhanThongTin);
    layoutThongTin->addWidget(nhanSoTrang);
    layoutThongTin->addStretch();
    layoutThongTin->addWidget(nhanSoCot);
    layoutThongTin->addWidget(spinSoCot);
    layoutChinh->addLayout(layoutThongTin);
    
    // Danh sách
    danhSachDocGia = new QListWidget(this);
    danhSachDocGia->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(danhSachDocGia, &QListWidget::currentRowChanged, this, &HopThoaiHangDoiIn::xuLyChonDocGia);
    layoutChinh->addWidget(danhSachDocGia);
    
    // Buttons
    layoutNut = new QHBoxLayout();
    
    nutThemDocGia = new QPushButton(LanguageManager::instance().getText("BTN_ADD_READER_QUEUE"), this);
    nutThemNhieu = new QPushButton(LanguageManager::instance().getText("BTN_ADD_BATCH_QUEUE"), this);
    nutXoa = new QPushButton(LanguageManager::instance().getText("BTN_DELETE_QUEUE"), this);
    nutXoaTatCa = new QPushButton(LanguageManager::instance().getText("BTN_DELETE_ALL_QUEUE"), this);
    nutInA4 = new QPushButton(LanguageManager::instance().getText("BTN_PRINT_A4"), this);
    nutXuatPDF = new QPushButton(LanguageManager::instance().getText("BTN_EXPORT_PDF"), this);
    nutDong = new QPushButton(LanguageManager::instance().getText("BTN_CLOSE"), this);
    
    nutThemDocGia->setStyleSheet("background: #2ecc71; color: white; padding: 8px; font-weight: bold;");
    nutThemNhieu->setStyleSheet("background: #27ae60; color: white; padding: 8px;");
    nutXoa->setStyleSheet("background: #e74c3c; color: white; padding: 8px;");
    nutXoaTatCa->setStyleSheet("background: #c0392b; color: white; padding: 8px;");
    nutInA4->setStyleSheet("background: #3498db; color: white; padding: 8px; font-weight: bold;");
    nutXuatPDF->setStyleSheet("background: #9b59b6; color: white; padding: 8px;");
    nutDong->setStyleSheet("padding: 8px;");
    
    connect(nutThemDocGia, &QPushButton::clicked, this, &HopThoaiHangDoiIn::xuLyThemDocGia);
    connect(nutThemNhieu, &QPushButton::clicked, this, &HopThoaiHangDoiIn::xuLyThemNhieuDocGia);
    connect(nutXoa, &QPushButton::clicked, this, &HopThoaiHangDoiIn::xuLyXoaDocGia);
    connect(nutXoaTatCa, &QPushButton::clicked, this, &HopThoaiHangDoiIn::xuLyXoaTatCa);
    connect(nutInA4, &QPushButton::clicked, this, &HopThoaiHangDoiIn::xuLyInA4);
    connect(nutXuatPDF, &QPushButton::clicked, this, &HopThoaiHangDoiIn::xuLyXuatPDF);
    connect(nutDong, &QPushButton::clicked, this, &QDialog::accept);
    
    layoutNut->addWidget(nutThemDocGia);
    layoutNut->addWidget(nutThemNhieu);
    layoutNut->addWidget(nutXoa);
    layoutNut->addWidget(nutXoaTatCa);
    layoutNut->addWidget(nutInA4);
    layoutNut->addWidget(nutXuatPDF);
    layoutNut->addStretch();
    layoutNut->addWidget(nutDong);
    
    layoutChinh->addLayout(layoutNut);
}

void HopThoaiHangDoiIn::xuLyThemDocGia() {
    bool ok;
    int maThe = QInputDialog::getInt(this, LanguageManager::instance().getText("TITLE_ADD"), 
                                      LanguageManager::instance().getText("MSG_INPUT_READER_ID"), 
                                      1, 1, 999999, 1, &ok);
    
    if (!ok) return;
    
    // Tìm độc giả trong cây AVL
    extern PTRDG timDocGia(PTRDG root, int mathe);
    PTRDG docGia = timDocGia(rootDocGia, maThe);
    
    if (!docGia) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_ERROR"), LanguageManager::instance().getText("MSG_READER_NOT_FOUND_SIMPLE") + ": " + QString::number(maThe));
        return;
    }
    
    if (docGia->data.TrangThai != 1) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_ERROR"), LanguageManager::instance().getText("MSG_READER_LOCKED"));
        return;
    }
    
    if (PrintQueueManager::instance()->them(docGia)) {
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"), 
                                QString(LanguageManager::instance().getText("MSG_ADDED_READER"))
                                .arg(QString::fromStdString(docGia->data.Ho))
                                .arg(QString::fromStdString(docGia->data.Ten))
                                .arg(docGia->data.MaThe));
    } else {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), LanguageManager::instance().getText("MSG_READER_ALREADY_QUEUED"));
    }
}

void HopThoaiHangDoiIn::xuLyThemNhieuDocGia() {
    if (!rootDocGia) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_ERROR"), LanguageManager::instance().getText("MSG_READER_LIST_EMPTY"));
        return;
    }
    
    // Thu thập tất cả độc giả đang hoạt động
    QList<PTRDG> danhSach;
    
    std::function<void(PTRDG)> duyetCay = [&](PTRDG p) {
        if (!p) return;
        duyetCay(p->left);
        if (p->data.TrangThai == 1 && !PrintQueueManager::instance()->daTonTai(p->data.MaThe)) {
            danhSach.append(p);
        }
        duyetCay(p->right);
    };
    
    duyetCay(rootDocGia);
    
    if (danhSach.isEmpty()) {
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_INFO"), LanguageManager::instance().getText("MSG_ALL_READERS_QUEUED"));
        return;
    }
    
    int ret = QMessageBox::question(this, LanguageManager::instance().getText("TITLE_WARNING"), 
                                      LanguageManager::instance().getText("MSG_CONFIRM_ADD_BATCH").arg(danhSach.size()),
                                      QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        for (PTRDG dg : danhSach) {
            PrintQueueManager::instance()->them(dg);
        }
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"), LanguageManager::instance().getText("MSG_ADDED_COUNT").arg(danhSach.size()));
    }
}

void HopThoaiHangDoiIn::xuLyXoaDocGia() {
    int row = danhSachDocGia->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_ERROR"), LanguageManager::instance().getText("MSG_SELECT_DELETE"));
        return;
    }
    
    PrintQueueManager::instance()->xoa(row);
}

void HopThoaiHangDoiIn::xuLyXoaTatCa() {
    if (PrintQueueManager::instance()->soLuong() == 0) {
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_INFO"), LanguageManager::instance().getText("MSG_QUEUE_EMPTY"));
        return;
    }
    
    int ret = QMessageBox::question(this, LanguageManager::instance().getText("TITLE_WARNING"), 
                                      LanguageManager::instance().getText("MSG_CONFIRM_DELETE_ALL"),
                                      QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        PrintQueueManager::instance()->xoaTatCa();
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"), LanguageManager::instance().getText("MSG_SUCCESS_DELETED_ALL")); // Assume MSG_SUCCESS_DELETED_ALL exists or check next
    }
}

void HopThoaiHangDoiIn::xuLyInA4() {
    QList<PTRDG> danhSach = PrintQueueManager::instance()->layDanhSach();
    
    if (danhSach.isEmpty()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_ERROR"), LanguageManager::instance().getText("MSG_QUEUE_EMPTY_ADD"));
        return;
    }
    
    int soCot = spinSoCot->value();
    int soTrang = (danhSach.size() + (soCot * 5) - 1) / (soCot * 5);
    
    int ret = QMessageBox::question(this, LanguageManager::instance().getText("TITLE_INFO"), 
                                      LanguageManager::instance().getText("MSG_CONFIRM_PRINT_A4")
                                      .arg(danhSach.size())
                                      .arg(soTrang)
                                      .arg(soCot),
                                      QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (PrintHelper::inA4NhieuThe(danhSach, soCot, this)) {
            QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"), LanguageManager::instance().getText("MSG_PRINT_SENT"));
        }
    }
}

void HopThoaiHangDoiIn::xuLyXuatPDF() {
    QList<PTRDG> danhSach = PrintQueueManager::instance()->layDanhSach();
    
    if (danhSach.isEmpty()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_ERROR"), LanguageManager::instance().getText("MSG_QUEUE_EMPTY"));
        return;
    }
    
    // Instead of forcing PDF here, we use the standard helper which offers PDF option.
    // Ideally we would directly save to PDF, but since logic is encapsulated in Helper,
    // we delegate to it. The user can simply click "Lưu PDF" in the resulting dialog.
    // This ensures consistency.
    int soCot = spinSoCot->value();
    PrintHelper::inA4NhieuThe(danhSach, soCot, this);
}

void HopThoaiHangDoiIn::xuLyChonDocGia(int hang) {
    Q_UNUSED(hang);
    // Có thể thêm hiển thị preview ở đây
}

void HopThoaiHangDoiIn::capNhatDanhSach() {
    danhSachDocGia->clear();
    
    QList<PTRDG> danhSach = PrintQueueManager::instance()->layDanhSach();
    
    for (int i = 0; i < danhSach.size(); ++i) {
        PTRDG dg = danhSach[i];
        QString hoTen = QString::fromStdString(dg->data.Ho) + " " + QString::fromStdString(dg->data.Ten);
        QString item = QString("%1. [%2] %3 - %4")
                           .arg(i + 1)
                           .arg(dg->data.MaThe)
                           .arg(hoTen)
                           .arg(dg->data.Phai ? "Nữ" : "Nam");
        danhSachDocGia->addItem(item);
    }
    
    capNhatThongTin();
}

void HopThoaiHangDoiIn::capNhatThongTin() {
    int soThe = PrintQueueManager::instance()->soLuong();
    int soCot = spinSoCot->value();
    int soTrang = soThe > 0 ? (soThe + (soCot * 5) - 1) / (soCot * 5) : 0;
    
    nhanThongTin->setText(LanguageManager::instance().getText("LBL_QUEUE_COUNT").arg(soThe));
    nhanSoTrang->setText(LanguageManager::instance().getText("LBL_QUEUE_PAGES").arg(soTrang).arg(soCot));
    
    // Enable/disable buttons
    bool coThe = soThe > 0;
    nutXoa->setEnabled(coThe);
    nutXoaTatCa->setEnabled(coThe);
    nutInA4->setEnabled(coThe);
    nutXuatPDF->setEnabled(coThe);
}
