# 🎨 Modern Dashboard UI - Hướng Dẫn Sử Dụng

## 📋 Tổng Quan

Hệ thống đã được redesign hoàn toàn theo **Material Design 3** và **SaaS Dashboard Style**, mang đến giao diện hiện đại, chuyên nghiệp như các ứng dụng web management.

---

## 🎨 Color Palette

### Primary Colors
- **Background**: `#F4F7FE` - Light Blue-Grey (Nền chính)
- **Surface**: `#FFFFFF` - Pure White (Card, Sidebar)
- **Primary**: `#4318FF` - Modern Indigo (Buttons, Links)
- **Secondary**: `#007BFF` - Bright Blue

### Text Colors
- **Dark**: `#2B3674` - Navy (Tiêu đề, Text chính)
- **Light**: `#A3AED0` - Grey (Subtitle, Caption)

### Status Colors
- **Success**: `#05CD99` - Teal Green
- **Warning**: `#FFB547` - Amber
- **Danger**: `#E31A1A` - Red

---

## 🏗️ Cấu Trúc Layout

```
┌─────────────────────────────────────────────┐
│  SIDEBAR (250px)  │  MAIN CONTENT (flexible) │
│                   │                          │
│  ┌─────────┐     │  ┌──────────────────┐   │
│  │ LOGO    │     │  │  Top Bar         │   │
│  └─────────┘     │  │  (Search+Filter) │   │
│                   │  └──────────────────┘   │
│  📊 Dashboard     │                          │
│  📚 Books         │  ┌──────────────────┐   │
│  👥 Readers       │  │  Data Table      │   │
│  📖 Loans         │  │  (No gridlines)  │   │
│  ...              │  └──────────────────┘   │
└─────────────────────────────────────────────┘
```

---

## 🔧 Sử Dụng Status Badge (Pill/Tag)

### ✅ Hàm Helper: `MainWindow::TaoTagTrangThai()`

Đây là **hàm quan trọng nhất** để tạo badge trạng thái đẹp như hình thiết kế.

#### 📝 Signature
```cpp
static QWidget* MainWindow::TaoTagTrangThai(int trangThai);
```

#### 📊 Tham số
- `trangThai`:
  - `0` = **Có sẵn** (CHO_MUON_DUOC) → 🟢 Green badge
  - `1` = **Đang mượn** (DANG_MUON) → 🟡 Yellow badge
  - `2` = **Thanh lý** (THANH_LY) → ⚪ Grey badge
  - `Other` = **Quá hạn** → 🔴 Red badge

#### 💡 Ví dụ sử dụng trong Table

```cpp
// Trong hàm HienThiBangSach() hoặc tương tự
for (int i = 0; i < soLuongSach; i++) {
    PTRDS sach = danhSachSach[i];
    
    // ... Set các cột khác (ISBN, Tên, Tác giả, etc.)
    
    // CỘT TRẠNG THÁI - Dùng setCellWidget thay vì setItem
    int trangThai = sach->trangThai; // Giả sử có field này
    QWidget* badgeWidget = MainWindow::TaoTagTrangThai(trangThai);
    
    tableWidget->setCellWidget(row, 7, badgeWidget); // Cột 7 = Trạng thái
}
```

#### 🎯 So sánh: Cũ vs Mới

**Cách CŨ (Plain text):**
```cpp
// ❌ XẤU - Chỉ là text đơn giản
tableWidget->setItem(row, 7, new QTableWidgetItem("Có sẵn"));
```

**Cách MỚI (Modern Badge):**
```cpp
// ✅ ĐẸP - Badge bo tròn, màu sắc, icon
QWidget* badge = MainWindow::TaoTagTrangThai(CHO_MUON_DUOC);
tableWidget->setCellWidget(row, 7, badge);
```

---

## 📐 Component Styles

### 1. Buttons

#### Primary Button
```cpp
QPushButton* btn = new QPushButton("Thêm Sách", this);
btn->setProperty("class", "btn-primary");
// Gradient Indigo với shadow
```

#### Success Button
```cpp
btn->setProperty("class", "btn-success");
// Teal Green
```

#### Warning Button
```cpp
btn->setProperty("class", "btn-warning");
// Amber
```

#### Danger Button
```cpp
btn->setProperty("class", "btn-danger");
// Red
```

### 2. Input Fields (Rounded Pill Style)
```cpp
QLineEdit* searchBox = new QLineEdit(this);
searchBox->setPlaceholderText("🔍 Tìm kiếm...");
// Tự động áp dụng: border-radius 16px, padding 12px 20px
```

### 3. Tables (No Gridlines)
```cpp
QTableView* table = new QTableView(this);
// Tự động áp dụng:
// - gridline-color: transparent
// - border-radius: 16px
// - Header: UPPERCASE, letter-spacing
// - Rows: hover effect
```

### 4. Sidebar (Active State)
```cpp
QListWidget* sidebar = new QListWidget(this);
// Selected item tự động có:
// - border-left: 4px solid #4318FF
// - background: #F4F7FE
```

---

## 🎬 Demo Code: Complete Table Setup

