#include "utils/AuditLogger.h"
#include "utils/DatabaseManager.h"
#include "models/UserManager.h"
#include "utils/EventBus.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

const QString AuditLogger::ACTION_LOGIN = "LOGIN";
const QString AuditLogger::ACTION_LOGOUT = "LOGOUT";
const QString AuditLogger::ACTION_INSERT = "INSERT";
const QString AuditLogger::ACTION_UPDATE = "UPDATE";
const QString AuditLogger::ACTION_DELETE = "DELETE";
const QString AuditLogger::ACTION_RESTORE = "RESTORE";

void AuditLogger::log(const QString& action, const QString& table, const QString& objectId, const QString& details) {
    // 1. Lấy thông tin người dùng hiện tại
    UserInfo user = UserManager::instance().currentUser();
    int userId = user.id;

    // Nếu không có user (VD: system action hoặc chưa login), có thể để 0 hoặc null
    if (userId <= 0) {
        // Tùy chọn: Ghi nhận là SYSTEM (id=0 hoặc null)
    }

    // 2. Lấy thời gian hiện tại
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    // 3. Chuẩn bị SQL
    QSqlDatabase db = DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    QSqlQuery q(db);
    q.prepare("INSERT INTO nhat_ky_he_thong (nguoi_dung_id, hanh_dong, bang_du_lieu, doi_tuong_id, chi_tiet, thoi_gian) "
              "VALUES (:u, :a, :b, :o, :d, :t)");
    
    q.bindValue(":u", userId > 0 ? userId : QVariant()); // Insert NULL if explicit 0 or use 0 if column is NOT NULL. Let's use QVariant() which is NULL.
    q.bindValue(":a", action);
    q.bindValue(":b", table);
    q.bindValue(":o", objectId);
    q.bindValue(":d", details);
    q.bindValue(":t", timestamp);

    // 4. Thực thi (Silent failure - chỉ log ra debug nếu lỗi để tránh crash app chính)
    if (!q.exec()) {
        qWarning() << "[AuditLogger] Failed to log:" << q.lastError().text();
    } else {
        qDebug() << "[Audit] Logged:" << action << table << objectId;
        // Notify UI via EventBus
        EventBus::instance().newSystemLog(action, details);
    }
}
