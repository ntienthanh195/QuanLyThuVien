#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QString>
#include <QFile>
#include <QTextStream>

class FileManager {
public:
    static bool fileExists(const QString &filePath);
    static QString readTextFile(const QString &filePath);
    static bool writeTextFile(const QString &filePath, const QString &content);
    static bool appendTextFile(const QString &filePath, const QString &content);
    static bool createBackup(const QString &filePath);
    
private:
    FileManager() = default;
};

#endif // FILEMANAGER_H
