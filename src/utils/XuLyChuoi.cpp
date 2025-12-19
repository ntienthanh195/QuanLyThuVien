/**
 * @file XuLyChuoi.cpp
 * @path src/utils/XuLyChuoi.cpp
 * @description Tiện ích xử lý chuỗi ký tự Tiếng Việt.
 * Cung cấp các hàm: chuyển đổi Unicode, cắt khoảng trắng, chuẩn hóa tên riêng và các thao tác liên quan đến tìm kiếm.
 * @project Library Management System (QuanLyThuVien)
 */
#include "utils/XuLyChuoi.h"

#include "utils/NgayThang.h"
#include "utils/ThongBao.h"
#include "Constants.h"
#include <stdexcept>
#include <cctype>
#include <cstring>
#include <QString>
#include <QRegularExpression>

//TIEN ICH NHO (HO TRO NOI BO) 

inline bool laKyTuSo(char kyTu) { 
    return kyTu >= '0' && kyTu <= '9'; 
}

inline bool laKyTuChuHoa(char kyTu) { 
    return kyTu >= 'A' && kyTu <= 'Z'; 
}

inline bool laKyTuChuThuong(char kyTu) { 
    return kyTu >= 'a' && kyTu <= 'z'; 
}

inline bool laKyTuChuCai(char kyTu) { 
    return laKyTuChuHoa(kyTu) || laKyTuChuThuong(kyTu); 
}

inline bool laKyTuKhoangTrang(char kyTu) { 
    return kyTu == ' ' || kyTu == '\t' || kyTu == '\n' || 
           kyTu == '\r' || kyTu == '\f' || kyTu == '\v'; 
}

//CHUYEN DOI CHU HOA/THUONG 

// Chuyen toan bo chuoi thanh chu hoa
std::string ChuyenInHoa(const std::string& chuoi) {
    std::string ketQua = chuoi;
    for (size_t i = 0; i < ketQua.length(); ++i) {
        ketQua[i] = std::toupper(static_cast<unsigned char>(ketQua[i]));
    }
    return ketQua;
}

// Chuyen toan bo chuoi thanh chu thuong
std::string ChuyenInThuong(const std::string& chuoi) {
    if (chuoi.empty()) return "";
    
    try {
        std::string ketQua = chuoi;
        for (size_t i = 0; i < ketQua.length(); ++i) {
            ketQua[i] = std::tolower(static_cast<unsigned char>(ketQua[i]));
        }
        return ketQua;
    } catch (...) {
        // Neu co loi, tra ve chuoi goc
        return chuoi;
    }
}

// Chuyen chuoi thanh dang Title Case (Chu Cai Dau Moi Tu Viet Hoa)
std::string ChuyenThanhTitleCase(const std::string& chuoi) {
    std::string ketQua = ChuyenInThuong(chuoi);
    bool laDauTu = true;
    
    for (size_t i = 0; i < ketQua.length(); ++i) {
        char kyTu = ketQua[i];
        if (laDauTu && laKyTuChuThuong(kyTu)) {
            ketQua[i] = kyTu - 'a' + 'A';
            laDauTu = false;
        } else if (laKyTuKhoangTrang(kyTu)) {
            laDauTu = true;
        } else {
            laDauTu = false;
        }
    }
    return ketQua;
}

// Chuyen chuoi thanh dang Title Case ho tro tieng Viet (UTF-8)
// DEPRECATED: Use ChuanHoaHoTenHienDai() instead for safer Unicode handling
std::string ChuyenThanhTitleCaseUnicode(const std::string& chuoi) {
    // Wrapper around modern Qt-based implementation to avoid crashes
    return ChuanHoaHoTenHienDai(chuoi);
}

