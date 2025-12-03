// ===============================================
// FILE: ManHinhQuanLyDocGia.cpp (PHẦN 1: STATE & HELPERS)
// MÔ TẢ: Implementation màn hình độc giả
// ===============================================

#include "ManHinhQuanLyDocGia.h"
#include "TrangThaiManHinhDocGia.h"
#include "GiaoDienSFML.h"
#include "DocGia.h"
#include "Constants.h"
#include "TienIchGiaoDien.h"
#include "XuLyChuoi.h"
#include "KiemTraDuLieu.h"
// Biến state duy nhất
static DocGiaState state;

// ===== FORWARD DECLARATIONS =====
static void XuLyTextInputDocGia(sf::Event event, DocGiaState& s);
static void VeBangDocGia(sf::RenderWindow &window, const sf::Font &font, const DocGiaState& s);
static void VeFormNhapDocGia(sf::RenderWindow &window, const sf::Font &font, const DocGiaState& s);
static void VeModalChiTietMuon(sf::RenderWindow &window, const sf::Font &font, const DocGiaState& s);
static void VeKhungThongBaoDocGia(sf::RenderWindow &window, const sf::Font &font, const DocGiaState& s);

static void CapNhatDuLieuHienThi(DocGiaState& s);
static void ThucHienThemHoacSuaDocGia(DocGiaState& s);
static void ThucHienXoaDocGia(DocGiaState& s);

// ===== HELPER: Cắt chuỗi với "..." =====
static inline std::string CatChuoi(const std::string& str, size_t maxLen) {
    return (str.length() > maxLen) ? str.substr(0, maxLen - 3) + "..." : str;
}

// ===============================================
// PHẦN 2: VẼ BẢNG ĐỘC GIẢ
// ===============================================

