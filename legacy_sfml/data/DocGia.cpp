#include "DocGia.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <string>
#include "QuanLySach.h"
#include "Constants.h"
#include "NgayThang.h"
// Cần thêm thư viện XuLyChuoi.h cho hàm BoDauVaThuong()
#include "XuLyChuoi.h" 
// Cần thêm thư viện QuanLySach.h nếu sử dụng TimDanhMucSachTuMaSach
// #include "QuanLySach.h" 

// helper thay the max/abs tu <algorithm>
static inline int my_max(int a, int b) { return (a > b) ? a : b; }
static inline int my_abs(int x) { return (x < 0) ? -x : x; }

// forward-declare static helpers
static PTRDG InsertDocGiaRec(PTRDG pavtree, PTRDG node);
static PTRDG xoaDocGiaRec(PTRDG root, int mathe);
// Lay lai prototype vi static helper dung
PTRDG timDocGia(PTRDG root, int mathe); 

// hang so cho sinh ma
static const int MA_THE_MIN = 10000;
static const int MA_THE_MAX = 99999;
static const int NUM_RANGES = 9; // chia range de phan tan

// bien static theo doi RNG
static bool daKhoiTao = false;
static int khoangHienTai = 0;
static int countTrongKhoang = 0;

int static sinhMaTheNgauNhien(PTRDG root) {
    if (!daKhoiTao) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        daKhoiTao = true;
    }

    const int sizeKhoang = (MA_THE_MAX - MA_THE_MIN + 1) / NUM_RANGES;
    for (int i = 0; i < MAX_LAN_THU_SINH_MA; ++i) {
        int rangeStart = MA_THE_MIN + khoangHienTai * sizeKhoang;
        int rangeEnd = rangeStart + sizeKhoang - 1;
        int ma = rangeStart + (std::rand() % (rangeEnd - rangeStart + 1));
        if (timDocGia(root, ma) == nullptr) {
            ++countTrongKhoang;
            if (countTrongKhoang >= sizeKhoang / NUM_RANGES) {
                countTrongKhoang = 0;
                khoangHienTai = (khoangHienTai + 1) % NUM_RANGES;
            }
            return ma;
        }
    }

    // fallback: linear search
    int ma = MA_THE_MIN;
    while (ma <= MA_THE_MAX && timDocGia(root, ma) != nullptr) ++ma;
    return (ma <= MA_THE_MAX) ? ma : -1;
}
//AVL
// get/update height and balance helpers
static inline int getHeight(PTRDG n) {
    return (n == nullptr) ? 0 : n->height;
}
static inline void updateHeight(PTRDG n) {
    if (n != nullptr) n->height = 1 + my_max(getHeight(n->left), getHeight(n->right));
}
static inline int getBalance(PTRDG n) {
    return (n == nullptr) ? 0 : getHeight(n->left) - getHeight(n->right);
}

// rotations update heights
PTRDG RotateLeft(PTRDG p) {
    PTRDG q = p->right;
    PTRDG B = q->left;

    q->left = p;
    p->right = B;

    updateHeight(p);
    updateHeight(q);
    return q;
}

PTRDG RotateRight(PTRDG p) {
    PTRDG q = p->left;
    PTRDG B = q->right;

    q->right = p;
    p->left = B;

    updateHeight(p);
    updateHeight(q);
    return q;
}

// Them AVL - dung height de tinh balance O(1)
static PTRDG InsertDocGiaRec(PTRDG pavltree, PTRDG node) {
    if (pavltree == nullptr) {
        return node;
    }

    if (node->data.MaThe < pavltree->data.MaThe)
        pavltree->left = InsertDocGiaRec(pavltree->left, node);
    else if (node->data.MaThe > pavltree->data.MaThe)
        pavltree->right = InsertDocGiaRec(pavltree->right, node);
    else {
        // duplicate id -> do nothing
        return pavltree;
    }

    // cap nhat height va rebalance
    updateHeight(pavltree);
    int balance = getBalance(pavltree);

    // LL (Left Left Case)
    if (balance > 1 && getBalance(pavltree->left) >= 0)
        return RotateRight(pavltree);
    // RR (Right Right Case)
    if (balance < -1 && getBalance(pavltree->right) <= 0)
        return RotateLeft(pavltree);
    // LR (Left Right Case)
    if (balance > 1 && getBalance(pavltree->left) < 0) {
        pavltree->left = RotateLeft(pavltree->left);
        return RotateRight(pavltree);
    }
    // RL (Right Left Case)
    if (balance < -1 && getBalance(pavltree->right) > 0) {
        pavltree->right = RotateRight(pavltree->right);
        return RotateLeft(pavltree);
    }

    return pavltree;
}

void InsertDocGia(PTRDG &root, PTRDG node) {
    // ensure new node has height = 1
    if (node != nullptr) node->height = 1;
    root = InsertDocGiaRec(root, node);
}

