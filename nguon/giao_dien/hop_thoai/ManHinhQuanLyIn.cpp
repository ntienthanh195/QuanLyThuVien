#include "giao_dien/hop_thoai/ManHinhQuanLyIn.h"
#include "utils/LanguageManager.h"
#include "utils/PrintHelper.h"
#include "utils/ClipboardHelper.h" // Added
#include "utils/SearchableListWidget.h" // NEW
#include "models/QuanLySach.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QDebug>
#include <QMessageBox>
#include <QScrollArea>
#include <QTimer>

#include "views/dialogs/ManHinhCauHinh.h"
#include "giao_dien/hop_thoai/HopThoaiCauHinhIn.h"

// Helper to create Action Cards (Big Buttons)
static QPushButton* createActionCard(QString title, QString desc, bool isPrimary = false) {
    QPushButton *btn = new QPushButton();
    btn->setCursor(Qt::PointingHandCursor); // ✅ FIX: Add cursor feedback
    btn->setMinimumSize(180, 60); // Compact Size
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    QString baseColor = isPrimary ? "#2d2d3e" : "#252530";
    QString borderColor = isPrimary ? "#73a2dfff" : "#3E3E42";
    QString hoverColor = isPrimary ? "#36364d" : "#2d2d3e";
    
    btn->setStyleSheet(QString(
        "QPushButton { background-color: %1; border: 1px solid %2; border-radius: 6px; text-align: left; padding: 10px; }"
        "QPushButton:hover { background-color: %3; border-color: %2; }"
        "QPushButton:pressed { background-color: #202030; }"
    ).arg(baseColor, borderColor, hoverColor));
    
    QVBoxLayout *vbox = new QVBoxLayout(btn);
    vbox->setContentsMargins(5,5,5,5);
    vbox->setSpacing(2);
    
    QLabel *lblTitle = new QLabel(title);
    lblTitle->setStyleSheet(QString("font-size: 14px; font-weight: bold; color: %1; background: transparent; border: none;").arg(isPrimary ? "#5D9CEC" : "#ffffff"));
    lblTitle->setAttribute(Qt::WA_TransparentForMouseEvents); // FIX: Let clicks pass through
    
    QLabel *lblDesc = new QLabel(desc);
    lblDesc->setStyleSheet("font-size: 11px; color: #999; background: transparent; border: none;");
    lblDesc->setWordWrap(true);
    lblDesc->setAttribute(Qt::WA_TransparentForMouseEvents); // FIX: Let clicks pass through
    
    vbox->addWidget(lblTitle);
    vbox->addWidget(lblDesc);
    
    return btn;
}

