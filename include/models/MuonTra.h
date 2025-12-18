#ifndef MUONTRA_H
#define MUONTRA_H

#include <string>
#include "models/DocGia.h"
#include "models/DauSach.h"

// Main transaction functions
std::string MuonSach(PTRDG docGia, const std::string& isbn, PTRDS dsDauSach[], int soLuongDauSach);
std::string TraSach(PTRDG docGia, const std::string& maSach, PTRDS dsDauSach[], int soLuongDauSach);
std::string BaoMatSach(PTRDG docGia, const std::string& maSach, PTRDS dsDauSach[], int soLuongDauSach);
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
int LayDSSachDangMuon(PTRDG docGia, ThongTinSachDangMuon_DTO ketQua[], int maxKetQua, PTRDS dsDauSach[], int soLuongDauSach);
int TinhSoNgayQuaHanLonNhat(PTRDG docgia);

#endif // MUONTRA_H
