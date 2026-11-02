#include "test_search_parser.h"

namespace CodeShelf {

void TestSearchParser::testEmptySearch() {
    SearchQuery q = SearchParser::parse(QString());
    QVERIFY(q.isEmpty());

    SearchQuery q2 = SearchParser::parse(QStringLiteral("    "));
    QVERIFY(q2.isEmpty());
}

void TestSearchParser::testFreeTerms() {
    SearchQuery q = SearchParser::parse(QStringLiteral("binary search tree"));
    QCOMPARE(q.freeTerms.size(), 3);
    QCOMPARE(q.freeTerms.at(0), QStringLiteral("binary"));
    QCOMPARE(q.freeTerms.at(1), QStringLiteral("search"));
    QCOMPARE(q.freeTerms.at(2), QStringLiteral("tree"));
}

void TestSearchParser::testExactPhrase() {
    SearchQuery q = SearchParser::parse(QStringLiteral("\"std::vector<int>\""));
    QCOMPARE(q.exactPhrases.size(), 1);
    QCOMPARE(q.exactPhrases.at(0), QStringLiteral("std::vector<int>"));
}

void TestSearchParser::testOperators() {
    SearchQuery q = SearchParser::parse(QStringLiteral("lang:cpp tag:graphs is:fav type:note"));
    QCOMPARE(q.languageFilter, QStringLiteral("cpp"));
    QCOMPARE(q.tagFilter, QStringLiteral("graphs"));
    QVERIFY(q.favoriteFilter.has_value() && *q.favoriteFilter == true);
    QVERIFY(q.typeFilter.has_value() && *q.typeFilter == SnippetType::TechnicalNote);
}

void TestSearchParser::testCombinedSearch() {
    SearchQuery q = SearchParser::parse(QStringLiteral("dijkstra \"shortest path\" lang:python is:pin"));
    QCOMPARE(q.freeTerms.size(), 1);
    QCOMPARE(q.freeTerms.at(0), QStringLiteral("dijkstra"));
    QCOMPARE(q.exactPhrases.size(), 1);
    QCOMPARE(q.exactPhrases.at(0), QStringLiteral("shortest path"));
    QCOMPARE(q.languageFilter, QStringLiteral("python"));
    QVERIFY(q.pinnedFilter.has_value() && *q.pinnedFilter == true);
}

} // namespace CodeShelf
