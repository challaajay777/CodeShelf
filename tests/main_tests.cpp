#include <QApplication>
#include <QTest>
#include <iostream>

#include "unit/test_validation.h"
#include "unit/test_search_parser.h"
#include "unit/test_content_hash.h"
#include "unit/test_diff.h"
#include "unit/test_duplicate.h"
#include "unit/test_import_export.h"
#include "integration/test_database.h"
#include "integration/test_migrations.h"
#include "integration/test_backup_restore.h"
#include "gui/test_gui.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    int status = 0;

    std::cout << "\n=========================================\n";
    std::cout << " RUNNING CODESHELF AUTOMATED TEST SUITE \n";
    std::cout << "=========================================\n\n";

    auto runSuite = [&](const char* name, QObject* testObj) {
        std::cout << "--- Suite: " << name << " ---\n" << std::flush;
        QString logFile = QStringLiteral("result_%1.txt").arg(name);
        QStringList testArgs = {QString::fromUtf8(argv[0]), QStringLiteral("-o"), QStringLiteral("%1,txt").arg(logFile)};
        int res = QTest::qExec(testObj, testArgs);
        std::cout << "Result: " << (res == 0 ? "PASSED" : "FAILED") << " (code " << res << ")\n" << std::flush;
        if (res != 0) {
            QFile f(logFile);
            if (f.open(QIODevice::ReadOnly)) {
                std::cout << f.readAll().toStdString() << "\n" << std::flush;
            }
        }
        std::cout << "\n" << std::flush;
        return res;
    };

    { CodeShelf::TestValidation test; status |= runSuite("TestValidation", &test); }
    { CodeShelf::TestSearchParser test; status |= runSuite("TestSearchParser", &test); }
    { CodeShelf::TestContentHash test; status |= runSuite("TestContentHash", &test); }
    { CodeShelf::TestDiff test; status |= runSuite("TestDiff", &test); }
    { CodeShelf::TestDuplicate test; status |= runSuite("TestDuplicate", &test); }
    { CodeShelf::TestImportExport test; status |= runSuite("TestImportExport", &test); }
    { CodeShelf::TestDatabase test; status |= runSuite("TestDatabase", &test); }
    { CodeShelf::TestMigrations test; status |= runSuite("TestMigrations", &test); }
    { CodeShelf::TestBackupRestore test; status |= runSuite("TestBackupRestore", &test); }
    { CodeShelf::TestGui test; status |= runSuite("TestGui", &test); }

    std::cout << "\n=========================================\n";
    std::cout << " ALL SUITES FINISHED - EXIT STATUS: " << status << "\n";
    std::cout << "=========================================\n";

    return status;
}