ManHinhQuanLyIn::ManHinhQuanLyIn(PTRDG caNhan, const std::vector<PTRDG>& danhSach, QWidget *parent) 
    : QDialog(parent), docGiaHienTai(caNhan), m_danhSach(danhSach), sachHienTai(nullptr) {
    setWindowTitle(LanguageManager::instance().getText("DIALOG_PRINT_CENTER_TITLE"));
    resize(900, 600); // Larger size for dashboard
    
    // Apply Global Dialog Style
    setStyleSheet(
        "QDialog { background-color: #1e1e2d; color: #E0E0E0; font-family: 'Segoe UI', Arial; }"
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { width: 8px; background: #2b2b40; }"
        "QScrollBar::handle:vertical { background: #555; border-radius: 4px; }"
        "QGroupBox { font-size: 14px; font-weight: bold; color: #E0E0E0; border: 1px solid #3E3E42; border-radius: 8px; margin-top: 10px; padding-top: 25px; background: #252534; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 15px; top: 0px; padding: 0 5px; color: #5D9CEC; background: #1e1e2d; border-radius: 4px; }"
    );
    
    thieLapGiaoDien();
    ClipboardHelper::setupGlobalCopyPaste(this);

    // NEW: Populate Reader SearchList
    // If m_danhSach is empty, load all readers from global tree
    if (m_danhSach.empty()) {
        extern PTRDG rootDocGia;
        if (rootDocGia) {
            std::function<void(PTRDG)> traverse = [&](PTRDG p) {
                if (!p) return;
                traverse(p->left);
                m_danhSach.push_back(p);
                traverse(p->right);
            };
            traverse(rootDocGia);
        }
    }
    
    if (!m_danhSach.empty() && readerSearchList) {
        QStringList readerNames;
        QVariantList readerData;
        
        for (const auto& dg : m_danhSach) {
            if (!dg) continue;
            QString display = QString("%1 %2 (Mã: %3) | Mượn: %4")
                .arg(QString::fromStdString(dg->data.Ho))
                .arg(QString::fromStdString(dg->data.Ten))
                .arg(dg->data.MaThe)
                .arg(dg->data.soSachDangMuon);
            readerNames << display;
            readerData << QVariant::fromValue(dg);
        }
        
        readerSearchList->setItems(readerNames, readerData);
        
        // Connect signals
        connect(readerSearchList, &SearchableListWidget::itemDoubleClicked,
            this, &ManHinhQuanLyIn::xuLyInTheDocGia);
    }

    // NEW: Populate Book SearchList
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    if (soLuongDauSach > 0 && bookSearchList) {
        QStringList bookNames;
        QVariantList bookData;
        
        for (int i = 0; i < soLuongDauSach; ++i) {
            PTRDS sach = dsDauSach[i];
            if (!sach) continue;
            
            QString display = QString("%1 (ISBN: %2) | SL: %3")
                .arg(QString::fromStdString(sach->tenSach))
                .arg(QString::fromStdString(sach->ISBN))
                .arg(sach->tongBanSao);
            bookNames << display;
            
            // We need to store PTRDS in QVariant. 
            // Since PTRDS is a pointer, we can use QVariant::fromValue or cast to void*
            // But QVariant needs registered type or void*. Let's use void* wrapper or UserRole logic in widget.
            bookData << QVariant::fromValue((void*)sach); 
        }
        
        bookSearchList->setItems(bookNames, bookData);
        
         connect(bookSearchList, &SearchableListWidget::itemDoubleClicked,
            this, &ManHinhQuanLyIn::xuLyInTemSach);
    }
}

// Smart Context Switching
void ManHinhQuanLyIn::setMode(Mode mode) {
    if (!scrollArea || !groupReader || !groupBook) return;
    
    // Show/Hide sections based on context
    switch(mode) {
        case READER_CONTEXT:
            groupReader->setVisible(true);
            groupBook->setVisible(false);
            groupSys->setVisible(true);
            // Show only reader reports
            if (btnReportUser) btnReportUser->setVisible(true);
            if (btnReportBook) btnReportBook->setVisible(false);
            QTimer::singleShot(100, this, [this](){ 
                scrollArea->ensureWidgetVisible(groupReader); 
            });
            break;
            
        case BOOK_CONTEXT:
            groupReader->setVisible(false);
            groupBook->setVisible(true);
            groupSys->setVisible(true);
            // Show only book reports
            if (btnReportUser) btnReportUser->setVisible(false);
            if (btnReportBook) btnReportBook->setVisible(true);
            QTimer::singleShot(100, this, [this](){ 
                scrollArea->ensureWidgetVisible(groupBook); 
            });
            break;
            
        case ALL:
        default:
            groupReader->setVisible(true);
            groupBook->setVisible(true);
            groupSys->setVisible(true);
            // Show all reports
            if (btnReportUser) btnReportUser->setVisible(true);
            if (btnReportBook) btnReportBook->setVisible(true);
            QTimer::singleShot(100, this, [this](){ 
                scrollArea->ensureWidgetVisible(groupReader); 
            });
            break;
    }
}

ManHinhQuanLyIn::~ManHinhQuanLyIn() {}

void ManHinhQuanLyIn::setBookContext(PTRDS sach) {
    sachHienTai = sach;
}

void ManHinhQuanLyIn::setCurrentTab(int) {
    // Deprecated in Unified View
}

QWidget* ManHinhQuanLyIn::taoTrangTheDocGia() { return nullptr; }
QWidget* ManHinhQuanLyIn::taoTrangTemSach() { return nullptr; }
QWidget* ManHinhQuanLyIn::taoTrangBaoCao() { return nullptr; }

void ManHinhQuanLyIn::thieLapDanhSachChucNang() {}

void ManHinhQuanLyIn::thieLapGiaoDien() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);
    
    // === HEADER ===
    QHBoxLayout *header = new QHBoxLayout();
    QLabel *title = new QLabel(LanguageManager::instance().getText("HEADER_CENTER_COMMAND"));
    title->setStyleSheet("font-size: 20px; font-weight: 800; color: #5D9CEC; text-transform: uppercase; letter-spacing: 1px;");
    header->addWidget(title);
    header->addStretch();
    
    QPushButton *btnConfig = new QPushButton(LanguageManager::instance().getText("BTN_CONFIG_PRINTER"));
    btnConfig->setCursor(Qt::PointingHandCursor);
    btnConfig->setStyleSheet("QPushButton { background-color: #2b2b40; color: #ddd; border: 1px solid #444; padding: 8px 15px; border-radius: 4px; } QPushButton:hover { background: #3E3E42; color: white; }");
    connect(btnConfig, &QPushButton::clicked, this, &ManHinhQuanLyIn::moManHinhCauHinh);
    header->addWidget(btnConfig);
    
    mainLayout->addLayout(header);
    
    // === SCROLLABLE DASHBOARD ===
    scrollArea = new QScrollArea(); // Use member
    scrollArea->setWidgetResizable(true);
    QWidget *dashboard = new QWidget();
    scrollArea->setWidget(dashboard);
    
    QVBoxLayout *dashLayout = new QVBoxLayout(dashboard);
    dashLayout->setSpacing(25);
    dashLayout->setContentsMargins(0,0,10,0);
    
    // --- SECTION 1: READER CONTEXT ---
    groupReader = new QGroupBox(LanguageManager::instance().getText("GROUP_READER_MGMT")); // Use member
    QVBoxLayout *readerLayout = new QVBoxLayout(groupReader);
    
    // NEW: Searchable List Widget with Select All
    QHBoxLayout *readerListHeader = new QHBoxLayout();
    readerListHeader->addStretch();
    QPushButton *btnSelectAllReaders = new QPushButton("Chọn tất cả", this);
    btnSelectAllReaders->setCursor(Qt::PointingHandCursor);
    btnSelectAllReaders->setStyleSheet(
        "QPushButton {"
        "   background: transparent;"
        "   color: #5BA3F5;"
        "   border: none;"
        "   font-size: 11px;"
        "   text-decoration: underline;"
        "   padding: 0 5px;"
        "}"
        "QPushButton:hover { color: #3699ff; }"
    );
    readerListHeader->addWidget(btnSelectAllReaders);
    readerLayout->addLayout(readerListHeader);
    
    readerSearchList = new SearchableListWidget(SearchableListWidget::MULTI_SELECT);
    readerLayout->addWidget(readerSearchList);
    
    // Connect Select All toggle
    connect(btnSelectAllReaders, &QPushButton::clicked, this, [this, btnSelectAllReaders](){
        if (readerSearchList) {
            readerSearchList->selectAll(m_readerSelectAllToggle);
            btnSelectAllReaders->setText(m_readerSelectAllToggle ? "Bỏ chọn tất cả" : "Chọn tất cả");
            m_readerSelectAllToggle = !m_readerSelectAllToggle;
        }
    });
    
    // Reader Actions Grid
    QGridLayout *gridReader = new QGridLayout();
    gridReader->setSpacing(10);
    
    // MERGED BUTTON: Just "In Thẻ"
    QPushButton *btnInThe = createActionCard(LanguageManager::instance().getText("BTN_PRINT_CARD"), LanguageManager::instance().getText("DESC_PRINT_CARD"), true);
    // Removed btnInTheBatch
    
    QPushButton *btnInPhieu = createActionCard(LanguageManager::instance().getText("BTN_PRINT_BORROW_SLIP"), LanguageManager::instance().getText("DESC_PRINT_BORROW_SLIP"), true);
    QPushButton *btnInDS = createActionCard(LanguageManager::instance().getText("BTN_EXPORT_LIST_A4"), LanguageManager::instance().getText("DESC_EXPORT_LIST_A4"), true);
    
    // Always enable
    btnInThe->setEnabled(true);
    btnInPhieu->setEnabled(true);
    
    // Connect to Smart Handler
    connect(btnInThe, &QPushButton::clicked, this, &ManHinhQuanLyIn::xuLyInTheDocGia);
    // Removed batch connect
    
    connect(btnInDS, &QPushButton::clicked, this, &ManHinhQuanLyIn::xuLyInDanhSach);
    connect(btnInPhieu, &QPushButton::clicked, this, [this](){
        if(docGiaHienTai) {
            // ... (keep existing logic or update)
             QMessageBox::information(this, LanguageManager::instance().getText("TITLE_INFO"), LanguageManager::instance().getText("MSG_SELECT_READER_FIRST"));
        } else {
             QMessageBox::information(this, LanguageManager::instance().getText("TITLE_INFO"), LanguageManager::instance().getText("MSG_SELECT_READER_FIRST"));
        }
    });

    gridReader->addWidget(btnInThe, 0, 0);
    gridReader->addWidget(btnInPhieu, 0, 1);
    gridReader->addWidget(btnInDS, 0, 2);
    // Removed extra widget
    readerLayout->addLayout(gridReader);
    dashLayout->addWidget(groupReader);
    
    
    // --- SECTION 2: BOOK CONTEXT ---
    groupBook = new QGroupBox(LanguageManager::instance().getText("GROUP_BOOK_MGMT")); // Use member
    QVBoxLayout *bookLayout = new QVBoxLayout(groupBook);
    
    // NEW: Searchable List Widget for Books with Select All
    QHBoxLayout *bookListHeader = new QHBoxLayout();
    bookListHeader->addStretch();
    QPushButton *btnSelectAllBooks = new QPushButton("Chọn tất cả", this);
    btnSelectAllBooks->setCursor(Qt::PointingHandCursor);
    btnSelectAllBooks->setStyleSheet(
        "QPushButton {"
        "   background: transparent;"
        "   color: #5BA3F5;"
        "   border: none;"
        "   font-size: 11px;"
        "   text-decoration: underline;"
        "   padding: 0 5px;"
        "}"
        "QPushButton:hover { color: #3699ff; }"
    );
    bookListHeader->addWidget(btnSelectAllBooks);
    bookLayout->addLayout(bookListHeader);
    
    bookSearchList = new SearchableListWidget(SearchableListWidget::MULTI_SELECT);
    bookLayout->addWidget(bookSearchList);
    
    // Connect Select All toggle
    connect(btnSelectAllBooks, &QPushButton::clicked, this, [this, btnSelectAllBooks](){
        if (bookSearchList) {
            bookSearchList->selectAll(m_bookSelectAllToggle);
            btnSelectAllBooks->setText(m_bookSelectAllToggle ? "Bỏ chọn tất cả" : "Chọn tất cả");
            m_bookSelectAllToggle = !m_bookSelectAllToggle;
        }
    });
    
    QGridLayout *gridBook = new QGridLayout();
    gridBook->setSpacing(10);
    
    // MERGED BUTTON: Just "In Tem"
    QPushButton *btnInTem = createActionCard(LanguageManager::instance().getText("BTN_PRINT_STAMP"), LanguageManager::instance().getText("DESC_PRINT_STAMP"), true);
    // Removed btnInTemBatch
    
    QPushButton *btnInDanhMuc = createActionCard(LanguageManager::instance().getText("BTN_PRINT_CATALOG"), LanguageManager::instance().getText("DESC_PRINT_CATALOG"), true);
    
    // btnInTem->setEnabled(sachHienTai != nullptr);
    btnInTem->setEnabled(true);
    
    connect(btnInTem, &QPushButton::clicked, this, &ManHinhQuanLyIn::xuLyInTemSach);
    // Removed batch connect
    connect(btnInDanhMuc, &QPushButton::clicked, this, &ManHinhQuanLyIn::xuLyInDanhMucSach);
    
    gridBook->addWidget(btnInTem, 0, 0);
    gridBook->addWidget(btnInDanhMuc, 0, 1);
    gridBook->addWidget(new QWidget(), 0, 2); // Spacer 
    // Removed extra widget
    // gridBook->addWidget(btnInTemBatch, 0, 1);
    // gridBook->addWidget(btnInDanhMuc, 0, 2);
    gridBook->addWidget(new QWidget(), 0, 3); // Spacer
    
    bookLayout->addLayout(gridBook);
    dashLayout->addWidget(groupBook);
    
    
    // --- SECTION 3: REPORTS & SYSTEM ---
    groupSys = new QGroupBox(LanguageManager::instance().getText("GROUP_REPORT_STATS")); // Use member
    QGridLayout *gridSys = new QGridLayout(groupSys);
    gridSys->setSpacing(10);
    
    btnReportUser = createActionCard(LanguageManager::instance().getText("BTN_REPORT_READER"), LanguageManager::instance().getText("DESC_REPORT_READER"), false);
    btnReportBook = createActionCard(LanguageManager::instance().getText("BTN_REPORT_BOOK"), LanguageManager::instance().getText("DESC_REPORT_BOOK"), false);
    
    // NEW: Configuration Button (Reused variable from header if possible, or just new local)
    // There was already a btnConfig in header? No, it was local in header section of this function?
    // Let's check upper code. Ah, line 215 had one.
    // Let's assume we want the one in the grid.
    
    QPushButton* btnConfigGrid = createActionCard(LanguageManager::instance().getText("BTN_CONFIG_PRINT"), LanguageManager::instance().getText("DESC_CONFIG_PRINT"), false);
    
    connect(btnReportUser, &QPushButton::clicked, this, &ManHinhQuanLyIn::xuLyInBaoCaoDocGia);
    connect(btnReportBook, &QPushButton::clicked, this, &ManHinhQuanLyIn::xuLyInBaoCaoSach);
    connect(btnConfigGrid, &QPushButton::clicked, this, &ManHinhQuanLyIn::moManHinhCauHinh);
    
    gridSys->addWidget(btnReportUser, 0, 0);
    gridSys->addWidget(btnReportBook, 0, 1);
    gridSys->addWidget(btnConfigGrid, 0, 2); // Add to Grid
    gridSys->addWidget(new QWidget(), 0, 3); // Spacer
    gridSys->addWidget(new QWidget(), 0, 3); // Spacer
    
    dashLayout->addWidget(groupSys);
    
    // Finisher
    dashLayout->addStretch();
    mainLayout->addWidget(scrollArea);
    
    // === FOOTER ===
    QHBoxLayout *footer = new QHBoxLayout();
    footer->addStretch();
    QPushButton *btnClose = new QPushButton(LanguageManager::instance().getText("BTN_CLOSE"));
    btnClose->setCursor(Qt::PointingHandCursor);
    btnClose->setMinimumSize(100, 40);
    btnClose->setStyleSheet("QPushButton { background: #3E3E42; color: white; border: none; font-weight: bold; border-radius: 4px; } QPushButton:hover { background: #555; }");
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    footer->addWidget(btnClose);
    mainLayout->addLayout(footer);
}

