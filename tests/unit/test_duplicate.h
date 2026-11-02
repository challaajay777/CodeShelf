#pragma once

#include <QObject>
#include <QTest>
#include "services/DuplicateService.h"
#include "repositories/SnippetRepository.h"
#include "database/Database.h"

namespace CodeShelf {

class TestDuplicate : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testExactDuplicateDetection();
    void testNormalizedDuplicateDetection();
    void testNonDuplicate();

private:
    Database& m_db{Database::instance()};
};

} // namespace CodeShelf
