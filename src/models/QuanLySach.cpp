#include "models/QuanLySach.h"
#include <mutex>
#include "utils/ThongBao.h"
#include "models/KiemTraDuLieu.h"
#include "utils/XuLyChuoi.h"
#include "models/ThaoTacFile.h"
#include "models/NhapLieu.h"
#include "Constants.h"
#include <iomanip>
#include <ctime>
#include <stdexcept>
#include <sstream>
#include <sstream>
#include "utils/EventBus.h"
#include <algorithm> // For std::sort

#include "utils/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include "utils/AuditLogger.h"



//  BIEN TOAN CUC 
// Da duoc dinh nghia trong main.cpp, chi can extern
// PTRDS dsDauSach[MAX_DAUSACH] = {nullptr};
// int soLuongDauSach = 0;
// bool duLieuDaThayDoi = false;



//  NHOM 1: SINH MA & CHUYEN DOI TRANG THAI

// sinh ma sach tu dong: ISBN-1, ISBN-2, ISBN-3, ...
std::string sinhMaSach(const std::string &chiSo, int &soThuTu) {
    std::string ma = chiSo + "-" + std::to_string(soThuTu);     // tao ma sach
    std::stringstream dummy;
    
    if (timDanhMucTheoMaSach(ma, dsDauSach, soLuongDauSach, dummy, true)) {
        return "Loi: Ma sach da ton tai: " + ma;                // ma bi trung
    }
    
    return ma;                                                  // thanh cong
}

// tra ve ten trang thai dang chuoi
const char* TenTrangThai(TrangThaiSach tt) {
    if (tt == CHO_MUON_DUOC) return "Cho muon duoc";           // 0
    if (tt == DANG_MUON)     return "Dang muon";               // 1
    if (tt == THANH_LY)      return "Thanh ly";                // 2
    return "Khong ro";                                         // khong xac dinh
}

// chuyen chuoi trang thai thanh enum
int PhanTichTrangThaiSach(const std::string &s, std::ostream &out) {
    std::string tt = ChuyenInThuong(CatKhoangTrang(s));
    if (tt == "0" || tt == "0\r" || tt == "cho muon duoc") return CHO_MUON_DUOC;
    if (tt == "1" || tt == "1\r" || tt == "dang muon")     return DANG_MUON;
    if (tt == "2" || tt == "2\r" || tt == "thanh ly")      return THANH_LY;
    thongBao(out, "Trang thai khong hop le: " + tt, LOI);
    return -1;
}



//  NHOM 2: QUAN LY DANH MUC SACH (DMS - DANH SACH LIEN KET DON)


// them ban sao vao dau danh sach lien ket 
std::string themDanhMucSach(PTRDMS &dms, const std::string &ma, TrangThaiSach tt, const std::string &vt) {
    if (tt < 0 || tt > 2)                return "Loi: Trang thai khong hop le!";  // kiem tra trang thai
    if (vt.length() > MAX_VI_TRI_KE)     return "Loi: Vi tri qua dai!";           // kiem tra do dai
    
    PTRDMS node = new DanhMucSach{ma, tt, vt, dms};  // tao node moi
    dms = node;                                       // chen vao dau
    return "";                                        // thanh cong
}

// tim so lon nhat trong danh sach ban sao O(M) M la so ban sao
int TimSoThuTuLonNhat(PTRDS dauSach) {
    if (!dauSach || !dauSach->dms) return 0;                   // chua co ban sao nao
    
    int soLonNhat = 0;                                         // khoi tao so lon nhat
    PTRDMS pTemp = dauSach->dms;                               // bat dau tu dau DSLK
    int dem = 0;                                               // bao ve vong lap
    
    while (pTemp && dem < MAX_BAN_SAO) {                      // duyet het DSLK
        int soHienTai = LaySoHauToMaSach(pTemp);              // lay so hau to
        if (soHienTai > soLonNhat) {                          // neu lon hon
            soLonNhat = soHienTai;                            // cap nhat so lon nhat
        }
        pTemp = pTemp->next;                                   // node tiep theo
        dem++;                                                 // tang dem
    }
    
    return soLonNhat;                                          // tra ve so lon nhat
}

// them nhieu ban sao vao dau sach O(N*M) N so luong them, M kiem tra trung
std::string ThemNhieuBanSao(PTRDS dauSach, int soLuong, const std::string &viTri) {
    // validation som
    if (!dauSach) {
        return "Loi: Dau sach khong ton tai!";
    }
    
    if (soLuong <= 0) {
        return "Loi: So luong phai lon hon 0!";
    }
    
    if (soLuong > MAX_BAN_SAO) {
        return "Loi: So luong khong hop le (toi da " + std::to_string(MAX_BAN_SAO) + ")!";
    }
    
    // kiem tra tong so ban sao khong vuot qua MAX
    if (dauSach->tongBanSao + soLuong > MAX_BAN_SAO) {
        return "Loi: Tong so ban sao se vuot qua gioi han " + std::to_string(MAX_BAN_SAO) + "!";
    }
    
    // kiem tra vi tri (neu co)
    std::string viTriChuan = viTri;
    if (!viTri.empty()) {
        viTriChuan = ChuanHoaViTri(viTri);
        if (viTriChuan.empty()) {
            return "Loi: Vi tri khong hop le! Chi duoc chua chu cai va so. VD: A-12, B5, Ke C-3";
        }
    } else {
        // neu khong co vi tri -> lay tu ban sao cu
        if (dauSach->dms && !dauSach->dms->viTri.empty()) {
            viTriChuan = dauSach->dms->viTri;
        }
    }
    
    // tim so thu tu lon nhat
    int soThuTu = TimSoThuTuLonNhat(dauSach) + 1;
    
    // kiem tra overflow
    if (soThuTu > INT_MAX - soLuong) {
        return "Loi: So thu tu qua lon, khong the them ban sao!";
    }
    
    int themThanhCong = 0;                                     // dem so ban sao them duoc
    std::string loiTrongLoop = "";                             // luu loi neu co
    
    // vong lap them tung ban sao
    for (int i = 0; i < soLuong; ++i) {
        // tao ma sach moi
        std::string maSach = dauSach->ISBN + "-" + std::to_string(soThuTu);
        
        // dam bao khong trung
        std::stringstream dummy;
        int baoVe = 0;
        while (timDanhMucTheoMaSach(maSach, dsDauSach, soLuongDauSach, dummy, true)) {
            if (++baoVe > MAX_BAN_SAO) {
                loiTrongLoop = "Loi: Khong the sinh ma sach duy nhat!";
                break;
            }
            if (soThuTu >= INT_MAX) {
                loiTrongLoop = "Loi: So thu tu vuot gioi han!";
                break;
            }
            ++soThuTu;
            maSach = dauSach->ISBN + "-" + std::to_string(soThuTu);
        }
        
        if (!loiTrongLoop.empty()) break;
        
        // kiem tra do dai ma sach
        if (maSach.length() > static_cast<size_t>(DO_DAI_MA_SACH_MAX)) {
            loiTrongLoop = "Loi: Ma sach '" + maSach + "' qua dai!";
            break;
        }
        
        // goi backend them ban sao
        std::string loiThem = themDanhMucSach(dauSach->dms, maSach, CHO_MUON_DUOC, viTriChuan);
        if (!loiThem.empty()) {
            loiTrongLoop = loiThem;
            break;
        }
        
        themThanhCong++;
        soThuTu++;
    }
    
    // cap nhat tong ban sao
    dauSach->tongBanSao += themThanhCong;
    duLieuDaThayDoi = true;
    // Notify listeners that this DauSach changed (copies added)
    try { EventBus::instance().dauSachUpdated(QString::fromStdString(dauSach->ISBN)); } catch(...) {}
    
    // tra ve ket qua
    if (!loiTrongLoop.empty()) {
        if (themThanhCong > 0) {
            return "Chi them duoc " + std::to_string(themThanhCong) + "/" + std::to_string(soLuong) + " ban sao. Loi: " + loiTrongLoop;
        }
        return loiTrongLoop;
    }
    
    return "";  // thanh cong
}

// chen node DMS vao dau sach (dung khi doc file)
bool ChenNodeDMSVaoDauSach(PTRDS d, const std::string &ma, int tt, const std::string &vt) {
    if (!d || tt < 0 || tt > 2) return false;                  // kiem tra hop le
    
    PTRDMS node = new DanhMucSach{ma, static_cast<TrangThaiSach>(tt), vt, d->dms};
    d->dms = node;                                              // chen vao dau
    return true;
}

