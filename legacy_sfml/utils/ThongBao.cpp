#include "ThongBao.h"

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