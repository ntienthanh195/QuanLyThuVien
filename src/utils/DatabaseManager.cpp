#include "utils/DatabaseManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "utils/BackupManager.h"
#include <QFile>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

bool DatabaseManager::connect(const QString& path) {
    // Nếu đã mở DB từ trước, đóng trước khi mở kết nối mới
    if (m_db.isOpen()) m_db.close();

    // Bảo vệ: yêu cầu path không rỗng và file phải tồn tại. Không tự tạo DB mới ở đây.
    // Nếu DB thiếu, caller (hoặc code khởi tạo) phải quyết định migrate hoặc báo lỗi.
    if (path.isEmpty()) {
        qWarning() << "[DatabaseManager] connect() gọi với path rỗng — hủy kết nối.";
        return false;
    }
    if (!QFile::exists(path)) {
        qWarning() << "[DatabaseManager] File DB không tồn tại:" << path << "— connect bị hủy.";
        return false;
    }

    m_path = path;
    // Use connection name unique per process
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(m_path);
    bool ok = m_db.open();
    if (!ok) {
        qWarning() << "[DatabaseManager] Failed to open DB:" << m_db.lastError().text();
        return ok;
    }

    // Database tuning for better concurrent read performance and faster writes in bulk operations
    // Enable WAL mode, set synchronous to NORMAL, set busy timeout and adjust cache size.
    QSqlQuery q(m_db);
    q.exec("PRAGMA journal_mode = WAL;");
    q.exec("PRAGMA synchronous = NORMAL;");
    q.exec("PRAGMA busy_timeout = 5000;");
    // negative cache_size means size in KB; -2000 => ~2MB cache
    q.exec("PRAGMA cache_size = -2000;");

    return true;
}

void DatabaseManager::close() {
    if (m_db.isOpen()) m_db.close();
    // remove default connection
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
}

bool DatabaseManager::beginTransaction() {
    if (!m_db.isOpen()) return false;
    return m_db.transaction();
}

bool DatabaseManager::commit() {
    if (!m_db.isOpen()) return false;
    return m_db.commit();
}

bool DatabaseManager::rollback() {
    if (!m_db.isOpen()) return false;
    return m_db.rollback();
}

bool DatabaseManager::execute(const QString& sql) {
    if (!m_db.isOpen()) return false;
    // If this looks like a write operation, create a backup first (if enabled)
    auto isWriteSql = [](const QString &s)->bool {
        QString t = s.trimmed().toUpper();
        return t.startsWith("INSERT") || t.startsWith("UPDATE") || t.startsWith("DELETE") ||
               t.startsWith("CREATE") || t.startsWith("ALTER") || t.startsWith("DROP") ||
               t.startsWith("REPLACE") || t.startsWith("VACUUM");
    };

    if (m_autoBackup && isWriteSql(sql)) {
        if (QFile::exists(m_path)) {
            std::string res = AutoBackup();
            if (!res.empty()) {
                qWarning() << "[DatabaseManager] AutoBackup warning/error:" << QString::fromStdString(res);
            } else {
                qDebug() << "[DatabaseManager] AutoBackup created before write.";
            }
        }
    }

    QSqlQuery q(m_db);
    bool ok = q.exec(sql);
    if (!ok) qDebug() << "[DatabaseManager] SQL error:" << q.lastError().text() << "SQL:" << sql;
    return ok;
}

QSqlError DatabaseManager::lastError() const {
    return m_db.lastError();
}

bool DatabaseManager::createTables() {
    if (!m_db.isOpen()) return false;
    // If DB file exists and auto-backup enabled, create backup before schema changes
    if (m_autoBackup && !m_path.isEmpty()) {
        if (QFile::exists(m_path)) {
            std::string res = AutoBackup();
            if (!res.empty()) {
                qWarning() << "[DatabaseManager] AutoBackup warning/error before createTables:" << QString::fromStdString(res);
            } else {
                qDebug() << "[DatabaseManager] AutoBackup created before createTables.";
            }
        }
    }

    // Wrap in transaction
    QSqlQuery q(m_db);
    bool ok = m_db.transaction();
    if (!ok) {
        qWarning() << "[DatabaseManager] Cannot start transaction:" << m_db.lastError().text();
    }

    // dau_sach
    ok &= q.exec(
        "CREATE TABLE IF NOT EXISTS dau_sach ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "isbn TEXT UNIQUE NOT NULL,"
        "ten_sach TEXT NOT NULL,"
        "so_trang INTEGER,"
        "tac_gia TEXT,"
        "nam_xuat_ban INTEGER,"
        "the_loai TEXT,"
        "so_luot_muon INTEGER DEFAULT 0"
        ");"
    );

    // ban_sao
    ok &= q.exec(
        "CREATE TABLE IF NOT EXISTS ban_sao ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "ma_sach TEXT UNIQUE NOT NULL,"
        "dau_sach_id INTEGER NOT NULL,"
        "trang_thai INTEGER DEFAULT 0,"
        "vi_tri TEXT,"
        "FOREIGN KEY(dau_sach_id) REFERENCES dau_sach(id) ON DELETE CASCADE"
        ");"
    );

    // doc_gia
    ok &= q.exec(
        "CREATE TABLE IF NOT EXISTS doc_gia ("
        "ma_the INTEGER PRIMARY KEY,"
        "ho TEXT,"
        "ten TEXT,"
        "phai INTEGER,"
        "trang_thai INTEGER DEFAULT 1,"
        "so_dien_thoai TEXT,"
        "email TEXT,"
        "ngay_dang_ky TEXT,"
        "ngay_het_han TEXT,"
        "so_sach_dang_muon INTEGER DEFAULT 0"
        ");"
    );

    // muon_tra
    ok &= q.exec(
        "CREATE TABLE IF NOT EXISTS muon_tra ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "ma_the INTEGER NOT NULL,"
        "ban_sao_id INTEGER NOT NULL,"
        "ngay_muon TEXT NOT NULL,"
        "ngay_tra TEXT,"
        "trang_thai INTEGER DEFAULT 0,"
        "tien_phat INTEGER DEFAULT 0,"
        "FOREIGN KEY(ma_the) REFERENCES doc_gia(ma_the) ON DELETE CASCADE,"
        "FOREIGN KEY(ban_sao_id) REFERENCES ban_sao(id)"
        ");"
    );

    if (!m_db.commit()) {
        qWarning() << "[DatabaseManager] Commit failed:" << m_db.lastError().text();
        m_db.rollback();
        return false;
    }

    return ok;
}

DatabaseManager::~DatabaseManager() {
    if (m_db.isOpen()) m_db.close();
}
