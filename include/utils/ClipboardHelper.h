#ifndef CLIPBOARD_HELPER_H
#define CLIPBOARD_HELPER_H

#include <QWidget>
#include <QTableView>

class ClipboardHelper {
public:
    // Recursively enables text selection for all QLabel children
    static void setupTextInteraction(QWidget *root);

    // Installs event filter to handle Ctrl+C for copying table content
    static void setupTableCopy(QTableView *table);

    // Global helper to setup everything for a complex widget
    static void setupGlobalCopyPaste(QWidget *root);
};

#endif // CLIPBOARD_HELPER_H
