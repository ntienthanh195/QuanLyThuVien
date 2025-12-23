#include "utils/FileManager.h"
#include <QFileInfo>
#include <QDateTime>
#include "utils/FilePathManager.h"
#include <QDir>

bool FileManager::fileExists(const QString &filePath) {
    return QFileInfo::exists(filePath);
}

QString FileManager::readTextFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    file.close();
    
    return content;
}

bool FileManager::writeTextFile(const QString &filePath, const QString &content) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << content;
    file.close();
    
    return true;
}

bool FileManager::appendTextFile(const QString &filePath, const QString &content) {
    QFile file(filePath);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << content;
    file.close();
    
    return true;
}

bool FileManager::createBackup(const QString &filePath) {
    if (!fileExists(filePath)) {
        return false;
    }
    // Ensure centralized backup directory exists under data/backup
    QString filesDir = FilePathManager::getFilesDirectory();
    QString backupsDir = QDir(filesDir).filePath("backup");
    QDir dir;
    if (!dir.exists(backupsDir)) {
        dir.mkpath(backupsDir);
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QFileInfo fileInfo(filePath);
    QString backupFileName = QString("backup_%1_%2").arg(timestamp, fileInfo.fileName());
    QString backupPath = QDir(backupsDir).filePath(backupFileName);

    return QFile::copy(filePath, backupPath);
}
