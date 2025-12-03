#include "views/WidgetMuonTraSach.h"
#include "models/DocGia.h"
#include "models/QuanLySach.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QDate>
#include <QInputDialog>

extern PTRDG rootDocGia;
extern PTRDS dsDauSach[MAX_DAUSACH];
extern int soLuongDauSach;

// Late fee: 5000 VND per day
const int TIEN_PHAT_MOT_NGAY = 5000;

// ==================== LoanTableModel Implementation ====================

LoanTableModel::LoanTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int LoanTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return loans.size();
}

int LoanTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 7; // Ma Sach, Ten Sach, ISBN, Ngay Muon, Ngay Tra, Qua Han, Phi Phat
}

QVariant LoanTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= loans.size())
        return QVariant();
    
    const LoanInfo &info = loans[index.row()];
    
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return info.maSach;
            case 1: return info.bookName;
            case 2: return info.isbn;
            case 3: return QString::fromStdString(info.loan->data.NgayMuon);
            case 4: return info.loan->data.TrangThai == 0 ? "-" : QString::fromStdString(info.loan->data.NgayTra);
            case 5: return info.overdueDays > 0 ? QString::number(info.overdueDays) + " ngày" : "-";
            case 6: return info.lateFee > 0 ? QString::number(info.lateFee) + " đ" : "-";
        }
    } else if (role == Qt::ForegroundRole) {
        if (index.column() == 5 || index.column() == 6) {
            if (info.overdueDays > 0) {
                return QColor(Qt::red);
            }
        }
    } else if (role == Qt::FontRole) {
        if (info.overdueDays >= 7 && (index.column() == 5 || index.column() == 6)) {
            QFont font;
            font.setBold(true);
            return font;
        }
    }
    
    return QVariant();
}

QVariant LoanTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();
    
    switch (section) {
        case 0: return "Mã Sách";
        case 1: return "Tên Sách";
        case 2: return "ISBN";
        case 3: return "Ngày Mượn";
        case 4: return "Ngày Trả";
        case 5: return "Quá Hạn";
        case 6: return "Phí Phạt";
    }
    return QVariant();
}

void LoanTableModel::setReader(PTRDG reader, PTRDS dsds[], int n) {
    beginResetModel();
    loans.clear();
    
    if (!reader) {
        endResetModel();
        return;
    }
    
    MUONTRA mt = reader->data.dsmt;
    while (mt) {
        if (mt->data.TrangThai == 0 && mt->data.banSaoSach) { // Only borrowing books
            LoanInfo info;
            info.loan = mt;
            info.maSach = QString::fromStdString(mt->data.banSaoSach->maSach);
            
            // Find book name and ISBN
            info.bookName = "N/A";
            info.isbn = "N/A";
            for (int i = 0; i < n; i++) {
                PTRDMS dms = dsds[i]->dms;
                while (dms) {
                    if (dms->maSach == mt->data.banSaoSach->maSach) {
                        info.bookName = QString::fromStdString(dsds[i]->tenSach);
                        info.isbn = QString::fromStdString(dsds[i]->ISBN);
                        break;
                    }
                    dms = dms->next;
                }
                if (info.bookName != "N/A") break;
            }
            
            // Calculate overdue
            QDate borrowDate = QDate::fromString(QString::fromStdString(mt->data.NgayMuon), "dd/MM/yyyy");
            QDate today = QDate::currentDate();
            int daysBorrowed = borrowDate.daysTo(today);
            info.overdueDays = (daysBorrowed > 7) ? (daysBorrowed - 7) : 0;
            info.lateFee = info.overdueDays * TIEN_PHAT_MOT_NGAY;
            
            loans.push_back(info);
        }
        mt = mt->next;
    }
    
    endResetModel();
}

void LoanTableModel::clear() {
    beginResetModel();
    loans.clear();
    endResetModel();
}

MUONTRA LoanTableModel::getLoanAt(int row) const {
    if (row >= 0 && row < loans.size()) {
        return loans[row].loan;
    }
    return nullptr;
}

// ==================== WidgetMuonTraSach Implementation ====================

