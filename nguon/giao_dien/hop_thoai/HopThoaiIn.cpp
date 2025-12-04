#include "hop_thoai/HopThoaiIn.h"
#include "hop_thoai/HopThoaiHangDoiIn.h"
#include "utils/PrintHelper.h"
#include "utils/PrintQueueManager.h"
#include <QMessageBox>
#include <QSettings>
#include <QInputDialog>

HopThoaiIn::HopThoaiIn(QWidget *parent)
    : QDialog(parent)
    , docGiaHienTai(nullptr)
    , dauSachHienTai(nullptr)
    , rootDocGia(nullptr)
    , rootSach(nullptr)
{
    setWindowTitle("In Ấn Thư Viện");
    setMinimumWidth(500);
    
    taoGiaoDien();
    
    // Load cấu hình đã lưu
    QSettings settings("HVPG", "QuanLyThuVien");
    QString tenThuVien = settings.value("print/tenThuVien", "HỌC VIỆN PHẬT GIÁO NAM TÔNG CẦN THƠ").toString();
    
    PrintHelper::PrintConfig config;
    config.tenThuVien = tenThuVien;
    config.tenVietTat = settings.value("print/tenVietTat", "HVPG NAMTONG CANTHO").toString();
    config.inMauSac = settings.value("print/inMauSac", false).toBool();
    PrintHelper::setCauHinh(config);
}

HopThoaiIn::~HopThoaiIn() {
}

void HopThoaiIn::setDocGia(PTRDG dg) {
    docGiaHienTai = dg;
    nutInThe->setEnabled(dg != nullptr);
    nutInPhieuMuon->setEnabled(dg != nullptr);
    nutInPhieuTra->setEnabled(dg != nullptr);
}

void HopThoaiIn::setDauSach(PTRDS ds) {
    dauSachHienTai = ds;
    nutInTem->setEnabled(ds != nullptr);
    nutInNhieuTem->setEnabled(ds != nullptr);
}

void HopThoaiIn::setDanhSachDocGia(PTRDG root) {
    rootDocGia = root;
    nutInDSDocGia->setEnabled(root != nullptr);
    nutInNhieuThe->setEnabled(root != nullptr);
}

void HopThoaiIn::setDanhSachSach(PTRDS root) {
    rootSach = root;
    nutInDSSach->setEnabled(root != nullptr);
}

// ============================================================================
// TẠO GIAO DIỆN
// ============================================================================

void HopThoaiIn::taoGiaoDien() {
    layoutChinh = new QVBoxLayout(this);
    
    // Tiêu đề
    QLabel* lblTieuDe = new QLabel("<h2>🖨️ In Ấn Thư Viện</h2>");
    lblTieuDe->setAlignment(Qt::AlignCenter);
    layoutChinh->addWidget(lblTieuDe);
    
    // 3 nhóm chức năng
    taoNhomInThe();
    taoNhomInTem();
    taoNhomInBaoCao();
    
    layoutChinh->addStretch();
    
    // Nút điều khiển
    taoNutDieuKhien();
}

void HopThoaiIn::taoNhomInThe() {
    nhomInThe = new QGroupBox("In Thẻ Độc Giả");
    QVBoxLayout* layout = new QVBoxLayout(nhomInThe);
    
    // Mô tả
    QLabel* lblMoTa = new QLabel("In thẻ thư viện (85.6mm x 54mm) với pháp danh + mã vạch");
    lblMoTa->setStyleSheet("color: #666; font-size: 10pt;");
    layout->addWidget(lblMoTa);
    
    // Nút in đơn
    nutInThe = new QPushButton("In Thẻ Độc Giả Hiện Tại");
    nutInThe->setEnabled(true); // Luôn enable
    connect(nutInThe, &QPushButton::clicked, this, &HopThoaiIn::xuLyInTheDocGia);
    layout->addWidget(nutInThe);
    
    // Nút in hàng loạt - mở dialog quản lý queue
    nutInNhieuThe = new QPushButton("In Hàng Loạt Thẻ (Toàn Bộ)");
    nutInNhieuThe->setEnabled(true); // Luôn enable
    connect(nutInNhieuThe, &QPushButton::clicked, this, &HopThoaiIn::xuLyInNhieuThe);
    layout->addWidget(nutInNhieuThe);
    
    layoutChinh->addWidget(nhomInThe);
}

