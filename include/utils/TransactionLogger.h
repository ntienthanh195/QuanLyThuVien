#ifndef TRANSACTIONLOGGER_H
#define TRANSACTIONLOGGER_H

#include <QString>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include "models/DocGia.h"
#include "models/DauSach.h"

/**
 * @brief TransactionLogger - Ghi lịch sử giao dịch mượn/trả sách
 * 
 * File log: data/muontralogs.txt
 * Format: timestamp|maThe|hoTen|ISBN|tenSach|loai|trangThai
 * - timestamp: yyyy-MM-dd HH:mm:ss
 * - maThe: Mã thẻ độc giả
 * - hoTen: Họ tên đầy đủ
 * - ISBN: ISBN sách (hoặc mã sách)
 * - tenSach: Tên sách
 * - loai: MUON hoặc TRA hoặc MAT
 * - trangThai: Trạng thái giao dịch (thành công/thất bại)
 */
class TransactionLogger {
public:
    enum LoaiGiaoDich {
        MUON,
        TRA,
        MAT
    };
    
    /**
     * @brief Ghi log giao dịch mượn sách
     * @param docGia Con trỏ đến độc giả
     * @param isbn ISBN/mã sách
     * @param tenSach Tên sách
     * @param thanhCong true nếu giao dịch thành công
     * @return true nếu ghi log thành công
     */
    static bool ghiLogMuon(PTRDG docGia, const QString& isbn, const QString& tenSach, bool thanhCong = true);
    
    /**
     * @brief Ghi log giao dịch trả sách
     * @param docGia Con trỏ đến độc giả
     * @param maSach Mã sách trả
     * @param tenSach Tên sách
     * @param thanhCong true nếu giao dịch thành công
     * @return true nếu ghi log thành công
     */
    static bool ghiLogTra(PTRDG docGia, const QString& maSach, const QString& tenSach, bool thanhCong = true);
    
    /**
     * @brief Ghi log báo mất sách
     * @param docGia Con trỏ đến độc giả
     * @param maSach Mã sách mất
     * @param tenSach Tên sách
     * @return true nếu ghi log thành công
     */
    static bool ghiLogMat(PTRDG docGia, const QString& maSach, const QString& tenSach);
    
    /**
     * @brief Đọc toàn bộ lịch sử giao dịch
     * @return Nội dung file log dạng QString
     */
    static QString docLichSu();
    
    /**
     * @brief Đọc lịch sử của một độc giả cụ thể
     * @param maThe Mã thẻ độc giả
     * @return Danh sách giao dịch của độc giả
     */
    static QStringList docLichSuDocGia(int maThe);
    
private:
    static const QString LOG_FILE_PATH;
    
    /**
     * @brief Ghi một dòng log vào file
     * @param loai Loại giao dịch
     * @param docGia Con trỏ độc giả
     * @param maSach Mã sách/ISBN
     * @param tenSach Tên sách
     * @param thanhCong Trạng thái giao dịch
     * @return true nếu ghi thành công
     */
    static bool ghiLog(LoaiGiaoDich loai, PTRDG docGia, const QString& maSach, 
                      const QString& tenSach, bool thanhCong);
    
    /**
     * @brief Chuyển loại giao dịch sang string
     */
    static QString loaiGiaoDichToString(LoaiGiaoDich loai);
};

#endif // TRANSACTIONLOGGER_H