// Bo dau tieng Viet va chuyen thanh chu thuong (dung cho so sanh)
std::string BoDauVaThuong(const std::string& chuoi) {
    // Buoc 1: Chuyen sang chu thuong
    std::string ketQua = ChuyenInThuong(chuoi);
    
    // Buoc 2: Bo dau Unicode tieng Viet (thay the ky tu co dau thanh khong dau)
    // Map: ky tu co dau -> ky tu khong dau (dung cho tieng Viet UTF-8)
    const char* coDau[] = {
        "à", "á", "ả", "ã", "ạ", "ă", "ằ", "ắ", "ẳ", "ẵ", "ặ",
        "â", "ầ", "ấ", "ẩ", "ẫ", "ậ",
        "è", "é", "ẻ", "ẽ", "ẹ", "ê", "ề", "ế", "ể", "ễ", "ệ",
        "ì", "í", "ỉ", "ĩ", "ị",
        "ò", "ó", "ỏ", "õ", "ọ", "ô", "ồ", "ố", "ổ", "ỗ", "ộ",
        "ơ", "ờ", "ớ", "ở", "ỡ", "ợ",
        "ù", "ú", "ủ", "ũ", "ụ", "ư", "ừ", "ứ", "ử", "ữ", "ự",
        "ỳ", "ý", "ỷ", "ỹ", "ỵ",
        "đ"
    };
    const char* khongDau[] = {
        "a", "a", "a", "a", "a", "a", "a", "a", "a", "a", "a",
        "a", "a", "a", "a", "a", "a",
        "e", "e", "e", "e", "e", "e", "e", "e", "e", "e", "e",
        "i", "i", "i", "i", "i",
        "o", "o", "o", "o", "o", "o", "o", "o", "o", "o", "o",
        "o", "o", "o", "o", "o", "o",
        "u", "u", "u", "u", "u", "u", "u", "u", "u", "u", "u",
        "y", "y", "y", "y", "y",
        "d"
    };
    
    const int soKyTu = sizeof(coDau) / sizeof(coDau[0]); // Tu dong tinh kich thuoc an toan
    
    // Thay the tung ky tu co dau
    for (int i = 0; i < soKyTu; ++i) {
        size_t pos = 0;
        std::string timKiem = coDau[i];
        std::string thayThe = khongDau[i];
        
        // Tim va thay the tat ca xuat hien
        while ((pos = ketQua.find(timKiem, pos)) != std::string::npos) {
            ketQua.replace(pos, timKiem.length(), thayThe);
            pos += thayThe.length();
        }
    }
    
    return ketQua;
}

//CAT VA CHUAN HOA KHOANG TRANG 

// Cat khoang trang dau va cuoi chuoi
std::string CatKhoangTrang(const std::string& chuoi) {
    size_t viTriTrai = 0, viTriPhai = chuoi.size();
    
    while (viTriTrai < viTriPhai && laKyTuKhoangTrang(chuoi[viTriTrai])) {
        ++viTriTrai;
    }
    while (viTriPhai > viTriTrai && laKyTuKhoangTrang(chuoi[viTriPhai - 1])) {
        --viTriPhai;
    }
    
    return chuoi.substr(viTriTrai, viTriPhai - viTriTrai);
}

// Chuan hoa khoang trang: Thay nhieu khoang trang thanh 1 va cat dau/cuoi
std::string ChuanHoaKhoangTrang(const std::string& chuoi) {
    if (chuoi.empty()) return "";
    std::string ketQua;
    ketQua.reserve(chuoi.size());
    bool coKhoangTrangTruocDo = false;
    
    for (size_t i = 0; i < chuoi.length(); ++i) {
        char kyTu = chuoi[i];
        char kyTuXuLy = laKyTuKhoangTrang(kyTu) ? ' ' : kyTu;
        
        if (kyTuXuLy == ' ') {
            if (!coKhoangTrangTruocDo) {
                ketQua.push_back(' ');
                coKhoangTrangTruocDo = true;
            }
        } else {
            ketQua.push_back(kyTuXuLy);
            coKhoangTrangTruocDo = false;
        }
    }
    
    return CatKhoangTrang(ketQua);
}

