#include "test_database.h"
#include <QTemporaryDir>
#include "repositories/SnippetRepository.h"
#include "repositories/CollectionRepository.h"
#include "repositories/TagRepository.h"

namespace CodeShelf {

static QTemporaryDir* s_tempDir = nullptr;

void TestDatabase::initTestCase() {
    s_tempDir = new QTemporaryDir();
    QString testDbPath = s_tempDir->filePath(QStringLiteral("test_db_integ.db"));
    QVERIFY(Database::instance().open(testDbPath));
}

void TestDatabase::cleanupTestCase() {
    Database::instance().close();
    delete s_tempDir;
    s_tempDir = nullptr;
}

void TestDatabase::testForeignKeysEnforced() {
    auto& db = Database::instance();
    QMutexLocker locker(&db.mutex());

    // Verify PRAGMA foreign_keys is 1
    SqliteStatement stmt(db.rawDb(), QStringLiteral("PRAGMA foreign_keys;"));
    QVERIFY(stmt.step() == SQLITE_ROW);
    QCOMPARE(stmt.getInt(0), 1);

    // Attempting to insert snippet_tags with non-existent tag must fail foreign key constraint
    SqliteStatement badInsert(db.rawDb(), QStringLiteral("INSERT INTO snippet_tags (snippet_id, tag_id) VALUES ('fake_snip', 'fake_tag');"));
    QVERIFY(!badInsert.execute());
}

void TestDatabase::testTransactionCommitAndRollback() {
    auto& db = Database::instance();
    CollectionRepository colRepo(db);

    // 1. Rollback test
    {
        QMutexLocker locker(&db.mutex());
        SqliteTransaction tx(db.rawDb());
        SqliteStatement stmt(db.rawDb(), QStringLiteral("INSERT INTO collections (id, name, created_at, updated_at) VALUES ('c_temp', 'Temp Col', '2026-01-01', '2026-01-01');"));
        QVERIFY(stmt.execute());
        // Do not commit -> Destructor rolls back
    }
    QVERIFY(!colRepo.getById(QStringLiteral("c_temp")).has_value());

    // 2. Commit test
    {
        QMutexLocker locker(&db.mutex());
        SqliteTransaction tx(db.rawDb());
        SqliteStatement stmt(db.rawDb(), QStringLiteral("INSERT INTO collections (id, name, created_at, updated_at) VALUES ('c_perm', 'Perm Col', '2026-01-01', '2026-01-01');"));
        QVERIFY(stmt.execute());
        QVERIFY(tx.commit());
    }
    QVERIFY(colRepo.getById(QStringLiteral("c_perm")).has_value());
}

void TestDatabase::testSnippetCrudAndSoftDelete() {
    auto& db = Database::instance();
    SnippetRepository repo(db);

    QString err;
    Snippet s(QStringLiteral("CRUD Test"), QStringLiteral("console.log('hi');"), QStringLiteral("JavaScript"));
    s.setDescription(QStringLiteral("Test desc"));
    s.setTags({QStringLiteral("web"), QStringLiteral("js")});
    s.setFavorite(true);

    // Create
    QVERIFY(repo.create(s, err));

    // Read
    auto opt = repo.getById(s.id());
    QVERIFY(opt.has_value());
    QCOMPARE(opt->title(), QStringLiteral("CRUD Test"));
    QCOMPARE(opt->body(), QStringLiteral("console.log('hi');"));
    QCOMPARE(opt->tags().size(), 2);
    QVERIFY(opt->isFavorite());

    // Update
    opt->setTitle(QStringLiteral("Updated Title"));
    QVERIFY(repo.update(*opt, true, err));
    auto updatedOpt = repo.getById(s.id());
    QVERIFY(updatedOpt.has_value());
    QCOMPARE(updatedOpt->title(), QStringLiteral("Updated Title"));

    // Soft delete (Move to Trash)
    QVERIFY(repo.softDelete(s.id(), err));
    auto deletedOpt = repo.getById(s.id());
    QVERIFY(deletedOpt.has_value());
    QVERIFY(deletedOpt->isDeleted());

    // Restore from Trash
    QVERIFY(repo.restore(s.id(), err));
    auto restoredOpt = repo.getById(s.id());
    QVERIFY(restoredOpt.has_value());
    QVERIFY(!restoredOpt->isDeleted());

    // Permanent delete
    QVERIFY(repo.permanentDelete(s.id(), err));
    QVERIFY(!repo.getById(s.id()).has_value());
}

void TestDatabase::testCollectionOperations() {
    auto& db = Database::instance();
    CollectionRepository colRepo(db);
    SnippetRepository snipRepo(db);

    QString err;
    Collection c(QStringLiteral("Web Dev"), QStringLiteral("Web snippets"));
    QVERIFY(colRepo.create(c, err));

    Snippet s(QStringLiteral("HTML Boilerplate"), QStringLiteral("<!DOCTYPE html>"), QStringLiteral("HTML"));
    s.setCollectionId(c.id());
    QVERIFY(snipRepo.create(s, err));

    // Deleting collection with deleteSnippets = false should unfile the snippet, not delete it
    QVERIFY(colRepo.deleteCollection(c.id(), false, err));
    auto snipOpt = snipRepo.getById(s.id());
    QVERIFY(snipOpt.has_value());
    QVERIFY(snipOpt->collectionId().isEmpty()); // Successfully unfiled
}

void TestDatabase::testTagOperations() {
    auto& db = Database::instance();
    TagRepository tagRepo(db);
    SnippetRepository snipRepo(db);

    QString err;
    Tag t1 = tagRepo.getOrCreateTag(QStringLiteral("Frontend"), err);
    QVERIFY(!t1.id().isEmpty());

    Tag t2 = tagRepo.getOrCreateTag(QStringLiteral("UI"), err);
    QVERIFY(!t2.id().isEmpty());

    // Merge t1 into t2
    QVERIFY(tagRepo.mergeTags(t1.id(), t2.id(), err));
    QVERIFY(!tagRepo.getById(t1.id()).has_value());
    QVERIFY(tagRepo.getById(t2.id()).has_value());
}

void TestDatabase::testIntegrityCheck() {
    auto& db = Database::instance();
    QStringList issues;
    QVERIFY(db.checkIntegrity(issues));
    QCOMPARE(issues.first(), QStringLiteral("ok"));
}

} // namespace CodeShelf