static void VeBangDocGia(sf::RenderWindow &window, const sf::Font &font, const DocGiaState& s) {
    float headerY = BANG_Y + 50.f;
    float contentY = headerY + 40.f;
    float tableBottom = CHIEU_CAO - (PADDING * 5.5f) - (NUT_CAO * 2);
    
    // Tiêu đề tùy theo chế độ
    std::string tieuDe = "DANH SACH DOC GIA";
    if (s.cheDoXem == XEM_DANH_SACH_MA_THE) tieuDe += " (Theo Ma The)";
    else if (s.cheDoXem == XEM_DANH_SACH_TEN_HO) tieuDe += " (Theo Ten + Ho)";
    else tieuDe += " - QUA HAN (Double-click de chon)";
    
    VeKhung(window, BANG_X, headerY, BANG_RONG, tableBottom - headerY, tieuDe, font);
    
    // ===== CỘT: STT | Mã Thẻ | Họ Tên | Phái | Trạng Thái | Đang Mượn | [Quá Hạn] =====
    float colWidths[] = {0.05f, 0.12f, 0.35f, 0.10f, 0.15f, 0.13f, 0.10f};
    float colX[8];
    colX[0] = BANG_X + PADDING;
    for (int i = 0; i < 7; ++i) {
        colX[i + 1] = colX[i] + colWidths[i] * (BANG_RONG - PADDING);
        if (i < 6) {
            sf::RectangleShape line(sf::Vector2f(1.f, tableBottom - contentY));
            line.setPosition(colX[i + 1] - PADDING / 2.f, contentY);
            line.setFillColor(MAU_BANG_BORDER);
            window.draw(line);
        }
    }
    
    // ===== HEADER =====
    sf::Text headerText;
    headerText.setFont(font);
    headerText.setCharacterSize(FONT_SIZE_BINH_THUONG);
    headerText.setFillColor(MAU_NHAN);
    
    std::string headers[] = {"STT", "Ma The", "Ho Ten", "Phai", "Trang Thai", "Dang Muon", "Qua Han"};
    
    float headerBoxHeight = 30.f;
    sf::RectangleShape headerBg(sf::Vector2f(BANG_RONG - PADDING * 0.8, headerBoxHeight));
    headerBg.setPosition(BANG_X + PADDING * 0.4, contentY);
    headerBg.setFillColor(MAU_BANG_HEADER);
    window.draw(headerBg);
    
    for (int i = 0; i < 7; ++i) {
        headerText.setString(headers[i]);
        headerText.setPosition(colX[i], contentY + (headerBoxHeight - headerText.getCharacterSize()) / 2.f);
        window.draw(headerText);
    }
    
    sf::RectangleShape headerLine(sf::Vector2f(BANG_RONG - 2 * PADDING, 1.f));
    headerLine.setPosition(BANG_X + PADDING, contentY + 30.f);
    headerLine.setFillColor(MAU_BANG_BORDER);
    window.draw(headerLine);
    
    // ===== DATA ROWS =====
    float currentY = contentY + 35.f;
    int startIndex = (s.trangHienTai - 1) * SACH_MOI_TRANG;
    int endIndex = std::min(startIndex + SACH_MOI_TRANG, 
                            (s.cheDoXem == XEM_DOC_GIA_QUA_HAN) ? s.soLuongQuaHan : s.soLuongDocGia);
    
    float rowHeight = 30.f;
    sf::Text dataText;
    dataText.setFont(font);
    dataText.setCharacterSize(FONT_SIZE_BINH_THUONG);
    
    for (int i = startIndex; i < endIndex; ++i) {
        PTRDG dg = nullptr;
        int soNgayQuaHan = 0;
        
        // Lấy dữ liệu tùy theo chế độ
        if (s.cheDoXem == XEM_DOC_GIA_QUA_HAN) {
            dg = s.mangQuaHan[i].docGia;
            soNgayQuaHan = s.mangQuaHan[i].soNgayQuaHanMax;
        } else {
            dg = s.mangDocGia[i];
        }
        
        if (!dg) continue;
        
        // Highlight dòng được chọn
        bool isSelected = (dg->data.MaThe == s.maTheDocGiaDuocChon);
        if (isSelected) {
            sf::RectangleShape highlight(sf::Vector2f(BANG_RONG - PADDING * 0.8, rowHeight));
            highlight.setPosition(BANG_X + PADDING * 0.4, currentY - 2.f);
            highlight.setFillColor(MAU_NHAN);
            window.draw(highlight);
        }
        
        dataText.setFillColor(isSelected ? sf::Color::Black : MAU_CHU);
        float textY = currentY + (rowHeight - dataText.getCharacterSize()) / 2.f;
        
        // STT
        dataText.setString(std::to_string(i + 1));
        dataText.setPosition(colX[0], textY);
        window.draw(dataText);
        
        // Mã Thẻ
        dataText.setString(std::to_string(dg->data.MaThe));
        dataText.setPosition(colX[1], textY);
        window.draw(dataText);
        
        // Họ Tên
        std::string hoTen = dg->data.Ho + " " + dg->data.Ten;
        dataText.setString(CatChuoi(hoTen, 30));
        dataText.setPosition(colX[2], textY);
        window.draw(dataText);
        
        // Phái
        dataText.setString(dg->data.Phai ? "Nu" : "Nam");
        dataText.setPosition(colX[3], textY);
        window.draw(dataText);
        
        // Trạng Thái (màu xanh/đỏ)
        std::string trangThai = dg->data.TrangThai ? "Hoat dong" : "Khoa";
        dataText.setFillColor(isSelected ? sf::Color::Black : (dg->data.TrangThai ? MAU_THANH_CONG : MAU_LOI));
        dataText.setString(trangThai);
        dataText.setPosition(colX[4], textY);
        window.draw(dataText);
        
        dataText.setFillColor(isSelected ? sf::Color::Black : MAU_CHU);
        
        // Đang Mượn
        dataText.setString(std::to_string(dg->data.soSachDangMuon) + "/3");
        dataText.setPosition(colX[5], textY);
        window.draw(dataText);
        
        // Quá Hạn (chỉ hiện khi > 0)
        if (soNgayQuaHan > 0 || s.cheDoXem == XEM_DOC_GIA_QUA_HAN) {
            dataText.setFillColor(isSelected ? sf::Color::Black : MAU_LOI);
            dataText.setString(std::to_string(soNgayQuaHan) + " ngay");
            dataText.setPosition(colX[6], textY);
            window.draw(dataText);
        }
        
        currentY += rowHeight;
        if (currentY > tableBottom - PADDING) break;
    }
    
    // ===== NÚT CHỨC NĂNG =====
    float buttonY = tableBottom + PADDING;
    float pageY = buttonY + NUT_CAO + PADDING / 2;
    
    if (s.maTheDocGiaDuocChon != -1) {
        float btnWidth = (BANG_RONG - (PADDING * 4)) / 4.f;
        float btnX = BANG_X;
        
        TaoNut(font, NUT_CHI_TIET_DOC_GIA, btnX, buttonY, btnWidth, NUT_CAO, "CHI TIET", MAU_NHAN, MAU_CHU_NUT);
        btnX += btnWidth + PADDING;
        TaoNut(font, NUT_SUA_DOC_GIA, btnX, buttonY, btnWidth, NUT_CAO, "SUA", MAU_NEN_NUT, MAU_CHU_NUT);
        btnX += btnWidth + PADDING;
        TaoNut(font, NUT_XOA_DOC_GIA, btnX, buttonY, btnWidth, NUT_CAO, "XOA", MAU_LOI, MAU_CHU_NUT);
        btnX += btnWidth + PADDING;
        TaoNut(font, NUT_HUY_CHON_DOC_GIA, btnX, buttonY, btnWidth, NUT_CAO, "HUY CHON", MAU_NEN_NUT, MAU_CHU_NUT);
    }
    
    // ===== PHÂN TRANG =====
    TaoNut(font, NUT_TRANG_TRUOC_DG, BANG_X, pageY, 120.f, NUT_CAO, "<< Trang Truoc", MAU_NEN_NUT, MAU_CHU_NUT);
    TaoNut(font, NUT_TRANG_SAU_DG, BANG_X + 130.f, pageY, 120.f, NUT_CAO, "Trang Sau >>", MAU_NEN_NUT, MAU_CHU_NUT);
    
    sf::Text pageInfo = TaoVanBan(font, "Trang " + std::to_string(s.trangHienTai) + " / " + std::to_string(s.tongSoTrang), 16, MAU_CHU);
    pageInfo.setPosition(BANG_X + 260.f, pageY + NUT_CAO / 2.f - pageInfo.getLocalBounds().height / 2.f - 2.f);
    window.draw(pageInfo);
}

// ===============================================
// PHẦN 3: FORM NHẬP LIỆU
// ===============================================