void HopThoaiIn::taoNhomInTem() {
    nhomInTem = new QGroupBox("In Tem Mã Vạch Sách");
    QVBoxLayout* layout = new QVBoxLayout(nhomInTem);
    
    // Mô tả
    QLabel* lblMoTa = new QLabel("In tem dán gáy sách (50mm x 30mm)");
    lblMoTa->setStyleSheet("color: #666; font-size: 10pt;");
    layout->addWidget(lblMoTa);
    
    // Nút in đơn
    nutInTem = new QPushButton("In Tem Sách Hiện Tại");
    nutInTem->setEnabled(true); // Luôn enable
    connect(nutInTem, &QPushButton::clicked, this, &HopThoaiIn::xuLyInTemSach);
    layout->addWidget(nutInTem);
    
    // Nút in nhiều với số lượng
    QHBoxLayout* layoutNhieu = new QHBoxLayout();
    
    nutInNhieuTem = new QPushButton("In Nhiều Tem:");
    nutInNhieuTem->setEnabled(true); // Luôn enable
    connect(nutInNhieuTem, &QPushButton::clicked, this, &HopThoaiIn::xuLyInNhieuTem);
    
    spinSoLuongTem = new QSpinBox();
    spinSoLuongTem->setMinimum(1);
    spinSoLuongTem->setMaximum(100);
    spinSoLuongTem->setValue(5);
    spinSoLuongTem->setSuffix(" tem");
    
    layoutNhieu->addWidget(nutInNhieuTem);
    layoutNhieu->addWidget(spinSoLuongTem);
    layout->addLayout(layoutNhieu);
    
    layoutChinh->addWidget(nhomInTem);
}

void HopThoaiIn::taoNhomInBaoCao() {
    nhomInBaoCao = new QGroupBox("In Báo Cáo / Biên Bản");
    QVBoxLayout* layout = new QVBoxLayout(nhomInBaoCao);
    
    // Mô tả
    QLabel* lblMoTa = new QLabel("In danh sách và phiếu mượn/trả (khổ A4)");
    lblMoTa->setStyleSheet("color: #666; font-size: 10pt;");
    layout->addWidget(lblMoTa);
    
    // Danh sách
    nutInDSDocGia = new QPushButton("📋 In Danh Sách Độc Giả");
    nutInDSDocGia->setEnabled(false);
    connect(nutInDSDocGia, &QPushButton::clicked, this, &HopThoaiIn::xuLyInDanhSachDocGia);
    layout->addWidget(nutInDSDocGia);
    
    nutInDSSach = new QPushButton("In Danh Mục Sách");
    nutInDSSach->setEnabled(false);
    connect(nutInDSSach, &QPushButton::clicked, this, &HopThoaiIn::xuLyInDanhSachSach);
    layout->addWidget(nutInDSSach);
    
    // Phiếu
    QHBoxLayout* layoutPhieu = new QHBoxLayout();
    
    nutInPhieuMuon = new QPushButton("Phiếu Mượn");
    nutInPhieuMuon->setEnabled(false);
    connect(nutInPhieuMuon, &QPushButton::clicked, this, &HopThoaiIn::xuLyInPhieuMuon);
    
    nutInPhieuTra = new QPushButton("Phiếu Trả");
    nutInPhieuTra->setEnabled(false);
    connect(nutInPhieuTra, &QPushButton::clicked, this, &HopThoaiIn::xuLyInPhieuTra);
    
    layoutPhieu->addWidget(nutInPhieuMuon);
    layoutPhieu->addWidget(nutInPhieuTra);
    layout->addLayout(layoutPhieu);
    
    layoutChinh->addWidget(nhomInBaoCao);
}

