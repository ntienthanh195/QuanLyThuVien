# 🎨 UX REDESIGN PLAN - Gom Chức Năng Liên Quan

## 📋 Mục Tiêu
**Tận dụng 100% backend có sẵn**, gom tất cả chức năng liên quan vào 1 trang để UX mượt mà, giảm navigation phức tạp.

---

## 🏗️ Kiến Trúc Backend Hiện Tại

### Module 1: QuanLySach.h (8 Nhóm - 114 Dòng)
```cpp
// NHÓM 1: Sinh mã & Chuyển đổi trạng thái
sinhMaSach(), TenTrangThai(), PhanTichTrangThaiSach()

// NHÓM 2: Quản lý Danh Mục Sách (DSLK đơn)
themDanhMucSach(), capNhatTrangThaiSach(), timDanhMucTheoMaSach(), 
ThemNhieuBanSao()

// NHÓM 3: Quản lý Đầu Sách (Mảng con trỏ)
themDauSach(), TimDauSachTheoISBN()

// NHÓM 4: Tìm kiếm & Liệt kê
timKiemLogic(), TimTheLoaiDuyNhat(), TimSachTheoTheLoai(), 
DemSoLuongBanSaoTheoTrangThai(), LayThongTinDauSach()

// NHÓM 5: Sắp xếp
sapXepDauSachTheoTen(), SapXepBanSaoTheoMa(), sapXepKetQuaTimKiem()

// NHÓM 6: Quản lý bộ nhớ
GiaiPhongDanhMucSach(), GiaiPhongDauSach()

// NHÓM 7: Xóa & Cập nhật
XoaDauSach(), XoaSachTheoMaSach(), CapNhatDauSach(), 
CapNhatViTriTatCaBanSao(), KiemTraCoTheXoaDauSach()

// NHÓM 8: SERVICE LAYER (Encapsulation)
LayDanhSachDauSach(), LaySoLuongDauSach(), LayDanhSachTimKiem(),
LayDanhSachTheoTheLoai(), ThemDauSachMoi(), ThemBanSaoMoi(),
CapNhatThongTinDauSach(), XoaDauSachTheoISBN(), ThanhLyBanSaoTheoMa()
```

### Module 2: DocGia.h (AVL Tree)
```cpp
// Cây AVL
taoDocGia(), xoaDocGia(), timDocGia(), hieuChinhDocGia()

// Sắp xếp
QuickSortDocGia(), DuyetCayRaMang(), InDocGiaTheoTenHo()

// Mượn Trả (Tích hợp với QuanLySach)
MuonSach(), TraSach(), BaoMatSach(), themMuonTra(),
TimBanSaoCoTheMuon(), DemBanSaoCoTheMuon()

// Thống kê
LayDSSachDangMuon(), TinhSoNgayQuaHanLonNhat(), 
LayDSDocGiaQuaHan(), LayTopSach()

// File I/O
saveDocGia(), loadDocGia()
```

### Module 3: ThaoTacFile.h
```cpp
NapDanhSachDauSach(), NapDanhMucSach(), 
GhiDanhSachDauSach(), GhiDanhMucSach()
```

### Module 4: KiemTraDuLieu.h (Validation Engine)
```cpp
KiemTraChuoiRong(), KiemTraTrungISBN(), KiemTraISBN(),
KiemTraNamXuatBan(), KiemTraTongSoBanSao()
```

---

## 🎯 UX REDESIGN - 4 Trang Chính

### ✅ 1. DASHBOARD (Tổng Quan)
**Mục đích**: Overview toàn hệ thống

