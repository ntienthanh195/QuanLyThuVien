#ifndef NGAYTHANG_H
#define NGAYTHANG_H

#include <string>
#include <iostream>
#include <ctime>
#include "ThongBao.h" 

// Cac ham xu ly ngay thang
bool ChuanHoaNgay(std::string& s, std::ostream& out = std::cout); // Chua dinh nghia trong file .cpp ?
bool phanTichNgay(const std::string& s, int& ngay, int& thang, int& nam);
int LayNamHienTai();
time_t ChuyenNgaySangTimeT(const std::string& sNgay, std::ostream& out = std::cout); // Sua kieu tra ve
bool kiemTraQuaHan(std::string ngayMuon, std::ostream& out = std::cout); // Chua dinh nghia trong file .cpp ?
std::string layNgayHienTai(std::ostream& out = std::cout);
int tinhSoNgayQuaHan(std::string ngayMuon, std::ostream& out = std::cout);

#endif // NGAYTHANG_H