#ifndef GIAODIEN_SFML_H
#define GIAODIEN_SFML_H

#include <SFML/Graphics.hpp>
#include <string>
#include "QuanLySach.h"


// Cac hang so giao dien

// Kich thuoc cua so va cac thanh phan
const unsigned int CHIEU_RONG = 1600;
const unsigned int CHIEU_CAO = 800;

// Mau sac giao dien
const sf::Color MAU_NEN(30, 30, 45);
const sf::Color MAU_TIEU_DE(255, 200, 100);
const sf::Color MAU_NEN_NUT(100, 100, 110);
const sf::Color MAU_CHU_NUT(220, 220, 220);
const sf::Color MAU_CHU(200, 200, 200);
const sf::Color MAU_NHAN(100, 150, 255);
const sf::Color MAU_NUT_BACK(80, 80, 90);
const sf::Color MAU_KHUNG(50, 50, 65);
const sf::Color MAU_VIEN(150, 150, 160);
const sf::Color MAU_INPUT_NEN(60, 60, 75);
const sf::Color MAU_INPUT_CHU(210, 210, 210);
const sf::Color MAU_INPUT_BORDER(120, 120, 130);
const sf::Color MAU_INPUT_ACTIVE_BORDER(100, 150, 255);
const sf::Color MAU_LOI(255, 100, 100);
const sf::Color MAU_THANH_CONG(100, 255, 100);
const sf::Color MAU_BANG_HEADER(45, 45, 60);
const sf::Color MAU_BANG_BORDER(80, 80, 90);

// Kich thuoc font chu
const unsigned int FONT_SIZE_NHO = 16;
const unsigned int FONT_SIZE_BINH_THUONG = 18;
const unsigned int FONT_SIZE_TIEU_DE_KHUNG = 22;
const unsigned int FONT_SIZE_TIEU_DE_LON = 32;
const unsigned int FONT_SIZE_TIEU_DE_MAIN = 42;

// Kich thuoc layout
const float THANH_TAB_CAO = 40.f;
const float PADDING = 20.f;
const float FORM_RONG = 450.f;
const float BANG_X = PADDING;
const float BANG_Y = THANH_TAB_CAO + PADDING;
const float BANG_RONG = CHIEU_RONG - FORM_RONG - 3 * PADDING;
const float FORM_X = BANG_X + BANG_RONG + PADDING;
const float FORM_Y = BANG_Y + 50.f;
const float KHUNG_THONG_BAO_Y = CHIEU_CAO - PADDING - 140.f;
const float KHUNG_THONG_BAO_CAO = 140.f;
const float NUT_CAO = 40.f;
const float INPUT_CAO = 40.f;
const float INPUT_RONG = FORM_RONG - 2 * PADDING - 110.f;
const int SACH_MOI_TRANG = 17;


// Enum cac trang thai man hinh

enum TrangThaiManHinh {
    MENU_CHINH,           // Man hinh menu chinh
    MAN_HINH_THONG_TIN,   // Man hinh thong tin
    QUAN_LY_SACH,         // Man hinh quan ly sach
    QUAN_LY_DOC_GIA,      // Man hinh quan ly doc gia
    MUON_TRA_SACH         // Man hinh muon tra sach
};


// Enum cac ID UI cho cac nut, input, modal

enum MaUI {
    // Chung
    KHONG_XAC_DINH,           // Khong xac dinh
    NUT_BACK,                 // Nut quay lai
    HANG_SACH,                // Click vao dong sach

    // Menu chinh
    NUT_QUAN_LY_DAU_SACH,     // Vao quan ly dau sach
    NUT_QUAN_LY_DOC_GIA,      // Vao quan ly doc gia
    NUT_MUON_TRA_SACH,        // Vao muon tra sach
    NUT_THONG_TIN,            // Xem thong tin
    NUT_THOAT,                // Thoat chuong trinh

    // Quan ly sach
    INPUT_TIM_SACH,           // O tim sach
    NUT_TIM,                  // Nut tim kiem
    NUT_XOA_TIM,              // Nut xoa tim kiem
    NUT_CHUYEN_CHE_DO_XEM,    // Chuyen che do xem
    INPUT_ISBN,               // O nhap ISBN
    INPUT_TEN_SACH,           // O nhap ten sach
    INPUT_SO_TRANG,           // O nhap so trang
    INPUT_TAC_GIA,            // O nhap tac gia
    INPUT_NAM_XB,             // O nhap nam xuat ban
    INPUT_THE_LOAI,           // O nhap the loai
    INPUT_SO_LUONG,           // O nhap so luong ban sao
    INPUT_VI_TRI,             // O nhap vi tri sach
    NUT_THEM_SACH,            // Nut them sach
    NUT_SUA,                  // Nut sua sach
    NUT_XOA,                  // Nut xoa sach
    NUT_XAC_NHAN_XOA,         // Nut xac nhan xoa
    NUT_HUY_XOA,              // Nut huy xoa
    NUT_TRANG_TRUOC,          // Nut trang truoc
    NUT_TRANG_SAU,            // Nut trang sau
    NUT_HUY_CHON,             // Nut huy chon sach
    NUT_MODAL_TRANG_TRUOC,    // Nut trang truoc trong modal
    NUT_MODAL_TRANG_SAU,      // Nut trang sau trong modal

