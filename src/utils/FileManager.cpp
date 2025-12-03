#include "utils/FileManager.h"
#include <QFileInfo>
#include <QDateTime>

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
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QFileInfo fileInfo(filePath);
    QString backupPath = fileInfo.path() + "/backup_" + timestamp + "_" + fileInfo.fileName();
    
    return QFile::copy(filePath, backupPath);
}
