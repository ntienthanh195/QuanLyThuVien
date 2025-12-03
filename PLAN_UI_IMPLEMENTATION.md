# KẾ HOẠCH HOÀN THIỆN UI THEO HÌNH MẪU

## ✅ ĐÃ HOÀN THÀNH

### 1. Dashboard (Bảng điều khiển) ✅
- [x] Tạo WidgetBangDieuKhien với layout hiện đại
- [x] Card cảnh báo (Sách quá hạn, Thẻ sắp hết hạn)  
- [x] Card thống kê (Tổng sách, Độc giả, Đang mượn, Lượt hôm nay)
- [x] Nút tác vụ nhanh (Thêm sách, Đăng ký độc giả, Tạo phiếu mượn)
- [x] Tích hợp vào MainWindow
- [x] Sidebar navigation với icons

### 2. Stylesheet QSS ✅
- [x] Màu chủ đạo: #2563EB (Blue)
- [x] Bo tròn góc, shadows
- [x] Typography hiện đại
- [x] Button styles (primary, success, warning, danger)
- [x] Table styles
- [x] Input fields, combobox
- [x] Tabs, scrollbar, badges

### 3. MainWindow ✅
- [x] Tích hợp sidebar menu
- [x] QSplitter layout
- [x] Áp dụng stylesheet toàn cục
- [x] Navigation giữa các màn hình

## 🚧 ĐANG THỰC HIỆN

### 4. Màn hình Quản lý Sách (WidgetQuanLySach)
**Cần cập nhật theo hình mẫu:**

#### Header Section
- [ ] Tiêu đề "Quản lý sách (Danh mục)"
- [ ] Icon menu hamburger (nếu cần)
- [ ] Dropdown "Tìm kiếm" (STT, ISBN, Tên sách, Tác giả...)
- [ ] Thanh tìm kiếm placeholder "Tìm theo ISBN, Tên Sách, Tác-Giả..."
- [ ] Icon search trong input

#### Filter & Actions Section  
- [ ] Dropdown "Lọc theo" với icon filter
- [ ] Dropdown "Tất cả thể loại" 
- [ ] Dropdown "Tất cả trạng thái"
- [ ] Nút "➕ Thêm" màu xanh primary
- [ ] Nút "✏️ Sửa" màu vàng warning  
- [ ] Nút "🗑️ Xóa" màu đỏ danger

#### Table Section
- [ ] Bảng với các cột: STT, ISBN, TÊN SÁCH, TÁC GIẢ, THỂ LOẠI, SỐ LƯỢNG, VỊ TRÍ, TRẠNG THÁI, HÀNH ĐỘNG
- [ ] Badge trạng thái:
  - "Đang mượn" - màu vàng (#FEF3C7 bg, #92400E text)
  - "Có sẵn" - màu xanh lá (#D1FAE5 bg, #065F46 text)  
  - "Đã thanh lý" - màu xám (#F1F5F9 bg, #475569 text)
- [ ] Icons hành động: ✏️ (edit), 🗑️ (delete)
- [ ] Hover effect trên rows
- [ ] Alternating row colors nhẹ

## 📋 SẮP THỰC HIỆN

### 5. Màn hình Quản lý Độc Giả (WidgetQuanLyDocGia)
**Theo hình mẫu:**

#### Header
- [ ] Tiêu đề "Quản lý độc giả"
- [ ] Nút "+" thêm độc giả
- [ ] Thanh tìm kiếm "Tìm theo tên, mã độc giả..."

#### Tabs Filter
- [ ] Tab "🔧 Lọc" (active)
- [ ] Tab "Tất cả"
- [ ] Tab "Sinh viên"  
- [ ] Tab "Giảng viên"
- [ ] Tab "Đã hết hạn"

#### Card List
- [ ] Danh sách dạng card (không phải table)
- [ ] Mỗi card hiển thị:
  - Avatar tròn bên trái
  - Tên độc giả (bold, lớn)
  - Mã độc giả (nhỏ, màu xám)
  - Icon menu 3 chấm bên phải
- [ ] Hover effect
- [ ] Shadow nhẹ

### 6. Polish & Refinements
- [ ] Test tất cả màn hình
- [ ] Responsive behavior
- [ ] Loading states
- [ ] Error handling UI
- [ ] Confirmation dialogs đẹp
- [ ] Tooltips
- [ ] Keyboard shortcuts
- [ ] Accessibility

## 📐 DESIGN TOKENS

### Colors
- Primary: #2563EB
- Success: #10B981  
- Warning: #F59E0B
- Danger: #DC2626
- Purple: #8B5CF6
- Gray-50: #F9FAFB
- Gray-100: #F3F4F6
- Gray-600: #4B5563
- Gray-900: #111827

### Spacing
- xs: 4px
- sm: 8px  
- md: 12px
- lg: 16px
- xl: 20px
- 2xl: 24px
- 3xl: 32px

### Border Radius
- sm: 6px
- md: 8px
- lg: 12px
- full: 9999px

### Typography
- Title: 24px/28px, bold
- Subtitle: 18px/24px, semibold  
- Body: 14px/20px, regular
- Small: 12px/16px, regular

---

## 🎯 NEXT STEPS

1. **Hoàn thiện WidgetQuanLySach:**
   - Thêm các dropdown filter
   - Cập nhật table với badges trạng thái
   - Thêm icons cho các nút
   - Style theo Material Design

2. **Hoàn thiện WidgetQuanLyDocGia:**
   - Tạo layout dạng card list
   - Thêm tabs filter  
   - Avatar cho mỗi độc giả
   - Menu 3 chấm cho actions

3. **Testing & Polish:**
   - Test trên dữ liệu thực
   - Kiểm tra responsive
   - Fix bugs UI
   - Optimize performance
