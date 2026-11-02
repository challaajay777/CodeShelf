#pragma once

#include <QString>
#include <sqlite3.h>

namespace CodeShelf {

class Migrations {
public:
    static constexpr int CurrentSchemaVersion = 1;

    static bool runMigrations(sqlite3* db, bool fts5Available, QString& errorOut);
    static int getCurrentVersion(sqlite3* db);
    static bool setVersion(sqlite3* db, int version);

private:
    static bool migrateToV1(sqlite3* db, bool fts5Available, QString& errorOut);
};

} // namespace CodeShelf
