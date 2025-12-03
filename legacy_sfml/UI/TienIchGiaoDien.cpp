#include "TienIchGiaoDien.h"
#include <algorithm>                                                    // dung std::min de gioi han gia tri mau

// file chua cac ham helper ve ui co ban (van ban, khung, nut, input)
// muc dich: tai su dung code, giam trung lap

// tao doi tuong sf::text voi cac thuoc tinh co ban
sf::Text TaoVanBan(const sf::Font& font, const std::string& chuoi, unsigned int coChu, sf::Color mauSac) {
    sf::Text text;
    text.setFont(font);                                                 // dat font chu
    text.setString(chuoi);                                              // dat noi dung chuoi
    text.setCharacterSize(coChu);                                       // dat co chu
    text.setFillColor(mauSac);                                          // dat mau chu
    return text;                                                        // tra ve doi tuong van ban
}

// can giua goc (origin) cua van ban theo ca 2 chieu
void CanGiuaGocVanBan(sf::Text& text) {
    sf::FloatRect bounds = text.getLocalBounds();                       // lay kich thuoc hinh chu nhat bao quanh
    text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f); // dat origin vao tam
}

// can giua goc (origin) cua van ban theo chieu ngang, giu top o tren
void CanGiuaGocXVanBan(sf::Text& text) {
    sf::FloatRect bounds = text.getLocalBounds();                       // lay kich thuoc hinh chu nhat
    text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top);      // dat origin x vao giua, y o tren
}

// ve khung hinh chu nhat voi vien va tieu de
void VeKhung(sf::RenderWindow& window, float x, float y, float w, float h, const std::string& tieuDe, const sf::Font& font) {

    // ve hinh chu nhat nen
    sf::RectangleShape khungNen(sf::Vector2f(w, h));                    // tao hinh chu nhat kich thuoc w x h
    khungNen.setPosition(x, y);                                         // dat vi tri goc tren trai
    khungNen.setFillColor(MAU_KHUNG);                                   // dat mau nen khung
    khungNen.setOutlineThickness(1.f);                                  // dat do day vien 1 pixel
    khungNen.setOutlineColor(MAU_VIEN);                                 // dat mau vien
    window.draw(khungNen);                                              // ve khung len cua so

    // ve tieu de neu co
    if (!tieuDe.empty()) {                                              // kiem tra tieu de khong rong
        sf::Text txtTieuDe = TaoVanBan(font, tieuDe, FONT_SIZE_TIEU_DE_KHUNG, MAU_TIEU_DE); // tao van ban tieu de
        txtTieuDe.setPosition(x + PADDING, y + PADDING/2.f);            // dat vi tri tieu de (lech padding)
        window.draw(txtTieuDe);                                         // ve tieu de len cua so
    }
}

// tao doi tuong nut bam (them vao mang toan cuc cacelement)
void TaoNut(const sf::Font& font, MaUI idNut, float x, float y, float rong, float cao, const std::string& nhan, sf::Color mauNen, sf::Color mauChu) {
    // kiem tra con cho trong mang khong
    if (soLuongElement < SO_ELEMENT_TOI_DA) {
        UIElement& nut = cacElement[soLuongElement];                    // lay phan tu tiep theo (tham chieu)
        nut.id = idNut;                                                 // gan id cho nut
        nut.laInput = false;                                            // danh dau la nut (khong phai input)

        // cau hinh hinh dang (hinh chu nhat)
        nut.hinhDang.setSize(sf::Vector2f(rong, cao));                  // dat kich thuoc nut
        nut.hinhDang.setPosition(x, y);                                 // dat vi tri nut

        // hieu ung highlight khi hover chuot
        if (elementHover == idNut) {                                    // neu dang hover nut nay
            int r = std::min(255, mauNen.r + 20);                       // tang mau do (gioi han 255)
            int g = std::min(255, mauNen.g + 20);                       // tang mau xanh la (gioi han 255)
            int b = std::min(255, mauNen.b + 20);                       // tang mau xanh duong (gioi han 255)
            nut.hinhDang.setFillColor(sf::Color(r, g, b));              // dat mau highlight
        } else {
            nut.hinhDang.setFillColor(mauNen);                          // dat mau nen binh thuong
        }
        nut.hinhDang.setOutlineThickness(1.f);                          // dat vien 1 pixel
        nut.hinhDang.setOutlineColor(MAU_VIEN);                         // dat mau vien

        // tao van ban (nhan) tren nut
        nut.vanBan = TaoVanBan(font, nhan, FONT_SIZE_BINH_THUONG, mauChu); // tao van ban
        CanGiuaGocVanBan(nut.vanBan);                                   // can giua origin
        nut.vanBan.setPosition(x + rong / 2.0f, y + cao / 2.0f);        // dat van ban vao tam nut

        soLuongElement++;                                               // tang so luong element
    }
}

