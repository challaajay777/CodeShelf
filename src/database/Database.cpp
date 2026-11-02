#include "Database.h"
#include "Migrations.h"
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

namespace CodeShelf {

Database& Database::instance() {
    static Database db;
    return db;
}

Database::Database() = default;

Database::~Database() {
    close();
}

bool Database::open(const QString& dbPath) {
    QMutexLocker locker(&m_mutex);
    if (m_db) {
        close();
    }

    if (dbPath.isEmpty()) {
        const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        m_dbPath = dataDir + QStringLiteral("/codeshelf.db");
    } else {
        m_dbPath = dbPath;
        if (m_dbPath != QStringLiteral(":memory:")) {
            QFileInfo fi(m_dbPath);
            QDir().mkpath(fi.absolutePath());
        }
    }

    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    int rc = sqlite3_open_v2(m_dbPath.toUtf8().constData(), &m_db, flags, nullptr);
    if (rc != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        if (m_db) {
            sqlite3_close(m_db);
            m_db = nullptr;
        }
        return false;
    }

    if (!configureConnection()) {
        close();
        return false;
    }

    m_fts5Available = detectFts5();

    // Run schema migrations
    QString migrationErr;
    if (!Migrations::runMigrations(m_db, m_fts5Available, migrationErr)) {
        m_lastError = migrationErr;
        close();
        return false;
    }

    return true;
}

void Database::close() {
    if (m_db) {
        sqlite3_close_v2(m_db);
        m_db = nullptr;
    }
}

bool Database::configureConnection() {
    if (!m_db) return false;

    char* errMsg = nullptr;

    // Enable busy timeout (5000 ms)
    sqlite3_busy_timeout(m_db, 5000);

    // Verify foreign keys enabled
    if (sqlite3_exec(m_db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &errMsg) != SQLITE_OK) {
        m_lastError = errMsg ? QString::fromUtf8(errMsg) : QStringLiteral("Failed enabling foreign keys.");
        if (errMsg) sqlite3_free(errMsg);
        return false;
    }

    // Verify foreign keys are truly ON
    sqlite3_stmt* stmt = nullptr;
    int fkStatus = 0;
    if (sqlite3_prepare_v2(m_db, "PRAGMA foreign_keys;", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            fkStatus = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    if (fkStatus != 1) {
        m_lastError = QStringLiteral("Foreign key enforcement could not be verified.");
        return false;
    }

    // Enable WAL mode for disk databases
    if (m_dbPath != QStringLiteral(":memory:")) {
        sqlite3_exec(m_db, "PRAGMA journal_mode = WAL;", nullptr, nullptr, nullptr);
        sqlite3_exec(m_db, "PRAGMA synchronous = NORMAL;", nullptr, nullptr, nullptr);
    }

    return true;
}

bool Database::detectFts5() {
    if (!m_db) return false;
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, "CREATE VIRTUAL TABLE temp.codeshelf_fts_check USING fts5(content);", -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_exec(m_db, "DROP TABLE temp.codeshelf_fts_check;", nullptr, nullptr, nullptr);
        return true;
    }
    if (stmt) {
        sqlite3_finalize(stmt);
    }
    return false;
}

int Database::schemaVersion() const {
    QMutexLocker locker(&m_mutex);
    return Migrations::getCurrentVersion(m_db);
}

bool Database::checkIntegrity(QStringList& issuesOut) {
    QMutexLocker locker(&m_mutex);
    issuesOut.clear();
    if (!m_db) {
        issuesOut.append(QStringLiteral("Database is not open."));
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, "PRAGMA integrity_check;", -1, &stmt, nullptr) != SQLITE_OK) {
        issuesOut.append(QString::fromUtf8(sqlite3_errmsg(m_db)));
        return false;
    }

    bool clean = true;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const QString res = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        if (res.compare(QStringLiteral("ok"), Qt::CaseInsensitive) != 0) {
            clean = false;
            issuesOut.append(res);
        }
    }
    sqlite3_finalize(stmt);

    if (clean) {
        issuesOut.append(QStringLiteral("ok"));
    }
    return clean;
}

