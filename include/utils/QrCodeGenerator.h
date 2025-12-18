#ifndef QRCODEGENERATOR_H
#define QRCODEGENERATOR_H

#include <QImage>
#include <QString>

/**
 * @brief Simple QR Code generator wrapper
 * Sử dụng thư viện qrcodegen của Nayuki
 */
class QrCodeGenerator {
public:
    /**
     * @brief Tạo ảnh QR code từ văn bản
     * @param text Văn bản cần mã hóa
     * @param size Kích thước ảnh đầu ra (pixel)
     * @param border Lề trắng xung quanh (số module)
     * @return QImage chứa mã QR, hoặc QImage rỗng nếu lỗi
     */
    static QImage generateQrImage(const QString& text, int size = 200, int border = 2);
    
    /**
     * @brief Lưu ảnh QR vào file
     * @param text Văn bản cần mã hóa
     * @param filePath Đường dẫn file đầu ra
     * @param size Kích thước ảnh
     * @return true nếu thành công
     */
    static bool saveQrToFile(const QString& text, const QString& filePath, int size = 200);

private:
    QrCodeGenerator() = delete; // Utility class, không khởi tạo
};

#endif // QRCODEGENERATOR_H
