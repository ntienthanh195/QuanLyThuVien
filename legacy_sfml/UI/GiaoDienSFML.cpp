#include "GiaoDienSFML.h"
#include "TienIchGiaoDien.h"
#include "ManHinhMenuChinh.h"
#include "ManHinhThongTin.h"
#include "ManHinhQuanLySach.h"
//cua binh
#include "GiaoDienQuanLyDocGia.h"
#include "GiaoDienMuonTra.h"
#include <iostream>

// Khai bao cac bien toan cuc
TrangThaiManHinh manHinhHienTai = MENU_CHINH;           // man hinh hien tai (mac dinh: menu)
UIElement cacElement[SO_ELEMENT_TOI_DA];                // mang luu cac element ui (nut, input)
int soLuongElement = 0;                                 // so luong element dang su dung
bool yeuCauThoat = false;                               // co yeu cau thoat chuong trinh
MaUI inputHoatDong = KHONG_XAC_DINH;                    // input dang duoc focus
MaUI elementHover = KHONG_XAC_DINH;                     // element dang hover chuot
std::string noiDungThongBao = "Chao mung!";             // noi dung thong bao hien thi
int loaiThongBao = 0;                                   // loai thong bao (0: binh thuong, 1: loi, 2: thanh cong)

bool KhoiTaoGiaoDienSFML(sf::RenderWindow &window, sf::Font &font, const std::string &fontPath) {
    (void)window;                                                       // danh dau bien chua dung truc tiep
    std::cout << "Dang khoi tao cua so SFML..." << std::endl;
    if (!font.loadFromFile(fontPath)) {                                 // tai font tu file
        std::cerr << "Loi: Khong the tai file font: " << fontPath << std::endl;
        return false;                                                   // bao loi neu khong load duoc
    }
    std::cout << "Khoi tao SFML thanh cong!" << std::endl;
    KhoiTaoManHinhSach();                                               // khoi tao du lieu man hinh sach
    return true;                                                        // khoi tao thanh cong
}

// ham ve man hinh hien tai
static void VeManHinhHienTai(sf::RenderWindow &window, const sf::Font &font) {
    window.clear(MAU_NEN);                                              // xoa man hinh (mau nen)
    soLuongElement = 0;                                                 // reset so luong element

switch (manHinhHienTai) {
    case MENU_CHINH:
        VeMenuChinhSFML(window, font);
        break;
    case MAN_HINH_THONG_TIN:
        VeManHinhThongTinSFML(window, font);
        break;
    case QUAN_LY_SACH:
        VeManHinhQuanLySach(window, font);
        break;

    // [THÊM 2 CASE NÀY VÀO]
    case QUAN_LY_DOC_GIA:
        VeManHinhQuanLyDocGia(window, font); // Tên hàm phải khớp
        break;
    case MUON_TRA_SACH:
        VeManHinhMuonTra(window, font); // Tên hàm phải khớp
        break;

    // (Thêm default để tắt cảnh báo hoàn toàn)
    default:
        VeMenuChinhSFML(window, font); // Quay về menu nếu không rõ
        break;
}
    switch (manHinhHienTai) {
        case MENU_CHINH:
            VeMenuChinhSFML(window, font);
            break;
        case MAN_HINH_THONG_TIN:
            VeManHinhThongTinSFML(window, font);
            break;
        case QUAN_LY_SACH:
            VeManHinhQuanLySach(window, font);
            break;
        case QUAN_LY_DOC_GIA:
            VeManHinhQuanLyDocGia(window, font);
            break;
        case MUON_TRA_SACH:
            VeManHinhMuonTra(window, font);
            break;
        default:
            VeMenuChinhSFML(window, font);
            break;
    }

    // ve tat ca cac element da tao (nut, input)
    for (int i = 0; i < soLuongElement; ++i) {
        window.draw(cacElement[i].hinhDang);                            // ve hinh dang (hinh chu nhat)
        window.draw(cacElement[i].vanBan);                              // ve van ban chinh
        if (cacElement[i].laInput) {                                    // neu la input
            window.draw(cacElement[i].vanBanPhu);                       // ve van ban phu (noi dung input)
        }
    }
}

