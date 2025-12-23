#ifndef GLOBALSCANNERFILTER_H
#define GLOBALSCANNERFILTER_H

#include <QObject>
#include <QString>
#include <QElapsedTimer>

class GlobalScannerFilter : public QObject
{
    Q_OBJECT
public:
    explicit GlobalScannerFilter(QObject *parent = nullptr);
    
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    
private:
    QString scanBuffer;                    // Accumulate scanned characters
    QElapsedTimer scanTimer;               // Track timing between keystrokes
    static const int SCAN_TIMEOUT_MS = 100; // Max gap between scanner chars
    static const int MIN_BARCODE_LENGTH = 3; // Minimum valid barcode length
    
    bool isScannerInput() const;           // Detect if input pattern matches scanner
    void processScanCode(const QString& code); // Process complete barcode
    
signals:
    void scanDetected(const QString& barcode); // Emitted when valid barcode detected
};

#endif // GLOBALSCANNERFILTER_H
