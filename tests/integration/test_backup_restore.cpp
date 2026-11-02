#include "test_backup_restore.h"
#include <QTemporaryDir>
#include "repositories/SnippetRepository.h"

namespace CodeShelf {

static QTemporaryDir* s_tempDir = nullptr;

void TestBackupRestore::initTestCase() {
    s_tempDir = new QTemporaryDir();
    QString testDbPath = s_tempDir->filePath(QStringLiteral("test_live.db"));
    QVERIFY(Database::instance().open(testDbPath));
}

void TestBackupRestore::cleanupTestCase() {
    Database::instance().close();
    delete s_tempDir;
    s_tempDir = nullptr;
}

void TestBackupRestore::testWalSafeOnlineBackupAndRestore() {
    auto& db = Database::instance();
    SnippetRepository repo(db);
    BackupService backupService(db);

    // Insert a snippet
    QString err;
    Snippet s(QStringLiteral("Pre-Backup Snippet"), QStringLiteral("int a = 100;"));
    QVERIFY(repo.create(s, err));

    // Create backup
    QString backupPath;
    QVERIFY(backupService.createBackup(s_tempDir->path(), backupPath, err));
    QVERIFY(QFile::exists(backupPath));

    // Delete snippet in live database
    QVERIFY(repo.permanentDelete(s.id(), err));
    QVERIFY(!repo.getById(s.id()).has_value());

    // Restore from backup
    QVERIFY2(backupService.restoreBackup(backupPath, err), qPrintable(err));

    // Snippet should be back!
    auto restored = repo.getById(s.id());
    QVERIFY(restored.has_value());
    QCOMPARE(restored->title(), QStringLiteral("Pre-Backup Snippet"));
}

void TestBackupRestore::testRestoreInvalidFile() {
    auto& db = Database::instance();
    BackupService backupService(db);

    const QString fakePath = s_tempDir->filePath(QStringLiteral("fake_backup.db"));
    QFile f(fakePath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("not an sqlite db file");
    f.close();

    QString err;
    QVERIFY(!backupService.restoreBackup(fakePath, err));
}

} // namespace CodeShelf