void HopThoaiIn::taoNutDieuKhien() {
    QHBoxLayout* layoutNut = new QHBoxLayout();
    
    nutCauHinh = new QPushButton("⚙️ Cấu Hình");
    connect(nutCauHinh, &QPushButton::clicked, this, &HopThoaiIn::xuLyCauHinh);
    
    nutDong = new QPushButton("Đóng");
    connect(nutDong, &QPushButton::clicked, this, &QDialog::accept);
    
    layoutNut->addWidget(nutCauHinh);
    layoutNut->addStretch();
    layoutNut->addWidget(nutDong);
    
    layoutChinh->addLayout(layoutNut);
}

// ============================================================================
// XỬ LÝ EVENTS
// ============================================================================

void HopThoaiIn::xuLyInTheDocGia() {
    // Hỏi mã thẻ độc giả
    bool ok;
    int maThe = QInputDialog::getInt(this, "In thẻ độc giả", 
                                      "Nhập mã thẻ độc giả:", 
                                      1, 1, 999999, 1, &ok);
    
    if (!ok) return;
    
    // Tìm độc giả
    extern PTRDG timDocGia(PTRDG root, int mathe);
    extern PTRDG rootDocGia;
    PTRDG docGia = timDocGia(rootDocGia, maThe);
    
    if (!docGia) {
        QMessageBox::warning(this, "Lỗi", "Không tìm thấy độc giả với mã thẻ: " + QString::number(maThe));
        return;
    }
    
    if (docGia->data.TrangThai != 1) {
        QMessageBox::warning(this, "Lỗi", "Độc giả này đã bị khóa!");
        return;
    }
    
    // In thẻ
    if (PrintHelper::inTheDocGia(docGia, this)) {
        QMessageBox::information(this, "Thành công", 
                                QString("Đã in thẻ cho: %1 %2")
                                .arg(QString::fromStdString(docGia->data.Ho))
                                .arg(QString::fromStdString(docGia->data.Ten)));
    }
}

void HopThoaiIn::xuLyInNhieuThe() {
    extern PTRDG rootDocGia;
    
    if (!rootDocGia) {
        QMessageBox::warning(this, "Lỗi", "Danh sách độc giả trống!");
        return;
    }
    
    // Mở dialog hàng đợi in hàng loạt
    HopThoaiHangDoiIn *dialogHangDoi = new HopThoaiHangDoiIn(rootDocGia, this);
    dialogHangDoi->exec();
    delete dialogHangDoi;
}

void HopThoaiIn::xuLyInTemSach() {
    // Hỏi ISBN sách
    bool ok;
    QString isbn = QInputDialog::getText(this, "In tem sách", 
                                          "Nhập ISBN hoặc mã sách:", 
                                          QLineEdit::Normal, "", &ok);
    
    if (!ok || isbn.trimmed().isEmpty()) return;
    
    // Tìm sách
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    PTRDS dauSach = nullptr;
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] && QString::fromStdString(dsDauSach[i]->ISBN) == isbn) {
            dauSach = dsDauSach[i];
            break;
        }
    }
    
    if (!dauSach) {
        QMessageBox::warning(this, "Lỗi", "Không tìm thấy sách với ISBN: " + isbn);
        return;
    }
    
    if (dauSach->tongBanSao == 0) {
        QMessageBox::warning(this, "Lỗi", "Sách này chưa có bản sao!");
        return;
    }
    
    // In tem cho bản sao đầu tiên
    PTRDMS banSao = dauSach->dms;
    if (banSao) {
        if (PrintHelper::inTemMaVachSach(
            QString::fromStdString(banSao->maSach),
            QString::fromStdString(dauSach->tenSach),
            this
        )) {
            QMessageBox::information(this, "Thành công", 
                                    QString("Đã in tem cho: %1")
                                    .arg(QString::fromStdString(dauSach->tenSach)));
        }
    }
}

