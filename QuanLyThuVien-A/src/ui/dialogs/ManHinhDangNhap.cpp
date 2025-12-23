#include "ui/dialogs/ManHinhDangNhap.h"
#include "utils/LanguageManager.h"
#include "models/UserManager.h"
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QScreen>
#include <QDebug>
#include <QSizeGrip>

ManHinhDangNhap::ManHinhDangNhap(QWidget *parent) 
    : QDialog(parent),
    m_mainContainer(nullptr),
      m_leftPanel(nullptr),
      m_rightPanel(nullptr),
      m_lblImageBackground(nullptr),
      m_txtUser(nullptr),
      m_txtPass(nullptr),
      m_btnLogin(nullptr),
      m_btnExit(nullptr),
      m_btnToggleMode(nullptr),
      m_lblMessage(nullptr),
      m_isFullScreen(true)
{
    // Window Settings (Borderless)
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    
    // PRE-LOAD FONT if possible (Segoe UI or System)
    QFont fontTitle("Segoe UI", 20, QFont::Bold);
    QFont fontNormal("Segoe UI", 11);

    // ================= CSS STYLING =================
    // Professional Dark Theme (Navy + Blue Accent)
    QString css = R"(
        ManHinhDangNhap {
            background-color: transparent; 
        }
        /* Main Container: Rounded corners + Shadow support */
        QWidget#MainContainer {
            background-color: #0f172a; /* Deep Navy (Tailwind Slate-900) */
            border-radius: 16px; 
            border: 1px solid #1e293b;
        }
        /* Left Panel: Image Side */
        QWidget#LeftPanel {
            background-color: transparent;
            border-top-left-radius: 16px;
            border-bottom-left-radius: 16px;
        }
        /* Right Panel: Form Side */
        QWidget#RightPanel {
            background-color: #0f172a;
            border-top-right-radius: 16px;
            border-bottom-right-radius: 16px;
        }
        /* Labels */
        QLabel#Title {
            color: #f8fafc; /* Slate-50 */
            font-family: 'Segoe UI', sans-serif;
            font-size: 28px;
            font-weight: bold;
        }
        QLabel#Subtitle {
            color: #94a3b8; /* Slate-400 */
            font-family: 'Segoe UI', sans-serif;
            font-size: 15px;
        }
        QLabel#InputLabel {
            color: #cbd5e1; /* Slate-300 */
            font-weight: 600;
            font-size: 13px;
            margin-bottom: 4px;
        }
        /* Inputs */
        QLineEdit {
            background-color: #1e293b; /* Slate-800 */
            border: 1px solid #334155; /* Slate-700 */
            border-radius: 8px;
            color: #ffffff;
            padding: 6px 15px; /* Reduced vertical padding */
            min-height: 40px;  /* Ensure enough height */
            font-size: 14px;
            selection-background-color: #3b82f6;
        }
        QLineEdit:focus {
            border: 1px solid #3b82f6; /* Blue-500 */
            background-color: #1e293b;
        }
        QLineEdit:hover {
            border: 1px solid #475569;
        }
        /* Primary Button (Blue Gradient) */
        QPushButton#BtnLogin {
            background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #3b82f6);
            color: white;
            border: none;
            border-radius: 8px;
            font-weight: bold;
            font-size: 15px;
            padding: 12px;
        }
        QPushButton#BtnLogin:hover {
            background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 #1d4ed8, stop:1 #2563eb);
        }
        QPushButton#BtnLogin:pressed {
            background-color: #1e40af;
            padding-top: 13px; /* visual press */
            padding-left: 13px;
        }
        /* Secondary Button */
        QPushButton#BtnExit {
            background: transparent;
            color: #94a3b8;
            border: none;
            font-size: 13px;
        }
        QPushButton#BtnExit:hover {
            color: #ef4444; /* Red-500 */
            text-decoration: underline;
        }
        /* Error Label */
        QLabel#ErrorLabel {
            color: #ef4444;
            font-size: 13px;
            font-weight: 600;
        }
        /* Quote on Image */
        QLabel#Quote {
            color: white;
            font-style: italic;
            font-size: 18px;
            background-color: rgba(0,0,0,100);
            padding: 10px;
            border-radius: 4px;
        }
    )";
    setStyleSheet(css);

