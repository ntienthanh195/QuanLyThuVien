#!/usr/bin/env python3
"""
Set all copies (`ban_sao`) to available (trang_thai = 0).
Creates a backup of the DB before applying changes and prints a short summary.
"""
import os, shutil, datetime, sqlite3

root = os.path.abspath(os.path.dirname(__file__) + os.sep + '..')
db_path = os.path.join(root, 'data', 'files', 'library.db')
if not os.path.exists(db_path):
    print('DB not found at', db_path)
    raise SystemExit(1)

# backup
ts = datetime.datetime.now().strftime('%Y%m%d_%H%M%S')
backup_path = os.path.join(os.path.dirname(db_path), f'library.db.bak_setavail_{ts}')
shutil.copy2(db_path, backup_path)
print('Backup created:', backup_path)

con = sqlite3.connect(db_path)
cur = con.cursor()
# Count current non-zero statuses
cur.execute("SELECT COUNT(*) FROM ban_sao WHERE trang_thai IS NULL OR trang_thai<>0")
before = cur.fetchone()[0]
print('Copies not available (before):', before)

# Update
cur.execute("UPDATE ban_sao SET trang_thai=0 WHERE trang_thai IS NULL OR trang_thai<>0")
con.commit()
changed = cur.rowcount if hasattr(cur, 'rowcount') else None
# Recount
cur.execute("SELECT COUNT(*) FROM ban_sao WHERE trang_thai<>0")
after = cur.fetchone()[0]
print('Copies not available (after):', after)
print('Rows changed reported by sqlite:', changed)
con.close()
print('Done')
