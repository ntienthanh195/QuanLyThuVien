#ifndef FILEPATHMANAGER_H
#define FILEPATHMANAGER_H

#include <QString>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>

/**
 * @brief Quản lý đường dẫn file tập trung
 * Đảm bảo tất cả file đọc/ghi đều nằm trong thư mục files/ ở gốc dự án
 */
class FilePathManager {
public:
    // Khởi tạo đường dẫn gốc (gọi 1 lần khi khởi động app)
    static void initialize() {
        if (!isInitialized) {
            QString exePath = QCoreApplication::applicationDirPath();
            qDebug() << "[FilePathManager] Exe path:" << exePath;
            
            // Thử các đường dẫn có thể
            QStringList possiblePaths = {
                exePath,                                      // Chạy trực tiếp từ thư mục chứa exe
                QDir(exePath + "/..").absolutePath(),         // debug/ hoặc release/
                QDir(exePath + "/../..").absolutePath(),      // build/Desktop_Qt_*/
                QDir(exePath + "/../../..").absolutePath(),   // build/Desktop_Qt_*/debug hoặc release
                QDir(exePath + "/../../../..").absolutePath(), // Thêm 1 cấp nữa
                "E:/Qt/QuanLyThuVien"                         // Đường dẫn tuyệt đối backup
            };
            
            // Tìm thư mục chứa files/dausach.txt
            bool found = false;
            qDebug() << "[FilePathManager] Đang tìm files/dausach.txt...";
            for (const QString& path : possiblePaths) {
                QString testPath = path + "/files/dausach.txt";
                qDebug() << "  Kiểm tra:" << testPath << "->" << QFile::exists(testPath);
                if (QFile::exists(testPath)) {
                    rootPath = path;
                    filesDir = path + "/files";
                    found = true;
                    qDebug() << "[FilePathManager] ✓ Tìm thấy tại:" << path;
                    break;
                }
            }
            
            if (!found) {
                qWarning() << "[FilePathManager] ⚠️ KHÔNG tìm thấy files/dausach.txt";
                qWarning() << "[FilePathManager] Fallback to:" << exePath;
                // Fallback to exe path
                rootPath = exePath;
                filesDir = exePath + "/files";
                QDir().mkpath(filesDir);
            }
            
            isInitialized = true;
        }
    }
    
    // Lấy đường dẫn đầy đủ của file
    static QString getFilePath(const QString& filename) {
        if (!isInitialized) initialize();
        return filesDir + "/" + filename;
    }
    
    // Lấy đường dẫn thư mục files/
    static QString getFilesDirectory() {
        if (!isInitialized) initialize();
        return filesDir;
    }
    
    // Lấy đường dẫn gốc dự án
    static QString getRootPath() {
        if (!isInitialized) initialize();
        return rootPath;
    }
    
    // Các file chuẩn
    static QString getDauSachPath() { return getFilePath("dausach.txt"); }
    static QString getDanhMucSachPath() { return getFilePath("danhmucsach.txt"); }
    static QString getDocGiaPath() { return getFilePath("docgia.txt"); }
    static QString getMuonTraPath() { return getFilePath("muontra.txt"); }
    
    // Chuyển đổi sang std::string cho C++ cũ
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
