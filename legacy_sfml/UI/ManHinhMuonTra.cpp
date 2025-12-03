
#include "GiaoDienMuonTra.h"
#include "TrangThaiManHinhMuonTra.h"
#include "GiaoDienSFML.h"
#include "TienIchGiaoDien.h"
#include "DocGia.h"
#include "QuanLySach.h"
#include "XuLyChuoi.h"
#include "KiemTraDuLieu.h"
#include "NgayThang.h"
#include "Constants.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <functional>

// ===== BIẾN TRẠNG THÁI DUY NHẤT =====
static MuonTraState state;

// ===== KHAI BÁO HÀM NỘI BỘ (static) =====
static void VeBangDocGia(sf::RenderWindow &window, const sf::Font &font, const MuonTraState& currentState);
static void VeFormMuonTra(sf::RenderWindow &window, const sf::Font &font, const MuonTraState& currentState);
static void VeKhungThongBaoSFML(sf::RenderWindow &window, const sf::Font &font, const MuonTraState& currentState);
static void VeModalTop10(sf::RenderWindow &window, const sf::Font &font, MuonTraState& currentState);
static void VeModalQuaHan(sf::RenderWindow &window, const sf::Font &font, MuonTraState& currentState);

static void XuLyTextInput(sf::Event event, MuonTraState& currentState);
static void ThucHienTimKiemNoiBo(MuonTraState& currentState);
static void ThucHienMuonSachSFML(MuonTraState& currentState);
static void ThucHienTraSachSFML(MuonTraState& currentState);
static void ThucHienBaoMatSachSFML(MuonTraState& currentState);
static void TaiDuLieuTop10(MuonTraState& currentState);
static void TaiDuLieuQuaHan(MuonTraState& currentState);

static void XoaFormNhapLieuSFML(MuonTraState& currentState);
static void CapNhatPhanTrangSFML(MuonTraState& currentState);
static void CapNhatSachDangMuon(MuonTraState& currentState);
static int LayDanhSachDocGiaCoLoc(PTRDG root, DocGiaTableDTO arr[], int maxKetQua, const std::string& tuKhoa);

// ===== HÀM TIỆN ÍCH =====
static inline std::string CatChuoiVoiDauCham(const std::string& str, size_t maxLen) {
    return (str.length() > maxLen) ? str.substr(0, maxLen - 3) + "..." : str;
}

// ===== LAYOUT CONSTANTS (Đồng bộ 100% với ManHinhQuanLySach) =====
// Thu gọn bảng độc giả xuống 58%, mở rộng form mượn/trả lên 40%
const float BANG_MT_X = BANG_X;              // = PADDING
const float BANG_MT_Y = BANG_Y + 50.f;       // = THANH_TAB_CAO + PADDING + 50 (sau search bar)
const float BANG_MT_RONG = CHIEU_RONG * 0.58f;  // Thu gọn từ 70% xuống 58%
const float BANG_MT_CAO = CHIEU_CAO - BANG_MT_Y - KHUNG_THONG_BAO_CAO - PADDING * 3 - NUT_CAO * 2;

const float FORM_MT_X = BANG_MT_X + BANG_MT_RONG + PADDING * 2.5f;  // Tăng khoảng cách
const float FORM_MT_Y = FORM_Y;              // = BANG_Y + 50
const float FORM_MT_RONG = CHIEU_RONG - FORM_MT_X - PADDING;  // Tự động tính
const float FORM_MT_CAO = KHUNG_THONG_BAO_Y - FORM_MT_Y - PADDING;

// ===== ĐỊNH NGHĨA HÀM VẼ =====

