import re

def hex_to_rgb(h):
    h = h.lstrip('#')
    return tuple(int(h[i:i+2],16) for i in (0,2,4))

def srgb_to_linear(c):
    c = c/255.0
    return c/12.92 if c<=0.03928 else ((c+0.055)/1.055)**2.4

def luminance(rgb):
    r,g,b = rgb
    R = srgb_to_linear(r)
    G = srgb_to_linear(g)
    B = srgb_to_linear(b)
    return 0.2126*R + 0.7152*G + 0.0722*B

def contrast_ratio(hex1, hex2):
    L1 = luminance(hex_to_rgb(hex1))
    L2 = luminance(hex_to_rgb(hex2))
    lighter = max(L1,L2)
    darker = min(L1,L2)
    return (lighter+0.05)/(darker+0.05)

colors = ['#071233', '#232938', '#2c3e50', '#1a1f2e', '#041726']
against = ['#FFFFFF', '#E2E8F0']

out = []
for c in colors:
    out.append(f"Color {c} (rgb={hex_to_rgb(c)})")
    for a in against:
        cr = contrast_ratio(c,a)
        out.append(f"  vs {a}: contrast ratio = {cr:.2f}")
    out.append("")

path = 'tools/reports/colors_contrast.txt'
with open(path,'w',encoding='utf-8') as f:
    f.write('\n'.join(out))
print('Wrote contrast report to', path)
