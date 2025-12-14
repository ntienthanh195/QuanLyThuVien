from openpyxl import Workbook
import csv
import os

root = r"e:\Qt\QuanLyThuVien"
files = [
    (os.path.join(root, 'data', 'import_sach.csv'), os.path.join(root, 'data', 'import_sach.xlsx'), 'DauSach'),
    (os.path.join(root, 'data', 'import_docgia.csv'), os.path.join(root, 'data', 'import_docgia.xlsx'), 'DocGia')
]

results = []
for csv_path, xlsx_path, sheet_name in files:
    try:
        if not os.path.exists(csv_path):
            results.append((csv_path, 'MISSING'))
            continue
        wb = Workbook()
        ws = wb.active
        ws.title = sheet_name
        with open(csv_path, 'r', encoding='utf-8-sig', newline='') as f:
            reader = csv.reader(f)
            for row in reader:
                if row and isinstance(row[0], str) and row[0].startswith('\u000b'):
                    row[0] = row[0].lstrip('\u000b')
                cleaned = [ (c if c is not None else '') for c in row ]
                ws.append(cleaned)
        wb.save(xlsx_path)
        results.append((csv_path, xlsx_path, 'OK'))
    except Exception as e:
        results.append((csv_path, 'ERROR', str(e)))

for r in results:
    print(r)
