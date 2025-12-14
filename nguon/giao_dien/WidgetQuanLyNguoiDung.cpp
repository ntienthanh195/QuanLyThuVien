#include "giao_dien/WidgetQuanLyNguoiDung.h"
#include "models/UserManager.h"
#include "utils/DatabaseManager.h"
#include "utils/EventBus.h" // Added for Realtime Updates
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QInputDialog>
#include <QIcon>
#include <QLabel>

WidgetQuanLyNguoiDung::WidgetQuanLyNguoiDung(QWidget *parent) 
    : QWidget(parent)
    , bangNguoiDung(nullptr)
    , moHinh(nullptr)
    , oTimKiem(nullptr)
    , nutThem(nullptr)
    , nutSua(nullptr)
    , nutXoa(nullptr)
    , nutDatLaiMatKhau(nullptr)
{
    thietLapGiaoDien();
    taiDuLieu();
}

void WidgetQuanLyNguoiDung::thietLapGiaoDien() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // Title
    QLabel *header = new QLabel("QUẢN LÝ NGƯỜI DÙNG & PHÂN QUYỀN");
    header->setStyleSheet("font-size: 24px; font-weight: bold; color: #E0E0E0; margin-bottom: 10px;");
    mainLayout->addWidget(header);

    // Toolbar
    QHBoxLayout *toolbar = new QHBoxLayout();
    
    oTimKiem = new QLineEdit();
    oTimKiem->setPlaceholderText(" Tìm kiếm theo tên hoặc tài khoản...");
    oTimKiem->setStyleSheet("QLineEdit { padding: 8px; border-radius: 4px; border: 1px solid #3E3E42; background: #2D2D30; color: white; }");
    
    auto createBtn = [](const QString &text, const QString &color) {
        QPushButton *btn = new QPushButton(text);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: white; border: none; border-radius: 4px; padding: 8px 16px; font-weight: bold; }"
            "QPushButton:hover { filter: brightness(1.1); }"
        ).arg(color));
        return btn;
    };

    nutThem = createBtn("Thêm Người Dùng", "#0078D4");
    nutSua = createBtn("Sửa Thông Tin", "#2D2D30");
    nutXoa = createBtn("Xóa Người Dùng", "#C42B1C");
    nutDatLaiMatKhau = createBtn("Đặt Lại Mật Khẩu", "#CE9178");

    toolbar->addWidget(oTimKiem, 1);
    toolbar->addWidget(nutThem);
    toolbar->addWidget(nutSua);
    toolbar->addWidget(nutDatLaiMatKhau);
    toolbar->addWidget(nutXoa);
    
    mainLayout->addLayout(toolbar);

    // Table
    bangNguoiDung = new QTableView();
    moHinh = new QStandardItemModel(this);
    moHinh->setColumnCount(5);
    moHinh->setHeaderData(0, Qt::Horizontal, "ID");
    moHinh->setHeaderData(1, Qt::Horizontal, "Tên Đăng Nhập");
    moHinh->setHeaderData(2, Qt::Horizontal, "Họ Tên");
    moHinh->setHeaderData(3, Qt::Horizontal, "Vai Trò");
    moHinh->setHeaderData(4, Qt::Horizontal, "Trạng Thái");

    bangNguoiDung->setModel(moHinh);
    bangNguoiDung->setSelectionBehavior(QAbstractItemView::SelectRows);
    bangNguoiDung->setSelectionMode(QAbstractItemView::SingleSelection);
    bangNguoiDung->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bangNguoiDung->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    bangNguoiDung->verticalHeader()->setVisible(false);
    bangNguoiDung->setAlternatingRowColors(true);
    bangNguoiDung->setStyleSheet(
        "QTableView { background-color: #1e1e2d; color: #E0E0E0; gridline-color: #3E3E42; border: 1px solid #3E3E42; }"
        "QHeaderView::section { background-color: #2D2D30; color: #E0E0E0; padding: 8px; border: none; }"
        "QTableView::item { padding: 5px; }"
        "QTableView::item:selected { background-color: #0078D4; }"
    );
    // Hide ID column
    bangNguoiDung->setColumnHidden(0, true);

    mainLayout->addWidget(bangNguoiDung);

    // Connections
    connect(oTimKiem, &QLineEdit::textChanged, this, &WidgetQuanLyNguoiDung::xuLyTimKiem);
    connect(nutThem, &QPushButton::clicked, this, &WidgetQuanLyNguoiDung::xuLyThem);
    connect(nutSua, &QPushButton::clicked, this, &WidgetQuanLyNguoiDung::xuLySua);
    connect(nutXoa, &QPushButton::clicked, this, &WidgetQuanLyNguoiDung::xuLyXoa);
    connect(nutDatLaiMatKhau, &QPushButton::clicked, this, &WidgetQuanLyNguoiDung::xuLyDatLaiMatKhau);
}