//KIEM TRA VA LAY KY TU 

// Lay chi cac ky tu so tu chuoi
std::string LayChiCacKyTuSo(const std::string& chuoi) {
    std::string ketQua;
    ketQua.reserve(chuoi.size());
    
    for (size_t i = 0; i < chuoi.length(); ++i) {
        if (laKyTuSo(chuoi[i])) {
            ketQua.push_back(chuoi[i]);
        }
    }
    return ketQua;
}

// Kiem tra chuoi co rong hoac chi chua khoang trang
bool KiemTraChuoiRongHoacKhoangTrang(const std::string& chuoi) {
    for (size_t i = 0; i < chuoi.length(); ++i) {
        if (!laKyTuKhoangTrang(chuoi[i])) {
            return false;
        }
    }
    return true;
}

//CHUYEN DOI CHUOI THANH SO 

// Chuyen chuoi thanh so nguyen, ho tro kiem tra gioi han
bool ChuyenChuoiThanhSoNguyen(const std::string& chuoi, int& ketQua, bool chiChoPhepSoDuong) {
    if (chuoi.empty()) {
        return false;
    }
    
    long long dau = 1;
    size_t viTri = 0;
    
    // Kiem tra dau am
    if (chuoi[0] == '-') {
        if (chiChoPhepSoDuong) {
            return false;
        }
        dau = -1;
        viTri = 1;
        if (chuoi.size() == 1) {
            return false;  // Chi co dau '-'
        }
    }
    
    // Chuyen doi tung ky tu thanh so
    long long soTichLuy = 0;
    for (; viTri < chuoi.size(); ++viTri) {
        if (!laKyTuSo(chuoi[viTri])) {
            return false;
        }
        soTichLuy = soTichLuy * 10 + (chuoi[viTri] - '0');
        
        // Kiem tra tran so
        if (soTichLuy * dau > INT_MAX || soTichLuy * dau < INT_MIN) {
            return false;
        }
    }
    
    ketQua = int(soTichLuy * dau);
    return true;
}

//CHUAN HOA THEO DINH DANG RIENG 

// Chuan hoa chuoi cho tim kiem: Bo dau tieng Viet, chuyen chu thuong va chuan hoa khoang trang
std::string ChuanHoaChuoiTimKiem(const std::string& chuoi) {
    // Buoc 1: Bo dau va chuyen thanh chu thuong
    std::string chuoiBoDau = BoDauVaThuong(chuoi);
    
    // Buoc 2: Chuan hoa khoang trang (bo khoang trang dau/cuoi, collapse multiple spaces)
    std::string ketQua;
    ketQua.reserve(chuoiBoDau.size());
    bool coKhoangTrangTruocDo = false;
    bool daBatDau = false;
    
    for (size_t i = 0; i < chuoiBoDau.length(); ++i) {
        char kyTu = chuoiBoDau[i];
        char kyTuXuLy = laKyTuKhoangTrang(kyTu) ? ' ' : kyTu;
        
        if (kyTuXuLy == ' ') {
            if (!coKhoangTrangTruocDo && daBatDau) {
                ketQua.push_back(' ');
                coKhoangTrangTruocDo = true;
            }
        } else {
            ketQua.push_back(kyTuXuLy);
            coKhoangTrangTruocDo = false;
            daBatDau = true;
        }
    }
    
    // Cat khoang trang cuoi (neu con)
    while (!ketQua.empty() && ketQua.back() == ' ') {
        ketQua.pop_back();
    }
    
    return ketQua;
}

// Chuan hoa ho ten nguoi: collapse spaces, trim, and apply title-case (UTF-8 aware)
std::string ChuanHoaHoTen(const std::string& chuoiHoTen) {
    // Su dung ham hien dai hon voi Qt
    return ChuanHoaHoTenHienDai(chuoiHoTen);
}