WidgetMuonTraSach::WidgetMuonTraSach(QWidget *parent)
    : QWidget(parent)
    , currentReader(nullptr)
    , selectedLoan(nullptr)
{
    loanModel = new LoanTableModel(this);
    setupUI();
    setupConnections();
}

WidgetMuonTraSach::~WidgetMuonTraSach() {
}

void WidgetMuonTraSach::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Tieu de
    QLabel *titleLabel = new QLabel("QUẢN LÝ MƯỢN - TRẢ SÁCH", this);
    QFont titleFont;
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Toolbar
    toolbar = new QToolBar(this);
    toolbar->setIconSize(QSize(24, 24));
    toolbar->setStyleSheet("QToolBar { background: #ecf0f1; border-radius: 5px; padding: 5px; }");
    
    QAction *actQuickBorrow = toolbar->addAction("⚡ Mượn Nhanh");
    actQuickBorrow->setToolTip("Quét mã và mượn nhanh (F2)");
    actQuickBorrow->setShortcut(Qt::Key_F2);
    connect(actQuickBorrow, &QAction::triggered, this, &WidgetMuonTraSach::onQuickBorrow);
    
    QAction *actBulkReturn = toolbar->addAction("📦 Trả Hàng Loạt");
    actBulkReturn->setToolTip("Trả nhiều sách cùng lúc");
    connect(actBulkReturn, &QAction::triggered, this, &WidgetMuonTraSach::onBulkReturn);
    
    toolbar->addSeparator();
    
    QAction *actRenew = toolbar->addAction("🔄 Gia Hạn");
    actRenew->setToolTip("Gia hạn sách đã chọn");
    connect(actRenew, &QAction::triggered, this, &WidgetMuonTraSach::onRenewBook);
    
    QAction *actCalculateFees = toolbar->addAction("💰 Tính Phí");
    actCalculateFees->setToolTip("Tính tổng phí phạt");
    connect(actCalculateFees, &QAction::triggered, this, &WidgetMuonTraSach::onCalculateFees);
    
    toolbar->addSeparator();
    
    QAction *actPrintReceipt = toolbar->addAction("🖨️ In Phiếu");
    actPrintReceipt->setToolTip("In phiếu mượn/trả (Ctrl+P)");
    actPrintReceipt->setShortcut(QKeySequence::Print);
    connect(actPrintReceipt, &QAction::triggered, this, &WidgetMuonTraSach::onPrintReceipt);
    
    mainLayout->addWidget(toolbar);
    
    // Reader info panel
    readerInfoLabel = new QLabel(this);
    readerInfoLabel->setStyleSheet(
        "background-color: #e8f4f8; "
        "border: 2px solid #3498db; "
        "border-radius: 5px; "
        "padding: 10px; "
        "font-weight: bold;");
    readerInfoLabel->setText("🔍 Nhập mã thẻ để xem thông tin độc giả...");
    mainLayout->addWidget(readerInfoLabel);
    
    // Group box cho muon/tra sach
    QGroupBox *actionGroup = new QGroupBox("Thao Tác Mượn/Trả", this);
    QFormLayout *formLayout = new QFormLayout(actionGroup);
    
    readerIdEdit = new QLineEdit(this);
    readerIdEdit->setPlaceholderText("Nhập mã thẻ độc giả...");
    formLayout->addRow("Mã thẻ độc giả:", readerIdEdit);
    
    isbnEdit = new QLineEdit(this);
    isbnEdit->setPlaceholderText("Nhập ISBN hoặc mã sách...");
    formLayout->addRow("ISBN / Mã sách:", isbnEdit);
    
    QHBoxLayout *actionButtons = new QHBoxLayout();
    btnBorrow = new QPushButton("📚 Mượn Sách", this);
    btnReturn = new QPushButton("✅ Trả Sách", this);
    btnReportLost = new QPushButton("❌ Báo Mất", this);
    
    btnBorrow->setStyleSheet("background-color: #27ae60; color: white; padding: 8px; font-weight: bold;");
    btnReturn->setStyleSheet("background-color: #3498db; color: white; padding: 8px; font-weight: bold;");
    btnReportLost->setStyleSheet("background-color: #e74c3c; color: white; padding: 8px; font-weight: bold;");
    
    actionButtons->addWidget(btnBorrow);
    actionButtons->addWidget(btnReturn);
    actionButtons->addWidget(btnReportLost);
    formLayout->addRow("", actionButtons);
    
    mainLayout->addWidget(actionGroup);
    
    // Bang hien thi sach dang muon
    QLabel *tableLabel = new QLabel("Danh Sách Sách Đang Mượn:", this);
    tableLabel->setStyleSheet("font-weight: bold; font-size: 12pt;");
    mainLayout->addWidget(tableLabel);
    
    currentLoansTable = new QTableView(this);
    currentLoansTable->setModel(loanModel);
    currentLoansTable->setAlternatingRowColors(true);
    currentLoansTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    currentLoansTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    currentLoansTable->setSelectionMode(QAbstractItemView::SingleSelection);
    currentLoansTable->horizontalHeader()->setStretchLastSection(true);
    currentLoansTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    mainLayout->addWidget(currentLoansTable);
    
    // Thanh trang thai
    statusLabel = new QLabel("", this);
    statusLabel->setStyleSheet("color: #27ae60; font-weight: bold;");
    mainLayout->addWidget(statusLabel);
    
    // Cac nut bao cao
    QHBoxLayout *reportLayout = new QHBoxLayout();
    btnViewOverdue = new QPushButton("📅 Xem Quá Hạn", this);
    btnTopBooks = new QPushButton("🏆 Top 10 Sách", this);
    btnBack = new QPushButton("⬅️ Quay Lại", this);
    
    btnViewOverdue->setStyleSheet("background-color: #e67e22; color: white; padding: 8px; font-weight: bold;");
    btnTopBooks->setStyleSheet("background-color: #9b59b6; color: white; padding: 8px; font-weight: bold;");
    btnBack->setStyleSheet("background-color: #7f8c8d; color: white; padding: 8px; font-weight: bold;");
    
    reportLayout->addWidget(btnViewOverdue);
    reportLayout->addWidget(btnTopBooks);
    reportLayout->addStretch();
    reportLayout->addWidget(btnBack);
    
    mainLayout->addLayout(reportLayout);
}

