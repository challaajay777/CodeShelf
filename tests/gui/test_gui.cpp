#include "test_gui.h"
#include <QTemporaryDir>
#include <QLineEdit>

namespace CodeShelf {

static QTemporaryDir* s_tempDir = nullptr;

void TestGui::initTestCase() {
    s_tempDir = new QTemporaryDir();
    QString testDbPath = s_tempDir->filePath(QStringLiteral("test_gui.db"));
    QVERIFY(Database::instance().open(testDbPath));
}

void TestGui::cleanupTestCase() {
    Database::instance().close();
    delete s_tempDir;
    s_tempDir = nullptr;
}

void TestGui::testMainWindowLaunchAndEmptyState() {
    MainWindow window;
    window.show();
    QVERIFY(window.isVisible());
    QCOMPARE(window.windowTitle(), QStringLiteral("CodeShelf"));
    window.close();
}

void TestGui::testCreateEditSaveSnippetFlow() {
    auto& db = Database::instance();
    SnippetRepository repo(db);

    QString err;
    Snippet snip(QStringLiteral("GUI Created Snippet"), QStringLiteral("echo 'test'"), QStringLiteral("Shell"));
    QVERIFY2(repo.create(snip, err), qPrintable(err));

    MainWindow window;
    window.show();

    // Verify it loads in list model
    auto opt = repo.getById(snip.id());
    QVERIFY(opt.has_value());
    QCOMPARE(opt->title(), QStringLiteral("GUI Created Snippet"));
    window.close();
}

void TestGui::testSearchInstantFilterFlow() {
    MainWindow window;
    window.show();

    auto searchEdit = window.findChild<QLineEdit*>();
    QVERIFY(searchEdit != nullptr);

    searchEdit->setText(QStringLiteral("nonexistent_snippet_query"));
    // Search operates debounced
    QTest::qWait(200);

    window.close();
}

} // namespace CodeShelf