// ham xu ly su kien tong quat
static void XuLySuKienSFML(sf::RenderWindow &window) {
    sf::Event event;

    // cap nhat hover (element dang di chuot qua)
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);             // lay vi tri chuot
    MaUI idHover = LayElementTaiToaDo(mousePos.x, mousePos.y);          // tim element tai vi tri chuot
    bool isHoveredElementInput = false;                                 // co phai input khong
    for (int i = 0; i < soLuongElement; ++i) {
        if (cacElement[i].id == idHover && cacElement[i].laInput) {     // neu la input
            isHoveredElementInput = true;                               // danh dau
            break;
        }
    }
    if (!isHoveredElementInput)                                         // neu khong phai input
        elementHover = idHover;                                         // cap nhat hover
    else
        elementHover = KHONG_XAC_DINH;                                  // khong hien hieu ung hover cho input

    while (window.pollEvent(event)) {                                   // lap qua cac su kien
        if (event.type == sf::Event::Closed) {                          // neu dong cua so
            window.close();                                             // dong cua so
            yeuCauThoat = true;                                         // dat co thoat
        }

        // dieu phoi su kien den dung man hinh
        switch (manHinhHienTai) {
            case MENU_CHINH:
                XuLySuKienMenuChinh(window, event);
                break;
            case MAN_HINH_THONG_TIN:
                XuLySuKienThongTin(window, event);
                break;
            case QUAN_LY_SACH:
                XuLySuKienManHinhSach(window, event);
                break;
            case QUAN_LY_DOC_GIA:
                XuLySuKienManHinhDocGia(window, event);
                break;
            case MUON_TRA_SACH:
                XuLySuKienManHinhMuonTra(window, event);
                break;
            default:
                break;
        }
        // xu ly escape toan cuc (neu man hinh con khong xu ly)
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            if (manHinhHienTai == MENU_CHINH) {                         // neu dang o menu
                window.close();                                         // dong cua so
                yeuCauThoat = true;                                     // dat co thoat
            }
            // cac man hinh khac tu xu ly escape trong ham xu ly rieng
        }
    }
}

void ChayChuongTrinhSFML(sf::RenderWindow &window, sf::Font &font) {
    std::cout << "[DEBUG] Bat dau vong lap SFML..." << std::endl;
    yeuCauThoat = false;                                                // reset co thoat

    while (window.isOpen() && !yeuCauThoat) {                           // vong lap chinh (60 FPS mac dinh)
        XuLySuKienSFML(window);                                         // xu ly su kien (chuot, phim, ...)
        VeManHinhHienTai(window, font);                                 // ve man hinh hien tai
        window.display();                                               // hien thi len man hinh
    }

    std::cout << "[DEBUG] Ket thuc vong lap SFML." << std::endl;
}

void CapNhatThongBaoSFML(const std::string &msg, int loai) {
    noiDungThongBao = msg;                                              // cap nhat noi dung thong bao
    loaiThongBao = loai;                                                // cap nhat loai (0: normal, 1: error, 2: success)
}

MaUI LayElementTaiToaDo(int mouseX, int mouseY) {
    // Kiem tra cac nut/input truoc (uu tien cao nhat)
    for (int i = 0; i < soLuongElement; ++i) {
        if (cacElement[i].hinhDang.getGlobalBounds().contains(static_cast<float>(mouseX), static_cast<float>(mouseY))) {
            return cacElement[i].id;
        }
    }

    // Kiem tra vung bang danh sach (tuy theo man hinh)
    float tableTop, tableBottom;
    if (manHinhHienTai == QUAN_LY_SACH) {
        tableTop = BANG_Y + 90 + 30;
        tableBottom = CHIEU_CAO - (PADDING * 5.5f) - (NUT_CAO * 2);
        if (mouseX >= BANG_X + PADDING && mouseX < BANG_X + BANG_RONG - PADDING && 
            mouseY >= tableTop && mouseY < tableBottom) {
            return HANG_SACH;
        }
    } else if (manHinhHienTai == QUAN_LY_DOC_GIA) {
        tableTop = BANG_Y + 50.f + 40.f + 35.f;
        tableBottom = CHIEU_CAO - (PADDING * 5.5f) - (NUT_CAO * 2);
        if (mouseX >= BANG_X + PADDING && mouseX < BANG_X + BANG_RONG - PADDING && 
            mouseY >= tableTop && mouseY < tableBottom) {
            return HANG_SACH;
        }
    } else if (manHinhHienTai == MUON_TRA_SACH) {
        if (mouseY > BANG_Y + 150.f && mouseY < CHIEU_CAO - 50.f && 
            mouseX >= BANG_X && mouseX <= BANG_X + BANG_RONG) {
            return HANG_SACH;
        }
    }

    return KHONG_XAC_DINH;
}
