#include "ManHinhQuanLySach.h"
#include "GiaoDienSFML.h"
#include "TienIchGiaoDien.h"
#include "TrangThaiManHinhSach.h"
#include "QuanLySach.h"
#include "KiemTraDuLieu.h"
#include "XuLyChuoi.h"
#include "Constants.h"
#include "NgayThang.h"
#include <iostream>
#include <algorithm>
#include <sstream>

static SachState state;
static void VeBangSach(sf::RenderWindow &window, const sf::Font &font, const SachState& currentState);
static void VeFormThemSach(sf::RenderWindow &window, const sf::Font &font, const SachState& currentState);
static void VeKhungThongBaoSFML(sf::RenderWindow &window, const sf::Font &font, const SachState& currentState);
static void VeModalChiTietBanSao(sf::RenderWindow &window, const sf::Font &font, SachState& currentState);
static void VeModalThemBanSao(sf::RenderWindow &window, const sf::Font &font, const SachState& currentState);
static void VeDanhSachTheoTheLoai(sf::RenderWindow& window, const sf::Font& font, SachState& currentState);
static void CapNhatDuLieuXemTheoTheLoai(SachState& currentState);

static void XuLyTextInput(sf::Event event, SachState& currentState);
static void ThucHienTimKiemNoiBo(SachState& currentState);
static void ThucHienThemHoacSuaSachSFML(SachState& currentState);
static void ThucHienXoaSachSFML(SachState& currentState);
static void ThucHienThanhLySach(SachState& currentState, const std::string& maSach);
static void ThucHienXoaBanSao(SachState& currentState, const std::string& maSach);
static void ThucHienThemBanSao(SachState& currentState);

static void XoaFormNhapLieuSFML(SachState& currentState);
static void DienFormVoiSachDuocChon(SachState& currentState);
static void CapNhatPhanTrangSFML(SachState& currentState);
static void ResetVaTaiLaiDuLieu(SachState& currentState, bool xoaForm = true, bool xoaISBN = false);

static inline std::string CatChuoiVoiDauCham(const std::string& str, size_t maxLen) {
    return (str.length() > maxLen) ? str.substr(0, maxLen - 3) + "..." : str;
}

