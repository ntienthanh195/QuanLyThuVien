#include "utils/SearchableListWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

SearchableListWidget::SearchableListWidget(Mode mode, QWidget* parent)
    : QWidget(parent)
    , m_mode(mode)
{
    setupUI();
}

void SearchableListWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);
    
    // Search box with clear button
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(5);
    
    m_searchBox = new QLineEdit();
    m_searchBox->setPlaceholderText("Nhập để tìm kiếm...");
    m_searchBox->setStyleSheet(
        "QLineEdit { "
        "  background: #252530; color: #E0E0E0; "
        "  border: 1px solid #3E3E42; border-radius: 4px; "
        "  padding: 8px; font-size: 13px; "
        "}"
        "QLineEdit:focus { border-color: #5D9CEC; }"
    );
    
    m_btnClear = new QPushButton("✕");
    m_btnClear->setMaximumWidth(32);
    m_btnClear->setMaximumHeight(32);
    m_btnClear->setCursor(Qt::PointingHandCursor);
    m_btnClear->setStyleSheet(
        "QPushButton { "
        "  background: #3E3E42; color: #999; "
        "  border: none; border-radius: 4px; "
        "  font-size: 16px; "
        "}"
        "QPushButton:hover { background: #555; color: #E0E0E0; }"
    );
    
    searchLayout->addWidget(m_searchBox);
    searchLayout->addWidget(m_btnClear);
    mainLayout->addLayout(searchLayout);
    
    // List widget
    m_listWidget = new QListWidget();
    m_listWidget->setMinimumHeight(200);
    m_listWidget->setMaximumHeight(300);
    m_listWidget->setStyleSheet(
        "QListWidget { "
        "  background: #252530; color: #E0E0E0; "
        "  border: 1px solid #3E3E42; border-radius: 4px; "
        "  padding: 5px; "
        "}"
        "QListWidget::item { "
        "  padding: 8px; border-radius: 3px; "
        "}"
        "QListWidget::item:hover { "
        "  background: #2b2b40; "
        "}"
        "QListWidget::item:selected { "
        "  background: #5D9CEC; color: white; "
        "}"
    );
    
    if (m_mode == MULTI_SELECT) {
        m_listWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    } else {
        m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    }
    
    mainLayout->addWidget(m_listWidget);
    
    // Connections
    connect(m_searchBox, &QLineEdit::textChanged, this, &SearchableListWidget::onSearchTextChanged);
    connect(m_btnClear, &QPushButton::clicked, this, &SearchableListWidget::onClearClicked);
    connect(m_listWidget, &QListWidget::itemDoubleClicked, this, &SearchableListWidget::onItemDoubleClicked);
    connect(m_listWidget, &QListWidget::itemSelectionChanged, this, &SearchableListWidget::onSelectionChanged);
}

void SearchableListWidget::setItems(const QStringList& displayTexts, const QVariantList& userData) {
    m_allTexts = displayTexts;
    m_allData = userData;
    
    m_listWidget->clear();
    
    for (int i = 0; i < displayTexts.size(); ++i) {
        QListWidgetItem* item = new QListWidgetItem(displayTexts[i]);
        if (i < userData.size()) {
            item->setData(Qt::UserRole, userData[i]);
        }
        m_listWidget->addItem(item);
    }
}

void SearchableListWidget::clear() {
    m_listWidget->clear();
    m_searchBox->clear();
    m_allTexts.clear();
    m_allData.clear();
}

QList<int> SearchableListWidget::selectedIndices() const {
    QList<int> indices;
    for (QListWidgetItem* item : m_listWidget->selectedItems()) {
        indices.append(m_listWidget->row(item));
    }
    return indices;
}

QVariantList SearchableListWidget::selectedData() const {
    QVariantList data;
    for (QListWidgetItem* item : m_listWidget->selectedItems()) {
        data.append(item->data(Qt::UserRole));
    }
    return data;
}

int SearchableListWidget::selectedCount() const {
    return m_listWidget->selectedItems().count();
}

void SearchableListWidget::onSearchTextChanged(const QString& text) {
    QString query = text.trimmed();
    
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem* item = m_listWidget->item(i);
        bool matches = query.isEmpty() || 
                      item->text().contains(query, Qt::CaseInsensitive);
        item->setHidden(!matches);
    }
}

void SearchableListWidget::onClearClicked() {
    m_searchBox->clear();
    m_searchBox->setFocus();
}

void SearchableListWidget::onItemDoubleClicked(QListWidgetItem* item) {
    if (item) {
        emit itemDoubleClicked(item->data(Qt::UserRole));
    }
}

void SearchableListWidget::onSelectionChanged() {
    emit selectionChanged(selectedCount());
}

void SearchableListWidget::selectAll(bool select) {
    if (select) {
        m_listWidget->selectAll();
    } else {
        m_listWidget->clearSelection();
    }
}
