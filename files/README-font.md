DejaVu Sans (recommended)

This project uses DejaVu Sans as a freely distributable fallback font for the SFML UI.

Where it came from
- DejaVu fonts (DejaVu Sans) — open font family derived from Bitstream Vera.
- Official repository: https://github.com/dejavu-fonts/dejavu-fonts

License
- The DejaVu fonts are distributed under the Bitstream Vera (public domain-like) and the SIL Open Font License (OFL). You may redistribute this font with the project.

How to use
- Place `DejaVuSans.ttf` into `files/` (this repository) and the application will automatically pick it up.
- If you prefer not to commit the font, download it from the link above and place it into `files/` on your machine.

Notes
- Do NOT commit proprietary fonts (e.g., Arial) if you plan to make the repository public.
- If you want me to include the font binary in the repo, confirm and I'll download it into `files/DejaVuSans.ttf` and record the source and license here.
Included fallback (convenience)
- This repository includes a small fallback font copied from the SFML examples (`tuffy.ttf`) as `files/DejaVuSans.ttf` so the project can run out-of-the-box. The original `tuffy.ttf` file is bundled with the SFML examples that are included in `lib/SFML/examples` in this repository.

If you prefer the official DejaVu Sans, replace `files/DejaVuSans.ttf` with your preferred open font and update this README accordingly.