static std::string CatChuoiTheoRong(const sf::Font& font, const std::string& src, unsigned int coChu, float rongToiDa) {
    sf::Text tmp = TaoVanBan(font, src, coChu, MAU_CHU);
    if (tmp.getLocalBounds().width <= rongToiDa)
        return src;
    
    // Binary search
    int left = 0, right = static_cast<int>(src.size()) - 3;
    int best = 0;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        tmp.setString(src.substr(0, mid) + "...");
        if (tmp.getLocalBounds().width <= rongToiDa) {
            best = mid;
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return best > 0 ? src.substr(0, best) + "..." : "...";
}

static inline bool KiemTraChiChuaSo(const std::string& str, std::string& loiRa, const std::string& tenTruong) {
    for(char c : str) {
        if (!isdigit(c)) {
            loiRa = "Loi: " + tenTruong + " chi duoc chua chu so (0-9)!\nKy tu '" + std::string(1, c) + "' khong hop le.";
            return false;
        }
    }
    return true;
}

#define VALIDATE_AND_RETURN(condition, errorMsg, inputType) \
    if (condition) { \
        CapNhatThongBaoSFML(errorMsg, 1); \
        inputHoatDong = inputType; \
        return; \
    }

static inline void KhoiTaoStack(UndoStack *s) {
    s->top = -1;
}
static inline bool KiemTraStackRong(UndoStack *s) {
    return s->top == -1;
}
static inline bool KiemTraStackDay(UndoStack *s) {
    return s->top == MAX_UNDO_STEPS - 1;
}

static void PushStack(UndoStack *s, const std::string& trangThai) {
    if (!KiemTraStackDay(s)) {
        // Tranh push trang thai trung lap
        if (KiemTraStackRong(s) || s->history[s->top].data != trangThai) {
            s->top++;
            s->history[s->top].data = trangThai;
        }
    }
    // Neu stack day, bo qua trang thai cu hon (khong lam gi)
}

// Ham lay va xoa trang thai khoi dinh stack
static std::string PopStack(UndoStack *s) {
    if (!KiemTraStackRong(s)) {
        std::string data = s->history[s->top].data;
        s->top--;    // Giam dinh stack
        return data; // Tra ve trang thai da lay
    }
    return ""; // Tra ve chuoi rong neu stack rong
}

// DINH NGHIA HAM

static void VeBangSach(sf::RenderWindow &window, const sf::Font &font, const SachState& currentState){
    float headerY = BANG_Y + 50.f;
    float contentY = headerY + 40.f;
    // Tinh day bang, tru khong gian cho nut
    float tableBottom = CHIEU_CAO - (PADDING * 5.5f) - (NUT_CAO * 2);

    VeKhung(window, BANG_X, headerY, BANG_RONG, tableBottom - headerY, "DANH SACH DAU SACH (Double-click de chon)", font);

    // Phan chia ty le cot (tong = 1.0)
    float colWidths[] = {0.05f, 0.16f, 0.30f, 0.07f, 0.18f, 0.07f, 0.11f, 0.06f};
    float colX[9];
    colX[0] = BANG_X + PADDING;
    for (int i = 0; i < 8; ++i) {
        colX[i + 1] = colX[i] + colWidths[i] * (BANG_RONG - PADDING);
        if (i < 7) {
            sf::RectangleShape line(sf::Vector2f(1.f, tableBottom - contentY));
            line.setPosition(colX[i + 1] - PADDING / 2.f, contentY);
            line.setFillColor(MAU_BANG_BORDER);
            window.draw(line);
        }
    }

    sf::Text headerText;
    headerText.setFont(font);
    headerText.setCharacterSize(FONT_SIZE_BINH_THUONG);
    headerText.setFillColor(MAU_NHAN);

    std::string headers[] = {"STT", "ISBN", "Ten Sach", "Trang", "Tac Gia", "NXB", "The Loai", "SL"};

    float headerBoxHeight = 30.f;
    sf::RectangleShape headerBg(sf::Vector2f(BANG_RONG - PADDING * 0.8, headerBoxHeight));
    headerBg.setPosition(BANG_X + PADDING * 0.4, contentY);
    headerBg.setFillColor(MAU_BANG_HEADER);
    window.draw(headerBg);

    for (int i = 0; i < 8; ++i) {
        headerText.setString(headers[i]);
        headerText.setPosition(colX[i], contentY + (headerBoxHeight - headerText.getCharacterSize()) / 2.f);
        window.draw(headerText);
    }
    sf::RectangleShape headerLine(sf::Vector2f(BANG_RONG - 2 * PADDING, 1.f));
    headerLine.setPosition(BANG_X + PADDING, contentY + 30.f);
    headerLine.setFillColor(MAU_BANG_BORDER);
    window.draw(headerLine);

    float currentY = contentY + 35.f;
    int startIndex = (currentState.trangHienTai - 1) * SACH_MOI_TRANG;
    int endIndex = (startIndex + SACH_MOI_TRANG < currentState.soLuongKetQuaTimKiem)
                       ? (startIndex + SACH_MOI_TRANG)
                       : currentState.soLuongKetQuaTimKiem;

    float rowHeight = 30.f;

    sf::Text dataText;
    dataText.setFont(font);
    dataText.setCharacterSize(FONT_SIZE_BINH_THUONG);

    // TOI UU: Cache highlight shape de tai su dung
    sf::RectangleShape highlight(sf::Vector2f(BANG_RONG - PADDING * 0.8, rowHeight));
    highlight.setFillColor(MAU_NHAN);
    
    for (int i = startIndex; i < endIndex && currentY <= tableBottom - PADDING; ++i) {
        PTRDS d = currentState.ketQuaTimKiem[i].sach;
        if (!d) continue;

        bool isSelected = (d->ISBN == currentState.isbnSachDuocChon);
        if (isSelected) {
            highlight.setPosition(BANG_X + PADDING * 0.4, currentY - 2.f);
            window.draw(highlight);
        }
        dataText.setFillColor(isSelected ? sf::Color::Black : MAU_CHU);

        const float textY = currentY + (rowHeight - dataText.getCharacterSize()) / 2.f;
        
        // Ve tat ca cot trong mot lan
        const std::string dataCols[] = {
            std::to_string(i + 1),
            d->ISBN,
            CatChuoiVoiDauCham(d->tenSach, 25),
            std::to_string(d->soTrang),
            CatChuoiVoiDauCham(d->tacGia, 15),
            std::to_string(d->namXuatBan),
            CatChuoiVoiDauCham(d->theLoai, 8),
            std::to_string(d->tongBanSao)
        };
        
        for (int col = 0; col < 8; ++col) {
            dataText.setString(dataCols[col]);
            dataText.setPosition(colX[col], textY);
            window.draw(dataText);
        }

        currentY += rowHeight;
    }

    float buttonY = tableBottom + PADDING;
    float pageY = buttonY + NUT_CAO + PADDING / 2;

    if (!currentState.isbnSachDuocChon.empty()) {
        float btnWidth = (BANG_RONG - (PADDING * 4)) / 5.f; // Chia deu 5 nut
        float btnSpacing = PADDING;
        float btnX = BANG_X;

        TaoNut(font, NUT_THEM_BAN_SAO, btnX, buttonY, btnWidth, NUT_CAO, "THEM BAN SAO", MAU_NHAN, MAU_CHU_NUT);
        btnX += btnWidth + btnSpacing;
        TaoNut(font, NUT_CHI_TIET_SACH, btnX, buttonY, btnWidth, NUT_CAO, "CHI TIET", MAU_NEN_NUT, MAU_CHU_NUT); // Doi mau
        btnX += btnWidth + btnSpacing;
        TaoNut(font, NUT_SUA, btnX, buttonY, btnWidth, NUT_CAO, "SUA", MAU_NEN_NUT, MAU_CHU_NUT);
        btnX += btnWidth + btnSpacing;
        TaoNut(font, NUT_XOA, btnX, buttonY, btnWidth, NUT_CAO, "XOA", MAU_LOI, MAU_CHU_NUT);
        btnX += btnWidth + btnSpacing;
        TaoNut(font, NUT_HUY_CHON, btnX, buttonY, btnWidth, NUT_CAO, "HUY CHON", MAU_NEN_NUT, MAU_CHU_NUT);
    }

    TaoNut(font, NUT_TRANG_TRUOC, BANG_X, pageY, 120.f, NUT_CAO, "<< Trang Truoc", MAU_NEN_NUT, MAU_CHU_NUT);
    TaoNut(font, NUT_TRANG_SAU, BANG_X + 130.f, pageY, 120.f, NUT_CAO, "Trang Sau >>", MAU_NEN_NUT, MAU_CHU_NUT);

    sf::Text pageInfo = TaoVanBan(font, "Trang " + std::to_string(currentState.trangHienTai) + " / " + std::to_string(currentState.tongSoTrang), 16, MAU_CHU);
    pageInfo.setPosition(BANG_X + 260.f, pageY + NUT_CAO / 2.f - pageInfo.getLocalBounds().height / 2.f - 2.f);
    window.draw(pageInfo);
}

static void VeFormThemSach(sf::RenderWindow &window, const sf::Font &font, const SachState& currentState) {
    // TOI UU: Static cache cho cac hints khong doi
    static const int namHienTai = LayNamHienTai();
    static const std::string goiYNamXB = std::to_string(NAM_XUAT_BAN_MIN) + "-" + std::to_string(namHienTai);
    static const std::string goiYSoTrang = "1 - " + std::to_string(MAX_SO_TRANG);
    static const std::string goiYSoLuong = "1 - " + std::to_string(MAX_BAN_SAO) + " ban sao";
    static const std::string goiYISBN = "10 hoac 13 chu so";
    static const std::string goiYTenSachBase = "Toi da " + std::to_string(MAX_TEN_SACH) + " ky tu, bat dau bang chu";
    static const std::string goiYTacGiaBase = "Toi da " + std::to_string(MAX_TAC_GIA) + " ky tu, bat dau bang chu";
    static const std::string goiYTheLoaiBase = "Toi da " + std::to_string(MAX_THE_LOAI) + " ky tu, bat dau bang chu";
    
    // Hint text dong - chi tinh khi can
    const std::string goiYTenSach = currentState.chuoiTenSach.empty() 
        ? goiYTenSachBase
        : std::to_string(currentState.chuoiTenSach.length()) + "/" + std::to_string(MAX_TEN_SACH);
    const std::string goiYTacGia = currentState.chuoiTacGia.empty()
        ? goiYTacGiaBase
        : std::to_string(currentState.chuoiTacGia.length()) + "/" + std::to_string(MAX_TAC_GIA);
    const std::string goiYTheLoai = currentState.chuoiTheLoai.empty()
        ? goiYTheLoaiBase
        : std::to_string(currentState.chuoiTheLoai.length()) + "/" + std::to_string(MAX_THE_LOAI);

    std::string formTieuDe = currentState.dangSua ? "HIEU CHINH DAU SACH" : "THEM DAU SACH";

    // Tinh chieu cao form cho khop
    float formHeight = KHUNG_THONG_BAO_Y - FORM_Y - PADDING;
    VeKhung(window, FORM_X, FORM_Y, FORM_RONG, formHeight, formTieuDe, font);
    float currentY = FORM_Y + 40.f;
    float labelX = FORM_X + PADDING;
    float inputSpacing = PADDING / 2.f;

    TaoInput(font, INPUT_ISBN, labelX, currentY, INPUT_RONG, INPUT_CAO, "ISBN (*):", currentState.chuoiISBN, goiYISBN, currentState.dangSua);
    currentY += INPUT_CAO + inputSpacing;
    TaoInput(font, INPUT_TEN_SACH, labelX, currentY, INPUT_RONG, INPUT_CAO, "Ten Sach (*):", currentState.chuoiTenSach, goiYTenSach);
    currentY += INPUT_CAO + inputSpacing;
    TaoInput(font, INPUT_SO_TRANG, labelX, currentY, INPUT_RONG, INPUT_CAO, "So Trang (*):", currentState.chuoiSoTrang, goiYSoTrang);
    currentY += INPUT_CAO + inputSpacing;
    TaoInput(font, INPUT_TAC_GIA, labelX, currentY, INPUT_RONG, INPUT_CAO, "Tac Gia (*):", currentState.chuoiTacGia, goiYTacGia);
    currentY += INPUT_CAO + inputSpacing;
    TaoInput(font, INPUT_NAM_XB, labelX, currentY, INPUT_RONG, INPUT_CAO, "Nam XB (*):", currentState.chuoiNamXB, goiYNamXB);
    currentY += INPUT_CAO + inputSpacing;
    TaoInput(font, INPUT_THE_LOAI, labelX, currentY, INPUT_RONG, INPUT_CAO, "The Loai (*):", currentState.chuoiTheLoai, goiYTheLoai);
    currentY += INPUT_CAO + inputSpacing;
    
    // TOI UU: Su dung cache vi tri thay vi goi TimViTriDuyNhat() O(N*M) moi frame
    std::string goiYViTri;
    if (currentState.soViTriCache == 0) {
        goiYViTri = "VD: A1, B-5";
    } else {
        goiYViTri.reserve(50); // Pre-allocate
        const int maxDisplay = (currentState.soViTriCache < 3) ? currentState.soViTriCache : 3;
        for (int i = 0; i < maxDisplay; ++i) {
            goiYViTri += currentState.cacViTriCache[i];
            if (i < maxDisplay - 1) goiYViTri += ", ";
        }
        if (currentState.soViTriCache > 3) goiYViTri += "...";
    }
    
    TaoInput(font, INPUT_VI_TRI, labelX, currentY, INPUT_RONG, INPUT_CAO, "Vi Tri:", currentState.chuoiViTri, goiYViTri);
    currentY += INPUT_CAO + inputSpacing;

    if (!currentState.dangSua) {
        TaoInput(font, INPUT_SO_LUONG, labelX, currentY, INPUT_RONG, INPUT_CAO, "So Luong (*):", currentState.chuoiSoLuong, goiYSoLuong);
        currentY += INPUT_CAO + PADDING * 1.5f;
    }
    else {
        currentY += PADDING * 1.5f;
    }

    float totalWidth = FORM_RONG - 2 * PADDING;
    float nutRong = (totalWidth - PADDING) / 2;
    float nutCao = NUT_CAO + 10.f;

    std::string nutChinhLabel = currentState.dangSua ? "LUU THAY DOI" : "THEM SACH";

    if (currentState.dangSua) {
        TaoNut(font, NUT_THEM_SACH, labelX, currentY, nutRong, nutCao, nutChinhLabel, MAU_NHAN, MAU_CHU_NUT);
        TaoNut(font, NUT_BACK, labelX + nutRong + PADDING, currentY, nutRong, nutCao, "HUY", MAU_NEN_NUT, MAU_CHU_NUT);
    }
    else {
        float nutRongDon = 250.f;
        float nutXDon = labelX + (totalWidth - nutRongDon) / 2;
        TaoNut(font, NUT_THEM_SACH, nutXDon, currentY, nutRongDon, nutCao, nutChinhLabel, MAU_NHAN, MAU_CHU_NUT);
    }
}

static void VeKhungThongBaoSFML(sf::RenderWindow &window, const sf::Font &font, const SachState& currentState) {
    VeKhung(window, FORM_X, KHUNG_THONG_BAO_Y, FORM_RONG, KHUNG_THONG_BAO_CAO, "THONG BAO", font);

    if (currentState.xacNhanXoa) { // Ve hop thoai xac nhan xoa
        sf::Text txtConfirm = TaoVanBan(font, "Ban co chac muon xoa Dau Sach nay?\n(Chi xoa duoc neu khong con ban sao)", FONT_SIZE_BINH_THUONG, MAU_LOI);
        // Can chinh vi tri Y chu
        txtConfirm.setPosition(FORM_X + PADDING, KHUNG_THONG_BAO_Y + 35.f);
        window.draw(txtConfirm);

        // Can chinh vi tri Y nut
        float buttonY = KHUNG_THONG_BAO_Y + 85.f;

        // Tinh lai chieu rong nut cho vua
        float totalWidth = FORM_RONG - 2 * PADDING;
        float nutRong = (totalWidth - PADDING) / 2;

        TaoNut(font, NUT_XAC_NHAN_XOA, FORM_X + PADDING, buttonY, nutRong, NUT_CAO, "XAC NHAN XOA", MAU_LOI, MAU_CHU_NUT);
        TaoNut(font, NUT_HUY_XOA, FORM_X + PADDING + nutRong + PADDING, buttonY, nutRong, NUT_CAO, "HUY", MAU_NEN_NUT, MAU_CHU_NUT);
    }
    else {
        // Ve thong bao binh thuong (co word wrap)
        if (noiDungThongBao.empty()) {
            return; // Khong ve gi neu khong co noi dung
        }

        sf::Color mauChuThongBao = MAU_CHU;
        if (loaiThongBao == 1)
            mauChuThongBao = MAU_LOI;
        else if (loaiThongBao == 2)
            mauChuThongBao = MAU_THANH_CONG;

        // Tinh chieu rong toi da cho text (tru padding)
        float maxTextWidth = FORM_RONG - 2 * PADDING - 10.f; // Tru them 10px de an toan
        
        // Xu ly word wrap cho text
        std::string wrappedText = WordWrapText(font, noiDungThongBao, FONT_SIZE_BINH_THUONG, maxTextWidth);
        
        // Ve text (SFML tu dong xu ly '\n' de xuong dong)
        sf::Text txtMsg = TaoVanBan(font, wrappedText, FONT_SIZE_BINH_THUONG, mauChuThongBao);
        
        // Tinh vi tri Y de can giua theo chieu doc
        // Lay chieu cao thuc te cua text (bao gom nhieu dong)
        float textHeight = txtMsg.getLocalBounds().height;
        float startY = KHUNG_THONG_BAO_Y + 35.f; // Bat dau tu duoi tieu de
        float availableHeight = KHUNG_THONG_BAO_CAO - 35.f - PADDING; // Chieu cao co the dung
        
        // Gioi han so dong toi da (tranh tran ra ngoai)
        float maxLines = 4.f; // Toi da 4 dong
        float lineHeight = static_cast<float>(FONT_SIZE_BINH_THUONG) * 1.3f; // Chieu cao 1 dong (co khoang cach)
        float maxTextHeight = maxLines * lineHeight;
        
        // Neu text qua cao, cat bot (chi lay phan dau)
        if (textHeight > maxTextHeight) {
            // Dem so dong hien tai
            int soDong = 1;
            for (size_t i = 0; i < wrappedText.length(); ++i) {
                if (wrappedText[i] == '\n') soDong++;
            }
            
            // Neu nhieu hon maxLines, cat bot
            if (soDong > static_cast<int>(maxLines)) {
                int dongDaCat = 0;
                std::string textDaCat = "";
                for (size_t i = 0; i < wrappedText.length(); ++i) {
                    if (wrappedText[i] == '\n') {
                        dongDaCat++;
                        if (dongDaCat >= static_cast<int>(maxLines)) {
                            textDaCat += "..."; // Them "..." de bao hieu bi cat
                            break;
                        }
                    }
                    textDaCat += wrappedText[i];
                }
                wrappedText = textDaCat;
                txtMsg.setString(wrappedText);
                textHeight = txtMsg.getLocalBounds().height;
            }
        }
        
        float textY = startY;
        if (textHeight < availableHeight) {
            textY = startY + (availableHeight - textHeight) / 2.f;
        } else {
            textY = startY;
        }
        
        txtMsg.setPosition(FORM_X + PADDING, textY);
        window.draw(txtMsg);
    }
}

static void ThucHienThemBanSao(SachState& currentState) {
    int soLuongThem = 0;
    const std::string chuoiSach = CatKhoangTrang(currentState.soLuongBanSaoCanThemStr);
    if (!ChuyenChuoiThanhSoNguyen(chuoiSach, soLuongThem, true)) {
        soLuongThem = 0;
    }

    std::string ketQua = ThemBanSaoMoi(currentState.isbnSachDuocChon, soLuongThem);
    
    if (!ketQua.empty()) {
        CapNhatThongBaoSFML(ketQua, 1);
    } else {
        CapNhatThongBaoSFML("Da them " + std::to_string(soLuongThem) + " ban sao thanh cong!", 2);
        currentState.vuaThucHienThanhCong = true;
    }

    currentState.hienThiModalThemBS = false;
    currentState.soLuongBanSaoCanThemStr = "";
    inputHoatDong = KHONG_XAC_DINH;
    ThucHienTimKiemNoiBo(currentState);
}

static void VeModalThemBanSao(sf::RenderWindow &window, const sf::Font &font, const SachState& currentState) {
    sf::RectangleShape overlay(sf::Vector2f(CHIEU_RONG, CHIEU_CAO));
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(overlay);

    float modalRong = 500.f;
    float modalCao = 200.f;
    float modalX = BANG_X + (BANG_RONG - modalRong) / 2.f + 20.f; 
    float modalY = (CHIEU_CAO - modalCao) / 2.f;
    float paddingNoiBo = PADDING;

    VeKhung(window, modalX, modalY, modalRong, modalCao, "THEM BAN SAO", font);
    float inputY = modalY + 60.f;
    float inputHopRong = modalRong - 2 * paddingNoiBo;
    std::string hintSoLuong = currentState.soLuongBanSaoCanThemStr.empty() 
        ? "Nhap so luong (1-" + std::to_string(MAX_BAN_SAO) + ", chi nhan so)"
        : "";
    TaoInput(font, INPUT_SO_LUONG_THEM, modalX + paddingNoiBo, inputY, inputHopRong, INPUT_CAO, "", currentState.soLuongBanSaoCanThemStr, hintSoLuong);
    
    inputY += INPUT_CAO + PADDING / 2.f;
    sf::Text txtNote = TaoVanBan(font, "Vi tri se tu dong gan theo ban sao cu hoac mac dinh 'Ke Chinh'", FONT_SIZE_NHO, sf::Color(150, 150, 150));
    txtNote.setPosition(modalX + paddingNoiBo, inputY);
    window.draw(txtNote);
    float buttonY = modalY + modalCao - NUT_CAO - paddingNoiBo;
    float totalWidth = modalRong - 2 * paddingNoiBo;
    float nutRong = (totalWidth - paddingNoiBo) / 2.f;
    TaoNut(font, NUT_XAC_NHAN_THEM_BS, modalX + paddingNoiBo, buttonY, nutRong, NUT_CAO, "XAC NHAN", MAU_NHAN, MAU_CHU_NUT);
    TaoNut(font, NUT_HUY_THEM_BS, modalX + paddingNoiBo + nutRong + paddingNoiBo, buttonY, nutRong, NUT_CAO, "HUY", MAU_NEN_NUT, MAU_CHU_NUT);
}

static void CapNhatDuLieuXemTheoTheLoai(SachState& currentState) {
    LayDanhSachTheoTheLoai(currentState.cacTheLoaiCache, currentState.soTheLoaiCache);
    LayDanhSachViTri(currentState.cacViTriCache, currentState.soViTriCache);
    currentState.canCapNhatCache = false;
    float headerY = BANG_Y + 50.f;
    float contentY = headerY + 40.f;
    float contentBottom = CHIEU_CAO - (PADDING * 3.0f) - (NUT_CAO * 2);
    float contentHeight = contentBottom - contentY;
    contentHeight = std::max(contentHeight, 10.f);
    
    // Tinh toan tong chieu cao noi dung (su dung cache)
    currentState.totalContentHeightTheLoai = PADDING;
    float lineSpacing = 8.f;
    float groupSpacing = 20.f;
    float afterTitleSpacing = 35.f;
    float afterGroupSpacing = 10.f;
    float textBlockHeight = FONT_SIZE_BINH_THUONG * 2 + lineSpacing;
    
    // DUYET QUA TUNG THE LOAI - GOI SERVICE LAYER DE LAY SO LUONG
    for (int i = 0; i < currentState.soTheLoaiCache; ++i) {
        if (i > 0) currentState.totalContentHeightTheLoai += groupSpacing;
        currentState.totalContentHeightTheLoai += afterTitleSpacing;
        PTRDS tempSach[MAX_DAUSACH];
        int soSach = LayDanhSachSachTheoTheLoai(currentState.cacTheLoaiCache[i], tempSach, MAX_DAUSACH);
        currentState.totalContentHeightTheLoai += (float)soSach * textBlockHeight;
        currentState.totalContentHeightTheLoai += afterGroupSpacing;
    }
    currentState.totalContentHeightTheLoai += PADDING;
    currentState.totalContentHeightTheLoai = std::max(currentState.totalContentHeightTheLoai, contentHeight);
}

static void VeDanhSachTheoTheLoai(sf::RenderWindow& window, const sf::Font& font, SachState& currentState) {
    // buoc 0: dinh nghia vung ve noi dung
    float headerY = BANG_Y + 50.f;
    float contentY = headerY + 40.f;
    float contentBottom = CHIEU_CAO - (PADDING * 3.0f) - (NUT_CAO * 2);
    float contentWidth = BANG_RONG;
    float contentHeight = contentBottom - contentY;
    contentHeight = std::max(contentHeight, 10.f);

    VeKhung(window, BANG_X, headerY, contentWidth, contentHeight + (contentY - headerY), "DANH SACH DAU SACH THEO THE LOAI (Cuon chuot de xem)", font);

    const std::string* cacTheLoai = currentState.cacTheLoaiCache;
    const int soTheLoai = currentState.soTheLoaiCache;

    float maxScroll = 0.f;
    if (currentState.totalContentHeightTheLoai > contentHeight) {
        maxScroll = currentState.totalContentHeightTheLoai - contentHeight;
    }
    if (currentState.scrollOffsetYTheLoai < 0.f) currentState.scrollOffsetYTheLoai = 0.f;
    if (currentState.scrollOffsetYTheLoai > maxScroll) currentState.scrollOffsetYTheLoai = maxScroll;

    float viewportTop = contentY / (float)CHIEU_CAO;
    viewportTop = std::max(0.f, std::min(1.f, viewportTop));
    float viewportHeight = contentHeight / (float)CHIEU_CAO;
    viewportHeight = std::max(0.f, std::min(1.f, viewportHeight));

    float rongThanhCuon = 8.f;
    float rongNoiDungView = contentWidth - rongThanhCuon - PADDING * 0.6f;
    rongNoiDungView = std::max(0.f, rongNoiDungView);

    currentState.theLoaiView.setViewport(sf::FloatRect(
        BANG_X / (float)CHIEU_RONG,
        viewportTop,
        rongNoiDungView / (float)CHIEU_RONG,
        viewportHeight
    ));
    
    currentState.theLoaiView.setSize(rongNoiDungView, contentHeight);
    currentState.theLoaiView.setCenter(rongNoiDungView / 2.f, contentHeight / 2.f + currentState.scrollOffsetYTheLoai);

    window.setView(currentState.theLoaiView);
    float currentY = PADDING; // y bat dau tuong doi (so voi view)
    float contentX = PADDING; // x bat dau tuong doi (so voi view)
    float bookIndentX = contentX + 35.f; // thut le cho sach
    float lineWidth = rongNoiDungView - 2 * PADDING; // chieu rong duong ke (theo view)
    
    // Khai bao cac hang so layout (can thiet cho vong lap ve)
    float lineSpacing = 8.f;
    float groupSpacing = 20.f;
    float afterTitleSpacing = 35.f;
    float afterGroupSpacing = 10.f;
    float textBlockHeight = FONT_SIZE_BINH_THUONG * 2 + lineSpacing;

    for (int i = 0; i < soTheLoai; ++i) {
        std::string currentTheLoai = cacTheLoai[i];

        if (i > 0) {
             sf::RectangleShape separator(sf::Vector2f(lineWidth, 1.f));
             separator.setPosition(contentX, currentY - 10.f);
             separator.setFillColor(MAU_BANG_BORDER);
             window.draw(separator);
             currentY += groupSpacing;
        }

        sf::Text genreTitle = TaoVanBan(font, "THE LOAI: " + currentTheLoai, FONT_SIZE_TIEU_DE_KHUNG, MAU_NHAN);
        genreTitle.setPosition(contentX, currentY);
        window.draw(genreTitle);
        currentY += afterTitleSpacing;

        PTRDS sachCungTheLoai[MAX_DAUSACH];
        int soSach = LayDanhSachSachTheoTheLoai(currentTheLoai, sachCungTheLoai, MAX_DAUSACH);

        sf::Text bookInfo = TaoVanBan(font, "", FONT_SIZE_BINH_THUONG, MAU_CHU);
        for (int j = 0; j < soSach; ++j) {
            PTRDS sach = sachCungTheLoai[j];
            if (!sach) continue;

            std::string sttStr = std::to_string(j + 1) + ". ";
            float rongChoNoiDung = rongNoiDungView - bookIndentX - PADDING;
            if (rongChoNoiDung < 20.f) rongChoNoiDung = 20.f;

            std::string tenSachSub = CatChuoiTheoRong(font, sach->tenSach, FONT_SIZE_BINH_THUONG, rongChoNoiDung);
            std::string metaInfo = "   ISBN: " + sach->ISBN + " | TG: " + sach->tacGia + " | NXB: " + std::to_string(sach->namXuatBan);
            std::string metaInfoSub = CatChuoiTheoRong(font, metaInfo, FONT_SIZE_BINH_THUONG, rongChoNoiDung);
            std::string infoLine1 = tenSachSub;
            std::string infoLine2 = metaInfoSub;
            float line1Y = currentY;

            bookInfo.setString(sttStr);
            bookInfo.setPosition(contentX + 5.f, line1Y);
            window.draw(bookInfo);
            bookInfo.setString(infoLine1);
            bookInfo.setPosition(bookIndentX, line1Y);
            window.draw(bookInfo);
            float line2Y = line1Y + FONT_SIZE_BINH_THUONG + (lineSpacing / 2);
            bookInfo.setString(infoLine2);
            bookInfo.setPosition(bookIndentX, line2Y);
            window.draw(bookInfo);
            currentY = line1Y + textBlockHeight;
        }
        currentY += afterGroupSpacing;
    }

    window.setView(window.getDefaultView());
    if (currentState.totalContentHeightTheLoai > contentHeight) {
        float scrollBarHeight = contentHeight; // chieu cao thanh cuon = chieu cao vung noi dung
        // vi tri X cua thanh cuon (ben phai khung noi dung)
        float scrollBarX = BANG_X + contentWidth - rongThanhCuon - PADDING * 0.4f;
        float scrollBarY = contentY; // bat dau tu y cua vung content (thay vi headerY)

        // ve duong ray
        sf::RectangleShape scrollTrack(sf::Vector2f(rongThanhCuon, scrollBarHeight));
        scrollTrack.setPosition(scrollBarX, scrollBarY);
        scrollTrack.setFillColor(MAU_BANG_HEADER);
        window.draw(scrollTrack);

        // tinh toan tay cam
        float handleHeight = scrollBarHeight * (contentHeight / currentState.totalContentHeightTheLoai);
        handleHeight = std::max(handleHeight, 20.f); // chieu cao toi thieu
        float handleY = scrollBarY;
        if (maxScroll > 0) { // tinh vi tri y cua tay cam
             handleY = scrollBarY + (currentState.scrollOffsetYTheLoai / maxScroll) * (scrollBarHeight - handleHeight);
             handleY = std::min(handleY, scrollBarY + scrollBarHeight - handleHeight);
        }
        // ve tay cam
        sf::RectangleShape scrollHandle(sf::Vector2f(rongThanhCuon, handleHeight));
        scrollHandle.setPosition(scrollBarX, handleY);
        scrollHandle.setFillColor(MAU_VIEN);
        window.draw(scrollHandle);
    }
}


// Ham Ve CHINH
void VeManHinhQuanLySach(sf::RenderWindow &window, const sf::Font &font) {
    // ve thanh tieu de man hinh
    sf::RectangleShape topBar(sf::Vector2f(CHIEU_RONG, THANH_TAB_CAO));
    topBar.setFillColor(MAU_KHUNG);
    window.draw(topBar);
    sf::Text title = TaoVanBan(font, "QUAN LY DAU SACH", FONT_SIZE_BINH_THUONG, MAU_TIEU_DE);
    title.setPosition(PADDING, PADDING / 2.f - title.getLocalBounds().height / 2.f);
    window.draw(title);

    // ve nut quay ve menu
    TaoNut(font, NUT_BACK, CHIEU_RONG - PADDING - 100.f, PADDING / 4.f, 100.f, NUT_CAO * 0.8f, "< MENU", MAU_NUT_BACK, MAU_CHU_NUT);
    // ve o tim kiem va cac nut lien quan
    std::string goiYTimKiem = "Nhap ten sach, tac gia, ISBN hoac the loai de tim kiem...";
    TaoInput(font, INPUT_TIM_SACH, BANG_X, BANG_Y, 500.f, INPUT_CAO, "Tim Kiem:", state.chuoiTimKiem, goiYTimKiem);
    TaoNut(font, NUT_TIM, BANG_X + 110.f + 500.f + 10.f, BANG_Y, 100.f, NUT_CAO, "TIM", MAU_NHAN, MAU_CHU_NUT);
    TaoNut(font, NUT_XOA_TIM, BANG_X + 110.f + 500.f + 10.f + 100.f + 10.f, BANG_Y, 100.f, NUT_CAO, "XOA TIM", MAU_NEN_NUT, MAU_CHU_NUT);

    float nutChuyenX = BANG_X + 110.f + 500.f + 10.f + 100.f + 10.f + 100.f + PADDING; // sau nut Xoa Tim
    // thay doi nhan nut tuy theo che do hien tai
    std::string nhanNutChuyen = (state.cheDoXemHienTai == XEM_TIM_KIEM) ? "Xem theo The Loai" : "Xem Tim Kiem/Tat Ca";
    TaoNut(font, NUT_CHUYEN_CHE_DO_XEM, nutChuyenX, BANG_Y, 220.f /* rong hon chut */, NUT_CAO, nhanNutChuyen, MAU_NEN_NUT, MAU_CHU_NUT);

    // Goi ham ve bang/danh sach tuy theo che do ---
    if (state.cheDoXemHienTai == XEM_TIM_KIEM) {
        VeBangSach(window, font, state); // ve bang phan trang nhu cu neu la che do tim kiem
    }
    else {
        VeDanhSachTheoTheLoai(window, font, state); // ve danh sach gom nhom neu la che do the loai
    }

    VeFormThemSach(window, font, state);      // ve form them/sua sach ben phai
    VeKhungThongBaoSFML(window, font, state); // ve khung thong bao ben duoi form

    if (state.hienThiModalBanSao) { // modal chi tiet ban sao
        VeModalChiTietBanSao(window, font, state);
    }
    else if (state.hienThiModalThemBS) { // modal them ban sao
        VeModalThemBanSao(window, font, state);
    }
}

// Ham xu ly backend cho modal
static void ThucHienThanhLySach(SachState& /*currentState*/, const std::string& maSach) {
    std::string ketQua = ThanhLyBanSaoTheoMa(maSach);
    
    if (ketQua.empty()) {
        CapNhatThongBaoSFML("Da thanh ly ma sach: " + maSach, 2);
    } else {
        CapNhatThongBaoSFML(ketQua, 1);
    }
}

static void ThucHienXoaBanSao(SachState& /*currentState*/, const std::string& maSach) {
    std::string ketQua = XoaBanSaoTheoMa(maSach);
    
    if (ketQua.empty()) {
        CapNhatThongBaoSFML("Da xoa ban sao: " + maSach, 2);
    } else {
        CapNhatThongBaoSFML(ketQua, 1);
    }
}


static void VeModalChiTietBanSao(sf::RenderWindow &window, const sf::Font &font, SachState& currentState) {
    PTRDS dauSach = TimDauSach(currentState.idModalSachDuocChon);
    if (!dauSach) {
        CapNhatThongBaoSFML("Loi: Khong tim thay dau sach de xem chi tiet!", 1);
        currentState.hienThiModalBanSao = false;
        return;
    }

    sf::RectangleShape overlay(sf::Vector2f(CHIEU_RONG, CHIEU_CAO));
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(overlay);

    float modalRong = 800.f;
    float modalCao = 500.f;
    float modalX = (CHIEU_RONG - modalRong) / 2.f - 200.f;
    float modalY = (CHIEU_CAO - modalCao) / 2.f - 40.f;
    float paddingNoiBo = 20.f;

    std::string tieuDeModal = CatChuoiVoiDauCham(dauSach->tenSach, 45);
    VeKhung(window, modalX, modalY, modalRong, modalCao, "CHI TIET BAN SAO: " + tieuDeModal, font);
    
    sf::Text hintText = TaoVanBan(font, "Luu y: 'Thanh Ly' truoc, sau do 'Xoa' de loai bo khoi he thong", FONT_SIZE_NHO - 2, sf::Color(150, 150, 150));
    hintText.setPosition(modalX + paddingNoiBo, modalY + 35.f);
    window.draw(hintText);
    
    int tongBanSao = dauSach->tongBanSao;
    
    sf::Text infoText = TaoVanBan(font, "Tong: " + std::to_string(tongBanSao) + " ban sao", FONT_SIZE_NHO, MAU_NHAN);
    infoText.setPosition(modalX + modalRong - 200.f, modalY + 35.f);
    window.draw(infoText);

    float currentY = modalY + 60.f;
    float col1X = modalX + paddingNoiBo;
    float col2X = col1X + 50.f;
    float col3X = col2X + 250.f;
    float col4X = col3X + 150.f;
    float col5X = col4X + 140.f;
    sf::Text header = TaoVanBan(font, "STT", FONT_SIZE_BINH_THUONG, MAU_NHAN);
    header.setPosition(col1X, currentY);
    window.draw(header);
    header.setString("Ma Sach");
    header.setPosition(col2X, currentY);
    window.draw(header);
    header.setString("Trang Thai");
    header.setPosition(col3X, currentY);
    window.draw(header);
    header.setString("Vi Tri");
    header.setPosition(col4X, currentY);
    window.draw(header);
    header.setString("Hanh Dong");
    header.setPosition(col5X, currentY);
    window.draw(header);
    currentY += 30.f;

    PTRDMS tempBanSao[MAX_BAN_SAO];
    int soLuongBSTrongDSLK = LayDanhSachBanSaoSapXep(dauSach->ISBN, tempBanSao, MAX_BAN_SAO);
    sf::Text dataText = TaoVanBan(font, "", FONT_SIZE_NHO, MAU_CHU);
    int sttBatDau = (currentState.trangModal - 1) * SO_SACH_MOI_TRANG_MODAL;
    int sttKetThuc = sttBatDau + SO_SACH_MOI_TRANG_MODAL;
    sttKetThuc = std::min(sttKetThuc, soLuongBSTrongDSLK); // Khong vuot qua so luong thuc te

    currentState.soLuongNutModal = 0; // Reset mang nut dong

    for (int stt = sttBatDau; stt < sttKetThuc; ++stt) {
        PTRDMS p = tempBanSao[stt]; // Lay con tro tu mang tam
        // Ve STT (stt + 1 vi stt bat dau tu 0)
        dataText.setString(std::to_string(stt + 1));
        dataText.setPosition(col1X, currentY);
        window.draw(dataText);
        // Ve Ma Sach
        dataText.setString(p->maSach);
        dataText.setPosition(col2X, currentY);
        window.draw(dataText);

        // Ve Trang Thai (voi mau tuong ung)
        const char *trangThaiText = TenTrangThai(p->trangThai);
        sf::Color trangThaiColor = MAU_CHU;
        if (p->trangThai == DANG_MUON)
            trangThaiColor = MAU_LOI;           // Do - Canh bao
        else if (p->trangThai == THANH_LY)
            trangThaiColor = MAU_VIEN;          // Vang - Cho xoa
        else if (p->trangThai == CHO_MUON_DUOC)
            trangThaiColor = MAU_THANH_CONG;    // Xanh - San sang

        dataText.setString(trangThaiText);
        dataText.setFillColor(trangThaiColor);
        dataText.setPosition(col3X, currentY);
        window.draw(dataText);
        dataText.setFillColor(MAU_CHU); // Tra lai mau mac dinh

        // Ve Vi Tri
        dataText.setString(p->viTri);
        dataText.setPosition(col4X, currentY);
        window.draw(dataText);

        // Ve cac nut hanh dong tuy theo trang thai (LOGIC ROI RAC)
        float nutY = currentY - 2.f;
        float nutRong = 100.f;
        float nutCao = 25.f;
        
        if (p->trangThai == CHO_MUON_DUOC) {
            // CHO_MUON_DUOC -> Chi hien nut "Thanh Ly"
            sf::RectangleShape nutTL(sf::Vector2f(nutRong, nutCao));
            nutTL.setPosition(col5X, nutY);
            nutTL.setFillColor(MAU_VIEN);
            nutTL.setOutlineColor(MAU_VIEN);
            nutTL.setOutlineThickness(1.f);
            window.draw(nutTL);
            
            sf::Text txtTL = TaoVanBan(font, "[!] Thanh Ly", FONT_SIZE_NHO - 2, MAU_CHU_NUT);
            CanGiuaGocVanBan(txtTL);
            txtTL.setPosition(col5X + nutRong / 2.f, nutY + nutCao / 2.f);
            window.draw(txtTL);
            
            // Luu nut (chi co Thanh Ly)
            if (currentState.soLuongNutModal < SO_SACH_MOI_TRANG_MODAL) {
                currentState.cacNutTrongModal[currentState.soLuongNutModal].maSach = p->maSach;
                currentState.cacNutTrongModal[currentState.soLuongNutModal].khuVucNhanThanhLy = nutTL.getGlobalBounds();
                currentState.cacNutTrongModal[currentState.soLuongNutModal].khuVucNhanXoa = sf::FloatRect(); // Rong
                currentState.soLuongNutModal++;
            }
        }
        else if (p->trangThai == THANH_LY) {
            // THANH_LY -> Chi hien nut "Xoa"
            sf::RectangleShape nutXoa(sf::Vector2f(nutRong, nutCao));
            nutXoa.setPosition(col5X, nutY);
            nutXoa.setFillColor(MAU_LOI);
            nutXoa.setOutlineColor(MAU_LOI);
            nutXoa.setOutlineThickness(1.f);
            window.draw(nutXoa);
            
            sf::Text txtXoa = TaoVanBan(font, "[X] Xoa", FONT_SIZE_NHO - 2, MAU_CHU_NUT);
            CanGiuaGocVanBan(txtXoa);
            txtXoa.setPosition(col5X + nutRong / 2.f, nutY + nutCao / 2.f);
            window.draw(txtXoa);
            
            // Luu nut (chi co Xoa)
            if (currentState.soLuongNutModal < SO_SACH_MOI_TRANG_MODAL) {
                currentState.cacNutTrongModal[currentState.soLuongNutModal].maSach = p->maSach;
                currentState.cacNutTrongModal[currentState.soLuongNutModal].khuVucNhanThanhLy = sf::FloatRect(); // Rong
                currentState.cacNutTrongModal[currentState.soLuongNutModal].khuVucNhanXoa = nutXoa.getGlobalBounds();
                currentState.soLuongNutModal++;
            }
        }
        // DANG_MUON -> Khong co nut nao (hien dau '-')
        
        currentY += 30.f; // Xuong hang
    } // Ket thuc vong for

    // Ve Hop Thoai Xac Nhan Thanh Ly
    if (currentState.xacNhanThanhLy) {
        sf::RectangleShape confirmOverlay(sf::Vector2f(modalRong, modalCao));
        confirmOverlay.setPosition(modalX, modalY);
        confirmOverlay.setFillColor(sf::Color(0, 0, 0, 100));
        window.draw(confirmOverlay);
        float confirmRong = 400.f;
        float confirmCao = 150.f;
        float confirmX = modalX + (modalRong - confirmRong) / 2.f;
        float confirmY = modalY + (modalCao - confirmCao) / 2.f;
        VeKhung(window, confirmX, confirmY, confirmRong, confirmCao, "XAC NHAN THANH LY", font);
        sf::Text confirmText = TaoVanBan(font, "Ban co chac muon thanh ly ma sach:\n" + currentState.maSachCanThanhLy + "?", FONT_SIZE_BINH_THUONG, MAU_LOI);
        confirmText.setPosition(confirmX + paddingNoiBo, confirmY + 45.f);
        window.draw(confirmText);
        float confirmBtnY = confirmY + confirmCao - 50.f;
        float confirmBtnWidth = (confirmRong - 3 * paddingNoiBo) / 2.f;
        TaoNut(font, NUT_XAC_NHAN_THANHLY, confirmX + paddingNoiBo, confirmBtnY, confirmBtnWidth, 40.f, "XAC NHAN", MAU_LOI, MAU_CHU_NUT);
        TaoNut(font, NUT_HUY_THANHLY, confirmX + 2 * paddingNoiBo + confirmBtnWidth, confirmBtnY, confirmBtnWidth, 40.f, "HUY", MAU_NEN_NUT, MAU_CHU_NUT);
    }
    
    // Ve Hop Thoai Xac Nhan Xoa Ban Sao
    if (currentState.xacNhanXoaBanSao) {
        sf::RectangleShape confirmOverlay(sf::Vector2f(modalRong, modalCao));
        confirmOverlay.setPosition(modalX, modalY);
        confirmOverlay.setFillColor(sf::Color(0, 0, 0, 100));
        window.draw(confirmOverlay);
        float confirmRong = 400.f;
        float confirmCao = 150.f;
        float confirmX = modalX + (modalRong - confirmRong) / 2.f;
        float confirmY = modalY + (modalCao - confirmCao) / 2.f;
        VeKhung(window, confirmX, confirmY, confirmRong, confirmCao, "XAC NHAN XOA BAN SAO", font);
        sf::Text confirmText = TaoVanBan(font, "Ban co chac muon xoa ban sao:\n" + currentState.maSachCanXoa + "?", FONT_SIZE_BINH_THUONG, MAU_LOI);
        confirmText.setPosition(confirmX + paddingNoiBo, confirmY + 45.f);
        window.draw(confirmText);
        float confirmBtnY = confirmY + confirmCao - 50.f;
        float confirmBtnWidth = (confirmRong - 3 * paddingNoiBo) / 2.f;
        TaoNut(font, NUT_XAC_NHAN_XOA_BANSAO, confirmX + paddingNoiBo, confirmBtnY, confirmBtnWidth, 40.f, "XAC NHAN", MAU_LOI, MAU_CHU_NUT);
        TaoNut(font, NUT_HUY_XOA_BANSAO, confirmX + 2 * paddingNoiBo + confirmBtnWidth, confirmBtnY, confirmBtnWidth, 40.f, "HUY", MAU_NEN_NUT, MAU_CHU_NUT);
    }
    
    
    // Ve cac nut dieu khien phia duoi modal (Dong + Phan trang)
    float bottomY = modalY + modalCao - 50.f;
    
    // TOI UU: Tan dung field tongBanSao thay vi duyet DSLK dem lai
    int tongBanSaoModal = dauSach->tongBanSao;
    int tongSoTrangModal = (tongBanSaoModal + SO_SACH_MOI_TRANG_MODAL - 1) / SO_SACH_MOI_TRANG_MODAL;
    if (tongSoTrangModal < 1) tongSoTrangModal = 1;
    
    // Nut phan trang (neu co nhieu hon 1 trang)
    if (tongSoTrangModal > 1) {
        TaoNut(font, NUT_MODAL_TRANG_TRUOC, modalX + paddingNoiBo, bottomY, 80.f, 40.f, "<< Truoc", MAU_NEN_NUT, MAU_CHU_NUT);
        TaoNut(font, NUT_MODAL_TRANG_SAU, modalX + paddingNoiBo + 90.f, bottomY, 80.f, 40.f, "Sau >>", MAU_NEN_NUT, MAU_CHU_NUT);
        
        // Hien thi "Trang X / Y"
        sf::Text pageInfoModal = TaoVanBan(font, "Trang " + std::to_string(currentState.trangModal) + " / " + std::to_string(tongSoTrangModal), FONT_SIZE_NHO, MAU_CHU);
        pageInfoModal.setPosition(modalX + paddingNoiBo + 180.f, bottomY + 10.f);
        window.draw(pageInfoModal);
    }
    
    // Nut Dong (ben phai)
    TaoNut(font, NUT_DONG_MODAL_CHITIET, modalX + modalRong - 120.f - paddingNoiBo, bottomY, 120.f, 40.f, "DONG", MAU_LOI, MAU_CHU_NUT);
}

// Ham Xu Ly Su Kien CHINH
void XuLySuKienManHinhSach(sf::RenderWindow &window, sf::Event event) {
    (void)window; // danh dau khong dung bien window

    // Xu ly cuon chuot cho che do xem The Loai
    if (state.cheDoXemHienTai == XEM_THEO_THE_LOAI && !state.hienThiModalBanSao && !state.hienThiModalThemBS) { // Chi cuon khi o che do TL va khong co modal
        if (event.type == sf::Event::MouseWheelScrolled) {
            if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                // Dieu chinh scrollOffsetYTheLoai dua tren delta, 30.f la toc do cuon
                state.scrollOffsetYTheLoai -= event.mouseWheelScroll.delta * 30.f;

                // Gioi han lai scroll sau khi cuon
                float maxScroll = 0.f;
                if (state.totalContentHeightTheLoai > (CHIEU_CAO - (PADDING * 5.5f) - (NUT_CAO * 2) - (BANG_Y + 90.f))) { // Recalculate contentHeight equivalent
                    maxScroll = state.totalContentHeightTheLoai - (CHIEU_CAO - (PADDING * 5.5f) - (NUT_CAO * 2) - (BANG_Y + 90.f));
                }
                if (state.scrollOffsetYTheLoai < 0.f)
                    state.scrollOffsetYTheLoai = 0.f;
                if (state.scrollOffsetYTheLoai > maxScroll)
                    state.scrollOffsetYTheLoai = maxScroll;

                return; // Da xu ly scroll, khong can lam gi them voi su kien nay
            }
        }
    }

    // chan tat ca su kien neu modal dang hien
    if (state.hienThiModalBanSao || state.hienThiModalThemBS) {
        if (state.hienThiModalThemBS) { // Xu ly rieng cho Modal Them Ban Sao
            if (event.type == sf::Event::TextEntered || 
                (event.type == sf::Event::KeyPressed && 
                 (event.key.code == sf::Keyboard::Backspace || 
                  event.key.code == sf::Keyboard::Tab ||
                  event.key.code == sf::Keyboard::Z))) {
                if (inputHoatDong == INPUT_SO_LUONG_THEM) {
                    XuLyTextInput(event, state);
                }
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    MaUI elementNhan = LayElementTaiToaDo(event.mouseButton.x, event.mouseButton.y);
                    if (elementNhan == NUT_XAC_NHAN_THEM_BS) {
                        ThucHienThemBanSao(state);
                    }
                    else if (elementNhan == NUT_HUY_THEM_BS) {
                        state.hienThiModalThemBS = false;
                        state.soLuongBanSaoCanThemStr = "";
                        inputHoatDong = KHONG_XAC_DINH;
                        CapNhatThongBaoSFML("Da huy them ban sao.", 0);
                    }
                    else if (elementNhan == INPUT_SO_LUONG_THEM) {
                        inputHoatDong = INPUT_SO_LUONG_THEM;
                    }
                    else {
                        inputHoatDong = KHONG_XAC_DINH;
                    }
                }
            }
            return; // Chan cac su kien khac
        }
        else if (state.hienThiModalBanSao) { // Xu ly rieng cho Modal Chi Tiet
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    MaUI elementNhan = LayElementTaiToaDo(event.mouseButton.x, event.mouseButton.y);

                    // Xu ly hop thoai xac nhan THANH LY
                    if (state.xacNhanThanhLy) {
                        if (elementNhan == NUT_XAC_NHAN_THANHLY) {
                            ThucHienThanhLySach(state, state.maSachCanThanhLy);
                            state.xacNhanThanhLy = false;
                            state.maSachCanThanhLy = "";
                        }
                        else if (elementNhan == NUT_HUY_THANHLY) {
                            state.xacNhanThanhLy = false;
                            state.maSachCanThanhLy = "";
                            CapNhatThongBaoSFML("Da huy thanh ly.", 0);
                        }
                        return;
                    }
                    
                    // Xu ly hop thoai xac nhan XOA BAN SAO
                    if (state.xacNhanXoaBanSao) {
                        if (elementNhan == NUT_XAC_NHAN_XOA_BANSAO) {
                            ThucHienXoaBanSao(state, state.maSachCanXoa);
                            state.xacNhanXoaBanSao = false;
                            state.maSachCanXoa = "";
                        }
                        else if (elementNhan == NUT_HUY_XOA_BANSAO) {
                            state.xacNhanXoaBanSao = false;
                            state.maSachCanXoa = "";
                            CapNhatThongBaoSFML("Da huy xoa ban sao.", 0);
                        }
                        return;
                    }

                    if (elementNhan == NUT_DONG_MODAL_CHITIET) {
                        state.hienThiModalBanSao = false;
                        state.idModalSachDuocChon = "";
                        state.trangModal = 1;
                        return;
                    }
                    
                    // Xu ly phan trang modal
                    if (elementNhan == NUT_MODAL_TRANG_TRUOC) {
                        if (state.trangModal > 1) {
                            state.trangModal--;
                        }
                        return;
                    }
                    if (elementNhan == NUT_MODAL_TRANG_SAU) {
                        PTRDS dauSachCheck = TimDauSach(state.idModalSachDuocChon);
                        if (dauSachCheck) {
                            int tongBS = dauSachCheck->tongBanSao; // Su dung field co san thay vi duyet DSLK
                            int tongTrang = (tongBS + SO_SACH_MOI_TRANG_MODAL - 1) / SO_SACH_MOI_TRANG_MODAL;
                            if (state.trangModal < tongTrang) {
                                state.trangModal++;
                            }
                        }
                        return;
                    }

                    // Kiem tra click vao nut Thanh Ly hoac Xoa
                    for (int i = 0; i < state.soLuongNutModal; ++i) {
                        // Nut Thanh Ly
                        if (state.cacNutTrongModal[i].khuVucNhanThanhLy.contains(event.mouseButton.x, event.mouseButton.y)) {
                            state.xacNhanThanhLy = true;
                            state.maSachCanThanhLy = state.cacNutTrongModal[i].maSach;
                            return;
                        }
                        // Nut Xoa
                        if (state.cacNutTrongModal[i].khuVucNhanXoa.contains(event.mouseButton.x, event.mouseButton.y)) {
                            state.xacNhanXoaBanSao = true;
                            state.maSachCanXoa = state.cacNutTrongModal[i].maSach;
                            return;
                        }
                    }
                }
            }
            return; // Chan cac su kien khac
        }
    }

    // Code xu ly binh thuong (khi modal khong hien)
    // Xu ly nhap lieu van ban (go phim, xoa, undo...)
    if (event.type == sf::Event::TextEntered || 
        (event.type == sf::Event::KeyPressed && 
         (event.key.code == sf::Keyboard::Backspace || 
          event.key.code == sf::Keyboard::Enter || 
          event.key.code == sf::Keyboard::Tab ||
          event.key.code == sf::Keyboard::Z))) { // Them Z de xu ly Ctrl+Z
        if (inputHoatDong != KHONG_XAC_DINH) {
            XuLyTextInput(event, state);
        }
    }

    // Xu ly nhan chuot
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            MaUI elementNhan = LayElementTaiToaDo(event.mouseButton.x, event.mouseButton.y);

            // TOI UU: Dung switch cho performance tot hon if-else chain
            bool clickedInput = false;
            switch (elementNhan) {
                case INPUT_TIM_SACH:
                case INPUT_ISBN:
                case INPUT_TEN_SACH:
                case INPUT_SO_TRANG:
                case INPUT_TAC_GIA:
                case INPUT_NAM_XB:
                case INPUT_THE_LOAI:
                case INPUT_VI_TRI:
                case INPUT_SO_LUONG:
                case INPUT_SO_LUONG_THEM:
                    if (!(state.dangSua && elementNhan == INPUT_ISBN)) {
                        if (inputHoatDong != elementNhan) {
                            KhoiTaoStack(&state.undoStack);
                            state.stackDaKhoiTao = false;
                            inputHoatDong = elementNhan;
                            CapNhatThongBaoSFML("", 0);
                        }
                        clickedInput = true;
                    }
                    break;
                default:
                    break;
            }

            // Neu click ra ngoai o input thi deactive
            if (!clickedInput && inputHoatDong != KHONG_XAC_DINH) {
                inputHoatDong = KHONG_XAC_DINH;
                KhoiTaoStack(&state.undoStack);
                state.stackDaKhoiTao = false; // Reset flag
                CapNhatThongBaoSFML("", 0);
            }

            // Xu ly click vao NUT (neu khong phai click vao input)
            if (!clickedInput) {
                switch (elementNhan) {
                // Nut Quay Ve Menu hoac Huy Sua
                case NUT_BACK:
                    if (state.dangSua) { // Neu dang sua thi Huy Sua
                        state.dangSua = false;
                        XoaFormNhapLieuSFML(state);
                        CapNhatThongBaoSFML("Da huy hieu chinh.", 0);
                    }
                    else {                                // Neu khong thi Quay Ve Menu
                        manHinhHienTai = MENU_CHINH; // Chuyen man hinh
                        // Reset cac trang thai cua man hinh Sach
                        state.dangSua = false;
                        state.xacNhanXoa = false;
                        inputHoatDong = KHONG_XAC_DINH;
                        state.isbnSachDuocChon = "";
                        state.cheDoXemHienTai = XEM_TIM_KIEM; // Reset che do xem
                        state.scrollOffsetYTheLoai = 0.f;     // Reset vi tri cuon
                        CapNhatThongBaoSFML("", 0);     // Xoa thong bao
                    }
                    break; // Ket thuc case NUT_BACK
                // Nut Tim Kiem
                case NUT_TIM:
                    inputHoatDong = KHONG_XAC_DINH;
                    state.cheDoXemHienTai = XEM_TIM_KIEM; // Nhan Tim thi luon chuyen ve che do Tim Kiem
                    state.scrollOffsetYTheLoai = 0.f;
                    ThucHienTimKiemNoiBo(state);
                    break;
                // Nut Xoa Tim Kiem (Reset)
                case NUT_XOA_TIM:
                    state.chuoiTimKiem = "";
                    inputHoatDong = KHONG_XAC_DINH;
                    state.cheDoXemHienTai = XEM_TIM_KIEM; // Reset cung chuyen ve che do Tim Kiem
                    state.scrollOffsetYTheLoai = 0.f;
                    ThucHienTimKiemNoiBo(state);
                    break;
                // Nut Them Sach / Luu Thay Doi
                case NUT_THEM_SACH:
                    inputHoatDong = KHONG_XAC_DINH;
                    ThucHienThemHoacSuaSachSFML(state);
                    break;

                case NUT_CHUYEN_CHE_DO_XEM:
                    if (state.cheDoXemHienTai == XEM_TIM_KIEM) {
                        state.cheDoXemHienTai = XEM_THEO_THE_LOAI;
                        state.scrollOffsetYTheLoai = 0.f;
                        // Reset trang thai khong can thiet cho che do xem the loai
                        state.isbnSachDuocChon = "";
                        state.dangSua = false;
                        state.xacNhanXoa = false;
                        inputHoatDong = KHONG_XAC_DINH;
                        CapNhatDuLieuXemTheoTheLoai(state); 
                        CapNhatThongBaoSFML("Dang xem theo the loai.", 0);
                    }
                    else {
                        state.cheDoXemHienTai = XEM_TIM_KIEM;
                        state.scrollOffsetYTheLoai = 0.f;
                        ThucHienTimKiemNoiBo(state); // Tai lai danh sach tim kiem/tat ca
                        // Thong bao da duoc cap nhat trong ThucHienTimKiemNoiBo
                    }
                    break;

                case NUT_TRANG_TRUOC:
                    if (state.cheDoXemHienTai == XEM_TIM_KIEM && state.trangHienTai > 1)
                        state.trangHienTai--;
                    break;
                case NUT_TRANG_SAU:
                    if (state.cheDoXemHienTai == XEM_TIM_KIEM && state.trangHienTai < state.tongSoTrang)
                        state.trangHienTai++;
                    break;
                case NUT_THEM_BAN_SAO:
                    if (state.cheDoXemHienTai == XEM_TIM_KIEM && !state.isbnSachDuocChon.empty()) {
                        state.hienThiModalThemBS = true;
                        state.soLuongBanSaoCanThemStr = "";
                        inputHoatDong = INPUT_SO_LUONG_THEM;
                        CapNhatThongBaoSFML("Nhap so luong ban sao can them.", 0);
                    }
                    else if (state.cheDoXemHienTai == XEM_THEO_THE_LOAI) {
                        CapNhatThongBaoSFML("Khong dung duoc o che do xem The Loai!", 1);
                    }
                    else {
                        CapNhatThongBaoSFML("Loi: Vui long chon sach truoc!", 1);
                    }
                    break;
                case NUT_CHI_TIET_SACH:
                    if (state.cheDoXemHienTai != XEM_TIM_KIEM) {
                        CapNhatThongBaoSFML("Khong dung duoc o che do xem The Loai!", 1);
                    }
                    else if (state.isbnSachDuocChon.empty()) {
                        CapNhatThongBaoSFML("Loi: Vui long chon sach truoc!", 1);
                    }
                    else {
                        state.hienThiModalBanSao = true;
                        state.idModalSachDuocChon = state.isbnSachDuocChon;
                        state.trangModal = 1;
                    }
                    break;
                case NUT_SUA:
                    if (state.cheDoXemHienTai != XEM_TIM_KIEM) {
                        CapNhatThongBaoSFML("Khong dung duoc o che do xem The Loai!", 1);
                    }
                    else if (state.isbnSachDuocChon.empty()) {
                        CapNhatThongBaoSFML("Vui long chon mot dau sach de sua!", 1);
                    }
                    else {
                        DienFormVoiSachDuocChon(state);
                        state.dangSua = true;
                        state.xacNhanXoa = false;
                        inputHoatDong = INPUT_TEN_SACH;
                        CapNhatThongBaoSFML("Dang hieu chinh sach...", 0);
                    }
                    break;
                case NUT_XOA:
                    if (state.cheDoXemHienTai == XEM_TIM_KIEM) {
                        if (state.isbnSachDuocChon.empty()) {
                            CapNhatThongBaoSFML("Loi: Vui long chon mot dau sach de xoa!", 1);
                        }
                        else {
                            // GOI SERVICE BACKEND - KIEM TRA BUSINESS RULE
                            if (CoTheBanSao(state.isbnSachDuocChon)) {
                                PTRDS sachCanXoa = TimDauSach(state.isbnSachDuocChon);
                                std::string tenNgan = CatChuoiVoiDauCham(sachCanXoa->tenSach, 30);
                                CapNhatThongBaoSFML("Khong the xoa '" + tenNgan + "' (con ban sao)!", 1);
                            }
                            else {
                                state.xacNhanXoa = true;
                                state.dangSua = false;
                                inputHoatDong = KHONG_XAC_DINH;
                                CapNhatThongBaoSFML("", 0);
                            }
                        }
                    }
                    else {
                        CapNhatThongBaoSFML("Chuc nang nay khong dung duoc o che do xem theo the loai.", 1);
                    }
                    break;
                case NUT_XAC_NHAN_XOA: // Giu nguyen, chi goi khi xacNhanXoa=true
                    if (state.xacNhanXoa && !state.isbnSachDuocChon.empty())
                        ThucHienXoaSachSFML(state);
                    state.xacNhanXoa = false;
                    break;
                case NUT_HUY_XOA: // Giu nguyen
                    state.xacNhanXoa = false;
                    CapNhatThongBaoSFML("Da huy thao tac xoa.", 0);
                    break;
                case NUT_HUY_CHON:
                    if (state.cheDoXemHienTai == XEM_TIM_KIEM) { // chi huy chon o che do tim kiem
                        state.isbnSachDuocChon = "";
                        state.dangSua = false;
                        state.xacNhanXoa = false;
                        XoaFormNhapLieuSFML(state);
                        CapNhatThongBaoSFML("Da huy chon.", 0);
                    }
                    break;
                case HANG_SACH:
                    if (state.cheDoXemHienTai != XEM_TIM_KIEM) break;
                    
                    // Tinh chi so sach duoc click
                    {
                        const float contentY = BANG_Y + 90 + 35;
                        const float rowHeight = 30.f;
                        const int startIndex = (state.trangHienTai - 1) * SACH_MOI_TRANG;
                        const int rowIndex = static_cast<int>((event.mouseButton.y - contentY) / rowHeight);
                        const int actualIndex = startIndex + rowIndex;

                        // Validate chi so va lay ISBN
                        if (actualIndex < 0 || actualIndex >= state.soLuongKetQuaTimKiem || 
                            !state.ketQuaTimKiem[actualIndex].sach) {
                            break;
                        }
                        
                        const std::string& clickedISBN = state.ketQuaTimKiem[actualIndex].sach->ISBN;
                        const float elapsed = state.doubleClickClock.getElapsedTime().asSeconds();
                        
                        if (elapsed < state.THOI_GIAN_DOUBLE_CLICK && clickedISBN == state.isbnClickCuoi) {
                            state.isbnSachDuocChon = clickedISBN;
                            state.dangSua = false;
                            state.xacNhanXoa = false;
                            std::string tenSachHienThi = CatChuoiVoiDauCham(state.ketQuaTimKiem[actualIndex].sach->tenSach, 35);
                            CapNhatThongBaoSFML("Da chon: " + tenSachHienThi, 0);
                            inputHoatDong = KHONG_XAC_DINH;
                            XoaFormNhapLieuSFML(state);
                            state.isbnClickCuoi = "";
                        }
                        else {
                            // Click lan dau
                            state.isbnClickCuoi = clickedISBN;
                            state.doubleClickClock.restart();
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }
    // Xu ly nhan phim ESCAPE (thoat input, huy sua/xoa, quay ve menu)
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
        if (inputHoatDong != KHONG_XAC_DINH) {
            inputHoatDong = KHONG_XAC_DINH;
        }
        else if (state.dangSua) {
            state.dangSua = false;
            XoaFormNhapLieuSFML(state);
            CapNhatThongBaoSFML("Da huy hieu chinh.", 0);
        }
        else if (state.xacNhanXoa) {
            state.xacNhanXoa = false;
            CapNhatThongBaoSFML("Da huy thao tac xoa.", 0);
        }
        else {
            manHinhHienTai = MENU_CHINH;
            state.cheDoXemHienTai = XEM_TIM_KIEM; // Reset che do xem khi thoat
            state.scrollOffsetYTheLoai = 0.f;
            CapNhatThongBaoSFML("", 0);
        }
    }
}


// Ham xu ly nhap lieu text 
static void XuLyTextInput(sf::Event event, SachState& currentState) {
    // Kiem tra xem co input nao dang active khong
    if (inputHoatDong == KHONG_XAC_DINH) return;
    // Khong xu ly Tab neu la o nhap so luong trong modal Them BS
    if(inputHoatDong == INPUT_SO_LUONG_THEM && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Tab) return;

    // Khoi tao stack lan dau (neu chua)
    if (!currentState.stackDaKhoiTao) {
        KhoiTaoStack(&currentState.undoStack);
        currentState.stackDaKhoiTao = true;
    }

    // TOI UU: Struct cho field info de giam code lap
    struct FieldInfo {
        std::string* target;
        int maxLen;
        bool numOnly;
        const char* name;
    };
    
    // Map input ID -> field info
    FieldInfo fieldInfo;
    switch (inputHoatDong) {
        case INPUT_TIM_SACH:      fieldInfo = {&currentState.chuoiTimKiem, 50, false, "Tim kiem"}; break;
        case INPUT_ISBN:          fieldInfo = {&currentState.chuoiISBN, 13, true, "ISBN"}; break;
        case INPUT_TEN_SACH:      fieldInfo = {&currentState.chuoiTenSach, MAX_TEN_SACH, false, "Ten sach"}; break;
        case INPUT_SO_TRANG:      fieldInfo = {&currentState.chuoiSoTrang, 5, true, "So trang"}; break;
        case INPUT_TAC_GIA:       fieldInfo = {&currentState.chuoiTacGia, MAX_TAC_GIA, false, "Tac gia"}; break;
        case INPUT_NAM_XB:        fieldInfo = {&currentState.chuoiNamXB, 4, true, "Nam XB"}; break;
        case INPUT_THE_LOAI:      fieldInfo = {&currentState.chuoiTheLoai, MAX_THE_LOAI, false, "The loai"}; break;
        case INPUT_VI_TRI:        fieldInfo = {&currentState.chuoiViTri, 20, false, "Vi tri"}; break;
        case INPUT_SO_LUONG:      fieldInfo = {&currentState.chuoiSoLuong, 3, true, "So luong"}; break;
        case INPUT_SO_LUONG_THEM: fieldInfo = {&currentState.soLuongBanSaoCanThemStr, 3, true, "So luong them"}; break;
        default: return;
    }
    
    std::string* targetString = fieldInfo.target;
    const int maxLen = fieldInfo.maxLen;
    const bool chiNhanSo = fieldInfo.numOnly;
    const char* fieldName = fieldInfo.name;

    bool changed = false; // Co de kiem tra xem noi dung co thay doi khong
    std::string currentInputContent = (targetString != nullptr) ? *targetString : ""; // Luu noi dung hien tai TRUOC KHI thay doi

    // Xu ly Ctrl + Z (Undo) - XU LY TRUOC cac event khac
    // Dung sf::Keyboard::isKeyPressed vi event.key.control khong on dinh tren Windows
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Z && 
        (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl))) {
        
        if (!KiemTraStackRong(&currentState.undoStack) && targetString != nullptr) {
            // Pop trang thai cu va gan lai
            std::string previousState = PopStack(&currentState.undoStack);
            *targetString = previousState;
            changed = true; // Danh dau da thay doi de chay validation
            
            // Xoa thong bao loi (neu co)
            if (loaiThongBao == 1) {
                CapNhatThongBaoSFML("", 0);
            }
        } else {
            // Stack rong, khong co gi de undo
            if (targetString != nullptr) {
                CapNhatThongBaoSFML("Khong co gi de undo!", 0);
            }
        }
        return; // Return sau khi xu ly de tranh xu ly tiep
    }

    // Xu ly Enter cho INPUT_SO_LUONG_THEM (xac nhan them ban sao)
    if (event.type == sf::Event::KeyPressed && inputHoatDong == INPUT_SO_LUONG_THEM) {
        if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Enter) {
            ThucHienThemBanSao(currentState); // currentState o day la tham so SachState&
            return;
        }
    }

    // Xu ly go phim (CHI khi khong phai Ctrl+Z da duoc xu ly)
    if (event.type == sf::Event::TextEntered && !(event.type == sf::Event::KeyPressed && event.key.control && event.key.code == sf::Keyboard::Z)) {
        // Chi lay ky tu ASCII, khong lay Backspace, Enter, Tab
        if (event.text.unicode < 128 && event.text.unicode != 8 && event.text.unicode != 13 && event.text.unicode != 9) {
            char enteredChar = static_cast<char>(event.text.unicode);
            if (targetString->length() < static_cast<size_t>(maxLen)) { // Kiem tra do dai toi da
                bool allowChar = false;
                std::string loiNhap;
                loiNhap.reserve(100); // TOI UU: Pre-allocate
                
                if (chiNhanSo) {
                    if (enteredChar >= '0' && enteredChar <= '9') {
                        allowChar = true;
                    } else if (enteredChar >= 32) {
                        loiNhap = "Loi: " + std::string(fieldName) + " chi duoc nhap so (0-9)!";
                    }
                } else {
                    // Cho phep chu, so, khoang trang va 1 so ky tu dac biet
                    if (isalnum(enteredChar) || enteredChar == ' ' || enteredChar == '+' || enteredChar == '#' || enteredChar == '-' || enteredChar == '.' || enteredChar == '_') {
                        // Khong cho phep nhieu khoang trang lien tiep
                        if (enteredChar == ' ' && !targetString->empty() && targetString->back() == ' ') {
                            loiNhap = "Loi: Khong duoc nhap nhieu khoang trang lien tiep!";
                        }
                        else {
                            allowChar = true;
                        }
                    } else if (enteredChar >= 32) { // Ky tu khong hop le
                        loiNhap = "Loi: Ky tu '" + std::string(1, enteredChar) + "' khong duoc phep nhap!";
                    }
                }
                
                // Hien thi loi neu co
                if (!loiNhap.empty()) {
                    CapNhatThongBaoSFML(loiNhap, 1);
                }
                
                // Neu ky tu hop le
                if (allowChar) {
                    // Dam bao stack da duoc khoi tao
                    if (!currentState.stackDaKhoiTao) {
                        KhoiTaoStack(&currentState.undoStack);
                        currentState.stackDaKhoiTao = true;
                    }
                    
                    // Push trang thai cu vao stack TRUOC KHI them ky tu moi
                    PushStack(&currentState.undoStack, currentInputContent);
                    
                    // Them ky tu vao chuoi
                    *targetString += enteredChar;
                    changed = true; // Danh dau da thay doi
                    
                    // Xoa loi neu co (validation se chay lai o phan duoi)
                    if(loaiThongBao == 1) CapNhatThongBaoSFML("", 0);
                }
            } else if (event.text.unicode >= 32 && event.text.unicode < 128) { // Vuot qua gioi han
                CapNhatThongBaoSFML("Loi: " + std::string(fieldName) + " da dat gioi han " + std::to_string(maxLen) + " ky tu!\nHien tai: " + std::to_string(targetString->length()) + " ky tu. Vui long rut ngan.", 1);
            }
        }
    } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Backspace && 
               !sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && !sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) { // Xu ly Backspace (khong phai khi giu Ctrl)
        if (!targetString->empty() && targetString != nullptr) {
            // Dam bao stack da duoc khoi tao
            if (!currentState.stackDaKhoiTao) {
                KhoiTaoStack(&currentState.undoStack);
                currentState.stackDaKhoiTao = true;
            }
            
            // Push trang thai CU vao stack TRUOC KHI xoa
            PushStack(&currentState.undoStack, currentInputContent);
            
            // Xoa ky tu cuoi
            targetString->pop_back();
            changed = true; // Danh dau da thay doi
            
            // Xoa loi neu co (validation se chay lai o phan duoi)
            if(loaiThongBao == 1) CapNhatThongBaoSFML("", 0);
        }
    }

    // Kiem tra loi ngay khi dang go (realtime validation)
    // Validate cho TAT CA cac truong (ke ca khi xoa text)
    if (changed) {
        std::string loi = "";
        std::string tempStr = *targetString;
        
        // TOI UU: Nhom cac truong validation giong nhau
        switch (inputHoatDong) {
            case INPUT_TIM_SACH:
                break; // Khong validate
            case INPUT_SO_LUONG_THEM:
                // Validate format so luong - GOI HELPER
                if (!tempStr.empty()) {
                    KiemTraChiChuaSo(tempStr, loi, "So luong");
                }
                break;
            case INPUT_ISBN:
                // Realtime validation cho ISBN
                // Luu y: Khi SUA thi ISBN la readonly, nen khong can validate realtime
                // Nhung neu user van go duoc (bug) thi van validate de dam bao
                if (!currentState.dangSua) {
                    // CHI VALIDATE KHI THEM - GOI BACKEND
                    loi = KiemTraISBN(tempStr);
                }
                // Khi SUA (dangSua = true): Khong validate realtime vi ISBN readonly
                break;
            case INPUT_TEN_SACH:
            case INPUT_TAC_GIA:
            case INPUT_THE_LOAI:
                // Validate chuoi text - GOI BACKEND
                loi = KiemTraChuoiVaDodai(tempStr, std::string(fieldName), maxLen);
                break;
            case INPUT_SO_TRANG:
                // Validate format so - GOI HELPER
                if (!tempStr.empty()) {
                    KiemTraChiChuaSo(tempStr, loi, "So trang");
                }
                break;
            case INPUT_NAM_XB:
                // Validate format nam - GOI HELPER + check do dai
                if (!tempStr.empty()) {
                    if (KiemTraChiChuaSo(tempStr, loi, "Nam xuat ban")) {
                        if (tempStr.length() > 4) {
                            loi = "Loi: Nam xuat ban phai co 4 chu so!\nHien tai co " + std::to_string(tempStr.length()) + " chu so.";
                        }
                    }
                }
                break;
            case INPUT_VI_TRI:
                // Validate vi tri (cho phep rong hoac dinh dang hop le)
                if (!tempStr.empty()) {
                    std::string viTriTest = ChuanHoaViTri(tempStr);
                    if (viTriTest.empty()) {
                        loi = "Loi: Vi tri khong hop le!\nChi duoc chua chu cai va so.\nVi du: A-12, B5, Ke C-3.";
                    }
                }
                break;
            case INPUT_SO_LUONG:
                // Validate format so luong - GOI HELPER
                if (!tempStr.empty()) {
                    KiemTraChiChuaSo(tempStr, loi, "So luong");
                }
                break;
            default: break;
        }
        
        // Chi hien thi loi cho cac truong khac INPUT_TIM_SACH
        if (inputHoatDong != INPUT_TIM_SACH) {
            if (!loi.empty()) {
                CapNhatThongBaoSFML(loi, 1);
            } else if (loaiThongBao == 1) {
                // Xoa loi neu khong con loi
                CapNhatThongBaoSFML("", 0);
            }
        }
    }

    // Xu ly nhan Enter/Tab de nhay input (khong ap dung cho modal)
    if (event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Tab) && inputHoatDong != INPUT_SO_LUONG_THEM) {
         std::string loiCuoi = "";
        std::string tempStrCuoi = *targetString;
        tempStrCuoi = CatKhoangTrang(tempStrCuoi);
        switch (inputHoatDong) {
            case INPUT_ISBN:
                loiCuoi = KiemTraChuoiRong(tempStrCuoi, std::string(fieldName));
                if (loiCuoi.empty() && !tempStrCuoi.empty()) {
                    std::string isbnChuanTam;
                    std::string loiChuan = ChuanHoaISBNFile(tempStrCuoi, isbnChuanTam);
                    if (!loiChuan.empty()) {
                        loiCuoi = loiChuan;
                    } else if (!currentState.dangSua) {
                        std::string loiTrung = KiemTraTrungISBN(isbnChuanTam);
                        if (!loiTrung.empty()) loiCuoi = loiTrung;
                    }
                }
                break;
            case INPUT_TEN_SACH:
            case INPUT_TAC_GIA:
            case INPUT_THE_LOAI:
                // Backend KiemTraChuoiVaDodai() se validate - UI khong can check them
                break;
            case INPUT_SO_TRANG:
                // Backend se validate range - UI chi check format co ban
                break;
            case INPUT_NAM_XB:
                // Backend KiemTraNamXuatBan() se validate - UI khong can duplicate
                break;
            case INPUT_SO_LUONG:
                // Backend se validate range - UI chi check format co ban
                break;
            default: break;
        }

        if (loiCuoi.empty()) {
            KhoiTaoStack(&currentState.undoStack); // Reset Stack khi chuyen input
            currentState.stackDaKhoiTao = false; // Reset flag
            if (inputHoatDong >= INPUT_TIM_SACH && inputHoatDong < INPUT_SO_LUONG) {
                inputHoatDong = static_cast<MaUI>(inputHoatDong + 1);
                CapNhatThongBaoSFML("", 0);
            } else if (inputHoatDong == INPUT_SO_LUONG) {
                inputHoatDong = KHONG_XAC_DINH;
                if (event.key.code == sf::Keyboard::Enter) ThucHienThemHoacSuaSachSFML(currentState);
            } else {
                inputHoatDong = KHONG_XAC_DINH;
            }
        } else {
            CapNhatThongBaoSFML(loiCuoi, 1);
        }
    }
}


