#include "utils/Utf8Helper.h"
#include <iostream>

std::string Utf8Helper::toStdString(const QString& qstr) {
    return qstr.toUtf8().toStdString();
}

QString Utf8Helper::fromStdString(const std::string& str) {
    return QString::fromUtf8(str.c_str());
}

std::string Utf8Helper::ensureUtf8(const std::string& str) {
    // Assuming the input string is already UTF-8 or ASCII
    // In complex scenarios, we might need to detect encoding, but for now 
    // we trust the system uses UTF-8 consistently.
    return str;
}

bool Utf8Helper::readUtf8Line(std::istream& stream, std::string& line) {
    return (bool)std::getline(stream, line);
}
