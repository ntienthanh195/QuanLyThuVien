#!/usr/bin/env python3
import random, os, csv, datetime

root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
csv_path = os.path.join(root, 'data', 'import_sach.csv')

# compute ISBN-13 checksum for 12-digit string
def isbn13_checksum(d12):
    total = 0
    for i, ch in enumerate(d12):
        n = int(ch)
        total += n if i % 2 == 0 else n * 3
    check = (10 - (total % 10)) % 10
    return str(check)

# generate unique isbn13
def gen_isbn13(existing):
    while True:
        core = '978' + ''.join(str(random.randint(0,9)) for _ in range(9))
        check = isbn13_checksum(core)
        isbn = core + check
        if isbn not in existing:
            return isbn

# sample Khmer titles/authors (20)
khmer_titles = [
    "ពុទ្ធសាសនា និងជីវិត", "ព្រះធម៌ក្នុងជីវិតប្រចាំថ្ងៃ", "ចរិតព្រះពុទ្ធសាសនា", "ធម៌សម្រាប់សង្គម",
    "សូត្រខ្មែរ - ការសិក្សា", "បុរាណពុទ្ធសាសនា", "វិចារណសាស្ត្រ និងពុទ្ធសាសនា", "ការ​បង្រៀន​ពុទ្ធ​សាសនា",
    "ពិធីបុណ្យ និងប្រពៃណី", "ព្រះពុទ្ធនិងសិល្បៈ", "ចម្រៀងព្រះពុទ្ធ", "ព្រះធម៌សម្រាប់ក្មេងៗ",
    "ព្រះធម៌វិចារ", "និទានពុទ្ធសាសនា", "សំណេរពុទ្ធសាសនា", "ជីវចក្រ​ពុទ្ធសាសនា",
    "ប្រវត្តិសាស្ត្រពុទ្ធសាសនា", "ការអប់រំព្រះពុទ្ធ", "ធម៌ស្ដីពីការអត្រា", "សេចក្ដីអនុស្សាវរីយ៍"
]
khmer_authors = ["ព្រះសង្ឃ វត្ត", "សម្តេច សុភ័ក្រ", "គ្រូ សុខា", "គ្រូ ឃុន ចន្ធ្រា", "ព្រះសង្ឃ ទាយ"]

# sample Vietnamese Buddhist titles (10)
vn_titles = [
    "Dharma cho đời sống", "Thiền và cuộc sống", "Giáo lý Phật giáo căn bản", "Hành thiền cho người mới",
    "Tri thức về Phật giáo", "Lễ hội Phật giáo Việt Nam", "Thiền sư và bài học", "Tâm và Thiền",
    "Sống an lạc theo Phật", "Những bản kinh chọn lọc"
]
vn_authors = ["Thích Nhất Hạnh", "Thích Thanh Từ", "Thích Phước Hải", "Thích Nhật Từ"]

# read existing isbns to avoid duplicates
existing = set()
if os.path.exists(csv_path):
    with open(csv_path, 'r', encoding='utf-8-sig', newline='') as f:
        reader = csv.reader(f)
        rows = list(reader)
        for r in rows[1:]:
            if r:
                isbn = r[0].lstrip('\u000b') if r[0] else ''
                existing.add(isbn)

new_rows = []
# generate 20 Khmer Buddhist books
for i in range(20):
    isbn = gen_isbn13(existing)
    existing.add(isbn)
    title = khmer_titles[i % len(khmer_titles)] + f" (Khmer Edition {i+1})"
    author = khmer_authors[i % len(khmer_authors)]
    year = random.randint(2000, 2024)
    genre = "ពុទ្ធសាសនា"
    location = f"Ke KP{(i%10)+1}"
    copies = 5
    new_rows.append([isbn, title, str(random.randint(150,400)), author, str(year), genre, location, str(copies)])

# generate 10 Vietnamese Buddhist books
for i in range(10):
    isbn = gen_isbn13(existing)
    existing.add(isbn)
    title = vn_titles[i % len(vn_titles)] + f" (Tái bản {i+1})"
    author = vn_authors[i % len(vn_authors)]
    year = random.randint(1995, 2024)
    genre = "Phật giáo"
    location = f"Ke PV{(i%10)+1}"
    copies = 5
    new_rows.append([isbn, title, str(random.randint(120,500)), author, str(year), genre, location, str(copies)])

# append to CSV
with open(csv_path, 'a', encoding='utf-8', newline='') as f:
    writer = csv.writer(f)
    for r in new_rows:
        writer.writerow(r)

print(f'Appended {len(new_rows)} book rows to {csv_path}')
