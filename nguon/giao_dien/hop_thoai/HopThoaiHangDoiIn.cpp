#include "hop_thoai/HopThoaiHangDoiIn.h"
#include "utils/PrintQueueManager.h"
#include "utils/PrintHelper.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QApplication>

HopThoaiHangDoiIn::HopThoaiHangDoiIn(PTRDG root, QWidget *parent)
    : QDialog(parent), rootDocGia(root)
{
    setWindowTitle("Hàng Đợi In Thẻ Hàng Loạt");
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
    nhanTieuDe = new QLabel("📋 HÀNG ĐỢI IN THẺ ĐỘC GIẢ", this);
    nhanTieuDe->setAlignment(Qt::AlignCenter);
    nhanTieuDe->setStyleSheet("font-size: 16pt; font-weight: bold; padding: 10px; background: #3498db; color: white;");
    layoutChinh->addWidget(nhanTieuDe);
    
    // Thông tin
    layoutThongTin = new QHBoxLayout();
    nhanThongTin = new QLabel("Số thẻ: 0", this);
    nhanSoTrang = new QLabel("Số trang A4: 0", this);
    nhanThongTin->setStyleSheet("font-size: 11pt; padding: 5px;");
    nhanSoTrang->setStyleSheet("font-size: 11pt; padding: 5px;");
    
    QLabel* nhanSoCot = new QLabel("Số cột:", this);
    spinSoCot = new QSpinBox(this);
    spinSoCot->setRange(1, 3);
    spinSoCot->setValue(2);
    spinSoCot->setToolTip("Số cột trên mỗi trang A4 (1-3)");
    
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
    
    nutThemDocGia = new QPushButton("➕ Thêm độc giả", this);
    nutThemNhieu = new QPushButton("➕➕ Thêm nhiều", this);
    nutXoa = new QPushButton("🗑️ Xóa", this);
    nutXoaTatCa = new QPushButton("🗑️ Xóa tất cả", this);
    nutInA4 = new QPushButton("🖨️ In A4", this);
    nutXuatPDF = new QPushButton("📄 Xuất PDF", this);
    nutDong = new QPushButton("❌ Đóng", this);
    
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
    int maThe = QInputDialog::getInt(this, "Thêm độc giả", 
                                      "Nhập mã thẻ độc giả:", 
                                      1, 1, 999999, 1, &ok);
    
    if (!ok) return;
    
    // Tìm độc giả trong cây AVL
    extern PTRDG timDocGia(PTRDG root, int mathe);
    PTRDG docGia = timDocGia(rootDocGia, maThe);
    
    if (!docGia) {
        QMessageBox::warning(this, "Lỗi", "Không tìm thấy độc giả với mã thẻ: " + QString::number(maThe));
        return;
    }
    
    if (docGia->data.TrangThai != 1) {
        QMessageBox::warning(this, "Lỗi", "Độc giả này đã bị khóa!");
        return;
    }
    
    if (PrintQueueManager::instance()->them(docGia)) {
        QMessageBox::information(this, "Thành công", 
                                QString("Đã thêm: %1 %2 (Mã: %3)")
                                .arg(QString::fromStdString(docGia->data.Ho))
                                .arg(QString::fromStdString(docGia->data.Ten))
                                .arg(docGia->data.MaThe));
    } else {
        QMessageBox::warning(this, "Cảnh báo", "Độc giả này đã có trong hàng đợi!");
    }
}

void HopThoaiHangDoiIn::xuLyThemNhieuDocGia() {
    if (!rootDocGia) {
        QMessageBox::warning(this, "Lỗi", "Danh sách độc giả trống!");
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
        QMessageBox::information(this, "Thông báo", "Tất cả độc giả đang hoạt động đã có trong hàng đợi!");
        return;
    }
    
    int ret = QMessageBox::question(this, "Xác nhận", 
                                      QString("Thêm %1 độc giả vào hàng đợi?").arg(danhSach.size()),
                                      QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        for (PTRDG dg : danhSach) {
            PrintQueueManager::instance()->them(dg);
        }
        QMessageBox::information(this, "Thành công", QString("Đã thêm %1 độc giả!").arg(danhSach.size()));
    }
}

void HopThoaiHangDoiIn::xuLyXoaDocGia() {
    int row = danhSachDocGia->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng chọn độc giả cần xóa!");
        return;
    }
    
    PrintQueueManager::instance()->xoa(row);
}

void HopThoaiHangDoiIn::xuLyXoaTatCa() {
    if (PrintQueueManager::instance()->soLuong() == 0) {
        QMessageBox::information(this, "Thông báo", "Hàng đợi đã trống!");
        return;
    }
    
    int ret = QMessageBox::question(this, "Xác nhận", 
                                      "Xóa toàn bộ hàng đợi?",
                                      QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        PrintQueueManager::instance()->xoaTatCa();
        QMessageBox::information(this, "Thành công", "Đã xóa toàn bộ hàng đợi!");
    }
}

void HopThoaiHangDoiIn::xuLyInA4() {
    QList<PTRDG> danhSach = PrintQueueManager::instance()->layDanhSach();
    
    if (danhSach.isEmpty()) {
        QMessageBox::warning(this, "Lỗi", "Hàng đợi trống! Vui lòng thêm độc giả trước.");
        return;
    }
    
    int soCot = spinSoCot->value();
    int soTrang = (danhSach.size() + (soCot * 5) - 1) / (soCot * 5);
    
    int ret = QMessageBox::question(this, "Xác nhận in", 
                                      QString("In %1 thẻ (%2 trang A4, %3 cột)?")
                                      .arg(danhSach.size())
                                      .arg(soTrang)
                                      .arg(soCot),
                                      QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (PrintHelper::inA4NhieuThe(danhSach, soCot, this)) {
            QMessageBox::information(this, "Thành công", "Đã gửi lệnh in!");
        }
    }
}

void HopThoaiHangDoiIn::xuLyXuatPDF() {
    QList<PTRDG> danhSach = PrintQueueManager::instance()->layDanhSach();
    
    if (danhSach.isEmpty()) {
        QMessageBox::warning(this, "Lỗi", "Hàng đợi trống!");
        return;
    }
    
    QString tenFile = QFileDialog::getSaveFileName(this, "Xuất PDF", 
                                                     QDir::homePath() + "/the_doc_gia.pdf",
                                                     "PDF Files (*.pdf)");
    
    if (tenFile.isEmpty()) return;
    
    int soCot = spinSoCot->value();
    
    // Sử dụng inA4NhieuThe để tạo HTML và in ra PDF
    if (PrintHelper::inA4NhieuThe(danhSach, soCot, nullptr)) {
        QMessageBox::information(this, "Thành công", "Đã xuất PDF: " + tenFile);
    }
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
    
    nhanThongTin->setText(QString("📊 Số thẻ: %1").arg(soThe));
    nhanSoTrang->setText(QString("📄 Số trang A4: %1 (%2 cột)").arg(soTrang).arg(soCot));
    
    // Enable/disable buttons
    bool coThe = soThe > 0;
    nutXoa->setEnabled(coThe);
    nutXoaTatCa->setEnabled(coThe);
    nutInA4->setEnabled(coThe);
    nutXuatPDF->setEnabled(coThe);
}
