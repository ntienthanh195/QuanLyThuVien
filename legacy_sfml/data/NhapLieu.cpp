#include "NhapLieu.h"
#include "KiemTraDuLieu.h"
#include "XuLyChuoi.h"
#include "ThongBao.h"
#include "Constants.h"

static inline void kiemTraEOF(std::istream& in) {
    if (in.eof() || in.fail()) {
        throw std::invalid_argument("Loi: Ket thuc du lieu dau vao!");
    }
}
[[noreturn]] static inline void nemLoiQuaSoLanNhap() {
    throw std::invalid_argument("Loi: Qua so lan nhap sai cho phep!");
}


std::string NhapTen(const char* nhan, size_t minLength, size_t maxLength, std::istream& in, std::ostream& out) {
    std::string goc;
    int soLanThu = 0;
    while (soLanThu < SO_LAN_THU_TOI_DA) {
        out << "Nhap " << (nhan ? nhan : "ten") << " (" << minLength << "-" << maxLength << " ky tu): ";
        std::getline(in, goc);
        kiemTraEOF(in);
        std::string gocCat = CatKhoangTrang(goc);
        if (gocCat.empty()) {
            thongBao(out, (nhan ? nhan : "Truong") + std::string(" khong duoc rong!"), LOI);
            ++soLanThu;
            thongBaoSoLanThuConLai(soLanThu, out, SO_LAN_THU_TOI_DA);
            continue;
        }
        std::string ketQua;
        
        std::string loiTen = ChuanHoaTenUnicode(gocCat, minLength, maxLength, ketQua);
        if (!loiTen.empty()) { // Kiem tra chuoi loi
            thongBao(out, loiTen, LOI); // In loi tra ve
            ++soLanThu;
            thongBaoSoLanThuConLai(soLanThu, out, SO_LAN_THU_TOI_DA);
            continue;
        }
        return ketQua;
    }
    nemLoiQuaSoLanNhap();
}

int NhapSoNguyen(const char* nhan, int minVal, int maxVal, std::istream& in, std::ostream& out) {
    int soLanThu = 0;
    while (soLanThu < SO_LAN_THU_TOI_DA) {
        out << "Nhap " << (nhan ? nhan : "so") << " (" << minVal << "-" << maxVal << "): ";
        std::string input;
        std::getline(in, input);
        kiemTraEOF(in);
        int ketQua;
        if (!ChuyenChuoiThanhSoNguyen(CatKhoangTrang(input), ketQua, true) || ketQua < minVal || ketQua > maxVal) {
            thongBao(out, (nhan ? nhan : "So") + std::string(" khong hop le!"), LOI);
            ++soLanThu;
            thongBaoSoLanThuConLai(soLanThu, out, SO_LAN_THU_TOI_DA);
            continue;
        }
        return ketQua;
    }
    nemLoiQuaSoLanNhap();
}

std::string NhapPhai(std::istream& in, std::ostream& out) {
    std::string phaiNhapVao;
    int soLanThu = 0;
    while (soLanThu < SO_LAN_THU_TOI_DA) {
        out << "Nhap phai (Nam/Nu): ";
        std::getline(in, phaiNhapVao);
        kiemTraEOF(in);
        std::string phaiCat = CatKhoangTrang(phaiNhapVao);
        if (phaiCat.empty()) {
            thongBao(out, "Phai khong duoc rong!", LOI);
            ++soLanThu;
            thongBaoSoLanThuConLai(soLanThu, out, SO_LAN_THU_TOI_DA);
            continue;
        }
        std::string phaiChuanHoa = ChuanHoaGioiTinh(phaiNhapVao);
        if (!phaiChuanHoa.empty()) {
            return phaiChuanHoa;
        }
        thongBao(out, "Chi Nam hoac Nu!", LOI);
        ++soLanThu;
        thongBaoSoLanThuConLai(soLanThu, out, SO_LAN_THU_TOI_DA);
    }
    nemLoiQuaSoLanNhap();
}


std::string NhapISBNThuCong(std::istream& in, std::ostream& out) {
    int soLanThu = 0;
    while (soLanThu < SO_LAN_THU_TOI_DA) {
        out << "Nhap ISBN (10 hoac 13 ky tu): ";
        std::string isbnRaw;
        std::getline(in, isbnRaw);
        kiemTraEOF(in);
        std::string isbnChuan;

        std::string loiChuanHoa = ChuanHoaISBNFile(isbnRaw, isbnChuan);
        if (!loiChuanHoa.empty()) {
            thongBao(out, loiChuanHoa, LOI);
            ++soLanThu;
            thongBaoSoLanThuConLai(soLanThu, out, SO_LAN_THU_TOI_DA);
            continue;
        }
  
        std::string loiTrung = KiemTraTrungISBN(isbnChuan);
        if (!loiTrung.empty()) {
            thongBao(out, loiTrung, LOI);
            ++soLanThu;
            thongBaoSoLanThuConLai(soLanThu, out, SO_LAN_THU_TOI_DA);
            continue;
        }
        return isbnChuan;
    }
    nemLoiQuaSoLanNhap();
}