#ifndef CODE128_H
#define CODE128_H

#include <QString>
#include <QVector>
#include <QStringList>

class Code128 {
public:
    // Generates the boolean pattern (true=bar, false=space)
    // moduleWidth: width in pixels per module (bar/space unit)
    static QVector<bool> encode(const QString &text);

    // Creates an SVG or HTML-ready pattern string (1=bar, 0=space)
    static QString encodeToString(const QString &text);
    
private:
    static int getCode128Value(char c);
    static const int CODE128_B_START;
    static const int CODE128_C_START;
    static const int CODE128_STOP;
    static const int patterns[][6]; // Widths of [Bar, Space, Bar, Space, Bar, Space]
};

#endif // CODE128_H
