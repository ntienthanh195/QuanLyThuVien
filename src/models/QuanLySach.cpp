#include "models/QuanLySach.h"
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
    while (p) {                                                 // duyet DSLK don
        if (p->maSach == ma) return p;                         // tim thay - early return
        p = p->next;                                            // node tiep theo
    }
    
    if (!silent) {                                             // khong tim thay trong DSLK
        thongBao(out, "Khong tim thay ma sach: " + ma + " trong danh muc cua ISBN=" + isbn, LOI);
    }
    return nullptr;
}



//  NHOM 3: QUAN LY DAU SACH (MANG CON TRO)


// them dau sach moi vao mang
bool themDauSach(PTRDS ds[], int &n, const std::string &isbn, const std::string &ten, int trang,
                 const std::string &tg, int nam, const std::string &tl, bool anLang) {
    
    if (n >= MAX_DAUSACH) {                                    // kiem tra gioi han
        if (!anLang) thongBao(std::cout, "Gioi han dau sach!", LOI);
        return false;
    }

    std::string loi, isbnChuan;
    
    // kiem tra ISBN
    if (!KiemTraChuoiRong(isbn, "ISBN").empty() ||
        !(loi = ChuanHoaISBNFile(isbn, isbnChuan)).empty() ||
        !(loi = KiemTraTrungISBN(isbnChuan)).empty()) {
        if (!anLang) thongBao(std::cout, loi, LOI);
        return false;
    }

    // kiem tra cac truong khac
    if (!(loi = KiemTraChuoiVaDodai(ten, "Ten sach", MAX_TEN_SACH)).empty() ||
        trang <= 0 || trang > MAX_SO_TRANG ||
        !(loi = KiemTraChuoiVaDodai(tg, "Tac gia", MAX_TAC_GIA)).empty() ||
        !(loi = KiemTraNamXuatBan(nam)).empty() ||
        !(loi = KiemTraChuoiVaDodai(tl, "The loai", MAX_THE_LOAI)).empty()) {
        if (!anLang) thongBao(std::cout, loi, LOI);
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
    node->dms = nullptr;                                       // chua co ban sao
    node->tongBanSao = 0;                                      // tong = 0
    node->soLuotMuon = 0;                                      // khoi tao so luot muon

    ds[n++] = node;                                            // them vao mang
    sapXepDauSachTheoTen(ds, 0, n - 1);                        // sap xep lai
    duLieuDaThayDoi = true;                                    // danh dau thay doi
    
    if (!anLang) {
        thongBao(std::cout, "Them thanh cong: " + node->tenSach, THONG_TIN);
    }
    return true;
}

// tim dau sach theo ISBN - linear O(N)
PTRDS TimDauSachTheoISBN(PTRDS ds[], int n, const std::string &isbn) {
    for (int i = 0; i < n; ++i) {                              // duyet mang
        if (ds[i] && ds[i]->ISBN == isbn) {                    // tim thay - early return
            return ds[i];
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

// Phan hoach Lomuto O(N)
int PhanHoach(PTRDS arr[], int left, int right) {
    // Chon median-of-three lam pivot, dua ve cuoi
    if (left < 0 || right >= MAX_DAUSACH || left >= right)
        return left;                       // kiem tra hop le

    int pivotIdx = ChonPivot(arr, left, right);   // chon pivot
    hoanDoi(arr[pivotIdx], arr[right]);           // dua pivot ve cuoi
    PTRDS pivot = arr[right];                     // lay gia tri pivot
    if (!pivot) return left;                      // tranh loi null

    // Chia mang: tat ca <= pivot ve ben trai (Lomuto)
    int i = left - 1;                             // chi so vung nho
    for (int j = left; j < right; ++j) {
        if (!arr[j]) continue;                   // bo qua null
        if (arr[j]->tenSach <= pivot->tenSach) { // nho hon hoac bang pivot
            i++;
            hoanDoi(arr[i], arr[j]);            // dua sang ben trai
        }
    }

    // Dua pivot ve dung vi tri
    hoanDoi(arr[i + 1], arr[right]);             // dat pivot dung vi tri
    return i + 1;                                // tra ve vi tri pivot
}

// Quicksort dau sach theo ten O(N log N)
void sapXepDauSachTheoTen(PTRDS arr[], int left, int right) {
    if (left >= right || left < 0 || right >= MAX_DAUSACH)
        return;                          // dung khi mang rong, 1 phan tu, > max

    int p = PhanHoach(arr, left, right); // phan hoach, chia mang

    sapXepDauSachTheoTen(arr, left, p - 1);  // de quy trai
    sapXepDauSachTheoTen(arr, p + 1, right); // de quy phai
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
static int phanHoachKetQua(KetQuaTimKiem arr[], std::string tenThuong[], int left, int right) {
    if (left < 0 || right >= MAX_KET_QUA_TIM_KIEM || left > right) {  // kiem tra bounds
        return left;                                           // tra ve gia tri an toan
    }
    
    int loaiKhopPivot = arr[right].loaiKhop;                   // pivot = loai khop cuoi
    std::string tenPivot = tenThuong[right];                   // ten pivot
    int i = left - 1;                                          // chi so phan hoach
    
    for (int j = left; j < right; ++j) {
        bool canSwap = false;                                  // can doi vi tri
        
        // so sanh: loai khop truoc, neu bang thi so sanh ten
        if (arr[j].loaiKhop < loaiKhopPivot) {
            canSwap = true;                                    // loai khop nho hon
        } else if (arr[j].loaiKhop == loaiKhopPivot && tenThuong[j] < tenPivot) {
            canSwap = true;                                    // cung loai, ten nho hon
        }
        
        if (canSwap) {
            ++i;
            hoanDoiKetQua(arr[i], arr[j]);                     // doi ket qua
            // doi ca cache
            std::string temp = tenThuong[i];
            tenThuong[i] = tenThuong[j];
            tenThuong[j] = temp;
        }
    }
    
    hoanDoiKetQua(arr[i + 1], arr[right]);                     // dat pivot dung vi tri
    std::string temp = tenThuong[i + 1];
    tenThuong[i + 1] = tenThuong[right];
    tenThuong[right] = temp;
    
    return i + 1;                                              // tra ve vi tri pivot
}

// quicksort ket qua O(N log N) trung binh, O(N^2) xau nhat
static void sapXepNhanhKetQua(KetQuaTimKiem arr[], std::string tenThuong[], int left, int right) {
    if (left < right && left >= 0 && right < MAX_KET_QUA_TIM_KIEM) {
        int p = phanHoachKetQua(arr, tenThuong, left, right);        // phan hoach
        sapXepNhanhKetQua(arr, tenThuong, left, p - 1);              // de quy trai
        sapXepNhanhKetQua(arr, tenThuong, p + 1, right);             // de quy phai
    }
}

// quicksort ket qua O(N log N) trung binh, O(N^2) xau nhat
void sapXepKetQuaTimKiem(KetQuaTimKiem arr[], int n) {
    if (n <= 1) return;                                        // khong can sap xep
    
    // cache chuoi da chuyen thuong truoc khi sap xep
    std::string tenThuong[MAX_KET_QUA_TIM_KIEM];               // mang cache ten da chuyen thuong
    
    for (int i = 0; i < n; ++i) {                              // chuan bi cache
        if (arr[i].sach) {
            tenThuong[i] = ChuyenInThuong(arr[i].sach->tenSach);
        }
    }
    
    sapXepNhanhKetQua(arr, tenThuong, 0, n - 1);               // goi quicksort
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
                          int trang, const std::string &tg, int nam, const std::string &tl, const std::string &viTri) {
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



//  NHOM 8: SERVICE LAYER CHO UI (ENCAPSULATION - SEPARATION OF CONCERNS)


// lay con tro mang dau sach (read-only, UI chi doc khong ghi)
PTRDS* LayDanhSachDauSach() {
    return dsDauSach;  // tra ve con tro mang
}

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
            ketQua[soLuongKetQua].sach = dsDauSach[i];
            ketQua[soLuongKetQua].loaiKhop = 0;  // 0 = tat ca
            soLuongKetQua++;
        }
    } else {
        // Tim kiem logic (co sap xep ket qua ben trong)
        soLuongKetQua = timKiemLogic(dsDauSach, soLuongDauSach, tuKhoaChuan, ketQua);
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
                           int soLuongBanSao, const std::string& viTri) {
    // validation so luong ban sao
    if (soLuongBanSao <= 0) {
        return "Loi: So luong ban sao phai lon hon 0!";
    }
    if (soLuongBanSao > MAX_BAN_SAO) {
        return "Loi: So luong ban sao vuot qua gioi han " + std::to_string(MAX_BAN_SAO) + "!";
    }
    
    // goi backend them dau sach
    bool ok = themDauSach(dsDauSach, soLuongDauSach, isbn, ten, trang, tg, nam, tl, true);
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
    return "";  // thanh cong
}

// cap nhat thong tin dau sach (tu dong sap xep neu ten doi)
std::string CapNhatThongTinDauSach(const std::string& isbn, const std::string& ten, int trang,
                                   const std::string& tg, int nam, const std::string& tl, 
                                   const std::string& viTri) {
    // goi backend cap nhat
    std::string loi = CapNhatDauSach(dsDauSach, soLuongDauSach, isbn, ten, trang, tg, nam, tl, viTri);
    
    if (loi.empty()) {
        // tu dong sap xep sau khi cap nhat (phong truong hop ten thay doi)
        sapXepDauSachTheoTen(dsDauSach, 0, soLuongDauSach - 1);
        duLieuDaThayDoi = true;
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
    while (p != nullptr && count < maxKetQua) {
        mangKetQua[count++] = p;
        p = p->next;
    }
    
    // Sap xep theo ma sach
    SapXepBanSaoTheoMa(mangKetQua, count);
    
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
    ThongKeSach thongKe = {0};
    
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

