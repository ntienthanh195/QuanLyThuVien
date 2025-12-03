#include "views/dialogs/ManHinhTop10DocGia.h"
#include "models/DocGia.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <vector>
#include <algorithm>

extern PTRDG rootDocGia;

// Struct để lưu thông tin độc giả và số lượt mượn
struct DocGiaVaSoLuot {
    PTRDG docGia;
    int soLuotMuon;
    int soDangMuon;
};

ManHinhTop10DocGia::ManHinhTop10DocGia(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Top 10 Độc Giả Hoạt Động Nhiều Nhất");
    setMinimumSize(900, 600);
    setModal(true);

    thietLapGiaoDien();
    apDungPhongCachHienDai();
    taiDuLieuTop10();
}

void ManHinhTop10DocGia::thietLapGiaoDien() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel* titleLabel = new QLabel("TOP 10 ĐỘC GIẢ HOẠT ĐỘNG NHIỀU NHẤT");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 20px; "
        "font-weight: bold; "
        "color: white; "
        "padding: 15px; "
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 #065f46, stop:1 #047857); "
        "border-radius: 8px; "
        "margin-bottom: 10px;"
    );
    mainLayout->addWidget(titleLabel);

    // Info label
    QLabel* infoLabel = new QLabel(
        "<b>Thống kê:</b> Dựa trên tổng số lượt mượn sách của độc giả"
    );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet(
        "color: #1e293b; "
        "font-size: 13px; "
        "padding: 10px; "
        "background-color: #f1f5f9; "
        "border-left: 4px solid #047857; "
        "border-radius: 5px;"
    );
    mainLayout->addWidget(infoLabel);

    // Table
    bangKetQua = new QTableWidget(this);
    bangKetQua->setColumnCount(6);
    bangKetQua->setHorizontalHeaderLabels({
        "Hạng",
        "Mã Thẻ", 
        "Họ và Tên",
        "Giới Tính",
        "Tổng Mượn",
        "Đang Mượn"
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
    bangKetQua->setColumnWidth(1, 100); // ID
    bangKetQua->setColumnWidth(2, 250); // Name
    bangKetQua->setColumnWidth(3, 100); // Gender
    bangKetQua->setColumnWidth(4, 120); // Total
    bangKetQua->setColumnWidth(5, 120); // Current
    
    mainLayout->addWidget(bangKetQua);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    nutLamMoi = new QPushButton("Làm Mới");
    nutLamMoi->setObjectName("refreshButton");
    nutLamMoi->setMinimumHeight(40);
    connect(nutLamMoi, &QPushButton::clicked, this, &ManHinhTop10DocGia::taiDuLieuTop10);

    nutDong = new QPushButton("Đóng");
    nutDong->setObjectName("closeButton");
    nutDong->setMinimumHeight(40);
    connect(nutDong, &QPushButton::clicked, this, &QDialog::accept);

    buttonLayout->addStretch();
    buttonLayout->addWidget(nutLamMoi);
    buttonLayout->addWidget(nutDong);

    mainLayout->addLayout(buttonLayout);
}

void ManHinhTop10DocGia::apDungPhongCachHienDai() {
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
            background-color: #047857;
            color: white;
        }
        
        QHeaderView::section {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #047857, stop:1 #065f46);
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
                stop:0 #047857, stop:1 #065f46);
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
                stop:0 #059669, stop:1 #047857);
        }
        
        QPushButton#refreshButton:pressed {
            background: #065f46;
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

void ManHinhTop10DocGia::taiDuLieuTop10() {
    bangKetQua->setRowCount(0);

    if (!rootDocGia) {
        QMessageBox::information(this, "Thông Báo",
            "Chưa có độc giả nào trong hệ thống.");
        return;
    }

    // Thu thập tất cả độc giả và đếm số lượt mượn
    std::vector<DocGiaVaSoLuot> danhSach;
    
    std::function<void(PTRDG)> duyetCay = [&](PTRDG node) {
        if (!node) return;
        
        duyetCay(node->left);
        
        // Đếm tổng số lượt mượn và số đang mượn
        DocGiaVaSoLuot item;
        item.docGia = node;
        item.soLuotMuon = 0;
        item.soDangMuon = 0;
        
        MUONTRA mt = node->data.dsmt;
        while (mt) {
            item.soLuotMuon++;
            if (mt->data.TrangThai == 0) { // Đang mượn
                item.soDangMuon++;
            }
            mt = mt->next;
        }
        
        if (item.soLuotMuon > 0) {
            danhSach.push_back(item);
        }
        
        duyetCay(node->right);
    };
    
    duyetCay(rootDocGia);

    if (danhSach.empty()) {
        QMessageBox::information(this, "Thông Báo",
            "Chưa có độc giả nào mượn sách.");
        return;
    }

    // Sắp xếp theo số lượt mượn giảm dần
    std::sort(danhSach.begin(), danhSach.end(), 
        [](const DocGiaVaSoLuot& a, const DocGiaVaSoLuot& b) {
            return a.soLuotMuon > b.soLuotMuon;
        });

    // Lấy top 10
    int count = std::min(10, (int)danhSach.size());
    bangKetQua->setRowCount(count);
    
    for (int i = 0; i < count; i++) {
        const DocGiaVaSoLuot& item = danhSach[i];

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

        // Mã thẻ
        QTableWidgetItem* idItem = new QTableWidgetItem(
            QString::number(item.docGia->data.MaThe)
        );
        idItem->setTextAlignment(Qt::AlignCenter);
        bangKetQua->setItem(i, 1, idItem);

        // Họ tên
        QString hoTen = QString::fromStdString(item.docGia->data.Ho + " " + item.docGia->data.Ten);
        QTableWidgetItem* nameItem = new QTableWidgetItem(hoTen);
        bangKetQua->setItem(i, 2, nameItem);

        // Giới tính
        QString gioiTinh = item.docGia->data.Phai ? "Nam" : "Nữ";
        QTableWidgetItem* genderItem = new QTableWidgetItem(gioiTinh);
        genderItem->setTextAlignment(Qt::AlignCenter);
        bangKetQua->setItem(i, 3, genderItem);

        // Tổng lượt mượn
        QTableWidgetItem* totalItem = new QTableWidgetItem(
            QString::number(item.soLuotMuon)
        );
        totalItem->setTextAlignment(Qt::AlignCenter);
        totalItem->setFont(QFont("Arial", 12, QFont::Bold));
        totalItem->setForeground(QBrush(QColor(5, 150, 105))); // Green
        bangKetQua->setItem(i, 4, totalItem);

        // Đang mượn
        QTableWidgetItem* currentItem = new QTableWidgetItem(
            QString::number(item.soDangMuon)
        );
        currentItem->setTextAlignment(Qt::AlignCenter);
        currentItem->setFont(QFont("Arial", 11, QFont::Bold));
        if (item.soDangMuon > 0) {
            currentItem->setForeground(QBrush(QColor(234, 88, 12))); // Orange
        } else {
            currentItem->setForeground(QBrush(QColor(107, 114, 128))); // Gray
        }
        bangKetQua->setItem(i, 5, currentItem);

        // Set row height
        bangKetQua->setRowHeight(i, 45);
    }
}
