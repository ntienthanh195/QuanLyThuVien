#include "utils/FilePathManager.h"
#include "models/ThaoTacFile.h"
#include "utils/BackupManager.h"

// ================== FilePathManager Mock ==================
QString FilePathManager::rootPath = "";
QString FilePathManager::filesDir = "";
bool FilePathManager::isInitialized = false;

// No-op init
// Note: We can leave static members, but methods need impl
// Since implementation is in FilePathManager.cpp in real app, we implement here in mock.

// ================== ThaoTacFile Mock ==================
void NapDanhSachDauSach(const char* path, PTRDS dsDauSach[], int& soLuongDauSach, std::ostream& out) {
    soLuongDauSach = 0; // Mock empty load
}

void NapDanhMucSach(const char* path, PTRDS dsDauSach[], int soLuongDauSach, std::ostream& out) {}

bool GhiDanhSachDauSach(const char* path, PTRDS dsDauSach[], int soLuongDauSach, bool silent, std::ostream& out) {
    return true; // Mock success
}

bool GhiDanhMucSach(const char* path, PTRDS dsDauSach[], int soLuongDauSach, bool silent, std::ostream& out) {
    return true;
}

void InTongKet(const std::string& hanhDong, int thanhCong, int tongSo, int boQua, std::ostream& out) {}

void ThaoTacFile_LuuGiaoDich(MUONTRA dsmt, std::ofstream& file) {}
void ThaoTacFile_NapGiaoDich(PTRDG docGia, std::ifstream& file, PTRDS dsDauSach[], int n) {}
void ThaoTacFile_LuuNodeVaoFile(PTRDG node, std::ofstream& file) {}
bool GhiDocGiaFile(const char* path, PTRDG root, bool silent, std::ostream& out) { return true; }
PTRDG NapDocGiaFile(const char* path, PTRDS dsDauSach[], int soLuongDS, std::ostream& out) { return nullptr; }
bool ThaoTacFile_BackupFile(const std::string& srcPath, const std::string& backupPath) { return true; }

// ================== BackupManager Mock ==================
// Assuming BackupManager functions are static or free, check imports?
// DocGia.cpp includes "utils/BackupManager.h"
// Let's assume standard BackupManager class or namespace.
// If BackupManager.cpp is not linked, we need to mock whatever DocGia uses.
// DocGia doesn't seemingly call BackupManager directly in the snippet I saw, but let's provide if tricky.
// Wait, I didn't see explicit usage in DocGia snippet. Maybe line 12 include?
// If linker errors on BackupManager, I will add them.
// For now, ThaoTacFile.h uses `ThaoTacFile_BackupFile`. Confirmed mock above.
