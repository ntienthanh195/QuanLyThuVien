/**
 * @file DatabaseManager.cpp
 * @path src/utils/DatabaseManager.cpp
 * @description Quản lý kết nối và thao tác với SQLite database.
 * Xử lý: khởi tạo bảng, thực thi câu lệnh SQL, và là cầu nối lưu trữ cho Audit Log, Settings, Reports.
 * @project Library Management System (QuanLyThuVien)
 */
#include "utils/DatabaseManager.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash> // Added for seeding
#include "utils/BackupManager.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include "utils/XuLyChuoi.h"

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

#include <QDebug>
#include <QSqlError>

bool DatabaseManager::connect(const QString& path) {
    // Nếu đã mở DB từ trước, đóng trước khi mở kết nối mới
    if (m_db.isOpen()) m_db.close();

    // Ensure path is not empty
    if (path.isEmpty()) {
        qWarning() << "[DatabaseManager] connect() called with empty path!";
        return false;
    }
    
    qDebug() << "[DB] Opening:" << path;
    
    // Ensure directory exists so SQLite can create the file
    QFileInfo info(path);
    QDir dir = info.absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "[DatabaseManager] Cannot create directory:" << dir.absolutePath();
            return false;
        }
    }

    m_path = path;
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(m_path);
    bool ok = m_db.open();
    if (!ok) {
        qCritical() << "[DB] FATAL ERROR:" << m_db.lastError().text();
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
        "so_luot_muon INTEGER DEFAULT 0,"
        "gia_tien INTEGER DEFAULT 0,"
        "nha_xuat_ban TEXT"
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
        "so_sach_dang_muon INTEGER DEFAULT 0,"
        "lop TEXT,"
        "ngay_sinh TEXT,"
        "loai_doc_gia INTEGER DEFAULT 0"
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
        "so_lan_gia_han INTEGER DEFAULT 0,"
        "da_thanh_toan INTEGER DEFAULT 0,"
        "FOREIGN KEY(ma_the) REFERENCES doc_gia(ma_the) ON DELETE CASCADE,"
        "FOREIGN KEY(ban_sao_id) REFERENCES ban_sao(id)"
        ");"
    );

    // muon_tra_archive (Mirror for historical data)
    ok &= q.exec(
        "CREATE TABLE IF NOT EXISTS muon_tra_archive ("
        "id INTEGER PRIMARY KEY,"
        "ma_the INTEGER,"
        "ban_sao_id INTEGER,"
        "ngay_muon TEXT,"
        "ngay_tra TEXT,"
        "trang_thai INTEGER,"
        "tien_phat INTEGER,"
        "so_lan_gia_han INTEGER,"
        "da_thanh_toan INTEGER"
        ");"
    );

    // Optimized indices for history and reporting
    q.exec("CREATE INDEX IF NOT EXISTS idx_muon_tra_card ON muon_tra(ma_the)");
    q.exec("CREATE INDEX IF NOT EXISTS idx_muon_tra_date ON muon_tra(ngay_muon)");
    q.exec("CREATE INDEX IF NOT EXISTS idx_muon_tra_status ON muon_tra(trang_thai)");
    q.exec("CREATE INDEX IF NOT EXISTS idx_doc_gia_name ON doc_gia(ho, ten)");

    // MIGRATION FOR FEATURE 4 (Existing DBs)
    // Add columns if they don't exist
    // so_lan_gia_han
    {
        QSqlQuery checkCol(m_db);
        if (checkCol.exec("SELECT so_lan_gia_han FROM muon_tra LIMIT 0")) {
            // Column exists
        } else {
            qDebug() << "[DatabaseManager] Adding missing column 'so_lan_gia_han' to 'muon_tra'...";
            QSqlQuery addCol(m_db);
            addCol.exec("ALTER TABLE muon_tra ADD COLUMN so_lan_gia_han INTEGER DEFAULT 0");
        }
    }
    // da_thanh_toan
    {
        QSqlQuery checkCol(m_db);
        if (checkCol.exec("SELECT da_thanh_toan FROM muon_tra LIMIT 0")) {
            // Column exists
        } else {
            qDebug() << "[DatabaseManager] Adding missing column 'da_thanh_toan' to 'muon_tra'...";
            QSqlQuery addCol(m_db);
            addCol.exec("ALTER TABLE muon_tra ADD COLUMN da_thanh_toan INTEGER DEFAULT 0");
        }
    }

    // PHASE 2 MIGRATION: DOC_GIA & DAU_SACH
    // Table: doc_gia - New Headers: lop, ngay_sinh, loai_doc_gia (sdt, email already exist in create but maybe missing in old DB)
    {
        const char* newCols[][3] = {
            {"so_dien_thoai", "TEXT", ""},
            {"email", "TEXT", ""},
            {"lop", "TEXT", ""},
            {"ngay_sinh", "TEXT", ""},
            {"loai_doc_gia", "INTEGER", "DEFAULT 0"}
        };
        for (auto& col : newCols) {
            QSqlQuery check(m_db);
            if (!check.exec(QString("SELECT %1 FROM doc_gia LIMIT 0").arg(col[0]))) {
                 qDebug() << "[DatabaseManager] Adding missing column '" << col[0] << "' to 'doc_gia'...";
                 QSqlQuery add(m_db);
                 add.exec(QString("ALTER TABLE doc_gia ADD COLUMN %1 %2 %3").arg(col[0]).arg(col[1]).arg(col[2]));
            }
        }
    }
    // Table: dau_sach - New Headers: nha_xuat_ban
    {
         QSqlQuery check(m_db);
         if (!check.exec("SELECT nha_xuat_ban FROM dau_sach LIMIT 0")) {
             qDebug() << "[DatabaseManager] Adding missing column 'nha_xuat_ban' to 'dau_sach'...";
             QSqlQuery add(m_db);
             add.exec("ALTER TABLE dau_sach ADD COLUMN nha_xuat_ban TEXT");
         }
    }

    // ==========================================
    // PHASE 3: ADMIN & SECURITY TABLES
    // ==========================================
    
    qDebug() << "[DatabaseManager] Checking/Creating 'vai_tro' table...";
    bool roleOk = q.exec(
        "CREATE TABLE IF NOT EXISTS vai_tro ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "ten_vai_tro TEXT UNIQUE NOT NULL,"
        "mo_ta TEXT,"
        "quyen_han TEXT" // JSON or delimited string
        ");"
    );
    if (!roleOk) qDebug() << "Error creating vai_tro:" << q.lastError().text();
    ok &= roleOk;

    qDebug() << "[DatabaseManager] Checking/Creating 'nguoi_dung' table...";
    bool userOk = q.exec(
        "CREATE TABLE IF NOT EXISTS nguoi_dung ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "ten_dang_nhap TEXT UNIQUE NOT NULL,"
        "mat_khau_hash TEXT NOT NULL,"
        "muoi TEXT NOT NULL,"
        "ho_ten TEXT,"
        "vai_tro_id INTEGER,"
        "trang_thai INTEGER DEFAULT 1,"
        "FOREIGN KEY(vai_tro_id) REFERENCES vai_tro(id)"
        ");"
    );
    if (!userOk) qDebug() << "Error creating nguoi_dung:" << q.lastError().text();
    ok &= userOk;

    // ==========================================
    // PHASE 2: AUDIT & SETTINGS (NEW)
    // ==========================================

    // nhat_ky_he_thong (Audit Log)
    qDebug() << "[DatabaseManager] Creating 'nhat_ky_he_thong' table...";
    ok &= q.exec(
        "CREATE TABLE IF NOT EXISTS nhat_ky_he_thong ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "nguoi_dung_id INTEGER,"
        "hanh_dong TEXT,"       // CREATE, UPDATE, DELETE, LOGIN, RESTORE
        "bang_du_lieu TEXT,"    // SACH, DOC_GIA, ...
        "doi_tuong_id TEXT,"    // ID record effected
        "chi_tiet TEXT,"        // Description/JSON diff
        "thoi_gian TEXT,"       // YYYY-MM-DD HH:MM:SS
        "FOREIGN KEY(nguoi_dung_id) REFERENCES nguoi_dung(id)"
        ");"
    );

    // cai_dat (Settings)
    qDebug() << "[DatabaseManager] Creating 'cai_dat' table...";
    ok &= q.exec(
        "CREATE TABLE IF NOT EXISTS cai_dat ("
        "khoa TEXT PRIMARY KEY," // Key (e.g. 'loan_limit')
        "gia_tri TEXT,"          // Value
        "mo_ta TEXT,"            // Description
        "kieu_du_lieu TEXT DEFAULT 'string'" // int, bool, color...
        ");"
    );

    // SEED DATA (Default Admin)
    // Check if roles exist
    QSqlQuery checkRole("SELECT count(*) FROM vai_tro", m_db);
    if (checkRole.next()) {
        int count = checkRole.value(0).toInt();
        qDebug() << "[DatabaseManager] Role count:" << count;
        if (count == 0) {
            // Create default roles
            qDebug() << "[DatabaseManager] Seeding default roles...";
            q.prepare("INSERT INTO vai_tro (ten_vai_tro, mo_ta, quyen_han) VALUES (?, ?, ?)");
            
            // Admin
            q.addBindValue("Admin");
            q.addBindValue("Quản trị viên toàn quyền");
            q.addBindValue("*"); // All permissions
            if(!q.exec()) qDebug() << "Failed to insert Admin role:" << q.lastError().text();
            
            // Librarian (Thủ thư)
            q.prepare("INSERT INTO vai_tro (ten_vai_tro, mo_ta, quyen_han) VALUES (?, ?, ?)");
            q.addBindValue("ThuThu");
            q.addBindValue("Thủ thư - Quản lý sách và mượn trả");
            q.addBindValue("DOC_GIA,SACH,MUON_TRA,BAO_CAO");
            if(!q.exec()) qDebug() << "Failed to insert ThuThu role:" << q.lastError().text();
        }
    } else {
        qDebug() << "[DatabaseManager] Failed to count roles:" << checkRole.lastError().text();
    }
    
    // Check if users exist
    QSqlQuery checkUser("SELECT count(*) FROM nguoi_dung", m_db);
    if (checkUser.next() && checkUser.value(0).toInt() == 0) {
        // Create default admin user (admin / 123456)
        // Note: Password hashing should normally be done via helper, doing manual here for bootstrap
        // SHA256("123456" + "somesalt")
        // Demo implementation:
        QString salt = "somesalt";
        QString password = "admin";
        QString hash = QString(QCryptographicHash::hash((password + salt).toUtf8(), QCryptographicHash::Sha256).toHex());
        
        q.prepare("INSERT INTO nguoi_dung (ten_dang_nhap, mat_khau_hash, muoi, ho_ten, vai_tro_id, trang_thai) "
                  "VALUES (:u, :p, :s, :n, (SELECT id FROM vai_tro WHERE ten_vai_tro='Admin'), 1)");
        q.bindValue(":u", "admin");
        q.bindValue(":p", hash);
        q.bindValue(":s", salt);
        q.bindValue(":n", "Administrator");
        q.exec();
        
        qDebug() << "[DatabaseManager] Seeded default admin user.";
    }


    // Seed Default Settings if empty or missing critical ones
    {
        qDebug() << "[DatabaseManager] Ensuring default settings exist...";
        struct SettingSeed { const char* key; const char* val; const char* desc; const char* type; };
        SettingSeed seeds[] = {
            {"SO_SACH_MUON_TOI_DA", "3", "Số sách tối đa một độc giả được mượn", "int"},
            {"SO_NGAY_MUON_TOI_DA", "14", "Thời hạn mượn sách (ngày)", "int"},
            {"TIEN_PHAT_MOI_NGAY", "5000", "Tiền phạt quá hạn mỗi ngày (VNĐ)", "int"},
            {"CHO_PHEP_MUON_KHI_QUA_HAN", "false", "Cho phép mượn tiếp khi đang có sách quá hạn", "bool"},
            {"TIEU_DE_THU_VIEN", "THƯ VIỆN TỔNG HỢP", "Tên hiển thị trên tiêu đề ứng dụng", "string"},
            {"MAU_CHU_DAO", "#3b82f6", "Màu chủ đạo của giao diện", "color"},
            {"PHI_MAT_SACH", "20000", "Phí phạt khi làm mất sách", "int"},
            {"SO_LAN_GIA_HAN_TOI_DA", "1", "Số lần tối đa được gia hạn một cuốn sách", "int"}
        };
        
        q.prepare("INSERT OR IGNORE INTO cai_dat (khoa, gia_tri, mo_ta, kieu_du_lieu) VALUES (?, ?, ?, ?)");
        for (const auto& s : seeds) {
            q.addBindValue(s.key);
            q.addBindValue(s.val);
            q.addBindValue(s.desc);
            q.addBindValue(s.type);
            q.exec();
        }
    }

    if (!m_db.commit()) {
        qWarning() << "[DatabaseManager] Commit failed:" << m_db.lastError().text();
        m_db.rollback();
        return false;
    }

    // Run automatic normalization for book codes
    normalizeBookCodes();

    return ok;
}