// Stub old creation methods to keep linker happy if headers rely on them, 
// OR simpler: Implementation plan says we keep existing slots but UI changes.
// The methods taoTrang... are private helpers, we can just empty them or remove them.
// But we need to keep the SLOTS implementation (xuLyInTheDocGia etc).

// End of helpers section


// End of helpers section

void ManHinhQuanLyIn::xuLyInTheHangLoat() {
    if (!readerSearchList) return;
    
    QVariantList selected = readerSearchList->selectedData();
    QList<PTRDG> danhSachQList;
    
    if (selected.isEmpty()) {
        // NEW LOGIC: Offer to print ALL
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "In Tất Cả?",
            QString("Bạn chưa chọn ai. In tất cả %1 học viên?").arg(m_danhSach.size()),
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
            // Print ALL readers from m_danhSach
            for (PTRDG dg : m_danhSach) {
                if (dg) danhSachQList.append(dg);
            }
        } else {
            return;  // User cancelled
        }
    } else {
        // Print only selected
        for (const QVariant& var : selected) {
            PTRDG dg = var.value<PTRDG>();
            if (dg) danhSachQList.append(dg);
        }
    }
    
    if (danhSachQList.isEmpty()) return;

    if (QMessageBox::question(this, "Xác Nhận", 
            QString("In %1 thẻ học viên?").arg(danhSachQList.size()),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        PrintHelper::inNhieuTheDocGia(danhSachQList, this);
    }
}

