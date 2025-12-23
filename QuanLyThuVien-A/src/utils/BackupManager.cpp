#include "utils/BackupManager.h"
#include "utils/FilePathManager.h"
#include "models/ThaoTacFile.h"
#include "utils/NgayThang.h"
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <map>
#include <filesystem>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#include <dirent.h>
#endif

// Lay thu muc backup mac dinh
std::string LayThuMucBackupMacDinh() {
    return FilePathManager::getFilePath("backup").toStdString();
}

// Tao ten file backup theo format
std::string TaoTenFileBackup(const std::string& loaiFile) {
    time_t now = time(nullptr);
    tm* ltm = localtime(&now);
    
    char buffer[120];
    // If backing up the SQLite DB, use .db extension; otherwise use .txt
    const char* ext = (loaiFile == "library") ? ".db" : ".txt";
    sprintf(buffer, "backup_%04d%02d%02d_%02d%02d%02d_%s%s",
            1900 + ltm->tm_year,
            1 + ltm->tm_mon,
            ltm->tm_mday,
            ltm->tm_hour,
            ltm->tm_min,
            ltm->tm_sec,
            loaiFile.c_str(), ext);

    return std::string(buffer);
}

// Kiem tra va tao thu muc backup
bool KiemTraThuMucBackup(const std::string& thuMuc) {
    std::string duongDan = thuMuc.empty() ? LayThuMucBackupMacDinh() : thuMuc;
    
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(duongDan.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        // Thu muc khong ton tai, tao moi
        if (mkdir(duongDan.c_str(), 0777) != 0) {
            return false;
        }
    }
#else
    struct stat st;
    if (stat(duongDan.c_str(), &st) != 0) {
        if (mkdir(duongDan.c_str(), 0777) != 0) {
            return false;
        }
    }
#endif
    
    return true;
}

// Sao chep file
static bool SaoChepFile(const std::string& nguon, const std::string& dich) {
    std::ifstream src(nguon, std::ios::binary);
    if (!src.is_open()) return false;
    
    std::ofstream dst(dich, std::ios::binary);
    if (!dst.is_open()) {
        src.close();
        return false;
    }
    
    dst << src.rdbuf();
    
    src.close();
    dst.close();
    
    return true;
}

// Tao backup thu cong
std::string TaoBackup(const std::string& thuMucBackup) {
    std::string thuMuc = thuMucBackup.empty() ? LayThuMucBackupMacDinh() : thuMucBackup;
    
    // Kiem tra va tao thu muc backup
    if (!KiemTraThuMucBackup(thuMuc)) {
        return "Loi: Khong the tao thu muc backup!";
    }
    
    // Backup only the SQLite DB (single source of truth)
    std::string dbFile = FilePathManager::getLibraryDbPathStd();
    if (dbFile.empty()) dbFile = FilePathManager::getFilePath("files/library.db").toStdString();
    
    std::string loai = "library";

    std::string tenBackup = TaoTenFileBackup(loai);
    std::string duongDanBackup = thuMuc + "/" + tenBackup;

    if (SaoChepFile(dbFile, duongDanBackup)) {
        return ""; // success
    } else {
        return std::string("Loi: Khong the backup file '") + dbFile + "'";
    }
}

// Tao backup tu dong
std::string AutoBackup() {
    return TaoBackup("");
}

// Phuc hoi du lieu tu backup
std::string PhucHoiBackup(const std::string& duongDanBackup) {
    if (duongDanBackup.empty()) {
        return "Loi: Duong dan backup khong hop le!";
    }
    
    // Xac dinh loai file backup (support .txt and .db backups)
    std::string loaiFile;
    if (duongDanBackup.find("dausach") != std::string::npos) {
        loaiFile = "dausach";
    } else if (duongDanBackup.find("danhmucsach") != std::string::npos) {
        loaiFile = "danhmucsach";
    } else if (duongDanBackup.find("docgia") != std::string::npos) {
        loaiFile = "docgia";
    } else if (duongDanBackup.find("muontra") != std::string::npos) {
        loaiFile = "muontra";
    } else if (duongDanBackup.find("library") != std::string::npos || duongDanBackup.find(".db") != std::string::npos) {
        loaiFile = "library";
    } else {
        return "Loi: Khong xac dinh duoc loai file backup!";
    }

    // Xac dinh file dich
    std::string fileDich;
    if (loaiFile == "library") {
        fileDich = FilePathManager::getFilePath("files/library.db").toStdString();
    } else {
        fileDich = FilePathManager::getFilePath(QString::fromStdString(loaiFile + ".txt")).toStdString();
    }

    // Sao chep file backup ve file goc
    if (SaoChepFile(duongDanBackup, fileDich)) {
        return ""; // Thanh cong
    } else {
        return "Loi: Khong the phuc hoi file tu backup!";
    }
}

