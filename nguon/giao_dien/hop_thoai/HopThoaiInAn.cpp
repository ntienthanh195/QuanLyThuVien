#include "hop_thoai/HopThoaiInAn.h"
#include "utils/LanguageManager.h"
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
    
    setWindowTitle(LanguageManager::instance().getText("DIALOG_PRINT_MANAGER_TITLE"));
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

// Support for dragging the frameless window
void HopThoaiInAn::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // Only allow dragging from the top area (Header)
        if (event->position().y() < 60) {
            m_dragging = true;
            m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
        }
    }
}

void HopThoaiInAn::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        if (m_dragging) {
            move(event->globalPosition().toPoint() - m_dragPosition);
            event->accept();
        }
    }
}

void HopThoaiInAn::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
    }
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
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header container
    QWidget* headerWidget = new QWidget(this);
    headerWidget->setStyleSheet("background-color: #1a1a2e; border-bottom: 1px solid #323248;");
    headerWidget->setFixedHeight(60);
    QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(20, 0, 20, 0);

    QLabel* titleIcon = new QLabel("🖨️"); // Header icon can stay or be removed based on preference, keeping for now as logo
    titleIcon->setStyleSheet("font-size: 24px;");
    QLabel* titleLabel = new QLabel(LanguageManager::instance().getText("HEADER_PRINT_LIBRARY"));
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #ffffff; letter-spacing: 1px;");
    
    headerLayout->addWidget(titleIcon);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    
    // Nút đóng ở header
    QPushButton* closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(30, 30);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { color: #888; background: transparent; border: none; font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { color: #fff; background: #c0392b; border-radius: 4px; }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    headerLayout->addWidget(closeBtn);

    mainLayout->addWidget(headerWidget);

    // Scroll Area for Body
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea { background: #1e1e2d; border: none; } QScrollBar:vertical { width: 6px; background: #1b1b29; } QScrollBar::handle:vertical { background: #323248; border-radius: 3px; }");

    QWidget* content = new QWidget();
    content->setStyleSheet("background-color: #1e1e2d;");
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setSpacing(15);
    contentLayout->setContentsMargins(15, 15, 15, 15);

    // Helper to create GroupBox (Compact)
    auto createGroup = [](const QString& title) -> QGroupBox* {
        QGroupBox* box = new QGroupBox(title);
        box->setStyleSheet(
            "QGroupBox { "
            "  background-color: #2b2b40; "
            "  border: 1px solid #323248; "
            "  border-radius: 6px; "
            "  margin-top: 18px; "
            "  padding-top: 25px; " // Reduced top padding
            "  padding-bottom: 10px; "
            "  padding-left: 10px; padding-right: 10px;"
            "  font-weight: bold; "
            "  font-size: 13px; "
            "  color: #ffffff; "
            "} "
            "QGroupBox::title { "
            "  subcontrol-origin: margin; "
            "  left: 10px; top: -8px; "
            "  padding: 0 4px; "
            "  color: #5d9cec; "
            "  background-color: #1e1e2d;" // match parent bg to hide border line cleanly
            "}"
        );
        return box;
    };

    // Helper for Buttons (Compact Grid Cards)
    auto createBtn = [](const QString& text, const QString& iconStr, const QString& subtext) -> QPushButton* {
        QPushButton* btn = new QPushButton();
        QVBoxLayout* l = new QVBoxLayout(btn);
        l->setContentsMargins(8, 8, 8, 8);
        l->setSpacing(2);
        
        // Icon + Title Row
        QHBoxLayout* titleRow = new QHBoxLayout();
        titleRow->setSpacing(6);
        titleRow->setContentsMargins(0,0,0,0);
        
        QLabel* icon = new QLabel(iconStr);
        icon->setStyleSheet("font-size: 18px; color: #5d9cec; border: none; background: transparent;");
        titleRow->addWidget(icon);
        
        QLabel* mainLbl = new QLabel(text);
        mainLbl->setStyleSheet("font-size: 13px; font-weight: 600; color: #fff; background: transparent; border: none;");
        titleRow->addWidget(mainLbl);
        titleRow->addStretch();
        l->addLayout(titleRow);
        
        if (!subtext.isEmpty()) {
            QLabel* subLbl = new QLabel(subtext);
            subLbl->setWordWrap(true); 
            subLbl->setStyleSheet("font-size: 10px; color: #a0a0a0; background: transparent; border: none;");
            l->addWidget(subLbl);
        }
        
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton { "
            "  background: #323242; "
            "  border: 1px solid #3E3E48; "
            "  border-radius: 5px; "
            "  text-align: left; "
            "} "
            "QPushButton:hover { "
            "  background: #3e3e56; "
            "  border-color: #5d9cec; "
            "} "
            "QPushButton:pressed { background: #282838; }"
        );
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); 
        btn->setToolTip(text); // Tooltip fallback
        return btn;
    };

    // === ROW 1: USER & BOOK (2 Columns) ===
    QHBoxLayout* mainRow = new QHBoxLayout();
    mainRow->setSpacing(15);

    // Col 1: Reader Cards
    QGroupBox* gbThe = createGroup(LanguageManager::instance().getText("GROUP_READER_CARDS"));
    QVBoxLayout* lThe = new QVBoxLayout(gbThe);
    lThe->setSpacing(8);
    lThe->setContentsMargins(5, 20, 5, 5);

    QPushButton* btnInThe = createBtn(LanguageManager::instance().getText("BTN_PRINT_CARD_CURRENT"), "👤", LanguageManager::instance().getText("DESC_PRINT_CARD_CURRENT"));
    connect(btnInThe, &QPushButton::clicked, this, &HopThoaiInAn::xuLyInTheDocGiaHienTai);
    lThe->addWidget(btnInThe);

    QPushButton* btnInLoat = createBtn(LanguageManager::instance().getText("BTN_PRINT_CARD_BATCH"), "👥", LanguageManager::instance().getText("DESC_PRINT_CARD_BATCH"));
    connect(btnInLoat, &QPushButton::clicked, this, &HopThoaiInAn::xuLyInHangLoatThe);
    lThe->addWidget(btnInLoat);
    lThe->addStretch();
    
    mainRow->addWidget(gbThe);

    // Col 2: Book Labels
    QGroupBox* gbTem = createGroup(LanguageManager::instance().getText("GROUP_BOOK_STAMPS"));
    QVBoxLayout* lTem = new QVBoxLayout(gbTem);
    lTem->setSpacing(8);
    lTem->setContentsMargins(5, 20, 5, 5);

    QPushButton* btnTimTem = createBtn(LanguageManager::instance().getText("BTN_SEARCH_PRINT_STAMP"), "🏷️", LanguageManager::instance().getText("DESC_SEARCH_PRINT_STAMP"));
    connect(btnTimTem, &QPushButton::clicked, this, &HopThoaiInAn::xuLyTimVaInTem);
    lTem->addWidget(btnTimTem);
    lTem->addStretch();

    mainRow->addWidget(gbTem);
    contentLayout->addLayout(mainRow);

    // === ROW 2: REPORTS (Grid 2x2) ===
    QGroupBox* gbBaoCao = createGroup(LanguageManager::instance().getText("GROUP_REPORTS"));
    QGridLayout* gridBaoCao = new QGridLayout(gbBaoCao);
    gridBaoCao->setContentsMargins(10, 25, 10, 10);
    gridBaoCao->setSpacing(10);

    QPushButton* btnDSDocGia = createBtn(LanguageManager::instance().getText("BTN_LIST_READERS"), "📄", LanguageManager::instance().getText("DESC_LIST_READERS"));
    connect(btnDSDocGia, &QPushButton::clicked, this, &HopThoaiInAn::xuLyInDanhSachDocGia);
    gridBaoCao->addWidget(btnDSDocGia, 0, 0);

    QPushButton* btnDSSach = createBtn(LanguageManager::instance().getText("BTN_LIST_BOOKS"), "📚", LanguageManager::instance().getText("DESC_LIST_BOOKS"));
    connect(btnDSSach, &QPushButton::clicked, this, &HopThoaiInAn::xuLyInDanhMucSach);
    gridBaoCao->addWidget(btnDSSach, 0, 1);

    QPushButton* btnPhieuMuon = createBtn(LanguageManager::instance().getText("BTN_BORROW_SLIP"), "📥", LanguageManager::instance().getText("DESC_BORROW_SLIP"));
    connect(btnPhieuMuon, &QPushButton::clicked, this, &HopThoaiInAn::xuLyInPhieuMuon);
    gridBaoCao->addWidget(btnPhieuMuon, 1, 0);

    QPushButton* btnPhieuTra = createBtn(LanguageManager::instance().getText("BTN_RETURN_SLIP"), "📤", LanguageManager::instance().getText("DESC_RETURN_SLIP"));
    connect(btnPhieuTra, &QPushButton::clicked, this, &HopThoaiInAn::xuLyInPhieuTra);
    gridBaoCao->addWidget(btnPhieuTra, 1, 1);

    contentLayout->addWidget(gbBaoCao);

    // Bottom Actions
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    bottomLayout->setContentsMargins(0, 10, 0, 0);
    
    QPushButton* btnCauHinh = new QPushButton(LanguageManager::instance().getText("BTN_PRINT_CONFIG"));
    btnCauHinh->setCursor(Qt::PointingHandCursor);
    btnCauHinh->setFlat(true);
    btnCauHinh->setStyleSheet(
        "QPushButton { color: #888; font-size: 13px; text-align: left; padding: 5px; } "
        "QPushButton:hover { color: #fff; text-decoration: underline; }"
    );
    connect(btnCauHinh, &QPushButton::clicked, this, &HopThoaiInAn::xuLyCauHinhIn);
    
    bottomLayout->addWidget(btnCauHinh);
    bottomLayout->addStretch();

    contentLayout->addLayout(bottomLayout);
    contentLayout->addStretch(); // Push everything up

    scroll->setWidget(content);
    mainLayout->addWidget(scroll);
}

