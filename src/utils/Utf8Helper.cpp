#include "utils/Utf8Helper.h"
#include <QByteArray>
#include <fstream>

std::string Utf8Helper::toStdString(const QString& qstr) {
    // Convert QString to UTF-8 encoded QByteArray, then to std::string
    QByteArray utf8Bytes = qstr.toUtf8();
    return std::string(utf8Bytes.constData(), utf8Bytes.size());
}

QString Utf8Helper::fromStdString(const std::string& str) {
    // Assume std::string is UTF-8 encoded and convert to QString
    return QString::fromUtf8(str.c_str(), static_cast<int>(str.size()));
}

std::string Utf8Helper::ensureUtf8(const std::string& str) {
    // Convert to QString and back to ensure UTF-8 encoding
    QString qstr = QString::fromUtf8(str.c_str());
    return toStdString(qstr);
}

bool Utf8Helper::readUtf8Line(std::istream& stream, std::string& line) {
    if (!std::getline(stream, line)) {
        return false;
    }
    
    // Ensure the line is UTF-8 encoded
    line = ensureUtf8(line);
    return true;
}
