#!/usr/bin/env python3
"""
Import from Excel (.xlsx) to prepare data for QuanLyThuVien.
- Reads sheets 'DauSach' and 'BanSao' (or fallbacks)
- Validates ISBN, year, numeric fields
- Produces preview CSVs and an import text file for `dausach` format
- Safe by default: use --apply to write import files into data/ (does NOT modify DB directly)

Usage:
  python tools/import_from_excel.py --file books.xlsx --mode merge --preview
  python tools/import_from_excel.py --generate-sample --preview
  python tools/import_from_excel.py --file books.xlsx --apply --mode merge

Modes when encountering existing ISBN: skip | merge | replace
"""
import argparse
import csv
import datetime
import os
import re
import sys
import sqlite3
import shutil
import time

try:
    from openpyxl import Workbook, load_workbook
except Exception as e:
    print("Missing dependency 'openpyxl'. Run: pip install openpyxl")
    raise


def normalize_isbn(raw: str) -> str:
    s = re.sub(r"[^0-9Xx]", "", str(raw))
    # uppercase X
    s = s.upper()
    return s


def is_valid_isbn(isbn: str) -> bool:
    isbn = isbn.strip().upper()
    if len(isbn) == 10:
        # ISBN-10 checksum
        if not re.match(r"^[0-9]{9}[0-9X]$", isbn):
            return False
        total = 0
        for i, ch in enumerate(isbn[:9]):
            total += (i + 1) * int(ch)
        check = total % 11
        check_char = 'X' if check == 10 else str(check)
        return isbn[-1] == check_char
    elif len(isbn) == 13:
        if not re.match(r"^[0-9]{13}$", isbn):
            return False
        total = 0
        for i, ch in enumerate(isbn[:12]):
            n = int(ch)
            total += n if i % 2 == 0 else n * 3
        check = (10 - (total % 10)) % 10
        return int(isbn[-1]) == check
    else:
        return False


def read_sheet_as_dicts(wb, sheet_name):
    if sheet_name in wb.sheetnames:
        ws = wb[sheet_name]
    else:
        # fallback to first sheet
        ws = wb[wb.sheetnames[0]]
    rows = list(ws.values)
    if not rows:
        return []
    headers = [str(h).strip() if h is not None else "" for h in rows[0]]
    data = []
    for r in rows[1:]:
        row = {headers[i] if i < len(headers) else f"col{i}": (r[i] if i < len(r) else None) for i in range(len(headers))}
        data.append(row)
    return data


def generate_sample(path):
    wb = Workbook()
    ds = wb.active
    ds.title = "DauSach"
    ds.append(["isbn", "ten_sach", "so_trang", "tac_gia", "nam_xb", "the_loai", "location", "tong_ban_sao", "gia_tien"])
    ds.append(["9780306406157", "Sample Book A", 200, "Nguyen Van A", 2020, "Khoa hoc", "Ke A1", 2, 85000])
    ds.append(["9786040000644", "Ai Trong Doi Song", 300, "Pham Thao", 2023, "Tri Tue Nhan Tao", "Ke B2", 1, 120000])

    bs = wb.create_sheet("BanSao")
    bs.append(["isbn", "ma_sach", "status", "vi_tri", "date_added"])
    bs.append(["9780306406157", "9780306406157-001", "CHO_MUON_DUOC", "Ke A1", "2025-12-06"])
    bs.append(["9780306406157", "9780306406157-002", "DANG_MUON", "Ke A1", "2025-12-07"])

    wb.save(path)
    print(f"Sample workbook written: {path}")
    return path


