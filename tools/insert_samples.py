import sqlite3
import os
from datetime import datetime, timedelta

cwd = os.getcwd()
data_dir = os.path.join(cwd, 'data')
if not os.path.isdir(data_dir):
    print('No data/ directory found in', cwd)
    raise SystemExit(2)

db_path = os.path.join(data_dir, 'files', 'library.db')
if not os.path.exists(db_path):
    print('DB not found at', db_path)
    raise SystemExit(3)

con = sqlite3.connect(db_path)
cur = con.cursor()

# Inspect columns for doc_gia and muon_tra
def cols(table):
    cur.execute("PRAGMA table_info(%s)" % table)
    return [r[1] for r in cur.fetchall()]

doc_cols = cols('doc_gia')
muon_cols = cols('muon_tra')
print('doc_gia columns:', doc_cols)
print('muon_tra columns:', muon_cols)

# Prepare sample readers
now = datetime.now()
readers = [
    {
        'ma_the': 'R1001',
        'ho': 'Nguyen',
        'ten': 'An',
        'phai': 0,
        'trang_thai': 1,
        'so_dien_thoai': '0123456789',
        'email': 'nguyen.an@example.local',
        'ngay_dang_ky': now.strftime('%Y-%m-%d'),
        'ngay_het_han': (now + timedelta(days=365)).strftime('%Y-%m-%d'),
        'so_sach_dang_muon': 1
    },
    {
        'ma_the': 'R1002',
        'ho': 'Tran',
        'ten': 'Binh',
        'phai': 0,
        'trang_thai': 1,
        'so_dien_thoai': '0987654321',
        'email': 'tran.binh@example.local',
        'ngay_dang_ky': now.strftime('%Y-%m-%d'),
        'ngay_het_han': (now + timedelta(days=365)).strftime('%Y-%m-%d'),
        'so_sach_dang_muon': 0
    }
]

# Insert or replace readers using available columns
for r in readers:
    keys = [k for k in doc_cols if k in r]
    if not keys:
        print('No matching columns to insert for doc_gia, skipping')
        continue
    cols_sql = ','.join(keys)
    vals_sql = ','.join(['?'] * len(keys))
    vals = [r[k] for k in keys]
    sql = f"INSERT OR REPLACE INTO doc_gia ({cols_sql}) VALUES ({vals_sql})"
    try:
        cur.execute(sql, vals)
    except Exception as e:
        print('Failed to insert doc_gia row. Error:', e)
        print('SQL:', sql)
        print('Vals:', vals)
        # Retry with string-casted values as a last resort
        try:
            vals2 = [str(v) for v in vals]
            cur.execute(sql, vals2)
            print('Retry succeeded with string-casted values.')
        except Exception as e2:
            print('Retry failed:', e2)
            continue

# Create sample loan records (link to existing ban_sao ids if possible)
# We'll pick ban_sao ids 1 and 6 which exist in the current DB sample
loan1 = {
    'ma_the': 'R1001',
    'ban_sao_id': 1,
    'ngay_muon': now.strftime('%Y-%m-%d'),
    'ngay_tra': (now + timedelta(days=14)).strftime('%Y-%m-%d'),
    'trang_thai': 0, # 0 = borrowed
    'tien_phat': 0
}
loan2 = {
    'ma_the': 'R1002',
    'ban_sao_id': 6,
    'ngay_muon': now.strftime('%Y-%m-%d'),
    'ngay_tra': (now + timedelta(days=7)).strftime('%Y-%m-%d'),
    'trang_thai': 0,
    'tien_phat': 0
}

for loan in (loan1, loan2):
    keys = [k for k in muon_cols if k in loan]
    if not keys:
        print('No matching columns to insert for muon_tra, skipping')
        continue
    cols_sql = ','.join(keys)
    vals_sql = ','.join(['?'] * len(keys))
    vals = [loan[k] for k in keys]
    sql = f"INSERT INTO muon_tra ({cols_sql}) VALUES ({vals_sql})"
    try:
        cur.execute(sql, vals)
    except Exception as e:
        print('Failed to insert muon_tra row. Error:', e)
        print('SQL:', sql)
        print('Vals:', vals)
        try:
            vals2 = [str(v) for v in vals]
            cur.execute(sql, vals2)
            print('Retry succeeded with string-casted values for muon_tra.')
        except Exception as e2:
            print('Retry failed for muon_tra:', e2)
            continue

con.commit()
print('Inserted sample readers and loans into DB.')

# Also write legacy text files for compatibility
legacy_docgia = os.path.join(data_dir, 'docgia.txt')
legacy_muontra = os.path.join(data_dir, 'muontra.txt')

with open(legacy_docgia, 'w', encoding='utf-8') as f:
    for r in readers:
        # format: MaThe|Ho|Ten|GioiTinh|TrangThai
        line = f"{r['ma_the']}|{r['ho']}|{r['ten']}|{r['phai']}|{r['trang_thai']}\n"
        f.write(line)

with open(legacy_muontra, 'w', encoding='utf-8') as f:
    # format: MaSach|MaThe|NgayMuon|NgayTra|TrangThai
    # map ban_sao_id to ma_sach by querying ban_sao
    cur.execute('SELECT id, ma_sach FROM ban_sao')
    id_to_masach = {row[0]: row[1] for row in cur.fetchall()}
    for loan in (loan1, loan2):
        ma_sach = id_to_masach.get(loan['ban_sao_id'], f"UNKNOWN-{loan['ban_sao_id']}")
        line = f"{ma_sach}|{loan['ma_the']}|{loan['ngay_muon']}|{loan['ngay_tra']}|{loan['trang_thai']}\n"
        f.write(line)

con.close()
print('Wrote legacy files:', legacy_docgia, legacy_muontra)
