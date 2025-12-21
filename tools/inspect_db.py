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
print('DB path:', db_path)
con = sqlite3.connect(db_path)
cur = con.cursor()
for t in ('dau_sach','ban_sao','doc_gia','muon_tra'):
    try:
        cur.execute(f'SELECT COUNT(*) FROM {t}')
        print(f'{t}:', cur.fetchone()[0])
    except Exception as e:
        print(f'{t}: error ->', e)

# check danhmucsach.txt
dmf = os.path.join(files_dir, 'danhmucsach.txt')
print('\nChecking', dmf, 'for unmatched ISBNs...')
if os.path.exists(dmf):
    with open(dmf, encoding='utf-8') as f:
        for i, line in enumerate(f, start=1):
            line=line.strip()
            if not line: continue
            parts=line.split('|')
            if not parts: continue
            ma_sach=parts[0].strip()
            isbn=ma_sach.split('-')[0]
            cur.execute('SELECT id FROM dau_sach WHERE isbn=?', (isbn,))
            r=cur.fetchone()
            if not r:
                print('Line', i, 'ma_sach', ma_sach, '-> missing dau_sach for isbn', isbn)
else:
    print('danhmucsach.txt not found')
con.close()