```cpp
void HienThiBangSach() {
    // 1. Tạo table
    QTableWidget* table = new QTableWidget(this);
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({
        "ISBN", "Tên Sách", "Tác Giả", "Thể Loại",
        "Năm XB", "Số Trang", "Số Lượng", "Trạng Thái"
    });
    
    // 2. Cấu hình table
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->horizontalHeader()->setStretchLastSection(true);
    
    // 3. Load data
    extern PTRDS dsDauSach[MAX_DAUSACH];
    extern int soLuongDauSach;
    
    table->setRowCount(soLuongDauSach);
    
    for (int i = 0; i < soLuongDauSach; i++) {
        PTRDS sach = dsDauSach[i];
        
        // Các cột text thông thường
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(sach->ISBN)));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(sach->tenSach)));
        table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(sach->tacGia)));
        table->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(sach->theLoai)));
        table->setItem(i, 4, new QTableWidgetItem(QString::number(sach->namXuatBan)));
        table->setItem(i, 5, new QTableWidgetItem(QString::number(sach->soTrang)));
        table->setItem(i, 6, new QTableWidgetItem(QString::number(sach->tongBanSao)));
        
        // ⭐ CỘT TRẠNG THÁI - DÙNG BADGE
        int trangThai = (sach->tongBanSao > 0) ? CHO_MUON_DUOC : THANH_LY;
        QWidget* statusBadge = MainWindow::TaoTagTrangThai(trangThai);
        table->setCellWidget(i, 7, statusBadge);
    }
}
```

---

## 📦 Files Liên Quan

### Stylesheets
- `resources/modern-dashboard.qss` - **Modern UI stylesheet** (ĐANG DÙNG)
- `resources/styles.qss` - Old stylesheet (fallback)

### Source Files
- `src/views/MainWindow.cpp` - Contains `TaoTagTrangThai()` implementation
- `include/views/MainWindow.h` - Header declaration

### Loading Priority
```cpp
// MainWindow.cpp::apDungStylesheet()
1. Thử load ":/modern-dashboard.qss" ← PRIORITY
2. Fallback ":/styles.qss" nếu không tìm thấy
```

---

## 🎯 Best Practices

### ✅ DO
1. **Luôn dùng `setCellWidget` cho cột Status**, không dùng `setItem`
2. **Dùng `[class="btn-primary"]`** để style buttons
3. **Căn giữa badges** bằng QHBoxLayout container
4. **Dùng icons/emojis** trong text để tăng visual appeal

### ❌ DON'T
1. Không hard-code colors trong code C++ → Dùng QSS
2. Không dùng gridlines trong table (đã ẩn trong QSS)
3. Không dùng border cứng nhắc → Dùng box-shadow thay thế
4. Không dùng font < 12px (khó đọc)

---

## 🔄 Migration Guide (Từ UI cũ)

### Bước 1: Update Table Rendering
```cpp
// CŨ
tableWidget->setItem(row, colIndex, new QTableWidgetItem(text));

// MỚI (cho cột Status)
QWidget* badge = MainWindow::TaoTagTrangThai(statusValue);
tableWidget->setCellWidget(row, colIndex, badge);
```

### Bước 2: Update Button Styles
```cpp
// CŨ
QPushButton* btn = new QPushButton("Text");

// MỚI
QPushButton* btn = new QPushButton("Text");
btn->setProperty("class", "btn-primary"); // Thêm dòng này
```

### Bước 3: Update Search Box
```cpp
// CŨ
QLineEdit* search = new QLineEdit();

// MỚI
QLineEdit* search = new QLineEdit();
search->setPlaceholderText("🔍 Tìm kiếm...");
// Rounded corners tự động apply qua QSS
```

---

## 🐛 Troubleshooting

### Issue: Stylesheet không load
```cpp
// Check trong MainWindow constructor
qDebug() << "✅ Loaded modern-dashboard.qss"; // Phải thấy dòng này
```

### Issue: Badge không hiển thị
```cpp
// Check: Phải dùng setCellWidget, KHÔNG phải setItem
table->setCellWidget(row, col, widget); // ✅ Correct
table->setItem(row, col, item);         // ❌ Wrong for badges
```

### Issue: Colors bị sai
```cpp
// Verify QSS file được include trong resources.qrc
<file>modern-dashboard.qss</file>
```

---

## 📊 Color Reference Card

```css
/* Copy vào code QSS của bạn nếu cần custom */
#4318FF  /* Primary - Indigo */
#05CD99  /* Success - Teal */
#FFB547  /* Warning - Amber */
#E31A1A  /* Danger - Red */
#F4F7FE  /* Background */
#2B3674  /* Text Dark */
#A3AED0  /* Text Light */
```

---

## 🚀 Next Steps

1. **Apply badges** cho tất cả tables (Books, Readers, Loans)
2. **Customize colors** cho từng status type
3. **Add icons** vào sidebar menu items
4. **Implement dark mode** (optional)
5. **Add animations** cho hover/click effects

---

## 📞 Contact & Support

- **Developer**: Qt/C++ Team
- **Design System**: Material Design 3 + SaaS Dashboard
- **Last Updated**: December 2, 2025

---

**Chúc bạn code vui vẻ! 🎉**
