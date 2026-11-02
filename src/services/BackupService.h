#pragma once

#include <QString>
#include <QDateTime>
#include "../database/Database.h"

namespace CodeShelf {

class BackupService {
public:
    explicit BackupService(Database& db = Database::instance());

    bool createBackup(const QString& destinationFolder, QString& createdPathOut, QString& errorOut);
    bool restoreBackup(const QString& backupFilePath, QString& errorOut);
    static QString generateBackupFilename(const QDateTime& dt = QDateTime::currentDateTimeUtc());

private:
    Database& m_db;
};

} // namespace CodeShelf
