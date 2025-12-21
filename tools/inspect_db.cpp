#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QSqlQuery>
#include "utils/DatabaseManager.h"
#include "utils/FilePathManager.h"

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

    // Prefer data/ and fall back to files/ — use FilePathManager for consistent logic
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
        qCritical() << "Neither data/ nor files/ directory found; looked at:" << filesDir;
        return 2;
    }

    QString dbPath = QDir(filesDir).filePath("library.db");
    if (!QFile::exists(dbPath)) {
        qCritical() << "Database file not found:" << dbPath;
        return 3;
    }

    DatabaseManager &db = DatabaseManager::instance();
    if (!db.connect(dbPath)) {
        qCritical() << "Cannot open database at" << dbPath;
        return 4;
    }

    QSqlDatabase sql = db.database();

    auto countTable = [&](const QString &t){
        QSqlQuery q(sql);
        q.prepare(QString("SELECT COUNT(*) FROM %1").arg(t));
        if (!q.exec()) return -1;
        if (q.next()) return q.value(0).toInt();
        return -1;
    };

    qInfo() << "DB path:" << dbPath;
    qInfo() << "Counts:";
    qInfo() << " dau_sach:" << countTable("dau_sach");
    qInfo() << " ban_sao:" << countTable("ban_sao");
    qInfo() << " doc_gia:" << countTable("doc_gia");
    qInfo() << " muon_tra:" << countTable("muon_tra");

    // Check danhmucsach lines for missing dau_sach
    QString danhmucFile = QDir(filesDir).filePath("danhmucsach.txt");
    qInfo() << "Checking" << danhmucFile << "for unmatched ISBNs...";
    QStringList lines = readLines(danhmucFile);
    int idx = 0;
    for (const QString &ln : lines) {
        idx++;
        QStringList parts = ln.split('|');
        if (parts.isEmpty()) continue;
        QString maSach = parts[0].trimmed();
        QString isbn = maSach.split('-').first();
        QSqlQuery q(sql);
        q.prepare("SELECT id FROM dau_sach WHERE isbn = ?");
        q.addBindValue(isbn);
        if (!(q.exec() && q.next())) {
            qWarning() << "Line" << idx << ": ma_sach" << maSach << "-> missing dau_sach for isbn" << isbn;
        }
    }

    db.close();
    return 0;
}
