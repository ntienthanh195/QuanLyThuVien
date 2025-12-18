import re
import os
from collections import defaultdict

root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
patterns = {
    'hex6': re.compile(r"#[0-9A-Fa-f]{6}\b"),
    'hex3': re.compile(r"#[0-9A-Fa-f]{3}\b"),
    'rgb': re.compile(r"rgb\s*\([^\)]*\)"),
    'qcolor': re.compile(r"QColor\s*\([^\)]*\)"),
}
file_exts = {'.cpp', '.cc', '.c', '.h', '.hpp', '.qss', '.css', '.ui', '.rc', '.pro', '.txt', '.md', '.py', '.qml', '.qrc'}

results = defaultdict(list)

for dirpath, dirnames, filenames in os.walk(root):
    # skip hidden .git etc
    if '.git' in dirpath:
        continue
    for fn in filenames:
        ext = os.path.splitext(fn)[1].lower()
        if ext not in file_exts:
            continue
        path = os.path.join(dirpath, fn)
        try:
            with open(path, 'r', encoding='utf-8', errors='ignore') as f:
                lines = f.readlines()
        except Exception as e:
            continue
        for i, line in enumerate(lines, start=1):
            for k, p in patterns.items():
                for m in p.finditer(line):
                    token = m.group(0)
                    results[token].append(f"{os.path.relpath(path, root)}:{i}")

out_lines = []
out_lines.append(f"Scanned root: {root}\n")
out_lines.append(f"Found {len(results)} unique color tokens.\n\n")
for token, places in sorted(results.items()):
    out_lines.append(f"{token} ({len(places)} occurrences)")
    for p in places[:10]:
        out_lines.append(f"  - {p}")
    if len(places) > 10:
        out_lines.append(f"  - ... and {len(places)-10} more\n")
    else:
        out_lines.append("")

os.makedirs(os.path.join(root, 'tools', 'reports'), exist_ok=True)
report_path = os.path.join(root, 'tools', 'reports', 'colors_report_raw.txt')
with open(report_path, 'w', encoding='utf-8') as outf:
    outf.write('\n'.join(out_lines))

print('Wrote report to', report_path)