// Batch Print Book Labels
void ManHinhQuanLyIn::xuLyInTemHangLoat() {
    if (!bookSearchList) return;

    QVariantList selected = bookSearchList->selectedData();
    QList<QPair<PTRDS, int>> printList;
    
    if (selected.isEmpty()) {
        // NEW LOGIC: Offer to print ALL books
        extern PTRDS dsDauSach[];
        extern int soLuongDauSach;
        
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "In Tất Cả?",
            QString("Bạn chưa chọn sách nào. In tem cho tất cả %1 đầu sách?").arg(soLuongDauSach),
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
            // Print ALL books
            for (int i = 0; i < soLuongDauSach; i++) {
                if (dsDauSach[i]) {
                    printList.append({dsDauSach[i], dsDauSach[i]->tongBanSao});
                }
            }
        } else {
            return;
        }
    } else {
        // Print only selected
        for (const QVariant& var : selected) {
            PTRDS sach = static_cast<PTRDS>(var.value<void*>());
            if (sach) {
                printList.append({sach, sach->tongBanSao});
            }
        }
    }

    if (!printList.isEmpty()) {
        if (QMessageBox::question(this, "Xác Nhận", 
                QString("In %1 nhãn sách?").arg(printList.size()),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            PrintHelper::inNhieuTemSachBatch(printList, this);
        }
    }
}

