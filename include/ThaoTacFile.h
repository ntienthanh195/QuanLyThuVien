#ifndef THAOTACFILE_H
#define THAOTACFILE_H

#include <string>
#include <iostream>
#include "DauSach.h"
#include "ThongBao.h"
#include "Constants.h"

static constexpr char FILE_DAUSACH[] = "files/dausach.txt";
static constexpr char FILE_DANHMUCSACH[] = "files/danhmucsach.txt";

// Đọc ghi file
void NapDanhSachDauSach(const char* path, PTRDS dsDauSach[], int& soLuongDauSach, std::ostream& out = std::cout);
void NapDanhMucSach(const char* path, PTRDS dsDauSach[], int soLuongDauSach, std::ostream& out = std::cout);
bool GhiDanhSachDauSach(const char* path, PTRDS dsDauSach[], int soLuongDauSach, bool silent = false, std::ostream& out = std::cout);
bool GhiDanhMucSach(const char* path, PTRDS dsDauSach[], int soLuongDauSach, bool silent = false, std::ostream& out = std::cout);

// Tổng kết đọc/ghi
void InTongKet(const std::string& hanhDong, int thanhCong, int tongSo, int boQua, std::ostream& out = std::cout);

#endif // THAOTACFILE_H