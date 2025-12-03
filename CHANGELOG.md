# 📝 CHANGELOG - Hệ Thống Quản Lý Thư Viện

## [4.0.0] - 2025-01-09 - GIAI ĐOẠN 4: ADVANCED FEATURES 🚀

### ✨ NEW - Stage 4.1: Top 10 Books Report
- **ManHinhTop10 Dialog (NEW - 350+ lines):**
  - Displays top 10 most borrowed books with visual ranking
  - **Medals System:** 🥇 Gold, 🥈 Silver, 🥉 Bronze for top 3
  - **Color-coded backgrounds:**
    * Gold #FFD700 for 1st place
    * Silver #C0C0C0 for 2nd place
    * Bronze #CD7F32 for 3rd place
  - **6-column table:** Rank, ISBN, Book Name (bold top 3), Author, Borrow Count (green bold), Percentage
  - **Statistics panel:** Total books tracked, total borrows
  - **Keyboard shortcuts:** Ctrl+T to open, F5 to refresh, Esc to close
  - **Performance:** Instant loading with std::sort algorithm
  - **Edge case:** Shows message when no books borrowed

### ⏰ NEW - Stage 4.2: Overdue Readers Report
- **ManHinhQuaHan Dialog (NEW - 380+ lines):**
  - Tracks all readers with overdue borrowed books
  - **4-level severity system with color coding:**
    * 🔴 **Nghiêm Trọng** (≥30 days): Red background #e74c3c
    * 🟠 **Cao** (≥14 days): Orange background #e67e22
    * 🟡 **Trung Bình** (≥7 days): Yellow background #f39c12
    * 🟢 **Nhẹ** (<7 days): Light yellow background #f1c40f
  - **7-column detailed table:** Card ID (bold), Name, Gender, Overdue Books (red bold), Total Days, Max Days (bold), Severity (colored)
  - **Sorting:** By max overdue days descending (worst first)
  - **Data processing:** Efficient AVL tree InOrder traversal + linked list iteration
  - **Statistics:** Shows total readers with overdue books
  - **Keyboard shortcuts:** Ctrl+Shift+O to open, F5 to refresh, Esc to close
  - **Edge case:** Shows ✅ "No overdue readers" message when clean

### 💾 ENHANCED - Stage 4.3: Auto Backup on Exit
- **Smart backup prompt when closing application:**
  - **3-button choice:** Yes (backup & exit), No (exit without backup), Cancel (stay in app)
  - **Scenario 1 - Data Changed:**
    * Prompts: "Data changed will be auto-saved. Create backup before exit?"
    * Yes: Creates backup, shows success message, exits
    * No: Saves data, exits without backup
    * Cancel: Stays in app (no save, no exit)
  - **Scenario 2 - No Data Changed:**
    * Still offers backup option for safety
    * Same 3-button choice
  - **Feedback:** QMessageBox shows backup success/error messages
  - **File naming:** backup_YYYYMMDD_HHMMSS_loai.txt with timestamp
  - **Integration:** Seamlessly integrated into MainWindow::closeEvent()

