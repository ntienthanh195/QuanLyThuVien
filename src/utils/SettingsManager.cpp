/**
 * @file SettingsManager.cpp
 * @path src/utils/SettingsManager.cpp
 * @description Quản lý cài đặt cấu hình ứng dụng (App Settings).
 * Xử lý: lưu trữ/truy xuất tham số hệ thống (số ngày mượn tối đa, số sách tối đa, thông tin thư viện) vào Database.
 * @project Library Management System (QuanLyThuVien)
 */
#include "utils/SettingsManager.h"

#include "utils/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

SettingsManager& SettingsManager::instance() {
    static SettingsManager inst;
    return inst;
}

SettingsManager::SettingsManager() {
    // Tải lần đầu tiên
    reload();
}

void SettingsManager::reload() {
    m_cache.clear();
    QSqlDatabase db = DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    QSqlQuery q(db);
    if (q.exec("SELECT khoa, gia_tri, kieu_du_lieu FROM cai_dat")) {
        while (q.next()) {
            QString key = q.value(0).toString();
            QString valStr = q.value(1).toString();
            QString type = q.value(2).toString();

            QVariant val;
            if (type == "int") val = valStr.toInt();
            else if (type == "double" || type == "float") val = valStr.toDouble();
            else if (type == "bool") val = (valStr == "true" || valStr == "1");
            else val = valStr;

            m_cache.insert(key, val);
        }
    }
    m_loaded = true;
    qDebug() << "[SettingsManager] Reloaded " << m_cache.size() << " settings.";
}

QVariant SettingsManager::get(const QString& key, const QVariant& defaultValue) {
    if (!m_loaded) reload();
    if (m_cache.contains(key)) return m_cache.value(key);
    return defaultValue;
}

int SettingsManager::getInt(const QString& key, int defaultValue) {
    return get(key, defaultValue).toInt();
}

double SettingsManager::getDouble(const QString& key, double defaultValue) {
    return get(key, defaultValue).toDouble();
}

bool SettingsManager::getBool(const QString& key, bool defaultValue) {
    return get(key, defaultValue).toBool();
}

QString SettingsManager::getString(const QString& key, const QString& defaultValue) {
    return get(key, defaultValue).toString();
}

// Include EventBus
#include "utils/EventBus.h"

void SettingsManager::set(const QString& key, const QVariant& value, const QString& type) {
    QSqlDatabase db = DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    // 1. Update Cache
    m_cache.insert(key, value);

    // 2. Update DB
    QSqlQuery q(db);
    // SQLite UPSERT syntax (INSERT OR REPLACE)
    q.prepare("INSERT OR REPLACE INTO cai_dat (khoa, gia_tri, kieu_du_lieu) VALUES (:k, :v, :t)");
    q.bindValue(":k", key);
    q.bindValue(":v", value.toString());
    q.bindValue(":t", type);
    
    if (!q.exec()) {
        qWarning() << "[SettingsManager] Set failed:" << q.lastError().text();
    }
    
    // 3. Notify system
    EventBus::instance().settingsUpdated();
}