static void VeFormNhapDocGia(sf::RenderWindow &window, const sf::Font &font, const DocGiaState& s) {
    std::string formTieuDe = s.dangSua ? "HIEU CHINH DOC GIA" : "THEM DOC GIA";
    float formHeight = KHUNG_THONG_BAO_Y - FORM_Y - PADDING;
    
    VeKhung(window, FORM_X, FORM_Y, FORM_RONG, formHeight, formTieuDe, font);
    
    float currentY = FORM_Y + 40.f;
    float labelX = FORM_X + PADDING;
    float inputSpacing = PADDING / 2.f;
    
    // Hint động
    std::string goiYHo = s.chuoiHo.empty() ? "Toi da " + std::to_string(MAX_TAC_GIA) + " ky tu" : std::to_string(s.chuoiHo.length()) + "/" + std::to_string(MAX_TAC_GIA);
    std::string goiYTen = s.chuoiTen.empty() ? "Toi da " + std::to_string(MAX_TAC_GIA) + " ky tu" : std::to_string(s.chuoiTen.length()) + "/" + std::to_string(MAX_TAC_GIA);
    
    // Input Họ
    TaoInput(font, INPUT_HO_DOC_GIA, labelX, currentY, INPUT_RONG, INPUT_CAO, "Ho (*):", s.chuoiHo, goiYHo);
    currentY += INPUT_CAO + inputSpacing;
    
    // Input Tên
    TaoInput(font, INPUT_TEN_DOC_GIA, labelX, currentY, INPUT_RONG, INPUT_CAO, "Ten (*):", s.chuoiTen, goiYTen);
    currentY += INPUT_CAO + inputSpacing;
    
    // ===== RADIO BUTTON: PHÁI =====
    sf::Text lblPhai = TaoVanBan(font, "Phai (*):", FONT_SIZE_BINH_THUONG, MAU_NHAN);
    lblPhai.setPosition(labelX, currentY);
    window.draw(lblPhai);
    
    float radioX = labelX + 110.f;
    float radioSize = 15.f;
    
    // Radio Nam
    sf::CircleShape radioNam(radioSize / 2.f);
    radioNam.setPosition(radioX, currentY + 5.f);
    radioNam.setFillColor(sf::Color::Transparent);
    radioNam.setOutlineThickness(2.f);
    radioNam.setOutlineColor(MAU_VIEN);
    window.draw(radioNam);
    
    if (s.phaiDuocChon == 0) { // Nam được chọn
        sf::CircleShape innerNam(radioSize / 4.f);
        innerNam.setPosition(radioX + radioSize / 4.f, currentY + 5.f + radioSize / 4.f);
        innerNam.setFillColor(MAU_NHAN);
        window.draw(innerNam);
    }
    
    sf::Text txtNam = TaoVanBan(font, "Nam", FONT_SIZE_BINH_THUONG, MAU_CHU);
    txtNam.setPosition(radioX + radioSize + 5.f, currentY);
    window.draw(txtNam);
    
    // Radio Nữ
    radioX += 100.f;
    sf::CircleShape radioNu(radioSize / 2.f);
    radioNu.setPosition(radioX, currentY + 5.f);
    radioNu.setFillColor(sf::Color::Transparent);
    radioNu.setOutlineThickness(2.f);
    radioNu.setOutlineColor(MAU_VIEN);
    window.draw(radioNu);
    
    if (s.phaiDuocChon == 1) { // Nữ được chọn
        sf::CircleShape innerNu(radioSize / 4.f);
        innerNu.setPosition(radioX + radioSize / 4.f, currentY + 5.f + radioSize / 4.f);
        innerNu.setFillColor(MAU_NHAN);
        window.draw(innerNu);
    }
    
    sf::Text txtNu = TaoVanBan(font, "Nu", FONT_SIZE_BINH_THUONG, MAU_CHU);
    txtNu.setPosition(radioX + radioSize + 5.f, currentY);
    window.draw(txtNu);
    
    currentY += INPUT_CAO + inputSpacing;
    
    // ===== RADIO BUTTON: TRẠNG THÁI =====
    sf::Text lblTrangThai = TaoVanBan(font, "Trang Thai (*):", FONT_SIZE_BINH_THUONG, MAU_NHAN);
    lblTrangThai.setPosition(labelX, currentY);
    window.draw(lblTrangThai);
    
    radioX = labelX + 110.f;
    
    // Radio Hoạt động
    sf::CircleShape radioHD(radioSize / 2.f);
    radioHD.setPosition(radioX, currentY + 5.f);
    radioHD.setFillColor(sf::Color::Transparent);
    radioHD.setOutlineThickness(2.f);
    radioHD.setOutlineColor(MAU_VIEN);
    window.draw(radioHD);
    
    if (s.trangThaiDuocChon == 1) {
        sf::CircleShape innerHD(radioSize / 4.f);
        innerHD.setPosition(radioX + radioSize / 4.f, currentY + 5.f + radioSize / 4.f);
        innerHD.setFillColor(MAU_THANH_CONG);
        window.draw(innerHD);
    }
    
    sf::Text txtHD = TaoVanBan(font, "Hoat dong", FONT_SIZE_BINH_THUONG, MAU_CHU);
    txtHD.setPosition(radioX + radioSize + 5.f, currentY);
    window.draw(txtHD);
    
    // Radio Khóa
    radioX += 140.f;
    sf::CircleShape radioKhoa(radioSize / 2.f);
    radioKhoa.setPosition(radioX, currentY + 5.f);
    radioKhoa.setFillColor(sf::Color::Transparent);
    radioKhoa.setOutlineThickness(2.f);
    radioKhoa.setOutlineColor(MAU_VIEN);
    window.draw(radioKhoa);
    
    if (s.trangThaiDuocChon == 0) {
        sf::CircleShape innerKhoa(radioSize / 4.f);
        innerKhoa.setPosition(radioX + radioSize / 4.f, currentY + 5.f + radioSize / 4.f);
        innerKhoa.setFillColor(MAU_LOI);
        window.draw(innerKhoa);
    }
    
    sf::Text txtKhoa = TaoVanBan(font, "Khoa", FONT_SIZE_BINH_THUONG, MAU_CHU);
    txtKhoa.setPosition(radioX + radioSize + 5.f, currentY);
    window.draw(txtKhoa);
    
    currentY += INPUT_CAO + PADDING * 1.5f;
    
    // ===== NÚT THÊM/SỬA =====
    float totalWidth = FORM_RONG - 2 * PADDING;
    float nutRong = (totalWidth - PADDING) / 2;
    float nutCao = NUT_CAO + 10.f;
    std::string nutChinhLabel = s.dangSua ? "LUU THAY DOI" : "THEM DOC GIA";
    
    if (s.dangSua) {
        TaoNut(font, NUT_LUU_DOC_GIA, labelX, currentY, nutRong, nutCao, nutChinhLabel, MAU_NHAN, MAU_CHU_NUT);
        TaoNut(font, NUT_HUY_DOC_GIA, labelX + nutRong + PADDING, currentY, nutRong, nutCao, "HUY", MAU_NEN_NUT, MAU_CHU_NUT);
    } else {
        float nutRongDon = 250.f;
        float nutXDon = labelX + (totalWidth - nutRongDon) / 2;
        TaoNut(font, NUT_THEM_DOC_GIA, nutXDon, currentY, nutRongDon, nutCao, nutChinhLabel, MAU_NHAN, MAU_CHU_NUT);
    }
}

