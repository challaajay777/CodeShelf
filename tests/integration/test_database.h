#pragma once

#include <QObject>
#include <QTest>
#include "database/Database.h"

namespace CodeShelf {

class TestDatabase : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testForeignKeysEnforced();
    void testTransactionCommitAndRollback();
    void testSnippetCrudAndSoftDelete();
    void testCollectionOperations();
    void testTagOperations();
    void testIntegrityCheck();
};

} // namespace CodeShelf
