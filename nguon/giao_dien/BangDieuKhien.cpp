#include "giao_dien/BangDieuKhien.h"
#include "giao_dien/hop_thoai/HopThoaiThongKe.h"
#include "giao_dien/hop_thoai/HopThoaiTop10.h"
#include "giao_dien/hop_thoai/HopThoaiQuetMaVach.h"
#include "giao_dien/hop_thoai/HopThoaiIn.h"
#include "giao_dien/hop_thoai/HopThoaiInAn.h"
#include "giao_dien/hop_thoai/HopThoaiHangDoiIn.h"
#include "models/QuanLySach.h"
#include "models/DocGia.h"
#include "utils/PrintHelper.h"
#include "utils/EventBus.h"
#if defined(__has_include)
#  if __has_include("utils/DBBatcher.h")
#    include "utils/DBBatcher.h"
#  endif
#endif
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTableView>
#include <QAbstractTableModel>
#include <vector>
#include <algorithm>
#include <QHeaderView>
#include <QScrollArea>
#include <QFont>
#include <QPieSeries>
#include <QChart>
#include <QInputDialog>
#include <QMessageBox>

namespace {
    // File-local ActivityModel used by the dashboard activity view
    struct GiaoDichInfoLocal { QString thoiGian; QString tenDocGia; QString tenSach; int trangThai; };
    class ActivityModel : public QAbstractTableModel {
    public:
        using Item = GiaoDichInfoLocal;
        ActivityModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {}
        int rowCount(const QModelIndex &parent = QModelIndex()) const override { Q_UNUSED(parent); return (int)m_items.size(); }
        int columnCount(const QModelIndex &parent = QModelIndex()) const override { Q_UNUSED(parent); return 4; }
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
            if (!index.isValid()) return QVariant();
            int r = index.row(); if (r < 0 || r >= (int)m_items.size()) return QVariant();
            const Item &it = m_items[r];
            if (role == Qt::DisplayRole) {
                switch (index.column()) {
                    case 0: return it.thoiGian;
                    case 1: return it.tenDocGia;
                    case 2: return it.tenSach;
                    case 3: {
                        if (it.trangThai == 0) return QString("● Mượn");
                        if (it.trangThai == 1) return QString("✓ Trả");
                        return QString("✕ Mất");
                    }
                }
            }
            if (role == Qt::ForegroundRole && index.column() == 3) {
                if (it.trangThai == 0) return QColor("#fbbf24");
                if (it.trangThai == 1) return QColor("#34d399");
                return QColor("#f87171");
            }
            return QVariant();
        }
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
            if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
                switch (section) {
                    case 0: return QStringLiteral("Thời gian");
                    case 1: return QStringLiteral("Tên độc giả");
                    case 2: return QStringLiteral("Tiêu đề sách");
                    case 3: return QStringLiteral("Trạng thái");
                }
            }
            return QAbstractTableModel::headerData(section, orientation, role);
        }
        void setList(const std::vector<Item> &list) { beginResetModel(); m_items = list; endResetModel(); }
        void prepend(const Item &it) { beginResetModel(); m_items.insert(m_items.begin(), it); if (m_items.size() > 30) m_items.resize(30); endResetModel(); }
    private:
        std::vector<Item> m_items;
    };
}

BangDieuKhien::BangDieuKhien(QWidget *cha) : QWidget(cha),
    labelSoLieuTongSach(nullptr),
    labelSoLieuTongDocGia(nullptr),
    labelSoLieuDangMuon(nullptr),
    labelSoLieuQuaHan(nullptr),
    bangHoatDong(nullptr),
    panelCanhBao(nullptr)
{
    qDebug() << "Bắt đầu khởi tạo BangDieuKhien";
    thieLapGiaoDien();
    qDebug() << "Hoàn thành thieLapGiaoDien, bỏ qua capNhatThongKe trong constructor";
    // KHÔNG gọi capNhatThongKe() ở đây - sẽ gọi sau khi window hiển thị

    // Kết nối EventBus để nhận các sự kiện và ghi vào bảng Hoạt động gần đây
    try {
        connect(&EventBus::instance(), &EventBus::docGiaAdded, this, [this](int maThe){
            QString msg = QString("Thêm độc giả: Mã thẻ %1").arg(maThe);
            appendActivity(msg);
            capNhatRealtimeTuWidget();
        });
        connect(&EventBus::instance(), &EventBus::docGiaUpdated, this, [this](int maThe){
            QString msg = QString("Cập nhật độc giả: Mã thẻ %1").arg(maThe);
            appendActivity(msg);
            capNhatRealtimeTuWidget();
        });
        connect(&EventBus::instance(), &EventBus::docGiaRemoved, this, [this](int maThe){
            QString msg = QString("Xóa độc giả: Mã thẻ %1").arg(maThe);
            appendActivity(msg);
            capNhatRealtimeTuWidget();
        });

        connect(&EventBus::instance(), &EventBus::dauSachAdded, this, [this](const QString &isbn){
            QString msg = QString("Thêm đầu sách: %1").arg(isbn);
            appendActivity(msg);
            capNhatRealtimeTuWidget();
        });
        connect(&EventBus::instance(), &EventBus::dauSachUpdated, this, [this](const QString &isbn){
            QString msg = QString("Cập nhật đầu sách: %1").arg(isbn);
            appendActivity(msg);
            capNhatRealtimeTuWidget();
        });
        connect(&EventBus::instance(), &EventBus::dauSachRemoved, this, [this](const QString &isbn){
            QString msg = QString("Xóa đầu sách: %1").arg(isbn);
            appendActivity(msg);
            capNhatRealtimeTuWidget();
        });

        connect(&EventBus::instance(), &EventBus::muonTraAdded, this, [this](int maThe, const QString &maSach){
            QString msg = QString("Mượn sách: Mã thẻ %1 - Mã sách %2").arg(maThe).arg(maSach);
            appendActivity(msg);
            capNhatRealtimeTuWidget();
        });
        connect(&EventBus::instance(), &EventBus::muonTraUpdated, this, [this](int maThe, const QString &maSach){
            QString msg = QString("Cập nhật mượn/trả: Mã thẻ %1 - Mã sách %2").arg(maThe).arg(maSach);
            appendActivity(msg);
            capNhatRealtimeTuWidget();
        });
        connect(&EventBus::instance(), &EventBus::muonTraRemoved, this, [this](int maThe, const QString &maSach){
            QString msg = QString("Xóa giao dịch mượn/trả: Mã thẻ %1 - Mã sách %2").arg(maThe).arg(maSach);
            appendActivity(msg);
            capNhatRealtimeTuWidget();
        });

        connect(&EventBus::instance(), &EventBus::dataChanged, this, [this](){
            QString msg = QString("Dữ liệu bên ngoài thay đổi (import/backup/khác)");
            appendActivity(msg);
            capNhatRealtimeTuWidget();
        });
    } catch (...) {
        qWarning() << "Không thể kết nối EventBus trong BangDieuKhien constructor";
    }
}