void DatabaseManager::normalizeBookCodes() {
    if (!m_db.isOpen()) return;

    qDebug() << "[DatabaseManager] Checking for book code normalization...";
    
    QSqlQuery query(m_db);
    // Select all copy IDs from ban_sao
    if (!query.exec("SELECT id, ma_sach FROM ban_sao")) {
        qWarning() << "[DatabaseManager] Failed to select book codes for normalization:" << query.lastError().text();
        return;
    }

    int count = 0;
    QSqlQuery updateQuery(m_db);
    
    while (query.next()) {
        int id = query.value(0).toInt();
        std::string maSach = query.value(1).toString().toStdString();
        std::string normalized = ChuanHoaMaBanSao(maSach);
        
        if (normalized != maSach) {
            updateQuery.prepare("UPDATE ban_sao SET ma_sach = :ma WHERE id = :id");
            updateQuery.bindValue(":ma", QString::fromStdString(normalized));
            updateQuery.bindValue(":id", id);
            
            if (updateQuery.exec()) {
                count++;
                qDebug() << "[DatabaseManager] Normalized book code:" << QString::fromStdString(maSach) << "->" << QString::fromStdString(normalized);
            } else {
                qWarning() << "[DatabaseManager] Failed to update book code:" << query.lastError().text();
            }
        }
    }

    if (count > 0) {
        qDebug() << "[DatabaseManager] Automatically normalized" << count << "book code(s).";
    }
}

