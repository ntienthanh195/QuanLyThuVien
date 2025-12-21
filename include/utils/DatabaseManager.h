/**
 * @file DatabaseManager.h
 * @path include/utils/DatabaseManager.h
 * @description Header quản lý Database (Singleton).
 * @project Library Management System (QuanLyThuVien)
 */
#ifndef DATABASEMANAGER_H

#define DATABASEMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <memory>

/**
 * Simple SQLite DatabaseManager singleton wrapper.
 * Provides connect/close and createTables migration helper.
 */
class DatabaseManager {
public:
    static DatabaseManager& instance();

    // Connect to sqlite file at path. Returns true if opened.
    bool connect(const QString& path);
    void close();

    // Begin/commit/rollback
    bool beginTransaction();
    bool commit();
    bool rollback();

    // Execute SQL (non-prepared) - convenience
    bool execute(const QString& sql);

    // Create schema tables if not exists
    bool createTables();

    // Automatic normalization for book copy codes
    void normalizeBookCodes();

    // Archiving & Cleanup
    bool archiveOldRecords(int monthsThreshold = 6);

    // Last error
    QSqlError lastError() const;

    QSqlDatabase database() const { return m_db; }

    // Enable/disable automatic backups before write operations
    void setAutoBackup(bool enable) { m_autoBackup = enable; }
    bool autoBackup() const { return m_autoBackup; }

private:
    DatabaseManager() = default;
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase m_db;
    QString m_path;
    bool m_autoBackup = false; // Disabled by default, use periodic backup instead
};

#endif // DATABASEMANAGER_H