BangDieuKhien::~BangDieuKhien() {}

void BangDieuKhien::thieLapGiaoDien() {
    // Đặt màu nền xanh nhạt gradient giống Quản lý sách
    setStyleSheet(
        "QWidget#BangDieuKhien {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "        stop:0 #1a1d29, stop:0.5 #1e2230, stop:1 #1a1d29);"
        "}"
    );
    setObjectName("BangDieuKhien");
    
    QVBoxLayout *boTriChinh = new QVBoxLayout(this);
    boTriChinh->setContentsMargins(30, 30, 30, 30);
    boTriChinh->setSpacing(20);
    
    // Tạo các thẻ thống kê (4 cards ở trên)
    taoTheThongKe(boTriChinh);
    
    // Tạo layout 3 cột: Hoạt động gần đây + Cảnh báo & Tác vụ + In Ấn
    QHBoxLayout *layoutNoiDung = new QHBoxLayout();
    layoutNoiDung->setSpacing(20);
    
    // Bảng hoạt động gần đây (bên trái)
    QWidget *khungHoatDong = taoBangHoatDongGanDay();
    khungHoatDong->setMinimumWidth(500);
    
    // Panel cảnh báo & tác vụ (bên phải) - Đã tích hợp In Ấn vào đây
    QWidget *khungCanhBao = taoPanelCanhBao();
    khungCanhBao->setFixedWidth(320);
    
    layoutNoiDung->addWidget(khungHoatDong, 1);
    layoutNoiDung->addWidget(khungCanhBao);
    
    boTriChinh->addLayout(layoutNoiDung, 1);
}

void BangDieuKhien::taoTheThongKe(QVBoxLayout *boTri) {
    // Lấy thống kê từ backend
    extern PTRDG rootDocGia;
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    // KIỂM TRA DỮ LIỆU HỢP LỆ - QUAN TRỌNG ĐỂ TRÁNH CRASH
    if (!boTri) {
        qCritical() << "BangDieuKhien::taoTheThongKe - boTri is NULL!";
        return;
    }
    
    if (soLuongDauSach < 0 || soLuongDauSach > MAX_DAUSACH) {
        qWarning() << "BangDieuKhien: Dữ liệu sách không hợp lệ!" << soLuongDauSach;
        // Hiển thị thẻ với giá trị 0
        QHBoxLayout *layoutThe = new QHBoxLayout();
        layoutThe->setSpacing(15);
        layoutThe->addWidget(taoTheNho("■", "0", "Tổng số sách", "#3B82F6"));
        layoutThe->addWidget(taoTheNho("◆", "0", "Tổng độc giả", "#10B981"));
        layoutThe->addWidget(taoTheNho("●", "0", "Đang mượn", "#F59E0B"));
        layoutThe->addWidget(taoTheNho("▲", "0", "Sách quá hạn", "#EF4444"));
        boTri->addLayout(layoutThe);
        return;
    }
    
    // Kiểm tra con trỏ dsDauSach không bị lỗi
    bool dataSafe = true;
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] == nullptr) {
            qWarning() << "dsDauSach[" << i << "] is NULL - dữ liệu không an toàn";
            dataSafe = false;
            break;
        }
    }
    
    if (!dataSafe) {
        qWarning() << "BangDieuKhien: Phát hiện con trỏ NULL trong dsDauSach";
    }
    
    qDebug() << "BangDieuKhien: Đang lấy thống kê với" << soLuongDauSach << "đầu sách";
    
    ThongKeSach tkSach = {0, 0, 0, 0, 0, 0};
    ThongKeDocGia tkDocGia = {0, 0, 0, 0, 0};
    
    try {
        tkSach = LayThongKeSach();
        qDebug() << "LayThongKeSach() thành công -" << tkSach.tongSoBanSao << "bản sao";
        
        // QUAN TRỌNG: Chỉ gọi LayThongKeDocGia nếu dataSafe
        if (dataSafe && rootDocGia != nullptr) {
            tkDocGia = LayThongKeDocGia(rootDocGia, dsDauSach, soLuongDauSach);
            qDebug() << "LayThongKeDocGia() thành công -" << tkDocGia.tongSoDocGia << "độc giả";
        } else {
            qWarning() << "Bỏ qua LayThongKeDocGia do dữ liệu không an toàn hoặc rootDocGia NULL";
        }
    } catch (const std::exception& e) {
        qCritical() << "Lỗi khi lấy thống kê:" << e.what();
        // Không return, tiếp tục hiển thị với giá trị 0
    } catch (...) {
        qCritical() << "Lỗi không xác định khi lấy thống kê";
        // Không return, tiếp tục hiển thị với giá trị 0
    }
    
    // Layout 4 cột cho 4 thẻ
    QHBoxLayout *layoutThe = new QHBoxLayout();
    layoutThe->setSpacing(15);
    
    // 4 thẻ với icon và màu riêng - LƯU CÁC LABEL ĐỂ CẬP NHẬT SAU
    QWidget *theTongSach = taoTheNho("■", QString::number(tkSach.tongSoBanSao), "Tổng số sách", "#3B82F6", &labelSoLieuTongSach);
    QWidget *theTongDocGia = taoTheNho("◆", QString::number(tkDocGia.tongSoDocGia), "Tổng độc giả", "#10B981", &labelSoLieuTongDocGia);
    QWidget *theDangMuon = taoTheNho("●", QString::number(tkSach.soBanSaoDangMuon), "Đang mượn", "#F59E0B", &labelSoLieuDangMuon);
    QWidget *theQuaHan = taoTheNho("▲", QString::number(tkDocGia.docGiaCoSachQuaHan), "Sách quá hạn", "#EF4444", &labelSoLieuQuaHan);
    
    layoutThe->addWidget(theTongSach);
    layoutThe->addWidget(theTongDocGia);
    layoutThe->addWidget(theDangMuon);
    layoutThe->addWidget(theQuaHan);
    
    boTri->addLayout(layoutThe);
}

