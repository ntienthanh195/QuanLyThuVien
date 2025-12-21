import sqlite3
import os

db = os.path.join(os.getcwd(),'data','files','library.db')
con = sqlite3.connect(db)
cur = con.cursor()
for t in ('doc_gia','muon_tra'):
    print('Table',t)
    cur.execute(f"PRAGMA table_info({t})")
    for r in cur.fetchall():
        print(r)
    print()
con.close()