**Layout**:
```
┌────────────────────────────────────────────────┐
│  📊 THỐNG KÊ TỔNG QUAN                         │
├─────────────┬─────────────┬─────────────┬──────┤
│ 📚 Tổng Sách│ 👥 Độc Giả  │ 📖 Đang Mượn│ ⚠️ QH│
│    12,480   │    3,256    │     1,842   │  15  │
└─────────────┴─────────────┴─────────────┴──────┘

┌────────────────────────────────────────────────┐
│  ⚠️ CẢNH BÁO                                   │
│  • 15 sách quá hạn (> 7 ngày)                  │
│  • 8 thẻ độc giả sắp hết hạn                   │
└────────────────────────────────────────────────┘

┌────────────────────────────────────────────────┐
│  🏆 TOP 10 SÁCH ĐƯỢC MƯỢN NHIỀU NHẤT           │
│  (Dùng: LayTopSach())                          │
└────────────────────────────────────────────────┘

┌────────────────────────────────────────────────┐
│  🚀 QUICK ACTIONS                              │
│  [➕ Thêm Sách] [➕ Thêm Độc Giả] [📖 Mượn]   │
└────────────────────────────────────────────────┘
```

**Backend Functions**:
- `LaySoLuongDauSach()` → Tổng sách
- `DemTongSoBanSao()` → Tổng bản sao
- `LayDSDocGiaQuaHan()` → Độc giả quá hạn
- `LayTopSach()` → Top 10

---

### ✅ 2. QUẢN LÝ SÁCH (All-in-One với Tabs)

#### Tab 1: "📚 Danh Sách Sách"
```
┌────────────────────────────────────────────────┐
│ 🔍 [Tìm kiếm...] [Thể loại ▼] [Trạng thái ▼] │
│ [➕ Thêm Sách] [Import CSV] [Export CSV]      │
├────────────────────────────────────────────────┤
│  Table: ISBN | Tên | Tác giả | Thể loại |...  │
│  (Dùng: LayDanhSachDauSach())                  │
│  (Filter: LayDanhSachSachTheoTheLoai())        │
│  (Search: LayDanhSachTimKiem())                │
└────────────────────────────────────────────────┘
```

**Backend Functions**:
- `LayDanhSachDauSach()` → Load toàn bộ
- `LayDanhSachTimKiem()` → Search multi-field
- `LayDanhSachTheoTheLoai()` → Filter dropdown
- `LayDanhSachSachTheoTheLoai()` → Filter results

#### Tab 2: "🔍 Chi Tiết Sách"
**Hiện khi double-click row trong table**

```
┌────────────────────────────────────────────────┐
│  📖 THÔNG TIN ĐẦU SÁCH                         │
│  ISBN: 9786040000001                           │
│  Tên: Lập Trình C++                            │
│  Tác giả: Nguyễn Văn A                         │
│  (Dùng: TimDauSach())                          │
├────────────────────────────────────────────────┤
│  📦 DANH SÁCH BẢN SAO                          │
│  (Dùng: LayDanhSachBanSaoSapXep())             │
│  ┌──────────────────────────────────────────┐ │
│  │ Mã Sách  │ Trạng Thái │ Vị Trí          │ │
│  │ 001-1    │ ✓ Có sẵn   │ Kệ A1           │ │
│  │ 001-2    │ 📖 Đang mượn│ Kệ A1          │ │
│  └──────────────────────────────────────────┘ │
│  [➕ Thêm Bản Sao] [🗑️ Thanh Lý]             │
└────────────────────────────────────────────────┘
```

**Backend Functions**:
- `TimDauSach()` → Chi tiết đầu sách
- `LayThongTinDauSach()` → Thống kê (tongBanSao, choMuonDuoc, dangMuon)
- `LayDanhSachBanSaoSapXep()` → Danh sách bản sao sorted
- `DemBanSaoCoTheMuon()` → Số bản sao có thể mượn

#### Tab 3: "✏️ Thêm/Sửa/Xóa"
**Form thao tác với validation**

```
┌────────────────────────────────────────────────┐
│  ➕ THÊM ĐẦU SÁCH MỚI                          │
│  ISBN:      [_________________] (13 số)        │
│  Tên sách:  [_________________]                │
│  Tác giả:   [_________________]                │
│  Số trang:  [____] (1-9999)                    │
│  Năm XB:    [____] (validation)                │
│  Thể loại:  [_________________]                │
│  Số bản sao:[____] (1-999)                     │
│  Vị trí:    [_________________]                │
│                                                 │
│  [✓ Thêm Sách] [❌ Hủy]                        │
└────────────────────────────────────────────────┘

Validation Engine (KiemTraDuLieu.h):
- KiemTraISBN() → Check format + checksum
- KiemTraTrungISBN() → Check duplicate
- KiemTraNamXuatBan() → Validate year
- KiemTraTongSoBanSao() → Check limit
```

