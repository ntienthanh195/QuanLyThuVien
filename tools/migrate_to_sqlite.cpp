#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>

#include "utils/DatabaseManager.h"
#include "utils/FilePathManager.h"

static bool ensureBackup(const QString &filesDir, QString &outBackupDir) {
    QDir dir(filesDir);
    if (!dir.exists()) return false;
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    // create under centralized backup folder: data/backup/backup_migration_<ts>
    QString backupsRoot = dir.filePath("backup");
    QDir backupRoot;
    if (!backupRoot.exists(backupsRoot)) {
        if (!backupRoot.mkpath(backupsRoot)) return false;
    }
    outBackupDir = QDir(backupsRoot).filePath(QString("backup_migration_%1").arg(timestamp));
    QDir backup;
    if (!backup.mkpath(outBackupDir)) return false;

    QStringList entries = dir.entryList(QDir::Files);
    for (const QString &f : entries) {
        QString src = dir.filePath(f);
        QString dst = QDir(outBackupDir).filePath(f);
        if (!QFile::copy(src, dst)) {
            qWarning() << "[migrate] Failed to copy" << src << "->" << dst;
        }
    }
    qInfo() << "[migrate] Backup created:" << outBackupDir;
    return true;
}

static QStringList readLines(const QString &path) {
    QStringList result;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return result;
    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        result << line;
    }
    return result;
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    // Use FilePathManager to resolve data/ vs files/
    QString filesDir;
    try {
        FilePathManager::initialize();
        filesDir = FilePathManager::getFilesDirectory();
    } catch (...) {
        QString cwd = QDir::currentPath();
        QString dataDir = QDir(cwd).filePath("data");
        QString oldDir = QDir(cwd).filePath("files");
        if (QDir(dataDir).exists()) filesDir = dataDir;
        else filesDir = oldDir;
    }
    if (!QDir(filesDir).exists()) {
        qCritical() << "Neither data/ nor files/ directory found (after FilePathManager)";
        return 2;
    }

    QString dbPath = QDir(filesDir).filePath("files/library.db");

    // Backup files before migration
    QString backupDir;
    if (!ensureBackup(filesDir, backupDir)) {
        qWarning() << "[migrate] Warning: could not create backup directory.";
    }

    // Connect database
    DatabaseManager &db = DatabaseManager::instance();
    if (!db.connect(dbPath)) {
        qCritical() << "[migrate] Cannot open database at" << dbPath;
        return 3;
    }

    qInfo() << "[migrate] Creating tables...";
    if (!db.createTables()) {
        qCritical() << "[migrate] Failed to create tables:" << db.lastError().text();
        return 4;
    }

    // Migrate dausach.txt
    QString dausachFile = QDir(filesDir).filePath("dausach.txt");
    QString danhmucFile = QDir(filesDir).filePath("danhmucsach.txt");
    QString docgiaFile = QDir(filesDir).filePath("docgia.txt");
    QString muontraFile = QDir(filesDir).filePath("muontra.txt");

    qInfo() << "[migrate] Reading" << dausachFile;
    QStringList dausachLines = readLines(dausachFile);
    QSqlDatabase sql = db.database();
    QSqlQuery q(sql);

    sql.transaction();

    // Insert dau_sach
    q.prepare("INSERT OR IGNORE INTO dau_sach(isbn, ten_sach, so_trang, tac_gia, nam_xuat_ban, the_loai) VALUES (?, ?, ?, ?, ?, ?)");
    for (const QString &ln : dausachLines) {
        QStringList parts = ln.split('|');
        if (parts.size() < 6) continue;
        QString isbn = parts[0].trimmed();
        QString ten = parts[1].trimmed();
        int soTrang = parts[2].trimmed().toInt();
        QString tacGia = parts[3].trimmed();
        int nam = parts[4].trimmed().toInt();
        QString theLoai = parts[5].trimmed();
        q.addBindValue(isbn);
        q.addBindValue(ten);
        q.addBindValue(soTrang);
        q.addBindValue(tacGia);
        q.addBindValue(nam);
        q.addBindValue(theLoai);
        if (!q.exec()) {
            qWarning() << "[migrate] Failed insert dau_sach:" << q.lastError().text() << ln;
        }
    }

    // Insert ban_sao from danhmucsach.txt
    q.prepare("INSERT OR IGNORE INTO ban_sao(ma_sach, dau_sach_id, trang_thai, vi_tri) VALUES (?, ?, ?, ?)");
    QStringList dmsLines = readLines(danhmucFile);
    for (const QString &ln : dmsLines) {
        QStringList parts = ln.split('|');
        if (parts.size() < 3) continue;
        QString maSach = parts[0].trimmed();
        int trangThai = parts[1].trimmed().toInt();
        QString viTri = parts[2].trimmed();
        // Extract ISBN from maSach (format: ISBN-N)
        QString isbn = maSach.split('-').first();
        // find dau_sach id
        QSqlQuery q2(sql);
        q2.prepare("SELECT id FROM dau_sach WHERE isbn = ?");
        q2.addBindValue(isbn);
        if (q2.exec() && q2.next()) {
            int dauId = q2.value(0).toInt();
            q.addBindValue(maSach);
            q.addBindValue(dauId);
            q.addBindValue(trangThai);
            q.addBindValue(viTri);
            if (!q.exec()) {
                qWarning() << "[migrate] Failed insert ban_sao:" << q.lastError().text() << ln;
            }
        } else {
            qWarning() << "[migrate] Could not find dau_sach for maSach" << maSach << "(isbn" << isbn << ")";
        }
    }

    // Insert doc_gia
    q.prepare("INSERT OR REPLACE INTO doc_gia(ma_the, ho, ten, phai, trang_thai) VALUES (?, ?, ?, ?, ?)");
    QStringList docgiaLines = readLines(docgiaFile);
    for (const QString &ln : docgiaLines) {
        QStringList parts = ln.split('|');
        if (parts.size() < 5) continue;
        int maThe = parts[0].trimmed().toInt();
        QString ho = parts[1].trimmed();
        QString ten = parts[2].trimmed();
        int phai = parts[3].trimmed().toInt();
        int trangThai = parts[4].trimmed().toInt();
        q.addBindValue(maThe);
        q.addBindValue(ho);
        q.addBindValue(ten);
        q.addBindValue(phai);
        q.addBindValue(trangThai);
        if (!q.exec()) {
            qWarning() << "[migrate] Failed insert doc_gia:" << q.lastError().text() << ln;
        }
    }

    // Insert muon_tra
    q.prepare("INSERT INTO muon_tra(ma_the, ban_sao_id, ngay_muon, ngay_tra, trang_thai) VALUES (?, ?, ?, ?, ?)");
    QStringList muonLines = readLines(muontraFile);
    for (const QString &ln : muonLines) {
        QStringList parts = ln.split('|');
        if (parts.size() < 5) continue;
        int maThe = parts[0].trimmed().toInt();
        QString maSach = parts[1].trimmed();
        QString ngayMuon = parts[2].trimmed();
        QString ngayTra = parts[3].trimmed();
        int trangThai = parts[4].trimmed().toInt();
        // Find ban_sao id
        QSqlQuery q3(sql);
        q3.prepare("SELECT id FROM ban_sao WHERE ma_sach = ?");
        q3.addBindValue(maSach);
        if (q3.exec() && q3.next()) {
            int banSaoId = q3.value(0).toInt();
            q.addBindValue(maThe);
            q.addBindValue(banSaoId);
            q.addBindValue(ngayMuon);
            q.addBindValue(ngayTra.isEmpty() ? QVariant(QVariant::String) : ngayTra);
            q.addBindValue(trangThai);
            if (!q.exec()) {
                qWarning() << "[migrate] Failed insert muon_tra:" << q.lastError().text() << ln;
            }
        } else {
            qWarning() << "[migrate] Could not find ban_sao for" << maSach;
        }
    }

    if (!sql.commit()) {
        qWarning() << "[migrate] Commit failed:" << sql.lastError().text();
    } else {
        qInfo() << "[migrate] Migration finished successfully.";
    }

    db.close();
    return 0;
}
