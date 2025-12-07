#ifndef BACKUP_MANAGER_H
#define BACKUP_MANAGER_H

#include <string>
#include <vector>

// Cau truc thong tin backup
struct ThongTinBackup {
    std::string tenFile;          // Ten file backup
    std::string duongDanDayDu;    // Duong dan day du
    std::string ngayTao;          // Ngay tao backup (YYYYMMDD_HHMMSS)
    long long kichThuoc;          // Kich thuoc file (bytes)
};

// Backup Manager - Quan ly sao luu va phuc hoi du lieu

// Tao backup thu cong
std::string TaoBackup(const std::string& thuMucBackup = "");

// Tao backup tu dong (goi moi khi luu file)
std::string AutoBackup();

// Phuc hoi du lieu tu file backup
std::string PhucHoiBackup(const std::string& duongDanBackup);

// Lay danh sach tat ca backup
int LayDanhSachBackup(ThongTinBackup ketQua[], int maxKetQua, const std::string& thuMucBackup = "");

// Xoa file backup
std::string XoaBackup(const std::string& duongDanBackup);

// Xoa tat ca backup cu (giu lai n backup moi nhat)
int XoaBackupCu(int soLuongGiuLai = 10, const std::string& thuMucBackup = "");

// Kiem tra va tao thu muc backup neu chua co
bool KiemTraThuMucBackup(const std::string& thuMuc);

// Lay duong dan thu muc backup mac dinh
std::string LayThuMucBackupMacDinh();

// Tao ten file backup theo format: backup_YYYYMMDD_HHMMSS_loai.txt
std::string TaoTenFileBackup(const std::string& loaiFile);

// Archive old backups: create monthly zips for months older than 1 month,
// and yearly zips for full past years. Archives stored under data/backup_archives
void AutoArchiveBackups();

// Force pack a specific month (YYYY, MM) into a zip
bool PackMonth(int year, int month);

// Pack all months of a year into a yearly zip
bool PackYear(int year);

// Consolidate legacy backup folders (e.g. data/backup, data/backup_migration_*)
// into the centralized `data/backup` folder. Returns number of files moved.
int ConsolidateLegacyBackups(const std::string& rootPath = "");
#endif // BACKUP_MANAGER_H