// ----- VẼ BẢNG ĐỌC GIẢ (Layout đồng bộ ManHinhQuanLySach) -----
static void VeBangDocGia(sf::RenderWindow &window, const sf::Font &font, const MuonTraState& currentState){
    // Search bar ở trên cùng (giống ManHinhQuanLySach)
    float searchY = BANG_Y;  // Đúng vị trí như màn hình sách
    std::string goiYTimKiem = currentState.chuoiTimKiem.empty() ? "Nhap ten, ma the de tim kiem..." : "";
    TaoInput(font, INPUT_MT_TIM_DOC_GIA, BANG_MT_X, searchY, 500.f, INPUT_CAO, 
             "Tim Kiem:", currentState.chuoiTimKiem, goiYTimKiem);
    
    // Các nút cùng hàng với search (TIM, XOA TIM style)
    float btnX = BANG_MT_X + 110.f + 500.f + 10.f;
    // TaoNut(font, NUT_TIM, btnX, searchY, 100.f, NUT_CAO, "TIM", MAU_NHAN, MAU_CHU_NUT);
    // btnX += 100.f + 10.f;
    TaoNut(font, NUT_XOA_TIM, btnX, searchY, 100.f, NUT_CAO, "XOA TIM", MAU_NEN_NUT, MAU_CHU_NUT);
    btnX += 100.f + 10.f;
    TaoNut(font, NUT_MT_VAO_TOP_10, btnX, searchY, 110.f, NUT_CAO, "TOP 10", MAU_NHAN, MAU_CHU_NUT);
    btnX += 110.f + 10.f;
    TaoNut(font, NUT_MT_TAB_QUAHAN, btnX, searchY, 110.f, NUT_CAO, "QUA HAN", MAU_LOI, MAU_CHU_NUT);
    
    float headerY = BANG_MT_Y;
    float contentY = headerY + 40.f;
    float tableBottom = CHIEU_CAO - (PADDING * 5.5f) - (NUT_CAO * 2);

    VeKhung(window, BANG_MT_X, headerY, BANG_MT_RONG, tableBottom - headerY, 
            "DANH SACH DOC GIA (Double-click de chon)", font);

    // Tỷ lệ cột tối ưu cho bảng 58%: STT nhỏ, Mã Thẻ vừa, Họ Tên rộng, Phái, Trạng Thái, Sách
    float colWidths[] = {0.05f, 0.11f, 0.38f, 0.10f, 0.22f, 0.12f};  // Tổng = 0.98
    float colX[7];
    colX[0] = BANG_MT_X + PADDING;
    for (int i = 0; i < 6; ++i) {
        colX[i + 1] = colX[i] + colWidths[i] * (BANG_MT_RONG - PADDING);
        if (i < 5) {
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

    std::string headers[] = {"STT", "Ma The", "Ho Ten", "Phai", "Trang Thai", "Sach"};

    float headerBoxHeight = 30.f;
    sf::RectangleShape headerBg(sf::Vector2f(BANG_MT_RONG - PADDING * 0.8, headerBoxHeight));
    headerBg.setPosition(BANG_MT_X + PADDING * 0.4, contentY);
    headerBg.setFillColor(MAU_BANG_HEADER);
    window.draw(headerBg);

    for (int i = 0; i < 6; ++i) {
        headerText.setString(headers[i]);
        headerText.setPosition(colX[i], contentY + (headerBoxHeight - headerText.getCharacterSize()) / 2.f);
        window.draw(headerText);
    }
    
    sf::RectangleShape headerLine(sf::Vector2f(BANG_MT_RONG - 2 * PADDING, 1.f));
    headerLine.setPosition(BANG_MT_X + PADDING, contentY + 30.f);
    headerLine.setFillColor(MAU_BANG_BORDER);
    window.draw(headerLine);


    // Định nghĩa hitbox bắt đầu ngay từ đường kẻ dưới header.
    float tableBodyTop = contentY + 30.f; // Tọa độ Y bắt đầu vùng dữ liệu
    float tableBodyHeight = tableBottom - tableBodyTop; // Chiều cao vùng dữ liệu

    // Tạo vùng hình chữ nhật ảo (không cần fill color vì chỉ dùng để bắt click)
    sf::RectangleShape tableHitbox(sf::Vector2f(BANG_MT_RONG, tableBodyHeight));
    tableHitbox.setPosition(BANG_MT_X, tableBodyTop);

    tableHitbox.setFillColor(sf::Color::Transparent);

    // Đăng ký vào hệ thống UI
    UIElement elemTable;
    elemTable.hinhDang = tableHitbox;
    elemTable.id = HANG_SACH; // Gán ID quan trọng này

    // Đẩy vào mảng quản lý element toàn cục
    if (soLuongElement < SO_ELEMENT_TOI_DA) {
        cacElement[soLuongElement++] = elemTable;
    }

    float currentY = contentY + 35.f;
    int startIndex = (currentState.trangHienTai - 1) * DOC_GIA_MOI_TRANG;
    int endIndex = (startIndex + DOC_GIA_MOI_TRANG < currentState.soLuongKetQuaTimKiem)
                       ? (startIndex + DOC_GIA_MOI_TRANG)
                       : currentState.soLuongKetQuaTimKiem;
    
    const float ROW_HEIGHT = 30.f;

    // TOI UU: Cache highlight shape de tai su dung
    sf::RectangleShape highlight(sf::Vector2f(BANG_MT_RONG - PADDING * 0.8, ROW_HEIGHT));
    highlight.setFillColor(MAU_NHAN);
    
    for (int i = startIndex; i < endIndex && currentY <= tableBottom - PADDING; ++i) {
        PTRDG dg = currentState.ketQuaTimKiem[i].docGia;
        if (!dg) continue;

        bool isSelected = (dg->data.MaThe == currentState.maTheDocGiaDuocChon);
        if (isSelected) {
            highlight.setPosition(BANG_MT_X + PADDING * 0.4, currentY - 2.f);
            window.draw(highlight);
        }

        sf::Text dataText;
        dataText.setFont(font);
        dataText.setCharacterSize(FONT_SIZE_BINH_THUONG);
        dataText.setFillColor(isSelected ? sf::Color::Black : MAU_CHU);

        const float textY = currentY + (ROW_HEIGHT - dataText.getCharacterSize()) / 2.f;
        
        // STT
        dataText.setString(std::to_string(i + 1));
        dataText.setPosition(colX[0], textY);
        window.draw(dataText);

        // Ma The
        dataText.setString(std::to_string(dg->data.MaThe));
        dataText.setPosition(colX[1], textY);
        window.draw(dataText);

        // Ho Ten
        std::string hoTen = dg->data.Ho + " " + dg->data.Ten;
        dataText.setString(CatChuoiVoiDauCham(hoTen, 25));
        dataText.setPosition(colX[2], textY);
        window.draw(dataText);

        // Phai
        dataText.setString(dg->data.Phai == 0 ? "Nam" : "Nu");
        dataText.setPosition(colX[3], textY);
        window.draw(dataText);

        // Trang Thai với color
        std::string trangThai = (dg->data.TrangThai == 0) ? "Khoa" : "Hoat dong";
        sf::Color colorTT = (dg->data.TrangThai == 1) ? MAU_THANH_CONG : MAU_LOI;
        dataText.setFillColor(colorTT);
        dataText.setString(trangThai);
        dataText.setPosition(colX[4], textY);
        window.draw(dataText);

        // Cột Sách (số sách đang mượn)
        int soSachMuon = dg->data.soSachDangMuon;
        if (soSachMuon == 0 && dg->data.dsmt) {
            MUONTRA p = dg->data.dsmt;
            while (p && soSachMuon < 10) {
                if (p->data.TrangThai == 0) soSachMuon++;
                p = p->next;
            }
        }
        std::string sachText = std::to_string(soSachMuon) + "/3";
        sf::Color sachColor = MAU_CHU;
        if (soSachMuon >= 3) sachColor = MAU_LOI;
        else if (soSachMuon >= 2) sachColor = MAU_VIEN;
        dataText.setFillColor(sachColor);
        dataText.setString(sachText);
        dataText.setPosition(colX[5], textY);
        window.draw(dataText);

        currentY += ROW_HEIGHT;
    }

    // Pagination controls + HỦY CHỌN cùng hàng
    float phanTrangY = CHIEU_CAO - PADDING * 4.f - NUT_CAO * 1.8f;
    float btnPrevX = BANG_MT_X + PADDING;
    
    TaoNut(font, NUT_MT_PREV_PAGE, btnPrevX, phanTrangY, 130.f, NUT_CAO, "<< Trang Truoc", MAU_NEN_NUT, MAU_CHU_NUT);
    TaoNut(font, NUT_MT_NEXT_PAGE, btnPrevX + 138.f, phanTrangY, 130.f, NUT_CAO, "Trang Sau >>", MAU_NEN_NUT, MAU_CHU_NUT);
    
    // Page counter
    std::string trangText = "Trang " + std::to_string(currentState.trangHienTai) + " / " + std::to_string(currentState.tongSoTrang);
    sf::Text txtTrang = TaoVanBan(font, trangText, FONT_SIZE_BINH_THUONG, MAU_CHU);
    txtTrang.setPosition(btnPrevX + 276.f, phanTrangY + 8.f);
    window.draw(txtTrang);
    
    // Statistics display
    std::string statsText = "Tong: " + std::to_string(currentState.soLuongKetQuaTimKiem) + " doc gia";
    sf::Text txtStats = TaoVanBan(font, statsText, FONT_SIZE_NHO, sf::Color(150, 150, 160));
    txtStats.setPosition(btnPrevX + 400.f, phanTrangY + 10.f);
    window.draw(txtStats);
    
    // Nút HỦY CHỌN cùng hàng bên phải
    if (currentState.docGiaDangChon) {
        float huyChonX = BANG_MT_X + BANG_MT_RONG - 160.f;
        TaoNut(font, NUT_HUY_CHON, huyChonX, phanTrangY, 160.f, NUT_CAO, 
               "HUY CHON DOC GIA", sf::Color(100, 100, 110), MAU_CHU_NUT);
    }
}

// ----- VẼ FORM MƯỢN/TRẢ (Style đồng bộ ManHinhQuanLySach) -----
static void VeFormMuonTra(sf::RenderWindow &window, const sf::Font &font, const MuonTraState& currentState) {
    float formHeight = KHUNG_THONG_BAO_Y - FORM_MT_Y - PADDING;
    VeKhung(window, FORM_MT_X, FORM_MT_Y, FORM_MT_RONG, formHeight, "MUON / TRA SACH", font);
    
    float currentY = FORM_MT_Y + 40.f;
    float labelX = FORM_MT_X + PADDING;
    float contentWidth = FORM_MT_RONG - 2 * PADDING;

    if (currentState.docGiaDangChon) {
        PTRDG dg = currentState.docGiaDangChon;
        
        // ===== CARD: Thông tin độc giả =====
        float cardPadding = 10.f;
        float cardHeight = 80.f;
        
        // Card border only (no background)
        sf::RectangleShape cardBorder(sf::Vector2f(contentWidth, cardHeight));
        cardBorder.setPosition(labelX, currentY);
        cardBorder.setFillColor(sf::Color::Transparent);
        cardBorder.setOutlineThickness(1.5f);
        cardBorder.setOutlineColor(sf::Color(100, 100, 120));
        window.draw(cardBorder);
        
        float cardY = currentY + cardPadding;
        
        // Header card
        sf::Text txtCardHeader = TaoVanBan(font, "DOC GIA", FONT_SIZE_NHO, MAU_NHAN);
        txtCardHeader.setStyle(sf::Text::Bold);
        txtCardHeader.setPosition(labelX + cardPadding, cardY);
        window.draw(txtCardHeader);
        cardY += 18.f;
        
        // Info lines
        std::string line1 = "Ma The: " + std::to_string(dg->data.MaThe) + 
                           "  |  " + dg->data.Ho + " " + dg->data.Ten;
        std::string line2 = "Phai: " + std::string(dg->data.Phai == 0 ? "Nam" : "Nu") +
                           "  |  Trang Thai: " + 
                           std::string(dg->data.TrangThai == 1 ? "Hoat dong" : "Khoa");
        
        sf::Text txtLine1 = TaoVanBan(font, line1, FONT_SIZE_NHO, MAU_CHU);
        txtLine1.setPosition(labelX + cardPadding, cardY);
        window.draw(txtLine1);
        
        sf::Text txtLine2 = TaoVanBan(font, line2, FONT_SIZE_NHO - 1, sf::Color(100, 100, 110));
        txtLine2.setPosition(labelX + cardPadding, cardY + 18.f);
        window.draw(txtLine2);
        
        currentY += cardHeight + 15.f;
        
        // ===== MINI TABLE: Sách đang mượn =====
        sf::Text txtSachHeader = TaoVanBan(font, "SACH DANG MUON (" + 
                                           std::to_string(currentState.slSachDangMuon) + "/3)", 
                                           FONT_SIZE_BINH_THUONG, MAU_NHAN);
        txtSachHeader.setPosition(labelX, currentY);
        window.draw(txtSachHeader);
        currentY += 25.f;
        
        float tableHeight = 120.f;  // Fixed height cho 3 rows max
        
        // Table border (transparent background)
        sf::RectangleShape tableBorder(sf::Vector2f(contentWidth, tableHeight));
        tableBorder.setPosition(labelX, currentY);
        tableBorder.setFillColor(sf::Color::Transparent);
        tableBorder.setOutlineThickness(1.f);
        tableBorder.setOutlineColor(sf::Color(80, 80, 100));
        window.draw(tableBorder);
        
        float tableY = currentY + 5.f;
        
        // Table header background (subtle)
        sf::RectangleShape headerBg(sf::Vector2f(contentWidth - 2.f, 22.f));
        headerBg.setPosition(labelX + 1.f, tableY);
        sf::Color headerColor = MAU_BANG_HEADER;
        headerColor.a = 100;  // Semi-transparent
        headerBg.setFillColor(headerColor);
        window.draw(headerBg);
        
        // Column headers
        sf::Text colHeader1 = TaoVanBan(font, "Ma Sach", FONT_SIZE_NHO, MAU_NHAN);
        colHeader1.setPosition(labelX + 8.f, tableY + 4.f);
        window.draw(colHeader1);
        
        sf::Text colHeader2 = TaoVanBan(font, "Ten Sach", FONT_SIZE_NHO, MAU_NHAN);
        colHeader2.setPosition(labelX + contentWidth * 0.35f, tableY + 4.f);
        window.draw(colHeader2);
        
        sf::Text colHeader3 = TaoVanBan(font, "Ngay Muon", FONT_SIZE_NHO, MAU_NHAN);
        colHeader3.setPosition(labelX + contentWidth * 0.75f, tableY + 4.f);
        window.draw(colHeader3);
        
        tableY += 28.f;
        
        // Table rows
        if (currentState.slSachDangMuon > 0) {
            for (int i = 0; i < currentState.slSachDangMuon && i < 3; ++i) {
                const ThongTinSachDangMuon_DTO& dto = currentState.listSachDangMuon[i];
                
                // Kiểm tra quá hạn (>7 ngày)
                bool quaHan = (dto.soNgayGiu > 7);
                bool sapQuaHan = (dto.soNgayGiu >= 5 && dto.soNgayGiu <= 7);
                
                // Row highlight if selected (khi hover hoặc đã click)
                bool isSelected = (dto.maSach == currentState.maSachDangChon);
                
                // Hover effect (check mouse position)
                bool isHovering = false;
                if (elementHover >= static_cast<MaUI>(HANG_SACH + 1000 + i) && 
                    elementHover <= static_cast<MaUI>(HANG_SACH + 1000 + i)) {
                    isHovering = true;
                }
                
                // Background color dựa trên trạng thái
                sf::Color bgColor = sf::Color::Transparent;
                if (isSelected) {
                    bgColor = sf::Color(255, 250, 205, 180);  // Vàng sáng khi chọn
                } else if (quaHan) {
                    bgColor = sf::Color(255, 100, 100, 80);  // Đỏ nhạt nếu quá hạn
                } else if (sapQuaHan) {
                    bgColor = sf::Color(255, 200, 100, 60);  // Cam nhạt nếu sắp quá hạn
                } else if (isHovering) {
                    bgColor = sf::Color(80, 80, 100, 50);  // Xám nhạt khi hover
                }
                
                if (bgColor != sf::Color::Transparent) {
                    sf::RectangleShape rowBg(sf::Vector2f(contentWidth - 2.f, 22.f));
                    rowBg.setPosition(labelX + 1.f, tableY);
                    rowBg.setFillColor(bgColor);
                    window.draw(rowBg);
                }
                
                // Store row bounds for click detection (invisible hitbox)
                sf::RectangleShape rowHitbox(sf::Vector2f(contentWidth - 2.f, 22.f));
                rowHitbox.setPosition(labelX + 1.f, tableY);
                rowHitbox.setFillColor(sf::Color::Transparent);
                UIElement elem;
                elem.hinhDang = rowHitbox;
                elem.id = static_cast<MaUI>(HANG_SACH + 1000 + i);  // Unique ID cho mỗi row
                if (soLuongElement < SO_ELEMENT_TOI_DA) {
                    cacElement[soLuongElement++] = elem;
                }
                
                // Màu text dựa trên trạng thái quá hạn
                sf::Color textColor = quaHan ? sf::Color(220, 50, 50) : (sapQuaHan ? sf::Color(255, 150, 0) : MAU_CHU);
                
                sf::Text cell1 = TaoVanBan(font, CatChuoiVoiDauCham(dto.maSach, 15), FONT_SIZE_NHO - 1, textColor);
                cell1.setPosition(labelX + 8.f, tableY + 4.f);
                window.draw(cell1);
                
                sf::Text cell2 = TaoVanBan(font, CatChuoiVoiDauCham(dto.tenSach, 22), FONT_SIZE_NHO - 1, textColor);
                cell2.setPosition(labelX + contentWidth * 0.35f, tableY + 4.f);
                window.draw(cell2);
                
                // Ngày mượn + cảnh báo quá hạn
                std::string ngayText = dto.ngayMuon;
                if (quaHan) {
                    ngayText += " (!" + std::to_string(dto.soNgayGiu) + " ngay)";
                } else if (sapQuaHan) {
                    ngayText += " (" + std::to_string(dto.soNgayGiu) + " ngay)";
                }
                sf::Text cell3 = TaoVanBan(font, ngayText, FONT_SIZE_NHO - 1, textColor);
                cell3.setPosition(labelX + contentWidth * 0.75f, tableY + 4.f);
                window.draw(cell3);
                
                tableY += 24.f;
            }
        } else {
            sf::Text txtEmpty = TaoVanBan(font, "(Chua muon sach nao)", FONT_SIZE_NHO, sf::Color(150, 150, 160));
            txtEmpty.setPosition(labelX + contentWidth / 2.f - 70.f, tableY + 20.f);
            window.draw(txtEmpty);
        }
        
        currentY += tableHeight + 15.f;
        
        // ===== INPUT: Mã sách (thu gọn 70% chiều rộng) =====
        float inputWidth = contentWidth * 0.7f;
        std::string goiYMaSach = currentState.chuoiMaSach.empty() ? "VD: 9780321563842-1" : "";
        TaoInput(font, INPUT_MT_TIM_SACH, labelX, currentY, inputWidth, INPUT_CAO,
                "Ma Sach (*):", currentState.chuoiMaSach, goiYMaSach);
        
        // Quick hint bên cạnh input
        if (currentState.slSachDangMuon > 0 && currentState.maSachDangChon.empty()) {
            sf::Text txtHint = TaoVanBan(font, "<-- Click sach", FONT_SIZE_NHO - 2, sf::Color(150, 150, 160));
            txtHint.setPosition(labelX + inputWidth + 10.f, currentY + INPUT_CAO / 2.f - 6.f);
            window.draw(txtHint);
        }
        
        currentY += INPUT_CAO + 12.f;
        
        // ===== SMART BUTTONS: Hiển thị theo context =====
        float btnW = contentWidth;  // Full width
        float btnH = NUT_CAO + 5.f;
        
        bool daDienMaSach = !currentState.chuoiMaSach.empty();
        bool daChonSachDangMuon = !currentState.maSachDangChon.empty();
        bool coTheMuon = (currentState.slSachDangMuon < 3);
        bool docGiaBiKhoa = (dg->data.TrangThai == 0);
        
        // CONTEXT 1: Đã chọn sách đang mượn → chỉ hiện TRA + BAO MAT
        if (daChonSachDangMuon && daDienMaSach) {
            // Disable nếu độc giả bị khoá
            sf::Color mauTra = docGiaBiKhoa ? sf::Color(100, 100, 100) : MAU_THANH_CONG;
            sf::Color mauBaoMat = docGiaBiKhoa ? sf::Color(100, 100, 100) : MAU_LOI;
            
            // Nút TRA SACH (nổi bật)
            TaoNut(font, NUT_MT_XAC_NHAN_TRA, labelX, currentY, btnW, btnH + 3.f, "TRA SACH", mauTra, MAU_CHU_NUT);
            currentY += btnH + 11.f;
            
            // Nút BÁO MẤT
            TaoNut(font, NUT_MT_BAO_MAT, labelX, currentY, btnW, btnH, "BAO MAT SACH", mauBaoMat, MAU_CHU_NUT);
            currentY += btnH + 10.f;
            
            // Helper text with icon
            std::string helpMsg = docGiaBiKhoa 
                ? "⚠ Doc gia bi khoa. Khong the thao tac."
                : "✓ Da chon sach dang muon. Chon thao tac ben tren.";
            sf::Color helpColor = docGiaBiKhoa ? sf::Color(255, 150, 0) : sf::Color(100, 200, 100);
            sf::Text txtHelp = TaoVanBan(font, helpMsg, FONT_SIZE_NHO - 2, helpColor);
            txtHelp.setPosition(labelX, currentY);
            window.draw(txtHelp);
        }
        // CONTEXT 2: Chưa chọn/nhập mã sách mới → hiện đủ 3 nút
        else {
            bool coSachDeTra = (currentState.slSachDangMuon > 0);
            sf::Color mauMuon = (coTheMuon && !docGiaBiKhoa) ? MAU_THANH_CONG : sf::Color(100, 100, 100);
            sf::Color mauTra = (coSachDeTra && !docGiaBiKhoa) ? MAU_NHAN : sf::Color(100, 100, 100);
            sf::Color mauBaoMat = (coSachDeTra && !docGiaBiKhoa) ? MAU_LOI : sf::Color(100, 100, 100);
            
            // Nút MƯỢN SÁCH
            TaoNut(font, NUT_MT_XAC_NHAN_MUON, labelX, currentY, btnW, btnH, "MUON SACH", mauMuon, MAU_CHU_NUT);
            currentY += btnH + 8.f;
            
            // Nút TRẢ SÁCH
            TaoNut(font, NUT_MT_XAC_NHAN_TRA, labelX, currentY, btnW, btnH, "TRA SACH", mauTra, MAU_CHU_NUT);
            currentY += btnH + 8.f;
            
            // Nút BÁO MẤT
            TaoNut(font, NUT_MT_BAO_MAT, labelX, currentY, btnW, btnH, "BAO MAT SACH", mauBaoMat, MAU_CHU_NUT);
            currentY += btnH + 10.f;
            
            // Helper text động với icon
            std::string helpText;
            sf::Color helpColor;
            
            if (docGiaBiKhoa) {
                helpText = "⚠ Doc gia bi khoa. Khong the muon/tra sach.";
                helpColor = sf::Color(220, 100, 80);
            } else if (!coTheMuon) {
                helpText = "⚠ Doc gia da muon du 3 sach. Tra sach truoc khi muon tiep.";
                helpColor = sf::Color(220, 100, 80);
            } else if (currentState.slSachDangMuon > 0) {
                helpText = "✓ Click vao sach trong bang de chon, hoac nhap ma sach moi.";
                helpColor = sf::Color(120, 120, 140);
            } else {
                helpText = "→ Nhap ma sach de muon cho doc gia.";
                helpColor = sf::Color(120, 120, 140);
            }
            
            sf::Text txtHelp = TaoVanBan(font, helpText, FONT_SIZE_NHO - 2, helpColor);
            txtHelp.setPosition(labelX, currentY);
            window.draw(txtHelp);
        }
        
        // Helper text
        std::string helpText = coTheMuon 
            ? "* Click vao dong sach trong bang de chon truoc khi TRA/MAT"
            : "! Doc gia da muon du 3 sach. Vui long tra sach truoc khi muon tiep";
        sf::Color helpColor = coTheMuon ? sf::Color(120, 120, 140) : sf::Color(220, 100, 80);
        sf::Text txtHelp = TaoVanBan(font, helpText, FONT_SIZE_NHO - 2, helpColor);
        txtHelp.setPosition(labelX, currentY);
        window.draw(txtHelp);
        
    } else {
        // ===== Empty state: Chưa chọn độc giả =====
        float emptyCardH = 100.f;
        
        sf::RectangleShape emptyCard(sf::Vector2f(contentWidth, emptyCardH));
        emptyCard.setPosition(labelX, currentY + 40.f);
        emptyCard.setFillColor(sf::Color::Transparent);
        emptyCard.setOutlineThickness(1.5f);
        emptyCard.setOutlineColor(sf::Color(100, 100, 120));
        window.draw(emptyCard);
        
        sf::Text txtIcon = TaoVanBan(font, "?", 48, sf::Color(180, 180, 200));
        txtIcon.setPosition(labelX + contentWidth / 2.f - 15.f, currentY + 50.f);
        window.draw(txtIcon);
        
        sf::Text txtEmpty1 = TaoVanBan(font, "Chua chon doc gia", FONT_SIZE_BINH_THUONG, sf::Color(120, 120, 140));
        txtEmpty1.setPosition(labelX + contentWidth / 2.f - 75.f, currentY + 100.f);
        window.draw(txtEmpty1);
        
        sf::Text txtEmpty2 = TaoVanBan(font, "Double-click vao dong trong bang ben trai de chon", 
                                      FONT_SIZE_NHO, sf::Color(150, 150, 170));
        txtEmpty2.setPosition(labelX + 30.f, currentY + 122.f);
        window.draw(txtEmpty2);
    }
}

// ----- VẼ THÔNG BÁO (dùng lại global notification system) -----
static void VeKhungThongBaoSFML(sf::RenderWindow &window, const sf::Font &font, const MuonTraState& currentState) {
    (void)currentState;
    
    VeKhung(window, FORM_MT_X, KHUNG_THONG_BAO_Y, FORM_MT_RONG, KHUNG_THONG_BAO_CAO, "THONG BAO", font);

    if (noiDungThongBao.empty()) return;

    sf::Color mauChuThongBao = MAU_CHU;
    if (loaiThongBao == 1) mauChuThongBao = MAU_LOI;
    else if (loaiThongBao == 2) mauChuThongBao = MAU_THANH_CONG;

    float maxTextWidth = FORM_MT_RONG - 2 * PADDING - 10.f;
    std::string wrappedText = WordWrapText(font, noiDungThongBao, FONT_SIZE_BINH_THUONG, maxTextWidth);
    sf::Text txtMsg = TaoVanBan(font, wrappedText, FONT_SIZE_BINH_THUONG, mauChuThongBao);
    txtMsg.setPosition(FORM_MT_X + PADDING, KHUNG_THONG_BAO_Y + 35.f);
    window.draw(txtMsg);
}

// ----- VẼ MODAL TOP 10 (pattern từ VeModalChiTietBanSao) -----
static void VeModalTop10(sf::RenderWindow &window, const sf::Font &font, MuonTraState& currentState) {
    // Overlay
    sf::RectangleShape overlay(sf::Vector2f(CHIEU_RONG, CHIEU_CAO));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);

    // Modal box
    float modalW = CHIEU_RONG * 0.85f;
    float modalH = CHIEU_CAO * 0.80f;
    float modalX = (CHIEU_RONG - modalW) / 2.f;
    float modalY = (CHIEU_CAO - modalH) / 2.f;

    VeKhung(window, modalX, modalY, modalW, modalH, "TOP 10 SACH DUOC MUON NHIEU NHAT", font);

    float currentY = modalY + 50.f;
    float labelX = modalX + PADDING;

    // Table header
    std::string headers[] = {"#", "ISBN", "Ten Sach", "Tac Gia", "The Loai", "Luot Muon"};
    float colWidths[] = {0.05f, 0.15f, 0.30f, 0.20f, 0.15f, 0.15f};
    float colX[7];
    colX[0] = labelX;
    for (int i = 0; i < 6; ++i) {
        colX[i + 1] = colX[i] + colWidths[i] * (modalW - 2 * PADDING);
    }

    sf::Text headerText;
    headerText.setFont(font);
    headerText.setCharacterSize(FONT_SIZE_BINH_THUONG);
    headerText.setFillColor(MAU_NHAN);

    for (int i = 0; i < 6; ++i) {
        headerText.setString(headers[i]);
        headerText.setPosition(colX[i], currentY);
        window.draw(headerText);
    }
    currentY += 30.f;

    // Draw rows với highlight top 3 và icon medal
    for (int i = 0; i < currentState.soLuongTop && i < 10; ++i) {
        const TopSachDTO& dto = currentState.top10[i];
        if (!dto.dauSach) continue;
        
        // Background color với alpha nhẹ hơn
        sf::Color bgColor = sf::Color::Transparent;
        std::string medalIcon = "";
        if (i == 0) {
            bgColor = sf::Color(255, 215, 0, 100); // Gold nhạt
            medalIcon = "1";  // 🥇
        } else if (i == 1) {
            bgColor = sf::Color(192, 192, 192, 100); // Silver nhạt
            medalIcon = "2";  // 🥈
        } else if (i == 2) {
            bgColor = sf::Color(205, 127, 50, 100); // Bronze nhạt
            medalIcon = "3";  // 🥉
        }
        
        if (bgColor != sf::Color::Transparent) {
            sf::RectangleShape rowBg(sf::Vector2f(modalW - 2 * PADDING, 28.f));
            rowBg.setPosition(labelX, currentY);
            rowBg.setFillColor(bgColor);
            window.draw(rowBg);
        }

        sf::Text cellText;
        cellText.setFont(font);
        cellText.setCharacterSize(FONT_SIZE_NHO);
        cellText.setFillColor(MAU_CHU);

        // Rank với icon medal cho top 3
        std::string rankText = medalIcon.empty() ? std::to_string(i + 1) : medalIcon + " #" + std::to_string(i + 1);
        cellText.setString(rankText);
        cellText.setPosition(colX[0], currentY + 5.f);
        if (!medalIcon.empty()) {
            cellText.setStyle(sf::Text::Bold);
            cellText.setFillColor(MAU_NHAN);
        }
        window.draw(cellText);
        cellText.setStyle(sf::Text::Regular);

        cellText.setFillColor(MAU_CHU);
        cellText.setString(CatChuoiVoiDauCham(dto.dauSach->ISBN, 12));
        cellText.setPosition(colX[1], currentY + 5.f);
        window.draw(cellText);

        cellText.setString(CatChuoiVoiDauCham(dto.dauSach->tenSach, 30));
        cellText.setPosition(colX[2], currentY + 5.f);
        window.draw(cellText);

        cellText.setString(CatChuoiVoiDauCham(dto.dauSach->tacGia, 20));
        cellText.setPosition(colX[3], currentY + 5.f);
        window.draw(cellText);

        cellText.setString(CatChuoiVoiDauCham(dto.dauSach->theLoai, 15));
        cellText.setPosition(colX[4], currentY + 5.f);
        window.draw(cellText);

        cellText.setString(std::to_string(dto.dauSach->soLuotMuon));
        cellText.setPosition(colX[5], currentY + 5.f);
        window.draw(cellText);

        currentY += 28.f;
    }

    // Close button
    float btnY = modalY + modalH - 60.f;
    TaoNut(font, NUT_MT_VAO_TOP_10, modalX + modalW - 140.f, btnY, 120.f, 40.f, "DONG", MAU_LOI, MAU_CHU_NUT);
}

// ----- VẼ MODAL QUÁ HẠN -----
static void VeModalQuaHan(sf::RenderWindow &window, const sf::Font &font, MuonTraState& currentState) {
    // Overlay
    sf::RectangleShape overlay(sf::Vector2f(CHIEU_RONG, CHIEU_CAO));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);

    float modalW = CHIEU_RONG * 0.85f;
    float modalH = CHIEU_CAO * 0.85f;
    float modalX = (CHIEU_RONG - modalW) / 2.f;
    float modalY = (CHIEU_CAO - modalH) / 2.f;

    VeKhung(window, modalX, modalY, modalW, modalH, "DOC GIA QUA HAN TRA SACH", font);

    float currentY = modalY + 50.f;
    float labelX = modalX + PADDING;
    // Header
    std::string headers[] = {"STT", "Ma The", "Ho Ten", "Sach Qua Han", "Ngay Muon", "So Ngay Qua"};
    float colWidths[] = {0.05f, 0.10f, 0.25f, 0.30f, 0.15f, 0.15f};
    float colX[7];
    colX[0] = labelX;
    for (int i = 0; i < 6; ++i) {
        colX[i + 1] = colX[i] + colWidths[i] * (modalW - 2 * PADDING);
    }

    sf::Text headerText;
    headerText.setFont(font);
    headerText.setCharacterSize(FONT_SIZE_BINH_THUONG);
    headerText.setFillColor(MAU_NHAN);

    for (int i = 0; i < 6; ++i) {
        headerText.setString(headers[i]);
        headerText.setPosition(colX[i], currentY);
        window.draw(headerText);
    }

    // Ve duong ke duoi header
    sf::RectangleShape line(sf::Vector2f(modalW - 2 * PADDING, 1.f));
    line.setPosition(labelX, currentY + 30.f);
    line.setFillColor(sf::Color(100, 100, 100));
    window.draw(line);

    currentY += 30.f;

    // Ve o du lieu co phan trang
    int startIndex = (currentState.trangQuaHanHienTai - 1) * QUA_HAN_MOI_TRANG;
    int endIndex = startIndex + QUA_HAN_MOI_TRANG;
    if (endIndex > currentState.soLuongQuaHan) 
        endIndex = currentState.soLuongQuaHan;

    for (int i = startIndex; i < endIndex; ++i) {
        const DocGiaQuaHanDTO& dto = currentState.dsQuaHan[i];
        if (!dto.docGia) continue;
        PTRDG dg = dto.docGia;
        
        // Color based on severity với alpha nhẹ hơn và icon cảnh báo
        sf::Color bgColor = sf::Color::Transparent;
        sf::Color textColor = MAU_CHU;
        std::string warnIcon = "";
        
        if (dto.soNgayQuaHanMax > 14) {
            bgColor = sf::Color(255, 100, 100, 80); // Đỏ nhạt - nghiêm trọng
            textColor = sf::Color(200, 50, 50);
            warnIcon = "!!! ";
        } else if (dto.soNgayQuaHanMax > 7) {
            bgColor = sf::Color(255, 200, 100, 60); // Cam nhạt - cảnh báo
            textColor = sf::Color(220, 120, 0);
            warnIcon = "!! ";
        } else {
            bgColor = sf::Color(255, 255, 150, 50); // Vàng nhạt - nhẹ
            textColor = sf::Color(180, 150, 0);
            warnIcon = "! ";
        }
        
        sf::RectangleShape rowBg(sf::Vector2f(modalW - 2 * PADDING, 28.f));
        rowBg.setPosition(labelX, currentY);
        rowBg.setFillColor(bgColor);
        window.draw(rowBg);

        sf::Text cellText;
        cellText.setFont(font);
        cellText.setCharacterSize(FONT_SIZE_NHO);
        cellText.setFillColor(textColor);

        // STT
        cellText.setString(std::to_string(i + 1));
        cellText.setPosition(colX[0], currentY + 5.f);
        window.draw(cellText);

        // Ma the
        cellText.setString(std::to_string(dg->data.MaThe));
        cellText.setPosition(colX[1], currentY + 5.f);
        window.draw(cellText);

        // Ho ten
        std::string hoTen = dg->data.Ho + " " + dg->data.Ten;
        cellText.setString(CatChuoiVoiDauCham(hoTen, 25)); // tang do dai cat chuoi
        cellText.setPosition(colX[2], currentY + 5.f);
        window.draw(cellText);

        // Text phu
        cellText.setFillColor(sf::Color(150, 150, 150));
        cellText.setString("(xem the doc gia)");
        cellText.setPosition(colX[3], currentY + 5.f);
        window.draw(cellText);

        cellText.setPosition(colX[4], currentY + 5.f);
        window.draw(cellText);

        // Hiển thị số ngày quá hạn với icon cảnh báo
        std::string ngayText = warnIcon + std::to_string(dto.soNgayQuaHanMax) + " ngay";
        cellText.setFillColor(textColor);
        cellText.setStyle(sf::Text::Bold);
        cellText.setString(ngayText);
        cellText.setPosition(colX[5], currentY + 5.f);
        window.draw(cellText);

        currentY += 30.f;
    }

    // Ve thanh dieu huong
    float footerY = modalY + modalH - 50.f;
    float footerX = modalX + PADDING;

    // Nut trang truoc
    sf::Color colorPrev = (currentState.trangQuaHanHienTai > 1) ? MAU_NEN_NUT : sf::Color(80, 80, 80);
    TaoNut(font, NUT_MT_MODAL_PREV, footerX, footerY, 100.f, 35.f, "<< Truoc", colorPrev, MAU_CHU_NUT);

    // Thong tin trang
    std::string pageInfo = "Trang " + std::to_string(currentState.trangQuaHanHienTai) + " / " + std::to_string(currentState.tongTrangQuaHan);
    sf::Text txtPage = TaoVanBan(font, pageInfo, FONT_SIZE_BINH_THUONG, MAU_CHU);
    txtPage.setPosition(footerX + 120.f, footerY + 8.f);
    window.draw(txtPage);

    // Nut trang sau
    sf::Color colorNext = (currentState.trangQuaHanHienTai < currentState.tongTrangQuaHan) ? MAU_NEN_NUT : sf::Color(80, 80, 80);
    TaoNut(font, NUT_MT_MODAL_NEXT, footerX + 250.f, footerY, 100.f, 35.f, "Sau >>", colorNext, MAU_CHU_NUT);

    // Nut dong 
    TaoNut(font, NUT_MT_TAB_QUAHAN, modalX + modalW - 140.f, footerY, 120.f, 35.f, "DONG", MAU_LOI, MAU_CHU_NUT);

    // float btnY = modalY + modalH - 60.f;
    // TaoNut(font, NUT_MT_TAB_QUAHAN, modalX + modalW - 140.f, btnY, 120.f, 40.f, "DONG", MAU_LOI, MAU_CHU_NUT);
}

