#include "utils/FocusGuard.h"
#include <QApplication>

FocusGuard::FocusGuard(QObject *parent) 
    : QObject(parent)
    , m_target(nullptr)
    , m_enabled(true)
{
    m_refocusTimer = new QTimer(this);
    m_refocusTimer->setSingleShot(true);
    m_refocusTimer->setInterval(100); // Wait 100ms before refocusing to avoid fighting with system
    
    connect(m_refocusTimer, &QTimer::timeout, this, &FocusGuard::performRefocus);
}

void FocusGuard::setTarget(QLineEdit* inputWidget) {
    if (m_target == inputWidget) return;
    
    // Remove filter from old target if any
    if (m_target) {
        m_target->removeEventFilter(this);
    }
    
    m_target = inputWidget;
    
    if (m_target) {
        m_target->installEventFilter(this);
        // FORCE FOCUS IMMEDIATELY
        if (m_enabled) {
            m_target->setFocus();
        }
    }
}

void FocusGuard::setEnabled(bool enabled) {
    m_enabled = enabled;
    if (enabled && m_target) {
        m_target->setFocus();
    }
}

bool FocusGuard::eventFilter(QObject *obj, QEvent *event) {
    if (!m_enabled || !m_target || obj != m_target) {
        return QObject::eventFilter(obj, event);
    }
    
    if (event->type() == QEvent::FocusOut) {
        // Detected focus loss!
        // Don't fight immediately inside the event handler (can cause loops or crashes)
        // Schedule a refocus
        if (!m_refocusTimer->isActive()) {
            // qDebug() << "FocusGuard: Lost focus, scheduling restore...";
            m_refocusTimer->start();
        }
    }
    
    return QObject::eventFilter(obj, event);
}

void FocusGuard::performRefocus() {
    if (m_enabled && m_target && !m_target->hasFocus()) {
        // Check if application is active (don't steal focus from other apps)
        if (QApplication::activeWindow()) {
            // qDebug() << "FocusGuard: Restoring focus to target.";
            m_target->setFocus();
            m_target->selectAll(); // Optional: select all text ready for next scan
        }
    }
}