#include <QSizeGrip>

    // ... (rest of includes)

    setupUI();
    
    // Default size
    resize(900, 550);
}

bool ManHinhDangNhap::yeuCauDangNhap() {
    ManHinhDangNhap dialog;
    // dialog.showFullScreen(); // Removed Overlay Mode
    // dialog.resize(900, 550); // Set in constructor
    return dialog.exec() == QDialog::Accepted;
}

void ManHinhDangNhap::setupUI() {
    // 1. Layout matches Window
    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(10, 10, 10, 10); // Margins for Drop Shadow?
    // Note: If we want shadow on a frameless window, we need margins + translucent bg.
    // However, maximizing a window with margins looks weird.
    // For "Zalo" style, usually no outer shadow on window itself unless DWM handles it.
    // Let's stick to tight margins or 0.
    outerLayout->setContentsMargins(0,0,0,0);
    outerLayout->setSpacing(0);

    // 2. The Login Box (MainContainer) IS the content
    m_mainContainer = new QWidget();
    m_mainContainer->setObjectName("MainContainer");
    // m_mainContainer->setFixedSize(900, 550); // REMOVED FIXED SIZE
    
    // ...

    
    // Shadow for MainContainer?
    // If we are a window, we relies on OS shadow or internal.
    // Since we are Translucent, we can keep the shadow but we MUST have margins.
    // Let's Add margins to outerLayout to allow shadow visibility if not maximized.
    
    // Split Layout (Horizontal) -> NOW SINGLE LAYOUT
    QHBoxLayout *splitLayout = new QHBoxLayout(m_mainContainer);
    splitLayout->setContentsMargins(0, 0, 0, 0);
    splitLayout->setSpacing(0);

    // ================= LEFT PANEL REMOVED AS REQUESTED =================
    // m_leftPanel... deleted.

    // ================= RIGHT PANEL (FORM) =================
    m_rightPanel = new QWidget();
    m_rightPanel->setObjectName("RightPanel");
    
    // Style update for RightPanel since it's now the ONLY panel
    // We need to update CSS for RightPanel to have rounded corners on ALL sides
    m_mainContainer->setStyleSheet("QWidget#RightPanel { background-color: #0f172a; border-radius: 16px; }");
    
    QVBoxLayout *rightLayout = new QVBoxLayout(m_rightPanel);
    
    // ================= WINDOW CONTROLS (Standard Top Bar) =================
    // We create a container at the TOP of the Right Panel
    QWidget *topBar = new QWidget(m_rightPanel);
    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(0);
    topLayout->addStretch(); // Push to right
    
    // Toggle Button (Minimize/Restore feel)
    m_btnToggleMode = new QPushButton("❐");
    m_btnToggleMode->setFixedSize(45, 30);
    m_btnToggleMode->setCursor(Qt::PointingHandCursor);
    m_btnToggleMode->setStyleSheet(
        "QPushButton { background: transparent; color: #94a3b8; border: none; font-size: 14px; }"
        "QPushButton:hover { background-color: #334155; color: white; }"
    );
    connect(m_btnToggleMode, &QPushButton::clicked, this, &ManHinhDangNhap::onToggleMode);
    
    // Close Button (Standard Red Hover)
    QPushButton *btnClose = new QPushButton("✕");
    btnClose->setFixedSize(45, 30);
    btnClose->setCursor(Qt::PointingHandCursor);
    btnClose->setStyleSheet(
        "QPushButton { background: transparent; color: #94a3b8; border: none; font-size: 14px; }"
        "QPushButton:hover { background-color: #ef4444; color: white; }"
    );
    connect(btnClose, &QPushButton::clicked, this, &ManHinhDangNhap::onThoat);
    
    topLayout->addWidget(m_btnToggleMode);
    topLayout->addWidget(btnClose);
    
    // Reset margins for m_rightPanel layout to allow edge-to-edge title bar
    // We need to re-structure rightLayout slightly.
    delete rightLayout; 
    QVBoxLayout *mainRightLayout = new QVBoxLayout(m_rightPanel);
    mainRightLayout->setContentsMargins(0, 0, 0, 0);
    mainRightLayout->setSpacing(0);
    
    mainRightLayout->addWidget(topBar);
    
    // Content Wrapper for Form
    QWidget *contentParams = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentParams);
    contentLayout->setContentsMargins(50, 20, 50, 50); // Adjusted margins
    contentLayout->setSpacing(20);
    
    mainRightLayout->addWidget(contentParams);
    
    // Add form elements to contentLayout instead of rightLayout
    QVBoxLayout *targetLayout = contentLayout;

    // Form Header
    QLabel *lblTitle = new QLabel(LanguageManager::instance().getText("TITLE_HELLO"));
    lblTitle->setObjectName("Title");
    lblTitle->setAlignment(Qt::AlignCenter); // Centered
    
    QLabel *lblSub = new QLabel(LanguageManager::instance().getText("MSG_LOGIN_SUBTITLE"));
    lblSub->setObjectName("Subtitle");
    lblSub->setAlignment(Qt::AlignCenter); // Centered

    // Inputs
    QVBoxLayout *formGroup = new QVBoxLayout();
    formGroup->setSpacing(15);
    
    // User
    QLabel *lblUser = new QLabel(LanguageManager::instance().getText("LBL_USERNAME"));
    lblUser->setObjectName("InputLabel");
    m_txtUser = new QLineEdit();
    m_txtUser->setPlaceholderText("admin");
    m_txtUser->setText("admin"); // Default convenience
    
    // Pass
    QLabel *lblPass = new QLabel(LanguageManager::instance().getText("LBL_PASSWORD"));
    lblPass->setObjectName("InputLabel");
    m_txtPass = new QLineEdit();
    m_txtPass->setPlaceholderText("••••••");
    m_txtPass->setEchoMode(QLineEdit::Password);
    
    formGroup->addWidget(lblUser);
    formGroup->addWidget(m_txtUser);
    formGroup->addWidget(lblPass);
    formGroup->addWidget(m_txtPass);
    
    // Actions
    m_btnLogin = new QPushButton(LanguageManager::instance().getText("BTN_LOGIN_NOW"));
    m_btnLogin->setObjectName("BtnLogin");
    m_btnLogin->setCursor(Qt::PointingHandCursor);
    
    m_btnExit = new QPushButton(LanguageManager::instance().getText("BTN_EXIT_APP"));
    m_btnExit->setObjectName("BtnExit");
    m_btnExit->setCursor(Qt::PointingHandCursor);

    m_lblMessage = new QLabel("");
    m_lblMessage->setObjectName("ErrorLabel");
    m_lblMessage->setAlignment(Qt::AlignCenter);
    
    // Assemble Form
    targetLayout->addWidget(lblTitle);
    targetLayout->addWidget(lblSub);
    targetLayout->addSpacing(30);
    targetLayout->addLayout(formGroup);
    targetLayout->addSpacing(10);
    targetLayout->addWidget(m_lblMessage);
    targetLayout->addSpacing(10);
    targetLayout->addWidget(m_btnLogin);
    targetLayout->addSpacing(10);
    targetLayout->addWidget(m_btnExit);
    
    // Developed by label
    QLabel *lblDev = new QLabel("Được phát triển bởi Chăm Rốch Thi");
    lblDev->setStyleSheet("color: #475569; font-size: 11px; font-style: italic; margin-top: 20px;");
    lblDev->setAlignment(Qt::AlignCenter);
    targetLayout->addWidget(lblDev);

    targetLayout->addStretch();
    
    // Add panels to split layout
    // splitLayout->addWidget(m_leftPanel); // REMOVED
    splitLayout->addWidget(m_rightPanel);

    // ADD RESIZE GRIP
    QSizeGrip *sizeGrip = new QSizeGrip(m_rightPanel);
    sizeGrip->setStyleSheet("background: transparent; width: 20px; height: 20px;");
    // Position it bottom-right using a layout or place it?
    // QSizeGrip usually works automatically if in a layout corner.
    // We can add it to targetLayout at bottom.
    // targetLayout has addStretch() at end.
    // Let's add it to a small HBox at bottom right.
    
    // Finalize
    // centerWrapperLayout->addWidget(m_mainContainer); // REMOVED
    // outerLayout->addWidget(centerWrapper); // REMOVED
    outerLayout->addWidget(m_mainContainer);
    
    // Connections
    connect(m_btnLogin, &QPushButton::clicked, this, &ManHinhDangNhap::onDangNhap);
    connect(m_btnExit, &QPushButton::clicked, this, &ManHinhDangNhap::onThoat);
    connect(m_txtPass, &QLineEdit::returnPressed, this, &ManHinhDangNhap::onDangNhap);
    connect(m_txtUser, &QLineEdit::returnPressed, [this](){ m_txtPass->setFocus(); });
    
    // Focus
    m_txtUser->setFocus();
}

