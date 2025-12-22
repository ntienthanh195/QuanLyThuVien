# HE THONG QUAN LY THU VIEN (V2.5)

Phien ban: 2.5 (Final Release)
Ngay cap nhat: 21/12/2025
Trang thai: San sang trien khai (Production Ready)

---

## 1. GIOI THIEU CHUNG

Phan mem Quan Ly Thu Vien V2 la giai phap phan mem tong the duoc thiet ke de tu dong hoa hoan toan cac quy trinh nghiep vu tai thu vien quy mo vua va lon. He thong tap trung vao do on dinh, toc do xu ly du lieu lon va kha nang tu dong hoa cao giup giam thieu sai sot cua con nguoi.

Du an duoc xay dung tren nen tang cong nghe C++ va Qt Framework, mang lai hieu nang vuot troi so voi cac giai phap dua tren Web hoac .NET truyen thong.

---

## 2. TINH NANG CHI TIET

### A. Quan Ly Muon Tra (Smart Circulation)
Day la module cot loi cua he thong, duoc toi uu cho quy trinh lam viec lien tuc:
- Muon tra tu dong: Ho tro may quet ma vach (Barcode Scanner). He thong tu dong phan tich ma dau vao de xac dinh la Ma The Doc Gia hay Ma Sach.
- Validation chat che: Tu dong chan muon neu doc gia co sach qua han, no phat chua dong, hoac the het han.
- Tinh toan tien phat: Tu dong tinh toan so tien phat dua tren so ngay qua han va don gia duoc cau hinh (Ho tro tinh nang an xa/giam gia).
- Xac nhan am thanh: Tich hop phan hoi am thanh (Beep/Warning) giup thu thu lam viec khong can nhin man hinh.

### B. Kiem Ke Kho (Inventory Audit) - MOI
Giai phap kiem ke kho chuyen nghiep, tach biet hoan toan voi quy trinh muon tra:
- Che do quet toc do cao: Chap nhan du lieu tu may quet lien tuc ma khong can thao tac chuot.
- Phat hien sai lech thong minh:
  + Mat sach (Missing): Sach co trong he thong nhung khong tim thay tren ke.
  + Sach lau (Surplus): Sach co tren ke nhung khong co trong he thong.
  + Sai trang thai (Wrong Status): Sach he thong bao "Dang muon" nhung lai nam tren ke (Phat hien loi quy trinh).
- Xuat bao cao kiem ke: Tu dong xuat file PDF chi tiet voi day du so lieu chenh lech de ky duyet.

### C. Quan Ly Tai Chinh & Thong Ke
- Quan ly thu chi: Theo doi tien phat, in bien lai thu tien tu dong.
- Luu tru (Archiving): Co che tu dong chuyen du lieu muon tra cu (> 6 thang) sang kho luu tru rieng, giup database chinh luon nhe va nhanh.
- Bao cao da dang: Ho tro xuat bao cao ra PDF va Excel cho nhieu muc dich (Bao cao thang, Top doc gia, Tinh hinh sach hu hong).

### D. Quan Ly Doc Gia & Sach
- The doc gia: Tao va in the doc gia tich hop ma QR Code dinh danh.
- Import Excel Toc do cao: Tich hop thu vien Native C++ (QXlsx) cho phep nhap lieu 5.000 dau sach trong duoi 2 giay, tu dong loc trung lap.
- Chuan hoa du lieu: Tu dong sua loi chinh ta, chuan hoa ten rieng va so dien thoai.

---

## 3. CAU TRUC KI THUAT

D u an ap dung kien truc MVC (Model-View-Controller) chat che:

1. Tang Data Access (Model):
   - Su dung SQLite voi che do WALL (Write-Ahead Logging) cho hieu suat cao.
   - Bao mat tuyet doi chong SQL Injection bang Prepared Statements.
   - DatabaseSchema tu dong khoi tao va migrates khi nang cap phien ban.

2. Tang Nghiep Vu (Controller/Utils):
   - UserManager: Quan ly phien dang nhap, phan quyen (RBAC).
   - PrintHelper: Xu ly in an, xuat PDF, tao bao cao HTML template.
   - ExcelHelper: Xu ly nhap xuat du lieu Excel toc do cao.

3. Tang Giao Dien (View):
   - Su dung Qt Widgets duoc tuy bien giao dien (QSS) hien dai.
   - Ho tro da ngon ngu (Vietnamese / English / Khmer) thay doi tuc thi (Runtime retagging).
   - He thong Icon Vector (SVG) giup hien thi sac net tren man hinh do phan giai cao.

## 4. HUONG DAN CAI DAT VA BIEN DICH (BUILD)

### Yeu Cau He Thong
- He dieu hanh: Windows 10/11 (64-bit).
- Compiler: MinGW 11.2.0 hoac moi hon (GCC 13+).
- Qt Framework: Phien ban 6.x (Khuyen nghi 6.10).

### Cac Buoc Bien Dich (Tu Source Code)

Buoc 1: Chuan bi moi truong
Dam bao da cai dat Qt Creator va MinGW. Them duong dan `qmake` va `mingw32-make` vao bien moi truong PATH.

Buoc 2: Cau hinh Project
Mo Command Prompt tai thu muc du an va chay lenh:
qmake QuanLyThuVien_V2.pro "CONFIG+=release"

Buoc 3: Bien dich
Chay lenh:
mingw32-make

Buoc 4: Dong goi (Deploy)
Su dung script PowerShell di kem de tu dong gom cac thu vien DLL can thiet:
./tools/scripts/deploy.ps1

Ket qua se nam trong thu muc `release_v2/`.

---

## 5. THONG TIN BAN QUYEN

San pham thuoc so huu cua Chăm Rốch Thi.
Nghiem cam sao chep, phan phoi duoi hinh thuc thuong mai ma khong co su dong y bang van ban.

(c) 2025 Rocco & Team. All rights reserved.