// ===== LOGIC FUNCTIONS =====

static void CapNhatPhanTrangSFML(MuonTraState& currentState) {
    int totalItems = currentState.soLuongKetQuaTimKiem;
    currentState.tongSoTrang = (totalItems + DOC_GIA_MOI_TRANG - 1) / DOC_GIA_MOI_TRANG;
    if (currentState.tongSoTrang == 0) currentState.tongSoTrang = 1;
    if (currentState.trangHienTai > currentState.tongSoTrang)
        currentState.trangHienTai = currentState.tongSoTrang;
}

static int LayDanhSachDocGiaCoLoc(PTRDG root, DocGiaTableDTO arr[], int maxKetQua, const std::string& tuKhoa){
    int count = 0;

    //Su dung lamda function 
    std::function<void(PTRDG)> inorderLoc = [&](PTRDG node) {
        if(!node || count >= maxKetQua) return;

        inorderLoc(node->left);

        bool thoaMan = false;

        if(tuKhoa.empty()) 
            thoaMan = true;
        else {
            //Logic 1: Tên Bắt đầu bằng từ khóa
            std::string hoTen = node->data.Ho + " " + node->data.Ten;
            if(KiemTraBatDauBang(hoTen, tuKhoa)){
                thoaMan = true;
            }
            else if(KiemTraBatDauBang(std::to_string(node->data.MaThe),tuKhoa)) {
                thoaMan = true;
            }
        }

        //Nếu thỏa mãn thì thêm vào danh sách
        if(thoaMan && count < maxKetQua){
            arr[count].docGia = node;
            arr[count].loaiKhop = 0;
            count++;
        }

        inorderLoc(node->right);
    };
    inorderLoc(root);
    return count;
}

