#include "utils/SettingsManager.h"
#include <QDebug>

SettingsManager& SettingsManager::instance() {
    static SettingsManager inst;
    return inst;
}

SettingsManager::SettingsManager() {
    // Default mock values
    m_cache["SO_NGAY_MUON_TOI_DA"] = 7;
    m_cache["TIEN_PHAT_MOI_NGAY"] = 5000;
}

void SettingsManager::reload() {
    // No-op for mock
}

QVariant SettingsManager::get(const QString& key, const QVariant& defaultValue) {
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

void SettingsManager::set(const QString& key, const QVariant& value, const QString& type) {
    Q_UNUSED(type);
    m_cache.insert(key, value);
}
