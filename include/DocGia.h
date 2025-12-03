#ifndef DOCGIA_H
#define DOCGIA_H
#include <string>
#include <fstream>
#include "DauSach.h"
using namespace std;

//danh sach muon tra
struct MuonTra{
    PTRDMS banSaoSach;// thay the string maSach thanh con tro chi toi DanhMucSach* va khai bao trang thai 
    string NgayMuon;
    string NgayTra;
    int TrangThai;//0: dang muon, 1: da tra, 2: lam mat sach
};
struct NodeMT{
    MuonTra data;
    NodeMT* next;
};
typedef NodeMT* MUONTRA;

//PTRDGdoc gia
struct TheDocGia{
    int MaThe;
    string Ho;
    string Ten;
    bool Phai;//0 nam 1 nu
    int TrangThai = 1;//0 = khoa, 1=hoat dong
    
    // CẢI TIẾN: Thêm trường đếm sách để kiểm tra giới hạn O(1)
    int soSachDangMuon = 0; 
    
    MUONTRA dsmt;//danh sach muon tra linked list
};

//binary search tree (AVL)
struct NodeDG{
    TheDocGia data;
    int bf;
    int height;
    NodeDG *left, *right;
};
typedef NodeDG* PTRDG;

struct ThongTinQuaHan{
    PTRDG docGia;
    int soNgayQuaHanMax;
};
//struct tu them
struct ThongTinSachDangMuon_DTO {
    string maSach;
    string tenSach;     // <== Thêm trường này
    string ngayMuon;
    int soNgayGiu;    // <== Thêm trường này
};

struct TopSachDTO {
    PTRDS dauSach;//dung con tro thay vi tao mot field da co san
};

struct DocGiaQuaHanDTO {
    PTRDG docGia;
    int soNgayQuaHanMax;
};
// Prototype AVL
PTRDG RotateLeft(PTRDG root);
PTRDG RotateRight(PTRDG root);
void InsertDocGia(PTRDG& root, PTRDG node);

// Prototype Chức năng chính (a)
PTRDG taoDocGia(std::string ho, std::string ten, bool phai, int trangthai, PTRDG root);
PTRDG taoDocGia(std::string ho, std::string ten, bool phai, int trangthai, int mathe);
void xoaDocGia(PTRDG& root, int mathe);
PTRDG timDocGia(PTRDG root, int mathe);
void inDocGiaInOrder(PTRDG root); // In theo Mã thẻ tăng dần
void display(PTRDG root); // Hàm in 1 độc giả

// CẢI TIẾN: Thay đổi prototype để tránh rò rỉ bộ nhớ (a)
void hieuChinhDocGia(PTRDG root, const TheDocGia& new_data);

// Prototype cho chức năng sắp xếp (b)
void QuickSortDocGia(PTRDG arr[], int low, int high);
void DuyetCayRaMang(PTRDG root, PTRDG arr[], int &count);
void InDocGiaTheoTenHo(PTRDG root); // In ra danh sách sắp xếp theo Tên+Họ

// Prototype Giải phóng bộ nhớ
void GiaiPhongCay(PTRDG& root);
void giaiPhongDsmt(MUONTRA& dsmt);

// Prototype Quản lý Mượn Trả (f) (g)
std::string MuonSach(PTRDG docGia, const std::string& isbn, PTRDS dsDauSach[], int soLuongDauSach);
std::string TraSach(PTRDG docGia, const std::string& maSach, PTRDS dsDauSach[], int soLuongDauSach);
std::string BaoMatSach(PTRDG docGia, const std::string& maSach, PTRDS dsDauSach[], int soLuongDauSach);
void themMuonTra(PTRDG docgia, PTRDMS banSaoSach);//khi muon sach
void themMuonTra(PTRDG docgia, const MuonTra &mt);//khi load file
PTRDMS TimBanSaoCoTheMuon(PTRDS dauSach);
PTRDMS TimBanSaoTheoMa(PTRDS dauSach, const std::string& maSach);
int DemBanSaoCoTheMuon(PTRDS dauSach);
// Prototype (h)
int LayDSSachDangMuon(PTRDG docGia, ThongTinSachDangMuon_DTO ketQua[], int maxKetQua, PTRDS dsDauSach[], int soLuongDauSach);

// Prototype Danh sách độc giả mượn quá hạn (i)
int TinhSoNgayQuaHanLonNhat(PTRDG docgia);
int LayDSDocGiaQuaHan(PTRDG root, DocGiaQuaHanDTO arr[], int maxKetQua);
int LayTopSach(PTRDS dsDauSach[], int soLuongDS, TopSachDTO arr[]);

// Prototype File I/O
string PTRDMS_to_String(PTRDMS p);
PTRDMS MaSach_to_PTRDMS(const string& maSach, PTRDS dsDauSach[], int soLuongDauSach);
void LuuGiaoDich(MUONTRA dsmt, std::ofstream& file);
void NapGiaoDich(PTRDG docGia, std::ifstream& file, PTRDS dsDauSach[], int n);

void saveDocGia(PTRDG root);
PTRDG loadDocGia(PTRDS dsDauSach[], int soLuongDS);

// Prototype helper ham loc
bool KiemTraBatDauBang(const std::string& fullString, const std::string& prefix);


#endif