// quan ly cay doc gia
// tao PTRDG docgia
PTRDG taoDocGia(std::string ho, std::string ten, bool phai, int trangthai, PTRDG root) {
    PTRDG dg = new NodeDG;
    dg->data.MaThe = sinhMaTheNgauNhien(root);
    dg->data.Ho = ho;
    dg->data.Ten = ten;
    dg->data.Phai = phai;
    dg->data.TrangThai = trangthai;
    dg->data.dsmt = nullptr;
    dg->data.soSachDangMuon = 0; 
    dg->left = dg->right = nullptr;
    dg->height = 1;
    return dg;
}
PTRDG taoDocGia(std::string ho, std::string ten, bool phai, int trangthai, int mathe) {
    PTRDG dg = new NodeDG;
    dg->data.MaThe = mathe;
    dg->data.Ho = ho;
    dg->data.Ten = ten;
    dg->data.Phai = phai;
    dg->data.TrangThai = trangthai;
    dg->data.dsmt = nullptr;
    dg->data.soSachDangMuon = 0; 
    dg->left = dg->right = nullptr;
    dg->height = 1;
    return dg;
}

// giai phong dsmt
void giaiPhongDsmt(MUONTRA& dsmt) {
    while (dsmt != nullptr) {
        MUONTRA tmp = dsmt;
        dsmt = dsmt->next;
        delete tmp;
    }
}

// them ham giai phong toan bo cay
void GiaiPhongCay(PTRDG &root) {
    if (root == nullptr) return;
    GiaiPhongCay(root->left);
    GiaiPhongCay(root->right);
    giaiPhongDsmt(root->data.dsmt);
    delete root;
    root = nullptr;
}

// xoa node - cap nhat height va rebalance
static PTRDG xoaDocGiaRec(PTRDG root, int mathe) {
    if (root == nullptr) return root;

    if (mathe < root->data.MaThe) {
        root->left = xoaDocGiaRec(root->left, mathe);
    } else if (mathe > root->data.MaThe) {
        root->right = xoaDocGiaRec(root->right, mathe);
    } else {
        // found node
        if (root->left == nullptr || root->right == nullptr) {
            // Trường hợp 0 hoặc 1 con
            giaiPhongDsmt(root->data.dsmt);
            PTRDG temp = root->left ? root->left : root->right;
            delete root;
            return temp;
        } else {
            // Trường hợp 2 con: lấy inorder successor (succ)
            PTRDG succ = root->right;
            while (succ->left != nullptr) succ = succ->left;

            // CẢI TIẾN: Sửa lỗi bộ nhớ và cập nhật trường mới
            giaiPhongDsmt(root->data.dsmt); 
            
            // Sao chép dữ liệu của succ sang root
            root->data.MaThe = succ->data.MaThe;
            root->data.Ho = succ->data.Ho;
            root->data.Ten = succ->data.Ten;
            root->data.Phai = succ->data.Phai;
            root->data.TrangThai = succ->data.TrangThai;
            root->data.soSachDangMuon = succ->data.soSachDangMuon; // Cập nhật trường mới
            
            // Chuyển quyền sở hữu con trỏ dsmt từ succ sang root
            root->data.dsmt = succ->data.dsmt;
            succ->data.dsmt = nullptr;

            // Đệ quy xóa nút successor
            root->right = xoaDocGiaRec(root->right, root->data.MaThe);
        }
    }

    // cap nhat height va rebalance 
    if (root == nullptr) return root;
    updateHeight(root);
    int balance = getBalance(root);
    int leftBalance = getBalance(root->left);
    int rightBalance = getBalance(root->right);

    // LL
    if (balance > 1 && leftBalance >= 0)
        return RotateRight(root);
    // LR
    if (balance > 1 && leftBalance < 0) {
        root->left = RotateLeft(root->left);
        return RotateRight(root);
    }
    // RR
    if (balance < -1 && rightBalance <= 0)
        return RotateLeft(root);
    // RL
    if (balance < -1 && rightBalance > 0) {
        root->right = RotateRight(root->right);
        return RotateLeft(root);
    }

    return root;
}
void xoaDocGia(PTRDG& root, int mathe) {
    root = xoaDocGiaRec(root, mathe);
}

// tim doc gia
PTRDG timDocGia(PTRDG root, int mathe) {
    while (root != nullptr && mathe != root->data.MaThe) {
        if (mathe < root->data.MaThe) root = root->left;
        else root = root->right;
    }
    return root;
}

// hieu chinh doc gia
void hieuChinhDocGia(PTRDG root, const TheDocGia& new_data) {
    PTRDG p = timDocGia(root, new_data.MaThe);
    if (p == nullptr) {
        std::cerr << "Khong tim thay doc gia co ma the " << new_data.MaThe << std::endl;
        return;
    }

    // Chỉ cập nhật các trường được phép thay đổi
    if (p->data.Ho != new_data.Ho) p->data.Ho = new_data.Ho;
    if (p->data.Ten != new_data.Ten) p->data.Ten = new_data.Ten;
    if (p->data.Phai != new_data.Phai) p->data.Phai = new_data.Phai;
    if (p->data.TrangThai != new_data.TrangThai) p->data.TrangThai = new_data.TrangThai;
    // Không cập nhật MaThe, dsmt, soSachDangMuon
}

