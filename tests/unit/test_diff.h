#pragma once

#include <QObject>
#include <QTest>
#include "core/DiffGenerator.h"

namespace CodeShelf {

class TestDiff : public QObject {
    Q_OBJECT

private slots:
    void testIdenticalDiff();
    void testAdditionsAndDeletions();
    void testEmptyOldText();
    void testEmptyNewText();
};

} // namespace CodeShelf