QWidget* BangDieuKhien::taoTheNho(const QString &icon, const QString &soLieu, const QString &nhan, const QString &mau, QLabel **outLabelSoLieu) {
    QWidget *the = new QWidget();
    the->setMinimumHeight(140);
    the->setStyleSheet(QString(
        "QWidget {"
        "    background-color: #1e2230;"
        "    border: 1px solid #2d3140;"
        "    border-radius: 12px;"
        "}"
    ));
    
    QVBoxLayout *boTri = new QVBoxLayout(the);
    boTri->setContentsMargins(20, 20, 20, 20);
    boTri->setSpacing(10);
    
    // Icon với background màu mờ (opacity 15%)
    QLabel *labelIcon = new QLabel();
    labelIcon->setFixedSize(56, 56);
    labelIcon->setText(icon);
    labelIcon->setAlignment(Qt::AlignCenter);
    
    // Chuyển màu hex sang RGBA với opacity 15%
    QColor mauGoc(mau);
    QString mauMo = QString("rgba(%1, %2, %3, 0.15)")
        .arg(mauGoc.red())
        .arg(mauGoc.green())
        .arg(mauGoc.blue());
    
    labelIcon->setStyleSheet(QString(
        "background-color: %1;"
        "border-radius: 28px;"
        "font-size: 26px;"
        "color: %2;"
        "border: none;"
    ).arg(mauMo).arg(mau));
    
    // Số liệu lớn - LƯU POINTER ĐỂ CẬP NHẬT SAU
    QLabel *labelSoLieu = new QLabel(soLieu);
    labelSoLieu->setStyleSheet(
        "font-size: 36px;"
        "font-weight: 700;"
        "color: #FFFFFF;"
        "background: transparent;"
        "border: none;"
    );
    
    // Trả về pointer để cập nhật sau
    if (outLabelSoLieu) {
        *outLabelSoLieu = labelSoLieu;
    }
    
    // Nhãn (không uppercase, màu xám nhẹ hơn)
    QLabel *labelNhan = new QLabel(nhan);
    labelNhan->setStyleSheet(
        "font-size: 12px;"
        "color: #a1a1aa;"
        "background: transparent;"
        "font-weight: 500;"
        "letter-spacing: 0.3px;"
        "border: none;"
    );
    
    boTri->addWidget(labelIcon);
    boTri->addSpacing(5);
    boTri->addWidget(labelSoLieu);
    boTri->addWidget(labelNhan);
    boTri->addStretch();
    
    return the;
}

void BangDieuKhien::taoNutTruyCapNhanh() {
    QWidget *khungNut = new QWidget();
    QHBoxLayout *boTriNut = new QHBoxLayout(khungNut);
    boTriNut->setSpacing(20);
    
    nutQuanLySach = new QPushButton("📖 Quản Lý Sách");
    nutQuanLyDocGia = new QPushButton("👥 Quản Lý Độc Giả");
    nutMuonTraSach = new QPushButton("🔄 Mượn Trả Sách");
    nutXemBaoCao = new QPushButton("📊 Thống Kê");
    
    QPushButton *nutTop10 = new QPushButton("🏆 Top 10");
    
    QString kieuNut = 
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "        stop:0 #3699FF, stop:1 #2080E8);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    padding: 18px 30px;"
        "    font-size: 15px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "        stop:0 #187DE4, stop:1 #1060C0);"
        "    transform: translateY(-2px);"
        "}"
        "QPushButton:pressed {"
        "    background: #0F5FC2;"
        "    transform: translateY(0px);"
        "}";
    
    QString kieuNutXanh = kieuNut;
    kieuNutXanh.replace("#3699FF", "#1BC5BD").replace("#2080E8", "#0F9A94")
               .replace("#187DE4", "#0F9A94").replace("#1060C0", "#0A7A75");
    
    QString kieuNutVang = kieuNut;
    kieuNutVang.replace("#3699FF", "#FFA800").replace("#2080E8", "#E89500")
               .replace("#187DE4", "#D68600").replace("#1060C0", "#B57200");
    
    nutQuanLySach->setStyleSheet(kieuNut);
    nutQuanLyDocGia->setStyleSheet(kieuNut);
    nutMuonTraSach->setStyleSheet(kieuNut);
    nutXemBaoCao->setStyleSheet(kieuNutXanh);
    nutTop10->setStyleSheet(kieuNutVang);
    
    // Thêm shadow cho các nút
    for (auto *nut : {nutQuanLySach, nutQuanLyDocGia, nutMuonTraSach, nutXemBaoCao, nutTop10}) {
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
        shadow->setBlurRadius(15);
        shadow->setXOffset(0);
        shadow->setYOffset(3);
        shadow->setColor(QColor(0, 0, 0, 50));
        nut->setGraphicsEffect(shadow);
    }
    
    boTriNut->addWidget(nutQuanLySach);
    boTriNut->addWidget(nutQuanLyDocGia);
    boTriNut->addWidget(nutMuonTraSach);
    boTriNut->addWidget(nutXemBaoCao);
    boTriNut->addWidget(nutTop10);
    
    boTriChinh->addWidget(khungNut, 3, 0, 1, 4);
    
    // Kết nối signals
    connect(nutQuanLySach, &QPushButton::clicked, this, &BangDieuKhien::xuLyNutQuanLySach);
    connect(nutQuanLyDocGia, &QPushButton::clicked, this, &BangDieuKhien::xuLyNutQuanLyDocGia);
    connect(nutMuonTraSach, &QPushButton::clicked, this, &BangDieuKhien::xuLyNutMuonTraSach);
    connect(nutXemBaoCao, &QPushButton::clicked, this, &BangDieuKhien::xuLyNutXemBaoCao);
    connect(nutTop10, &QPushButton::clicked, this, [this]() {
        HopThoaiTop10 *dialog = new HopThoaiTop10(this);
        dialog->exec();
        delete dialog;
    });
}