bool Database::vacuumDatabase(QString& errorOut) {
    QMutexLocker locker(&m_mutex);
    if (!m_db) {
        errorOut = QStringLiteral("Database is not open.");
        return false;
    }

    // Run WAL checkpoint first
    sqlite3_wal_checkpoint_v2(m_db, nullptr, SQLITE_CHECKPOINT_TRUNCATE, nullptr, nullptr);

    char* errMsg = nullptr;
    int rc = sqlite3_exec(m_db, "VACUUM;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        errorOut = errMsg ? QString::fromUtf8(errMsg) : QStringLiteral("VACUUM failed.");
        if (errMsg) sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::rebuildFtsIndex(QString& errorOut) {
    QMutexLocker locker(&m_mutex);
    if (!m_db) {
        errorOut = QStringLiteral("Database is not open.");
        return false;
    }
    if (!m_fts5Available) {
        errorOut = QStringLiteral("FTS5 is not available in this environment.");
        return false;
    }

    SqliteTransaction tx(m_db);
    sqlite3_exec(m_db, "DELETE FROM snippets_fts;", nullptr, nullptr, nullptr);

    const char* populateSql = R"(
        INSERT INTO snippets_fts (snippet_id, title, description, body, tags)
        SELECT s.id, s.title, COALESCE(s.description, ''), s.body,
               COALESCE((SELECT GROUP_CONCAT(t.display_name, ' ')
                         FROM snippet_tags st
                         JOIN tags t ON st.tag_id = t.id
                         WHERE st.snippet_id = s.id), '')
        FROM snippets s
        WHERE s.deleted_at IS NULL;
    )";

    char* errMsg = nullptr;
    if (sqlite3_exec(m_db, populateSql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        errorOut = errMsg ? QString::fromUtf8(errMsg) : QStringLiteral("Failed populating FTS index.");
        if (errMsg) sqlite3_free(errMsg);
        return false;
    }

    return tx.commit();
}

bool Database::createBackup(const QString& destinationPath, QString& errorOut) {
    QMutexLocker locker(&m_mutex);
    if (!m_db) {
        errorOut = QStringLiteral("Source database is not open.");
        return false;
    }

    // Open target database file
    sqlite3* destDb = nullptr;
    int rc = sqlite3_open_v2(destinationPath.toUtf8().constData(), &destDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK) {
        errorOut = destDb ? QString::fromUtf8(sqlite3_errmsg(destDb)) : QStringLiteral("Cannot open destination backup file.");
        if (destDb) sqlite3_close(destDb);
        return false;
    }

    sqlite3_backup* backup = sqlite3_backup_init(destDb, "main", m_db, "main");
    if (!backup) {
        errorOut = QString::fromUtf8(sqlite3_errmsg(destDb));
        sqlite3_close(destDb);
        return false;
    }

    // Step through copying pages
    rc = sqlite3_backup_step(backup, -1);
    sqlite3_backup_finish(backup);

    if (rc != SQLITE_DONE) {
        errorOut = QString::fromUtf8(sqlite3_errmsg(destDb));
        sqlite3_close(destDb);
        return false;
    }

    sqlite3_close(destDb);
    return true;
}

bool Database::restoreBackup(const QString& sourceBackupPath, QString& errorOut) {
    QMutexLocker locker(&m_mutex);
    if (!m_db) {
        errorOut = QStringLiteral("Target database is not open.");
        return false;
    }

    // 1. Validate backup database first
    sqlite3* srcDb = nullptr;
    int rc = sqlite3_open_v2(sourceBackupPath.toUtf8().constData(), &srcDb, SQLITE_OPEN_READONLY, nullptr);
    if (rc != SQLITE_OK) {
        errorOut = QStringLiteral("Cannot open backup source file for reading.");
        if (srcDb) sqlite3_close(srcDb);
        return false;
    }

    int backupVersion = Migrations::getCurrentVersion(srcDb);
    sqlite3_close(srcDb);

    if (backupVersion <= 0 || backupVersion > Migrations::CurrentSchemaVersion) {
        errorOut = QStringLiteral("Backup file has incompatible or corrupted schema version: %1").arg(backupVersion);
        return false;
    }

    const QString savedDbPath = m_dbPath;

    if (savedDbPath == QStringLiteral(":memory:")) {
        // In-memory restore using online backup API
        sqlite3* memSrc = nullptr;
        sqlite3_open_v2(sourceBackupPath.toUtf8().constData(), &memSrc, SQLITE_OPEN_READONLY, nullptr);
        sqlite3_backup* backup = sqlite3_backup_init(m_db, "main", memSrc, "main");
        if (!backup) {
            errorOut = QString::fromUtf8(sqlite3_errmsg(m_db));
            sqlite3_close(memSrc);
            return false;
        }
        rc = sqlite3_backup_step(backup, -1);
        sqlite3_backup_finish(backup);
        sqlite3_close(memSrc);
        return (rc == SQLITE_DONE);
    }

    // 2. Disk restore: close active connection to release locks and page caches
    close();

    // 3. Clean up existing WAL and SHM files
    QFile::remove(savedDbPath + QStringLiteral("-wal"));
    QFile::remove(savedDbPath + QStringLiteral("-shm"));
    QFile::remove(savedDbPath);

    // 4. Copy restored database file into place
    if (!QFile::copy(sourceBackupPath, savedDbPath)) {
        errorOut = QStringLiteral("Failed copying backup file to database location.");
        // Try to re-open whatever is there
        locker.unlock();
        open(savedDbPath);
        return false;
    }

    // 5. Re-open cleanly
    locker.unlock();
    return open(savedDbPath);
}

} // namespace CodeShelf
