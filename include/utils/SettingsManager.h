/**
 * @file SettingsManager.h
 * @path include/utils/SettingsManager.h
 * @description Header quản lý Cài đặt hệ thống (Singleton).
 * @project Library Management System (QuanLyThuVien)
 */
#ifndef SETTINGSMANAGER_H

#define SETTINGSMANAGER_H

#include <QString>
#include <QVariant>
#include <QMap>

class SettingsManager {
public:
    static SettingsManager& instance();

    // Lấy giá trị cài đặt, nếu chưa có trả về defaultValue
    QVariant get(const QString& key, const QVariant& defaultValue = QVariant());
    
    // Lấy giá trị dưới dạng chuyên biệt
    int getInt(const QString& key, int defaultValue = 0);
    double getDouble(const QString& key, double defaultValue = 0.0);
    bool getBool(const QString& key, bool defaultValue = false);
    QString getString(const QString& key, const QString& defaultValue = "");

    // Lưu giá trị cài đặt
    void set(const QString& key, const QVariant& value, const QString& type = "string");

    // Tải lại toàn bộ cài đặt từ DB vào cache (để tăng tốc)
    void reload();

private:
    SettingsManager();
    QMap<QString, QVariant> m_cache;
    bool m_loaded = false;
};

#endif // SETTINGSMANAGER_H
