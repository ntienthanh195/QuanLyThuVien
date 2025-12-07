#include "utils/DBBatcher.h"
#include <vector>
#include <string>
#include <mutex>

// Refer to the global reader tree defined in main.cpp
extern PTRDG rootDocGia;

// Ghi chú (tiếng Việt):
// - Mục tiêu của DBBatcher là cung cấp snapshot/batch API cho UI để tránh gọi các hàm
//   truy vấn/duyệt danh sách trực tiếp trên luồng giao diện.
// - Implementation hiện tại là wrapper mỏng quanh các hàm backend hiện có
//   (LayDanhSachDauSach, LayDanhSachBanSaoSapXep, TimDocGia*, LayDanhSachTimKiem, ...)
//   và chuyển dữ liệu sang STL containers (std::vector) để dễ dùng trong models.

namespace DBBatcher {

std::vector<PTRDS> snapshotDauSach() {
    std::vector<PTRDS> out;
    // Khóa mutex toàn cục để tránh race khi backend có thể swap dsDauSach trong background
    std::lock_guard<std::mutex> lock(g_ds_mutex);
    PTRDS* arr = LayDanhSachDauSach();
    int n = LaySoLuongDauSach();
    out.reserve(n > 0 ? n : 0);
    for (int i = 0; i < n; ++i) {
        if (arr[i]) out.push_back(arr[i]);
    }
    return out;
}

std::vector<PTRDMS> layBanSaoTheoISBN(const std::string &isbn) {
    std::vector<PTRDMS> out;
    PTRDMS buf[MAX_BAN_SAO];
    int cnt = LayDanhSachBanSaoSapXep(isbn, buf, MAX_BAN_SAO);
    if (cnt <= 0) return out;
    out.reserve(cnt);
    for (int i = 0; i < cnt; ++i) {
        if (buf[i]) out.push_back(buf[i]);
    }
    return out;
}

std::vector<KetQuaTimKiem> timKiemDauSach(const std::string &tuKhoa, int maxResults) {
    std::vector<KetQuaTimKiem> out;
    KetQuaTimKiem buf[1024];
    int cnt = 0;
    // Reuse existing backend search implementation
    LayDanhSachTimKiem(tuKhoa, buf, cnt, maxResults);
    if (cnt <= 0) return out;
    out.reserve(cnt);
    for (int i = 0; i < cnt; ++i) {
        out.push_back(buf[i]);
    }
    return out;
}

std::vector<PTRDG> snapshotDocGia() {
    std::vector<PTRDG> out;
    PTRDG tmp[2000]; // reasonable upper bound
    int cnt = 0;
    DuyetCayRaMang(::rootDocGia, tmp, cnt);
    if (cnt <= 0) return out;
    out.reserve(cnt);
    for (int i = 0; i < cnt; ++i) {
        if (tmp[i]) out.push_back(tmp[i]);
    }
    return out;
}

std::vector<PTRDG> timDocGiaTheoHoTen(const std::string &hoTen, int maxResults) {
    std::vector<PTRDG> out;
    PTRDG buf[1024];
    int cnt = TimDocGiaTheoHoTen(::rootDocGia, hoTen, buf, maxResults);
    if (cnt <= 0) return out;
    out.reserve(cnt);
    for (int i = 0; i < cnt; ++i) {
        if (buf[i]) out.push_back(buf[i]);
    }
    return out;
}

std::vector<PTRDG> timDocGiaTheoTen(const std::string &ten, int maxResults) {
    std::vector<PTRDG> out;
    PTRDG buf[1024];
    int cnt = TimDocGiaTheoTen(::rootDocGia, ten, buf, maxResults);
    if (cnt <= 0) return out;
    out.reserve(cnt);
    for (int i = 0; i < cnt; ++i) {
        if (buf[i]) out.push_back(buf[i]);
    }
    return out;
}

std::vector<PTRDG> timDocGiaTheoHo(const std::string &ho, int maxResults) {
    std::vector<PTRDG> out;
    PTRDG buf[1024];
    int cnt = TimDocGiaTheoHo(::rootDocGia, ho, buf, maxResults);
    if (cnt <= 0) return out;
    out.reserve(cnt);
    for (int i = 0; i < cnt; ++i) {
        if (buf[i]) out.push_back(buf[i]);
    }
    return out;
}

std::vector<PTRDG> timDocGiaTheoTrangThai(int status, int maxResults) {
    std::vector<PTRDG> out;
    PTRDG buf[1024];
    int cnt = TimDocGiaTheoTrangThai(::rootDocGia, status, buf, maxResults);
    if (cnt <= 0) return out;
    out.reserve(cnt);
    for (int i = 0; i < cnt; ++i) {
        if (buf[i]) out.push_back(buf[i]);
    }
    return out;
}

} // namespace DBBatcher
