#include "utils/GlobalScannerFilter.h"
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QDebug>

GlobalScannerFilter::GlobalScannerFilter(QObject *parent)
    : QObject(parent)
{
    scanTimer.start();
}

bool GlobalScannerFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        
        // Check if we should ignore this event (e.g., if it's from a text input that should handle it)
        QLineEdit *lineEdit = qobject_cast<QLineEdit*>(obj);
        
        // Enter/Return key = end of scan
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (isScannerInput() && !scanBuffer.isEmpty()) {
                qDebug() << "[GlobalScanner] ✓ Detected complete scan:" << scanBuffer;
                emit scanDetected(scanBuffer);
                scanBuffer.clear();
                
                // ONLY consume Enter key if NOT in a line edit
                // This prevents Enter from being processed twice
                if (!lineEdit) {
                    qDebug() << "[GlobalScanner] Consuming Enter key (scan complete)";
                    return true;
                }
            }
            scanBuffer.clear(); // Clear buffer on Enter regardless
        }
        // Accumulate printable characters (letters, digits, hyphens, etc.)
        else if (!keyEvent->text().isEmpty() && keyEvent->text().at(0).isPrint()) {
            qint64 elapsed = scanTimer.elapsed();
            
            // If too much time passed, this is a new scan sequence
            if (elapsed > SCAN_TIMEOUT_MS) {
                if (!scanBuffer.isEmpty()) {
                    qDebug() << "[GlobalScanner] Timeout - clearing buffer (was:" << scanBuffer << ")";
                }
                scanBuffer.clear();
            }
            
            // Add character to buffer
            QString newChar = keyEvent->text();
            scanBuffer += newChar;
            scanTimer.restart();
            
            qDebug() << "[GlobalScanner] Char:" << newChar << "| Buffer:" << scanBuffer << "| Elapsed:" << elapsed << "ms";
            
            // CRITICAL FIX: DO NOT consume events during scanning!
            // Let all characters pass through normally to avoid garbled input.
            // We only emit the accumulated buffer when Enter is pressed.
        }
        // Clear buffer on special keys (except modifiers)
        else if (keyEvent->key() != Qt::Key_Shift && 
                 keyEvent->key() != Qt::Key_Control && 
                 keyEvent->key() != Qt::Key_Alt &&
                 keyEvent->key() != Qt::Key_Meta) {
            if (!scanBuffer.isEmpty()) {
                qDebug() << "[GlobalScanner] Special key pressed, clearing buffer";
                scanBuffer.clear();
            }
        }
    }
    
    // ALWAYS pass events through during scanning
    // Only consume Enter key after scan is complete (handled above)
    return false;
}

bool GlobalScannerFilter::isScannerInput() const
{
    // Scanner characteristics:
    // 1. Fast input (< 100ms between chars)
    // 2. Minimum length (at least 3 chars for valid barcode)
    qint64 elapsed = scanTimer.elapsed();
    bool isFastInput = (elapsed < SCAN_TIMEOUT_MS);
    bool hasMinLength = (scanBuffer.length() >= MIN_BARCODE_LENGTH);
    
    return isFastInput && hasMinLength;
}

void GlobalScannerFilter::processScanCode(const QString& code)
{
    // This method can be used for additional processing if needed
    // Currently, we just emit the signal
    emit scanDetected(code);
}