### 📚 ENHANCED - Stage 4.4: Upgraded About Dialog
- **Version display:** 4.0.0 (Qt 6.10.1) in green bold
- **Build info:** Shows build date and time using __DATE__ __TIME__ macros
- **Professional frame:** Gray background (#ecf0f1) with rounded corners
- **New features section:** Lists all Stage 4 enhancements:
  * 🥇 Top 10 books report
  * ⏰ Overdue tracking with severity levels
  * 💾 Auto backup on exit
  * 🎨 Professional UI with gradients
  * ⌨️ Keyboard shortcuts (Ctrl+F, F5, Esc...)
  * 💡 Detailed tooltips
- **Credits section:** Team information and course details
- **Technology stack:** C++17, Qt 6.10.1, MinGW 13.1.0
- **Window title:** "Về Hệ Thống 📚"
- **Size:** Increased to 600x600 for better readability

### 🎨 UI/UX Improvements
- **Consistent design language:**
  * Gradient titles across all dialogs
  * Top 10: Orange-red gradient (#f39c12 → #e74c3c)
  * Overdue: Red gradient (#e74c3c → #c0392b)
- **Professional table styling:**
  * Alternating row colors for readability
  * Bold fonts for emphasis (rankings, critical data)
  * 45px row height for comfortable viewing
  * Custom cell backgrounds for severity/medals
- **Visual feedback:**
  * Hover effects on buttons
  * Color-coded data (green for good, red for bad)
  * Icons in labels (📊 🥇 🟠 💾 etc.)
- **Keyboard accessibility:**
  * All dialogs support Esc to close
  * F5 to refresh data
  * Ctrl+shortcuts for opening dialogs

### 🔧 Technical Improvements
- **Data structure optimization:**
  * ManHinhTop10: Vector of structs (SachVaSoLuotMuon) for sorting
  * ManHinhQuaHan: Vector of structs (DocGiaQuaHan) for severity calculation
  * Efficient std::sort with lambda comparators
- **AVL tree traversal:**
  * InOrder traversal for collecting all active readers
  * Proper linked list iteration (MUONTRA) for loan records
  * Edge case handling (empty nodes, no borrows)
- **Date calculation:**
  * Uses tinhSoNgayQuaHan() for accurate overdue day count
  * Handles current date comparison properly
- **Memory management:**
  * Proper cleanup in dialog destructors
  * No memory leaks (vectors auto-managed)
- **Error handling:**
  * Backup operations with try-catch (implicitly in TaoBackup)
  * Empty dataset handling in all dialogs
  * User-friendly error messages

### 📝 Documentation
- **NEW: TESTING_STAGE4.md (143 lines):**
  * Comprehensive testing checklist for all Stage 4 features
  * Test scenarios for Top 10, Overdue, Auto-backup, About
  * Performance benchmarks and success criteria
  * Edge case testing guide
- **UPDATED: CHANGELOG.md:**
  * Full version 4.0.0 changelog with detailed feature descriptions
  * Technical details and usage instructions
- **UPDATED: LibraryManagement.pro:**
  * Added ManHinhTop10.h/.cpp to build
  * Added ManHinhQuaHan.h/.cpp to build

### 🐛 Known Issues
- **Warning:** Unused parameter 'iconPath' in MainWindow::createMenuButton
  * Not critical, build succeeds
  * Will fix in future polish pass

### 📊 Build Information
- **Compiler:** MinGW 13.1.0 (g++)
- **C++ Standard:** C++17 (-std=gnu++1z)
- **Qt Version:** 6.10.1
- **Build Status:** ✅ Success (1 warning)
- **Lines Added:** ~850 lines across 2 new dialogs + MainWindow enhancements
- **Files Changed:** 
  * ManHinhTop10.h/.cpp (NEW)
  * ManHinhQuaHan.h/.cpp (NEW)
  * ManHinhGioiThieu.cpp (ENHANCED)
  * MainWindow.cpp (ENHANCED with auto-backup)
  * LibraryManagement.pro (UPDATED)

---

## [3.0.0] - 2025-12-02 - GIAI ĐOẠN 3: POLISH & ENHANCEMENTS ✨

### ✅ Added
- **Tooltips toàn diện:**
  - Tooltips chi tiết cho tất cả search type combos
  - Hướng dẫn sử dụng cho từng loại tìm kiếm
  - Tooltips cho tất cả buttons với keyboard shortcuts
  
- **Keyboard Shortcuts:**
  - Main menu: Ctrl+B/R/L/T/S/K/O, F1, Ctrl+Q
  - Search: Enter để tìm, Esc để xóa
  - Dialogs: Ctrl+S để lưu, Esc để hủy
  
- **Visual Enhancements:**
  - Input validation feedback với viền màu (xanh/đỏ)
  - Better file size formatting (KB/MB) trong backup dialog
  - Info panel trong statistics dialog
  
- **Documentation:**
  - FEATURES.md - Tài liệu đầy đủ tính năng
  - CHANGELOG.md - Lịch sử thay đổi

### ✅ Improved
- Search widgets với detailed tooltips
- Backup dialog hiển thị size theo KB/MB
- Dialog buttons với keyboard shortcuts
- Input fields với visual feedback

---

## [2.0.0] - 2025-12-02 - GIAI ĐOẠN 2: UI INTEGRATION 🎨

### ✅ Added - Stage 2.1: Validation Forms
- **ManHinhThemDocGia:**
  - Validation họ/tên với `KiemTraTenNguoi()`
  - QMessageBox cảnh báo chi tiết
  - Visual feedback cho invalid inputs
  
- **ManHinhSuaDocGia:**
  - Same validation pattern
  - Kiểm tra không thể khóa nếu đang mượn sách

### ✅ Added - Stage 2.2: Statistics Dialog
- **ManHinhThongKe (NEW - 371 lines):**
  - 13 metrics trong 2-column layout
  - Book stats: 7 chỉ số (tổng sách, bản sao, có thể mượn, đang mượn, thanh lý, tỷ lệ %, thể loại)
  - Reader stats: 6 chỉ số (tổng độc giả, hoạt động, khóa, đang mượn, tổng mượn, quá hạn)
  - Color-coded labels (xanh/xanh dương/đỏ/cam)
  - Button "📊 THỐNG KÊ HỆ THỐNG" trong main menu

### ✅ Added - Stage 2.3: Advanced Search
- **WidgetQuanLySach enhancements:**
  - QComboBox với 5 loại tìm kiếm
  - 🔍 Tìm thông thường
  - 📅 Tìm theo năm
  - 📆 Tìm khoảng năm (2015-2020)
  - ✍️ Tìm theo tác giả
  - 📊 Lọc theo trạng thái
  - Clear button với Esc shortcut
  
- **WidgetQuanLyDocGia enhancements:**
  - QComboBox với 5 loại tìm kiếm
  - 🔍 Tìm theo mã thẻ
  - 👤 Tìm theo tên
  - 👥 Tìm theo họ
  - 👤👥 Tìm theo họ tên
  - 📊 Lọc theo trạng thái
  - Clear button với Esc shortcut

### ✅ Added - Stage 2.4: Backup Management
- **ManHinhBackup (NEW - 405 lines):**
  - 6 thao tác backup: Create, Restore, Delete, Cleanup, Refresh, Close
  - QListWidget hiển thị "📁 filename | 📅 date | 💾 size"
  - Color-coded buttons (xanh/xanh dương/đỏ/cam/xám)
  - Double confirmation cho restore/delete
  - QInputDialog cho cleanup count
  - Button "💾 SAO LƯU DỮ LIỆU" trong main menu

### ✅ Added - Stage 2.5: Error Handling
- Try-catch blocks trong `ManHinhThemDocGia::khiNhanLuu()`
- Try-catch blocks trong `ManHinhSuaDocGia::khiNhanLuu()`
- QMessageBox::critical cho exception handling

### ✅ Added - Stage 2.6: Testing & Integration
- All features tested and verified
- Build successful với Qt 6.10.1
- Only cosmetic warning (unused iconPath parameter)

---

## [1.0.0] - 2025-12-01 - GIAI ĐOẠN 1: BACKEND IMPROVEMENTS 🔧

### ✅ Added - Validation Functions
- `KiemTraTenNguoi(ten, loai)`:
  - Kiểm tra chỉ chứa chữ cái và khoảng trắng
  - Kiểm tra độ dài tối đa 50 ký tự
  - Return error message nếu không hợp lệ

### ✅ Added - Statistics Functions
- `LayThongKeSach(ds[], n)`:
  - Return ThongKeSach struct với 7 metrics
  - Tổng đầu sách, bản sao, có thể mượn, đang mượn, thanh lý
  - Tỷ lệ có thể mượn, số thể loại
  
- `LayThongKeDocGia(root, ds[], n)`:
  - Return ThongKeDocGia struct với 6 metrics
  - Tổng độc giả, hoạt động, khóa, đang mượn
  - Tổng sách mượn, có sách quá hạn

### ✅ Added - Search Functions (Books)
- `TimSachTheoNamXuatBan(ds[], n, year, kq[])`:
  - Tìm sách xuất bản năm cụ thể
  - Return count và fill kq[] array
  
- `TimSachTheoKhoangNam(ds[], n, start, end, kq[])`:
  - Tìm sách trong khoảng năm
  - Return count và fill kq[] array
  
- `TimSachTheoTacGia(ds[], n, author, kq[])`:
  - Tìm sách của tác giả (case-insensitive)
  - Return count và fill kq[] array
  
- `LocSachTheoTrangThai(ds[], n, status, kq[])`:
  - Lọc sách theo trạng thái (0/1/2)
  - Return count và fill kq[] array

### ✅ Added - Search Functions (Readers)
- `TimDocGiaTheoTen(root, ten, kq[], maxKq)`:
  - Tìm độc giả theo tên (case-insensitive)
  - Duyệt cây AVL, return count
  
- `TimDocGiaTheoHo(root, ho, kq[], maxKq)`:
  - Tìm độc giả theo họ (case-insensitive)
  - Return count và fill kq[] array
  
- `TimDocGiaTheoHoTen(root, hoTen, kq[], maxKq)`:
  - Tìm theo họ và tên kết hợp
  - Return count và fill kq[] array
  
- `TimDocGiaTheoTrangThai(root, status, kq[], maxKq)`:
  - Lọc theo trạng thái (0=Khóa, 1=Hoạt động)
  - Return count và fill kq[] array

### ✅ Added - Backup System
- `TaoBackup(thuMuc)`:
  - Tạo backup thủ công
  - Format: backup_YYYYMMDD_HHMMSS_loai.txt
  - Return error message hoặc empty string
  
- `AutoBackup()`:
  - Gọi tự động mỗi lần lưu
  - Same format as manual backup
  
- `PhucHoiBackup(duongDan)`:
  - Phục hồi từ backup file
  - Overwrite current data files
  - Return error message hoặc empty string
  
- `LayDanhSachBackup(kq[], maxKq, thuMuc)`:
  - List tất cả backup files
  - Return ThongTinBackup array
  - Sorted by date descending
  
- `XoaBackup(duongDan)`:
  - Xóa backup file cụ thể
  - Return error message hoặc empty string
  
- `XoaBackupCu(soGiuLai, thuMuc)`:
  - Xóa backup cũ, giữ N mới nhất
  - Return số lượng đã xóa

### ✅ Added - Helper Functions
- `KiemTraThuMucBackup(thuMuc)` - Tạo thư mục nếu chưa có
- `LayThuMucBackupMacDinh()` - Return "files/"
- `TaoTenFileBackup(loai)` - Generate filename với timestamp

---

## [0.9.0] - 2025-11-30 - INITIAL VERSION 🎬

### ✅ Core Features
- Quản lý sách với danh sách tĩnh
- Quản lý độc giả với cây AVL
- Mượn trả sách với giao dịch
- File I/O cho persistence
- Basic UI với Qt 6

### ✅ Data Structures
- `DauSach` - Thông tin sách
- `DanhMucSach` - Danh sách bản sao (linked list)
- `TheDocGia` - Thông tin độc giả
- `NodeDocGia` - Node cây AVL
- `GiaoDichMuon` - Lịch sử mượn sách

---

## 📊 Summary by Version

| Version | Date | Features Added | Lines Changed |
|---------|------|----------------|---------------|
| 3.0.0 | 2025-12-02 | Polish & Enhancements | ~500 |
| 2.0.0 | 2025-12-02 | UI Integration | ~2000 |
| 1.0.0 | 2025-12-01 | Backend Improvements | ~1500 |
| 0.9.0 | 2025-11-30 | Initial Version | ~6000 |
| **TOTAL** | | **Complete System** | **~10,000+** |

---

## 🎯 Next Steps (Future)

### Potential Enhancements:
- [ ] Export statistics to PDF
- [ ] Print functionality
- [ ] Batch import from CSV
- [ ] User authentication system
- [ ] Transaction history viewer
- [ ] Email notifications for overdue books
- [ ] Barcode scanner integration
- [ ] Online catalog

---

**Maintained by:** Development Team  
**Last Updated:** 2 tháng 12, 2025  
**Version:** 3.0.0 (Production Ready) ✅