void WidgetMuonTraSach::setupConnections() {
    connect(btnBorrow, &QPushButton::clicked, this, &WidgetMuonTraSach::onBorrowBookClicked);
    connect(btnReturn, &QPushButton::clicked, this, &WidgetMuonTraSach::onReturnBookClicked);
    connect(btnReportLost, &QPushButton::clicked, this, &WidgetMuonTraSach::onReportLostClicked);
    connect(btnViewOverdue, &QPushButton::clicked, this, &WidgetMuonTraSach::onViewOverdueClicked);
    connect(btnTopBooks, &QPushButton::clicked, this, &WidgetMuonTraSach::onViewTopBooksClicked);
    connect(btnBack, &QPushButton::clicked, this, &WidgetMuonTraSach::onBackClicked);
    
    // Auto-update when reader ID changes
    connect(readerIdEdit, &QLineEdit::textChanged, this, &WidgetMuonTraSach::onReaderIdChanged);
    
    // Track selected loan
    connect(currentLoansTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            [this]() {
                QModelIndexList selected = currentLoansTable->selectionModel()->selectedRows();
                if (!selected.isEmpty()) {
                    selectedLoan = loanModel->getLoanAt(selected.first().row());
                } else {
                    selectedLoan = nullptr;
                }
            });
}

void WidgetMuonTraSach::refreshData() {
    readerIdEdit->clear();
    isbnEdit->clear();
    statusLabel->clear();
}

void WidgetMuonTraSach::showMessage(const QString &message, bool isError) {
    statusLabel->setText(message);
    statusLabel->setStyleSheet(isError ? 
        "color: #e74c3c; font-weight: bold;" : 
        "color: #27ae60; font-weight: bold;");
}