// hien thi
void display(PTRDG root) {
    if (root == nullptr) {
        std::cerr << "Khong the hien thi: con tro doc gia null!" << std::endl;
        return;
    }

    std::cout << "MaThe: " << root->data.MaThe
         << " | HoTen: " << root->data.Ho << " " << root->data.Ten
         << " | Phai: " << (root->data.Phai ? "Nu" : "Nam")
         << " | TrangThai: " << (root->data.TrangThai ? "Hoat dong" : "Khoa")
         << " | Sach Dang Muon: " << root->data.soSachDangMuon
         << " | AVL Height: " << root->height
         << std::endl;
}

void inDocGiaInOrder(PTRDG root) {
    if (root != nullptr) {
        inDocGiaInOrder(root->left);
        display(root);
        inDocGiaInOrder(root->right);
    }
}

//B.In danh sach doc gia
// Helper: So sánh hai chuỗi theo Tên + Họ (sử dụng BoDauVaThuong)
static int SoSanhTenHo(PTRDG dg1, PTRDG dg2) {
    // 1. So sánh theo Tên (không dấu, không phân biệt hoa/thường)
    std::string ten1 = ChuanHoaChuoiTimKiem(dg1->data.Ten);
    std::string ten2 = ChuanHoaChuoiTimKiem(dg2->data.Ten);
    if (ten1 != ten2) {
        return (ten1 < ten2) ? -1 : 1;
    }
    
    // 2. Nếu Tên bằng nhau, so sánh theo Họ
    std::string ho1 = ChuanHoaChuoiTimKiem(dg1->data.Ho);
    std::string ho2 = ChuanHoaChuoiTimKiem(dg2->data.Ho);
    if (ho1 != ho2) {
        return (ho1 < ho2) ? -1 : 1;
    }
    
    // 3. Nếu Tên và Họ giống nhau, so sánh theo Mã thẻ để đảm bảo thứ tự ổn định
    if (dg1->data.MaThe < dg2->data.MaThe) return -1;
    if (dg1->data.MaThe > dg2->data.MaThe) return 1;
    
    return 0; 
}

static void SwapDG(PTRDG& a, PTRDG& b) {
    PTRDG temp = a; a = b; b = temp;
}

static int PartitionDG(PTRDG arr[], int low, int high) {
    PTRDG pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        if (SoSanhTenHo(arr[j], pivot) < 0) {
            i++;
            SwapDG(arr[i], arr[j]);
        }
    }
    SwapDG(arr[i + 1], arr[high]);
    return (i + 1);
}

void QuickSortDocGia(PTRDG arr[], int low, int high) {
    if (low < high) {
        int pi = PartitionDG(arr, low, high);
        QuickSortDocGia(arr, low, pi - 1);
        QuickSortDocGia(arr, pi + 1, high);
    }
}

// Duyệt cây InOrder ra mảng
void DuyetCayRaMang(PTRDG root, PTRDG arr[], int &count) {
    if (root != nullptr) {
        DuyetCayRaMang(root->left, arr, count);
        // Giả định MAX_DAUSACH (1000) là kích thước mảng an toàn
        if (count < MAX_DAUSACH) { 
            arr[count++] = root;
        }
        DuyetCayRaMang(root->right, arr, count);
    }
}
// --- HẾT LOGIC SẮP XẾP ---

//F.Muon Sach
// quan ly muon tra
// tim ban sao co the muon cua 1 dau sach (O(M) - M: so ban sao)
PTRDMS TimBanSaoCoTheMuon(PTRDS dauSach) {
    if (dauSach == nullptr) return nullptr;
    
    PTRDMS p = dauSach->dms;
    int dem = 0;
    
    while (p != nullptr && dem < SO_VONG_LAP_DMS_MAX) {
        // chi lay ban sao co trang thai CHO_MUON_DUOC (0)
        if (p->trangThai == CHO_MUON_DUOC) {
            return p;  // tra ve ban sao dau tien co the muon
        }
        p = p->next;
        dem++;
    }
    
    if (dem >= SO_VONG_LAP_DMS_MAX) {
        thongBao(std::cout, "Vong lap vo han DMS!", LOI);
    }
    
    return nullptr;  // khong co ban sao nao san sang
}

// tim ban sao theo ma sach trong 1 dau sach (O(M))
PTRDMS TimBanSaoTheoMa(PTRDS dauSach, const std::string& maSach) {
    if (dauSach == nullptr) return nullptr;
    
    PTRDMS p = dauSach->dms;
    int dem = 0;
    
    while (p != nullptr && dem < SO_VONG_LAP_DMS_MAX) {
        if (p->maSach == maSach) {
            return p;  // tim thay
        }
        p = p->next;
        dem++;
    }
    
    if (dem >= SO_VONG_LAP_DMS_MAX) {
        thongBao(std::cout, "Vong lap vo han DMS!", LOI);
    }
    
    return nullptr;  // khong tim thay
}