//============================================================================
// HAM XU LY UNICODE HIEN DAI (SU DUNG QT)
//============================================================================

// Chuyen std::string UTF-8 sang QString
QString ChuyenSangQString(const std::string& chuoi) {
    return QString::fromUtf8(chuoi.c_str());
}

// Chuyen QString sang std::string UTF-8
std::string ChuyenSangStdString(const QString& qstr) {
    return qstr.toUtf8().toStdString();
}

// Chuan hoa ho ten hien dai voi Qt (tot nhat cho tieng Viet)
std::string ChuanHoaHoTenHienDai(const std::string& chuoiHoTen) {
    if (chuoiHoTen.empty()) return "";
    
    QString qstr = ChuyenSangQString(chuoiHoTen);
    
    // Buoc 1: Cat va chuan hoa khoang trang
    qstr = qstr.trimmed();
    qstr = qstr.simplified();  // Thay nhieu khoang trang thanh 1
    
    if (qstr.isEmpty()) return "";
    
    // Buoc 2: Tach thanh cac tu
    QStringList cacTu = qstr.split(' ', Qt::SkipEmptyParts);
    
    // Buoc 3: Title Case - viet hoa chu cai dau moi tu, lowercase con lai
    for (int i = 0; i < cacTu.size(); ++i) {
        QString& tu = cacTu[i];
        if (tu.isEmpty()) continue;
        
        // Chuyen toan bo thanh lowercase truoc
        tu = tu.toLower();
        
        // Viet hoa ky tu dau tien (ho tro UTF-8/Unicode dung cach)
        if (tu.length() > 0) {
            tu[0] = tu[0].toUpper();
        }
    }
    
    // Buoc 4: Ghep lai thanh chuoi hoan chinh
    QString ketQua = cacTu.join(' ');
    
    return ChuyenSangStdString(ketQua);
}

// Kiem tra tinh hop le cua ho ten
bool KiemTraHoTenHopLe(const std::string& hoTen, std::string& thongBaoLoi) {
    if (hoTen.empty()) {
        thongBaoLoi = "Ho ten khong duoc de trong!";
        return false;
    }
    
    QString qstr = ChuyenSangQString(hoTen).trimmed();
    
    if (qstr.isEmpty()) {
        thongBaoLoi = "Ho ten chi chua khoang trang!";
        return false;
    }
    
    // Kiem tra do dai (2-100 ky tu)
    if (qstr.length() < 2) {
        thongBaoLoi = "Ho ten qua ngan (toi thieu 2 ky tu)!";
        return false;
    }
    
    if (qstr.length() > 100) {
        thongBaoLoi = "Ho ten qua dai (toi da 100 ky tu)!";
        return false;
    }
    
    // Kiem tra ky tu dau tien khong phai la so hoac ky tu dac biet
    QChar kyTuDau = qstr[0];
    if (kyTuDau.isDigit()) {
        thongBaoLoi = "Ho ten khong duoc bat dau bang so!";
        return false;
    }
    
    // Kiem tra cac ky tu hop le: chu cai, khoang trang, dau + - .
    QRegularExpression regex("^[\\p{L}\\s.'-]+$", QRegularExpression::UseUnicodePropertiesOption);
    if (!regex.match(qstr).hasMatch()) {
        thongBaoLoi = "Ho ten chua ky tu khong hop le! Chi cho phep chu cai, khoang trang, dau . - '";
        return false;
    }
    
    // Kiem tra khong qua nhieu khoang trang lien tiep (sau simplified)
    if (qstr.contains("  ")) {
        thongBaoLoi = "Ho ten chua qua nhieu khoang trang lien tiep!";
        return false;
    }
    
    thongBaoLoi = "";
    return true;
}

