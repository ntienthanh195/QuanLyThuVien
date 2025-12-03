#include "ManHinhThongTin.h"
#include "GiaoDienSFML.h"
#include "TienIchGiaoDien.h"

void VeManHinhThongTinSFML(sf::RenderWindow &window, const sf::Font &font){
    float centerX = CHIEU_RONG / 2.0f;                                   // tinh toa do giua man hinh
    float currentY = 100.f;                                              // y bat dau ve

    // ve tieu de truong
    sf::Text tieuDe = TaoVanBan(font, "Hoc vien Cong nghe Buu Chinh Vien Thong Co So Tai TPHCM", FONT_SIZE_TIEU_DE_LON, MAU_TIEU_DE);
    CanGiuaGocXVanBan(tieuDe);                                          // can giua theo chieu ngang
    tieuDe.setPosition(centerX, currentY);                               // dat vi tri tieu de
    window.draw(tieuDe);                                                // ve len cua so
    currentY += 100.f;                                                  // xuong hang

    float labelX = centerX - 400.f;                                      // x cho nhan (ben trai)
    float valueX = centerX - 150.f;                                      // x cho gia tri (ben phai)
    unsigned int coChuInfo = 26;                                         // co chu cho thong tin

    // ve thong tin mon hoc
    sf::Text labelMonHoc = TaoVanBan(font, "* Mon hoc:", coChuInfo, MAU_CHU);
    labelMonHoc.setPosition(labelX, currentY);                          // dat vi tri nhan
    sf::Text valueMonHoc = TaoVanBan(font, "Cau Truc Du Lieu & Giai Thuat", coChuInfo, MAU_NHAN);
    valueMonHoc.setPosition(valueX, currentY);                          // dat vi tri gia tri
    window.draw(labelMonHoc);                                           // ve nhan
    window.draw(valueMonHoc);                                           // ve gia tri
    currentY += 50.f;                                                   // xuong hang

    // ve thong tin giang vien
    sf::Text labelGV = TaoVanBan(font, "* Giang vien:", coChuInfo, MAU_CHU);
    labelGV.setPosition(labelX, currentY);                              // dat vi tri nhan
    sf::Text valueGV = TaoVanBan(font, "Luu Nguyen Ky Thu", coChuInfo, MAU_NHAN);
    valueGV.setPosition(valueX, currentY);                              // dat vi tri gia tri
    window.draw(labelGV);                                               // ve nhan
    window.draw(valueGV);                                               // ve gia tri
    currentY += 50.f;                                                   // xuong hang

    // ve thong tin do an
    sf::Text labelDA = TaoVanBan(font, "* Do An:", coChuInfo, MAU_CHU);
    labelDA.setPosition(labelX, currentY);                              // dat vi tri nhan
    sf::Text valueDA = TaoVanBan(font, "Quan Ly Thu Vien", coChuInfo, MAU_NHAN);
    valueDA.setPosition(valueX, currentY);                              // dat vi tri gia tri
    window.draw(labelDA);                                               // ve nhan
    window.draw(valueDA);                                               // ve gia tri
    currentY += 100.f;                                                  // xuong hang (khoang lon)

    // ve thong tin sinh vien
    sf::Text labelSV = TaoVanBan(font, "* Sinh vien:", coChuInfo, MAU_CHU);
    labelSV.setPosition(labelX, currentY);                              // dat vi tri nhan
    sf::Text valueSV1 = TaoVanBan(font, "1. Cham Roch Thi - N24DECE041 - E24CQCE01-N", coChuInfo, MAU_NHAN);
    valueSV1.setPosition(valueX, currentY);                             // dat vi tri sv 1
    currentY += 40.f;                                                   // xuong hang (hep)
    sf::Text valueSV2 = TaoVanBan(font, "2. Tran Gia Binh - N24DECE05 - E24CQCE01-N", coChuInfo, MAU_NHAN);
    valueSV2.setPosition(valueX, currentY);                             // dat vi tri sv 2
    window.draw(labelSV);                                               // ve nhan
    window.draw(valueSV1);                                              // ve sv 1
    window.draw(valueSV2);                                              // ve sv 2
    currentY += 120.f;                                                  // xuong hang (khoang lon)

    sf::Text nam = TaoVanBan(font, "-- 2025 --", 20, MAU_CHU);          // tao text nam
    CanGiuaGocXVanBan(nam);                                             // can giua theo chieu ngang
    nam.setPosition(centerX, currentY);                                 // dat vi tri nam
    window.draw(nam);                                                   // ve nam
    TaoNut(font, NUT_BACK, 20.f, 20.f, 120.f, 40.f, "< MENU", MAU_NUT_BACK, MAU_CHU_NUT); // nut quay ve menu
}

// xu ly su kien
void XuLySuKienThongTin(sf::RenderWindow &window, sf::Event event) {
    (void)window;                                                       // danh dau bien chua dung

    if (event.type == sf::Event::MouseButtonPressed) {                  // xu ly click chuot
        if (event.mouseButton.button == sf::Mouse::Left) {
            MaUI elementNhan = LayElementTaiToaDo(event.mouseButton.x, event.mouseButton.y); // lay element duoc click
            if (elementNhan == NUT_BACK) {                              // neu click nut back
                manHinhHienTai = MENU_CHINH;                            // chuyen ve menu
            }
        }
    }
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) { // xu ly phim escape
        manHinhHienTai = MENU_CHINH;                                    // chuyen ve menu
    }
}