#include "qrcode/Code128.h"
#include <QDebug>

// Code128 logic based on standard specification
// Tables storing width of bars and spaces. Each character has 3 bars and 3 spaces (except Stop).
// Representation: B S B S B S (widths 1..4)
// Total width = 11 modules per char (except Stop = 13)

const int Code128::patterns[][6] = {
    {2,1,2,2,2,2},{2,2,2,1,2,2},{2,2,2,2,2,1},{1,2,1,2,2,3},{1,2,1,3,2,2},{1,3,1,2,2,2},{1,2,2,2,1,3},{1,2,2,3,1,2}, // 0-7
    {1,3,2,2,1,2},{2,2,1,2,1,3},{2,2,1,3,1,2},{2,3,1,2,1,2},{1,1,2,2,3,2},{1,2,2,1,3,2},{1,2,2,2,3,1},{1,1,3,2,2,2}, // 8-15
    {1,2,3,1,2,2},{1,2,3,2,2,1},{2,2,3,2,1,1},{2,2,1,1,3,2},{2,2,1,2,3,1},{2,1,3,2,1,2},{2,2,3,1,1,2},{3,1,2,1,3,1}, // 16-23
    {3,1,1,2,2,2},{3,2,1,1,2,2},{3,2,1,2,2,1},{3,1,2,2,1,2},{3,2,2,1,1,2},{3,2,2,2,1,1},{2,1,2,1,2,3},{2,1,2,3,2,1}, // 24-31
    {2,3,2,1,2,1},{1,1,1,3,2,3},{1,3,1,1,2,3},{1,3,1,3,2,1},{1,1,2,3,1,3},{1,3,2,1,1,3},{1,3,2,3,1,1},{2,1,1,3,1,3}, // 32-39
    {2,3,1,1,1,3},{2,3,1,3,1,1},{1,1,2,1,3,3},{1,1,2,3,3,1},{1,3,2,1,3,1},{1,1,3,1,2,3},{1,1,3,3,2,1},{1,3,3,1,2,1}, // 40-47
    {3,1,3,1,2,1},{2,1,1,3,3,1},{2,3,1,1,3,1},{2,1,3,1,1,3},{2,1,3,3,1,1},{2,1,3,1,3,1},{3,1,1,1,2,3},{3,1,1,3,2,1}, // 48-55
    {3,3,1,1,2,1},{3,1,2,1,1,3},{3,1,2,3,1,1},{3,3,2,1,1,1},{3,1,4,1,1,1},{2,2,1,4,1,1},{4,3,1,1,1,1},{1,1,1,2,2,4}, // 56-63
    {1,1,1,4,2,2},{1,2,1,1,2,4},{1,2,1,4,2,1},{1,4,1,1,2,2},{1,4,1,2,2,1},{1,1,2,2,1,4},{1,1,2,4,1,2},{1,2,2,1,1,4}, // 64-71
    {1,2,2,4,1,1},{1,4,2,1,1,2},{1,4,2,2,1,1},{2,4,1,2,1,1},{2,2,1,1,1,4},{4,1,3,1,1,1},{2,4,1,1,1,2},{1,3,4,1,1,1}, // 72-79
    {1,1,1,2,4,2},{1,2,1,1,4,2},{1,2,1,2,4,1},{1,1,4,2,1,2},{1,2,4,1,1,2},{1,2,4,2,1,1},{4,1,1,2,1,2},{4,2,1,1,1,2}, // 80-87
    {4,2,1,2,1,1},{2,1,2,1,4,1},{2,1,4,1,2,1},{4,1,2,1,2,1},{1,1,1,1,4,3},{1,1,1,3,4,1},{1,3,1,1,4,1},{1,1,4,1,1,3}, // 88-95
    {1,1,4,3,1,1},{4,1,1,1,1,3},{4,1,1,3,1,1},{1,1,3,1,4,1},{1,1,4,1,3,1},{3,1,1,1,4,1},{4,1,1,1,3,1},{2,1,1,4,1,2}, // 96-103
    {2,1,1,2,1,4},{2,1,1,2,3,2} // 104, 105 (Start B, Start C partially) - Wait, table is standard up to 102.
    // Spec: 103=StartA, 104=StartB, 105=StartC, 106=Stop
};

