#include "views/dialogs/ManHinhBaoCaoQuaHan.h"
#include "models/DocGia.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>

extern PTRDG rootDocGia;

ManHinhBaoCaoQuaHan::ManHinhBaoCaoQuaHan(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("⚠️ Danh Sách Độc Giả Quá Hạn");
    setMinimumSize(850, 600);
    setModal(true);

    thietLapGiaoDien();
    apDungPhongCachHienDai();
    taiDuLieuQuaHan();
}

void ManHinhBaoCaoQuaHan::thietLapGiaoDien() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title with icon
    QLabel* titleLabel = new QLabel("⚠️ DANH SÁCH ĐỘC GIẢ QUÁ HẠN TRẢ SÁCH");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 20px; "
        "font-weight: bold; "
        "padding: 15px; "
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 #e74c3c, stop:1 #c0392b); "
        "color: white; "
        "border-radius: 8px; "
        "margin-bottom: 10px;"
    );
    mainLayout->addWidget(titleLabel);

    // Warning label
    QLabel* warningLabel = new QLabel(
        "⚠️ <b>Cảnh báo:</b> Những độc giả dưới đây đang mượn sách quá hạn. "
        "Vui lòng liên hệ để nhắc nhở trả sách!"
    );
    warningLabel->setWordWrap(true);
    warningLabel->setStyleSheet(
        "color: #c0392b; "
        "font-size: 13px; "
        "padding: 10px; "
        "background-color: #fadbd8; "
        "border-left: 4px solid #e74c3c; "
        "border-radius: 5px;"
    );
    mainLayout->addWidget(warningLabel);

    // Table
    bangKetQua = new QTableWidget(this);
    bangKetQua->setColumnCount(5);
    bangKetQua->setHorizontalHeaderLabels({
        "🎫 Mã Thẻ",
        "👤 Họ Tên", 
        "⚧ Giới Tính",
        "⏰ Số Ngày Quá Hạn (Max)",
        "📊 Trạng Thái"
    });
    
    bangKetQua->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangKetQua->setSelectionMode(QAbstractItemView::SingleSelection);
    bangKetQua->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bangKetQua->setAlternatingRowColors(true);
    bangKetQua->horizontalHeader()->setStretchLastSection(true);
    bangKetQua->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    bangKetQua->verticalHeader()->setVisible(false);
    
    // Set column widths
    bangKetQua->setColumnWidth(0, 120); // Card ID
    bangKetQua->setColumnWidth(1, 250); // Name
    bangKetQua->setColumnWidth(2, 100); // Gender
    bangKetQua->setColumnWidth(3, 180); // Overdue days
    
    mainLayout->addWidget(bangKetQua);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    nutLamMoi = new QPushButton("🔄 Làm Mới");
    nutLamMoi->setObjectName("refreshButton");
    nutLamMoi->setMinimumHeight(40);
    connect(nutLamMoi, &QPushButton::clicked, this, &ManHinhBaoCaoQuaHan::taiDuLieuQuaHan);

    nutDong = new QPushButton("❌ Đóng");
    nutDong->setObjectName("closeButton");
    nutDong->setMinimumHeight(40);
    connect(nutDong, &QPushButton::clicked, this, &QDialog::accept);

    buttonLayout->addStretch();
    buttonLayout->addWidget(nutLamMoi);
    buttonLayout->addWidget(nutDong);

    mainLayout->addLayout(buttonLayout);
}

void ManHinhBaoCaoQuaHan::apDungPhongCachHienDai() {
    setStyleSheet(R"(
        QDialog {
            background-color: #ffffff;
        }
        
        QTableWidget {
            background-color: white;
            border: 2px solid #e74c3c;
            border-radius: 8px;
            gridline-color: #fadbd8;
            font-size: 13px;
        }
        
        QTableWidget::item {
            padding: 8px;
            border-bottom: 1px solid #fadbd8;
        }
        
        QTableWidget::item:selected {
            background-color: #e74c3c;
            color: white;
        }
        
        QHeaderView::section {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #e74c3c, stop:1 #c0392b);
            color: white;
            padding: 10px;
            border: none;
            font-weight: bold;
            font-size: 13px;
        }
        
        QHeaderView::section:first {
            border-top-left-radius: 6px;
        }
        
        QHeaderView::section:last {
            border-top-right-radius: 6px;
        }
        
        QPushButton#refreshButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498db, stop:1 #2980b9);
            color: white;
            border: none;
            border-radius: 5px;
            padding: 10px 30px;
            font-weight: bold;
            font-size: 14px;
            min-width: 120px;
        }
        
        QPushButton#refreshButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #5dade2, stop:1 #3498db);
        }
        
        QPushButton#refreshButton:pressed {
            background: #2980b9;
        }
        
        QPushButton#closeButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #7f8c8d, stop:1 #6c7a7b);
            color: white;
            border: none;
            border-radius: 5px;
            padding: 10px 30px;
            font-weight: bold;
            font-size: 14px;
            min-width: 100px;
        }
        
        QPushButton#closeButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #95a5a6, stop:1 #7f8c8d);
        }
        
        QPushButton#closeButton:pressed {
            background: #6c7a7b;
        }
    )");
}

