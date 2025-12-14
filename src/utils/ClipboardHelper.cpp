#include "utils/ClipboardHelper.h"
#include <QLabel>
#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QAbstractItemModel>
#include <QItemSelectionModel>

// Event Filter to handle Ctrl+C on TableViews
class TableCopyFilter : public QObject {
public:
    TableCopyFilter(QObject *parent) : QObject(parent) {}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->matches(QKeySequence::Copy)) {
                QTableView *table = qobject_cast<QTableView *>(obj);
                if (table) {
                    copySelection(table);
                    return true;
                }
            }
        }
        return QObject::eventFilter(obj, event);
    }

private:
    void copySelection(QTableView *table) {
        QItemSelectionModel *selection = table->selectionModel();
        if (!selection || !selection->hasSelection()) return;

        QModelIndexList indexes = selection->selectedIndexes();
        if (indexes.isEmpty()) return;

        // Sort indexes by row then column
        std::sort(indexes.begin(), indexes.end(), [](const QModelIndex &a, const QModelIndex &b) {
            if (a.row() != b.row()) return a.row() < b.row();
            return a.column() < b.column();
        });

        QString text;
        int currentRow = indexes.first().row();
        for (int i = 0; i < indexes.count(); ++i) {
            const QModelIndex &idx = indexes.at(i);
            
            // Add newline if row changes
            if (idx.row() != currentRow) {
                text += '\n';
                currentRow = idx.row();
            } else if (i > 0) {
                text += '\t'; // Tab separate columns
            }
            
            text += idx.data(Qt::DisplayRole).toString();
        }

        QApplication::clipboard()->setText(text);
    }
};

// Event Filter to handle Double-Click on Labels (Select All + Copy)
class LabelClickFilter : public QObject {
public:
    LabelClickFilter(QObject *parent) : QObject(parent) {}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::MouseButtonDblClick) {
            QLabel *label = qobject_cast<QLabel *>(obj);
            if (label) {
                // Select All
                if (label->hasSelectedText()) {
                    // Already selected? Maybe standard behavior is enough, 
                    // but user wants "Auto Copy".
                }
                label->setSelection(0, label->text().length());
                
                // Auto Copy
                QApplication::clipboard()->setText(label->text());
                
                // Ensure visual feedback
                label->setFocus();
                label->repaint();
                
                // Visual Feedback (Optional - Tooltip?)
                // QToolTip::showText(QCursor::pos(), "Đã sao chép", label);
                
                return true; // Eat event? Or let default handling happen?
                             // Default handling of dblclick might accept selection. 
                             // Let's return false to let Qt do selection UI updates if needed, 
                             // but we manually forced selection so it's fine.
            }
        }
        return QObject::eventFilter(obj, event);
    }
};

void ClipboardHelper::setupTextInteraction(QWidget *root) {
    if (!root) return;
    
    // Find all labels including the root itself if it is one
    QList<QLabel *> labels = root->findChildren<QLabel *>();
    if (QLabel *l = qobject_cast<QLabel*>(root)) labels.append(l);

    for (QLabel *label : labels) {
        label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        label->setCursor(Qt::IBeamCursor);
        
        // Install Double Click Filter
        LabelClickFilter *filter = new LabelClickFilter(label);
        label->installEventFilter(filter);
    }
}

void ClipboardHelper::setupTableCopy(QTableView *table) {
    if (!table) return;
    
    // Check if filter already installed to prevent duplicates? 
    // Usually safe to add, but better to check children of table for filter? 
    // For simplicity, just add.
    
    TableCopyFilter *filter = new TableCopyFilter(table);
    table->installEventFilter(filter);
}

void ClipboardHelper::setupGlobalCopyPaste(QWidget *root) {
    if (!root) return;
    
    setupTextInteraction(root);
    
    QList<QTableView *> tables = root->findChildren<QTableView *>();
    for (QTableView *table : tables) {
        setupTableCopy(table);
    }
}