// ===============================================
// PHẦN 4: THÔNG BÁO & MODAL
// ===============================================

static void VeKhungThongBaoDocGia(sf::RenderWindow &window, const sf::Font &font, const DocGiaState& s) {
    VeKhung(window, FORM_X, KHUNG_THONG_BAO_Y, FORM_RONG, KHUNG_THONG_BAO_CAO, "THONG BAO", font);
    
    if (s.xacNhanXoa) {
        sf::Text txtConfirm = TaoVanBan(font, 
            "Ban co chac muon xoa Doc Gia nay?\n(Chi xoa duoc neu chua muon sach)", 
            FONT_SIZE_BINH_THUONG, MAU_LOI);
        txtConfirm.setPosition(FORM_X + PADDING, KHUNG_THONG_BAO_Y + 35.f);
        window.draw(txtConfirm);
        
        float buttonY = KHUNG_THONG_BAO_Y + 85.f;
        float totalWidth = FORM_RONG - 2 * PADDING;
        float nutRong = (totalWidth - PADDING) / 2;
        
        TaoNut(font, NUT_XAC_NHAN_XOA_DG, FORM_X + PADDING, buttonY, nutRong, NUT_CAO, "XAC NHAN XOA", MAU_LOI, MAU_CHU_NUT);
        TaoNut(font, NUT_HUY_XOA_DG, FORM_X + PADDING + nutRong + PADDING, buttonY, nutRong, NUT_CAO, "HUY", MAU_NEN_NUT, MAU_CHU_NUT);
    } else {
        // Hiển thị thông báo bình thường (từ biến global noiDungThongBao)
        if (noiDungThongBao.empty()) return;
        
        sf::Color mauChu = MAU_CHU;
        if (loaiThongBao == 1) mauChu = MAU_LOI;
        else if (loaiThongBao == 2) mauChu = MAU_THANH_CONG;
        
        float maxTextWidth = FORM_RONG - 2 * PADDING - 10.f;
        std::string wrappedText = WordWrapText(font, noiDungThongBao, FONT_SIZE_BINH_THUONG, maxTextWidth);
        
        sf::Text txtMsg = TaoVanBan(font, wrappedText, FONT_SIZE_BINH_THUONG, mauChu);
        txtMsg.setPosition(FORM_X + PADDING, KHUNG_THONG_BAO_Y + 35.f);
        window.draw(txtMsg);
    }
}

static void VeModalChiTietMuon(sf::RenderWindow &window, const sf::Font &font, const DocGiaState& s) {
    // Vẽ overlay
    sf::RectangleShape overlay(sf::Vector2f(CHIEU_RONG, CHIEU_CAO));
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(overlay);
    
    float modalRong = 700.f;
    float modalCao = 400.f;
    float modalX = (CHIEU_RONG - modalRong) / 2.f;
    float modalY = (CHIEU_CAO - modalCao) / 2.f;
    float padding = 20.f;
    
    VeKhung(window, modalX, modalY, modalRong, modalCao, "SACH DANG MUON - " + s.tenDocGiaDuocChon, font);
    
    // Hiển thị số lượng
    sf::Text infoText = TaoVanBan(font, "Tong: " + std::to_string(s.soLuongSachDangMuon) + "/3 cuon", FONT_SIZE_NHO, MAU_NHAN);
    infoText.setPosition(modalX + modalRong - 150.f, modalY + 35.f);
    window.draw(infoText);
    
    // Header
    float currentY = modalY + 60.f;
    float col1X = modalX + padding;
    float col2X = col1X + 50.f;
    float col3X = col2X + 200.f;
    float col4X = col3X + 250.f;
    
    sf::Text header = TaoVanBan(font, "STT", FONT_SIZE_BINH_THUONG, MAU_NHAN);
    header.setPosition(col1X, currentY);
    window.draw(header);
    header.setString("Ma Sach");
    header.setPosition(col2X, currentY);
    window.draw(header);
    header.setString("Ten Sach");
    header.setPosition(col3X, currentY);
    window.draw(header);
    header.setString("Ngay Muon");
    header.setPosition(col4X, currentY);
    window.draw(header);
    
    currentY += 30.f;
    
    // Data rows
    sf::Text dataText = TaoVanBan(font, "", FONT_SIZE_NHO, MAU_CHU);
    for (int i = 0; i < s.soLuongSachDangMuon; ++i) {
        dataText.setString(std::to_string(i + 1));
        dataText.setPosition(col1X, currentY);
        window.draw(dataText);
        
        dataText.setString(s.dsSachDangMuon[i].maSach);
        dataText.setPosition(col2X, currentY);
        window.draw(dataText);
        
        dataText.setString(CatChuoi(s.dsSachDangMuon[i].tenSach, 25));
        dataText.setPosition(col3X, currentY);
        window.draw(dataText);
        
        dataText.setString(s.dsSachDangMuon[i].ngayMuon);
        dataText.setPosition(col4X, currentY);
        window.draw(dataText);
        
        currentY += 25.f;
    }
    
    // Nút đóng
    float btnY = modalY + modalCao - 60.f;
    TaoNut(font, NUT_DONG_MODAL_DG, modalX + modalRong - 130.f - padding, btnY, 130.f, 40.f, "DONG", MAU_LOI, MAU_CHU_NUT);
}

