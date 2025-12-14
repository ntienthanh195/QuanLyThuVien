#!/usr/bin/env python3
"""
Apply sample CSV entries into data/files/library.db.
- Backs up DB before changes.
- Inserts new doc_gia rows (skips if ma_the exists).
- Inserts dau_sach if isbn not exists, and create ban_sao copies using 'tong_ban_sao' column.
"""
import os, csv, sqlite3, shutil, datetime, re

root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
data_dir = os.path.join(root, 'data')
db_dir = os.path.join(data_dir, 'files')
db_path = os.path.join(db_dir, 'library.db')

if not os.path.exists(db_path):
    print('DB not found at', db_path)
    raise SystemExit(1)

# Backup
ts = datetime.datetime.now().strftime('%Y%m%d_%H%M%S')
backup = os.path.join(db_dir, f'library.db.bak_apply_{ts}')
shutil.copy2(db_path, backup)
print('Backup created:', backup)

con = sqlite3.connect(db_path)
cur = con.cursor()
con.execute('PRAGMA foreign_keys = ON')

# Helpers
def normalize_isbn(raw):
    if raw is None: return ''
    s = re.sub(r"[^0-9Xx]", "", str(raw))
    return s.upper()

inserted_docgia = 0
inserted_dausach = 0
inserted_bansao = 0

# Insert doc_gia from CSV
docgia_csv = os.path.join(data_dir, 'import_docgia.csv')
if os.path.exists(docgia_csv):
    with open(docgia_csv, 'r', encoding='utf-8-sig', newline='') as f:
        reader = csv.reader(f)
        rows = list(reader)
    if rows:
        headers = [h.strip() for h in rows[0]]
        for r in rows[1:]:
            if not r: continue
            # handle stray control
            if r and isinstance(r[0], str) and r[0].startswith('\u000b'):
                r[0] = r[0].lstrip('\u000b')
            data = {headers[i]: (r[i] if i < len(r) else '') for i in range(len(headers))}
            try:
                ma_the = int(data.get('ma_the') or data.get('ma') or 0)
            except Exception:
                continue
            if ma_the == 0:
                continue
            cur.execute('SELECT 1 FROM doc_gia WHERE ma_the=?', (ma_the,))
            if cur.fetchone():
                continue
            ho = data.get('ho','').strip()
            ten = data.get('ten','').strip()
            phai = int(data.get('phai') or 0)
            trang_thai = int(data.get('trang_thai') or 1)
            so_dt = data.get('so_dien_thoai') or None
            email = data.get('email') or None
            ngay_dk = data.get('ngay_dang_ky') or None
            cur.execute('INSERT INTO doc_gia(ma_the, ho, ten, phai, trang_thai, so_dien_thoai, email, ngay_dang_ky) VALUES(?,?,?,?,?,?,?,?)',
                        (ma_the, ho, ten, phai, trang_thai, so_dt, email, ngay_dk))
            inserted_docgia += 1
    else:
        print('No rows in', docgia_csv)
else:
    print('Docgia CSV not found:', docgia_csv)

# Insert dau_sach and ban_sao from CSV
dausach_csv = os.path.join(data_dir, 'import_sach.csv')
if os.path.exists(dausach_csv):
    with open(dausach_csv, 'r', encoding='utf-8-sig', newline='') as f:
        reader = csv.reader(f)
        rows = list(reader)
    if rows:
        headers = [h.strip() for h in rows[0]]
        for r in rows[1:]:
            if not r: continue
            if r and isinstance(r[0], str) and r[0].startswith('\u000b'):
                r[0] = r[0].lstrip('\u000b')
            data = {headers[i]: (r[i] if i < len(r) else '') for i in range(len(headers))}
            isbn_raw = data.get('isbn') or data.get('ISBN') or ''
            isbn = normalize_isbn(isbn_raw)
            title = data.get('ten_sach') or data.get('title') or ''
            try:
                pages = int(data.get('so_trang') or 0)
            except Exception:
                pages = 0
            author = data.get('tac_gia') or data.get('author') or ''
            try:
                year = int(data.get('nam_xb') or 0)
            except Exception:
                year = 0
            genre = data.get('the_loai') or ''
            location = data.get('location') or data.get('vi_tri') or ''
            try:
                copies = int(data.get('tong_ban_sao') or data.get('total_copies') or 1)
            except Exception:
                copies = 1

            if not isbn:
                # skip rows without ISBN to avoid inserting broken data
                continue
            # check if dau_sach exists
            cur.execute('SELECT id FROM dau_sach WHERE isbn=?', (isbn,))
            row = cur.fetchone()
            if row:
                dau_id = row[0]
            else:
                cur.execute('INSERT INTO dau_sach(isbn, ten_sach, so_trang, tac_gia, nam_xuat_ban, the_loai) VALUES(?,?,?,?,?,?)',
                            (isbn, title, pages, author, year, genre))
                dau_id = cur.lastrowid
                inserted_dausach += 1
            # Insert ban_sao copies
            # find existing count of copies for this isbn to avoid ma_sach collisions
            cur.execute('SELECT COUNT(*) FROM ban_sao WHERE dau_sach_id=?', (dau_id,))
            existing = cur.fetchone()[0]
            for i in range(1, copies+1):
                seq = existing + i
                ma_sach = f"{isbn}-{seq:03d}"
                # ensure unique ma_sach
                cur.execute('SELECT 1 FROM ban_sao WHERE ma_sach=?', (ma_sach,))
                if cur.fetchone():
                    # if exists, find next available seq
                    seq2 = seq
                    while True:
                        seq2 += 1
                        ma_sach = f"{isbn}-{seq2:03d}"
                        cur.execute('SELECT 1 FROM ban_sao WHERE ma_sach=?', (ma_sach,))
                        if not cur.fetchone():
                            break
                cur.execute('INSERT INTO ban_sao(ma_sach, dau_sach_id, trang_thai, vi_tri) VALUES(?,?,?,?)',
                            (ma_sach, dau_id, 0, location))
                inserted_bansao += 1
    else:
        print('No rows in', dausach_csv)
else:
    print('Dausach CSV not found:', dausach_csv)

con.commit()
con.close()
print(f'Inserted docgia: {inserted_docgia}, dausach: {inserted_dausach}, bansao: {inserted_bansao}')
print('Done')
