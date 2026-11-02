#include "test_duplicate.h"
#include <QTemporaryDir>

namespace CodeShelf {

static QTemporaryDir* s_tempDir = nullptr;

void TestDuplicate::initTestCase() {
    s_tempDir = new QTemporaryDir();
    QString testDbPath = s_tempDir->filePath(QStringLiteral("test_duplicate.db"));
    QVERIFY(m_db.open(testDbPath));
}

void TestDuplicate::cleanupTestCase() {
    m_db.close();
    delete s_tempDir;
    s_tempDir = nullptr;
}

void TestDuplicate::testExactDuplicateDetection() {
    SnippetRepository repo(m_db);
    DuplicateService dupService(repo);

    QString err;
    Snippet s1(QStringLiteral("Original"), QStringLiteral("int x = 42;"));
    QVERIFY(repo.create(s1, err));

    Snippet s2(QStringLiteral("Copycat"), QStringLiteral("int x = 42;"));
    QVERIFY(repo.create(s2, err));

    auto dups = dupService.findDuplicatesForSnippet(s1);
    QCOMPARE(dups.size(), 1);
    QCOMPARE(dups.first().snippet.id(), s2.id());
    QCOMPARE(dups.first().kind, DuplicateMatchKind::ExactHash);
}

void TestDuplicate::testNormalizedDuplicateDetection() {
    SnippetRepository repo(m_db);
    DuplicateService dupService(repo);

    QString err;
    Snippet s1(QStringLiteral("Unix LF"), QStringLiteral("void foo() {\n    return;\n}"));
    QVERIFY(repo.create(s1, err));

    Snippet s2(QStringLiteral("Windows CRLF with trailing spaces"), QStringLiteral("void foo() {\r\n    return;   \r\n}\r\n"));
    QVERIFY(repo.create(s2, err));

    auto dups = dupService.findDuplicatesForSnippet(s1, true);
    QCOMPARE(dups.size(), 1);
    QCOMPARE(dups.first().kind, DuplicateMatchKind::NormalizedWhitespace);
}

void TestDuplicate::testNonDuplicate() {
    SnippetRepository repo(m_db);
    DuplicateService dupService(repo);

    QString err;
    Snippet s(QStringLiteral("Unique code"), QStringLiteral("std::cout << \"unique\" << std::endl;"));
    QVERIFY(repo.create(s, err));

    auto dups = dupService.findDuplicatesForSnippet(s, true);
    QCOMPARE(dups.size(), 0);
}

} // namespace CodeShelf