void HopThoaiIn::xuLyInNhieuTem() {
    // Hỏi ISBN sách
    bool ok;
    QString isbn = QInputDialog::getText(this, "In nhiều tem", 
                                          "Nhập ISBN hoặc mã sách:", 
                                          QLineEdit::Normal, "", &ok);
    
    if (!ok || isbn.trimmed().isEmpty()) return;
    
    // Tìm sách
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    PTRDS dauSach = nullptr;
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] && QString::fromStdString(dsDauSach[i]->ISBN) == isbn) {
            dauSach = dsDauSach[i];
            break;
        }
    }
    
    if (!dauSach) {
        QMessageBox::warning(this, "Lỗi", "Không tìm thấy sách với ISBN: " + isbn);
        return;
    }
    
    int soLuong = spinSoLuongTem->value();
    if (PrintHelper::inNhieuTemSach(dauSach, soLuong, this)) {
        QMessageBox::information(this, "Thành công", 
                                QString("Đã in %1 tem cho: %2")
                                .arg(soLuong)
                                .arg(QString::fromStdString(dauSach->tenSach)));
    }
}

void HopThoaiIn::xuLyInDanhSachDocGia() {
    if (!rootDocGia) {
        QMessageBox::warning(this, "Lỗi", "Danh sách độc giả trống!");
        return;
    }
    
    PrintHelper::inDanhSachDocGia(rootDocGia, this);
}

void HopThoaiIn::xuLyInDanhSachSach() {
    if (!rootSach) {
        QMessageBox::warning(this, "Lỗi", "Danh sách sách trống!");
        return;
    }
    
    // rootSach là PTRDS (DauSach*), cần truyền mảng PTRDS*
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    PrintHelper::inDanhSachSach(dsDauSach, soLuongDauSach, this);
}

void HopThoaiIn::xuLyInPhieuMuon() {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, "Lỗi", "Chưa chọn độc giả!");
        return;
    }
    
    // TODO: Lấy danh sách sách đang mượn từ lịch sử
    QList<QString> danhSachSach;
    danhSachSach << "Kinh Pháp Cú" << "Thanh Tịnh Đạo Luận";
    
    PrintHelper::inPhieuMuonTra(docGiaHienTai, danhSachSach, true, this);
}

void HopThoaiIn::xuLyInPhieuTra() {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, "Lỗi", "Chưa chọn độc giả!");
        return;
    }
    
    // TODO: Lấy danh sách sách đang mượn
    QList<QString> danhSachSach;
    danhSachSach << "Kinh Pháp Cú";
    
    PrintHelper::inPhieuMuonTra(docGiaHienTai, danhSachSach, false, this);
}

void HopThoaiIn::xuLyCauHinh() {
    bool ok;
    QString tenMoi = QInputDialog::getText(this, "Cấu Hình", 
                                            "Tên thư viện (hiển thị trên thẻ/tem):",
                                            QLineEdit::Normal,
                                            PrintHelper::getCauHinh().tenThuVien,
                                            &ok);
    
    if (ok && !tenMoi.isEmpty()) {
        PrintHelper::PrintConfig config = PrintHelper::getCauHinh();
        config.tenThuVien = tenMoi;
        
        // Tạo tên viết tắt tự động
        QStringList words = tenMoi.split(' ', Qt::SkipEmptyParts);
        QString vietTat;
        for (const QString& word : words) {
            if (!word.isEmpty()) {
                vietTat += word[0].toUpper();
            }
        }
        config.tenVietTat = vietTat;
        
        PrintHelper::setCauHinh(config);
        
        // Lưu cấu hình
        QSettings settings("HVPG", "QuanLyThuVien");
        settings.setValue("print/tenThuVien", tenMoi);
        settings.setValue("print/tenVietTat", vietTat);
        
        QMessageBox::information(this, "Thành công", "Đã cập nhật cấu hình!");
    }
}