// Start A/B/C and Stop are special.
// Start A (103): 211412
// Start B (104): 211214
// Start C (105): 211232
// Stop  (106): 2331112 (7 elements!) -> handled separately

const int Code128::CODE128_B_START = 104;
const int Code128::CODE128_C_START = 105;
const int Code128::CODE128_STOP = 106;

QVector<bool> Code128::encode(const QString &text) {
    QVector<bool> result;
    if (text.isEmpty()) return result;

    // Determine Mode: Try to use C (digits) if possible for density
    // Simple logic: If strict digits and even length -> C. Else B.
    // (A standard optimizer would switch modes mid-string, but here keep it simple to fix user issue)
    
    bool useC = true;
    for (QChar c : text) {
        if (!c.isDigit()) { useC = false; break; }
    }
    if (useC && (text.length() % 2 != 0)) useC = false; // Code C requires pairs
    // Actually standard allows odd length by switching, but pure C needs pairs.
    // If odd digits, we can pad '0' at start? No, that changes value. Use B if odd or mixed.

    int startCode = useC ? CODE128_C_START : CODE128_B_START;
    int checkSum = startCode;
    
    // Add Start Pattern
    auto addPattern = [&](int idx) {
        if (idx < 0 || idx > 106) return;
        if (idx == 106) {
           // Stop pattern 2,3,3,1,1,1,2
           int stopPat[] = {2,3,3,1,1,1,2};
           for (int w : stopPat) {
               bool bar = (result.isEmpty()) ? true : !result.last(); // Alternating logic? No.
               // wait, we construct Bar/Space sequence.
               // Actually logic: B S B S B S (B S B S B S B for stop)
           }
        }
    };
    
    QVector<int> codes;
    codes.append(startCode);

    if (useC) {
        for (int i = 0; i < text.length(); i += 2) {
            int val = text.mid(i, 2).toInt();
            codes.append(val);
        }
    } else {
        for (int i = 0; i < text.length(); ++i) {
            int val = text[i].toLatin1() - 32; // Code 128B map: ASCII 32..127 -> 0..95
            if (val < 0) val = 0; // fallback
            codes.append(val);
        }
    }

    // Calculate Checksum
    for (int i = 1; i < codes.size(); ++i) {
        checkSum += codes[i] * i;
    }
    checkSum %= 103;
    codes.append(checkSum);
    codes.append(CODE128_STOP);

    // Convert codes to boolean pattern
    // Logic: Each code maps to 6 widths (B S B S B S)
    // Start with Bar.
    
    for (int code : codes) {
        if (code == CODE128_STOP) {
            // Stop: 2 3 3 1 1 1 2 (B S B S B S B)
            int widths[] = {2,3,3,1,1,1,2};
            bool isBar = true;
            for (int w : widths) {
                for (int k=0; k<w; ++k) result.append(isBar);
                isBar = !isBar;
            }
        } else {
            // Standard: 6 widths
            // Handle Start codes from manual defines if table is short
            int const* p = nullptr;
            int manualStartB[] = {2,1,1,2,1,4};
            int manualStartC[] = {2,1,1,2,3,2};
            
            if (code < 103) p = patterns[code];
            else if (code == 104) p = manualStartB;
            else if (code == 105) p = manualStartC;
            
            if (p) {
                bool isBar = true;
                for (int i=0; i<6; ++i) {
                     for (int k=0; k<p[i]; ++k) result.append(isBar);
                     isBar = !isBar;
                }
            }
        }
    }

    return result;
}

QString Code128::encodeToString(const QString &text) {
    QVector<bool> bits = encode(text);
    QString s;
    for (bool b : bits) s += (b ? "1" : "0");
    return s;
}
