# Hướng Dẫn Đóng Gói Ứng Dụng Qt

## 📦 Cách Đóng Gói Ứng Dụng Để Chuyển Sang Máy Khác

### Bước 1: Build Ứng Dụng ở Chế Độ Release

1. Build ứng dụng ở chế độ Release:
```powershell
# Chạy qmake
C:/Qt/6.10.1/mingw_64/bin/qmake.exe QuanLyThuVien_V2.pro -spec win32-g++ "CONFIG+=release"

# Compile
C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe release
```

Hoặc dùng task có sẵn trong VS Code:
- Nhấn `Ctrl+Shift+P`
- Chọn `Tasks: Run Task`
- Chọn `Build & Compile Qt (Full Build)`

### Bước 2: Thu Thập Dependencies với windeployqt

Sau khi build xong, file `.exe` sẽ nằm trong thư mục `release/`. Bạn cần copy tất cả DLL cần thiết:

```powershell
# Di chuyển vào thư mục release
cd release

# Chạy windeployqt để tự động copy các DLL Qt cần thiết
C:/Qt/6.10.1/mingw_64/bin/windeployqt.exe LibraryManagement.exe --release --no-translations
```

Lệnh này sẽ tự động:
- Copy tất cả Qt DLL cần thiết (Qt6Core.dll, Qt6Gui.dll, Qt6Widgets.dll, etc.)
- Copy plugins cần thiết (platforms/, styles/, imageformats/)
- Copy các file Qt platform abstraction

### Bước 3: Copy MinGW Runtime DLLs

Ngoài Qt DLLs, bạn còn cần copy các MinGW runtime DLLs:

```powershell
# Copy các DLL MinGW runtime vào thư mục release
copy C:\Qt\Tools\mingw1310_64\bin\libgcc_s_seh-1.dll release\
copy C:\Qt\Tools\mingw1310_64\bin\libstdc++-6.dll release\
copy C:\Qt\Tools\mingw1310_64\bin\libwinpthread-1.dll release\
```

### Bước 4: Copy Dữ Liệu và Resources

Copy thư mục `data/` (chứa dữ liệu) vào thư mục release:

```powershell
# Copy thư mục data
xcopy /E /I data release\data
```

### Bước 5: Tạo Cấu Trúc Thư Mục Cuối Cùng

Cấu trúc thư mục release cuối cùng sẽ như sau:
```
release/
├── LibraryManagement.exe          (File thực thi chính)
├── Qt6Core.dll
├── Qt6Gui.dll
├── Qt6Widgets.dll
├── Qt6PrintSupport.dll
├── Qt6Charts.dll
├── libgcc_s_seh-1.dll
├── libstdc++-6.dll
├── libwinpthread-1.dll
├── platforms/
│   └── qwindows.dll
├── styles/
│   └── qwindowsvistastyle.dll
├── imageformats/
│   ├── qgif.dll
│   ├── qjpeg.dll
│   └── qpng.dll
└── data/
    ├── danhmucsach.txt
    ├── dausach.txt
    ├── docgia.txt
    └── muontra.txt
```

### Bước 6: Tạo Script Tự Động Đóng Gói

Đã tạo file `deploy.ps1` để tự động hóa quá trình đóng gói!

## 🚀 CÁCH SỬ DỤNG NHANH

### Phương Án 1: Dùng Script Tự Động (Khuyên Dùng)

```powershell
# 1. Build ứng dụng ở chế độ Release
C:/Qt/6.10.1/mingw_64/bin/qmake.exe QuanLyThuVien_V2.pro -spec win32-g++ "CONFIG+=release"
C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe release

# 2. Chạy script đóng gói
.\deploy.ps1
```

Script sẽ tự động:
- ✓ Copy file .exe
- ✓ Thu thập tất cả Qt DLLs
- ✓ Copy MinGW runtime DLLs
- ✓ Copy dữ liệu và resources
- ✓ Tạo thư mục portable hoàn chỉnh
- ✓ Tạo file hướng dẫn sử dụng

Kết quả: Thư mục `QuanLyThuVien_Portable` chứa toàn bộ ứng dụng!

### Phương Án 2: Thủ Công (Chi Tiết Hơn)

Nếu bạn muốn kiểm soát từng bước:

