#include "utils/QrCodeGenerator.h"
#include "qrcode/qrcodegen.hpp"
#include <QPainter>
#include <QFile>

using namespace qrcodegen;

QImage QrCodeGenerator::generateQrImage(const QString& text, int size, int border) {
    if (text.isEmpty() || size <= 0) {
        return QImage();
    }
    
    try {
        // Tạo QR code từ text (sử dụng error correction level MEDIUM)
        QrCode qr = QrCode::encodeText(text.toUtf8().constData(), QrCode::Ecc::MEDIUM);
        
        // Tính toán kích thước
        int qrSize = qr.getSize();
        int scale = size / (qrSize + 2 * border);
        if (scale < 1) scale = 1;
        
        int finalSize = (qrSize + 2 * border) * scale;
        
        // Tạo ảnh với nền trắng
        QImage image(finalSize, finalSize, QImage::Format_RGB32);
        image.fill(Qt::white);
        
        // Vẽ QR code
        QPainter painter(&image);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);
        
        for (int y = 0; y < qrSize; y++) {
            for (int x = 0; x < qrSize; x++) {
                if (qr.getModule(x, y)) {
                    int px = (x + border) * scale;
                    int py = (y + border) * scale;
                    painter.drawRect(px, py, scale, scale);
                }
            }
        }
        
        return image;
        
    } catch (const std::exception& e) {
        qWarning("QR Code generation failed: %s", e.what());
        return QImage();
    }
}

bool QrCodeGenerator::saveQrToFile(const QString& text, const QString& filePath, int size) {
    QImage qrImage = generateQrImage(text, size);
    if (qrImage.isNull()) {
        return false;
    }
    
    return qrImage.save(filePath);
}
