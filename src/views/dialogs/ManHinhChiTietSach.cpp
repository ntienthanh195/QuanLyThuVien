#include "views/dialogs/ManHinhChiTietSach.h"
#include "models/QuanLySach.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollBar>

ManHinhChiTietSach::ManHinhChiTietSach(PTRDS book, QWidget *parent)
    : QDialog(parent)
    , sachDuocChon(book)
    , dongBanSaoDuocChon(-1)
{
    setWindowTitle("Chi Tiết Sách");
    setMinimumSize(750, 550);
    thietLapGiaoDien();
    thietLapKetNoi();
    taiThongTinSach();
    taiDanhSachBanSao();
    capNhatTongQuat();
}

ManHinhChiTietSach::~ManHinhChiTietSach() {
}

void ManHinhChiTietSach::thietLapGiaoDien() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Title
    QLabel *titleLabel = new QLabel("📋 CHI TIẾT ĐẦU SÁCH", this);
    QFont titleFont;
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #2c3e50; margin-bottom: 5px;");
    mainLayout->addWidget(titleLabel);
    
    // Book info group
    QGroupBox *infoGroup = new QGroupBox("Thông Tin Chung", this);
    infoGroup->setStyleSheet(
        "QGroupBox {"
        "   border: 2px solid #3498db;"
        "   border-radius: 8px;"
        "   margin-top: 10px;"
        "   padding-top: 15px;"
        "   font-weight: bold;"
        "   background-color: #ffffff;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top left;"
        "   left: 15px;"
        "   top: -10px;"
        "   padding: 0 5px;"
        "   color: #3498db;"
        "   background-color: #ffffff;"
        "}"
    );
    
    QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);
    nhanTongQuat = new QLabel(this);
    nhanTongQuat->setWordWrap(true);
    nhanTongQuat->setStyleSheet("padding: 10px; font-size: 10pt;");
    infoLayout->addWidget(nhanTongQuat);
    mainLayout->addWidget(infoGroup);
    
    // Copies table group
    QGroupBox *copiesGroup = new QGroupBox("Danh Sách Bản Sao", this);
    copiesGroup->setStyleSheet(infoGroup->styleSheet());
    
    QVBoxLayout *copiesLayout = new QVBoxLayout(copiesGroup);
    
    bangBanSao = new QTableWidget(this);
    bangBanSao->setColumnCount(4);
    bangBanSao->setHorizontalHeaderLabels({"Mã Sách", "Trạng Thái", "Vị Trí", "Thao Tác"});
    bangBanSao->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangBanSao->setSelectionMode(QAbstractItemView::SingleSelection);
    bangBanSao->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bangBanSao->setAlternatingRowColors(true);
    bangBanSao->horizontalHeader()->setStretchLastSection(true);
    bangBanSao->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    bangBanSao->verticalHeader()->setVisible(false);
    bangBanSao->setMinimumHeight(250);
    
    bangBanSao->setStyleSheet(
        "QTableWidget {"
        "   border: 1px solid #bdc3c7;"
        "   border-radius: 5px;"
        "   gridline-color: #ecf0f1;"
        "   font-size: 10pt;"
        "}"
        "QTableWidget::item {"
        "   padding: 8px;"
        "}"
        "QTableWidget::item:selected {"
        "   background-color: #3498db;"
        "   color: white;"
        "}"
        "QHeaderView::section {"
        "   background-color: #34495e;"
        "   color: white;"
        "   padding: 10px;"
        "   border: none;"
        "   font-weight: bold;"
        "}"
    );
    
    copiesLayout->addWidget(bangBanSao);
    mainLayout->addWidget(copiesGroup);
    
    // Status label
    nhanTrangThai = new QLabel("", this);
    nhanTrangThai->setAlignment(Qt::AlignCenter);
    nhanTrangThai->setMinimumHeight(30);
    mainLayout->addWidget(nhanTrangThai);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    nutThanhLy = new QPushButton("🗑️ Thanh Lý Bản Sao", this);
    nutThanhLy->setEnabled(false);
    nutThanhLy->setMinimumHeight(40);
    nutThanhLy->setStyleSheet(
        "QPushButton {"
        "   background-color: #e67e22;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   font-size: 10pt;"
        "   font-weight: bold;"
        "   padding: 8px 15px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #d68910;"
        "}"
        "QPushButton:disabled {"
        "   background-color: #95a5a6;"
        "}"
    );
    
    nutLamMoi = new QPushButton("🔄 Làm Mới", this);
    nutLamMoi->setMinimumHeight(40);
    nutLamMoi->setStyleSheet(
        "QPushButton {"
        "   background-color: #3498db;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   font-size: 10pt;"
        "   font-weight: bold;"
        "   padding: 8px 15px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2980b9;"
        "}"
    );
    
    nutDong = new QPushButton("❌ Đóng", this);
    nutDong->setMinimumHeight(40);
    nutDong->setStyleSheet(
        "QPushButton {"
        "   background-color: #7f8c8d;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   font-size: 10pt;"
        "   font-weight: bold;"
        "   padding: 8px 15px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #5d6d7e;"
        "}"
    );
    
    buttonLayout->addWidget(nutThanhLy);
    buttonLayout->addWidget(nutLamMoi);
    buttonLayout->addStretch();
    buttonLayout->addWidget(nutDong);
    mainLayout->addLayout(buttonLayout);
}

