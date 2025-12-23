# BÁO CÁO PHÂN TÍCH TOÀN DIỆN DỰ ÁN QUẢN LÝ THƯ VIỆN

**Ngày lập báo cáo:** 23 Tháng 12, 2025  
**Phiên bản dự án:** 2.5  
**Người phân tích:** AI Analysis System  
**Trạng thái dự án:** Production-Ready

---

## 📋 MỤC LỤC

1. [Tổng Quan Dự Án](#1-tổng-quan-dự-án)
2. [Phân Tích Backend & Database](#2-phân-tích-backend--database)
3. [Phân Tích Business Logic & Models](#3-phân-tích-business-logic--models)
4. [Phân Tích UI/UX](#4-phân-tích-uiux)
5. [Đánh Giá Kiến Trúc & Code Quality](#5-đánh-giá-kiến-trúc--code-quality)
6. [Phân Tích Utilities & Helpers](#6-phân-tích-utilities--helpers)
7. [Điểm Mạnh](#7-điểm-mạnh)
8. [Điểm Yếu & Rủi Ro](#8-điểm-yếu--rủi-ro)
9. [Khuyến Nghị Cải Thiện](#9-khuyến-nghị-cải-thiện)
10. [Kết Luận](#10-kết-luận)

---

## 1. TỔNG QUAN DỰ ÁN

### 1.1. Thông Tin Cơ Bản

**Tên dự án:** Hệ Thống Quản Lý Thư Viện (Library Management System)  
**Phiên bản:** 2.5  
**Ngôn ngữ:** C++17  
**Framework:** Qt 6.10 (Qt Widgets)  
**Cơ sở dữ liệu:** SQLite (chế độ WAL)  
**Nền tảng:** Windows 10/11  
**Mục đích:** Phần mềm quản lý thư viện offline chuyên dụng cho môi trường không có internet

### 1.2. Tính Năng Chính

#### ✅ **Nghiệp vụ Mượn - Trả (Circulation)**
- Hỗ trợ máy quét mã vạch tự động
- Tự động phân biệt mã sách và mã thẻ độc giả
- Ràng buộc dữ liệu: Khóa quyền mượn khi quá hạn hoặc nợ phí
- Xử lý sự cố: Báo mất sách, đền bù, hoàn tiền
- Âm thanh thông báo khi quét thành công/lỗi
- Hệ thống gia hạn sách (Feature 4)

#### ✅ **Kiểm kê kho (Inventory)**
- Chế độ quét nhanh với máy quét
- Báo cáo chênh lệch: sách mất, sai vị trí, sai trạng thái
- Tích hợp với database realtime

#### ✅ **In ấn & Mã vạch**
- In thẻ độc giả với mã QR
- In tem nhãn sách (spine labels)
- Xuất báo cáo PDF (thống kê, danh sách quá hạn, biên bản)
- Hàng đợi in thông minh (PrintQueueManager)

#### ✅ **Quản lý dữ liệu**
- Import/Export Excel (hàng nghìn bản ghi trong vài giây)
- Hỗ trợ đa ngôn ngữ (tiếng Việt, Khmer - font Siemreap)
- Auto Backup trước khi thay đổi lớn
- Audit Log đầy đủ

#### ✅ **Quản lý người dùng & bảo mật (Phase 3)**
- Hệ thống đăng nhập với mã hóa mật khẩu (SHA256 + Salt)
- Phân quyền theo vai trò (Admin, Thủ thư)
- Audit Log theo từng người dùng
- Đăng xuất và khởi động lại session

### 1.3. Cấu Trúc Thư Mục

```
QuanLyThuVien/
├── include/              # Header files
│   ├── models/          # Data models (DauSach, DocGia, MuonTra)
│   ├── ui/              # UI widgets & dialogs
│   └── utils/           # Utilities (Database, Print, Backup...)
├── src/                 # Source implementations
│   ├── models/
│   ├── ui/
│   └── utils/
├── resources/           # QSS styles, icons, fonts
│   ├── icons/
│   ├── fonts/          # Khmer (Siemreap), Barcode font
│   └── *.qss           # 3 theme variants
├── data/               # Runtime data
│   ├── files/          # Database (library.db)
│   ├── backup/         # Backups
│   ├── logs/           # Application logs
│   └── qrcodes/        # Generated QR codes
├── tools/              # Python scripts (import, migration, cleanup)
├── tests/              # Unit tests
└── installer/          # Inno Setup installer
```

---

## 2. PHÂN TÍCH BACKEND & DATABASE

### 2.1. Kiến Trúc Database

**Công nghệ:** SQLite 3 với chế độ WAL (Write-Ahead Logging)

**Schema chính:**

#### 📘 **Bảng `dau_sach` (Book Titles)**
```sql
- id (PK, AUTO_INCREMENT)
- isbn (UNIQUE, NOT NULL) - Mã ISBN quốc tế
- ten_sach (NOT NULL) - Tên sách
- so_trang (INTEGER)
- tac_gia (TEXT)
- nam_xuat_ban (INTEGER)
- the_loai (TEXT)
- so_luot_muon (DEFAULT 0) - Tracking popularity
- gia_tien (DEFAULT 0) - Giá sách (VNĐ)
```

#### 📗 **Bảng `ban_sao` (Book Copies)**
```sql
- id (PK)
- ma_sach (UNIQUE, NOT NULL) - Mã cụ thể (ISBN-1, ISBN-2...)
- dau_sach_id (FK -> dau_sach) ON DELETE CASCADE
- trang_thai (0: Trong kho, 1: Đang mượn, 2: Mất, 3: Thanh lý)
- vi_tri (TEXT) - Vị trí kệ (A-12, Kệ B5...)
```

#### 👥 **Bảng `doc_gia` (Readers/Students)**
```sql
- ma_the (PK) - Mã thẻ độc giả
- ho, ten (TEXT)
- phai (INTEGER: 0=Nam, 1=Nữ)
- trang_thai (0: Khóa, 1: Hoạt động)
- co_the_muon (BOOLEAN) - Cached permission flag
- so_dien_thoai, email, ngay_sinh (TEXT)
- lop (TEXT) - Đơn vị/Lớp
- loai_doc_gia (0: Học sinh, 1: Giáo viên)
- ngay_dang_ky, ngay_het_han (DATE)
- so_sach_dang_muon (DEFAULT 0)
```

#### 📋 **Bảng `muon_tra` (Transactions)**
```sql
- id (PK)
- ma_the (FK -> doc_gia) ON DELETE CASCADE
- ban_sao_id (FK -> ban_sao)
- ngay_muon, ngay_tra (DATE)
- trang_thai (0-5: Finite State Machine)
  * 0: Đang mượn
  * 1: Đã trả
  * 2: Mất - Chờ thanh toán
  * 3: Mất - Đã thanh toán
  * 4: Mất - Chờ hoàn tiền
  * 5: Mất - Đã hoàn tiền
- tien_phat (INTEGER, DEFAULT 0)
- so_lan_gia_han (DEFAULT 0)
- da_thanh_toan (BOOLEAN, DEFAULT 0)
```

#### 🔐 **Bảng `vai_tro` (Roles) & `nguoi_dung` (Users) - Phase 3**
```sql
vai_tro:
- id, ten_vai_tro, mo_ta, quyen_han (TEXT/JSON)

nguoi_dung:
- id, ten_dang_nhap (UNIQUE), mat_khau_hash, muoi (salt)
- ho_ten, vai_tro_id (FK), trang_thai
```

#### 📝 **Bảng `nhat_ky_he_thong` (Audit Log)**
```sql
- id, nguoi_dung_id (FK), hanh_dong, bang_du_lieu
- doi_tuong_id, chi_tiet, thoi_gian
```

#### ⚙️ **Bảng `cai_dat` (Settings)**
```sql
- khoa (PK) - Setting key
- gia_tri - Value (as TEXT)
- mo_ta - Description
- kieu_du_lieu (string/int/bool/color)
```

**Ví dụ cài đặt:**
- `SO_SACH_MUON_TOI_DA`: 3
- `SO_NGAY_MUON_TOI_DA`: 14
- `TIEN_PHAT_MOI_NGAY`: 5000 VNĐ
- `SO_LAN_GIA_HAN_TOI_DA`: 1

### 2.2. Database Manager (`DatabaseManager.cpp`)

**Đặc điểm:**
- **Pattern:** Singleton thread-safe
- **Transaction Support:** BEGIN/COMMIT/ROLLBACK
- **Migration:** `createTables()` - Tự động tạo schema & seed data
- **Auto-normalization:** Chuẩn hóa mã sách (uppercase ISBN)
- **Archiving:** `archiveOldRecords()` - Dọn dẹp giao dịch cũ

**Seed Data:**
- Admin user: `admin` / `admin` (SHA256 + salt)
- Default roles: Admin, Thủ thư
- Default settings (configurable)

### 2.3. Đánh Giá Database

#### ✅ Điểm Mạnh:
1. **Schema chuẩn:** Foreign keys, CASCADE delete, indexes
2. **Tính toàn vẹn:** Ràng buộc UNIQUE, NOT NULL
3. **Extensible:** Dễ thêm cột mới (ALTER TABLE)
4. **Offline-first:** SQLite phù hợp cho môi trường không internet
5. **WAL mode:** Tăng tốc đọc/ghi đồng thời
6. **Audit trail:** Đầy đủ nhật ký hệ thống

#### ⚠️ Điểm Yếu:
1. **Không có indexes:** Thiếu indexes trên `ma_sach`, `isbn`, `ma_the` → Chậm khi data lớn
2. **No connection pooling:** Mỗi lần mở/đóng connection
3. **No prepared statement caching:** Tạo mới QSqlQuery mỗi lần
4. **Archiving thủ công:** Chưa tự động dọn dẹp theo schedule
5. **Backup chưa tối ưu:** Copy toàn bộ file DB thay vì incremental backup

---

## 3. PHÂN TÍCH BUSINESS LOGIC & MODELS

### 3.1. Data Structures

#### 📚 **DauSach (Book Title)**
```cpp
struct DauSach {
    std::string ISBN;
    std::string tenSach;
    int soTrang;
    std::string tacGia;
    int namXuatBan;
    std::string theLoai;
    PTRDMS dms; // Linked list ban sao
    std::vector<PTRDMS> dms_vector; // Vector để duyệt nhanh (Phase 5)
    int tongBanSao;
    int soLuotMuon; // Top 10 tracking
    long long giaTien; // VNĐ
    std::string NhaXuatBan;
};
```

**Lưu trữ:** `std::vector<std::shared_ptr<DauSach>> dsDauSach` (global)

#### 📖 **DanhMucSach (Book Copy)**
```cpp
struct DanhMucSach {
    std::string maSach;         // ISBN-1, ISBN-2...
    TrangThaiSach trangThai;    // Enum (0-3)
    std::string viTri;          // Kệ A-12
    std::shared_ptr<DanhMucSach> next; // Linked list
};
```

**Tổ chức:** Mỗi DauSach có 1 danh sách liên kết đơn các bản sao

#### 👤 **DocGia (Reader) - AVL Tree**
```cpp
struct TheDocGia {
    int MaThe;
    std::string Ho, Ten, Phai;
    TrangThaiTheDocGia TrangThai; // 0: Khóa, 1: Hoạt động
    MUONTRA dsmt; // Vector giao dịch
    
    // Cached fields (optimization)
    int soSachDangMuon;
    long long tongNo;
    time_t ngayHetHanSomNhat;
    bool coTheMuon;
    
    // Phase 2 fields
    string SDT, Lop, Email, NgaySinh;
    int LoaiDocGia; // 0: Học sinh, 1: Giáo viên
};

struct NodeDG {
    TheDocGia data;
    int bf, height; // AVL balance
    PTRDG left, right;
};
```

**Cấu trúc:** Cây AVL (self-balancing binary search tree) → Tìm kiếm O(log N)

#### 📋 **MuonTra (Transaction)**
```cpp
struct MuonTra {
    PTRDMS banSaoSach; // Pointer to book copy
    string NgayMuon, NgayTra;
    int TrangThai; // 0-5 (FSM)
    
    // Feature 4: Financial
    int soLanGiaHan;
    bool daThanhToan;
    long long tienPhat;
    long long tienHoanTra; // Refund amount
    
    string maSachCached; // Cached ma_sach to avoid dangling pointers
};
```

### 3.2. Core Algorithms

#### 🔍 **Tìm Kiếm Sách**
```cpp
// Binary search trên vector đã sắp xếp
PTRDS TimDauSachTheoISBN(vector<PTRDS>& ds, int n, string isbn)
// Complexity: O(log N)
```

#### 🌲 **AVL Tree Operations**
```cpp
void InsertDocGia(PTRDG &root, PTRDG node, bool syncDB)
PTRDG timDocGia(PTRDG root, int mathe)
// Complexity: O(log N) insert/search
// Auto-balancing: RotateLeft, RotateRight
```

#### 📦 **Quản Lý Bản Sao**
```cpp
PTRDMS TimBanSaoCoTheMuon(PTRDS dauSach)
// Traverse linked list: O(M) where M = số bản sao
// Guard: SO_VONG_LAP_DMS_MAX = 1000 (prevent infinite loop)
```

#### 💰 **Tính Phí Phạt (Financial State Machine)**
```cpp
long long TinhPhiPhatQuaHan(time_t ngayMuon, time_t ngayTraThucTe)
// Formula: (days_overdue) * TIEN_PHAT_MOI_NGAY
// Settings-driven: SettingsManager::instance().getInt("TIEN_PHAT_MOI_NGAY", 5000)
```

#### 🔒 **Kiểm Tra Quyền Mượn**
```cpp
void CapNhatQuyenMuon(PTRDG docGia)
// Kiểm tra:
// 1. Trạng thái thẻ (Admin lock)
// 2. Ngày hết hạn
// 3. Tổng nợ (configurable buffer)
// 4. Sách quá hạn
// → Update docGia->data.coTheMuon
```

### 3.3. Business Rules (Nghiệp vụ)

#### 📌 **Mượn Sách**
1. Kiểm tra quyền mượn (realtime update)
2. Kiểm tra số lượng tối đa (configurable: 3 cuốn)
3. Kiểm tra quá hạn cache (optimization)
4. Smart borrow: Input có thể là ISBN (auto-pick) hoặc mã sách cụ thể
5. Transactional: BEGIN → Update DB → COMMIT
6. Event Bus: Emit `sachMuon` để UI cập nhật

#### 📌 **Trả Sách**
1. Tính phí phạt nếu quá hạn
2. Update trạng thái bản sao: DANG_MUON → CHO_MUON_DUOC
3. Update số sách đang mượn
4. Sync DB
5. Event Bus: Emit `sachTra`

#### 📌 **Thanh Toán Phí Phạt**
```cpp
std::string ThanhToanPhat(PTRDG docGia, const std::string& maSach)
// State transitions:
// GD_MAT_CHO_THANH_TOAN → GD_MAT_DA_THANH_TOAN
// GD_MAT_CHO_HOAN_TIEN → GD_MAT_DA_HOAN_TIEN
```

#### 📌 **Gia Hạn Sách**
```cpp
std::string GiaHanSach(PTRDG docGia, const std::string& maSach)
// Kiểm tra số lần gia hạn tối đa (configurable: 1 lần)
// Update ngày mượn (fake extend)
// Prevent abuse: không gia hạn nếu quá hạn
```

### 3.4. Đánh Giá Business Logic

#### ✅ Điểm Mạnh:
1. **Rõ ràng:** Logic nghiệp vụ phân tách rõ ràng
2. **Flexible:** Settings-driven (không hardcode giá trị)
3. **Safe:** Transaction-based, rollback khi lỗi
4. **Extensible:** FSM cho phép mở rộng trạng thái
5. **Optimized:** Cache quyền mượn, ngày hết hạn
6. **Smart Borrow:** Tự động chọn bản sao hoặc theo yêu cầu

#### ⚠️ Điểm Yếu:
1. **Redundancy:** Dữ liệu lưu cả memory (vector/tree) và DB
2. **Sync risk:** Memory và DB có thể desync nếu không cẩn thận
3. **Complexity O(M):** Duyệt linked list bản sao chậm nếu M lớn
4. **No async:** Mọi thao tác đều blocking (không dùng QConcurrent)
5. **Global variables:** `dsDauSach`, `rootDocGia` → Khó test

---

## 4. PHÂN TÍCH UI/UX

### 4.1. Kiến Trúc Giao Diện

**Framework:** Qt Widgets (không dùng QML)  
**Pattern:** Stacked Widget + Sidebar Navigation

#### 🏠 **CuaSoChinh (Main Window)**
```
┌──────────────────────────────────────┐
│ [Sidebar]  │  [Stacked Content]      │
│            │                          │
│ Dashboard  │  🏠 Dashboard            │
│ Sách       │  📚 Quản lý sách         │
│ Độc giả   │  👥 Quản lý độc giả      │
│ Mượn/Trả   │  📋 Mượn/Trả sách       │
│ Người dùng │  ⚙️ Admin panel          │
│            │                          │
└──────────────────────────────────────┘
```

**Responsive:**
- Tự động phát hiện kích thước màn hình (10"-17")
- Sidebar có thể thu gọn (280px ↔ 80px)
- Font scaling tự động theo DPI
- Lưu/khôi phục kích thước cửa sổ

#### 🎨 **Themes (QSS Stylesheets)**

3 theme variants:
1. **styles.qss** - Material Modern (màu xanh chủ đạo #2563EB)
2. **modern-dashboard.qss** - Enterprise theme
3. **enterprise-theme.qss** - Professional theme

**Đặc điểm:**
- Material Design principles
- Gradient backgrounds
- Hover effects, transitions
- Icon integration
- Consistent spacing (8px grid)

### 4.2. Các Widget Chính

#### 📊 **BangDieuKhien (Dashboard)**
```
┌─────────────────────────────────────────┐
│ Header: Search + Hello User            │
├─────────────────────────────────────────┤
│ 📈 Stats Cards (4 cards):              │
│ - Tổng sách | Độc giả | Đang mượn | Quá hạn │
├─────────────────────────────────────────┤
│ [Activity Log]     │ [Cảnh báo]         │
│ Real-time activity │ Sách quá hạn       │
│ Log from EventBus  │ Độc giả có nợ      │
└─────────────────────────────────────────┘
```

**Tính năng:**
- Real-time updates từ EventBus
- Debounced refresh (700ms)
- Quick actions (8 buttons: Quản lý sách, Mượn/Trả, In thẻ...)
- Activity table với color-coded actions

#### 📚 **WidgetQuanLySach (Book Management)**
```
┌─────────────────────────────────────────┐
│ 🔍 Search bar + Filters                │
├─────────────────────────────────────────┤
│ [Table: Danh sách sách]                │
│ ISBN | Tên | Tác giả | Bản sao | ...   │
├─────────────────────────────────────────┤
│ [Thêm] [Sửa] [Xóa] [Import Excel]     │
└─────────────────────────────────────────┘
```

**Tính năng:**
- Tìm kiếm realtime (debounced)
- Filter theo thể loại
- Import/Export Excel (ImportManager)
- Chi tiết sách (ManHinhChiTietSach dialog)
- Thêm nhiều bản sao cùng lúc

#### 👥 **WidgetQuanLyDocGia (Reader Management)**
```
┌─────────────────────────────────────────┐
│ 🔍 Search (Mã thẻ, Tên, Lớp...)       │
├─────────────────────────────────────────┤
│ [Table: Danh sách độc giả]             │
│ Mã thẻ | Họ tên | Lớp | Trạng thái...  │
├─────────────────────────────────────────┤
│ [Thêm] [Sửa] [Khóa/Mở] [In thẻ]       │
└─────────────────────────────────────────┘
```

**Tính năng:**
- AVL tree traversal để hiển thị
- Khóa/Mở thẻ realtime
- In thẻ hàng loạt (PrintQueueManager)
- Chi tiết độc giả + Lịch sử mượn
- Import độc giả từ Excel

#### 📋 **WidgetMuonTraSach (Circulation)**
```
┌─────────────────────────────────────────┐
│ 🎯 Mượn sách | Trả sách | Gia hạn     │
├─────────────────────────────────────────┤
│ 🔍 Mã thẻ: [__________] [Quét]        │
│ 📚 Mã sách/ISBN: [__________] [Quét]  │
├─────────────────────────────────────────┤
│ [Thông tin độc giả]                    │
│ - Sách đang mượn: 2/3                  │
│ - Tổng nợ: 15,000 VNĐ                 │
├─────────────────────────────────────────┤
│ [Lịch sử mượn trả]                     │
└─────────────────────────────────────────┘
```

**Tính năng:**
- Barcode scanner integration (GlobalScannerFilter - WIP)
- Smart borrow (auto-pick hoặc cụ thể)
- Âm thanh feedback (QSound)
- Real-time validation
- Hiển thị nợ, ngày hết hạn
- Thanh toán phí phạt inline

### 4.3. Dialogs Chuyên Dụng

| Dialog | Mục đích |
|--------|----------|
| **HopThoaiThemSach** | Thêm sách mới (wizard-style) |
| **HopThoaiSuaSach** | Sửa thông tin sách |
| **HopThoaiThemDocGia** | Thêm độc giả (with validation) |
| **ManHinhChiTietSach** | Chi tiết sách + Danh sách bản sao |
| **ManHinhChiTietDocGia** | Chi tiết độc giả + Lịch sử đầy đủ |
| **HopThoaiThongKe** | Thống kê toàn diện (charts) |
| **HopThoaiTop10** | Top 10 sách/độc giả |
| **ManHinhKiemKe** | Kiểm kê kho (barcode scanning) |
| **ManHinhQuanLyIn** | In thẻ/tem nhãn hàng loạt |
| **ManHinhDangNhap** | Login screen (Phase 3) |

### 4.4. UX Features

#### ✨ **Animation & Transitions**
- `AnimationHelper`: Fade in/out, slide animations
- Page transitions: Smooth fade (300ms)
- Toast notifications (auto-dismiss)

#### 🔊 **Audio Feedback**
- Beep khi quét thành công
- Warning sound khi lỗi
- Configurable (có thể tắt)

#### 📱 **Responsive Design**
- `ResponsiveHelper`: Phát hiện màn hình 10"-17"
- Auto font scaling: 1.0x (small) → 1.3x (large)
- Sidebar collapsible: 280px → 80px
- Grid layout auto-reflow

#### 🌐 **Đa Ngôn Ngữ**
- `LanguageManager`: Vi, Khmer
- Font Siemreap cho tiếng Khmer
- Language switching realtime (WIP)

#### 🔍 **Search & Filter**
- Debounced search (500ms)
- Multi-field search (tên, tác giả, ISBN...)
- Filter theo thể loại, trạng thái
- Highlight results

#### 📋 **Copy/Paste**
- `ClipboardHelper`: Global Ctrl+C support
- Copy table rows, ISBN, thông tin độc giả

### 4.5. Đánh Giá UI/UX

#### ✅ Điểm Mạnh:
1. **Modern:** Material Design, gradient, icons
2. **Responsive:** Tự động điều chỉnh theo màn hình
3. **Intuitive:** Navigation rõ ràng, sidebar + stacked
4. **Fast:** Debounced search, lazy loading
5. **Accessible:** Font scaling, high contrast
6. **Professional:** 3 themes, consistent styling
7. **Real-time:** EventBus updates UI ngay lập tức

#### ⚠️ Điểm Yếu:
1. **Qt Widgets limitations:**
   - Không smooth như QML
   - Animation hạn chế
   - Touch support kém
2. **Scanner integration chưa hoàn thiện:**
   - GlobalScannerFilter có issues
   - Phải dùng widget-level event filter
3. **No dark mode toggle:** Phải restart để đổi theme
4. **Heavy widgets:** Tất cả widget load cùng lúc (không lazy)
5. **No virtualization:** QTableWidget load tất cả rows (chậm nếu >10k)
6. **Search highlighting:** Không có
7. **No keyboard shortcuts:** Ít phím tắt (chỉ có Ctrl+C)

---

## 5. ĐÁNH GIÁ KIẾN TRÚC & CODE QUALITY

### 5.1. Design Patterns Đã Sử Dụng

#### 🏗️ **Architectural Patterns**

1. **MVC (Model-View-Controller):**
   - **Model:** `models/` (DauSach, DocGia, MuonTra)
   - **View:** `ui/` (widgets, dialogs)
   - **Controller:** `ui/*.cpp` (event handlers)

2. **Singleton:**
   - `DatabaseManager::instance()`
   - `EventBus::instance()`
   - `UserManager::instance()`
   - `SettingsManager::instance()`
   - All singletons thread-safe (QMutex)

3. **Observer Pattern:**
   - `EventBus` - Pub/Sub system
   - Widgets subscribe to events (sachMuon, sachTra, docGiaAdded...)
   - Real-time updates without tight coupling

4. **Factory Pattern:**
   - `taoDocGia()` - Tạo độc giả mới
   - `taoTheThongKe()` - Tạo stat cards

5. **Strategy Pattern:**
   - `PrintHelper` - Multiple print strategies (card, label, PDF)
   - `ImportManager` - Multiple import formats

6. **State Machine:**
   - `TrangThaiGiaoDich` - 6-state FSM cho transactions
   - Transitions: DANG_MUON → DA_TRA → MAT_CHO_THANH_TOAN...

### 5.2. Code Organization

#### 📁 **Separation of Concerns**
```
✅ Good:
- Models tách biệt khỏi UI
- Utils reusable (NgayThang, XuLyChuoi...)
- Business logic trong models/, UI logic trong ui/

⚠️ Issues:
- Global variables (dsDauSach, rootDocGia) trong main.cpp
- Một số logic nghiệp vụ rơi vào UI (validation trong dialogs)
```

#### 🧩 **Modularity**
```
✅ Good:
- Mỗi widget là 1 file riêng
- Utils có thể tái sử dụng
- Header/source tách biệt

⚠️ Issues:
- Một số file quá lớn (QuanLySach.cpp: 1723 lines)
- DocGia.cpp: 1593 lines
- Nên refactor thành nhiều file nhỏ hơn
```

#### 🔗 **Dependencies**
```
✅ Good:
- Qt dependencies rõ ràng
- Include guards đầy đủ
- Forward declarations để giảm compile time

⚠️ Issues:
- Circular dependencies (DocGia ↔ MuonTra)
- Global variables tạo tight coupling
```

### 5.3. Code Quality Metrics

#### 📊 **Complexity Analysis**

| File | Lines | Functions | Avg Complexity | Max Complexity |
|------|-------|-----------|----------------|----------------|
| QuanLySach.cpp | 1723 | ~50 | Moderate | High (nested loops) |
| DocGia.cpp | 1593 | ~40 | Moderate | High (AVL tree) |
| MuonTra.cpp | 789 | ~20 | Low | Moderate |
| CuaSoChinh.cpp | 977 | ~30 | Low | Low |
| BangDieuKhien.cpp | 1978 | ~40 | Moderate | Moderate |

**Cyclomatic Complexity:**
- Hầu hết functions: 1-5 (Simple)
- Một số functions: 6-10 (Moderate)
- Một vài hot spots: >10 (Complex - cần refactor)

#### 🧪 **Test Coverage**

```
tests/
├── tst_DocGia.h       - AVL tree tests
├── tst_MuonTra.h      - Transaction tests
├── tst_NgayThang.h    - Date utility tests
└── tst_Perf.h         - Performance tests

❌ Coverage: ~30% (ước tính)
❌ Chưa có CI/CD integration
❌ Chưa test UI widgets
```

#### 🐛 **Error Handling**

```cpp
✅ Good:
- Exception handling trong hot paths
- Input validation đầy đủ
- Defensive programming (null checks, bounds)

⚠️ Issues:
- Một số functions trả về string error thay vì exception
- Không có custom exception classes
- Log level không consistent (cout, qDebug, qWarning mix)
```

#### 🔒 **Security**

```
✅ Good:
- Password hashing (SHA256 + Salt)
- SQL injection prevention (prepared statements)
- Input sanitization

⚠️ Issues:
- Salt cố định ("somesalt") - should be random per user
- No brute-force protection (login attempts)
- No encryption for sensitive data in DB
```

#### 🚀 **Performance**

```
✅ Optimizations:
- AVL tree: O(log N) search
- Binary search on sorted vector
- Cached permissions (coTheMuon)
- Cached earliest due date
- Vector + LinkedList hybrid (Phase 5)

⚠️ Bottlenecks:
- O(M) linked list traversal (bản sao)
- No database indexes
- Full table scan cho search
- UI blocking operations (no async)
- Load all rows vào QTableWidget
```

### 5.4. Memory Management

#### 🧠 **Smart Pointers**

```cpp
✅ Modern C++:
- std::shared_ptr<DauSach> (PTRDS)
- std::shared_ptr<DanhMucSach> (PTRDMS)
- std::shared_ptr<NodeDG> (PTRDG)
- No manual delete needed

⚠️ Risks:
- Circular references trong linked lists
- Potential memory leaks nếu không cẩn thận
- Global shared_ptr containers (dsDauSach)
```

#### 💾 **Memory Usage**

```
Estimated for 10,000 books + 5,000 readers:
- DauSach: ~10MB (1KB per book)
- DocGia AVL: ~5MB (1KB per reader)
- UI Widgets: ~50MB
Total: ~65-100MB (lightweight)

✅ SQLite database: ~10-50MB (depending on transactions)
```

### 5.5. Maintainability

#### 📝 **Documentation**

```
✅ Good:
- File headers với @file, @path, @description
- Doxygen-style comments
- README.md đầy đủ

⚠️ Issues:
- Inline comments ít (especially trong complex algorithms)
- Không có API documentation
- Không có user manual
```

#### 🔄 **Refactoring Potential**

**High Priority:**
1. Tách QuanLySach.cpp thành nhiều file
2. Remove global variables
3. Dependency injection cho singletons
4. Extract business logic từ UI

**Medium Priority:**
5. Add more unit tests
6. Implement async operations
7. Add database indexes
8. Refactor linked list → vector

**Low Priority:**
9. Migrate to QML (long-term)
10. Add more themes

---

## 6. PHÂN TÍCH UTILITIES & HELPERS

### 6.1. Core Utilities

#### 🗄️ **DatabaseManager**
- Singleton, thread-safe
- Transaction support
- Schema migration
- Seed data
- **Rating:** ⭐⭐⭐⭐ (Very good)

#### 📅 **NgayThang**
- Date conversion (string ↔ time_t)
- Validation (leap year, range)
- Format: YYYY-MM-DD
- **Rating:** ⭐⭐⭐⭐⭐ (Excellent)

#### 🔤 **XuLyChuoi**
- Unicode normalization (tiếng Việt)
- Case conversion
- Trim, title case
- **Rating:** ⭐⭐⭐⭐ (Good, Qt optimized)

#### 📡 **EventBus**
- Qt signals/slots wrapper
- Decouples UI components
- Real-time updates
- **Rating:** ⭐⭐⭐⭐⭐ (Essential pattern)

#### 🖨️ **PrintHelper**
- Print reader cards (QR code)
- Print book labels (barcode)
- Export PDF reports
- Configurable margins
- **Rating:** ⭐⭐⭐⭐ (Feature-rich)

#### 💾 **BackupManager**
- Manual backup (copy DB)
- Periodic backup (30 mins)
- Archive old backups (monthly zip)
- **Rating:** ⭐⭐⭐ (Basic, needs improvement)

#### 📝 **AuditLogger**
- Log all CRUD operations
- Track user actions
- Timestamp + details
- **Rating:** ⭐⭐⭐⭐ (Good for compliance)

#### ⚙️ **SettingsManager**
- Key-value store (cai_dat table)
- Cached settings
- Type-safe getInt/getString/getBool
- **Rating:** ⭐⭐⭐⭐ (Flexible)

#### 📊 **ImportManager**
- Import sách/độc giả từ Excel
- Batch insert (transaction)
- Validation + preview
- **Rating:** ⭐⭐⭐⭐ (Powerful)

#### 🎬 **AnimationHelper**
- Fade in/out
- Slide animations
- Smooth transitions
- **Rating:** ⭐⭐⭐ (Limited by Qt Widgets)

#### 📱 **ResponsiveHelper**
- Screen size detection
- Font scaling
- Window geometry save/restore
- **Rating:** ⭐⭐⭐⭐⭐ (Critical for UX)

#### 🔍 **SearchableListWidget**
- Real-time filter
- Debounced search
- Reusable component
- **Rating:** ⭐⭐⭐⭐ (Useful)

#### 🎯 **ToastNotification**
- Auto-dismiss messages
- Non-blocking
- Color-coded (success/error/warning)
- **Rating:** ⭐⭐⭐⭐ (Modern UX)

### 6.2. Đánh Giá Tổng Thể Utilities

#### ✅ Điểm Mạnh:
1. **Complete:** Bao phủ hầu hết nhu cầu
2. **Reusable:** Dễ tái sử dụng trong dự án khác
3. **Well-designed:** Singleton patterns, thread-safe
4. **Qt-optimized:** Leverage Qt classes (QSettings, QSqlDatabase...)

#### ⚠️ Điểm Yếu:
1. **No async:** Tất cả đều blocking
2. **Backup naive:** Copy toàn bộ file (slow for large DB)
3. **No retry logic:** Network/file errors không retry
4. **Limited error handling:** Some utils throw, some return string

---

## 7. ĐIỂM MẠNH

### 🏆 Tổng Kết Điểm Mạnh

1. **✅ Offline-First Architecture**
   - Không phụ thuộc internet
   - SQLite embedded database
   - Portable deployment

2. **✅ Modern C++ Practices**
   - C++17 features
   - Smart pointers (no manual memory)
   - STL containers (vector, map)

3. **✅ Robust Business Logic**
   - AVL tree (O(log N) search)
   - Finite State Machine (transactions)
   - Settings-driven rules
   - Transaction safety (ACID)

4. **✅ Professional UI/UX**
   - Material Design
   - Responsive layout
   - Real-time updates
   - Multiple themes

5. **✅ Complete Feature Set**
   - Mượn/Trả/Gia hạn
   - Kiểm kê kho
   - In ấn hàng loạt
   - Import/Export Excel
   - Audit log
   - User management

6. **✅ Internationalization**
   - Tiếng Việt + Khmer
   - Unicode support (UTF-8)
   - Custom fonts

7. **✅ Data Integrity**
   - Foreign keys
   - Cascade deletes
   - Input validation
   - Business rules enforcement

8. **✅ Security (Phase 3)**
   - Password hashing
   - Role-based access
   - SQL injection prevention

9. **✅ Maintainability**
   - Clean architecture
   - Separation of concerns
   - Documented code
   - Unit tests (partial)

10. **✅ Performance**
    - Cached permissions
    - Debounced UI updates
    - AVL tree indexing

---

## 8. ĐIỂM YẾU & RỦI RO

### ⚠️ Critical Issues

#### 🚨 **1. Global State Management**
```cpp
extern std::vector<std::shared_ptr<DauSach>> dsDauSach;
extern PTRDG rootDocGia;
extern bool duLieuDaThayDoi;
```
**Vấn đề:**
- Hard to test (mocking impossible)
- Thread-unsafe
- Tight coupling
- Desync risk with database

**Giải pháp:**
- Dependency injection
- Wrap trong Repository class
- Singleton repository với mutex

#### 🚨 **2. Database Indexing**
```sql
-- Missing indexes:
CREATE INDEX idx_isbn ON dau_sach(isbn);
CREATE INDEX idx_ma_sach ON ban_sao(ma_sach);
CREATE INDEX idx_ma_the ON doc_gia(ma_the);
CREATE INDEX idx_muontra_mathe ON muon_tra(ma_the);
```
**Impact:** Slow queries khi data > 10,000 rows

#### 🚨 **3. Blocking UI Operations**
```cpp
// All operations block main thread:
NapDanhSachTuDB(); // Có thể mất vài giây
MuonSach(...);      // DB I/O blocking
```
**Giải pháp:**
- QtConcurrent::run()
- QFuture + progress dialog
- Background worker threads

#### 🚨 **4. Security Weaknesses**
```cpp
QString salt = "somesalt"; // ❌ Fixed salt for all users
QString hash = QCryptographicHash::hash((password + salt).toUtf8(), QCryptographicHash::Sha256);
```
**Vấn đề:**
- Rainbow table attack possible
- No brute-force protection
- No password complexity rules

**Giải pháp:**
- Random salt per user
- bcrypt/Argon2 thay vì SHA256
- Login attempt throttling

#### 🚨 **5. Memory Leak Potential**
```cpp
struct DanhMucSach {
    std::shared_ptr<DanhMucSach> next; // Circular reference risk
};
```
**Giải pháp:**
- Use std::weak_ptr cho back pointers
- Hoặc migrate toàn bộ sang std::vector

### ⚠️ Major Issues

#### 6. **No Virtualization (UI Performance)**
- QTableWidget loads all rows
- Slow with >5,000 items
- **Fix:** QAbstractItemModel + custom view

#### 7. **Backup Strategy Naive**
- Copy entire DB file (slow)
- No incremental backup
- **Fix:** SQLite backup API, incremental, compression

#### 8. **No Rollback UI**
- User cannot undo actions
- **Fix:** Command pattern, undo stack

#### 9. **Scanner Integration Incomplete**
- GlobalScannerFilter has bugs
- Fall back to widget-level filter
- **Fix:** Proper event filter hierarchy

#### 10. **File Size Concerns**
- QuanLySach.cpp: 1723 lines
- DocGia.cpp: 1593 lines
- BangDieuKhien.cpp: 1978 lines
- **Fix:** Refactor into smaller modules

### 🔶 Minor Issues

11. No dark mode toggle (restart required)
12. Limited keyboard shortcuts
13. No search highlighting
14. No drag-and-drop
15. No multi-language UI switching (needs restart)
16. No crash reporting system
17. No automatic updates
18. No data export to CSV (only Excel)
19. No email notifications
20. No advanced filtering (date ranges, complex queries)

---

## 9. KHUYẾN NGHỊ CẢI THIỆN

### 🎯 Priority 1: Critical (Ngay lập tức)

#### 1. **Thêm Database Indexes**
```sql
CREATE INDEX idx_isbn ON dau_sach(isbn);
CREATE INDEX idx_ma_sach ON ban_sao(ma_sach);
CREATE INDEX idx_ma_the_docgia ON doc_gia(ma_the);
CREATE INDEX idx_muontra_mathe ON muon_tra(ma_the);
CREATE INDEX idx_muontra_bansao ON muon_tra(ban_sao_id);
CREATE INDEX idx_muontra_trangthai ON muon_tra(trang_thai);
```
**Impact:** Tăng tốc search 10-100x

#### 2. **Fix Security - Random Salt**
```cpp
QString generateRandomSalt(int length = 32) {
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    QString salt;
    for (int i = 0; i < length; ++i) {
        salt.append(chars.at(QRandomGenerator::global()->bounded(chars.length())));
    }
    return salt;
}
```

#### 3. **Remove Global Variables**
```cpp
// Current:
extern std::vector<PTRDS> dsDauSach;

// Refactor to:
class LibraryRepository {
public:
    static LibraryRepository& instance();
    std::vector<PTRDS> getDauSach();
    void addDauSach(PTRDS sach);
    // ... etc
private:
    std::vector<PTRDS> m_dsDauSach;
    QMutex m_mutex;
};
```

### 🎯 Priority 2: High (Trong 1-2 tuần)

#### 4. **Async Operations với QtConcurrent**
```cpp
QFuture<void> future = QtConcurrent::run([this]() {
    NapDanhSachTuDB();
});

QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
connect(watcher, &QFutureWatcher<void>::finished, [this]() {
    // Update UI
    capNhatThongKe();
});
watcher->setFuture(future);
```

#### 5. **UI Virtualization cho Large Tables**
```cpp
class BookTableModel : public QAbstractTableModel {
    // Only load visible rows
    QVariant data(const QModelIndex &index, int role) const override {
        // Load from DB on-demand
    }
};

QTableView* view = new QTableView;
view->setModel(new BookTableModel);
```

#### 6. **Incremental Backup**
```cpp
// Sử dụng SQLite Backup API
sqlite3* pSrc = ...; // Source DB
sqlite3* pDest = ...; // Destination
sqlite3_backup* pBackup = sqlite3_backup_init(pDest, "main", pSrc, "main");
sqlite3_backup_step(pBackup, -1); // Copy all pages
sqlite3_backup_finish(pBackup);
```

#### 7. **Refactor Large Files**
```
QuanLySach.cpp → 
    QuanLySach_Core.cpp (search, sort)
    QuanLySach_CRUD.cpp (add, delete, update)
    QuanLySach_Import.cpp (import/export)
```

### 🎯 Priority 3: Medium (Trong 1 tháng)

#### 8. **Add More Unit Tests**
```cpp
// Target: 70% coverage
tests/
├── tst_QuanLySach.h     (search, add, delete)
├── tst_MuonTra.h        (borrow, return, extend)
├── tst_Security.h       (login, permissions)
├── tst_Database.h       (CRUD, transactions)
└── tst_Integration.h    (end-to-end scenarios)
```

#### 9. **Command Pattern cho Undo/Redo**
```cpp
class Command {
public:
    virtual void execute() = 0;
    virtual void undo() = 0;
};

class AddBookCommand : public Command {
    void execute() override { /* Add book */ }
    void undo() override { /* Remove book */ }
};

QUndoStack undoStack;
undoStack.push(new AddBookCommand(...));
```

#### 10. **Scanner Integration Fix**
```cpp
// Proper event filter hierarchy
class BarcodeInputFilter : public QObject {
protected:
    bool eventFilter(QObject* obj, QEvent* event) override {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            // Accumulate barcode input
            // Emit signal when Enter pressed
        }
    }
};
```

#### 11. **Dark Mode Toggle**
```cpp
class ThemeManager : public QObject {
public:
    void setTheme(Theme theme) {
        QFile file(getThemePath(theme));
        file.open(QFile::ReadOnly);
        QString styleSheet = file.readAll();
        qApp->setStyleSheet(styleSheet);
        emit themeChanged(theme);
    }
};
```

### 🎯 Priority 4: Low (Nice to have)

12. **Migrate to QML** (long-term, for smoother UI)
13. **Add Charts Library** (QCustomPlot cho advanced charts)
14. **Add Email Notifications** (SMTP integration)
15. **Add Auto-Update** (check for new versions)
16. **Add Crash Reporter** (Breakpad/Sentry)
17. **Add CSV Export**
18. **Add Advanced Filtering** (date ranges, regex)
19. **Add Drag-and-Drop** (for file import)
20. **Add Touch Gestures** (for tablets)

---

## 10. KẾT LUẬN

### 📊 Tổng Quan Đánh Giá

| Khía Cạnh | Điểm (1-10) | Nhận Xét |
|-----------|-------------|----------|
| **Backend & Database** | 8/10 | Solid, cần indexes |
| **Business Logic** | 9/10 | Rất tốt, settings-driven |
| **UI/UX** | 8/10 | Modern, cần async ops |
| **Code Quality** | 7/10 | Good, cần refactor một số file lớn |
| **Security** | 6/10 | Basic, cần cải thiện salt/encryption |
| **Performance** | 7/10 | OK cho 10k records, cần index |
| **Maintainability** | 7/10 | Good docs, cần more tests |
| **Scalability** | 6/10 | OK cho thư viện nhỏ/trung bình |
| **Extensibility** | 8/10 | Plugin-friendly, settings-driven |
| **Deployment** | 9/10 | Portable, Inno Setup installer |

**Tổng điểm:** **7.5/10** - **Production-Ready với một số cải thiện cần thiết**

### ✅ Đánh Giá Chung

**Hệ thống Quản Lý Thư Viện V2.5** là một dự án **chất lượng cao**, được thiết kế tốt và triển khai đầy đủ tính năng cho môi trường thư viện offline. Dự án thể hiện:

1. **Kiến trúc vững chắc:** MVC, Observer, Singleton patterns
2. **Business logic rõ ràng:** FSM, settings-driven rules
3. **UI hiện đại:** Material Design, responsive, real-time updates
4. **Bảo mật cơ bản:** Login, roles, audit log
5. **Performance tốt:** AVL tree, caching, debouncing

### ⚠️ Rủi Ro Cần Lưu Ý

1. **Global state:** Khó test, thread-unsafe
2. **No indexes:** Chậm khi data lớn (>10k)
3. **Security gaps:** Fixed salt, no brute-force protection
4. **Blocking UI:** Tất cả I/O đều blocking
5. **Memory leak potential:** Circular refs trong linked lists

### 🎯 Lộ Trình Phát Triển Đề Xuất

#### Phase 1 (1 tuần): Critical Fixes
- ✅ Add database indexes
- ✅ Fix security (random salt)
- ✅ Remove global variables

#### Phase 2 (2 tuần): Performance
- ✅ Async operations (QtConcurrent)
- ✅ UI virtualization
- ✅ Incremental backup

#### Phase 3 (1 tháng): Quality
- ✅ Refactor large files
- ✅ Add unit tests (70% coverage)
- ✅ Command pattern (undo/redo)

#### Phase 4 (2-3 tháng): Features
- ✅ Dark mode toggle
- ✅ Scanner integration fix
- ✅ Advanced filtering
- ✅ Email notifications

### 💡 Khuyến Nghị Cuối Cùng

**Dự án này đã sẵn sàng để triển khai (production-ready)**, nhưng nên thực hiện các cải tiến Priority 1 & 2 để đảm bảo:
- **Hiệu suất tốt** khi data lớn (indexes)
- **Bảo mật đủ** cho môi trường thực tế (random salt)
- **UX mượt mà** (async operations)

**Điểm nổi bật:** Dự án thể hiện kỹ năng lập trình C++/Qt tốt, hiểu rõ nghiệp vụ thư viện, và quan tâm đến UX. Với một số cải thiện nhỏ, đây sẽ là một sản phẩm **xuất sắc**.

---

## 📞 THÔNG TIN LIÊN HỆ

**Tác giả:** Chăm Rốch Thi  
**Email:** [Contact via GitHub]  
**Địa chỉ:** Cần Thơ, Việt Nam  
**Giấy phép:** © 2025. Giữ toàn quyền.

---

**Ngày hoàn thành báo cáo:** 23 Tháng 12, 2025  
**Phiên bản báo cáo:** 1.0  
**Công cụ phân tích:** AI-powered Code Review System  

---

*Báo cáo này được tạo tự động dựa trên phân tích mã nguồn và kiến trúc dự án. Mọi phản hồi và đóng góp đều được hoan nghênh.*
