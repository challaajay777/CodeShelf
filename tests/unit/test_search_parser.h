#pragma once

#include <QObject>
#include <QTest>
#include "search/SearchParser.h"

namespace CodeShelf {

class TestSearchParser : public QObject {
    Q_OBJECT

private slots:
    void testEmptySearch();
    void testFreeTerms();
    void testExactPhrase();
    void testOperators();
    void testCombinedSearch();
};

} // namespace CodeShelf