void WidgetMuonTraSach::updateCurrentLoans(int readerId) {
    PTRDG reader = timDocGia(rootDocGia, readerId);
    if (!reader) {
        showMessage("Không tìm thấy độc giả!", true);
        return;
    }
    
    // TODO: Hien thi danh sach sach dang muon
    showMessage(QString("Độc giả %1 %2 đang mượn %3 sách")
        .arg(QString::fromStdString(reader->data.Ho))
        .arg(QString::fromStdString(reader->data.Ten))
        .arg(reader->data.soSachDangMuon));
}

void WidgetMuonTraSach::onBorrowBookClicked() {
    bool ok;
    int maThe = readerIdEdit->text().toInt(&ok);
    
    if (!ok || maThe <= 0) {
        showMessage("Vui lòng nhập mã thẻ hợp lệ!", true);
        return;
    }
    
    QString isbn = isbnEdit->text().trimmed();
    if (isbn.isEmpty()) {
        showMessage("Vui lòng nhập ISBN!", true);
        return;
    }
    
    PTRDG reader = timDocGia(rootDocGia, maThe);
    if (!reader) {
        showMessage("Không tìm thấy độc giả!", true);
        return;
    }
    
    std::string result = MuonSach(reader, isbn.toStdString(), 
                                   LayDanhSachDauSach(), LaySoLuongDauSach());
    
    if (result.empty()) {
        showMessage("Mượn sách thành công!");
        updateCurrentLoans(maThe);
    } else {
        showMessage(QString::fromStdString(result), true);
    }
}

void WidgetMuonTraSach::onReturnBookClicked() {
    bool ok;
    int maThe = readerIdEdit->text().toInt(&ok);
    
    if (!ok || maThe <= 0) {
        showMessage("Vui lòng nhập mã thẻ hợp lệ!", true);
        return;
    }
    
    QString maSach = isbnEdit->text().trimmed();
    if (maSach.isEmpty()) {
        showMessage("Vui lòng nhập mã sách!", true);
        return;
    }
    
    PTRDG reader = timDocGia(rootDocGia, maThe);
    if (!reader) {
        showMessage("Không tìm thấy độc giả!", true);
        return;
    }
    
    std::string result = TraSach(reader, maSach.toStdString(),
                                  LayDanhSachDauSach(), LaySoLuongDauSach());
    
    if (result.empty()) {
        showMessage("Trả sách thành công!");
        updateCurrentLoans(maThe);
    } else {
        showMessage(QString::fromStdString(result), true);
    }
}

void WidgetMuonTraSach::onReportLostClicked() {
    bool ok;
    int maThe = readerIdEdit->text().toInt(&ok);
    
    if (!ok || maThe <= 0) {
        showMessage("Vui lòng nhập mã thẻ hợp lệ!", true);
        return;
    }
    
    QString maSach = isbnEdit->text().trimmed();
    if (maSach.isEmpty()) {
        showMessage("Vui lòng nhập mã sách!", true);
        return;
    }
    
    PTRDG reader = timDocGia(rootDocGia, maThe);
    if (!reader) {
        showMessage("Không tìm thấy độc giả!", true);
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::warning(this,
        "Xác nhận báo mất",
        "Bạn có chắc muốn báo mất sách này?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        std::string result = BaoMatSach(reader, maSach.toStdString(),
                                        LayDanhSachDauSach(), LaySoLuongDauSach());
        
        if (result.empty()) {
            showMessage("Đã báo mất sách!");
            updateCurrentLoans(maThe);
        } else {
            showMessage(QString::fromStdString(result), true);
        }
    }
}

void WidgetMuonTraSach::onViewOverdueClicked() {
    QMessageBox::information(this, "Danh Sách Quá Hạn", "Chức năng đang phát triển...");
}

void WidgetMuonTraSach::onViewTopBooksClicked() {
    QMessageBox::information(this, "Top 10 Sách", "Chức năng đang phát triển...");
}

void WidgetMuonTraSach::onBackClicked() {
    emit backToHome();
}

// ==================== NEW ADVANCED FEATURES ====================

