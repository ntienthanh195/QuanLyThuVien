#ifndef DAUSACH_H
#define DAUSACH_H

#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include "Constants.h"

enum TrangThaiSach {
    SACH_TRONG_KHO = 0,     // Trong kho (Có thể mượn)
    SACH_DANG_MUON = 1,     // Đang mượn
    SACH_MAT = 2,           // Mất (Chưa tìm thấy)
    SACH_THANH_LY = 3,      // Đã thanh lý (Không còn trong biên chế)
    
    // Legacy Compatibility
    CHO_MUON_DUOC = SACH_TRONG_KHO,
    DANG_MUON = SACH_DANG_MUON,
    THANH_LY = SACH_THANH_LY
};

// cau truc Danh Muc Sach (DSLK don cac ban sao cua mot Dau Sach)
struct DanhMucSach {
    std::string maSach;         // Ma duy nhat cho tung cuon (ISBN-SoThuTu)
    TrangThaiSach trangThai;    // 0, 1, 2 theo enum TrangThaiSach
    std::string viTri;          // vi tri cu the tren ke sach (vd: "Ke A1")
    std::shared_ptr<DanhMucSach> next = nullptr;   //con tro toi node ban sao tiep theo

    DanhMucSach(std::string ma, TrangThaiSach tt, std::string vt, std::shared_ptr<DanhMucSach> n = nullptr)
        : maSach(ma), trangThai(tt), viTri(vt), next(n) {}
};

typedef std::shared_ptr<DanhMucSach> PTRDMS;    // con tro thong minh toi mot DanhMucSach

// Cau truc Dau Sach (thong tin chung cua mot tua sach)
struct DauSach {
    std::string ISBN;           // Ma so sach quoc te (duy nhat cho moi Dau Sach) 10 hoac 13 ki tu
    std::string tenSach;
    int soTrang;
    std::string tacGia;
    int namXuatBan;
    std::string theLoai;
    PTRDMS dms = nullptr;          // con tro toi dau danh sach lien ket cac ban sao (DanhMucSach)
    std::vector<PTRDMS> dms_vector; // Vector chứa bản sao để duyệt nhanh (Phase 5 Modernization)
    int tongBanSao;             // Tổng số lượng bản sao hiện có
    int soLuotMuon;             // them de de tim top 10 sach
    
    // Feature: Book Price (VNĐ)
    long long giaTien = 0;
    
    // NEW FIELDS (PHASE 2)
    std::string NhaXuatBan;
};

// typedef DauSach* PTRDS;         // Con tro toi mot DauSach
typedef std::shared_ptr<DauSach> PTRDS; // Smart Pointer toi mot DauSach

// Moved imports to top 

// Bien toan cuc (khai bao extern de cac file khac co the truy cap)
// Phase 5: Modernized to use std::vector and std::shared_ptr
extern std::vector<PTRDS> dsDauSach; // Vector luu tru cac Dau Sach (Smart Pointer)
extern int soLuongDauSach;           // So luong Dau Sach (deprecated usage, use dsDauSach.size())
extern bool duLieuDaThayDoi;         // de kiem tra viec luu file khi thoat

#endif // DAUSACH_H