// Chuan hoa ten Unicode hien dai (dung cho ten sach, tac gia, the loai)
std::string ChuanHoaTenHienDai(const std::string& ten, size_t doDaiToiThieu, 
                                 size_t doDaiToiDa, std::string& thongBaoLoi) {
    if (ten.empty()) {
        thongBaoLoi = "Ten khong duoc de trong!";
        return "";
    }
    
    QString qstr = ChuyenSangQString(ten);
    qstr = qstr.trimmed().simplified();
    
    if (qstr.isEmpty()) {
        thongBaoLoi = "Ten chi chua khoang trang!";
        return "";
    }
    
    // Kiem tra ky tu dau tien
    if (qstr[0].isDigit()) {
        thongBaoLoi = "Ten khong duoc bat dau bang so!";
        return "";
    }
    
    // Kiem tra do dai
    size_t doDai = static_cast<size_t>(qstr.length());
    if (doDai < doDaiToiThieu) {
        thongBaoLoi = "Ten qua ngan (toi thieu " + std::to_string(doDaiToiThieu) + " ky tu)!";
        return "";
    }
    
    if (doDai > doDaiToiDa) {
        thongBaoLoi = "Ten qua dai (toi da " + std::to_string(doDaiToiDa) + " ky tu)!";
        return "";
    }
    
    // Kiem tra ky tu hop le (chu cai, so, khoang trang, cac ky tu dac biet co ban)
    QRegularExpression regex("^[\\p{L}\\d\\s.,'#:+()\\-]+$", 
                            QRegularExpression::UseUnicodePropertiesOption);
    if (!regex.match(qstr).hasMatch()) {
        thongBaoLoi = "Ten chua ky tu khong hop le!";
        return "";
    }
    
    // Apply Title Case
    QStringList cacTu = qstr.split(' ', Qt::SkipEmptyParts);
    for (int i = 0; i < cacTu.size(); ++i) {
        QString& tu = cacTu[i];
        if (tu.isEmpty()) continue;
        tu = tu.toLower();
        if (tu.length() > 0) {
            tu[0] = tu[0].toUpper();
        }
    }
    
    QString ketQua = cacTu.join(' ');
    thongBaoLoi = "";
    return ChuyenSangStdString(ketQua);
}

// Chuan hoa gioi tinh: "Nam" hoac "Nu"
std::string ChuanHoaGioiTinh(const std::string& chuoiGioiTinh) {
    std::string chuoiChuan = ChuyenInThuong(CatKhoangTrang(chuoiGioiTinh));
    
    if (chuoiChuan == "nam") {
        return "Nam";
    }
    if (chuoiChuan == "nu") {
        return "Nu";
    }
    
    return std::string();  // Khong hop le
}

// Chuan hoa vi tri ke sach
std::string ChuanHoaViTri(const std::string& chuoiViTri) {
    std::string chuoiSach = ChuanHoaKhoangTrang(CatKhoangTrang(chuoiViTri));
    if (chuoiSach.empty()) {
        return std::string();
    }
    
    std::string cacTu[8];
    int soTu = TachTokenKhoangTrang(chuoiSach, cacTu, 8);
    if (soTu == 0) {
        return std::string();
    }
    
    // Lambda kiem tra co chu so trong chuoi
    auto coChuSo = [](const std::string& chuoiKiemTra) {
        for (size_t i = 0; i < chuoiKiemTra.length(); ++i) {
            if (chuoiKiemTra[i] >= '0' && chuoiKiemTra[i] <= '9') {
                return true;
            }
        }
        return false;
    };
    
    // Lambda kiem tra ky tu ASCII
    auto laChuCaiAscii = [](char kyTu) {
        return (kyTu >= 'A' && kyTu <= 'Z') || (kyTu >= 'a' && kyTu <= 'z');
    };
    
    // Neu bat dau bang "ke" hoac "Ke"
    std::string tuDauThuong = ChuyenInThuong(cacTu[0]);
    if (tuDauThuong == "ke") {
        if (soTu == 1) {
            return std::string();
        }
        
        std::string phanConLai = cacTu[1];
        for (int i = 2; i < soTu; ++i) {
            phanConLai += ' ' + cacTu[i];
        }
        return std::string("Ke ") + ChuyenInHoa(phanConLai);
    }
    
    // Neu co dau gach ngang va chu so: "A-12" -> "Ke A-12"
    if (chuoiSach.find('-') != std::string::npos && coChuSo(chuoiSach)) {
        return std::string("Ke ") + ChuyenInHoa(chuoiSach);
    }
    
    // Neu bat dau bang chu cai va chu so: "A12" -> "Ke A12"
    if (!chuoiSach.empty() && laChuCaiAscii(chuoiSach[0]) && 
        chuoiSach.size() > 1 && (chuoiSach[1] >= '0' && chuoiSach[1] <= '9')) {
        return std::string("Ke ") + ChuyenInHoa(chuoiSach);
    }
    
    return chuoiSach;
}

