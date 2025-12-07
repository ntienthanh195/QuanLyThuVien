#!/usr/bin/env python3
import sqlite3, os
cwd = os.path.abspath(os.path.dirname(__file__))
repo_root = os.path.abspath(os.path.join(cwd, '..'))
data_dir = os.path.join(repo_root, 'data')
db_path = os.path.join(data_dir, 'files', 'library.db')
con = sqlite3.connect(db_path)
cur = con.cursor()

# Write docgia.txt legacy format: MaThe|Ho|Ten|GioiTinh|TrangThai
cur.execute("SELECT ma_the, ho, ten, phai, trang_thai FROM doc_gia")
rows = cur.fetchall()
with open(os.path.join(data_dir, 'docgia.txt'), 'w', encoding='utf-8') as f:
    for r in rows:
        f.write(f"{r[0]}|{r[1]}|{r[2]}|{r[3]}|{r[4]}\n")

# Write muontra.txt legacy format: MaSach|MaThe|NgayMuon|NgayTra|TrangThai
# map ban_sao_id -> ma_sach
cur.execute('SELECT id, ma_sach FROM ban_sao')
map_id = {r[0]: r[1] for r in cur.fetchall()}
cur.execute('SELECT ma_the, ban_sao_id, ngay_muon, ngay_tra, trang_thai FROM muon_tra')
rows = cur.fetchall()
with open(os.path.join(data_dir, 'muontra.txt'), 'w', encoding='utf-8') as f:
    for r in rows:
        ma_sach = map_id.get(r[1], f'UNKNOWN-{r[1]}')
        f.write(f"{ma_sach}|{r[0]}|{r[2]}|{r[3]}|{r[4]}\n")

con.close()
print('Wrote legacy data/docgia.txt and data/muontra.txt from DB')