static void ThucHienTimKiemNoiBo(MuonTraState& currentState) {
    extern PTRDG rootDocGia;
    currentState.soLuongKetQuaTimKiem = 0;
    
    std::string tuKhoa = ChuanHoaKhoangTrang(currentState.chuoiTimKiem);
    
    // Load tất cả (search logic có thể thêm sau)
    currentState.soLuongKetQuaTimKiem = LayDanhSachDocGiaCoLoc(rootDocGia, currentState.ketQuaTimKiem, MAX_DAUSACH, tuKhoa);    
    
    currentState.trangHienTai = 1;
    CapNhatPhanTrangSFML(currentState);
    
    if (currentState.soLuongKetQuaTimKiem > 0) {
        CapNhatThongBaoSFML("Hien thi " + std::to_string(currentState.soLuongKetQuaTimKiem) + " doc gia.", 0);
    } else {
        CapNhatThongBaoSFML("Khong co doc gia nao.", 1);
    }
}

static void CapNhatSachDangMuon(MuonTraState& currentState) {
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    if (!currentState.docGiaDangChon) {
        currentState.slSachDangMuon = 0;
        return;
    }
    
    currentState.slSachDangMuon = LayDSSachDangMuon(
        currentState.docGiaDangChon,
        currentState.listSachDangMuon,
        10,
        dsDauSach,
        soLuongDauSach
    );
}

