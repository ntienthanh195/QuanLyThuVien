#!/usr/bin/env python3
"""
Remove recently-added Vietnamese Buddhist book rows from import_sach.csv and DB,
then append generated Khmer Buddhist book rows (with Khmer authors) to CSV and insert into DB.
"""
import os, csv, sqlite3, shutil, datetime, re, random

root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
data_csv = os.path.join(root, 'data', 'import_sach.csv')
db_path = os.path.join(root, 'data', 'files', 'library.db')

if not os.path.exists(data_csv):
    print('CSV not found:', data_csv); raise SystemExit(1)
if not os.path.exists(db_path):
    print('DB not found:', db_path); raise SystemExit(1)

# Read CSV
with open(data_csv, 'r', encoding='utf-8-sig', newline='') as f:
    rows = list(csv.reader(f))
if not rows:
    print('CSV empty'); raise SystemExit(1)
headers = rows[0]
body = rows[1:]

# Identify VN rows to remove: genre == 'Phật giáo' OR title contains '(Tái bản'
vn_rows = []
vn_isbns = []
new_body = []
for r in body:
    # guard
    if not r: continue
    genre = r[5] if len(r)>5 else ''
    title = r[1] if len(r)>1 else ''
    if ('Phật giáo' in genre) or ('(Tái bản' in title):
        vn_rows.append(r)
        if r and r[0]: vn_isbns.append(r[0].lstrip('\u000b'))
    else:
        new_body.append(r)

print('Found', len(vn_rows), 'Vietnamese book rows to remove')

# Backup CSV
bak_csv = data_csv + '.bak_' + datetime.datetime.now().strftime('%Y%m%d_%H%M%S')
shutil.copy2(data_csv, bak_csv)
print('CSV backup:', bak_csv)

# Remove VN rows and write back CSV (we will append Khmer rows later)
with open(data_csv, 'w', encoding='utf-8', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(headers)
    for r in new_body:
        writer.writerow(r)
print('Wrote CSV without Vietnamese rows')

# Delete VN books from DB (ban_sao then dau_sach)
con = sqlite3.connect(db_path)
cur = con.cursor()
cur.execute('PRAGMA foreign_keys = OFF')
for isbn in vn_isbns:
    if not isbn: continue
    # find dau_sach id
    cur.execute('SELECT id FROM dau_sach WHERE isbn=?', (isbn,))
    row = cur.fetchone()
    if row:
        dau_id = row[0]
        # delete ban_sao
        cur.execute('DELETE FROM ban_sao WHERE dau_sach_id=?', (dau_id,))
        # delete dau_sach
        cur.execute('DELETE FROM dau_sach WHERE id=?', (dau_id,))
        print('Deleted dau_sach and its copies for ISBN', isbn)
    else:
        print('No dau_sach found for ISBN', isbn)
con.commit()
cur.execute('PRAGMA foreign_keys = ON')
con.close()

# Generate 10 Khmer Buddhist book rows (similar to generator in repo)
khmer_titles = [
    "ពុទ្ធសាសនា និងជីវិត", "ព្រះធម៌ក្នុងជីវិតប្រចាំថ្ងៃ", "ចរិតព្រះពុទ្ធសាសនា", "ធម៌សម្រាប់សង្គម",
    "សូត្រខ្មែរ - ការសិក្សា", "បុរាណពុទ្ធសាសនា", "វិចារណសាស្ត្រ និងពុទ្ធសាសនា", "ការ​បង្រៀន​ពុទ្ធ​សាសនា",
    "ពិធីបុណ្យ និងប្រពៃណី", "ព្រះពុទ្ធនិងសិល្បៈ", "ចម្រៀងព្រះពុទ្ធ", "ព្រះធម៌សម្រាប់ក្មេងៗ",
    "ព្រះធម៌វិចារ", "និទានពុទ្ធសាសនា", "សំណេរពុទ្ធសាសនា", "ជីវចក្រ​ពុទ្ធសាសនា",
    "ប្រវត្តិសាស្ត្រពុទ្ធសាសនា", "ការអប់រំព្រះពុទ្ធ", "ធម៌ស្ដីពីការអត្រា", "សេចក្ដីអនុស្សាវរីយ៍"
]
khmer_authors = ["ព្រះសង្ឃ វត្ត", "សម្តេច សុភ័ក្រ", "គ្រូ សុខា", "គ្រូ ឃុន ចន្ធ្រា", "ព្រះសង្ឃ ទាយ"]

# helper isbn generator
import random

def isbn13_checksum(d12):
    total = 0
    for i,ch in enumerate(d12):
        n = int(ch)
        total += n if i%2==0 else n*3
    return str((10 - (total%10))%10)

def gen_isbn13(existing):
    while True:
        core = '978' + ''.join(str(random.randint(0,9)) for _ in range(9))
        check = isbn13_checksum(core)
        isbn = core + check
        if isbn not in existing:
            return isbn

# read existing isbns
existing = set()
with open(data_csv, 'r', encoding='utf-8-sig', newline='') as f:
    reader = csv.reader(f)
    rows2 = list(reader)
    for r in rows2[1:]:
        if r and r[0]: existing.add(r[0])

new_rows = []
for i in range(10):
    isbn = gen_isbn13(existing)
    existing.add(isbn)
    title = khmer_titles[(i) % len(khmer_titles)] + f" (ភាសាខ្មែរ {i+1})"
    author = khmer_authors[i % len(khmer_authors)]
    year = random.randint(2000, 2024)
    genre = "ពុទ្ធសាសនា"
    location = f"Ke KP{(i%10)+1}"
    copies = 5
    new_rows.append([isbn, title, str(random.randint(150,400)), author, str(year), genre, location, str(copies)])

# append new Khmer rows to CSV
with open(data_csv, 'a', encoding='utf-8', newline='') as f:
    writer = csv.writer(f)
    for r in new_rows:
        writer.writerow(r)
print('Appended', len(new_rows), 'Khmer book rows to CSV')

# Run apply_samples_to_db.py to insert new rows into DB
print('Now inserting new rows into DB...')
apply_script = os.path.join(root, 'tools', 'apply_samples_to_db.py')
if os.path.exists(apply_script):
    os.system(f'"{os.path.join(root, ".venv\\Scripts\\python.exe")}" "{apply_script}"')
else:
    print('apply_samples_to_db.py not found; skip DB insert')

print('Done')
