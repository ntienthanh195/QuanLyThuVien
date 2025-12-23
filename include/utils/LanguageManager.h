/**
 * @file LanguageManager.h
 * @status ACTIVE
 * @brief Quản lý đa ngôn ngữ (Singleton).
 * @note AI Chú ý: Đây là header chính thức. Bỏ qua file tại src/utils/LanguageManager.h
 */
/**
 * @file LanguageManager.h
 * @path include/utils/LanguageManager.h
 * @description Header quản lý Đa ngôn ngữ (Singleton). 
 * Cung cấp hàm getText(key) để lấy chuỗi ký tự theo ngôn ngữ hiện tại.
 * @project Library Management System (QuanLyThuVien)
 */
#ifndef LANGUAGEMANAGER_H

#define LANGUAGEMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>

enum class Language {
    Vietnamese,
    Khmer,
    English
};

class LanguageManager : public QObject
{
    Q_OBJECT
public:
    static LanguageManager& instance();

    void setLanguage(Language lang);
    Language currentLanguage() const;

    QString getText(const QString& key);
    
    // Helper to get next language (for single button toggle)
    void cycleLanguage();

signals:
    void languageChanged(Language newLang);

private:
    explicit LanguageManager(QObject *parent = nullptr);
    QMap<QString, QMap<Language, QString>> m_dictionary;
    Language m_currentLang;

    void loadDictionary();
    bool loadFromJson(Language lang, const QString& filePath);
};

#endif // LANGUAGEMANAGER_H