void ManHinhBaoCaoQuaHan::taiDuLieuQuaHan() {
    bangKetQua->setRowCount(0);

    // Prepare array for results (max 1000)
    DocGiaQuaHanDTO overdueReaders[1000];
    
    // Call backend function
    int count = LayDSDocGiaQuaHan(rootDocGia, overdueReaders, 1000);

    if (count == 0) {
        QMessageBox::information(this, "Thông Báo 💡",
            "🎉 Không có độc giả nào quá hạn trả sách!\n\n"
            "Tất cả độc giả đều trả sách đúng hạn.");
        return;
    }

    // Populate table
    bangKetQua->setRowCount(count);
    
    for (int i = 0; i < count; i++) {
        PTRDG reader = overdueReaders[i].docGia;
        int overdueDays = overdueReaders[i].soNgayQuaHanMax;
        
        if (!reader) continue;

        // Card ID
        QTableWidgetItem* cardItem = new QTableWidgetItem(
            QString::number(reader->data.MaThe)
        );
        cardItem->setTextAlignment(Qt::AlignCenter);
        cardItem->setFont(QFont("Courier", 11, QFont::Bold));
        bangKetQua->setItem(i, 0, cardItem);

        // Full name
        QString fullName = QString("%1 %2")
            .arg(QString::fromStdString(reader->data.Ho))
            .arg(QString::fromStdString(reader->data.Ten));
        QTableWidgetItem* nameItem = new QTableWidgetItem(fullName);
        nameItem->setFont(QFont("Arial", 11, QFont::Bold));
        bangKetQua->setItem(i, 1, nameItem);

        // Gender
        QString gender = reader->data.Phai == 0 ? "👨 Nam" : "👩 Nữ";
        QTableWidgetItem* genderItem = new QTableWidgetItem(gender);
        genderItem->setTextAlignment(Qt::AlignCenter);
        bangKetQua->setItem(i, 2, genderItem);

        // Overdue days with color coding
        QTableWidgetItem* daysItem = new QTableWidgetItem(
            QString("%1 ngày").arg(overdueDays)
        );
        daysItem->setTextAlignment(Qt::AlignCenter);
        daysItem->setFont(QFont("Arial", 12, QFont::Bold));
        
        // Color code based on severity
        QColor bgColor, textColor;
        if (overdueDays > 30) {
            bgColor = QColor(231, 76, 60);   // Dark red
            textColor = Qt::white;
        } else if (overdueDays > 14) {
            bgColor = QColor(241, 196, 15);  // Yellow
            textColor = QColor(52, 73, 94);
        } else {
            bgColor = QColor(230, 126, 34);  // Orange
            textColor = Qt::white;
        }
        
        daysItem->setBackground(QBrush(bgColor));
        daysItem->setForeground(QBrush(textColor));
        bangKetQua->setItem(i, 3, daysItem);

        // Status with icon
        QString statusText;
        QColor statusColor;
        if (overdueDays > 30) {
            statusText = "🚨 Nghiêm Trọng";
            statusColor = QColor(231, 76, 60);
        } else if (overdueDays > 14) {
            statusText = "⚠️ Cảnh Báo";
            statusColor = QColor(241, 196, 15);
        } else {
            statusText = "⏰ Nhắc Nhở";
            statusColor = QColor(230, 126, 34);
        }
        
        QTableWidgetItem* statusItem = new QTableWidgetItem(statusText);
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setFont(QFont("Arial", 11, QFont::Bold));
        statusItem->setForeground(QBrush(statusColor));
        bangKetQua->setItem(i, 4, statusItem);

        // Set row height
        bangKetQua->setRowHeight(i, 45);
    }

    // Show warning message
    QString message = QString("⚠️ Phát hiện %1 độc giả quá hạn trả sách!\n\n"
                             "Vui lòng liên hệ để nhắc nhở các độc giả này.")
                         .arg(count);
    QMessageBox::warning(this, "Cảnh Báo ⚠️", message);
}