// Cap nhat so trang hien thi
static void CapNhatPhanTrangSFML(SachState& currentState) {
    int totalItems = currentState.soLuongKetQuaTimKiem;
    currentState.tongSoTrang = (totalItems + SACH_MOI_TRANG - 1) / SACH_MOI_TRANG;
    if (currentState.tongSoTrang == 0)
        currentState.tongSoTrang = 1; // It nhat 1 trang
    if (currentState.trangHienTai > currentState.tongSoTrang)
        currentState.trangHienTai = currentState.tongSoTrang; // Khong vuot qua trang cuoi
}

// Goi backend de tim kiem/lay tat ca sach
static void ThucHienTimKiemNoiBo(SachState& currentState) {
    // GOI HAM BACKEND - Logic "Get All" va "Search" da duoc gom lai
    LayDanhSachTimKiem(currentState.chuoiTimKiem, currentState.ketQuaTimKiem, 
                       currentState.soLuongKetQuaTimKiem, MAX_DAUSACH);
    
    currentState.trangHienTai = 1;
    CapNhatPhanTrangSFML(currentState);
    
    // Reset trang thai sau tim kiem
    currentState.isbnSachDuocChon = "";
    currentState.dangSua = false;
    currentState.xacNhanXoa = false;

    // Hien thi thong bao (neu khong phai sau thao tac thanh cong)
    if (!currentState.vuaThucHienThanhCong) {
        std::string tuKhoa = ChuanHoaKhoangTrang(currentState.chuoiTimKiem);
        int tongSach = LaySoLuongDauSach();
        
        if (currentState.soLuongKetQuaTimKiem > 0 || tuKhoa.empty()) {
            if (tuKhoa.empty() && tongSach > 0)
                CapNhatThongBaoSFML("Hien thi tat ca " + std::to_string(currentState.soLuongKetQuaTimKiem) + " dau sach.", 0);
            else if (currentState.soLuongKetQuaTimKiem > 0)
                CapNhatThongBaoSFML("Tim thay " + std::to_string(currentState.soLuongKetQuaTimKiem) + " ket qua.", 0);
            else if (tongSach == 0 && tuKhoa.empty())
                CapNhatThongBaoSFML("Danh sach rong.", 0);
        }
        else {
            CapNhatThongBaoSFML("Khong tim thay ket qua cho '" + tuKhoa + "'.", 1);
        }
    }
    currentState.vuaThucHienThanhCong = false;
}