// dem so ban sao co the muon (O(M))
int DemBanSaoCoTheMuon(PTRDS dauSach) {
    if (dauSach == nullptr) return 0;
    
    int dem = 0;
    PTRDMS p = dauSach->dms;
    int vong = 0;
    
    while (p != nullptr && vong < SO_VONG_LAP_DMS_MAX) {
        if (p->trangThai == CHO_MUON_DUOC) {
            dem++;
        }
        p = p->next;
        vong++;
    }
    
    if (vong >= SO_VONG_LAP_DMS_MAX) {
        thongBao(std::cout, "Vong lap vo han DMS!", LOI);
    }
    
    return dem;
}

// ham backend: muon sach (O(log N) + O(M))
std::string MuonSach(PTRDG docGia, const std::string& isbn, PTRDS dsDauSach[], int soLuongDauSach) {
    if (docGia == nullptr) return "Loi: Doc gia khong hop le!";
    // Duyệt qua danh sách sách đang mượn để kiểm tra xem có cuốn nào quá hạn không
    MUONTRA p = docGia->data.dsmt;
    std::stringstream dummyStream; // Dùng stream ảo để hứng log (không in ra màn hình)
    
    while (p != nullptr) {
        // Chỉ kiểm tra sách đang mượn (TrangThai == 0)
        if (p->data.TrangThai == 0) {
            // Gọi hàm tinhSoNgayQuaHan từ NgayThang.cpp
            int soNgayQua = tinhSoNgayQuaHan(p->data.NgayMuon, dummyStream);
            
            // Nếu > 0 nghĩa là đã quá hạn (số ngày giữ > SO_NGAY_MUON_TOI_DA)
            if (soNgayQua > 0) {
                return "Loi: Doc gia co sach qua han " + std::to_string(soNgayQua) + " ngay! Vui long tra sach truoc.";
            }
        }
        p = p->next;
    }
    // ----------------------------------------

    // B1: Kiem tra gioi han muon (3 cuon)
    if (docGia->data.soSachDangMuon >= 3) {
        return "Loi: Doc gia da muon toi da 3 cuon!";
    }
    
    // B2: Tim dau sach
    PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
    if (dauSach == nullptr) {
        return "Loi: Khong tim thay dau sach!";
    }
    
    // B3: Tim ban sao co the muon
    PTRDMS banSao =  TimBanSaoCoTheMuon(dauSach);
    if (banSao == nullptr) {
        return "Loi: Tat ca ban sao dang duoc muon!";
    }
    
    // B4: Cap nhat trang thai sach
    banSao->trangThai = DANG_MUON;
    
    // B5: Them vao lich su muon tra
    MuonTra mt;
    mt.banSaoSach = banSao;
    mt.NgayMuon = layNgayHienTai(dummyStream); // Dùng dummyStream để tránh in log rác
    mt.NgayTra = "";
    mt.TrangThai = 0;  // dang muon
    
    themMuonTra(docGia, mt);
    
    duLieuDaThayDoi = true;

    dauSach->soLuotMuon++;
    return "";  // thanh cong
}

// ham backend: tra sach (O(log N) + O(L))
std::string TraSach(PTRDG docGia, const std::string& maSach, PTRDS dsDauSach[], int soLuongDauSach) {
    if (docGia == nullptr) return "Loi: Doc gia khong hop le!";
    
    // B1: Tim giao dich muon trong lich su
    MUONTRA curr = docGia->data.dsmt;
    MUONTRA found = nullptr;
    
    while (curr != nullptr) {
        if (curr->data.banSaoSach != nullptr && // 1. Kiem tra con tro
            curr->data.banSaoSach->maSach == maSach && // 2. So sanh maSach
            curr->data.TrangThai == 0) {  // dang muon
            found = curr;
            break;
        }
        curr = curr->next;
    }
    
    if (found == nullptr) {
        return "Loi: Doc gia khong muon sach nay!";
    }
    
    // B2: Cap nhat trang thai sach (thong qua con tro)
    if (found->data.banSaoSach != nullptr) {
        found->data.banSaoSach->trangThai = CHO_MUON_DUOC;
    } else {
        // Fallback: tim theo ma sach neu con tro bi null
        std::string isbn = TachISBNTuMaSach(maSach);
        PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
        if (dauSach != nullptr) {
            PTRDMS banSao = TimBanSaoTheoMa(dauSach, maSach);
            if (banSao != nullptr) {
                banSao->trangThai = CHO_MUON_DUOC;
            }
        }
    }
    
    // B3: Cap nhat lich su muon tra
    found->data.NgayTra = layNgayHienTai();
    found->data.TrangThai = 1;  // da tra
    
    docGia->data.soSachDangMuon--;
    
    duLieuDaThayDoi = true;
    return "";  // thanh cong
}

