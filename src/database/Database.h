#pragma once

#include <QString>
#include <QRecursiveMutex>
#include <sqlite3.h>
#include "SqliteHelper.h"

namespace CodeShelf {

class Database {
public:
    static Database& instance();

    bool open(const QString& dbPath = QString());
    void close();
    bool isOpen() const { return m_db != nullptr; }

    sqlite3* rawDb() { return m_db; }
    QRecursiveMutex& mutex() { return m_mutex; }

    bool isFts5Available() const { return m_fts5Available; }
    QString lastError() const { return m_lastError; }
    QString databasePath() const { return m_dbPath; }

    // Maintenance & Integrity
    bool checkIntegrity(QStringList& issuesOut);
    bool vacuumDatabase(QString& errorOut);
    bool rebuildFtsIndex(QString& errorOut);

    // Online WAL-Safe Backup API
    bool createBackup(const QString& destinationPath, QString& errorOut);
    bool restoreBackup(const QString& sourceBackupPath, QString& errorOut);

    // Schema info
    int schemaVersion() const;

private:
    Database();
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    bool detectFts5();
    bool configureConnection();

    sqlite3* m_db{nullptr};
    QString m_dbPath;
    QString m_lastError;
    bool m_fts5Available{false};
    mutable QRecursiveMutex m_mutex;
};

} // namespace CodeShelf