void BangDieuKhien::capNhatThongKe() {
    // Kiểm tra dữ liệu trước khi cập nhật
    extern int soLuongDauSach;
    
    if (soLuongDauSach < 0 || soLuongDauSach > MAX_DAUSACH) {
        qWarning() << "capNhatThongKe: Dữ liệu không hợp lệ, bỏ qua cập nhật" << soLuongDauSach;
        return;
    }
    
    qDebug() << "Cập nhật thống kê dashboard...";
    
    // Cập nhật lại toàn bộ giao diện
    try {
        thieLapGiaoDien();
    } catch (const std::exception& e) {
        qCritical() << "Lỗi khi thiết lập giao diện:" << e.what();
    } catch (...) {
        qCritical() << "Lỗi không xác định khi thiết lập giao diện";
    }
}

void BangDieuKhien::capNhatRealtimeTuWidget() {
    qDebug() << "📊 Dashboard nhận realtime update từ widget khác - CẬP NHẬT NGAY LẬP TỨC!";
    
    // Lấy dữ liệu mới nhất
    extern PTRDG rootDocGia;
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    if (soLuongDauSach < 0 || soLuongDauSach > MAX_DAUSACH) {
        qWarning() << "Dữ liệu không hợp lệ, bỏ qua realtime update";
        return;
    }
    
    try {
        // Lấy thống kê mới
        ThongKeSach tkSach = LayThongKeSach();
        ThongKeDocGia tkDocGia = LayThongKeDocGia(rootDocGia, dsDauSach, soLuongDauSach);
        
        // Cập nhật CHỈ TEXT của các label (KHÔNG tạo lại widget)
        if (labelSoLieuTongSach) {
            labelSoLieuTongSach->setText(QString::number(tkSach.tongSoBanSao));
            qDebug() << "  ✓ Cập nhật Tổng sách:" << tkSach.tongSoBanSao;
        }
        
        if (labelSoLieuTongDocGia) {
            labelSoLieuTongDocGia->setText(QString::number(tkDocGia.tongSoDocGia));
            qDebug() << "  ✓ Cập nhật Tổng độc giả:" << tkDocGia.tongSoDocGia;
        }
        
        if (labelSoLieuDangMuon) {
            labelSoLieuDangMuon->setText(QString::number(tkSach.soBanSaoDangMuon));
            qDebug() << "  ✓ Cập nhật Đang mượn:" << tkSach.soBanSaoDangMuon;
        }
        
        if (labelSoLieuQuaHan) {
            labelSoLieuQuaHan->setText(QString::number(tkDocGia.docGiaCoSachQuaHan));
            qDebug() << "  ✓ Cập nhật Quá hạn:" << tkDocGia.docGiaCoSachQuaHan;
        }
        
        // TODO: Cập nhật bảng hoạt động gần đây (nếu cần)
        // Cập nhật bảng hoạt động gần đây: làm mới các hàng (in-place)
        if (bangHoatDong) {
            try {
                // Xây dựng lại danhSachGiaoDich giống như trong taoBangHoatDongGanDay
                PTRDG dsDocGia[100];
                int dem = 0;
#if defined(__has_include) && __has_include("utils/DBBatcher.h")
                std::vector<PTRDG> _tmp = DBBatcher::snapshotDocGia();
                dem = static_cast<int>(std::min<size_t>(_tmp.size(), 100));
                for (int _i = 0; _i < dem; ++_i) dsDocGia[_i] = _tmp[_i];
#else
                DuyetCayRaMang(rootDocGia, dsDocGia, dem);
#endif

                struct GiaoDichInfo { QString thoiGian; QString tenDocGia; QString tenSach; int trangThai; };
                std::vector<GiaoDichInfo> danhSachGiaoDich;

                for (int i = 0; i < dem && (int)danhSachGiaoDich.size() < 20; i++) {
                    if (!dsDocGia[i]) continue;
                    if (!dsDocGia[i]->data.dsmt) continue;
                    auto mt = dsDocGia[i]->data.dsmt;
                    while (mt && (int)danhSachGiaoDich.size() < 20) {
                        GiaoDichInfo gd;
                        gd.thoiGian = QString::fromStdString(mt->data.NgayMuon);
                        gd.tenDocGia = QString::fromStdString(dsDocGia[i]->data.Ho) + " " + QString::fromStdString(dsDocGia[i]->data.Ten);
                        gd.trangThai = mt->data.TrangThai;
                        gd.tenSach = QString();
                        if (mt->data.banSaoSach) {
                            QString maSach = QString::fromStdString(mt->data.banSaoSach->maSach);
                            extern PTRDS dsDauSach[];
                            extern int soLuongDauSach;
                            for (int j = 0; j < soLuongDauSach; j++) {
                                if (dsDauSach[j]) {
                                    PTRDMS banSao = dsDauSach[j]->dms;
                                    while (banSao) {
                                        if (QString::fromStdString(banSao->maSach) == maSach) {
                                            gd.tenSach = QString::fromStdString(dsDauSach[j]->tenSach);
                                            break;
                                        }
                                        banSao = banSao->next;
                                    }
                                    if (!gd.tenSach.isEmpty()) break;
                                }
                            }
                        }
                        danhSachGiaoDich.push_back(gd);
                        mt = mt->next;
                    }
                }

                // Build model data and set into modelHoatDong (if available)
                int maxRows = qMin(9, (int)danhSachGiaoDich.size());
                std::vector<GiaoDichInfoLocal> modelList;
                modelList.reserve(maxRows);
                for (int i = 0; i < maxRows; ++i) {
                    GiaoDichInfoLocal it;
                    it.thoiGian = danhSachGiaoDich[i].thoiGian;
                    it.tenDocGia = danhSachGiaoDich[i].tenDocGia;
                    it.tenSach = danhSachGiaoDich[i].tenSach;
                    it.trangThai = danhSachGiaoDich[i].trangThai;
                    modelList.push_back(it);
                }
                if (modelHoatDong) {
                    if (auto am = dynamic_cast<ActivityModel*>(modelHoatDong)) {
                        am->setList(modelList);
                    }
                }
            } catch (...) {
                qWarning() << "Lỗi khi cập nhật bangHoatDong realtime";
            }
        }
        
        qDebug() << "✅ Dashboard đã cập nhật realtime thành công!";
        
    } catch (const std::exception& e) {
        qCritical() << "Lỗi khi cập nhật realtime:" << e.what();
    } catch (...) {
        qCritical() << "Lỗi không xác định khi cập nhật realtime";
    }
}

void BangDieuKhien::capNhatBieuDo() {
    // Sẽ implement sau
}