static void XoaFormNhapLieuSFML(MuonTraState& currentState) {
    currentState.chuoiMaSach = "";
    currentState.maSachDangChon = "";
    currentState.indexSachDangChon = -1;
}

static void ThucHienMuonSachSFML(MuonTraState& currentState) {
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    extern bool duLieuDaThayDoi;
    
    if (!currentState.docGiaDangChon) {
        CapNhatThongBaoSFML("Loi: Chua chon doc gia!", 1);
        inputHoatDong = INPUT_MT_TIM_SACH;
        return;
    }
    
    // Kiểm tra đã mượn đủ 3 sách chưa
    if (currentState.slSachDangMuon >= 3) {
        CapNhatThongBaoSFML("Loi: Doc gia da muon du 3 sach!\nVui long tra sach truoc khi muon tiep.", 1);
        return;
    }
    
    std::string maSach = CatKhoangTrang(currentState.chuoiMaSach);
    if (maSach.empty()) {
        CapNhatThongBaoSFML("Loi: Chua nhap ma sach!", 1);
        inputHoatDong = INPUT_MT_TIM_SACH;
        return;
    }
    
    // Validate format: ISBN-XXX
    size_t dashPos = maSach.find('-');
    if (dashPos == std::string::npos || dashPos == 0 || dashPos == maSach.length() - 1) {
        CapNhatThongBaoSFML("Loi: Ma sach khong hop le!\nDinh dang: ISBN-XXX\nVi du: 9780321563842-001", 1);
        inputHoatDong = INPUT_MT_TIM_SACH;
        return;
    }
    
    // Call backend
    std::string loi = MuonSach(currentState.docGiaDangChon, maSach, dsDauSach, soLuongDauSach);
    
    if (loi.empty()) {
        duLieuDaThayDoi = true;
        CapNhatThongBaoSFML("Muon sach thanh cong: " + maSach, 2);
        CapNhatSachDangMuon(currentState);
        XoaFormNhapLieuSFML(currentState);
        inputHoatDong = KHONG_XAC_DINH;
    } else {
        CapNhatThongBaoSFML(loi, 1);
        inputHoatDong = INPUT_MT_TIM_SACH;
    }
}

