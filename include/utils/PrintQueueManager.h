#ifndef PRINTQUEUEMANAGER_H
#define PRINTQUEUEMANAGER_H

#include <QList>
#include <QObject>
#include "models/DocGia.h"

class QWidget;

/**
 * @brief PrintQueueManager - Quản lý hàng đợi in thẻ độc giả
 * 
 * Singleton pattern để share giữa các dialog
 * Quản lý danh sách độc giả cần in thẻ hàng loạt
 */
class PrintQueueManager : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief Lấy instance duy nhất (singleton)
     */
    static PrintQueueManager* instance();
    
    /**
     * @brief Thêm độc giả vào hàng đợi in
     * @param docGia Con trỏ độc giả
     * @return true nếu thêm thành công, false nếu đã tồn tại
     */
    bool them(PTRDG docGia);
    
    /**
     * @brief Xóa độc giả khỏi hàng đợi theo index
     * @param index Vị trí trong danh sách (0-based)
     */
    void xoa(int index);
    
    /**
     * @brief Xóa toàn bộ hàng đợi
     */
    void xoaTatCa();
    
    /**
     * @brief Lấy danh sách độc giả trong hàng đợi
     */
    QList<PTRDG> layDanhSach() const;
    
    /**
     * @brief Số lượng độc giả trong hàng đợi
     */
    int soLuong() const;
    
    /**
     * @brief Kiểm tra độc giả đã có trong hàng đợi chưa
     * @param maThe Mã thẻ độc giả
     */
    bool daTonTai(int maThe) const;

    /**
     * @brief In hàng đợi hiện tại (mở dialog in)
     * @param parent Parent widget cho dialog in (có thể null)
     */
    void inHangDoi(QWidget *parent = nullptr);

    /**
     * @brief Xuất hàng đợi thành file PDF
     * @param filePath Đường dẫn file PDF đầu ra
     * @param title Tiêu đề trang in
     * @return true nếu xuất thành công
     */
    bool xuatHangDoiThanhPdf(const QString &filePath, const QString &title = "Danh sách độc giả");
    
    enum class ExportLayout { CardSize, ReportA4 };

    /**
    * @brief Xuất sample PDF dùng layout đã chọn và lưu vào data/sample_print.pdf
     */
    bool xuatSamplePdf(ExportLayout layout = ExportLayout::ReportA4);
    
signals:
    /**
     * @brief Signal phát ra khi hàng đợi thay đổi
     */
    void hangDoiThayDoi();
    
private:
    // Singleton - private constructor
    explicit PrintQueueManager(QObject *parent = nullptr);
    ~PrintQueueManager();
    
    // Prevent copy
    PrintQueueManager(const PrintQueueManager&) = delete;
    PrintQueueManager& operator=(const PrintQueueManager&) = delete;
    
    static PrintQueueManager* s_instance;
    QList<PTRDG> m_danhSach;
};

#endif // PRINTQUEUEMANAGER_H
