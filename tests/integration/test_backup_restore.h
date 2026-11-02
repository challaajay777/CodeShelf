#pragma once

#include <QObject>
#include <QTest>
#include "database/Database.h"
#include "services/BackupService.h"

namespace CodeShelf {

class TestBackupRestore : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testWalSafeOnlineBackupAndRestore();
    void testRestoreInvalidFile();
};

} // namespace CodeShelf