//CHUAN HOA DU LIEU 

// Chuan hoa ISBN (ham core ho tro nhieu che do)
std::string ChuanHoaISBNCore(const std::string& chuoiISBN, std::string& ketQua, 
                              bool tuDong, bool nhapThuCong, bool laDocFile) {
    std::string chuoi = CatKhoangTrang(chuoiISBN);
    
    if (chuoi.empty()) {
        return "Loi: ISBN khong duoc de trong!\nVui long nhap ISBN (10 hoac 13 chu so).";
    }
    
    if (tuDong || nhapThuCong || laDocFile) {
        std::string chuoiSo = LayChiCacKyTuSo(chuoi);
        
        if (chuoiSo.length() != 10 && chuoiSo.length() != 13) {
            return "Loi: ISBN phai co 10 hoac 13 chu so!\nHien tai co " + 
                   std::to_string(chuoiSo.length()) + " chu so.\n" +
                   "Vi du: 9780123456789 (13 so) hoac 0123456789 (10 so).";
        }
        
        ketQua = chuoiSo;
        return "";  // Thanh cong
    }
    
    return "Loi: Che do chuan hoa ISBN khong hop le!";
}

// Chuan hoa ISBN tu file (chi lay so)
std::string ChuanHoaISBNFile(const std::string& chuoiISBN, std::string& ketQua) {
    return ChuanHoaISBNCore(chuoiISBN, ketQua, true, false, true);
}

// Chuan hoa ten Unicode (ten sach, tac gia, the loai...)
// Chuan hoa ten Unicode (ten sach, tac gia, the loai...)
// UPDATED: Now uses Qt-based validation for safety
std::string ChuanHoaTenUnicode(const std::string& chuoiDauVao, size_t doDaiToiThieu, 
                                size_t doDaiToiDa, std::string& chuoiKetQua) {
    // Use modern Qt-based implementation
    std::string thongBaoLoi;
    std::string ketQua = ChuanHoaTenHienDai(chuoiDauVao, doDaiToiThieu, doDaiToiDa, thongBaoLoi);
    
    if (!thongBaoLoi.empty()) {
        // Convert modern error format to legacy format
        return "Loi: " + thongBaoLoi;
    }
    
    chuoiKetQua = ketQua;
    return "";  // Success
}

//  STRUCT VA HAM HO TRO TIM KIEM 

// Chuan hoa toan bo thong tin sach cho tim kiem
ThongTinSachChuanHoa ChuanHoaDuLieuSach(const std::string& tenSach, const std::string& tacGia,
                                        const std::string& theLoai, const std::string& isbn) {
    ThongTinSachChuanHoa ketQua;
    ketQua.tenSach = ChuanHoaChuoiTimKiem(tenSach);
    ketQua.tacGia = ChuanHoaChuoiTimKiem(tacGia);
    ketQua.theLoai = ChuanHoaChuoiTimKiem(theLoai);
    ketQua.isbn = ChuanHoaChuoiTimKiem(isbn);
    return ketQua;
}

//TACH CHUOI 

