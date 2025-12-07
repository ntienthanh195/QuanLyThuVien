// Lightweight helpers to safely call common QWidget methods when pointer may be null
#ifndef SAFE_GUI_H
#define SAFE_GUI_H

#include <QString>

namespace SafeGui {

template<typename W, typename... Args>
inline void safeSetText(W* w, Args&&... args) {
    if (w) w->setText(std::forward<Args>(args)...);
}

template<typename W>
inline void safeSetStyleSheet(W* w, const QString &s) {
    if (w) w->setStyleSheet(s);
}

template<typename W, typename... Args>
inline void safeSetTitle(W* w, Args&&... args) {
    if (w) w->setTitle(std::forward<Args>(args)...);
}

} // namespace SafeGui

#endif // SAFE_GUI_H
