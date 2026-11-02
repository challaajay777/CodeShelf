#pragma once

#include <QObject>
#include <QTest>
#include "core/ContentHash.h"

namespace CodeShelf {

class TestContentHash : public QObject {
    Q_OBJECT

private slots:
    void testExactHashConsistency();
    void testNormalizedHashDifferentLineEndings();
    void testNormalizedHashTrailingSpaces();
};

} // namespace CodeShelf