void WidgetMuonTraSach::onReaderIdChanged() {
    QString text = readerIdEdit->text().trimmed();
    if (text.isEmpty()) {
        currentReader = nullptr;
        loanModel->clear();
        readerInfoLabel->setText("🔍 Nhập mã thẻ để xem thông tin độc giả...");
        readerInfoLabel->setStyleSheet(
            "background-color: #e8f4f8; "
            "border: 2px solid #3498db; "
            "border-radius: 5px; "
            "padding: 10px; "
            "font-weight: bold;");
        return;
    }
    
    bool ok;
    int maThe = text.toInt(&ok);
    if (!ok || maThe <= 0) return;
    
    currentReader = timDocGia(rootDocGia, maThe);
    updateReaderInfo();
    
    if (currentReader) {
        loanModel->setReader(currentReader, dsDauSach, soLuongDauSach);
    } else {
        loanModel->clear();
    }
}

void WidgetMuonTraSach::updateReaderInfo() {
    if (!currentReader) {
        readerInfoLabel->setText("❌ Không tìm thấy độc giả với mã thẻ này!");
        readerInfoLabel->setStyleSheet(
            "background-color: #fadbd8; "
            "border: 2px solid #e74c3c; "
            "border-radius: 5px; "
            "padding: 10px; "
            "font-weight: bold; "
            "color: #e74c3c;");
        return;
    }
    
    QString status = currentReader->data.TrangThai ? "✅ Hoạt động" : "🔒 Khóa";
    QString statusColor = currentReader->data.TrangThai ? "#27ae60" : "#e74c3c";
    QString bgColor = currentReader->data.TrangThai ? "#d5f4e6" : "#fadbd8";
    
    int overdueDays = TinhSoNgayQuaHanLonNhat(currentReader);
    QString overdueInfo = "";
    if (overdueDays > 0) {
        overdueInfo = QString(" | ⚠️ Quá hạn: %1 ngày").arg(overdueDays);
    }
    
    readerInfoLabel->setText(QString(
        "👤 <b>%1 %2</b> (Mã: %3) | %4 | 📚 Đang mượn: %5%6")
        .arg(QString::fromStdString(currentReader->data.Ho))
        .arg(QString::fromStdString(currentReader->data.Ten))
        .arg(currentReader->data.MaThe)
        .arg(status)
        .arg(currentReader->data.soSachDangMuon)
        .arg(overdueInfo));
    
    readerInfoLabel->setStyleSheet(QString(
        "background-color: %1; "
        "border: 2px solid %2; "
        "border-radius: 5px; "
        "padding: 10px; "
        "font-weight: bold; "
        "color: %2;").arg(bgColor).arg(statusColor));
}

void WidgetMuonTraSach::onRenewBook() {
    if (!selectedLoan || !currentReader) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng chọn sách cần gia hạn!");
        return;
    }
    
    // Check if overdue
    int overdueDays = calculateLateFee(selectedLoan->data.NgayMuon) / TIEN_PHAT_MOT_NGAY;
    if (overdueDays > 0) {
        QMessageBox::warning(this, "Không thể gia hạn", 
            QString("Sách này đã quá hạn %1 ngày!\n"
                   "Vui lòng trả sách và thanh toán phí phạt trước khi mượn lại.").arg(overdueDays));
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Xác nhận gia hạn",
        "Gia hạn thêm 7 ngày cho sách này?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Update borrow date to today (extends by 7 days from now)
        QDate today = QDate::currentDate();
        selectedLoan->data.NgayMuon = today.toString("dd/MM/yyyy").toStdString();
        
        extern bool duLieuDaThayDoi;
        duLieuDaThayDoi = true;
        
        loanModel->setReader(currentReader, dsDauSach, soLuongDauSach);
        showMessage("✅ Đã gia hạn sách thành công!");
    }
}

