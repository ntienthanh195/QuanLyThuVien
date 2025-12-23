#include "utils/TransactionLogger.h"
#include "utils/FilePathManager.h"
#include <QDir>
#include <QDebug>

// Removed hardcoded LOG_FILE_PATH (static const)

bool TransactionLogger::ghiLogMuon(PTRDG docGia, const QString& isbn, const QString& tenSach, bool thanhCong) {
    return ghiLog(MUON, docGia, isbn, tenSach, thanhCong);
}

bool TransactionLogger::ghiLogTra(PTRDG docGia, const QString& maSach, const QString& tenSach, bool thanhCong) {
    return ghiLog(TRA, docGia, maSach, tenSach, thanhCong);
}

bool TransactionLogger::ghiLogMat(PTRDG docGia, const QString& maSach, const QString& tenSach) {
    return ghiLog(MAT, docGia, maSach, tenSach, true);
}

bool TransactionLogger::ghiLog(LoaiGiaoDich loai, PTRDG docGia, const QString& maSach, 
                               const QString& tenSach, bool thanhCong) {
    if (!docGia) {
        qWarning() << "TransactionLogger: docGia is null";
        return false;
    }
    
    // Mở file ở chế độ append (Phase 11: Dynamic Path)
    QString logPath = FilePathManager::getFilePath("muontralogs.txt");
    QFile file(logPath);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Không thể mở file log:" << logPath;
        return false;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    
    // Format: timestamp|maThe|hoTen|ISBN|tenSach|loai|trangThai
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString hoTen = QString::fromStdString(docGia->data.Ho) + " " + QString::fromStdString(docGia->data.Ten);
    QString loaiStr = loaiGiaoDichToString(loai);
    QString trangThaiStr = thanhCong ? "THANH_CONG" : "THAT_BAI";
    
    QString logLine = QString("%1|%2|%3|%4|%5|%6|%7\n")
                          .arg(timestamp)
                          .arg(docGia->data.MaThe)
                          .arg(hoTen)
                          .arg(maSach)
                          .arg(tenSach)
                          .arg(loaiStr)
                          .arg(trangThaiStr);
    
    out << logLine;
    file.close();
    
    qDebug() << "Đã ghi log:" << loaiStr << "-" << hoTen << "-" << tenSach;
    return true;
}

QString TransactionLogger::loaiGiaoDichToString(LoaiGiaoDich loai) {
    switch (loai) {
        case MUON: return "MUON";
        case TRA: return "TRA";
        case MAT: return "MAT";
        default: return "UNKNOWN";
    }
}

QString TransactionLogger::docLichSu() {
    QString logPath = FilePathManager::getFilePath("muontralogs.txt");
    QFile file(logPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Không thể đọc file log:" << logPath;
        return QString();
    }
    
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    file.close();
    
    return content;
}

QStringList TransactionLogger::docLichSuDocGia(int maThe) {
    QStringList result;
    
    QString logPath = FilePathManager::getFilePath("muontralogs.txt");
    QFile file(logPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return result;
    }
    
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split('|');
        
        // Format: timestamp|maThe|hoTen|ISBN|tenSach|loai|trangThai
        if (parts.size() >= 7) {
            int maTheLog = parts[1].toInt();
            if (maTheLog == maThe) {
                result.append(line);
            }
        }
    }
    
    file.close();
    return result;
}