//dung cho MaSach_to_PTRDMS
void themMuonTra(PTRDG docgia, PTRDMS banSaoSach) {
    if (docgia == nullptr) return;
    // Giả định SO_LAN_THU_TOI_DA là hằng số cho số sách mượn tối đa (ví dụ: 3)
    if (docgia->data.soSachDangMuon >= SO_LAN_THU_TOI_DA) { 
        std::cerr << "Lỗi: Độc giả đã mượn tối đa 3 cuốn sách." << std::endl;
        return;
    }

    MUONTRA node = new NodeMT;

    // 1. Gán con trỏ trực tiếp (Fix lỗi crash)
    node->data.banSaoSach = banSaoSach; 
    
    // 2. Gán dữ liệu giao dịch
    node->data.NgayMuon = layNgayHienTai();
    node->data.NgayTra = "";
    
    // 3. Gán trạng thái GIAO DỊCH, không phải trạng thái SÁCH (Fix lỗi logic)
    // 0 = Đang mượn
    node->data.TrangThai = 0; 

    node->next = docgia->data.dsmt;
    docgia->data.dsmt = node;
    
    // Cập nhật số lượng sách đang mượn
    docgia->data.soSachDangMuon++;
    // Cap nhat so Luot Muon
    std::string isbn = TachISBNTuMaSach(node->data.banSaoSach->maSach);
    PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
    if(dauSach != nullptr){
        dauSach->soLuotMuon++;//neu tim thay dauSach
    } else {
        std::cerr << "Canh bao: khong tim thay dau sach cho ma sach da muon";
    }
   
}

//dùng cho (NapGiaoDich)
void themMuonTra(PTRDG docgia, const MuonTra &mt) {
    if (docgia == nullptr) return;

    MUONTRA node = new NodeMT;
    node->data = mt;
    node->next = docgia->data.dsmt;
    docgia->data.dsmt = node;
    // Cập nhật bộ đếm dựa trên TRẠNG THÁI GIAO DỊCH (mt.TrangThai)
    // Chỉ tăng bộ đếm nếu giao dịch đó đang ở trạng thái "Đang mượn" (0)
    if (mt.TrangThai == 0) {
        docgia->data.soSachDangMuon++;
    }
}

//G.Tra Sach
string PTRDMS_to_String(PTRDMS p){
    return (p != nullptr)? p->maSach : "";
}

PTRDMS MaSach_to_PTRDMS(const string& maSach, PTRDS dsDauSach[], int soLuongDauSach){
    std::stringstream dummyStream;
    return timDanhMucTheoMaSach(maSach, dsDauSach,soLuongDauSach,dummyStream, true);
}

void LuuGiaoDich(MUONTRA dsmt, std::ofstream& file){
    MUONTRA p = dsmt;
    while(p != nullptr){
        std::string maSach = PTRDMS_to_String(p->data.banSaoSach);
        
        file << "MT|"
             << maSach << "|"
             << p->data.NgayMuon << "|"
             << p->data.NgayTra << "|"
             << p->data.TrangThai << "\n";
        p = p->next;
    }
}

void NapGiaoDich(PTRDG docGia, std::ifstream& file, PTRDS dsDauSach[], int n){
    std::string line;
    while(getline(file, line)){
        string fields[4];
        if(!TachTruong(line, '|', fields, 4)) continue;

        MuonTra mt;
        //Gan cac truong gia tri truc tiep(NgayMuon, NgayTra, TrangThai)
        mt.NgayMuon = fields[1];
        mt.NgayTra = fields[2];

        try{
            mt.TrangThai = stoi(fields[3]);
        } catch(...){
            cerr << "Loi chuyen doi trang thai giao dich: " << fields[3] << endl;
            continue;
        }
        // //Khoi phuc con tro tu ma sach
        // mt.banSaoSach = MaSach_to_PTRDMS(fields[0], dsDauSach, n);

        // //1. Lay ma sach tu file
        // std::string maSachFile = fields[0];

        // //2. Tach lay ISBN
        // std::string isbn = TachISBNTuMaSach(maSachFile);

        // //3. Tim dau sach va tang luot muon
        // PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, n, isbn);
        // if(dauSach != nullptr) 
        //     dauSach->soLuotMuon++;
        // if(mt.banSaoSach == nullptr){
        //     //canh bao neu con tra khong khoi phuc duoc
        //     std::cerr << "Canh bao: khong tim thay sach " << fields[0] << " de khoi phuc con tro"<< endl;
        // } else {
        //     std::string isbn = TachISBNTuMaSach(fields[0]);
        //     PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, n, isbn);
        //     if(dauSach != nullptr)
        //         dauSach->soLuotMuon++;
        // }
        std::cout << ">> Doc file MT: MaSach=" << fields[0]; 
        
        std::string maSachFile = fields[0]; 
        std::string isbn = TachISBNTuMaSach(maSachFile); 
        std::cout << " | ISBN tach duoc=" << isbn;

        PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, n, isbn);
        
        if (dauSach != nullptr) {
            dauSach->soLuotMuon++; 
            std::cout << " -> TIM THAY! Ten=" << dauSach->tenSach 
                      << " -> Tang len " << dauSach->soLuotMuon << "\n";
        } else {
            std::cout << " -> KHONG TIM THAY SACH NAY!\n";
        }

        themMuonTra(docGia, mt);
    }
}