void ManHinhQuanLyIn::xuLyInTheDocGia() {
    QList<PTRDG> listToPrint;
    bool isPrintAll = false;

    // Check Selection in List
    if (readerSearchList) {
        QVariantList selected = readerSearchList->selectedData();
        if (!selected.isEmpty()) {
            // Priority 1: Selection
            for (const QVariant& var : selected) {
                PTRDG dg = var.value<PTRDG>();
                if (dg) listToPrint.append(dg);
            }
        } else {
            // Priority 2: No Selection -> Print ALL
            // (Ignoring context 'target' because user rule "không chọn in tất cả" is stricter)
            isPrintAll = true; 
            for (auto dg : m_danhSach) {
                if (dg) listToPrint.append(dg);
            }
        }
    }
    
    if (listToPrint.isEmpty()) return;

    // Confirmation
    if (isPrintAll) {
         if (QMessageBox::question(this, "Xác Nhận In", 
                QString("Xác nhận in toàn bộ %1 thẻ học viên?").arg(listToPrint.size()),
                QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
            return;
        }
    }
    // Else (Selection): Maybe confirm if lots? Let's skip confirm for explicit selection (1 or few), 
    // or keep existing behavior? Existing had confirm for >1.
    // Let's confirm for Batch > 5? Or just obey user "không thông báo câu ngu".
    // Printing 2-3 cards is instant preview. No confirm needed for explicit selection.
    
    if (listToPrint.size() == 1) {
        PrintHelper::inTheDocGia(listToPrint.first(), this);
    } else {
        PrintHelper::inNhieuTheDocGia(listToPrint, this);
    }
}

void ManHinhQuanLyIn::xuLyInDanhSach() {
    extern PTRDG rootDocGia;
    PrintHelper::inDanhSachDocGia(rootDocGia, this);
}

void ManHinhQuanLyIn::xuLyInTemSach() {
    QList<QPair<PTRDS, int>> listToPrint;
    bool isPrintAll = false;
    
    // Check Selection
    if (bookSearchList) {
        QVariantList selected = bookSearchList->selectedData();
        if (!selected.isEmpty()) {
             // Priority 1: Selected Items
             for (const QVariant& var : selected) {
                PTRDS sach = static_cast<PTRDS>(var.value<void*>());
                if (sach) listToPrint.append({sach, sach->tongBanSao});
            }
        } else {
            // Priority 2: No Selection -> Print ALL
            isPrintAll = true;
            extern PTRDS dsDauSach[];
            extern int soLuongDauSach;
            for (int i = 0; i < soLuongDauSach; i++) {
                if (dsDauSach[i]) {
                    listToPrint.append({dsDauSach[i], dsDauSach[i]->tongBanSao});
                }
            }
        }
    }
    
    if (listToPrint.isEmpty()) return;

    // Confirmation for Batch
    if (isPrintAll) {
         if (QMessageBox::question(this, "Xác Nhận In", 
                QString("Xác nhận in toàn bộ %1 đầu sách (bao gồm các bản sao)?").arg(listToPrint.size()),
                QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
            return;
        }
    }
    
    // Dispatch
    // Always use batch function for consistency (it handles list of 1 just fine)
    PrintHelper::inNhieuTemSachBatch(listToPrint, this);
}

void ManHinhQuanLyIn::xuLyInDanhMucSach() {
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    if (soLuongDauSach <= 0) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARN"), LanguageManager::instance().getText("MSG_EMPTY_LIBRARY"));
        return;
    }
    PrintHelper::inDanhMucSach(dsDauSach, soLuongDauSach, this);
}

void ManHinhQuanLyIn::xuLyInBaoCaoDocGia() {
    xuLyInDanhSach();
}

void ManHinhQuanLyIn::xuLyInBaoCaoSach() {
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    if (soLuongDauSach <= 0) {
        QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARN"), LanguageManager::instance().getText("MSG_EMPTY_LIBRARY"));
        return;
    }
    PrintHelper::inDanhSachSach(dsDauSach, soLuongDauSach, this);
}

// End of file
void ManHinhQuanLyIn::moManHinhCauHinh() {
    PrintHelper::PrintConfig currentConfig = PrintHelper::getCauHinh();
    HopThoaiCauHinhIn dialog(currentConfig, this);
    if (dialog.exec() == QDialog::Accepted) {
        QMessageBox::information(this, "Thành Công", "Đã lưu cấu hình in ấn!");
    }
}