**Backend Functions**:
- `ThemDauSachMoi()` → Add book + copies (all-in-one)
- `ThemBanSaoMoi()` → Add copies to existing book
- `CapNhatThongTinDauSach()` → Edit book info
- `XoaDauSachTheoISBN()` → Delete (with validation)
- `ThanhLyBanSaoTheoMa()` → Liquidate copy

#### Tab 4: "📊 Thống Kê Sách"
```
┌────────────────────────────────────────────────┐
│  📊 THỐNG KÊ THEO THỂ LOẠI                     │
│  [Lập Trình C++  ] → 245 sách                  │
│  [Cơ Sở Dữ Liệu  ] → 189 sách                  │
│  [Mạng Máy Tính  ] → 167 sách                  │
│  (Dùng: LayDanhSachTheoTheLoai())              │
├────────────────────────────────────────────────┤
│  📍 THỐNG KÊ THEO VỊ TRÍ                       │
│  [Kệ A1] → 340 cuốn                            │
│  [Kệ B2] → 298 cuốn                            │
│  (Dùng: LayDanhSachViTri())                    │
└────────────────────────────────────────────────┘
```

**Backend Functions**:
- `LayDanhSachTheoTheLoai()` → Lấy tất cả thể loại unique
- `LayDanhSachSachTheoTheLoai()` → Đếm sách theo thể loại
- `LayDanhSachViTri()` → Lấy tất cả vị trí unique

---

### ✅ 3. QUẢN LÝ ĐỘC GIẢ (All-in-One với Tabs)

#### Tab 1: "👥 Danh Sách Độc Giả"
```
┌────────────────────────────────────────────────┐
│ 🔍 [Tìm theo mã thẻ/tên...] [Giới tính ▼]    │
│ [➕ Thêm Độc Giả] [📊 Sắp xếp: Tên ▼]         │
├────────────────────────────────────────────────┤
│  Card List View (Modern UI):                   │
│  ┌──────────────────────────────────────────┐ │
│  │ 👤 001 - Nguyễn Văn A     [Nam] [Hoạt động]│
│  │    Đang mượn: 2 sách                      │ │
│  └──────────────────────────────────────────┘ │
│  (Dùng: DuyetCayRaMang() → QuickSortDocGia()) │
└────────────────────────────────────────────────┘
```

**Backend Functions**:
- `DuyetCayRaMang()` → Convert AVL tree to array
- `QuickSortDocGia()` → Sort by Tên+Họ
- `timDocGia()` → Search by mã thẻ (O(log n))

#### Tab 2: "🔍 Chi Tiết Độc Giả"
```
┌────────────────────────────────────────────────┐
│  👤 THÔNG TIN ĐỘC GIẢ                          │
│  Mã thẻ: 001                                   │
│  Họ tên: Nguyễn Văn A                          │
│  Giới tính: Nam                                │
│  Trạng thái: ✓ Hoạt động                       │
│  Sách đang mượn: 2/3                           │
│  (Dùng: timDocGia())                           │
├────────────────────────────────────────────────┤
│  📚 SÁCH ĐANG MƯỢN                             │
│  (Dùng: LayDSSachDangMuon())                   │
│  ┌──────────────────────────────────────────┐ │
│  │ Mã Sách │ Tên Sách      │ Ngày Mượn |Ngày│ │
│  │ 001-1   │ Lập Trình C++ │ 01/12/25  │ 5  │ │
│  └──────────────────────────────────────────┘ │
│  [✏️ Sửa] [🔒 Khóa Thẻ] [📖 Mượn Sách]       │
└────────────────────────────────────────────────┘
```

**Backend Functions**:
- `timDocGia()` → Get reader info
- `LayDSSachDangMuon()` → List borrowed books with details
- `hieuChinhDocGia()` → Edit reader info
- Update `TrangThai` field → Lock/Unlock card

