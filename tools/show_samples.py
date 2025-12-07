import sqlite3, os, sys

cwd = os.getcwd()
files_dir = os.path.join(cwd, 'data')
if not os.path.isdir(files_dir):
    print('No data/ directory found in', cwd)
    sys.exit(2)
db_path = os.path.join(files_dir, 'files', 'library.db')
if not os.path.exists(db_path):
    print('DB not found at', db_path)
    sys.exit(3)

con = sqlite3.connect(db_path)
cur = con.cursor()

def show(table, cols='*', limit=10):
    try:
        cur.execute(f'SELECT {cols} FROM {table} LIMIT {limit}')
        rows = cur.fetchall()
        print(f'--- {table} (first {limit}) ---')
        if not rows:
            print('(no rows)')
            return
        for r in rows:
            print(r)
    except Exception as e:
        print(f'Error reading {table}:', e)

show('dau_sach', 'id, isbn, ten_sach, tac_gia, nam_xuat_ban', 10)
show('ban_sao', 'id, ma_sach, dau_sach_id, trang_thai, vi_tri', 10)
show('doc_gia', 'ma_the, ho, ten, phai, trang_thai, so_sach_dang_muon', 10)
show('muon_tra', 'id, ma_the, ban_sao_id, ngay_muon, ngay_tra, trang_thai', 10)

con.close()