// cap nhat trang thai cho ban sao 
bool capNhatTrangThaiSach(PTRDMS dms, const std::string &ma, TrangThaiSach tt) {
    while (dms) {                                              // duyet DSLK don
        if (dms->maSach == ma) {                               // tim thay - early return
            dms->trangThai = tt;                               // cap nhat trang thai
            return true;
        }
        dms = dms->next;                                       // node tiep theo
    }
    return false;                                              // khong tim thay
}

// tim node ban sao theo ma sach O(N*M) N dau sach, M ban sao
PTRDMS timDanhMucTheoMaSach(const std::string &ma, PTRDS ds[], int n, std::ostream &out, bool silent) {
    //trich ISBN -> Tim dau sach O(N) -> Tim trong 1 DSLK O(M)
    std::string isbn = TachISBNTuMaSach(ma);                   // lay ISBN tu ma sach
    if (isbn.empty()) {                                         // ma sach khong hop le
        if (!silent) thongBao(out, "Ma sach khong hop le: " + ma, LOI);
        return nullptr;
    }
    
    // Tim dau sach theo ISBN (chi can tim 1 dau sach, khong can duyet tat ca)
    PTRDS dauSach = TimDauSachTheoISBN(ds, n, isbn);
    if (!dauSach) {                                             // khong tim thay dau sach
        if (!silent) thongBao(out, "Khong tim thay dau sach cho ma: " + ma + " (ISBN=" + isbn + ")", LOI);
        return nullptr;
    }
    
    // Chi duyet DSLK cua dau sach nay (thay vi duyet N dau sach)
    PTRDMS p = dauSach->dms;
    std::string maUpper = ChuyenInHoa(ma);
    while (p) {                                                 // duyet DSLK don
        if (ChuyenInHoa(p->maSach) == maUpper) return p;       // tim thay (case-insensitive) - early return
        p = p->next;                                            // node tiep theo
    }
    
    if (!silent) {                                             // khong tim thay trong DSLK
        thongBao(out, "Khong tim thay ma sach: " + ma + " trong danh muc cua ISBN=" + isbn, LOI);
    }
    return nullptr;
}



//  NHOM 3: QUAN LY DAU SACH (MANG CON TRO)


// them dau sach moi vao mang
// them dau sach moi vao mang
bool themDauSach(PTRDS ds[], int &n, const std::string &isbn, const std::string &ten, int trang,
                 const std::string &tg, int nam, const std::string &tl, bool anLang, long long giaTien) {
    
    if (n >= MAX_DAUSACH) {                                    // kiem tra gioi han
        if (!anLang) thongBao(std::cout, "Gioi han dau sach!", LOI);
        return false;
    }

    std::string loi, isbnChuan;
    
    // kiem tra ISBN
    if (!KiemTraChuoiRong(isbn, "ISBN").empty() ||
        !(loi = ChuanHoaISBNFile(isbn, isbnChuan)).empty()) {
        if (!anLang) thongBao(std::cout, loi, LOI);
        return false;
    }
    
    // Skip KiemTraTrungISBN when loading from DB (anLang=true)
    if (!anLang && !(loi = KiemTraTrungISBN(isbnChuan)).empty()) {
        thongBao(std::cout, loi, LOI);
        return false;
    }

    // kiem tra cac truong khac
    std::string checkTen = KiemTraChuoiVaDodai(ten, "Ten sach", MAX_TEN_SACH);
    if (!checkTen.empty()) {
        if (anLang) std::cout << "[DB LOAD DEBUG] REJECTED book (ten): " << checkTen << " | Book: " << ten << "\n";
        if (!anLang) thongBao(std::cout, checkTen, LOI);
        return false;
    }
    
    if (trang <= 0 || trang > MAX_SO_TRANG) {
        if (anLang) std::cout << "[DB LOAD DEBUG] REJECTED book (trang): " << trang << " | Book: " << ten << "\n";
        if (!anLang) thongBao(std::cout, "So trang khong hop le", LOI);
        return false;
    }
    
    std::string checkTg = KiemTraChuoiVaDodai(tg, "Tac gia", MAX_TAC_GIA);
    if (!checkTg.empty()) {
        if (anLang) std::cout << "[DB LOAD DEBUG] REJECTED book (tac gia): " << checkTg << " | Book: " << ten << "\n";
        if (!anLang) thongBao(std::cout, checkTg, LOI);
        return false;
    }
    
    std::string checkNam = KiemTraNamXuatBan(nam);
    if (!checkNam.empty()) {
        if (anLang) std::cout << "[DB LOAD DEBUG] REJECTED book (nam): " << checkNam << " | Nam: " << nam << " | Book: " << ten << "\n";
        if (!anLang) thongBao(std::cout, checkNam, LOI);
        return false;
    }
    
    std::string checkTl = KiemTraChuoiVaDodai(tl, "The loai", MAX_THE_LOAI);
    if (!checkTl.empty()) {
        if (anLang) std::cout << "[DB LOAD DEBUG] REJECTED book (the loai): " << checkTl << " | Book: " << ten << "\n";
        if (!anLang) thongBao(std::cout, checkTl, LOI);
        return false;
    }

    // tao dau sach moi
    PTRDS node = new DauSach;
    node->ISBN = isbnChuan;                                    // gan ISBN
    node->tenSach = ChuyenThanhTitleCase(ten);                 // gan ten sach
    node->soTrang = trang;                                     // gan so trang
    node->tacGia = ChuyenThanhTitleCase(tg);                   // gan tac gia
    node->namXuatBan = nam;                                    // gan nam XB
    node->theLoai = ChuyenThanhTitleCase(tl);                  // gan the loai
    node->soLuotMuon = 0;                                      // khoi tao so luot muon
    node->giaTien = giaTien;                                   // gan gia tien
    node->dms = nullptr;                                       // chua co ban sao
    node->tongBanSao = 0;                                      // tong = 0

    ds[n++] = node;                                            // them vao mang
    sapXepDauSachTheoTen(ds, 0, n - 1);                        // sap xep lai
    duLieuDaThayDoi = true;                                    // danh dau thay doi
    
    if (!anLang) {
        thongBao(std::cout, "Them thanh cong: " + node->tenSach, THONG_TIN);
    } else {
        std::cout << "[DB LOAD DEBUG] Successfully added: " << node->tenSach << " (ISBN: " << node->ISBN << ")\n";
    }
    // Emit event: new book added
    try {
        EventBus::instance().dauSachAdded(QString::fromStdString(node->ISBN));
    } catch(...) {}
    return true;
}

// tim dau sach theo ISBN - linear O(N)
PTRDS TimDauSachTheoISBN(PTRDS ds[], int n, const std::string &isbn) {
    // Defensive checks
    if (!ds || n <= 0) return nullptr;
    if (n > MAX_DAUSACH) n = MAX_DAUSACH;
    if (isbn.empty()) return nullptr;

    for (int i = 0; i < n; ++i) {                              // duyet mang
        PTRDS cur = ds[i];
        if (cur && cur->ISBN == isbn) {                        // tim thay - early return
            return cur;
        }
    }
    return nullptr;                                             // khong thay
}



//  NHOM 4: TIM KIEM & LIET KE


// tim kiem sach theo tu khoa O(N*L) N dau sach, L do dai du lieu
int timKiemLogic(PTRDS ds[], int n, const std::string &tk, KetQuaTimKiem kq[]) {
    if (tk.empty() || n == 0) return 0;                        // kiem tra rong
    
    std::string tuKhoaChuan = ChuanHoaChuoiTimKiem(tk);        // chuan hoa tu khoa
    int soKetQua = 0;                                          // dem ket qua tim duoc
    
    for (int i = 0; i < n; ++i) {                              // duyet dau sach
        PTRDS p = ds[i];
        if (!p) continue;                                       // bo qua null
        
        // goi 1 lan ChuanHoaDuLieuSach de tranh lap lai
        auto duLieuChuan = ChuanHoaDuLieuSach(p->tenSach, p->tacGia, p->theLoai, p->ISBN);
        int loaiKhop = 0;                                      // xac dinh loai khop
        
        // uu tien: ten sach > tac gia > the loai > isbn
        if      (duLieuChuan.tenSach.find(tuKhoaChuan) != std::string::npos)  loaiKhop = 1;
        else if (duLieuChuan.tacGia.find(tuKhoaChuan) != std::string::npos)   loaiKhop = 2;
        else if (duLieuChuan.theLoai.find(tuKhoaChuan) != std::string::npos)  loaiKhop = 3;
        else if (duLieuChuan.isbn.find(tuKhoaChuan) != std::string::npos)     loaiKhop = 4;
        
        // 5. Tim theo MA SACH (Ma Ca Biet) - Duyet DSLK
        if (loaiKhop == 0 && p->dms) {
            PTRDMS pCopy = p->dms;
            int safety = 0;
            while (pCopy && safety < MAX_BAN_SAO) {
                // Chuyen ma sach sang chu thuong de so sanh (tuKhoaChuan da duoc lower case trong timKiemLogic)
                std::string maCopy = ChuyenInThuong(pCopy->maSach);
                if (maCopy.find(tuKhoaChuan) != std::string::npos) {
                    loaiKhop = 1; // Match Code -> High Priority (Equivalent to Exact Name)
                    break;
                }
                pCopy = pCopy->next;
                safety++;
            }
        }
        
        if (loaiKhop > 0 && soKetQua < MAX_KET_QUA_TIM_KIEM) {  // gioi han ket qua
            kq[soKetQua++] = {p, loaiKhop};
        }
        
        if (soKetQua >= MAX_KET_QUA_TIM_KIEM) break;           // dung khi du
    }
    
    sapXepKetQuaTimKiem(kq, soKetQua);                         // sap xep ket qua
    return soKetQua;
}

