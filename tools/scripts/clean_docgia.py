"""
Script để xóa các hàm duplicate trong DocGia.cpp
"""

input_file = r"e:\Qt\QuanLyThuVien\src\models\DocGia.cpp"
output_file = r"e:\Qt\QuanLyThuVien\src\models\DocGia_cleaned.cpp"

# Đọc file
with open(input_file, 'r', encoding='utf-8') as f:
    lines = f.readlines()

# Định nghĩa các hàm cần xóa (start line, end line) - 1-indexed
functions_to_delete = [
    # Orphaned code và các hàm helper
    (625, 703),   # Orphaned code + TimBanSaoTheoMa + DemBanSaoCoTheMuon
    (706, 846),   # MuonSach
    (848, 945),   # TraSach  
    (947, 1005),  # themMuonTra (2 overloads)
    (1016, 1090), # LayDSSachDangMuon
    (1092, 1109), # TinhSoNgayQuaHanLonNhat 
    (1286, 1336), # BaoMatSach
]

# Convert to 0-indexed
functions_to_delete = [(start-1, end) for start, end in functions_to_delete]

# Tạo set các dòng cần xóa
lines_to_delete = set()
for start, end in functions_to_delete:
    for i in range(start, end):
        lines_to_delete.add(i)

# Ghi file mới, bỏ qua các dòng cần xóa
with open(output_file, 'w', encoding='utf-8') as f:
    for i, line in enumerate(lines):
        if i not in lines_to_delete:
            f.write(line)

print(f"Cleaned file written to: {output_file}")
print(f"Deleted {len(lines_to_delete)} lines")
print(f"Original: {len(lines)} lines")
print(f"New: {len(lines) - len(lines_to_delete)} lines")
