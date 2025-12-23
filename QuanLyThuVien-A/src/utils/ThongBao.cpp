/**
 * @file ThongBao.cpp
 * @path src/utils/ThongBao.cpp
 * @description Tiện ích hiển thị các hộp thoại thông báo (Message Boxes).
 * Đồng nhất cách hiển thị thông báo thành công, lỗi, cảnh báo và câu hỏi xác nhận trên toàn bộ ứng dụng.
 * @project Library Management System (QuanLyThuVien)
 */
#include "utils/ThongBao.h"


void thongBao(std::ostream& out, const std::string& msg, LoaiThongBao loai) {
    const char* tienTo[] = {"Loi: ", "Thong tin: ", "Canh bao: "};
    out << tienTo[loai] << msg << "\n";
}

std::string thongBaoLoi(int soDong, const std::string& chiTiet, bool canhBao) {
    std::string prefix = canhBao ? std::string("Canh bao: ") : std::string("Loi: ");
    return prefix + "Dong " + std::to_string(soDong) + ": " + chiTiet;
}

std::string thongBaoLoiDMS(int soDong, const std::string& chiTiet, bool canhBao) {
    std::string prefix = canhBao ? std::string("Canh bao: ") : std::string("Loi: ");
    return prefix + "Dong DMS " + std::to_string(soDong) + ": " + chiTiet;
}

void thongBaoSoLanThuConLai(int soLanDaThu, std::ostream& out, int maxLanThu) {
    if (soLanDaThu < maxLanThu) {
        out << "Ban con " << (maxLanThu - soLanDaThu) << "/" << maxLanThu << " lan thu." << std::endl;
    }
}