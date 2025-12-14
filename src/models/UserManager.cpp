#include "models/UserManager.h"
#include "utils/AuditLogger.h"
#include "utils/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QVariant>
#include <QDebug>
#include <QUuid>

UserManager& UserManager::instance() {
    static UserManager inst;
    return inst;
}

bool UserManager::login(const QString& username, const QString& password) {
    if (username.isEmpty() || password.isEmpty()) return false;
    
    QSqlDatabase db = DatabaseManager::instance().database();
    if (!db.isOpen()) return false;
    
    QSqlQuery q(db);
    q.prepare("SELECT u.id, u.mat_khau_hash, u.muoi, u.ho_ten, r.ten_vai_tro, r.quyen_han "
              "FROM nguoi_dung u "
              "LEFT JOIN vai_tro r ON u.vai_tro_id = r.id "
              "WHERE u.ten_dang_nhap = :u AND u.trang_thai = 1");
    q.bindValue(":u", username);
    
    if (q.exec() && q.next()) {
        int id = q.value(0).toInt();
        QString storedHash = q.value(1).toString();
        QString salt = q.value(2).toString();
        QString fullName = q.value(3).toString();
        QString role = q.value(4).toString();
        QString permissions = q.value(5).toString();
        
        QString inputHash = hashPassword(password, salt);
        
        if (inputHash == storedHash) {
            m_currentUser.id = id;
            m_currentUser.username = username;
            m_currentUser.fullName = fullName;
            m_currentUser.role = role;
            m_currentUser.permissions = permissions;
            m_loggedIn = true;
            
            qDebug() << "[UserManager] Login success:" << username;
            
            // Log Login Action
            AuditLogger::log("Đăng nhập", "HỆ THỐNG", QString::number(id), "Người dùng " + username + " đã đăng nhập");
            
            emit userChanged(m_currentUser);
            return true;
        } else {
            qDebug() << "[UserManager] Login Failed: Wrong password for" << username;
        }
    } else {
        qDebug() << "[UserManager] Login Failed: User not found or error" << q.lastError().text();
    }
    
    return false;
}

void UserManager::logout() {
    if (m_loggedIn) {
        AuditLogger::log("Đăng xuất", "HỆ THỐNG", QString::number(m_currentUser.id), "Người dùng " + m_currentUser.username + " đã đăng xuất");
    }
    m_loggedIn = false;
    m_currentUser = UserInfo();
    emit userChanged(m_currentUser);
}

bool UserManager::isLoggedIn() const {
    return m_loggedIn;
}

UserInfo UserManager::currentUser() const {
    return m_currentUser;
}

bool UserManager::hasPermission(const QString& permission) const {
    if (!m_loggedIn) return false;
    // "*" means all permissions (Admin)
    if (m_currentUser.permissions == "*") return true;
    
    // Exact match in comma-separated list
    QStringList perms = m_currentUser.permissions.split(',', Qt::SkipEmptyParts);
    for (const QString& p : perms) {
        if (p.trimmed().compare(permission, Qt::CaseInsensitive) == 0) return true;
    }
    return false;
}


// BACKDOOR: Force login for Dev mode
void UserManager::forceLogin(const UserInfo& user) {
    m_currentUser = user;
    m_loggedIn = true;
    qDebug() << "[UserManager] BACKDOOR Login success:" << user.username;
    emit userChanged(m_currentUser);
}

bool UserManager::addUser(const QString& username, const QString& password, const QString& fullName, int roleId) {
    if (!hasPermission("ADMIN")) return false; // Basic safeguard
    
    // Generate simple salt
    QString salt = QUuid::createUuid().toString();
    QString hash = hashPassword(password, salt);
    
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("INSERT INTO nguoi_dung (ten_dang_nhap, mat_khau_hash, muoi, ho_ten, vai_tro_id, trang_thai) "
              "VALUES (:u, :p, :s, :n, :r, 1)");
    q.bindValue(":u", username);
    q.bindValue(":p", hash);
    q.bindValue(":s", salt);
    q.bindValue(":n", fullName);
    q.bindValue(":r", roleId);
    
    return q.exec();
}

bool UserManager::changePassword(int userId, const QString& newPassword) {
    // Only Admin can change passwords (for now)
    if (!hasPermission("ADMIN")) return false; 
    
    // Generate new salt for security when changing password
    QString salt = QUuid::createUuid().toString();
    QString hash = hashPassword(newPassword, salt);
    
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("UPDATE nguoi_dung SET mat_khau_hash = :p, muoi = :s WHERE id = :id");
    q.bindValue(":p", hash);
    q.bindValue(":s", salt);
    q.bindValue(":id", userId);
    
    return q.exec();
}

QString UserManager::hashPassword(const QString& password, const QString& salt) {
    QByteArray data = (password + salt).toUtf8();
    return QString(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}