#### Tab 3: "⚠️ Độc Giả Quá Hạn"
```
┌────────────────────────────────────────────────┐
│  ⚠️ DANH SÁCH ĐỘC GIẢ QUÁ HẠN                 │
│  (Dùng: LayDSDocGiaQuaHan())                   │
│  ┌──────────────────────────────────────────┐ │
│  │ Mã  │ Họ Tên        │ Ngày QH Nhiều Nhất │ │
│  │ 001 │ Nguyễn Văn A  │ 15 ngày            │ │
│  │ 042 │ Trần Thị B    │ 8 ngày             │ │
│  └──────────────────────────────────────────┘ │
│  [🔔 Gửi Thông Báo] [📧 Export]               │
└────────────────────────────────────────────────┘
```

**Backend Functions**:
- `LayDSDocGiaQuaHan()` → Get overdue readers
- `TinhSoNgayQuaHanLonNhat()` → Calculate max overdue days

---

### ✅ 4. MƯỢN TRẢ SÁCH (1 Trang - Workflow)

**Layout duy nhất, thay đổi state theo action**:

```
┌────────────────────────────────────────────────┐
│  📖 MƯỢN - TRẢ SÁCH                            │
├────────────────────────────────────────────────┤
│  BƯỚC 1: NHẬP MÃ THẺ ĐỘC GIẢ                   │
│  Mã thẻ: [_____] [🔍 Tìm]                     │
├────────────────────────────────────────────────┤
│  BƯỚC 2: THÔNG TIN ĐỘC GIẢ (Auto-fill)        │
│  Họ tên: Nguyễn Văn A                          │
│  Sách đang mượn: 2/3                           │
│  Trạng thái: ✓ Hoạt động                       │
│                                                 │
│  📚 Sách đang mượn:                            │
│  ┌──────────────────────────────────────────┐ │
│  │ ☑ 001-1 │ Lập Trình C++ │ 01/12 │ 5 ngày│ │
│  │ ☐ 002-3 │ Cơ Sở Dữ Liệu │ 28/11 │ 8 ngày│ │
│  └──────────────────────────────────────────┘ │
├────────────────────────────────────────────────┤
│  BƯỚC 3: CHỌN HÀNH ĐỘNG                        │
│  [📖 Mượn Sách Mới] [✓ Trả Sách] [⚠️ Báo Mất] │
└────────────────────────────────────────────────┘

--- Khi chọn "Mượn Sách Mới" ---
┌────────────────────────────────────────────────┐
│  🔍 TÌM SÁCH ĐỂ MƯỢN                           │
│  [Tìm theo ISBN/Tên...] [Tìm]                 │
│  (Dùng: LayDanhSachTimKiem())                  │
│                                                 │
│  Kết quả: [Lập Trình C++] - 3 bản sao sẵn     │
│  (Dùng: DemBanSaoCoTheMuon())                  │
│  Chọn bản sao: [001-3 ▼] (Kệ A1)              │
│  (Dùng: TimBanSaoCoTheMuon())                  │
│                                                 │
│  [✓ Xác Nhận Mượn] [❌ Hủy]                    │
│  (Call: MuonSach())                            │
└────────────────────────────────────────────────┘
```

**Backend Functions (Workflow Integration)**:
1. **Tìm độc giả**: `timDocGia()` → O(log n) AVL search
2. **Hiện sách đang mượn**: `LayDSSachDangMuon()`
3. **Tìm sách để mượn**: `LayDanhSachTimKiem()`
4. **Check số lượng sẵn**: `DemBanSaoCoTheMuon()`
5. **Chọn bản sao**: `TimBanSaoCoTheMuon()`
6. **Xác nhận mượn**: `MuonSach()` → Auto update both DocGia & DauSach
7. **Trả sách**: `TraSach()` → Update PTRDMS status
8. **Báo mất**: `BaoMatSach()` → Update status = 2

**Validation**:
- Check `soSachDangMuon < 3` (Max 3 books)
- Check `TrangThai == 1` (Card active)
- Check book availability

---

## 🔧 Technical Implementation

