#pragma once

#include <QObject>
#include <QTest>
#include "database/Database.h"
#include "database/Migrations.h"

namespace CodeShelf {

class TestMigrations : public QObject {
    Q_OBJECT

private slots:
    void testFreshDatabaseMigration();
    void testMigrationIdempotence();
    void testNewerVersionRejection();
};

} // namespace CodeShelf
