#!/usr/bin/env python3
"""
Backup the library DB and set all copies (ban_sao) to available (trang_thai = 0).
Run from repo root or directly with the venv python.
"""
import os
import shutil
import datetime
import sqlite3
import sys

# Compute repo root (one level up from this script's directory)
THIS_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.abspath(os.path.join(THIS_DIR, '..'))
DB_PATH = os.path.join(REPO_ROOT, 'data', 'files', 'library.db')

if not os.path.exists(DB_PATH):
    print('ERROR: DB not found at', DB_PATH)
    sys.exit(2)

# Ensure backup directory
backup_dir = os.path.join(os.path.dirname(DB_PATH), 'backup')
os.makedirs(backup_dir, exist_ok=True)

ts = datetime.datetime.now().strftime('%Y%m%d_%H%M%S')
backup_path = os.path.join(backup_dir, f'library.db.bak_{ts}')
shutil.copy2(DB_PATH, backup_path)
print('Backup created:', backup_path)

# Connect and update
con = sqlite3.connect(DB_PATH)
cur = con.cursor()
try:
    # Count rows currently not available
    cur.execute('SELECT COUNT(*) FROM ban_sao WHERE trang_thai != 0')
    before = cur.fetchone()[0]

    cur.execute('UPDATE ban_sao SET trang_thai = 0 WHERE trang_thai != 0')
    changed = cur.rowcount
    con.commit()

    cur.execute('SELECT COUNT(*) FROM ban_sao WHERE trang_thai != 0')
    after = cur.fetchone()[0]

    print(f'Rows that were not available before: {before}')
    print(f'Rows affected by UPDATE: {changed}')
    print(f'Rows still not available after update: {after}')

except Exception as e:
    con.rollback()
    print('Error while updating DB:', e)
    sys.exit(1)
finally:
    con.close()

print('Done. All copies set to available (trang_thai=0).')