void BangDieuKhien::appendActivity(const QString &text) {
    if (!bangHoatDong) return;
    QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    // Best-effort: push into model if available
    if (modelHoatDong) {
        if (auto am = dynamic_cast<ActivityModel*>(modelHoatDong)) {
            GiaoDichInfoLocal it;
            it.thoiGian = ts;
            it.tenDocGia = text;
            it.tenSach = QString();
            it.trangThai = 0;
            am->prepend(it);
            return;
        }
    }
    // Fallback: nothing to do (view not ready)
}

void BangDieuKhien::xuLyNutQuanLySach() {
    emit yeuCauChuyenDenQuanLySach();
}

void BangDieuKhien::xuLyNutQuanLyDocGia() {
    emit yeuCauChuyenDenQuanLyDocGia();
}

void BangDieuKhien::xuLyNutMuonTraSach() {
    emit yeuCauChuyenDenMuonTraSach();
}

void BangDieuKhien::xuLyNutXemBaoCao() {
    HopThoaiThongKe *dialog = new HopThoaiThongKe(this);
    dialog->exec();
    delete dialog;
}

QWidget* BangDieuKhien::taoTheTongQuan(const QString &tieuDe, const QString &giaTri, 
                                        const QString &bieuTuong, const QColor &mauNen) {
    QWidget *the = new QWidget();
    
    // Tạo gradient từ màu gốc sang màu tối hơn
    QColor mauDam = mauNen.darker(120);
    
    the->setStyleSheet(QString(
        "QWidget {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "        stop:0 %1, stop:1 %2);"
        "    border-radius: 16px;"
        "    border: none;"
        "}"
    ).arg(mauNen.name()).arg(mauDam.name()));
    
    // Thêm shadow effect
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(20);
    shadow->setXOffset(0);
    shadow->setYOffset(4);
    shadow->setColor(QColor(0, 0, 0, 60));
    the->setGraphicsEffect(shadow);
    
    QVBoxLayout *boTri = new QVBoxLayout(the);
    boTri->setContentsMargins(30, 30, 30, 30);
    boTri->setSpacing(12);
    
    // Icon lớn hơn với background tròn
    QWidget *khungIcon = new QWidget();
    khungIcon->setFixedSize(70, 70);
    khungIcon->setStyleSheet(QString(
        "background-color: rgba(255, 255, 255, 0.15);"
        "border-radius: 35px;"
    ));
    
    QLabel *nhanBieuTuong = new QLabel(bieuTuong, khungIcon);
    nhanBieuTuong->setStyleSheet("font-size: 40px; background: transparent;");
    nhanBieuTuong->setAlignment(Qt::AlignCenter);
    nhanBieuTuong->setGeometry(0, 0, 70, 70);
    
    // Số liệu to, bold, nổi bật
    QLabel *nhanGiaTri = new QLabel(giaTri);
    nhanGiaTri->setStyleSheet(
        "font-size: 56px; "
        "font-weight: 900; "
        "color: #FFFFFF; "
        "background: transparent;"
        "letter-spacing: -2px;"
    );
    
    // Label với màu sáng hơn
    QLabel *nhanTieuDe = new QLabel(tieuDe);
    nhanTieuDe->setStyleSheet(
        "font-size: 15px; "
        "color: rgba(255, 255, 255, 0.85); "
        "background: transparent;"
        "font-weight: 500;"
    );
    
    boTri->addWidget(khungIcon);
    boTri->addSpacing(8);
    boTri->addWidget(nhanGiaTri);
    boTri->addWidget(nhanTieuDe);
    boTri->addStretch();
    
    // Thêm hover effect
    the->setAttribute(Qt::WA_Hover);
    the->installEventFilter(new QObject());
    
    return the;
}

void BangDieuKhien::taoThongBaoNhanh() {
    // Sẽ implement sau
}

void BangDieuKhien::taoBieuDoTronThongKe() {
    // Sẽ implement sau
}

