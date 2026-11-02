#include "test_validation.h"

namespace CodeShelf {

void TestValidation::testTitleValidation() {
    QVERIFY(!Validation::validateSnippetTitle(QString()).valid);
    QVERIFY(!Validation::validateSnippetTitle(QStringLiteral("   ")).valid);
    QVERIFY(Validation::validateSnippetTitle(QStringLiteral("Valid Title")).valid);

    QString longTitle(300, QLatin1Char('A'));
    QVERIFY(!Validation::validateSnippetTitle(longTitle).valid);
}

void TestValidation::testSourceUrlValidation() {
    // Optional
    QVERIFY(Validation::validateSourceUrl(QString()).valid);

    // Valid http / https
    QVERIFY(Validation::validateSourceUrl(QStringLiteral("https://github.com/test")).valid);
    QVERIFY(Validation::validateSourceUrl(QStringLiteral("http://example.com/snippet")).valid);

    // Disallowed schemes
    QVERIFY(!Validation::validateSourceUrl(QStringLiteral("javascript:alert(1)")).valid);
    QVERIFY(!Validation::validateSourceUrl(QStringLiteral("file:///C:/Windows/System32")).valid);
    QVERIFY(!Validation::validateSourceUrl(QStringLiteral("data:text/html,test")).valid);
}

void TestValidation::testTagValidationAndNormalization() {
    QVERIFY(!Validation::validateTagName(QString()).valid);
    QVERIFY(!Validation::validateTagName(QStringLiteral("tag,invalid")).valid);
    QVERIFY(Validation::validateTagName(QStringLiteral("c++")).valid);

    QCOMPARE(Validation::normalizeTag(QStringLiteral("  JavaScript  ")), QStringLiteral("javascript"));
}

void TestValidation::testCollectionNameValidation() {
    QVERIFY(!Validation::validateCollectionName(QString()).valid);
    QVERIFY(Validation::validateCollectionName(QStringLiteral("Algorithms")).valid);
}

} // namespace CodeShelf
