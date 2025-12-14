#!/usr/bin/env python3
"""
Scan `data/files/library.db` for common invalid records and produce a report + SQL delete script.
- Backs up the DB to `data/files/library.db.bak_<ts>` before doing anything.
- Produces `data/cleanup_report.txt` and `data/cleanup_delete.sql`.
- Does NOT execute deletions automatically; you can inspect and run the SQL yourself or let this script run them if you change `APPLY_DELETES=True`.
"""
import sqlite3, os, shutil, datetime, re

APPLY_DELETES = False  # set True to apply deletions automatically

root = os.path.abspath(os.path.dirname(__file__) + os.sep + '..')
data_dir = os.path.join(root, 'data', 'files')
db_path = os.path.join(data_dir, 'library.db')

if not os.path.exists(db_path):
    print('DB not found at', db_path)
    raise SystemExit(1)

# backup
ts = datetime.datetime.now().strftime('%Y%m%d_%H%M%S')
backup_path = os.path.join(data_dir, f'library.db.bak_{ts}')
shutil.copy2(db_path, backup_path)
print('Backup created:', backup_path)

con = sqlite3.connect(db_path)
con.row_factory = sqlite3.Row
cur = con.cursor()

report_lines = []
delete_statements = []

# helpers
def is_valid_isbn(isbn: str) -> bool:
    if isbn is None: return False
    s = re.sub(r"[^0-9Xx]", "", str(isbn))
    s = s.upper()
    if len(s) == 10:
        if not re.match(r"^[0-9]{9}[0-9X]$", s): return False
        total = 0
        for i, ch in enumerate(s[:9]):
            total += (i + 1) * int(ch)
        check = total % 11
        check_char = 'X' if check == 10 else str(check)
        return s[-1] == check_char
    if len(s) == 13:
        if not re.match(r"^[0-9]{13}$", s): return False
        total = 0
        for i, ch in enumerate(s[:12]):
            n = int(ch)
            total += n if i % 2 == 0 else n * 3
        check = (10 - (total % 10)) % 10
        return int(s[-1]) == check
    return False

# 1) doc_gia: missing ma_the or name parts
report_lines.append('=== DOC_GIA checks ===')
cur.execute('PRAGMA table_info(doc_gia)')
if cur.fetchall():
    cur.execute("SELECT ma_the, ho, ten, phai, trang_thai FROM doc_gia")
    rows = cur.fetchall()
    missing_name = [r for r in rows if (r['ho'] is None or str(r['ho']).strip()=='') or (r['ten'] is None or str(r['ten']).strip()=='')]
    null_id = [r for r in rows if r['ma_the'] is None]
    dup_ids = []
    ids = [r['ma_the'] for r in rows]
    from collections import Counter
    cnt = Counter(ids)
    for k,v in cnt.items():
        if k is None: continue
        if v>1:
            dup_ids.append((k,v))
    report_lines.append(f'Total doc_gia rows: {len(rows)}')
    report_lines.append(f'Rows with missing ho/ten: {len(missing_name)}')
    report_lines.append(f'Rows with NULL ma_the: {len(null_id)}')
    report_lines.append(f'Duplicate ma_the counts: {dup_ids}')
    if missing_name:
        report_lines.append('Sample missing-name rows:')
        for r in missing_name[:10]: report_lines.append(str(dict(r)))
    # propose deletions for rows with NULL ma_the or missing name
    for r in null_id:
        delete_statements.append(f"DELETE FROM doc_gia WHERE rowid={r['rowid']};")
    # for missing name, we don't auto-delete, but offer statements
    for r in missing_name:
        delete_statements.append(f"-- REVIEW: doc_gia id={r['ma_the']} ho='{r['ho']}' ten='{r['ten']}'  -- consider deleting: DELETE FROM doc_gia WHERE ma_the={r['ma_the']};")
else:
    report_lines.append('Table doc_gia not found')

# 2) dau_sach: invalid/missing isbn or title
report_lines.append('\n=== DAU_SACH checks ===')
cur.execute('PRAGMA table_info(dau_sach)')
if cur.fetchall():
    cur.execute("SELECT id, isbn, ten_sach, tac_gia FROM dau_sach")
    rows = cur.fetchall()
    missing_isbn = [r for r in rows if (r['isbn'] is None or str(r['isbn']).strip()=='')]
    missing_title = [r for r in rows if (r['ten_sach'] is None or str(r['ten_sach']).strip()=='')]
    invalid_isbn = [r for r in rows if r['isbn'] and (not is_valid_isbn(r['isbn']))]
    # duplicates by isbn
    isbns = [ (r['id'], r['isbn']) for r in rows if r['isbn']]
    cnt = Counter([x[1] for x in isbns])
    dups = [ (isbn, c) for isbn,c in cnt.items() if c>1]
    report_lines.append(f'Total dau_sach rows: {len(rows)}')
    report_lines.append(f'Missing ISBN: {len(missing_isbn)}')
    report_lines.append(f'Missing title: {len(missing_title)}')
    report_lines.append(f'Invalid ISBN (by checksum/format): {len(invalid_isbn)}')
    report_lines.append(f'Duplicate ISBNs: {dups[:10]}')
    if invalid_isbn:
        report_lines.append('Sample invalid ISBN rows:')
        for r in invalid_isbn[:10]: report_lines.append(str(dict(r)))
    # propose deletes for rows with missing isbn
    for r in missing_isbn:
        delete_statements.append(f"DELETE FROM dau_sach WHERE id={r['id']};")
    for r in missing_title:
        delete_statements.append(f"-- REVIEW: dau_sach id={r['id']} has empty title; consider deleting: DELETE FROM dau_sach WHERE id={r['id']};")