QWidget* BangDieuKhien::taoBangHoatDongGanDay() {
    QWidget *khung = new QWidget();
    khung->setStyleSheet(
        "QWidget {"
        "    background-color: #1e2230;"
        "    border: 1px solid #2d3140;"
        "    border-radius: 12px;"
        "}"
    );
    
    QVBoxLayout *boTri = new QVBoxLayout(khung);
    boTri->setContentsMargins(0, 0, 0, 0);
    boTri->setSpacing(0);
    
    // Header
    QWidget *header = new QWidget();
    header->setStyleSheet("background-color: #252830; border: none; border-top-left-radius: 12px; border-top-right-radius: 12px;");
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 18, 24, 18);
    
    QLabel *tieuDe = new QLabel("Hoạt động gần đây");
    tieuDe->setStyleSheet("font-size: 14px; font-weight: 700; color: #E0E0E0; background: transparent; border: none; letter-spacing: 0.3px;");
    
    QPushButton *nutMenu = new QPushButton("⋯");
    nutMenu->setStyleSheet("background: transparent; border: none; color: #858585; font-size: 20px; padding: 0; max-width: 24px;");
    
    headerLayout->addWidget(tieuDe);
    headerLayout->addStretch();
    headerLayout->addWidget(nutMenu);
    
    // Table (model/view)
    QTableView *view = new QTableView();
    this->bangHoatDong = view;
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setShowGrid(false);
    view->setAlternatingRowColors(true);
    view->verticalHeader()->setVisible(false);
    view->horizontalHeader()->setStretchLastSection(false);
    view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    view->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    view->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    view->setStyleSheet(
        "QTableView {"
        "    background-color: #1e2230;"
        "    border: none;"
        "    gridline-color: #2d3140;"
        "    font-size: 13px;"
        "}"
        "QTableView::item {"
        "    padding: 12px 10px;"
        "    color: #E0E0E0;"
        "    border-bottom: 1px solid #252830;"
        "}"
        "QTableView::item:alternate {"
        "    background-color: #252830;"
        "}"
        "QTableView::item:hover {"
        "    background-color: rgba(0, 120, 212, 0.08);"
        "}"
        "QTableView::item:selected {"
        "    background-color: #2d5f8d;"
        "    color: #ffffff;"
        "}"
        "QTableView::item:alternate:selected {"
        "    background-color: #2d5f8d;"
        "    color: #ffffff;"
        "}"
        "QHeaderView::section {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "        stop:0 #2d3140, stop:1 #252830);"
        "    color: #E0E0E0;"
        "    padding: 11px 8px;"
        "    border: none;"
        "    border-right: 1px solid #3E4451;"
        "    border-bottom: 2px solid #3E4451;"
        "    font-weight: 600;"
        "    font-size: 12px;"
        "}"
        "QHeaderView::section:hover {"
        "    background-color: #2d3140;"
        "    color: #0078D4;"
        "}"
        "QScrollBar:vertical {"
        "    background: #1e2230;"
        "    width: 10px;"
        "    margin: 0;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #2d3140;"
        "    border-radius: 5px;"
        "    min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: #3E4451;"
        "}"
    );
    
    // Dữ liệu mẫu từ backend
    extern PTRDG rootDocGia;
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    // KIỂM TRA AN TOÀN trước khi truy cập dữ liệu
    if (rootDocGia && soLuongDauSach >= 0 && soLuongDauSach <= MAX_DAUSACH) {
        PTRDG dsDocGia[100];
        int dem = 0;
        
        try {
            DuyetCayRaMang(rootDocGia, dsDocGia, dem);
        } catch (...) {
            qWarning() << "Lỗi khi duyệt cây độc giả";
            dem = 0;
        }
        
        int soHang = 0;
        
        // Tạo list tất cả giao dịch để sắp xếp theo thời gian
        struct GiaoDichInfo {
            QString thoiGian;
            QString tenDocGia;
            QString tenSach;
            int trangThai;
        };
        std::vector<GiaoDichInfo> danhSachGiaoDich;
        
        for (int i = 0; i < dem && i < 100; i++) {
            if (!dsDocGia[i]) continue;
            if (!dsDocGia[i]->data.dsmt) continue;
            
            // Duyệt qua tất cả giao dịch của độc giả
            auto mt = dsDocGia[i]->data.dsmt;
            while (mt && danhSachGiaoDich.size() < 20) {
                GiaoDichInfo gd;
                gd.thoiGian = QString::fromStdString(mt->data.NgayMuon);
                gd.tenDocGia = QString("%1 %2")
                    .arg(QString::fromStdString(dsDocGia[i]->data.Ho))
                    .arg(QString::fromStdString(dsDocGia[i]->data.Ten));
                gd.trangThai = mt->data.TrangThai;
                
                // Tìm tên sách
                if (mt->data.banSaoSach) {
                    QString maSach = QString::fromStdString(mt->data.banSaoSach->maSach);
                    for (int j = 0; j < soLuongDauSach; j++) {
                        if (dsDauSach[j]) {
                            PTRDMS banSao = dsDauSach[j]->dms;
                            while (banSao) {
                                if (QString::fromStdString(banSao->maSach) == maSach) {
                                    gd.tenSach = QString::fromStdString(dsDauSach[j]->tenSach);
                                    break;
                                }
                                banSao = banSao->next;
                            }
                            if (!gd.tenSach.isEmpty()) break;
                        }
                    }
                }
                
                danhSachGiaoDich.push_back(gd);
                mt = mt->next;
            }
        }
        
        // Build model data and attach to view
        int maxRows = qMin(9, (int)danhSachGiaoDich.size());
        std::vector<GiaoDichInfoLocal> modelList;
        modelList.reserve(maxRows);
        for (int i = 0; i < maxRows; ++i) {
            GiaoDichInfoLocal it;
            it.thoiGian = danhSachGiaoDich[i].thoiGian;
            it.tenDocGia = danhSachGiaoDich[i].tenDocGia;
            it.tenSach = danhSachGiaoDich[i].tenSach;
            it.trangThai = danhSachGiaoDich[i].trangThai;
            modelList.push_back(it);
        }

        ActivityModel *model = new ActivityModel(this);
        model->setList(modelList);
        this->modelHoatDong = model;
        view->setModel(model);
    }
    
    boTri->addWidget(header);
    boTri->addWidget(view);
    
    return khung;
}