// Lay kich thuoc file
static long long LayKichThuocFile(const std::string& duongDan) {
    std::ifstream file(duongDan, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return 0;
    
    long long size = file.tellg();
    file.close();
    return size;
}

// Lay danh sach backup
int LayDanhSachBackup(ThongTinBackup ketQua[], int maxKetQua, const std::string& thuMucBackup) {
    std::string thuMuc = thuMucBackup.empty() ? LayThuMucBackupMacDinh() : thuMucBackup;
    
    if (!KiemTraThuMucBackup(thuMuc)) {
        return 0;
    }
    
    int count = 0;
    
#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    // Find all files starting with backup_ (we'll filter extensions later)
    std::string searchPath = thuMuc + "\\backup_*";
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (count >= maxKetQua) break;

            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::string name = findData.cFileName;
                // only accept backup_*.txt or backup_*.db
                if (name.rfind("backup_", 0) != 0) continue;
                if (name.find(".txt") == std::string::npos && name.find(".db") == std::string::npos) continue;

                ketQua[count].tenFile = findData.cFileName;
                ketQua[count].duongDanDayDu = thuMuc + "/" + findData.cFileName;

                // Lay ngay gio tu ten file (backup_YYYYMMDD_HHMMSS_loai.txt|.db)
                std::string ten = findData.cFileName;
                size_t pos1 = ten.find("_");
                size_t pos2 = ten.find("_", pos1 + 1);
                if (pos1 != std::string::npos && pos2 != std::string::npos) {
                    ketQua[count].ngayTao = ten.substr(pos1 + 1, pos2 - pos1 - 1) + "_" + 
                                           ten.substr(pos2 + 1, 6);
                }

                ketQua[count].kichThuoc = LayKichThuocFile(ketQua[count].duongDanDayDu);
                count++;
            }
        } while (FindNextFileA(hFind, &findData) != 0);

        FindClose(hFind);
    }
#else
    DIR* dir = opendir(thuMuc.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr && count < maxKetQua) {
            std::string tenFile = entry->d_name;
            if (tenFile.find("backup_") == 0 && (tenFile.find(".txt") != std::string::npos || tenFile.find(".db") != std::string::npos)) {
                ketQua[count].tenFile = tenFile;
                ketQua[count].duongDanDayDu = thuMuc + "/" + tenFile;
                
                size_t pos1 = tenFile.find("_");
                size_t pos2 = tenFile.find("_", pos1 + 1);
                if (pos1 != std::string::npos && pos2 != std::string::npos) {
                    ketQua[count].ngayTao = tenFile.substr(pos1 + 1, pos2 - pos1 - 1) + "_" + 
                                           tenFile.substr(pos2 + 1, 6);
                }
                
                ketQua[count].kichThuoc = LayKichThuocFile(ketQua[count].duongDanDayDu);
                count++;
            }
        }
        closedir(dir);
    }
#endif
    
    // Sap xep theo ngay tao (moi nhat truoc)
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (ketQua[i].ngayTao < ketQua[j].ngayTao) {
                ThongTinBackup temp = ketQua[i];
                ketQua[i] = ketQua[j];
                ketQua[j] = temp;
            }
        }
    }
    
    return count;
}

// Xoa file backup
std::string XoaBackup(const std::string& duongDanBackup) {
    if (duongDanBackup.empty()) {
        return "Loi: Duong dan backup khong hop le!";
    }
    
    if (remove(duongDanBackup.c_str()) == 0) {
        return ""; // Thanh cong
    } else {
        return "Loi: Khong the xoa file backup!";
    }
}

// Xoa backup cu (giu lai n backup moi nhat)
int XoaBackupCu(int soLuongGiuLai, const std::string& thuMucBackup) {
    if (soLuongGiuLai < 1) soLuongGiuLai = 1;
    
    ThongTinBackup dsBackup[1000];
    int soLuongBackup = LayDanhSachBackup(dsBackup, 1000, thuMucBackup);
    
    if (soLuongBackup <= soLuongGiuLai) {
        return 0; // Khong can xoa
    }
    
    int soLuongDaXoa = 0;
    
    // Xoa cac backup cu (tu vi tri soLuongGiuLai tro di)
    for (int i = soLuongGiuLai; i < soLuongBackup; i++) {
        if (XoaBackup(dsBackup[i].duongDanDayDu).empty()) {
            soLuongDaXoa++;
        }
    }
    
    return soLuongDaXoa;
}

// ---------------------- Archiving utilities -----------------------------

static std::string LayThuMucArchiveMacDinh() {
    return FilePathManager::getFilePath("backup_archives").toStdString();
}