void HopThoaiInAn::apDungPhongCachModern() {
    // Styling is now applied directly in UI setup
}

// ==================== SLOT IMPLEMENTATIONS ====================

void HopThoaiInAn::xuLyTimVaInTem() {
    bool ok;
    QString tuKhoa = QInputDialog::getText(this, LanguageManager::instance().getText("TITLE_SEARCH_BOOK"), 
        LanguageManager::instance().getText("MSG_ENTER_ISBN_NAME"), QLineEdit::Normal, "", &ok);
    
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
                
                int soLuong = QInputDialog::getInt(this, LanguageManager::instance().getText("TITLE_STAMP_QUANTITY"),
                    LanguageManager::instance().getText("MSG_ENTER_STAMP_QUANTITY"), 5, 1, 100, 1, &ok);
                
                if (ok) {
                    PrintHelper::inNhieuTemSach(dsDauSach[i], soLuong, this);
                }
                return;
            }
        }
    }
    
    QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_NOT_FOUND"), 
        LanguageManager::instance().getText("MSG_NOT_FOUND_KEYWORD") + tuKhoa);
}

void HopThoaiInAn::xuLyInTheDocGiaHienTai() {
    if (!kiemTraDocGiaHopLe()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARN"), 
            LanguageManager::instance().getText("MSG_SELECT_READER_FIRST"));
        return;
    }
    
    PrintHelper::inTheDocGiaTuDong(docGiaHienTai, this);
}