    // Modal chi tiet ban sao
    NUT_CHI_TIET_SACH,        // Nut xem chi tiet ban sao
    NUT_DONG_MODAL_CHITIET,   // Nut dong modal chi tiet
    NUT_XAC_NHAN_THANHLY,     // Nut xac nhan thanh ly
    NUT_HUY_THANHLY,          // Nut huy thanh ly
    NUT_XAC_NHAN_XOA_BANSAO,  // Nut xac nhan xoa ban sao
    NUT_HUY_XOA_BANSAO,       // Nut huy xoa ban sao

    // Modal them ban sao
    NUT_THEM_BAN_SAO,         // Nut mo modal them ban sao
    INPUT_SO_LUONG_THEM,      // O nhap so luong ban sao them
    NUT_XAC_NHAN_THEM_BS,     // Nut xac nhan them ban sao
    NUT_HUY_THEM_BS,          // Nut huy them ban sao

    // Quan ly doc gia
    NUT_XEM_THEO_MA,          // Xem doc gia theo ma
    NUT_XEM_THEO_TEN,         // Xem doc gia theo ten
    NUT_XEM_QUA_HAN,          // Xem doc gia qua han
    INPUT_HO_DOC_GIA,         // O nhap ho doc gia
    INPUT_TEN_DOC_GIA,        // O nhap ten doc gia
    NUT_THEM_DOC_GIA,         // Nut them doc gia
    NUT_LUU_DOC_GIA,          // Nut luu doc gia
    NUT_HUY_DOC_GIA,          // Nut huy doc gia
    NUT_CHI_TIET_DOC_GIA,     // Nut chi tiet doc gia
    NUT_SUA_DOC_GIA,          // Nut sua doc gia
    NUT_XOA_DOC_GIA,          // Nut xoa doc gia
    NUT_HUY_CHON_DOC_GIA,     // Nut huy chon doc gia
    NUT_TRANG_TRUOC_DG,       // Nut trang truoc doc gia
    NUT_TRANG_SAU_DG,         // Nut trang sau doc gia
    NUT_XAC_NHAN_XOA_DG,      // Nut xac nhan xoa doc gia
    NUT_HUY_XOA_DG,           // Nut huy xoa doc gia
    NUT_DONG_MODAL_DG,        // Nut dong modal doc gia

    // Muon tra sach
    NUT_MT_VAO_MUON_TRA,      // Vao man hinh muon tra
    NUT_MT_VAO_TOP_10,        // Vao top 10 sach

    // Dock trai
    INPUT_MT_TIM_DOC_GIA,     // O tim doc gia dock trai
    INPUT_MT_TIM_SACH,        // O tim sach dock trai
    NUT_MT_TIM_DG_BTN,        // Nut tim doc gia
    NUT_MT_TIM_SACH_BTN,      // Nut tim sach
    NUT_MT_PREV_PAGE,         // Nut trang truoc dock trai
    NUT_MT_NEXT_PAGE,         // Nut trang sau dock trai

    // Dock phai
    NUT_MT_HANHDONG_XEM,      // Nut hanh dong xem
    NUT_MT_HANHDONG_MUON,     // Nut hanh dong muon
    NUT_MT_HANHDONG_TRA,      // Nut hanh dong tra
    NUT_MT_XAC_NHAN_MUON,     // Nut xac nhan muon
    NUT_MT_XAC_NHAN_TRA,      // Nut xac nhan tra
    NUT_MT_HUY_HANH_DONG,     // Nut huy hanh dong

    // Modal xac nhan muon/tra
    NUT_MT_XAC_NHAN_MODAL,    // Nut xac nhan muon/tra
    NUT_MT_HUY_MODAL,         // Nut huy muon/tra
    
    // Additional buttons for borrow/return screen
    NUT_MT_BAO_MAT,           // Nut bao mat sach
    NUT_MT_TAB_QUAHAN,         // Nut tab qua han

    NUT_MT_MODAL_PREV,
    NUT_MT_MODAL_NEXT
};


// Struct va bien toan cuc cho UI

struct UIElement {
    sf::RectangleShape hinhDang; // Hinh chu nhat UI
    sf::Text vanBan;             // Chu chinh
    sf::Text vanBanPhu;          // Chu phu (neu co)
    MaUI id;                     // ID UI
    bool laInput = false;        // Co phai la input khong
};

#define SO_ELEMENT_TOI_DA 100 // So luong UIElement toi da tren 1 man hinh

extern TrangThaiManHinh manHinhHienTai;
extern UIElement cacElement[SO_ELEMENT_TOI_DA];
extern int soLuongElement;
extern bool yeuCauThoat;
extern MaUI inputHoatDong;
extern MaUI elementHover;
extern std::string noiDungThongBao;
extern int loaiThongBao;


// Khai bao ham giao dien

bool KhoiTaoGiaoDienSFML(sf::RenderWindow &window, sf::Font &font, const std::string &fontPath);
void ChayChuongTrinhSFML(sf::RenderWindow &window, sf::Font &font);
void CapNhatThongBaoSFML(const std::string &msg, int loai = 0);
MaUI LayElementTaiToaDo(int mouseX, int mouseY);

#endif // GIAODIEN_SFML_H