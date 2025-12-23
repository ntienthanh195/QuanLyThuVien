# 🖨️ Modern Tech Dark Mode - Print Dialog (HopThoaiInAn)

## 📋 Tổng Quan

Hộp thoại **In Ấn Thư Viện** đã được nâng cấp lên phong cách **Modern Tech Dark Mode** với các đặc điểm:

- ✨ **Glassmorphism effects** - Hiệu ứng kính mờ hiện đại
- 🌟 **Neon blue accents** - Màu xanh điện phát sáng
- 🎨 **Gradient backgrounds** - Nền gradient đa tầng
- 💫 **Drop shadows** - Đổ bóng chuyên nghiệp
- 🎯 **High contrast** - Độ tương phản cao, dễ đọc

---

## 🎨 Phong Cách Thiết Kế

### Color Palette
```css
Primary Background:   #0F172A (Slate 950) → #1E293B (Slate 800)
Card Background:      rgba(30, 41, 59, 0.8) - Semi-transparent glass
Primary Accent:       #3B82F6 (Electric Blue) → #2563EB (Blue 600)
Text Primary:         #F1F5F9 (Slate 100)
Text Secondary:       #CBD5E1 (Slate 300)
Text Muted:          #64748B (Slate 500)
Border Glass:        rgba(59, 130, 246, 0.3) - Semi-transparent blue
Glow Effect:         rgba(59, 130, 246, 0.5) - Neon blue glow
```

### Typography
```
Header Title:        28px, Bold (700), Letter-spacing 2px
Section Titles:      15px, Bold (700), Letter-spacing 1px
Button Text:         14px, Semi-bold (600)
Body Text:           13px, Regular (400)
Subtitle:            13px, Italic
```

### Spacing & Dimensions
```
Dialog Min Width:    800px
Dialog Min Height:   700px
Section Padding:     20px
Card Border Radius:  12px
Button Height:       48px (primary), 44-46px (secondary)
Blur Radius:         15-30px (shadows)
```

---

## 🏗️ Cấu Trúc Giao Diện

### 1️⃣ HEADER (Floating with Glow)
```
🖨️  IN ẤN THƯ VIỆN
    Library Printing Center
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```
- Gradient background: `#1E293B → #0F172A → #1E293B`
- Neon icon với radial gradient glow
- Text shadow xanh điện
- Drop shadow: 30px blur, blue glow

### 2️⃣ SECTION 1: In Thẻ Độc Giả
```
📇  IN THẺ ĐỘC GIẢ
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

┌────────────────────────────────┐
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━ │
│  📇  THẺ THƯ VIỆN  📇         │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━ │
│  85.6mm × 54mm (CR80)          │
└────────────────────────────────┘

In thẻ thư viện chuẩn CR80 với QR code và mã độc giả

[🖨  In Thẻ Độc Giả Hiện Tại]  ← Electric blue gradient button
[📚  In Hàng Loạt (Toàn Bộ)]   ← Electric blue gradient button
```

**Chi Tiết:**
- Preview card: Gradient `#1E3A5F → #2D5A7B`, border 2px `#3B82F6`
- Buttons: Gradient `#3B82F6 → #2563EB` với glow effect
- Hover: Box-shadow 25px blue glow

### 3️⃣ SECTION 2: In Tem Mã Vạch Sách
```
🏷  IN TEM MÃ VẠCH SÁCH
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

┌────────────────────────────────┐
│  ║ ║║  ║ ║║║  ║║ ║  ║║║ ║    │  ← White background
│  ISBN BARCODE                  │
│  50mm × 30mm                   │
└────────────────────────────────┘

In tem dán gáy sách với mã ISBN

[🖨  In Tem Sách Hiện Tại]

┌─────────────────────────────────────────────────┐
│ 📊 Số lượng: [    5 tem    ▲▼]  [🖨 IN]      │  ← Glass container
└─────────────────────────────────────────────────┘
```

**Chi Tiết:**
- Barcode preview: White background với dashed border `#3B82F6`
- Quantity input: Dark glass container với blue border
- SpinBox: Electric blue arrows với hover effect

### 4️⃣ SECTION 3: In Báo Cáo / Biên Bản
```
📋  IN BÁO CÁO / BIÊN BẢN
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

┌────────────────────────────────┐
│ ━━━━━━━━━━━━━━━━━━━━━━━━━━━  │  ← White gradient
│ 📄  DANH SÁCH / BIÊN BẢN       │
│ ━━━━━━━━━━━━━━━━━━━━━━━━━━━  │
│ Khổ A4 (210mm × 297mm)         │
└────────────────────────────────┘

In danh sách và phiếu mượn/trả định dạng A4

[👥  Danh Sách Độc Giả]
[📚  Danh Mục Sách]

┌─────────────────────────────────────────────────┐
│ [📤 Phiếu Mượn]    [📥 Phiếu Trả]             │  ← Glass container
└─────────────────────────────────────────────────┘
```

