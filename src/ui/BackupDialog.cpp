#include "BackupDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include "../settings/AppSettings.h"

namespace CodeShelf {

BackupDialog::BackupDialog(BackupService& backupService, QWidget* parent)
    : QDialog(parent)
    , m_backupService(backupService)
{
    setWindowTitle(QStringLiteral("WAL-Safe Backup & Restore"));
    resize(580, 360);
    setupUi();
}

void BackupDialog::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);

    // 1. Create Backup Group
    auto backupGroup = new QGroupBox(QStringLiteral("Create Timestamped Backup"), this);
    auto bgLayout = new QVBoxLayout(backupGroup);

    auto folderRow = new QHBoxLayout();
    m_backupFolderEdit = new QLineEdit(AppSettings::instance().backupDirectory(), this);
    m_browseFolderBtn = new QPushButton(QStringLiteral("Browse..."), this);
    folderRow->addWidget(new QLabel(QStringLiteral("Destination Folder:")));
    folderRow->addWidget(m_backupFolderEdit, 1);
    folderRow->addWidget(m_browseFolderBtn);
    bgLayout->addLayout(folderRow);

    m_createBackupBtn = new QPushButton(QStringLiteral("Create Live WAL-Safe Backup Now"), this);
    bgLayout->addWidget(m_createBackupBtn);

    m_backupStatusLabel = new QLabel(this);
    m_backupStatusLabel->setStyleSheet(QStringLiteral("color: #444;"));
    bgLayout->addWidget(m_backupStatusLabel);

    mainLayout->addWidget(backupGroup);

    // 2. Restore Backup Group
    auto restoreGroup = new QGroupBox(QStringLiteral("Restore Database from Backup"), this);
    auto rgLayout = new QVBoxLayout(restoreGroup);

    auto fileRow = new QHBoxLayout();
    m_restoreFileEdit = new QLineEdit(this);
    m_restoreFileEdit->setPlaceholderText(QStringLiteral("Select .db backup file to restore..."));
    m_browseFileBtn = new QPushButton(QStringLiteral("Browse..."), this);
    fileRow->addWidget(new QLabel(QStringLiteral("Backup File:")));
    fileRow->addWidget(m_restoreFileEdit, 1);
    fileRow->addWidget(m_browseFileBtn);
    rgLayout->addLayout(fileRow);

    m_restoreBtn = new QPushButton(QStringLiteral("Restore from File"), this);
    rgLayout->addWidget(m_restoreBtn);

    m_restoreStatusLabel = new QLabel(this);
    m_restoreStatusLabel->setStyleSheet(QStringLiteral("color: #444;"));
    rgLayout->addWidget(m_restoreStatusLabel);

    mainLayout->addWidget(restoreGroup);

    auto closeBtn = new QPushButton(QStringLiteral("Close"), this);
    mainLayout->addWidget(closeBtn, 0, Qt::AlignRight);

    connect(m_browseFolderBtn, &QPushButton::clicked, this, &BackupDialog::chooseBackupFolder);
    connect(m_createBackupBtn, &QPushButton::clicked, this, &BackupDialog::createBackup);
    connect(m_browseFileBtn, &QPushButton::clicked, this, &BackupDialog::chooseRestoreFile);
    connect(m_restoreBtn, &QPushButton::clicked, this, &BackupDialog::restoreBackup);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void BackupDialog::chooseBackupFolder() {
    QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("Select Backup Folder"), m_backupFolderEdit->text());
    if (!dir.isEmpty()) {
        m_backupFolderEdit->setText(dir);
        AppSettings::instance().setBackupDirectory(dir);
    }
}

void BackupDialog::createBackup() {
    const QString folder = m_backupFolderEdit->text().trimmed();
    if (folder.isEmpty()) return;

    QString createdPath;
    QString err;
    if (m_backupService.createBackup(folder, createdPath, err)) {
        m_backupStatusLabel->setText(QStringLiteral("Backup created successfully:\n%1").arg(createdPath));
        QMessageBox::information(this, QStringLiteral("Backup Complete"), QStringLiteral("Backup created:\n%1").arg(createdPath));
    } else {
        m_backupStatusLabel->setText(QStringLiteral("Backup failed: %1").arg(err));
        QMessageBox::warning(this, QStringLiteral("Backup Failed"), err);
    }
}

void BackupDialog::chooseRestoreFile() {
    QString file = QFileDialog::getOpenFileName(this, QStringLiteral("Select Backup File"), m_backupFolderEdit->text(),
                                              QStringLiteral("SQLite Databases (*.db);;All Files (*.*)"));
    if (!file.isEmpty()) {
        m_restoreFileEdit->setText(file);
    }
}

void BackupDialog::restoreBackup() {
    const QString filePath = m_restoreFileEdit->text().trimmed();
    if (filePath.isEmpty()) return;

    if (QMessageBox::warning(this, QStringLiteral("Confirm Database Restore"),
                             QStringLiteral("Restoring will replace the active database with the contents of the backup.\n\n"
                                            "An automatic safety backup of your current database will be created first.\n\n"
                                            "Do you want to proceed?"),
                             QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    QString err;
    if (m_backupService.restoreBackup(filePath, err)) {
        m_restoreStatusLabel->setText(QStringLiteral("Restore completed successfully."));
        QMessageBox::information(this, QStringLiteral("Restore Complete"), QStringLiteral("Database restored successfully. Active library will now reload."));
        emit databaseRestored();
        accept();
    } else {
        m_restoreStatusLabel->setText(QStringLiteral("Restore failed: %1").arg(err));
        QMessageBox::critical(this, QStringLiteral("Restore Failed"), err);
    }
}

} // namespace CodeShelf