### Widget Structure
```cpp
// 1. Dashboard
WidgetBangDieuKhien
  - Card thống kê
  - Table cảnh báo
  - Top 10 chart

// 2. Quản Lý Sách (QTabWidget)
WidgetQuanLySach
  ├─ Tab 0: DanhSachSachWidget (table + filter)
  ├─ Tab 1: ChiTietSachWidget (details + copies)
  ├─ Tab 2: ThemSuaXoaSachWidget (form + validation)
  └─ Tab 3: ThongKeSachWidget (charts)

// 3. Quản Lý Độc Giả (QTabWidget)
WidgetQuanLyDocGia
  ├─ Tab 0: DanhSachDocGiaWidget (card list + search)
  ├─ Tab 1: ChiTietDocGiaWidget (info + borrowed books)
  └─ Tab 2: DocGiaQuaHanWidget (overdue list)

// 4. Mượn Trả (State Machine)
WidgetMuonTraSach
  - State 0: Nhập mã thẻ
  - State 1: Hiện thông tin + actions
  - State 2a: Mượn sách (search → select → confirm)
  - State 2b: Trả sách (select → confirm)
  - State 2c: Báo mất (confirm)
```

### Backend Integration
```cpp
// Service Layer Pattern
QuanLySach.h → NHÓM 8 (Service Layer)
  ├─ LayDanhSachDauSach() → Read-only access
  ├─ LayDanhSachTimKiem() → Search with priority
  ├─ ThemDauSachMoi() → All-in-one add
  └─ ... (No direct access to dsDauSach!)

DocGia.h → AVL Operations
  ├─ timDocGia() → O(log n) search
  ├─ MuonSach() → Atomic transaction
  └─ LayDSSachDangMuon() → DTO pattern

ThaoTacFile.h → Persistence
  ├─ NapDanhSachDauSach() → Startup
  └─ GhiDanhSachDauSach() → Shutdown

KiemTraDuLieu.h → Validation
  ├─ KiemTraISBN() → Client-side
  └─ KiemTraTrungISBN() → Server-side
```

---

## 📊 So Sánh: Cũ vs Mới

### ❌ Cũ (Phân Tán)
```
Quản Lý Sách
  ├─ Widget A: Danh sách
  ├─ Dialog B: Thêm sách
  ├─ Dialog C: Chi tiết sách
  ├─ Dialog D: Thêm bản sao
  ├─ Dialog E: Thanh lý
  └─ Dialog F: Import/Export

→ User phải mở 6 màn hình khác nhau!
→ Mất context khi chuyển dialog
```

### ✅ Mới (Gom Nhóm)
```
Quản Lý Sách (1 Widget - 4 Tabs)
  ├─ Tab "Danh Sách": Table + Filter + Quick Actions
  ├─ Tab "Chi Tiết": Auto-switch khi double-click
  ├─ Tab "Thao Tác": Form tổng hợp (Add/Edit/Delete)
  └─ Tab "Thống Kê": Charts + Reports

→ Mọi thứ trong 1 màn hình!
→ Context được giữ nguyên
→ Faster workflow
```

---

## 🎯 Benefits

1. **Giảm Cognitive Load**
   - 4 trang thay vì 10+ dialogs
   - Context switching ít hơn

2. **Tận Dụng 100% Backend**
   - Service Layer pattern
   - No duplicate code
   - Validation centralized

3. **Better UX**
   - Tabs thay vì dialogs popup
   - All-in-one workflows
   - Modern dashboard layout

4. **Maintainability**
   - Clear separation of concerns
   - Backend = Business Logic
   - UI = Presentation only

---

## 🚀 Implementation Priority

### Phase 1: Dashboard ✅ (DONE)
- WidgetBangDieuKhien với cards

### Phase 2: Mượn Trả Sách (HIGH PRIORITY)
- Workflow tối ưu nhất
- Most used feature

### Phase 3: Quản Lý Sách (MEDIUM)
- Tab-based redesign
- Integrate validation

### Phase 4: Quản Lý Độc Giả (MEDIUM)
- Card list view
- AVL tree integration

---

**Next Steps**: Implement WidgetMuonTraSach với workflow design!