// dem so luong ban sao theo trang thai O(M) M la so ban sao
int DemSoLuongBanSaoTheoTrangThai(PTRDS dauSach, TrangThaiSach trangThai) {
    if (!dauSach || !dauSach->dms) return 0;                   // khong co ban sao
    
    int dem = 0;                                               // khoi tao bo dem
    PTRDMS pTemp = dauSach->dms;                               // bat dau tu dau DSLK
    int baoVe = 0;                                             // tranh loop vo tan
    
    while (pTemp && baoVe < MAX_BAN_SAO) {                    // duyet het DSLK
        if (pTemp->trangThai == trangThai) {                  // neu khop trang thai
            dem++;                                             // tang bo dem
        }
        pTemp = pTemp->next;                                   // node tiep theo
        baoVe++;                                               // tang bao ve
    }
    
    return dem;                                                // tra ve ket qua
}

// lay thong tin thong ke dau sach O(M)
ThongTinDauSach LayThongTinDauSach(PTRDS dauSach) {
    ThongTinDauSach thongTin = {0, 0, 0, 0};                  // khoi tao struct
    
    if (!dauSach || !dauSach->dms) return thongTin;            // khong co ban sao
    
    PTRDMS pTemp = dauSach->dms;                               // bat dau tu dau DSLK
    int baoVe = 0;                                             // tranh loop vo tan
    
    while (pTemp && baoVe < MAX_BAN_SAO) {                    // duyet het DSLK
        thongTin.tongBanSao++;                                 // tang tong
        
        // dem theo tung trang thai
        if (pTemp->trangThai == CHO_MUON_DUOC) {
            thongTin.choMuonDuoc++;
        } else if (pTemp->trangThai == DANG_MUON) {
            thongTin.dangMuon++;
        } else if (pTemp->trangThai == THANH_LY) {
            thongTin.thanhLy++;
        }
        
        pTemp = pTemp->next;                                   // node tiep theo
        baoVe++;                                               // tang bao ve
    }
    
    return thongTin;                                           // tra ve ket qua
}


// lay the loai duy nhat O(N + K log K) K la so the loai
int TimTheLoaiDuyNhat(PTRDS ds[], int n, std::string mangTheLoai[]) {
    if (n == 0) return 0;                                      // mang rong
    
    int soTheLoai = 0;                                         // so the loai duy nhat
    
    for (int i = 0; i < n; ++i) {                              // duyet tat ca the loai
        if (!ds[i]) continue;                                   // bo qua neu null
        if (soTheLoai < MAX_DAUSACH) {
            mangTheLoai[soTheLoai++] = ds[i]->theLoai;
        }
    }
    
    if (soTheLoai == 0) return 0;                              // khong co the loai
    
    SapXepTheLoaiTheoTen(mangTheLoai, soTheLoai);             // sap xep 1 lan
    
    int theLoaiDuyNhat = 0;                                     // loai bo trung lap
    for (int i = 0; i < soTheLoai; ++i) {
        if (i == 0 || mangTheLoai[i] != mangTheLoai[i - 1]) {
            mangTheLoai[theLoaiDuyNhat++] = mangTheLoai[i];
        }
    }
    return theLoaiDuyNhat;
}

// tim sach theo the loai O(N)
int TimSachTheoTheLoai(PTRDS ds[], int n, const std::string &theLoaiCanTim, PTRDS mangKetQua[]) {
    int soKetQua = 0;                                          // dem ket qua
    
    for (int i = 0; i < n; ++i) {                              // duyet dau sach
        if (ds[i] && ds[i]->theLoai == theLoaiCanTim && soKetQua < MAX_DAUSACH) {
            mangKetQua[soKetQua++] = ds[i];                    // them vao ket qua
        }
    }
    return soKetQua;
}

// lay vi tri duy nhat O(N*M + K log K) K la so vi tri
int TimViTriDuyNhat(PTRDS ds[], int n, std::string mangViTri[]) {
    if (n == 0) return 0;                                      // mang rong
    
    int soViTri = 0;                                           // so vi tri
    
    for (int i = 0; i < n; ++i) {                              // duyet tat ca vi tri
        if (!ds[i]) continue;                                   // bo qua neu null
        
        PTRDMS banSao = ds[i]->dms;                            // duyet danh sach ban sao
        while (banSao) {
            if (!banSao->viTri.empty() && soViTri < MAX_DAUSACH) {
                mangViTri[soViTri++] = banSao->viTri;
            }
            banSao = banSao->next;
        }
    }
    
    if (soViTri == 0) return 0;                                // khong co vi tri
    
    SapXepTheLoaiTheoTen(mangViTri, soViTri);                 // sap xep 1 lan
    
    int viTriDuyNhat = 0;                                      // loai bo trung lap
    for (int i = 0; i < soViTri; ++i) {
        if (i == 0 || mangViTri[i] != mangViTri[i - 1]) {
            mangViTri[viTriDuyNhat++] = mangViTri[i];
        }
    }
    return viTriDuyNhat;
}


//  NHOM 5: SAP XEP


// hoan doi hai con tro dau sach
void hoanDoi(PTRDS &a, PTRDS &b) {
    PTRDS temp = a;
    a = b;
    b = temp;
}

// Chon median-of-three lam pivot O(1)
int ChonPivot(PTRDS arr[], int left, int right) {
    int mid = left + (right - left) / 2;      // lay chi so giua

    if (arr[mid] && arr[left] && arr[mid]->tenSach < arr[left]->tenSach)
        hoanDoi(arr[left], arr[mid]);         // dua nho nhat ve left

    if (arr[right] && arr[left] && arr[right]->tenSach < arr[left]->tenSach)
        hoanDoi(arr[left], arr[right]);       // dua nho nhat ve left

    if (arr[right] && arr[mid] && arr[right]->tenSach < arr[mid]->tenSach)
        hoanDoi(arr[mid], arr[right]);        // dua lon nhat ve right

    return mid;                                // tra ve vi tri median
}

// Quicksort dau sach theo ten (Su dung std::sort de tranh stack overflow)
void sapXepDauSachTheoTen(PTRDS arr[], int left, int right) {
    if (left >= right || left < 0 || right >= MAX_DAUSACH) return;

    std::sort(arr + left, arr + right + 1, [](PTRDS a, PTRDS b) {
        if (!a) return false;
        if (!b) return true;
        return a->tenSach < b->tenSach;
    });
}

