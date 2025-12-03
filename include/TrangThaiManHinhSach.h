#ifndef TRANG_THAI_MAN_HINH_SACH_H
#define TRANG_THAI_MAN_HINH_SACH_H

#include <SFML/Graphics.hpp>
#include <string>
#include "Constants.h"
#include "DauSach.h"

// Bien trang thai CUC BO cho Man Hinh Sach

// trang thai che do xem
enum CheDoXemSach {
    XEM_TIM_KIEM,     // che do mac dinh: bang phan trang, tim kiem
    XEM_THEO_THE_LOAI // che do xem gom nhom theo the loai
};

// Bien cho Modal Chi Tiet
const int SO_SACH_MOI_TRANG_MODAL = 10;

struct NutModal {
    std::string maSach;
    sf::FloatRect khuVucNhanThanhLy;
    sf::FloatRect khuVucNhanXoa;
};

//DINH NGHIA STACK UNDO
const int MAX_UNDO_STEPS = 20; // Luu toi da 20 buoc hoan tac

// Node luu 1 trang thai van ban
struct UndoStackNode {
    std::string data;
};

// Cau truc Stack
struct UndoStack {
    UndoStackNode history[MAX_UNDO_STEPS]; // Mang luu lich su
    int top = -1;                          // Chi so dinh stack (-1 la rong)
};


struct SachState {
    // Trang thai bang chinh
    KetQuaTimKiem ketQuaTimKiem[MAX_DAUSACH];
    int soLuongKetQuaTimKiem = 0;
    int trangHienTai = 1;
    int tongSoTrang = 1;

    // Trang thai che do xem
    CheDoXemSach cheDoXemHienTai = XEM_TIM_KIEM;
    float scrollOffsetYTheLoai = 0.f;
    float totalContentHeightTheLoai = 0.f;
    sf::View theLoaiView;
    
    // Cache danh sach the loai va vi tri (chi cap nhat khi du lieu thay doi)
    std::string cacTheLoaiCache[MAX_DAUSACH];
    int soTheLoaiCache = 0;
    std::string cacViTriCache[MAX_DAUSACH];
    int soViTriCache = 0;
    bool canCapNhatCache = true; // Flag danh dau can cap nhat cache
    
    // Trang thai form nhap lieu
    std::string chuoiTimKiem = "";
    std::string chuoiISBN = "";
    std::string chuoiTenSach = "";
    std::string chuoiSoTrang = "";
    std::string chuoiTacGia = "";
    std::string chuoiNamXB = "";
    std::string chuoiTheLoai = "";
    std::string chuoiViTri = "";
    std::string chuoiSoLuong = "";

    // Trang thai logic
    std::string isbnSachDuocChon = "";
    bool dangSua = false;
    bool xacNhanXoa = false;
    bool vuaThucHienThanhCong = false;

    // Trang thai double click
    sf::Clock doubleClickClock;
    std::string isbnClickCuoi = "";
    const float THOI_GIAN_DOUBLE_CLICK = 0.3f;
    std::string isbnRowClicked = "";

    // Trang thai Modal Chi Tiet Ban Sao
    bool hienThiModalBanSao = false;
    std::string idModalSachDuocChon = "";
    int trangModal = 1;
    NutModal cacNutTrongModal[SO_SACH_MOI_TRANG_MODAL];
    int soLuongNutModal = 0;

    // Trang thai Modal Xac Nhan Thanh Ly
    bool xacNhanThanhLy = false;
    std::string maSachCanThanhLy = "";

    // Trang thai Modal Xac Nhan Xoa Ban Sao
    bool xacNhanXoaBanSao = false;
    std::string maSachCanXoa = "";

    // Trang thai Modal Them Ban Sao
    bool hienThiModalThemBS = false;
    std::string soLuongBanSaoCanThemStr = "";

    // Trang thai Undo
    UndoStack undoStack;
    bool stackDaKhoiTao = false;
};


#endif // TRANG_THAI_MAN_HINH_SACH_H