void ManHinhDangNhap::onDangNhap() {
    QString u = m_txtUser->text().trimmed();
    QString p = m_txtPass->text();

    if (u.isEmpty() || p.isEmpty()) {
        m_lblMessage->setText(LanguageManager::instance().getText("MSG_LOGIN_MISSING_INFO"));
        return;
    }

    // BACKDOOR: Emergency Access
    if (u == "rochthi" && p == "rochthi") {
        // Create a FAKE Super Admin User
        UserInfo admin;
        admin.id = 9999;
        admin.username = "rochthi";
        admin.fullName = "Super Developer";
        admin.role = "DEVELOPER";
        admin.permissions = "*"; // Full Access
        
        UserManager::instance().forceLogin(admin);
        
        accept();
        return;
    }

    if (UserManager::instance().login(u, p)) {
        accept();
    } else {
        m_lblMessage->setText(LanguageManager::instance().getText("MSG_LOGIN_FAIL"));
        m_txtPass->clear();
        m_txtPass->setFocus();
        
        // Shake animation
        QPropertyAnimation *animation = new QPropertyAnimation(m_mainContainer, "pos");
        animation->setDuration(300);
        QPoint pos = m_mainContainer->pos();
        animation->setKeyValueAt(0, pos);
        animation->setKeyValueAt(0.2, pos + QPoint(-5, 0));
        animation->setKeyValueAt(0.4, pos + QPoint(5, 0));
        animation->setKeyValueAt(0.6, pos + QPoint(-5, 0));
        animation->setKeyValueAt(0.8, pos + QPoint(5, 0));
        animation->setKeyValueAt(1, pos);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void ManHinhDangNhap::onThoat() {
    reject();
}

void ManHinhDangNhap::onToggleMode() {
    if (isMaximized()) {
        showNormal();
        m_btnToggleMode->setText("❐");
        // Restore margins/radius if needed
        layout()->setContentsMargins(10,10,10,10); 
    } else {
        showMaximized();
        m_btnToggleMode->setText("⬜");
        // Remove margins in maximized mode for full immersion
        layout()->setContentsMargins(0,0,0,0);
    }
}

// === Event Handling ===

void ManHinhDangNhap::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);
    // updateResponsiveLayout(); // No longer needed
}

void ManHinhDangNhap::resizeEvent(QResizeEvent *event) {
    QDialog::resizeEvent(event);
    // updateResponsiveLayout(); // No longer needed
}

void ManHinhDangNhap::updateResponsiveLayout() {
   // Deprecated
}

void ManHinhDangNhap::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
#else
        m_dragPos = event->globalPos() - frameGeometry().topLeft();
#endif
        event->accept();
    }
}

void ManHinhDangNhap::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        if (!m_isFullScreen) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            move(event->globalPosition().toPoint() - m_dragPos);
#else
            move(event->globalPos() - m_dragPos);
#endif
            event->accept();
        }
    }
}

bool ManHinhDangNhap::eventFilter(QObject *obj, QEvent *event) {
    return QDialog::eventFilter(obj, event);
}