else:
    report_lines.append('Table dau_sach not found')

# 3) ban_sao checks
report_lines.append('\n=== BAN_SAO checks ===')
cur.execute('PRAGMA table_info(ban_sao)')
if cur.fetchall():
    cur.execute("SELECT id, ma_sach, dau_sach_id, trang_thai FROM ban_sao")
    rows = cur.fetchall()
    missing_masach = [r for r in rows if (r['ma_sach'] is None or str(r['ma_sach']).strip()=='')]
    missing_dausach = [r for r in rows if r['dau_sach_id'] is None]
    # check dau_sach_id exists
    cur.execute("SELECT id FROM dau_sach")
    ds_ids = set([r[0] for r in cur.fetchall()])
    refs_invalid = [r for r in rows if (r['dau_sach_id'] is not None and r['dau_sach_id'] not in ds_ids)]
    report_lines.append(f'Total ban_sao rows: {len(rows)}')
    report_lines.append(f'Missing ma_sach: {len(missing_masach)}')
    report_lines.append(f'Missing dau_sach_id: {len(missing_dausach)}')
    report_lines.append(f'ban_sao with non-existent dau_sach_id: {len(refs_invalid)}')
    for r in missing_masach[:10]: report_lines.append(str(dict(r)))
    for r in refs_invalid[:10]: report_lines.append(str(dict(r)))
    for r in missing_masach:
        delete_statements.append(f"DELETE FROM ban_sao WHERE id={r['id']};")
    for r in refs_invalid:
        delete_statements.append(f"DELETE FROM ban_sao WHERE id={r['id']};")
else:
    report_lines.append('Table ban_sao not found')

# 4) muon_tra referential integrity
report_lines.append('\n=== MUON_TRA checks ===')
cur.execute('PRAGMA table_info(muon_tra)')
if cur.fetchall():
    cur.execute("SELECT id, ma_the, ban_sao_id, ngay_muon, ngay_tra FROM muon_tra")
    rows = cur.fetchall()
    # check ma_the exists
    cur.execute("SELECT ma_the FROM doc_gia")
    dg_ids = set([r[0] for r in cur.fetchall()])
    cur.execute("SELECT id FROM ban_sao")
    bs_ids = set([r[0] for r in cur.fetchall()])
    missing_reader_ref = [r for r in rows if (r['ma_the'] is None or r['ma_the'] not in dg_ids)]
    missing_bansao_ref = [r for r in rows if (r['ban_sao_id'] is None or r['ban_sao_id'] not in bs_ids)]
    report_lines.append(f'Total muon_tra rows: {len(rows)}')
    report_lines.append(f'Borrow rows with missing/invalid reader ref: {len(missing_reader_ref)}')
    report_lines.append(f'Borrow rows with missing/invalid ban_sao ref: {len(missing_bansao_ref)}')
    for r in missing_reader_ref[:10]: report_lines.append(str(dict(r)))
    for r in missing_bansao_ref[:10]: report_lines.append(str(dict(r)))
    for r in missing_reader_ref + missing_bansao_ref:
        delete_statements.append(f"DELETE FROM muon_tra WHERE id={r['id']};")
else:
    report_lines.append('Table muon_tra not found')

# write report and deletes
out_dir = os.path.join(root, 'data')
if not os.path.exists(out_dir): os.makedirs(out_dir, exist_ok=True)
report_file = os.path.join(out_dir, 'cleanup_report.txt')
delete_file = os.path.join(out_dir, 'cleanup_delete.sql')
with open(report_file, 'w', encoding='utf-8') as f:
    f.write('\n'.join(report_lines))
print('Report written to', report_file)
with open(delete_file, 'w', encoding='utf-8') as f:
    f.write('-- SQL statements to clean the DB. Review before running.' + '\n')
    for s in delete_statements:
        f.write(s + '\n')
print('Delete SQL written to', delete_file)

if APPLY_DELETES:
    print('\nAPPLY_DELETES is True -> executing deletions (use with caution)')
    with open(delete_file, 'r', encoding='utf-8') as f:
        sqls = [l.strip() for l in f if l.strip() and not l.strip().startswith('--')]
    try:
        con.executescript('\n'.join(sqls))
        con.commit()
        print('Deletions applied.')
    except Exception as e:
        print('Error applying deletions:', e)

con.close()
print('Done')
