#include "test_import_export.h"
#include <QTemporaryDir>
#include <QFile>

namespace CodeShelf {

static QTemporaryDir* s_tempDir = nullptr;

void TestImportExport::initTestCase() {
    s_tempDir = new QTemporaryDir();
    QString testDbPath = s_tempDir->filePath(QStringLiteral("test_io.db"));
    QVERIFY(Database::instance().open(testDbPath));
}

void TestImportExport::cleanupTestCase() {
    Database::instance().close();
    delete s_tempDir;
    s_tempDir = nullptr;
}

void TestImportExport::testMarkdownCodeFenceEscaping() {
    // Ordinary code without backticks -> standard 3 backticks
    QCOMPARE(MarkdownExportHandler::determineSafeCodeFence(QStringLiteral("int x = 1;")), QStringLiteral("```"));

    // Code containing 3 backticks -> must use 4 backticks
    const QString codeWithBackticks = QStringLiteral("Use ```markdown``` here");
    QCOMPARE(MarkdownExportHandler::determineSafeCodeFence(codeWithBackticks), QStringLiteral("````"));

    // Code containing 4 backticks -> must use 5 backticks
    const QString codeWith4 = QStringLiteral("````code block````");
    QCOMPARE(MarkdownExportHandler::determineSafeCodeFence(codeWith4), QStringLiteral("`````"));

    Snippet s(QStringLiteral("Backtick Snippet"), codeWithBackticks, QStringLiteral("Markdown"));
    QString md = MarkdownExportHandler::snippetToMarkdown(s);
    QVERIFY(md.contains(QStringLiteral("````markdown")));
}

void TestImportExport::testJsonExportAndImportRoundTrip() {
    auto& db = Database::instance();
    SnippetRepository snipRepo(db);
    CollectionRepository colRepo(db);
    TagRepository tagRepo(db);
    TemplateRepository tmplRepo(db);
    JsonBackupHandler handler(snipRepo, colRepo, tagRepo, tmplRepo);

    // Create sample data
    QString err;
    Collection col(QStringLiteral("Export Col"));
    QVERIFY(colRepo.create(col, err));

    Snippet snip(QStringLiteral("RoundTrip Snippet"), QStringLiteral("printf(\"hello\\n\");"), QStringLiteral("C"));
    snip.setCollectionId(col.id());
    snip.setTags({QStringLiteral("stdio"), QStringLiteral("c-lang")});
    QVERIFY(snipRepo.create(snip, err));

    // Export to JSON
    const QString jsonPath = s_tempDir->filePath(QStringLiteral("roundtrip.json"));
    QVERIFY(handler.exportBackup(jsonPath, {}, true, true, err));

    // Preview import from exported file
    ImportPreviewSummary preview;
    QVERIFY(handler.parseAndPreview(jsonPath, preview, err));
    QCOMPARE(preview.totalInFile, 1);
    QCOMPARE(preview.duplicateIdCount, 1); // Already exists in active DB
}

void TestImportExport::testImportInvalidJson() {
    auto& db = Database::instance();
    SnippetRepository snipRepo(db);
    CollectionRepository colRepo(db);
    TagRepository tagRepo(db);
    TemplateRepository tmplRepo(db);
    JsonBackupHandler handler(snipRepo, colRepo, tagRepo, tmplRepo);

    const QString badJsonPath = s_tempDir->filePath(QStringLiteral("corrupted.json"));
    QFile f(badJsonPath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("{ invalid json text [");
    f.close();

    ImportPreviewSummary preview;
    QString err;
    QVERIFY(!handler.parseAndPreview(badJsonPath, preview, err));
    QVERIFY(err.contains(QStringLiteral("JSON syntax error")));
}

void TestImportExport::testImportDuplicateIdConflict() {
    auto& db = Database::instance();
    SnippetRepository snipRepo(db);
    CollectionRepository colRepo(db);
    TagRepository tagRepo(db);
    TemplateRepository tmplRepo(db);
    JsonBackupHandler handler(snipRepo, colRepo, tagRepo, tmplRepo);

    Snippet orig(QStringLiteral("Unique Title"), QStringLiteral("content A"));
    QString err;
    QVERIFY(snipRepo.create(orig, err));

    // Create a JSON payload with same ID but different title/content
    const QString clashPath = s_tempDir->filePath(QStringLiteral("clash.json"));
    QFile f(clashPath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    QString json = QStringLiteral(
        "{\n"
        "  \"format\": \"codeshelf-backup\",\n"
        "  \"schema_version\": 1,\n"
        "  \"exported_at\": \"2026-11-02T00:00:00Z\",\n"
        "  \"application_version\": \"0.1.0\",\n"
        "  \"snippets\": [\n"
        "    {\n"
        "      \"id\": \"%1\",\n"
        "      \"title\": \"Clashing Title\",\n"
        "      \"body\": \"content B\",\n"
        "      \"language\": \"Plain Text\",\n"
        "      \"tags\": []\n"
        "    }\n"
        "  ]\n"
        "}\n"
    ).arg(orig.id());
    f.write(json.toUtf8());
    f.close();

    ImportPreviewSummary preview;
    QVERIFY(handler.parseAndPreview(clashPath, preview, err));
    QCOMPARE(preview.duplicateIdCount, 1);

    // Apply with KeepBothNewId
    QVERIFY(handler.applyImport(preview, ConflictResolution::KeepBothNewId, err));

    // Both should now exist
    auto snips = snipRepo.getAllActive();
    bool foundImported = false;
    for (const auto& s : snips) {
        if (s.title().contains(QStringLiteral("Clashing Title"))) {
            foundImported = true;
            QVERIFY(s.id() != orig.id()); // Must have been assigned a new ID
        }
    }
    QVERIFY(foundImported);
}

} // namespace CodeShelf
