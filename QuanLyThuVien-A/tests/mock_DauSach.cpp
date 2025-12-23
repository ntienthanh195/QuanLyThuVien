#include "QuanLySach.h"
#include "DauSach.h"
#include "Constants.h"
#include <iostream>
#include <vector>

#include <vector>


// ================== MOCK QuanLySach.cpp ==================
// Essential Stubs only

// Globals
PTRDS dsDauSach[MAX_DAUSACH];
int soLuongDauSach = 0;
bool duLieuDaThayDoi = false;
std::mutex g_ds_mutex;

// Used by DocGia/MuonTra
PTRDMS timDanhMucTheoMaSach(const std::string &ma, PTRDS [], int /*n*/, std::ostream& /*out*/, bool /*silent*/) {
    if (ma.find("B00") != std::string::npos) {
        static NodeDMS node;
        node.maSach = ma;
        node.trangThai = 0; 
        return &node;
    }
    return nullptr;
}

PTRDMS MaSach_to_PTRDMS(const std::string& maSach, PTRDS [], int /*soLuongDauSach*/) {
    return timDanhMucTheoMaSach(maSach, nullptr, 0, std::cout, true);
}

std::string PTRDMS_to_String(PTRDMS p) { return p ? p->maSach : ""; }

PTRDMS TimBanSaoTheoMa(PTRDS, const std::string& maSach) {
   return timDanhMucTheoMaSach(maSach, nullptr, 0, std::cout, true);
}

bool capNhatTrangThaiSach(PTRDMS dms, const std::string&, TrangThaiSach tt) {
     if(dms) dms->trangThai = tt;
     return true;
}

void CapNhatTongBanSao(PTRDS [], int) {}

// Likely not used but harmless
PTRDS TimDauSachTheoISBN(PTRDS [], int, const std::string&) { return nullptr; }

// If DocGia calls these:
std::string ChuanHoaISBNFile(const std::string&, std::string&) { return ""; }
std::string KiemTraTrungISBN(const std::string&) { return ""; }
std::string KiemTraChuoiVaDodai(const std::string&, const std::string&, int) { return ""; }
std::string KiemTraNamXuatBan(int) { return ""; }

// Required global function signatures (if referenced)


// Required global function signatures (if referenced)
// If linker errors appear, we add them.

// For now, removing ALL the problematic search/sort functions.
