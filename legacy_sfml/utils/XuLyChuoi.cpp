#include "XuLyChuoi.h"
#include "NgayThang.h"
#include "ThongBao.h"
#include "Constants.h"
#include <stdexcept>
#include <cctype>

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
    std::string ketQua = chuoi;
    for (size_t i = 0; i < ketQua.length(); ++i) {
        ketQua[i] = std::tolower(static_cast<unsigned char>(ketQua[i]));
    }
    return ketQua;
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

// Bo dau tieng Viet va chuyen thanh chu thuong (dung cho so sanh)
// std::string BoDauVaThuong(const std::string& chuoi) {
//     // Buoc 1: Chuyen sang chu thuong
//     std::string ketQua = ChuyenInThuong(chuoi);
    
//     // Buoc 2: Bo dau Unicode tieng Viet (thay the ky tu co dau thanh khong dau)
//     // Map: ky tu co dau -> ky tu khong dau (dung cho tieng Viet UTF-8)
//     const char* coDau[] = {
//         "à", "á", "ả", "ã", "ạ", "ă", "ằ", "ắ", "ẳ", "ẵ", "ặ",
//         "â", "ầ", "ấ", "ẩ", "ẫ", "ậ",
//         "è", "é", "ẻ", "ẽ", "ẹ", "ê", "ề", "ế", "ể", "ễ", "ệ",
//         "ì", "í", "ỉ", "ĩ", "ị",
//         "ò", "ó", "ỏ", "õ", "ọ", "ô", "ồ", "ố", "ổ", "ỗ", "ộ",
//         "ơ", "ờ", "ớ", "ở", "ỡ", "ợ",
//         "ù", "ú", "ủ", "ũ", "ụ", "ư", "ừ", "ứ", "ử", "ữ", "ự",
//         "ỳ", "ý", "ỷ", "ỹ", "ỵ",
//         "đ"
//     };
//     const char* khongDau[] = {
//         "a", "a", "a", "a", "a", "a", "a", "a", "a", "a", "a",
//         "a", "a", "a", "a", "a", "a",
//         "e", "e", "e", "e", "e", "e", "e", "e", "e", "e", "e",
//         "i", "i", "i", "i", "i",
//         "o", "o", "o", "o", "o", "o", "o", "o", "o", "o", "o",
//         "o", "o", "o", "o", "o", "o",
//         "u", "u", "u", "u", "u", "u", "u", "u", "u", "u", "u",
//         "y", "y", "y", "y", "y",
//         "d"
//     };
    
//     const int soKyTu = 69; // Tong so cap ky tu co dau -> khong dau
    
//     // Thay the tung ky tu co dau
//     for (int i = 0; i < soKyTu; ++i) {
//         size_t pos = 0;
//         std::string timKiem = coDau[i];
//         std::string thayThe = khongDau[i];
        
//         // Tim va thay the tat ca xuat hien
//         while ((pos = ketQua.find(timKiem, pos)) != std::string::npos) {
//             ketQua.replace(pos, timKiem.length(), thayThe);
//             pos += thayThe.length();
//         }
//     }
    
//     return ketQua;
// }

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

// Chuan hoa chuoi cho tim kiem: Chuyen chu thuong va chuan hoa khoang trang
std::string ChuanHoaChuoiTimKiem(const std::string& chuoi) {
    std::string ketQua;
    ketQua.reserve(chuoi.size());
    bool coKhoangTrangTruocDo = false;
    bool daBatDau = false;
    
    for (size_t i = 0; i < chuoi.length(); ++i) {
        char kyTu = chuoi[i];
        char kyTuThuong = std::tolower(static_cast<unsigned char>(kyTu));
        char kyTuXuLy = laKyTuKhoangTrang(kyTuThuong) ? ' ' : kyTuThuong;
        
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
std::string ChuanHoaTenUnicode(const std::string& chuoiDauVao, size_t doDaiToiThieu, 
                                size_t doDaiToiDa, std::string& chuoiKetQua) {
    // Kiem tra rong
    if (chuoiDauVao.empty()) {
        return "Loi: Khong duoc de trong!\nVui long nhap thong tin hop le.";
    }
    
    std::string chuoiDaCat = CatKhoangTrang(chuoiDauVao);
    if (chuoiDaCat.empty() || KiemTraChuoiRongHoacKhoangTrang(chuoiDaCat)) {
        return "Loi: Chi chua khoang trang!\nVui long nhap ky tu hop le, khong chi co khoang trang.";
    }
    
    // Kiem tra ky tu dau tien phai la chu cai
    if (!laKyTuChuCai(chuoiDaCat[0])) {
        return "Loi: Ky tu dau tien phai la chu cai!";
    }
    
    // Kiem tra cac ky tu con lai
    for (size_t i = 1; i < chuoiDaCat.length(); ++i) {
        char kyTu = chuoiDaCat[i];
        bool kyTuHopLe = laKyTuChuCai(kyTu) || laKyTuSo(kyTu) || laKyTuKhoangTrang(kyTu) ||
                         kyTu == '+' || kyTu == '#' || kyTu == '-' || 
                         kyTu == '.' || kyTu == '_';
        
        if (!kyTuHopLe) {
            return "Loi: Chi duoc chua chu cai, so, khoang trang va cac ky tu: + # - . _";
        }
    }
    
    // Chuan hoa khoang trang
    std::string chuoiChuan = ChuanHoaKhoangTrang(chuoiDaCat);
    
    // Kiem tra do dai sau khi chuan hoa
    if (chuoiChuan.empty()) {
        return "Loi: Sau khi chuan hoa bi rong!";
    }
    
    if (chuoiChuan.length() < doDaiToiThieu) {
        return "Loi: Qua ngan (>= " + std::to_string(doDaiToiThieu) + ")!";
    }
    
    if (chuoiChuan.length() > doDaiToiDa) {
        return "Loi: Qua dai (<= " + std::to_string(doDaiToiDa) + ")!";
    }
    
    // Gan ket qua
    chuoiKetQua = chuoiChuan;
    return "";  // Thanh cong
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
