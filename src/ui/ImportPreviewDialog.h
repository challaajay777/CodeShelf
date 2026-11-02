#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include "../import_export/ImportPreviewModel.h"
#include "../import_export/JsonBackupHandler.h"

namespace CodeShelf {

class ImportPreviewDialog : public QDialog {
    Q_OBJECT

public:
    ImportPreviewDialog(ImportPreviewSummary& preview, JsonBackupHandler& handler, QWidget* parent = nullptr);

signals:
    void importCompleted();

private slots:
    void applyImport();
    void onGlobalResolutionChanged(int index);

private:
    void setupUi();

    ImportPreviewSummary& m_preview;
    JsonBackupHandler& m_handler;

    QLabel* m_summaryLabel{nullptr};
    QComboBox* m_globalResolutionCombo{nullptr};
    QTableWidget* m_table{nullptr};
    QPushButton* m_applyBtn{nullptr};
};

} // namespace CodeShelf
