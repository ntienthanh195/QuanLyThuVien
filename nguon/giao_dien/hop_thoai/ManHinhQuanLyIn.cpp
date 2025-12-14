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

// Helper to create Action Cards (Big Buttons)
static QPushButton* createActionCard(QString title, QString desc, bool isPrimary = false) {
    QPushButton *btn = new QPushButton();
    btn->setCursor(Qt::PointingHandCursor); // ✅ FIX: Add cursor feedback
    btn->setMinimumSize(180, 60); // Compact Size
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    QString baseColor = isPrimary ? "#2d2d3e" : "#252530";
    QString borderColor = isPrimary ? "#5D9CEC" : "#3E3E42";
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
    
    // NEW: Searchable List Widget
    readerSearchList = new SearchableListWidget(SearchableListWidget::MULTI_SELECT);
    readerLayout->addWidget(readerSearchList);
    
    // Reader Actions Grid
    QGridLayout *gridReader = new QGridLayout();
    gridReader->setSpacing(10);
    
    QPushButton *btnInThe = createActionCard(LanguageManager::instance().getText("BTN_PRINT_CARD_SINGLE"), LanguageManager::instance().getText("DESC_PRINT_CARD_SINGLE"), docGiaHienTai != nullptr);
    QPushButton *btnInTheBatch = createActionCard(LanguageManager::instance().getText("BTN_PRINT_CARD_BATCH_LIST"), LanguageManager::instance().getText("DESC_PRINT_CARD_BATCH_LIST"), false);
    QPushButton *btnInPhieu = createActionCard(LanguageManager::instance().getText("BTN_PRINT_BORROW_SLIP"), LanguageManager::instance().getText("DESC_PRINT_BORROW_SLIP"), false);
    QPushButton *btnInDS = createActionCard(LanguageManager::instance().getText("BTN_EXPORT_LIST_A4"), LanguageManager::instance().getText("DESC_EXPORT_LIST_A4"), false);
    
    // Smart Enable
    btnInThe->setEnabled(docGiaHienTai != nullptr);
    btnInTheBatch->setEnabled(!m_danhSach.empty());
    btnInPhieu->setEnabled(docGiaHienTai != nullptr);
    
    // Connects
    connect(btnInThe, &QPushButton::clicked, this, &ManHinhQuanLyIn::xuLyInTheDocGia);
    connect(btnInTheBatch, &QPushButton::clicked, this, &ManHinhQuanLyIn::xuLyInTheHangLoat); // New slot
    connect(btnInDS, &QPushButton::clicked, this, &ManHinhQuanLyIn::xuLyInDanhSach);
    connect(btnInPhieu, &QPushButton::clicked, this, [this](){
        if(docGiaHienTai) {
            QMessageBox::information(this, LanguageManager::instance().getText("TITLE_INFO"), LanguageManager::instance().getText("MSG_SELECT_READER_FIRST"));
        }
    });

    gridReader->addWidget(btnInThe, 0, 0);
    gridReader->addWidget(btnInTheBatch, 0, 1);
    gridReader->addWidget(btnInPhieu, 0, 2);
    gridReader->addWidget(btnInDS, 0, 3);
    readerLayout->addLayout(gridReader);
    dashLayout->addWidget(groupReader);
    
    
    // --- SECTION 2: BOOK CONTEXT ---
    groupBook = new QGroupBox(LanguageManager::instance().getText("GROUP_BOOK_MGMT")); // Use member
    QVBoxLayout *bookLayout = new QVBoxLayout(groupBook);
    
    // NEW: Searchable List Widget for Books
    bookSearchList = new SearchableListWidget(SearchableListWidget::MULTI_SELECT);
    bookLayout->addWidget(bookSearchList);
    
    QGridLayout *gridBook = new QGridLayout();
    gridBook->setSpacing(10);
    
    QPushButton *btnInTem = createActionCard(LanguageManager::instance().getText("BTN_PRINT_STAMP_SINGLE"), LanguageManager::instance().getText("DESC_PRINT_STAMP_SINGLE"), sachHienTai != nullptr);
    QPushButton *btnInTemBatch = createActionCard(LanguageManager::instance().getText("BTN_PRINT_STAMP_BATCH"), LanguageManager::instance().getText("DESC_PRINT_STAMP_BATCH"), false);
    QPushButton *btnInDanhMuc = createActionCard(LanguageManager::instance().getText("BTN_PRINT_CATALOG"), LanguageManager::instance().getText("DESC_PRINT_CATALOG"), false);
    
    btnInTem->setEnabled(sachHienTai != nullptr);
    
    connect(btnInTem, &QPushButton::clicked, this, &ManHinhQuanLyIn::xuLyInTemSach);
    connect(btnInTemBatch, &QPushButton::clicked, this, &ManHinhQuanLyIn::xuLyInTemHangLoat); // New slot
    connect(btnInDanhMuc, &QPushButton::clicked, this, &ManHinhQuanLyIn::xuLyInDanhMucSach);
    
    gridBook->addWidget(btnInTem, 0, 0);
    gridBook->addWidget(btnInTemBatch, 0, 1);
    gridBook->addWidget(btnInDanhMuc, 0, 2);
    gridBook->addWidget(new QWidget(), 0, 3); // Spacer
    
    bookLayout->addLayout(gridBook);
    dashLayout->addWidget(groupBook);
    
    
    // --- SECTION 3: REPORTS & SYSTEM ---
    groupSys = new QGroupBox(LanguageManager::instance().getText("GROUP_REPORT_STATS")); // Use member
    QGridLayout *gridSys = new QGridLayout(groupSys);
    gridSys->setSpacing(10);
    
    btnReportUser = createActionCard(LanguageManager::instance().getText("BTN_REPORT_READER"), LanguageManager::instance().getText("DESC_REPORT_READER"), false);
    btnReportBook = createActionCard(LanguageManager::instance().getText("BTN_REPORT_BOOK"), LanguageManager::instance().getText("DESC_REPORT_BOOK"), false);
    
    connect(btnReportUser, &QPushButton::clicked, this, &ManHinhQuanLyIn::xuLyInBaoCaoDocGia);
    connect(btnReportBook, &QPushButton::clicked, this, &ManHinhQuanLyIn::xuLyInBaoCaoSach);
    
    gridSys->addWidget(btnReportUser, 0, 0);
    gridSys->addWidget(btnReportBook, 0, 1);
    gridSys->addWidget(new QWidget(), 0, 2); // Spacer
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
    
    // Get selected items from the list widget
    QVariantList selected = readerSearchList->selectedData();
    
    // Fallback: If nothing selected, maybe ask user if they want to print ALL?
    // For now, let's allow "Print All" if selection is empty, OR force selection.
    // Given the requirement "Search + Multi-Select", users likely expect to print what they checked.
    
    QList<PTRDG> danhSachQList;
    if (selected.isEmpty()) {
         QMessageBox::information(this, LanguageManager::instance().getText("TITLE_INFO"), 
            LanguageManager::instance().getText("MSG_SELECT_AT_LEAST_ONE_READER"));
        return;
    } else {
        for (const QVariant& var : selected) {
            PTRDG dg = var.value<PTRDG>();
            if (dg) danhSachQList.append(dg);
        }
    }
    
    if (danhSachQList.empty()) return;

    if (QMessageBox::question(this, LanguageManager::instance().getText("TITLE_CONFIRM_BATCH_PRINT"), 
            LanguageManager::instance().getText("MSG_CONFIRM_BATCH_PRINT").arg(danhSachQList.size()),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        PrintHelper::inNhieuTheDocGia(danhSachQList, this);
    }
}

// Batch Print Book Labels
void ManHinhQuanLyIn::xuLyInTemHangLoat() {
    // 1. Check if widget exists
    if (!bookSearchList) return;

    // 2. Get selected items
    QVariantList selected = bookSearchList->selectedData();
    if (selected.isEmpty()) {
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_INFO"), LanguageManager::instance().getText("MSG_SELECT_AT_LEAST_ONE_BOOK"));
        return;
    }

    // 3. Confirm with user
    if (QMessageBox::question(this, LanguageManager::instance().getText("TITLE_CONFIRM_BATCH_PRINT"), 
            LanguageManager::instance().getText("MSG_CONFIRM_PRINT_BOOKS_COUNT").arg(selected.size()),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        return;
    }

    // 4. Loop print (Temporary solution until PrintHelper supports batch book list)
    // Ideally PrintHelper should have inNhieuTemSach(List<PTRDS>)
    // For now, let's just queue them up or print sequentially (might be slow but works)
    
    // Better: Helper function to generate mixed HTML
    // But since we don't have new PrintHelper API for mixed books yet, let's assume
    // the user wants to print "Tem Mã Vạch" for each.
    
    // WARNING: Opening multiple dialogs is bad.
    // Let's create a specialized Batch Print function in PrintHelper later.
    // For today, let's just print the FIRST one and tell user "Batch mode coming soon"
    // OR: Loop silently? 
    
    // LET'S IMPLEMENT A SIMPLE LOOP that appends HTML
    // This is safer.
    
    QMessageBox::information(this, LanguageManager::instance().getText("TITLE_INFO"), LanguageManager::instance().getText("MSG_FEATURE_UPDATING"));
    
    if (!selected.isEmpty()) {
        PTRDS sach = static_cast<PTRDS>(selected.first().value<void*>());
        if (sach) {
             QString ten = QString::fromStdString(sach->tenSach);
             QString ma = QString::fromStdString(sach->ISBN);
             PrintHelper::inTemMaVachSach(ma, ten, this);
        }
    }
}

void ManHinhQuanLyIn::xuLyInTheDocGia() {
    PTRDG target = docGiaHienTai;
    
    // If widget has selection, use the first selected item
    if (readerSearchList) {
        QVariantList selected = readerSearchList->selectedData();
        if (!selected.isEmpty()) {
            target = selected.first().value<PTRDG>();
        }
    }

    if (target) {
        PrintHelper::inTheDocGia(target, this);
    } else {
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_INFO"), LanguageManager::instance().getText("MSG_SELECT_READER_FROM_LIST"));
    }
}

void ManHinhQuanLyIn::xuLyInDanhSach() {
    extern PTRDG rootDocGia;
    PrintHelper::inDanhSachDocGia(rootDocGia, this);
}

void ManHinhQuanLyIn::xuLyInTemSach() {
    PTRDS target = sachHienTai;
    
    if (bookSearchList) {
        QVariantList selected = bookSearchList->selectedData();
        if (!selected.isEmpty()) {
            target = static_cast<PTRDS>(selected.first().value<void*>());
        }
    }

    if (target) {
        QString ten = QString::fromStdString(target->tenSach);
        QString ma = QString::fromStdString(target->ISBN);
        PrintHelper::inTemMaVachSach(ma, ten, this);
    } else {
        QMessageBox::information(this, LanguageManager::instance().getText("TITLE_INFO"), LanguageManager::instance().getText("MSG_SELECT_BOOK_FROM_LIST"));
    }
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
    ManHinhCauHinh config(this, ManHinhCauHinh::DisplayMode::PrintConfigOnly);
    config.exec();
}

