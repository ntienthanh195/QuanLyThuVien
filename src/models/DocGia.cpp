#include "models/DocGia.h"
#include "utils/FilePathManager.h"
#include "utils/BackupManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <string>
#include "models/ThaoTacFile.h"
#include "models/QuanLySach.h"
#include "Constants.h"
#include "utils/NgayThang.h"
// Cần thêm thư viện XuLyChuoi.h cho hàm BoDauVaThuong()
#include "utils/XuLyChuoi.h" 
// Database support
#include "utils/DatabaseManager.h"
#include <QSqlQuery>
#include <QVariant>
#include <functional>
// Cần thêm thư viện QuanLySach.h nếu sử dụng TimDanhMucSachTuMaSach
// #include "QuanLySach.h" 

#include "utils/EventBus.h"
#include "utils/Perf.h"

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
    // Emit event that a new reader was added
    try {
        if (node) EventBus::instance().docGiaAdded(node->data.MaThe);
    } catch(...) {}
}

// quan ly cay doc gia
// tao PTRDG docgia
PTRDG taoDocGia(std::string ho, std::string ten, bool phai, int trangthai, PTRDG root) {
    PTRDG dg = new NodeDG;
    dg->data.MaThe = sinhMaTheNgauNhien(root);
    // Normalize name fields
    dg->data.Ho = ChuanHoaHoTen(ho);
    dg->data.Ten = ChuanHoaHoTen(ten);
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
    // Normalize name fields
    dg->data.Ho = ChuanHoaHoTen(ho);
    dg->data.Ten = ChuanHoaHoTen(ten);
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
    // Defensive delete: handle possible self-loop corruption and avoid infinite loops
    while (dsmt != nullptr) {
        MUONTRA next = dsmt->next;
        // If node points to itself, delete and break
        if (next == dsmt) {
            delete dsmt;
            dsmt = nullptr;
            break;
        }
        delete dsmt;
        dsmt = next;
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
    // check existence before deletion
    PTRDG found = timDocGia(root, mathe);
    root = xoaDocGiaRec(root, mathe);
    if (found) {
        try {
            EventBus::instance().docGiaRemoved(mathe);
        } catch(...) {}
    }
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
    std::string hoChuan = ChuanHoaHoTen(new_data.Ho);
    std::string tenChuan = ChuanHoaHoTen(new_data.Ten);
    if (p->data.Ho != hoChuan) p->data.Ho = hoChuan;
    if (p->data.Ten != tenChuan) p->data.Ten = tenChuan;
    if (p->data.Phai != new_data.Phai) p->data.Phai = new_data.Phai;
    if (p->data.TrangThai != new_data.TrangThai) p->data.TrangThai = new_data.TrangThai;
    // Không cập nhật MaThe, dsmt, soSachDangMuon
    // Emit update event
    try {
        EventBus::instance().docGiaUpdated(new_data.MaThe);
    } catch(...) {}
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
    if (maSach.empty()) return nullptr;
    if (maSach.length() > DO_DAI_MA_SACH_MAX) return nullptr; // guard insane input

    PTRDMS p = dauSach->dms;
    if (!p) return nullptr; // no copies

    int dem = 0;

    while (p != nullptr && dem < SO_VONG_LAP_DMS_MAX) {
        // defensive: ensure p->maSach is reasonable before comparing
        if (!p->maSach.empty() && p->maSach == maSach) {
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

    // Use both logical and defensive caps to avoid infinite loops
    int limit = SO_VONG_LAP_DMS_MAX;
    if (limit <= 0) limit = MAX_BAN_SAO;

    while (p != nullptr && vong < limit) {
        // protect against malformed nodes
        if (p->trangThai == CHO_MUON_DUOC) {
            dem++;
        }

        // detect trivial self-loop
        if (p->next == p) {
            thongBao(std::cout, "Phat hien node DMS self-loop trong DemBanSaoCoTheMuon.", CANH_BAO);
            break;
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
    int safety = 0;
    while (p != nullptr && safety < SO_VONG_LAP_MT_MAX) {
        // detect trivial self-loop
        if (p->next == p) {
            thongBao(std::cout, "Phat hien node MUONTRA self-loop trong MuonSach. Dung duyet.", CANH_BAO);
            break;
        }
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
        ++safety;
    }
    if (safety >= SO_VONG_LAP_MT_MAX) {
        thongBao(std::cout, "Phat hien vong lap vo han MUONTRA trong MuonSach.", LOI);
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
    mt.maSachCached = (banSao != nullptr) ? banSao->maSach : std::string();
    mt.NgayMuon = layNgayHienTai(dummyStream); // Dùng dummyStream để tránh in log rác
    mt.NgayTra = "";
    mt.TrangThai = 0;  // dang muon
    
    themMuonTra(docGia, mt);
    
    duLieuDaThayDoi = true;

    dauSach->soLuotMuon++;
    // Notify listeners
    try {
        if (!mt.maSachCached.empty()) {
            EventBus::instance().muonTraAdded(docGia->data.MaThe, QString::fromStdString(mt.maSachCached));
        } else if (banSao != nullptr) {
            EventBus::instance().muonTraAdded(docGia->data.MaThe, QString::fromStdString(banSao->maSach));
        }
        EventBus::instance().docGiaUpdated(docGia->data.MaThe);
    } catch(...) {}
    return "";  // thanh cong
}

// ham backend: tra sach (O(log N) + O(L))
std::string TraSach(PTRDG docGia, const std::string& maSach, PTRDS dsDauSach[], int soLuongDauSach) {
    if (docGia == nullptr) return "Loi: Doc gia khong hop le!";
    
    // B1: Tim giao dich muon trong lich su
    MUONTRA curr = docGia->data.dsmt;
    MUONTRA found = nullptr;
    int safety = 0;
    while (curr != nullptr && safety < SO_VONG_LAP_MT_MAX) {
        if (curr->next == curr) {
            thongBao(std::cout, "Phat hien node MUONTRA self-loop trong TraSach. Dung duyet.", CANH_BAO);
            break;
        }
        // Prefer cached maSach if present (safer than dereferencing possibly-dangling PTRDMS)
        std::string currMaSach = !curr->data.maSachCached.empty() ? curr->data.maSachCached : (curr->data.banSaoSach ? curr->data.banSaoSach->maSach : std::string());
        if (!currMaSach.empty() && currMaSach == maSach && curr->data.TrangThai == 0) {
            found = curr;
            break;
        }
        curr = curr->next;
        ++safety;
    }
    if (safety >= SO_VONG_LAP_MT_MAX) {
        thongBao(std::cout, "Phat hien vong lap vo han MUONTRA trong TraSach.", LOI);
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
    // Emit events: muonTraUpdated + docGiaUpdated
    try {
        EventBus::instance().muonTraUpdated(docGia->data.MaThe, QString::fromStdString(maSach));
        EventBus::instance().docGiaUpdated(docGia->data.MaThe);
    } catch(...) {}
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
    node->data.maSachCached = (banSaoSach != nullptr) ? banSaoSach->maSach : std::string();
    
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
    // Phát sự kiện realtime
    try {
        if (!node->data.maSachCached.empty()) {
            EventBus::instance().muonTraAdded(docgia->data.MaThe, QString::fromStdString(node->data.maSachCached));
        } else if (node->data.banSaoSach != nullptr) {
            EventBus::instance().muonTraAdded(docgia->data.MaThe, QString::fromStdString(node->data.banSaoSach->maSach));
        }
        EventBus::instance().docGiaUpdated(docgia->data.MaThe);
    } catch(...) {}
   
}

//dùng cho (NapGiaoDich)
void themMuonTra(PTRDG docgia, const MuonTra &mt) {
    if (docgia == nullptr) return;

    MUONTRA node = new NodeMT;
    node->data = mt;
    node->next = docgia->data.dsmt;
    docgia->data.dsmt = node;
    // NOTE: This overload is used when loading existing transactions from storage.
    // The caller (loader) is responsible for setting `docgia->data.soSachDangMuon`
    // from the persisted value. Do NOT modify the counter here to avoid double-counting.
}

//G.Tra Sach
string PTRDMS_to_String(PTRDMS p){
    return (p != nullptr)? p->maSach : "";
}

PTRDMS MaSach_to_PTRDMS(const string& maSach, PTRDS dsDauSach[], int soLuongDauSach){
    std::stringstream dummyStream;
    return timDanhMucTheoMaSach(maSach, dsDauSach,soLuongDauSach,dummyStream, true);
}



//H.Liet ke danh sach cac ma sach, ten sach ma 1 doc gia co so the X dang muon
// lay danh sach sach dang muon cua doc gia (O(L) - L: so giao dich)
// ham backend: lay danh sach sach dang muon (O(L))
int LayDSSachDangMuon(PTRDG docGia, ThongTinSachDangMuon_DTO ketQua[], int maxKetQua, PTRDS dsDauSach[], int soLuongDauSach) {
    if (docGia == nullptr || ketQua == nullptr) return -1;  // loi: khong tim thay doc gia
    if (maxKetQua <= 0) return 0; // nothing to fill
    if (maxKetQua > MAX_DAUSACH) maxKetQua = MAX_DAUSACH;
    if (soLuongDauSach < 0 || soLuongDauSach > MAX_DAUSACH) {
        // invalid catalog array - treat as empty so lookups return nullptr safely
        soLuongDauSach = 0;
        dsDauSach = nullptr;
    }
    
    MUONTRA curr = docGia->data.dsmt;
    if (curr == nullptr) return 0;  // doc gia chua muon cuon nao

    int dem = 0;
    int safety = 0;

    // duyet toan bo danh sach muon with safety checks
    while (curr != nullptr && safety < SO_VONG_LAP_MT_MAX) {
        // detect trivial self-loop to avoid infinite loops / corruption
        if (curr->next == curr) {
            thongBao(std::cout, "Phat hien node MUONTRA self-loop. Dung duyet.", CANH_BAO);
            break;
        }

        // chi xu ly sach dang muon (TrangThai = 0)
        if (curr->data.TrangThai == 0) {
            // kiem tra con tro banSaoSach an toan
            if (curr->data.banSaoSach != nullptr) {
                std::string maSach = curr->data.banSaoSach->maSach;
                std::string isbn = TachISBNTuMaSach(maSach);

                // tim dau sach de lay ten sach
                PTRDS dauSach = nullptr;
                if (dsDauSach != nullptr && soLuongDauSach > 0) {
                    dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
                }

                // Diagnostic logging: append a brief record so failing reproductions can reveal state
                try {
                    std::ofstream dbg;
                    dbg.open("data/debug_muontra.log", std::ios::app);
                    if (dbg.is_open()) {
                        dbg << "LayDSSachDangMuon: docGia=" << docGia << " curr=" << curr << " safety=" << safety << " dem=" << dem << "\n";
                        if (!curr->data.maSachCached.empty()) dbg << "  maSachCached=" << curr->data.maSachCached << "\n";
                        if (curr->data.banSaoSach) dbg << "  banSaoPtr=" << curr->data.banSaoSach << " banSao.maSach=" << curr->data.banSaoSach->maSach << "\n";
                        else dbg << "  banSaoPtr=null\n";
                        dbg << "  soLuongDauSach=" << soLuongDauSach << "\n";
                        if (dauSach) dbg << "  dauSachPtr=" << dauSach << " ten_len=" << dauSach->tenSach.size() << "\n";
                        else dbg << "  dauSachPtr=null\n";
                        dbg.close();
                    }
                } catch(...) {}

                // luu vao mang ket qua
                if (dem < maxKetQua) {
                    ketQua[dem].maSach = maSach;
                    ketQua[dem].tenSach = (dauSach != nullptr) ? dauSach->tenSach : "";
                    ketQua[dem].ngayMuon = curr->data.NgayMuon;
                    dem++;
                }
            } else {
                // defensive: skip malformed entry but keep traversing
                thongBao(std::cout, "Canh bao: MUONTRA co banSaoSach == nullptr.", CANH_BAO);
            }
        }

        curr = curr->next;
        ++safety;
    }

    if (safety >= SO_VONG_LAP_MT_MAX) {
        thongBao(std::cout, "Phat hien vong lap vo han trong danh sach muon (MUONTRA).", LOI);
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
    int safety = 0;
    while (curr != nullptr && safety < SO_VONG_LAP_MT_MAX) {
        if (curr->next == curr) {
            thongBao(std::cout, "Phat hien node MUONTRA self-loop trong TinhSoNgayQuaHanLonNhat. Dung duyet.", CANH_BAO);
            break;
        }
        if (curr->data.TrangThai == 0) {
            std::stringstream dummy;  // thay std::cout bang stream ao
            quaHan = tinhSoNgayQuaHan(curr->data.NgayMuon, dummy);

            if (quaHan > maxQuaHan)
                maxQuaHan = quaHan;
        }
        curr = curr->next;
        ++safety;
    }
    if (safety >= SO_VONG_LAP_MT_MAX) {
        thongBao(std::cout, "Phat hien vong lap vo han MUONTRA trong TinhSoNgayQuaHanLonNhat.", LOI);
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
    // Delegate to ThaoTacFile centralized writer
    ThaoTacFile_LuuNodeVaoFile(node, file);
}

void saveDocGia(PTRDG root){
    try {
        // Tao backup truoc khi luu
        std::string loiBackup = AutoBackup();
        if (!loiBackup.empty()) {
            std::cerr << "Canh bao backup: " << loiBackup << std::endl;
        }
        
        std::string filePath = FilePathManager::getDocGiaPathStd();

        // Try writing to DB first
        QString dbPath = FilePathManager::getLibraryDbPath();
        // Diagnostic: log DB path availability
        if (dbPath.isEmpty()) {
            std::cerr << "[saveDocGia] Warning: FilePathManager returned empty DB path. Falling back to legacy file save.\n";
        } else {
            std::cerr << "[saveDocGia] Using DB path: " << dbPath.toStdString() << "\n";
        }

        if (DatabaseManager::instance().connect(dbPath)) {
            std::cerr << "[saveDocGia] Database connected successfully.\n";
            bool ok = true;
            DatabaseManager::instance().beginTransaction();

            // Clear existing doc_gia and muon_tra so we rebuild from in-memory tree
            // This ensures deletions in memory are persisted (previous logic only did INSERT OR REPLACE)
            if (!DatabaseManager::instance().execute("DELETE FROM muon_tra;")) ok = false;
            if (!DatabaseManager::instance().execute("DELETE FROM doc_gia;")) ok = false;

            // Insert/replace all doc_gia
            // helper: count active loans in a MUONTRA list
            auto countActiveLoans = [](MUONTRA head)->int{
                int cnt = 0;
                MUONTRA p = head;
                int safety = 0;
                while (p != nullptr && safety < SO_VONG_LAP_MT_MAX) {
                    if (p->next == p) break;
                    if (p->data.TrangThai == 0) ++cnt;
                    p = p->next; ++safety;
                }
                return cnt;
            };

            std::function<void(PTRDG)> insertRec;
            insertRec = [&](PTRDG node) {
                if (!node) return;
                insertRec(node->left);
                QSqlQuery q(DatabaseManager::instance().database());
                q.prepare("INSERT OR REPLACE INTO doc_gia (ma_the, ho, ten, phai, trang_thai, so_dien_thoai, email, ngay_dang_ky, ngay_het_han, so_sach_dang_muon) VALUES (:ma, :ho, :ten, :phai, :tt, :sdtt, :email, :ndk, :nhh, :so);");
                q.bindValue(":ma", node->data.MaThe);
                q.bindValue(":ho", QString::fromStdString(node->data.Ho));
                q.bindValue(":ten", QString::fromStdString(node->data.Ten));
                q.bindValue(":phai", node->data.Phai ? 1 : 0);
                q.bindValue(":tt", node->data.TrangThai);
                q.bindValue(":sdtt", QString::fromStdString(std::string()));
                q.bindValue(":email", QString::fromStdString(std::string()));
                q.bindValue(":ndk", QString::fromStdString(std::string()));
                q.bindValue(":nhh", QString::fromStdString(std::string()));
                // Persist canonical count: recompute from in-memory MUONTRA list to avoid stale/mismatched values
                int canonicalSo = countActiveLoans(node->data.dsmt);
                node->data.soSachDangMuon = canonicalSo; // keep in-memory consistent
                q.bindValue(":so", canonicalSo);
                if (!q.exec()) ok = false;
                insertRec(node->right);
            };

            insertRec(root);

            // Rebuild muon_tra: delete all then reinsert
            if (ok) {
                if (!DatabaseManager::instance().execute("DELETE FROM muon_tra;")) ok = false;
                else {
                    // traverse again and insert muon records
                    std::function<void(PTRDG)> insertMuonRec;
                    insertMuonRec = [&](PTRDG node) {
                        if (!node) return;
                        insertMuonRec(node->left);
                        MUONTRA p = node->data.dsmt;
                            int safetyP = 0;
                            while (p && safetyP < SO_VONG_LAP_MT_MAX) {
                                if (p->next == p) {
                                    thongBao(std::cout, "Phat hien node MUONTRA self-loop trong saveDocGia->insertMuonRec. Dung duyet.", CANH_BAO);
                                    break;
                                }
                                // find ban_sao id
                                QSqlQuery q2(DatabaseManager::instance().database());
                                q2.prepare("SELECT id FROM ban_sao WHERE ma_sach = :ma LIMIT 1");
                                // Use cached maSach when available to avoid dereferencing dangling pointers
                                std::string maForQuery = !p->data.maSachCached.empty() ? p->data.maSachCached : (p->data.banSaoSach ? p->data.banSaoSach->maSach : std::string());
                                q2.bindValue(":ma", QString::fromStdString(maForQuery));
                                if (q2.exec() && q2.next()) {
                                    int banId = q2.value(0).toInt();
                                    QSqlQuery q3(DatabaseManager::instance().database());
                                    q3.prepare("INSERT INTO muon_tra (ma_the, ban_sao_id, ngay_muon, ngay_tra, trang_thai, tien_phat) VALUES (:ma, :bid, :nm, :nt, :tt, :tp);");
                                    q3.bindValue(":ma", node->data.MaThe);
                                    q3.bindValue(":bid", banId);
                                    q3.bindValue(":nm", QString::fromStdString(p->data.NgayMuon));
                                    q3.bindValue(":nt", QString::fromStdString(p->data.NgayTra));
                                    q3.bindValue(":tt", p->data.TrangThai);
                                    q3.bindValue(":tp", 0);
                                    if (!q3.exec()) ok = false;
                                }
                                p = p->next;
                                ++safetyP;
                            }
                            if (safetyP >= SO_VONG_LAP_MT_MAX) {
                                thongBao(std::cout, "Phat hien vong lap vo han MUONTRA khi luu muon_tra (saveDocGia).", LOI);
                            }
                        insertMuonRec(node->right);
                    };
                    insertMuonRec(root);
                }
            }

            if (ok) {
                DatabaseManager::instance().commit();
                std::cerr << "[saveDocGia] Commit successful.\n";
            } else {
                DatabaseManager::instance().rollback();
                // Print DB last error for diagnostics
                QSqlError err = DatabaseManager::instance().lastError();
                std::cerr << "[saveDocGia] ERROR: commit failed, rolled back. QSqlError: " << err.text().toStdString() << "\n";
            }
            DatabaseManager::instance().close();
            std::cout << "Thong tin: Da Luu doc_gia -> DB." << std::endl;
            // DB save succeeded — do not write legacy text file when using DB-only mode
            try { EventBus::instance().dataChanged(); } catch(...) {}
            return;
        }

        std::ofstream file(filePath);

        if(!file.is_open()){
            std::cerr << "Loi: khong the mo file '" << filePath << "' de ghi!" << std::endl;
            return;
        }

        LuuNodeVaoFile(root, file);

        file.close();

        std::cout << "Thong tin: Da Luu file docgia.txt thanh cong." << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Loi ngoai le khi luu doc gia: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Loi khong xac dinh khi luu doc gia" << std::endl;
    }
}
//Load
PTRDG loadDocGia(PTRDS dsDauSach[], int soLuongDS){
    PerfTimer perf("loadDocGia");
    try {
        PTRDG root = nullptr;

        // Try loading from DB first
        QString dbPath = FilePathManager::getLibraryDbPath();
        if (DatabaseManager::instance().connect(dbPath)) {
            QSqlQuery q(DatabaseManager::instance().database());
            if (q.exec("SELECT ma_the, ho, ten, phai, trang_thai, so_sach_dang_muon FROM doc_gia ORDER BY ma_the")) {
                while (q.next()) {
                    int maThe = q.value(0).toInt();
                    QString hoQ = q.value(1).toString();
                    QString tenQ = q.value(2).toString();
                    int phaiQ = q.value(3).toInt();
                    int trangQ = q.value(4).toInt();
                    int soSachDangMuon = q.value(5).toInt();

                    PTRDG node = taoDocGia(hoQ.toStdString(), tenQ.toStdString(), phaiQ != 0, trangQ, maThe);
                    node->data.soSachDangMuon = soSachDangMuon;
                    InsertDocGia(root, node);
                }

                // Load muon_tra and attach to docGia nodes
                QSqlQuery q2(DatabaseManager::instance().database());
                if (q2.exec("SELECT m.ma_the, b.ma_sach, m.ngay_muon, m.ngay_tra, m.trang_thai FROM muon_tra m JOIN ban_sao b ON m.ban_sao_id = b.id ORDER BY m.id")) {
                    while (q2.next()) {
                        int maThe = q2.value(0).toInt();
                        QString maSachQ = q2.value(1).toString();
                        QString ngayMuonQ = q2.value(2).toString();
                        QString ngayTraQ = q2.value(3).toString();
                        int trangQ = q2.value(4).toInt();

                        PTRDG p = timDocGia(root, maThe);
                        if (!p) continue;

                        MuonTra mt;
                        mt.NgayMuon = ngayMuonQ.toStdString();
                        mt.NgayTra = ngayTraQ.toStdString();
                        mt.TrangThai = trangQ;
                        // Resolve ban_sao pointer from ma_sach
                        mt.banSaoSach = MaSach_to_PTRDMS(maSachQ.toStdString(), dsDauSach, soLuongDS);
                        mt.maSachCached = maSachQ.toStdString();

                        // Ensure the in-memory copy status reflects the loan record we just loaded.
                        // If the transaction is active (TrangThai == 0) mark the ban_sao as DANG_MUON,
                        // otherwise mark it as available (CHO_MUON_DUOC). This keeps
                        // LayThongKeSach() consistent with muon_tra table.
                        if (mt.banSaoSach != nullptr) {
                            if (mt.TrangThai == 0) {
                                mt.banSaoSach->trangThai = DANG_MUON;
                            } else {
                                mt.banSaoSach->trangThai = CHO_MUON_DUOC;
                            }
                        }

                        if (!maSachQ.isEmpty()) {
                            std::string isbn = TachISBNTuMaSach(maSachQ.toStdString());
                            PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDS, isbn);
                            if (dauSach != nullptr) {
                                dauSach->soLuotMuon++;
                            }
                        }

                        themMuonTra(p, mt);
                    }
                }

                // After loading muon_tra rows and attaching them to docGia nodes,
                // recompute `soSachDangMuon` from the attached MUONTRA lists to ensure consistency
                std::function<void(PTRDG)> recomputeCounts;
                recomputeCounts = [&](PTRDG node) {
                    if (!node) return;
                    recomputeCounts(node->left);
                    // count active loans
                    int cnt = 0;
                    MUONTRA p = node->data.dsmt;
                    int safety = 0;
                    while (p != nullptr && safety < SO_VONG_LAP_MT_MAX) {
                        if (p->next == p) break;
                        if (p->data.TrangThai == 0) ++cnt;
                        p = p->next; ++safety;
                    }
                    node->data.soSachDangMuon = cnt;
                    recomputeCounts(node->right);
                };

                recomputeCounts(root);

                DatabaseManager::instance().close();
                return root;
            }
            DatabaseManager::instance().close();
        }

        // Fallback to legacy file-based loader (delegated)
        std::string filePath = FilePathManager::getDocGiaPathStd();
        PTRDG rootFromFile = NapDocGiaFile(filePath.c_str(), dsDauSach, soLuongDS);
        return rootFromFile;
    } catch (const std::exception &e) {
        std::cerr << "Loi ngoai le khi nap doc gia: " << e.what() << std::endl;
        return nullptr;
    } catch (...) {
        std::cerr << "Loi khong xac dinh khi nap doc gia" << std::endl;
        return nullptr;
    }
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


//  THONG KE - STATISTICS FUNCTIONS FOR DOC GIA 

// helper de duyet cay va dem
static void DemDocGiaRecursive(PTRDG node, int& tong, int& hoatDong, int& khoa, 
                               int& dangMuon, int& tongSachMuon, int& coQuaHan,
                               PTRDS dsDauSach[], int soLuongDauSach) {
    if (!node) return;
    
    // Dem doc gia
    tong++;
    
    // Dem theo trang thai
    if (node->data.TrangThai == 1) {
        hoatDong++;
    } else {
        khoa++;
    }
    
    // Dem so sach dang muon
    int sachDangMuonCuaDocGia = 0;
    MUONTRA pTemp = node->data.dsmt;
    bool coSachQuaHan = false;
    int safety = 0;
    while (pTemp != nullptr && safety < SO_VONG_LAP_MT_MAX) {
        if (pTemp->next == pTemp) {
            thongBao(std::cout, "Phat hien node MUONTRA self-loop trong DemDocGiaRecursive. Dung duyet.", CANH_BAO);
            break;
        }
        if (pTemp->data.TrangThai == 0) {  // dang muon
            sachDangMuonCuaDocGia++;

            // kiem tra qua han
            std::stringstream dummyStream;
            int soNgayQuaHan = tinhSoNgayQuaHan(pTemp->data.NgayMuon, dummyStream);
            if (soNgayQuaHan > 0) {
                coSachQuaHan = true;
            }
        }
        pTemp = pTemp->next;
        ++safety;
    }
    if (safety >= SO_VONG_LAP_MT_MAX) {
        thongBao(std::cout, "Phat hien vong lap vo han MUONTRA trong DemDocGiaRecursive.", LOI);
    }
    
    if (sachDangMuonCuaDocGia > 0) {
        dangMuon++;
        tongSachMuon += sachDangMuonCuaDocGia;
    }
    
    if (coSachQuaHan) {
        coQuaHan++;
    }
    
    // Duyet cay con
    DemDocGiaRecursive(node->left, tong, hoatDong, khoa, dangMuon, tongSachMuon, coQuaHan, dsDauSach, soLuongDauSach);
    DemDocGiaRecursive(node->right, tong, hoatDong, khoa, dangMuon, tongSachMuon, coQuaHan, dsDauSach, soLuongDauSach);
}

// lay thong ke tong quan doc gia
ThongKeDocGia LayThongKeDocGia(PTRDG root, PTRDS dsDauSach[], int soLuongDauSach) {
    ThongKeDocGia thongKe = {};
    thongKe.tongSoDocGia = 0;
    thongKe.docGiaHoatDong = 0;
    thongKe.docGiaBiKhoa = 0;
    thongKe.docGiaDangMuonSach = 0;
    thongKe.tongSoSachDangMuon = 0;
    thongKe.docGiaCoSachQuaHan = 0;
    
    DemDocGiaRecursive(root, thongKe.tongSoDocGia, thongKe.docGiaHoatDong, 
                      thongKe.docGiaBiKhoa, thongKe.docGiaDangMuonSach,
                      thongKe.tongSoSachDangMuon, thongKe.docGiaCoSachQuaHan,
                      dsDauSach, soLuongDauSach);
    
    return thongKe;
}

// helper de dem doc gia theo trang thai
static void DemTheoTrangThaiRec(PTRDG node, int trangThai, int& dem) {
    if (!node) return;
    
    if (node->data.TrangThai == trangThai) {
        dem++;
    }
    
    DemTheoTrangThaiRec(node->left, trangThai, dem);
    DemTheoTrangThaiRec(node->right, trangThai, dem);
}

// dem doc gia theo trang thai (0: khoa, 1: hoat dong)
int DemDocGiaTheoTrangThai(PTRDG root, int trangThai) {
    int dem = 0;
    DemTheoTrangThaiRec(root, trangThai, dem);
    return dem;
}

// helper de dem doc gia dang muon sach
static void DemDangMuonRec(PTRDG node, int& dem) {
    if (!node) return;
    
    // kiem tra doc gia co sach dang muon khong
    MUONTRA pTemp = node->data.dsmt;
    int safety = 0;
    while (pTemp != nullptr && safety < SO_VONG_LAP_MT_MAX) {
        if (pTemp->next == pTemp) {
            thongBao(std::cout, "Phat hien node MUONTRA self-loop trong DemDangMuonRec. Dung duyet.", CANH_BAO);
            break;
        }
        if (pTemp->data.TrangThai == 0) {  // dang muon
            dem++;
            break;  // chi dem 1 lan cho moi doc gia
        }
        pTemp = pTemp->next;
        ++safety;
    }
    if (safety >= SO_VONG_LAP_MT_MAX) {
        thongBao(std::cout, "Phat hien vong lap vo han MUONTRA trong DemDangMuonRec.", LOI);
    }
    
    DemDangMuonRec(node->left, dem);
    DemDangMuonRec(node->right, dem);
}

// dem doc gia dang muon sach
int DemDocGiaDangMuonSach(PTRDG root) {
    int dem = 0;
    DemDangMuonRec(root, dem);
    return dem;
}


//  TIM KIEM NANG CAO DOC GIA 

// helper de duyet cay va tim theo ten
static void TimTheoTenRec(PTRDG node, const std::string& tenThuong, PTRDG ketQua[], int& count, int maxKetQua) {
    if (!node || count >= maxKetQua) return;
    
    // duyet in-order
    TimTheoTenRec(node->left, tenThuong, ketQua, count, maxKetQua);
    
    if (count < maxKetQua) {
        std::string tenNodeThuong = BoDauVaThuong(node->data.Ten);
        
        if (tenNodeThuong.find(tenThuong) != std::string::npos) {
            ketQua[count++] = node;
        }
    }
    
    TimTheoTenRec(node->right, tenThuong, ketQua, count, maxKetQua);
}

// tim doc gia theo ten (khong phan biet hoa/thuong)
int TimDocGiaTheoTen(PTRDG root, const std::string& ten, PTRDG ketQua[], int maxKetQua) {
    if (!root || !ketQua || maxKetQua <= 0 || ten.empty()) return 0;
    
    std::string tenThuong = BoDauVaThuong(ten);
    int count = 0;
    
    TimTheoTenRec(root, tenThuong, ketQua, count, maxKetQua);
    
    return count;
}

// helper de duyet cay va tim theo ho
static void TimTheoHoRec(PTRDG node, const std::string& hoThuong, PTRDG ketQua[], int& count, int maxKetQua) {
    if (!node || count >= maxKetQua) return;
    
    // duyet in-order
    TimTheoHoRec(node->left, hoThuong, ketQua, count, maxKetQua);
    
    if (count < maxKetQua) {
        std::string hoNodeThuong = BoDauVaThuong(node->data.Ho);
        
        if (hoNodeThuong.find(hoThuong) != std::string::npos) {
            ketQua[count++] = node;
        }
    }
    
    TimTheoHoRec(node->right, hoThuong, ketQua, count, maxKetQua);
}

// tim doc gia theo ho (khong phan biet hoa/thuong)
int TimDocGiaTheoHo(PTRDG root, const std::string& ho, PTRDG ketQua[], int maxKetQua) {
    if (!root || !ketQua || maxKetQua <= 0 || ho.empty()) return 0;
    
    std::string hoThuong = BoDauVaThuong(ho);
    int count = 0;
    
    TimTheoHoRec(root, hoThuong, ketQua, count, maxKetQua);
    
    return count;
}

// helper de lay danh sach theo trang thai
static void LayTheoTrangThaiRec(PTRDG node, int trangThai, PTRDG ketQua[], int& count, int maxKetQua) {
    if (!node || count >= maxKetQua) return;
    
    // duyet in-order
    LayTheoTrangThaiRec(node->left, trangThai, ketQua, count, maxKetQua);
    
    if (count < maxKetQua && node->data.TrangThai == trangThai) {
        ketQua[count++] = node;
    }
    
    LayTheoTrangThaiRec(node->right, trangThai, ketQua, count, maxKetQua);
}

// lay danh sach doc gia theo trang thai
int TimDocGiaTheoTrangThai(PTRDG root, int trangThai, PTRDG ketQua[], int maxKetQua) {
    if (!root || !ketQua || maxKetQua <= 0) return 0;
    
    int count = 0;
    LayTheoTrangThaiRec(root, trangThai, ketQua, count, maxKetQua);
    
    return count;
}

// helper de tim theo ho + ten
static void TimTheoHoTenRec(PTRDG node, const std::string& hoTenThuong, PTRDG ketQua[], int& count, int maxKetQua) {
    if (!node || count >= maxKetQua) return;
    
    // duyet in-order
    TimTheoHoTenRec(node->left, hoTenThuong, ketQua, count, maxKetQua);
    
    if (count < maxKetQua) {
        std::string hoTenNode = BoDauVaThuong(node->data.Ho + " " + node->data.Ten);
        
        if (hoTenNode.find(hoTenThuong) != std::string::npos) {
            ketQua[count++] = node;
        }
    }
    
    TimTheoHoTenRec(node->right, hoTenThuong, ketQua, count, maxKetQua);
}

// tim doc gia theo ho + ten
int TimDocGiaTheoHoTen(PTRDG root, const std::string& hoTen, PTRDG ketQua[], int maxKetQua) {
    if (!root || !ketQua || maxKetQua <= 0 || hoTen.empty()) return 0;
    
    std::string hoTenThuong = BoDauVaThuong(hoTen);
    int count = 0;
    
    TimTheoHoTenRec(root, hoTenThuong, ketQua, count, maxKetQua);
    
    return count;
}
// One-off migration: create backup of docgia file and re-save normalized data
void MigrateNormalizeDocGiaFile(PTRDG root) {
    try {
        std::string filePath = FilePathManager::getDocGiaPathStd();
        // Create centralized backup under data/backup with timestamp
        std::string filesDir = FilePathManager::getFilesDirectory().toStdString();
        std::string backupsDir = filesDir + "/backup";
        try {
            std::error_code ec;
            std::filesystem::create_directories(backupsDir, ec);
            if (ec) {
                std::cerr << "Warning: could not ensure backup dir: " << ec.message() << std::endl;
            }
        } catch (...) {
            // ignore
        }

        std::string backupPath = backupsDir + "/docgia.bak_" + std::to_string(std::time(nullptr));

        // Create backup copy via centralized helper
        if (ThaoTacFile_BackupFile(filePath, backupPath)) {
            std::cout << "Backup created: " << backupPath << std::endl;
        } else {
            std::cout << "No existing docgia file to backup/migrate or backup failed." << std::endl;
        }

        // Now re-save using current in-memory tree (taoDocGia already normalizes names)
        if (root != nullptr) {
            saveDocGia(root);
            std::cout << "Migration: docgia file rewritten with normalized names." << std::endl;
        } else {
            std::cout << "Migration: root is null, skipping save." << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << "Loi khi migrate docgia: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Loi khong xac dinh khi migrate docgia" << std::endl;
    }
}