void HopThoaiInAn::xuLyInHangLoatThe() {
    if (!rootDocGia) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARN"), LanguageManager::instance().getText("MSG_NO_READERS"));
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        LanguageManager::instance().getText("TITLE_CONFIRM_BATCH_PRINT"),
        LanguageManager::instance().getText("MSG_CONFIRM_BATCH_PRINT"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        PrintHelper::inHangLoatTheDocGia(rootDocGia, this);
    }
}

void HopThoaiInAn::xuLyInDanhSachDocGia() {
    if (!rootDocGia) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARN"), LanguageManager::instance().getText("MSG_NO_READERS"));
        return;
    }
    
    PrintHelper::inDanhSachDocGia(rootDocGia, this);
}

void HopThoaiInAn::xuLyInDanhMucSach() {
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    
    if (soLuongDauSach == 0) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARN"), LanguageManager::instance().getText("MSG_NO_BOOKS_SYSTEM"));
        return;
    }
    
    PrintHelper::inDanhMucSach(dsDauSach, soLuongDauSach, this);
}

void HopThoaiInAn::xuLyInPhieuMuon() {
    if (!kiemTraDocGiaHopLe()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARN"), 
            LanguageManager::instance().getText("MSG_SELECT_READER_FIRST"));
        return;
    }
    
    if (docGiaHienTai->data.soSachDangMuon == 0) {
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_INFO"), 
            LanguageManager::instance().getText("MSG_READER_NO_BORROW"));
        return;
    }
    
    MUONTRA p = docGiaHienTai->data.dsmt;
    if (p && p->data.banSaoSach) {
        QString maSach = QString::fromStdString(p->data.banSaoSach->maSach);
        QString ngayMuon = QString::fromStdString(p->data.NgayMuon);
        
        QString tenSach = LanguageManager::instance().getText("LBL_UNKNOWN");
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
                if (tenSach != LanguageManager::instance().getText("LBL_UNKNOWN")) break;
            }
        }
        
        PrintHelper::inPhieuMuonTuDong(docGiaHienTai, maSach, tenSach, ngayMuon, this);
    }
}

void HopThoaiInAn::xuLyInPhieuTra() {
    if (!kiemTraDocGiaHopLe()) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARN"), LanguageManager::instance().getText("MSG_SELECT_READER_FIRST"));
        return;
    }
    
    if (docGiaHienTai->data.soSachDangMuon == 0) {
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_INFO"), 
            LanguageManager::instance().getText("MSG_READER_NO_RETURN"));
        return;
    }
    
    QList<QString> danhSachSach;
    MUONTRA p = docGiaHienTai->data.dsmt;
    
    while (p) {
        if (p->data.banSaoSach && p->data.TrangThai == 0) {
            QString maSach = QString::fromStdString(p->data.banSaoSach->maSach);
            
            QString tenSach = LanguageManager::instance().getText("LBL_UNKNOWN");
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
                    if (tenSach != LanguageManager::instance().getText("LBL_UNKNOWN")) break;
                }
            }
            
            danhSachSach.append(QString("%1 (Mã: %2)").arg(tenSach).arg(maSach));
        }
        p = p->next;
    }
    
    if (danhSachSach.isEmpty()) {
        QMessageBox::information(this, "Thông Báo", 
            LanguageManager::instance().getText("MSG_NO_BOOKS_TO_RETURN"));
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
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SAVE_CONFIG"), LanguageManager::instance().getText("MSG_CONFIG_SAVED"));
    }
}
