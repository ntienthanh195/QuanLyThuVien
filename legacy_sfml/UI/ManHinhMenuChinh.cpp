#include "ManHinhMenuChinh.h"
#include "GiaoDienSFML.h"
#include "TienIchGiaoDien.h"
#include "ManHinhQuanLySach.h" 
#include "TrangThaiManHinhDocGia.h"  
#include "ManHinhQuanLyDocGia.h"
#include "TrangThaiManHinhMuonTra.h"

// Ve Man Hinh 
void VeMenuChinhSFML(sf::RenderWindow& window, const sf::Font& font) {
    sf::Text tieuDe = TaoVanBan(font, "QUAN LY THU VIEN", FONT_SIZE_TIEU_DE_MAIN, MAU_TIEU_DE);
    CanGiuaGocXVanBan(tieuDe);
    tieuDe.setPosition(CHIEU_RONG / 2.0f, 130.f);
    window.draw(tieuDe);

    float nutRong = 350.f;
    float nutCao = 60.f;
    float khoangCachNut = 25.f;
    float x = (CHIEU_RONG - nutRong) / 2.0f;
    float y = 250.f;

    TaoNut(font, NUT_QUAN_LY_DAU_SACH, x, y, nutRong, nutCao, "1. QUAN LY DAU SACH", MAU_NEN_NUT, MAU_CHU_NUT);
    y += nutCao + khoangCachNut;
    TaoNut(font, NUT_QUAN_LY_DOC_GIA, x, y, nutRong, nutCao, "2. QUAN LY DOC GIA", MAU_NEN_NUT, MAU_CHU_NUT);
    y += nutCao + khoangCachNut;
    TaoNut(font, NUT_MUON_TRA_SACH, x, y, nutRong, nutCao, "3. MUON TRA SACH", MAU_NEN_NUT, MAU_CHU_NUT);
    y += nutCao + khoangCachNut;
    TaoNut(font, NUT_THONG_TIN, x, y, nutRong, nutCao, "4. THONG TIN", MAU_NEN_NUT, MAU_CHU_NUT);
    y += nutCao + khoangCachNut;
    TaoNut(font, NUT_THOAT, x, y, nutRong, nutCao, "5. THOAT", MAU_NEN_NUT, MAU_CHU_NUT);
}

// Xu Ly Su Kien
void XuLySuKienMenuChinh(sf::RenderWindow& window, sf::Event event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            MaUI elementNhan = LayElementTaiToaDo(event.mouseButton.x, event.mouseButton.y);
            
            switch (elementNhan) {
                case NUT_QUAN_LY_DAU_SACH:
                    manHinhHienTai = QUAN_LY_SACH;
                    KhoiTaoManHinhSach(); // Reset lai man hinh sach
                    CapNhatThongBaoSFML("Chuyen den man hinh Quan Ly Sach.", 0);
                    break;
                
                case NUT_QUAN_LY_DOC_GIA:
                    manHinhHienTai = QUAN_LY_DOC_GIA;
                    KhoiTaoManHinhDocGia(); // Goi ham khoi tao man hinh Doc Gia
                    CapNhatThongBaoSFML("Chuyen den man hinh Quan Ly Doc Gia.", 0);
                    break;
                
                case NUT_MUON_TRA_SACH:
                    manHinhHienTai = MUON_TRA_SACH;
                    KhoiTaoManHinhMuonTra(); // Goi ham khoi tao man hinh Muon Tra
                    CapNhatThongBaoSFML("Chuyen den man hinh Muon Tra Sach.", 0);
                    break;

                case NUT_THONG_TIN:
                    manHinhHienTai = MAN_HINH_THONG_TIN;
                    CapNhatThongBaoSFML("Chuyen den man hinh Thong Tin.", 0);
                    break;
                case NUT_THOAT:
                    window.close();
                    yeuCauThoat = true;
                    break;

                default:
                    break;
            }
        }
    }
    
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Num1: case sf::Keyboard::Numpad1:
                manHinhHienTai = QUAN_LY_SACH;
                KhoiTaoManHinhSach();
                CapNhatThongBaoSFML("Chuyen den man hinh Quan Ly Sach.", 0);
                break;
                
            case sf::Keyboard::Num2: case sf::Keyboard::Numpad2:
                manHinhHienTai = QUAN_LY_DOC_GIA;
                KhoiTaoManHinhDocGia();
                CapNhatThongBaoSFML("Chuyen den man hinh Quan Ly Doc Gia.", 0);
                break;

            case sf::Keyboard::Num3: case sf::Keyboard::Numpad3:
                manHinhHienTai = MUON_TRA_SACH;
                KhoiTaoManHinhMuonTra();
                CapNhatThongBaoSFML("Chuyen den man hinh Muon Tra Sach.", 0);
                break;

            case sf::Keyboard::Num4: case sf::Keyboard::Numpad4:
                manHinhHienTai = MAN_HINH_THONG_TIN;
                CapNhatThongBaoSFML("Chuyen den man hinh Thong Tin.", 0);
                break;
                
            case sf::Keyboard::Num5: case sf::Keyboard::Numpad5:
                window.close();
                yeuCauThoat = true;
                break;
                
            default:
                break;
        }
    }
}