bool DatabaseManager::archiveOldRecords(int monthsThreshold) {
    if (!m_db.isOpen()) return false;

    qDebug() << "[DatabaseManager] Starting archiving process (threshold:" << monthsThreshold << "months)...";

    if (!beginTransaction()) return false;

    // 1. Move old finished records to archive
    // Converting dd/MM/yyyy to YYYY-MM-DD for comparison
    QString dateIso = "SUBSTR(ngay_muon, 7, 4) || '-' || SUBSTR(ngay_muon, 4, 2) || '-' || SUBSTR(ngay_muon, 1, 2)";
    QString thresholdDate = QDate::currentDate().addMonths(-monthsThreshold).toString("yyyy-MM-dd");

    QSqlQuery q(m_db);
    q.prepare(QString("INSERT INTO muon_tra_archive "
                      "SELECT * FROM muon_tra "
                      "WHERE (%1 < :date) "
                      "AND (trang_thai = 1 OR trang_thai = 2) " // Only Returned (1) or Lost (2)
                      "AND (ngay_tra IS NOT NULL AND ngay_tra != '')").arg(dateIso));
    q.bindValue(":date", thresholdDate);

    if (!q.exec()) {
        qWarning() << "[DatabaseManager] Archiving failed (Phase 1):" << q.lastError().text();
        rollback();
        return false;
    }

    int movedCount = q.numRowsAffected();
    if (movedCount > 0) {
        // 2. Delete moved records from primary table
        QSqlQuery qDel(m_db);
        qDel.prepare(QString("DELETE FROM muon_tra "
                             "WHERE id IN (SELECT id FROM muon_tra_archive) "
                             "AND id IN (SELECT id FROM muon_tra WHERE (%1 < :date))").arg(dateIso));
        qDel.bindValue(":date", thresholdDate);

        if (!qDel.exec()) {
             qWarning() << "[DatabaseManager] Archiving failed (Phase 2):" << qDel.lastError().text();
             rollback();
             return false;
        }
    }

    if (!commit()) return false;

    // 3. Optional: Shrink DB file
    if (movedCount > 0) {
        qDebug() << "[DatabaseManager] Archived" << movedCount << "records. Running VACUUM...";
        m_db.close(); // Vacuum requires no transactions or sometimes even closed connections depending on driver, 
                      // but mostly it just needs no active statements.
        {
            QSqlDatabase db2 = QSqlDatabase::addDatabase("QSQLITE", "vacuum_conn");
            db2.setDatabaseName(m_path);
            if (db2.open()) {
                QSqlQuery qVac(db2);
                qVac.exec("VACUUM");
                db2.close();
            }
        }
        QSqlDatabase::removeDatabase("vacuum_conn");
        connect(m_path); // Reconnect main
    } else {
        qDebug() << "[DatabaseManager] No records met the archiving criteria.";
    }

    return true;
}

DatabaseManager::~DatabaseManager() {
    if (m_db.isOpen()) m_db.close();
}