// Ensure archive dir exists
static bool KiemTraThuMucArchive(const std::string& thuMuc) {
    std::string path = thuMuc.empty() ? LayThuMucArchiveMacDinh() : thuMuc;
    std::error_code ec;
    if (!std::filesystem::exists(path)) {
        if (!std::filesystem::create_directories(path, ec)) return false;
    }
    return true;
}

// Helper to run compression command. Uses PowerShell Compress-Archive on Windows,
// or zip on POSIX.
static bool CompressPathsToZip(const std::vector<std::string>& paths, const std::string& destZip) {
    if (paths.empty()) return false;
    std::string cmd;
#ifdef _WIN32
    // Build a PowerShell Compress-Archive command using wildcards when possible.
    // If multiple explicit files, join them with commas inside an array expression.
    std::string psPaths;
    for (size_t i = 0; i < paths.size(); ++i) {
        std::string p = paths[i];
        // Escape single quotes by doubling
        for (size_t pos = 0; (pos = p.find("'", pos)) != std::string::npos; pos += 2) p.replace(pos, 1, "''");
        psPaths += "'" + p + "'";
        if (i + 1 < paths.size()) psPaths += ",";
    }
    // Use -Force to overwrite
    cmd = "powershell -NoProfile -Command \"Compress-Archive -LiteralPath @(" + psPaths + ") -DestinationPath '" + destZip + "' -Force\"";
#else
    // POSIX: use zip -j to junk paths (store files at root of zip)
    // Build command like: zip -j -q dest.zip file1 file2 ...
    cmd = "zip -j -q \"" + destZip + "\"";
    for (auto &p : paths) {
        cmd += " \"" + p + "\"";
    }
#endif
    int rc = std::system(cmd.c_str());
    return rc == 0;
}

// AutoArchiveBackups: create monthly zips for months older than one month,
// then create yearly zips for full past years.
void AutoArchiveBackups() {
    std::string backupDir = LayThuMucBackupMacDinh();
    std::string archiveDir = LayThuMucArchiveMacDinh();
    if (!KiemTraThuMucBackup(backupDir)) return;
    if (!KiemTraThuMucArchive(archiveDir)) return;

    // Determine current year/month
    time_t now = time(nullptr);
    tm localNow;
#ifdef _WIN32
    localtime_s(&localNow, &now);
#else
    localtime_r(&now, &localNow);
#endif
    int curYear = localNow.tm_year + 1900;
    int curMonth = localNow.tm_mon + 1;

    // Collect backup files and group by year-month
    std::map<std::pair<int,int>, std::vector<std::string>> byMonth;
    for (const auto &entry : std::filesystem::directory_iterator(backupDir)) {
        if (!entry.is_regular_file()) continue;
        std::string name = entry.path().filename().string();
        if (name.rfind("backup_", 0) != 0) continue;
        // Expect format backup_YYYYMMDD_HHMMSS_...ext
        if (name.size() < 15) continue;
        std::string datePart = name.substr(7, 8); // YYYYMMDD
        if (datePart.size() != 8) continue;
        int y = std::stoi(datePart.substr(0,4));
        int m = std::stoi(datePart.substr(4,2));
        byMonth[{y,m}].push_back(entry.path().string());
    }

    // For each month older than last month (i.e., month < current month of current year or year < current year)
    for (auto &kv : byMonth) {
        int y = kv.first.first;
        int m = kv.first.second;
        // skip current month
        if (y == curYear && m >= curMonth) continue;
        // create month zip name
        char zname[64];
        sprintf(zname, "%s/month_%04d%02d.zip", archiveDir.c_str(), y, m);
        std::string dest = zname;
        // if already exists, skip
        if (std::filesystem::exists(dest)) continue;
        // compress
        if (CompressPathsToZip(kv.second, dest)) {
            // remove original files after successful compression
            for (auto &p : kv.second) {
                std::error_code ec; std::filesystem::remove(p, ec);
            }
        }
    }

    // Yearly packaging: for any year < curYear, pack all month_YYYYMM.zip into year_YYYY.zip
    // Collect months zips
    std::map<int, std::vector<std::string>> monthsByYear;
    for (const auto &entry : std::filesystem::directory_iterator(archiveDir)) {
        if (!entry.is_regular_file()) continue;
        std::string name = entry.path().filename().string();
        if (name.rfind("month_", 0) != 0) continue;
        // name like month_YYYYMM.zip
        if (name.size() < 11) continue;
        int y = std::stoi(name.substr(6,4));
        // int m = std::stoi(name.substr(10,2));
        if (y < curYear) monthsByYear[y].push_back(entry.path().string());
    }

    for (auto &kv : monthsByYear) {
        int y = kv.first;
        char yname[64];
        sprintf(yname, "%s/year_%04d.zip", archiveDir.c_str(), y);
        std::string dest = yname;
        if (std::filesystem::exists(dest)) continue;
        if (CompressPathsToZip(kv.second, dest)) {
            // remove month zips
            for (auto &p : kv.second) {
                std::error_code ec; std::filesystem::remove(p, ec);
            }
        }
    }
}

