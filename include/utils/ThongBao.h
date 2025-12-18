#ifndef THONGBAO_H
#define THONGBAO_H

#include <string>
#include <iostream>

#include "Constants.h"

// Dinh nghia cac loai thong bao
enum LoaiThongBao { LOI, THONG_TIN, CANH_BAO };

// Hien thi thong bao chung
void thongBao(std::ostream& out, const std::string& msg, LoaiThongBao loai);

// Tao chuoi thong bao loi khi doc file DauSach.txt
std::string thongBaoLoi(int soDong, const std::string& chiTiet, bool canhBao = false);

// Tao chuoi thong bao loi khi doc file DanhMucSach.txt
std::string thongBaoLoiDMS(int soDong, const std::string& chiTiet, bool canhBao = false);

// Hien thi so lan thu con lai khi nhap lieu sai
void thongBaoSoLanThuConLai(int soLanDaThu, std::ostream& out, int maxLanThu = SO_LAN_THU_TOI_DA);

#endif // THONGBAO_H