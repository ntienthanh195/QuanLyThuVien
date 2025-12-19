/**
 * @file FilePathManager.h
 * @path include/utils/FilePathManager.h
 * @description Header quản lý đường dẫn (Singleton).
 * @project Library Management System (QuanLyThuVien)
 */
#ifndef FILEPATHMANAGER_H

#define FILEPATHMANAGER_H

#include <QString>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>
#include <iostream>
#include "utils/BackupManager.h"

/**
 * FilePathManager
 * - Quản lý các đường dẫn file chuẩn của ứng dụng (data/...)
 * - Thiết kế đơn giản: ứng dụng mong đợi `data/` nằm cùng cấp thư mục chứa exe.
 * - Không cố gắng dò nhiều đường dẫn hoặc tự động copy file legacy.
 */
class FilePathManager {
public:
    // Khởi tạo đường dẫn gốc (gọi 1 lần khi khởi động app)
    static void initialize() {
        if (!isInitialized) {
            QString exePath = QCoreApplication::applicationDirPath();
            
            // Priority 0: APP_SOURCE_DIR (From IDE build)
            #ifdef APP_SOURCE_DIR
            QString srcPath = QString(APP_SOURCE_DIR);
            if (QFile::exists(srcPath + "/data/files/library.db")) {
                rootPath = srcPath;
                filesDir = srcPath + "/data";
                qDebug() << "[Path] Root found via APP_SOURCE_DIR:" << rootPath;
                isInitialized = true;
                return;
            }
            #endif

            // Priority 1: "data" directory next to exe (Portable mode)
            QDir exeDir(exePath);
            QString portableDb = exeDir.absoluteFilePath("data/files/library.db");
            
            if (QFile::exists(portableDb)) {
                rootPath = exeDir.absolutePath();
                filesDir = exeDir.absolutePath() + "/data";
                qDebug() << "[Path] Root found (Portable):" << filesDir;
            } else {
                // Priority 2: Probe upwards (Development mode/Shadow Build)
                QDir probeDir(exePath);
                bool found = false;
                for (int i = 0; i < 6; ++i) { 
                    QString candDb = probeDir.absoluteFilePath("data/files/library.db");
                    if (QFile::exists(candDb)) {
                        rootPath = probeDir.absolutePath();
                        filesDir = probeDir.absolutePath() + "/data";
                        found = true;
                        qDebug() << "[Path] Root found at level" << i << ":" << filesDir;
                        break;
                    }
                    if (!probeDir.cdUp()) break;
                }
                
                if (!found) {
                    rootPath = exePath;
                    filesDir = exePath + "/data";
                    qWarning() << "[Path] Root NOT found, using default:" << filesDir;
                }
            }

            // Ensure critical directories exist
            QDir d(filesDir);
            d.mkpath("files");
            d.mkpath("logs/receipts");
            d.mkpath("backup");

            isInitialized = true;
        }
    }

    // Lấy đường dẫn đầy đủ của file/thư mục trong data/
    static QString getFilePath(const QString& relPath) {
        if (!isInitialized) initialize();
        return QDir(filesDir).absoluteFilePath(relPath);
    }

    // Trả về đường dẫn TUYỆT ĐỐI tới DB chính
    static QString getLibraryDbPath() {
        return getFilePath("files/library.db");
    }

    static std::string getLibraryDbPathStd() { return getLibraryDbPath().toStdString(); }

    // Các helper đường dẫn chuẩn (Toàn bộ trả về Absolute Path)
    static QString getFilesDirectory() { if (!isInitialized) initialize(); return filesDir; }
    static QString getRootPath() { if (!isInitialized) initialize(); return rootPath; }
    
    static QString getDauSachPath() { return getFilePath("dausach.txt"); }
    static QString getDanhMucSachPath() { return getFilePath("danhmucsach.txt"); }
    static QString getDocGiaPath() { return getFilePath("docgia.txt"); }
    static QString getMuonTraPath() { return getFilePath("muontra.txt"); }
    static QString getPrintLogPath() { return getFilePath("logs/receipts"); }

    static std::string getDauSachPathStd() { return getDauSachPath().toStdString(); }
    static std::string getDanhMucSachPathStd() { return getDanhMucSachPath().toStdString(); }
    static std::string getDocGiaPathStd() { return getDocGiaPath().toStdString(); }
    static std::string getMuonTraPathStd() { return getMuonTraPath().toStdString(); }

private:
    static QString rootPath;
    static QString filesDir;
    static bool isInitialized;
};

#endif // FILEPATHMANAGER_H
