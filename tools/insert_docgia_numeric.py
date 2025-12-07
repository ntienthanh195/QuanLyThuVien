#!/usr/bin/env python3
import sqlite3, os
from datetime import datetime, timedelta

cwd = os.path.abspath(os.path.dirname(__file__))
repo_root = os.path.abspath(os.path.join(cwd, '..'))
data_dir = os.path.join(repo_root, 'data')
if not os.path.isdir(data_dir):
    print('No data/ directory found in', cwd)
    raise SystemExit(2)

db_path = os.path.join(data_dir, 'files', 'library.db')
if not os.path.exists(db_path):
    print('DB not found at', db_path)
    raise SystemExit(3)

con = sqlite3.connect(db_path)
cur = con.cursor()

# get columns
cur.execute("PRAGMA table_info(doc_gia)")
cols_info = cur.fetchall()
cols = [c[1] for c in cols_info]
print('doc_gia columns:', cols)

# Prepare numeric readers
now = datetime.now()
readers = [
    { 'ma_the': 1001, 'ho': 'Nguyen', 'ten': 'An', 'phai': 0, 'trang_thai': 1, 'so_dien_thoai': '0123456789', 'email': 'nguyen.an@example.local', 'ngay_dang_ky': now.strftime('%Y-%m-%d'), 'ngay_het_han': (now + timedelta(days=365)).strftime('%Y-%m-%d'), 'so_sach_dang_muon': 1 },
    { 'ma_the': 1002, 'ho': 'Tran', 'ten': 'Binh', 'phai': 0, 'trang_thai': 1, 'so_dien_thoai': '0987654321', 'email': 'tran.binh@example.local', 'ngay_dang_ky': now.strftime('%Y-%m-%d'), 'ngay_het_han': (now + timedelta(days=365)).strftime('%Y-%m-%d'), 'so_sach_dang_muon': 0 }
]

inserted = 0
for r in readers:
    keys = [k for k in cols if k in r]
    if not keys:
        print('No matching columns to insert for doc_gia, skipping')
        continue
    cols_sql = ','.join(keys)
    vals_sql = ','.join(['?']*len(keys))
    vals = [r[k] for k in keys]
    sql = f"INSERT OR REPLACE INTO doc_gia ({cols_sql}) VALUES ({vals_sql})"
    try:
        cur.execute(sql, vals)
        inserted += 1
    except Exception as e:
        print('Insert failed:', e)
        # try cast to proper types
        try:
            vals2 = []
            for v in vals:
                if isinstance(v, int):
                    vals2.append(int(v))
                else:
                    vals2.append(str(v))
            cur.execute(sql, vals2)
            inserted += 1
        except Exception as e2:
            print('Retry insert failed:', e2)
            continue

con.commit()
con.close()
print('Inserted numeric doc_gia rows:', inserted)
