#!/usr/bin/env python3
import sqlite3, os
cwd = os.path.abspath(os.path.dirname(__file__))
repo_root = os.path.abspath(os.path.join(cwd, '..'))
data_dir = os.path.join(repo_root, 'data')
db_path = os.path.join(data_dir, 'files', 'library.db')
con = sqlite3.connect(db_path)
cur = con.cursor()
# Map R1001 -> 1001, R1002 -> 1002
cur.execute("UPDATE muon_tra SET ma_the = '1001' WHERE ma_the = 'R1001'")
cur.execute("UPDATE muon_tra SET ma_the = '1002' WHERE ma_the = 'R1002'")
con.commit()
# Show muon_tra
cur.execute("SELECT id, ma_the, ban_sao_id, ngay_muon, ngay_tra, trang_thai FROM muon_tra LIMIT 10")
for r in cur.fetchall():
    print(r)
con.close()
