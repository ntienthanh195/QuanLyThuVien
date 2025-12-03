#ifndef TRANG_THAI_MAN_HINH_DOC_GIA_H
#define TRANG_THAI_MAN_HINH_DOC_GIA_H

#include "DocGia.h"
#include <SFML/Graphics.hpp>
#include <string>

// ===== 1. ENUM CHẾ ĐỘ XEM =====
enum CheDoXemDocGia {
    XEM_DANH_SACH_MA_THE,     // Hiển thị theo mã thẻ (InOrder)
    XEM_DANH_SACH_TEN_HO,     // Hiển thị theo tên+họ (Sorted)
    XEM_DOC_GIA_QUA_HAN       // Chế độ xem quá hạn (màu đỏ)
};

// ===== 2. STATE STRUCTURE =====
struct DocGiaState {
    // --- Dữ liệu hiển thị ---
    PTRDG mangDocGia[MAX_DAUSACH];        // Mảng con trỏ cho sắp xếp
    int soLuongDocGia;                     // Số lượng độc giả
    ThongTinQuaHan mangQuaHan[MAX_DAUSACH]; // Mang doc gia qua han (dung ThongTinQuaHan thay vi DocGiaQuaHanDTO)
    int soLuongQuaHan;                     // Số lượng quá hạn
    
    // --- Chế độ xem ---
    CheDoXemDocGia cheDoXem;
    
    // --- Phân trang ---
    int trangHienTai;
    int tongSoTrang;
    
    // --- Chọn dòng ---
    int maTheDocGiaDuocChon;               // -1 nếu chưa chọn
    std::string tenDocGiaDuocChon;         // Dùng cho thông báo
    
    // --- Form nhập liệu ---
    bool dangSua;
    bool xacNhanXoa;
    std::string chuoiHo;
    std::string chuoiTen;
    int phaiDuocChon;                      // 0=Nam, 1=Nữ
    int trangThaiDuocChon;                 // 0=Khóa, 1=Hoạt động
    
    // --- Modal Chi Tiết ---
    bool hienThiModalChiTiet;              // Hiện modal sách đang mượn
    ThongTinSachDangMuon_DTO dsSachDangMuon[10]; // Tối đa 10 sách
    int soLuongSachDangMuon;
    
    // --- Undo Stack ---
    //UndoStack undoStack;
    bool stackDaKhoiTao;
    
    // --- Double Click ---
    sf::Clock doubleClickClock;
    int maTheClickCuoi;
    const float THOI_GIAN_DOUBLE_CLICK = 0.3f;
    
    // Constructor
    DocGiaState() : 
        soLuongDocGia(0), 
        soLuongQuaHan(0),
        cheDoXem(XEM_DANH_SACH_MA_THE),
        trangHienTai(1), 
        tongSoTrang(1),
        maTheDocGiaDuocChon(-1),
        dangSua(false),
        xacNhanXoa(false),
        phaiDuocChon(0),
        trangThaiDuocChon(1),
        hienThiModalChiTiet(false),
        soLuongSachDangMuon(0),
        stackDaKhoiTao(false),
        maTheClickCuoi(-1) {}
};

#endif