// Consolidate legacy backup folders into the centralized data/backup directory.
// Searches common legacy locations under the given root path (or current working dir)
// and moves any files named "backup_*.(txt|db)" into data/backup.
int ConsolidateLegacyBackups(const std::string& rootPath) {
    std::string root = rootPath.empty() ? std::string(".") : rootPath;
    std::string destDir = LayThuMucBackupMacDinh(); // data/backup
    // ensure dest exists
    KiemTraThuMucBackup(destDir);

    int moved = 0;
    std::error_code ec;

    // scan top-level entries under root
    for (const auto &entry : std::filesystem::directory_iterator(root, ec)) {
        if (ec) break;
        if (!entry.is_directory()) continue;
        std::string name = entry.path().filename().string();
        // consider directories that contain "backup" in name (legacy)
        if (name.find("backup") == std::string::npos) continue;

        // move files inside this directory that match backup_*.txt or backup_*.db
        for (const auto &f : std::filesystem::directory_iterator(entry.path(), ec)) {
            if (ec) break;
            if (!f.is_regular_file()) continue;
            std::string fname = f.path().filename().string();
            if (fname.rfind("backup_", 0) != 0) continue;
            // only accept .txt or .db
            if (fname.find(".txt") == std::string::npos && fname.find(".db") == std::string::npos) continue;

            std::string src = f.path().string();
            std::string dst = destDir + "/" + fname;
            // if dst already exists, append a suffix
            if (std::filesystem::exists(dst)) {
                // add numeric suffix to avoid overwrite
                int i = 1;
                std::string base = fname;
                std::string::size_type pos = base.rfind('.');
                std::string body = (pos==std::string::npos)? base : base.substr(0,pos);
                std::string ext = (pos==std::string::npos)? std::string() : base.substr(pos);
                do {
                    dst = destDir + "/" + body + "_" + std::to_string(i) + ext;
                    ++i;
                } while (std::filesystem::exists(dst));
            }

            std::error_code rc;
            std::filesystem::rename(src, dst, rc);
            if (!rc) {
                ++moved;
            } else {
                // fallback to copy+remove
                std::ifstream in(src, std::ios::binary);
                std::ofstream out(dst, std::ios::binary);
                if (in.is_open() && out.is_open()) {
                    out << in.rdbuf();
                    in.close();
                    out.close();
                    std::filesystem::remove(src, rc);
                    if (!rc) ++moved;
                }
            }
        }
    }

    return moved;
}

bool PackMonth(int year, int month) {
    std::string backupDir = LayThuMucBackupMacDinh();
    std::string archiveDir = LayThuMucArchiveMacDinh();
    if (!KiemTraThuMucArchive(archiveDir)) return false;
    char pattern[32]; sprintf(pattern, "%04d%02d", year, month);
    std::vector<std::string> toPack;
    for (const auto &entry : std::filesystem::directory_iterator(backupDir)) {
        if (!entry.is_regular_file()) continue;
        std::string name = entry.path().filename().string();
        if (name.find(std::string("backup_") + pattern) == 0) {
            toPack.push_back(entry.path().string());
        }
    }
    if (toPack.empty()) return false;
    char zname[64]; sprintf(zname, "%s/month_%04d%02d.zip", archiveDir.c_str(), year, month);
    std::string dest = zname;
    if (!CompressPathsToZip(toPack, dest)) return false;
    for (auto &p : toPack) { std::error_code ec; std::filesystem::remove(p, ec); }
    return true;
}

bool PackYear(int year) {
    std::string archiveDir = LayThuMucArchiveMacDinh();
    if (!KiemTraThuMucArchive(archiveDir)) return false;
    std::vector<std::string> monthZips;
    for (const auto &entry : std::filesystem::directory_iterator(archiveDir)) {
        if (!entry.is_regular_file()) continue;
        std::string name = entry.path().filename().string();
        if (name.rfind("month_", 0) == 0) {
            int y = std::stoi(name.substr(6,4));
            if (y == year) monthZips.push_back(entry.path().string());
        }
    }
    if (monthZips.empty()) return false;
    char yname[64]; sprintf(yname, "%s/year_%04d.zip", archiveDir.c_str(), year);
    std::string dest = yname;
    if (!CompressPathsToZip(monthZips, dest)) return false;
    for (auto &p : monthZips) { std::error_code ec; std::filesystem::remove(p, ec); }
    return true;
}