//H.Liet ke danh sach cac ma sach, ten sach ma 1 doc gia co so the X dang muon
// lay danh sach sach dang muon cua doc gia (O(L) - L: so giao dich)
// ham backend: lay danh sach sach dang muon (O(L))
int LayDSSachDangMuon(PTRDG docGia, ThongTinSachDangMuon_DTO ketQua[], int maxKetQua, PTRDS dsDauSach[], int soLuongDauSach) {
    if (docGia == nullptr || ketQua == nullptr) return -1;  // loi: khong tim thay doc gia
    
    MUONTRA curr = docGia->data.dsmt;
    if (curr == nullptr) return 0;  // doc gia chua muon cuon nao
    
    int dem = 0;
    
    // duyet toan bo danh sach muon
    while (curr != nullptr) {
        // chi xu ly sach dang muon (TrangThai = 0)
        if (curr->data.TrangThai == 0) {
            // kiem tra con tro banSaoSach an toan
            if (curr->data.banSaoSach != nullptr) {
                std::string maSach = curr->data.banSaoSach->maSach;
                std::string isbn = TachISBNTuMaSach(maSach);
                
                // tim dau sach de lay ten sach
                PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
                
                // luu vao mang ket qua
                if (dem < maxKetQua) {
                    ketQua[dem].maSach = maSach;
                    ketQua[dem].tenSach = (dauSach != nullptr) ? dauSach->tenSach : "";
                    ketQua[dem].ngayMuon = curr->data.NgayMuon;
                    dem++;
                }
            }
        }
        curr = curr->next;
    }
    
    return dem;  // tra ve so luong sach dang muon
}

//I.In Doc Gia Qua Han 
// tinh so ngay qua han lon nhat cua 1 doc gia (O(L))
int TinhSoNgayQuaHanLonNhat(PTRDG docgia) {
    if (docgia == nullptr) return 0;
    
    int maxQuaHan = 0;
    int quaHan = 0;
    MUONTRA curr = docgia->data.dsmt;
    
    while (curr != nullptr) {
        if (curr->data.TrangThai == 0) {
            std::stringstream dummy;  // thay std::cout bang stream ao
            quaHan = tinhSoNgayQuaHan(curr->data.NgayMuon, dummy);
            
            if (quaHan > maxQuaHan)
                maxQuaHan = quaHan;
        }
        curr = curr->next;
    }
    return maxQuaHan;
}

// so sanh qua han (giam dan)
static int SoSanhQuaHan(const ThongTinQuaHan& a, const ThongTinQuaHan& b) {
    if (a.soNgayQuaHanMax != b.soNgayQuaHanMax) {
        return (a.soNgayQuaHanMax > b.soNgayQuaHanMax) ? -1 : 1;
    }
    return SoSanhTenHo(a.docGia, b.docGia);
}

// thu thap doc gia qua han (duyet inorder)
static void ThuThapDocGiaQuaHanRec(PTRDG root, ThongTinQuaHan arr[], int& count) {
    if (root == nullptr) return;
    
    // duyet InOrder (LNR)
    ThuThapDocGiaQuaHanRec(root->left, arr, count);
    
    // xu ly node hien tai
    int maxQuaHan = TinhSoNgayQuaHanLonNhat(root);  // O(L)
    
    if (maxQuaHan > 0) {
        if (count < MAX_DAUSACH) {
            arr[count].docGia = root;
            arr[count].soNgayQuaHanMax = maxQuaHan;
            count++;
        }
    }
    
    ThuThapDocGiaQuaHanRec(root->right, arr, count);
}

// ham ho tro sap xep
static void SwapQH(ThongTinQuaHan& a, ThongTinQuaHan& b) {
    ThongTinQuaHan temp = a; a = b; b = temp;
}

static int PartitionQH(ThongTinQuaHan arr[], int low, int high) {
    ThongTinQuaHan pivot = arr[high];
    int i = low - 1;
    
    for (int j = low; j <= high - 1; j++) {
        if (SoSanhQuaHan(arr[j], pivot) < 0) {
            i++;
            SwapQH(arr[j], arr[i]);
        }
    }
    SwapQH(arr[i + 1], arr[high]);
    return i + 1;
}

static void QuickSortQuaHan(ThongTinQuaHan arr[], int low, int high) {
    if (low < high) {
        int pi = PartitionQH(arr, low, high);
        QuickSortQuaHan(arr, low, pi - 1);
        QuickSortQuaHan(arr, pi + 1, high);
    }
}

