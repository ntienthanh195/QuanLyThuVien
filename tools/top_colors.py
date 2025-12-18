import re, os
from collections import Counter

root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
patterns = [re.compile(r"#[0-9A-Fa-f]{6}\b"), re.compile(r"QColor\s*\([^\)]*\)"), re.compile(r"rgb\s*\([^\)]*\)")]
file_exts = {'.cpp', '.cc', '.c', '.h', '.hpp', '.qss', '.css', '.ui', '.rc', '.pro', '.txt', '.md', '.py', '.qml', '.qrc'}
counts = Counter()
locations = {}
for dirpath, dirnames, filenames in os.walk(root):
    for fn in filenames:
        ext = os.path.splitext(fn)[1].lower()
        if ext not in file_exts:
            continue
        path = os.path.join(dirpath, fn)
        try:
            with open(path, 'r', encoding='utf-8', errors='ignore') as f:
                lines = f.readlines()
        except:
            continue
        for i,line in enumerate(lines, start=1):
            for p in patterns:
                for m in p.finditer(line):
                    token = m.group(0)
                    counts[token]+=1
                    locations.setdefault(token,[]).append(f"{os.path.relpath(path, root)}:{i}")

out_lines = []
out_lines.append(f"Top colors by occurrence (total unique: {len(counts)})\n")
for token, c in counts.most_common(60):
    out_lines.append(f"{token} - {c} occurrences")
    for loc in locations[token][:5]:
        out_lines.append(f"  - {loc}")
    if len(locations[token])>5:
        out_lines.append(f"  - ... and {len(locations[token])-5} more")
    out_lines.append("")

os.makedirs(os.path.join(root,'tools','reports'), exist_ok=True)
path = os.path.join(root,'tools','reports','top_colors.txt')
with open(path,'w',encoding='utf-8') as f:
    f.write('\n'.join(out_lines))
print('Wrote', path)