def preview_and_prepare(dauSachRows, banSaoRows, out_dir, docGiaRows=None, muonTraRows=None):
    os.makedirs(out_dir, exist_ok=True)
    dausach_preview = os.path.join(out_dir, "preview_dausach.csv")
    bansao_preview = os.path.join(out_dir, "preview_bansao.csv")

    # Normalize and validate
    ds_out = []
    ds_errors = []
    for i, r in enumerate(dauSachRows, start=2):
        isbn_raw = r.get('isbn') or r.get('ISBN') or r.get('ma_isbn') or r.get('maISBN')
        title = r.get('ten_sach') or r.get('title') or r.get('Ten Sach') or r.get('Tên sách')
        pages = r.get('so_trang') or r.get('pages') or r.get('Số trang')
        author = r.get('tac_gia') or r.get('author')
        year = r.get('nam_xb') or r.get('year')
        genre = r.get('the_loai') or r.get('genre')
        location = r.get('location') or r.get('vi_tri')
        total_copies = r.get('tong_ban_sao') or r.get('total_copies') or 0
        price = r.get('gia_tien') or r.get('price') or r.get('Gi\u00e1 ti\u1ec1n') or 0

        isbn = normalize_isbn(isbn_raw or "")
        valid = True
        reasons = []
        if not isbn:
            valid = False; reasons.append('Missing ISBN')
        elif not (len(isbn) in (10,13)):
            valid = False; reasons.append('ISBN length not 10/13')
        else:
            if not is_valid_isbn(isbn):
                reasons.append('ISBN checksum invalid')
        if not title or str(title).strip() == "":
            valid = False; reasons.append('Missing title')
        if not author or str(author).strip() == "":
            valid = False; reasons.append('Missing author')
        try:
            pages_n = int(pages) if pages not in (None, "") else 0
            if pages_n < 0: reasons.append('Invalid pages')
        except Exception:
            valid = False; reasons.append('Pages not an integer')
        try:
            year_n = int(year) if year not in (None, "") else 0
            cur = datetime.datetime.now().year
            if year_n != 0 and not (1000 <= year_n <= cur):
                reasons.append('Year out of range')
        except Exception:
            valid = False; reasons.append('Year not integer')

        ds_out.append({
            'isbn': isbn,
            'title': str(title or '').strip(),
            'pages': pages_n if 'pages_n' in locals() else 0,
            'author': str(author or '').strip(),
            'year': year_n if 'year_n' in locals() else 0,
            'genre': str(genre or '').strip(),
            'location': str(location or '').strip(),
            'total_copies': int(total_copies) if total_copies not in (None, '') else 0,
            'price': int(price) if price not in (None, '') else 0,
            'valid': valid,
            'reasons': '; '.join(reasons)
        })
        if not valid:
            ds_errors.append((i, isbn, reasons))

    bs_out = []
    bs_errors = []
    for i, r in enumerate(banSaoRows, start=2):
        isbn_raw = r.get('isbn') or r.get('ISBN')
        copy_id = r.get('ma_sach') or r.get('copy_id') or r.get('ma')
        status = r.get('status') or r.get('trang_thai')
        location = r.get('vi_tri') or r.get('location')
        date_added = r.get('date_added') or r.get('ngay_nhap')

        isbn = normalize_isbn(isbn_raw or "")
        ok = True
        reasons = []
        if not isbn:
            ok = False; reasons.append('Missing ISBN')
        elif len(isbn) not in (10,13):
            ok = False; reasons.append('ISBN length invalid')
        if not copy_id or str(copy_id).strip() == "":
            # generate copy id from isbn
            copy_id = f"{isbn}-AUTO-{i:03d}"
        if not status:
            status = 'CHO_MUON_DUOC'

        bs_out.append({
            'isbn': isbn,
            'ma_sach': str(copy_id).strip(),
            'status': str(status).strip(),
            'location': str(location or '').strip(),
            'date_added': str(date_added or '').strip(),
            'valid': ok,
            'reasons': '; '.join(reasons)
        })
        if not ok:
            bs_errors.append((i, isbn, reasons))

    # write previews
    with open(dausach_preview, 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow(['isbn','title','pages','author','year','genre','location','total_copies','price','valid','reasons'])
        for r in ds_out:
            writer.writerow([r['isbn'], r['title'], r['pages'], r['author'], r['year'], r['genre'], r['location'], r['total_copies'], r['price'], r['valid'], r['reasons']])

    with open(bansao_preview, 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow(['isbn','ma_sach','status','location','date_added','valid','reasons'])
        for r in bs_out:
            writer.writerow([r['isbn'], r['ma_sach'], r['status'], r['location'], r['date_added'], r['valid'], r['reasons']])

    report = {
        'dausach_count': len(ds_out),
        'dausach_errors': len(ds_errors),
        'bansao_count': len(bs_out),
        'bansao_errors': len(bs_errors),
        'dausach_preview_csv': dausach_preview,
        'bansao_preview_csv': bansao_preview
    }
    # handle doc_gia preview if provided
    dg_out = []
    dg_errors = []
    if docGiaRows:
        dg_preview = os.path.join(out_dir, "preview_docgia.csv")
        for i, r in enumerate(docGiaRows, start=2):
            ma_the = r.get('ma_the') or r.get('ma') or r.get('MaThe')
            ho = r.get('ho') or r.get('ho_ten') or r.get('Ho')
            ten = r.get('ten') or r.get('Ten')
            phai = r.get('phai') or r.get('gioi_tinh') or 0
            trang_thai = r.get('trang_thai') or r.get('status') or 1
            try:
                ma_the_n = int(ma_the) if ma_the not in (None, '') else None
            except Exception:
                ma_the_n = None
            valid = ma_the_n is not None and ho and ten
            reasons = []
            if ma_the_n is None: reasons.append('Invalid ma_the')
            if not ho or not ten: reasons.append('Missing name parts')
            dg_out.append({
                'ma_the': ma_the_n,
                'ho': str(ho or '').strip(),
                'ten': str(ten or '').strip(),
                'phai': int(phai) if str(phai).isdigit() else 0,
                'trang_thai': int(trang_thai) if str(trang_thai).isdigit() else 1,
                'valid': valid,
                'reasons': '; '.join(reasons)
            })
            if not valid:
                dg_errors.append((i, ma_the, reasons))
        with open(dg_preview, 'w', newline='', encoding='utf-8') as f:
            w = csv.writer(f)
            w.writerow(['ma_the','ho','ten','phai','trang_thai','valid','reasons'])
            for r in dg_out:
                w.writerow([r['ma_the'], r['ho'], r['ten'], r['phai'], r['trang_thai'], r['valid'], r['reasons']])
        report['docgia_count'] = len(dg_out)
        report['docgia_errors'] = len(dg_errors)
        report['docgia_preview_csv'] = dg_preview

    # handle muon_tra preview if provided
    mt_out = []
    mt_errors = []
    if muonTraRows:
        mt_preview = os.path.join(out_dir, "preview_muontra.csv")
        for i, r in enumerate(muonTraRows, start=2):
            ma_the = r.get('ma_the') or r.get('ma') or r.get('MaThe')
            ma_sach = r.get('ma_sach') or r.get('ma_sach_copy') or r.get('ma')
            ngay_muon = r.get('ngay_muon') or r.get('date_borrow')
            ngay_tra = r.get('ngay_tra') or r.get('date_return')
            trang_thai = r.get('trang_thai') or r.get('status') or 0
            try:
                ma_the_n = int(ma_the) if ma_the not in (None, '') else None
            except Exception:
                ma_the_n = None
            valid = ma_the_n is not None and ma_sach
            reasons = []
            if ma_the_n is None: reasons.append('Invalid ma_the')
            if not ma_sach: reasons.append('Missing ma_sach')
            mt_out.append({
                'ma_the': ma_the_n,
                'ma_sach': str(ma_sach or '').strip(),
                'ngay_muon': str(ngay_muon or '').strip(),
                'ngay_tra': str(ngay_tra or '').strip(),
                'trang_thai': int(trang_thai) if str(trang_thai).isdigit() else 0,
                'valid': valid,
                'reasons': '; '.join(reasons)
            })
            if not valid:
                mt_errors.append((i, ma_the, reasons))
        with open(mt_preview, 'w', newline='', encoding='utf-8') as f:
            w = csv.writer(f)
            w.writerow(['ma_the','ma_sach','ngay_muon','ngay_tra','trang_thai','valid','reasons'])
            for r in mt_out:
                w.writerow([r['ma_the'], r['ma_sach'], r['ngay_muon'], r['ngay_tra'], r['trang_thai'], r['valid'], r['reasons']])
        report['muontra_count'] = len(mt_out)
        report['muontra_errors'] = len(mt_errors)
        report['muontra_preview_csv'] = mt_preview

    return report, ds_out, bs_out, dg_out, mt_out


def write_import_text(ds_out, out_dir):
    ts = datetime.datetime.now().strftime('%Y%m%d_%H%M%S')
    out_path = os.path.join(out_dir, f'dausach_import_{ts}.txt')
    with open(out_path, 'w', encoding='utf-8') as f:
        for r in ds_out:
            # write in existing pipe format: ISBN|Title|Pages|Author|Year|Genre
            f.write(f"{r['isbn']}|{r['title']}|{r['pages']}|{r['author']}|{r['year']}|{r['genre']}\n")
    return out_path


def map_status_to_int(status):
    if status is None:
        return 0
    s = str(status).strip().lower()
    if s == "":
        return 0
    if s.isdigit():
        try:
            return int(s)
        except Exception:
            return 0
    # common textual mappings
    if "dang" in s and "muon" in s:
        return 1
    if "thanh" in s or "thanh ly" in s or "thanh_ly" in s:
        return 2
    if "cho" in s or "sẵn" in s or "sang" in s:
        return 0
    # fallback default
    return 0


def apply_to_db(ds_out, bs_out, dg_out=None, mt_out=None, mode='merge', data_dir='data'):
    db_path = os.path.join(data_dir, 'files', 'library.db')
    if not os.path.isdir(data_dir):
        os.makedirs(data_dir, exist_ok=True)

    # backup existing DB if present
    if os.path.exists(db_path):
        ts = datetime.datetime.now().strftime('%Y%m%d_%H%M%S')
        # ensure centralized backup directory under data/backup
        backups_dir = os.path.join(os.path.dirname(db_path), 'backup')
        os.makedirs(backups_dir, exist_ok=True)
        bak_name = f'library.db.bak_{ts}'
        bak = os.path.join(backups_dir, bak_name)
        shutil.copy2(db_path, bak)
        print('Backup created:', bak)

    # connect
    con = sqlite3.connect(db_path)
    cur = con.cursor()

    # Ensure tables exist (same schema as DatabaseManager)
    cur.executescript('''
    CREATE TABLE IF NOT EXISTS dau_sach (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        isbn TEXT UNIQUE NOT NULL,
        ten_sach TEXT NOT NULL,
        so_trang INTEGER,
        tac_gia TEXT,
        nam_xuat_ban INTEGER,
        the_loai TEXT,
        so_luot_muon INTEGER DEFAULT 0
    );
    CREATE TABLE IF NOT EXISTS ban_sao (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        ma_sach TEXT UNIQUE NOT NULL,
        dau_sach_id INTEGER NOT NULL,
        trang_thai INTEGER DEFAULT 0,
        vi_tri TEXT
    );
    CREATE TABLE IF NOT EXISTS doc_gia (
        ma_the INTEGER PRIMARY KEY,
        ho TEXT,
        ten TEXT,
        phai INTEGER,
        trang_thai INTEGER DEFAULT 1,
        so_dien_thoai TEXT,
        email TEXT,
        ngay_dang_ky TEXT,
        ngay_het_han TEXT,
        so_sach_dang_muon INTEGER DEFAULT 0
    );
    CREATE TABLE IF NOT EXISTS muon_tra (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        ma_the INTEGER NOT NULL,
        ban_sao_id INTEGER NOT NULL,
        ngay_muon TEXT NOT NULL,
        ngay_tra TEXT,
        trang_thai INTEGER DEFAULT 0,
        tien_phat INTEGER DEFAULT 0
    );
    ''')

    try:
        con.execute('BEGIN')

        # Insert/update dau_sach
        for r in ds_out:
            isbn = r['isbn']
            if not isbn:
                print('Skipping dau_sach with empty ISBN')
                continue
            if mode == 'skip':
                cur.execute('SELECT id FROM dau_sach WHERE isbn=?', (isbn,))
                if cur.fetchone():
                    continue
                cur.execute('INSERT INTO dau_sach(isbn, ten_sach, so_trang, tac_gia, nam_xuat_ban, the_loai, gia_tien) VALUES (?, ?, ?, ?, ?, ?, ?)',
                            (isbn, r.get('title',''), r.get('pages',0), r.get('author',''), r.get('year',0), r.get('genre',''), r.get('price',0)))
            elif mode == 'replace':
                cur.execute('DELETE FROM dau_sach WHERE isbn=?', (isbn,))
                cur.execute('INSERT INTO dau_sach(isbn, ten_sach, so_trang, tac_gia, nam_xuat_ban, the_loai, gia_tien) VALUES (?, ?, ?, ?, ?, ?, ?)',
                            (isbn, r.get('title',''), r.get('pages',0), r.get('author',''), r.get('year',0), r.get('genre',''), r.get('price',0)))
            else:  # merge
                cur.execute('SELECT id, ten_sach, so_trang, tac_gia, nam_xuat_ban, the_loai, gia_tien FROM dau_sach WHERE isbn=?', (isbn,))
                row = cur.fetchone()
                if row:
                    did = row[0]
                    updates = []
                    params = []
                    if r.get('title'):
                        updates.append('ten_sach=?'); params.append(r.get('title'))
                    if r.get('pages'):
                        updates.append('so_trang=?'); params.append(r.get('pages'))
                    if r.get('author'):
                        updates.append('tac_gia=?'); params.append(r.get('author'))
                    if r.get('year'):
                        updates.append('nam_xuat_ban=?'); params.append(r.get('year'))
                    if r.get('genre'):
                        updates.append('the_loai=?'); params.append(r.get('genre'))
                    if r.get('price'):
                        updates.append('gia_tien=?'); params.append(r.get('price'))
                    if updates:
                        params.append(did)
                        cur.execute('UPDATE dau_sach SET ' + ','.join(updates) + ' WHERE id=?', params)
                else:
                    cur.execute('INSERT INTO dau_sach(isbn, ten_sach, so_trang, tac_gia, nam_xuat_ban, the_loai, gia_tien) VALUES (?, ?, ?, ?, ?, ?, ?)',
                                (isbn, r.get('title',''), r.get('pages',0), r.get('author',''), r.get('year',0), r.get('genre',''), r.get('price',0)))

        # Insert/update ban_sao
        for r in bs_out:
            isbn = r['isbn']
            ma_sach = r['ma_sach']
            if not isbn or not ma_sach:
                print('Skipping bansao with missing isbn/ma_sach')
                continue
            # find dau_sach id
            cur.execute('SELECT id FROM dau_sach WHERE isbn=?', (isbn,))
            row = cur.fetchone()
            if not row:
                print('Could not find dau_sach for ISBN', isbn, '- skipping ma_sach', ma_sach)
                continue
            did = row[0]
            tt = map_status_to_int(r.get('status'))
            vi_tri = r.get('location','')

            if mode == 'skip':
                cur.execute('SELECT id FROM ban_sao WHERE ma_sach=?', (ma_sach,))
                if cur.fetchone():
                    continue
                cur.execute('INSERT INTO ban_sao(ma_sach, dau_sach_id, trang_thai, vi_tri) VALUES (?, ?, ?, ?)', (ma_sach, did, tt, vi_tri))
            elif mode == 'replace':
                cur.execute('DELETE FROM ban_sao WHERE ma_sach=?', (ma_sach,))
                cur.execute('INSERT INTO ban_sao(ma_sach, dau_sach_id, trang_thai, vi_tri) VALUES (?, ?, ?, ?)', (ma_sach, did, tt, vi_tri))
            else:  # merge
                cur.execute('SELECT id, trang_thai, vi_tri FROM ban_sao WHERE ma_sach=?', (ma_sach,))
                brow = cur.fetchone()
                if brow:
                    bids = []
                    params = []
                    if r.get('status') is not None:
                        bids.append('trang_thai=?'); params.append(tt)
                    if vi_tri:
                        bids.append('vi_tri=?'); params.append(vi_tri)
                    if bids:
                        params.append(brow[0])
                        cur.execute('UPDATE ban_sao SET ' + ','.join(bids) + ' WHERE id=?', params)
                else:
                    cur.execute('INSERT INTO ban_sao(ma_sach, dau_sach_id, trang_thai, vi_tri) VALUES (?, ?, ?, ?)', (ma_sach, did, tt, vi_tri))

        # Insert/update doc_gia if provided
        if dg_out:
            for r in dg_out:
                ma = r.get('ma_the')
                if ma is None:
                    print('Skipping doc_gia with invalid ma_the')
                    continue
                ho = r.get('ho','')
                ten = r.get('ten','')
                phai = int(r.get('phai',0))
                tt = int(r.get('trang_thai',1))
                if mode == 'skip':
                    cur.execute('SELECT ma_the FROM doc_gia WHERE ma_the=?',(ma,))
                    if cur.fetchone():
                        continue
                    cur.execute('INSERT INTO doc_gia(ma_the, ho, ten, phai, trang_thai) VALUES (?, ?, ?, ?, ?)', (ma, ho, ten, phai, tt))
                elif mode == 'replace':
                    cur.execute('DELETE FROM doc_gia WHERE ma_the=?', (ma,))
                    cur.execute('INSERT INTO doc_gia(ma_the, ho, ten, phai, trang_thai) VALUES (?, ?, ?, ?, ?)', (ma, ho, ten, phai, tt))
                else:  # merge
                    cur.execute('SELECT ma_the FROM doc_gia WHERE ma_the=?',(ma,))
                    if cur.fetchone():
                        # update non-empty fields
                        updates = []
                        params = []
                        if ho: updates.append('ho=?'); params.append(ho)
                        if ten: updates.append('ten=?'); params.append(ten)
                        if updates:
                            params.append(ma)
                            cur.execute('UPDATE doc_gia SET ' + ','.join(updates) + ' WHERE ma_the=?', params)
                    else:
                        cur.execute('INSERT INTO doc_gia(ma_the, ho, ten, phai, trang_thai) VALUES (?, ?, ?, ?, ?)', (ma, ho, ten, phai, tt))

        # Insert/update muon_tra if provided
        if mt_out:
            for r in mt_out:
                ma = r.get('ma_the')
                ma_sach = r.get('ma_sach')
                if ma is None or not ma_sach:
                    print('Skipping muon_tra with invalid ma_the/ma_sach')
                    continue
                # find ban_sao id
                cur.execute('SELECT id FROM ban_sao WHERE ma_sach=?', (ma_sach,))
                bsrow = cur.fetchone()
                if not bsrow:
                    print('Could not find ban_sao for', ma_sach, '- skipping muon_tra')
                    continue
                ban_sao_id = bsrow[0]
                # ensure doc_gia exists
                cur.execute('SELECT ma_the FROM doc_gia WHERE ma_the=?', (ma,))
                if not cur.fetchone():
                    # insert minimal doc_gia
                    cur.execute('INSERT INTO doc_gia(ma_the, ho, ten, phai, trang_thai) VALUES (?, ?, ?, ?, ?)', (ma, '', '', 0, 1))
                ngay_muon = r.get('ngay_muon','')
                ngay_tra = r.get('ngay_tra','')
                tt = int(r.get('trang_thai',0))
                # always insert a new muon_tra row
                cur.execute('INSERT INTO muon_tra(ma_the, ban_sao_id, ngay_muon, ngay_tra, trang_thai) VALUES (?, ?, ?, ?, ?)', (ma, ban_sao_id, ngay_muon, ngay_tra if ngay_tra else None, tt))

        con.commit()
        print('DB apply completed successfully.')
    except Exception as e:
        con.rollback()
        print('Error applying to DB, rolled back. Error:', e)
    finally:
        con.close()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--file', '-f', help='Excel file (.xlsx) to import')
    parser.add_argument('--generate-sample', action='store_true', help='Generate sample Excel and exit')
    parser.add_argument('--preview', action='store_true', help='Only preview and write preview CSVs')
    parser.add_argument('--apply', action='store_true', help='Write import files into data/ (safe)')
    parser.add_argument('--to-db', action='store_true', help='Apply import directly into SQLite DB at data/library.db (creates backup)')
    parser.add_argument('--mode', choices=['skip','merge','replace'], default='merge', help='How to handle existing ISBNs')
    parser.add_argument('--out', default='tools', help='Output directory for previews/import files')

    args = parser.parse_args()

    if args.generate_sample:
        sample = os.path.join('tools', 'sample_import.xlsx')
        generate_sample(sample)
        print('Run again with --file', sample)
        return

    if not args.file:
        print('No file provided. Use --file <xlsx> or --generate-sample')
        return

    if not os.path.exists(args.file):
        print('File not found:', args.file)
        return

    wb = load_workbook(args.file, data_only=True)
    # read dauSach, banSao, doc_gia, muon_tra if present
    dauSachRows = read_sheet_as_dicts(wb, 'DauSach')
    banSaoRows = read_sheet_as_dicts(wb, 'BanSao') if 'BanSao' in wb.sheetnames else []
    docGiaRows = read_sheet_as_dicts(wb, 'DocGia') if 'DocGia' in wb.sheetnames else []
    muonTraRows = read_sheet_as_dicts(wb, 'MuonTra') if 'MuonTra' in wb.sheetnames else []

    report, ds_out, bs_out, dg_out, mt_out = preview_and_prepare(dauSachRows, banSaoRows, args.out, docGiaRows, muonTraRows)

    print('\n=== Preview Report ===')
    print(f"DauSach rows: {report['dausach_count']}, errors: {report['dausach_errors']}")
    print(f"BanSao rows: {report['bansao_count']}, errors: {report['bansao_errors']}")
    print('Preview CSVs:')
    print(' -', report['dausach_preview_csv'])
    print(' -', report['bansao_preview_csv'])
    if 'docgia_preview_csv' in report:
        print(' -', report['docgia_preview_csv'])
    if 'muontra_preview_csv' in report:
        print(' -', report['muontra_preview_csv'])

    if args.apply:
        out_path = write_import_text(ds_out, args.out if args.out else 'data')
        print('\nWrote import text:', out_path)
        # Also write detailed bansao CSV to out
        ts = datetime.datetime.now().strftime('%Y%m%d_%H%M%S')
        bansao_out = os.path.join(args.out, f'bansao_import_{ts}.csv')
        with open(bansao_out, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(['isbn','ma_sach','status','location','date_added'])
            for r in bs_out:
                writer.writerow([r['isbn'], r['ma_sach'], r['status'], r['location'], r['date_added']])
        print('Wrote bansao import file:', bansao_out)
        print('\nNOTE: This action writes import files only. It does NOT directly modify the SQLite DB.\n')

    print('\nDone.')

    # Optionally apply directly to the SQLite DB
    if args.to_db:
        print('\nApplying directly to SQLite DB...')
        apply_to_db(ds_out, bs_out, dg_out, mt_out, mode=args.mode, data_dir='data')

if __name__ == '__main__':
    main()