// ===============================================
// PHẦN 5: HÀM VẼ CHÍNH
// ===============================================

void VeManHinhQuanLyDocGia(sf::RenderWindow &window, const sf::Font &font) {
    // Thanh tiêu đề
    sf::RectangleShape topBar(sf::Vector2f(CHIEU_RONG, THANH_TAB_CAO));
    topBar.setFillColor(MAU_KHUNG);
    window.draw(topBar);
    
    sf::Text title = TaoVanBan(font, "QUAN LY DOC GIA", FONT_SIZE_BINH_THUONG, MAU_TIEU_DE);
    title.setPosition(PADDING, PADDING / 2.f - title.getLocalBounds().height / 2.f);
    window.draw(title);
    
    // Nút quay về menu
    TaoNut(font, NUT_BACK, CHIEU_RONG - PADDING - 100.f, PADDING / 4.f, 100.f, NUT_CAO * 0.8f, "< MENU", MAU_NUT_BACK, MAU_CHU_NUT);
    
    // Các nút chuyển chế độ xem
    float nutY = BANG_Y;
    float nutX = BANG_X;
    float nutRong = 150.f;
    
    TaoNut(font, NUT_XEM_THEO_MA, nutX, nutY, nutRong, NUT_CAO, "Theo Ma The", MAU_NEN_NUT, MAU_CHU_NUT);
    nutX += nutRong + 10.f;
    TaoNut(font, NUT_XEM_THEO_TEN, nutX, nutY, nutRong, NUT_CAO, "Theo Ten+Ho", MAU_NEN_NUT, MAU_CHU_NUT);
    nutX += nutRong + 10.f;
    TaoNut(font, NUT_XEM_QUA_HAN, nutX, nutY, nutRong + 50.f, NUT_CAO, "Doc Gia Qua Han", MAU_LOI, MAU_CHU_NUT);
    
    // Vẽ các thành phần
    VeBangDocGia(window, font, state);
    VeFormNhapDocGia(window, font, state);
    VeKhungThongBaoDocGia(window, font, state);
    
    if (state.hienThiModalChiTiet) {
        VeModalChiTietMuon(window, font, state);
    }
}

// ===============================================
// PHẦN 6: CẬP NHẬT DỮ LIỆU
// ===============================================

static void CapNhatDuLieuHienThi(DocGiaState& s) {
    extern PTRDG rootDocGia;
    s.soLuongDocGia = 0;
    s.soLuongQuaHan = 0;
    
    if (s.cheDoXem == XEM_DANH_SACH_MA_THE) {
        // InOrder traversal → tự động tăng dần mã thẻ
        DuyetCayRaMang(rootDocGia, s.mangDocGia, s.soLuongDocGia);
    } 
    else if (s.cheDoXem == XEM_DANH_SACH_TEN_HO) {
        // Duyệt → mảng → QuickSort theo Tên+Họ
        DuyetCayRaMang(rootDocGia, s.mangDocGia, s.soLuongDocGia);
        if (s.soLuongDocGia > 1) {
            QuickSortDocGia(s.mangDocGia, 0, s.soLuongDocGia - 1);
        }
    }
    else if (s.cheDoXem == XEM_DOC_GIA_QUA_HAN) {
        // Lay danh sach qua han - wrapper de dung ThongTinQuaHan
        DocGiaQuaHanDTO temp[MAX_DAUSACH];
        int count = LayDSDocGiaQuaHan(rootDocGia, temp, MAX_DAUSACH);
        s.soLuongQuaHan = count;
        for (int i = 0; i < count; ++i) {
            s.mangQuaHan[i].docGia = temp[i].docGia;
            s.mangQuaHan[i].soNgayQuaHanMax = temp[i].soNgayQuaHanMax;
        }
    }
    
    // Cập nhật phân trang
    int totalItems = (s.cheDoXem == XEM_DOC_GIA_QUA_HAN) ? s.soLuongQuaHan : s.soLuongDocGia;
    s.tongSoTrang = (totalItems + SACH_MOI_TRANG - 1) / SACH_MOI_TRANG;
    if (s.tongSoTrang == 0) s.tongSoTrang = 1;
    if (s.trangHienTai > s.tongSoTrang) s.trangHienTai = s.tongSoTrang;
}

