#include "BackupService.h"
#include <QDir>
#include <QFileInfo>

namespace CodeShelf {

BackupService::BackupService(Database& db)
    : m_db(db)
{
}

QString BackupService::generateBackupFilename(const QDateTime& dt) {
    return QStringLiteral("codeshelf_backup_%1_%2.db")
        .arg(dt.toString(QStringLiteral("yyyyMMdd_hhmmss_zzz")),
             QString::number(QDateTime::currentMSecsSinceEpoch() % 10000));
}

bool BackupService::createBackup(const QString& destinationFolder, QString& createdPathOut, QString& errorOut) {
    QDir dir(destinationFolder);
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        errorOut = QStringLiteral("Cannot access or create destination folder: %1").arg(destinationFolder);
        return false;
    }

    const QString fileName = generateBackupFilename();
    const QString targetPath = dir.filePath(fileName);

    if (!m_db.createBackup(targetPath, errorOut)) {
        return false;
    }

    createdPathOut = targetPath;
    return true;
}

bool BackupService::restoreBackup(const QString& backupFilePath, QString& errorOut) {
    QFileInfo fi(backupFilePath);
    if (!fi.exists() || !fi.isFile()) {
        errorOut = QStringLiteral("Backup file does not exist: %1").arg(backupFilePath);
        return false;
    }

    // First take a safety backup of current data before overwriting, with unique safety prefix
    const QString safetyName = QStringLiteral("safety_backup_%1_%2.db")
                                   .arg(QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd_hhmmss_zzz")),
                                        QString::number(QDateTime::currentMSecsSinceEpoch() % 10000));
    const QString safetyPath = fi.dir().filePath(safetyName);
    QString safetyErr;
    if (!m_db.createBackup(safetyPath, safetyErr)) {
        errorOut = QStringLiteral("Failed to create safety backup prior to restore: %1").arg(safetyErr);
        return false;
    }

    // Now restore from source backup
    if (!m_db.restoreBackup(backupFilePath, errorOut)) {
        return false;
    }

    return true;
}

} // namespace CodeShelf
