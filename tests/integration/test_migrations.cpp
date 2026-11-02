#include "test_migrations.h"
#include <QTemporaryDir>

namespace CodeShelf {

void TestMigrations::testFreshDatabaseMigration() {
    QTemporaryDir tempDir;
    sqlite3* rawDb = nullptr;
    const QString dbPath = tempDir.filePath(QStringLiteral("migration_fresh.db"));

    QCOMPARE(sqlite3_open(dbPath.toUtf8().constData(), &rawDb), SQLITE_OK);

    QString err;
    QVERIFY(Migrations::runMigrations(rawDb, true, err));
    QCOMPARE(Migrations::getCurrentVersion(rawDb), 1);

    sqlite3_close(rawDb);
}

void TestMigrations::testMigrationIdempotence() {
    QTemporaryDir tempDir;
    sqlite3* rawDb = nullptr;
    const QString dbPath = tempDir.filePath(QStringLiteral("migration_idem.db"));

    QCOMPARE(sqlite3_open(dbPath.toUtf8().constData(), &rawDb), SQLITE_OK);

    QString err;
    QVERIFY(Migrations::runMigrations(rawDb, true, err));
    QCOMPARE(Migrations::getCurrentVersion(rawDb), 1);

    // Running second time should succeed immediately
    QVERIFY(Migrations::runMigrations(rawDb, true, err));
    QCOMPARE(Migrations::getCurrentVersion(rawDb), 1);

    sqlite3_close(rawDb);
}

void TestMigrations::testNewerVersionRejection() {
    QTemporaryDir tempDir;
    sqlite3* rawDb = nullptr;
    const QString dbPath = tempDir.filePath(QStringLiteral("migration_future.db"));

    QCOMPARE(sqlite3_open(dbPath.toUtf8().constData(), &rawDb), SQLITE_OK);

    // Artificially set user_version to 99
    QVERIFY(Migrations::setVersion(rawDb, 99));

    QString err;
    QVERIFY(!Migrations::runMigrations(rawDb, true, err));
    QVERIFY(err.contains(QStringLiteral("newer schema version")));

    sqlite3_close(rawDb);
}

} // namespace CodeShelf
