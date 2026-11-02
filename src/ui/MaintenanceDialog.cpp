#include "MaintenanceDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

namespace CodeShelf {

MaintenanceDialog::MaintenanceDialog(MaintenanceService& maintService, QWidget* parent)
    : QDialog(parent)
    , m_maintService(maintService)
{
    setWindowTitle(QStringLiteral("Database Maintenance & Health"));
    resize(640, 440);
    setupUi();
    refreshStats();
}

void MaintenanceDialog::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);

    m_statsLabel = new QLabel(this);
    m_statsLabel->setStyleSheet(QStringLiteral("background: #f4f6f8; border: 1px solid #ddd; padding: 8px; line-height: 1.4;"));
    mainLayout->addWidget(m_statsLabel);

    // Maintenance buttons
    auto btnRow = new QHBoxLayout();
    m_integrityBtn = new QPushButton(QStringLiteral("Check Integrity"), this);
    m_rebuildFtsBtn = new QPushButton(QStringLiteral("Rebuild FTS Index"), this);
    m_vacuumBtn = new QPushButton(QStringLiteral("Compact (Vacuum)"), this);

    btnRow->addWidget(m_integrityBtn);
    btnRow->addWidget(m_rebuildFtsBtn);
    btnRow->addWidget(m_vacuumBtn);
    mainLayout->addLayout(btnRow);

    // Execution output log
    mainLayout->addWidget(new QLabel(QStringLiteral("Maintenance Log Output:")));
    m_logOutput = new QTextEdit(this);
    m_logOutput->setReadOnly(true);
    m_logOutput->setFont(QFont(QStringLiteral("Consolas"), 9));
    mainLayout->addWidget(m_logOutput, 1);

    auto closeBtn = new QPushButton(QStringLiteral("Close"), this);
    mainLayout->addWidget(closeBtn, 0, Qt::AlignRight);

    connect(m_integrityBtn, &QPushButton::clicked, this, &MaintenanceDialog::runIntegrityCheck);
    connect(m_rebuildFtsBtn, &QPushButton::clicked, this, &MaintenanceDialog::rebuildFts);
    connect(m_vacuumBtn, &QPushButton::clicked, this, &MaintenanceDialog::runVacuum);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void MaintenanceDialog::refreshStats() {
    auto stats = m_maintService.getDatabaseStats();
    double mb = static_cast<double>(stats.fileSizeBytes) / (1024.0 * 1024.0);

    m_statsLabel->setText(QStringLiteral(
        "<b>Database Health Statistics:</b><br>"
        "File Size: %1 MB (%2 bytes)<br>"
        "Active Snippets: %3 | Trash: %4<br>"
        "Collections: %5 | Tags: %6<br>"
        "Schema Version: %7 | FTS5 Indexing: %8"
    ).arg(QString::number(mb, 'f', 2))
     .arg(stats.fileSizeBytes)
     .arg(stats.activeSnippets)
     .arg(stats.trashSnippets)
     .arg(stats.totalCollections)
     .arg(stats.totalTags)
     .arg(stats.schemaVersion)
     .arg(stats.fts5Enabled ? QStringLiteral("Enabled") : QStringLiteral("Fallback Search Mode")));
}

void MaintenanceDialog::runIntegrityCheck() {
    m_logOutput->append(QStringLiteral("Running SQLite integrity check (PRAGMA integrity_check)..."));
    QStringList issues;
    bool ok = m_maintService.checkIntegrity(issues);
    if (ok) {
        m_logOutput->append(QStringLiteral("-> Integrity Check Result: PASS (ok)\n"));
    } else {
        m_logOutput->append(QStringLiteral("-> Integrity Check FAILED with issues:\n") + issues.join(QLatin1Char('\n')) + QStringLiteral("\n"));
    }
}

void MaintenanceDialog::rebuildFts() {
    m_logOutput->append(QStringLiteral("Rebuilding FTS5 full-text search index..."));
    QString err;
    if (m_maintService.rebuildFts(err)) {
        m_logOutput->append(QStringLiteral("-> Full-text search index rebuilt successfully.\n"));
    } else {
        m_logOutput->append(QStringLiteral("-> FTS rebuild error: ") + err + QStringLiteral("\n"));
    }
}

void MaintenanceDialog::runVacuum() {
    if (QMessageBox::question(this, QStringLiteral("Compact Database"),
                              QStringLiteral("Compacting the database (VACUUM) repacks data pages to reclaim unused space. Proceed?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    m_logOutput->append(QStringLiteral("Executing safe WAL checkpoint and VACUUM compaction..."));
    QString err;
    if (m_maintService.vacuum(err)) {
        m_logOutput->append(QStringLiteral("-> Compaction completed successfully.\n"));
        refreshStats();
        emit databaseCompacted();
    } else {
        m_logOutput->append(QStringLiteral("-> Compaction error: ") + err + QStringLiteral("\n"));
    }
}

} // namespace CodeShelf
