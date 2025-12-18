#ifndef CONFIRMATIONDIALOG_H
#define CONFIRMATIONDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>

/**
 * @brief Custom confirmation dialog with preview
 */
class ConfirmationDialog : public QDialog {
    Q_OBJECT

public:
    enum class Type {
        Delete,
        Warning,
        Info,
        Question
    };

    explicit ConfirmationDialog(QWidget *parent = nullptr);
    
    // Setup dialog
    void setTitle(const QString &title);
    void setMessage(const QString &message);
    void setDetailedInfo(const QString &details);
    void setType(Type type);
    void setConfirmButtonText(const QString &text);
    void setCancelButtonText(const QString &text);
    
    // Show with settings
    static bool confirm(QWidget *parent, const QString &title, 
                       const QString &message, const QString &details = QString(),
                       Type type = Type::Question);
    
    static bool confirmDelete(QWidget *parent, const QString &itemName,
                             const QString &details = QString());

private:
    QLabel *iconLabel;
    QLabel *titleLabel;
    QLabel *messageLabel;
    QTextEdit *detailsEdit;
    QPushButton *confirmButton;
    QPushButton *cancelButton;
    
    void setupUI();
    void applyTheme();
    QString getIconForType(Type type) const;
    QString getColorForType(Type type) const;
};

#endif // CONFIRMATIONDIALOG_H
