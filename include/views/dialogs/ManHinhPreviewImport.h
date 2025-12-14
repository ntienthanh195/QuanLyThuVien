#ifndef MANHINHPREVIEWIMPORT_H
#define MANHINHPREVIEWIMPORT_H

#include <QDialog>
#include <QTabWidget>
#include <QTableView>
#include <QStandardItemModel>
#include "utils/ImportManager.h"

class ManHinhPreviewImport : public QDialog {
    Q_OBJECT
public:
    explicit ManHinhPreviewImport(const ImportManager::PreviewResult &result, const QString &excelPath, ImportManager *importer, QWidget *parent = nullptr);
    ~ManHinhPreviewImport();

private slots:
    void onBtnApplyClicked();
    void onBtnCancelClicked();

private:
    void setupUI();
    void loadCsv(const QString &csvPath, QTableView *tableView);

    ImportManager::PreviewResult m_result;
    QString m_excelPath;
    ImportManager *m_importer;
    QTabWidget *m_tabWidget;
};

#endif // MANHINHPREVIEWIMPORT_H