// ===============================================
// PHẦN 7: XỬ LÝ THÊM/SỬA/XÓA
// ===============================================

static void ThucHienThemHoacSuaDocGia(DocGiaState& s) {
    extern PTRDG rootDocGia;
    extern bool duLieuDaThayDoi;
    
    // Validate
    std::string ho = CatKhoangTrang(s.chuoiHo);
    std::string ten = CatKhoangTrang(s.chuoiTen);
    
    std::string loi = KiemTraChuoiVaDodai(ho, "Ho", MAX_TAC_GIA);
    if (!loi.empty()) {
        CapNhatThongBaoSFML(loi, 1);
        inputHoatDong = INPUT_HO_DOC_GIA;
        return;
    }
    
    loi = KiemTraChuoiVaDodai(ten, "Ten", MAX_TAC_GIA);
    if (!loi.empty()) {
        CapNhatThongBaoSFML(loi, 1);
        inputHoatDong = INPUT_TEN_DOC_GIA;
        return;
    }
    
    if (!s.dangSua) {
        // THÊM MỚI
        PTRDG dg = taoDocGia(ho, ten, s.phaiDuocChon != 0, s.trangThaiDuocChon, rootDocGia);
        if (dg == nullptr || dg->data.MaThe == -1) {
            CapNhatThongBaoSFML("Loi: Khong the sinh ma the!", 1);
            return;
        }
        
        InsertDocGia(rootDocGia, dg);
        duLieuDaThayDoi = true;
        
        CapNhatThongBaoSFML("Them thanh cong: " + ho + " " + ten + " (Ma: " + std::to_string(dg->data.MaThe) + ")", 2);
        
        // Reset form
        s.chuoiHo = "";
        s.chuoiTen = "";
        s.phaiDuocChon = 0;
        s.trangThaiDuocChon = 1;
        
        CapNhatDuLieuHienThi(s);
        inputHoatDong = KHONG_XAC_DINH;
    } 
    else {
        // CẬP NHẬT
        PTRDG dg = timDocGia(rootDocGia, s.maTheDocGiaDuocChon);
        if (dg == nullptr) {
            CapNhatThongBaoSFML("Loi: Khong tim thay doc gia!", 1);
            return;
        }
        
        TheDocGia newData;
        newData.MaThe = s.maTheDocGiaDuocChon;
        newData.Ho = ho;
        newData.Ten = ten;
        newData.Phai = (s.phaiDuocChon != 0);
        newData.TrangThai = s.trangThaiDuocChon;
        
        hieuChinhDocGia(rootDocGia, newData);
        duLieuDaThayDoi = true;
        
        CapNhatThongBaoSFML("Cap nhat thanh cong: " + ho + " " + ten, 2);
        
        // Reset
        s.dangSua = false;
        s.chuoiHo = "";
        s.chuoiTen = "";
        s.maTheDocGiaDuocChon = -1;
        
        CapNhatDuLieuHienThi(s);
        inputHoatDong = KHONG_XAC_DINH;
    }
}

static void ThucHienXoaDocGia(DocGiaState& s) {
    extern PTRDG rootDocGia;
    extern bool duLieuDaThayDoi;
    
    if (s.maTheDocGiaDuocChon == -1) {
        CapNhatThongBaoSFML("Loi: Chua chon doc gia de xoa!", 1);
        return;
    }
    
    PTRDG dg = timDocGia(rootDocGia, s.maTheDocGiaDuocChon);
    if (dg == nullptr) {
        CapNhatThongBaoSFML("Loi: Khong tim thay doc gia!", 1);
        return;
    }
    
    // Kiểm tra còn sách đang mượn
    if (dg->data.soSachDangMuon > 0) {
        CapNhatThongBaoSFML("Khong the xoa! Doc gia dang muon " + std::to_string(dg->data.soSachDangMuon) + " cuon sach.", 1);
        return;
    }
    
    // Xóa
    xoaDocGia(rootDocGia, s.maTheDocGiaDuocChon);
    duLieuDaThayDoi = true;
    
    CapNhatThongBaoSFML("Xoa thanh cong: " + s.tenDocGiaDuocChon, 2);
    
    s.maTheDocGiaDuocChon = -1;
    s.tenDocGiaDuocChon = "";
    s.xacNhanXoa = false;
    
    CapNhatDuLieuHienThi(s);
}

// ===============================================
// PHẦN 8: XỬ LÝ SỰ KIỆN
// ===============================================