void WidgetMuonTraSach::onCalculateFees() {
    if (!currentReader) {
        QMessageBox::warning(this, "Chưa chọn độc giả", "Vui lòng nhập mã thẻ độc giả!");
        return;
    }
    
    int totalFee = 0;
    int overdueCount = 0;
    QString details;
    
    MUONTRA mt = currentReader->data.dsmt;
    while (mt) {
        if (mt->data.TrangThai == 0) { // Borrowing
            int fee = calculateLateFee(mt->data.NgayMuon);
            if (fee > 0) {
                overdueCount++;
                totalFee += fee;
                
                // Find book name
                QString bookName = "N/A";
                if (mt->data.banSaoSach) {
                    for (int i = 0; i < soLuongDauSach; i++) {
                        PTRDMS dms = dsDauSach[i]->dms;
                        while (dms) {
                            if (dms->maSach == mt->data.banSaoSach->maSach) {
                                bookName = QString::fromStdString(dsDauSach[i]->tenSach);
                                break;
                            }
                            dms = dms->next;
                        }
                        if (bookName != "N/A") break;
                    }
                }
                
                int days = fee / TIEN_PHAT_MOT_NGAY;
                details += QString("• %1: %2 ngày = %3 đ\n")
                          .arg(bookName)
                          .arg(days)
                          .arg(QString::number(fee).rightJustified(10, ' '));
            }
        }
        mt = mt->next;
    }
    
    if (totalFee == 0) {
        QMessageBox::information(this, "Phí Phạt 💰", 
            "✅ Độc giả không có sách quá hạn!\nKhông có phí phạt.");
    } else {
        QMessageBox::warning(this, "Phí Phạt 💰",
            QString("Độc giả có <b>%1 sách</b> quá hạn:\n\n%2\n"
                   "<b>Tổng phí phạt: %3 đ</b>")
            .arg(overdueCount)
            .arg(details)
            .arg(QString::number(totalFee).rightJustified(10, ' ')));
    }
}

void WidgetMuonTraSach::onPrintReceipt() {
    if (!currentReader) {
        QMessageBox::warning(this, "Chưa chọn độc giả", "Vui lòng nhập mã thẻ độc giả!");
        return;
    }
    
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize::A5);
    
    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() != QDialog::Accepted) {
        return;
    }
    
    QString html = "<html><head><style>"
                   "body { font-family: Arial, sans-serif; margin: 20px; }"
                   ".header { text-align: center; border-bottom: 3px solid #2c3e50; padding-bottom: 10px; }"
                   ".info { margin: 20px 0; }"
                   "table { width: 100%; border-collapse: collapse; margin-top: 20px; }"
                   "th { background-color: #3498db; color: white; padding: 10px; }"
                   "td { border: 1px solid #ddd; padding: 8px; }"
                   ".total { font-size: 14pt; font-weight: bold; margin-top: 20px; }"
                   ".footer { margin-top: 40px; text-align: right; }"
                   "</style></head><body>";
    
    html += "<div class='header'>";
    html += "<h1>📚 PHIẾU MƯỢN/TRẢ SÁCH</h1>";
    html += "<p>Hệ Thống Quản Lý Thư Viện</p>";
    html += "</div>";
    
    html += "<div class='info'>";
    html += QString("<p><b>Độc giả:</b> %1 %2</p>")
            .arg(QString::fromStdString(currentReader->data.Ho))
            .arg(QString::fromStdString(currentReader->data.Ten));
    html += QString("<p><b>Mã thẻ:</b> %1</p>").arg(currentReader->data.MaThe);
    html += QString("<p><b>Ngày in:</b> %1</p>").arg(QDate::currentDate().toString("dd/MM/yyyy"));
    html += "</div>";
    
    html += "<h3>Danh Sách Sách Đang Mượn:</h3>";
    html += "<table>";
    html += "<tr><th>STT</th><th>Tên Sách</th><th>ISBN</th><th>Ngày Mượn</th><th>Quá Hạn</th><th>Phí Phạt</th></tr>";
    
    MUONTRA mt = currentReader->data.dsmt;
    int stt = 1;
    int totalFee = 0;
    
    while (mt) {
        if (mt->data.TrangThai == 0 && mt->data.banSaoSach) {
            QString bookName = "N/A";
            QString isbn = "N/A";
            
            for (int i = 0; i < soLuongDauSach; i++) {
                PTRDMS dms = dsDauSach[i]->dms;
                while (dms) {
                    if (dms->maSach == mt->data.banSaoSach->maSach) {
                        bookName = QString::fromStdString(dsDauSach[i]->tenSach);
                        isbn = QString::fromStdString(dsDauSach[i]->ISBN);
                        break;
                    }
                    dms = dms->next;
                }
                if (bookName != "N/A") break;
            }
            
            int fee = calculateLateFee(mt->data.NgayMuon);
            totalFee += fee;
            int overdueDays = fee / TIEN_PHAT_MOT_NGAY;
            
            html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6 đ</td></tr>")
                    .arg(stt++)
                    .arg(bookName)
                    .arg(isbn)
                    .arg(QString::fromStdString(mt->data.NgayMuon))
                    .arg(overdueDays > 0 ? QString::number(overdueDays) + " ngày" : "-")
                    .arg(fee);
        }
        mt = mt->next;
    }
    
    html += "</table>";
    
    if (totalFee > 0) {
        html += QString("<div class='total'>💰 Tổng phí phạt: <span style='color: red;'>%1 đ</span></div>")
                .arg(QString::number(totalFee).rightJustified(10, ' '));
    }
    
    html += "<div class='footer'>";
    html += "<p>Thủ thư</p>";
    html += "<p>(Ký tên)</p>";
    html += "</div>";
    
    html += "</body></html>";
    
    QTextDocument document;
    document.setHtml(html);
    document.print(&printer);
    
    showMessage("✅ Đã in phiếu mượn/trả!");
}

