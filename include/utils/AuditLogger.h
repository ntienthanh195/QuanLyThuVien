#ifndef AUDITLOGGER_H
#define AUDITLOGGER_H

#include <QString>
#include <QObject>

/**
 * @brief Lớp AuditLogger - Ghi nhật ký hoạt động hệ thống
 */
class AuditLogger {
public:
    // Các loại hành động chuẩn
    static const QString ACTION_LOGIN;
    static const QString ACTION_LOGOUT;
    static const QString ACTION_INSERT;
    static const QString ACTION_UPDATE;
    static const QString ACTION_DELETE;
    static const QString ACTION_RESTORE;

    /**
     * @brief Ghi lại một hành động vào nhật ký
     * @param action Loại hành động (VD: "INSERT", "DELETE")
     * @param table Tên bảng hoặc mô-đun bị tác động (VD: "SACH", "DOC_GIA")
     * @param objectId ID của đối tượng (nếu có)
     * @param details Chi tiết thay đổi (VD: "Đổi tên A -> B")
     */
    static void log(const QString& action, const QString& table, const QString& objectId, const QString& details);
};

#endif // AUDITLOGGER_H