static void ThucHienTraSachSFML(MuonTraState& currentState) {
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    extern bool duLieuDaThayDoi;
    
    if (!currentState.docGiaDangChon) {
        CapNhatThongBaoSFML("Loi: Chua chon doc gia!", 1);
        return;
    }
    
    if (currentState.maSachDangChon.empty()) {
        CapNhatThongBaoSFML("Loi: Chua chon sach de tra!\nClick vao dong sach trong danh sach de chon.", 1);
        return;
    }
    
    std::string loi = TraSach(currentState.docGiaDangChon, currentState.maSachDangChon, dsDauSach, soLuongDauSach);
    
    if (loi.empty()) {
        duLieuDaThayDoi = true;
        CapNhatThongBaoSFML("Tra sach thanh cong: " + currentState.maSachDangChon, 2);
        CapNhatSachDangMuon(currentState);
        XoaFormNhapLieuSFML(currentState);
    } else {
        CapNhatThongBaoSFML(loi, 1);
    }
}

static void ThucHienBaoMatSachSFML(MuonTraState& currentState) {
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    extern bool duLieuDaThayDoi;
    
    if (!currentState.docGiaDangChon) {
        CapNhatThongBaoSFML("Loi: Chua chon doc gia!", 1);
        return;
    }
    
    if (currentState.maSachDangChon.empty()) {
        CapNhatThongBaoSFML("Loi: Chua chon sach de bao mat!\nClick vao dong sach trong danh sach de chon.", 1);
        return;
    }
    
    std::string loi = BaoMatSach(currentState.docGiaDangChon, currentState.maSachDangChon, dsDauSach, soLuongDauSach);
    
    if (loi.empty()) {
        duLieuDaThayDoi = true;
        CapNhatThongBaoSFML("Bao mat sach thanh cong: " + currentState.maSachDangChon, 2);
        CapNhatSachDangMuon(currentState);
        XoaFormNhapLieuSFML(currentState);
    } else {
        CapNhatThongBaoSFML(loi, 1);
    }
}

