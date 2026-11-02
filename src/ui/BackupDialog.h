#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "../services/BackupService.h"

namespace CodeShelf {

class BackupDialog : public QDialog {
    Q_OBJECT

public:
    explicit BackupDialog(BackupService& backupService, QWidget* parent = nullptr);

signals:
    void databaseRestored();

private slots:
    void chooseBackupFolder();
    void createBackup();
    void chooseRestoreFile();
    void restoreBackup();

private:
    void setupUi();

    BackupService& m_backupService;

    QLineEdit* m_backupFolderEdit{nullptr};
    QPushButton* m_browseFolderBtn{nullptr};
    QPushButton* m_createBackupBtn{nullptr};
    QLabel* m_backupStatusLabel{nullptr};

    QLineEdit* m_restoreFileEdit{nullptr};
    QPushButton* m_browseFileBtn{nullptr};
    QPushButton* m_restoreBtn{nullptr};
    QLabel* m_restoreStatusLabel{nullptr};
};

} // namespace CodeShelf
