#ifndef SAFE_GUI_H
#define SAFE_GUI_H

#include <QString>
#include <QLabel>
#include <QGroupBox>

/**
 * @file SafeGui.h
 * @description Bo tro cac ham giao dien an toan, tranh crash khi tro den NULL.
 */
namespace SafeGui {
    inline void safeSetText(QLabel* label, const QString& text) {
        if (label) {
            label->setText(text);
        }
    }
    
    inline void safeSetTitle(QGroupBox* group, const QString& title) {
        if (group) {
            group->setTitle(title);
        }
    }

    inline void safeSetEnabled(QWidget* widget, bool enabled) {
        if (widget) {
            widget->setEnabled(enabled);
        }
    }
}

#endif // SAFE_GUI_H
