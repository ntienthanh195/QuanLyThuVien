/**
 * @file LanguageManager.cpp
 * @path src/utils/LanguageManager.cpp
 * @description Quản lý đa ngôn ngữ (Localization).
 * Xử lý: tải file ngôn ngữ (JSON), chuyển đổi ngôn ngữ thời gian thực.
 * @project Library Management System (QuanLyThuVien)
 */
#include "utils/LanguageManager.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QCoreApplication>
#include <QDir>

LanguageManager& LanguageManager::instance()
{
    static LanguageManager _instance;
    return _instance;
}

LanguageManager::LanguageManager(QObject *parent) : QObject(parent), m_currentLang(Language::Vietnamese)
{
    loadDictionary();
}

void LanguageManager::setLanguage(Language lang)
{
    if (m_currentLang != lang) {
        m_currentLang = lang;
        emit languageChanged(m_currentLang);
    }
}

Language LanguageManager::currentLanguage() const
{
    return m_currentLang;
}

void LanguageManager::cycleLanguage()
{
    switch (m_currentLang) {
    case Language::Vietnamese:
        setLanguage(Language::Khmer);
        break;
    case Language::Khmer:
        setLanguage(Language::English);
        break;
    case Language::English:
        setLanguage(Language::Vietnamese);
        break;
    }
}

QString LanguageManager::getText(const QString& key)
{
    if (key.isEmpty()) {
        return QString();
    }
    
    if (m_dictionary.contains(key)) {
        if (m_dictionary[key].contains(m_currentLang)) {
            QString text = m_dictionary[key][m_currentLang];
            if (!text.isEmpty()) {
                return text;
            }
        }
        // Fallback to Vietnamese
        if (m_dictionary[key].contains(Language::Vietnamese)) {
            return m_dictionary[key][Language::Vietnamese];
        }
    }
    
    qWarning() << "LanguageManager: Key not found:" << key;
    return key; 
}

void LanguageManager::loadDictionary()
{
    QString baseDir = QCoreApplication::applicationDirPath();
    
    // Attempt standard locations: assets/languages/ relative to executable
    QString viPath = baseDir + "/assets/languages/languages_vi.json";
    QString kmPath = baseDir + "/assets/languages/languages_km.json";
    QString enPath = baseDir + "/assets/languages/languages_en.json";

    // Fallback if not found (for dev environment)
    if (!QFile::exists(viPath)) {
        viPath = QDir::currentPath() + "/assets/languages/languages_vi.json";
        kmPath = QDir::currentPath() + "/assets/languages/languages_km.json";
        enPath = QDir::currentPath() + "/assets/languages/languages_en.json";
    }

    qDebug() << "[LanguageManager] Loading JSON languages...";
    loadFromJson(Language::Vietnamese, viPath);
    loadFromJson(Language::Khmer, kmPath);
    loadFromJson(Language::English, enPath);
}

bool LanguageManager::loadFromJson(Language lang, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[LanguageManager] Could not open file:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (doc.isNull()) {
        qWarning() << "[LanguageManager] Parse error in" << filePath << ":" << error.errorString();
        return false;
    }

    if (!doc.isObject()) {
        qWarning() << "[LanguageManager] JSON is not an object:" << filePath;
        return false;
    }

    QJsonObject obj = doc.object();
    int count = 0;
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        m_dictionary[it.key()][lang] = it.value().toString();
        count++;
    }

    qDebug() << "[LanguageManager] Loaded" << count << "keys for language" << (int)lang << "from" << filePath;
    return true;
}
