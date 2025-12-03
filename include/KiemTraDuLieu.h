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
#endif
