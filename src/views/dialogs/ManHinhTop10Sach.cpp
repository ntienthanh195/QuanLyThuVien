#include "views/dialogs/ManHinhTop10Sach.h"
#include "models/DocGia.h"
#include "models/DauSach.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>

extern DauSach* dsDauSach[];
extern int soLuongDauSach;

ManHinhTop10Sach::ManHinhTop10Sach(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Top 10 Sách Được Mượn Nhiều Nhất");
    setMinimumSize(900, 600);
    setModal(true);

    thietLapGiaoDien();
    apDungPhongCachHienDai();
    taiDuLieuTop10();
}

void ManHinhTop10Sach::thietLapGiaoDien() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel* titleLabel = new QLabel("TOP 10 SÁCH ĐƯỢC MƯỢN NHIỀU NHẤT");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 20px; "
        "font-weight: bold; "
        "color: white; "
        "padding: 15px; "
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 #1e3a8a, stop:1 #1e40af); "
        "border-radius: 8px; "
        "margin-bottom: 10px;"
    );
    mainLayout->addWidget(titleLabel);

    // Info label
    QLabel* infoLabel = new QLabel(
        "<b>Thống kê:</b> Dựa trên số lượt mượn của từng đầu sách"
    );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet(
        "color: #1e293b; "
        "font-size: 13px; "
        "padding: 10px; "
        "background-color: #f1f5f9; "
        "border-left: 4px solid #1e40af; "
        "border-radius: 5px;"
    );
    mainLayout->addWidget(infoLabel);

    // Table
    bangKetQua = new QTableWidget(this);
    bangKetQua->setColumnCount(6);
    bangKetQua->setHorizontalHeaderLabels({
        "Hạng",
        "ISBN", 
        "Tên Sách",
        "Tác Giả",
        "Thể Loại",
        "Số Lượt Mượn"
    });
    
    bangKetQua->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangKetQua->setSelectionMode(QAbstractItemView::SingleSelection);
    bangKetQua->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bangKetQua->setAlternatingRowColors(true);
    bangKetQua->horizontalHeader()->setStretchLastSection(false);
    bangKetQua->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    bangKetQua->verticalHeader()->setVisible(false);
    
    // Set column widths
    bangKetQua->setColumnWidth(0, 60);  // Rank
    bangKetQua->setColumnWidth(1, 130); // ISBN
    bangKetQua->setColumnWidth(2, 280); // Title
    bangKetQua->setColumnWidth(3, 180); // Author
    bangKetQua->setColumnWidth(4, 120); // Genre
    bangKetQua->setColumnWidth(5, 120); // Count
    
    mainLayout->addWidget(bangKetQua);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    nutLamMoi = new QPushButton("Làm Mới");
    nutLamMoi->setObjectName("refreshButton");
    nutLamMoi->setMinimumHeight(40);
    connect(nutLamMoi, &QPushButton::clicked, this, &ManHinhTop10Sach::taiDuLieuTop10);

    nutDong = new QPushButton("Đóng");
    nutDong->setObjectName("closeButton");
    nutDong->setMinimumHeight(40);
    connect(nutDong, &QPushButton::clicked, this, &QDialog::accept);

    buttonLayout->addStretch();
    buttonLayout->addWidget(nutLamMoi);
    buttonLayout->addWidget(nutDong);

    mainLayout->addLayout(buttonLayout);
}

void ManHinhTop10Sach::apDungPhongCachHienDai() {
    setStyleSheet(R"(
        QDialog {
            background-color: #ffffff;
        }
        
        QTableWidget {
            background-color: white;
            border: 2px solid #bdc3c7;
            border-radius: 8px;
            gridline-color: #ecf0f1;
            font-size: 13px;
        }
        
        QTableWidget::item {
            padding: 8px;
            border-bottom: 1px solid #ecf0f1;
        }
        
        QTableWidget::item:selected {
            background-color: #3498db;
            color: white;
        }
        
        QHeaderView::section {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498db, stop:1 #2980b9);
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

void ManHinhTop10Sach::taiDuLieuTop10() {
    bangKetQua->setRowCount(0);

    // Prepare array for results
    TopSachDTO topBooks[10];
    
    // Call backend function
    int count = LayTopSach(dsDauSach, soLuongDauSach, topBooks);

    if (count == 0) {
        QMessageBox::information(this, "Thông Báo",
            "Chưa có dữ liệu mượn sách nào trong hệ thống.");
        return;
    }

    // Populate table
    bangKetQua->setRowCount(count);
    
    for (int i = 0; i < count; i++) {
        DauSach* book = topBooks[i].dauSach;
        if (!book) continue;

        // Rank with medal icons
        QString rankIcon;
        QColor rankColor;
        if (i == 0) {
            rankIcon = "🥇";
            rankColor = QColor(255, 215, 0); // Gold
        } else if (i == 1) {
            rankIcon = "🥈";
            rankColor = QColor(192, 192, 192); // Silver
        } else if (i == 2) {
            rankIcon = "🥉";
            rankColor = QColor(205, 127, 50); // Bronze
        } else {
            rankIcon = QString::number(i + 1);
            rankColor = QColor(52, 73, 94);
        }

        QTableWidgetItem* rankItem = new QTableWidgetItem(rankIcon);
        rankItem->setTextAlignment(Qt::AlignCenter);
        rankItem->setFont(QFont("Arial", 14, QFont::Bold));
        rankItem->setBackground(QBrush(rankColor.lighter(180)));
        bangKetQua->setItem(i, 0, rankItem);

        // ISBN
        QTableWidgetItem* isbnItem = new QTableWidgetItem(
            QString::fromStdString(book->ISBN)
        );
        isbnItem->setTextAlignment(Qt::AlignCenter);
        bangKetQua->setItem(i, 1, isbnItem);

        // Book name
        QTableWidgetItem* nameItem = new QTableWidgetItem(
            QString::fromStdString(book->tenSach)
        );
        bangKetQua->setItem(i, 2, nameItem);

        // Author
        QTableWidgetItem* authorItem = new QTableWidgetItem(
            QString::fromStdString(book->tacGia)
        );
        bangKetQua->setItem(i, 3, authorItem);

        // Genre
        QTableWidgetItem* genreItem = new QTableWidgetItem(
            QString::fromStdString(book->theLoai)
        );
        genreItem->setTextAlignment(Qt::AlignCenter);
        bangKetQua->setItem(i, 4, genreItem);

        // Borrow count
        QTableWidgetItem* countItem = new QTableWidgetItem(
            QString::number(book->soLuotMuon)
        );
        countItem->setTextAlignment(Qt::AlignCenter);
        countItem->setFont(QFont("Arial", 12, QFont::Bold));
        countItem->setForeground(QBrush(QColor(231, 76, 60))); // Red
        bangKetQua->setItem(i, 5, countItem);

        // Set row height
        bangKetQua->setRowHeight(i, 45);
    }
    
    // Update info label instead of popup
    QString message = QString("Hiển thị %1 đầu sách được mượn nhiều nhất").arg(count);
    // Could update a status label here if needed
}