```powershell
# Bước 1: Build Release
C:/Qt/6.10.1/mingw_64/bin/qmake.exe QuanLyThuVien_V2.pro -spec win32-g++ "CONFIG+=release"
C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe release

# Bước 2: Tạo thư mục deploy
New-Item -ItemType Directory -Path "QuanLyThuVien_Deploy" -Force

# Bước 3: Copy exe
Copy-Item "release\LibraryManagement.exe" "QuanLyThuVien_Deploy\"

# Bước 4: Chạy windeployqt
cd QuanLyThuVien_Deploy
C:/Qt/6.10.1/mingw_64/bin/windeployqt.exe LibraryManagement.exe --release --no-translations

# Bước 5: Copy MinGW DLLs
Copy-Item "C:\Qt\Tools\mingw1310_64\bin\libgcc_s_seh-1.dll" .
Copy-Item "C:\Qt\Tools\mingw1310_64\bin\libstdc++-6.dll" .
Copy-Item "C:\Qt\Tools\mingw1310_64\bin\libwinpthread-1.dll" .

# Bước 6: Copy dữ liệu
cd ..
Copy-Item -Recurse "data" "QuanLyThuVien_Deploy\"
```

## 📤 Chuyển Sang Máy Khác

### Cách 1: Nén File ZIP

```powershell
# Nén thư mục thành file ZIP
Compress-Archive -Path "QuanLyThuVien_Portable" -DestinationPath "QuanLyThuVien_v1.0.zip"
```

Gửi file ZIP cho người khác, họ chỉ cần:
1. Giải nén
2. Chạy file `LibraryManagement.exe`
3. Không cần cài đặt gì thêm!

### Cách 2: USB/Ổ Đĩa Mạng

Copy toàn bộ thư mục `QuanLyThuVien_Portable` sang USB hoặc ổ đĩa mạng.

## 🔧 Xử Lý Lỗi Thường Gặp

### Lỗi: "The program can't start because Qt6Core.dll is missing"

**Nguyên nhân:** Chưa chạy windeployqt hoặc thiếu Qt DLLs

**Giải pháp:**
```powershell
cd QuanLyThuVien_Portable
C:/Qt/6.10.1/mingw_64/bin/windeployqt.exe LibraryManagement.exe --release
```

### Lỗi: "Entry Point Not Found" hoặc lỗi MinGW

**Nguyên nhân:** Thiếu MinGW runtime DLLs

**Giải pháp:** Copy thủ công các DLL:
```powershell
Copy-Item "C:\Qt\Tools\mingw1310_64\bin\libgcc_s_seh-1.dll" QuanLyThuVien_Portable\
Copy-Item "C:\Qt\Tools\mingw1310_64\bin\libstdc++-6.dll" QuanLyThuVien_Portable\
Copy-Item "C:\Qt\Tools\mingw1310_64\bin\libwinpthread-1.dll" QuanLyThuVien_Portable\
```

### Lỗi: Không tìm thấy file dữ liệu

**Nguyên nhân:** Thiếu thư mục `data/`

**Giải pháp:**
```powershell
Copy-Item -Recurse "data" "QuanLyThuVien_Portable\"
```

## 📋 Checklist Trước Khi Phân Phối

- [ ] Build ở chế độ Release (không phải Debug!)
- [ ] Chạy windeployqt để thu thập Qt DLLs
- [ ] Copy đủ 3 MinGW runtime DLLs
- [ ] Copy thư mục `data/` với dữ liệu mẫu
- [ ] Test trên máy sạch (không cài Qt)
- [ ] Kiểm tra kích thước file (nên < 100MB)
- [ ] Tạo file README hướng dẫn sử dụng

## 💡 Mẹo Nâng Cao

### Tạo Installer với Inno Setup

Nếu bạn muốn tạo file cài đặt chuyên nghiệp:

1. Tải Inno Setup: https://jrsoftware.org/isinfo.php
2. Tạo script `.iss` để đóng gói ứng dụng
3. Tạo Start Menu shortcuts, Desktop icons, etc.

### Giảm Kích Thước Ứng Dụng

```powershell
# Sử dụng UPX để nén file exe (tùy chọn)
upx --best LibraryManagement.exe

# Loại bỏ debug symbols
strip LibraryManagement.exe
```

### Ký Số Ứng Dụng (Code Signing)

Để tránh cảnh báo Windows SmartScreen, bạn nên ký số ứng dụng với certificate hợp lệ.

## 🎯 Yêu Cầu Hệ Thống Người Dùng

- **OS:** Windows 7 SP1 trở lên (64-bit)
- **RAM:** Tối thiểu 512 MB
- **Dung lượng:** Khoảng 80-100 MB
- **Màn hình:** Độ phân giải tối thiểu 1024x768

## 📞 Hỗ Trợ

Nếu gặp vấn đề khi đóng gói, hãy kiểm tra:
1. Đường dẫn Qt có đúng không
2. Đã build ở chế độ Release chưa
3. File .exe có chạy được trên máy dev không

