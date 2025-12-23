# BÁO CÁO PHÂN TÍCH MÀU SẮC - HỆ THỐNG QUẢN LÝ THƯ VIỆN

**Ngày lập:** 23/12/2025  
**Phiên bản:** 2.0  
**Người lập:** Phân tích tự động từ mã nguồn

---

## 📋 MỤC LỤC

1. [Tổng quan hệ thống màu](#1-tổng-quan-hệ-thống-màu)
2. [Phân tích chi tiết theo Tab](#2-phân-tích-chi-tiết-theo-tab)
3. [Bảng tổng hợp màu sắc](#3-bảng-tổng-hợp-màu-sắc)
4. [Font và Typography](#4-font-và-typography)
5. [Kích thước nút và spacing](#5-kích-thước-nút-và-spacing)
6. [File stylesheet sử dụng](#6-file-stylesheet-sử-dụng)

---

## 1. TỔNG QUAN HỆ THỐNG MÀU

### 1.1 Các File Stylesheet Chính

Ứng dụng sử dụng **3 file stylesheet chính**:

| File | Đường dẫn | Mục đích sử dụng |
|------|-----------|------------------|
| **styles.qss** | `resources/styles.qss` | Theme sáng - Material Modern (Blue) |
| **enterprise-theme.qss** | `resources/enterprise-theme.qss` | Theme tối - Enterprise Professional |
| **modern-dashboard.qss** | `resources/modern-dashboard.qss` | Theme tối - Deep Dark Professional (Đang dùng) |

> **Theme hiện tại:** Ứng dụng đang sử dụng **modern-dashboard.qss** với màu chủ đạo là **Midnight Professional Dark Theme**

### 1.2 Bảng Màu Chính (Color Palette)

#### 🎨 Màu Cơ Bản
```css
/* BACKGROUND COLORS - Từ tối đến sáng */
Sidebar (Darkest):     #1a1f2e (Midnight blue - neo xanh đen)
Background Main:       #232938 (Mid-tone neutral - Nền chính)
Card Background:       #353b4a (Card containers)
Table Body:            #2d3748 (Lightest - Content focus)
Table Alternate:       #262e3d (Darker alternating row)
Table Header:          #1e2633 (Darkest in table)
Border Color:          #424959 (Subtle separation)
Border Dark:           #1a2332 (Very subtle)
```

#### 🔵 Màu Accent & Action
```css
Primary Blue:          #4A90E2 (Nút chính, accent)
Primary Blue Hover:    #3a7bc8 (Khi hover)
Primary Blue Pressed:  #2a5a8a (Khi nhấn)
Primary Gradient:      #5ba3f5 → #3d7dc4
```

#### ✅ Màu Trạng Thái
```css
Success Green:         #1BC5BD (Nút thành công)
Success Green Alt:     #48bb78 (Alternative green)
Success Dark:          #0e9d95 (Hover state)

Warning Orange:        #f6ad55 (Cảnh báo)
Warning Alt:           #ffb84d (Alternative orange)

Danger Red:            #F64E60 (Nguy hiểm)
Danger Hover:          #d32f2f (Hover state)
Danger Alt:            #fc8181 (Alternative red)
```

#### ⚪ Màu Text
```css
Text Primary:          #e8eef5 (Bright white - Text chính)
Text Secondary:        #a0aec0 (Readable gray - Text phụ)
Text Muted:            #B5B5C3 (Muted gray)
Text Dark:             #6C6C80 (Disabled text)
```

---

## 2. PHÂN TÍCH CHI TIẾT THEO TAB

### 2.1 🏠 TAB TRANG CHỦ (BangDieuKhien)

**File implementation:** `src/ui/BangDieuKhien.cpp`  
**Stylesheet áp dụng:** `modern-dashboard.qss` + Inline styles

#### Màu Nền
```css
Widget Background: qlineargradient(
    x1:0, y1:0, x2:1, y2:1,
    stop:0 #1a1d29,
    stop:0.5 #1e2230,
    stop:1 #1a1d29
) /* Gradient xanh đen */
```

#### Thẻ Thống Kê (Statistics Cards)
```css
Card Background:       #353b4a
Card Border:           #424959
Card Title Color:      #FFFFFF (Bold 24px)
Card Value Color:      #5ba3f5 (Large numbers)
Card Description:      #A0A0B0 (12px)
```

#### Alert Cards (Cảnh báo)
```css
Alert Danger:          
  - Background: #353b4a
  - Border-left: 4px solid #ff6b6b (Red)

Alert Warning:
  - Background: #353b4a
  - Border-left: 4px solid #ffb84d (Orange)
```

#### Bảng Hoạt Động (Activity Table)
```css
Table Background:      #1a1f2e (Match sidebar)
Table Alternate:       #1e2633
Table Header BG:       #141b26
Table Header Text:     #a0aec0 (12px, UPPERCASE, letter-spacing: 0.5px)
Table Border:          #141920
Gridline:              rgba(255,255,255,0.05)
Selected Row:          #4A7BA7
Hover Row:             #2f4558
```

#### Nút Tác Vụ
```css
Primary Button (Quản lý sách, etc):
  - Background: #4A90E2
  - Hover: #3a7bc8
  - Text: white
  - Padding: 10px 20px
  - Border-radius: 8px
  - Font-weight: 700
```

---

### 2.2 👥 TAB ĐỘC GIẢ (WidgetQuanLyDocGia)

**File implementation:** `src/ui/WidgetQuanLyDocGia.cpp`  
**Stylesheet:** `modern-dashboard.qss` + Inline overrides

#### Màu Nền Widget
```css
Background Main:       #232938
```

#### Nút Thao Tác
```css
/* Nút Thêm Độc Giả (Primary) */
Background:            #0078D4 (Microsoft Blue)
Hover:                 #106EBE
Pressed:               #005A9E
Padding:               6px 12px
Border-radius:         4px
Font-weight:           600

/* Nút Xóa (Danger) */
Background:            #C42B1C (Dark red)
Hover:                 #A80000
Disabled:              #2D2D30 with color #666

/* Nút Phụ (Secondary) */
Background:            #2D2D30
Border:                1px solid #3E3E42
Hover Background:      #3E3E42
Hover Border:          #0078D4
```

#### Thanh Tìm Kiếm
```css
Search Input:
  - Background: #1a1d23
  - Color: #E0E0E0
  - Border: 1px solid #151820
  - Border-radius: 5px
  - Padding: 8-12px
  - Font-size: 13px
  
Search Button:
  - Background: #5ba3f5 (Blue gradient)
  - Hover: #4a93e3
  - Pressed: #3d7dc4
```

#### ComboBox (Chọn trạng thái, giới tính)
```css
Background:            #1a1d23
Color:                 #E0E0E0
Border:                1px solid #151820
Dropdown Background:   #242933
Selection BG:          #4A90E2
```

#### Bảng Độc Giả
```css
Background:            #2c3e50 (Deep slate blue)
Alternate Row:         #273747
Header Background:     #34495e
Header Text:           #b8c5d6
Border:                #1e2938
Selected Row:          #4A7BA7

/* ⚠️ LƯU Ý: Màu này GIỐNG với tab Quản lý Sách */
```

#### Label Thông Tin Độc Giả (Selected Info)
```css
/* Khi có độc giả được chọn */
Background Odd:        qlineargradient(
                         x1:0, y1:0, x2:1, y2:1,
                         stop:0 #3a3f4d, stop:1 #353b4a
                       )
Border:                1px solid #424959
Padding:               18px
Color:                 #f0f4f8

Background Even:       qlineargradient(
                         stop:0 #0f1418, stop:1 #071119
                       )
```

---

### 2.3 📚 TAB SÁCH (WidgetQuanLySach)

**File implementation:** `src/ui/WidgetQuanLySach.cpp`  
**Stylesheet:** `modern-dashboard.qss`

#### Màu Nền
```css
Background:            #232938
Header Container:      transparent
```

#### Nút Thao Tác Header
```css
/* Nút Thêm Sách (Primary) */
Background:            #0078D4
Hover:                 #106EBE
Pressed:               #005A9E
Padding:               6px 12px
Font-weight:           600

/* Nút Nhập/Xuất File (Secondary) */
Background:            #2D2D30
Color:                 #E0E0E0
Border:                1px solid #3E3E42
Hover Background:      #3E3E42
Hover Border:          #0078D4

/* Nút Xóa */
Background:            Động theo hàm (Primary color)
Disabled Background:   #2D2D30
Disabled Color:        #666
```

#### Thanh Tìm Kiếm Sách
```css
Search Container:      transparent

Search Input:
  - Background: #1a1d23
  - Color: #E0E0E0
  - Border: 1px solid #0f161d
  - Border-radius: 5px
  - Padding: 0 12px
  - Min-height: 40px
  
Filter ComboBox:
  - Background: #1a1d23
  - Dropdown: #242933
  - Selection: #4A90E2
  
Search Button:
  - Background: #5ba3f5
  - Hover: #4a93e3
  - Pressed: #3d7dc4
  - Min-height: 40px

Refresh Button:
  - Background: #242933
  - Color: #CCCCCC
  - Hover: #2d3541
```

#### Bảng Sách
```css
Table Class:           "InnerTable"

/* Màu thực tế hiển thị (do selector cao hơn): */
Background:            #2c3e50 (Deep slate blue)
Alternate Row:         #273747
Header Background:     #34495e
Header Text:           #b8c5d6
Border:                #1e2938

/* Màu từ class "InnerTable" (bị override): */
/* Background:          #1a1f2e (Sidebar darkness) */
/* Alternate Row:       #1e2633 */
/* Header Background:   #141b26 */
/* Header Text:         #a0aec0 */

Gridline:              rgba(255,255,255,0.05)
Selected Item:         #4A7BA7
Item Padding:          6px

/* ⚠️ LƯU Ý: Màu này GIỐNG với tab Độc Giả */
```

#### Context Menu
```css
Menu Background:       #2D2D30
Menu Text:             white
Menu Border:           1px solid #3E3E42
Selected Item:         #0078D4
Border-radius:         4px
```

#### Hint Label
```css
Color:                 #666
Font-style:            italic
Font-size:             11px
```

---

### 2.4 📖 TAB MƯỢN TRẢ (WidgetMuonTraSach)

**File implementation:** `src/ui/WidgetMuonTraSach.cpp`  
**Stylesheet:** `modern-dashboard.qss`

#### Màu Chính
```css
Widget Background:     #232938 (Midnight Professional)
```

#### Bảng Độc Giả (Readers Table)
```css
/* Tương tự Tab Độc Giả */
Background:            #2c3e50
Alternate:             #273747
Header:                #34495e
Selected:              #4A7BA7
```

#### GroupBox (Thông tin mượn trả)
```css
GroupBox Profile:
  - Background: #353b4a
  - Border: 1px solid #424959
  - Title Color: #ffb84d (Orange)
  - Font-weight: bold

GroupBox Books:
  - Title Color: #5ba3f5 (Blue)

GroupBox Input:
  - Title Color: #5cd68d (Green)
```

#### Nút Mượn/Trả
```css
/* Nút Mượn (Success) */
Background:            #1BC5BD (Teal)
Hover:                 #0e9d95
Text:                  white
Padding:               10px 20px
Border-radius:         8px

/* Nút Trả (Primary) */
Background:            #4A90E2
Hover:                 #3a7bc8

/* Nút Báo Mất (Danger) */
Background:            #F64E60
Hover:                 #d32f2f
```

#### Input Fields
```css
QLineEdit (Dark):
  - Background: #2a2e39
  - Color: #f0f4f8
  - Border: 1px solid #424959
  - Focus Border: #5ba3f5
  - Padding: 8px 12px
  - Border-radius: 8px
```

#### Bảng Sách Đang Mượn
```css
Inner Table Class:     "InnerTable"
Background:            #1a1f2e
Header:                #141b26
Gridline:              rgba(255,255,255,0.05)
```

---

## 3. BẢNG TỔNG HỢP MÀU SẮC

### 3.1 Màu Nền (Backgrounds)

| Element | Màu | HEX Code | Sử dụng |
|---------|-----|----------|---------|
| Sidebar | Midnight Blue | `#1a1f2e` | Thanh bên trái |
| Main Background | Neutral Dark | `#232938` | Nền chính tất cả tab |
| Card Container | Slate Gray | `#353b4a` | Card, GroupBox |
| Table Body | Content Gray | `#2d3748` | Nền bảng sáng |
| Table Header | Dark Slate | `#1e2633` | Header bảng |
| Input Field | Deep Dark | `#1a1d23` | Ô nhập liệu |
| Input Field Alt | Dark Gray | `#2a2e39` | Ô nhập liệu thay thế |
| Dropdown | Mid Dark | `#242933` | Menu dropdown |

### 3.2 Màu Accent & Primary

| Type | Màu | HEX Code | Sử dụng |
|------|-----|----------|---------|
| Primary | Blue | `#4A90E2` | Nút chính, link |
| Primary Hover | Dark Blue | `#3a7bc8` | Hover state |
| Primary Pressed | Darker Blue | `#2a5a8a` | Active state |
| Gradient Start | Light Blue | `#5ba3f5` | Gradient bắt đầu |
| Gradient End | Mid Blue | `#3d7dc4` | Gradient kết thúc |
| MS Blue | Microsoft Blue | `#0078D4` | Nút thêm (Add) |
| MS Blue Hover | Dark MS Blue | `#106EBE` | Hover |
| MS Blue Press | Darker MS Blue | `#005A9E` | Pressed |

### 3.3 Màu Semantic (Ý nghĩa)

| Type | Màu | HEX Code | Sử dụng |
|------|-----|----------|---------|
| Success | Teal | `#1BC5BD` | Nút xác nhận, thành công |
| Success Alt | Green | `#48bb78` | Alternative success |
| Success Dark | Dark Teal | `#0e9d95` | Hover success |
| Success Light | Light Green | `#5cd68d` | Label accent |
| Warning | Orange | `#f6ad55` | Cảnh báo |
| Warning Bright | Bright Orange | `#ffb84d` | Alert border, title |
| Danger | Red | `#F64E60` | Xóa, lỗi |
| Danger Hover | Dark Red | `#d32f2f` | Hover danger |
| Danger Alt | Light Red | `#fc8181` | Alternative danger |
| Danger Dark | Dark Red 2 | `#C42B1C` | Delete button |
| Danger Hover 2 | Darker Red | `#A80000` | Delete hover |
| Danger Alert | Bright Red | `#ff6b6b` | Alert border |

### 3.4 Màu Text

| Type | Màu | HEX Code | Sử dụng |
|------|-----|----------|---------|
| Primary Text | Bright White | `#e8eef5` | Text chính |
| White | Pure White | `#FFFFFF` | Tiêu đề, nút text |
| Secondary Text | Gray | `#a0aec0` | Text phụ |
| Muted Text | Light Gray | `#B5B5C3` | Text nhạt |
| Disabled Text | Dark Gray | `#6C6C80` | Text disabled |
| Input Text | Light | `#E0E0E0` | Text trong input |
| Light Text | Very Light | `#f0f4f8` | Text sáng |
| Info Text | Blue Gray | `#b8c5d6` | Text thông tin |
| Dark Text | Very Dark | `#666` | Text disabled alt |
| Subtle Text | Dark Subtle | `#CCCCCC` | Text tinh tế |
| Result Text | Light Subtle | `#CFCFCF` | Kết quả tìm kiếm |

### 3.5 Màu Border & Divider

| Type | Màu | HEX Code | Sử dụng |
|------|-----|----------|---------|
| Border Primary | Mid Gray | `#424959` | Border chính |
| Border Dark | Very Dark | `#1a2332` | Border tối |
| Border Subtle | Dark Border | `#141920` | Border tinh tế |
| Border Input | Dark Input | `#151820` | Border input |
| Border Light | Light Gray | `#3E3E42` | Border sáng hơn |
| Separator Line | Dark Line | `#0f161d` | Đường phân cách |
| Gridline | Transparent | `rgba(255,255,255,0.05)` | Đường lưới bảng |

### 3.6 Màu Selection & Hover

| State | Màu | HEX Code | Sử dụng |
|-------|-----|----------|---------|
| Selected Row | Blue Gray | `#4A7BA7` | Hàng được chọn |
| Hover Row | Dark Hover | `#2f4558` | Hover trên hàng |
| Hover Button | Dark BG | `#3E3E42` | Hover nút secondary |
| Hover Input | Mid Dark | `#2d3541` | Hover input |
| Selection BG | Primary Blue | `#4A90E2` | Dropdown selection |

---

## 4. FONT VÀ TYPOGRAPHY

### 4.1 Font Family

```css
Primary Font:          "Segoe UI", "Roboto", sans-serif
Alternative:           "Inter", system-ui, -apple-system
Fallback:              sans-serif
```

### 4.2 Font Sizes

| Element | Size | Weight | Sử dụng |
|---------|------|--------|---------|
| Page Title | 24px | 700 (Bold) | Tiêu đề trang |
| Section Title | 18px | 600 (Semi-bold) | Tiêu đề section |
| Card Title | 14px | 600 (Semi-bold) | Tiêu đề card |
| Body Text | 13px | 400-500 | Text chính |
| Button Text | 13px | 700 (Bold) | Text nút |
| Input Text | 13px | 500 | Text input |
| Table Header | 11-12px | 700 (Bold) | Header bảng |
| Small Text | 11-12px | 400 | Text nhỏ |
| Caption | 11px | 400 | Chú thích |
| Hint Text | 11px | italic | Gợi ý |

### 4.3 Text Styling

```css
Table Header:
  - text-transform: uppercase
  - letter-spacing: 0.5px - 1.2px
  
Bold Labels:
  - font-weight: 600-700
  
Link/Accent Text:
  - color: #5ba3f5 or #4A90E2
```

---

## 5. KÍCH THƯỚC NÚT VÀ SPACING

### 5.1 Kích Thước Nút (Buttons)

#### Standard Buttons
```css
Padding:               10px 20px
Border-radius:         8px
Min-height:            không xác định (auto)
Font-size:             13px
Font-weight:           700 (Bold)
```

#### Compact Buttons
```css
Padding:               6px 12px
Border-radius:         6px
Min-height:            34px
Font-size:             13px
Font-weight:           600
```

#### Small Action Buttons (Header)
```css
Padding:               6px 12px
Border-radius:         4px
Font-weight:           600
```

#### Icon Buttons
```css
Min-width:             40px
Min-height:            40px
Padding:               0
Border-radius:         8px
```

#### Icon Buttons with Text
```css
Min-width:             90px
Min-height:            40px
Padding:               6px 12px
Icon Size:             18px x 18px
```

### 5.2 Input Fields

```css
Padding:               8px - 12px
Border-radius:         5px - 8px
Border:                1px - 2px solid
Min-height:            40px (Search bars)
Font-size:             13px
```

### 5.3 Tables

```css
Cell Padding:          6px - 8px
Header Padding:        10px - 14px (vertical) 8px - 12px (horizontal)
Border-radius:         6px
```

### 5.4 Cards & Containers

```css
Card Padding:          12px - 18px
Card Border-radius:    8px - 12px
Card Margin-bottom:    8px - 15px
```

### 5.5 Layout Spacing

```css
Main Layout Margin:    30px (all sides)
Layout Spacing:        20px
Section Spacing:       15px - 20px
GroupBox Padding:      15px
```

---

## 6. FILE STYLESHEET SỬ DỤNG

### 6.1 File Chính: `modern-dashboard.qss`

**Vị trí:** `resources/modern-dashboard.qss`  
**Số dòng:** 1020 dòng  
**Mô tả:** Theme tối chính đang được sử dụng cho toàn bộ ứng dụng

**Các class đặc biệt:**
- `.WidgetBangDieuKhien` - Dashboard widget
- `.WidgetQuanLySach` - Book management
- `.WidgetQuanLyDocGia` - Reader management
- `.WidgetMuonTraSach` - Borrow/Return
- `QFrame[class="Card"]` - Card containers
- `QTableView[class="InnerTable"]` - Tables
- `QLabel[class="SelectedInfo"]` - Selected reader info
- `QPushButton[class="Primary"]` - Primary buttons
- `QPushButton[class="Success"]` - Success buttons
- `QPushButton[class="Danger"]` - Danger buttons
- `QPushButton[class="IconButton"]` - Icon buttons
- `QGroupBox[class="GroupBoxProfile"]` - Profile groups (Orange title)
- `QGroupBox[class="GroupBoxBooks"]` - Books groups (Blue title)
- `QGroupBox[class="GroupBoxInput"]` - Input groups (Green title)

### 6.2 File Thứ Hai: `enterprise-theme.qss`

**Vị trí:** `resources/enterprise-theme.qss`  
**Số dòng:** 621 dòng  
**Mô tả:** Theme tối Enterprise Professional (Backup theme)

**Đặc điểm:**
- Màu sidebar: `#1a1f2e` (giống modern-dashboard)
- Màu chính: `#232938`
- Tương tự modern-dashboard nhưng ít customization hơn

### 6.3 File Thứ Ba: `styles.qss`

**Vị trí:** `resources/styles.qss`  
**Số dòng:** 532 dòng  
**Mô tả:** Theme sáng - Material Modern (Không dùng)

**Màu chính:**
- Primary: `#2563EB` (Blue)
- Success: `#10B981` (Green)
- Danger: `#DC2626` (Red)
- Background: `#F8FAFC` (Light gray)
- White backgrounds cho card, table, input

### 6.4 Inline Styles trong Code

Các widget cũng sử dụng **inline styles** trực tiếp trong code C++:

**Ví dụ trong `WidgetQuanLySach.cpp`:**
```cpp
nutThemSach->setStyleSheet(
    "QPushButton { background-color: #0078D4; color: white; ... }"
);
```

**Ví dụ trong `BangDieuKhien.cpp`:**
```cpp
setStyleSheet(
    "QWidget#BangDieuKhien {"
    "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
    "        stop:0 #1a1d29, stop:0.5 #1e2230, stop:1 #1a1d29);"
    "}"
);
```

---

## 7. SO SÁNH MÀU GIỮA CÁC TAB

### 7.1 Bảng So Sánh Chi Tiết

| Element | Trang chủ | Độc giả | Mượn trả | Sách |
|---------|-----------|---------|----------|------|
| **Background Widget** | `#1a1d29` gradient | `#232938` | `#232938` | `#232938` |
| **Table Background** | `#1a1f2e` | `#2c3e50` ✓ | `#2c3e50` ✓ | `#2c3e50` ✓ |
| **Table Alternate** | `#1e2633` | `#273747` ✓ | `#273747` ✓ | `#273747` ✓ |
| **Table Header** | `#141b26` | `#34495e` ✓ | `#34495e` ✓ | `#34495e` ✓ |
| **Header Text** | `#a0aec0` | `#b8c5d6` ✓ | `#b8c5d6` ✓ | `#b8c5d6` ✓ |
| **Primary Button** | `#4A90E2` | `#0078D4` | `#1BC5BD` | `#0078D4` |
| **Selected Row** | `#4A7BA7` | `#4A7BA7` ✓ | `#4A7BA7` ✓ | `#4A7BA7` ✓ |

**✓ = Màu giống nhau**

### 7.2 Nhận Xét Quan Trọng

**🔍 Phát hiện:** Tab **Độc giả**, **Mượn trả**, và **Sách** đều sử dụng **CÙNG MÀU BẢNG**:
- Nền: `#2c3e50` (Deep slate blue)
- Alternating: `#273747`
- Header: `#34495e`

Màu này được định nghĩa trong [modern-dashboard.qss](resources/modern-dashboard.qss#L244) với selector mạnh:
```css
QWidget#WidgetQuanLySach QTableView,
QWidget#WidgetQuanLyDocGia QTableView {
    background-color: #2c3e50;
    alternate-background-color: #273747;
}
```

Mặc dù code có set class `"InnerTable"` với màu tối hơn (`#1a1f2e`), nhưng **bị override** bởi selector theo object name.

**Chỉ có tab Trang chủ** sử dụng màu bảng khác (tối hơn - `#1a1f2e`).

---

## 8. THỐNG KÊ TỔNG HỢP

### 8.1 Số Lượng Màu Sử Dụng

| Loại | Số lượng |
|------|----------|
| Màu nền (Background) | 15+ |
| Màu text | 12+ |
| Màu accent/primary | 8+ |
| Màu semantic (Success/Warning/Danger) | 12+ |
| Màu border | 7+ |
| **Tổng cộng** | **~55 màu khác nhau** |

### 7.2 Màu Được Dùng Nhiều Nhất

1. **#232938** - Background chính (Tất cả các tab trừ Trang chủ)
2. **#2c3e50** - Table background (3 tab: Độc giả, Mượn trả, Sách) ⭐
3. **#4A90E2** - Primary blue (Nút, accent, selection)
4. **#1a1f2e** - Sidebar và table Trang chủ
5. **#e8eef5** - Text chính
6. **#353b4a** - Card container
7. **#424959** - Border chính
8. **#1BC5BD** - Success action
9. **#F64E60** - Danger action

⭐ **Màu #2c3e50** được dùng cho tất cả bảng ở 3 tab quản lý chính!

### 7.3 Font Size Chính

- **13px** - Sử dụng nhiều nhất (body, button, input)
- **24px** - Tiêu đề trang
- **11-12px** - Text nhỏ, hint, header table

### 7.4 Border Radius Phổ Biến

- **8px** - Standard (Nút, card)
- **6px** - Compact (Compact button, small elements)
- **4px** - Small (Secondary buttons)
- **12px** - Large (Dialog, large card)

---

## 8. KẾT LUẬN & KHUYẾN NGHỊ

### 9.1 Điểm Mạnh

✅ **Nhất quán màu sắc:** Hệ thống màu được thiết kế nhất quán với palette rõ ràng  
✅ **Hierarchy rõ ràng:** Phân cấp màu sắc giúp phân biệt các element quan trọng  
✅ **Dark theme chuyên nghiệp:** Phù hợp cho môi trường làm việc lâu dài  
✅ **Semantic colors:** Màu có ý nghĩa rõ ràng (xanh = primary, đỏ = danger, etc)  
✅ **Bảng đồng nhất:** 3 tab quản lý (Độc giả, Sách, Mượn trả) dùng cùng màu bảng → UX nhất quán
9.2 Cần Cải Thiện

⚠️ **Quá nhiều inline styles:** Nên chuyển về stylesheet để dễ maintain  
⚠️ **3 file stylesheet:** Có thể gộp hoặc chỉ giữ 1 file chính  
⚠️ **Một số màu hardcoded:** Nên dùng CSS variables hoặc constants  
⚠️ **Contrast ratio:** Một số màu text cần kiểm tra accessibility  
⚠️ **Override không cần thiết:** Class "InnerTable" bị override hoàn toàn bởi selector theo object names  
⚠️ *9.3 Đề Xuất

1. **Tạo file colors.qss riêng** để định nghĩa biến màu
2. **Sử dụng CSS Custom Properties** (nếu Qt hỗ trợ)
3. **Remove theme không dùng** (styles.qss nếu không cần theme sáng)
4. **Consolidate inline styles** về stylesheet chính
5. **Document màu sắc trong code** với comments rõ ràng
6. **Loại bỏ class "InnerTable"** nếu không dùng, hoặc xóa override selector mạnh hơn
7. **Tách riêng màu Trang chủ** nếu muốn giữ khác biệtn theme sáng)
4. **Consolidate inline styles** về stylesheet chính
5. **Document màu sắc trong code** với comments rõ ràng

---

## 9. BẢNG TRA CỨU NHANH

### Màu Theo Tab

| Tab | Background | Primary Action | Table BG⭐ | `#34495e` ⭐ |
| Mượn trả | `#232938` | `#1BC5BD` (Mượn) | `#2c3e50` ⭐ | `#34495e` ⭐ |
| Sách | `#232938` | `#0078D4` | `#2c3e50` ⭐ | `#34495e` ⭐ |

⭐ = **3 tab này dùng cùng màu bảng** | `#141b26` |
| Độc giả | `#232938` | `#0078D4` | `#2c3e50` | `#34495e` |
| Mượn trả | `#232938` | `#1BC5BD` (Mượn) | `#1a1f2e` | `#141b26` |
| Sách | `#232938` | `#0078D4` | `#1a1f2e` | `#141b26` |

### Màu Theo Hành Động

| Hành động | Màu | HEX | Hover |
|-----------|-----|-----|-------|
| Thêm/Tạo mới | Blue | `#0078D4` | `#106EBE` |
| Xác nhận/Lưu | Teal | `#1BC5BD` | `#0e9d95` |
| Xóa/Hủy | Red | `#C42B1C` | `#A80000` |
| Tìm kiếm | Light Blue | `#5ba3f5` | `#4a93e3` |
| Làm mới | Gray | `#242933` | `#2d3541` |

---

**Người lập báo cáo:** Hệ thống phân tích tự động  
**Nguồn dữ liệu:** 
- `resources/modern-dashboard.qss` (1020 dòng)
- `resources/enterprise-theme.qss` (621 dòng)
- `resources/styles.qss` (532 dòng)
- `src/ui/*.cpp` (Inline styles)

**Ghi chú:** Báo cáo này phân tích đầy đủ màu sắc, font, kích thước được sử dụng trong 4 tab chính của ứng dụng Quản lý thư viện.
