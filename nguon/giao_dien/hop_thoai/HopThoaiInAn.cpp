#include "hop_thoai/HopThoaiInAn.h"
#include "utils/PrintHelper.h"
#include "utils/ThongBao.h"
#include "utils/FilePathManager.h"
#include "models/ThaoTacFile.h"
#include "models/QuanLySach.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QScrollArea>
#include <QSizePolicy>
#include "hop_thoai/HopThoaiCauHinhIn.h"
#include "utils/EventBus.h"

extern PTRDG rootDocGia;
extern PTRDS dsDauSach[MAX_DAUSACH];
extern int soLuongDauSach;

// ==================== CONSTRUCTOR / DESTRUCTOR ====================

HopThoaiInAn::HopThoaiInAn(QWidget *parent)
    : QDialog(parent), 
      docGiaHienTai(nullptr), 
      dsDauSach(nullptr), 
      soLuongDauSach(0) {
    
    setWindowTitle("Quản Lý In Ấn");
    setMinimumWidth(400);
    setMinimumHeight(400);
    resize(450, 500);
    setModal(true);
    
    thieLapGiaoDien();
    apDungPhongCachModern();

    // Realtime: update dialog state if underlying data changes
    connect(&EventBus::instance(), &EventBus::docGiaRemoved, this, [this](int maThe){
        if (docGiaHienTai && docGiaHienTai->data.MaThe == maThe) {
            docGiaHienTai = nullptr;
        }
    });
    connect(&EventBus::instance(), &EventBus::docGiaUpdated, this, [this](int maThe){
        Q_UNUSED(maThe);
        this->update();
    });
    connect(&EventBus::instance(), &EventBus::dauSachUpdated, this, [this](const QString&){ this->update(); });
    connect(&EventBus::instance(), &EventBus::dauSachAdded, this, [this](const QString&){ this->update(); });
}

HopThoaiInAn::~HopThoaiInAn() {
}

// ==================== PUBLIC METHODS ====================

void HopThoaiInAn::setDocGiaHienTai(PTRDG docGia) {
    docGiaHienTai = docGia;
}

void HopThoaiInAn::setDanhSachDauSach(PTRDS* ds, int soLuong) {
    dsDauSach = ds;
    soLuongDauSach = soLuong;
}

// ==================== UI SETUP ====================

void HopThoaiInAn::thieLapGiaoDien() {
    // Use a scrollable area so the dialog can fit on small screens
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    QWidget* content = new QWidget(scroll);
    QVBoxLayout* layoutChinh = new QVBoxLayout(content);
    layoutChinh->setSpacing(12);
    layoutChinh->setContentsMargins(20, 20, 20, 20);
    
    // ========== TIÊU ĐỀ ĐƠN GIẢN ==========
    QLabel* tieuDe = new QLabel("Quản Lý In Ấn");
    tieuDe->setStyleSheet(
        "font-size: 20px; "
        "font-weight: 600; "
        "color: #FFFFFF; "
        "margin-bottom: 10px;"
    );
    layoutChinh->addWidget(tieuDe);
    
    // Đường phân cách
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background: #3E3E42; max-height: 1px;");
    layoutChinh->addWidget(line);
    
    // ========== NHÓM THẺ ĐỘC GIẢ ==========
    QLabel* labelThe = new QLabel("Thẻ Độc Giả");
    labelThe->setStyleSheet("font-size: 12px; font-weight: 600; color: #9CDCFE; margin-top: 16px; text-transform: uppercase; letter-spacing: 1px;");
    layoutChinh->addWidget(labelThe);

    auto makeMainButton = [&](const QString& text){
        QPushButton* b = new QPushButton(text);
        b->setMinimumHeight(48);
        b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        b->setStyleSheet("text-align: left; padding: 10px 14px; font-size: 13px;");
        return b;
    };

    QPushButton* nutInThe = makeMainButton("In Thẻ Độc Giả Hiện Tại");
    connect(nutInThe, &QPushButton::clicked, this, &HopThoaiInAn::xuLyInTheDocGiaHienTai);
    layoutChinh->addWidget(nutInThe);

    QPushButton* nutInHangLoat = makeMainButton("In Hàng Loạt Thẻ");
    connect(nutInHangLoat, &QPushButton::clicked, this, &HopThoaiInAn::xuLyInHangLoatThe);
    layoutChinh->addWidget(nutInHangLoat);
    
    // ========== NHÓM TEM SÁCH ==========
    QLabel* labelTem = new QLabel("Tem Sách");
    labelTem->setStyleSheet("font-size: 12px; font-weight: 600; color: #9CDCFE; margin-top: 16px; text-transform: uppercase; letter-spacing: 1px;");
    layoutChinh->addWidget(labelTem);

    QPushButton* nutTimSach = makeMainButton("Tìm & In Tem Sách");
    connect(nutTimSach, &QPushButton::clicked, this, &HopThoaiInAn::xuLyTimVaInTem);
    layoutChinh->addWidget(nutTimSach);
    
    // ========== NHÓM BÁO CÁO ==========
    QLabel* labelBaoCao = new QLabel("Báo Cáo & Danh Sách");
    labelBaoCao->setStyleSheet("font-size: 12px; font-weight: 600; color: #9CDCFE; margin-top: 16px; text-transform: uppercase; letter-spacing: 1px;");
    layoutChinh->addWidget(labelBaoCao);

    QPushButton* nutDSDocGia = makeMainButton("Xuất Danh Sách Độc Giả (PDF)");
    connect(nutDSDocGia, &QPushButton::clicked, this, &HopThoaiInAn::xuLyInDanhSachDocGia);
    layoutChinh->addWidget(nutDSDocGia);

    QPushButton* nutDMSach = makeMainButton("Xuất Dan Mục Sách (PDF)");
    connect(nutDMSach, &QPushButton::clicked, this, &HopThoaiInAn::xuLyInDanhMucSach);
    layoutChinh->addWidget(nutDMSach);

    QPushButton* nutPhieuMuon = makeMainButton("In Phiếu Mượn Sách");
    connect(nutPhieuMuon, &QPushButton::clicked, this, &HopThoaiInAn::xuLyInPhieuMuon);
    layoutChinh->addWidget(nutPhieuMuon);

    QPushButton* nutPhieuTra = makeMainButton("In Phiếu Trả Sách");
    connect(nutPhieuTra, &QPushButton::clicked, this, &HopThoaiInAn::xuLyInPhieuTra);
    layoutChinh->addWidget(nutPhieuTra);
    
    layoutChinh->addStretch();

    // Footer buttons (aligned horizontally)
    QHBoxLayout* footer = new QHBoxLayout();
    footer->setSpacing(10);
    QPushButton* nutCauHinh = makeMainButton("Cấu hình In");
    nutCauHinh->setMinimumHeight(42);
    nutCauHinh->setMaximumWidth(200);
    connect(nutCauHinh, &QPushButton::clicked, this, &HopThoaiInAn::xuLyCauHinhIn);

    QPushButton* nutDong = makeMainButton("Đóng");
    nutDong->setMinimumHeight(42);
    nutDong->setMaximumWidth(200);
    connect(nutDong, &QPushButton::clicked, this, &QDialog::accept);

    footer->addStretch();
    footer->addWidget(nutCauHinh);
    footer->addWidget(nutDong);
    layoutChinh->addLayout(footer);

    // Put content into scroll area
    content->setLayout(layoutChinh);
    scroll->setWidget(content);
    mainLayout->addWidget(scroll);
}

