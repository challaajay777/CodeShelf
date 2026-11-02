#include "test_diff.h"

namespace CodeShelf {

void TestDiff::testIdenticalDiff() {
    const QString text = QStringLiteral("line 1\nline 2\nline 3");
    auto diff = DiffGenerator::generateDiff(text, text);

    QCOMPARE(diff.size(), 3);
    for (const auto& line : diff) {
        QCOMPARE(line.type, DiffLineType::Context);
    }
}

void TestDiff::testAdditionsAndDeletions() {
    const QString oldText = QStringLiteral("apple\nbanana\ncherry");
    const QString newText = QStringLiteral("apple\nblueberry\ncherry\ndate");

    auto diff = DiffGenerator::generateDiff(oldText, newText);

    bool foundBananaDel = false;
    bool foundBlueberryAdd = false;
    bool foundDateAdd = false;

    for (const auto& line : diff) {
        if (line.type == DiffLineType::Deletion && line.text == QStringLiteral("banana")) {
            foundBananaDel = true;
        }
        if (line.type == DiffLineType::Addition && line.text == QStringLiteral("blueberry")) {
            foundBlueberryAdd = true;
        }
        if (line.type == DiffLineType::Addition && line.text == QStringLiteral("date")) {
            foundDateAdd = true;
        }
    }

    QVERIFY(foundBananaDel);
    QVERIFY(foundBlueberryAdd);
    QVERIFY(foundDateAdd);
}

void TestDiff::testEmptyOldText() {
    auto diff = DiffGenerator::generateDiff(QString(), QStringLiteral("new line"));
    QCOMPARE(diff.size(), 1);
    QCOMPARE(diff.first().type, DiffLineType::Addition);
}

void TestDiff::testEmptyNewText() {
    auto diff = DiffGenerator::generateDiff(QStringLiteral("old line"), QString());
    QCOMPARE(diff.size(), 1);
    QCOMPARE(diff.first().type, DiffLineType::Deletion);
}

} // namespace CodeShelf
