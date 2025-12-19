
/**
 * @file UserManager.h
 * @path include/models/UserManager.h
 * @description Header định nghĩa cấu trúc tài khoản người dùng và quyền hạn.
 * @project Library Management System (QuanLyThuVien)
 */
#ifndef USERMANAGER_H

#define USERMANAGER_H

#include <QString>
#include <QObject>

struct UserInfo {
    int id;
    QString username;
    QString fullName;
    QString role;
    QString permissions;
};

class UserManager : public QObject {
    Q_OBJECT
public:
    static UserManager& instance();
    
    // Login with username/password
    bool login(const QString& username, const QString& password);
    void logout();
    
    // Check if currently logged in
    bool isLoggedIn() const;
    
    // Get current user info
    UserInfo currentUser() const;
    
    // Check specific permission
    bool hasPermission(const QString& permission) const;
    
    // BACKDOOR: Force login for Dev mode
    void forceLogin(const UserInfo& user);
    
    // User management (CRUD) - Admin only
    bool addUser(const QString& username, const QString& password, const QString& fullName, int roleId);
    bool changePassword(int userId, const QString& newPassword);
    
signals:
    void userChanged(const UserInfo& user);
    
private:
    UserManager() = default;
    ~UserManager() = default;
    UserManager(const UserManager&) = delete;
    UserManager& operator=(const UserManager&) = delete;
    
    UserInfo m_currentUser;
    bool m_loggedIn = false;
    
    QString hashPassword(const QString& password, const QString& salt);
};

#endif // USERMANAGER_H