void WidgetQuanLyNguoiDung::taiDuLieu() {
    moHinh->removeRows(0, moHinh->rowCount());
    QString filter = oTimKiem->text();

    // Use default connection
    if (!QSqlDatabase::database().isOpen()) {
        qDebug() << "ERROR: Database is NOT open!";
        return;
    }
    QSqlQuery q; 
    QString sql = "SELECT u.id, u.ten_dang_nhap, u.ho_ten, r.ten_vai_tro, u.trang_thai "
                  "FROM nguoi_dung u "
                  "LEFT JOIN vai_tro r ON u.vai_tro_id = r.id ";
    
    if (!filter.isEmpty()) {
        sql += "WHERE u.ten_dang_nhap LIKE :f OR u.ho_ten LIKE :f ";
    }
    
    q.prepare(sql);
    if (!filter.isEmpty()) {
        q.bindValue(":f", "%" + filter + "%");
    }

    if (q.exec()) {
        while (q.next()) {
            QList<QStandardItem*> row;
            row << new QStandardItem(q.value(0).toString()); // ID
            row << new QStandardItem(q.value(1).toString()); // Username
            row << new QStandardItem(q.value(2).toString()); // Full Name
            row << new QStandardItem(q.value(3).toString()); // Role
            
            QString status = q.value(4).toInt() == 1 ? "Hoạt động" : "Đã khóa";
            row << new QStandardItem(status);
            
            moHinh->appendRow(row);
        }
    }
}

void WidgetQuanLyNguoiDung::xuLyTimKiem(const QString &text) {
    Q_UNUSED(text);
    taiDuLieu();
}

void WidgetQuanLyNguoiDung::xuLyThem() {
    bool ok;
    QString username = QInputDialog::getText(this, "Thêm Người Dùng", "Tên đăng nhập:", QLineEdit::Normal, "", &ok);
    if (!ok || username.isEmpty()) return;

    QString fullName = QInputDialog::getText(this, "Thêm Người Dùng", "Họ và Tên:", QLineEdit::Normal, "", &ok);
    if (!ok || fullName.isEmpty()) return;
    
    QStringList roles; roles << "Admin" << "ThuThu";
    QString roleName = QInputDialog::getItem(this, "Chọn Vai Trò", "Vai trò:", roles, 1, false, &ok);
    if (!ok) return;
    
    QString password = QInputDialog::getText(this, "Mật Khẩu", "Mật khẩu:", QLineEdit::Password, "", &ok);
    if (!ok || password.isEmpty()) return;
    
    int roleId = (roleName == "Admin") ? 1 : 2;
    
    // Explicitly check DB connection
    if (!DatabaseManager::instance().database().isOpen()) {
        QMessageBox::critical(this, "Lỗi Hệ Thống", "Kết nối cơ sở dữ liệu bị mất!");
        return;
    }

    if (UserManager::instance().addUser(username, password, fullName, roleId)) {
        QMessageBox::information(this, "Thành công", "Đã thêm người dùng mới!");
        emit EventBus::instance().nguoiDungAdded(username);
        taiDuLieu();
    } else {
        // Show actual SQL error if possible, or generic message
        QMessageBox::warning(this, "Lỗi", "Không thể thêm người dùng.\nNguyên nhân: Tên đăng nhập đã tồn tại hoặc lỗi CSDL.");
    }
}