// tao doi tuong o nhap lieu (them vao mang toan cuc cacelement)
void TaoInput(const sf::Font& font, MaUI idInput, float x, float y, float rong, float cao, const std::string& label, const std::string& giaTri, const std::string& goiY, bool readOnly) {
    if (soLuongElement < SO_ELEMENT_TOI_DA) {                           // kiem tra con cho
        UIElement& input = cacElement[soLuongElement];                  // lay phan tu tiep theo
        input.id = idInput;                                             // gan id
        input.laInput = true;                                           // danh dau la input

        // xu ly label (nhan) ben trai (neu co)
        const bool coLabel = !label.empty();                            // kiem tra co label khong
        float labelWidth = coLabel ? 100.f : 0.f;                       // chieu rong label
        float khoangCachLabel = coLabel ? 10.f : 0.f;                   // khoang cach giua label va input

        // tao van ban label
        input.vanBan = TaoVanBan(font, label, FONT_SIZE_BINH_THUONG, MAU_CHU); // tao van ban label
        if (coLabel) {                                                  // neu co label
            input.vanBan.setPosition(x, y + (cao - input.vanBan.getCharacterSize()) / 2.f); // can giua y
        }

        // tinh vi tri x cua hop input (sau label)
        float inputX = x + labelWidth + khoangCachLabel;                // vi tri x hop input

        // cau hinh hinh dang hop input
        input.hinhDang.setSize(sf::Vector2f(rong, cao));                // dat kich thuoc hop
        input.hinhDang.setPosition(inputX, y);                          // dat vi tri hop
        input.hinhDang.setFillColor(readOnly ? MAU_KHUNG : MAU_INPUT_NEN); // mau nen (mo neu readonly)
        input.hinhDang.setOutlineThickness(1.f);                        // vien 1 pixel

        // logic mau vien (doi voi input active hoac co loi)
        sf::Color borderColor = MAU_INPUT_BORDER;                       // mau vien mac dinh
        bool coLoiOInputNay = (loaiThongBao == 1 && inputHoatDong == idInput); // co loi tai input nay khong

        if (inputHoatDong == idInput && !readOnly) {                    // neu input dang active va khong readonly
            borderColor = coLoiOInputNay ? MAU_LOI : MAU_INPUT_ACTIVE_BORDER; // do neu loi, xanh neu binh thuong
        } else if (readOnly) {                                          // neu readonly
            borderColor = MAU_VIEN;                                     // mau mo
        }
        input.hinhDang.setOutlineColor(borderColor);                    // dat mau vien

        // logic hien thi noi dung ben trong hop (gia tri hoac goi y)
        std::string displayText = giaTri;                               // mac dinh hien thi gia tri
        sf::Color displayColor = readOnly ? MAU_VIEN : MAU_INPUT_CHU;   // mau chu (mo neu readonly)

        // hien thi goi y neu hop rong va khong active
        if (giaTri.empty() && inputHoatDong != idInput && !goiY.empty()) { // neu rong va khong active
            displayText = goiY;                                         // hien thi goi y
            displayColor = MAU_VIEN;                                    // mau mo (placeholder)
        }

        // them dau nhay "_" neu input dang active
        if (inputHoatDong == idInput && !readOnly) displayText += "_";  // them cursor

        // tao van ban noi dung ben trong hop
        input.vanBanPhu = TaoVanBan(font, displayText, FONT_SIZE_BINH_THUONG, displayColor); // tao van ban
        input.vanBanPhu.setPosition(inputX + 5.f, y + (cao - input.vanBanPhu.getCharacterSize()) / 2.f); // dat vi tri (padding trai 5px)

        soLuongElement++;                                               // tang so luong element
    }
}