void ManHinhChiTietSach::thietLapKetNoi() {
    connect(nutThanhLy, &QPushButton::clicked, this, &ManHinhChiTietSach::khiNhanThanhLy);
    connect(nutLamMoi, &QPushButton::clicked, this, &ManHinhChiTietSach::khiNhanLamMoi);
    connect(nutDong, &QPushButton::clicked, this, &ManHinhChiTietSach::khiNhanDong);
    connect(bangBanSao, &QTableWidget::cellClicked, this, &ManHinhChiTietSach::khiChonBanSao);
}

void ManHinhChiTietSach::taiThongTinSach() {
    if (!sachDuocChon) return;
    
    QString info = QString(
        "📖 <b>Tên sách:</b> %1<br>"
        "✍️ <b>Tác giả:</b> %2<br>"
        "📚 <b>ISBN:</b> %3<br>"
        "📂 <b>Thể loại:</b> %4<br>"
        "📅 <b>Năm xuất bản:</b> %5<br>"
        "📄 <b>Số trang:</b> %6"
    )
    .arg(QString::fromStdString(sachDuocChon->tenSach))
    .arg(QString::fromStdString(sachDuocChon->tacGia))
    .arg(QString::fromStdString(sachDuocChon->ISBN))
    .arg(QString::fromStdString(sachDuocChon->theLoai))
    .arg(sachDuocChon->namXuatBan)
    .arg(sachDuocChon->soTrang);
    
    nhanTongQuat->setText(info);
}

void ManHinhChiTietSach::taiDanhSachBanSao() {
    bangBanSao->setRowCount(0);
    
    if (!sachDuocChon) return;
    
    // Get sorted copies
    PTRDMS sortedCopies[1000];
    int count = LayDanhSachBanSaoSapXep(sachDuocChon->ISBN, sortedCopies, 1000);
    
    bangBanSao->setRowCount(count);
    
    for (int i = 0; i < count; i++) {
        PTRDMS copy = sortedCopies[i];
        if (!copy) continue;
        
        // Ma sach
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::fromStdString(copy->maSach));
        idItem->setTextAlignment(Qt::AlignCenter);
        bangBanSao->setItem(i, 0, idItem);
        
        // Trang thai with icon and color
        QString statusIcon = layBieuTuongTrangThai(copy->trangThai);
        QString statusText = layVanBanTrangThai(copy->trangThai);
        QString statusColor = layMauTrangThai(copy->trangThai);
        
        QTableWidgetItem *statusItem = new QTableWidgetItem(statusIcon + " " + statusText);
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setBackground(QColor(statusColor));
        if (copy->trangThai != THANH_LY) {
            statusItem->setForeground(Qt::white);
        }
        bangBanSao->setItem(i, 1, statusItem);
        
        // Vi tri
        QString location = QString::fromStdString(copy->viTri);
        if (location.isEmpty()) location = "Chưa xác định";
        QTableWidgetItem *locationItem = new QTableWidgetItem(location);
        locationItem->setTextAlignment(Qt::AlignCenter);
        bangBanSao->setItem(i, 2, locationItem);
        
        // Action button (only for CHO_MUON_DUOC status)
        QString action = (copy->trangThai == CHO_MUON_DUOC) ? "Có thể thanh lý" : "-";
        QTableWidgetItem *actionItem = new QTableWidgetItem(action);
        actionItem->setTextAlignment(Qt::AlignCenter);
        bangBanSao->setItem(i, 3, actionItem);
    }
    
    bangBanSao->resizeColumnsToContents();
}

