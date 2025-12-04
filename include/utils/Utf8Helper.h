#ifndef UTF8HELPER_H
#define UTF8HELPER_H

#include <QString>
#include <string>

/**
 * @brief Utility class for safe UTF-8 conversion between QString and std::string
 * 
 * This class ensures proper UTF-8 encoding when converting between Qt's QString
 * (UTF-16 internally) and C++ std::string (used in backend with UTF-8 encoding).
 */
class Utf8Helper {
public:
    /**
     * @brief Convert QString to UTF-8 encoded std::string
     * @param qstr Input QString
     * @return UTF-8 encoded std::string
     */
    static std::string toStdString(const QString& qstr);
    
    /**
     * @brief Convert UTF-8 encoded std::string to QString
     * @param str Input UTF-8 encoded std::string
     * @return QString with proper Unicode support
     */
    static QString fromStdString(const std::string& str);
    
    /**
     * @brief Convert std::string to UTF-8 if not already (for file I/O)
     * @param str Input std::string (may be in any encoding)
     * @return UTF-8 encoded std::string
     */
    static std::string ensureUtf8(const std::string& str);
    
    /**
     * @brief Read UTF-8 line from file stream safely
     * @param stream Input file stream
     * @param line Output std::string to store the line
     * @return true if line was read successfully
     */
    static bool readUtf8Line(std::istream& stream, std::string& line);
};

#endif // UTF8HELPER_H