void XuLySuKienManHinhDocGia(sf::RenderWindow &window, sf::Event event) {
    (void)window;
    extern PTRDG rootDocGia;
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    // Xử lý modal
    if (state.hienThiModalChiTiet) {
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            MaUI element = LayElementTaiToaDo(event.mouseButton.x, event.mouseButton.y);
            if (element == NUT_DONG_MODAL_DG) {
                state.hienThiModalChiTiet = false;
            }
        }
        return;
    }
    
    // Xử lý text input
    if (event.type == sf::Event::TextEntered || 
        (event.type == sf::Event::KeyPressed && 
         (event.key.code == sf::Keyboard::Backspace || 
          event.key.code == sf::Keyboard::Tab ||
          event.key.code == sf::Keyboard::Z))) {
        if (inputHoatDong == INPUT_HO_DOC_GIA || inputHoatDong == INPUT_TEN_DOC_GIA) {
            // Gọi hàm XuLyTextInput từ ManHinhQuanLySach (tái sử dụng)
            XuLyTextInputDocGia(event, state);
        }
    }
    
    // Xử lý mouse click
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        MaUI element = LayElementTaiToaDo(event.mouseButton.x, event.mouseButton.y);
        
        // Click vào input
        if (element == INPUT_HO_DOC_GIA || element == INPUT_TEN_DOC_GIA) {
            inputHoatDong = element;
            CapNhatThongBaoSFML("", 0);
            return;
        }
        
        // Click ra ngoài input
        if (inputHoatDong != KHONG_XAC_DINH && element != INPUT_HO_DOC_GIA && element != INPUT_TEN_DOC_GIA) {
            inputHoatDong = KHONG_XAC_DINH;
        }
        
        // Xử lý các nút
        switch (element) {
            case NUT_BACK:
                if (state.dangSua) {
                    state.dangSua = false;
                    state.chuoiHo = "";
                    state.chuoiTen = "";
                    CapNhatThongBaoSFML("Da huy hieu chinh.", 0);
                } else {
                    manHinhHienTai = MENU_CHINH;
                    state.maTheDocGiaDuocChon = -1;
                    state.cheDoXem = XEM_DANH_SACH_MA_THE;
                    CapNhatThongBaoSFML("", 0);
                }
                break;
                
            case NUT_XEM_THEO_MA:
                state.cheDoXem = XEM_DANH_SACH_MA_THE;
                state.trangHienTai = 1;
                state.maTheDocGiaDuocChon = -1;
                CapNhatDuLieuHienThi(state);
                CapNhatThongBaoSFML("Hien thi theo ma the tang dan.", 0);
                break;
                
            case NUT_XEM_THEO_TEN:
                state.cheDoXem = XEM_DANH_SACH_TEN_HO;
                state.trangHienTai = 1;
                state.maTheDocGiaDuocChon = -1;
                CapNhatDuLieuHienThi(state);
                CapNhatThongBaoSFML("Hien thi theo ten + ho tang dan.", 0);
                break;
                
            case NUT_XEM_QUA_HAN:
                state.cheDoXem = XEM_DOC_GIA_QUA_HAN;
                state.trangHienTai = 1;
                state.maTheDocGiaDuocChon = -1;
                CapNhatDuLieuHienThi(state);
                if (state.soLuongQuaHan > 0) {
                    CapNhatThongBaoSFML("Tim thay " + std::to_string(state.soLuongQuaHan) + " doc gia qua han.", 1);
                } else {
                    CapNhatThongBaoSFML("Khong co doc gia qua han.", 0);
                }
                break;
                
            case NUT_THEM_DOC_GIA:
            case NUT_LUU_DOC_GIA:
                inputHoatDong = KHONG_XAC_DINH;
                ThucHienThemHoacSuaDocGia(state);
                break;
                
            case NUT_HUY_DOC_GIA:
                state.dangSua = false;
                state.chuoiHo = "";
                state.chuoiTen = "";
                CapNhatThongBaoSFML("Da huy.", 0);
                break;
                
            case NUT_CHI_TIET_DOC_GIA:
                if (state.maTheDocGiaDuocChon != -1) {
                    PTRDG dg = timDocGia(rootDocGia, state.maTheDocGiaDuocChon);
                    if (dg) {
                        state.soLuongSachDangMuon = LayDSSachDangMuon(dg, state.dsSachDangMuon, 10, dsDauSach, soLuongDauSach);
                        state.hienThiModalChiTiet = true;
                    }
                }
                break;
                
            case NUT_SUA_DOC_GIA:
                if (state.maTheDocGiaDuocChon != -1) {
                    PTRDG dg = timDocGia(rootDocGia, state.maTheDocGiaDuocChon);
                    if (dg) {
                        state.dangSua = true;
                        state.chuoiHo = dg->data.Ho;
                        state.chuoiTen = dg->data.Ten;
                        state.phaiDuocChon = dg->data.Phai ? 1 : 0;
                        state.trangThaiDuocChon = dg->data.TrangThai;
                        inputHoatDong = INPUT_HO_DOC_GIA;
                        CapNhatThongBaoSFML("Dang hieu chinh doc gia...", 0);
                    }
                }
                break;
                
            case NUT_XOA_DOC_GIA:
                if (state.maTheDocGiaDuocChon != -1) {
                    PTRDG dg = timDocGia(rootDocGia, state.maTheDocGiaDuocChon);
                    if (dg && dg->data.soSachDangMuon > 0) {
                        CapNhatThongBaoSFML("Khong the xoa! Doc gia dang muon " + std::to_string(dg->data.soSachDangMuon) + " cuon.", 1);
                    } else if (dg) {
                        state.xacNhanXoa = true;
                        CapNhatThongBaoSFML("", 0);
                    }
                }
                break;
                
            case NUT_XAC_NHAN_XOA_DG:
                ThucHienXoaDocGia(state);
                break;
                
            case NUT_HUY_XOA_DG:
                state.xacNhanXoa = false;
                CapNhatThongBaoSFML("Da huy xoa.", 0);
                break;
                
            case NUT_HUY_CHON_DOC_GIA:
                state.maTheDocGiaDuocChon = -1;
                state.tenDocGiaDuocChon = "";
                state.dangSua = false;
                state.chuoiHo = "";
                state.chuoiTen = "";
                CapNhatThongBaoSFML("Da huy chon.", 0);
                break;
                
            case NUT_TRANG_TRUOC_DG:
                if (state.trangHienTai > 1) state.trangHienTai--;
                break;
                
            case NUT_TRANG_SAU_DG:
                if (state.trangHienTai < state.tongSoTrang) state.trangHienTai++;
                break;
                
            case HANG_SACH:
            {
                // Double-click để chọn dòng
                float contentY = BANG_Y + 90 + 35;
                float rowHeight = 30.f;
                int startIndex = (state.trangHienTai - 1) * SACH_MOI_TRANG;
                int rowIndex = static_cast<int>((event.mouseButton.y - contentY) / rowHeight);
                int actualIndex = startIndex + rowIndex;
                
                PTRDG dg = nullptr;
                if (state.cheDoXem == XEM_DOC_GIA_QUA_HAN) {
                    if (actualIndex >= 0 && actualIndex < state.soLuongQuaHan) {
                        dg = state.mangQuaHan[actualIndex].docGia;
                    }
                } else {
                    if (actualIndex >= 0 && actualIndex < state.soLuongDocGia) {
                        dg = state.mangDocGia[actualIndex];
                    }
                }
                
                if (dg) {
                    int maTheClicked = dg->data.MaThe;
                    float elapsed = state.doubleClickClock.getElapsedTime().asSeconds();
                    
                    if (elapsed < state.THOI_GIAN_DOUBLE_CLICK && maTheClicked == state.maTheClickCuoi) {
                        // Double-click
                        state.maTheDocGiaDuocChon = maTheClicked;
                        state.tenDocGiaDuocChon = dg->data.Ho + " " + dg->data.Ten;
                        CapNhatThongBaoSFML("Da chon: " + state.tenDocGiaDuocChon, 0);
                        state.maTheClickCuoi = -1;
                    } else {
                        // Single-click
                        state.maTheClickCuoi = maTheClicked;
                        state.doubleClickClock.restart();
                    }
                }
                break;
            }
            default:
                break;
        }
    }
    
    // Xử lý ESC
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
        if (inputHoatDong != KHONG_XAC_DINH) {
            inputHoatDong = KHONG_XAC_DINH;
        } else if (state.dangSua) {
            state.dangSua = false;
            state.chuoiHo = "";
            state.chuoiTen = "";
            CapNhatThongBaoSFML("Da huy hieu chinh.", 0);
        } else if (state.xacNhanXoa) {
            state.xacNhanXoa = false;
            CapNhatThongBaoSFML("Da huy xoa.", 0);
        } else {
            manHinhHienTai = MENU_CHINH;
            CapNhatThongBaoSFML("", 0);
        }
    }
}