static void TaiDuLieuTop10(MuonTraState& currentState) {
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;

    TopSachDTO bufferTam[MAX_DAUSACH];

    int soLuongLayDuoc = LayTopSach(dsDauSach, soLuongDauSach, bufferTam);
    
    currentState.soLuongTop = soLuongLayDuoc;
    for(int i = 0; i < soLuongLayDuoc; ++i) {
        currentState.top10[i] = bufferTam[i];
    }
    
    if (currentState.soLuongTop > 0) {
        CapNhatThongBaoSFML("Tai du lieu Top 10 thanh cong.", 0);
    } else {
        CapNhatThongBaoSFML("Chua co du lieu muon sach.", 1);
    }
}

static void TaiDuLieuQuaHan(MuonTraState& currentState) {
    extern PTRDG rootDocGia;
    
    currentState.soLuongQuaHan = LayDSDocGiaQuaHan(rootDocGia, currentState.dsQuaHan, MAX_DAUSACH);

    //Doan tinh trang
    currentState.trangHienTai = 1;// Reset ve trang 1
    if (currentState.soLuongQuaHan > 0) {
        currentState.tongTrangQuaHan = (currentState.soLuongQuaHan + QUA_HAN_MOI_TRANG - 1) / QUA_HAN_MOI_TRANG;
    } else {
        currentState.tongTrangQuaHan = 1;
    }
    
    if (currentState.soLuongQuaHan > 0) {
        CapNhatThongBaoSFML("Tim thay " + std::to_string(currentState.soLuongQuaHan) + " doc gia qua han.", 0);
    } else {
        CapNhatThongBaoSFML("Khong co doc gia qua han.", 0);
    }
}

static void XuLyTextInput(sf::Event event, MuonTraState& currentState) {
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Backspace) {
        if (inputHoatDong == INPUT_MT_TIM_DOC_GIA && !currentState.chuoiTimKiem.empty()) {
            currentState.chuoiTimKiem.pop_back();
            ThucHienTimKiemNoiBo(currentState);
        }
        else if (inputHoatDong == INPUT_MT_TIM_SACH && !currentState.chuoiMaSach.empty()) {
            currentState.chuoiMaSach.pop_back();
        }
    }
    
    if (event.type == sf::Event::TextEntered) {
        if (event.text.unicode < 128 && event.text.unicode != 8 && event.text.unicode != 13 && event.text.unicode != 9) {
            char ch = static_cast<char>(event.text.unicode);
            
            if (inputHoatDong == INPUT_MT_TIM_DOC_GIA) {
                if (currentState.chuoiTimKiem.length() < 100) {
                    currentState.chuoiTimKiem += ch;
                    ThucHienTimKiemNoiBo(currentState);
                }
            }
            else if (inputHoatDong == INPUT_MT_TIM_SACH) {
                if (currentState.chuoiMaSach.length() < 50) {
                    currentState.chuoiMaSach += ch;
                }
            }
        }
    }
}

// ===== PUBLIC API =====

void KhoiTaoManHinhMuonTra() {
    state.Reset();
    ThucHienTimKiemNoiBo(state);
}

void VeManHinhMuonTra(sf::RenderWindow &window, const sf::Font &font) {
    // Thanh tiêu đề (giống ManHinhQuanLySach)
    sf::RectangleShape topBar(sf::Vector2f(CHIEU_RONG, THANH_TAB_CAO));
    topBar.setFillColor(MAU_KHUNG);
    window.draw(topBar);
    sf::Text title = TaoVanBan(font, "MUON / TRA SACH", FONT_SIZE_BINH_THUONG, MAU_TIEU_DE);
    title.setPosition(PADDING, PADDING / 2.f - title.getLocalBounds().height / 2.f);
    window.draw(title);

    // Nút quay về menu (đúng vị trí như màn hình sách)
    TaoNut(font, NUT_BACK, CHIEU_RONG - PADDING - 100.f, PADDING / 4.f, 100.f, NUT_CAO * 0.8f, "< MENU", MAU_NUT_BACK, MAU_CHU_NUT);
    
    // Modal check
    if (state.hienThiModalTop10) {
        VeModalTop10(window, font, state);
        return;
    }
    
    if (state.hienThiModalQuaHan) {
        VeModalQuaHan(window, font, state);
        return;
    }
    
    // Main UI (search bar được vẽ trong VeBangDocGia)
    VeBangDocGia(window, font, state);
    VeFormMuonTra(window, font, state);
    VeKhungThongBaoSFML(window, font, state);
}

