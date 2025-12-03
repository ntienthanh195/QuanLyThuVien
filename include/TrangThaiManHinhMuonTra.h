// [FILE: TrangThaiManHinhMuonTra.h]
#ifndef TRANG_THAI_MAN_HINH_MUON_TRA_H
#define TRANG_THAI_MAN_HINH_MUON_TRA_H

#include "DocGia.h"
#include "QuanLySach.h"
#include <SFML/Graphics.hpp>
#include <string>

// Hang so phan trang (đồng bộ với SACH_MOI_TRANG)
const int DOC_GIA_MOI_TRANG = 17; // So doc gia hien thi moi trang (giống SACH_MOI_TRANG)

const int QUA_HAN_MOI_TRANG = 15; // So doc gia qua han hien thi moi trang

// Cau truc DTO cho doc gia hien thi trong bang (giong KetQuaTimKiem)
struct DocGiaTableDTO {
    PTRDG docGia;
    int loaiKhop; // 0: Exact match, 1: Partial match, 2: No search (show all)
};

// Cau truc state don gian theo pattern SachState
struct MuonTraState {
    // ===== TRANG THAI BANG CHINH (Table View) =====
    DocGiaTableDTO ketQuaTimKiem[MAX_DAUSACH];  // Danh sach doc gia hien thi
    int soLuongKetQuaTimKiem;                   // So luong doc gia tim thay
    int trangHienTai;                           // Trang hien tai (1-based)
    int tongSoTrang;                            // Tong so trang
    
    // ===== TRANG THAI FORM NHAP LIEU (Form State) =====
    std::string chuoiTimKiem;                   // Tu khoa tim kiem doc gia
    std::string chuoiMaSach;                    // Ma sach can muon/tra
    
    // ===== TRANG THAI LOGIC (Logic State) =====
    int maTheDocGiaDuocChon;                    // Ma the doc gia dang chon (0 = chua chon)
    PTRDG docGiaDangChon;                       // Con tro doc gia dang chon
    std::string maSachDangChon;                 // Ma sach ca biet dang chon (cho tra/bao mat)
    int indexSachDangChon;                      // Index trong mang listSachDangMuon
    
    // ===== DATA CACHE (Cached Data) =====
    ThongTinSachDangMuon_DTO listSachDangMuon[10];  // Sach dang muon cua doc gia duoc chon
    int slSachDangMuon;                              // So luong sach dang muon
    
    // ===== TRANG THAI DOUBLE CLICK (Double Click Detection) =====
    sf::Clock doubleClickClock;
    int maTheClickCuoi;                         // Ma the click lan cuoi
    const float THOI_GIAN_DOUBLE_CLICK = 0.3f;
    int maTheRowClicked;                        // Ma the row vua click
    
    // ===== TRANG THAI MODAL (Modal State) =====
    bool hienThiModalTop10;                     // Hien thi modal top 10
    bool hienThiModalQuaHan;                    // Hien thi modal doc gia qua han
    TopSachDTO top10[10];                       // Data top 10 sach
    int soLuongTop;                             // So luong sach trong top 10
    DocGiaQuaHanDTO dsQuaHan[MAX_DAUSACH];      // Data doc gia qua han
    int soLuongQuaHan;                          // So luong doc gia qua han

    int trangQuaHanHienTai;                     // Trang hien tai cua modal qua han
    int tongTrangQuaHan;                       // Tong so trang cua modal hien tai
    
    // ===== TRANG THAI XAC NHAN (Confirmation State) =====
    bool xacNhanMuon;                           // Hien thi hop thoai xac nhan muon
    bool xacNhanTra;                            // Hien thi hop thoai xac nhan tra
    bool xacNhanBaoMat;                         // Hien thi hop thoai xac nhan bao mat
    
    // Constructor
    MuonTraState() { 
        Reset(); 
    }
    
    void Reset() {
        // Reset table state
        soLuongKetQuaTimKiem = 0;
        trangHienTai = 1;
        tongSoTrang = 1;
        
        // Reset form state
        chuoiTimKiem = "";
        chuoiMaSach = "";
        
        // Reset logic state
        maTheDocGiaDuocChon = 0;
        docGiaDangChon = nullptr;
        maSachDangChon = "";
        indexSachDangChon = -1;
        
        // Reset cache
        slSachDangMuon = 0;
        
        // Reset double click
        maTheClickCuoi = 0;
        maTheRowClicked = 0;
        
        // Reset modal state
        hienThiModalTop10 = false;
        hienThiModalQuaHan = false;
        soLuongTop = 0;
        soLuongQuaHan = 0;

        trangQuaHanHienTai = 1;
        tongTrangQuaHan = 1;
        
        // Reset confirmation state
        xacNhanMuon = false;
        xacNhanTra = false;
        xacNhanBaoMat = false;
    }
};

// API
void KhoiTaoManHinhMuonTra();
void VeManHinhMuonTra(sf::RenderWindow &window, const sf::Font &font);
void XuLySuKienManHinhMuonTra(sf::RenderWindow &window, sf::Event event);

#endif