void HopThoaiInAn::apDungPhongCachModern() {
    setStyleSheet(
        "QDialog {"
        "   background: #1E1E1E;"
        "}"
        "QLabel {"
        "   color: #E0E0E0;"
        "   font-weight: 500;"
        "}"
        "QPushButton {"
        "   background: #2D2D30;"
        "   color: #CCCCCC;"
        "   border: 1px solid #3E3E42;"
        "   border-radius: 4px;"
        "   padding: 10px 16px;"
        "   font-size: 13px;"
        "   font-weight: 500;"
        "   text-align: left;"
        "}"
        "QPushButton:hover {"
        "   background: #3E3E42;"
        "   border-color: #007ACC;"
        "   color: #FFFFFF;"
        "}"
        "QPushButton:pressed {"
        "   background: #007ACC;"
        "   border-color: #007ACC;"
        "   color: #FFFFFF;"
        "}"
    );
}

// ==================== SLOT IMPLEMENTATIONS ====================

void HopThoaiInAn::xuLyTimVaInTem() {
    bool ok;
    QString tuKhoa = QInputDialog::getText(this, "Tìm Sách", 
        "Nhập ISBN hoặc tên sách:", QLineEdit::Normal, "", &ok);
    
    if (!ok || tuKhoa.trimmed().isEmpty()) {
        return;
    }
    
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i]) {
            QString tenSach = QString::fromStdString(dsDauSach[i]->tenSach);
            QString isbn = QString::fromStdString(dsDauSach[i]->ISBN);
            
            if (tenSach.contains(tuKhoa, Qt::CaseInsensitive) || 
                isbn.contains(tuKhoa, Qt::CaseInsensitive)) {
                
                int soLuong = QInputDialog::getInt(this, "Số Lượng Tem",
                    "Số tem cần in:", 5, 1, 100, 1, &ok);
                
                if (ok) {
                    PrintHelper::inNhieuTemSach(dsDauSach[i], soLuong, this);
                }
                return;
            }
        }
    }
    
    QMessageBox::warning(this, "Không Tìm Thấy", 
        "Không tìm thấy sách với từ khóa: " + tuKhoa);
}

void HopThoaiInAn::xuLyInTheDocGiaHienTai() {
    if (!kiemTraDocGiaHopLe()) {
        QMessageBox::warning(this, "Cảnh Báo", 
            "Vui lòng chọn độc giả trước khi in thẻ!");
        return;
    }
    
    PrintHelper::inTheDocGiaTuDong(docGiaHienTai, this);
}

