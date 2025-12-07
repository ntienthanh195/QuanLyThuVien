#ifndef FILEPATHMANAGER_H
#define FILEPATHMANAGER_H

#include <QString>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>
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
    // Chỉ gán rootPath và filesDir; không tự động tạo hay copy file.
    static void initialize() {
        if (!isInitialized) {
            QString exePath = QCoreApplication::applicationDirPath();
            rootPath = exePath;

            // Thay vì luôn dùng exeDir/data, dò lên dần các thư mục cha để tìm file "data/files/library.db"
            // Mục tiêu: luôn tham chiếu tới 1 file canonical trong repository nếu có.
            QDir probeDir(exePath);
            QString foundData;
            for (int i = 0; i < 12; ++i) { // dò lên tối đa 12 mức để phòng các trường hợp build trong sâu
                QString candidateDb = probeDir.filePath("data/files/library.db");
                if (QFile::exists(candidateDb)) {
                    foundData = probeDir.filePath("data");
                    break;
                }
                if (!probeDir.cdUp()) break;
            }

            if (!foundData.isEmpty()) {
                filesDir = foundData;
                qDebug() << "[FilePathManager] Đã tìm thấy data dir tại:" << filesDir;
            } else {
                // Fallback: dùng exeDir/data như trước nhưng log rõ cảnh báo
                filesDir = QDir(rootPath).filePath("data"); // không tự tạo
                qWarning() << "[FilePathManager] KHÔNG tìm thấy 'data/files/library.db' khi dò từ exe; đang dùng" << filesDir;
                qWarning() << "-> Nếu bạn muốn ép buộc dùng file cụ thể, đặt file library.db vào 'data/files' gần exe hoặc chạy ứng dụng từ thư mục gốc dự án.";
            }

            // Thử gom backup cũ nếu có — lỗi sẽ bị bỏ qua
            try {
                int moved = ConsolidateLegacyBackups(rootPath.toStdString());
                if (moved > 0) qDebug() << "[FilePathManager] Đã hợp nhất" << moved << "backup cũ vào data/backup";
            } catch(...) {
                qWarning() << "[FilePathManager] Lỗi khi hợp nhất backup (bỏ qua)";
            }

            isInitialized = true;
        }
    }

    // Lấy đường dẫn đầy đủ của file trong data/
    static QString getFilePath(const QString& filename) {
        if (!isInitialized) initialize();
        return QDir(filesDir).filePath(filename);
    }

    // Trả về đường dẫn tới DB chính `data/files/library.db`.
    // Nếu file không tồn tại, trả về chuỗi rỗng và log cảnh báo — caller phải xử lý.
    static QString getLibraryDbPath() {
        if (!isInitialized) initialize();
        QDir d(filesDir);
        QString dbPath = d.filePath("files/library.db");
        if (!QFile::exists(dbPath)) {
            qWarning() << "[FilePathManager] KHÔNG tìm thấy file DB tại:" << dbPath;
            return QString();
        }
        return dbPath;
    }

    static std::string getLibraryDbPathStd() { return getLibraryDbPath().toStdString(); }

    // Các helper đường dẫn chuẩn
    static QString getFilesDirectory() { if (!isInitialized) initialize(); return filesDir; }
    static QString getRootPath() { if (!isInitialized) initialize(); return rootPath; }
    static QString getDauSachPath() { return getFilePath("dausach.txt"); }
    static QString getDanhMucSachPath() { return getFilePath("danhmucsach.txt"); }
    static QString getDocGiaPath() { return getFilePath("docgia.txt"); }
    static QString getMuonTraPath() { return getFilePath("muontra.txt"); }

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