void WidgetMuonTraSach::onQuickBorrow() {
    QMessageBox::information(this, "Mượn Nhanh ⚡", 
        "Tính năng mượn nhanh với máy quét mã vạch sẽ được triển khai trong phiên bản sau.\n\n"
        "Hiện tại vui lòng sử dụng chức năng mượn thông thường.");
}

void WidgetMuonTraSach::onBulkReturn() {
    if (!currentReader) {
        QMessageBox::warning(this, "Chưa chọn độc giả", "Vui lòng nhập mã thẻ độc giả!");
        return;
    }
    
    if (currentReader->data.soSachDangMuon == 0) {
        QMessageBox::information(this, "Trả Hàng Loạt", "Độc giả không có sách nào đang mượn!");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Xác nhận trả hàng loạt",
        QString("Bạn có muốn trả <b>TẤT CẢ %1 sách</b> đang mượn?").arg(currentReader->data.soSachDangMuon),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        int returnedCount = 0;
        int totalFee = 0;
        
        MUONTRA mt = currentReader->data.dsmt;
        while (mt) {
            if (mt->data.TrangThai == 0 && mt->data.banSaoSach) {
                // Calculate fee before returning
                totalFee += calculateLateFee(mt->data.NgayMuon);
                
                // Mark as returned
                mt->data.TrangThai = 1;
                mt->data.NgayTra = QDate::currentDate().toString("dd/MM/yyyy").toStdString();
                mt->data.banSaoSach->trangThai = CHO_MUON_DUOC;
                
                returnedCount++;
            }
            mt = mt->next;
        }
        
        currentReader->data.soSachDangMuon = 0;
        
        extern bool duLieuDaThayDoi;
        duLieuDaThayDoi = true;
        
        loanModel->setReader(currentReader, dsDauSach, soLuongDauSach);
        updateReaderInfo();
        
        QString message = QString("✅ Đã trả <b>%1 sách</b> thành công!").arg(returnedCount);
        if (totalFee > 0) {
            message += QString("\n\n💰 Tổng phí phạt: <b>%1 đ</b>").arg(totalFee);
        }
        
        QMessageBox::information(this, "Trả Hàng Loạt", message);
        showMessage(QString("Đã trả %1 sách").arg(returnedCount));
    }
}

int WidgetMuonTraSach::calculateLateFee(const std::string& ngayMuon) {
    QDate borrowDate = QDate::fromString(QString::fromStdString(ngayMuon), "dd/MM/yyyy");
    QDate today = QDate::currentDate();
    int daysBorrowed = borrowDate.daysTo(today);
    int overdueDays = (daysBorrowed > 7) ? (daysBorrowed - 7) : 0;
    return overdueDays * TIEN_PHAT_MOT_NGAY;
}