void KhoiTaoManHinhSach() {
    ThucHienTimKiemNoiBo(state);
}

static void XoaFormNhapLieuSFML(SachState& currentState) {
    currentState.chuoiISBN = "";
    currentState.chuoiTenSach = "";
    currentState.chuoiSoTrang = "";
    currentState.chuoiTacGia = "";
    currentState.chuoiNamXB = "";
    currentState.chuoiTheLoai = "";
    currentState.chuoiViTri = "";
    currentState.chuoiSoLuong = "";
}

static void DienFormVoiSachDuocChon(SachState& currentState) {
    PTRDS sach = TimDauSach(currentState.isbnSachDuocChon);
    if (sach) {
        currentState.chuoiISBN = sach->ISBN;
        currentState.chuoiTenSach = sach->tenSach;
        currentState.chuoiSoTrang = std::to_string(sach->soTrang);
        currentState.chuoiTacGia = sach->tacGia;
        currentState.chuoiNamXB = std::to_string(sach->namXuatBan);
        currentState.chuoiTheLoai = sach->theLoai;
        currentState.chuoiViTri = LayViTriDauTienCuaDauSach(sach->ISBN);
    }
    else {
        CapNhatThongBaoSFML("Loi: Khong tim thay sach de sua!", 1);
        XoaFormNhapLieuSFML(currentState);
        currentState.isbnSachDuocChon = "";
    }
}


