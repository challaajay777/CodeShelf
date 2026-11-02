#include "ImportPreviewDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

namespace CodeShelf {

ImportPreviewDialog::ImportPreviewDialog(ImportPreviewSummary& preview, JsonBackupHandler& handler, QWidget* parent)
    : QDialog(parent)
    , m_preview(preview)
    , m_handler(handler)
{
    setWindowTitle(QStringLiteral("Import Preview & Conflict Resolution"));
    resize(780, 520);
    setupUi();
}

void ImportPreviewDialog::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);

    // Summary banner
    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setText(QStringLiteral(
        "Total items: %1 | New: %2 | Duplicate ID: %3 | Duplicate Content: %4 | Invalid: %5"
    ).arg(m_preview.totalInFile)
     .arg(m_preview.newCount)
     .arg(m_preview.duplicateIdCount)
     .arg(m_preview.duplicateContentCount)
     .arg(m_preview.invalidCount));
    m_summaryLabel->setStyleSheet(QStringLiteral("font-weight: bold; padding: 4px; background: #eef2f5; border: 1px solid #ccd;"));
    mainLayout->addWidget(m_summaryLabel);

    // Conflict strategy
    auto conflictRow = new QHBoxLayout();
    conflictRow->addWidget(new QLabel(QStringLiteral("Default Conflict Resolution:"), this));
    m_globalResolutionCombo = new QComboBox(this);
    m_globalResolutionCombo->addItem(QStringLiteral("Skip Conflicting Snippets"), static_cast<int>(ConflictResolution::Skip));
    m_globalResolutionCombo->addItem(QStringLiteral("Keep Both (Assign New ID)"), static_cast<int>(ConflictResolution::KeepBothNewId));
    m_globalResolutionCombo->addItem(QStringLiteral("Update Existing Snippet"), static_cast<int>(ConflictResolution::UpdateExisting));
    conflictRow->addWidget(m_globalResolutionCombo);
    conflictRow->addStretch();
    mainLayout->addLayout(conflictRow);

    // Items table
    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({QStringLiteral("Title"), QStringLiteral("Language"), QStringLiteral("Status"), QStringLiteral("Details")});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);

    m_table->setRowCount(m_preview.snippetItems.size());
    for (int i = 0; i < m_preview.snippetItems.size(); ++i) {
        const auto& item = m_preview.snippetItems[i];

        auto titleItem = new QTableWidgetItem(item.snippet.title());
        auto langItem = new QTableWidgetItem(item.snippet.language());
        QString statusStr;
        switch (item.status) {
            case ImportItemStatus::NewItem: statusStr = QStringLiteral("New"); break;
            case ImportItemStatus::DuplicateIdMatch: statusStr = QStringLiteral("ID Conflict"); break;
            case ImportItemStatus::DuplicateContentMatch: statusStr = QStringLiteral("Content Match"); break;
            case ImportItemStatus::Invalid: statusStr = QStringLiteral("Invalid"); break;
        }
        auto statusItem = new QTableWidgetItem(statusStr);
        auto detailItem = new QTableWidgetItem(item.reason);

        m_table->setItem(i, 0, titleItem);
        m_table->setItem(i, 1, langItem);
        m_table->setItem(i, 2, statusItem);
        m_table->setItem(i, 3, detailItem);
    }

    mainLayout->addWidget(m_table, 1);

    // Bottom action row
    auto bottomRow = new QHBoxLayout();
    bottomRow->addStretch();
    m_applyBtn = new QPushButton(QStringLiteral("Apply Import"), this);
    m_applyBtn->setDefault(true);
    auto cancelBtn = new QPushButton(QStringLiteral("Cancel"), this);

    bottomRow->addWidget(m_applyBtn);
    bottomRow->addWidget(cancelBtn);
    mainLayout->addLayout(bottomRow);

    connect(m_globalResolutionCombo, &QComboBox::currentIndexChanged, this, &ImportPreviewDialog::onGlobalResolutionChanged);
    connect(m_applyBtn, &QPushButton::clicked, this, &ImportPreviewDialog::applyImport);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void ImportPreviewDialog::onGlobalResolutionChanged(int /*index*/) {
    // Selection changed
}

void ImportPreviewDialog::applyImport() {
    auto res = static_cast<ConflictResolution>(m_globalResolutionCombo->currentData().toInt());
    QString err;
    if (!m_handler.applyImport(m_preview, res, err)) {
        QMessageBox::critical(this, QStringLiteral("Import Failed"), err);
        return;
    }

    QMessageBox::information(this, QStringLiteral("Import Successful"), QStringLiteral("Snippets and collections have been imported successfully."));
    emit importCompleted();
    accept();
}

} // namespace CodeShelf
