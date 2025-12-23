/**
 * @file MuonTra.h
 * @path include/models/MuonTra.h
 * @description Header định nghĩa cấu trúc dữ liệu mượn trả (MuonTra).
 * Quản lý danh sách liên kết kép các lượt mượn của độc giả.
 * @project Library Management System (QuanLyThuVien)
 */
#ifndef MUONTRA_H

#define MUONTRA_H

#include <string>
#include "models/DocGia.h"
#include "models/DauSach.h"

// Main transaction functions
std::string MuonSach(PTRDG docGia, const std::string& isbn, std::vector<PTRDS>& dsDauSach, int soLuongDauSach);
std::string TraSach(PTRDG docGia, const std::string& maSach, std::vector<PTRDS>& dsDauSach, int soLuongDauSach);
std::string BaoMatSach(PTRDG docGia, const std::string& maSach, std::vector<PTRDS>& dsDauSach, int soLuongDauSach);
std::string GiaHanMuon(PTRDG docGia, const std::string& maSach);
std::string ThanhToanPhat(PTRDG docGia, const std::string& maSach);

// Helper functions for book copies
PTRDMS TimBanSaoCoTheMuon(PTRDS dauSach);
PTRDMS TimBanSaoTheoMa(PTRDS dauSach, const std::string& maSach);
int DemBanSaoCoTheMuon(PTRDS dauSach);

// Transaction management helpers
void themMuonTra(PTRDG docgia, PTRDMS banSaoSach);
void themMuonTra(PTRDG docgia, const MuonTra &mt);

// Reporting / Listing
int LayDSSachDangMuon(PTRDG docGia, ThongTinSachDangMuon_DTO ketQua[], int maxKetQua, std::vector<PTRDS>& dsDauSach, int soLuongDauSach);
int TinhSoNgayQuaHanLonNhat(PTRDG docgia);
void CapNhatNgayHetHanSomNhat(PTRDG docGia); // Recalculate O(1) stats

#endif // MUONTRA_H