QWidget* BangDieuKhien::taoPanelCanhBao() {
    QWidget *khung = new QWidget();
    khung->setStyleSheet(
        "QWidget {"
        "    background-color: #1e2230;"
        "    border: 1px solid #2d3140;"
        "    border-radius: 12px;"
        "}"
    );
    
    QVBoxLayout *boTri = new QVBoxLayout(khung);
    boTri->setContentsMargins(0, 0, 0, 0);
    boTri->setSpacing(0);
    
    // Header
    QWidget *header = new QWidget();
    header->setStyleSheet("background-color: #252830; border: none; border-top-left-radius: 12px; border-top-right-radius: 12px;");
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 18, 24, 18);
    
    QLabel *tieuDe = new QLabel("Cảnh báo & Tác vụ");
    tieuDe->setStyleSheet("font-size: 14px; font-weight: 700; color: #E0E0E0; background: transparent; border: none; letter-spacing: 0.3px;");
    
    QPushButton *nutMenu = new QPushButton("⋯");
    nutMenu->setStyleSheet("background: transparent; border: none; color: #858585; font-size: 20px; padding: 0; max-width: 24px;");
    
    headerLayout->addWidget(tieuDe);
    headerLayout->addStretch();
    headerLayout->addWidget(nutMenu);
    
    // Scroll area cho các items
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: #1e2230; width: 8px; }"
        "QScrollBar::handle:vertical { background: #2d3140; border-radius: 4px; }"
        "QScrollBar::handle:vertical:hover { background: #3E4451; }"
    );
    
    QWidget *noiDung = new QWidget();
    noiDung->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout *noiDungLayout = new QVBoxLayout(noiDung);
    noiDungLayout->setContentsMargins(0, 0, 0, 0);
    noiDungLayout->setSpacing(0);
    
    // Lấy thống kê với kiểm tra an toàn
    extern PTRDG rootDocGia;
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    ThongKeDocGia tkDocGia = {0, 0, 0, 0, 0};
    
    // Kiểm tra dữ liệu hợp lệ trước khi gọi hàm
    if (rootDocGia && soLuongDauSach >= 0 && soLuongDauSach <= MAX_DAUSACH) {
        try {
            tkDocGia = LayThongKeDocGia(rootDocGia, dsDauSach, soLuongDauSach);
        } catch (...) {
            qWarning() << "Lỗi khi lấy thống kê độc giả trong taoPanelCanhBao";
        }
    }
    
    // Tạo các mục cảnh báo
    auto taoMucCanhBao = [this](const QString &icon, const QString &nhan, const QString &mau, const QString &moTa = "", std::function<void()> callback = nullptr) -> QWidget* {
        QWidget *container = new QWidget();
        container->setStyleSheet(
            "QWidget { background: transparent; border: none; }"
        );
        
        QVBoxLayout *containerLayout = new QVBoxLayout(container);
        containerLayout->setContentsMargins(0, 0, 0, 0);
        containerLayout->setSpacing(0);
        
        QPushButton *muc = new QPushButton();
        muc->setFixedHeight(72);
        muc->setStyleSheet(
            "QPushButton { "
            "    background: transparent; "
            "    border: none; "
            "    border-bottom: 1px solid #2d3140; "
            "    text-align: left; "
            "    padding: 0;"
            "}"
            "QPushButton:hover { background-color: rgba(0, 120, 212, 0.08); }"
        );
        muc->setCursor(Qt::PointingHandCursor);
        
        QHBoxLayout *layout = new QHBoxLayout(muc);
        layout->setContentsMargins(20, 16, 20, 16);
        layout->setSpacing(14);
        
        // Icon với màu pastel
        QLabel *iconLabel = new QLabel();
        iconLabel->setFixedSize(40, 40);
        iconLabel->setText(icon);
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        
        // Chuyển hex sang RGBA với opacity 20%
        QColor mauGoc(mau);
        QString mauMo = QString("rgba(%1, %2, %3, 0.20)")
            .arg(mauGoc.red())
            .arg(mauGoc.green())
            .arg(mauGoc.blue());
        
        iconLabel->setStyleSheet(QString(
            "background-color: %1; border-radius: 8px; font-size: 18px; color: %2; border: none;"
        ).arg(mauMo).arg(mau));
        
        // Text
        QVBoxLayout *textLayout = new QVBoxLayout();
        textLayout->setSpacing(4);
        
        QLabel *nhanLabel = new QLabel(nhan);
        nhanLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #E0E0E0; background: transparent; border: none;");
        nhanLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        
        QLabel *moTaLabel = nullptr;
        if (!moTa.isEmpty()) {
            moTaLabel = new QLabel(moTa);
            moTaLabel->setStyleSheet("font-size: 11px; color: #9B9B9B; background: transparent; border: none;");
            moTaLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        }
        
        textLayout->addWidget(nhanLabel);
        if (moTaLabel) textLayout->addWidget(moTaLabel);
        
        // Arrow
        QLabel *arrow = new QLabel("›");
        arrow->setStyleSheet("font-size: 20px; color: #3E4451; background: transparent; border: none;");
        arrow->setAttribute(Qt::WA_TransparentForMouseEvents);
        
        layout->addWidget(iconLabel);
        layout->addLayout(textLayout, 1);
        layout->addWidget(arrow);
        
        containerLayout->addWidget(muc);
        
        // Kết nối callback
        if (callback) {
            QObject::connect(muc, &QPushButton::clicked, callback);
        }
        
        return container;
    };
    
    // Các mục cảnh báo & tác vụ với chức năng click
    noiDungLayout->addWidget(taoMucCanhBao("▲", QString("%1 Sách quá hạn").arg(tkDocGia.docGiaCoSachQuaHan), "#fb923c", "", 
        [this]() { emit yeuCauChuyenDenMuonTraSach(); }));
    
    noiDungLayout->addWidget(taoMucCanhBao("◆", QString("%1 Thẻ sắp hết hạn").arg(3), "#fbbf24", "",
        [this]() { emit yeuCauChuyenDenQuanLyDocGia(); }));
    
    noiDungLayout->addWidget(taoMucCanhBao("●", QString("%1 Thẻ hết hạn").arg(3), "#f87171", "",
        [this]() { emit yeuCauChuyenDenQuanLyDocGia(); }));
    
    noiDungLayout->addWidget(taoMucCanhBao("🔍", "Quét mã vạch", "#60a5fa", "Quét sách/độc giả",
        [this]() { xuLyNutQuetMa(); }));
    
    noiDungLayout->addWidget(taoMucCanhBao("🖨️", "In Ấn Thư Viện", "#a855f7", "In thẻ, tem, báo cáo",
        [this]() { xuLyMoDialogInAn(); }));
    
    noiDungLayout->addWidget(taoMucCanhBao("■", "Thống kê sách", "#34d399", "",
        [this]() { emit yeuCauXemBaoCao(); }));
    
    noiDungLayout->addWidget(taoMucCanhBao("◐", "Quản lý sách", "#a78bfa", "",
        [this]() { emit yeuCauChuyenDenQuanLySach(); }));
    
    noiDungLayout->addStretch();
    
    scrollArea->setWidget(noiDung);
    
    boTri->addWidget(header);
    boTri->addWidget(scrollArea, 1);
    
    return khung;
}

void BangDieuKhien::xuLyNutInThe() {
    extern PTRDG rootDocGia;
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    // Mở dialog in và truyền dữ liệu
    HopThoaiIn *dialogIn = new HopThoaiIn(this);
    dialogIn->setDanhSachDocGia(rootDocGia);
    if (soLuongDauSach > 0) {
        dialogIn->setDauSach(dsDauSach[0]);
    }
    dialogIn->exec();
    delete dialogIn;
}

void BangDieuKhien::xuLyNutQuetMa() {
    // Mở dialog quét mã (tất cả loại)
    HopThoaiQuetMaVach *dialogQuet = new HopThoaiQuetMaVach(HopThoaiQuetMaVach::QUET_TAT_CA, this);
    
    connect(dialogQuet, &HopThoaiQuetMaVach::maVachDuocQuet, this, 
        [](const QString &ma, const QString &loai) {
            qDebug() << "Đã quét mã:" << ma << "Loại:" << loai;
        });
    
    if (dialogQuet->exec() == QDialog::Accepted) {
        QString ma = dialogQuet->layMaQuet();
        QString loai = dialogQuet->layLoaiMaQuet();
        
        // Chuyển đến tab tương ứng
        if (loai == "SACH") {
            emit yeuCauChuyenDenQuanLySach();
        } else if (loai == "DOC_GIA") {
            emit yeuCauChuyenDenQuanLyDocGia();
        }
    }
    
    delete dialogQuet;
}

