#include "utils/BackupManager.h"
#include "models/ThaoTacFile.h"
#include "utils/NgayThang.h"
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>

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
    return "files/backup";
}

// Tao ten file backup theo format
std::string TaoTenFileBackup(const std::string& loaiFile) {
    time_t now = time(nullptr);
    tm* ltm = localtime(&now);
    
    char buffer[100];
    sprintf(buffer, "backup_%04d%02d%02d_%02d%02d%02d_%s.txt",
            1900 + ltm->tm_year,
            1 + ltm->tm_mon,
            ltm->tm_mday,
            ltm->tm_hour,
            ltm->tm_min,
            ltm->tm_sec,
            loaiFile.c_str());
    
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
    
    // Backup cac file du lieu
    std::string cacFile[] = {
        "files/dausach.txt",
        "files/danhmucsach.txt",
        "files/docgia.txt",
        "files/muontra.txt"
    };
    
    std::string cacLoai[] = {
        "dausach",
        "danhmucsach",
        "docgia",
        "muontra"
    };
    
    int soLuongBackup = 0;
    std::stringstream kq;
    
    for (int i = 0; i < 4; i++) {
        std::string tenBackup = TaoTenFileBackup(cacLoai[i]);
        std::string duongDanBackup = thuMuc + "/" + tenBackup;
        
        if (SaoChepFile(cacFile[i], duongDanBackup)) {
            soLuongBackup++;
        } else {
            kq << "Canh bao: Khong the backup " << cacFile[i] << "\\n";
        }
    }
    
    if (soLuongBackup == 4) {
        return ""; // Thanh cong
    } else if (soLuongBackup > 0) {
        return "Canh bao: Chi backup duoc " + std::to_string(soLuongBackup) + "/4 file!\\n" + kq.str();
    } else {
        return "Loi: Khong the backup bat ky file nao!";
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
    
    // Xac dinh loai file backup
    std::string loaiFile;
    if (duongDanBackup.find("dausach") != std::string::npos) {
        loaiFile = "dausach";
    } else if (duongDanBackup.find("danhmucsach") != std::string::npos) {
        loaiFile = "danhmucsach";
    } else if (duongDanBackup.find("docgia") != std::string::npos) {
        loaiFile = "docgia";
    } else if (duongDanBackup.find("muontra") != std::string::npos) {
        loaiFile = "muontra";
    } else {
        return "Loi: Khong xac dinh duoc loai file backup!";
    }
    
    // Xac dinh file dich
    std::string fileDich = "files/" + loaiFile + ".txt";
    
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
    std::string searchPath = thuMuc + "\\backup_*.txt";
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (count >= maxKetQua) break;
            
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                ketQua[count].tenFile = findData.cFileName;
                ketQua[count].duongDanDayDu = thuMuc + "/" + findData.cFileName;
                
                // Lay ngay gio tu ten file (backup_YYYYMMDD_HHMMSS_loai.txt)
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
            if (tenFile.find("backup_") == 0 && tenFile.find(".txt") != std::string::npos) {
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