// Tach cac truong tu dong du lieu (vd: "a|b|c")
bool TachTruong(const std::string& dong, char kyTuPhanCach, std::string mangKetQua[], int soTruongMongDoi) {
    int chiSoTruong = 0;
    std::string truongHienTai;
    
    for (size_t i = 0; i < dong.length(); ++i) {
        char kyTu = dong[i];
        
        if (kyTu == kyTuPhanCach) {
            if (chiSoTruong < soTruongMongDoi) {
                mangKetQua[chiSoTruong++] = truongHienTai;
            }
            truongHienTai.clear();
        } else {
            truongHienTai.push_back(kyTu);
        }
    }
    
    // Them truong cuoi cung
    if (chiSoTruong < soTruongMongDoi) {
        mangKetQua[chiSoTruong++] = truongHienTai;
    }
    
    return chiSoTruong == soTruongMongDoi;
}

// Tach chuoi thanh cac token phan cach boi khoang trang
int TachTokenKhoangTrang(const std::string& chuoi, std::string mangToken[], int soTokenToiDa) {
    int soToken = 0;
    std::string tokenHienTai;
    
    for (size_t i = 0; i < chuoi.length(); ++i) {
        char kyTu = chuoi[i];
        
        if (laKyTuKhoangTrang(kyTu)) {
            if (!tokenHienTai.empty()) {
                if (soToken < soTokenToiDa) {
                    mangToken[soToken++] = tokenHienTai;
                }
                tokenHienTai.clear();
            }
        } else {
            tokenHienTai.push_back(kyTu);
        }
    }
    
    // Them token cuoi cung
    if (!tokenHienTai.empty() && soToken < soTokenToiDa) {
        mangToken[soToken++] = tokenHienTai;
    }
    
    return soToken;
}

// Tach ISBN tu ma sach (vd: "ISBN-123" -> "ISBN")
std::string TachISBNTuMaSach(const std::string& maSach) {
    size_t viTriGachNgang = maSach.find('-');
    
    if (viTriGachNgang == std::string::npos) {
        return maSach;  // Khong co dau '-'
    }
    
    return maSach.substr(0, viTriGachNgang);
}

// Lay so hau to tu ma sach (vd: "ISBN-123" -> 123)
int LaySoHauToMaSach(PTRDMS nodeSach) {
    if (!nodeSach || nodeSach->maSach.empty()) {
        return -1;
    }
    
    size_t viTriGachNgangCuoi = nodeSach->maSach.find_last_of('-');
    
    if (viTriGachNgangCuoi == std::string::npos || 
        viTriGachNgangCuoi >= nodeSach->maSach.length() - 1) {
        return -1;  // Khong co dau '-' hoac khong co ky tu sau dau '-'
    }
    
    std::string chuoiSo = nodeSach->maSach.substr(viTriGachNgangCuoi + 1);
    int ketQua = -1;
    
    if (ChuyenChuoiThanhSoNguyen(chuoiSo, ketQua, true)) {
        return ketQua;
    }
    
    return -1;  // Khong phai so
}

//TIM KIEM VA KHOP CHUOI 

// Kiem tra chuoi nguon co chua tat ca cac tu khoa
bool ChuaTatCaTuKhoa(const std::string& chuoiNguon, const std::string mangTuKhoa[], int soTuKhoa) {
    for (int i = 0; i < soTuKhoa; ++i) {
        if (chuoiNguon.find(mangTuKhoa[i]) == std::string::npos) {
            return false;
        }
    }
    return true;
}

// Kiem tra tieu de co khop voi tu khoa (khong dau)
bool KhopTieuDeKhongDau(const std::string& tieuDeChuan, const std::string& tuKhoaChuan,
                        const std::string mangToken[], int soToken) {
    (void)tuKhoaChuan;  // Khong su dung tham so nay
    return ChuaTatCaTuKhoa(tieuDeChuan, mangToken, soToken);
}