void ManHinhChiTietSach::capNhatTongQuat() {
    if (!sachDuocChon) return;
    
    ThongTinDauSach info = LayThongTinDauSach(sachDuocChon->ISBN);
    
    QString summary = QString(
        "📊 <b>Tổng bản sao:</b> %1 | "
        "✅ <b>Có thể mượn:</b> %2 | "
        "📖 <b>Đang mượn:</b> %3 | "
        "🗑️ <b>Đã thanh lý:</b> %4"
    )
    .arg(info.tongBanSao)
    .arg(info.choMuonDuoc)
    .arg(info.dangMuon)
    .arg(info.thanhLy);
    
    nhanTrangThai->setText(summary);
    nhanTrangThai->setStyleSheet("color: #2c3e50; font-weight: bold; padding: 8px; background-color: #ecf0f1; border-radius: 5px;");
}

QString ManHinhChiTietSach::layBieuTuongTrangThai(TrangThaiSach status) {
    switch (status) {
        case CHO_MUON_DUOC: return "✅";
        case DANG_MUON: return "📖";
        case THANH_LY: return "🗑️";
        default: return "❓";
    }
}

QString ManHinhChiTietSach::layVanBanTrangThai(TrangThaiSach status) {
    switch (status) {
        case CHO_MUON_DUOC: return "Có thể mượn";
        case DANG_MUON: return "Đang được mượn";
        case THANH_LY: return "Đã thanh lý";
        default: return "Không rõ";
    }
}

QString ManHinhChiTietSach::layMauTrangThai(TrangThaiSach status) {
    switch (status) {
        case CHO_MUON_DUOC: return "#27ae60";  // Green
        case DANG_MUON: return "#3498db";      // Blue
        case THANH_LY: return "#bdc3c7";       // Gray
        default: return "#95a5a6";
    }
}

void ManHinhChiTietSach::hienThiThongBao(const QString &message, bool isError) {
    nhanTrangThai->setText((isError ? "❌ " : "✅ ") + message);
    nhanTrangThai->setStyleSheet(
        QString("color: %1; font-weight: bold; padding: 8px; background-color: %2; border-radius: 5px;")
        .arg(isError ? "#e74c3c" : "#27ae60")
        .arg(isError ? "#fadbd8" : "#d5f4e6")
    );
}

void ManHinhChiTietSach::khiChonBanSao(int row, int column) {
    Q_UNUSED(column);
    
    if (row < 0 || row >= bangBanSao->rowCount()) {
        dongBanSaoDuocChon = -1;
        maBanSaoDuocChon.clear();
        nutThanhLy->setEnabled(false);
        return;
    }
    
    dongBanSaoDuocChon = row;
    QTableWidgetItem *idItem = bangBanSao->item(row, 0);
    if (idItem) {
        maBanSaoDuocChon = idItem->text().toStdString();
        
        // Check if can retire - must be available (status = 0)
        QTableWidgetItem *statusItem = bangBanSao->item(row, 1);
        if (statusItem && statusItem->text() == "Có sẵn") {
            nutThanhLy->setEnabled(true);
        } else {
            nutThanhLy->setEnabled(false);
        }
    }
}

void ManHinhChiTietSach::khiNhanThanhLy() {
    if (maBanSaoDuocChon.empty()) {
        hienThiThongBao("Vui lòng chọn bản sao cần thanh lý!", true);
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::warning(
        this,
        "Xác nhận thanh lý",
        QString("Bạn có chắc muốn thanh lý bản sao:\n%1?\n\n"
                "⚠️ Hành động này không thể hoàn tác!")
            .arg(QString::fromStdString(maBanSaoDuocChon)),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        std::string result = ThanhLyBanSaoTheoMa(maBanSaoDuocChon);
        
        if (result.empty()) {
            hienThiThongBao("Đã thanh lý bản sao thành công!");
            taiDanhSachBanSao();
            capNhatTongQuat();
            emit sachDaCapNhat();
            dongBanSaoDuocChon = -1;
            maBanSaoDuocChon.clear();
            nutThanhLy->setEnabled(false);
        } else {
            hienThiThongBao(QString::fromStdString(result), true);
        }
    }
}

void ManHinhChiTietSach::khiNhanLamMoi() {
    taiDanhSachBanSao();
    capNhatTongQuat();
    hienThiThongBao("Đã làm mới dữ liệu!");
}

void ManHinhChiTietSach::khiNhanDong() {
    accept();
}