// ham cat text thanh nhieu dong neu vuot qua maxwidth (word wrap)
std::string WordWrapText(const sf::Font& font, const std::string& text, unsigned int fontSize, float maxWidth) {
    if (text.empty() || maxWidth <= 0) {                                // kiem tra input hop le
        return text;                                                    // tra ve nguyen neu khong hop le
    }

    // tao text tam de do chieu rong
    sf::Text tempText;                                                  // text de tinh toan chieu rong
    tempText.setFont(font);                                             // dat font
    tempText.setCharacterSize(fontSize);                                // dat co chu
    
    std::string result = "";                                            // ket qua sau khi wrap
    std::string currentLine = "";                                        // dong hien tai dang xu ly
    std::string currentWord = "";                                        // tu hien tai dang xu ly
    
    for (size_t i = 0; i < text.length(); ++i) {                        // duyet tung ky tu
        char c = text[i];                                               // ky tu hien tai
        
        // xu ly ky tu xuong dong
        if (c == '\n') {                                                 // neu gap \n
            if (!currentWord.empty()) {                                 // them tu hien tai vao dong
                if (!currentLine.empty()) currentLine += " ";            // them khoang trang
                currentLine += currentWord;                             // them tu
                currentWord = "";                                        // reset tu
            }
            if (!currentLine.empty()) {                                 // them dong vao ket qua
                if (!result.empty()) result += "\n";                     // them \n
                result += currentLine;                                  // them dong
                currentLine = "";                                        // reset dong
            } else {                                                    // dong trong
                if (!result.empty()) result += "\n";                     // them \n
                result += " ";                                           // them dong trong (space)
            }
            continue;                                                   // tiep tuc vong lap
        }
        
        // xu ly khoang trang (ket thuc tu)
        if (c == ' ') {                                                  // neu gap khoang trang
            std::string testLine = currentLine;                         // tao dong test
            if (!testLine.empty() && !currentWord.empty()) testLine += " "; // them khoang trang
            testLine += currentWord;                                    // them tu hien tai
            tempText.setString(testLine);                               // dat string de tinh chieu rong
            
            if (tempText.getLocalBounds().width > maxWidth && !currentLine.empty()) { // neu qua rong
                if (!result.empty()) result += "\n";                     // xuong dong
                result += currentLine;                                  // them dong cu
                currentLine = currentWord;                              // bat dau dong moi voi tu hien tai
                currentWord = "";                                        // reset tu
            } else {                                                    // neu con vua
                if (!currentLine.empty()) currentLine += " ";            // them khoang trang
                currentLine += currentWord;                             // them tu vao dong
                currentWord = "";                                        // reset tu
            }
        } else {                                                        // ky tu binh thuong
            currentWord += c;                                           // them vao tu hien tai
        }
    }
    
    // xu ly tu cuoi cung (sau vong lap)
    if (!currentWord.empty()) {                                         // neu con tu chua xu ly
        std::string testLine = currentLine;                             // tao dong test
        if (!testLine.empty()) testLine += " ";                          // them khoang trang
        testLine += currentWord;                                        // them tu cuoi
        tempText.setString(testLine);                                   // tinh chieu rong
        
        if (tempText.getLocalBounds().width > maxWidth && !currentLine.empty()) { // neu qua rong
            if (!result.empty()) result += "\n";                         // xuong dong
            result += currentLine;                                      // them dong cu
            currentLine = currentWord;                                  // bat dau dong moi
        } else {                                                        // neu con vua
            if (!currentLine.empty()) currentLine += " ";                // them khoang trang
            currentLine += currentWord;                                 // them tu vao dong
        }
    }
    
    // them dong cuoi cung
    if (!currentLine.empty()) {                                         // neu con dong chua them
        if (!result.empty()) result += "\n";                             // them \n
        result += currentLine;                                          // them dong cuoi
    }
    
    return result.empty() ? text : result;                              // tra ve ket qua hoac text goc
}

// xu ly nhap chuoi tu ban phim
void XuLyNhapChuoi(sf::Event event, std::string& str, size_t maxLen) {
    if (event.text.unicode < 128) {                                     // chi nhan ky tu ASCII
        char c = static_cast<char>(event.text.unicode);
        
        if (c == '\b' && !str.empty()) {                                // backspace
            str.pop_back();
        } else if (c >= 32 && c != 127 && str.length() < maxLen) {      // ky tu hien thi
            str += c;
        }
    }
}