// ham backend chinh: lay ds doc gia qua han (cap nhat dung DocGiaQuaHanDTO)
int LayDSDocGiaQuaHan(PTRDG root, DocGiaQuaHanDTO arr[], int maxKetQua) {
    if (root == nullptr || arr == nullptr || maxKetQua <= 0) return 0;
    
    // Tao mang tam ThongTinQuaHan de thu thap
    ThongTinQuaHan temp[1000];
    int count = 0;
    
    // 1. thu thap du lieu
    ThuThapDocGiaQuaHanRec(root, temp, count);  // O(N * L)
    
    // 2. sap xep mang quicksort
    if (count > 1) {
        QuickSortQuaHan(temp, 0, count - 1);  // O(N log N)
    }
    
    // 3. chuyen sang DocGiaQuaHanDTO
    int result = (count < maxKetQua) ? count : maxKetQua;
    for (int i = 0; i < result; ++i) {
        arr[i].docGia = temp[i].docGia;
        arr[i].soNgayQuaHanMax = temp[i].soNgayQuaHanMax;
    }
    
    return result;
}

//ham backend chinh: lay top 10 sach dang muon
static int SoSanhLM(const TopSachDTO& a, const TopSachDTO& b){
    //Sap xep soLuotMuon (giam dan)
    int LuotMuonA = (a.dauSach != nullptr) ? a.dauSach->soLuotMuon : 0;
    int LuotMuonB = (b.dauSach != nullptr) ? b.dauSach->soLuotMuon : 0;

    if(LuotMuonA != LuotMuonB){
        return (LuotMuonA > LuotMuonB) ? -1 : 1;
    }

    //Sap xep tenSach (tang dan)
    std::string tenA = (a.dauSach != nullptr) ? ChuanHoaChuoiTimKiem(a.dauSach->tenSach) : "";
    std::string tenB = (b.dauSach != nullptr) ? ChuanHoaChuoiTimKiem(b.dauSach->tenSach) : "";
    
    if (tenA != tenB) {
        return (tenA < tenB) ? -1 : 1;
    }
    return 0;//trung nhau
}

static void SwapTS(TopSachDTO& a, TopSachDTO& b){
    TopSachDTO temp = a; a = b; b = temp;
}

static int PartitionTS(TopSachDTO arr[], int low, int high){
    TopSachDTO pivot = arr[high];
    int i = (low - 1);

    for(int j = low; j < high - 1; j++){
        if (SoSanhLM(arr[j], pivot) < 0){
            i++;
            SwapTS(arr[i], arr[j]);
        }
    }
    SwapTS(arr[i + 1], arr[high]);
    return (i + 1);
}

static void QuickSortTS(TopSachDTO arr[], int low, int high){
    if(low < high){
        int pi = PartitionTS(arr, low, high);
        QuickSortTS(arr, low, pi - 1);
        QuickSortTS(arr, pi + 1, high);
    }
}
int LayTopSach(PTRDS dsDauSach[], int soLuongDS, TopSachDTO arr[]){
    // --- ĐOẠN DEBUG---
    std::cout << "\n--- DEBUG TOP 10 ---\n";
    std::cout << "So luong dau sach nhan duoc: " << soLuongDS << "\n";
    
    if(dsDauSach == nullptr) {
        std::cout << "LOI: Mang dsDauSach bi NULL!\n";
        return 0;
    }
    // -----------------

    if(arr == nullptr || soLuongDS == 0)  return 0;

    int count = 0;
    
    for(int i = 0; i < soLuongDS; i++){
        // ĐK QUAN TRỌNG: Lấy cả sách có lượt mượn >= 0 (thay vì > 0)
        if(dsDauSach[i] != nullptr && dsDauSach[i]->soLuotMuon >= 0){ 
            if(count < MAX_DAUSACH){
                arr[count].dauSach = dsDauSach[i];
                count++;
            }
        }
    }
    
    std::cout << "So luong sach lay duoc vao Top: " << count << "\n"; // In kết quả đếm
    
    if(count == 0) return 0; 

    if(count > 1){
        QuickSortTS(arr, 0, count - 1);
    }

    return (count < 10) ? count : 10;
}

//Save
//ham de quy
static void LuuNodeVaoFile(PTRDG node, std::ofstream& file){
    if(node == nullptr) return;

    // (N) Ghi thong tin Doc Gia
    file << "DG|"
         << node->data.MaThe << "|"
         << node->data.Ho << "|"
         << node->data.Ten << "|"
         << node->data.Phai << "|"
         << node->data.TrangThai << "\n";

    // (N) Ghi danh sach muon tra
    LuuGiaoDich(node->data.dsmt, file);

    // Ghi dau hieu ket thuc
    file << "END_DG\n";

    // (L) De quy sang trai
    LuuNodeVaoFile(node->left, file);
    // (R) De quy sang phai
    LuuNodeVaoFile(node->right, file);
}

