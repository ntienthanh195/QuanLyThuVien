#ifndef KIEMTRADULIEU_H
#define KIEMTRADULIEU_H

#include <string>
#include <iostream> 
#include "DauSach.h"

std::string KiemTraChuoiRong(const std::string& chuoi, const std::string& tenTruong);
std::string KiemTraTrungISBN(const std::string& ISBN);
bool KiemTraTrungmaSach(const std::string& maSach, std::ostream& out);
std::string KiemTraChuoiVaDodai(const std::string& str, const std::string& fieldName, int maxLen);
std::string KiemTraNamXuatBan(int nam);
std::string KiemTraISBN(const std::string& ISBN);
bool KiemTraDanhSachHopLe(void* danhSach, int soLuong, int maxSize, const std::string& tenDS, std::ostream& out);
std::string KiemTraTongSoBanSao(const std::string& isbn, int soLuongThem);

// Validation functions mới
std::string KiemTraSoDienThoai(const std::string& sdt);
std::string KiemTraCMND(const std::string& cmnd);
std::string KiemTraEmail(const std::string& email);
std::string KiemTraNgayHopLe(int ngay, int thang, int nam);
std::string KiemTraSoNguyen(const std::string& str, const std::string& tenTruong, int min = INT_MIN, int max = INT_MAX);
std::string KiemTraSoThuc(const std::string& str, const std::string& tenTruong, double min = -1e9, double max = 1e9);
std::string KiemTraTenNguoi(const std::string& ten, const std::string& tenTruong);

#endif