void WidgetQuanLyNguoiDung::xuLySua() {
    QModelIndex idx = bangNguoiDung->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng chọn người dùng cần sửa!");
        return;
    }
    
    QString id = moHinh->item(idx.row(), 0)->text();
    QString currentName = moHinh->item(idx.row(), 2)->text();
    QString currentRole = moHinh->item(idx.row(), 3)->text();

    bool ok;
    QString newName = QInputDialog::getText(this, "Sửa Thông Tin", "Họ tên mới:", QLineEdit::Normal, currentName, &ok);
    if (!ok || newName.isEmpty()) return;

    QStringList roles; roles << "Admin" << "ThuThu";
    // Pre-select current role
    int currentIdx = (currentRole == "Admin") ? 0 : 1;
    QString newRoleName = QInputDialog::getItem(this, "Sửa Vai Trò", "Vai trò:", roles, currentIdx, false, &ok);
    if (!ok) return;

    int newRoleId = (newRoleName == "Admin") ? 1 : 2;

    QSqlQuery q;
    q.prepare("UPDATE nguoi_dung SET ho_ten = :n, vai_tro_id = :r WHERE id = :id");
    q.bindValue(":n", newName);
    q.bindValue(":r", newRoleId);
    q.bindValue(":id", id);
    
    if (q.exec()) {
        QMessageBox::information(this, "Thành công", "Đã cập nhật thông tin!");
        emit EventBus::instance().nguoiDungUpdated(moHinh->item(idx.row(), 1)->text());
        taiDuLieu();
    } else {
        QMessageBox::critical(this, "Lỗi", "Cập nhật thất bại: " + q.lastError().text());
    }
}

void WidgetQuanLyNguoiDung::xuLyXoa() {
    QModelIndex idx = bangNguoiDung->currentIndex();
    if (!idx.isValid()) return;
    
    QString id = moHinh->item(idx.row(), 0)->text();
    QString username = moHinh->item(idx.row(), 1)->text();
    
    if (username == "admin") {
        QMessageBox::warning(this, "Cảnh báo", "Không thể xóa tài khoản Admin mặc định!");
        return;
    }

    if (QMessageBox::question(this, "Xác nhận", "Bạn có chắc muốn xóa tài khoản " + username + "?") == QMessageBox::Yes) {
        QSqlQuery q;
        q.prepare("DELETE FROM nguoi_dung WHERE id = :id");
        q.bindValue(":id", id);
        if (q.exec()) {
            emit EventBus::instance().nguoiDungRemoved(username);
            taiDuLieu();
        } else {
             QMessageBox::critical(this, "Lỗi", "Xóa thất bại: " + q.lastError().text());
        }
    }
}

void WidgetQuanLyNguoiDung::xuLyDatLaiMatKhau() {
    QModelIndex idx = bangNguoiDung->currentIndex();
    if (!idx.isValid()) return;
    
    QString id = moHinh->item(idx.row(), 0)->text();
    bool ok;
    QString newPass = QInputDialog::getText(this, "Đặt Lại Mật Khẩu", "Nhập mật khẩu mới:", QLineEdit::Password, "", &ok);
    if (ok && !newPass.isEmpty()) {
        if (UserManager::instance().changePassword(id.toInt(), newPass)) {
            QMessageBox::information(this, "Thành công", "Đã đổi mật khẩu!");
        }
    }
}

// void taiLaiDanhSach() placeholder
void WidgetQuanLyNguoiDung::taiLaiDanhSach() {
    taiDuLieu();
}