void saveDocGia(PTRDG root){
    std::ofstream file("files/docgia.txt");

    if(!file.is_open()){
        std::cerr << "Loi: khong the mo file 'files/docgia.txt' de ghi!" << std::endl;
        return;
    }

    LuuNodeVaoFile(root, file);

    file.close();

    std::cout << "Thong tin: Da Luu file docgia.txt thanh cong." << std::endl;
}
//Load
PTRDG loadDocGia(PTRDS dsDauSach[], int soLuongDS){
    std::ifstream file("files/docgia.txt");
    if(!file.is_open()){
        std::cerr << "Khong mo duoc file docgia.txt de doc!" << std::endl;
        return nullptr;
    }

    PTRDG root = nullptr;
    std::string line;
    PTRDG docGiaHienTai = nullptr;
    std::string fields[6];//mang tam cho DG hoac MT

    while(std::getline(file, line)){
        if(line.empty()) continue;

        // Tach tien to (DG, MT, END_DG)
        std::string prefix;
        std::string data;
        size_t pos = line.find("|");
        if(pos != std::string::npos){
            prefix = line.substr(0, pos);
            data = line.substr(pos + 1);
        } else {
            prefix = line; // truong hop END_DG
        }
        
        //bat dau doc Doc Gia 
        if(prefix == "DG"){
            if(!TachTruong(data, '|', fields, 5)) continue; //Tach 5 truong con lai

            int mathe, phai, trangthai;
            std::string ho, ten;

            try{
                mathe = std::stoi(fields[0]);
                ho = fields[1];
                ten = fields[2];
                phai = std::stoi(fields[3]);
                trangthai = std::stoi(fields[4]);
            } catch(...){
                continue; //Loi parsing
            }

            // Tao node moi va gan no la node hien tai
            docGiaHienTai = taoDocGia(ho, ten, phai != 0, trangthai, mathe);

            // Chen vao cay AVL
            InsertDocGia(root, docGiaHienTai);
        } else if(prefix == "MT") {
            if(docGiaHienTai == nullptr) continue;//Bo qua neu chua co Doc Gia
            if(!TachTruong(data, '|', fields, 4)) continue;//Tach 4 truong con lai

            MuonTra mt;
            mt.NgayMuon = fields[1];
            mt.NgayTra = fields[2];
            mt.TrangThai = std::stoi(fields[3]);

            //Khoi phuc con tro
            mt.banSaoSach = MaSach_to_PTRDMS(fields[0], dsDauSach, soLuongDS);

            std::string maSachFile = fields[0];

            //Kiem tra du lieu hong trong file (MT|| )
            if(!maSachFile.empty()){
                std::string isbn = TachISBNTuMaSach(maSachFile);
                PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDS, isbn);

                if(dauSach != nullptr){
                    dauSach->soLuotMuon++;

                    std::cout << "[DEBUG] + 1 Luot muon cho: " << dauSach->tenSach << "\n";
                }
            }

            //Them vao DSLK
            themMuonTra(docGiaHienTai, mt);
        } else if(prefix == "END_DG"){
            docGiaHienTai = nullptr; //reset
        }
    }
    file.close();
    return root;
}

// Ham bao mat sach (TrangThai = 2)
std::string BaoMatSach(PTRDG docGia, const std::string& maSach, PTRDS dsDauSach[], int soLuongDauSach) {
    if (!docGia) return "Loi: Doc gia khong ton tai!";
    if (maSach.empty()) return "Loi: Ma sach khong hop le!";
    
    // Tim ISBN tu ma sach
    std::string isbn = TachISBNTuMaSach(maSach);
    if (isbn.empty()) return "Loi: Ma sach khong hop le!";
    
    // Tim dau sach
    PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
    if (!dauSach) return "Loi: Khong tim thay dau sach!";
    
    // Tim ban sao
    PTRDMS banSao = TimBanSaoTheoMa(dauSach, maSach);
    if (!banSao) return "Loi: Khong tim thay ma sach: " + maSach;
    
    // Tim giao dich muon dang hoat dong
    MUONTRA p = docGia->data.dsmt;
    while (p) {
        if (p->data.banSaoSach == banSao && p->data.TrangThai == 0) {
            // Cap nhat trang thai = 2 (Mat)
            p->data.TrangThai = 2;
            p->data.NgayTra = layNgayHienTai();
            
            // Thanh ly sach
            banSao->trangThai = THANH_LY;
            duLieuDaThayDoi = true;
            
            return ""; // Thanh cong
        }
        p = p->next;
    }
    
    return "Loi: Khong tim thay giao dich muon sach nay!";
}

// Hàm kiểm tra chuỗi fullString có BẮT ĐẦU bằng prefix hay không
bool KiemTraBatDauBang(const std::string& fullString, const std::string& prefix) {
    if(prefix.empty()) return true;
    if(prefix.length() > fullString.length()) return false;

    for(size_t i = 0; i < prefix.length(); i++){
        if(std::tolower(fullString[i]) != std::tolower(prefix[i])){
            return false;
        }
    }
    // DEBUG
    // std::cout << "Match: [" << prefix << "] in [" << fullString << "] -> TRUE\n";
    return true;
}