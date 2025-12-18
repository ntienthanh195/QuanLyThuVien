#ifndef SEARCHABLELISTWIDGET_H
#define SEARCHABLELISTWIDGET_H

#include <QWidget>
#include <QVariant>
#include <QList>

class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;

/**
 * @brief Reusable widget with search box and selectable list
 * Supports both single and multi-select modes
 */
class SearchableListWidget : public QWidget {
    Q_OBJECT

public:
    enum Mode {
        SINGLE_SELECT,
        MULTI_SELECT
    };

    explicit SearchableListWidget(Mode mode = MULTI_SELECT, QWidget* parent = nullptr);

    // Data management
    void setItems(const QStringList& displayTexts, const QVariantList& userData);
    void clear();

    // Selection
    QList<int> selectedIndices() const;
    QVariantList selectedData() const;
    int selectedCount() const;
    void selectAll(bool select = true); // NEW: Select/Deselect all items

signals:
    void selectionChanged(int count);
    void itemDoubleClicked(const QVariant& data);

private slots:
    void onSearchTextChanged(const QString& text);
    void onClearClicked();
    void onItemDoubleClicked(QListWidgetItem* item);
    void onSelectionChanged();

private:
    void setupUI();

    Mode m_mode;
    QLineEdit* m_searchBox;
    QPushButton* m_btnClear;
    QListWidget* m_listWidget;
    
    // Store original data for filtering
    QStringList m_allTexts;
    QVariantList m_allData;
};

#endif // SEARCHABLELISTWIDGET_H