void KhoiTaoManHinhDocGia() {
    CapNhatDuLieuHienThi(state);
}

// ===============================================
// PHẦN 9: XỬ LÝ TEXT INPUT (Tái sử dụng pattern)
// ===============================================

void XuLyTextInputDocGia(sf::Event event, DocGiaState& s) {
    if (inputHoatDong != INPUT_HO_DOC_GIA && inputHoatDong != INPUT_TEN_DOC_GIA) return;
    
    std::string* targetString = nullptr;
    int maxLen = MAX_TAC_GIA;
    std::string fieldName = "";
    
    if (inputHoatDong == INPUT_HO_DOC_GIA) {
        targetString = &s.chuoiHo;
        fieldName = "Ho";
    } else {
        targetString = &s.chuoiTen;
        fieldName = "Ten";
    }
    
    // Xử lý Ctrl+Z (Undo)
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Z && 
        (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl))) {
        // Logic undo tương tự ManHinhQuanLySach
        return;
    }
    
    // Xử lý TextEntered
    if (event.type == sf::Event::TextEntered) {
        if (event.text.unicode < 128 && event.text.unicode != 8 && event.text.unicode != 13 && event.text.unicode != 9) {
            char c = static_cast<char>(event.text.unicode);
            
            if (targetString->length() < static_cast<size_t>(maxLen)) {
                bool allow = false;
                
                if (isalnum(c) || c == ' ') {
                    if (targetString->empty() && !isalpha(c)) {
                        CapNhatThongBaoSFML("Loi: " + fieldName + " phai bat dau bang chu cai!", 1);
                    } else if (c == ' ' && !targetString->empty() && targetString->back() == ' ') {
                        CapNhatThongBaoSFML("Loi: Khong duoc nhap nhieu khoang trang lien tiep!", 1);
                    } else {
                        allow = true;
                    }
                }
                
                if (allow) {
                    *targetString += c;
                    if (loaiThongBao == 1) CapNhatThongBaoSFML("", 0);
                }
            } else {
                CapNhatThongBaoSFML("Loi: " + fieldName + " da dat gioi   han " + std::to_string(maxLen) + " ky tu!", 1);
            }
        }
    } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Backspace) {
        if (!targetString->empty()) {
            targetString->pop_back();
            if (loaiThongBao == 1) CapNhatThongBaoSFML("", 0);
        }
    }
    
    // Xử lý Enter/Tab
    if (event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Tab)) {
        std::string loi = KiemTraChuoiVaDodai(*targetString, fieldName, maxLen);
        
        if (loi.empty()) {
            if (inputHoatDong == INPUT_HO_DOC_GIA) {
                inputHoatDong = INPUT_TEN_DOC_GIA;
            } else {
                inputHoatDong = KHONG_XAC_DINH;
                if (event.key.code == sf::Keyboard::Enter) {
                    ThucHienThemHoacSuaDocGia(s);
                }
            }
            CapNhatThongBaoSFML("", 0);
        } else {
            CapNhatThongBaoSFML(loi, 1);
        }
    }
}