### 5️⃣ FOOTER
```
[⚙  CẤU HÌNH]  ← Transparent with border    [ĐÓNG]  ← Blue gradient
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## 🛠️ Kỹ Thuật Triển Khai

### Helper Methods

#### `apDungPhongCachModern()`
```cpp
// Apply global gradient background to dialog
setStyleSheet(
    "QDialog {"
    "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
    "   stop:0 #0F172A, stop:0.5 #1E293B, stop:1 #0F172A);"
    "   border-radius: 16px;"
    "}"
);
```

#### `taoHieuUngBong(int blurRadius, QColor color)`
```cpp
// Create drop shadow effect for depth
QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
shadow->setBlurRadius(20);          // Soft blur
shadow->setColor(QColor(0, 0, 0, 100));  // Semi-transparent black
shadow->setOffset(0, 4);            // Slight vertical offset
```

#### `taoNutXanhDien(const QString& text)`
```cpp
// Electric blue gradient button with glow
background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
    stop:0 #3B82F6, stop:1 #2563EB);
    
// Hover state
background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
    stop:0 #2563EB, stop:1 #1D4ED8);
box-shadow: 0 0 25px rgba(59, 130, 246, 0.6);  // Neon glow
```

#### `taoKhungGlass(const QString& tieuDe)`
```cpp
// Glassmorphism card effect
background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
    stop:0 rgba(30, 41, 59, 0.8),    // Semi-transparent
    stop:1 rgba(15, 23, 42, 0.9));
border: 2px solid rgba(59, 130, 246, 0.3);  // Glass border
border-radius: 12px;
backdrop-filter: blur(10px);  // Glass blur (if supported)
```

---

## ✨ Hiệu Ứng Đặc Biệt

### 🌟 Glassmorphism
- Semi-transparent backgrounds
- Soft blur effects
- Subtle gradients
- Layered depth

### 💫 Neon Glow
- Text shadows với blue color
- Box shadows cho buttons
- Radial gradients cho icons
- Hover state brightening

### 🎨 Gradient Overlays
- Multi-stop linear gradients
- Radial gradients cho highlights
- Transparent to solid transitions

### 🔘 Button Interactions
```
Default:  Gradient #3B82F6 → #2563EB
Hover:    Gradient #2563EB → #1D4ED8 + 25px glow
Pressed:  Solid #1E40AF + padding shift
```

---

## 📱 Preview Elements

### Member Card Preview
- Size: 100px height
- Background: `#1E3A5F → #2D5A7B` gradient
- Border: 2px solid electric blue
- Content: Emoji + text simulation
- Shadow: 15px blur, blue tint

### Barcode Label Preview
- Size: 80px height
- Background: White
- Border: 2px dashed electric blue
- Font: Monospace (Courier New)
- Content: ASCII barcode simulation

### A4 Document Preview
- Size: 90px height
- Background: `#FFFFFF → #F3F4F6` gradient
- Border: 2px solid `#94A3B8`
- Content: Document header simulation

---

## 🔧 File Changes

### Modified Files
1. `bao_gom/giao_dien/hop_thoai/HopThoaiInAn.h`
   - Added: `QGraphicsDropShadowEffect`, `QGraphicsBlurEffect`, `QPropertyAnimation`
   - Added methods: `apDungPhongCachModern()`, `taoHieuUngBong()`, `taoNutXanhDien()`, `taoKhungGlass()`

2. `nguon/giao_dien/hop_thoai/HopThoaiInAn.cpp`
   - Complete UI rebuild with Modern Tech Dark Mode
   - Glassmorphism effects on all sections
   - Electric blue gradient buttons
   - Preview cards for each print type
   - Professional spacing and shadows

---

## 🚀 Cách Sử Dụng

### Mở Dialog
```cpp
// Từ CuaSoChinh hoặc widget khác
HopThoaiInAn* dialog = new HopThoaiInAn(this);

// Truyền dữ liệu (optional)
dialog->setDocGiaHienTai(docGiaHienTai);
dialog->setDanhSachDauSach(dsDauSach, soLuongDauSach);

// Hiển thị
dialog->exec();
```

### Tích Hợp với Menu
```cpp
// Thêm vào menu hoặc toolbar
QAction* actIn = new QAction("🖨 In Ấn", this);
connect(actIn, &QAction::triggered, this, &CuaSoChinh::hienThiIn);
```

---

## 🎯 Tương Lai & Nâng Cấp

### Đang Có
✅ Modern Tech Dark Mode
✅ Glassmorphism effects
✅ Electric blue accents
✅ Drop shadows & depth
✅ Preview cards
✅ Gradient buttons

### Có Thể Thêm
🔲 Animation on hover (QPropertyAnimation)
🔲 Live print preview thumbnails
🔲 Custom printer selection dropdown
🔲 Print history log
🔲 Export to PDF settings
🔲 Batch processing progress bar
🔲 Keyboard shortcuts (Ctrl+P, etc.)
🔲 Print templates manager

---

## 📸 Design Inspiration

**Phong cách:** Modern Tech Dashboard (Dribbble aesthetic)  
**Tham khảo:** Figma, Notion, VS Code dark themes  
**Công nghệ:** Qt6 QSS styling + Graphics effects  
**Target:** 8k resolution ready, high DPI displays  

---

## 🎨 Brand Identity

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
🖨️  LIBRARY PRINTING CENTER
    Professional • Modern • Efficient
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

**Motto:** "Chuyên nghiệp trong từng trang in"

---

**Build Date:** 2025-12-04  
**Version:** 2.0 Modern Tech  
**Status:** ✅ Production Ready