// insertion sort ban sao O(N^2)
void SapXepBanSaoTheoMa(PTRDMS arr[], int n) {
    if (!arr || n <= 1) return;
    for (int i = 1; i < n; ++i) {
        PTRDMS key = arr[i];
        int soKey = LaySoHauToMaSach(key);
        int j = i - 1;
        while (j >= 0) {
            int soJ = LaySoHauToMaSach(arr[j]);
            bool canSwap = false;
            if      (soJ == -1 && soKey != -1) canSwap = true;      // a loi b ok
            else if (soJ != -1 && soKey == -1) canSwap = false;     // a ok b loi
            else if (soJ != -1 && soKey != -1 && soJ > soKey) canSwap = true; // a > b doi cho
            else canSwap = false;                                  // truong hop con lai
            if (!canSwap) break;
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = key;
    }
}

// insertion sort the loai O(N^2)
void SapXepTheLoaiTheoTen(std::string arr[], int n) {
    for (int i = 1; i < n; ++i) {                              // duyet tu 1
        std::string key = arr[i];                              // lay phan tu hien tai
        int j = i - 1;                                         // vi tri so sanh
        
        while (j >= 0 && arr[j] > key) {                       // dich phai
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = key;                                      // chen vao vi tri dung
    }
}


// hoan doi ket qua tim kiem
void hoanDoiKetQua(KetQuaTimKiem &a, KetQuaTimKiem &b) {
    KetQuaTimKiem temp = a;
    a = b;
    b = temp;
}


// phan hoach ket qua O(N)
// quicksort ket qua O(N log N) (Su dung std::sort)
void sapXepKetQuaTimKiem(KetQuaTimKiem arr[], int n) {
    if (n <= 1) return;
    
    // Su dung std::sort voi custom comparator
    std::sort(arr, arr + n, [](const KetQuaTimKiem& a, const KetQuaTimKiem& b) {
        if (a.loaiKhop != b.loaiKhop) {
            return a.loaiKhop < b.loaiKhop; // Loai khop nho hon (uu tien cao hon) dung truoc
        }
        
        // Neu cung loai khop, so sanh ten sach (case-insensitive)
        std::string tenA = "";
        std::string tenB = "";
        
        if (a.sach) tenA = ChuyenInThuong(a.sach->tenSach);
        if (b.sach) tenB = ChuyenInThuong(b.sach->tenSach);
        
        return tenA < tenB;
    });
}



//  NHOM 6: QUAN LY BO NHO


// giai phong danh muc sach (danh sach lien ket)
void GiaiPhongDanhMucSach(PTRDMS &dms) {
    int dem = 0;                                               // dem de bao ve vong lap
    while (dms && dem < SO_VONG_LAP_DMS_MAX) {                // duyet het danh sach
        PTRDMS temp = dms;                                     // luu node hien tai
        dms = dms->next;                                       // chuyen sang node sau
        delete temp;                                           // xoa node cu
        dem++;                                                 // tang dem
    }
    
    if (dms) {                                                 // con node sau SO_VONG_LAP_DMS_MAX
        thongBao(std::cout, "Phat hien vong lap trong DSLK! Cat chuoi de tranh leak.", LOI);
        dms = nullptr;                                         // cat chuoi, chap nhan mat vai node thay vi crash
    }
}

// giai phong mot dau sach
void GiaiPhongDauSach(PTRDS &d) {
    if (d) {                                                   // neu ton tai
        GiaiPhongDanhMucSach(d->dms);                         // xoa danh sach ban sao
        delete d;                                              // xoa dau sach
        d = nullptr;                                           // dat null
    }
}

// giai phong toan bo dau sach
void GiaiPhongToanBoDauSach(PTRDS ds[], int &n) {
    for (int i = 0; i < n; ++i) {                             // duyet mang
        GiaiPhongDauSach(ds[i]);                              // xoa tung dau sach
    }
    n = 0;                                                     // reset so luong
}

// dem tong so ban sao 
int DemTongSoBanSao(PTRDS ds[], int n) {
    int tong = 0;                                              // khoi tao tong
    
    for (int i = 0; i < n; ++i) {                             // duyet dau sach
        if (ds[i]) {                                           // neu ton tai
            tong += ds[i]->tongBanSao;                         // cong truc tiep field
        }
    }
    return tong;                                               // tra ve tong
}

// cap nhat tong ban sao cho tat ca dau sach
void CapNhatTongBanSao(PTRDS ds[], int n) {
    for (int i = 0; i < n; ++i) {                             // duyet dau sach
        if (!ds[i]) continue;                                  // bo qua null
        
        int dem = 0;                                           // khoi tao dem
        PTRDMS p = ds[i]->dms;                                // lay danh sach ban sao
        
        while (p) {                                            // duyet DSLK don
            ++dem;                                             // tang dem
            p = p->next;                                       // node tiep theo
            
            // phong vong lap vo han
            if (dem > SO_VONG_LAP_DMS_MAX) {
                thongBao(std::cout, "Canh bao: Vong lap vo han DMS cho ISBN " + ds[i]->ISBN + "!", CANH_BAO);
                break;
            }
        }
        
        ds[i]->tongBanSao = dem;                              // cap nhat tong
        
        // Canh bao neu vuot qua MAX_BAN_SAO
        if (dem > MAX_BAN_SAO) {
            thongBao(std::cout, "Canh bao: Dau sach ISBN " + ds[i]->ISBN + " co " + std::to_string(dem) + " ban sao, vuot qua gioi han " + std::to_string(MAX_BAN_SAO) + "!", CANH_BAO);
        }
    }
}



//  NHOM 7: XOA & CAP NHAT


// xoa dau sach theo ISBN 
std::string XoaDauSach(PTRDS ds[], int &n, const std::string &isbn) {
    int idx = -1;                                              // vi tri dau sach
    for (int i = 0; i < n; ++i) {                             // duyet mang
        if (ds[i] && ds[i]->ISBN == isbn) {                   // tim thay
            idx = i;
            break;
        }
    }
    
    if (idx == -1)      return "Loi: Khong tim thay ISBN: " + isbn;  // khong tim thay
    if (ds[idx]->dms)   return "Loi: Con ban sao, khong xoa duoc!";  // con ban sao
    
    GiaiPhongDauSach(ds[idx]);                                // giai phong bo nho
    
    for (int i = idx; i < n - 1; ++i) {                       // dich mang sang trai
        ds[i] = ds[i + 1];
    }
    
    ds[--n] = nullptr;                                         // giam so luong
    duLieuDaThayDoi = true;                                    // danh dau thay doi
    // Emit event: remove book
    try {
        EventBus::instance().dauSachRemoved(QString::fromStdString(isbn));
    } catch(...) {}
    return "";                                                 // thanh cong
}

// xoa sach theo ma sach
bool XoaSachTheoMaSach(PTRDS ds[], int n, const std::string &ma, std::ostream &out) {
    std::string isbn = TachISBNTuMaSach(ma);                  // lay ISBN tu ma sach
    if (isbn.empty()) {                                        // ma khong hop le
        thongBao(out, "Ma sach khong hop le!", LOI);
        return false;
    }
    
    PTRDS d = TimDauSachTheoISBN(ds, n, isbn);                // tim dau sach
    if (!d) {                                                  // khong tim thay
        thongBao(out, "Khong tim thay dau sach!", LOI);
        return false;
    }

    PTRDMS curr = d->dms;                                      // node hien tai
    PTRDMS prev = nullptr;                                     // node truoc
    
    while (curr) {                                             // duyet DSLK don
        if (curr->maSach == ma) {                             // tim thay ma sach
            // chi khong cho xoa sach DANG_MUON
            // con CHO_MUON_DUOC va THANH_LY thi duoc phep xoa
            if (curr->trangThai == DANG_MUON) {               // dang muon
                thongBao(out, "Khong xoa duoc sach dang muon!", LOI);
                return false;
            }
            
            if (prev)   prev->next = curr->next;              // xoa node giua/cuoi
            else        d->dms = curr->next;                  // xoa node dau
            
            delete curr;                                       // giai phong bo nho
            d->tongBanSao--;                                   // giam so luong
            duLieuDaThayDoi = true;                            // danh dau thay doi
            
            //Neu xoa het ban sao, canh bao nen xoa dau sach
            if (d->tongBanSao == 0) {
                thongBao(out, "Xoa thanh cong! Luu y: Da xoa het ban sao, nen xoa dau sach!", THONG_TIN);
            } else {
                thongBao(out, "Xoa thanh cong!", THONG_TIN);
            }
            return true;
        }
        
        prev = curr;                                           // luu node truoc
        curr = curr->next;                                     // node tiep theo
    }
    
    thongBao(out, "Khong tim thay ma sach!", LOI);
    return false;
}

// cap nhat dau sach
std::string CapNhatDauSach(PTRDS ds[], int n, const std::string &isbn, const std::string &ten, 
                          int trang, const std::string &tg, int nam, const std::string &tl, const std::string &viTri, long long giaTien = -1) {
    PTRDS d = TimDauSachTheoISBN(ds, n, isbn);                // tim dau sach
    if (!d) return "Loi: Khong tim thay ISBN!";               // khong tim thay
    
    std::string loi;                                           // bien luu loi
    
    // kiem tra validation
    if (!(loi = KiemTraChuoiVaDodai(ten, "Ten sach", MAX_TEN_SACH)).empty() ||        // kiem tra ten
        trang <= 0 || trang > MAX_SO_TRANG ||                                         // kiem tra trang
        !(loi = KiemTraChuoiVaDodai(tg, "Tac gia", MAX_TAC_GIA)).empty() ||          // kiem tra tac gia
        !(loi = KiemTraNamXuatBan(nam)).empty() ||                                    // kiem tra nam
        !(loi = KiemTraChuoiVaDodai(tl, "The loai", MAX_THE_LOAI)).empty()) {        // kiem tra the loai
        return loi;                                            // tra ve loi
    }
    
    // kiem tra va chuan hoa vi tri (neu co)
    std::string viTriChuan = viTri;
    if (!viTri.empty()) {
        viTriChuan = ChuanHoaViTri(viTri);
        if (viTriChuan.empty()) {
            return "Loi: Vi tri khong hop le! Vd: A-12, B5, Ke C-3";
        }
    }

    bool tenDoi = (d->tenSach != ChuyenThanhTitleCase(ten));  // kiem tra ten doi
    d->tenSach = ChuyenThanhTitleCase(ten);                   // cap nhat ten
    d->soTrang = trang;                                        // cap nhat trang
    d->tacGia = ChuyenThanhTitleCase(tg);                     // cap nhat tac gia
    d->namXuatBan = nam;                                       // cap nhat nam
    d->theLoai = ChuyenThanhTitleCase(tl);                    // cap nhat the loai
    if (giaTien != -1) d->giaTien = giaTien;                   // cap nhat gia tien
    
    // cap nhat vi tri cho tat ca ban sao (neu co)
    if (!viTriChuan.empty()) {
        PTRDMS p = d->dms;
        while (p) {
            p->viTri = viTriChuan;                             // cap nhat vi tri
            p = p->next;
        }
    }
    
    if (tenDoi) {                                              // neu ten doi
        sapXepDauSachTheoTen(ds, 0, n - 1);                   // sap xep lai
    }
    
    duLieuDaThayDoi = true;                                    // danh dau thay doi
    // Emit event: book updated
    try {
        EventBus::instance().dauSachUpdated(QString::fromStdString(isbn));
    } catch(...) {}
    return "";                                                 // thanh cong
}

// cap nhat vi tri cho tat ca ban sao cua dau sach O(M)
std::string CapNhatViTriTatCaBanSao(PTRDS dauSach, const std::string &viTri) {
    if (!dauSach) {
        return "Loi: Dau sach khong ton tai!";
    }
    
    if (!dauSach->dms) {
        return "Loi: Dau sach chua co ban sao nao!";
    }
    
    // kiem tra va chuan hoa vi tri
    std::string viTriChuan = ChuanHoaViTri(viTri);
    if (viTriChuan.empty()) {
        return "Loi: Vi tri khong hop le! Chi duoc chua chu cai va so. VD: A-12, B5, Ke C-3";
    }
    
    int capNhatThanhCong = 0;                                  // dem so ban sao cap nhat
    PTRDMS pTemp = dauSach->dms;                               // bat dau tu dau DSLK
    int baoVe = 0;                                             // tranh loop vo tan
    
    while (pTemp && baoVe < MAX_BAN_SAO) {                    // duyet het DSLK
        if (pTemp->viTri != viTriChuan) {                      // chi cap nhat neu khac
            pTemp->viTri = viTriChuan;                         // cap nhat vi tri
            capNhatThanhCong++;                                // tang bo dem
        }
        pTemp = pTemp->next;                                   // node tiep theo
        baoVe++;                                               // tang bao ve
    }
    
    if (capNhatThanhCong > 0) {                                // co thay doi
        duLieuDaThayDoi = true;                                // danh dau thay doi
        // Notify listeners that copies (DMS) for this DauSach changed
        try { EventBus::instance().dauSachUpdated(QString::fromStdString(dauSach->ISBN)); } catch(...) {}
    }
    
    return "";                                                 // thanh cong
}

// kiem tra co the xoa dau sach khong O(M)
bool KiemTraCoTheXoaDauSach(PTRDS dauSach) {
    if (!dauSach) return false;                                // null pointer
    
    // neu chua co ban sao nao -> co the xoa ngay
    if (!dauSach->dms) return true;
    
    // kiem tra tat ca ban sao
    PTRDMS pTemp = dauSach->dms;
    int baoVe = 0;
    
    while (pTemp && baoVe < MAX_BAN_SAO) {
        // neu co bat ky ban sao nao dang muon -> khong xoa duoc
        if (pTemp->trangThai == DANG_MUON) {
            return false;
        }
        pTemp = pTemp->next;
        baoVe++;
    }
    
// neu khong co ban sao nao dang muon -> co the xoa
    return true;
}

// ==========================================
// DATABASE INTEGRATION
// ==========================================
// DATABASE INTEGRATION
// ==========================================

void NapDanhSachTuDB() {
    // Clean up old memory
    GiaiPhongToanBoDauSach(dsDauSach, soLuongDauSach);
    
    QSqlDatabase db = DatabaseManager::instance().database();
    if (!db.isOpen()) {
        std::cout << "[DB DEBUG] Database is NOT open in NapDanhSachTuDB!\n";
        return;
    }
    
    std::cout << "[DB DEBUG] NapDanhSachTuDB: Starting load from " << db.databaseName().toStdString() << "\n";

    // 1. Load Dau Sach
    QSqlQuery q(db);
    if (q.exec("SELECT id, isbn, ten_sach, so_trang, tac_gia, nam_xuat_ban, the_loai, so_luot_muon, gia_tien FROM dau_sach ORDER BY id")) {
        while (q.next()) {
            int id = q.value(0).toInt();
            Q_UNUSED(id);
            std::string isbn = q.value(1).toString().toStdString();
            std::string ten = q.value(2).toString().toStdString();
            int trang = q.value(3).toInt();
            std::string tg = q.value(4).toString().toStdString();
            int nam = q.value(5).toInt();
            std::string tl = q.value(6).toString().toStdString();
            int luotMuon = q.value(7).toInt();
            long long gia = q.value(8).toLongLong();

            themDauSach(dsDauSach, soLuongDauSach, isbn, ten, trang, tg, nam, tl, true, gia);
            
            // Update soLuotMuon
            PTRDS p = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
            if (p) p->soLuotMuon = luotMuon;
        }
    } else {
        std::cout << "[DB DEBUG] Failed to execute query: " << q.lastError().text().toStdString() << "\n";
    }
    
    // 2. Load Ban Sao
    if (q.exec("SELECT ma_sach, dau_sach_id, trang_thai, vi_tri FROM ban_sao")) {
        while (q.next()) {
            std::string ma = q.value(0).toString().toStdString();
            // We need to link back to the book.
            // Since we ordered by ID in DauSach query, and we are processing BanSao...
            // Wait, we need the ISBN. The legacy way used ISBN from file or looked up by ISBN.
            // Here we have dau_sach_id. We can't map ID->ptr easily without a map.
            // But we can query the ISBN for this ID.
            int dauId = q.value(1).toInt();
            int tt = q.value(2).toInt();
            std::string vt = q.value(3).toString().toStdString();
            
            QSqlQuery q2(db);
            q2.prepare("SELECT isbn FROM dau_sach WHERE id = ?");
            q2.addBindValue(dauId);
            if (q2.exec() && q2.next()) {
                std::string isbn = q2.value(0).toString().toStdString();
                PTRDS ds = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
                if (ds) {
                    // Use low-level linked list insertion (like legacy NapDanhMucSach)
                     ChenNodeDMSVaoDauSach(ds, ma, tt, vt);
                }
            }
        }
    }
    
    CapNhatTongBanSao(dsDauSach, soLuongDauSach);
    sapXepDauSachTheoTen(dsDauSach, 0, soLuongDauSach - 1);
    std::cout << "[DB DEBUG] Finished loading. Total books: " << soLuongDauSach << "\n";
}    




//  NHOM 8: SERVICE LAYER CHO UI (ENCAPSULATION - SEPARATION OF CONCERNS)


// lay con tro mang dau sach (read-only, UI chi doc khong ghi)
PTRDS* LayDanhSachDauSach() {
    return dsDauSach;  // tra ve con tro mang
}

// Define the global mutex declared in the header
std::mutex g_ds_mutex;

// lay so luong dau sach hien tai
int LaySoLuongDauSach() {
    return soLuongDauSach;
}

// kiem tra du lieu co thay doi (de biet co can luu file)
bool KiemTraDuLieuThayDoi() {
    return duLieuDaThayDoi;
}


//  TRUY VAN - TIM KIEM & LOC DU LIEU 

// lay danh sach tim kiem (gom logic "lay tat ca" neu khong co tu khoa)
void LayDanhSachTimKiem(const std::string& tuKhoa, KetQuaTimKiem* ketQua, int& soLuongKetQua, int maxKetQua) {
    soLuongKetQua = 0;  // reset ket qua cu

    if (!ketQua || maxKetQua <= 0) return;  // validate input

    std::string tuKhoaChuan = ChuanHoaKhoangTrang(tuKhoa);

    if (tuKhoaChuan.empty()) {
        // Logic "Get All" - Copy toan bo danh sach (da duoc sap xep)
        for (int i = 0; i < soLuongDauSach && soLuongKetQua < maxKetQua; ++i) {
            if (!dsDauSach[i]) continue; // skip null entries
            ketQua[soLuongKetQua].sach = dsDauSach[i];
            ketQua[soLuongKetQua].loaiKhop = 0;  // 0 = tat ca
            soLuongKetQua++;
        }
    } else {
        // Tim kiem logic: use an internal temp buffer to protect against
        // callers that pass a smaller maxKetQua than internal limits.
        KetQuaTimKiem tmp[MAX_KET_QUA_TIM_KIEM];
        int found = timKiemLogic(dsDauSach, soLuongDauSach, tuKhoaChuan, tmp);

        if (found <= 0) {
            soLuongKetQua = 0;
            return;
        }

        // Copy up to caller's capacity (maxKetQua) from tmp results
        int toCopy = found;
        if (toCopy > maxKetQua) toCopy = maxKetQua;
        for (int i = 0; i < toCopy; ++i) {
            // defensive: ensure tmp entry is valid
            if (!tmp[i].sach) continue;
            ketQua[soLuongKetQua++] = tmp[i];
        }
    }
}

// lay danh sach the loai duy nhat (tu dong sap xep)
void LayDanhSachTheoTheLoai(std::string* cacTheLoai, int& soTheLoai) {
    if (!cacTheLoai) {
        soTheLoai = 0;
        return;
    }
    soTheLoai = TimTheLoaiDuyNhat(dsDauSach, soLuongDauSach, cacTheLoai);
}

// lay danh sach vi tri duy nhat
void LayDanhSachViTri(std::string* cacViTri, int& soViTri) {
    if (!cacViTri) {
        soViTri = 0;
        return;
    }
    soViTri = TimViTriDuyNhat(dsDauSach, soLuongDauSach, cacViTri);
}

// lay danh sach sach cung the loai (da sap xep tang dan theo ten)
// Logic: Vi dsDauSach luon duoc sap xep sau moi thao tac them/sua (auto sort),
//        nen ket qua filter theo the loai se tu dong da sap xep theo ten sach
int LayDanhSachSachTheoTheLoai(const std::string& theLoai, PTRDS* ketQua, int maxKetQua) {
    if (!ketQua || maxKetQua <= 0) return 0;
    return TimSachTheoTheLoai(dsDauSach, soLuongDauSach, theLoai, ketQua);
}

// tim 1 dau sach theo ISBN (wrapper)
PTRDS TimDauSach(const std::string& isbn) {
    return TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
}

// lay thong tin tong quan dau sach (wrapper)
ThongTinDauSach LayThongTinDauSach(const std::string& isbn) {
    PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
    if (!dauSach) {
        return {0, 0, 0, 0};  // tra ve struct rong
    }
    return LayThongTinDauSach(dauSach);
}


//  COMMAND - THAO TAC DU LIEU (CREATE, UPDATE, DELETE) 

// them dau sach + ban sao (all-in-one, tu dong sap xep)
std::string ThemDauSachMoi(const std::string& isbn, const std::string& ten, int trang,
                           const std::string& tg, int nam, const std::string& tl, 
                           int soLuongBanSao, const std::string& viTri, long long giaTien) {
    // validation so luong ban sao
    if (soLuongBanSao <= 0) {
        return "Loi: So luong ban sao phai lon hon 0!";
    }
    if (soLuongBanSao > MAX_BAN_SAO) {
        return "Loi: So luong ban sao vuot qua gioi han " + std::to_string(MAX_BAN_SAO) + "!";
    }
    
    // goi backend them dau sach
    bool ok = themDauSach(dsDauSach, soLuongDauSach, isbn, ten, trang, tg, nam, tl, true, giaTien);
    if (!ok) {
        return "Loi: Khong them duoc dau sach (Backend error)!";
    }
    
    // tim lai dau sach vua them
    PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
    if (!dauSach) {
        return "Loi: Khong tim thay dau sach vua them!";
    }
    
    // logic tu dong lay vi tri (Business Rule)
    std::string viTriCanDung = viTri;
    if (viTriCanDung.empty() && dauSach->dms && !dauSach->dms->viTri.empty()) {
        viTriCanDung = dauSach->dms->viTri;  // lay tu ban sao cu
    }
    
    // them ban sao
    std::string loiThem = ThemNhieuBanSao(dauSach, soLuongBanSao, viTriCanDung);
    if (!loiThem.empty()) {
        return loiThem;
    }
    
    // tu dong sap xep sau khi them thanh cong
    sapXepDauSachTheoTen(dsDauSach, 0, soLuongDauSach - 1);
    duLieuDaThayDoi = true;
    
    // --- DB SYNC ---
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("INSERT INTO dau_sach (isbn, ten_sach, so_trang, tac_gia, nam_xuat_ban, the_loai, gia_tien) VALUES (?, ?, ?, ?, ?, ?, ?)");
    q.addBindValue(QString::fromStdString(isbn));
    q.addBindValue(QString::fromStdString(ChuyenThanhTitleCase(ten)));
    q.addBindValue(trang);
    q.addBindValue(QString::fromStdString(ChuyenThanhTitleCase(tg)));
    q.addBindValue(nam);
    q.addBindValue(QString::fromStdString(ChuyenThanhTitleCase(tl)));
    q.addBindValue((qint64)giaTien);
    
    int newBookId = -1;
    if (q.exec()) {
        newBookId = q.lastInsertId().toInt();
    } else {
        std::cout << "[DB ERROR] Failed to insert dau_sach: " << q.lastError().text().toStdString() << "\n";
    }

    // Sync newly added copies (they are at the head of the list)
    if (newBookId != -1 && dauSach && dauSach->dms) {
        PTRDMS p = dauSach->dms;
        int count = 0;
        DatabaseManager::instance().database().transaction();
        while (p && count < soLuongBanSao) {
            QSqlQuery qc(DatabaseManager::instance().database());
            // FIXED: Added dau_sach_id column
            qc.prepare("INSERT INTO ban_sao (ma_sach, dau_sach_id, trang_thai, vi_tri) VALUES (?, ?, ?, ?)");
            qc.addBindValue(QString::fromStdString(p->maSach));
            qc.addBindValue(newBookId); 
            qc.addBindValue((int)p->trangThai);
            qc.addBindValue(QString::fromStdString(p->viTri));
            if (!qc.exec()) {
                 std::cout << "[DB ERROR] Failed to insert ban_sao: " << qc.lastError().text().toStdString() << "\n";
            }
            
            p = p->next;
            count++;
        }
        DatabaseManager::instance().database().commit();
    }
    // --- END DB SYNC ---
    
    // [AUDIT LOG]
    AuditLogger::log(AuditLogger::ACTION_INSERT, "DAU_SACH", QString::fromStdString(isbn), 
                     "Them dau sach moi: " + QString::fromStdString(ten));
    
    return "";  // thanh cong
}

// them ban sao vao dau sach da co (logic tu dong lay vi tri)
std::string ThemBanSaoMoi(const std::string& isbn, int soLuong, const std::string& viTri) {
    // validation
    if (soLuong <= 0) {
        return "Loi: So luong phai lon hon 0!";
    }
    if (soLuong > MAX_BAN_SAO) {
        return "Loi: So luong vuot qua gioi han " + std::to_string(MAX_BAN_SAO) + "!";
    }
    
    // tim dau sach
    PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
    if (!dauSach) {
        return "Loi: Khong tim thay dau sach ISBN '" + isbn + "'!";
    }
    
    // logic tu dong lay vi tri (Business Rule - giấu kín ở Backend)
    std::string viTriCanDung = viTri;
    if (viTriCanDung.empty() && dauSach->dms && !dauSach->dms->viTri.empty()) {
        viTriCanDung = dauSach->dms->viTri;  // lay tu ban sao cu
    }
    
    // goi backend them
    std::string loiThem = ThemNhieuBanSao(dauSach, soLuong, viTriCanDung);
    if (!loiThem.empty()) {
        return loiThem;
    }
    
    duLieuDaThayDoi = true;
    // Notify listeners that a copy's status changed (thanh ly)
    try { EventBus::instance().dauSachUpdated(QString::fromStdString(isbn)); } catch(...) {}

    // --- DB SYNC ---
    // Update newly added copies (head of list)
    if (dauSach && dauSach->dms) {
        // Query ID of the book
        int bookId = -1;
        QSqlQuery qId(DatabaseManager::instance().database());
        qId.prepare("SELECT id FROM dau_sach WHERE isbn = ?");
        qId.addBindValue(QString::fromStdString(isbn));
        if (qId.exec() && qId.next()) {
            bookId = qId.value(0).toInt();
        }

        if (bookId != -1) {
            PTRDMS p = dauSach->dms;
            int count = 0;
            DatabaseManager::instance().database().transaction();
            while (p && count < soLuong) {
                QSqlQuery qc(DatabaseManager::instance().database());
                qc.prepare("INSERT INTO ban_sao (ma_sach, dau_sach_id, trang_thai, vi_tri) VALUES (?, ?, ?, ?)");
                qc.addBindValue(QString::fromStdString(p->maSach));
                qc.addBindValue(bookId);
                qc.addBindValue((int)p->trangThai);
                qc.addBindValue(QString::fromStdString(p->viTri));
                if (!qc.exec()) {
                    std::cout << "[DB ERROR] Failed to insert ban_sao (ThemBanSaoMoi): " << qc.lastError().text().toStdString() << "\n";
                }
                
                p = p->next;
                count++;
            }
            DatabaseManager::instance().database().commit();
        } else {
             std::cout << "[DB ERROR] Cannot find book ID for ISBN: " << isbn << " to add copies.\n";
        }
    }
    // --- END DB SYNC ---

    // [AUDIT LOG]
    AuditLogger::log(AuditLogger::ACTION_INSERT, "BAN_SAO", QString::fromStdString(isbn), 
                     QString("Them %1 ban sao vao dau sach %2").arg(soLuong).arg(QString::fromStdString(isbn)));

    return "";  // thanh cong
}

// cap nhat thong tin dau sach (tu dong sap xep neu ten doi)
std::string CapNhatThongTinDauSach(const std::string& isbn, const std::string& ten, int trang,
                                   const std::string& tg, int nam, const std::string& tl, 
                                   const std::string& viTri, long long giaTien) {
    // goi backend cap nhat
    std::string loi = CapNhatDauSach(dsDauSach, soLuongDauSach, isbn, ten, trang, tg, nam, tl, viTri, giaTien);
    
    if (loi.empty()) {
        // tu dong sap xep sau khi cap nhat (phong truong hop ten thay doi)
        sapXepDauSachTheoTen(dsDauSach, 0, soLuongDauSach - 1);
        duLieuDaThayDoi = true;
        
        // --- DB SYNC ---
        QSqlQuery q(DatabaseManager::instance().database());
        if (giaTien != -1) {
            q.prepare("UPDATE dau_sach SET ten_sach=?, so_trang=?, tac_gia=?, nam_xuat_ban=?, the_loai=?, gia_tien=? WHERE isbn=?");
            q.addBindValue(QString::fromStdString(ChuyenThanhTitleCase(ten)));
            q.addBindValue(trang);
            q.addBindValue(QString::fromStdString(ChuyenThanhTitleCase(tg)));
            q.addBindValue(nam);
            q.addBindValue(QString::fromStdString(ChuyenThanhTitleCase(tl)));
            q.addBindValue((qint64)giaTien);
            q.addBindValue(QString::fromStdString(isbn));
        } else {
            q.prepare("UPDATE dau_sach SET ten_sach=?, so_trang=?, tac_gia=?, nam_xuat_ban=?, the_loai=? WHERE isbn=?");
            q.addBindValue(QString::fromStdString(ChuyenThanhTitleCase(ten)));
            q.addBindValue(trang);
            q.addBindValue(QString::fromStdString(ChuyenThanhTitleCase(tg)));
            q.addBindValue(nam);
            q.addBindValue(QString::fromStdString(ChuyenThanhTitleCase(tl)));
            q.addBindValue(QString::fromStdString(isbn));
        }
        q.exec();
        
        // Update position for all copies if provided
        if (!viTri.empty()) {
             QSqlQuery qc(DatabaseManager::instance().database());
             qc.prepare("UPDATE ban_sao SET vi_tri=? WHERE ma_sach LIKE ?");
             qc.addBindValue(QString::fromStdString(ChuanHoaViTri(viTri)));
             qc.addBindValue(QString::fromStdString(isbn + "-%"));
             qc.exec();
        }
        // --- END DB SYNC ---
        
        // [AUDIT LOG]
        AuditLogger::log(AuditLogger::ACTION_UPDATE, "DAU_SACH", QString::fromStdString(isbn), 
                         "Cap nhat thong tin dau sach: " + QString::fromStdString(ten));
    }
    
    return loi;  // rong = thanh cong, khac rong = loi
}

// xoa dau sach (validation + auto sort) - HAM PUBLIC CHO UI
std::string XoaDauSachTheoISBN(const std::string& isbn) {
    // tim dau sach
    PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
    if (!dauSach) {
        return "Loi: Khong tim thay dau sach de xoa!";
    }
    
    // kiem tra co the xoa (khong co ban sao dang muon)
    if (dauSach->dms != nullptr) {
        return "Loi: Khong the xoa vi con ban sao! Vui long xoa het ban sao truoc.";
    }
    
    // goi ham noi bo de xoa
    std::string loi = XoaDauSach(dsDauSach, soLuongDauSach, isbn);
    
    if (loi.empty()) {
        // tu dong sap xep sau khi xoa
        if (soLuongDauSach > 1) {
            sapXepDauSachTheoTen(dsDauSach, 0, soLuongDauSach - 1);
        }
        duLieuDaThayDoi = true;
        
        // --- DB SYNC ---
        QSqlQuery q(DatabaseManager::instance().database());
        q.prepare("DELETE FROM dau_sach WHERE isbn=?");
        q.addBindValue(QString::fromStdString(isbn));
        q.exec();
        // --- END DB SYNC ---

        // [AUDIT LOG]
        AuditLogger::log(AuditLogger::ACTION_DELETE, "DAU_SACH", QString::fromStdString(isbn), "Xoa dau sach");
    }
    
    return loi;
}

// thanh ly 1 ban sao (cap nhat trang thai)
std::string ThanhLyBanSaoTheoMa(const std::string& maSach) {
    // tach ISBN tu ma sach
    std::string isbn = TachISBNTuMaSach(maSach);
    if (isbn.empty()) {
        return "Loi: Ma sach khong hop le!";
    }
    
    // tim dau sach
    PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
    if (!dauSach) {
        return "Loi: Khong tim thay dau sach!";
    }
    
    // cap nhat trang thai
    bool thanhCong = capNhatTrangThaiSach(dauSach->dms, maSach, THANH_LY);
    if (!thanhCong) {
        return "Loi: Khong tim thay ban sao de thanh ly!";
    }
    
    duLieuDaThayDoi = true;
    // Notify listeners that a copy has been marked as disposed
    try { EventBus::instance().dauSachUpdated(QString::fromStdString(isbn)); } catch(...) {}
    
    // --- DB SYNC ---
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("UPDATE ban_sao SET trang_thai=? WHERE ma_sach=?");
    q.addBindValue(THANH_LY);
    q.addBindValue(QString::fromStdString(maSach));
    q.exec();
    // --- END DB SYNC ---
    
    return "";  // thanh cong
}

// xoa 1 ban sao (validation + cap nhat tong)
std::string XoaBanSaoTheoMa(const std::string& maSach) {
    // tim ban sao
    std::stringstream dummy;
    PTRDMS banSao = timDanhMucTheoMaSach(maSach, dsDauSach, soLuongDauSach, dummy, true);
    
    if (!banSao) {
        return "Loi: Khong tim thay ban sao de xoa!";
    }
    
    // kiem tra trang thai (chi xoa duoc khi khong DANG_MUON)
    if (banSao->trangThai == DANG_MUON) {
        return "Loi: Sach dang muon, khong the xoa!";
    }
    
    // goi backend xoa
    bool thanhCong = XoaSachTheoMaSach(dsDauSach, soLuongDauSach, maSach, dummy);
    if (!thanhCong) {
        return "Loi: Khong the xoa ban sao!";
    }
    
    // cap nhat lai tong so ban sao
    CapNhatTongBanSao(dsDauSach, soLuongDauSach);
    duLieuDaThayDoi = true;
    // Notify listeners that a DauSach was modified (copy removed)
    try { EventBus::instance().dauSachUpdated(QString::fromStdString(TachISBNTuMaSach(maSach))); } catch(...) {}

    // --- DB SYNC ---
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("DELETE FROM ban_sao WHERE ma_sach=?");
    q.addBindValue(QString::fromStdString(maSach));
    q.exec();
    // --- END DB SYNC ---
    
    return "";  // thanh cong
}


//  UTILITY - HO TRO UI 

// danh dau du lieu da thay doi (de UI biet can luu file)
void DanhDauDuLieuThayDoi() {
    duLieuDaThayDoi = true;
}

// lay danh sach ban sao da sap xep theo ma (che giau DSLK)
int LayDanhSachBanSaoSapXep(const std::string& isbn, PTRDMS* mangKetQua, int maxKetQua) {
    if (!mangKetQua || maxKetQua <= 0) return 0;
    
    PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
    if (!dauSach) return 0;
    
    // Chuyen DSLK sang mang
    int count = 0;
    PTRDMS p = dauSach->dms;
    int safety = 0;
    int limit = (maxKetQua > MAX_BAN_SAO) ? MAX_BAN_SAO : maxKetQua;
    while (p != nullptr && count < limit && safety < SO_VONG_LAP_DMS_MAX) {
        // Defensive: if node looks invalid, stop collecting further
        if (p == nullptr) break;
        mangKetQua[count++] = p;
        // Detect trivial self-loop
        if (p->next == p) {
            thongBao(std::cout, "Phat hien node DMS self-loop, cat vong lap.", CANH_BAO);
            break;
        }
        p = p->next;
        ++safety;
    }
    
    // Sap xep theo ma sach
    if (count > 1) SapXepBanSaoTheoMa(mangKetQua, count);
    
    return count;
}

// lay vi tri cua ban sao dau tien (che giau truy cap ->dms)
std::string LayViTriDauTienCuaDauSach(const std::string& isbn) {
    PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
    if (!dauSach || !dauSach->dms) return "";
    return dauSach->dms->viTri;
}

// kiem tra dau sach co ban sao hay khong (business rule)
bool CoTheBanSao(const std::string& isbn) {
    PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
    return (dauSach && dauSach->dms != nullptr);
}


//  NHOM 9: THONG KE - STATISTICS FUNCTIONS 

// lay thong ke toan he thong O(N*M)
ThongKeSach LayThongKeSach() {
    ThongKeSach thongKe{};
    
    thongKe.tongSoDauSach = soLuongDauSach;
    
    // dem so the loai khac nhau
    std::string cacTheLoai[MAX_DAUSACH];
    thongKe.soTheLoaiKhacNhau = TimTheLoaiDuyNhat(dsDauSach, soLuongDauSach, cacTheLoai);
    
    // duyet qua tung dau sach
    for (int i = 0; i < soLuongDauSach; i++) {
        if (!dsDauSach[i]) continue;
        
        // duyet qua tung ban sao
        PTRDMS pTemp = dsDauSach[i]->dms;
        while (pTemp) {
            thongKe.tongSoBanSao++;
            
            if (pTemp->trangThai == CHO_MUON_DUOC) {
                thongKe.soBanSaoCoTheMuon++;
            } else if (pTemp->trangThai == DANG_MUON) {
                thongKe.soBanSaoDangMuon++;
            } else if (pTemp->trangThai == THANH_LY) {
                thongKe.soBanSaoThanhLy++;
            }
            
            pTemp = pTemp->next;
        }
    }
    
    // tinh ty le
    if (thongKe.tongSoBanSao > 0) {
        thongKe.tyLeChoMuon = (double)thongKe.soBanSaoCoTheMuon * 100.0 / thongKe.tongSoBanSao;
    }
    
    return thongKe;
}

// dem ban sao theo trang thai cua 1 dau sach O(M)
int DemSachTheoTrangThai(PTRDS dauSach, TrangThaiSach trangThai) {
    if (!dauSach) return 0;
    
    int dem = 0;
    PTRDMS pTemp = dauSach->dms;
    
    while (pTemp) {
        if (pTemp->trangThai == trangThai) {
            dem++;
        }
        pTemp = pTemp->next;
    }
    
    return dem;
}

// dem tong so ban sao theo trang thai trong he thong O(N*M)
int DemTongSoBanSaoTheoTrangThai(TrangThaiSach trangThai) {
    int tong = 0;
    
    for (int i = 0; i < soLuongDauSach; i++) {
        if (!dsDauSach[i]) continue;
        tong += DemSachTheoTrangThai(dsDauSach[i], trangThai);
    }
    
    return tong;
}


//  NHOM 10: TIM KIEM NANG CAO 

// tim sach theo nam xuat ban O(N)
int TimSachTheoNamXuatBan(PTRDS ds[], int n, int nam, PTRDS kq[]) {
    if (!ds || !kq || n <= 0) return 0;
    
    int count = 0;
    for (int i = 0; i < n && i < MAX_DAUSACH; i++) {
        if (!ds[i]) continue;
        
        if (ds[i]->namXuatBan == nam) {
            kq[count++] = ds[i];
        }
    }
    
    // sap xep ket qua theo ten
    if (count > 1) {
        sapXepDauSachTheoTen(kq, 0, count - 1);
    }
    
    return count;
}

// tim sach trong khoang nam O(N)
int TimSachTheoKhoangNam(PTRDS ds[], int n, int namBatDau, int namKetThuc, PTRDS kq[]) {
    if (!ds || !kq || n <= 0) return 0;
    if (namBatDau > namKetThuc) {
        // swap neu sai thu tu
        int temp = namBatDau;
        namBatDau = namKetThuc;
        namKetThuc = temp;
    }
    
    int count = 0;
    for (int i = 0; i < n && i < MAX_DAUSACH; i++) {
        if (!ds[i]) continue;
        
        if (ds[i]->namXuatBan >= namBatDau && ds[i]->namXuatBan <= namKetThuc) {
            kq[count++] = ds[i];
        }
    }
    
    // sap xep ket qua theo nam giam dan (moi nhat truoc)
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (kq[i]->namXuatBan < kq[j]->namXuatBan) {
                hoanDoi(kq[i], kq[j]);
            }
        }
    }
    
    return count;
}

