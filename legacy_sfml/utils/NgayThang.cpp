#include "NgayThang.h"
#include "ThongBao.h"
#include "Constants.h"
#include "XuLyChuoi.h" 
#include <ctime>
#include <iomanip>
#include <sstream>

// Lay nam hien tai tu he thong
int LayNamHienTai(){
    time_t now = time(nullptr);                             // lay thoi gian he thong
    struct tm *t = localtime(&now);                         // chuyen sang cau truc tm
    return t->tm_year + 1900;                               // tm_year tinh tu 1900
}

// Lay ngay hien tai theo dang "dd/mm/yyyy"
std::string layNgayHienTai(std::ostream &out){
    (void)out;                                              // khong dung out
    time_t now = time(nullptr);                             // lay thoi gian he thong
    struct tm *t = localtime(&now);                         // chuyen sang cau truc tm
    char buf[11];                                           // buffer cho chuoi ngay
    snprintf(buf, sizeof(buf), "%02d/%02d/%04d", t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);
    return std::string(buf);                                // tra ve chuoi ngay
}

// Chuyen chuoi ngay "dd/mm/yyyy" sang time_t
time_t ChuyenNgaySangTimeT(const std::string &ngay, std::ostream &out) {
    int day, month, year;
    
    if (!phanTichNgay(ngay, day, month, year)) {           // phan tich chuoi ngay
        thongBao(out, "Ngay khong dung dinh dang: " + ngay, LOI);
        return -1;                                          // tra ve -1 neu loi
    }
    
    std::tm t{};                                            // khoi tao cau truc tm bang 0
    t.tm_mday = day;                                        // ngay trong thang
    t.tm_mon = month - 1;                                   // thang (0-11)
    t.tm_year = year - 1900;                                // nam tinh tu 1900
    t.tm_hour = 0; t.tm_min = 0; t.tm_sec = 0;             // gio phut giay = 0
    t.tm_isdst = -1;                                        // tu dong xac dinh DST

    time_t result = mktime(&t);                             // chuyen sang time_t
    if (result == -1) {                                     // kiem tra loi
        thongBao(out, "Khong the chuyen ngay sang time_t: " + ngay, LOI);
    }
    return result;                                          // tra ve time_t
}

// Tinh so ngay qua han muon sach
int tinhSoNgayQuaHan(std::string ngayMuon, std::ostream& out){
    time_t now = time(nullptr);                             // lay thoi gian hien tai
    time_t tMuon = ChuyenNgaySangTimeT(ngayMuon, out);     // chuyen ngay muon sang time_t
    if (tMuon == -1) return -1;                            // loi chuyen doi

    double diff_seconds = difftime(now, tMuon);            // tinh chenh lech giay
    double diff_days = diff_seconds / (60 * 60 * 24);      // chuyen sang ngay
    int soNgayQuaHan = static_cast<int>(diff_days) - SO_NGAY_MUON_TOI_DA;
    return soNgayQuaHan > 0 ? soNgayQuaHan : 0;            // tra ve 0 neu chua qua han
}

// Phan tich chuoi "dd/mm/yyyy" thanh cac so nguyen
bool phanTichNgay(const std::string &ngay, int &d, int &m, int &y){
    if (ngay.length() != 10 || ngay[2] != '/' || ngay[5] != '/')  // kiem tra dinh dang co ban
        return false;
    
    std::string dayStr = ngay.substr(0, 2);                 // tach ngay
    std::string monthStr = ngay.substr(3, 2);               // tach thang
    std::string yearStr = ngay.substr(6, 4);                // tach nam
    
    if (!ChuyenChuoiThanhSoNguyen(dayStr, d, true) ||      // chuyen ngay sang so
        !ChuyenChuoiThanhSoNguyen(monthStr, m, true) ||     // chuyen thang sang so
        !ChuyenChuoiThanhSoNguyen(yearStr, y, false)) {     // chuyen nam sang so
        return false;                                        // loi chuyen doi
    }
    
    if (d < 1 || d > 31 || m < 1 || m > 12 || y < NAM_XUAT_BAN_MIN)  // kiem tra gia tri hop le
        return false;
    
    return true;                                             // hop le
}