void HopThoaiInAn::xuLyInHangLoatThe() {
    if (!rootDocGia) {
        QMessageBox::warning(this, "Cảnh Báo", "Không có độc giả nào trong hệ thống!");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Xác Nhận In Hàng Loạt",
        "In thẻ cho TẤT CẢ độc giả trong hệ thống?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        PrintHelper::inHangLoatTheDocGia(rootDocGia, this);
    }
}

void HopThoaiInAn::xuLyInDanhSachDocGia() {
    if (!rootDocGia) {
        QMessageBox::warning(this, "Cảnh Báo", "Không có độc giả nào trong hệ thống!");
        return;
    }
    
    PrintHelper::inDanhSachDocGia(rootDocGia, this);
}

void HopThoaiInAn::xuLyInDanhMucSach() {
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    
    if (soLuongDauSach == 0) {
        QMessageBox::warning(this, "Cảnh Báo", "Không có sách nào trong hệ thống!");
        return;
    }
    
    PrintHelper::inDanhMucSach(dsDauSach, soLuongDauSach, this);
}

void HopThoaiInAn::xuLyInPhieuMuon() {
    if (!kiemTraDocGiaHopLe()) {
        QMessageBox::warning(this, "Cảnh Báo", 
            "Vui lòng chọn độc giả có sách đang mượn!");
        return;
    }
    
    if (docGiaHienTai->data.soSachDangMuon == 0) {
        QMessageBox::information(this, "Thông Báo", 
            "Độc giả này chưa mượn sách nào!");
        return;
    }
    
    MUONTRA p = docGiaHienTai->data.dsmt;
    if (p && p->data.banSaoSach) {
        QString maSach = QString::fromStdString(p->data.banSaoSach->maSach);
        QString ngayMuon = QString::fromStdString(p->data.NgayMuon);
        
        QString tenSach = "Không rõ";
        extern PTRDS dsDauSach[MAX_DAUSACH];
        extern int soLuongDauSach;
        
        for (int i = 0; i < soLuongDauSach; i++) {
            if (dsDauSach[i]) {
                PTRDMS dms = dsDauSach[i]->dms;
                while (dms) {
                    if (QString::fromStdString(dms->maSach) == maSach) {
                        tenSach = QString::fromStdString(dsDauSach[i]->tenSach);
                        break;
                    }
                    dms = dms->next;
                }
                if (tenSach != "Không rõ") break;
            }
        }
        
        PrintHelper::inPhieuMuonTuDong(docGiaHienTai, maSach, tenSach, ngayMuon, this);
    }
}

void HopThoaiInAn::xuLyInPhieuTra() {
    if (!kiemTraDocGiaHopLe()) {
        QMessageBox::warning(this, "Cảnh Báo", "Vui lòng chọn độc giả!");
        return;
    }
    
    if (docGiaHienTai->data.soSachDangMuon == 0) {
        QMessageBox::information(this, "Thông Báo", 
            "Độc giả này không có sách nào đang mượn để trả!");
        return;
    }
    
    QList<QString> danhSachSach;
    MUONTRA p = docGiaHienTai->data.dsmt;
    
    while (p) {
        if (p->data.banSaoSach && p->data.TrangThai == 0) {
            QString maSach = QString::fromStdString(p->data.banSaoSach->maSach);
            
            QString tenSach = "Không rõ";
            extern PTRDS dsDauSach[MAX_DAUSACH];
            extern int soLuongDauSach;
            
            for (int i = 0; i < soLuongDauSach; i++) {
                if (dsDauSach[i]) {
                    PTRDMS dms = dsDauSach[i]->dms;
                    while (dms) {
                        if (QString::fromStdString(dms->maSach) == maSach) {
                            tenSach = QString::fromStdString(dsDauSach[i]->tenSach);
                            break;
                        }
                        dms = dms->next;
                    }
                    if (tenSach != "Không rõ") break;
                }
            }
            
            danhSachSach.append(QString("%1 (Mã: %2)").arg(tenSach).arg(maSach));
        }
        p = p->next;
    }
    
    if (danhSachSach.isEmpty()) {
        QMessageBox::information(this, "Thông Báo", 
            "Không có sách nào cần trả!");
        return;
    }
    
    PrintHelper::inPhieuMuonTra(docGiaHienTai, danhSachSach, false, this);
}

bool HopThoaiInAn::kiemTraDocGiaHopLe() {
    return (docGiaHienTai != nullptr);
}

void HopThoaiInAn::xuLyCauHinhIn() {
    PrintHelper::PrintConfig cfg = PrintHelper::getCauHinh();
    HopThoaiCauHinhIn dlg(cfg, this);
    if (dlg.exec() == QDialog::Accepted) {
        PrintHelper::setCauHinh(dlg.getConfig());
        QMessageBox::information(this, "Lưu Cấu Hình", "Cấu hình in đã được lưu.");
    }
}