// loc sach co ban sao voi trang thai cu the O(N*M)
int LocSachTheoTrangThai(PTRDS ds[], int n, TrangThaiSach trangThai, PTRDS kq[]) {
    if (!ds || !kq || n <= 0) return 0;
    
    int count = 0;
    for (int i = 0; i < n && i < MAX_DAUSACH; i++) {
        if (!ds[i]) continue;
        
        // kiem tra dau sach co ban sao voi trang thai nay khong
        PTRDMS pTemp = ds[i]->dms;
        bool coTrangThai = false;
        
        while (pTemp) {
            if (pTemp->trangThai == trangThai) {
                coTrangThai = true;
                break;
            }
            pTemp = pTemp->next;
        }
        
        if (coTrangThai) {
            kq[count++] = ds[i];
        }
    }
    
    // sap xep ket qua theo ten
    if (count > 1) {
        sapXepDauSachTheoTen(kq, 0, count - 1);
    }
    
    return count;
}

// tim sach theo tac gia (bo dau, khong phan biet hoa/thuong) O(N)
int TimSachTheoTacGia(PTRDS ds[], int n, const std::string &tacGia, PTRDS kq[]) {
    if (!ds || !kq || n <= 0 || tacGia.empty()) return 0;
    
    // Bo dau va chuyen thuong cho tu khoa tim kiem
    std::string tacGiaBoDau = BoDauVaThuong(tacGia);
    
    int count = 0;
    for (int i = 0; i < n && i < MAX_DAUSACH; i++) {
        if (!ds[i]) continue;
        
        // Bo dau va chuyen thuong cho du lieu
        std::string tacGiaSachBoDau = BoDauVaThuong(ds[i]->tacGia);
        
        // kiem tra chua chuoi tim kiem
        if (tacGiaSachBoDau.find(tacGiaBoDau) != std::string::npos) {
            kq[count++] = ds[i];
        }
    }
    
    // sap xep ket qua theo ten
    if (count > 1) {
        sapXepDauSachTheoTen(kq, 0, count - 1);
    }
    
    return count;
}

