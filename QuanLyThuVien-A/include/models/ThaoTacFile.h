/**
 * @file ThaoTacFile.h
 * @path include/models/ThaoTacFile.h
 * @description Header định nghĩa các hàm API cho thao tác đọc/ghi file dữ liệu.
 * @project Library Management System (QuanLyThuVien)
 */
#ifndef THAOTACFILE_H

#define THAOTACFILE_H

#include <string>
#include <iostream>
#include "DauSach.h"
#include "ThongBao.h"
#include "Constants.h"
#include "DocGia.h"

// Use data/ directory for storage. FilePathManager resolves actual path at runtime.
static constexpr char FILE_DAUSACH[] = "data/dausach.txt";
static constexpr char FILE_DANHMUCSACH[] = "data/danhmucsach.txt";

// Đọc ghi file
void NapDanhSachDauSach(const char* path, PTRDS dsDauSach[], int& soLuongDauSach, std::ostream& out = std::cout);
void NapDanhMucSach(const char* path, PTRDS dsDauSach[], int soLuongDauSach, std::ostream& out = std::cout);
bool GhiDanhSachDauSach(const char* path, PTRDS dsDauSach[], int soLuongDauSach, bool silent = false, std::ostream& out = std::cout);
bool GhiDanhMucSach(const char* path, PTRDS dsDauSach[], int soLuongDauSach, bool silent = false, std::ostream& out = std::cout);

// Tổng kết đọc/ghi
void InTongKet(const std::string& hanhDong, int thanhCong, int tongSo, int boQua, std::ostream& out = std::cout);

// DocGia file helpers (centralize legacy text I/O for docgia.txt / muontra)
void ThaoTacFile_LuuGiaoDich(MUONTRA dsmt, std::ofstream& file);
void ThaoTacFile_NapGiaoDich(PTRDG docGia, std::ifstream& file, PTRDS dsDauSach[], int n);
void ThaoTacFile_LuuNodeVaoFile(PTRDG node, std::ofstream& file);
bool GhiDocGiaFile(const char* path, PTRDG root, bool silent = false, std::ostream& out = std::cout);
PTRDG NapDocGiaFile(const char* path, PTRDS dsDauSach[], int soLuongDS, std::ostream& out = std::cout);
bool ThaoTacFile_BackupFile(const std::string& srcPath, const std::string& backupPath);

#endif // THAOTACFILE_H