void BangDieuKhien::xuLyMoDialogInAn() {
    extern PTRDG rootDocGia;
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    HopThoaiInAn *dialogIn = new HopThoaiInAn(this);
    dialogIn->setDocGiaHienTai(rootDocGia);
    dialogIn->setDanhSachDauSach(dsDauSach, soLuongDauSach);
    dialogIn->exec();
    delete dialogIn;
}

// ===================== PRINT PANEL SLOT IMPLEMENTATIONS =====================

void BangDieuKhien::xuLyInTheDocGiaHienTai() {
    extern PTRDG rootDocGia;
    
    // Tạo hộp thoại chọn độc giả
    if (!rootDocGia) {
        QMessageBox::warning(this, "Cảnh Báo", "Không có độc giả nào trong hệ thống!");
        return;
    }
    
    // Sử dụng QInputDialog để nhập mã thẻ
    bool ok;
    QString maThe = QInputDialog::getText(
        this,
        "Nhập Mã Thẻ Độc Giả",
        "Mã thẻ độc giả cần in:",
        QLineEdit::Normal,
        "",
        &ok
    );
    
    if (!ok || maThe.trimmed().isEmpty()) {
        return;
    }
    
    // Tìm độc giả theo mã thẻ bằng hàm timDocGia
    PTRDG docGia = timDocGia(rootDocGia, maThe.trimmed().toInt());
    
    if (docGia) {
        PrintHelper::inTheDocGiaTuDong(docGia, this);
    } else {
        QMessageBox::warning(this, "Không Tìm Thấy", "Không tìm thấy độc giả với mã thẻ: " + maThe);
    }
}

void BangDieuKhien::xuLyInHangLoatThe() {
    extern PTRDG rootDocGia;
    
    if (!rootDocGia) {
        QMessageBox::warning(this, "Cảnh Báo", "Không có độc giả nào trong hệ thống!");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Xác Nhận In Hàng Loạt",
        "In thẻ cho TẤT CẢ độc giả trong hệ thống?\n\nQuá trình này có thể mất vài phút.",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        PrintHelper::inHangLoatTheDocGia(rootDocGia, this);
    }
}

void BangDieuKhien::xuLyInTemSachHienTai() {
    // Yêu cầu nhập ISBN
    bool ok;
    QString isbn = QInputDialog::getText(
        this,
        "Nhập ISBN Sách",
        "ISBN của sách cần in tem:",
        QLineEdit::Normal,
        "",
        &ok
    );
    
    if (!ok || isbn.trimmed().isEmpty()) {
        return;
    }
    
    // Tìm sách theo ISBN
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] && QString::fromStdString(dsDauSach[i]->ISBN) == isbn.trimmed()) {
            PrintHelper::inTemSachTuDong(dsDauSach[i], this);
            return;
        }
    }
    
    QMessageBox::warning(this, "Không Tìm Thấy", "Không tìm thấy sách với ISBN: " + isbn);
}

void BangDieuKhien::xuLyInNhieuTem() {
    // Nhập số lượng tem
    bool ok1;
    int soLuong = QInputDialog::getInt(
        this,
        "Nhập Số Lượng Tem",
        "Số lượng tem cần in:",
        5,      // default value
        1,      // min
        100,    // max
        1,      // step
        &ok1
    );
    
    if (!ok1) {
        return;
    }
    
    // Nhập ISBN
    bool ok;
    QString isbn = QInputDialog::getText(
        this,
        "Nhập ISBN Sách",
        QString("ISBN của sách cần in %1 tem:").arg(soLuong),
        QLineEdit::Normal,
        "",
        &ok
    );
    
    if (!ok || isbn.trimmed().isEmpty()) {
        return;
    }
    
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    
    for (int i = 0; i < soLuongDauSach; i++) {
        if (dsDauSach[i] && QString::fromStdString(dsDauSach[i]->ISBN) == isbn.trimmed()) {
            PrintHelper::inNhieuTemSach(dsDauSach[i], soLuong, this);
            return;
        }
    }
    
    QMessageBox::warning(this, "Không Tìm Thấy", "Không tìm thấy sách với ISBN: " + isbn);
}

void BangDieuKhien::xuLyInDanhSachDocGia() {
    extern PTRDG rootDocGia;
    
    if (!rootDocGia) {
        QMessageBox::warning(this, "Cảnh Báo", "Không có độc giả nào trong hệ thống!");
        return;
    }
    
    PrintHelper::inDanhSachDocGia(rootDocGia, this);
}

void BangDieuKhien::xuLyInDanhMucSach() {
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    
    if (soLuongDauSach == 0) {
        QMessageBox::warning(this, "Cảnh Báo", "Không có sách nào trong hệ thống!");
        return;
    }
    
    PrintHelper::inDanhMucSach(dsDauSach, soLuongDauSach, this);
}

void BangDieuKhien::xuLyInPhieuMuon() {
    extern PTRDG rootDocGia;
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    
    // Nhập mã thẻ độc giả
    bool ok;
    QString maThe = QInputDialog::getText(
        this,
        "Nhập Mã Thẻ Độc Giả",
        "Mã thẻ độc giả có sách đang mượn:",
        QLineEdit::Normal,
        "",
        &ok
    );
    
    if (!ok || maThe.trimmed().isEmpty()) {
        return;
    }
    
    // Tìm độc giả bằng hàm timDocGia
    PTRDG docGia = timDocGia(rootDocGia, maThe.trimmed().toInt());
    
    if (!docGia) {
        QMessageBox::warning(this, "Không Tìm Thấy", "Không tìm thấy độc giả với mã thẻ: " + maThe);
        return;
    }
    
    // Kiểm tra xem độc giả có sách đang mượn không
    if (docGia->data.soSachDangMuon == 0) {
        QMessageBox::information(this, "Thông Báo", "Độc giả này chưa mượn sách nào!");
        return;
    }
    
    // In phiếu mượn cho sách đầu tiên
    MUONTRA mt = docGia->data.dsmt;
    if (mt && mt->data.banSaoSach) {
        QString maSach = QString::fromStdString(mt->data.banSaoSach->maSach);
        QString ngayMuon = QString::fromStdString(mt->data.NgayMuon);
        
        // Tìm tên sách từ mã sách
        QString tenSach = "Không rõ";
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
        
        PrintHelper::inPhieuMuonTuDong(docGia, maSach, tenSach, ngayMuon, this);
    }
}

void BangDieuKhien::xuLyInPhieuTra() {
    QMessageBox::information(this, "Thông Báo", "Chức năng in phiếu trả đang được phát triển!");
}