void XuLySuKienManHinhMuonTra(sf::RenderWindow &window, sf::Event event) {
    (void)window;
    
    // Block modal events
    if (state.hienThiModalTop10) {
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            MaUI elementNhan = LayElementTaiToaDo(event.mouseButton.x, event.mouseButton.y);
            if (elementNhan == NUT_MT_VAO_TOP_10) {
                state.hienThiModalTop10 = false;
            }
        }
        return;
    }
    
    if (state.hienThiModalQuaHan) {
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            MaUI elementNhan = LayElementTaiToaDo(event.mouseButton.x, event.mouseButton.y);
            
            if (elementNhan == NUT_MT_TAB_QUAHAN) {
                state.hienThiModalQuaHan = false; // Đóng modal
            }
            // Xử lý nút Trang Trước
            else if (elementNhan == NUT_MT_MODAL_PREV) {
                if (state.trangQuaHanHienTai > 1) {
                    state.trangQuaHanHienTai--;
                }
            }
            // Xử lý nút Trang Sau
            else if (elementNhan == NUT_MT_MODAL_NEXT) {
                if (state.trangQuaHanHienTai < state.tongTrangQuaHan) {
                    state.trangQuaHanHienTai++;
                }
            }
        }
        return;
    }
    
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
        // Nếu đang gõ ở ô tìm kiếm độc giả
        if (inputHoatDong == INPUT_MT_TIM_DOC_GIA) {
            inputHoatDong = KHONG_XAC_DINH; // Bỏ focus (tùy chọn)
            ThucHienTimKiemNoiBo(state);    // Gọi hàm tìm kiếm
            return; // Kết thúc xử lý sự kiện này
        }
    }
    // Text input
    if (event.type == sf::Event::TextEntered || 
        (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Backspace)) {
        if (inputHoatDong != KHONG_XAC_DINH) {
            XuLyTextInput(event, state);
        }
    }
    
    // Mouse events
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        MaUI elementNhan = LayElementTaiToaDo(event.mouseButton.x, event.mouseButton.y);
        
        // Xử lý nút BACK (quay về menu)
        if (elementNhan == NUT_BACK) {
            manHinhHienTai = MENU_CHINH;
            state.Reset();
            inputHoatDong = KHONG_XAC_DINH;
            CapNhatThongBaoSFML("", 0);
        }
        // Nút TÌM (giống màn hình sách)
        // else if (elementNhan == NUT_TIM) {
        //     inputHoatDong = KHONG_XAC_DINH;
        //     ThucHienTimKiemNoiBo(state);
        // }
        // Nút XÓA TÌM (giống màn hình sách)
        else if (elementNhan == NUT_XOA_TIM) {
            state.chuoiTimKiem = "";
            inputHoatDong = KHONG_XAC_DINH;
            ThucHienTimKiemNoiBo(state);
        }
        // Input tìm kiếm
        else if (elementNhan == INPUT_MT_TIM_DOC_GIA) {
            inputHoatDong = INPUT_MT_TIM_DOC_GIA;
            CapNhatThongBaoSFML("", 0);
        }
        // Các nút TOP 10, QUÁ HẠN
        else if (elementNhan == NUT_MT_VAO_TOP_10) {
            TaiDuLieuTop10(state);
            state.hienThiModalTop10 = true;
        }
        else if (elementNhan == NUT_MT_TAB_QUAHAN) {
            TaiDuLieuQuaHan(state);
            state.hienThiModalQuaHan = true;
        }
        
        // Input fields
        else if (elementNhan == INPUT_MT_TIM_SACH) {
            inputHoatDong = INPUT_MT_TIM_SACH;
            // Nếu click vào input khi đang chọn sách mượn → reset để nhập mới
            if (!state.maSachDangChon.empty() && state.chuoiMaSach == state.maSachDangChon) {
                state.maSachDangChon = "";
                state.chuoiMaSach = "";
                CapNhatThongBaoSFML("San sang nhap ma sach moi.", 0);
            }
        }
        
        // Pagination
        else if (elementNhan == NUT_MT_PREV_PAGE) {
            if (state.trangHienTai > 1) state.trangHienTai--;
        }
        else if (elementNhan == NUT_MT_NEXT_PAGE) {
            if (state.trangHienTai < state.tongSoTrang) state.trangHienTai++;
        }
        
        // Action buttons
        else if (elementNhan == NUT_MT_XAC_NHAN_MUON) {
            ThucHienMuonSachSFML(state);
        }
        else if (elementNhan == NUT_MT_XAC_NHAN_TRA) {
            ThucHienTraSachSFML(state);
        }
        else if (elementNhan == NUT_MT_BAO_MAT) {
            ThucHienBaoMatSachSFML(state);
        }
        // NÚT HỦY CHỌN - reset trạng thái chọn (dưới bảng danh sách)
        else if (elementNhan == NUT_HUY_CHON) {
            state.docGiaDangChon = nullptr;
            state.maTheDocGiaDuocChon = 0;
            state.maSachDangChon = "";
            state.slSachDangMuon = 0;
            state.chuoiMaSach = "";
            inputHoatDong = KHONG_XAC_DINH;
            CapNhatThongBaoSFML("✓ Da huy chon doc gia.", 0);
        }
        // CLICK vào sách đang mượn trong mini table → auto-fill mã sách
        else if (elementNhan >= static_cast<MaUI>(HANG_SACH + 1000) && 
                 elementNhan < static_cast<MaUI>(HANG_SACH + 1003)) {
            int rowIndex = elementNhan - static_cast<MaUI>(HANG_SACH + 1000);
            if (rowIndex >= 0 && rowIndex < state.slSachDangMuon) {
                const ThongTinSachDangMuon_DTO& dto = state.listSachDangMuon[rowIndex];
                state.maSachDangChon = dto.maSach;
                state.chuoiMaSach = dto.maSach;  // Auto-fill vào input
                inputHoatDong = KHONG_XAC_DINH;  // Không focus để người dùng thấy buttons rõ hơn
                CapNhatThongBaoSFML("✓ Da chon sach: " + dto.tenSach + ". Bay gio chi can bam TRA SACH hoac BAO MAT.", 2);
            }
        }
        
        // Reader table row click (đúng logic như HANG_SACH)
        else if (elementNhan == HANG_SACH) {
            const float headerY = BANG_MT_Y;
            const float contentY = headerY + 40.f + 35.f;
            const float rowHeight = 30.f;
            int rowIndex = static_cast<int>((event.mouseButton.y - contentY) / rowHeight);

            // LOGIC FIX: Nếu click hơi lệch lên trên (rowIndex = -1) nhưng vẫn nằm trong vùng HANG_SACH
            // thì ta ép nó về dòng 0
            if (rowIndex == -1 && (event.mouseButton.y - contentY) > -10.f) {
                rowIndex = 0;
            }

            const int startIndex = (state.trangHienTai - 1) * DOC_GIA_MOI_TRANG;
            const int actualIndex = startIndex + rowIndex;

            if (actualIndex >= 0 && actualIndex < state.soLuongKetQuaTimKiem && 
                state.ketQuaTimKiem[actualIndex].docGia) {
                
                PTRDG clickedDG = state.ketQuaTimKiem[actualIndex].docGia;
                int clickedMaThe = clickedDG->data.MaThe;
                const float elapsed = state.doubleClickClock.getElapsedTime().asSeconds();
                
                // Double-click detection
                if (elapsed < state.THOI_GIAN_DOUBLE_CLICK && clickedMaThe == state.maTheClickCuoi) {
                    state.docGiaDangChon = clickedDG;
                    state.maTheDocGiaDuocChon = clickedMaThe;
                    
                    // Reset input & state khi đổi độc giả
                    state.maSachDangChon = "";
                    state.chuoiMaSach = "";
                    inputHoatDong = KHONG_XAC_DINH;
                    
                    CapNhatSachDangMuon(state);
                    CapNhatThongBaoSFML("✓ Da chon doc gia: " + clickedDG->data.Ho + " " + clickedDG->data.Ten, 2);
                    state.maTheClickCuoi = 0;
                } else {
                    state.maTheClickCuoi = clickedMaThe;
                    state.doubleClickClock.restart();
                }
            }
        }
        
        else {
            inputHoatDong = KHONG_XAC_DINH;
        }
    }
}