static void ThucHienThemHoacSuaSachSFML(SachState& currentState) {
    currentState.chuoiTenSach = CatKhoangTrang(currentState.chuoiTenSach);
    currentState.chuoiTacGia = CatKhoangTrang(currentState.chuoiTacGia);
    currentState.chuoiTheLoai = CatKhoangTrang(currentState.chuoiTheLoai);
    
    std::string loi;
    int soTrang = 0;
    int namXB = 0;
    int soLuong = 0;
    std::string isbnChuan = currentState.chuoiISBN;

    loi = KiemTraChuoiVaDodai(currentState.chuoiTenSach, "Ten sach", MAX_TEN_SACH);
    VALIDATE_AND_RETURN(!loi.empty(), loi, INPUT_TEN_SACH);
    
    VALIDATE_AND_RETURN(
        !ChuyenChuoiThanhSoNguyen(CatKhoangTrang(currentState.chuoiSoTrang), soTrang, true) || soTrang <= 0 || soTrang > MAX_SO_TRANG,
        "Loi: So trang khong hop le! (1-" + std::to_string(MAX_SO_TRANG) + ")",
        INPUT_SO_TRANG
    );
    
    loi = KiemTraChuoiVaDodai(currentState.chuoiTacGia, "Tac gia", MAX_TAC_GIA);
    VALIDATE_AND_RETURN(!loi.empty(), loi, INPUT_TAC_GIA);
    
    VALIDATE_AND_RETURN(
        !ChuyenChuoiThanhSoNguyen(CatKhoangTrang(currentState.chuoiNamXB), namXB, true),
        "Loi: Nam xuat ban phai la so 4 chu so!",
        INPUT_NAM_XB
    );
    
    loi = KiemTraNamXuatBan(namXB);
    VALIDATE_AND_RETURN(!loi.empty(), loi, INPUT_NAM_XB);
    
    loi = KiemTraChuoiVaDodai(currentState.chuoiTheLoai, "The loai", MAX_THE_LOAI);
    VALIDATE_AND_RETURN(!loi.empty(), loi, INPUT_THE_LOAI);

    if (!currentState.chuoiViTri.empty()) {
        std::string viTriChuan = ChuanHoaViTri(currentState.chuoiViTri);
        VALIDATE_AND_RETURN(viTriChuan.empty(), "Loi: Vi tri khong hop le! VD: A-12, B5", INPUT_VI_TRI);
        currentState.chuoiViTri = viTriChuan;
    }

    if (!currentState.dangSua) {
        // Validation ISBN
        if (!(loi = KiemTraChuoiRong(currentState.chuoiISBN, "ISBN")).empty() ||
            !(loi = ChuanHoaISBNFile(currentState.chuoiISBN, isbnChuan)).empty() ||
            !(loi = KiemTraTrungISBN(isbnChuan)).empty()) {
            CapNhatThongBaoSFML(loi, 1);
            inputHoatDong = INPUT_ISBN;
            return;
        }

        // Validation so luong
        VALIDATE_AND_RETURN(
            !ChuyenChuoiThanhSoNguyen(CatKhoangTrang(currentState.chuoiSoLuong), soLuong, true) || soLuong <= 0 || soLuong > MAX_BAN_SAO,
            "Loi: So luong khong hop le! (1-" + std::to_string(MAX_BAN_SAO) + ")",
            INPUT_SO_LUONG
        );

        loi = KiemTraTongSoBanSao(isbnChuan, soLuong);
        VALIDATE_AND_RETURN(!loi.empty(), loi, INPUT_SO_LUONG);

        std::string ketQua = ThemDauSachMoi(isbnChuan, currentState.chuoiTenSach, soTrang,
                                            currentState.chuoiTacGia, namXB, currentState.chuoiTheLoai,
                                            soLuong, currentState.chuoiViTri);

        if (ketQua.empty()) {
            std::string tenHienThi = CatChuoiVoiDauCham(ChuyenThanhTitleCase(currentState.chuoiTenSach), 30);
            CapNhatThongBaoSFML("Them thanh cong: " + tenHienThi, 2);
            currentState.vuaThucHienThanhCong = true;
            
            ResetVaTaiLaiDuLieu(currentState);
            inputHoatDong = KHONG_XAC_DINH;
        }
        else {
            CapNhatThongBaoSFML(ketQua, 1);
        }
    }
    else {
        std::string ketQua = CapNhatThongTinDauSach(currentState.isbnSachDuocChon, currentState.chuoiTenSach, soTrang,
                                                    currentState.chuoiTacGia, namXB, currentState.chuoiTheLoai,
                                                    currentState.chuoiViTri);

        if (ketQua.empty()) {
            std::string tenHienThi = CatChuoiVoiDauCham(ChuyenThanhTitleCase(currentState.chuoiTenSach), 30);
            CapNhatThongBaoSFML("Cap nhat thanh cong: " + tenHienThi, 2);
            currentState.vuaThucHienThanhCong = true;
            
            ResetVaTaiLaiDuLieu(currentState);
            currentState.dangSua = false;
            inputHoatDong = KHONG_XAC_DINH;
            currentState.isbnSachDuocChon = "";
        }
        else {
            CapNhatThongBaoSFML(ketQua, 1);
        }
    }
}

static void ResetVaTaiLaiDuLieu(SachState& currentState, bool xoaForm, bool xoaISBN) {
    if (xoaForm) {
        XoaFormNhapLieuSFML(currentState);
    }
    if (xoaISBN) {
        currentState.isbnSachDuocChon = "";
    }
    ThucHienTimKiemNoiBo(currentState);
    currentState.canCapNhatCache = true;
    if (currentState.cheDoXemHienTai == XEM_THEO_THE_LOAI) {
        CapNhatDuLieuXemTheoTheLoai(currentState);
    }
}

static void ThucHienXoaSachSFML(SachState& currentState) {
    if (currentState.isbnSachDuocChon.empty()) {
        CapNhatThongBaoSFML("Loi: Chua chon dau sach de xoa!", 1);
        return;
    }

    std::string ketQua = XoaDauSachTheoISBN(currentState.isbnSachDuocChon);

    if (ketQua.empty()) {
        CapNhatThongBaoSFML("Xoa dau sach thanh cong!", 2);
        ResetVaTaiLaiDuLieu(currentState, false, true);
    }
    else {
        CapNhatThongBaoSFML(ketQua, 1);
    }
    currentState.xacNhanXoa = false;
}
