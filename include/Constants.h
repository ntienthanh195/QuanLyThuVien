#ifndef CONSTANTS_H
#define CONSTANTS_H

// gioi han kich thuoc
constexpr int MAX_DAUSACH = 1000;         // gioi han so dau sach
constexpr int MAX_BAN_SAO = 100;          // ioi han so ban sao moi dau sach
constexpr int MAX_KET_QUA_TIM_KIEM = 100; // so ket qua tim kiem toi da hien thi
constexpr int MAX_VI_TRI_KE = 50;         // do dai vi tri ke toi da
constexpr int MAX_TEN_SACH = 100;         // do dai toi da ten sach
constexpr int MAX_TAC_GIA = 50;           // do dai toi da ten tac gia
constexpr int MAX_THE_LOAI = 50;          // do dai toi da the loai
constexpr int DO_DAI_MA_SACH_MAX = 50;    // do dai toi da ma sach (ISBN-xxx)
constexpr int MAX_SO_TRANG = 10000;       // so trang toi da

// gioi han do dai toi thieu
constexpr int MIN_TEN_SACH = 1;           // do dai toi thieu ten sach
constexpr int MIN_TAC_GIA = 1;            // do dai toi thieu ten tac gia
constexpr int MIN_THE_LOAI = 1;           // do dai toi thieu the loai

// cau hinh ISBN
constexpr int DO_DAI_ISBN = 13;           // do dai ISBN-13 tieu chuan
constexpr int DO_DAI_ISBN_CHECKSUM = 12;  // do dai chuoi ISBN truoc checksum

// cau hinh Nam Xuat Ban
constexpr int NAM_XUAT_BAN_MIN = 1800;     // nam xuat ban toi thieu hop le
constexpr int NAM_XUAT_BAN_OFFSET_MAX = 0; // cho phep nam XB toi da la nam hien tai

// cau hinh Nhap Lieu
constexpr int SO_LAN_THU_TOI_DA = 3;       // so lan thu nhap sai toi da

// cau hinh xu ly noi bo
constexpr int SO_VONG_LAP_DMS_MAX = 1000;  // gioi han vong lap de tranh treo khi duyet DSLK loi
constexpr int MAX_LAN_THU_SINH_MA = 100;   // so lan thu sinh ma ngau nhien (neu can)

// cau hinh doc/Ghi File
constexpr int DUNG_LUONG_CACHE_MAC_DINH = 256; // kich thuoc buffer doc file
constexpr int SO_TRUONG_DAU_SACH = 6;          // so truong trong file DauSach.txt (ISBN|Ten|Trang|TG|Nam|TheLoai)
constexpr int SO_TRUONG_DMS = 3;               // so truong trong file DanhMucSach.txt (MaSach|TrangThai|ViTri)

// cau hinh Muon Tra Sach
constexpr int SO_NGAY_MUON_TOI_DA = 7;         // so ngay muon toi da cho phep

#endif // CONSTANTS_H